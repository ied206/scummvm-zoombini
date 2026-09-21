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

constexpr const char *PuzzleBase::kPuzzleBackgroundFormat;
constexpr const char *PuzzleBase::kZoombiniAnimationPath;
constexpr const char *PuzzleBase::kCrazyTurtleBackgroundPath;
constexpr const char *PuzzleBase::kWaterslideBackgroundPath;
constexpr const char *PuzzleBase::kAquacubeBackgroundPath;
constexpr const char *PuzzleBase::kMysticMarshBackgroundPath;
constexpr const char *PuzzleBase::kMagicWallBackgroundPath;
constexpr const char *PuzzleBase::kWallOfFleensBackgroundPath;
constexpr const char *PuzzleBase::kChezNorfBackgroundPath;
constexpr const char *PuzzleBase::kSnowboardBackgroundPath;
constexpr const char *PuzzleBase::kBooliesBackgroundPath;

// Public activity names paired with their internal resource directories.
static constexpr struct {
	PageId pageId;
	const char *name;
	const char *dir;
	const char *bgName; // Background BMP name (without bmp/ prefix or .bmp extension)
} kPuzzleInfo[] = {
	{kPageCrazyTurtle, "Turtle Hurdle", "crazy_turtle", "crazy_turtle/background"},
	{kPageWaterslide, "Pipes of Paloo", "waterslide", "waterslide/waterslides"},
	{kPageAquacube, "Aqua Cube", "aquacube", "aquacube/background"},
	{kPageMysticMarsh, "Bubble Bumpers", "mystic_marsh", "mystic_marsh/background1"},
	{kPageMagicWall, "Beetle Bug Alley", "magic_wall", "magic_wall/magic wall"},
	{kPageWallOfFleens, "Magic Mirrors", "wall_of_fleens", "wall_of_fleens/background"},
	{kPageChezNorf, "Chez Norf", "chez_norf", "chez_norf/baquegund"},
	{kPageSnowboard, "Snowboard Gulch", "snowboard", "snowboard/snowboard-EASY"},
	{kPageBoolies, "Boolie Boggle", "boolies", "Boolies/background"},
	{kPageNone, nullptr, nullptr, nullptr},
};

/* static */
const char *PuzzleBase::getPuzzleName(PageId pageId) {
	for (int i = 0; kPuzzleInfo[i].name; i++) {
		if (kPuzzleInfo[i].pageId == pageId)
			return kPuzzleInfo[i].name;
	}
	return "Unknown";
}

/* static */
const char *PuzzleBase::getPuzzleDir(PageId pageId) {
	for (int i = 0; kPuzzleInfo[i].dir; i++) {
		if (kPuzzleInfo[i].pageId == pageId)
			return kPuzzleInfo[i].dir;
	}
	return nullptr;
}

PuzzleBase::PuzzleBase(Zoombini2Engine *vm, PageId pageId)
	: InteractiveBase(vm), _puzzleLevel(vm->_state->_level) {
	_pageId = pageId;
}

void PuzzleBase::finishPuzzleRoster(BoardRecord **board) {
	_vm->_state->finishPuzzleRoster(_pageId, board, _vm->isStartingMapTransition(), _vm->_isSavedGame);
}

void PuzzleBase::init() {
	const char *name = getPuzzleName(_pageId);
	debug(1, "Puzzle::init - %s (page %d)", name, static_cast<int>(_pageId));

	// Load the background from the activity resource table.
	const char *bgName = nullptr;
	for (int i = 0; kPuzzleInfo[i].name; i++) {
		if (kPuzzleInfo[i].pageId == _pageId) {
			bgName = kPuzzleInfo[i].bgName;
			break;
		}
	}

	_vm->_gfx->getPageLayerStack()->clear();
	_backgroundPath.clear();
	_vm->_gfx->getPageLayerStack()->addLayer(1);
	if (bgName) {
		const Common::Path bgPath(Common::String::format(kPuzzleBackgroundFormat, bgName));
		if (!loadPrimaryLayerBackground(bgPath)) {
			debug(1, "Puzzle: Failed to load background for %s", name);
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

	_stateTimer = _vm->getGameTickCount();
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
		if (zoombini->_dragging) {
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
	return Common::String::format("%s (level %d, party %u)\nIndices below are one-based.\n", getPuzzleName(_pageId), _puzzleLevel, _puzzleZoombinis.size());
}

Common::String PuzzleBase::debugActorDescription(int index) const {
	if (index < 0 || static_cast<int>(_puzzleZoombinis.size()) <= index)
		return "(none)";
	return Common::String::format("Zoombini %d [%s]", index + 1, _puzzleZoombinis[index]->_traits.toStr().c_str());
}

void PuzzleBase::debugForceFinish() {
	if (_debugFinishPending)
		return;
	_debugFinishPending = true;
	for (ZoombiniRunner *actor : _puzzleZoombinis) {
		actor->clearMovement();
		actor->setAnimationCompleteCallback(nullptr);
		actor->_dragging = false;
		actor->_inputEnabled = false;
		actor->_puzzleStatus = 1;
		actor->_exitComplete = true;
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

} // End of namespace Zoombini2
