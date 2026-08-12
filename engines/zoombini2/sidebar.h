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
class BitBlock;
class HelpScreen;

/**
 * Global sidebar UI panel
 * 
 * Provides Help/Save/Go buttons on left side of screen during puzzle pages.
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

	/**
	 * Get help screen instance (may be in active modal state)
	 */
	HelpScreen *getHelpScreen() { return _helpScreen; }

private:
	/**
	 * Handle help button click
	 */
	void onHelpClick();

	/**
	 * Handle save button click
	 */
	void onSaveClick();

	/**
	 * Handle go/quit button click
	 */
	void onGoClick();

	Zoombini2Engine *_engine;

	// Help screen modal system
	HelpScreen *_helpScreen;

	// Button graphics
	BitBlock *_helpNormal;
	BitBlock *_helpHighlight;
	BitBlock *_saveNormal;
	BitBlock *_saveHighlight;
	BitBlock *_goNormal;
	BitBlock *_goHighlight;

	// Button positions (from original @ 0x462DA0)
	Common::Rect _helpButtonRect;   // (5, 480, 39, 514)
	Common::Rect _saveButtonRect;   // (5, 514, 39, 548)
	Common::Rect _goButtonRect;     // (5, 548, 39, 582)

	// Hover states
	bool _helpHovered;
	bool _saveHovered;
	bool _goHovered;

	// Saved background under buttons (34x102 at position 5,480)
	Graphics::ManagedSurface *_savedBackground;
};

} // End of namespace Zoombini2

#endif
