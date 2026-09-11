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

#include "common/debug.h"

#include "zoombini2/graphics.h"
#include "zoombini2/scripts.h"
#include "zoombini2/pages/shelter_base.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

ShelterRescueSiteBase::ShelterRescueSiteBase(Zoombini2Engine *vm)
	: ShelterBase(vm) {
}

ShelterRescueSiteBase::~ShelterRescueSiteBase() {
	if (0 <= _musicId)
		_vm->getSoundManager()->stop(_musicId);

	delete _selector;
	delete _porteSelect;
	delete _buttonUp;
	delete _buttonDown;
}

void ShelterRescueSiteBase::loadBackground(const char *path) {
	_vm->getScreen()->fillRect(Common::Rect32(kScreenWidth, kScreenHeight), 0);
	BitBlock background(_vm);
	if (background.load(Common::Path(path)))
		background.drawToSurface(_vm->getScreen(), Common::Point32(0, 0));
}

void ShelterRescueSiteBase::configureRosterLayout(const Common::Point32 &gridBasePos, const Common::Rect32 &scrollUpRect, const Common::Rect32 &scrollDownRect, const Common::Point32 &buttonUpPos, const Common::Point32 &buttonDownPos) {
	_gridBasePos = gridBasePos;
	_scrollUpRect = scrollUpRect;
	_scrollDownRect = scrollDownRect;
	_buttonUpPos = buttonUpPos;
	_buttonDownPos = buttonDownPos;
}

void ShelterRescueSiteBase::loadSelector(const char *path) {
	_selector = new RleBlock(_vm);
	_selector->loadFromFile(Common::Path(path));
}

void ShelterRescueSiteBase::loadPorteSelector(const char *path) {
	_porteSelect = new RleBlock(_vm);
	_porteSelect->loadFromFile(Common::Path(path));
}

void ShelterRescueSiteBase::loadScrollButtons(const char *buttonUpPath, const char *buttonDownPath) {
	_buttonUp = new Animation(_vm);
	_buttonUp->loadFromFile(Common::Path(buttonUpPath));

	_buttonDown = new Animation(_vm);
	_buttonDown->loadFromFile(Common::Path(buttonDownPath));
}

void ShelterRescueSiteBase::startMusic(const char *path) {
	_musicId = _vm->getSoundManager()->load(true, Common::Path(path), true);
	if (0 <= _musicId)
		_vm->getSoundManager()->play(_musicId);
}

void ShelterRescueSiteBase::resetRescueState() {
	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			const int idx = col * kGridRows + row;
			const Common::Point32 slotPos(_gridBasePos.x + col * kSlotWidth, _gridBasePos.y + row * kSlotHeight);
			_slotRects[idx] = Common::Rect32(slotPos.x + 18, slotPos.y + 30, slotPos.x + 58, slotPos.y + 87);
		}
	}

	_phase = 0;
	_phaseTimer = _vm->getGameTickCount();
	_scrollOffset = 0;
	_selectedZoombini = -1;
	_readyToDepart = false;
}

void ShelterRescueSiteBase::prepareWaitingBoard(BoardRecord **board) {
	GameState *state = _vm->getGameState();
	if (!state->hasPageVisit(_pageId, 1))
		GameState::clearBoard(board);
	_scrollOffset = GameState::findBoardScrollRow(board);
}

void ShelterRescueSiteBase::refillDepartureRoster(BoardRecord **board) {
	GameState::refillFromBoard(board, _vm->_globalZoombinis, 8);
	_departureRoster.clear();
	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		ZoombiniState *zoombini = _vm->_globalZoombinis[i];
		zoombini->_puzzleStatus = 0;
		zoombini->_inputEnabled = 1;
		if (i < 8)
			_departureRoster.push_back(zoombini);
	}
	_readyToDepart = _departureRoster.size() == 8;
}

void ShelterRescueSiteBase::saveRescueRoster(BoardRecord **board) {
	GameState *state = _vm->getGameState();
	Common::Array<ZoombiniState *> &roster = _vm->_globalZoombinis;
	for (uint i = 0; i < roster.size();) {
		ZoombiniState *zoombini = roster[i];
		bool departing = false;
		if (_vm->isStartingMapTransition()) {
			for (uint slot = 0; slot < _departureRoster.size(); slot++)
				departing = departing || _departureRoster[slot] == zoombini;
		}
		zoombini->_puzzleStatus = 1;
		if (departing) {
			i += 1;
		} else {
			GameState::storeInBoard(board, *zoombini);
			delete zoombini;
			roster.remove_at(i);
		}
	}
	_vm->writeGameSave(state->_playerName);
}

void ShelterRescueSiteBase::loadRosterAnimation() {
	_zoombiniAnimation = _vm->loadZoombiniAnimation(Common::Path("bmp/zombis/littleZomb.anm"));
	if (!_zoombiniAnimation)
		warning("RescueSite: Failed to load littleZomb.anm");
}

void ShelterRescueSiteBase::beginDeparture() {
	_phase = 1;
	_phaseTimer = _vm->getGameTickCount();
}

void ShelterRescueSiteBase::onUpdate() {
	if (_phase == 0) {
		// Wait for the player to choose a route direction.
	} else if (_phase == 1) {
		const uint32 elapsed = _vm->getGameTickCount() - _phaseTimer;
		if (1500 < elapsed) {
			_vm->_mapTransitionSourcePageId = static_cast<PageId>(_pageId);
			_vm->requestPageChange(kPageMapTrans);
		}
	}
}

void ShelterRescueSiteBase::onRenderScene(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();
	onRenderSite(screen);

	if (_buttonUp && 0 < _buttonUp->getFrameCount()) {
		const RleBlock *frame = _buttonUp->getFrame(0);
		if (frame)
			frame->drawToScreen(screen, _buttonUpPos, lut);
	}

	if (_buttonDown && 0 < _buttonDown->getFrameCount()) {
		const RleBlock *frame = _buttonDown->getFrame(0);
		if (frame)
			frame->drawToScreen(screen, _buttonDownPos, lut);
	}

	if (!_zoombiniAnimation)
		return;

	static constexpr int kStandingCell = 33;
	const int clipRight = _gridBasePos.x + 223;
	BoardRecord *const *board = getRescueBoard();
	for (int col = 0; col < kGridCols; col++) {
		const int colSlotBase = (col + _scrollOffset) * kGridRows;
		for (int row = 0; row < kGridRows; row++) {
			const int slotIdx = colSlotBase + row;
			if (slotIdx < 0 || kBoardRows * kBoardCols <= slotIdx)
				continue;

			const BoardRecord *record = board[slotIdx];
			if (!record)
				continue;

			const int x = _gridBasePos.x + col * kSlotWidth + 18;
			const int y = _gridBasePos.y + row * kSlotHeight + 30;
			const Common::Rect32 clip(_gridBasePos.x, 0, clipRight, kScreenHeight);
			_zoombiniAnimation->drawZoombini(screen, record->getTraits(), Common::Point32(x, y), kStandingCell, 0, lut, &clip);
			if (slotIdx == _selectedZoombini && _selector)
				_selector->drawToScreen(screen, Common::Point32(x - 10, y - 10), lut);
		}
	}
}

EventHandleResult ShelterRescueSiteBase::onLButtonDown(const Common::Point &pos) {
	if (_phase != 0)
		return EventHandleResult::kPassthrough;

	if (_scrollUpRect.contains(pos)) {
		if (0 < _scrollOffset) {
			_scrollOffset -= 1;
			debug(2, "RescueSite: Scroll up/left, offset=%d", _scrollOffset);
		}
		return EventHandleResult::kConsumed;
	}

	if (_scrollDownRect.contains(pos)) {
		if (_scrollOffset < kBoardRows - kGridCols)
			_scrollOffset += 1;
		debug(2, "RescueSite: Scroll down/right, offset=%d", _scrollOffset);
		return EventHandleResult::kConsumed;
	}

	const EventHandleResult siteResult = onSiteLButtonDown(pos);
	if (siteResult != EventHandleResult::kPassthrough)
		return siteResult;

	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			const Common::Point32 slotPos(_gridBasePos.x + col * kSlotWidth, _gridBasePos.y + row * kSlotHeight);
			const Common::Rect slotHit(slotPos.x + 24, slotPos.y + 30, slotPos.x + 64, slotPos.y + 87);
			if (slotHit.contains(pos)) {
				const int slotIdx = (col + _scrollOffset) * kGridRows + row;
				BoardRecord *const *board = getRescueBoard();
				if (0 <= slotIdx && slotIdx < kBoardRows * kBoardCols && board[slotIdx]) {
					_selectedZoombini = slotIdx;
					debug(2, "RescueSite: Selected zoombini %d at slot %d", slotIdx, slotIdx);
				} else {
					_selectedZoombini = -1;
				}
				return EventHandleResult::kConsumed;
			}
		}
	}
	return EventHandleResult::kPassthrough;
}

void ShelterRescueSiteBase::onRenderSite(ManagedSurface32 *screen) {
	(void)screen;
}

EventHandleResult ShelterRescueSiteBase::onSiteLButtonDown(const Common::Point &pos) {
	(void)pos;
	return EventHandleResult::kPassthrough;
}

} // End of namespace Zoombini2
