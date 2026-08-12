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

#ifndef ZOOMBINI2_SIDEBAR_H
#define ZOOMBINI2_SIDEBAR_H

#include "common/rect.h"

namespace Graphics {
class ManagedSurface;
}

namespace Zoombini2 {

class Zoombini2Engine;
class RleBlock;
class BitBlock;
class HelpScreen;
class Dialog;

/**
 * Global sidebar UI panel
 * 
 * Provides Help/Map/Go buttons on left side of screen during puzzle pages.
 * Positioned at x=5 with three 34x34 buttons stacked vertically.
 */
class Sidebar {
public:
	Sidebar(Zoombini2Engine *engine);
	~Sidebar();

	/**
	 * Draw sidebar buttons (called every frame)
	 * Returns true if sidebar is visible
	 */
	bool draw(Graphics::ManagedSurface *screen);
	/**
	 * Handle mouse click on sidebar buttons
	 * Returns true if click was handled
	 */
	bool handleClick(const Common::Point &pos);
	/**
	 * Handle mouse movement for hover states
	 */
	void handleMouseMove(const Common::Point &pos);
	/**
	 * Check if sidebar should be visible for current page
	 */
	bool shouldShow() const;
	bool isSaveConfirmationActive() const { return _confirmActive; }
	bool hasActiveDialog() const;
	/**
	 * Get help screen instance (may be in active modal state)
	 */
	HelpScreen *getHelpScreen() { return _helpScreen; }

private:
	Dialog *getActiveDialog() const;
	/**
	 * Handle help button click
	 */
	void onHelpClick();
	/**
	 * Handle Map button click
	 */
	void onMapClick();
	/**
	 * Handle Go button click
	 */
	void onGoClick();
	void openSaveConfirmation();
	void closeSaveConfirmation();
	void drawSaveConfirmation(Graphics::ManagedSurface *screen);
	int hitTestSaveConfirmation(const Common::Point &pos) const;
	void returnToMap();
	void updateGoBlink(bool goEnabled, int pageId);

	Zoombini2Engine *_engine;

	// Help screen modal system
	HelpScreen *_helpScreen;

	// Button graphics
	RleBlock *_helpNormal;
	RleBlock *_helpHighlight;
	RleBlock *_mapNormal;
	RleBlock *_mapHighlight;
	RleBlock *_goNormal;
	RleBlock *_goHighlight;
	RleBlock *_goDisabled;

	// Button positions
	Common::Rect _helpButtonRect; // (5, 480, 39, 514)
	Common::Rect _mapButtonRect; // (5, 514, 39, 548)
	Common::Rect _goButtonRect; // (5, 548, 39, 582)

	// Hover states
	bool _helpHovered;
	bool _mapHovered;
	bool _goHovered;
	bool _goWasEnabled;
	bool _goBlinkHighlighted;
	int _goBlinkTogglesRemaining;
	int _goPageId;
	uint32 _goBlinkDeadline;

	// Saved background under buttons (34x102 at position 5,480)
	Graphics::ManagedSurface *_savedBackground;
	Graphics::ManagedSurface *_confirmBackground;
	RleBlock *_confirmPanels[3];
	BitBlock *_confirmText;
	bool _confirmActive;
	int _confirmHover;
	Common::Point _confirmPosition;
};

} // End of namespace Zoombini2

#endif
