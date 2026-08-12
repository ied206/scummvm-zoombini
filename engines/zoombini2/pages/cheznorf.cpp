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

#include "zoombini2/pages/cheznorf.h"
#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"
#include "zoombini2/sound.h"

namespace Zoombini2 {

// ============================================================================
// ChezNorfPuzzle — Restaurant food-matching puzzle.
//
// Original: ChezNorf__Init_412C30. Object size 0x33C (828 bytes).
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
// Background: comande3 for diff1, comande1 for diff2, comande2 for diff3
// (from DrawBoard: diff1→this+84, diff2→this+76, diff3→this+80)
// ============================================================================

// Delay constants
static const uint32 kServeDelay = 1500;
static const uint32 kMatchDelay = 1000;
static const uint32 kRejectDelay = 2000;
static const uint32 kDoneDelay = 3000;

// Minimum freed zoombinis for success (from CheckFreeZoombinis: >= 4)
static const int kMinFreed = 4;

// Tolerance thresholds per difficulty (from CheckFoodMatch)
// Diff 1: 2 wrong guesses before reject, Diff 2: 1, Diff 3: 0
static const int kWrongTolerance[] = { 0, 2, 1, 0 };

// Max attempts (passes) per difficulty, indexed by diff (0 unused).
// Original: dword_489184 — ChezNorf__Init_412C30 stores this+568.
static const int kMaxAttempts[] = { 0, 5, 4, 3 };

// Clue attribute count shown per difficulty, indexed by diff (0 unused).
// Original: ChezNorf__SelectTableLayout — diff 1→8, diff 2→7, diff 3→8.
static const int kClueAttrCount[] = { 0, 8, 7, 8 };

// ============================================================================
// Construction / Destruction
// ============================================================================

ChezNorfPuzzle::ChezNorfPuzzle(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageChezNorf),
	  _state(kStateInit), _numTables(4), _difficulty(1),
	  _freedCount(0), _currentTable(-1), _selectedFood(kFoodNone),
	  _wrongCount(0), _maxAttempts(0), _clueAttrCount(0), _norfState(0),
	  _templateId(11), _pendingSlurp(-1), _pendingMiam(-1), _pendingGlouglou(-1),
	  _symbOK(nullptr), _symbNO(nullptr), _symbMaybe(nullptr),
	  _plato(nullptr), _platoMini(nullptr),
	  _norfDefault(nullptr), _highlightGfx(nullptr),
	  _musicId(-1) {

	for (int i = 0; i < 3; i++) {
		_slurpGfx[i] = nullptr;
		_miamGfx[i] = nullptr;
		_glouglouGfx[i] = nullptr;
		_comandeGfx[i] = nullptr;
	}

	memset(_foodGrid, 0, sizeof(_foodGrid));
	memset(_foodVals, 0, sizeof(_foodVals));

	for (int i = 0; i < kMaxTables; i++) {
		_tables[i].x = 0;
		_tables[i].y = 0;
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

ChezNorfPuzzle::~ChezNorfPuzzle() {
	if (_musicId >= 0) {
		SoundManager *snd = _engine->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	delete _symbOK;
	delete _symbNO;
	delete _symbMaybe;
	delete _plato;
	delete _platoMini;

	for (int i = 0; i < 3; i++) {
		delete _slurpGfx[i];
		delete _miamGfx[i];
		delete _glouglouGfx[i];
		delete _comandeGfx[i];
	}

	delete _norfDefault;
	delete _highlightGfx;
}

// ============================================================================
// Resource Loading
// ============================================================================

void ChezNorfPuzzle::loadResources() {
	const byte (*lut)[256] = _engine->getAlphaLUT();
	(void)lut;

	// Food symbol sprites (feedback indicators)
	_symbOK = new RleBlock();
	_symbOK->loadFromFile(Common::Path("bmp/chez_norf/symb_OK"));

	_symbNO = new RleBlock();
	_symbNO->loadFromFile(Common::Path("bmp/chez_norf/symb_NO"));

	_symbMaybe = new RleBlock();
	_symbMaybe->loadFromFile(Common::Path("bmp/chez_norf/symb_MAYBE"));

	// Plate sprites
	if (_difficulty == 1) {
		_plato = new RleBlock();
		_plato->loadFromFile(Common::Path("bmp/chez_norf/plato2"));
	} else {
		_plato = new RleBlock();
		_plato->loadFromFile(Common::Path("bmp/chez_norf/plato"));
	}

	_platoMini = new RleBlock();
	_platoMini->loadFromFile(Common::Path("bmp/chez_norf/plato_mini"));

	// Slurp (dessert) items
	static const char *slurpNames[] = { "slurp_glace", "slurp_pasteque", "slurp_tarte" };
	for (int i = 0; i < 3; i++) {
		_slurpGfx[i] = new RleBlock();
		_slurpGfx[i]->loadFromFile(Common::Path(Common::String::format("bmp/chez_norf/%s", slurpNames[i])));
	}

	// Miam (main dish) items
	static const char *miamNames[] = { "miam_poisson", "miam_salade", "miam_sandwitch" };
	for (int i = 0; i < 3; i++) {
		_miamGfx[i] = new RleBlock();
		_miamGfx[i]->loadFromFile(Common::Path(Common::String::format("bmp/chez_norf/%s", miamNames[i])));
	}

	// Glouglou (drink) items
	static const char *glouglouNames[] = { "glouglou_cafe", "glouglou_lait", "glouglou_orange" };
	for (int i = 0; i < 3; i++) {
		_glouglouGfx[i] = new RleBlock();
		_glouglouGfx[i]->loadFromFile(Common::Path(Common::String::format("bmp/chez_norf/%s", glouglouNames[i])));
	}

	// Command/order background overlays
	static const char *comandeNames[] = { "COMANDE1", "comande2", "comande3" };
	for (int i = 0; i < 3; i++) {
		_comandeGfx[i] = new RleBlock();
		_comandeGfx[i]->loadFromFile(Common::Path(Common::String::format("bmp/chez_norf/%s", comandeNames[i])));
	}

	// Norf default sprite
	_norfDefault = new RleBlock();
	_norfDefault->loadFromFile(Common::Path("bmp/chez_norf/norf/norfDeBaz"));

	// Highlight
	_highlightGfx = new RleBlock();
	_highlightGfx->loadFromFile(Common::Path("bmp/chez_norf/highlight"));
}

// ============================================================================
// Initialization
// ============================================================================

void ChezNorfPuzzle::init() {
	PuzzlePage::init();

	// BGM: 07-BB02.wav (IDA: ChezNorf__Init_412C30)
	if (SoundManager *snd = _engine->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("sounds/music/07-BB02.wav"), true);
		if (_musicId >= 0) { snd->playLoop(_musicId); snd->setVolume(_musicId, snd->_volumeMusic); }
	}

	_difficulty = _engine->getGameState()->_gameMode;
	if (_difficulty < 1) _difficulty = 1;
	if (_difficulty > 3) _difficulty = 3;

	_maxAttempts = kMaxAttempts[_difficulty];
	_clueAttrCount = kClueAttrCount[_difficulty];

	// Table count: 4 for difficulty 1-2, 6 for difficulty 3 (from Init)
	if (_difficulty == 1 || _difficulty == 2)
		_numTables = 4;
	else
		_numTables = 6;

	loadResources();

	// Set up table positions (from Init: x starting at 205, +85 each)
	for (int i = 0; i < _numTables; i++) {
		_tables[i].x = kTableStartX + i * kTableSpacing;
		_tables[i].y = kTablePlateY;

		// Hitbox for plate area (approximate from IDA; plate dimensions used)
		int plateW = _plato ? _plato->getWidth() : 85;
		int plateH = _plato ? _plato->getHeight() : 80;
		_tables[i].hitbox = Common::Rect(
			_tables[i].x, _tables[i].y,
			_tables[i].x + plateW, _tables[i].y + plateH);

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
	// Pick template: difficulty 1→11-14, difficulty 2→21-24, difficulty 3→31-34
	_templateId = (_difficulty * 10) + (_engine->getRandom()->getRandomNumber(3) + 1);
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
	_stateTimer = _engine->getGameTickCount();

	debug(1, "ChezNorfPuzzle::init — difficulty=%d numTables=%d maxAttempts=%d clueAttrs=%d",
		_difficulty, _numTables, _maxAttempts, _clueAttrCount);
}

// ============================================================================
// Food Grid & Answer Generation
// ============================================================================

void ChezNorfPuzzle::generateFoodVals() {
	// From GenerateClueLayout_453C80:
	// Generate 9 food values — 3 distinct per category.
	// Slurp (0-2): any 3 distinct values in [0,2]
	// Miam  (3-5): any 3 distinct values in [3,5]
	// Glouglou (6-8): any 3 distinct values in [6,8]
	Common::RandomSource *rng = _engine->getRandom();

	// Slurp
	_foodVals[0] = rng->getRandomNumber(2);
	do { _foodVals[1] = rng->getRandomNumber(2); } while (_foodVals[1] == _foodVals[0]);
	do { _foodVals[2] = rng->getRandomNumber(2); } while (_foodVals[2] == _foodVals[0] || _foodVals[2] == _foodVals[1]);
	// Miam
	_foodVals[3] = rng->getRandomNumber(2) + 3;
	do { _foodVals[4] = rng->getRandomNumber(2) + 3; } while (_foodVals[4] == _foodVals[3]);
	do { _foodVals[5] = rng->getRandomNumber(2) + 3; } while (_foodVals[5] == _foodVals[3] || _foodVals[5] == _foodVals[4]);
	// Glouglou
	_foodVals[6] = rng->getRandomNumber(2) + 6;
	do { _foodVals[7] = rng->getRandomNumber(2) + 6; } while (_foodVals[7] == _foodVals[6]);
	do { _foodVals[8] = rng->getRandomNumber(2) + 6; } while (_foodVals[8] == _foodVals[6] || _foodVals[8] == _foodVals[7]);
}

void ChezNorfPuzzle::setTableAnswersByTemplate() {
	// From GenerateClueLayout_453C80 switch(_templateId):
	// Assigns food values from _foodVals[0..8] to _answers[0..5].
	// Naming: s0=_foodVals[0], s1=_foodVals[1], s2=_foodVals[2],
	//         m0=_foodVals[3], m1=_foodVals[4], m2=_foodVals[5],
	//         g0=_foodVals[6], g1=_foodVals[7], g2=_foodVals[8].
	// Value 9 = wildcard (any food in that category accepted).
	// Original offsets: table t stores at this[20+6t..22+6t];
	// here we store in _answers[t].{slurp, miam, glouglou}.
	const int s0 = _foodVals[0], s1 = _foodVals[1], s2 = _foodVals[2];
	const int m0 = _foodVals[3], m1 = _foodVals[4], m2 = _foodVals[5];
	const int g0 = _foodVals[6], g1 = _foodVals[7], g2 = _foodVals[8];

	// Macro to set a table's 3-food answer tuple.
	// v18 in the IDA code is g2 (_foodVals[8]).
	switch (_templateId) {
	// --- Difficulty 1 (4 tables) ---
	case 11:
		// t0={s0,9,9}, t1={s2,9,9}, via LABEL_15: t2={m0,m1,9}, via LABEL_16: t3={m1,9,9}
		// Original: *(this+20)=s0, *(this+21)=9, *(this+22)=9
		//           *(this+26)=s2, *(this+27)=9, *(this+28)=9
		//           LABEL_15: *(this+32)=m0, *(this+33)=9, *(this+34)=9
		//           LABEL_16: *(this+38)=m1, *(this+39)=9, *(this+40)=9
		_answers[0] = { s0,  9,  9 };
		_answers[1] = { s2,  9,  9 };
		_answers[2] = { m0,  9,  9 };
		_answers[3] = { m1,  9,  9 };
		break;
	case 12:
		// *(this+20)=s0, *(this+21)=9, *(this+22)=9
		// *(this+26)=m2, *(this+27)=9, *(this+28)=9
		// LABEL_15(v19=s2): *(this+32)=s2, *(this+33)=9, *(this+34)=9
		// LABEL_16(v20=m1): *(this+38)=m1, *(this+39)=9, *(this+40)=9
		_answers[0] = { s0,  9,  9 };
		_answers[1] = { m2,  9,  9 };
		_answers[2] = { s2,  9,  9 };
		_answers[3] = { m1,  9,  9 };
		break;
	case 13:
		// *(this+21)=s0, *(this+20)=m0
		// *(this+26)=s1
		// v20=m2: *(this+22)=9, *(this+27)=9, *(this+28)=9, *(this+32)=9, *(this+33)=9, *(this+34)=9
		// LABEL_16(v20=m2): *(this+38)=m2, *(this+39)=9, *(this+40)=9
		_answers[0] = { m0, s0,  9 };
		_answers[1] = { s1,  9,  9 };
		_answers[2] = {  9,  9,  9 };
		_answers[3] = { m2,  9,  9 };
		break;
	case 14:
		// *(this+20)=s2, *(this+21)=9, *(this+22)=9
		// *(this+26)=s2, *(this+27)=s0, *(this+28)=9
		// *(this+32)=s1, *(this+33)=9
		// LABEL_21: *(this+38)=m0, *(this+39)=m1, *(this+34)=9, *(this+40)=9
		_answers[0] = { s2,  9,  9 };
		_answers[1] = { s2, s0,  9 };
		_answers[2] = { s1,  9,  9 };
		_answers[3] = { m0, m1,  9 };
		break;
	// --- Difficulty 2 (4 tables) ---
	case 21:
		// *(this+21)=g2, *(this+20)=s0, *(this+26)=g1
		// *(this+27)=g0, *(this+32)=m2, *(this+33)=s1
		// *(this+22)=9, *(this+28)=9
		// LABEL_21: *(this+38)=m0, *(this+39)=m1, *(this+34)=9, *(this+40)=9
		_answers[0] = { s0, g2,  9 };
		_answers[1] = { g1, g0,  9 };
		_answers[2] = { m2, s1,  9 };
		_answers[3] = { m0, m1,  9 };
		break;
	case 22:
		// *(this+27)=m1, *(this+32)=m2
		// *(this+20)=s2, *(this+26)=s1, *(this+39)=g2, *(this+33)=g1
		// *(this+21)=9, *(this+22)=9, *(this+28)=9, *(this+34)=9
		// *(this+38)=s1, *(this+40)=9
		_answers[0] = { s2,  9,  9 };
		_answers[1] = { s1, m1,  9 };
		_answers[2] = { m2, g1,  9 };
		_answers[3] = { s1, g2,  9 };
		break;
	case 23:
		// *(this+21)=s2, *(this+27)=m1, *(this+32)=g0, *(this+33)=m2
		// *(this+20)=s0, *(this+26)=g1, *(this+38)=s0
		// *(this+22)=9, *(this+28)=9, *(this+34)=9, *(this+39)=g1, *(this+40)=9
		_answers[0] = { s0, s2,  9 };
		_answers[1] = { g1, m1,  9 };
		_answers[2] = { g0, m2,  9 };
		_answers[3] = { s0, g1,  9 };
		break;
	case 24:
		// *(this+20)=m1, *(this+21)=9, *(this+22)=9
		// *(this+26)=s2, *(this+27)=9, *(this+28)=9
		// *(this+33)=g2, *(this+34)=9, *(this+40)=m2
		// *(this+32)=s1, *(this+38)=s0, *(this+39)=g0
		_answers[0] = { m1,  9,  9 };
		_answers[1] = { s2,  9,  9 };
		_answers[2] = { s1, g2,  9 };
		_answers[3] = { s0, g0, m2 };
		break;
	// --- Difficulty 3 (6 tables) ---
	case 31:
		// *(this+20)=g2, *(this+21)=g2, *(this+22)=m1
		// *(this+26)=s1, *(this+27)=m0, *(this+28)=9
		// *(this+32)=g1, *(this+33)=s2, *(this+34)=9
		// *(this+38)=m1, *(this+39)=s1, *(this+40)=9
		// *(this+44)=s1, *(this+45)=9, *(this+46)=9
		// LABEL_29(v49=m1): *(this+50)=m1, *(this+51)=9, *(this+52)=9
		_answers[0] = { g2, g2, m1 };
		_answers[1] = { s1, m0,  9 };
		_answers[2] = { g1, s2,  9 };
		_answers[3] = { m1, s1,  9 };
		_answers[4] = { s1,  9,  9 };
		_answers[5] = { m1,  9,  9 };
		break;
	case 32:
		// *(this+20)=s2, *(this+21)=g1, *(this+22)=g2
		// *(this+26)=g0, *(this+27)=g0, *(this+28)=9
		// *(this+32)=m1, *(this+33)=m2, *(this+34)=9
		// *(this+38)=s0, *(this+39)=9, *(this+40)=9
		// *(this+44)=s2, *(this+45)=9, *(this+46)=9
		// *(this+50)=m0, *(this+51)=g0, *(this+52)=9
		_answers[0] = { s2, g1, g2 };
		_answers[1] = { g0, g0,  9 };
		_answers[2] = { m1, m2,  9 };
		_answers[3] = { s0,  9,  9 };
		_answers[4] = { s2,  9,  9 };
		_answers[5] = { m0, g0,  9 };
		break;
	case 33:
		// *(this+20)=g2, *(this+21)=m2, *(this+22)=9
		// *(this+26)=9, *(this+27)=9, *(this+28)=9
		// *(this+32)=m1, *(this+33)=9, *(this+34)=9
		// *(this+38)=s2, *(this+39)=s1, *(this+40)=9
		// *(this+44)=s0, *(this+45)=9, *(this+46)=9
		// LABEL_29(v49=g1): *(this+50)=g1, *(this+51)=9, *(this+52)=9
		_answers[0] = { g2, m2,  9 };
		_answers[1] = {  9,  9,  9 };
		_answers[2] = { m1,  9,  9 };
		_answers[3] = { s2, s1,  9 };
		_answers[4] = { s0,  9,  9 };
		_answers[5] = { g1,  9,  9 };
		break;
	case 34:
		// *(this+20)=s2, *(this+21)=9, *(this+22)=9
		// *(this+26)=m0, *(this+27)=s1, *(this+28)=9
		// *(this+32)=g0, *(this+33)=g2, *(this+34)=9
		// *(this+38)=g0, *(this+39)=s1, *(this+40)=9
		// *(this+44)=m2, *(this+45)=9, *(this+46)=9
		// LABEL_29(v49=m1): *(this+50)=m1, *(this+51)=9, *(this+52)=9
		_answers[0] = { s2,  9,  9 };
		_answers[1] = { m0, s1,  9 };
		_answers[2] = { g0, g2,  9 };
		_answers[3] = { g0, s1,  9 };
		_answers[4] = { m2,  9,  9 };
		_answers[5] = { m1,  9,  9 };
		break;
	default:
		// Fallback: every table accepts any food
		for (int i = 0; i < kMaxTables; i++)
			_answers[i] = { 9, 9, 9 };
		break;
	}
}

void ChezNorfPuzzle::generateFoodGrid() {
	// The food board grid shows colored dot markers for the clue layout.
	// From DrawBoard_40F9B0: grid drawn in 3 sections (slurp/miam/glouglou),
	// each with columns at x=51+n*14 and rows at section_base_y+row*15.
	// Values 1/2/3 map to different dot sprites; we use _foodVals to fill.
	// Simplified: each food item in a category is placed across some cells.
	// This visual board is decorative/informational — leave as before.
	Common::RandomSource *rng = _engine->getRandom();

	for (int section = 0; section < 3; section++) {
		for (int col = 0; col < 6; col++) {
			for (int row = 0; row < 4; row++) {
				_foodGrid[section][col][row] = rng->getRandomNumber(2) + 1;
			}
		}
	}
}

// ============================================================================
// Update (per-frame tick — non-blocking model)
// ============================================================================

void ChezNorfPuzzle::update() {
	uint32 now = _engine->getGameTickCount();
	uint32 elapsed = now - _stateTimer;

	switch (_state) {
	case kStateInit:
		_state = kStateIdle;
		_stateTimer = _engine->getGameTickCount();
		break;

	case kStateIdle:
		// Waiting for player input — nothing to do
		break;

	case kStateServing:
		// Serving animation delay
		if (elapsed > kServeDelay) {
			_state = kStateMatching;
			_stateTimer = _engine->getGameTickCount();
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
			_stateTimer = _engine->getGameTickCount();
		}
		break;

	case kStateCorrect:
		// Correct match — free zoombini
		if (elapsed > kMatchDelay) {
			freeZoombini(_currentTable);
			_currentTable = -1;

			if (_freedCount >= kMinFreed) {
				_state = kStateDone;
			} else {
				_state = kStateIdle;
			}
			_stateTimer = _engine->getGameTickCount();
		}
		break;

	case kStateWrong:
		// Wrong match — reset table food
		if (elapsed > kRejectDelay) {
			if (_currentTable >= 0) {
				_tables[_currentTable].foodSlurp = -1;
				_tables[_currentTable].foodMiam = -1;
				_tables[_currentTable].foodGlouglou = -1;
				_tables[_currentTable].served = false;
			}
			_currentTable = -1;
			
			// Check if player is fired based on difficulty tolerance
			if (_wrongCount > kWrongTolerance[_difficulty]) {
				debug(1, "ChezNorfPuzzle: Player fired! Wrong count %d exceeds tolerance %d", 
					   _wrongCount, kWrongTolerance[_difficulty]);
				_state = kStateDone;
				_stateTimer = now;
			} else {
				_state = kStateIdle;
				_stateTimer = now;
			}
		}
		break;

	case kStateDone:
		// Puzzle complete — transition out
		if (elapsed > kDoneDelay) {
			debug(1, "ChezNorfPuzzle: done — freed %d zoombinis", _freedCount);
			_engine->_returningFromPuzzle = true;
			_engine->_maptransSourceWorld = _puzzleId;
			_engine->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

// ============================================================================
// Gameplay Logic
// ============================================================================

int ChezNorfPuzzle::findTableAtPos(const Common::Point &pos) const {
	for (int i = 0; i < _numTables; i++) {
		if (!_tables[i].completed && _tables[i].hitbox.contains(pos))
			return i;
	}
	return -1;
}

void ChezNorfPuzzle::serveFoodToTable(int tableIdx) {
	if (tableIdx < 0 || tableIdx >= _numTables)
		return;

	TableSlot &slot = _tables[tableIdx];
	if (slot.completed)
		return;

	// Requirements based on difficulty:
	// Diff 1: Main (Miam) and Drink (Glouglou)
	// Diff 2+: All three (Slurp, Miam, Glouglou)
	bool requirementsMet = false;
	if (_difficulty == 1) {
		requirementsMet = (_pendingMiam >= 0 && _pendingGlouglou >= 0);
	} else {
		requirementsMet = (_pendingSlurp >= 0 && _pendingMiam >= 0 && _pendingGlouglou >= 0);
	}

	if (!requirementsMet)
		return;

	slot.foodSlurp    = _pendingSlurp;
	slot.foodMiam     = _pendingMiam;
	slot.foodGlouglou = _pendingGlouglou;
	slot.served = true;

	// Clear pending selections
	_pendingSlurp    = -1;
	_pendingMiam     = -1;
	_pendingGlouglou = -1;

	_currentTable = tableIdx;
	_state = kStateServing;
	_stateTimer = _engine->getGameTickCount();
}

bool ChezNorfPuzzle::checkFoodMatch(int tableIdx) {
	// From IsFoodCorrect_453590: compare 3 food values against stored answers.
	// Value 9 in an answer slot is a wildcard (any served food accepted).
	// foodSlurp/foodMiam/foodGlouglou store the absolute food IDs (0-8).
	const TableSlot &slot = _tables[tableIdx];
	const FoodAnswer &answer = _answers[tableIdx];

	auto foodMatches = [](int served, int required) -> bool {
		return (required == 9) || (served == required);
	};

	return foodMatches(slot.foodSlurp,    answer.slurp)
		&& foodMatches(slot.foodMiam,     answer.miam)
		&& foodMatches(slot.foodGlouglou, answer.glouglou);
}

void ChezNorfPuzzle::freeZoombini(int tableIdx) {
	if (tableIdx < 0 || tableIdx >= _numTables)
		return;

	TableSlot &slot = _tables[tableIdx];
	if (slot.zoombiniIdx >= 0 && slot.zoombiniIdx < (int)_puzzleZoombinis.size()) {
		Zoombini *z = _puzzleZoombinis[slot.zoombiniIdx];
		z->_freeStatus = 1;  // Freed
		debug(2, "ChezNorf: freed zoombini %d from table %d", slot.zoombiniIdx, tableIdx);
	}

	slot.completed = true;
	_freedCount++;
}

int ChezNorfPuzzle::countFreeZoombinis() const {
	int count = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_freeStatus == 0)
			count++;
	}
	return count;
}

// ============================================================================
// Click Handling
// ============================================================================

void ChezNorfPuzzle::handleClick(const Common::Point &pos) {
	if (_state != kStateIdle)
		return;

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
		return;
	}

	// Check if clicking on a table plate — only serve if all 3 food categories selected
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
		return;
	}
}

int ChezNorfPuzzle::findFoodAtPos(const Common::Point &pos) const {
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
		section = 0;  // Slurp
	else if (pos.y >= 104 && pos.y < 157)
		section = 1;  // Miam
	else if (pos.y >= 164 && pos.y < 217)
		section = 2;  // Glouglou
	else
		return -1;

	// Within the section, divide x range into 3 zones for the 3 food items.
	// Board width: 135-51=84, so each zone is 28px.
	int col = (pos.x - 51) / 28;
	if (col > 2) col = 2;

	// Return index into _foodVals: section*3 + col
	return section * 3 + col;
}

// ============================================================================
// Drawing
// ============================================================================

void ChezNorfPuzzle::draw(Graphics::ManagedSurface *screen) {
	// Background (loaded by base class)
	if (_background) {
		_background->drawToSurface(screen, 0, 0);
	}

	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Order/command overlay background (from DrawBoard: diff1→comande3, diff2→comande1, diff3→comande2)
	RleBlock *bgOverlay = nullptr;
	switch (_difficulty) {
	case 1: bgOverlay = _comandeGfx[2]; break;  // comande3
	case 2: bgOverlay = _comandeGfx[0]; break;  // comande1
	case 3: bgOverlay = _comandeGfx[1]; break;  // comande2
	default: break;
	}
	if (bgOverlay && bgOverlay->isValid()) {
		bgOverlay->drawToScreen(screen, 0, 0, lut);
	}

	drawFoodBoard(screen);
	drawTables(screen);
	drawPlates(screen);
	drawNorf(screen);
	drawZoombinis(screen);

	// Draw feedback overlay for current state
	if (_currentTable >= 0) {
		int tx = _tables[_currentTable].x;
		int ty = _tables[_currentTable].y - 40;

		if (_state == kStateCorrect && _symbOK && _symbOK->isValid()) {
			_symbOK->drawToScreen(screen, tx, ty, lut);
		} else if (_state == kStateWrong && _symbNO && _symbNO->isValid()) {
			_symbNO->drawToScreen(screen, tx, ty, lut);
		}
	}
}

void ChezNorfPuzzle::drawFoodBoard(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw 3 sections of food grid dots from DrawBoard_40F9B0
	// Section 1: y starts at 47, step 15; Section 2: 104; Section 3: 164
	static const int kSectionStartY[] = { 47, 104, 164 };

	// Symbol type mapping: 1→symbNO, 2→symbOK, 3→symbMaybe (from DrawBoard)
	RleBlock *symbByType[] = { nullptr, _symbNO, _symbOK, _symbMaybe };

	for (int section = 0; section < 3; section++) {
		int baseX = 51;
		for (int col = 0; col < 6; col++) {
			int gridY = kSectionStartY[section];
			for (int row = 0; row < 3; row++) {
				int val = _foodGrid[section][col][row];
				if (val >= 1 && val <= 3 && symbByType[val] && symbByType[val]->isValid()) {
					symbByType[val]->drawToScreen(screen, baseX, gridY, lut);
				}
				gridY += 15;
			}
			baseX += 14;
		}
	}
}

void ChezNorfPuzzle::drawTables(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw highlight on non-completed tables
	for (int i = 0; i < _numTables; i++) {
		if (!_tables[i].completed && _highlightGfx && _highlightGfx->isValid()) {
			// Highlight drawn at norf slot position (y=205 area from Init)
			_highlightGfx->drawToScreen(screen, _tables[i].x, 205, lut);
		}
	}
}

void ChezNorfPuzzle::drawPlates(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (int i = 0; i < _numTables; i++) {
		if (_tables[i].completed)
			continue;

		// Draw plate at table position
		if (_plato && _plato->isValid()) {
			_plato->drawToScreen(screen, _tables[i].x, _tables[i].y, lut);
		}

		// Draw food items on served plates
		if (_tables[i].served) {
			int fx = _tables[i].x + 5;
			int fy = _tables[i].y + 5;

			// Draw slurp
			if (_tables[i].foodSlurp >= 0 && _tables[i].foodSlurp < 3) {
				RleBlock *gfx = _slurpGfx[_tables[i].foodSlurp];
				if (gfx && gfx->isValid())
					gfx->drawToScreen(screen, fx, fy, lut);
			}

			// Draw miam
			if (_tables[i].foodMiam >= 0 && _tables[i].foodMiam < 3) {
				RleBlock *gfx = _miamGfx[_tables[i].foodMiam];
				if (gfx && gfx->isValid())
					gfx->drawToScreen(screen, fx + 20, fy, lut);
			}

			// Draw glouglou
			if (_tables[i].foodGlouglou >= 0 && _tables[i].foodGlouglou < 3) {
				RleBlock *gfx = _glouglouGfx[_tables[i].foodGlouglou];
				if (gfx && gfx->isValid())
					gfx->drawToScreen(screen, fx + 40, fy, lut);
			}
		}
	}
}

void ChezNorfPuzzle::drawNorf(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw Norf default sprite at center area
	if (_norfDefault && _norfDefault->isValid()) {
		_norfDefault->drawToScreen(screen, 100, 350, lut);
	}
}

void ChezNorfPuzzle::drawZoombinis(Graphics::ManagedSurface *screen) {
	if (!_zoombiniGfx)
		return;

	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw zoombinis at their table positions
	for (int i = 0; i < _numTables; i++) {
		if (_tables[i].completed)
			continue;

		int zIdx = _tables[i].zoombiniIdx;
		if (zIdx < 0 || zIdx >= (int)_puzzleZoombinis.size())
			continue;

		const Zoombini *z = _puzzleZoombinis[zIdx];
		int x = _tables[i].x;
		int y = _tables[i].y - 60;  // Above the plate

		// Body sprite (standing, frame 0)
		const RleBlock *body = _zoombiniGfx->getFrame(0, 0);
		if (body)
			body->drawToScreen(screen, x, y, lut);

		// Features
		const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
		for (int feat = 1; feat <= 4; feat++) {
			int idx = feat * ZoombiniGfx::kDim2 + features[feat - 1];
			const RleBlock *frame = _zoombiniGfx->getFrame(idx, 0);
			if (frame)
				frame->drawToScreen(screen, x, y, lut);
		}
	}
}

} // End of namespace Zoombini2
