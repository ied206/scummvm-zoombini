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

#include "zoombini2/sidebar.h"
#include "zoombini2/zoombini2.h"
#include "zoombini2/help_screen.h"
#include "zoombini2/gfx.h"
#include "zoombini2/game_state.h"

#include "common/system.h"
#include "graphics/managed_surface.h"

namespace Zoombini2 {

Sidebar::Sidebar(Zoombini2Engine *engine)
	: _engine(engine), _helpScreen(nullptr),
	  _helpNormal(nullptr), _helpHighlight(nullptr),
	  _saveNormal(nullptr), _saveHighlight(nullptr),
	  _goNormal(nullptr), _goHighlight(nullptr),
	  _helpHovered(false), _saveHovered(false), _goHovered(false),
	  _savedBackground(nullptr) {

	// Initialize button rectangles (from original @ 0x462DA0)
	_helpButtonRect = Common::Rect(5, 480, 39, 514);   // 34x34
	_saveButtonRect = Common::Rect(5, 514, 39, 548);   // 34x34
	_goButtonRect = Common::Rect(5, 548, 39, 582);     // 34x34

	// Load button graphics
	_helpNormal = _engine->loadBitBlock("Bmp/MENU/PANEL - Aide NORMAL.bb");
	_helpHighlight = _engine->loadBitBlock("Bmp/MENU/PANEL - Aide HIGHLIGHT.bb");
	
	_saveNormal = _engine->loadBitBlock("Bmp/MENU/PANEL - Entrainement NORMAL.bb");
	_saveHighlight = _engine->loadBitBlock("Bmp/MENU/PANEL - Entraine HILITE.bb");
	
	_goNormal = _engine->loadBitBlock("Bmp/MENU/PANEL - Quitter NORMAL.bb");
	_goHighlight = _engine->loadBitBlock("Bmp/MENU/PANEL - Quitter HIGHLIGHT.bb");

	// Create help screen modal system
	_helpScreen = new HelpScreen(_engine);

	// Create saved background buffer (34x102 for all 3 buttons)
	// Must match screen format to avoid assert in copyRectToSurface
	_savedBackground = new Graphics::ManagedSurface(34, 102,
	                                                 _engine->getCurrentScreen()->format);
}

Sidebar::~Sidebar() {
	delete _helpScreen;
	delete _helpNormal;
	delete _helpHighlight;
	delete _saveNormal;
	delete _saveHighlight;
	delete _goNormal;
	delete _goHighlight;
	delete _savedBackground;
}

bool Sidebar::shouldShow() const {
	// Hide sidebar during certain page types (from original @ 0x462DA0)
	PageId currentPage = (PageId)_engine->getCurrentPageId();

	switch (currentPage) {
	// Always hide on these pages
	case kPageTLCLogo:
	case kPageTitleAnim:
	case kPageStoryBmp:
	case kPageStoryAnim:
	case kPageMenuAlt:
	case kPageWorldMap:
	case kPageCredits:
	case kPageMapTrans:
	case kPageFinal:
	case kPageTitleScreen:
	case kPageLogopoly:
		return false;

	// Show on all puzzle pages and hubs
	default:
		return true;
	}
}

bool Sidebar::draw(Graphics::ManagedSurface *screen) {
	// Check if help screen is active
	if (_helpScreen && _helpScreen->isActive()) {
		_helpScreen->draw(screen);
		return true;
	}

	// Check if sidebar should be visible
	if (!shouldShow()) {
		return false;
	}

	// Save background under sidebar buttons
	_savedBackground->copyRectToSurface(*screen,
	                                     0, 0,
	                                     Common::Rect(5, 480, 39, 582));

	// Draw help button
	if (_helpHovered && _helpHighlight) {
		_helpHighlight->drawToSurface(screen, _helpButtonRect.left, _helpButtonRect.top);
	} else if (_helpNormal) {
		_helpNormal->drawToSurface(screen, _helpButtonRect.left, _helpButtonRect.top);
	}

	// Draw save button
	if (_saveHovered && _saveHighlight) {
		_saveHighlight->drawToSurface(screen, _saveButtonRect.left, _saveButtonRect.top);
	} else if (_saveNormal) {
		_saveNormal->drawToSurface(screen, _saveButtonRect.left, _saveButtonRect.top);
	}

	// Draw go button
	if (_goHovered && _goHighlight) {
		_goHighlight->drawToSurface(screen, _goButtonRect.left, _goButtonRect.top);
	} else if (_goNormal) {
		_goNormal->drawToSurface(screen, _goButtonRect.left, _goButtonRect.top);
	}

	return true;
}

void Sidebar::handleMouseMove(const Common::Point &pos) {
	// If help screen is active, forward to help screen
	if (_helpScreen && _helpScreen->isActive()) {
		_helpScreen->handleMouseMove(pos);
		return;
	}

	if (!shouldShow()) {
		return;
	}

	_helpHovered = _helpButtonRect.contains(pos);
	_saveHovered = _saveButtonRect.contains(pos);
	_goHovered = _goButtonRect.contains(pos);
}

bool Sidebar::handleClick(const Common::Point &pos) {
	// If help screen is active, forward to help screen
	if (_helpScreen && _helpScreen->isActive()) {
		return _helpScreen->handleClick(pos);
	}

	if (!shouldShow()) {
		return false;
	}

	// Check button clicks
	if (_helpButtonRect.contains(pos)) {
		onHelpClick();
		return true;
	}

	if (_saveButtonRect.contains(pos)) {
		onSaveClick();
		return true;
	}

	if (_goButtonRect.contains(pos)) {
		onGoClick();
		return true;
	}

	return false;
}

void Sidebar::onHelpClick() {
	if (!_helpScreen) {
		return;
	}

	// Get current puzzle and difficulty
	PageId currentPage = (PageId)_engine->getCurrentPageId();
	int difficulty = _engine->getGameState()->getDifficulty();

	// Open help screen
	_helpScreen->open((int)currentPage, difficulty);
}

void Sidebar::onSaveClick() {
	// TODO: Implement full save workflow with save slot selection
	// For now, just show a warning that save button was clicked
	warning("Save button clicked - full save UI not yet implemented");
	
	// The full implementation should:
	// 1. Show save slot selection dialog
	// 2. Get save file name from user
	// 3. Call _engine->writeGameSave(saveName)
}

void Sidebar::onGoClick() {
	// Request page transition to map
	// This will trigger the current puzzle's completion check
	_engine->requestPageChange(kPageMapTrans);
}

} // End of namespace Zoombini2
