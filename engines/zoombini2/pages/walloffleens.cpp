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
#include "common/system.h"
#include "graphics/managed_surface.h"

#include "zoombini2/pages/walloffleens.h"
#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"
#include "zoombini2/sound.h"

namespace Zoombini2 {

// ============================================================================
// Timing constants (ms) — from IDA HandleInput_42FE40
// ============================================================================
static const uint32 kAimStepDelay = 200;   // Cannon rotation: 200ms per step
static const uint32 kFireDuration = 600;   // Cannonball flight time
static const uint32 kHitDelay = 1200;      // Show hit result
static const uint32 kMissDelay = 1200;     // Show miss result
static const uint32 kNextDelay = 800;      // Before next zoombini
static const uint32 kDoneDelay = 3000;     // Before page transition

// Grid origin positions by difficulty (from IDA dword_48FF9C/dword_48FFA0)
// Indexed as: kGridOrigins[difficulty][0=x, 1=y]
static const int kGridOrigins[][2] = {
	{  0,   0},  // unused (difficulty 0)
	{380, 200},  // diff 1 — fallback (uses cycling panels normally)
	{200,   7},  // diff 2 — 9×6 grid
	{100,   7},  // diff 3 — 12×6 grid
	{ 95,   7},  // diff 4 — 12×6 grid
};

// Diff 1 cycling panel origins (from IDA dword_48FFC8/dword_48FFCC, 6 panels)
static const int kDiff1PanelOrigins[][2] = {
	{182,  88},  // panel 0
	{344,  88},  // panel 1
	{506,  87},  // panel 2
	{183, 230},  // panel 3
	{345, 229},  // panel 4
	{508, 230},  // panel 5
};

// Cannon muzzle endpoint per angle (from IDA dword_490040/dword_490044)
static const int kCannonMuzzle[][2] = {
	{500, 432},  // angle 0
	{500, 432},  // angle 1
	{480, 421},  // angle 2
	{460, 418},  // angle 3
	{441, 404},  // angle 4 (center — pointing up)
	{415, 425},  // angle 5
	{385, 428},  // angle 6
	{379, 459},  // angle 7
	{379, 459},  // angle 8
};

// Mirrors per difficulty (from IDA Init_432150, offset +240228)
static const int kMirrorsPerDifficulty[] = {0, 12, 8, 6, 6};

// ============================================================================
// Constructor / Destructor
// ============================================================================

WallOfFleensPuzzle::WallOfFleensPuzzle(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageWallOfFleens),
	  _difficulty(1), _gridCols(3), _gridRows(2), _numFleens(6),
	  _initialZoombiniCount(0), _freedCount(0),
	  _currentZoombini(0), _selectedFleen(-1),
	  _gameState(kStateIdle), _actionTimer(0),
	  _gridPage(0), _gridOriginX(380), _gridOriginY(200),
	  _cannonAngle(4), _targetAngle(4),
	  _targetCol(0), _targetRow(0),
	  _cannonballX(0), _cannonballY(0),
	  _cannonballStartX(0), _cannonballStartY(0),
	  _cannonballEndX(0), _cannonballEndY(0),
	  _cannonballProgress(0),
	  _mirrorsLeft(12), _mirrorsTotal(12),
	  _cannonCache(nullptr), _slotActiveGfx(nullptr),
	  _slotEmptyGfx(nullptr), _slotCursorGfx(nullptr),
	  _tuyereGfx(nullptr), _highlightGfx(nullptr),
	  _lavaBubbleAnim(nullptr), _mirrorExplodeAnim(nullptr),
	  _musicId(-1) {

	for (int i = 0; i < kNumCannonAngles; i++)
		_cannonGfx[i] = nullptr;
	for (int i = 0; i < kNumMirrorStates; i++)
		_mirrorGfx[i] = nullptr;
	for (int i = 0; i < kNumLevelIndicators; i++)
		_levelRedGfx[i] = nullptr;
}

WallOfFleensPuzzle::~WallOfFleensPuzzle() {
	if (_musicId >= 0) {
		SoundManager *snd = _engine->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	for (int i = 0; i < kNumCannonAngles; i++)
		delete _cannonGfx[i];
	delete _cannonCache;
	delete _slotActiveGfx;
	delete _slotEmptyGfx;
	delete _slotCursorGfx;
	for (int i = 0; i < kNumMirrorStates; i++)
		delete _mirrorGfx[i];
	delete _tuyereGfx;
	for (int i = 0; i < kNumLevelIndicators; i++)
		delete _levelRedGfx[i];
	delete _highlightGfx;
	delete _lavaBubbleAnim;
	delete _mirrorExplodeAnim;
}

// ============================================================================
// Resource Loading
// ============================================================================

void WallOfFleensPuzzle::loadResources() {
	// Cannon angle sprites (canon00-08.rb)
	for (int i = 0; i < kNumCannonAngles; i++) {
		Common::Path path(Common::String::format("bmp/wall_of_fleens/canon0%d", i));
		_cannonGfx[i] = new RleBlock();
		_cannonGfx[i]->loadFromFile(path);
	}

	// Cannon cache/cover sprite
	_cannonCache = new RleBlock();
	_cannonCache->loadFromFile(Common::Path("bmp/wall_of_fleens/canon_cache"));

	// Mirror state sprites
	_mirrorGfx[kMirrorNormal] = new RleBlock();
	_mirrorGfx[kMirrorNormal]->loadFromFile(Common::Path("bmp/wall_of_fleens/mirror_nomal"));

	_mirrorGfx[kMirrorGris] = new RleBlock();
	_mirrorGfx[kMirrorGris]->loadFromFile(Common::Path("bmp/wall_of_fleens/mirror_GRIS"));

	_mirrorGfx[kMirrorNoir] = new RleBlock();
	_mirrorGfx[kMirrorNoir]->loadFromFile(Common::Path("bmp/wall_of_fleens/mirror_NOIR"));

	_mirrorGfx[kMirrorFelure] = new RleBlock();
	_mirrorGfx[kMirrorFelure]->loadFromFile(Common::Path("bmp/wall_of_fleens/mirror_felure"));

	_mirrorGfx[kMirrorEmpty] = new RleBlock();
	_mirrorGfx[kMirrorEmpty]->loadFromFile(Common::Path("bmp/wall_of_fleens/mirror_empty_tunnel"));

	// Nozzle sprite
	_tuyereGfx = new RleBlock();
	_tuyereGfx->loadFromFile(Common::Path("bmp/wall_of_fleens/tuyere"));

	// Level indicator sprites (LevelRED0-4)
	for (int i = 0; i < kNumLevelIndicators; i++) {
		Common::Path path(Common::String::format("bmp/wall_of_fleens/LevelRED%d", i));
		_levelRedGfx[i] = new RleBlock();
		_levelRedGfx[i]->loadFromFile(path);
	}

	// Lava bubble animation (background decoration)
	Common::Path lavaBubblePath("bmp/wall_of_fleens/lava_bubble");
	_lavaBubbleAnim = new Animation();
	if (!_lavaBubbleAnim->loadFromFile(lavaBubblePath)) {
		delete _lavaBubbleAnim;
		_lavaBubbleAnim = nullptr;
		warning("WallOfFleensPuzzle: Failed to load lava_bubble animation");
	}

	// Mirror explode animation (breaking effect)
	Common::Path mirrorExplodePath("bmp/wall_of_fleens/mirror_explode");
	_mirrorExplodeAnim = new Animation();
	if (!_mirrorExplodeAnim->loadFromFile(mirrorExplodePath)) {
		delete _mirrorExplodeAnim;
		_mirrorExplodeAnim = nullptr;
		warning("WallOfFleensPuzzle: Failed to load mirror_explode animation");
	}
}

// ============================================================================
// Init
// ============================================================================

void WallOfFleensPuzzle::init() {
	PuzzlePage::init();
	debug(1, "WallOfFleensPuzzle::init");

	// BGM: 05-BB01.wav (IDA: WallOfFleens__Init_432150)
	if (SoundManager *snd = _engine->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("sounds/music/05-BB01.wav"), true);
		if (_musicId >= 0) { snd->playLoop(_musicId); snd->setVolume(_musicId, snd->_volumeMusic); }
	}

	loadResources();

	// Determine difficulty from engine gameMode (1-based)
	_difficulty = _engine->getGameState()->_gameMode;
	if (_difficulty < 1) _difficulty = 1;
	if (_difficulty > 4) _difficulty = 4;

	_initialZoombiniCount = (int)_puzzleZoombinis.size();
	_freedCount = 0;
	_currentZoombini = 0;
	_selectedFleen = -1;
	_gridPage = 0;
	_cannonAngle = 4;  // Center position
	_targetAngle = 4;

	// Set mirror count from difficulty (from IDA Init_432150)
	_mirrorsTotal = kMirrorsPerDifficulty[_difficulty];
	_mirrorsLeft = _mirrorsTotal;

	buildGrid();
	generateFleenFeatures();

	_gameState = kStateIdle;
	_actionTimer = _engine->getGameTickCount();
}

// ============================================================================
// Grid Setup
// ============================================================================

void WallOfFleensPuzzle::buildGrid() {
	// Set grid dimensions based on difficulty (from BuildGridLayout_454990)
	switch (_difficulty) {
	case 1:
		_gridCols = 3;
		_gridRows = 2;
		break;
	case 2:
		_gridCols = 9;
		_gridRows = 6;
		break;
	case 3:
	case 4:
	default:
		_gridCols = 12;
		_gridRows = 6;
		break;
	}

	_numFleens = _gridCols * _gridRows;
	if (_numFleens > kMaxFleens)
		_numFleens = kMaxFleens;

	// Set grid origin based on difficulty (from IDA dword_48FF9C/dword_48FFA0)
	if (_difficulty == 1) {
		// Diff 1 uses cycling panels — start with first panel
		_gridOriginX = kDiff1PanelOrigins[_gridPage][0];
		_gridOriginY = kDiff1PanelOrigins[_gridPage][1];
	} else {
		int diffIdx = CLIP(_difficulty, 1, 4);
		_gridOriginX = kGridOrigins[diffIdx][0];
		_gridOriginY = kGridOrigins[diffIdx][1];
	}

	// Initialize grid cells with positions and hitboxes
	for (int row = 0; row < _gridRows; row++) {
		for (int col = 0; col < _gridCols; col++) {
			int idx = col + _gridCols * row;
			FleenCell &cell = _fleens[idx];
			cell.gridCol = col;
			cell.gridRow = row;
			cell.caught = false;

			int x = _gridOriginX + kCellWidth * col;
			int y = _gridOriginY + kCellHeight * row;
			cell.hitbox = Common::Rect(x, y, x + kCellWidth, y + kCellHeight);
		}
	}
}

void WallOfFleensPuzzle::generateFleenFeatures() {
	// From IDA InitGridDifficulty_455190:
	// Each fleen gets 4 random feature values (1-5).
	// No two fleens should have identical feature sets.
	Common::RandomSource *rng = _engine->getRandom();

	for (int i = 0; i < _numFleens; i++) {
		bool unique;
		do {
			unique = true;
			for (int f = 0; f < kNumFeatures; f++) {
				_fleens[i].features[f] = rng->getRandomNumber(kMaxFeatureVal - 1) + 1;
			}
			// Check uniqueness against all previously generated fleens
			for (int j = 0; j < i; j++) {
				bool same = true;
				for (int f = 0; f < kNumFeatures; f++) {
					if (_fleens[i].features[f] != _fleens[j].features[f]) {
						same = false;
						break;
					}
				}
				if (same) {
					unique = false;
					break;
				}
			}
		} while (!unique);
	}
}

// ============================================================================
// Update (per-frame non-blocking) — from IDA HandleInput_42FE40
// ============================================================================

void WallOfFleensPuzzle::update() {
	uint32 now = _engine->getGameTickCount();
	uint32 elapsed = now - _actionTimer;

	switch (_gameState) {
	case kStateIdle:
		// Waiting for player to click a fleen cell
		break;

	case kStateAiming:
		// Rotate cannon towards target angle, one step per kAimStepDelay
		// From IDA: if (target >= 4) current++; else current--;
		if (elapsed >= kAimStepDelay) {
			if (_cannonAngle == _targetAngle) {
				// Aimed correctly — fire!
				fireCannon();
			} else {
				// Rotate one step towards target
				if (_targetAngle >= 4) {
					_cannonAngle++;
				} else {
					_cannonAngle--;
				}
				_cannonAngle = CLIP(_cannonAngle, 0, kNumCannonAngles - 1);
				_actionTimer = now;
			}
		}
		break;

	case kStateFiring: {
		// Cannonball traveling from muzzle to target cell
		// Linear interpolation over kFireDuration
		if (elapsed >= kFireDuration) {
			_cannonballProgress = 1000;
			_cannonballX = _cannonballEndX;
			_cannonballY = _cannonballEndY;

			// Check feature match
			int fleenIdx = fleenIndexAt(_targetCol, _targetRow);
			int matchScore = countMatchingFeatures(fleenIdx);

			if (matchScore == kNumFeatures) {
				// Perfect match — catch the fleen
				catchFleen(fleenIdx);
			} else {
				// Miss — lose a mirror
				missFleen();
			}
		} else {
			// Interpolate cannonball position
			_cannonballProgress = (int)(elapsed * 1000 / kFireDuration);
			_cannonballX = _cannonballStartX + (_cannonballEndX - _cannonballStartX) * _cannonballProgress / 1000;
			_cannonballY = _cannonballStartY + (_cannonballEndY - _cannonballStartY) * _cannonballProgress / 1000;
		}
		break;
	}

	case kStateHit:
		// Show catch result, then advance
		if (elapsed >= kHitDelay) {
			advanceToNextZoombini();
		}
		break;

	case kStateMiss:
		// Show miss, then reset cannon and advance
		if (elapsed >= kMissDelay) {
			// Reset cannon angle back to center (from IDA: this+159 flag)
			_cannonAngle = 4;
			advanceToNextZoombini();
		}
		break;

	case kStateNextZoombini:
		// Brief delay before allowing next click
		if (elapsed >= kNextDelay) {
			_gameState = kStateIdle;
			_actionTimer = now;
		}
		break;

	case kStateDone:
		// Wait then transition out
		if (elapsed >= kDoneDelay) {
			_engine->_returningFromPuzzle = true;
			_engine->_maptransSourceWorld = _pageId;
			_engine->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

// ============================================================================
// Game Logic — faithful to IDA analysis
// ============================================================================

int WallOfFleensPuzzle::computeCannonAngle(int targetX, int targetY) const {
	// From IDA Zoombini__LoadAppearance_45C1A0:
	//   dx = targetX - cannonCenterX
	//   dist = sqrt(dx^2 + dy^2)
	//   angleDeg = acos(dx / dist) * 180 / PI
	//   if (cannonY < targetY) angleDeg = -angleDeg
	//   return angleDeg / (360 / totalDirections)  // totalDirections = 18
	int dx = targetX - kCannonCenterX;
	int dy = kCannonCenterY - targetY;
	double dist = sqrt((double)(dx * dx + dy * dy));
	if (dist < 1.0)
		return 4;  // Center

	double angleDeg = acos((double)dx / dist) * 180.0 / M_PI;
	if (kCannonCenterY < targetY)
		angleDeg = -angleDeg;

	int result = (int)angleDeg / 20;  // 360 / 18 = 20 degrees per step
	return CLIP(result, 0, kNumCannonAngles - 1);
}

int WallOfFleensPuzzle::countMatchingFeatures(int fleenIdx) const {
	// From IDA ResetGridState_4552D0:
	// Compare 4 features of fleen cell against current zoombini
	// Returns count of matching features (0-4)
	if (fleenIdx < 0 || fleenIdx >= _numFleens)
		return 0;
	if (_currentZoombini < 0 || _currentZoombini >= (int)_puzzleZoombinis.size())
		return 0;

	const FleenCell &cell = _fleens[fleenIdx];
	const Zoombini *z = _puzzleZoombinis[_currentZoombini];

	int matches = 0;
	// Features are 1-5 in both fleen cells and zoombini struct
	if (cell.features[0] == z->_featureA) matches++;
	if (cell.features[1] == z->_featureB) matches++;
	if (cell.features[2] == z->_featureC) matches++;
	if (cell.features[3] == z->_featureD) matches++;

	return matches;
}

int WallOfFleensPuzzle::fleenIndexAt(int col, int row) const {
	if (col < 0 || col >= _gridCols || row < 0 || row >= _gridRows)
		return -1;
	return col + _gridCols * row;
}

void WallOfFleensPuzzle::fireCannon() {
	// Set up cannonball trajectory from muzzle to target fleen center
	int angle = CLIP(_cannonAngle, 0, kNumCannonAngles - 1);
	_cannonballStartX = kCannonMuzzle[angle][0];
	_cannonballStartY = kCannonMuzzle[angle][1];

	// Target: center of the fleen cell
	_cannonballEndX = _gridOriginX + kCellWidth * _targetCol + kCellWidth / 2;
	_cannonballEndY = _gridOriginY + kCellHeight * _targetRow + kCellHeight / 2;

	_cannonballX = _cannonballStartX;
	_cannonballY = _cannonballStartY;
	_cannonballProgress = 0;

	_gameState = kStateFiring;
	_actionTimer = _engine->getGameTickCount();

	debug(2, "WallOfFleens: Firing cannon angle %d from (%d,%d) to (%d,%d)",
	      _cannonAngle, _cannonballStartX, _cannonballStartY,
	      _cannonballEndX, _cannonballEndY);
}

void WallOfFleensPuzzle::catchFleen(int fleenIdx) {
	_fleens[fleenIdx].caught = true;
	_freedCount++;
	_selectedFleen = fleenIdx;
	_gameState = kStateHit;
	_actionTimer = _engine->getGameTickCount();

	debug(2, "WallOfFleens: Caught fleen %d (col=%d row=%d), freed %d/%d",
	      fleenIdx, _fleens[fleenIdx].gridCol, _fleens[fleenIdx].gridRow,
	      _freedCount, kMinFreed);
}

void WallOfFleensPuzzle::missFleen() {
	_mirrorsLeft--;
	_gameState = kStateMiss;
	_actionTimer = _engine->getGameTickCount();

	debug(2, "WallOfFleens: Miss! Mirrors remaining: %d/%d",
	      _mirrorsLeft, _mirrorsTotal);
}

void WallOfFleensPuzzle::advanceToNextZoombini() {
	_currentZoombini++;
	_selectedFleen = -1;
	_cannonballProgress = 0;

	// Reset cannon back to center
	_cannonAngle = 4;
	_targetAngle = 4;

	checkCompletion();
	if (_gameState != kStateDone) {
		_gameState = kStateNextZoombini;
		_actionTimer = _engine->getGameTickCount();
	}
}

void WallOfFleensPuzzle::checkCompletion() {
	// From IDA CheckFreeZoombinis_432EB0:
	// freed = initialCount - currentRemainingCount
	// Complete when freed >= kMinFreed
	if (_freedCount >= kMinFreed) {
		debug(1, "WallOfFleens: Puzzle complete! Freed %d zoombinis", _freedCount);
		_gameState = kStateDone;
		_actionTimer = _engine->getGameTickCount();
		return;
	}

	// No mirrors left — round over (fail, but still transition)
	if (_mirrorsLeft <= 0) {
		debug(1, "WallOfFleens: No mirrors left, transitioning out");
		_gameState = kStateDone;
		_actionTimer = _engine->getGameTickCount();
		return;
	}

	// No more zoombinis to try
	if (_currentZoombini >= (int)_puzzleZoombinis.size()) {
		debug(1, "WallOfFleens: No more zoombinis, transitioning out");
		_gameState = kStateDone;
		_actionTimer = _engine->getGameTickCount();
	}
}

// ============================================================================
// HandleClick — from IDA HandleInput_42FE40 click section
// ============================================================================

void WallOfFleensPuzzle::handleClick(const Common::Point &pos) {
	if (_gameState != kStateIdle)
		return;

	if (_currentZoombini >= (int)_puzzleZoombinis.size())
		return;

	// Check if player clicked on an uncaught fleen cell
	for (int i = 0; i < _numFleens; i++) {
		if (_fleens[i].caught)
			continue;
		if (_fleens[i].hitbox.contains(pos)) {
			_selectedFleen = i;
			_targetCol = _fleens[i].gridCol;
			_targetRow = _fleens[i].gridRow;

			// Compute what angle the cannon should aim at
			int cellCenterX = _gridOriginX + kCellWidth * _targetCol + kCellWidth / 2;
			int cellCenterY = _gridOriginY + kCellHeight * _targetRow + kCellHeight / 2;
			_targetAngle = computeCannonAngle(cellCenterX, cellCenterY);

			if (_cannonAngle == _targetAngle) {
				// Already aimed — fire immediately
				fireCannon();
			} else {
				// Start rotating cannon
				_gameState = kStateAiming;
				_actionTimer = _engine->getGameTickCount();
			}

			debug(2, "WallOfFleens: Clicked fleen %d at col=%d row=%d, "
			      "target angle=%d, current=%d",
			      i, _targetCol, _targetRow, _targetAngle, _cannonAngle);
			return;
		}
	}
}

// ============================================================================
// Draw
// ============================================================================

void WallOfFleensPuzzle::draw(Graphics::ManagedSurface *screen) {
	if (_background) {
		_background->drawToSurface(screen, 0, 0);
	}

	// Draw lava bubble decoration (lower left area)
	if (_lavaBubbleAnim) {
		uint32 now = _engine->getGameTickCount();
		int frameCount = _lavaBubbleAnim->getFrameCount();
		if (frameCount > 0) {
			int frameIdx = (now / 100) % frameCount; // ~10 fps
			const RleBlock *frame = _lavaBubbleAnim->getFrame(frameIdx);
			if (frame) {
				const byte (*lut)[256] = _engine->getAlphaLUT();
				frame->drawToScreen(screen, 100, 450, lut);
			}
		}
	}

	drawGrid(screen);
	drawMirrors(screen);
	drawCannon(screen);
	drawCannonball(screen);
	drawZoombinis(screen);

	// Draw level indicator for diff 1 cycling panels
	if (_difficulty == 1 && _gridPage < kNumLevelIndicators && _levelRedGfx[_gridPage]) {
		_levelRedGfx[_gridPage]->drawToScreen(screen, 10, 10, nullptr);
	}
}

void WallOfFleensPuzzle::drawGrid(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (int i = 0; i < _numFleens; i++) {
		const FleenCell &cell = _fleens[i];
		int x = cell.hitbox.left;
		int y = cell.hitbox.top;

		if (cell.caught) {
			// Draw empty/destroyed slot
			if (_mirrorGfx[kMirrorEmpty]) {
				_mirrorGfx[kMirrorEmpty]->drawToScreen(screen, x, y, lut);
			}
			continue;
		}

		// Draw active fleen cell background
		if (_mirrorGfx[kMirrorNormal]) {
			_mirrorGfx[kMirrorNormal]->drawToScreen(screen, x, y, lut);
		}

		// Draw fleen zoombini sprite on the cell
		if (_zoombiniGfx) {
			int baseIdx = 0;
			const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
			int zx = x + 4;
			int zy = y + 4;
			if (frame)
				frame->drawToScreen(screen, zx, zy, lut);

			for (int slot = 1; slot <= kNumFeatures; slot++) {
				int featVal = cell.features[slot - 1];
				if (featVal < 1) featVal = 1;
				int featIdx = baseIdx + slot * ZoombiniGfx::kDim2 + featVal;
				frame = _zoombiniGfx->getFrame(featIdx, 0);
				if (frame)
					frame->drawToScreen(screen, zx, zy, lut);
			}
		}

		// Highlight the currently selected/targeted fleen
		if (i == _selectedFleen && (_gameState == kStateAiming || _gameState == kStateFiring)) {
			if (_tuyereGfx) {
				_tuyereGfx->drawToScreen(screen, x, y, lut);
			}
		}

		// Show hit result on caught fleen
		if (i == _selectedFleen && _gameState == kStateHit) {
			if (_highlightGfx) {
				_highlightGfx->drawToScreen(screen, x, y, lut);
			} else if (_mirrorGfx[kMirrorExplode]) {
				_mirrorGfx[kMirrorExplode]->drawToScreen(screen, x, y, lut);
			}
		}
	}
}

void WallOfFleensPuzzle::drawCannon(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw cannon sprite at correct angle (from IDA: drawn at 385, 465)
	int angle = CLIP(_cannonAngle, 0, kNumCannonAngles - 1);
	if (_cannonGfx[angle]) {
		_cannonGfx[angle]->drawToScreen(screen, kCannonDrawX, kCannonDrawY, lut);
	}

	// Draw cannon cache/cover overlay
	if (_cannonCache) {
		_cannonCache->drawToScreen(screen, kCannonDrawX, kCannonDrawY, lut);
	}
}

void WallOfFleensPuzzle::drawCannonball(Graphics::ManagedSurface *screen) {
	if (_gameState != kStateFiring || _cannonballProgress <= 0)
		return;

	// Draw a simple cannonball at current interpolated position
	// Use tuyere sprite as cannonball placeholder if no dedicated sprite
	const byte (*lut)[256] = _engine->getAlphaLUT();
	if (_tuyereGfx) {
		_tuyereGfx->drawToScreen(screen, _cannonballX - 8, _cannonballY - 8, lut);
	} else {
		// Fallback: draw a small rectangle
		screen->fillRect(Common::Rect(_cannonballX - 4, _cannonballY - 4,
		                              _cannonballX + 4, _cannonballY + 4), 0);
	}
}

void WallOfFleensPuzzle::drawMirrors(Graphics::ManagedSurface *screen) {
	// Draw mirror/chance indicators at bottom-right (from IDA: x=690+23*i, y=424)
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (int i = 0; i < _mirrorsTotal; i++) {
		int mx = 690 + 23 * i;
		int my = 424;

		// Show explode animation on the most recently broken mirror
		if (i == _mirrorsLeft && _gameState == kStateMiss && _mirrorExplodeAnim) {
			uint32 elapsed = _engine->getGameTickCount() - _actionTimer;
			int frameCount = _mirrorExplodeAnim->getFrameCount();
			if (frameCount > 0 && elapsed < (uint32)(frameCount * 60)) {
				int frameIdx = (elapsed / 60) % frameCount; // ~16.7 fps
				const RleBlock *frame = _mirrorExplodeAnim->getFrame(frameIdx);
				if (frame) {
					frame->drawToScreen(screen, mx, my, lut);
					continue; // Skip normal drawing for this mirror
				}
			}
		}

		if (i < _mirrorsLeft) {
			if (_mirrorGfx[kMirrorNormal])
				_mirrorGfx[kMirrorNormal]->drawToScreen(screen, mx, my, lut);
		} else {
			if (_mirrorGfx[kMirrorEmpty])
				_mirrorGfx[kMirrorEmpty]->drawToScreen(screen, mx, my, lut);
		}
	}
}

void WallOfFleensPuzzle::drawZoombinis(Graphics::ManagedSurface *screen) {
	if (!_zoombiniGfx || _puzzleZoombinis.empty())
		return;

	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw current zoombini near cannon (from IDA: cannon at 385,465)
	if (_currentZoombini < (int)_puzzleZoombinis.size()) {
		const Zoombini *z = _puzzleZoombinis[_currentZoombini];
		int x = kCannonDrawX - 60;
		int y = kCannonDrawY + 10;
		int baseIdx = 0;

		const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
		if (frame)
			frame->drawToScreen(screen, x, y, lut);

		const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
		for (int slot = 1; slot <= 4; slot++) {
			int featIdx = baseIdx + slot * ZoombiniGfx::kDim2 + features[slot - 1];
			frame = _zoombiniGfx->getFrame(featIdx, 0);
			if (frame)
				frame->drawToScreen(screen, x, y, lut);
		}
	}

	// Draw remaining zoombinis in a queue line
	int startX = 50;
	int startY = 550;
	int spacing = 35;
	int count = MIN((int)_puzzleZoombinis.size(), 16);
	for (int i = _currentZoombini + 1; i < count; i++) {
		const Zoombini *z = _puzzleZoombinis[i];
		int x = startX + (i - _currentZoombini - 1) * spacing;
		int y = startY;
		int baseIdx = 0;

		const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
		if (frame)
			frame->drawToScreen(screen, x, y, lut);

		const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
		for (int slot = 1; slot <= 4; slot++) {
			int featIdx = baseIdx + slot * ZoombiniGfx::kDim2 + features[slot - 1];
			frame = _zoombiniGfx->getFrame(featIdx, 0);
			if (frame)
				frame->drawToScreen(screen, x, y, lut);
		}
	}
}

} // End of namespace Zoombini2
