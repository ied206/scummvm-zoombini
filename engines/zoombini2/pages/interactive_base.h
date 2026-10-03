/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ZOOMBINI2_PAGES_INTERACTIVE_BASE_H
#define ZOOMBINI2_PAGES_INTERACTIVE_BASE_H

#include "zoombini2/pages/page_base.h"

namespace Zoombini2 {

enum class DialogMsgBoxButton;

/** Interactive screens, including menus, maps, shelters, and puzzles. */
class InteractiveBase : public PageBase {
public:
	/** Bind an interactive page to @p vm, recording @p category. */
	explicit InteractiveBase(Zoombini2Engine *vm, PageCategory category = PageCategory::kInteractive);

protected:
	/** Looped route-map music used by the World Map and menu pages. */
	static constexpr const char *kMapMusicPath = "#sounds/music/ZMR-MapScreen.wav";
	/** Start this page's map soundtrack. */
	void startMapMusic() { startPageMusic(Common::Path(kMapMusicPath)); }
};

/**
 * Manages shared Help, Map, and Go controls shown beside gameplay pages.
 *
 * The group uses the current pointer position for normal hover and the
 * position paired with a pending release for that frame's hit test.
 * It retains per-frame hover results and the saved screen region needed to
 * draw those controls. Help and Map confirmation use the
 * shared engine confirmation dialog.
 */
class Sidebar : public PageEventHandler, public Common::NonCopyable {
public:
	/** Bind sidebar policy and resources to @p vm. */
	explicit Sidebar(Zoombini2Engine *vm);
	/** Release the saved background; button sprites remain in the graphics shared cache. */
	~Sidebar();
	/** Poll input, consume one release, and paint visible controls or their active dialog. */
	void drawAndHandleInput(ManagedSurface32 *screen, bool inputAllowed);
	/** Arm a sidebar control release. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Queue a sidebar control release. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Return whether the active page exposes the sidebar. */
	bool shouldShow() const;
	/** Paint controls above the active Help dialog without accepting input. */
	void drawOverDialog(ManagedSurface32 *screen);
	/** Restart the Go-button attention blink with the original toggle count and deadline. */
	void restartGoBlink();

private:
	/** RLE sprite drawn for the idle Help control in the sidebar. */
	static constexpr const char *kHelpNormalPath = "Bmp/BARRE/QUOI.RB";
	/** RLE sprite drawn while the pointer hovers over the Help control. */
	static constexpr const char *kHelpHighlightPath = "Bmp/BARRE/QUOIROLL.RB";
	/** RLE sprite drawn for the idle Map control in the sidebar. */
	static constexpr const char *kMapNormalPath = "Bmp/BARRE/PATH.RB";
	/** RLE sprite drawn while the pointer hovers over the Map control. */
	static constexpr const char *kMapHighlightPath = "Bmp/BARRE/PATHROLL.RB";
	/** RLE sprite drawn for the enabled Go control when it is idle. */
	static constexpr const char *kGoNormalPath = "Bmp/BARRE/Next.rb";
	/** RLE sprite drawn for the Go control while hovered or during its attention blink. */
	static constexpr const char *kGoHighlightPath = "Bmp/BARRE/NextRoll.rb";
	/** RLE sprite drawn when the active page exposes Go but cannot use it yet. */
	static constexpr const char *kGoDisabledPath = "Bmp/BARRE/NextInvisible.rb";
	/** Sound effect played when the Help control opens the help overlay. */
	static constexpr const char *kHelpClickSoundPath = "sounds/fx/I-BS2.wav";
	/** Sound effect played when the Map control begins its return-to-map handling. */
	static constexpr const char *kMapClickSoundPath = "sounds/fx/I-BS1.wav";
	/** Bit-block message shown when returning to the map requires confirmation to abandon gameplay. */
	static constexpr const char *kAbandonConfirmationPath = "bmp/menu/Quit_panel_text_abandon";

	/** Open the help overlay. */
	void onHelpClick();
	/** Request a safe return to the map. */
	void onMapClick();
	/** Invoke the active page's Go action. */
	void onGoClick();
	/** Request the shared confirmation shown before abandoning active gameplay. */
	void requestAbandonConfirmation();
	/** Apply the result of the shared abandonment confirmation. */
	void handleAbandonConfirmation(DialogMsgBoxButton button);
	/** Save when required and return to the correct map mode. */
	void returnToMap();
	/** Advance or clear the explicitly requested Go-button attention blink for @p pageId. */
	void updateGoBlink(bool goEnabled, PageId pageId);
	/** Return whether @p pos is strictly inside @p rect. */
	static bool isPointStrictlyInside(const Common::Rect &rect, const Common::Point &pos);
	/** Return whether @p pos is in a fixed sidebar control region. */
	bool isInButtonRegion(const Common::Point &pos) const;
	/** Return whether a drag or page-local state must receive pointer events before the sidebar. */
	bool isInteractionBlocked() const;
	/** Update hover state from the pointer position used for this frame's controls. */
	void updateHoverState(const Common::Point &pos, bool inputAllowed);
	/** Paint the Help, Map, and Go controls over @p screen. */
	void drawControls(ManagedSurface32 *screen);
	/** Consume the release published by the input dispatcher. */
	void consumePendingRelease();

	/** Borrowed engine interface used by these controls. */
	Zoombini2Engine *_vm;

	/** Shared Help click sound registered with the sound manager. */
	int _helpClickSoundId = -1;
	/** Shared Map click sound registered with the sound manager. */
	int _mapClickSoundId = -1;

	/** Help button hit rectangle. */
	const Common::Rect _helpButtonRect = Common::Rect(5, 480, 39, 514);
	/** Map button hit rectangle. */
	const Common::Rect _mapButtonRect = Common::Rect(5, 514, 39, 548);
	/** Go button hit rectangle. */
	const Common::Rect _goButtonRect = Common::Rect(5, 548, 39, 582);

	/** Whether the pointer is over the Help button. */
	bool _helpHovered = false;
	/** Whether the pointer is over the Map button. */
	bool _mapHovered = false;
	/** Whether the pointer is over the Go button. */
	bool _goHovered = false;
	/** Whether a primary-button down event has armed a matching release. */
	bool _primaryButtonArmed = false;
	/** Whether a primary-button release awaits this frame's sidebar hit test. */
	bool _pendingMouseRelease = false;
	/** Pointer position paired with the pending primary-button release. */
	Common::Point _pendingMouseReleasePos;
	/** Whether the attention blink currently uses the highlighted sprite. */
	bool _goBlinkHighlighted = false;
	/** Number of attention-blink transitions still pending. */
	int _goBlinkTogglesRemaining = 0;
	/** Page identifier associated with the current Go-button blink state. */
	PageId _goPageId = kPageNone;
	/** Gameplay tick at which the next attention-blink transition occurs. */
	uint32 _goBlinkDeadline = 0;

	/** Saved screen region beneath the sidebar. */
	ManagedSurface32 *_savedBackground = nullptr;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_INTERACTIVE_BASE_H
