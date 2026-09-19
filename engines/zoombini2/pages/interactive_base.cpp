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
#include "zoombini2/pages/dialog_msgbox.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

#include "common/callback.h"
#include "common/config-manager.h"
#include "common/keyboard.h"
#include "zoombini2/metaengine.h"

namespace Zoombini2 {

constexpr const char *InteractiveBase::kMapMusicPath;

constexpr const char *Sidebar::kHelpNormalPath;
constexpr const char *Sidebar::kHelpHighlightPath;
constexpr const char *Sidebar::kMapNormalPath;
constexpr const char *Sidebar::kMapHighlightPath;
constexpr const char *Sidebar::kGoNormalPath;
constexpr const char *Sidebar::kGoHighlightPath;
constexpr const char *Sidebar::kGoDisabledPath;
constexpr const char *Sidebar::kHelpClickSoundPath;
constexpr const char *Sidebar::kMapClickSoundPath;
constexpr const char *Sidebar::kAbandonConfirmationPath;

Sidebar::Sidebar(Zoombini2Engine *vm)
	: _vm(vm) {

	_helpNormal = _vm->loadRleBlock(kHelpNormalPath);
	_helpHighlight = _vm->loadRleBlock(kHelpHighlightPath);
	_mapNormal = _vm->loadRleBlock(kMapNormalPath);
	_mapHighlight = _vm->loadRleBlock(kMapHighlightPath);
	_goNormal = _vm->loadRleBlock(kGoNormalPath);
	_goHighlight = _vm->loadRleBlock(kGoHighlightPath);
	_goDisabled = _vm->loadRleBlock(kGoDisabledPath);
	_helpClickSoundId = _vm->getSoundManager()->load(false, Common::Path(kHelpClickSoundPath), false);
	_mapClickSoundId = _vm->getSoundManager()->load(false, Common::Path(kMapClickSoundPath), false);

	// Create help screen modal system
	_helpScreen = new DialogHelp(_vm);

	// Create saved background buffer (34x102 for all 3 buttons).
	_savedBackground = _vm->_gfx->createSurface(Size32(34, 102));
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

void Sidebar::restartGoBlink() {
	_goBlinkHighlighted = false;
	_goBlinkTogglesRemaining = 30;
	_goBlinkDeadline = _vm->getGameTickCount() + 150;
}

bool Sidebar::isPointStrictlyInside(const Common::Rect &rect, const Common::Point &pos) {
	return rect.left < pos.x && pos.x < rect.right && rect.top < pos.y && pos.y < rect.bottom;
}

bool Sidebar::isInButtonRegion(const Common::Point &pos) const {
	return isPointStrictlyInside(_helpButtonRect, pos) || isPointStrictlyInside(_mapButtonRect, pos) ||
		   isPointStrictlyInside(_goButtonRect, pos);
}

bool Sidebar::isInteractionBlocked() const {
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[i];
		if (zoombini && zoombini->_dragging)
			return true;
	}
	const PageBase *page = _vm->getCurrentPage();
	return page && page->blocksSidebarInteraction();
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
	if (DialogBase *dialog = getActiveDialog()) {
		_primaryButtonArmed = false;
		_pendingMouseRelease = false;
		dialog->onLButtonUp(pos);
		return EventHandleResult::kConsumed;
	}
	if (isInteractionBlocked()) {
		_primaryButtonArmed = false;
		_pendingMouseRelease = false;
		return EventHandleResult::kPassthrough;
	}
	if (_primaryButtonArmed) {
		_primaryButtonArmed = false;
		_pendingMouseReleasePos = pos;
		_pendingMouseRelease = true;
	}
	if (shouldShow() && isInButtonRegion(pos))
		return EventHandleResult::kConsumed;
	return EventHandleResult::kPassthrough;
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
		// The original frame routine keeps painting the Help, Map, and Go
		// controls while the help overlay owns input. Rollover and release
		// handling stay gated off, so every control keeps its normal sprite.
		_pendingMouseRelease = false;
		updateHoverState(Common::Point(), false);
		dialog->render(screen);
		drawControls(screen);
		return;
	}

	if (!shouldShow()) {
		_pendingMouseRelease = false;
		updateHoverState(Common::Point(), false);
		return;
	}

	const bool pointerInputAllowed = inputAllowed && !isInteractionBlocked();
	const Common::Point32 mousePos = _vm->getMousePos();
	// Later mouse motion in the input batch must not move a pending release to another control.
	const Common::Point hitPos = _pendingMouseRelease ? _pendingMouseReleasePos : Common::Point(mousePos.x, mousePos.y);
	updateHoverState(hitPos, pointerInputAllowed);

	_savedBackground->copyRectToSurface(*screen,
										0, 0,
										Common::Rect(5, 480, 39, 582));

	if (pointerInputAllowed)
		consumePendingRelease();
	else
		_pendingMouseRelease = false;

	if (DialogBase *dialog = getActiveDialog()) {
		dialog->render(screen);
		drawControls(screen);
		return;
	}

	drawControls(screen);
}

void Sidebar::drawControls(ManagedSurface32 *screen) {
	const PageBase *page = _vm->getCurrentPage();
	if (!page)
		return;
	updateGoBlink(page->hasGoButton() && page->canUseGoButton(), page->getPageId());

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
	if (DialogBase *dialog = getActiveDialog()) {
		_primaryButtonArmed = false;
		_pendingMouseRelease = false;
		return dialog->onLButtonDown(pos);
	}
	if (isInteractionBlocked()) {
		_primaryButtonArmed = false;
		_pendingMouseRelease = false;
		return EventHandleResult::kPassthrough;
	}
	_primaryButtonArmed = true;

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
	int level = _vm->_state->getLevel();

	_helpScreen->open(static_cast<int>(currentPage), level);
}

void Sidebar::onMapClick() {
	_vm->getSoundManager()->play(_mapClickSoundId);
	if (!_vm->_isSavedGame) {
		returnToMap();
		return;
	}
	if (!ConfMan.getBool(::Zoombini2MetaEngine::kConfigSavefilesReadOnly, ConfMan.getActiveDomainName()) &&
		!_vm->writeGameSave(_vm->_state->_playerName))
		return;
	const PageBase *page = _vm->getCurrentPage();
	bool hasActive = false;
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[i];
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
	_vm->getMsgBoxDialog()->request(Common::Path(kAbandonConfirmationPath),
									new Common::Callback<Sidebar, DialogMsgBoxButton>(this, &Sidebar::handleAbandonConfirmation));
}

void Sidebar::handleAbandonConfirmation(DialogMsgBoxButton button) {
	if (button == DialogMsgBoxButton::kOkay01)
		returnToMap();
}

} // End of namespace Zoombini2
