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

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_walloffleens.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// Interaction timing in milliseconds.
// ============================================================================
static const uint32 kAimStepDelay = 200; // Cannon rotation: 200ms per step
static const uint32 kFireDuration = 600; // Cannonball flight time
static const uint32 kHitDelay = 1200;    // Show hit result
static const uint32 kMissDelay = 1200;   // Show miss result
static const uint32 kNextDelay = 800;    // Before next zoombini
static const uint32 kDoneDelay = 3000;   // Before page transition

// Grid origins indexed by difficulty.
static const Common::Point32 kGridOrigins[] = {
	Common::Point32(0, 0),     // unused (difficulty 0)
	Common::Point32(380, 200), // Difficulty one fallback; normal play cycles panel origins.
	Common::Point32(200, 7),   // Difficulty two uses a 9 by 6 grid.
	Common::Point32(100, 7),   // Difficulty three uses a 12 by 6 grid.
	Common::Point32(95, 7),    // Difficulty four uses a 12 by 6 grid.
};

// Six panel origins cycled by difficulty one.
static const Common::Point32 kDiff1PanelOrigins[] = {
	Common::Point32(182, 88),  // panel 0
	Common::Point32(344, 88),  // panel 1
	Common::Point32(506, 87),  // panel 2
	Common::Point32(183, 230), // panel 3
	Common::Point32(345, 229), // panel 4
	Common::Point32(508, 230), // panel 5
};

// Cannon muzzle endpoint indexed by angle.
static const Common::Point32 kCannonMuzzlePositions[] = {
	Common::Point32(500, 432), // angle 0
	Common::Point32(500, 432), // angle 1
	Common::Point32(480, 421), // angle 2
	Common::Point32(460, 418), // angle 3
	Common::Point32(441, 404), // Angle four is centered and points upward.
	Common::Point32(415, 425), // angle 5
	Common::Point32(385, 428), // angle 6
	Common::Point32(379, 459), // angle 7
	Common::Point32(379, 459), // angle 8
};

static const Common::Point32 kCannonDrawPosition(385, 465);
static const Common::Point32 kCannonCenterPosition(470, 550);

// Mirror allowances indexed by difficulty.
static const int kMirrorsPerDifficulty[] = {0, 12, 8, 6, 6};

// ============================================================================
// Constructor / Destructor
// ============================================================================

WallOfFleensPuzzle::WallOfFleensPuzzle(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageWallOfFleens),
	  _difficulty(1), _gridCols(3), _gridRows(2), _numFleens(6),
	  _initialZoombiniCount(0), _freedCount(0),
	  _currentZoombini(0), _selectedFleen(-1),
	  _gameState(kStateIdle00), _actionTimer(0),
	  _gridPage(0), _gridOrigin(380, 200),
	  _cannonAngle(4), _targetAngle(4),
	  _targetCol(0), _targetRow(0),
	  _cannonballPosition(0, 0),
	  _cannonballStartPosition(0, 0),
	  _cannonballEndPosition(0, 0),
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
	_mirrorGfx[kMirrorNormal00] = new RleBlock();
	_mirrorGfx[kMirrorNormal00]->loadFromFile(Common::Path("bmp/wall_of_fleens/mirror_nomal"));

	_mirrorGfx[kMirrorGris01] = new RleBlock();
	_mirrorGfx[kMirrorGris01]->loadFromFile(Common::Path("bmp/wall_of_fleens/mirror_GRIS"));

	_mirrorGfx[kMirrorNoir02] = new RleBlock();
	_mirrorGfx[kMirrorNoir02]->loadFromFile(Common::Path("bmp/wall_of_fleens/mirror_NOIR"));

	_mirrorGfx[kMirrorFelure03] = new RleBlock();
	_mirrorGfx[kMirrorFelure03]->loadFromFile(Common::Path("bmp/wall_of_fleens/mirror_felure"));

	_mirrorGfx[kMirrorEmpty05] = new RleBlock();
	_mirrorGfx[kMirrorEmpty05]->loadFromFile(Common::Path("bmp/wall_of_fleens/mirror_empty_tunnel"));

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

	// Start the Magic Mirrors music.
	if (SoundManager *snd = _engine->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("sounds/music/05-BB01.wav"), true);
		if (_musicId >= 0) {
			snd->playLoop(_musicId);
			snd->setVolume(_musicId, snd->_volumeMusic);
		}
	}

	loadResources();

	// Determine difficulty from engine gameMode (1-based)
	_difficulty = _engine->getGameState()->_gameMode;
	if (_difficulty < 1)
		_difficulty = 1;
	if (_difficulty > 4)
		_difficulty = 4;

	_initialZoombiniCount = (int)_puzzleZoombinis.size();
	_freedCount = 0;
	_currentZoombini = 0;
	_selectedFleen = -1;
	_gridPage = 0;
	_cannonAngle = 4; // Center position
	_targetAngle = 4;

	// Set the mirror allowance for the selected difficulty.
	_mirrorsTotal = kMirrorsPerDifficulty[_difficulty];
	_mirrorsLeft = _mirrorsTotal;

	buildGrid();
	generateFleenFeatures();

	_gameState = kStateIdle00;
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

	// Select the grid origin for this difficulty.
	if (_difficulty == 1) {
		// Difficulty one begins with the first cycling panel.
		_gridOrigin = kDiff1PanelOrigins[_gridPage];
	} else {
		int diffIdx = CLIP(_difficulty, 1, 4);
		_gridOrigin = kGridOrigins[diffIdx];
	}

	// Initialize grid cells with positions and hitboxes
	for (int row = 0; row < _gridRows; row++) {
		for (int col = 0; col < _gridCols; col++) {
			int idx = col + _gridCols * row;
			FleenCell &cell = _fleens[idx];
			cell.gridCol = col;
			cell.gridRow = row;
			cell.caught = false;

			Common::Point32 cellPosition(_gridOrigin.x + kCellWidth * col, _gridOrigin.y + kCellHeight * row);
			cell.hitbox = Common::Rect(
				static_cast<int16>(cellPosition.x), static_cast<int16>(cellPosition.y),
				static_cast<int16>(cellPosition.x + kCellWidth), static_cast<int16>(cellPosition.y + kCellHeight));
		}
	}
}

void WallOfFleensPuzzle::generateFleenFeatures() {
	// Each Fleen gets four random feature values in the range one through five.
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
// Per-frame interaction update.
// ============================================================================

void WallOfFleensPuzzle::update() {
	uint32 now = _engine->getGameTickCount();
	uint32 elapsed = now - _actionTimer;

	switch (_gameState) {
	case kStateIdle00:
		// Waiting for player to click a fleen cell
		break;

	case kStateAiming01:
		// Rotate cannon towards target angle, one step per kAimStepDelay
		// Rotate one angle step toward the selected target.
		if (elapsed >= kAimStepDelay) {
			if (_cannonAngle == _targetAngle) {
				// Fire after the cannon reaches its target angle.
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

	case kStateFiring02: {
		// Cannonball traveling from muzzle to target cell
		// Linear interpolation over kFireDuration
		if (elapsed >= kFireDuration) {
			_cannonballProgress = 1000;
			_cannonballPosition = _cannonballEndPosition;

			// Check feature match
			int fleenIdx = fleenIndexAt(_targetCol, _targetRow);
			int matchScore = countMatchingFeatures(fleenIdx);

			if (matchScore == kNumFeatures) {
				// Capture the Fleen after a complete feature match.
				catchFleen(fleenIdx);
			} else {
				// Consume a mirror after an incomplete feature match.
				missFleen();
			}
		} else {
			// Interpolate cannonball position
			_cannonballProgress = static_cast<int>(elapsed * 1000 / kFireDuration);
			_cannonballPosition.x = _cannonballStartPosition.x
				+ (_cannonballEndPosition.x - _cannonballStartPosition.x) * _cannonballProgress / 1000;
			_cannonballPosition.y = _cannonballStartPosition.y
				+ (_cannonballEndPosition.y - _cannonballStartPosition.y) * _cannonballProgress / 1000;
		}
		break;
	}

	case kStateHit03:
		// Show catch result, then advance
		if (elapsed >= kHitDelay) {
			advanceToNextZoombini();
		}
		break;

	case kStateMiss04:
		// Show miss, then reset cannon and advance
		if (elapsed >= kMissDelay) {
			// Return the cannon to its centered angle.
			_cannonAngle = 4;
			advanceToNextZoombini();
		}
		break;

	case kStateNextZoombini05:
		// Brief delay before allowing next click
		if (elapsed >= kNextDelay) {
			_gameState = kStateIdle00;
			_actionTimer = now;
		}
		break;

	case kStateDone06:
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
// Game logic
// ============================================================================

int WallOfFleensPuzzle::computeCannonAngle(const Common::Point32 &targetPosition) const {
	// Convert the target vector to degrees, divide it into direction sectors, and clamp the result.
	int32 dx = targetPosition.x - kCannonCenterPosition.x;
	int32 dy = kCannonCenterPosition.y - targetPosition.y;
	double dist = sqrt(static_cast<double>(dx * dx + dy * dy));
	if (dist < 1.0)
		return 4; // Center

	double angleDeg = acos(static_cast<double>(dx) / dist) * 180.0 / M_PI;
	if (kCannonCenterPosition.y < targetPosition.y)
		angleDeg = -angleDeg;

	int result = (int)angleDeg / 20; // 360 / 18 = 20 degrees per step
	return CLIP(result, 0, kNumCannonAngles - 1);
}

int WallOfFleensPuzzle::countMatchingFeatures(int fleenIdx) const {
	// Count matching visible features between the selected Fleen and current Zoombini.
	if (fleenIdx < 0 || fleenIdx >= _numFleens)
		return 0;
	if (_currentZoombini < 0 || _currentZoombini >= (int)_puzzleZoombinis.size())
		return 0;

	const FleenCell &cell = _fleens[fleenIdx];
	const ZoombiniState *z = _puzzleZoombinis[_currentZoombini];

	int matches = 0;
	// Features are 1-5 in both fleen cells and zoombini struct
	if (cell.features[0] == z->_featureA)
		matches++;
	if (cell.features[1] == z->_featureB)
		matches++;
	if (cell.features[2] == z->_featureC)
		matches++;
	if (cell.features[3] == z->_featureD)
		matches++;

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
	_cannonballStartPosition = kCannonMuzzlePositions[angle];

	// Target: center of the fleen cell
	_cannonballEndPosition = Common::Point32(
		_gridOrigin.x + kCellWidth * _targetCol + kCellWidth / 2,
		_gridOrigin.y + kCellHeight * _targetRow + kCellHeight / 2);

	_cannonballPosition = _cannonballStartPosition;
	_cannonballProgress = 0;

	_gameState = kStateFiring02;
	_actionTimer = _engine->getGameTickCount();

	debug(2, "WallOfFleens: Firing cannon angle %d from (%d,%d) to (%d,%d)",
		  _cannonAngle, _cannonballStartPosition.x, _cannonballStartPosition.y,
		  _cannonballEndPosition.x, _cannonballEndPosition.y);
}

void WallOfFleensPuzzle::catchFleen(int fleenIdx) {
	_fleens[fleenIdx].caught = true;
	_freedCount++;
	_selectedFleen = fleenIdx;
	_gameState = kStateHit03;
	_actionTimer = _engine->getGameTickCount();

	debug(2, "WallOfFleens: Caught fleen %d (col=%d row=%d), freed %d/%d",
		  fleenIdx, _fleens[fleenIdx].gridCol, _fleens[fleenIdx].gridRow,
		  _freedCount, kMinFreed);
}

void WallOfFleensPuzzle::missFleen() {
	_mirrorsLeft--;
	_gameState = kStateMiss04;
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
	if (_gameState != kStateDone06) {
		_gameState = kStateNextZoombini05;
		_actionTimer = _engine->getGameTickCount();
	}
}

void WallOfFleensPuzzle::checkCompletion() {
	// Complete when the required number of Zoombinis has been released.
	if (_freedCount >= kMinFreed) {
		debug(1, "WallOfFleens: Puzzle complete! Freed %d zoombinis", _freedCount);
		_gameState = kStateDone06;
		_actionTimer = _engine->getGameTickCount();
		return;
	}

	// A depleted mirror allowance ends the round and advances the route.
	if (_mirrorsLeft <= 0) {
		debug(1, "WallOfFleens: No mirrors left, transitioning out");
		_gameState = kStateDone06;
		_actionTimer = _engine->getGameTickCount();
		return;
	}

	// No more zoombinis to try
	if (_currentZoombini >= (int)_puzzleZoombinis.size()) {
		debug(1, "WallOfFleens: No more zoombinis, transitioning out");
		_gameState = kStateDone06;
		_actionTimer = _engine->getGameTickCount();
	}
}

// ============================================================================
// Click handling
// ============================================================================

void WallOfFleensPuzzle::handleClick(const Common::Point &pos) {
	if (_gameState != kStateIdle00)
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
			Common::Point32 cellCenterPosition(
				_gridOrigin.x + kCellWidth * _targetCol + kCellWidth / 2,
				_gridOrigin.y + kCellHeight * _targetRow + kCellHeight / 2);
			_targetAngle = computeCannonAngle(cellCenterPosition);

			if (_cannonAngle == _targetAngle) {
				// Fire immediately when the cannon is already aimed.
				fireCannon();
			} else {
				// Start rotating cannon
				_gameState = kStateAiming01;
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
				const byte(*lut)[256] = _engine->getAlphaLUT();
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
	const byte(*lut)[256] = _engine->getAlphaLUT();

	for (int i = 0; i < _numFleens; i++) {
		const FleenCell &cell = _fleens[i];
		Common::Point32 cellPosition(cell.hitbox.left, cell.hitbox.top);

		if (cell.caught) {
			// Draw empty/destroyed slot
			if (_mirrorGfx[kMirrorEmpty05]) {
				_mirrorGfx[kMirrorEmpty05]->drawToScreen(screen, cellPosition.x, cellPosition.y, lut);
			}
			continue;
		}

		// Draw active fleen cell background
		if (_mirrorGfx[kMirrorNormal00]) {
			_mirrorGfx[kMirrorNormal00]->drawToScreen(screen, cellPosition.x, cellPosition.y, lut);
		}

		// Draw fleen zoombini sprite on the cell
		if (_zoombiniGfx) {
			int baseIdx = 0;
			const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
			Common::Point32 zoombiniPosition(cellPosition.x + 4, cellPosition.y + 4);
			if (frame)
				frame->drawToScreen(screen, zoombiniPosition.x, zoombiniPosition.y, lut);

			for (int slot = 1; slot <= kNumFeatures; slot++) {
				int featVal = cell.features[slot - 1];
				if (featVal < 1)
					featVal = 1;
				int featIdx = baseIdx + slot * ZoombiniGraphics::kDim2 + featVal;
				frame = _zoombiniGfx->getFrame(featIdx, 0);
				if (frame)
					frame->drawToScreen(screen, zoombiniPosition.x, zoombiniPosition.y, lut);
			}
		}

		// Highlight the currently selected/targeted fleen
		if (i == _selectedFleen && (_gameState == kStateAiming01 || _gameState == kStateFiring02)) {
			if (_tuyereGfx) {
				_tuyereGfx->drawToScreen(screen, cellPosition.x, cellPosition.y, lut);
			}
		}

		// Show hit result on caught fleen
		if (i == _selectedFleen && _gameState == kStateHit03) {
			if (_highlightGfx) {
				_highlightGfx->drawToScreen(screen, cellPosition.x, cellPosition.y, lut);
			} else if (_mirrorGfx[kMirrorExplode04]) {
				_mirrorGfx[kMirrorExplode04]->drawToScreen(screen, cellPosition.x, cellPosition.y, lut);
			}
		}
	}
}

void WallOfFleensPuzzle::drawCannon(Graphics::ManagedSurface *screen) {
	const byte(*lut)[256] = _engine->getAlphaLUT();

	// Draw the cannon visual for its current angle.
	int angle = CLIP(_cannonAngle, 0, kNumCannonAngles - 1);
	if (_cannonGfx[angle]) {
		_cannonGfx[angle]->drawToScreen(screen, kCannonDrawPosition.x, kCannonDrawPosition.y, lut);
	}

	// Draw cannon cache/cover overlay
	if (_cannonCache) {
		_cannonCache->drawToScreen(screen, kCannonDrawPosition.x, kCannonDrawPosition.y, lut);
	}
}

void WallOfFleensPuzzle::drawCannonball(Graphics::ManagedSurface *screen) {
	if (_gameState != kStateFiring02 || _cannonballProgress <= 0)
		return;

	// Draw a simple cannonball at current interpolated position
	// Use tuyere sprite as cannonball placeholder if no dedicated sprite
	const byte(*lut)[256] = _engine->getAlphaLUT();
	if (_tuyereGfx) {
		_tuyereGfx->drawToScreen(screen, _cannonballPosition.x - 8, _cannonballPosition.y - 8, lut);
	} else {
		// Fallback: draw a small rectangle
		screen->fillRect(Common::Rect(static_cast<int16>(_cannonballPosition.x - 4), static_cast<int16>(_cannonballPosition.y - 4),
									  static_cast<int16>(_cannonballPosition.x + 4), static_cast<int16>(_cannonballPosition.y + 4)),
						 0);
	}
}

void WallOfFleensPuzzle::drawMirrors(Graphics::ManagedSurface *screen) {
	// Draw remaining-chance mirrors along the lower-right edge.
	const byte(*lut)[256] = _engine->getAlphaLUT();

	for (int i = 0; i < _mirrorsTotal; i++) {
		Common::Point32 mirrorPosition(690 + 23 * i, 424);

		// Show explode animation on the most recently broken mirror
		if (i == _mirrorsLeft && _gameState == kStateMiss04 && _mirrorExplodeAnim) {
			uint32 elapsed = _engine->getGameTickCount() - _actionTimer;
			int frameCount = _mirrorExplodeAnim->getFrameCount();
			if (frameCount > 0 && elapsed < (uint32)(frameCount * 60)) {
				int frameIdx = (elapsed / 60) % frameCount; // ~16.7 fps
				const RleBlock *frame = _mirrorExplodeAnim->getFrame(frameIdx);
				if (frame) {
					frame->drawToScreen(screen, mirrorPosition.x, mirrorPosition.y, lut);
					continue; // Skip normal drawing for this mirror
				}
			}
		}

		if (i < _mirrorsLeft) {
			if (_mirrorGfx[kMirrorNormal00])
				_mirrorGfx[kMirrorNormal00]->drawToScreen(screen, mirrorPosition.x, mirrorPosition.y, lut);
		} else {
			if (_mirrorGfx[kMirrorEmpty05])
				_mirrorGfx[kMirrorEmpty05]->drawToScreen(screen, mirrorPosition.x, mirrorPosition.y, lut);
		}
	}
}

void WallOfFleensPuzzle::drawZoombinis(Graphics::ManagedSurface *screen) {
	if (!_zoombiniGfx || _puzzleZoombinis.empty())
		return;

	const byte(*lut)[256] = _engine->getAlphaLUT();

	// Draw the current Zoombini beside the cannon.
	if (_currentZoombini < (int)_puzzleZoombinis.size()) {
		const ZoombiniState *z = _puzzleZoombinis[_currentZoombini];
		Common::Point32 zoombiniPosition(kCannonDrawPosition.x - 60, kCannonDrawPosition.y + 10);
		int baseIdx = 0;

		const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
		if (frame)
			frame->drawToScreen(screen, zoombiniPosition.x, zoombiniPosition.y, lut);

		const byte features[4] = {z->_featureA, z->_featureB, z->_featureC, z->_featureD};
		for (int slot = 1; slot <= 4; slot++) {
			int featIdx = baseIdx + slot * ZoombiniGraphics::kDim2 + features[slot - 1];
			frame = _zoombiniGfx->getFrame(featIdx, 0);
			if (frame)
				frame->drawToScreen(screen, zoombiniPosition.x, zoombiniPosition.y, lut);
		}
	}

	// Draw remaining zoombinis in a queue line
	const Common::Point32 queueStartPosition(50, 550);
	int spacing = 35;
	int count = MIN((int)_puzzleZoombinis.size(), 16);
	for (int i = _currentZoombini + 1; i < count; i++) {
		const ZoombiniState *z = _puzzleZoombinis[i];
		Common::Point32 zoombiniPosition(queueStartPosition.x + (i - _currentZoombini - 1) * spacing, queueStartPosition.y);
		int baseIdx = 0;

		const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
		if (frame)
			frame->drawToScreen(screen, zoombiniPosition.x, zoombiniPosition.y, lut);

		const byte features[4] = {z->_featureA, z->_featureB, z->_featureC, z->_featureD};
		for (int slot = 1; slot <= 4; slot++) {
			int featIdx = baseIdx + slot * ZoombiniGraphics::kDim2 + features[slot - 1];
			frame = _zoombiniGfx->getFrame(featIdx, 0);
			if (frame)
				frame->drawToScreen(screen, zoombiniPosition.x, zoombiniPosition.y, lut);
		}
	}
}

} // End of namespace Zoombini2
