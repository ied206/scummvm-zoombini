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
#include "graphics/managed_surface.h"

namespace Zoombini2 {

DialogHelp::DialogHelp(Zoombini2Engine *vm)
	: DialogBase(vm), _isActive(false), _currentPuzzleId(-1),
	  _currentLevel(-1), _currentPage(1), _savedScreen(nullptr),
	  _helpFrame(nullptr), _placeholder(nullptr),
	  _okButtonNormal(nullptr), _okButtonPushed(nullptr),
	  _leftArrowNormal(nullptr), _leftArrowEmpty(nullptr),
	  _rightArrowNormal(nullptr), _rightArrowEmpty(nullptr),
	  _helpPage(nullptr), _okButtonHovered(false),
	  _leftArrowHovered(false), _rightArrowHovered(false),
	  _pauseStartTime(0) {

	// Initialize the three bottom-panel button hitboxes.
	_okButtonRect = Common::Rect(597, 400, 671, 444);   // 74x44
	_leftArrowRect = Common::Rect(135, 400, 181, 444);  // 46x44
	_rightArrowRect = Common::Rect(209, 400, 253, 444); // 44x44

	// Load help screen UI elements
	_helpFrame = _vm->loadRleBlock("Bmp/MENU/help_screen_main.rb");
	_placeholder = _vm->loadRleBlock("Bmp/MENU/help_screen_placeholder.rb");

	_okButtonNormal = _vm->loadBitBlock("Bmp/MENU/help_screen_okbutton_normal.bb");
	_okButtonPushed = _vm->loadBitBlock("Bmp/MENU/help_screen_okbutton_pushed.bb");

	_leftArrowNormal = _vm->loadBitBlock("Bmp/MENU/help_screen_leftarro_norma.bb");
	_leftArrowEmpty = _vm->loadBitBlock("Bmp/MENU/help_screen_leftarro_empty.bb");

	_rightArrowNormal = _vm->loadBitBlock("Bmp/MENU/help_screen_rightarro_norma.bb");
	_rightArrowEmpty = _vm->loadBitBlock("Bmp/MENU/help_screen_rightarro_empty.bb");

	// Create saved screen buffer
	// Must match screen format to avoid assert in copyRectToSurface
	_savedScreen = new Graphics::ManagedSurface(kScreenWidth, kScreenHeight,
												_vm->getCurrentScreen()->format);
}

DialogHelp::~DialogHelp() {
	close();

	delete _helpFrame;
	delete _placeholder;
	delete _okButtonNormal;
	delete _okButtonPushed;
	delete _leftArrowNormal;
	delete _leftArrowEmpty;
	delete _rightArrowNormal;
	delete _rightArrowEmpty;
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
	Common::String path = Common::String::format("Bmp/help/%02d_help_%s_%02d.bb",
												 puzzleId,
												 getLevelString(level),
												 page);

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

	// Record pause start time
	_pauseStartTime = g_system->getMillis();
	_vm->_isPaused = true;
	_vm->_pauseTimeStart = _pauseStartTime;

	// Pause audio
	_vm->getSoundManager()->pauseAll();

	// Save current screen
	_savedScreen->copyFrom(*_vm->getCurrentScreen());

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
	_vm->getCurrentScreen()->copyFrom(*_savedScreen);

	// Resume audio
	_vm->getSoundManager()->resumeAll();

	// Update pause accumulator (for accurate timing)
	uint32 pauseDuration = g_system->getMillis() - _pauseStartTime;
	_vm->addPauseTime(pauseDuration);
	_vm->_isPaused = false;

	_currentPuzzleId = -1;
	_currentLevel = -1;
	_currentPage = 1;
}

bool DialogHelp::loadPage(int puzzleId, int level, int page) {
	// Free existing page
	freePage();

	// Construct help page path
	Common::String path = Common::String::format("Bmp/help/%02d_help_%s_%02d.bb",
												 puzzleId,
												 getLevelString(level),
												 page);

	// Load help page
	_helpPage = _vm->loadBitBlock(path);

	return (_helpPage != nullptr);
}

void DialogHelp::freePage() {
	delete _helpPage;
	_helpPage = nullptr;
}

void DialogHelp::onRenderScene(ManagedSurface32 *screen) {
	if (!_isActive) {
		return;
	}

	// Get alpha LUT for RleBlock rendering
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();
	screen->copyFrom(*_savedScreen);

	// Draw help frame overlay (darkened background)
	if (_helpFrame) {
		_helpFrame->drawToScreen(screen, Common::Point32(0, 0), lut);
	}

	// Draw the sheet inside the help frame.
	if (_helpPage) {
		_helpPage->drawToSurface(screen, Common::Point32(135, 191));
	} else if (_placeholder) {
		// Show placeholder if no help page loaded
		_placeholder->drawToScreen(screen, Common::Point32(0, 0), lut);
	}

	// Draw OK button
	if (_okButtonHovered && _okButtonPushed) {
		_okButtonPushed->drawToSurface(screen, Common::Point32(_okButtonRect.left, _okButtonRect.top));
	} else if (_okButtonNormal) {
		_okButtonNormal->drawToSurface(screen, Common::Point32(_okButtonRect.left, _okButtonRect.top));
	}

	// Draw left arrow
	bool leftEnabled = isPageValid(_currentPuzzleId, _currentLevel, _currentPage - 1);
	if (leftEnabled && _leftArrowNormal) {
		_leftArrowNormal->drawToSurface(screen, Common::Point32(_leftArrowRect.left, _leftArrowRect.top));
	} else if (_leftArrowEmpty) {
		_leftArrowEmpty->drawToSurface(screen, Common::Point32(_leftArrowRect.left, _leftArrowRect.top));
	}

	// Draw right arrow
	bool rightEnabled = isPageValid(_currentPuzzleId, _currentLevel, _currentPage + 1);
	if (rightEnabled && _rightArrowNormal) {
		_rightArrowNormal->drawToSurface(screen, Common::Point32(_rightArrowRect.left, _rightArrowRect.top));
	} else if (_rightArrowEmpty) {
		_rightArrowEmpty->drawToSurface(screen, Common::Point32(_rightArrowRect.left, _rightArrowRect.top));
	}
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
