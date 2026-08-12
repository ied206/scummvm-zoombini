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

#include "zoombini2/help_screen.h"
#include "zoombini2/zoombini2.h"
#include "zoombini2/gfx.h"
#include "zoombini2/sound.h"

#include "common/file.h"
#include "common/system.h"
#include "graphics/managed_surface.h"

namespace Zoombini2 {

HelpScreen::HelpScreen(Zoombini2Engine *engine) 
	: _engine(engine), _isActive(false), _currentPuzzleId(-1),
	  _currentDifficulty(-1), _currentPage(1), _savedScreen(nullptr),
	  _helpFrame(nullptr), _placeholder(nullptr),
	  _okButtonNormal(nullptr), _okButtonPushed(nullptr),
	  _leftArrowNormal(nullptr), _leftArrowEmpty(nullptr),
	  _rightArrowNormal(nullptr), _rightArrowEmpty(nullptr),
	  _helpPage(nullptr), _okButtonHovered(false),
	  _leftArrowHovered(false), _rightArrowHovered(false),
	  _pauseStartTime(0) {

	// Initialize button hitboxes (from original @ 0x464760)
	_okButtonRect = Common::Rect(597, 400, 671, 444);      // 74x44
	_leftArrowRect = Common::Rect(135, 400, 181, 444);     // 46x44
	_rightArrowRect = Common::Rect(209, 400, 253, 444);    // 44x44

	// Load help screen UI elements
	_helpFrame = _engine->loadRleBlock("Bmp/MENU/help_screen_main.rb");
	_placeholder = _engine->loadRleBlock("Bmp/MENU/help_screen_placeholder.rb");

	_okButtonNormal = _engine->loadBitBlock("Bmp/MENU/help_screen_okbutton_normal.bb");
	_okButtonPushed = _engine->loadBitBlock("Bmp/MENU/help_screen_okbutton_pushed.bb");
	
	_leftArrowNormal = _engine->loadBitBlock("Bmp/MENU/help_screen_leftarro_norma.bb");
	_leftArrowEmpty = _engine->loadBitBlock("Bmp/MENU/help_screen_leftarro_empty.bb");
	
	_rightArrowNormal = _engine->loadBitBlock("Bmp/MENU/help_screen_rightarro_norma.bb");
	_rightArrowEmpty = _engine->loadBitBlock("Bmp/MENU/help_screen_rightarro_empty.bb");

	// Create saved screen buffer
	// Must match screen format to avoid assert in copyRectToSurface
	_savedScreen = new Graphics::ManagedSurface(kScreenWidth, kScreenHeight, 
	                                             _engine->getCurrentScreen()->format);
}

HelpScreen::~HelpScreen() {
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

const char *HelpScreen::getDifficultyString(int difficulty) {
	switch (difficulty) {
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

bool HelpScreen::isPageValid(int puzzleId, int difficulty, int page) {
	// Construct help page path
	Common::String path = Common::String::format("Bmp/help/%02d_help_%s_%02d.bb",
	                                              puzzleId,
	                                              getDifficultyString(difficulty),
	                                              page);

	// Check if file exists in archive
	return _engine->hasResource(path);
}

bool HelpScreen::open(int puzzleId, int difficulty) {
	if (_isActive) {
		return false; // Already open
	}

	// Check if page 1 exists
	if (!isPageValid(puzzleId, difficulty, 1)) {
		debug("No help available for puzzle %d, difficulty %d", puzzleId, difficulty);
		return false;
	}

	_isActive = true;
	_currentPuzzleId = puzzleId;
	_currentDifficulty = difficulty;
	_currentPage = 1;

	// Record pause start time
	_pauseStartTime = g_system->getMillis();

	// Pause audio
	_engine->getSoundManager()->pauseAll();

	// Save current screen
	_savedScreen->copyFrom(*_engine->getCurrentScreen());

	// Load first help page
	if (!loadPage(puzzleId, difficulty, 1)) {
		// Failed to load - close and return
		close();
		return false;
	}

	return true;
}

void HelpScreen::close() {
	if (!_isActive) {
		return;
	}

	_isActive = false;

	// Free loaded help page
	freePage();

	// Restore saved screen
	_engine->getCurrentScreen()->copyFrom(*_savedScreen);

	// Resume audio
	_engine->getSoundManager()->resumeAll();

	// Update pause accumulator (for accurate timing)
	uint32 pauseDuration = g_system->getMillis() - _pauseStartTime;
	_engine->addPauseTime(pauseDuration);

	_currentPuzzleId = -1;
	_currentDifficulty = -1;
	_currentPage = 1;
}

bool HelpScreen::loadPage(int puzzleId, int difficulty, int page) {
	// Free existing page
	freePage();

	// Construct help page path
	Common::String path = Common::String::format("Bmp/help/%02d_help_%s_%02d.bb",
	                                              puzzleId,
	                                              getDifficultyString(difficulty),
	                                              page);

	// Load help page
	_helpPage = _engine->loadBitBlock(path);

	return (_helpPage != nullptr);
}

void HelpScreen::freePage() {
	delete _helpPage;
	_helpPage = nullptr;
}

void HelpScreen::draw(Graphics::ManagedSurface *screen) {
	if (!_isActive) {
		return;
	}

	// Get alpha LUT for RleBlock rendering
	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw help frame overlay (darkened background)
	if (_helpFrame) {
		_helpFrame->drawToScreen(screen, 0, 0, lut);
	}

	// Draw help page content (centered)
	if (_helpPage) {
		// Center the help page on screen
		int x = (kScreenWidth - _helpPage->getWidth()) / 2;
		int y = (kScreenHeight - _helpPage->getHeight()) / 2;
		_helpPage->drawToSurface(screen, x, y);
	} else if (_placeholder) {
		// Show placeholder if no help page loaded
		_placeholder->drawToScreen(screen, 0, 0, lut);
	}

	// Draw OK button
	if (_okButtonHovered && _okButtonPushed) {
		_okButtonPushed->drawToSurface(screen, _okButtonRect.left, _okButtonRect.top);
	} else if (_okButtonNormal) {
		_okButtonNormal->drawToSurface(screen, _okButtonRect.left, _okButtonRect.top);
	}

	// Draw left arrow
	bool leftEnabled = isPageValid(_currentPuzzleId, _currentDifficulty, _currentPage - 1);
	if (leftEnabled && _leftArrowNormal) {
		_leftArrowNormal->drawToSurface(screen, _leftArrowRect.left, _leftArrowRect.top);
	} else if (_leftArrowEmpty) {
		_leftArrowEmpty->drawToSurface(screen, _leftArrowRect.left, _leftArrowRect.top);
	}

	// Draw right arrow
	bool rightEnabled = isPageValid(_currentPuzzleId, _currentDifficulty, _currentPage + 1);
	if (rightEnabled && _rightArrowNormal) {
		_rightArrowNormal->drawToSurface(screen, _rightArrowRect.left, _rightArrowRect.top);
	} else if (_rightArrowEmpty) {
		_rightArrowEmpty->drawToSurface(screen, _rightArrowRect.left, _rightArrowRect.top);
	}
}

void HelpScreen::handleMouseMove(const Common::Point &pos) {
	if (!_isActive) {
		return;
	}

	_okButtonHovered = _okButtonRect.contains(pos);
	_leftArrowHovered = _leftArrowRect.contains(pos);
	_rightArrowHovered = _rightArrowRect.contains(pos);
}

bool HelpScreen::handleClick(const Common::Point &pos) {
	if (!_isActive) {
		return false;
	}

	// OK button - close help
	if (_okButtonRect.contains(pos)) {
		close();
		return true;
	}

	// Left arrow - previous page
	if (_leftArrowRect.contains(pos)) {
		if (isPageValid(_currentPuzzleId, _currentDifficulty, _currentPage - 1)) {
			if (loadPage(_currentPuzzleId, _currentDifficulty, _currentPage - 1)) {
				_currentPage--;
			}
		}
		return true;
	}

	// Right arrow - next page
	if (_rightArrowRect.contains(pos)) {
		if (isPageValid(_currentPuzzleId, _currentDifficulty, _currentPage + 1)) {
			if (loadPage(_currentPuzzleId, _currentDifficulty, _currentPage + 1)) {
				_currentPage++;
			}
		}
		return true;
	}

	// Click outside buttons - no action but still handled
	return true;
}

} // End of namespace Zoombini2
