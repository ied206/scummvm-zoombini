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

class BitBlock;
class DialogBase;
class DialogHelp;
class RleBlock;

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
 * It also owns the help overlay, map-return confirmation, hover state, and
 * the saved screen regions needed to draw those controls.
 */
class ThreeButtons : public PageEventHandler {
public:
	/** Bind three-button policy and resources to @p vm. */
	explicit ThreeButtons(Zoombini2Engine *vm);
	/** Release button resources, dialog state, and saved backgrounds. */
	~ThreeButtons();
	/** Poll input, consume one release, and paint visible controls or their active dialog. */
	void drawAndHandleInput(ManagedSurface32 *screen, bool inputAllowed);
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	EventHandleResult onKeyUp(const Common::KeyState &key) override;
	/** Return whether the active page exposes the three-button controls. */
	bool shouldShow() const;
	/** Return whether the map-return confirmation is active. */
	bool isSaveConfirmationActive() const { return _confirmActive; }
	/** Return whether help or the map-return confirmation currently owns input. */
	bool hasActiveDialog() const;
	/** Return the help overlay managed by the three-button controls. */
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
	/** Open the confirmation shown before leaving unsaved gameplay. */
	void openSaveConfirmation();
	/** Close the map-return confirmation and restore its saved background. */
	void closeSaveConfirmation();
	/** Draw the active map-return confirmation. */
	void drawSaveConfirmation(ManagedSurface32 *screen);
	/** Return the confirmation button under @p pos, or the neutral state. */
	int hitTestSaveConfirmation(const Common::Point &pos) const;
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
	/** Help overlay managed by the three-button controls. */
	DialogHelp *_helpScreen;

	/** Help button's normal sprite. */
	RleBlock *_helpNormal;
	/** Help button's highlighted sprite. */
	RleBlock *_helpHighlight;
	/** Map button's normal sprite. */
	RleBlock *_mapNormal;
	/** Map button's highlighted sprite. */
	RleBlock *_mapHighlight;
	/** Go button's normal sprite. */
	RleBlock *_goNormal;
	/** Go button's highlighted sprite. */
	RleBlock *_goHighlight;
	/** Go button's disabled sprite. */
	RleBlock *_goDisabled;
	/** Shared Help click sound registered with the sound manager. */
	int _helpClickSoundId;
	/** Shared Map click sound registered with the sound manager. */
	int _mapClickSoundId;

	/** Help button hit rectangle. */
	Common::Rect _helpButtonRect;
	/** Map button hit rectangle. */
	Common::Rect _mapButtonRect;
	/** Go button hit rectangle. */
	Common::Rect _goButtonRect;

	/** Whether the pointer is over the Help button. */
	bool _helpHovered;
	/** Whether the pointer is over the Map button. */
	bool _mapHovered;
	/** Whether the pointer is over the Go button. */
	bool _goHovered;
	/** Whether a primary-button down event has armed a matching release. */
	bool _primaryButtonArmed;
	/** Whether a primary-button release awaits this frame's sidebar hit test. */
	bool _pendingMouseRelease;
	/** Go-enabled value observed during the previous draw. */
	bool _goWasEnabled;
	/** Whether the attention blink currently uses the highlighted sprite. */
	bool _goBlinkHighlighted;
	/** Number of attention-blink transitions still pending. */
	int _goBlinkTogglesRemaining;
	/** Page identifier associated with the current Go-button blink state. */
	int _goPageId;
	/** Gameplay tick at which the next attention-blink transition occurs. */
	uint32 _goBlinkDeadline;

	/** Saved screen region beneath the three-button controls. */
	Graphics::ManagedSurface *_savedBackground;
	/** Saved screen region beneath the map-return confirmation. */
	Graphics::ManagedSurface *_confirmBackground;
	/** Confirmation panel sprites indexed by neutral, accept, and cancel state. */
	RleBlock *_confirmPanels[3];
	/** Localized map-return confirmation text. */
	BitBlock *_confirmText;
	/** Whether the map-return confirmation is active. */
	bool _confirmActive;
	/** Confirmation button currently under the pointer. */
	int _confirmHover;
	/** Screen position of the map-return confirmation. */
	Common::Point32 _confirmPos;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_INTERACTIVE_BASE_H
