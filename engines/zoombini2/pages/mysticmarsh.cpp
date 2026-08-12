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

#include "zoombini2/pages/mysticmarsh.h"
#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"
#include "zoombini2/sound.h"

namespace Zoombini2 {

// ============================================================================
// MysticMarshPuzzle — Swamp symbol grid puzzle.
//
// Original: MysticMarsh__Init_426770. Object size 0xC268 (49768 bytes).
//
// Grid layout: 16 columns × 12 rows, column-major order.
// Screen positions: X = 44*col + 10*row, Y = 1 + 31*row (diagonal layout).
// Crater hitbox: 43×50 pixels, offset from cell position.
//
// Difficulty: game mode (1-3) maps to internal difficulty (1-4):
//   Mode 1 → difficulty 1 (Level1A or 1B, 30 random attempts, best-of)
//   Mode 2 → difficulty 2 (Level2)
//   Mode 3 → difficulty 3 or 4 (Level3/Level4)
//
// Background: 6 variants selected by difficulty sub-level via switch on
//   *(gridObj+8): 0→bg1, 1→bg2, 2-5→bg3, 6→bg4, 7→bg5, 8→bg6.
//
// Resource paths (from IDA string table):
//   bmp/mystic_marsh/background%d.bmp, area%d.bmt
//   bmp/mystic_marsh/traits/%d-%d.bmp (feature 1-4, value 1-5)
//   bmp/mystic_marsh/symbols/%s.bmp (60 symbol names)
//   bmp/mystic_marsh/bubble%d.bmp (1-3)
//   bmp/mystic_marsh/crater.bmp, BubbleCrater.an
//   bmp/mystic_marsh/symbols/tourbi_anim.an
// ============================================================================

// Slot hitbox dimensions (from IDA: +43 width, +50 height)
static const int kSlotHitWidth = 43;
static const int kSlotHitHeight = 50;

// Placement animation delay (ms)
static const uint32 kPlaceDelay = 1000;

// Completion delay before transitioning (ms)
static const uint32 kDoneDelay = 2000;

// Minimum freed zoombinis for success (from CheckFree: >= 4)
static const int kMinFreed = 4;

// Symbol name table (60 entries, from off_48E69C → off_48E78C)
static const char *kSymbolNames[MysticMarshPuzzle::kNumSymbols] = {
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
	// Pad remaining entries (60 total, some may be duplicates in the table)
	"S_DIV1", "S_DIV1", "S_DIV1", "S_DIV1",
	"S_DIV1", "S_DIV1", "S_DIV1", "S_DIV1",
	"S_DIV1", "S_DIV1", "S_DIV1", "S_DIV1"
};

// Helper to get feature by axis index (0=hair, 1=eyes, 2=nose, 3=feet)
// Unused for now but will be needed for full rule generation.
#if 0
static byte getFeature(const Zoombini *z, int axis) {
	switch (axis) {
	case 0: return z->_featureA;
	case 1: return z->_featureB;
	case 2: return z->_featureC;
	case 3: return z->_featureD;
	default: return 0;
	}
}
#endif

MysticMarshPuzzle::MysticMarshPuzzle(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageMysticMarsh),
	  _state(kStateInit),
	  _freedCount(0),
	  _selectedZoombini(-1),
	  _bgIndex(1),
	  _difficulty(1),
	  _numSlots(0),
	  _craterGfx(nullptr),
	  _bubbleCraterAnim(nullptr),
	  _tourbiAnim(nullptr),
	  _musicId(-1) {

	memset(_grid, 0, sizeof(_grid));

	for (int i = 0; i < kNumSymbols; i++)
		_symbolGfx[i] = nullptr;

	for (int f = 0; f < 4; f++)
		for (int v = 0; v < 5; v++)
			_traitGfx[f][v] = nullptr;

	for (int i = 0; i < 3; i++)
		_bubbleGfx[i] = nullptr;

	for (int i = 0; i < kMaxSlots; i++) {
		_slots[i].cellCol = 0;
		_slots[i].cellRow = 0;
		_slots[i].x = 0;
		_slots[i].y = 0;
		_slots[i].hitbox = Common::Rect();
		_slots[i].zoombiniIdx = -1;
		_slots[i].occupied = false;
	}
}

MysticMarshPuzzle::~MysticMarshPuzzle() {
	if (_musicId >= 0) {
		SoundManager *snd = _engine->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	for (int i = 0; i < kNumSymbols; i++)
		delete _symbolGfx[i];

	for (int f = 0; f < 4; f++)
		for (int v = 0; v < 5; v++)
			delete _traitGfx[f][v];

	for (int i = 0; i < 3; i++)
		delete _bubbleGfx[i];

	delete _craterGfx;
	delete _bubbleCraterAnim;
	delete _tourbiAnim;
}

void MysticMarshPuzzle::init() {
	// Call base init for zoombini graphics and zoombini list.
	PuzzlePage::init();

	// BGM: 04-BS01.wav (IDA: MysticMarsh__Init_426770)
	if (SoundManager *snd = _engine->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("sounds/music/04-BS01.wav"), true);
		if (_musicId >= 0) { snd->playLoop(_musicId); snd->setVolume(_musicId, snd->_volumeMusic); }
	}

	int gameMode = CLIP(_engine->getGameState()->_gameMode, 1, 3);

	// Map game mode to difficulty (1-4)
	switch (gameMode) {
	case 1: _difficulty = 1; break;
	case 2: _difficulty = 2; break;
	case 3: _difficulty = 3; break;
	default: _difficulty = 1; break;
	}

	// Select background index from difficulty sub-level
	switch (_difficulty) {
	case 1: _bgIndex = 1; break;
	case 2: _bgIndex = 3; break;
	case 3: _bgIndex = 4; break;
	case 4: _bgIndex = 6; break;
	default: _bgIndex = 1; break;
	}

	// Reload background with difficulty-appropriate variant
	delete _background;
	_background = new BitBlock();
	Common::Path bgPath(Common::String::format("bmp/mystic_marsh/background%d", _bgIndex));
	if (!_background->load(bgPath)) {
		debug(1, "MysticMarshPuzzle: Failed to load background%d", _bgIndex);
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
	_stateTimer = _engine->getGameTickCount();

	// Generate a target launch sequence for this puzzle
	Common::RandomSource rnd("mysticmarsh_seq");
	_targetSequence.clear();
	for (int i = 0; i < (int)_puzzleZoombinis.size(); i++) {
		_targetSequence.push_back(rnd.getRandomNumber(4)); // 4 possible entrances
	}
}

void MysticMarshPuzzle::loadResources() {
	loadTraits();
	loadSymbols();
	loadBubbles();

	// Load crater sprite
	Common::Path craterPath("bmp/mystic_marsh/crater");
	_craterGfx = new RleBlock();
	if (!_craterGfx->loadFromFile(craterPath)) {
		delete _craterGfx;
		_craterGfx = nullptr;
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

	debug(2, "MysticMarshPuzzle: Resources loaded");
}

void MysticMarshPuzzle::loadTraits() {
	// Load trait icons: 4 features × 5 values
	// Original: bmp/mystic_marsh/traits/%d-%d.bmp (feature 1-4, value 1-5)
	for (int f = 0; f < 4; f++) {
		for (int v = 0; v < 5; v++) {
			Common::Path traitPath(Common::String::format(
				"bmp/mystic_marsh/traits/%d-%d", f + 1, v + 1));
			_traitGfx[f][v] = new RleBlock();
			if (!_traitGfx[f][v]->loadFromFile(traitPath)) {
				delete _traitGfx[f][v];
				_traitGfx[f][v] = nullptr;
			}
		}
	}
}

void MysticMarshPuzzle::loadSymbols() {
	// Load symbol sprites from bmp/mystic_marsh/symbols/%s.bmp
	for (int i = 0; i < kNumSymbols; i++) {
		Common::Path symPath(Common::String::format(
			"bmp/mystic_marsh/symbols/%s", kSymbolNames[i]));
		_symbolGfx[i] = new RleBlock();
		if (!_symbolGfx[i]->loadFromFile(symPath)) {
			delete _symbolGfx[i];
			_symbolGfx[i] = nullptr;
		}
	}
}

void MysticMarshPuzzle::loadBubbles() {
	// Load bubble sprites: bubble1.bmp through bubble3.bmp
	for (int i = 0; i < 3; i++) {
		Common::Path bubblePath(Common::String::format(
			"bmp/mystic_marsh/bubble%d", i + 1));
		_bubbleGfx[i] = new RleBlock();
		if (!_bubbleGfx[i]->loadFromFile(bubblePath)) {
			delete _bubbleGfx[i];
			_bubbleGfx[i] = nullptr;
		}
	}
}

void MysticMarshPuzzle::setupGrid() {
	// Initialize grid to empty.
	// Original grid is populated by GenerateRules → GenerateLevel*.
	// For now, use a simplified layout.
	for (int i = 0; i < kMaxCells; i++) {
		_grid[i].type = 0;
		_grid[i].symbolIdx = -1;
		_grid[i].x = 0;
		_grid[i].y = 0;
	}

	// Compute screen positions for each cell.
	// Grid is column-major: index = col*kGridRows + row.
	// Screen positions from IDA: X = 44*col + 10*row, Y = 1 + 31*row.
	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			int idx = col * kGridRows + row;
			_grid[idx].x = 44 * col + 10 * row;
			_grid[idx].y = 1 + 31 * row;
		}
	}

	// Place crater slots at predefined positions based on difficulty.
	// The original GenerateLevel* functions create complex connected paths,
	// but for initial implementation we place craters in a workable pattern.
	generateRules();
}

void MysticMarshPuzzle::generateRules() {
	// Simplified rule generation.
	// Original has 4 difficulty levels with complex graph-based placement
	// (MysticMarsh__GenerateLevel1A_441D30 etc., ~3000 bytes each).
	//
	// For now: place crater slots (type 60) at regular intervals.
	// Craters are where zoombinis are placed. We place enough for
	// the puzzle pack size.

	int numZoombinis = MIN((int)_puzzleZoombinis.size(), 8);
	int numCraters = MAX(numZoombinis, 4);

	// Place craters in a staggered pattern across the grid
	static const int craterPositions[][2] = {
		// {col, row} — distributed across the 16×12 grid
		{ 2,  3}, { 5,  2}, { 8,  4}, {11,  3},
		{ 3,  7}, { 6,  6}, { 9,  8}, {12,  7},
		{ 4, 10}, { 7,  9}, {10, 11}, {13, 10}
	};

	int maxCraters = MIN(numCraters, 12);

	for (int i = 0; i < maxCraters; i++) {
		int col = craterPositions[i][0];
		int row = craterPositions[i][1];
		int idx = col * kGridRows + row;
		_grid[idx].type = 60;
	}

	// Place some decorative symbols between craters
	Common::RandomSource *rnd = _engine->getRandom();
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

	debug(2, "MysticMarshPuzzle: Generated rules, %d craters", maxCraters);
}

void MysticMarshPuzzle::buildSlots() {
	// Build slot array from grid craters (type 60/61).
	// Original: slots stored in malloc'd Block, 44 bytes each (11 DWORDs).
	_numSlots = 0;

	for (int col = 0; col < kGridCols && _numSlots < kMaxSlots; col++) {
		for (int row = 0; row < kGridRows && _numSlots < kMaxSlots; row++) {
			int idx = col * kGridRows + row;
			if (_grid[idx].type != 60 && _grid[idx].type != 61)
				continue;

			Slot &slot = _slots[_numSlots];
			slot.cellCol = col;
			slot.cellRow = row;

			// Screen positions from IDA:
			// Hitbox left = 44*col + 10*row - 8
			// Hitbox top  = 31*row + 53
			int cellX = 44 * col + 10 * row;
			int cellY = 1 + 31 * row;

			slot.x = cellX - 25;   // Animation X offset
			slot.y = cellY - 10;   // Animation Y offset

			slot.hitbox = Common::Rect(
				cellX - 8,           // left
				cellY + 52,          // top (v36 + 52 = 1 + 31*row + 52)
				cellX - 8 + kSlotHitWidth,   // right
				cellY + 52 + kSlotHitHeight  // bottom
			);

			slot.zoombiniIdx = -1;
			slot.occupied = false;

			_numSlots++;
		}
	}

	debug(2, "MysticMarshPuzzle: Built %d slots", _numSlots);
}

void MysticMarshPuzzle::launchZoombini(int entranceIdx) {
	if (_hasActiveZ) return;
	if (_currentSequenceIdx >= (int)_puzzleZoombinis.size()) return;

	_hasActiveZ = true;
	_activeZ.zoombiniIdx = _currentSequenceIdx;
	
	// Entrance positions (simplified: 4 entrances across the bottom)
	_activeZ.cellCol = 2 + entranceIdx * 3;
	_activeZ.cellRow = 11;
	_activeZ.targetX = _grid[_activeZ.cellCol * kGridRows + _activeZ.cellRow].x;
	_activeZ.targetY = _grid[_activeZ.cellCol * kGridRows + _activeZ.cellRow].y;
	_activeZ.moveStartTime = _engine->getGameTickCount();

	_state = kStateLaunching;
	_stateTimer = _engine->getGameTickCount();
	
	debug(2, "MysticMarshPuzzle: Launching zoombini %d from entrance %d", 
		   _activeZ.zoombiniIdx, entranceIdx);
}

void MysticMarshPuzzle::moveZoombini() {
	if (!_hasActiveZ) return;

	uint32 now = _engine->getGameTickCount();
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

		_activeZ.targetX = _grid[_activeZ.cellCol * kGridRows + _activeZ.cellRow].x;
		_activeZ.targetY = _grid[_activeZ.cellCol * kGridRows + _activeZ.cellRow].y;
		_activeZ.moveStartTime = now;
	}
}

void MysticMarshPuzzle::freeZoombini(int zoombiniIdx) {
	_puzzleZoombinis[zoombiniIdx]->_freeStatus = 0;
	_freedCount++;
	_currentSequenceIdx++;
	_hasActiveZ = false;
	debug(1, "MysticMarshPuzzle: Freed zoombini %d (total: %d)", zoombiniIdx, _freedCount);
}

int MysticMarshPuzzle::countFreeZoombinis() const {
	int count = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_freeStatus == 0)
			count++;
	}
	return count;
}

void MysticMarshPuzzle::handleClick(const Common::Point &pos) {
	if (_state != kStateIdle)
		return;

	// Check for clicks on the 4 launch entrances at the bottom
	for (int i = 0; i < 4; i++) {
		int ex = 2 + i * 3;
		int ey = 11;
		int screenX = _grid[ex * kGridRows + ey].x;
		int screenY = _grid[ex * kGridRows + ey].y;
		
		Common::Rect entranceHitbox(screenX - 20, screenY, screenX + 20, screenY + 50);
		if (entranceHitbox.contains(pos)) {
			launchZoombini(i);
			return;
		}
	}
}

void MysticMarshPuzzle::update() {
	uint32 now = _engine->getGameTickCount();
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
			debug(1, "MysticMarshPuzzle: Complete, %d zoombinis freed", _freedCount);
			_engine->_returningFromPuzzle = true;
			_engine->_maptransSourceWorld = kPageMysticMarsh;
			_engine->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void MysticMarshPuzzle::draw(Graphics::ManagedSurface *screen) {
	// Draw background
	if (_background)
		_background->drawToSurface(screen, 0, 0);

	// Draw grid elements
	drawGrid(screen);

	// Draw active zoombini if any
	if (_hasActiveZ) {
		drawZoombinis(screen);
	}
}

void MysticMarshPuzzle::drawGrid(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			int idx = col * kGridRows + row;
			const GridCell &cell = _grid[idx];

			if (cell.type < 2 || cell.type == 62)
				continue;

			if (cell.type == 60 || cell.type == 61)
				continue;  // Craters drawn separately

			// Draw symbol sprite
			int symIdx = cell.type - 2;
			if (symIdx >= 0 && symIdx < kNumSymbols && _symbolGfx[symIdx]) {
				_symbolGfx[symIdx]->drawToScreen(screen, cell.x, cell.y, lut);
			}
		}
	}

	// Draw tourbi animation if present
	if (_tourbiAnim && _tourbiAnim->getFrameCount() > 0) {
		uint32 now = _engine->getGameTickCount();
		int frame = (now / 100) % _tourbiAnim->getFrameCount();
		const RleBlock *frameGfx = _tourbiAnim->getFrame(frame);
		if (frameGfx) {
			// Tourbi at fixed position (54×105, from Init: 0, 0)
			// Actual position depends on grid cell with TOURBI type
			for (int i = 0; i < kMaxCells; i++) {
				if (_grid[i].type == 46) { // TOURBI = symbol index 44, type = 44+2 = 46
					frameGfx->drawToScreen(screen, _grid[i].x, _grid[i].y, lut);
				}
			}
		}
	}
}

void MysticMarshPuzzle::drawSlots(Graphics::ManagedSurface *screen) {
	// No longer used.
}

void MysticMarshPuzzle::drawZoombinis(Graphics::ManagedSurface *screen) {
	if (!_zoombiniGfx || !_hasActiveZ)
		return;

	const byte (*lut)[256] = _engine->getAlphaLUT();
	
	// Draw the currently active zoombini at its interpolated position
	int zIdx = _activeZ.zoombiniIdx;
	if (zIdx >= 0 && zIdx < (int)_puzzleZoombinis.size()) {
		const Zoombini *z = _puzzleZoombinis[zIdx];
		int x = _activeZ.targetX;
		int y = _activeZ.targetY;

		// Draw body
		const RleBlock *frame = _zoombiniGfx->getFrame(0, 0);
		if (frame)
			frame->drawToScreen(screen, x, y, lut);

		// Draw features
		const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
		for (int slot2 = 1; slot2 <= 4; slot2++) {
			int featIdx = slot2 * ZoombiniGfx::kDim2 + features[slot2 - 1];
			frame = _zoombiniGfx->getFrame(featIdx, 0);
			if (frame)
				frame->drawToScreen(screen, x, y, lut);
		}
	}
}

} // End of namespace Zoombini2
