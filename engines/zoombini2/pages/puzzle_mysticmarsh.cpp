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
#include "common/random.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_mysticmarsh.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// PuzzleMysticMarsh - swamp symbol grid puzzle.
//
// The grid has 16 columns and 12 rows in column-major order.
// Screen positions: X = 44*col + 10*row, Y = 1 + 31*row (diagonal layout).
// Crater hitboxes are 43 by 50 pixels and offset from their cell positions.
//
// Game modes select an internal level and one of six background variants.
// ============================================================================

// Launch-slot hit-test dimensions.
static const int kSlotHitWidth = 43;
static const int kSlotHitHeight = 50;

// Placement animation delay (ms)
static const uint32 kPlaceDelay = 1000;

// Completion delay before transitioning (ms)
static const uint32 kDoneDelay = 2000;

// Minimum number of released Zoombinis required for success.
static const int kMinFreed = 4;

// Resource names for the 60 symbol slots.
static const char *kSymbolNames[PuzzleMysticMarsh::kNumSymbols] = {
	"S_DIV1", "S_DIV2", "S_DIV3", "S_DIV4",
	"C_DIV1", "C_DIV2", "C_DIV3", "C_DIV4",
	"RD_CY_DIV2", "DR_CY_DIV2", "UD_CY_DIV2",
	"DU_CY_DIV2", "LR_CY_DIV2", "RL_CY_DIV2",
	"LD_CONVERGER",
	"TRIGGER1", "TRIGGER2", "TRIGGER3", "TRIGGER4",
	"TRIGGER5", "TRIGGER6", "TRIGGER7",
	"UR_TCY_DIV2", "RU_TCY_DIV2", "RD_TCY_DIV2",
	"DR_TCY_DIV2", "LR_TCY_DIV2", "RL_TCY_DIV2",
	"LL_TCY_DIV3", "LU_TCY_DIV3", "LD_TCY_DIV3",
	"RR_TCY_DIV3", "RU_TCY_DIV3", "RD_TCY_DIV3",
	"UU_TCY_DIV3", "UL_TCY_DIV3", "UR_TCY_DIV3",
	"TS_SPOT1", "TS_SPOT2", "TS_SPOT3", "TS_SPOT4",
	"TS_SPOT5", "TS_SPOT6", "TS_SPOT7",
	"TOURBI",
	"EDGE",
	"ENTRY1", "ENTRY2",
	// The remaining resource slots intentionally reuse the straight divider.
	"S_DIV1", "S_DIV1", "S_DIV1", "S_DIV1",
	"S_DIV1", "S_DIV1", "S_DIV1", "S_DIV1",
	"S_DIV1", "S_DIV1", "S_DIV1", "S_DIV1"};

PuzzleMysticMarsh::PuzzleMysticMarsh(Zoombini2Engine *vm)
	: PuzzleBase(vm, kPageMysticMarsh),
	  _state(kStateInit),
	  _freedCount(0),
	  _selectedZoombini(-1),
	  _bgIndex(1),
	  _level(1),
	  _numSlots(0),
	  _craterImage(nullptr),
	  _bubbleCraterAnim(nullptr),
	  _tourbiAnim(nullptr),
	  _musicId(-1) {

	for (int i = 0; i < kMaxCells; i++) {
		_grid[i].type = 0;
		_grid[i].symbolIdx = 0;
		_grid[i].pos = Common::Point32();
	}

	for (int i = 0; i < kNumSymbols; i++)
		_symbolImage[i] = nullptr;

	for (int f = 0; f < 4; f++)
		for (int v = 0; v < 5; v++)
			_traitImage[f][v] = nullptr;

	for (int i = 0; i < 3; i++)
		_bubbleImage[i] = nullptr;

	for (int i = 0; i < kMaxSlots; i++) {
		_slots[i].cellCol = 0;
		_slots[i].cellRow = 0;
		_slots[i].pos = Common::Point32();
		_slots[i].hitbox = Common::Rect();
		_slots[i].zoombiniIdx = -1;
		_slots[i].occupied = false;
	}
}

PuzzleMysticMarsh::~PuzzleMysticMarsh() {
	if (_musicId >= 0) {
		SoundManager *snd = _vm->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	for (int i = 0; i < kNumSymbols; i++)
		delete _symbolImage[i];

	for (int f = 0; f < 4; f++)
		for (int v = 0; v < 5; v++)
			delete _traitImage[f][v];

	for (int i = 0; i < 3; i++)
		delete _bubbleImage[i];

	delete _craterImage;
	delete _bubbleCraterAnim;
	delete _tourbiAnim;
}

void PuzzleMysticMarsh::init() {
	// Call base init for zoombini graphics and zoombini list.
	PuzzleBase::init();

	// Start the Bubble Bumpers music.
	if (SoundManager *snd = _vm->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("#sounds/music/04-BS01.wav"), true);
		if (_musicId >= 0) {
			snd->playLoop(_musicId);
			snd->setVolume(_musicId, snd->_volumeMusic);
		}
	}

	int gameMode = CLIP(_vm->getGameState()->_level, 1, 3);

	// Map game mode to level (1-4)
	switch (gameMode) {
	case 1:
		_level = 1;
		break;
	case 2:
		_level = 2;
		break;
	case 3:
		_level = 3;
		break;
	default:
		_level = 1;
		break;
	}

	// Select background index from level sub-level
	switch (_level) {
	case 1:
		_bgIndex = 1;
		break;
	case 2:
		_bgIndex = 3;
		break;
	case 3:
		_bgIndex = 4;
		break;
	case 4:
		_bgIndex = 6;
		break;
	default:
		_bgIndex = 1;
		break;
	}

	// Reload background with level-appropriate variant
	delete _background;
	_background = new BitBlock();
	Common::Path bgPath(Common::String::format("#bmp/mystic_marsh/background%d", _bgIndex));
	if (!_background->load(bgPath)) {
		debug(1, "PuzzleMysticMarsh: Failed to load background%d", _bgIndex);
		delete _background;
		_background = nullptr;
	}

	// Load puzzle resources
	loadResources();

	// Setup grid
	setupGrid();
	buildSlots();

	_freedCount = 0;
	_selectedZoombini = -1;
	_currentSequenceIdx = 0;
	_hasActiveZ = false;
	_state = kStateIdle;
	_stateTimer = _vm->getGameTickCount();

	// Generate a target launch sequence for this puzzle
	Common::RandomSource rnd("mysticmarsh_seq");
	_targetSequence.clear();
	for (int i = 0; i < (int)_puzzleZoombinis.size(); i++) {
		_targetSequence.push_back(rnd.getRandomNumber(4)); // 4 possible entrances
	}
}

void PuzzleMysticMarsh::loadResources() {
	loadTraits();
	loadSymbols();
	loadBubbles();

	// Load crater sprite
	Common::Path craterPath("bmp/mystic_marsh/crater");
	_craterImage = new RleBlock();
	if (!_craterImage->loadFromFile(craterPath)) {
		delete _craterImage;
		_craterImage = nullptr;
	}

	// Load BubbleCrater animation
	Common::Path bubbleCraterPath("bmp/mystic_marsh/BubbleCrater");
	_bubbleCraterAnim = new Animation();
	if (!_bubbleCraterAnim->loadFromFile(bubbleCraterPath)) {
		delete _bubbleCraterAnim;
		_bubbleCraterAnim = nullptr;
	}

	// Load tourbi (whirlpool) animation
	Common::Path tourbiPath("bmp/mystic_marsh/symbols/tourbi_anim");
	_tourbiAnim = new Animation();
	if (!_tourbiAnim->loadFromFile(tourbiPath)) {
		delete _tourbiAnim;
		_tourbiAnim = nullptr;
	}

	debug(2, "PuzzleMysticMarsh: Resources loaded");
}

void PuzzleMysticMarsh::loadTraits() {
	// Load five values for each of the four visible features.
	for (int f = 0; f < 4; f++) {
		for (int v = 0; v < 5; v++) {
			Common::Path traitPath(Common::String::format(
				"bmp/mystic_marsh/traits/%d-%d", f + 1, v + 1));
			_traitImage[f][v] = new RleBlock();
			if (!_traitImage[f][v]->loadFromFile(traitPath)) {
				delete _traitImage[f][v];
				_traitImage[f][v] = nullptr;
			}
		}
	}
}

void PuzzleMysticMarsh::loadSymbols() {
	// Load symbol sprites from bmp/mystic_marsh/symbols/%s.bmp
	for (int i = 0; i < kNumSymbols; i++) {
		Common::Path symPath(Common::String::format(
			"bmp/mystic_marsh/symbols/%s", kSymbolNames[i]));
		_symbolImage[i] = new RleBlock();
		if (!_symbolImage[i]->loadFromFile(symPath)) {
			delete _symbolImage[i];
			_symbolImage[i] = nullptr;
		}
	}
}

void PuzzleMysticMarsh::loadBubbles() {
	// Load bubble sprites: bubble1.bmp through bubble3.bmp
	for (int i = 0; i < 3; i++) {
		Common::Path bubblePath(Common::String::format(
			"bmp/mystic_marsh/bubble%d", i + 1));
		_bubbleImage[i] = new RleBlock();
		if (!_bubbleImage[i]->loadFromFile(bubblePath)) {
			delete _bubbleImage[i];
			_bubbleImage[i] = nullptr;
		}
	}
}

void PuzzleMysticMarsh::setupGrid() {
	// Initialize the grid before generating the current approximate layout.
	for (int i = 0; i < kMaxCells; i++) {
		_grid[i].type = 0;
		_grid[i].symbolIdx = -1;
		_grid[i].pos = Common::Point32();
	}

	// Compute screen positions for each cell.
	// Grid is column-major: index = col*kGridRows + row.
	// Offset each row horizontally to produce the diagonal grid.
	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			int idx = col * kGridRows + row;
			_grid[idx].pos = Common::Point32(44 * col + 10 * row, 1 + 31 * row);
		}
	}

	// Place the current level's predefined crater slots.
	generateRules();
}

void PuzzleMysticMarsh::generateRules() {
	// The current implementation places crater slots at regular intervals.
	// Craters are where zoombinis are placed. We place enough for
	// the puzzle pack size.

	int numZoombinis = MIN((int)_puzzleZoombinis.size(), 8);
	int numCraters = MAX(numZoombinis, 4);

	// Place craters in a staggered pattern across the grid
	static const int craterPositions[][2] = {
		// {column, row}, distributed across the full grid.
		{2, 3},
		{5, 2},
		{8, 4},
		{11, 3},
		{3, 7},
		{6, 6},
		{9, 8},
		{12, 7},
		{4, 10},
		{7, 9},
		{10, 11},
		{13, 10}};

	int maxCraters = MIN(numCraters, 12);

	for (int i = 0; i < maxCraters; i++) {
		int col = craterPositions[i][0];
		int row = craterPositions[i][1];
		int idx = col * kGridRows + row;
		_grid[idx].type = 60;
	}

	// Place some decorative symbols between craters
	Common::RandomSource *rnd = _vm->getRandom();
	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			int idx = col * kGridRows + row;
			if (_grid[idx].type != 0)
				continue;

			// Sparse symbol placement (about 30% of empty cells)
			if (rnd->getRandomNumber(99) < 30) {
				// Pick a random symbol type (2-47, excluding tourbi/edge/entry)
				int symType = rnd->getRandomNumber(43) + 2;
				_grid[idx].type = symType;
				_grid[idx].symbolIdx = symType - 2;
			}
		}
	}

	debug(2, "PuzzleMysticMarsh: Generated rules, %d craters", maxCraters);
}

void PuzzleMysticMarsh::buildSlots() {
	// Build the fixed-capacity slot array from crater cells.
	_numSlots = 0;

	for (int col = 0; col < kGridCols && _numSlots < kMaxSlots; col++) {
		for (int row = 0; row < kGridRows && _numSlots < kMaxSlots; row++) {
			int idx = col * kGridRows + row;
			if (_grid[idx].type != 60 && _grid[idx].type != 61)
				continue;

			Slot &slot = _slots[_numSlots];
			slot.cellCol = col;
			slot.cellRow = row;

			// Offset the clickable crater area from the cell's drawing position.
			const Common::Point32 cellPos = _grid[idx].pos;
			slot.pos = Common::Point32(cellPos.x - 25, cellPos.y - 10);

			slot.hitbox = Common::Rect(
				static_cast<int16>(cellPos.x - 8),                    // left
				static_cast<int16>(cellPos.y + 52),                   // top
				static_cast<int16>(cellPos.x - 8 + kSlotHitWidth),    // right
				static_cast<int16>(cellPos.y + 52 + kSlotHitHeight)); // bottom

			slot.zoombiniIdx = -1;
			slot.occupied = false;

			_numSlots++;
		}
	}

	debug(2, "PuzzleMysticMarsh: Built %d slots", _numSlots);
}

void PuzzleMysticMarsh::launchZoombini(int entranceIdx) {
	if (_hasActiveZ)
		return;
	if (_currentSequenceIdx >= (int)_puzzleZoombinis.size())
		return;

	_hasActiveZ = true;
	_activeZ.zoombiniIdx = _currentSequenceIdx;

	// Entrance positions (simplified: 4 entrances across the bottom)
	_activeZ.cellCol = 2 + entranceIdx * 3;
	_activeZ.cellRow = 11;
	_activeZ.targetPos = _grid[_activeZ.cellCol * kGridRows + _activeZ.cellRow].pos;
	_activeZ.moveStartTime = _vm->getGameTickCount();

	_state = kStateLaunching;
	_stateTimer = _vm->getGameTickCount();

	debug(2, "PuzzleMysticMarsh: Launching zoombini %d from entrance %d",
		  _activeZ.zoombiniIdx, entranceIdx);
}

void PuzzleMysticMarsh::moveZoombini() {
	if (!_hasActiveZ)
		return;

	uint32 now = _vm->getGameTickCount();
	uint32 elapsed = now - _activeZ.moveStartTime;

	// Move one cell every 500ms
	if (elapsed > 500) {
		// Simple movement: move upwards (towards exit)
		// In a full implementation, this would check _grid[idx].type for arrows/diverters
		_activeZ.cellRow--;

		if (_activeZ.cellRow < 0) {
			// Reached the exit!
			_state = kStateFreeing;
			_stateTimer = now;
			return;
		}

		_activeZ.targetPos = _grid[_activeZ.cellCol * kGridRows + _activeZ.cellRow].pos;
		_activeZ.moveStartTime = now;
	}
}

void PuzzleMysticMarsh::freeZoombini(int zoombiniIdx) {
	_puzzleZoombinis[zoombiniIdx]->_puzzleStatus = 0;
	_freedCount++;
	_currentSequenceIdx++;
	_hasActiveZ = false;
	debug(1, "PuzzleMysticMarsh: Freed zoombini %d (total: %d)", zoombiniIdx, _freedCount);
}

int PuzzleMysticMarsh::countFreeZoombinis() const {
	int count = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_puzzleStatus == 0)
			count++;
	}
	return count;
}

EventHandleResult PuzzleMysticMarsh::onLButtonDown(const Common::Point &pos) {
	if (_state != kStateIdle)
		return EventHandleResult::kPassthrough;

	// Check for clicks on the 4 launch entrances at the bottom
	for (int i = 0; i < 4; i++) {
		int ex = 2 + i * 3;
		int ey = 11;
		const Common::Point32 screenPos = _grid[ex * kGridRows + ey].pos;

		Common::Rect entranceHitbox(
			static_cast<int16>(screenPos.x - 20),
			static_cast<int16>(screenPos.y),
			static_cast<int16>(screenPos.x + 20),
			static_cast<int16>(screenPos.y + 50));
		if (entranceHitbox.contains(pos)) {
			launchZoombini(i);
			return EventHandleResult::kConsumed;
		}
	}
	return EventHandleResult::kPassthrough;
}

void PuzzleMysticMarsh::onUpdate() {
	uint32 now = _vm->getGameTickCount();
	uint32 elapsed = now - _stateTimer;

	switch (_state) {
	case kStateInit:
		break;

	case kStateIdle:
		break;

	case kStateLaunching:
		moveZoombini();
		if (_state == kStateFreeing) {
			_stateTimer = now;
		}
		break;

	case kStateFreeing:
		if (elapsed > kPlaceDelay) {
			freeZoombini(_activeZ.zoombiniIdx);
			_state = kStateIdle;
			_stateTimer = now;

			if (_freedCount >= kMinFreed) {
				_state = kStateDone;
			}
		}
		break;

	case kStatePopping:
		if (elapsed > kPlaceDelay) {
			_hasActiveZ = false;
			_state = kStateIdle;
			_stateTimer = now;
		}
		break;

	case kStateDone:
		if (elapsed > kDoneDelay) {
			debug(1, "PuzzleMysticMarsh: Complete, %d zoombinis freed", _freedCount);
			_vm->_returningFromPuzzle = true;
			_vm->_mapTransitionSourcePageId = kPageMysticMarsh;
			_vm->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void PuzzleMysticMarsh::onRenderBackground(ManagedSurface32 *screen) {
	// Draw background
	if (_background)
		_background->drawToSurface(screen, Common::Point32(0, 0));

}

void PuzzleMysticMarsh::onRenderScene(ManagedSurface32 *screen) {
	// Draw grid elements
	drawGrid(screen);

}

void PuzzleMysticMarsh::drawGrid(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			int idx = col * kGridRows + row;
			const GridCell &cell = _grid[idx];

			if (cell.type < 2 || cell.type == 62)
				continue;

			if (cell.type == 60 || cell.type == 61)
				continue; // Craters drawn separately

			// Draw symbol sprite
			int symIdx = cell.type - 2;
			if (symIdx >= 0 && symIdx < kNumSymbols && _symbolImage[symIdx]) {
				_symbolImage[symIdx]->drawToScreen(screen, cell.pos, lut);
			}
		}
	}

	// Draw tourbi animation if present
	if (_tourbiAnim && _tourbiAnim->getFrameCount() > 0) {
		uint32 now = _vm->getGameTickCount();
		int frame = (now / 100) % _tourbiAnim->getFrameCount();
		const RleBlock *frameImage = _tourbiAnim->getFrame(frame);
		if (frameImage) {
			// Draw the whirlpool at its fixed origin.
			// Actual position depends on grid cell with TOURBI type
			for (int i = 0; i < kMaxCells; i++) {
				if (_grid[i].type == 46) { // TOURBI = symbol index 44, type = 44+2 = 46
					frameImage->drawToScreen(screen, _grid[i].pos, lut);
				}
			}
		}
	}
}

void PuzzleMysticMarsh::onRenderActors(ManagedSurface32 *screen) {
	if (!_zoombiniAnimation || !_hasActiveZ)
		return;

	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	// Draw the currently active zoombini at its interpolated position
	int zIdx = _activeZ.zoombiniIdx;
	if (zIdx >= 0 && zIdx < (int)_puzzleZoombinis.size()) {
		const ZoombiniState *z = _puzzleZoombinis[zIdx];
		const Common::Point32 pos = _activeZ.targetPos;

		_zoombiniAnimation->drawZoombini(screen, z->_traits, pos, 0, 0, lut);
	}
}

} // End of namespace Zoombini2
