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
class RleBlock;
enum class DialogMsgBoxButton;

/** Interactive screens, including menus, maps, shelters, and puzzles. */
class InteractiveBase : public PageBase {
public:
	/** Bind an interactive page to @p vm. */
	explicit InteractiveBase(Zoombini2Engine *vm) : PageBase(vm, PageCategory::kInteractive) {}
};

/**
 * Manages shared Help, Map, and Go controls shown beside gameplay pages.
 *
 * The group polls the final frame mouse state to draw, hit-test, and consume
 * one pending mouse release for its three controls.
 * It also owns the help overlay, hover state, and saved screen region needed
 * to draw those controls. Map-return confirmation uses the engine-owned
 * shared confirmation dialog.
 */
class Sidebar : public PageEventHandler {
public:
	/** Bind sidebar policy and resources to @p vm. */
	explicit Sidebar(Zoombini2Engine *vm);
	/** Release button resources, help state, and the saved background. */
	~Sidebar();
	/** Poll input, consume one release, and paint visible controls or their active dialog. */
	void drawAndHandleInput(ManagedSurface32 *screen, bool inputAllowed);
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	EventHandleResult onKeyUp(const Common::KeyState &key) override;
	/** Return whether the active page exposes the sidebar. */
	bool shouldShow() const;
	/** Return whether the help overlay currently owns input. */
	bool hasActiveDialog() const;
	/** Return the help overlay managed by the sidebar. */
	DialogHelp *getHelpScreen() { return _helpScreen; }

private:
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
	/** Start or advance the Go-button attention blink for @p pageId. */
	void updateGoBlink(bool goEnabled, int pageId);
	/** Return whether @p pos is strictly inside @p rect. */
	static bool isPointStrictlyInside(const Common::Rect &rect, const Common::Point &pos);
	/** Return whether @p pos is in a fixed sidebar control region. */
	bool isInButtonRegion(const Common::Point &pos) const;
	/** Update hover state from the final frame mouse position. */
	void updateHoverState(const Common::Point &pos, bool inputAllowed);
	/** Consume the release published by the input dispatcher. */
	void consumePendingRelease();

	/** Borrowed vm that owns these controls. */
	Zoombini2Engine *_vm;
	/** Help overlay managed by the sidebar. */
	DialogHelp *_helpScreen = nullptr;

	/** Help button's normal sprite. */
	RleBlock *_helpNormal = nullptr;
	/** Help button's highlighted sprite. */
	RleBlock *_helpHighlight = nullptr;
	/** Map button's normal sprite. */
	RleBlock *_mapNormal = nullptr;
	/** Map button's highlighted sprite. */
	RleBlock *_mapHighlight = nullptr;
	/** Go button's normal sprite. */
	RleBlock *_goNormal = nullptr;
	/** Go button's highlighted sprite. */
	RleBlock *_goHighlight = nullptr;
	/** Go button's disabled sprite. */
	RleBlock *_goDisabled = nullptr;
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
	/** Go-enabled value observed during the previous draw. */
	bool _goWasEnabled = false;
	/** Whether the attention blink currently uses the highlighted sprite. */
	bool _goBlinkHighlighted = false;
	/** Number of attention-blink transitions still pending. */
	int _goBlinkTogglesRemaining = 0;
	/** Page identifier associated with the current Go-button blink state. */
	int _goPageId = -1;
	/** Gameplay tick at which the next attention-blink transition occurs. */
	uint32 _goBlinkDeadline = 0;

	/** Saved screen region beneath the sidebar. */
	Graphics::ManagedSurface *_savedBackground = nullptr;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_INTERACTIVE_BASE_H
