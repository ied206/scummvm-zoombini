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

#ifndef ZOOMBINI2_PAGES_DIALOG_HELP_H
#define ZOOMBINI2_PAGES_DIALOG_HELP_H

#include "common/str.h"
#include "common/rect.h"
#include "zoombini2/pages/dialog_base.h"

namespace Graphics {
class ManagedSurface;
}

namespace Zoombini2 {

class Zoombini2Engine;
class BitBlock;
class RleBlock;

/**
 * Modal help screen system
 * 
 * Provides context-sensitive help for each puzzle/difficulty level.
 * Help pages are files under bmp/help with page, difficulty, and sheet names.
 */
class HelpScreen : public Dialog {
public:
	HelpScreen(Zoombini2Engine *engine);
	~HelpScreen() override;

	/**
	 * Check if a help page exists for the given parameters
	 */
	bool isPageValid(int puzzleId, int difficulty, int page);

	/**
	 * Open help screen for current puzzle/difficulty
	 * Returns true if successfully opened
	 */
	bool open(int puzzleId, int difficulty);

	/**
	 * Close help screen and restore game
	 */
	void close() override;

	/**
	 * Check if help screen is currently active
	 */
	bool isActive() const override { return _isActive; }

	/**
	 * Render help screen UI (called every frame when active)
	 */
	void draw(Graphics::ManagedSurface *screen) override;

	/**
	 * Handle mouse click in help screen
	 * Returns true if click was handled
	 */
	bool handleClick(const Common::Point &pos) override;

	/**
	 * Handle mouse movement for hover states
	 */
	void handleMouseMove(const Common::Point &pos) override;

private:
	/**
	 * Load a specific help page
	 * Returns true if successfully loaded
	 */
	bool loadPage(int puzzleId, int difficulty, int page);

	/**
	 * Free loaded help page
	 */
	void freePage();

	/**
	 * Get difficulty string for file path
	 */
	const char *getDifficultyString(int difficulty);

	Zoombini2Engine *_engine;

	// Help screen state
	bool _isActive;
	int _currentPuzzleId;
	int _currentDifficulty;
	int _currentPage;

	// Saved screen
	Graphics::ManagedSurface *_savedScreen;

	// UI elements
	RleBlock *_helpFrame;      // Help screen background frame
	RleBlock *_placeholder;    // Placeholder image if no help available

	// Navigation buttons
	BitBlock *_okButtonNormal;
	BitBlock *_okButtonPushed;
	BitBlock *_leftArrowNormal;
	BitBlock *_leftArrowEmpty;
	BitBlock *_rightArrowNormal;
	BitBlock *_rightArrowEmpty;

	// Loaded help page
	BitBlock *_helpPage;

	// Button hitboxes
	Common::Rect _okButtonRect;
	Common::Rect _leftArrowRect;
	Common::Rect _rightArrowRect;

	// Hover states
	bool _okButtonHovered;
	bool _leftArrowHovered;
	bool _rightArrowHovered;

	// Pause management
	uint32 _pauseStartTime;
};

} // End of namespace Zoombini2

#endif
