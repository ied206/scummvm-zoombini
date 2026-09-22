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

class DialogBase;
class DialogHelp;
enum class DialogMsgBoxButton;

/** Interactive screens, including menus, maps, shelters, and puzzles. */
class InteractiveBase : public PageBase {
public:
	/** Bind an interactive page to @p vm, recording @p category. */
	explicit InteractiveBase(Zoombini2Engine *vm, PageCategory category = PageCategory::kInteractive);

protected:
	/** Shared map soundtrack used by interactive pages that present the route map. */
	static constexpr const char *kMapMusicPath = "#sounds/music/ZMR-MapScreen.wav";
	/** Start this page's map soundtrack. */
	void startMapMusic() { startPageMusic(Common::Path(kMapMusicPath)); }
};

/**
 * Manages shared Help, Map, and Go controls shown beside gameplay pages.
 *
 * The group uses the current pointer position for normal hover and the
 * position paired with a pending release for that frame's hit test.
 * It also retains the help overlay, per-frame hover results, and saved screen
 * region needed to draw those controls. Map-return confirmation uses the
 * shared engine confirmation dialog.
 */
class Sidebar : public PageEventHandler {
public:
	/** Bind sidebar policy and resources to @p vm. */
	explicit Sidebar(Zoombini2Engine *vm);
	/** Release help state and the saved background; button sprites remain in the graphics shared cache. */
	~Sidebar();
	/** Poll input, consume one release, and paint visible controls or their active dialog. */
	void drawAndHandleInput(ManagedSurface32 *screen, bool inputAllowed);
	/** Arm a sidebar control release or route a press to the active Help overlay. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Queue a sidebar control release or route it to the active Help overlay. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Preserve normal event routing while the per-frame draw pass updates sidebar hover state. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Route key presses to the active Help overlay. */
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	/** Route key releases to the active Help overlay when one is present. */
	EventHandleResult onKeyUp(const Common::KeyState &key) override;
	/** Return whether the active page exposes the sidebar. */
	bool shouldShow() const;
	/** Return whether the help overlay currently handles input exclusively. */
	bool hasActiveDialog() const;
	/** Return the help overlay managed by the sidebar. */
	DialogHelp *getHelpScreen() { return _helpScreen; }
	/** Restart the Go-button attention blink with the original toggle count and deadline. */
	void restartGoBlink();

private:
	/** Normal Help control sprite. */
	static constexpr const char *kHelpNormalPath = "Bmp/BARRE/QUOI.RB";
	/** Highlighted Help control sprite. */
	static constexpr const char *kHelpHighlightPath = "Bmp/BARRE/QUOIROLL.RB";
	/** Normal Map control sprite. */
	static constexpr const char *kMapNormalPath = "Bmp/BARRE/PATH.RB";
	/** Highlighted Map control sprite. */
	static constexpr const char *kMapHighlightPath = "Bmp/BARRE/PATHROLL.RB";
	/** Normal enabled Go control sprite. */
	static constexpr const char *kGoNormalPath = "Bmp/BARRE/Next.rb";
	/** Highlighted Go control sprite. */
	static constexpr const char *kGoHighlightPath = "Bmp/BARRE/NextRoll.rb";
	/** Disabled Go control sprite. */
	static constexpr const char *kGoDisabledPath = "Bmp/BARRE/NextInvisible.rb";
	/** Sound played when the Help control opens its overlay. */
	static constexpr const char *kHelpClickSoundPath = "sounds/fx/I-BS2.wav";
	/** Sound played when the Map control begins map-return handling. */
	static constexpr const char *kMapClickSoundPath = "sounds/fx/I-BS1.wav";
	/** Confirmation panel shown before abandoning active gameplay for the map. */
	static constexpr const char *kAbandonConfirmationPath = "bmp/menu/Quit_panel_text_abandon";

	/** Return the active modal overlay, or nullptr. */
	DialogBase *getActiveDialog() const;
	/** Open or close the help overlay. */
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
	/** Help overlay managed by the sidebar. */
	DialogHelp *_helpScreen = nullptr;

	/** Shared Help click sound registered with the sound manager. */
	int _helpClickSoundId = -1;
	/** Shared Map click sound registered with the sound manager. */
	int _mapClickSoundId = -1;

	/** Help button hit rectangle. */
	Common::Rect _helpButtonRect = Common::Rect(5, 480, 39, 514);
	/** Map button hit rectangle. */
	Common::Rect _mapButtonRect = Common::Rect(5, 514, 39, 548);
	/** Go button hit rectangle. */
	Common::Rect _goButtonRect = Common::Rect(5, 548, 39, 582);

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
