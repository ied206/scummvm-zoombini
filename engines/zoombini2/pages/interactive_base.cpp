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

InteractiveBase::InteractiveBase(Zoombini2Engine *vm, PageCategory category)
	: PageBase(vm, category) {
	switch (category) {
	case PageCategory::kInteractive:
	case PageCategory::kPuzzle:
	case PageCategory::kShelter:
		break;
	default:
		error("InteractiveBase: invalid pageCategory(%d)", static_cast<int>(category));
		break;
	}
}

Sidebar::Sidebar(Zoombini2Engine *vm)
	: _vm(vm) {

	_vm->_gfx->loadSharedRleBlock(kHelpNormalPath);
	_vm->_gfx->loadSharedRleBlock(kHelpHighlightPath);
	_vm->_gfx->loadSharedRleBlock(kMapNormalPath);
	_vm->_gfx->loadSharedRleBlock(kMapHighlightPath);
	_vm->_gfx->loadSharedRleBlock(kGoNormalPath);
	_vm->_gfx->loadSharedRleBlock(kGoHighlightPath);
	_vm->_gfx->loadSharedRleBlock(kGoDisabledPath);
	_helpClickSoundId = _vm->getSoundManager()->load(false, Common::Path(kHelpClickSoundPath), false);
	_mapClickSoundId = _vm->getSoundManager()->load(false, Common::Path(kMapClickSoundPath), false);

	// Create saved background buffer (34x102 for all 3 buttons).
	_savedBackground = _vm->_gfx->createSurface(Size32(34, 102));
}

Sidebar::~Sidebar() {
	delete _savedBackground;
}

bool Sidebar::shouldShow() const {
	const PageBase *page = _vm->getCurrentPage();
	return page && page->hasSidebar();
}

void Sidebar::updateGoBlink(bool goEnabled, PageId pageId) {
	if (_goPageId != pageId) {
		_goPageId = pageId;
		_goBlinkHighlighted = false;
		_goBlinkTogglesRemaining = 0;
		_goBlinkDeadline = 0;
		return;
	}

	if (!goEnabled) {
		_goBlinkHighlighted = false;
		_goBlinkTogglesRemaining = 0;
		_goBlinkDeadline = 0;
	}

	const uint32 tick = _vm->getGameTickCount();
	if (0 < _goBlinkTogglesRemaining && _goBlinkDeadline < tick) {
		_goBlinkHighlighted = !_goBlinkHighlighted;
		_goBlinkTogglesRemaining -= 1;
		_goBlinkDeadline = tick + 150;
	}
}

void Sidebar::restartGoBlink() {
	_goPageId = _vm->getCurrentPageId();
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
		if (zoombini && zoombini->isDragging())
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

void Sidebar::drawAndHandleInput(ManagedSurface32 *screen, bool inputAllowed) {
	if (_vm->getActiveDialog()) {
		_pendingMouseRelease = false;
		updateHoverState(Common::Point(), false);
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

	if (_vm->getActiveDialog()) {
		updateHoverState(Common::Point(), false);
		return;
	}

	drawControls(screen);
}

void Sidebar::drawOverDialog(ManagedSurface32 *screen) {
	_pendingMouseRelease = false;
	updateHoverState(Common::Point(), false);
	drawControls(screen);
}

void Sidebar::drawControls(ManagedSurface32 *screen) {
	const PageBase *page = _vm->getCurrentPage();
	if (!page)
		return;
	updateGoBlink(page->hasGoButton() && page->canUseGoButton(), page->getPageId());

	const char *helpPath = kHelpNormalPath;
	if (_helpHovered && _vm->_gfx->loadSharedRleBlock(kHelpHighlightPath))
		helpPath = kHelpHighlightPath;
	_vm->_gfx->drawSharedRleBlock(screen, helpPath, Common::Point32(_helpButtonRect.left, _helpButtonRect.top));

	const char *mapPath = kMapNormalPath;
	if (_mapHovered && _vm->_gfx->loadSharedRleBlock(kMapHighlightPath))
		mapPath = kMapHighlightPath;
	_vm->_gfx->drawSharedRleBlock(screen, mapPath, Common::Point32(_mapButtonRect.left, _mapButtonRect.top));

	if (!page->hasGoButton())
		return;
	const bool goEnabled = page->canUseGoButton();
	const char *goPath = kGoNormalPath;
	if (!goEnabled && _vm->_gfx->loadSharedRleBlock(kGoDisabledPath))
		goPath = kGoDisabledPath;
	else if ((_goHovered || _goBlinkHighlighted) && _vm->_gfx->loadSharedRleBlock(kGoHighlightPath))
		goPath = kGoHighlightPath;
	_vm->_gfx->drawSharedRleBlock(screen, goPath, Common::Point32(_goButtonRect.left, _goButtonRect.top));
}

EventHandleResult Sidebar::onLButtonDown(const Common::Point &pos) {
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
	_vm->getSoundManager()->play(_helpClickSoundId);

	const PageId currentPage = _vm->getCurrentPageId();
	int level = _vm->_state->getLevel();

	_vm->openHelpDialog(currentPage, level);
}

void Sidebar::onMapClick() {
	_vm->getSoundManager()->play(_mapClickSoundId);
	PageBase *page = _vm->getCurrentPage();
	if (page)
		page->onMapButtonPressed();
	if (!_vm->_isSavedGame) {
		returnToMap();
		return;
	}
	if (!_vm->writeGameSave(_vm->_state->getPlayerName()))
		return;
	bool hasActive = false;
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[i];
		hasActive = hasActive || zoombini->canAdvanceFromPage() || zoombini->isInputEnabled();
	}
	if ((page && page->isShelter()) ||
		(!hasActive && (!_vm->getCurrentPage() || _vm->isStartingMapTransition())))
		returnToMap();
	else
		requestAbandonConfirmation();
}

void Sidebar::onGoClick() {
	PageBase *page = _vm->getCurrentPage();
	if (!page || !page->canUseGoButton())
		return;
	if (!page->onGoButtonPressed())
		return;
	if (!_vm->_isSavedGame) {
		returnToMap();
		return;
	}
	const PageId pageId = _vm->getCurrentPageId();
	_vm->_mapTransitionSourcePageId = pageId;
	_vm->requestPageChange(kPageMapTrans);
}

void Sidebar::returnToMap() {
	if (_vm->isDemo()) {
		_vm->requestPageChange(kPageTitleScreen);
		return;
	}
	_vm->requestPageChange(_vm->_isSavedGame ? kPageMenuLoad : kPageMenuPractice);
}

void Sidebar::requestAbandonConfirmation() {
	_vm->requestMsgBox(Common::Path(kAbandonConfirmationPath),
									new Common::Callback<Sidebar, DialogMsgBoxButton>(this, &Sidebar::handleAbandonConfirmation));
}

void Sidebar::handleAbandonConfirmation(DialogMsgBoxButton button) {
	if (button == DialogMsgBoxButton::kOkay01)
		returnToMap();
}

} // End of namespace Zoombini2
