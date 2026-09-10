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

#include "common/rect.h"
#include "common/str.h"
#include "zoombini2/pages/dialog_base.h"

namespace Graphics {
class ManagedSurface;
}

namespace Zoombini2 {

class Zoombini2Engine;
class BitBlock;
class RleBlock;

/**
 * Owns the context-sensitive help overlay for one puzzle and level.
 *
 * Help pages resolve below `bmp/help` from the puzzle, level, and sheet
 * number. The dialog pauses gameplay, retains the underlying screen, and
 * restores both the screen and gameplay clock when it closes.
 */
class DialogHelp : public DialogBase {
public:
	/** Bind the reusable help overlay to @p vm. */
	DialogHelp(Zoombini2Engine *vm);
	/** Release the active page, controls, and saved screen. */
	~DialogHelp() override;

	/** Return whether a help sheet exists for the supplied puzzle coordinates. */
	bool isPageValid(int puzzleId, int level, int page);

	/** Save the current screen and open the first help sheet. */
	bool open(int puzzleId, int level);

	/** Close the overlay, restore the saved screen, and resume gameplay timing. */
	void close() override;

	/** Return whether this overlay currently owns drawing and input. */
	bool isActive() const override { return _isActive; }

	/** Draw the frame, current help sheet, and navigation controls. */
	void onRenderScene(ManagedSurface32 *screen) override;

	/** Handle navigation or close-button input while the modal is active. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;

	/** Update navigation-control hover state for @p pos. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;

private:
	/** Replace the active help bitmap with the requested sheet. */
	bool loadPage(int puzzleId, int level, int page);
	/** Release the active help-sheet bitmap. */
	void freePage();
	/** Return the resource-directory label for @p level. */
	const char *getLevelString(int level);

	/** Whether the help overlay is active. */
	bool _isActive;
	/** Puzzle identifier used to resolve the active help resource. */
	int _currentPuzzleId;
	/** Level used to resolve the active help resource. */
	int _currentLevel;
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
