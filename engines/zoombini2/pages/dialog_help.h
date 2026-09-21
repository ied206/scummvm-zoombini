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

namespace Zoombini2 {

class Zoombini2Engine;

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
	/** Close the active help sheet and release the saved screen. */
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
	void onRenderContent(ManagedSurface32 *screen) override;

	/** Handle navigation or close-button input while the modal is active. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;

	/** Update navigation-control hover state for @p pos. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;

private:
	/** Resource paths and formats used by the help overlay. */
	static constexpr const char *kHelpFramePath = "Bmp/MENU/help_screen_main.rb";
	static constexpr const char *kPlaceholderPath = "Bmp/MENU/help_screen_placeholder.rb";
	static constexpr const char *kOkButtonNormalPath = "Bmp/MENU/help_screen_okbutton_normal.bb";
	static constexpr const char *kOkButtonPushedPath = "Bmp/MENU/help_screen_okbutton_pushed.bb";
	static constexpr const char *kLeftArrowNormalPath = "Bmp/MENU/help_screen_leftarro_norma.bb";
	static constexpr const char *kLeftArrowEmptyPath = "Bmp/MENU/help_screen_leftarro_empty.bb";
	static constexpr const char *kRightArrowNormalPath = "Bmp/MENU/help_screen_rightarro_norma.bb";
	static constexpr const char *kRightArrowEmptyPath = "Bmp/MENU/help_screen_rightarro_empty.bb";
	static constexpr const char *kHelpPageFormat = "Bmp/help/%02d_help_%s_%02d.bb";

	/** Replace the active help-sheet path with the requested sheet. */
	bool loadPage(int puzzleId, int level, int page);
	/** Forget the active help-sheet path. */
	void freePage();
	/** Return the resource-directory label for @p level. */
	const char *getLevelString(int level);

	/** Whether the help overlay is active. */
	bool _isActive = false;
	/** Puzzle identifier used to resolve the active help resource. */
	int _currentPuzzleId = -1;
	/** Level used to resolve the active help resource. */
	int _currentLevel = -1;
	/** One-based help sheet currently displayed. */
	int _currentPage = 1;
	/** Screen snapshot restored when the overlay closes. */
	ManagedSurface32 *_savedScreen = nullptr;
	/** Path of the active help-sheet bitmap in the graphics page cache. */
	Common::String _helpPagePath;
	/** Close-button hit rectangle. */
	Common::Rect _okButtonRect = Common::Rect(597, 400, 671, 444);
	/** Previous-sheet button hit rectangle. */
	Common::Rect _leftArrowRect = Common::Rect(135, 400, 181, 444);
	/** Next-sheet button hit rectangle. */
	Common::Rect _rightArrowRect = Common::Rect(209, 400, 253, 444);
	/** Whether the pointer is over the close button. */
	bool _okButtonHovered = false;
	/** Whether the pointer is over the previous-sheet button. */
	bool _leftArrowHovered = false;
	/** Whether the pointer is over the next-sheet button. */
	bool _rightArrowHovered = false;
	/** System tick captured when the help overlay paused gameplay. */
};

} // End of namespace Zoombini2

#endif
