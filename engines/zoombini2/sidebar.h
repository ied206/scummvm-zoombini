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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ZOOMBINI2_SIDEBAR_H
#define ZOOMBINI2_SIDEBAR_H

#include "common/rect.h"
#include "common/scummsys.h"

namespace Graphics {
class ManagedSurface;
}

namespace Zoombini2 {

class BitBlock;
class Dialog;
class HelpScreen;
class RleBlock;
class Zoombini2Engine;

/**
 * Owns the global Help, Map, and Go controls shown beside gameplay pages.
 *
 * The sidebar selects visibility and Go-button policy from the active page.
 * It also owns the help overlay, map-return confirmation, hover state, and
 * the saved screen regions needed to draw those controls.
 */
class Sidebar {
public:
	/** Bind sidebar policy and resources to @p engine. */
	explicit Sidebar(Zoombini2Engine *engine);
	/** Release button resources, dialog state, and saved backgrounds. */
	~Sidebar();

	/** Draw visible controls and return whether the sidebar was presented. */
	bool draw(Graphics::ManagedSurface *screen);
	/** Route @p pos to an active dialog or sidebar button. */
	bool handleClick(const Common::Point &pos);
	/** Update hover state for @p pos. */
	void handleMouseMove(const Common::Point &pos);
	/** Return whether the active page exposes the sidebar. */
	bool shouldShow() const;
	/** Return whether the map-return confirmation is active. */
	bool isSaveConfirmationActive() const { return _confirmActive; }
	/** Return whether help or the map-return confirmation currently owns input. */
	bool hasActiveDialog() const;
	/** Return the engine-owned help overlay. */
	HelpScreen *getHelpScreen() { return _helpScreen; }

private:
	/** Return the active modal overlay, or nullptr. */
	Dialog *getActiveDialog() const;
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
	void drawSaveConfirmation(Graphics::ManagedSurface *screen);
	/** Return the confirmation button under @p pos, or the neutral state. */
	int hitTestSaveConfirmation(const Common::Point &pos) const;
	/** Save when required and return to the correct map mode. */
	void returnToMap();
	/** Start or advance the Go-button attention blink for @p pageId. */
	void updateGoBlink(bool goEnabled, int pageId);

	/** Borrowed engine that owns this sidebar. */
	Zoombini2Engine *_engine;
	/** Owned help overlay. */
	HelpScreen *_helpScreen;

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

	/** Saved screen region beneath the sidebar buttons. */
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
	Common::Point _confirmPosition;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_SIDEBAR_H
