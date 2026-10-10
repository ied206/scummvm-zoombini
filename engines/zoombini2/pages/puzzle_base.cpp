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
#include "zoombini2/pages/puzzle_base.h"
#include "zoombini2/scripts.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleBase::kZoombiniAnimationPath;

PuzzleBase::PuzzleBase(Zoombini2Engine *vm, PageId pageId)
	: InteractiveBase(vm, PageCategory::kPuzzle), _puzzleLevel(vm->_state->_level) {
	_pageId = pageId;
}

void PuzzleBase::finishPuzzleRoster(StorageRecord **storage, bool perfectClearEligible) {
	_vm->_state->finishPuzzleRoster(_pageId, storage, _vm->isStartingMapTransition(), _vm->_isSavedGame, perfectClearEligible);
}

void PuzzleBase::init() {
	debug(1, "Puzzle::init - %s (page %d)", _puzzleName, static_cast<int>(_pageId));

	_vm->_gfx->getPageLayerStack()->clear();
	_backgroundPath.clear();
	_vm->_gfx->getPageLayerStack()->addLayer(1);
	if (_initialBackgroundPath) {
		const Common::Path bgPath(_initialBackgroundPath);
		if (!loadPrimaryLayerBackground(bgPath)) {
			debug(1, "Puzzle: Failed to load background for %s", _puzzleName);
		}
	}
	_vm->_gfx->getPageLayerStack()->addLayer(1);

	_zoombiniAnimation = _vm->loadZoombiniAnimation(Common::Path(kZoombiniAnimationPath), 50);
	if (!_zoombiniAnimation)
		debug(1, "Puzzle: Failed to load zoombini graphics");

	// Mirror the active party for puzzle rendering.
	_puzzleZoombinis.clear();
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		_puzzleZoombinis.push_back(_vm->_state->_activeZoombinis[i]);
	}

	_stateTimer = _vm->getTotalPlayTime();
}

bool PuzzleBase::loadPrimaryLayerBackground(const Common::Path &path) {
	const bool loaded = _vm->_gfx->getPageLayerStack()->loadLayerBackground(0, path);
	_backgroundPath = loaded ? path.toString('/') : Common::String();
	return loaded;
}

void PuzzleBase::drawPrimaryPageLayer(ManagedSurface32 *screen) {
	_vm->_gfx->getPageLayerStack()->drawFirstLayer(screen, true);
}

void PuzzleBase::renderZoombinis(ManagedSurface32 *screen) const {
	Common::Array<uint> order;
	const ZoombiniRunner *draggedZoombini = nullptr;
	for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
		const ZoombiniRunner *zoombini = _puzzleZoombinis[index];
		if (!zoombini)
			continue;
		if (zoombini->isDragging()) {
			draggedZoombini = zoombini;
			continue;
		}
		order.push_back(index);
	}
	ZoombiniRunner::sortDrawOrderByY(_puzzleZoombinis, order);
	for (uint index : order)
		_vm->_gfx->drawZoombiniRunner(screen, _puzzleZoombinis[index]);
	if (draggedZoombini)
		_vm->_gfx->drawZoombiniRunner(screen, draggedZoombini);
}

const char *PuzzleChanceInfo::typeName(Type type) {
	switch (type) {
	case Type::kNone:
		return "none";
	case Type::kAmorphous:
		return "amorphous";
	case Type::kInfinite:
		return "infinite";
	case Type::kSubmit:
		return "submit";
	case Type::kMistake:
		return "mistake";
	}
	return "?";
}

Common::String PuzzleBase::debugAnswerHeader() const {
	return Common::String::format("%s (level %d, party %u)\n", _puzzleName, _puzzleLevel, _puzzleZoombinis.size());
}

Common::String PuzzleBase::debugActorDescription(int index) const {
	if (index < 0 || static_cast<int>(_puzzleZoombinis.size()) <= index)
		return "(none)";
	const ZoombiniRunner *actor = _puzzleZoombinis[index];
	const Common::Point32 position = actor->getScreenPosition();
	return Common::String::format("Zoombini near (%d, %d): %s", position.x, position.y, actor->getTraits().toStr().c_str());
}

void PuzzleBase::debugForceFinish() {
	if (_debugFinishPending)
		return;
	_debugFinishPending = true;
	for (ZoombiniRunner *actor : _puzzleZoombinis) {
		actor->clearMovement();
		actor->setAnimationCompleteCallback(nullptr);
		actor->setDragging(false);
		actor->setInputEnabled(false);
		actor->setCanAdvanceFromPage(true);
		actor->setExitComplete(true);
	}
	applyDebugPuzzleCompletion();
	_vm->_zoombiniWalkingFlag = true;
	if (_vm->_isSavedGame) {
		_vm->_mapTransitionSourcePageId = _pageId;
		_vm->requestPageChange(kPageMapTrans);
	} else {
		_vm->requestPageChange(kPageMenuPractice);
	}
}

void PuzzleBase::debugPrepareRouteDeparture() {
	if (!_vm->_state)
		return;

	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i += 1) {
		if (_vm->_state->_activeZoombinis[i])
			_vm->_state->_activeZoombinis[i]->setCanAdvanceFromPage(true);
	}
}

} // End of namespace Zoombini2
