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
#include "zoombini2/pages/puzzle_cheznorf.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// PuzzleChezNorf - restaurant food-matching puzzle.
//
// Layout: Norf character at center, tables arranged right at x=205+i*85.
// Food board grid drawn in 3 sections at left side of screen.
// Each section = one food category (slurp, miam, glouglou).
//
// Food grid (from DrawBoard_40F9B0):
//   Section 1: y=47..92, step 15px (3 rows)
//   Section 2: y=104..149, step 15px
//   Section 3: y=164..209, step 15px
//   Columns: x=51, step 14px
//
// Table positions: X = 205 + i*85, plates at y=500.
//
// Background: comande3 for level one, comande1 for level two, and comande2 for level three.
// ============================================================================

// Delay constants
static const uint32 kServeDelay = 1500;
static const uint32 kMatchDelay = 1000;
static const uint32 kRejectDelay = 2000;
static const uint32 kDoneDelay = 3000;

// Minimum number of released Zoombinis required for success.
static const int kMinFreed = 4;

// Position of the first table plate.
static const Common::Point32 kFirstTablePos(205, 500);

// Tolerance thresholds per level (from CheckFoodMatch)
// Level 1: 2 wrong guesses before reject, Level 2: 1, Level 3: 0
static const int kWrongTolerance[] = {0, 2, 1, 0};

// Attempt limits indexed by level, with index zero unused.
static const int kMaxAttempts[] = {0, 5, 4, 3};

// Visible clue-attribute counts indexed by level, with index zero unused.
static const int kClueAttrCount[] = {0, 8, 7, 8};

// ============================================================================
// Construction / Destruction
// ============================================================================

PuzzleChezNorf::PuzzleChezNorf(Zoombini2Engine *vm)
	: PuzzleBase(vm, kPageChezNorf),
	  _state(kStateInit), _numTables(4), _level(1),
	  _freedCount(0), _currentTable(-1), _selectedFood(kFoodNone),
	  _wrongCount(0), _maxAttempts(0), _clueAttrCount(0), _norfState(0),
	  _templateId(11), _pendingSlurp(-1), _pendingMiam(-1), _pendingGlouglou(-1),
	  _symbOK(nullptr), _symbNO(nullptr), _symbMaybe(nullptr),
	  _plato(nullptr), _platoMini(nullptr),
	  _norfDefault(nullptr), _highlightImage(nullptr), _debugFont(nullptr),
	  _musicId(-1) {

	for (int i = 0; i < 3; i++) {
		_slurpImage[i] = nullptr;
		_miamImage[i] = nullptr;
		_glouglouImage[i] = nullptr;
		_comandeImage[i] = nullptr;
	}

	memset(_foodGrid, 0, sizeof(_foodGrid));
	memset(_foodVals, 0, sizeof(_foodVals));

	for (int i = 0; i < kMaxTables; i++) {
		_tables[i].pos = Common::Point32();
		_tables[i].hitbox = Common::Rect();
		_tables[i].zoombiniIdx = -1;
		_tables[i].foodSlurp = -1;
		_tables[i].foodMiam = -1;
		_tables[i].foodGlouglou = -1;
		_tables[i].served = false;
		_tables[i].completed = false;

		_answers[i].slurp = 0;
		_answers[i].miam = 0;
		_answers[i].glouglou = 0;
	}
}

PuzzleChezNorf::~PuzzleChezNorf() {
	if (_musicId >= 0) {
		SoundManager *snd = _vm->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	delete _symbOK;
	delete _symbNO;
	delete _symbMaybe;
	delete _plato;
	delete _platoMini;

	for (int i = 0; i < 3; i++) {
		delete _slurpImage[i];
		delete _miamImage[i];
		delete _glouglouImage[i];
		delete _comandeImage[i];
	}

	delete _norfDefault;
	delete _highlightImage;
	delete _debugFont;
}

// ============================================================================
// Resource Loading
// ============================================================================

void PuzzleChezNorf::loadResources() {
	// Food symbol sprites (feedback indicators)
	_symbOK = new RleBlock();
	_symbOK->loadFromFile(Common::Path("bmp/chez_norf/symb_OK"));

	_symbNO = new RleBlock();
	_symbNO->loadFromFile(Common::Path("bmp/chez_norf/symb_NO"));

	_symbMaybe = new RleBlock();
	_symbMaybe->loadFromFile(Common::Path("bmp/chez_norf/symb_MAYBE"));

	// Plate sprites
	if (_level == 1) {
		_plato = new RleBlock();
		_plato->loadFromFile(Common::Path("bmp/chez_norf/plato2"));
	} else {
		_plato = new RleBlock();
		_plato->loadFromFile(Common::Path("bmp/chez_norf/plato"));
	}

	_platoMini = new RleBlock();
	_platoMini->loadFromFile(Common::Path("bmp/chez_norf/plato_mini"));

	// Slurp (dessert) items
	static const char *slurpNames[] = {"slurp_glace", "slurp_pasteque", "slurp_tarte"};
	for (int i = 0; i < 3; i++) {
		_slurpImage[i] = new RleBlock();
		_slurpImage[i]->loadFromFile(Common::Path(Common::String::format("bmp/chez_norf/%s", slurpNames[i])));
	}

	// Miam (main dish) items
	static const char *miamNames[] = {"miam_poisson", "miam_salade", "miam_sandwitch"};
	for (int i = 0; i < 3; i++) {
		_miamImage[i] = new RleBlock();
		_miamImage[i]->loadFromFile(Common::Path(Common::String::format("bmp/chez_norf/%s", miamNames[i])));
	}

	// Glouglou (drink) items
	static const char *glouglouNames[] = {"glouglou_cafe", "glouglou_lait", "glouglou_orange"};
	for (int i = 0; i < 3; i++) {
		_glouglouImage[i] = new RleBlock();
		_glouglouImage[i]->loadFromFile(Common::Path(Common::String::format("bmp/chez_norf/%s", glouglouNames[i])));
	}

	// Command/order background overlays
	static const char *comandeNames[] = {"COMANDE1", "comande2", "comande3"};
	for (int i = 0; i < 3; i++) {
		_comandeImage[i] = new RleBlock();
		_comandeImage[i]->loadFromFile(Common::Path(Common::String::format("bmp/chez_norf/%s", comandeNames[i])));
	}

	// Norf default sprite
	_norfDefault = new RleBlock();
	_norfDefault->loadFromFile(Common::Path("bmp/chez_norf/norf/norfDeBaz"));

	// Highlight
	_highlightImage = new RleBlock();
	_highlightImage->loadFromFile(Common::Path("bmp/chez_norf/highlight"));
}

// ============================================================================
// Initialization
// ============================================================================

void PuzzleChezNorf::init() {
	PuzzleBase::init();

	// Start the restaurant music.
	if (SoundManager *snd = _vm->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("#sounds/music/07-BB02.wav"), true);
		if (_musicId >= 0) {
			snd->playLoop(_musicId);
			snd->setVolume(_musicId, snd->_volumeMusic);
		}
	}

	_level = _vm->getGameState()->_level;
	if (_level < 1)
		_level = 1;
	if (_level > 3)
		_level = 3;

	_maxAttempts = kMaxAttempts[_level];
	_clueAttrCount = kClueAttrCount[_level];

	// Table count: 4 for level 1-2, 6 for level 3 (from Init)
	if (_level == 1 || _level == 2)
		_numTables = 4;
	else
		_numTables = 6;

	loadResources();

	// Set up table positions (from Init: x starting at 205, +85 each)
	for (int i = 0; i < _numTables; i++) {
		_tables[i].pos = Common::Point32(kFirstTablePos.x + i * kTableSpacing, kFirstTablePos.y);

		// Use the loaded plate dimensions for its clickable table area.
		int plateW = _plato ? _plato->getWidth() : 85;
		int plateH = _plato ? _plato->getHeight() : 80;
		_tables[i].hitbox = Common::Rect(
			static_cast<int16>(_tables[i].pos.x), static_cast<int16>(_tables[i].pos.y),
			static_cast<int16>(_tables[i].pos.x + plateW), static_cast<int16>(_tables[i].pos.y + plateH));

		_tables[i].zoombiniIdx = -1;
		_tables[i].foodSlurp = -1;
		_tables[i].foodMiam = -1;
		_tables[i].foodGlouglou = -1;
		_tables[i].served = false;
		_tables[i].completed = false;
	}

	// Assign zoombinis to tables (one per table, skip first zoombini which is "Norf")
	for (int i = 0; i < _numTables && i + 1 < (int)_puzzleZoombinis.size(); i++) {
		_tables[i].zoombiniIdx = i + 1;
	}

	generateFoodVals();
	// Select one of four clue templates within the current level family.
	_templateId = (_level * 10) + (_vm->getRandom()->getRandomNumber(3) + 1);
	setTableAnswersByTemplate();
	generateFoodGrid();

	_state = kStateIdle;
	_freedCount = 0;
	_wrongCount = 0;
	_currentTable = -1;
	_selectedFood = kFoodNone;
	_pendingSlurp = -1;
	_pendingMiam = -1;
	_pendingGlouglou = -1;
	_stateTimer = _vm->getGameTickCount();

	debug(1, "PuzzleChezNorf::init - level=%d numTables=%d maxAttempts=%d clueAttrs=%d",
		  _level, _numTables, _maxAttempts, _clueAttrCount);
}

// ============================================================================
// Food Grid & Answer Generation
// ============================================================================

void PuzzleChezNorf::generateFoodVals() {
	// Generate nine food values, with three distinct values per category.
	// Slurp (0-2): any 3 distinct values in [0,2]
	// Miam  (3-5): any 3 distinct values in [3,5]
	// Glouglou (6-8): any 3 distinct values in [6,8]
	Common::RandomSource *rng = _vm->getRandom();

	// Slurp
	_foodVals[0] = rng->getRandomNumber(2);
	do {
		_foodVals[1] = rng->getRandomNumber(2);
	} while (_foodVals[1] == _foodVals[0]);
	do {
		_foodVals[2] = rng->getRandomNumber(2);
	} while (_foodVals[2] == _foodVals[0] || _foodVals[2] == _foodVals[1]);
	// Miam
	_foodVals[3] = rng->getRandomNumber(2) + 3;
	do {
		_foodVals[4] = rng->getRandomNumber(2) + 3;
	} while (_foodVals[4] == _foodVals[3]);
	do {
		_foodVals[5] = rng->getRandomNumber(2) + 3;
	} while (_foodVals[5] == _foodVals[3] || _foodVals[5] == _foodVals[4]);
	// Glouglou
	_foodVals[6] = rng->getRandomNumber(2) + 6;
	do {
		_foodVals[7] = rng->getRandomNumber(2) + 6;
	} while (_foodVals[7] == _foodVals[6]);
	do {
		_foodVals[8] = rng->getRandomNumber(2) + 6;
	} while (_foodVals[8] == _foodVals[6] || _foodVals[8] == _foodVals[7]);
}

void PuzzleChezNorf::setTableAnswersByTemplate() {
	// Assigns food values from _foodVals[0..8] to _answers[0..5].
	// Naming: s0=_foodVals[0], s1=_foodVals[1], s2=_foodVals[2],
	//         m0=_foodVals[3], m1=_foodVals[4], m2=_foodVals[5],
	//         g0=_foodVals[6], g1=_foodVals[7], g2=_foodVals[8].
	// Value 9 = wildcard (any food in that category accepted).
	// Each table stores its dessert, main-dish, and drink requirements in one answer record.
	const int s0 = _foodVals[0], s1 = _foodVals[1], s2 = _foodVals[2];
	const int m0 = _foodVals[3], m1 = _foodVals[4], m2 = _foodVals[5];
	const int g0 = _foodVals[6], g1 = _foodVals[7], g2 = _foodVals[8];

	switch (_templateId) {
	// Level one uses four tables.
	case 11:
		// Template 11.
		_answers[0] = {s0, 9, 9};
		_answers[1] = {s2, 9, 9};
		_answers[2] = {m0, 9, 9};
		_answers[3] = {m1, 9, 9};
		break;
	case 12:
		// Template 12.
		_answers[0] = {s0, 9, 9};
		_answers[1] = {m2, 9, 9};
		_answers[2] = {s2, 9, 9};
		_answers[3] = {m1, 9, 9};
		break;
	case 13:
		// Template 13.
		_answers[0] = {m0, s0, 9};
		_answers[1] = {s1, 9, 9};
		_answers[2] = {9, 9, 9};
		_answers[3] = {m2, 9, 9};
		break;
	case 14:
		// Template 14.
		_answers[0] = {s2, 9, 9};
		_answers[1] = {s2, s0, 9};
		_answers[2] = {s1, 9, 9};
		_answers[3] = {m0, m1, 9};
		break;
	// Level two uses four tables.
	case 21:
		// Template 21.
		_answers[0] = {s0, g2, 9};
		_answers[1] = {g1, g0, 9};
		_answers[2] = {m2, s1, 9};
		_answers[3] = {m0, m1, 9};
		break;
	case 22:
		// Template 22.
		_answers[0] = {s2, 9, 9};
		_answers[1] = {s1, m1, 9};
		_answers[2] = {m2, g1, 9};
		_answers[3] = {s1, g2, 9};
		break;
	case 23:
		// Template 23.
		_answers[0] = {s0, s2, 9};
		_answers[1] = {g1, m1, 9};
		_answers[2] = {g0, m2, 9};
		_answers[3] = {s0, g1, 9};
		break;
	case 24:
		// Template 24.
		_answers[0] = {m1, 9, 9};
		_answers[1] = {s2, 9, 9};
		_answers[2] = {s1, g2, 9};
		_answers[3] = {s0, g0, m2};
		break;
	// Level three uses six tables.
	case 31:
		// Template 31.
		_answers[0] = {g2, g2, m1};
		_answers[1] = {s1, m0, 9};
		_answers[2] = {g1, s2, 9};
		_answers[3] = {m1, s1, 9};
		_answers[4] = {s1, 9, 9};
		_answers[5] = {m1, 9, 9};
		break;
	case 32:
		// Template 32.
		_answers[0] = {s2, g1, g2};
		_answers[1] = {g0, g0, 9};
		_answers[2] = {m1, m2, 9};
		_answers[3] = {s0, 9, 9};
		_answers[4] = {s2, 9, 9};
		_answers[5] = {m0, g0, 9};
		break;
	case 33:
		// Template 33.
		_answers[0] = {g2, m2, 9};
		_answers[1] = {9, 9, 9};
		_answers[2] = {m1, 9, 9};
		_answers[3] = {s2, s1, 9};
		_answers[4] = {s0, 9, 9};
		_answers[5] = {g1, 9, 9};
		break;
	case 34:
		// Template 34.
		_answers[0] = {s2, 9, 9};
		_answers[1] = {m0, s1, 9};
		_answers[2] = {g0, g2, 9};
		_answers[3] = {g0, s1, 9};
		_answers[4] = {m2, 9, 9};
		_answers[5] = {m1, 9, 9};
		break;
	default:
		// Fallback: every table accepts any food
		for (int i = 0; i < kMaxTables; i++)
			_answers[i] = {9, 9, 9};
		break;
	}
}

void PuzzleChezNorf::generateFoodGrid() {
	// The food board grid shows colored dot markers for the clue layout.
	// From DrawBoard_40F9B0: grid drawn in 3 sections (slurp/miam/glouglou),
	// each with columns at x=51+n*14 and rows at section_base_y+row*15.
	// Values 1/2/3 map to different dot sprites; we use _foodVals to fill.
	// Simplified: each food item in a category is placed across some cells.
	// This board is decorative and does not own interaction state.
	Common::RandomSource *rng = _vm->getRandom();

	for (int section = 0; section < 3; section++) {
		for (int col = 0; col < 6; col++) {
			for (int row = 0; row < 4; row++) {
				_foodGrid[section][col][row] = rng->getRandomNumber(2) + 1;
			}
		}
	}
}

// ============================================================================
// Per-frame state-machine update
// ============================================================================

void PuzzleChezNorf::onUpdate() {
	uint32 now = _vm->getGameTickCount();
	uint32 elapsed = now - _stateTimer;

	switch (_state) {
	case kStateInit:
		_state = kStateIdle;
		_stateTimer = _vm->getGameTickCount();
		break;

	case kStateIdle:
		// Wait for player input.
		break;

	case kStateServing:
		// Serving animation delay
		if (elapsed > kServeDelay) {
			_state = kStateMatching;
			_stateTimer = _vm->getGameTickCount();
		}
		break;

	case kStateFoodServed:
	case kStateMatching:
		// Check match after delay
		if (elapsed > kMatchDelay && _currentTable >= 0) {
			if (checkFoodMatch(_currentTable)) {
				_state = kStateCorrect;
			} else {
				_state = kStateWrong;
				_wrongCount++;
			}
			_stateTimer = _vm->getGameTickCount();
		}
		break;

	case kStateCorrect:
		// Release the diner after a correct match.
		if (elapsed > kMatchDelay) {
			freeZoombini(_currentTable);
			_currentTable = -1;

			if (_freedCount >= kMinFreed) {
				_state = kStateDone;
			} else {
				_state = kStateIdle;
			}
			_stateTimer = _vm->getGameTickCount();
		}
		break;

	case kStateWrong:
		// Clear the table order after an incorrect match.
		if (elapsed > kRejectDelay) {
			if (_currentTable >= 0) {
				_tables[_currentTable].foodSlurp = -1;
				_tables[_currentTable].foodMiam = -1;
				_tables[_currentTable].foodGlouglou = -1;
				_tables[_currentTable].served = false;
			}
			_currentTable = -1;

			// Check if player is fired based on level tolerance
			if (_wrongCount > kWrongTolerance[_level]) {
				debug(1, "PuzzleChezNorf: Player fired! Wrong count %d exceeds tolerance %d",
					  _wrongCount, kWrongTolerance[_level]);
				_state = kStateDone;
				_stateTimer = now;
			} else {
				_state = kStateIdle;
				_stateTimer = now;
			}
		}
		break;

	case kStateDone:
		// Leave the puzzle after its completion delay.
		if (elapsed > kDoneDelay) {
			debug(1, "PuzzleChezNorf: done - freed %d Zoombinis", _freedCount);
			_vm->_returningFromPuzzle = true;
			_vm->_mapTransitionSourcePageId = static_cast<PageId>(_puzzleId);
			_vm->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

// ============================================================================
// Gameplay Logic
// ============================================================================

int PuzzleChezNorf::findTableAtPos(const Common::Point &pos) const {
	for (int i = 0; i < _numTables; i++) {
		if (!_tables[i].completed && _tables[i].hitbox.contains(pos))
			return i;
	}
	return -1;
}

void PuzzleChezNorf::serveFoodToTable(int tableIdx) {
	if (tableIdx < 0 || tableIdx >= _numTables)
		return;

	TableSlot &slot = _tables[tableIdx];
	if (slot.completed)
		return;

	// Requirements based on level:
	// Level 1: Main (Miam) and Drink (Glouglou)
	// Level 2+: All three (Slurp, Miam, Glouglou)
	bool requirementsMet = false;
	if (_level == 1) {
		requirementsMet = (_pendingMiam >= 0 && _pendingGlouglou >= 0);
	} else {
		requirementsMet = (_pendingSlurp >= 0 && _pendingMiam >= 0 && _pendingGlouglou >= 0);
	}

	if (!requirementsMet)
		return;

	slot.foodSlurp = _pendingSlurp;
	slot.foodMiam = _pendingMiam;
	slot.foodGlouglou = _pendingGlouglou;
	slot.served = true;

	// Clear pending selections
	_pendingSlurp = -1;
	_pendingMiam = -1;
	_pendingGlouglou = -1;

	_currentTable = tableIdx;
	_state = kStateServing;
	_stateTimer = _vm->getGameTickCount();
}

bool PuzzleChezNorf::checkFoodMatch(int tableIdx) {
	// From IsFoodCorrect_453590: compare 3 food values against stored answers.
	// Value 9 in an answer slot is a wildcard (any served food accepted).
	// foodSlurp/foodMiam/foodGlouglou store the absolute food IDs (0-8).
	const TableSlot &slot = _tables[tableIdx];
	const FoodAnswer &answer = _answers[tableIdx];

	auto foodMatches = [](int served, int required) -> bool {
		return (required == 9) || (served == required);
	};

	return foodMatches(slot.foodSlurp, answer.slurp) && foodMatches(slot.foodMiam, answer.miam) && foodMatches(slot.foodGlouglou, answer.glouglou);
}

void PuzzleChezNorf::freeZoombini(int tableIdx) {
	if (tableIdx < 0 || tableIdx >= _numTables)
		return;

	TableSlot &slot = _tables[tableIdx];
	if (slot.zoombiniIdx >= 0 && slot.zoombiniIdx < (int)_puzzleZoombinis.size()) {
		ZoombiniState *z = _puzzleZoombinis[slot.zoombiniIdx];
		z->_puzzleStatus = 1;
		debug(2, "ChezNorf: freed zoombini %d from table %d", slot.zoombiniIdx, tableIdx);
	}

	slot.completed = true;
	_freedCount++;
}

int PuzzleChezNorf::countFreeZoombinis() const {
	int count = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_puzzleStatus == 0)
			count++;
	}
	return count;
}

// ============================================================================
// Click Handling
// ============================================================================

EventHandleResult PuzzleChezNorf::onLButtonDown(const Common::Point &pos) {
	if (_state != kStateIdle)
		return EventHandleResult::kPassthrough;

	// Check food board click first (left side of screen).
	// Board sections: slurp y=47..92, miam y=104..149, glouglou y=164..209, x=51..135
	int foodIdx = findFoodAtPos(pos);
	if (foodIdx >= 0) {
		if (foodIdx < 3) {
			// Slurp section: select the food value _foodVals[foodIdx]
			_pendingSlurp = _foodVals[foodIdx];
			debug(2, "ChezNorf: selected slurp food %d (val=%d)", foodIdx, _pendingSlurp);
		} else if (foodIdx < 6) {
			// Miam section
			_pendingMiam = _foodVals[foodIdx];
			debug(2, "ChezNorf: selected miam food %d (val=%d)", foodIdx - 3, _pendingMiam);
		} else {
			// Glouglou section
			_pendingGlouglou = _foodVals[foodIdx];
			debug(2, "ChezNorf: selected glouglou food %d (val=%d)", foodIdx - 6, _pendingGlouglou);
		}
		return EventHandleResult::kConsumed;
	}

	// Serve a clicked table only after all three food categories are selected.
	int tableIdx = findTableAtPos(pos);
	if (tableIdx >= 0) {
		if (_pendingSlurp >= 0 && _pendingMiam >= 0 && _pendingGlouglou >= 0) {
			debug(2, "ChezNorf: serving table %d with s=%d m=%d g=%d",
				  tableIdx, _pendingSlurp, _pendingMiam, _pendingGlouglou);
			serveFoodToTable(tableIdx);
		} else {
			debug(2, "ChezNorf: clicked table %d but not all food selected (s=%d m=%d g=%d)",
				  tableIdx, _pendingSlurp, _pendingMiam, _pendingGlouglou);
		}
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

int PuzzleChezNorf::findFoodAtPos(const Common::Point &pos) const {
	// Food board is on the left portion of the screen.
	// From DrawBoard_40F9B0: columns at x=51..135 (6 cols * 14px), rows 15px apart.
	// Section 0 (slurp):    y = 47..107  (3 rows at 47, 62, 77 + ~15px height)
	// Section 1 (miam):     y = 104..164
	// Section 2 (glouglou): y = 164..224
	// Board x extent: x=51 to x=51+6*14=135.
	if (pos.x < 51 || pos.x > 135)
		return -1;

	// Determine which section (category) was clicked
	int section = -1;
	if (pos.y >= 47 && pos.y < 100)
		section = 0; // Slurp
	else if (pos.y >= 104 && pos.y < 157)
		section = 1; // Miam
	else if (pos.y >= 164 && pos.y < 217)
		section = 2; // Glouglou
	else
		return -1;

	// Within the section, divide x range into 3 zones for the 3 food items.
	// Board width: 135-51=84, so each zone is 28px.
	int col = (pos.x - 51) / 28;
	if (col > 2)
		col = 2;

	// Return index into _foodVals: section*3 + col
	return section * 3 + col;
}

// ============================================================================
// Drawing
// ============================================================================

void PuzzleChezNorf::onRenderScene(ManagedSurface32 *screen) {
	// Background (loaded by base class)
	if (_background) {
		_background->drawToSurface(screen, Common::Point32(0, 0));
	}

	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	// Select the order-panel background for the current level.
	RleBlock *bgOverlay = nullptr;
	switch (_level) {
	case 1:
		bgOverlay = _comandeImage[2];
		break; // comande3
	case 2:
		bgOverlay = _comandeImage[0];
		break; // comande1
	case 3:
		bgOverlay = _comandeImage[1];
		break; // comande2
	default:
		break;
	}
	if (bgOverlay && bgOverlay->isValid()) {
		bgOverlay->drawToScreen(screen, Common::Point32(0, 0), lut);
	}

	drawFoodBoard(screen);
	drawTables(screen);
	drawPlates(screen);
	drawNorf(screen);
}

void PuzzleChezNorf::onRenderForeground(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();
	// Draw feedback overlay for current state
	if (_currentTable >= 0) {
		const Common::Point32 feedbackPos(
			_tables[_currentTable].pos.x, _tables[_currentTable].pos.y - 40);

		if (_state == kStateCorrect && _symbOK && _symbOK->isValid()) {
			_symbOK->drawToScreen(screen, feedbackPos, lut);
		} else if (_state == kStateWrong && _symbNO && _symbNO->isValid()) {
			_symbNO->drawToScreen(screen, feedbackPos, lut);
		}
	}
	if (_vm->showChezNorfDebugOverlay())
		drawDebugOverlay(screen);
}

const char *PuzzleChezNorf::getDebugFoodName(int foodId) {
	static constexpr const char *names[] = {"IC", "Melon", "Pie", "Fish", "Salad", "Swich", "tea", "Milk", "Ornge", "Rien"};
	return 0 <= foodId && foodId < static_cast<int>(ARRAYSIZE(names)) ? names[foodId] : "?";
}

void PuzzleChezNorf::drawDebugOverlay(ManagedSurface32 *screen) {
	if (!_debugFont) {
		_debugFont = new BitmapFont();
		if (!_debugFont->load(Common::Path("bmp/typo"), 0, 255, 0)) {
			delete _debugFont;
			_debugFont = nullptr;
			return;
		}
	}

	const AlphaBlendLUT &lut = _vm->getAlphaLUT();
	const Common::String header = Common::String::format("%d - %d %d %d - %d %d %d - %d %d %d", _templateId, _foodVals[0], _foodVals[1],
														 _foodVals[2], _foodVals[3], _foodVals[4], _foodVals[5], _foodVals[6], _foodVals[7], _foodVals[8]);
	_debugFont->drawString(screen, Common::Point32(100, 0), header, lut);

	for (int tableIndex = 0; tableIndex < _numTables; tableIndex++) {
		const FoodAnswer &answer = _answers[tableIndex];
		const int x = 205 + tableIndex * kTableSpacing;
		_debugFont->drawString(screen, Common::Point32(x, 50), getDebugFoodName(answer.slurp), lut);
		_debugFont->drawString(screen, Common::Point32(x, 80), getDebugFoodName(answer.miam), lut);
		_debugFont->drawString(screen, Common::Point32(x, 110), getDebugFoodName(answer.glouglou), lut);
		const int assignmentCount = static_cast<int>(answer.slurp != 9) + static_cast<int>(answer.miam != 9) + static_cast<int>(answer.glouglou != 9);
		_debugFont->drawString(screen, Common::Point32(x, 140), Common::String::format("%d", assignmentCount), lut);
	}
}

void PuzzleChezNorf::drawFoodBoard(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	// Draw 3 sections of food grid dots from DrawBoard_40F9B0
	// Section 1: y starts at 47, step 15; Section 2: 104; Section 3: 164
	static const int kSectionStartY[] = {47, 104, 164};

	// Symbol values select incorrect, correct, or partial-match feedback.
	RleBlock *symbByType[] = {nullptr, _symbNO, _symbOK, _symbMaybe};

	for (int section = 0; section < 3; section++) {
		int baseX = 51;
		for (int col = 0; col < 6; col++) {
			int gridY = kSectionStartY[section];
			for (int row = 0; row < 3; row++) {
				int val = _foodGrid[section][col][row];
				if (val >= 1 && val <= 3 && symbByType[val] && symbByType[val]->isValid()) {
					symbByType[val]->drawToScreen(screen, Common::Point32(baseX, gridY), lut);
				}
				gridY += 15;
			}
			baseX += 14;
		}
	}
}

void PuzzleChezNorf::drawTables(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	// Draw highlight on non-completed tables
	for (int i = 0; i < _numTables; i++) {
		if (!_tables[i].completed && _highlightImage && _highlightImage->isValid()) {
			// Highlight drawn at norf slot position (y=205 area from Init)
			_highlightImage->drawToScreen(screen, Common::Point32(_tables[i].pos.x, 205), lut);
		}
	}
}

void PuzzleChezNorf::drawPlates(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	for (int i = 0; i < _numTables; i++) {
		if (_tables[i].completed)
			continue;

		// Draw plate at table position
		if (_plato && _plato->isValid()) {
			_plato->drawToScreen(screen, _tables[i].pos, lut);
		}

		// Draw food items on served plates
		if (_tables[i].served) {
			const Common::Point32 foodPos(_tables[i].pos.x + 5, _tables[i].pos.y + 5);

			// Draw slurp
			if (_tables[i].foodSlurp >= 0 && _tables[i].foodSlurp < 3) {
				RleBlock *image = _slurpImage[_tables[i].foodSlurp];
				if (image && image->isValid())
					image->drawToScreen(screen, foodPos, lut);
			}

			// Draw miam
			if (_tables[i].foodMiam >= 0 && _tables[i].foodMiam < 3) {
				RleBlock *image = _miamImage[_tables[i].foodMiam];
				if (image && image->isValid())
					image->drawToScreen(screen, Common::Point32(foodPos.x + 20, foodPos.y), lut);
			}

			// Draw glouglou
			if (_tables[i].foodGlouglou >= 0 && _tables[i].foodGlouglou < 3) {
				RleBlock *image = _glouglouImage[_tables[i].foodGlouglou];
				if (image && image->isValid())
					image->drawToScreen(screen, Common::Point32(foodPos.x + 40, foodPos.y), lut);
			}
		}
	}
}

void PuzzleChezNorf::drawNorf(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	// Draw Norf default sprite at center area
	if (_norfDefault && _norfDefault->isValid()) {
		_norfDefault->drawToScreen(screen, Common::Point32(100, 350), lut);
	}
}

void PuzzleChezNorf::onRenderActors(ManagedSurface32 *screen) {
	if (!_zoombiniAnimation)
		return;

	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	// Draw zoombinis at their table positions
	for (int i = 0; i < _numTables; i++) {
		if (_tables[i].completed)
			continue;

		int zIdx = _tables[i].zoombiniIdx;
		if (zIdx < 0 || zIdx >= (int)_puzzleZoombinis.size())
			continue;

		const ZoombiniState *z = _puzzleZoombinis[zIdx];
		const Common::Point32 pos(_tables[i].pos.x, _tables[i].pos.y - 60);

		_zoombiniAnimation->drawZoombini(screen, z->_traits, pos, 0, 0, lut);
	}
}

} // End of namespace Zoombini2
