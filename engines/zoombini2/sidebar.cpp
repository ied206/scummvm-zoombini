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
#include "zoombini2/graphics.h"
#include "zoombini2/pages/dialog_help.h"
#include "zoombini2/pages/shelter_rescue.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

#include "common/keyboard.h"
#include "common/system.h"
#include "graphics/managed_surface.h"

namespace Zoombini2 {

Sidebar::Sidebar(Zoombini2Engine *engine)
	: _engine(engine), _helpScreen(nullptr),
	  _helpNormal(nullptr), _helpHighlight(nullptr),
	  _mapNormal(nullptr), _mapHighlight(nullptr),
	  _goNormal(nullptr), _goHighlight(nullptr), _goDisabled(nullptr),
	  _helpHovered(false), _mapHovered(false), _goHovered(false), _goWasEnabled(false), _goBlinkHighlighted(false),
	  _goBlinkTogglesRemaining(0), _goPageId(-1), _goBlinkDeadline(0),
	  _savedBackground(nullptr), _confirmBackground(nullptr), _confirmPanels(),
	  _confirmText(nullptr), _confirmActive(false), _confirmHover(0) {

	// The sidebar keeps a fixed 34-pixel hit area for each button.
	_helpButtonRect = Common::Rect(5, 480, 39, 514); // 34x34
	_mapButtonRect = Common::Rect(5, 514, 39, 548);  // 34x34
	_goButtonRect = Common::Rect(5, 548, 39, 582);   // 34x34

	_helpNormal = _engine->loadRleBlock("Bmp/BARRE/QUOI.RB");
	_helpHighlight = _engine->loadRleBlock("Bmp/BARRE/QUOIROLL.RB");
	_mapNormal = _engine->loadRleBlock("Bmp/BARRE/PATH.RB");
	_mapHighlight = _engine->loadRleBlock("Bmp/BARRE/PATHROLL.RB");
	_goNormal = _engine->loadRleBlock("Bmp/BARRE/Next.rb");
	_goHighlight = _engine->loadRleBlock("Bmp/BARRE/NextRoll.rb");
	_goDisabled = _engine->loadRleBlock("Bmp/BARRE/NextInvisible.rb");

	// Create help screen modal system
	_helpScreen = new HelpScreen(_engine);

	// Create saved background buffer (34x102 for all 3 buttons)
	// Must match screen format to avoid assert in copyRectToSurface
	_savedBackground = new Graphics::ManagedSurface(34, 102, _engine->getCurrentScreen()->format);
}

Sidebar::~Sidebar() {
	closeSaveConfirmation();
	for (int i = 0; i < 3; i++)
		delete _confirmPanels[i];
	delete _confirmText;
	delete _confirmBackground;
	delete _helpScreen;
	delete _helpNormal;
	delete _helpHighlight;
	delete _mapNormal;
	delete _mapHighlight;
	delete _goNormal;
	delete _goHighlight;
	delete _goDisabled;
	delete _savedBackground;
}

Dialog *Sidebar::getActiveDialog() const {
	return _helpScreen && _helpScreen->isActive() ? _helpScreen : nullptr;
}

bool Sidebar::hasActiveDialog() const {
	return _confirmActive || getActiveDialog() != nullptr;
}

bool Sidebar::shouldShow() const {
	const Page *page = _engine->getCurrentPage();
	return page && page->hasSidebar();
}

void Sidebar::updateGoBlink(bool goEnabled, int pageId) {
	if (_goPageId != pageId) {
		_goPageId = pageId;
		_goWasEnabled = goEnabled;
		_goBlinkHighlighted = false;
		_goBlinkTogglesRemaining = 0;
		_goBlinkDeadline = 0;
		return;
	}

	if (goEnabled && !_goWasEnabled) {
		_goBlinkHighlighted = false;
		_goBlinkTogglesRemaining = 30;
		_goBlinkDeadline = _engine->getGameTickCount() + 150;
	} else if (!goEnabled) {
		_goBlinkHighlighted = false;
		_goBlinkTogglesRemaining = 0;
		_goBlinkDeadline = 0;
	}
	_goWasEnabled = goEnabled;

	const uint32 tick = _engine->getGameTickCount();
	if (0 < _goBlinkTogglesRemaining && _goBlinkDeadline < tick) {
		_goBlinkHighlighted = !_goBlinkHighlighted;
		_goBlinkTogglesRemaining -= 1;
		_goBlinkDeadline = tick + 150;
	}
}

bool Sidebar::draw(Graphics::ManagedSurface *screen) {
	if (_confirmActive) {
		drawSaveConfirmation(screen);
		return true;
	}
	// Check if help screen is active
	if (Dialog *dialog = getActiveDialog()) {
		dialog->draw(screen);
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
		_helpHighlight->drawToScreen(screen, _helpButtonRect.left, _helpButtonRect.top, _engine->getAlphaLUT());
	} else if (_helpNormal) {
		_helpNormal->drawToScreen(screen, _helpButtonRect.left, _helpButtonRect.top, _engine->getAlphaLUT());
	}

	// Draw Map button
	if (_mapHovered && _mapHighlight) {
		_mapHighlight->drawToScreen(screen, _mapButtonRect.left, _mapButtonRect.top, _engine->getAlphaLUT());
	} else if (_mapNormal) {
		_mapNormal->drawToScreen(screen, _mapButtonRect.left, _mapButtonRect.top, _engine->getAlphaLUT());
	}

	const Page *page = _engine->getCurrentPage();
	if (!page->hasGoButton())
		return true;
	const bool goEnabled = page->canUseGoButton();
	updateGoBlink(goEnabled, page->getPageId());
	if (!goEnabled && _goDisabled) {
		_goDisabled->drawToScreen(screen, _goButtonRect.left, _goButtonRect.top, _engine->getAlphaLUT());
	} else if ((_goHovered || _goBlinkHighlighted) && _goHighlight) {
		_goHighlight->drawToScreen(screen, _goButtonRect.left, _goButtonRect.top, _engine->getAlphaLUT());
	} else if (_goNormal) {
		_goNormal->drawToScreen(screen, _goButtonRect.left, _goButtonRect.top, _engine->getAlphaLUT());
	}

	return true;
}

void Sidebar::handleMouseMove(const Common::Point &pos) {
	if (_confirmActive) {
		_confirmHover = hitTestSaveConfirmation(pos);
		if (_engine->getLastKeyPressed() == Common::KEYCODE_ESCAPE)
			closeSaveConfirmation();
		return;
	}
	// If help screen is active, forward to help screen
	if (Dialog *dialog = getActiveDialog()) {
		dialog->handleMouseMove(pos);
		return;
	}

	if (!shouldShow()) {
		return;
	}

	_helpHovered = _helpButtonRect.contains(pos);
	_mapHovered = _mapButtonRect.contains(pos);
	const Page *page = _engine->getCurrentPage();
	_goHovered = page->hasGoButton() && page->canUseGoButton() && _goButtonRect.contains(pos);
}

bool Sidebar::handleClick(const Common::Point &pos) {
	if (_confirmActive) {
		const int button = hitTestSaveConfirmation(pos);
		if (button) {
			closeSaveConfirmation();
			if (button == 1)
				returnToMap();
		}
		return true;
	}
	// If help screen is active, forward to help screen
	if (Dialog *dialog = getActiveDialog()) {
		return dialog->handleClick(pos);
	}

	if (!shouldShow()) {
		return false;
	}

	// Check button clicks
	if (_helpButtonRect.contains(pos)) {
		onHelpClick();
		return true;
	}

	if (_mapButtonRect.contains(pos)) {
		onMapClick();
		return true;
	}

	if (_engine->getCurrentPage()->hasGoButton() && _engine->getCurrentPage()->canUseGoButton() && _goButtonRect.contains(pos)) {
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

void Sidebar::onMapClick() {
	if (!_engine->_isSavedGame) {
		returnToMap();
		return;
	}
	if (!_engine->writeGameSave(_engine->getGameState()->_playerName))
		return;
	const Page *page = _engine->getCurrentPage();
	bool hasActive = false;
	for (uint i = 0; i < _engine->_globalZoombinis.size(); i++) {
		const ZoombiniState *zoombini = _engine->_globalZoombinis[i];
		hasActive = hasActive || zoombini->_freeStatus == 1 || zoombini->_activeFlag == 1;
	}
	if ((page && page->isShelter()) ||
		(!hasActive && (!_engine->getCurrentPage() || _engine->isAdvancingWorld())))
		returnToMap();
	else
		openSaveConfirmation();
}

void Sidebar::onGoClick() {
	const Page *page = _engine->getCurrentPage();
	if (!page || !page->canUseGoButton())
		return;
	if (!_engine->_isSavedGame) {
		returnToMap();
		return;
	}
	const int world = _engine->getCurrentPageId();
	if (world == kPageRescue1 || world == kPageRescue2) {
		const RescuePage *rescue = static_cast<const RescuePage *>(_engine->getCurrentPage());
		if (!rescue->hasFullDepartureParty() || (world == kPageRescue1 && _engine->_routeDirection == 0))
			return;
	}
	_engine->_maptransSourceWorld = world;
	_engine->requestPageChange(kPageMapTrans);
}

void Sidebar::returnToMap() {
	_engine->requestPageChange(_engine->_isSavedGame ? kPageMenuLoad : kPageMenuNew);
}

void Sidebar::openSaveConfirmation() {
	static constexpr const char *kPanelPaths[3] = {
		"bmp/menu/QUIT_panel_nothing.rb", "bmp/menu/QUIT_panel_ok.rb", "bmp/menu/QUIT_panel_cancel.rb"};
	for (int i = 0; i < 3; i++) {
		if (!_confirmPanels[i]) {
			_confirmPanels[i] = new RleBlock();
			_confirmPanels[i]->loadFromFile(Common::Path(kPanelPaths[i]));
		}
		if (!_confirmPanels[i]->isValid())
			return;
	}
	if (!_confirmText) {
		_confirmText = new BitBlock();
		_confirmText->load(Common::Path("bmp/menu/Quit_panel_text_abandon"));
	}
	if (!_confirmBackground)
		_confirmBackground = new Graphics::ManagedSurface();
	_confirmBackground->copyFrom(*_engine->getCurrentScreen());
	_confirmPosition = Common::Point(400 - _confirmPanels[1]->getWidth() / 2, 300 - _confirmPanels[1]->getHeight() / 2);
	_confirmHover = 0;
	_confirmActive = true;
	_engine->_isPaused = true;
	_engine->_pauseTimeStart = g_system->getMillis();
	_engine->getSoundManager()->pauseAll();
}

void Sidebar::closeSaveConfirmation() {
	if (!_confirmActive)
		return;
	_confirmActive = false;
	_engine->getCurrentScreen()->copyFrom(*_confirmBackground);
	_engine->addPauseTime(g_system->getMillis() - _engine->_pauseTimeStart);
	_engine->_isPaused = false;
	_engine->getSoundManager()->resumeAll();
}

int Sidebar::hitTestSaveConfirmation(const Common::Point &pos) const {
	const int x = pos.x - _confirmPosition.x;
	const int y = pos.y - _confirmPosition.y;
	if (77 < y && y < 145) {
		if (207 < x && x < 272)
			return 1;
		if (287 < x && x < 352)
			return 2;
	}
	return 0;
}

void Sidebar::drawSaveConfirmation(Graphics::ManagedSurface *screen) {
	screen->copyFrom(*_confirmBackground);
	_confirmPanels[_confirmHover]->drawToScreen(screen, _confirmPosition.x, _confirmPosition.y, _engine->getAlphaLUT());
	_confirmText->drawToSurface(screen, _confirmPosition.x + 17, _confirmPosition.y + 17);
}

} // End of namespace Zoombini2
