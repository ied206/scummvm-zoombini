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
#include "zoombini2/scripts.h"
#include "zoombini2/pages/dialog_help.h"
#include "zoombini2/pages/dialog_msgbox.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

#include "common/callback.h"
#include "common/keyboard.h"
#include "graphics/managed_surface.h"

namespace Zoombini2 {

Sidebar::Sidebar(Zoombini2Engine *vm)
	: _vm(vm) {

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

Sidebar::~Sidebar() {
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

DialogBase *Sidebar::getActiveDialog() const {
	return _helpScreen && _helpScreen->isActive() ? _helpScreen : nullptr;
}

bool Sidebar::hasActiveDialog() const {
	return getActiveDialog() != nullptr;
}

bool Sidebar::shouldShow() const {
	const PageBase *page = _vm->getCurrentPage();
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

bool Sidebar::isPointStrictlyInside(const Common::Rect &rect, const Common::Point &pos) {
	return rect.left < pos.x && pos.x < rect.right && rect.top < pos.y && pos.y < rect.bottom;
}

bool Sidebar::isInButtonRegion(const Common::Point &pos) const {
	return isPointStrictlyInside(_helpButtonRect, pos) || isPointStrictlyInside(_mapButtonRect, pos) ||
		isPointStrictlyInside(_goButtonRect, pos);
}

void Sidebar::updateHoverState(const Common::Point &pos, bool inputAllowed) {
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

void Sidebar::consumePendingRelease() {
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

EventHandleResult Sidebar::onLButtonUp(const Common::Point &pos) {
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

EventHandleResult Sidebar::onKeyDown(const Common::KeyState &key, bool repeat) {
	if (DialogBase *dialog = getActiveDialog()) {
		dialog->onKeyDown(key, repeat);
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

EventHandleResult Sidebar::onKeyUp(const Common::KeyState &key) {
	if (DialogBase *dialog = getActiveDialog())
		dialog->onKeyUp(key);
	return hasActiveDialog() ? EventHandleResult::kConsumed : EventHandleResult::kPassthrough;
}
void Sidebar::drawAndHandleInput(ManagedSurface32 *screen, bool inputAllowed) {
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

EventHandleResult Sidebar::onMouseMove(const Common::Point &pos) {
	if (DialogBase *dialog = getActiveDialog()) {
		dialog->onMouseMove(pos);
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

EventHandleResult Sidebar::onLButtonDown(const Common::Point &pos) {
	_primaryButtonArmed = true;
	if (DialogBase *dialog = getActiveDialog()) {
		return dialog->onLButtonDown(pos);
	}

	if (shouldShow() && isInButtonRegion(pos))
		return EventHandleResult::kConsumed;

	return EventHandleResult::kPassthrough;
}

void Sidebar::onHelpClick() {
	if (!_helpScreen) {
		return;
	}
	_vm->getSoundManager()->play(_helpClickSoundId);

	PageId currentPage = static_cast<PageId>(_vm->getCurrentPageId());
	int level = _vm->getGameState()->getLevel();

	_helpScreen->open(static_cast<int>(currentPage), level);
}

void Sidebar::onMapClick() {
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
		requestAbandonConfirmation();
}

void Sidebar::onGoClick() {
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

void Sidebar::returnToMap() {
	_vm->requestPageChange(_vm->_isSavedGame ? kPageMenuLoad : kPageMenuPractice);
}

void Sidebar::requestAbandonConfirmation() {
	_vm->getMsgBoxDialog()->request(Common::Path("bmp/menu/Quit_panel_text_abandon"),
		new Common::Callback<Sidebar, DialogMsgBoxButton>(this, &Sidebar::handleAbandonConfirmation));
}

void Sidebar::handleAbandonConfirmation(DialogMsgBoxButton button) {
	if (button == DialogMsgBoxButton::kOkay01)
		returnToMap();
}

} // End of namespace Zoombini2
