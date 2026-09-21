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

#include "zoombini2/pages/dialog_help.h"
#include "zoombini2/graphics.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

#include "common/file.h"
#include "common/system.h"

namespace Zoombini2 {

constexpr const char *DialogHelp::kHelpFramePath;
constexpr const char *DialogHelp::kPlaceholderPath;
constexpr const char *DialogHelp::kOkButtonNormalPath;
constexpr const char *DialogHelp::kOkButtonPushedPath;
constexpr const char *DialogHelp::kLeftArrowNormalPath;
constexpr const char *DialogHelp::kLeftArrowEmptyPath;
constexpr const char *DialogHelp::kRightArrowNormalPath;
constexpr const char *DialogHelp::kRightArrowEmptyPath;
constexpr const char *DialogHelp::kHelpPageFormat;

DialogHelp::DialogHelp(Zoombini2Engine *vm)
	: DialogBase(vm) {

	// Load help screen UI elements
	_vm->_gfx->loadSharedRleBlock(kHelpFramePath);
	_vm->_gfx->loadSharedRleBlock(kPlaceholderPath);

	_vm->_gfx->loadSharedBitBlock(kOkButtonNormalPath);
	_vm->_gfx->loadSharedBitBlock(kOkButtonPushedPath);

	_vm->_gfx->loadSharedBitBlock(kLeftArrowNormalPath);
	_vm->_gfx->loadSharedBitBlock(kLeftArrowEmptyPath);

	_vm->_gfx->loadSharedBitBlock(kRightArrowNormalPath);
	_vm->_gfx->loadSharedBitBlock(kRightArrowEmptyPath);

	// Create the saved screen buffer in the current game screen format.
	_savedScreen = _vm->_gfx->createSurface(ManagedSurface32::kScreenSize);
}

DialogHelp::~DialogHelp() {
	close();

	delete _savedScreen;
}

const char *DialogHelp::getLevelString(int level) {
	switch (level) {
	case 1:
		return "easy";
	case 2:
		return "medium";
	case 3:
		return "hard";
	default:
		return "easy";
	}
}

bool DialogHelp::isPageValid(int puzzleId, int level, int page) {
	// Construct help page path
	Common::String path = Common::String::format(kHelpPageFormat, puzzleId, getLevelString(level), page);

	// Check if file exists in archive
	return _vm->hasResource(path);
}

bool DialogHelp::open(int puzzleId, int level) {
	if (_isActive) {
		return false; // Already open
	}

	// Check if page 1 exists
	if (!isPageValid(puzzleId, level, 1)) {
		debug("No help available for puzzle %d, level %d", puzzleId, level);
		return false;
	}

	_isActive = true;
	_currentPuzzleId = puzzleId;
	_currentLevel = level;
	_currentPage = 1;

	_vm->setDialogPaused(true);

	// Save current screen
	_vm->_gfx->captureScreen(_savedScreen);

	// Load first help page
	if (!loadPage(puzzleId, level, 1)) {
		// Failed to load - close and return
		close();
		return false;
	}

	return true;
}

void DialogHelp::close() {
	if (!_isActive) {
		return;
	}

	_isActive = false;

	// Free loaded help page
	freePage();

	// Restore saved screen
	_vm->_gfx->copyToScreen(*_savedScreen);

	_vm->setDialogPaused(false);

	_currentPuzzleId = -1;
	_currentLevel = -1;
	_currentPage = 1;
}

bool DialogHelp::loadPage(int puzzleId, int level, int page) {
	// Free existing page
	freePage();

	// Construct help page path
	Common::String path = Common::String::format(kHelpPageFormat, puzzleId, getLevelString(level), page);

	// Load help page
	if (!_vm->_gfx->loadPageBitBlock(path))
		return false;
	_helpPagePath = path;
	return true;
}

void DialogHelp::freePage() {
	_helpPagePath.clear();
}

void DialogHelp::onRenderContent(ManagedSurface32 *screen) {
	if (!_isActive) {
		return;
	}

	screen->copyFrom(*_savedScreen);

	// Draw help frame overlay (darkened background)
	_vm->_gfx->drawSharedRleBlock(screen, kHelpFramePath, Common::Point32(0, 0));

	// Draw the sheet inside the help frame.
	if (!_helpPagePath.empty()) {
		_vm->_gfx->drawPageBitBlock(screen, _helpPagePath, Common::Point32(135, 191));
	} else {
		// Show placeholder if no help page loaded
		_vm->_gfx->drawSharedRleBlock(screen, kPlaceholderPath, Common::Point32(0, 0));
	}

	// Draw OK button
	const char *okPath = kOkButtonNormalPath;
	if (_okButtonHovered && _vm->_gfx->loadSharedBitBlock(kOkButtonPushedPath))
		okPath = kOkButtonPushedPath;
	_vm->_gfx->drawSharedBitBlock(screen, okPath, Common::Point32(_okButtonRect.left, _okButtonRect.top));

	// Draw left arrow
	bool leftEnabled = isPageValid(_currentPuzzleId, _currentLevel, _currentPage - 1);
	const char *leftPath = kLeftArrowEmptyPath;
	if (leftEnabled && _vm->_gfx->loadSharedBitBlock(kLeftArrowNormalPath))
		leftPath = kLeftArrowNormalPath;
	_vm->_gfx->drawSharedBitBlock(screen, leftPath, Common::Point32(_leftArrowRect.left, _leftArrowRect.top));

	// Draw right arrow
	bool rightEnabled = isPageValid(_currentPuzzleId, _currentLevel, _currentPage + 1);
	const char *rightPath = kRightArrowEmptyPath;
	if (rightEnabled && _vm->_gfx->loadSharedBitBlock(kRightArrowNormalPath))
		rightPath = kRightArrowNormalPath;
	_vm->_gfx->drawSharedBitBlock(screen, rightPath, Common::Point32(_rightArrowRect.left, _rightArrowRect.top));
}

EventHandleResult DialogHelp::onMouseMove(const Common::Point &pos) {
	if (!_isActive) {
		return EventHandleResult::kPassthrough;
	}

	_okButtonHovered = _okButtonRect.contains(pos);
	_leftArrowHovered = _leftArrowRect.contains(pos);
	_rightArrowHovered = _rightArrowRect.contains(pos);
	return EventHandleResult::kConsumed;
}

EventHandleResult DialogHelp::onLButtonDown(const Common::Point &pos) {
	if (!_isActive) {
		return EventHandleResult::kPassthrough;
	}

	// OK button - close help
	if (_okButtonRect.contains(pos)) {
		close();
		return EventHandleResult::kConsumed;
	}

	// Left arrow - previous page
	if (_leftArrowRect.contains(pos)) {
		if (isPageValid(_currentPuzzleId, _currentLevel, _currentPage - 1)) {
			if (loadPage(_currentPuzzleId, _currentLevel, _currentPage - 1)) {
				_currentPage -= 1;
			}
		}
		return EventHandleResult::kConsumed;
	}

	// Right arrow - next page
	if (_rightArrowRect.contains(pos)) {
		if (isPageValid(_currentPuzzleId, _currentLevel, _currentPage + 1)) {
			if (loadPage(_currentPuzzleId, _currentLevel, _currentPage + 1)) {
				_currentPage += 1;
			}
		}
		return EventHandleResult::kConsumed;
	}

	// Clicks outside buttons retain the active modal and do not reach its page.
	return EventHandleResult::kPassthrough;
}

} // End of namespace Zoombini2
