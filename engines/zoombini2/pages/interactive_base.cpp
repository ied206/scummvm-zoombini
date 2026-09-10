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

#include "zoombini2/pages/interactive_base.h"
#include "zoombini2/graphics.h"
#include "zoombini2/pages/dialog_help.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

#include "common/keyboard.h"
#include "common/system.h"
#include "graphics/managed_surface.h"

namespace Zoombini2 {

ThreeButtons::ThreeButtons(Zoombini2Engine *vm)
	: _vm(vm), _helpScreen(nullptr),
	  _helpNormal(nullptr), _helpHighlight(nullptr),
	  _mapNormal(nullptr), _mapHighlight(nullptr),
	  _goNormal(nullptr), _goHighlight(nullptr), _goDisabled(nullptr),
	  _helpClickSoundId(-1), _mapClickSoundId(-1),
	  _helpHovered(false), _mapHovered(false), _goHovered(false), _primaryButtonArmed(false), _pendingMouseRelease(false),
	  _goWasEnabled(false), _goBlinkHighlighted(false),
	  _goBlinkTogglesRemaining(0), _goPageId(-1), _goBlinkDeadline(0),
	  _savedBackground(nullptr), _confirmBackground(nullptr), _confirmPanels(),
	  _confirmText(nullptr), _confirmActive(false), _confirmHover(0) {

	// The three-button group keeps a fixed 34-pixel hit area for each control.
	_helpButtonRect = Common::Rect(5, 480, 39, 514); // 34x34
	_mapButtonRect = Common::Rect(5, 514, 39, 548);  // 34x34
	_goButtonRect = Common::Rect(5, 548, 39, 582);   // 34x34

	_helpNormal = _vm->loadRleBlock("Bmp/BARRE/QUOI.RB");
	_helpHighlight = _vm->loadRleBlock("Bmp/BARRE/QUOIROLL.RB");
	_mapNormal = _vm->loadRleBlock("Bmp/BARRE/PATH.RB");
	_mapHighlight = _vm->loadRleBlock("Bmp/BARRE/PATHROLL.RB");
	_goNormal = _vm->loadRleBlock("Bmp/BARRE/Next.rb");
	_goHighlight = _vm->loadRleBlock("Bmp/BARRE/NextRoll.rb");
	_goDisabled = _vm->loadRleBlock("Bmp/BARRE/NextInvisible.rb");
	_helpClickSoundId = _vm->getSoundManager()->load(false, Common::Path("sounds/fx/I-BS2.wav"), false);
	_mapClickSoundId = _vm->getSoundManager()->load(false, Common::Path("sounds/fx/I-BS1.wav"), false);

	// Create help screen modal system
	_helpScreen = new DialogHelp(_vm);

	// Create saved background buffer (34x102 for all 3 buttons)
	// Must match screen format to avoid assert in copyRectToSurface
	_savedBackground = new Graphics::ManagedSurface(34, 102, _vm->getCurrentScreen()->format);
}

ThreeButtons::~ThreeButtons() {
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

DialogBase *ThreeButtons::getActiveDialog() const {
	return _helpScreen && _helpScreen->isActive() ? _helpScreen : nullptr;
}

bool ThreeButtons::hasActiveDialog() const {
	return _confirmActive || getActiveDialog() != nullptr;
}

bool ThreeButtons::shouldShow() const {
	const PageBase *page = _vm->getCurrentPage();
	return page && page->hasThreeButtons();
}

void ThreeButtons::updateGoBlink(bool goEnabled, int pageId) {
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
		_goBlinkDeadline = _vm->getGameTickCount() + 150;
	} else if (!goEnabled) {
		_goBlinkHighlighted = false;
		_goBlinkTogglesRemaining = 0;
		_goBlinkDeadline = 0;
	}
	_goWasEnabled = goEnabled;

	const uint32 tick = _vm->getGameTickCount();
	if (0 < _goBlinkTogglesRemaining && _goBlinkDeadline < tick) {
		_goBlinkHighlighted = !_goBlinkHighlighted;
		_goBlinkTogglesRemaining -= 1;
		_goBlinkDeadline = tick + 150;
	}
}

bool ThreeButtons::isPointStrictlyInside(const Common::Rect &rect, const Common::Point &pos) {
	return rect.left < pos.x && pos.x < rect.right && rect.top < pos.y && pos.y < rect.bottom;
}

bool ThreeButtons::isInButtonRegion(const Common::Point &pos) const {
	return isPointStrictlyInside(_helpButtonRect, pos) || isPointStrictlyInside(_mapButtonRect, pos) ||
		isPointStrictlyInside(_goButtonRect, pos);
}

void ThreeButtons::updateHoverState(const Common::Point &pos, bool inputAllowed) {
	if (!inputAllowed) {
		_helpHovered = false;
		_mapHovered = false;
		_goHovered = false;
		return;
	}

	_helpHovered = isPointStrictlyInside(_helpButtonRect, pos);
	_mapHovered = isPointStrictlyInside(_mapButtonRect, pos);
	const PageBase *page = _vm->getCurrentPage();
	_goHovered = page && page->hasGoButton() && page->canUseGoButton() && isPointStrictlyInside(_goButtonRect, pos);
}

void ThreeButtons::consumePendingRelease() {
	if (!_pendingMouseRelease)
		return;

	_pendingMouseRelease = false;
	if (_helpHovered) {
		onHelpClick();
	} else if (_mapHovered) {
		onMapClick();
	} else if (_goHovered) {
		onGoClick();
	}
}

EventHandleResult ThreeButtons::onLButtonUp(const Common::Point &pos) {
	if (_primaryButtonArmed) {
		_primaryButtonArmed = false;
		_pendingMouseRelease = true;
	}
	if (DialogBase *dialog = getActiveDialog()) {
		_pendingMouseRelease = false;
		dialog->onLButtonUp(pos);
		return EventHandleResult::kConsumed;
	}
	if (!hasActiveDialog() && shouldShow() && isInButtonRegion(pos))
		return EventHandleResult::kConsumed;
	return hasActiveDialog() ? EventHandleResult::kConsumed : EventHandleResult::kPassthrough;
}

EventHandleResult ThreeButtons::onKeyDown(const Common::KeyState &key, bool repeat) {
	if (_confirmActive) {
		if (key.keycode == Common::KEYCODE_ESCAPE)
			closeSaveConfirmation();
		return EventHandleResult::kConsumed;
	}
	if (DialogBase *dialog = getActiveDialog()) {
		dialog->onKeyDown(key, repeat);
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

EventHandleResult ThreeButtons::onKeyUp(const Common::KeyState &key) {
	if (DialogBase *dialog = getActiveDialog())
		dialog->onKeyUp(key);
	return hasActiveDialog() ? EventHandleResult::kConsumed : EventHandleResult::kPassthrough;
}
void ThreeButtons::drawAndHandleInput(ManagedSurface32 *screen, bool inputAllowed) {
	if (_confirmActive) {
		_pendingMouseRelease = false;
		drawSaveConfirmation(screen);
		return;
	}
	if (DialogBase *dialog = getActiveDialog()) {
		_pendingMouseRelease = false;
		dialog->render(screen);
		return;
	}

	if (!shouldShow()) {
		_pendingMouseRelease = false;
		updateHoverState(Common::Point(), false);
		return;
	}

	const PageBase *page = _vm->getCurrentPage();
	if (inputAllowed)
		updateGoBlink(page->hasGoButton() && page->canUseGoButton(), page->getPageId());
	const Common::Point32 mousePos = _vm->getMousePos();
	updateHoverState(Common::Point(mousePos.x, mousePos.y), inputAllowed);

	_savedBackground->copyRectToSurface(*screen,
										0, 0,
										Common::Rect(5, 480, 39, 582));

	if (inputAllowed)
		consumePendingRelease();
	else
		_pendingMouseRelease = false;

	if (_confirmActive) {
		drawSaveConfirmation(screen);
		return;
	}
	if (DialogBase *dialog = getActiveDialog()) {
		dialog->render(screen);
		return;
	}

	if (_helpHovered && _helpHighlight) {
		_helpHighlight->drawToScreen(screen, Common::Point32(_helpButtonRect.left, _helpButtonRect.top), _vm->getAlphaLUT());
	} else if (_helpNormal) {
		_helpNormal->drawToScreen(screen, Common::Point32(_helpButtonRect.left, _helpButtonRect.top), _vm->getAlphaLUT());
	}

	if (_mapHovered && _mapHighlight) {
		_mapHighlight->drawToScreen(screen, Common::Point32(_mapButtonRect.left, _mapButtonRect.top), _vm->getAlphaLUT());
	} else if (_mapNormal) {
		_mapNormal->drawToScreen(screen, Common::Point32(_mapButtonRect.left, _mapButtonRect.top), _vm->getAlphaLUT());
	}

	if (!page->hasGoButton())
		return;
	const bool goEnabled = page->canUseGoButton();
	if (!goEnabled && _goDisabled) {
		_goDisabled->drawToScreen(screen, Common::Point32(_goButtonRect.left, _goButtonRect.top), _vm->getAlphaLUT());
	} else if ((_goHovered || _goBlinkHighlighted) && _goHighlight) {
		_goHighlight->drawToScreen(screen, Common::Point32(_goButtonRect.left, _goButtonRect.top), _vm->getAlphaLUT());
	} else if (_goNormal) {
		_goNormal->drawToScreen(screen, Common::Point32(_goButtonRect.left, _goButtonRect.top), _vm->getAlphaLUT());
	}
}

EventHandleResult ThreeButtons::onMouseMove(const Common::Point &pos) {
	if (_confirmActive) {
		_confirmHover = hitTestSaveConfirmation(pos);
		return EventHandleResult::kConsumed;
	}
	if (DialogBase *dialog = getActiveDialog()) {
		dialog->onMouseMove(pos);
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

EventHandleResult ThreeButtons::onLButtonDown(const Common::Point &pos) {
	_primaryButtonArmed = true;
	if (_confirmActive) {
		const int button = hitTestSaveConfirmation(pos);
		if (button) {
			closeSaveConfirmation();
			if (button == 1)
				returnToMap();
		}
		return EventHandleResult::kConsumed;
	}
	if (DialogBase *dialog = getActiveDialog()) {
		return dialog->onLButtonDown(pos);
	}

	if (shouldShow() && isInButtonRegion(pos))
		return EventHandleResult::kConsumed;

	return EventHandleResult::kPassthrough;
}

void ThreeButtons::onHelpClick() {
	if (!_helpScreen) {
		return;
	}
	_vm->getSoundManager()->play(_helpClickSoundId);

	PageId currentPage = static_cast<PageId>(_vm->getCurrentPageId());
	int level = _vm->getGameState()->getLevel();

	_helpScreen->open(static_cast<int>(currentPage), level);
}

void ThreeButtons::onMapClick() {
	_vm->getSoundManager()->play(_mapClickSoundId);
	if (!_vm->_isSavedGame) {
		returnToMap();
		return;
	}
	if (!_vm->writeGameSave(_vm->getGameState()->_playerName))
		return;
	const PageBase *page = _vm->getCurrentPage();
	bool hasActive = false;
	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		const ZoombiniState *zoombini = _vm->_globalZoombinis[i];
		hasActive = hasActive || zoombini->_puzzleStatus == 1 || zoombini->_inputEnabled == 1;
	}
	if ((page && page->isShelter()) ||
		(!hasActive && (!_vm->getCurrentPage() || _vm->isStartingMapTransition())))
		returnToMap();
	else
		openSaveConfirmation();
}

void ThreeButtons::onGoClick() {
	const PageBase *page = _vm->getCurrentPage();
	if (!page || !page->canUseGoButton())
		return;
	if (!_vm->_isSavedGame) {
		returnToMap();
		return;
	}
	const PageId pageId = static_cast<PageId>(_vm->getCurrentPageId());
	_vm->_mapTransitionSourcePageId = pageId;
	_vm->requestPageChange(kPageMapTrans);
}

void ThreeButtons::returnToMap() {
	_vm->requestPageChange(_vm->_isSavedGame ? kPageMenuLoad : kPageMenuPractice);
}

void ThreeButtons::openSaveConfirmation() {
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
	_confirmBackground->copyFrom(*_vm->getCurrentScreen());
	_confirmPos = Common::Point32(400 - _confirmPanels[1]->getWidth() / 2, 300 - _confirmPanels[1]->getHeight() / 2);
	_confirmHover = 0;
	_confirmActive = true;
	_vm->_isPaused = true;
	_vm->_pauseTimeStart = g_system->getMillis();
	_vm->getSoundManager()->pauseAll();
}

void ThreeButtons::closeSaveConfirmation() {
	if (!_confirmActive)
		return;
	_confirmActive = false;
	_vm->getCurrentScreen()->copyFrom(*_confirmBackground);
	_vm->addPauseTime(g_system->getMillis() - _vm->_pauseTimeStart);
	_vm->_isPaused = false;
	_vm->getSoundManager()->resumeAll();
}

int ThreeButtons::hitTestSaveConfirmation(const Common::Point &pos) const {
	const int x = pos.x - _confirmPos.x;
	const int y = pos.y - _confirmPos.y;
	if (77 < y && y < 145) {
		if (207 < x && x < 272)
			return 1;
		if (287 < x && x < 352)
			return 2;
	}
	return 0;
}

void ThreeButtons::drawSaveConfirmation(ManagedSurface32 *screen) {
	screen->copyFrom(*_confirmBackground);
	_confirmPanels[_confirmHover]->drawToScreen(screen, _confirmPos, _vm->getAlphaLUT());
	_confirmText->drawToSurface(screen, Common::Point32(_confirmPos.x + 17, _confirmPos.y + 17));
}

} // End of namespace Zoombini2
