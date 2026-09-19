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

#include "zoombini2/pages/puzzle_walloffleens.h"
#include "common/debug.h"
#include "common/system.h"
#include "zoombini2/graphics.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleWallOfFleens::kCannonFormat;
constexpr const char *PuzzleWallOfFleens::kCannonCachePath;
constexpr const char *PuzzleWallOfFleens::kMirrorNormalPath;
constexpr const char *PuzzleWallOfFleens::kMirrorGrisPath;
constexpr const char *PuzzleWallOfFleens::kMirrorNoirPath;
constexpr const char *PuzzleWallOfFleens::kMirrorFelurePath;
constexpr const char *PuzzleWallOfFleens::kMirrorEmptyPath;
constexpr const char *PuzzleWallOfFleens::kTuyerePath;
constexpr const char *PuzzleWallOfFleens::kLevelIndicatorFormat;
constexpr const char *PuzzleWallOfFleens::kLavaBubblePath;
constexpr const char *PuzzleWallOfFleens::kMirrorExplodePath;
constexpr const char *PuzzleWallOfFleens::kMusicPath;

constexpr Size32 PuzzleWallOfFleens::kCellSize;

// ============================================================================
// Interaction timing in milliseconds.
// ============================================================================
static constexpr uint32 kAimStepDelay = 200; // Cannon rotation: 200ms per step
static constexpr uint32 kFireDuration = 600; // Cannonball flight time
static constexpr uint32 kHitDelay = 1200;    // Show hit result
static constexpr uint32 kMissDelay = 1200;   // Show miss result
static constexpr uint32 kNextDelay = 800;    // Before next zoombini
static constexpr uint32 kDoneDelay = 3000;   // Before page transition

// Grid origins indexed by level.
static constexpr Common::Point32 kGridOrigins[] = {
	Common::Point32(0, 0),     // unused (level 0)
	Common::Point32(380, 200), // Level one fallback; normal play cycles panel origins.
	Common::Point32(200, 7),   // Level two uses a 9 by 6 grid.
	Common::Point32(100, 7),   // Level three uses a 12 by 6 grid.
	Common::Point32(95, 7),    // Level four uses a 12 by 6 grid.
};

// Six panel origins cycled by level one.
static constexpr Common::Point32 kLevel1PanelOrigins[] = {
	Common::Point32(182, 88),  // panel 0
	Common::Point32(344, 88),  // panel 1
	Common::Point32(506, 87),  // panel 2
	Common::Point32(183, 230), // panel 3
	Common::Point32(345, 229), // panel 4
	Common::Point32(508, 230), // panel 5
};

// Cannon muzzle endpoint indexed by angle.
static constexpr Common::Point32 kCannonMuzzlePos[] = {
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

static constexpr Common::Point32 kCannonDrawPos = Common::Point32(385, 465);
static constexpr Common::Point32 kCannonCenterPos = Common::Point32(470, 550);

// Mirror allowances indexed by level.
static constexpr int kMirrorsPerLevel[] = {0, 12, 8, 6, 6};

// ============================================================================
// Constructor / Destructor
// ============================================================================

PuzzleWallOfFleens::PuzzleWallOfFleens(Zoombini2Engine *vm)
	: PuzzleBase(vm, kPageWallOfFleens) {
}

PuzzleWallOfFleens::~PuzzleWallOfFleens() {
	for (int i = 0; i < kNumCannonAngles; i++)
		delete _cannonImage[i];
	delete _cannonCache;
	delete _slotActiveImage;
	delete _slotEmptyImage;
	delete _slotCursorImage;
	for (int i = 0; i < kNumMirrorStates; i++)
		delete _mirrorImage[i];
	delete _tuyereImage;
	for (int i = 0; i < kNumLevelIndicators; i++)
		delete _levelRedImage[i];
	delete _highlightImage;
	delete _lavaBubbleAnim;
	delete _mirrorExplodeAnim;
	finishPuzzleRoster(_vm->_state->_rescue1Board);
}

// ============================================================================
// Resource Loading
// ============================================================================

void PuzzleWallOfFleens::loadResources() {
	// Cannon angle sprites (canon00-08.rb)
	for (int i = 0; i < kNumCannonAngles; i++) {
		Common::Path path(Common::String::format(kCannonFormat, i));
		_cannonImage[i] = new RleBlock(_vm);
		_cannonImage[i]->loadFromFile(path);
	}

	// Cannon cache/cover sprite
	_cannonCache = new RleBlock(_vm);
	_cannonCache->loadFromFile(Common::Path(kCannonCachePath));

	// Mirror state sprites
	_mirrorImage[kMirrorNormal00] = new RleBlock(_vm);
	_mirrorImage[kMirrorNormal00]->loadFromFile(Common::Path(kMirrorNormalPath));

	_mirrorImage[kMirrorGris01] = new RleBlock(_vm);
	_mirrorImage[kMirrorGris01]->loadFromFile(Common::Path(kMirrorGrisPath));

	_mirrorImage[kMirrorNoir02] = new RleBlock(_vm);
	_mirrorImage[kMirrorNoir02]->loadFromFile(Common::Path(kMirrorNoirPath));

	_mirrorImage[kMirrorFelure03] = new RleBlock(_vm);
	_mirrorImage[kMirrorFelure03]->loadFromFile(Common::Path(kMirrorFelurePath));

	_mirrorImage[kMirrorEmpty05] = new RleBlock(_vm);
	_mirrorImage[kMirrorEmpty05]->loadFromFile(Common::Path(kMirrorEmptyPath));

	// Nozzle sprite
	_tuyereImage = new RleBlock(_vm);
	_tuyereImage->loadFromFile(Common::Path(kTuyerePath));

	// Level indicator sprites (LevelRED0-4)
	for (int i = 0; i < kNumLevelIndicators; i++) {
		Common::Path path(Common::String::format(kLevelIndicatorFormat, i));
		_levelRedImage[i] = new RleBlock(_vm);
		_levelRedImage[i]->loadFromFile(path);
	}

	// Lava bubble animation (background decoration)
	Common::Path lavaBubblePath(kLavaBubblePath);
	_lavaBubbleAnim = new Animation(_vm);
	if (!_lavaBubbleAnim->loadFromFile(lavaBubblePath)) {
		delete _lavaBubbleAnim;
		_lavaBubbleAnim = nullptr;
		warning("PuzzleWallOfFleens: Failed to load lava_bubble animation");
	}

	// Mirror explode animation (breaking effect)
	Common::Path mirrorExplodePath(kMirrorExplodePath);
	_mirrorExplodeAnim = new Animation(_vm);
	if (!_mirrorExplodeAnim->loadFromFile(mirrorExplodePath)) {
		delete _mirrorExplodeAnim;
		_mirrorExplodeAnim = nullptr;
		warning("PuzzleWallOfFleens: Failed to load mirror_explode animation");
	}
}

// ============================================================================
// Init
// ============================================================================

void PuzzleWallOfFleens::init() {
	PuzzleBase::init();
	debug(1, "PuzzleWallOfFleens::init");

	// Start the Magic Mirrors music.
	startPageMusic(Common::Path(kMusicPath));

	loadResources();

	// Determine level from the game state (1-based).
	_level = _vm->_state->_level;
	if (_level < 1)
		_level = 1;
	if (_level > 4)
		_level = 4;

	_initialZoombiniCount = _puzzleZoombinis.size();
	_freedCount = 0;
	_currentZoombini = 0;
	_selectedFleen = -1;
	_gridPage = 0;
	_cannonAngle = 4; // Center position
	_targetAngle = 4;

	// Set the mirror allowance for the selected level.
	_mirrorsTotal = kMirrorsPerLevel[_level];
	_mirrorsLeft = _mirrorsTotal;

	buildGrid();
	generateFleenTraits();

	_gameState = kStateIdle00;
	_actionTimer = _vm->getGameTickCount();
}

// ============================================================================
// Grid Setup
// ============================================================================

void PuzzleWallOfFleens::buildGrid() {
	_vm->reseedRandomForV10();

	// Set grid dimensions based on level.
	switch (_level) {
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

	// Select the grid origin for this level.
	if (_level == 1) {
		// Level one begins with the first cycling panel.
		_gridOrigin = kLevel1PanelOrigins[_gridPage];
	} else {
		int levelIdx = CLIP(_level, 1, 4);
		_gridOrigin = kGridOrigins[levelIdx];
	}

	// Initialize grid cells with positions and hitboxes
	for (int row = 0; row < _gridRows; row++) {
		for (int col = 0; col < _gridCols; col++) {
			int idx = col + _gridCols * row;
			FleenCell &cell = _fleens[idx];
			cell.gridCol = col;
			cell.gridRow = row;
			cell.caught = false;

			Common::Point32 cellPos(_gridOrigin.x + kCellSize.width * col, _gridOrigin.y + kCellSize.height * row);
			cell.hitbox = Common::Rect(
				static_cast<int16>(cellPos.x), static_cast<int16>(cellPos.y),
				static_cast<int16>(cellPos.x + kCellSize.width), static_cast<int16>(cellPos.y + kCellSize.height));
		}
	}
}

void PuzzleWallOfFleens::generateFleenTraits() {
	// Each Fleen gets four random trait values in the range one through five.
	// No two Fleens should have identical trait tuples.
	for (int i = 0; i < _numFleens; i++) {
		bool unique;
		do {
			unique = true;
			ZmbTrait &traits = _fleens[i].traits;
			traits._feet = _vm->_rnd->getRandomNumber(kMaxTraitValue - 1) + 1;
			traits._nose = _vm->_rnd->getRandomNumber(kMaxTraitValue - 1) + 1;
			traits._hair = _vm->_rnd->getRandomNumber(kMaxTraitValue - 1) + 1;
			traits._eyes = _vm->_rnd->getRandomNumber(kMaxTraitValue - 1) + 1;
			// Check uniqueness against all previously generated Fleens.
			for (int j = 0; j < i; j++) {
				if (traits == _fleens[j].traits) {
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

void PuzzleWallOfFleens::onUpdate() {
	uint32 now = _vm->getGameTickCount();
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
			_cannonballPos = _cannonballEndPos;

			// Check feature match
			int fleenIdx = fleenIndexAt(_targetCol, _targetRow);
			int matchScore = countMatchingTraits(fleenIdx);

			if (matchScore == ZmbTrait::kTraitCount) {
				// Capture the Fleen after a complete feature match.
				catchFleen(fleenIdx);
			} else {
				// Consume a mirror after an incomplete feature match.
				missFleen();
			}
		} else {
			// Interpolate cannonball position
			_cannonballProgress = static_cast<int>(elapsed * 1000 / kFireDuration);
			_cannonballPos.x = _cannonballStartPos.x + (_cannonballEndPos.x - _cannonballStartPos.x) * _cannonballProgress / 1000;
			_cannonballPos.y = _cannonballStartPos.y + (_cannonballEndPos.y - _cannonballStartPos.y) * _cannonballProgress / 1000;
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
			_vm->_returningFromPuzzle = true;
			_vm->_mapTransitionSourcePageId = static_cast<PageId>(_pageId);
			_vm->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

// ============================================================================
// Game logic
// ============================================================================

int PuzzleWallOfFleens::computeCannonAngle(const Common::Point32 &targetPos) const {
	// Convert the target vector to degrees, divide it into direction sectors, and clamp the result.
	int32 dx = targetPos.x - kCannonCenterPos.x;
	int32 dy = kCannonCenterPos.y - targetPos.y;
	double dist = sqrt(static_cast<double>(dx * dx + dy * dy));
	if (dist < 1.0)
		return 4; // Center

	double angleDeg = acos(static_cast<double>(dx) / dist) * 180.0 / M_PI;
	if (kCannonCenterPos.y < targetPos.y)
		angleDeg = -angleDeg;

	int result = (int)angleDeg / 20; // 360 / 18 = 20 degrees per step
	return CLIP(result, 0, kNumCannonAngles - 1);
}

int PuzzleWallOfFleens::countMatchingTraits(int fleenIdx) const {
	// Count matching visible traits between the selected Fleen and current Zoombini.
	if (fleenIdx < 0 || fleenIdx >= _numFleens)
		return 0;
	if (_puzzleZoombinis.size() <= _currentZoombini)
		return 0;

	const FleenCell &cell = _fleens[fleenIdx];
	const ZoombiniRunner *z = _puzzleZoombinis[_currentZoombini];

	int matches = 0;
	for (int traitOrdinal = 0; traitOrdinal < ZmbTrait::kTraitCount; traitOrdinal++) {
		const ZmbTrait::TraitIndex traitIndex = static_cast<ZmbTrait::TraitIndex>(traitOrdinal);
		if (cell.traits.getValue(traitIndex) == z->_traits.getValue(traitIndex))
			matches += 1;
	}

	return matches;
}

int PuzzleWallOfFleens::fleenIndexAt(int col, int row) const {
	if (col < 0 || col >= _gridCols || row < 0 || row >= _gridRows)
		return -1;
	return col + _gridCols * row;
}

void PuzzleWallOfFleens::fireCannon() {
	// Set up cannonball trajectory from muzzle to target fleen center
	int angle = CLIP(_cannonAngle, 0, kNumCannonAngles - 1);
	_cannonballStartPos = kCannonMuzzlePos[angle];

	// Target: center of the fleen cell
	_cannonballEndPos = Common::Point32(
		_gridOrigin.x + kCellSize.width * _targetCol + kCellSize.width / 2,
		_gridOrigin.y + kCellSize.height * _targetRow + kCellSize.height / 2);

	_cannonballPos = _cannonballStartPos;
	_cannonballProgress = 0;

	_gameState = kStateFiring02;
	_actionTimer = _vm->getGameTickCount();

	debug(2, "WallOfFleens: Firing cannon angle %d from (%d,%d) to (%d,%d)",
		  _cannonAngle, _cannonballStartPos.x, _cannonballStartPos.y,
		  _cannonballEndPos.x, _cannonballEndPos.y);
}

void PuzzleWallOfFleens::catchFleen(int fleenIdx) {
	_fleens[fleenIdx].caught = true;
	_freedCount++;
	_selectedFleen = fleenIdx;
	_gameState = kStateHit03;
	_actionTimer = _vm->getGameTickCount();

	debug(2, "WallOfFleens: Caught fleen %d (col=%d row=%d), freed %d/%d",
		  fleenIdx, _fleens[fleenIdx].gridCol, _fleens[fleenIdx].gridRow,
		  _freedCount, kMinFreed);
}

void PuzzleWallOfFleens::missFleen() {
	_mirrorsLeft--;
	_gameState = kStateMiss04;
	_actionTimer = _vm->getGameTickCount();

	debug(2, "WallOfFleens: Miss! Mirrors remaining: %d/%d",
		  _mirrorsLeft, _mirrorsTotal);
}

void PuzzleWallOfFleens::advanceToNextZoombini() {
	_currentZoombini += 1;
	_selectedFleen = -1;
	_cannonballProgress = 0;

	// Reset cannon back to center
	_cannonAngle = 4;
	_targetAngle = 4;

	checkCompletion();
	if (_gameState != kStateDone06) {
		_gameState = kStateNextZoombini05;
		_actionTimer = _vm->getGameTickCount();
	}
}

void PuzzleWallOfFleens::checkCompletion() {
	// Complete when the required number of Zoombinis has been released.
	if (_freedCount >= kMinFreed) {
		debug(1, "WallOfFleens: Puzzle complete! Freed %d zoombinis", _freedCount);
		_gameState = kStateDone06;
		_actionTimer = _vm->getGameTickCount();
		return;
	}

	// A depleted mirror allowance ends the round and advances the route.
	if (_mirrorsLeft <= 0) {
		debug(1, "WallOfFleens: No mirrors left, transitioning out");
		_gameState = kStateDone06;
		_actionTimer = _vm->getGameTickCount();
		return;
	}

	// No more zoombinis to try
	if (_currentZoombini >= _puzzleZoombinis.size()) {
		debug(1, "WallOfFleens: No more zoombinis, transitioning out");
		_gameState = kStateDone06;
		_actionTimer = _vm->getGameTickCount();
	}
}

void PuzzleWallOfFleens::applyDebugPuzzleCompletion() {
	_freedCount = MAX(_freedCount, kMinFreed);
	if (_gameState != kStateDone06) {
		_gameState = kStateDone06;
		_actionTimer = _vm->getGameTickCount();
	}
}

// ============================================================================
// Click handling
// ============================================================================

EventHandleResult PuzzleWallOfFleens::onLButtonDown(const Common::Point &pos) {
	if (_gameState != kStateIdle00)
		return EventHandleResult::kPassthrough;

	if (_puzzleZoombinis.size() <= _currentZoombini)
		return EventHandleResult::kPassthrough;

	// Check if player clicked on an uncaught fleen cell
	for (int i = 0; i < _numFleens; i++) {
		if (_fleens[i].caught)
			continue;
		if (_fleens[i].hitbox.contains(pos)) {
			_selectedFleen = i;
			_targetCol = _fleens[i].gridCol;
			_targetRow = _fleens[i].gridRow;

			// Compute what angle the cannon should aim at
			Common::Point32 cellCenterPos(
				_gridOrigin.x + kCellSize.width * _targetCol + kCellSize.width / 2,
				_gridOrigin.y + kCellSize.height * _targetRow + kCellSize.height / 2);
			_targetAngle = computeCannonAngle(cellCenterPos);

			if (_cannonAngle == _targetAngle) {
				// Fire immediately when the cannon is already aimed.
				fireCannon();
			} else {
				// Start rotating cannon
				_gameState = kStateAiming01;
				_actionTimer = _vm->getGameTickCount();
			}

			debug(2, "WallOfFleens: Clicked fleen %d at col=%d row=%d, "
					 "target angle=%d, current=%d",
				  i, _targetCol, _targetRow, _targetAngle, _cannonAngle);
			return EventHandleResult::kConsumed;
		}
	}
	return EventHandleResult::kPassthrough;
}

// ============================================================================
// Draw
// ============================================================================

void PuzzleWallOfFleens::onRenderContent(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);

	// Draw lava bubble decoration (lower left area)
	if (_lavaBubbleAnim) {
		uint32 now = _vm->getGameTickCount();
		int frameCount = _lavaBubbleAnim->getFrameCount();
		if (0 < frameCount) {
			int frameIdx = (now / 100) % frameCount; // ~10 fps
			_vm->_gfx->drawAnimationFrame(screen, _lavaBubbleAnim, frameIdx, Common::Point32(100, 450));
		}
	}

	drawGrid(screen);
	drawMirrors(screen);
	drawCannon(screen);
	drawCannonball(screen);
}

void PuzzleWallOfFleens::onRenderForeground(ManagedSurface32 *screen) {
	// Draw level indicator for level 1 cycling panels.
	if (_level == 1 && _gridPage < kNumLevelIndicators && _levelRedImage[_gridPage]) {
		_vm->_gfx->drawRleBlock(screen, _levelRedImage[_gridPage], Common::Point32(10, 10));
	}
}

void PuzzleWallOfFleens::drawGrid(ManagedSurface32 *screen) {
	for (int i = 0; i < _numFleens; i++) {
		const FleenCell &cell = _fleens[i];
		Common::Point32 cellPos(cell.hitbox.left, cell.hitbox.top);

		if (cell.caught) {
			// Draw empty/destroyed slot
			_vm->_gfx->drawRleBlock(screen, _mirrorImage[kMirrorEmpty05], cellPos);
			continue;
		}

		// Draw active fleen cell background
		_vm->_gfx->drawRleBlock(screen, _mirrorImage[kMirrorNormal00], cellPos);

		// Draw fleen zoombini sprite on the cell
		if (_zoombiniAnimation) {
			Common::Point32 zoombiniPos(cellPos.x + 4, cellPos.y + 4);
			_vm->_gfx->drawZoombini(screen, _zoombiniAnimation, cell.traits, zoombiniPos, 0, 0);
		}

		// Highlight the currently selected/targeted fleen
		if (i == _selectedFleen && (_gameState == kStateAiming01 || _gameState == kStateFiring02)) {
			_vm->_gfx->drawRleBlock(screen, _tuyereImage, cellPos);
		}

		// Show hit result on caught fleen
		if (i == _selectedFleen && _gameState == kStateHit03) {
			if (_highlightImage) {
				_vm->_gfx->drawRleBlock(screen, _highlightImage, cellPos);
			} else if (_mirrorImage[kMirrorExplode04]) {
				_vm->_gfx->drawRleBlock(screen, _mirrorImage[kMirrorExplode04], cellPos);
			}
		}
	}
}

void PuzzleWallOfFleens::drawCannon(ManagedSurface32 *screen) {
	// Draw the cannon visual for its current angle.
	int angle = CLIP(_cannonAngle, 0, kNumCannonAngles - 1);
	_vm->_gfx->drawRleBlock(screen, _cannonImage[angle], kCannonDrawPos);

	// Draw cannon cache/cover overlay
	_vm->_gfx->drawRleBlock(screen, _cannonCache, kCannonDrawPos);
}

void PuzzleWallOfFleens::drawCannonball(ManagedSurface32 *screen) {
	if (_gameState != kStateFiring02 || _cannonballProgress <= 0)
		return;

	// Draw a simple cannonball at current interpolated position
	// Use tuyere sprite as cannonball placeholder if no dedicated sprite
	if (_tuyereImage) {
		_vm->_gfx->drawRleBlock(screen, _tuyereImage, Common::Point32(_cannonballPos.x - 8, _cannonballPos.y - 8));
	} else {
		// Fallback: draw a small rectangle
		const Common::Rect32 cannonballRect(_cannonballPos.x - 4, _cannonballPos.y - 4, _cannonballPos.x + 4, _cannonballPos.y + 4);
		_vm->_gfx->fillRect(screen, cannonballRect, 0);
	}
}

void PuzzleWallOfFleens::drawMirrors(ManagedSurface32 *screen) {
	// Draw remaining-chance mirrors along the lower-right edge.
	for (int i = 0; i < _mirrorsTotal; i++) {
		Common::Point32 mirrorPos(690 + 23 * i, 424);

		// Show explode animation on the most recently broken mirror
		if (i == _mirrorsLeft && _gameState == kStateMiss04 && _mirrorExplodeAnim) {
			uint32 elapsed = _vm->getGameTickCount() - _actionTimer;
			int frameCount = _mirrorExplodeAnim->getFrameCount();
			if (0 < frameCount && elapsed < static_cast<uint32>(frameCount * 60)) {
				int frameIdx = (elapsed / 60) % frameCount; // ~16.7 fps
				if (_mirrorExplodeAnim->getFrame(frameIdx)) {
					_vm->_gfx->drawAnimationFrame(screen, _mirrorExplodeAnim, frameIdx, mirrorPos);
					continue; // Skip normal drawing for this mirror
				}
			}
		}

		if (i < _mirrorsLeft) {
			_vm->_gfx->drawRleBlock(screen, _mirrorImage[kMirrorNormal00], mirrorPos);
		} else {
			_vm->_gfx->drawRleBlock(screen, _mirrorImage[kMirrorEmpty05], mirrorPos);
		}
	}
}

void PuzzleWallOfFleens::onRenderActors(ManagedSurface32 *screen) {
	if (!_zoombiniAnimation || _puzzleZoombinis.empty())
		return;

	// Draw the current Zoombini beside the cannon.
	if (_currentZoombini < _puzzleZoombinis.size()) {
		const ZoombiniRunner *z = _puzzleZoombinis[_currentZoombini];
		Common::Point32 zoombiniPos(kCannonDrawPos.x - 60, kCannonDrawPos.y + 10);
		_vm->_gfx->drawZoombini(screen, _zoombiniAnimation, z->_traits, zoombiniPos, 0, 0);
	}

	// Draw remaining zoombinis in a queue line
	const Common::Point32 queueStartPos(50, 550);
	int spacing = 35;
	const uint count = MIN<uint>(_puzzleZoombinis.size(), 16);
	for (uint i = _currentZoombini + 1; i < count; i++) {
		const ZoombiniRunner *z = _puzzleZoombinis[i];
		const int queueOffset = static_cast<int>(i - _currentZoombini - 1) * spacing;
		Common::Point32 zoombiniPos(queueStartPos.x + queueOffset, queueStartPos.y);
		_vm->_gfx->drawZoombini(screen, _zoombiniAnimation, z->_traits, zoombiniPos, 0, 0);
	}
}

} // End of namespace Zoombini2
