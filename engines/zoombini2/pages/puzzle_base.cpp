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
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

static const struct {
	int id;
	const char *name;
	const char *dir;
	const char *bgName; // Background BMP name (without bmp/ prefix or .bmp extension)
} kPuzzleInfo[] = {
	// Public activity names paired with their internal resource directories.
	{kPageCrazyTurtle, "Turtle Hurdle", "crazy_turtle", "crazy_turtle/background"},
	{kPageWaterSlide, "Pipes of Paloo", "waterslide", "waterslide/waterslides"},
	{kPageAquaCube, "Aqua Cube", "aquacube", "aquacube/background"},
	{kPageMysticMarsh, "Bubble Bumpers", "mystic_marsh", "mystic_marsh/background1"},
	{kPageMagicWall, "Beetle Bug Alley", "magic_wall", "magic_wall/magic wall"},
	{kPageWallOfFleens, "Magic Mirrors", "wall_of_fleens", "wall_of_fleens/background"},
	{kPageChezNorf, "Chez Norf", "chez_norf", "chez_norf/baquegund"},
	{kPageSnowboard, "Snowboard Gulch", "snowboard", "snowboard/snowboard-EASY"},
	{kPageBoolies, "Boolie Boggle", "boolies", "Boolies/background"},
	{0, nullptr, nullptr, nullptr}};

/* static */
const char *PuzzleBase::getPuzzleName(int puzzleId) {
	for (int i = 0; kPuzzleInfo[i].name; i++) {
		if (kPuzzleInfo[i].id == puzzleId)
			return kPuzzleInfo[i].name;
	}
	return "Unknown";
}

/* static */
const char *PuzzleBase::getPuzzleDir(int puzzleId) {
	for (int i = 0; kPuzzleInfo[i].dir; i++) {
		if (kPuzzleInfo[i].id == puzzleId)
			return kPuzzleInfo[i].dir;
	}
	return nullptr;
}

PuzzleBase::PuzzleBase(Zoombini2Engine *vm, int puzzleId)
	: InteractiveBase(vm), _puzzleId(puzzleId), _background(nullptr),
	  _zoombiniAnimation(nullptr), _stateTimer(0) {
	_pageId = puzzleId;
}

PuzzleBase::~PuzzleBase() {
	delete _background;
}

void PuzzleBase::init() {
	const char *name = getPuzzleName(_puzzleId);
	debug(1, "Puzzle::init - %s (page %d)", name, _puzzleId);

	// Load the background from the activity resource table.
	const char *bgName = nullptr;
	for (int i = 0; kPuzzleInfo[i].name; i++) {
		if (kPuzzleInfo[i].id == _puzzleId) {
			bgName = kPuzzleInfo[i].bgName;
			break;
		}
	}

	if (bgName) {
		Common::Path bgPath(Common::String::format("#bmp/%s", bgName));
		_background = new BitBlock();
		if (!_background->load(bgPath)) {
			debug(1, "Puzzle: Failed to load background for %s", name);
			delete _background;
			_background = nullptr;
		}
	}

	_zoombiniAnimation = _vm->loadZoombiniAnimation(Common::Path("bmp/zombis/littleZomb.anm"));
	if (!_zoombiniAnimation)
		debug(1, "Puzzle: Failed to load zoombini graphics");

	// Transfer zoombinis from global to puzzle
	_puzzleZoombinis.clear();
	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		_puzzleZoombinis.push_back(_vm->_globalZoombinis[i]);
	}

	_stateTimer = _vm->getGameTickCount();
}

void PuzzleBase::renderZoombinis(ManagedSurface32 *screen) const {
	Common::Array<uint> order;
	const ZoombiniState *draggedZoombini = nullptr;
	for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
		const ZoombiniState *zoombini = _puzzleZoombinis[index];
		if (!zoombini)
			continue;
		if (zoombini->_dragging) {
			draggedZoombini = zoombini;
			continue;
		}
		uint insertion = order.size();
		order.push_back(index);
		while (0 < insertion && zoombini->_screenPos.y < _puzzleZoombinis[order[insertion - 1]]->_screenPos.y) {
			order[insertion] = order[insertion - 1];
			insertion -= 1;
		}
		order[insertion] = index;
	}
	for (uint index : order)
		_puzzleZoombinis[index]->draw(screen, _vm->getAlphaLUT());
	if (draggedZoombini)
		draggedZoombini->draw(screen, _vm->getAlphaLUT());
}

} // End of namespace Zoombini2
