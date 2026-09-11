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

#include "zoombini2/pages/puzzle_boolies.h"
#include "common/debug.h"
#include "zoombini2/graphics.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// PuzzleBoolies - bowling puzzle.
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
// ============================================================================

// Ball roll animation duration (ms)
static const uint32 kBallRollDuration = 1500;

// Pin knocked animation duration (ms)
static const uint32 kPinKnockDuration = 500;

// Boat departure delay (ms)
static const uint32 kBoatDepartDelay = 2000;

// Spot positions (approximate, based on typical bowling layout).
static const Common::Point32 kSpotPos[5] = {
	Common::Point32(100, 500), // spot01
	Common::Point32(200, 500), // spot02
	Common::Point32(300, 500), // spot03 (center)
	Common::Point32(400, 500), // spot04
	Common::Point32(500, 500)  // spot05
};

// Spot hitbox size
static const int kSpotHitSize = 50;

// Pin grid layout (3 rows of pins)
static const int kPinRows = 3;
static const int kPinCols[] = {3, 4, 5}; // Pins per row
static const int kPinRowSpacing = 60;
static const int kPinColSpacing = 50;
static const Common::Point32 kPinBasePos(200, 150);

PuzzleBoolies::PuzzleBoolies(Zoombini2Engine *vm)
	: PuzzleBase(vm, kPageBoolies) {
}

PuzzleBoolies::~PuzzleBoolies() {
	if (_musicId >= 0) {
		SoundManager *snd = _vm->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	delete _ballPosImage;
	delete _ballNegImage;
	delete _pinImage;
	delete _pinLightedImage;
	delete _boatImage;
	delete _blockerImage;
	delete _fixeImage;
	delete _fixe2Image;
	delete _marcheAnim;
	delete _marche2Anim;
	delete _attendAnim;
	delete _attend2Anim;
	delete _rollAnim;
	delete _roll2Anim;
	delete _blockerAnim;
	for (int i = 0; i < 5; i++) {
		delete _spotImage[i];
	}
}

void PuzzleBoolies::init() {
	// Call base init for background and zoombini loading
	PuzzleBase::init();

	// Start the Boolie Boggle music.
	if (SoundManager *snd = _vm->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("#sounds/music/09-BB01.wav"), true);
		if (_musicId >= 0) {
			snd->playLoop(_musicId);
			snd->setVolume(_musicId, snd->_volumeMusic);
		}
	}

	const int level = _vm->getGameState()->getLevel();
	const int rescuedBooliesPerZoombini = getRescuedBooliesPerZoombini(level);
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i])
			_puzzleZoombinis[i]->_rescuedBooliesPerZoombini = rescuedBooliesPerZoombini;
	}
	debug(1, "PuzzleBoolies::init - level %d, rescued Boolies per Zoombini %d", level, rescuedBooliesPerZoombini);

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
	_stateTimer = _vm->getGameTickCount();
}

void PuzzleBoolies::loadResources() {
	// Load ball sprites
	Common::Path ballPosPath("bmp/boolies/ball_pos");
	_ballPosImage = new RleBlock(_vm);
	if (!_ballPosImage->loadFromFile(ballPosPath)) {
		delete _ballPosImage;
		_ballPosImage = nullptr;
	}

	Common::Path ballNegPath("bmp/boolies/ball_neg");
	_ballNegImage = new RleBlock(_vm);
	if (!_ballNegImage->loadFromFile(ballNegPath)) {
		delete _ballNegImage;
		_ballNegImage = nullptr;
	}

	// Load pin sprites
	Common::Path pinPath("bmp/boolies/pin");
	_pinImage = new RleBlock(_vm);
	if (!_pinImage->loadFromFile(pinPath)) {
		delete _pinImage;
		_pinImage = nullptr;
	}

	Common::Path pinLightedPath("bmp/boolies/pin_lighted");
	_pinLightedImage = new RleBlock(_vm);
	if (!_pinLightedImage->loadFromFile(pinLightedPath)) {
		delete _pinLightedImage;
		_pinLightedImage = nullptr;
	}

	// Load boat sprite
	Common::Path boatPath("bmp/boolies/bateau");
	_boatImage = new RleBlock(_vm);
	if (!_boatImage->loadFromFile(boatPath)) {
		delete _boatImage;
		_boatImage = nullptr;
	}

	// Load spot sprites
	for (int i = 0; i < 5; i++) {
		Common::Path spotPath(Common::String::format("bmp/boolies/spot%02d", i + 1));
		_spotImage[i] = new RleBlock(_vm);
		if (!_spotImage[i]->loadFromFile(spotPath)) {
			delete _spotImage[i];
			_spotImage[i] = nullptr;
		}
	}

	// Load blocker sprite
	Common::Path blockerPath("bmp/boolies/blocker");
	_blockerImage = new RleBlock(_vm);
	if (!_blockerImage->loadFromFile(blockerPath)) {
		delete _blockerImage;
		_blockerImage = nullptr;
	}

	// Load blocker animation
	_blockerAnim = new Animation(_vm);
	if (!_blockerAnim->loadFromFile(blockerPath)) {
		delete _blockerAnim;
		_blockerAnim = nullptr;
	}

	// Load fixed position sprites
	Common::Path fixePath("bmp/boolies/fixe");
	_fixeImage = new RleBlock(_vm);
	if (!_fixeImage->loadFromFile(fixePath)) {
		delete _fixeImage;
		_fixeImage = nullptr;
	}

	Common::Path fixe2Path("bmp/boolies/fixe2");
	_fixe2Image = new RleBlock(_vm);
	if (!_fixe2Image->loadFromFile(fixe2Path)) {
		delete _fixe2Image;
		_fixe2Image = nullptr;
	}

	// Load animations
	Common::Path marchePath("bmp/boolies/marche");
	_marcheAnim = new Animation(_vm);
	if (!_marcheAnim->loadFromFile(marchePath)) {
		delete _marcheAnim;
		_marcheAnim = nullptr;
	}

	Common::Path marche2Path("bmp/boolies/marche2");
	_marche2Anim = new Animation(_vm);
	if (!_marche2Anim->loadFromFile(marche2Path)) {
		delete _marche2Anim;
		_marche2Anim = nullptr;
	}

	Common::Path attendPath("bmp/boolies/attend");
	_attendAnim = new Animation(_vm);
	if (!_attendAnim->loadFromFile(attendPath)) {
		delete _attendAnim;
		_attendAnim = nullptr;
	}

	Common::Path attend2Path("bmp/boolies/attend2");
	_attend2Anim = new Animation(_vm);
	if (!_attend2Anim->loadFromFile(attend2Path)) {
		delete _attend2Anim;
		_attend2Anim = nullptr;
	}

	Common::Path rollPath("bmp/boolies/roll");
	_rollAnim = new Animation(_vm);
	if (!_rollAnim->loadFromFile(rollPath)) {
		delete _rollAnim;
		_rollAnim = nullptr;
	}

	Common::Path roll2Path("bmp/boolies/roll2");
	_roll2Anim = new Animation(_vm);
	if (!_roll2Anim->loadFromFile(roll2Path)) {
		delete _roll2Anim;
		_roll2Anim = nullptr;
	}

	debug(2, "PuzzleBoolies: Resources loaded");
}

void PuzzleBoolies::setupSpots() {
	// Setup launch spots with hitboxes
	for (int i = 0; i < 5; i++) {
		_spots[i].pos = kSpotPos[i];
		_spots[i].hitbox = Common::Rect(
			static_cast<int16>(_spots[i].pos.x - kSpotHitSize / 2),
			static_cast<int16>(_spots[i].pos.y - kSpotHitSize / 2),
			static_cast<int16>(_spots[i].pos.x + kSpotHitSize / 2),
			static_cast<int16>(_spots[i].pos.y + kSpotHitSize / 2));
		_spots[i].active = true;
	}

	debug(2, "PuzzleBoolies: Setup %d launch spots", 5);
}

void PuzzleBoolies::setupPins() {
	// Setup pins in bowling triangle formation
	_pins.clear();

	for (int row = 0; row < kPinRows; row++) {
		int numPins = kPinCols[row];
		// Center the row.
		Common::Point32 rowStartPos(
			kPinBasePos.x + (kPinCols[kPinRows - 1] - numPins) * kPinColSpacing / 2,
			kPinBasePos.y + row * kPinRowSpacing);

		for (int col = 0; col < numPins; col++) {
			Pin pin;
			pin.pos = Common::Point32(rowStartPos.x + col * kPinColSpacing, rowStartPos.y);
			pin.knocked = false;
			pin.lighted = false;
			_pins.push_back(pin);
		}
	}

	debug(2, "PuzzleBoolies: Setup %d pins", (int)_pins.size());
}

void PuzzleBoolies::assignZoombinis() {
	// Ball selection consumes the base-page roster in its existing order.
	debug(2, "PuzzleBoolies: Retained %d Zoombinis in roster order", static_cast<int>(_puzzleZoombinis.size()));
}

void PuzzleBoolies::launchBall(int spotIdx) {
	if (spotIdx < 0 || spotIdx >= 5)
		return;

	if (!_spots[spotIdx].active)
		return;

	// Find next available zoombini
	int zoomIdx = -1;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_puzzleStatus != 0) {
			zoomIdx = i;
			break;
		}
	}

	if (zoomIdx < 0) {
		debug(2, "PuzzleBoolies: No zoombinis available to launch");
		return;
	}

	// Setup ball
	_activeBall.type = (zoomIdx % 2 == 0) ? kBallPositive : kBallNegative;
	_activeBall.zoombiniIdx = zoomIdx;
	_activeBall.startPos = _spots[spotIdx].pos;
	_activeBall.pos = _activeBall.startPos;

	// Target is first row of pins (center)
	_activeBall.endPos = Common::Point32(
		kPinBasePos.x + kPinCols[kPinRows - 1] * kPinColSpacing / 2,
		kPinBasePos.y);

	_activeBall.rollStart = _vm->getGameTickCount();
	_currentSpot = spotIdx;
	_state = kStateBallRolling;

	debug(2, "PuzzleBoolies: Launched ball from spot %d (zoombini %d)", spotIdx, zoomIdx);
}

void PuzzleBoolies::advanceBallRoll() {
	if (_activeBall.type == kBallNone)
		return;

	uint32 elapsed = _vm->getGameTickCount() - _activeBall.rollStart;
	float progress = (float)elapsed / kBallRollDuration;

	if (progress >= 1.0f) {
		// Ball reached target
		_activeBall.pos = _activeBall.endPos;

		// Check for pin collision
		if (checkPinCollision()) {
			knockDownPins();
		}

		return;
	}

	// Linear interpolation for ball position
	_activeBall.pos.x = _activeBall.startPos.x + static_cast<int32>((_activeBall.endPos.x - _activeBall.startPos.x) * progress);
	_activeBall.pos.y = _activeBall.startPos.y + static_cast<int32>((_activeBall.endPos.y - _activeBall.startPos.y) * progress);
}

bool PuzzleBoolies::checkPinCollision() {
	// Check if ball collides with any standing pin
	for (uint i = 0; i < _pins.size(); i++) {
		if (_pins[i].knocked)
			continue;

		// Simple distance check
		int32 dx = _activeBall.pos.x - _pins[i].pos.x;
		int32 dy = _activeBall.pos.y - _pins[i].pos.y;
		int32 distSq = dx * dx + dy * dy;

		if (distSq < 30 * 30) { // Within 30 pixels
			return true;
		}
	}

	return false;
}

void PuzzleBoolies::knockDownPins() {
	// Knock down pins near the ball
	int knocked = 0;

	for (uint i = 0; i < _pins.size(); i++) {
		if (_pins[i].knocked)
			continue;

		int32 dx = _activeBall.pos.x - _pins[i].pos.x;
		int32 dy = _activeBall.pos.y - _pins[i].pos.y;
		int32 distSq = dx * dx + dy * dy;

		// Ball knocks down pins within range
		// Larger range for positive balls, smaller for negative
		int knockRange = (_activeBall.type == kBallPositive) ? 50 : 35;

		if (distSq < knockRange * knockRange) {
			_pins[i].knocked = true;
			knocked++;
			_pinsKnocked++;
		}
	}

	debug(2, "PuzzleBoolies: Knocked down %d pins (total: %d)", knocked, _pinsKnocked);

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

	_stateTimer = _vm->getGameTickCount();
}

void PuzzleBoolies::freeZoombini(int zoombiniIdx) {
	if (zoombiniIdx < 0 || zoombiniIdx >= (int)_puzzleZoombinis.size())
		return;

	// Mark the selected Zoombini as released.
	_puzzleZoombinis[zoombiniIdx]->_puzzleStatus = 0;
	_freedCount++;

	debug(1, "PuzzleBoolies: Freed zoombini %d (total freed: %d)", zoombiniIdx, _freedCount);
}

int PuzzleBoolies::countFreeZoombinis() const {
	int count = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_puzzleStatus == 0)
			count++;
	}
	return count;
}

void PuzzleBoolies::onUpdate() {
	_vm->reseedRandomForV10();

	uint32 now = _vm->getGameTickCount();
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
			debug(1, "PuzzleBoolies: Complete, %d zoombinis freed", _freedCount);
			_state = kStateDone;
			_stateTimer = now;
		}
		break;

	case kStateDone:
		// Wait before transitioning out
		if (elapsed > 1000) {
			debug(1, "PuzzleBoolies: Returning to map");
			_vm->_returningFromPuzzle = true;
			_vm->_mapTransitionSourcePageId = kPageBoolies;
			_vm->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void PuzzleBoolies::onRenderBackground(ManagedSurface32 *screen) {
	if (_background)
		_background->drawToSurface(screen, Common::Point32(0, 0));
}

void PuzzleBoolies::onRenderScene(ManagedSurface32 *screen) {

	// Draw game elements
	drawPins(screen);
	drawSpots(screen);
	drawBlockers(screen);
	drawBall(screen);
	drawBoat(screen);
}

void PuzzleBoolies::drawSpots(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	for (int i = 0; i < 5; i++) {
		if (!_spots[i].active)
			continue;

		RleBlock *image = _spotImage[i];
		if (image) {
			image->drawToScreen(screen, Common::Point32(_spots[i].pos.x - 25, _spots[i].pos.y - 25), lut);
		} else {
			// Fallback: draw circle
			screen->fillRect(Common::Rect32(
						 _spots[i].pos.x - 20, _spots[i].pos.y - 20,
						 _spots[i].pos.x + 20, _spots[i].pos.y + 20),
							 0x00FFFF);
		}
	}
}

void PuzzleBoolies::drawPins(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	for (uint i = 0; i < _pins.size(); i++) {
		const Pin &pin = _pins[i];

		if (pin.knocked)
			continue; // Don't draw knocked pins

		RleBlock *image = pin.lighted ? _pinLightedImage : _pinImage;
		if (image) {
			image->drawToScreen(screen, Common::Point32(pin.pos.x - 10, pin.pos.y - 20), lut);
		} else {
			// Fallback: draw triangle
			uint32 color = pin.lighted ? 0xFFFF00 : 0xFFFFFF;
			screen->drawLine(pin.pos.x, pin.pos.y - 20, pin.pos.x - 10, pin.pos.y, color);
			screen->drawLine(pin.pos.x, pin.pos.y - 20, pin.pos.x + 10, pin.pos.y, color);
		screen->drawLine(pin.pos.x - 10, pin.pos.y, pin.pos.x + 10, pin.pos.y, color);
		}
	}
}

void PuzzleBoolies::drawBall(ManagedSurface32 *screen) {
	if (_activeBall.type == kBallNone)
		return;

	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	RleBlock *image = (_activeBall.type == kBallPositive) ? _ballPosImage : _ballNegImage;
	if (image) {
		image->drawToScreen(screen, Common::Point32(_activeBall.pos.x - 15, _activeBall.pos.y - 15), lut);
	} else {
		// Fallback: draw circle
		uint32 color = (_activeBall.type == kBallPositive) ? 0x00FF00 : 0xFF0000;
		screen->fillRect(Common::Rect32(
						 _activeBall.pos.x - 15, _activeBall.pos.y - 15,
						 _activeBall.pos.x + 15, _activeBall.pos.y + 15),
						 color);
	}
}

void PuzzleBoolies::drawBoat(ManagedSurface32 *screen) {
	if (!_boatVisible)
		return;

	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	if (_boatImage) {
		_boatImage->drawToScreen(screen, _boatPos, lut);
	} else {
		// Fallback: draw simple boat shape
		screen->fillRect(Common::Rect32(
						 _boatPos.x, _boatPos.y + 20,
						 _boatPos.x + 80, _boatPos.y + 40),
						 0x8B4513);
		screen->fillRect(Common::Rect32(
						 _boatPos.x + 30, _boatPos.y,
						 _boatPos.x + 50, _boatPos.y + 30),
						 0xFFFFFF);
	}
}

void PuzzleBoolies::drawBlockers(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();
	uint32 now = _vm->getGameTickCount();

	for (uint i = 0; i < _blockers.size(); i++) {
		const Common::Point32 &pos = _blockers[i];

		// Prefer animated blocker if available
		if (_blockerAnim) {
			int frameCount = _blockerAnim->getFrameCount();
			if (frameCount > 0) {
				int frameIdx = (now / 120) % frameCount; // ~8 fps animation
				const RleBlock *frame = _blockerAnim->getFrame(frameIdx);
				if (frame)
					frame->drawToScreen(screen, pos, lut);
			}
		} else if (_blockerImage) {
			_blockerImage->drawToScreen(screen, pos, lut);
		} else {
			// Fallback: draw rectangle
			screen->fillRect(Common::Rect32(pos.x, pos.y, pos.x + 30, pos.y + 60), 0x800000);
		}
	}
}

void PuzzleBoolies::onRenderActors(ManagedSurface32 *screen) {
	// Draw zoombinis waiting on the boat (freed ones)
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	if (!_zoombiniAnimation)
		return;

	int freeIdx = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_puzzleStatus == 0) {
			// This zoombini is free - draw on boat
			const ZoombiniState *z = _puzzleZoombinis[i];
			Common::Point32 pos(_boatPos.x + 10 + (freeIdx % 4) * 18, _boatPos.y + 10 + (freeIdx / 4) * 20);

			_zoombiniAnimation->drawZoombini(screen, z->_traits, pos, 0, 0, lut);
			freeIdx += 1;
		}
	}
}

EventHandleResult PuzzleBoolies::onLButtonDown(const Common::Point &pos) {
	if (_state != kStateIdle)
		return EventHandleResult::kPassthrough;

	// Check which spot was clicked
	for (int i = 0; i < 5; i++) {
		if (_spots[i].active && _spots[i].hitbox.contains(pos)) {
			debug(2, "PuzzleBoolies: Clicked spot %d", i);
			launchBall(i);
			return EventHandleResult::kConsumed;
		}
	}

	debug(2, "PuzzleBoolies: Click at %d,%d (no spot)", pos.x, pos.y);
	return EventHandleResult::kPassthrough;
}

int PuzzleBoolies::getRescuedBooliesPerZoombini(int level) {
	switch (level) {
	case 1:
		return 2;
	case 2:
		return 3;
	case 3:
	case 4:
		return 4;
	default:
		return 0;
	}
}

} // End of namespace Zoombini2
