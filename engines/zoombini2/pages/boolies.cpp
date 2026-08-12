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

#include "zoombini2/pages/boolies.h"
#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"
#include "zoombini2/sound.h"

namespace Zoombini2 {

// ============================================================================
// BooliesPuzzle — Bowling puzzle.
//
// Original: Boolies__Init_406F40, Boolies__CheckFreeZoombinis_40DED0.
//
// Core mechanics:
//   - Zoombinis roll as bowling balls (positive or negative)
//   - 5 launch spots (spot01-05) to select starting position
//   - Pins arranged in a bowling formation
//   - Knock down pins to free zoombinis
//   - Blockers may obstruct some paths
//   - Freed zoombinis board the boat (bateau) to escape
//   - Goal: Free at least 4 zoombinis
//
// Path system:
//   - b_boolies1-4.pat: Main rolling paths per lane
//   - b_boolies*_exit.pat: Exit paths after pin collision
//   - jump0-2.pat: Jump paths for special moves
//
// Original object layout (0x4D8 bytes):
//   - Uses PathObject for ball movement curves
//   - AnimElement callbacks for pin collision detection
// ============================================================================

// Ball roll animation duration (ms)
static const uint32 kBallRollDuration = 1500;

// Pin knocked animation duration (ms)
static const uint32 kPinKnockDuration = 500;

// Boat departure delay (ms)
static const uint32 kBoatDepartDelay = 2000;

// Spot positions (approximate, based on typical bowling layout)
static const int kSpotPositions[5][2] = {
	{ 100, 500 },   // spot01
	{ 200, 500 },   // spot02
	{ 300, 500 },   // spot03 (center)
	{ 400, 500 },   // spot04
	{ 500, 500 }    // spot05
};

// Max attempts per zoombini, indexed by difficulty (0 unused).
// Original: Boolies__Init_406F40 — this+96 = kAttemptsPerDifficulty[diff]
static const int kAttemptsPerDifficulty[] = { 0, 2, 3, 4 };

// Spot hitbox size
static const int kSpotHitSize = 50;

// Pin grid layout (3 rows of pins)
static const int kPinRows = 3;
static const int kPinCols[] = { 3, 4, 5 };  // Pins per row
static const int kPinBaseY = 150;
static const int kPinRowSpacing = 60;
static const int kPinColSpacing = 50;
static const int kPinBaseX = 200;

// Boat position
static const int kBoatBaseX = 550;
static const int kBoatBaseY = 100;

BooliesPuzzle::BooliesPuzzle(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageBoolies),
	  _state(kStateInit),
	  _currentSpot(-1),
	  _freedCount(0),
	  _pinsKnocked(0),
	  _boatX(kBoatBaseX),
	  _boatY(kBoatBaseY),
	  _boatVisible(true),
	  _ballPosGfx(nullptr),
	  _ballNegGfx(nullptr),
	  _pinGfx(nullptr),
	  _pinLightedGfx(nullptr),
	  _boatGfx(nullptr),
	  _blockerGfx(nullptr),
	  _fixeGfx(nullptr),
	  _fixe2Gfx(nullptr),
	  _marcheAnim(nullptr),
	  _marche2Anim(nullptr),
	  _attendAnim(nullptr),
	  _attend2Anim(nullptr),
	  _rollAnim(nullptr),
	  _roll2Anim(nullptr),
	  _maxAttempts(0),
	  _blockerAnim(nullptr),
	  _musicId(-1) {

	for (int i = 0; i < 5; i++) {
		_spotGfx[i] = nullptr;
		_spots[i].active = false;
	}

	_activeBall.type = kBallNone;
	_activeBall.zoombiniIdx = -1;
}

BooliesPuzzle::~BooliesPuzzle() {
	if (_musicId >= 0) {
		SoundManager *snd = _engine->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	delete _ballPosGfx;
	delete _ballNegGfx;
	delete _pinGfx;
	delete _pinLightedGfx;
	delete _boatGfx;
	delete _blockerGfx;
	delete _fixeGfx;
	delete _fixe2Gfx;
	delete _marcheAnim;
	delete _marche2Anim;
	delete _attendAnim;
	delete _attend2Anim;
	delete _rollAnim;
	delete _roll2Anim;
	delete _blockerAnim;
	for (int i = 0; i < 5; i++) {
		delete _spotGfx[i];
	}
}

void BooliesPuzzle::init() {
	// Call base init for background and zoombini loading
	PuzzlePage::init();

	// BGM: 09-BB01.wav (IDA: Boolies__Init_406F40)
	if (SoundManager *snd = _engine->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("sounds/music/09-BB01.wav"), true);
		if (_musicId >= 0) { snd->playLoop(_musicId); snd->setVolume(_musicId, snd->_volumeMusic); }
	}

	int diff = CLIP(_engine->getGameState()->_gameMode, 1, 3);

	debug(1, "BooliesPuzzle::init — difficulty %d, maxAttempts %d", diff, _maxAttempts);

	// Load resources
	loadResources();

	// Setup launch spots
	setupSpots();

	// Setup pins
	setupPins();

	// Assign zoombinis as balls
	assignZoombinis();

	_freedCount = 0;
	_pinsKnocked = 0;
	_currentSpot = -1;
	_state = kStateIdle;
	_stateTimer = _engine->getGameTickCount();
}

void BooliesPuzzle::loadResources() {
	// Load ball sprites
	Common::Path ballPosPath("bmp/boolies/ball_pos");
	_ballPosGfx = new RleBlock();
	if (!_ballPosGfx->loadFromFile(ballPosPath)) {
		delete _ballPosGfx;
		_ballPosGfx = nullptr;
	}

	Common::Path ballNegPath("bmp/boolies/ball_neg");
	_ballNegGfx = new RleBlock();
	if (!_ballNegGfx->loadFromFile(ballNegPath)) {
		delete _ballNegGfx;
		_ballNegGfx = nullptr;
	}

	// Load pin sprites
	Common::Path pinPath("bmp/boolies/pin");
	_pinGfx = new RleBlock();
	if (!_pinGfx->loadFromFile(pinPath)) {
		delete _pinGfx;
		_pinGfx = nullptr;
	}

	Common::Path pinLightedPath("bmp/boolies/pin_lighted");
	_pinLightedGfx = new RleBlock();
	if (!_pinLightedGfx->loadFromFile(pinLightedPath)) {
		delete _pinLightedGfx;
		_pinLightedGfx = nullptr;
	}

	// Load boat sprite
	Common::Path boatPath("bmp/boolies/bateau");
	_boatGfx = new RleBlock();
	if (!_boatGfx->loadFromFile(boatPath)) {
		delete _boatGfx;
		_boatGfx = nullptr;
	}

	// Load spot sprites
	for (int i = 0; i < 5; i++) {
		Common::Path spotPath(Common::String::format("bmp/boolies/spot%02d", i + 1));
		_spotGfx[i] = new RleBlock();
		if (!_spotGfx[i]->loadFromFile(spotPath)) {
			delete _spotGfx[i];
			_spotGfx[i] = nullptr;
		}
	}

	// Load blocker sprite
	Common::Path blockerPath("bmp/boolies/blocker");
	_blockerGfx = new RleBlock();
	if (!_blockerGfx->loadFromFile(blockerPath)) {
		delete _blockerGfx;
		_blockerGfx = nullptr;
	}

	// Load blocker animation
	_blockerAnim = new Animation();
	if (!_blockerAnim->loadFromFile(blockerPath)) {
		delete _blockerAnim;
		_blockerAnim = nullptr;
	}

	// Load fixed position sprites
	Common::Path fixePath("bmp/boolies/fixe");
	_fixeGfx = new RleBlock();
	if (!_fixeGfx->loadFromFile(fixePath)) {
		delete _fixeGfx;
		_fixeGfx = nullptr;
	}

	Common::Path fixe2Path("bmp/boolies/fixe2");
	_fixe2Gfx = new RleBlock();
	if (!_fixe2Gfx->loadFromFile(fixe2Path)) {
		delete _fixe2Gfx;
		_fixe2Gfx = nullptr;
	}

	// Load animations
	Common::Path marchePath("bmp/boolies/marche");
	_marcheAnim = new Animation();
	if (!_marcheAnim->loadFromFile(marchePath)) {
		delete _marcheAnim;
		_marcheAnim = nullptr;
	}

	Common::Path marche2Path("bmp/boolies/marche2");
	_marche2Anim = new Animation();
	if (!_marche2Anim->loadFromFile(marche2Path)) {
		delete _marche2Anim;
		_marche2Anim = nullptr;
	}

	Common::Path attendPath("bmp/boolies/attend");
	_attendAnim = new Animation();
	if (!_attendAnim->loadFromFile(attendPath)) {
		delete _attendAnim;
		_attendAnim = nullptr;
	}

	Common::Path attend2Path("bmp/boolies/attend2");
	_attend2Anim = new Animation();
	if (!_attend2Anim->loadFromFile(attend2Path)) {
		delete _attend2Anim;
		_attend2Anim = nullptr;
	}

	Common::Path rollPath("bmp/boolies/roll");
	_rollAnim = new Animation();
	if (!_rollAnim->loadFromFile(rollPath)) {
		delete _rollAnim;
		_rollAnim = nullptr;
	}

	Common::Path roll2Path("bmp/boolies/roll2");
	_roll2Anim = new Animation();
	if (!_roll2Anim->loadFromFile(roll2Path)) {
		delete _roll2Anim;
		_roll2Anim = nullptr;
	}

	debug(2, "BooliesPuzzle: Resources loaded");
}

void BooliesPuzzle::setupSpots() {
	// Setup launch spots with hitboxes
	for (int i = 0; i < 5; i++) {
		_spots[i].x = kSpotPositions[i][0];
		_spots[i].y = kSpotPositions[i][1];
		_spots[i].hitbox = Common::Rect(
			_spots[i].x - kSpotHitSize / 2,
			_spots[i].y - kSpotHitSize / 2,
			_spots[i].x + kSpotHitSize / 2,
			_spots[i].y + kSpotHitSize / 2
		);
		_spots[i].active = true;
	}

	debug(2, "BooliesPuzzle: Setup %d launch spots", 5);
}

void BooliesPuzzle::setupPins() {
	// Setup pins in bowling triangle formation
	_pins.clear();

	for (int row = 0; row < kPinRows; row++) {
		int numPins = kPinCols[row];
		int rowY = kPinBaseY + row * kPinRowSpacing;

		// Center the row
		int rowStartX = kPinBaseX + (kPinCols[kPinRows - 1] - numPins) * kPinColSpacing / 2;

		for (int col = 0; col < numPins; col++) {
			Pin pin;
			pin.x = rowStartX + col * kPinColSpacing;
			pin.y = rowY;
			pin.knocked = false;
			pin.lighted = false;
			_pins.push_back(pin);
		}
	}

	debug(2, "BooliesPuzzle: Setup %d pins", (int)_pins.size());
}

void BooliesPuzzle::assignZoombinis() {
	// Assign zoombinis to be rolled as balls
	// Each zoombini gets a ball type (positive or negative) based on features

	Common::RandomSource rnd("boolies");

	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		// Alternate ball types or use zoombini features
		// Using simple alternation for now
		// Original likely uses feature matching
	}

	debug(2, "BooliesPuzzle: Assigned %d zoombinis as balls", (int)_puzzleZoombinis.size());
}

void BooliesPuzzle::launchBall(int spotIdx) {
	if (spotIdx < 0 || spotIdx >= 5)
		return;

	if (!_spots[spotIdx].active)
		return;

	// Find next available zoombini
	int zoomIdx = -1;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_freeStatus != 0) {  // Not yet freed
			zoomIdx = i;
			break;
		}
	}

	if (zoomIdx < 0) {
		debug(2, "BooliesPuzzle: No zoombinis available to launch");
		return;
	}

	// Setup ball
	_activeBall.type = (zoomIdx % 2 == 0) ? kBallPositive : kBallNegative;
	_activeBall.zoombiniIdx = zoomIdx;
	_activeBall.startX = _spots[spotIdx].x;
	_activeBall.startY = _spots[spotIdx].y;
	_activeBall.x = _activeBall.startX;
	_activeBall.y = _activeBall.startY;

	// Target is first row of pins (center)
	_activeBall.endX = kPinBaseX + kPinCols[kPinRows - 1] * kPinColSpacing / 2;
	_activeBall.endY = kPinBaseY;

	_activeBall.rollStart = _engine->getGameTickCount();
	_currentSpot = spotIdx;
	_state = kStateBallRolling;

	debug(2, "BooliesPuzzle: Launched ball from spot %d (zoombini %d)", spotIdx, zoomIdx);
}

void BooliesPuzzle::advanceBallRoll() {
	if (_activeBall.type == kBallNone)
		return;

	uint32 elapsed = _engine->getGameTickCount() - _activeBall.rollStart;
	float progress = (float)elapsed / kBallRollDuration;

	if (progress >= 1.0f) {
		// Ball reached target
		_activeBall.x = _activeBall.endX;
		_activeBall.y = _activeBall.endY;

		// Check for pin collision
		if (checkPinCollision()) {
			knockDownPins();
		}

		return;
	}

	// Linear interpolation for ball position
	_activeBall.x = _activeBall.startX + (int)((_activeBall.endX - _activeBall.startX) * progress);
	_activeBall.y = _activeBall.startY + (int)((_activeBall.endY - _activeBall.startY) * progress);
}

bool BooliesPuzzle::checkPinCollision() {
	// Check if ball collides with any standing pin
	for (uint i = 0; i < _pins.size(); i++) {
		if (_pins[i].knocked)
			continue;

		// Simple distance check
		int dx = _activeBall.x - _pins[i].x;
		int dy = _activeBall.y - _pins[i].y;
		int distSq = dx * dx + dy * dy;

		if (distSq < 30 * 30) {  // Within 30 pixels
			return true;
		}
	}

	return false;
}

void BooliesPuzzle::knockDownPins() {
	// Knock down pins near the ball
	int knocked = 0;

	for (uint i = 0; i < _pins.size(); i++) {
		if (_pins[i].knocked)
			continue;

		int dx = _activeBall.x - _pins[i].x;
		int dy = _activeBall.y - _pins[i].y;
		int distSq = dx * dx + dy * dy;

		// Ball knocks down pins within range
		// Larger range for positive balls, smaller for negative
		int knockRange = (_activeBall.type == kBallPositive) ? 50 : 35;

		if (distSq < knockRange * knockRange) {
			_pins[i].knocked = true;
			knocked++;
			_pinsKnocked++;
		}
	}

	debug(2, "BooliesPuzzle: Knocked down %d pins (total: %d)", knocked, _pinsKnocked);

	// Free the zoombini who was the ball
	if (knocked > 0) {
		freeZoombini(_activeBall.zoombiniIdx);
		_state = kStatePinsKnocked;
	} else {
		// Miss - return to idle
		_activeBall.type = kBallNone;
		_activeBall.zoombiniIdx = -1;
		_state = kStateIdle;
	}

	_stateTimer = _engine->getGameTickCount();
}

void BooliesPuzzle::freeZoombini(int zoombiniIdx) {
	if (zoombiniIdx < 0 || zoombiniIdx >= (int)_puzzleZoombinis.size())
		return;

	// Mark zoombini as free (byte+92 = 0 in original)
	_puzzleZoombinis[zoombiniIdx]->_freeStatus = 0;
	_freedCount++;

	debug(1, "BooliesPuzzle: Freed zoombini %d (total freed: %d)", zoombiniIdx, _freedCount);
}

int BooliesPuzzle::countFreeZoombinis() const {
	// Original: Boolies__CheckFreeZoombinis_40DED0
	int count = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_freeStatus == 0)
			count++;
	}
	return count;
}

void BooliesPuzzle::update() {
	uint32 now = _engine->getGameTickCount();
	uint32 elapsed = now - _stateTimer;

	switch (_state) {
	case kStateInit:
		// Should not happen after init()
		break;

	case kStateIdle:
		// Waiting for player click - handled in handleClick()
		break;

	case kStateBallRolling:
		// Animate ball movement
		advanceBallRoll();
		break;

	case kStatePinsKnocked:
		// Brief pause after knocking pins
		if (elapsed > kPinKnockDuration) {
			// Check win condition
			if (_freedCount >= 4) {
				_state = kStateZoombiniFreed;
				_stateTimer = now;
			} else {
				// More zoombinis needed, reset for next turn
				_activeBall.type = kBallNone;
				_activeBall.zoombiniIdx = -1;
				_state = kStateIdle;
			}
		}
		break;

	case kStateZoombiniFreed:
		// Show zoombini moving to boat
		if (elapsed > 1000) {
			_state = kStateBoatLeaving;
			_stateTimer = now;
		}
		break;

	case kStateBoatLeaving:
		// Boat departing animation
		if (elapsed > kBoatDepartDelay) {
			debug(1, "BooliesPuzzle: Complete, %d zoombinis freed", _freedCount);
			_state = kStateDone;
			_stateTimer = now;
		}
		break;

	case kStateDone:
		// Wait before transitioning out
		if (elapsed > 1000) {
			debug(1, "BooliesPuzzle: Returning to map");
			_engine->_returningFromPuzzle = true;
			_engine->_maptransSourceWorld = kPageBoolies;
			_engine->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void BooliesPuzzle::draw(Graphics::ManagedSurface *screen) {
	// Draw background
	if (_background)
		_background->drawToSurface(screen, 0, 0);

	// Draw game elements
	drawPins(screen);
	drawSpots(screen);
	drawBlockers(screen);
	drawBall(screen);
	drawBoat(screen);
	drawZoombinis(screen);
}

void BooliesPuzzle::drawSpots(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (int i = 0; i < 5; i++) {
		if (!_spots[i].active)
			continue;

		RleBlock *gfx = _spotGfx[i];
		if (gfx) {
			gfx->drawToScreen(screen, _spots[i].x - 25, _spots[i].y - 25, lut);
		} else {
			// Fallback: draw circle
			screen->fillRect(Common::Rect(
				_spots[i].x - 20, _spots[i].y - 20,
				_spots[i].x + 20, _spots[i].y + 20
			), 0x00FFFF);
		}
	}
}

void BooliesPuzzle::drawPins(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (uint i = 0; i < _pins.size(); i++) {
		const Pin &pin = _pins[i];

		if (pin.knocked)
			continue;  // Don't draw knocked pins

		RleBlock *gfx = pin.lighted ? _pinLightedGfx : _pinGfx;
		if (gfx) {
			gfx->drawToScreen(screen, pin.x - 10, pin.y - 20, lut);
		} else {
			// Fallback: draw triangle
			uint32 color = pin.lighted ? 0xFFFF00 : 0xFFFFFF;
			screen->drawLine(pin.x, pin.y - 20, pin.x - 10, pin.y, color);
			screen->drawLine(pin.x, pin.y - 20, pin.x + 10, pin.y, color);
			screen->drawLine(pin.x - 10, pin.y, pin.x + 10, pin.y, color);
		}
	}
}

void BooliesPuzzle::drawBall(Graphics::ManagedSurface *screen) {
	if (_activeBall.type == kBallNone)
		return;

	const byte (*lut)[256] = _engine->getAlphaLUT();

	RleBlock *gfx = (_activeBall.type == kBallPositive) ? _ballPosGfx : _ballNegGfx;
	if (gfx) {
		gfx->drawToScreen(screen, _activeBall.x - 15, _activeBall.y - 15, lut);
	} else {
		// Fallback: draw circle
		uint32 color = (_activeBall.type == kBallPositive) ? 0x00FF00 : 0xFF0000;
		screen->fillRect(Common::Rect(
			_activeBall.x - 15, _activeBall.y - 15,
			_activeBall.x + 15, _activeBall.y + 15
		), color);
	}
}

void BooliesPuzzle::drawBoat(Graphics::ManagedSurface *screen) {
	if (!_boatVisible)
		return;

	const byte (*lut)[256] = _engine->getAlphaLUT();

	if (_boatGfx) {
		_boatGfx->drawToScreen(screen, _boatX, _boatY, lut);
	} else {
		// Fallback: draw simple boat shape
		screen->fillRect(Common::Rect(_boatX, _boatY + 20, _boatX + 80, _boatY + 40), 0x8B4513);
		screen->fillRect(Common::Rect(_boatX + 30, _boatY, _boatX + 50, _boatY + 30), 0xFFFFFF);
	}
}

void BooliesPuzzle::drawBlockers(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();
	uint32 now = _engine->getGameTickCount();

	for (uint i = 0; i < _blockers.size(); i++) {
		const Common::Point &pos = _blockers[i];

		// Prefer animated blocker if available
		if (_blockerAnim) {
			int frameCount = _blockerAnim->getFrameCount();
			if (frameCount > 0) {
				int frameIdx = (now / 120) % frameCount;  // ~8 fps animation
				const RleBlock *frame = _blockerAnim->getFrame(frameIdx);
				if (frame)
					frame->drawToScreen(screen, pos.x, pos.y, lut);
			}
		} else if (_blockerGfx) {
			_blockerGfx->drawToScreen(screen, pos.x, pos.y, lut);
		} else {
			// Fallback: draw rectangle
			screen->fillRect(Common::Rect(pos.x, pos.y, pos.x + 30, pos.y + 60), 0x800000);
		}
	}
}

void BooliesPuzzle::drawZoombinis(Graphics::ManagedSurface *screen) {
	// Draw zoombinis waiting on the boat (freed ones)
	const byte (*lut)[256] = _engine->getAlphaLUT();

	if (!_zoombiniGfx)
		return;

	int freeIdx = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_freeStatus == 0) {
			// This zoombini is free - draw on boat
			const Zoombini *z = _puzzleZoombinis[i];
			int x = _boatX + 10 + (freeIdx % 4) * 18;
			int y = _boatY + 10 + (freeIdx / 4) * 20;

			// Draw zoombini body
			int baseIdx = 0;
			const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
			if (frame)
				frame->drawToScreen(screen, x, y, lut);

			// Features
			const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
			for (int feat = 1; feat <= 4; feat++) {
				int featIdx = baseIdx + feat * ZoombiniGfx::kDim2 + features[feat - 1];
				frame = _zoombiniGfx->getFrame(featIdx, 0);
				if (frame)
					frame->drawToScreen(screen, x, y, lut);
			}

			freeIdx++;
		}
	}
}

void BooliesPuzzle::handleClick(const Common::Point &pos) {
	if (_state != kStateIdle)
		return;

	// Check which spot was clicked
	for (int i = 0; i < 5; i++) {
		if (_spots[i].active && _spots[i].hitbox.contains(pos)) {
			debug(2, "BooliesPuzzle: Clicked spot %d", i);
			launchBall(i);
			return;
		}
	}

	debug(2, "BooliesPuzzle: Click at %d,%d (no spot)", pos.x, pos.y);
}

} // End of namespace Zoombini2
