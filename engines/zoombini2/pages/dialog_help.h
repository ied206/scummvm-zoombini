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
 * Owns the context-sensitive help overlay for one puzzle and difficulty.
 *
 * Help pages resolve below `bmp/help` from the puzzle, difficulty, and sheet
 * number. The dialog pauses gameplay, retains the underlying screen, and
 * restores both the screen and gameplay clock when it closes.
 */
class HelpScreen : public Dialog {
public:
	/** Bind the reusable help overlay to @p engine. */
	HelpScreen(Zoombini2Engine *engine);
	/** Release the active page, controls, and saved screen. */
	~HelpScreen() override;

	/** Return whether a help sheet exists for the supplied puzzle coordinates. */
	bool isPageValid(int puzzleId, int difficulty, int page);

	/** Save the current screen and open the first help sheet. */
	bool open(int puzzleId, int difficulty);

	/** Close the overlay, restore the saved screen, and resume gameplay timing. */
	void close() override;

	/** Return whether this overlay currently owns drawing and input. */
	bool isActive() const override { return _isActive; }

	/** Draw the frame, current help sheet, and navigation controls. */
	void draw(Graphics::ManagedSurface *screen) override;

	/** Handle navigation or close-button input and report whether it was consumed. */
	bool handleClick(const Common::Point &pos) override;

	/** Update navigation-control hover state for @p pos. */
	void handleMouseMove(const Common::Point &pos) override;

private:
	/** Replace the active help bitmap with the requested sheet. */
	bool loadPage(int puzzleId, int difficulty, int page);
	/** Release the active help-sheet bitmap. */
	void freePage();
	/** Return the resource-directory label for @p difficulty. */
	const char *getDifficultyString(int difficulty);

	/** Borrowed engine that owns this overlay. */
	Zoombini2Engine *_engine;
	/** Whether the help overlay is active. */
	bool _isActive;
	/** Puzzle identifier used to resolve the active help resource. */
	int _currentPuzzleId;
	/** Difficulty used to resolve the active help resource. */
	int _currentDifficulty;
	/** One-based help sheet currently displayed. */
	int _currentPage;
	/** Screen snapshot restored when the overlay closes. */
	Graphics::ManagedSurface *_savedScreen;
	/** Help-window frame sprite. */
	RleBlock *_helpFrame;
	/** Fallback sprite shown when no help sheet is available. */
	RleBlock *_placeholder;
	/** Normal close-button bitmap. */
	BitBlock *_okButtonNormal;
	/** Pressed close-button bitmap. */
	BitBlock *_okButtonPushed;
	/** Enabled previous-sheet button bitmap. */
	BitBlock *_leftArrowNormal;
	/** Disabled previous-sheet button bitmap. */
	BitBlock *_leftArrowEmpty;
	/** Enabled next-sheet button bitmap. */
	BitBlock *_rightArrowNormal;
	/** Disabled next-sheet button bitmap. */
	BitBlock *_rightArrowEmpty;
	/** Active help-sheet bitmap. */
	BitBlock *_helpPage;
	/** Close-button hit rectangle. */
	Common::Rect _okButtonRect;
	/** Previous-sheet button hit rectangle. */
	Common::Rect _leftArrowRect;
	/** Next-sheet button hit rectangle. */
	Common::Rect _rightArrowRect;
	/** Whether the pointer is over the close button. */
	bool _okButtonHovered;
	/** Whether the pointer is over the previous-sheet button. */
	bool _leftArrowHovered;
	/** Whether the pointer is over the next-sheet button. */
	bool _rightArrowHovered;
	/** System tick captured when the help overlay paused gameplay. */
	uint32 _pauseStartTime;
};

} // End of namespace Zoombini2

#endif
