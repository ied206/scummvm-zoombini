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

#include "zoombini2/pages/waterslide.h"
#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"
#include "zoombini2/sound.h"

namespace Zoombini2 {

// ============================================================================
// WaterslidePuzzle — Trait pair matching puzzle.
//
// Original: Waterslide__Init_43B490. Object size 0xBE58 (48728 bytes).
//
// Core mechanics:
//   - Zoombinis must pair up based on shared traits
//   - A random feature axis is chosen (hair/eyes/nose/feet)
//   - Two zoombinis with matching trait value can slide together
//   - Click to place zoombinis in slots
//   - Correct pairs slide down the waterslide
//   - Wrong pairs get rejected
//
// Algorithm (Diff 1):
//   1. Pick random feature axis (1-4)
//   2. Find zoombinis with same value for that feature
//   3. Create pairs
//   4. Player clicks slots to arrange zoombinis
//   5. When two slots match, pair slides down
// ============================================================================

// Animation timing (ms)
static const uint32 kMoveAnimDuration = 500;
static const uint32 kSlideAnimDuration = 1500;
static const uint32 kRejectAnimDuration = 1000;

// Slot hitbox size
static const int kSlotHitSize = 50;

// Helper to get feature by axis index (0=hair, 1=eyes, 2=nose, 3=feet)
static byte getFeature(const Zoombini *z, int axis) {
	switch (axis) {
	case 0: return z->_featureA;  // Hair
	case 1: return z->_featureB;  // Eyes
	case 2: return z->_featureC;  // Nose
	case 3: return z->_featureD;  // Feet
	default: return 0;
	}
}

// Slot positions (8 pairs = 16 slots, arranged in 2 columns)
// Left column (slots 0-7), Right column (slots 8-15)
static const int kSlotPositions[16][2] = {
	// Left column
	{ 150, 100 }, { 150, 150 }, { 150, 200 }, { 150, 250 },
	{ 150, 300 }, { 150, 350 }, { 150, 400 }, { 150, 450 },
	// Right column
	{ 450, 100 }, { 450, 150 }, { 450, 200 }, { 450, 250 },
	{ 450, 300 }, { 450, 350 }, { 450, 400 }, { 450, 450 }
};

// Pipe connection positions (between slot pairs)
static const int kPipePositions[8][2] = {
	{ 250, 100 }, { 250, 150 }, { 250, 200 }, { 250, 250 },
	{ 250, 300 }, { 250, 350 }, { 250, 400 }, { 250, 450 }
};

WaterslidePuzzle::WaterslidePuzzle(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageWaterslide),
	  _state(kStateInit),
	  _freedCount(0),
	  _selectedZoombini(-1),
	  _selectedSlot(-1),
	  _numSlots(0),
	  _numPairs(0),
	  _matchedPairs(0),
	  _pipeBlueHoriz(nullptr),
	  _pipeGreyHoriz(nullptr),
	  _pipeRedHoriz(nullptr),
	  _pipeBlueBigone(nullptr),
	  _pipeGreyBigone(nullptr),
	  _pipeRedBigone(nullptr),
	  _pastilleBlue(nullptr),
	  _pastilleGrey(nullptr),
	  _edgeNeutre(nullptr),
	  _blueFountainAnim(nullptr),
	  _littleTreeAnim(nullptr),
	  _valveAnim(nullptr),
	  _cascade1Anim(nullptr),
	  _cascade2Anim(nullptr),
	  _musicId(-1) {

	for (int i = 0; i < 4; i++) {
		_traitGfx[i] = nullptr;
	}

	for (int i = 0; i < kMaxSlots; i++) {
		_slots[i].state = kSlotEmpty;
		_slots[i].zoombiniIdx = -1;
		_slots[i].pairSlot = -1;
	}

	for (int i = 0; i < kMaxPairs; i++) {
		_pairs[i].zoombiniA = -1;
		_pairs[i].zoombiniB = -1;
		_pairs[i].matched = false;
	}
}

WaterslidePuzzle::~WaterslidePuzzle() {
	if (_musicId >= 0) {
		SoundManager *snd = _engine->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	// Free trait graphics
	for (int i = 0; i < 4; i++) {
		delete _traitGfx[i];
	}

	// Free pipe graphics
	delete _pipeBlueHoriz;
	delete _pipeGreyHoriz;
	delete _pipeRedHoriz;
	delete _pipeBlueBigone;
	delete _pipeGreyBigone;
	delete _pipeRedBigone;

	// Free other graphics
	delete _pastilleBlue;
	delete _pastilleGrey;
	delete _edgeNeutre;

	// Free animations
	delete _blueFountainAnim;
	delete _littleTreeAnim;
	delete _valveAnim;
	delete _cascade1Anim;
	delete _cascade2Anim;
}

void WaterslidePuzzle::init() {
	// Call base init for background and zoombini loading
	PuzzlePage::init();

	// BGM: 02-BS01.wav (IDA: Waterslide__Init_43B490)
	if (SoundManager *snd = _engine->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("sounds/music/02-BS01.wav"), true);
		if (_musicId >= 0) { snd->playLoop(_musicId); snd->setVolume(_musicId, snd->_volumeMusic); }
	}

	int diff = CLIP(_engine->getGameState()->_gameMode, 1, 3);
	debug(1, "WaterslidePuzzle::init — difficulty %d", diff);

	// Load resources
	loadResources();

	// Setup slots
	setupSlots();

	// Compute trait pairs based on difficulty
	computePairs();

	_freedCount = 0;
	_matchedPairs = 0;
	_selectedZoombini = -1;
	_selectedSlot = -1;
	_state = kStateIdle;
	_stateTimer = _engine->getGameTickCount();
}

void WaterslidePuzzle::loadResources() {
	// Load trait icons (4 features)
	for (int i = 0; i < 4; i++) {
		Common::Path traitPath(Common::String::format("bmp/waterslide/traits/%d", i + 1));
		_traitGfx[i] = new RleBlock();
		if (!_traitGfx[i]->loadFromFile(traitPath)) {
			delete _traitGfx[i];
			_traitGfx[i] = nullptr;
		}
	}

	// Load pipe graphics - blue
	Common::Path pipeBlueHPath("bmp/waterslide/pipes - blue/pipe - horizontal");
	_pipeBlueHoriz = new RleBlock();
	if (!_pipeBlueHoriz->loadFromFile(pipeBlueHPath)) {
		delete _pipeBlueHoriz;
		_pipeBlueHoriz = nullptr;
	}

	Common::Path pipeBlueBPath("bmp/waterslide/pipes - blue/pipe - lev1_bigone");
	_pipeBlueBigone = new RleBlock();
	if (!_pipeBlueBigone->loadFromFile(pipeBlueBPath)) {
		delete _pipeBlueBigone;
		_pipeBlueBigone = nullptr;
	}

	// Load pipe graphics - grey
	Common::Path pipeGreyHPath("bmp/waterslide/pipes - grey/pipe - horizontal");
	_pipeGreyHoriz = new RleBlock();
	if (!_pipeGreyHoriz->loadFromFile(pipeGreyHPath)) {
		delete _pipeGreyHoriz;
		_pipeGreyHoriz = nullptr;
	}

	// Load pipe graphics - red
	Common::Path pipeRedHPath("bmp/waterslide/pipes - red/pipe - horizontal");
	_pipeRedHoriz = new RleBlock();
	if (!_pipeRedHoriz->loadFromFile(pipeRedHPath)) {
		delete _pipeRedHoriz;
		_pipeRedHoriz = nullptr;
	}

	// Load pastilles
	Common::Path pastilleBluePath("bmp/waterslide/pastilles blue");
	_pastilleBlue = new RleBlock();
	if (!_pastilleBlue->loadFromFile(pastilleBluePath)) {
		delete _pastilleBlue;
		_pastilleBlue = nullptr;
	}

	Common::Path pastilleGreyPath("bmp/waterslide/pastilles grey");
	_pastilleGrey = new RleBlock();
	if (!_pastilleGrey->loadFromFile(pastilleGreyPath)) {
		delete _pastilleGrey;
		_pastilleGrey = nullptr;
	}

	// Load edge
	Common::Path edgePath("bmp/waterslide/edge neutre");
	_edgeNeutre = new RleBlock();
	if (!_edgeNeutre->loadFromFile(edgePath)) {
		delete _edgeNeutre;
		_edgeNeutre = nullptr;
	}

	// Load decorative animations
	Common::Path fountainPath("bmp/waterslide/blue fountain");
	_blueFountainAnim = new Animation();
	if (!_blueFountainAnim->loadFromFile(fountainPath)) {
		delete _blueFountainAnim;
		_blueFountainAnim = nullptr;
	}

	Common::Path treePath("bmp/waterslide/little tree");
	_littleTreeAnim = new Animation();
	if (!_littleTreeAnim->loadFromFile(treePath)) {
		delete _littleTreeAnim;
		_littleTreeAnim = nullptr;
	}

	Common::Path valvePath("bmp/waterslide/mr valve master");
	_valveAnim = new Animation();
	if (!_valveAnim->loadFromFile(valvePath)) {
		delete _valveAnim;
		_valveAnim = nullptr;
	}

	Common::Path cascade1Path("bmp/waterslide/pipe - cascade 1");
	_cascade1Anim = new Animation();
	if (!_cascade1Anim->loadFromFile(cascade1Path)) {
		delete _cascade1Anim;
		_cascade1Anim = nullptr;
	}

	Common::Path cascade2Path("bmp/waterslide/pipe - cascade 2");
	_cascade2Anim = new Animation();
	if (!_cascade2Anim->loadFromFile(cascade2Path)) {
		delete _cascade2Anim;
		_cascade2Anim = nullptr;
	}

	debug(2, "WaterslidePuzzle: Resources loaded");
}

void WaterslidePuzzle::setupSlots() {
	// Setup 16 slots (8 pairs)
	_numSlots = 16;

	for (int i = 0; i < _numSlots; i++) {
		_slots[i].x = kSlotPositions[i][0];
		_slots[i].y = kSlotPositions[i][1];
		_slots[i].hitbox = Common::Rect(
			_slots[i].x - kSlotHitSize / 2,
			_slots[i].y - kSlotHitSize / 2,
			_slots[i].x + kSlotHitSize / 2,
			_slots[i].y + kSlotHitSize / 2
		);
		_slots[i].state = kSlotEmpty;
		_slots[i].zoombiniIdx = -1;

		// Pair slots: 0 pairs with 8, 1 pairs with 9, etc.
		if (i < 8) {
			_slots[i].pairSlot = i + 8;
		} else {
			_slots[i].pairSlot = i - 8;
		}
	}

	debug(2, "WaterslidePuzzle: Setup %d slots", _numSlots);
}

void WaterslidePuzzle::computePairs() {
	int diff = CLIP(_engine->getGameState()->_gameMode, 1, 3);

	switch (diff) {
	case 1:
		computePairsDiff1();
		break;
	case 2:
		computePairsDiff2();
		break;
	default:
		computePairsDiff3();
		break;
	}

	debug(2, "WaterslidePuzzle: Computed %d pairs (diff %d)", _numPairs, diff);
}

void WaterslidePuzzle::computePairsDiff1() {
	// Simple random pairing
	// Pick a random feature axis, find zoombinis that share values

	Common::RandomSource rnd("waterslide");

	// Clear pairs
	_numPairs = 0;
	for (int i = 0; i < kMaxPairs; i++) {
		_pairs[i].zoombiniA = -1;
		_pairs[i].zoombiniB = -1;
		_pairs[i].matched = false;
	}

	// Mark which zoombinis are already paired
	Common::Array<bool> paired;
	paired.resize(_puzzleZoombinis.size(), false);

	// Try to pair zoombinis
	for (uint i = 0; i < _puzzleZoombinis.size() && _numPairs < kMaxPairs; i++) {
		if (paired[i])
			continue;

		const Zoombini *z1 = _puzzleZoombinis[i];

		// Pick a random feature axis (0-3: hair, eyes, nose, feet)
		int axis = rnd.getRandomNumber(3);
		byte z1Value = getFeature(z1, axis);

		// Find another zoombini with same value
		for (uint j = i + 1; j < _puzzleZoombinis.size(); j++) {
			if (paired[j])
				continue;

			const Zoombini *z2 = _puzzleZoombinis[j];
			if (getFeature(z2, axis) == z1Value) {
				// Found a match
				_pairs[_numPairs].zoombiniA = i;
				_pairs[_numPairs].zoombiniB = j;
				_pairs[_numPairs].featureAxis = axis;
				_pairs[_numPairs].sharedValue = z1Value;
				_pairs[_numPairs].matched = false;
				_numPairs++;

				paired[i] = true;
				paired[j] = true;
				break;
			}
		}
	}
}

void WaterslidePuzzle::computePairsDiff2() {
	// Similar to Diff1 but with different iteration order
	computePairsDiff1();  // Placeholder - same algorithm for now
}

void WaterslidePuzzle::computePairsDiff3() {
	// Complex bipartite graph matching
	// For now, use same algorithm (proper implementation would use
	// linked lists and iterative refinement as in 0x435BA0)
	computePairsDiff1();
}

void WaterslidePuzzle::clickSlot(int slotIdx) {
	if (slotIdx < 0 || slotIdx >= _numSlots)
		return;

	Slot &slot = _slots[slotIdx];

	if (_selectedZoombini < 0) {
		// No zoombini selected - maybe click on occupied slot to deselect
		if (slot.state == kSlotOccupied) {
			// Remove zoombini from slot
			slot.state = kSlotEmpty;
			slot.zoombiniIdx = -1;
			debug(2, "WaterslidePuzzle: Cleared slot %d", slotIdx);
		}
		return;
	}

	if (slot.state != kSlotEmpty) {
		// Slot already occupied
		return;
	}

	// Move zoombini to this slot
	moveZoombiniToSlot(_selectedZoombini, slotIdx);
}

void WaterslidePuzzle::moveZoombiniToSlot(int zoombiniIdx, int slotIdx) {
	if (zoombiniIdx < 0 || slotIdx < 0)
		return;

	// Clear any previous slot this zoombini was in
	for (int i = 0; i < _numSlots; i++) {
		if (_slots[i].zoombiniIdx == zoombiniIdx) {
			_slots[i].state = kSlotEmpty;
			_slots[i].zoombiniIdx = -1;
		}
	}

	// Place zoombini in new slot
	_slots[slotIdx].state = kSlotOccupied;
	_slots[slotIdx].zoombiniIdx = zoombiniIdx;

	debug(2, "WaterslidePuzzle: Placed zoombini %d in slot %d", zoombiniIdx, slotIdx);

	// Check if pair slot is also occupied
	int pairSlot = _slots[slotIdx].pairSlot;
	if (pairSlot >= 0 && _slots[pairSlot].state == kSlotOccupied) {
		// Both slots have zoombinis - check for match
		if (checkPairMatch(slotIdx, pairSlot)) {
			slideDownPair(slotIdx, pairSlot);
		} else {
			rejectPair(slotIdx, pairSlot);
		}
	}

	_selectedZoombini = -1;
	_selectedSlot = -1;
}

bool WaterslidePuzzle::checkPairMatch(int slotA, int slotB) {
	int zA = _slots[slotA].zoombiniIdx;
	int zB = _slots[slotB].zoombiniIdx;

	if (zA < 0 || zB < 0)
		return false;

	// Check if these zoombinis are a valid pair
	for (int i = 0; i < _numPairs; i++) {
		if (_pairs[i].matched)
			continue;

		if ((_pairs[i].zoombiniA == zA && _pairs[i].zoombiniB == zB) ||
		    (_pairs[i].zoombiniA == zB && _pairs[i].zoombiniB == zA)) {
			_pairs[i].matched = true;
			return true;
		}
	}

	return false;
}

void WaterslidePuzzle::slideDownPair(int slotA, int slotB) {
	debug(2, "WaterslidePuzzle: Sliding pair slots %d and %d", slotA, slotB);

	_slots[slotA].state = kSlotMatched;
	_slots[slotB].state = kSlotMatched;

	// Free both zoombinis
	freeZoombini(_slots[slotA].zoombiniIdx);
	freeZoombini(_slots[slotB].zoombiniIdx);

	_matchedPairs++;
	_state = kStateSliding;
	_stateTimer = _engine->getGameTickCount();
}

void WaterslidePuzzle::rejectPair(int slotA, int slotB) {
	debug(2, "WaterslidePuzzle: Rejecting pair slots %d and %d", slotA, slotB);

	_slots[slotA].state = kSlotRejected;
	_slots[slotB].state = kSlotRejected;
	_state = kStateRejecting;
	_stateTimer = _engine->getGameTickCount();
}

void WaterslidePuzzle::freeZoombini(int zoombiniIdx) {
	if (zoombiniIdx < 0 || zoombiniIdx >= (int)_puzzleZoombinis.size())
		return;

	_puzzleZoombinis[zoombiniIdx]->_freeStatus = 0;
	_freedCount++;

	debug(1, "WaterslidePuzzle: Freed zoombini %d (total: %d)", zoombiniIdx, _freedCount);
}

int WaterslidePuzzle::countFreeZoombinis() const {
	int count = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_freeStatus == 0)
			count++;
	}
	return count;
}

void WaterslidePuzzle::update() {
	uint32 now = _engine->getGameTickCount();
	uint32 elapsed = now - _stateTimer;

	switch (_state) {
	case kStateInit:
		break;

	case kStateIdle:
		// Wait for player clicks
		break;

	case kStateZoombiniMoving:
		// Zoombini animation moving to slot
		if (elapsed > kMoveAnimDuration) {
			_state = kStateIdle;
			_stateTimer = now;
		}
		break;

	case kStateCheckingMatch:
		// Brief pause while checking
		if (elapsed > 200) {
			_state = kStateIdle;
			_stateTimer = now;
		}
		break;

	case kStateSliding:
		// Pair sliding down animation
		if (elapsed > kSlideAnimDuration) {
			// Clear matched slots
			for (int i = 0; i < _numSlots; i++) {
				if (_slots[i].state == kSlotMatched) {
					_slots[i].state = kSlotEmpty;
					_slots[i].zoombiniIdx = -1;
				}
			}

			// Check completion
			if (_freedCount >= 4) {
				_state = kStateDone;
			} else {
				_state = kStateIdle;
			}
			_stateTimer = now;
		}
		break;

	case kStateRejecting:
		// Rejection animation
		if (elapsed > kRejectAnimDuration) {
			// Reset rejected slots
			for (int i = 0; i < _numSlots; i++) {
				if (_slots[i].state == kSlotRejected) {
					_slots[i].state = kSlotEmpty;
					_slots[i].zoombiniIdx = -1;
				}
			}
			_state = kStateIdle;
			_stateTimer = now;
		}
		break;

	case kStateDone:
		// Wait before transitioning
		if (elapsed > 2000) {
			debug(1, "WaterslidePuzzle: Complete, %d zoombinis freed", _freedCount);
			_engine->_returningFromPuzzle = true;
			_engine->_maptransSourceWorld = kPageWaterslide;
			_engine->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void WaterslidePuzzle::draw(Graphics::ManagedSurface *screen) {
	// Draw background
	if (_background)
		_background->drawToSurface(screen, 0, 0);

	// Draw decorations
	drawDecorations(screen);

	// Draw pipes
	drawPipes(screen);

	// Draw slots
	drawSlots(screen);

	// Draw trait indicators
	drawTraitIndicators(screen);

	// Draw zoombinis
	drawZoombinis(screen);
}

void WaterslidePuzzle::drawPipes(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw pipes connecting slot pairs
	for (int i = 0; i < 8; i++) {
		int leftSlot = i;
		int rightSlot = i + 8;

		RleBlock *pipeGfx = nullptr;

		// Determine pipe color based on slot states
		if (_slots[leftSlot].state == kSlotMatched || _slots[rightSlot].state == kSlotMatched) {
			pipeGfx = _pipeBlueHoriz;  // Blue for matched
		} else if (_slots[leftSlot].state == kSlotRejected || _slots[rightSlot].state == kSlotRejected) {
			pipeGfx = _pipeRedHoriz;   // Red for rejected
		} else {
			pipeGfx = _pipeGreyHoriz;  // Grey for neutral
		}

		if (pipeGfx) {
			pipeGfx->drawToScreen(screen, kPipePositions[i][0], kPipePositions[i][1], lut);
		}
	}
}

void WaterslidePuzzle::drawSlots(Graphics::ManagedSurface *screen) {
	// Draw slot indicators/pastilles
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (int i = 0; i < _numSlots; i++) {
		const Slot &slot = _slots[i];

		RleBlock *gfx = nullptr;
		if (slot.state == kSlotOccupied || slot.state == kSlotMatched) {
			gfx = _pastilleBlue;
		} else {
			gfx = _pastilleGrey;
		}

		if (gfx) {
			gfx->drawToScreen(screen, slot.x - 15, slot.y - 15, lut);
		} else {
			// Fallback: draw circle
			uint32 color = (slot.state == kSlotEmpty) ? 0x808080 : 0x0000FF;
			screen->fillRect(Common::Rect(slot.x - 10, slot.y - 10, slot.x + 10, slot.y + 10), color);
		}
	}
}

void WaterslidePuzzle::drawTraitIndicators(Graphics::ManagedSurface *screen) {
	// Draw trait icons near pairs to show what feature they match on
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (int i = 0; i < _numPairs; i++) {
		if (_pairs[i].matched)
			continue;

		int axis = _pairs[i].featureAxis;
		if (axis < 0 || axis >= 4)
			continue;

		RleBlock *traitGfx = _traitGfx[axis];
		if (!traitGfx)
			continue;

		// Draw near the corresponding pipe
		int pipeX = kPipePositions[i][0];
		int pipeY = kPipePositions[i][1];
		traitGfx->drawToScreen(screen, pipeX + 50, pipeY - 10, lut);
	}
}

void WaterslidePuzzle::drawDecorations(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();
	uint32 now = _engine->getGameTickCount();

	// Draw fountain animation
	if (_blueFountainAnim) {
		int frame = (now / 100) % _blueFountainAnim->getFrameCount();
		const RleBlock *frameGfx = _blueFountainAnim->getFrame(frame);
		if (frameGfx)
			frameGfx->drawToScreen(screen, 550, 50, lut);
	}

	// Draw tree animation
	if (_littleTreeAnim) {
		int frame = (now / 150) % _littleTreeAnim->getFrameCount();
		const RleBlock *frameGfx = _littleTreeAnim->getFrame(frame);
		if (frameGfx)
			frameGfx->drawToScreen(screen, 50, 100, lut);
	}

	// Draw valve master
	if (_valveAnim) {
		int frame = (now / 120) % _valveAnim->getFrameCount();
		const RleBlock *frameGfx = _valveAnim->getFrame(frame);
		if (frameGfx)
			frameGfx->drawToScreen(screen, 300, 50, lut);
	}
}

void WaterslidePuzzle::drawZoombinis(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	if (!_zoombiniGfx)
		return;

	// Draw zoombinis in slots
	for (int i = 0; i < _numSlots; i++) {
		const Slot &slot = _slots[i];
		if (slot.zoombiniIdx < 0)
			continue;

		const Zoombini *z = _puzzleZoombinis[slot.zoombiniIdx];
		int x = slot.x - 15;
		int y = slot.y - 20;

		// Draw zoombini
		int baseIdx = 0;
		const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
		if (frame)
			frame->drawToScreen(screen, x, y, lut);

		const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
		for (int feat = 1; feat <= 4; feat++) {
			int featIdx = baseIdx + feat * ZoombiniGfx::kDim2 + features[feat - 1];
			frame = _zoombiniGfx->getFrame(featIdx, 0);
			if (frame)
				frame->drawToScreen(screen, x, y, lut);
		}
	}

	// Draw unplaced zoombinis in a staging area
	int stageX = 50;
	int stageY = 450;
	int idx = 0;

	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		// Check if this zoombini is in any slot
		bool inSlot = false;
		for (int s = 0; s < _numSlots; s++) {
			if (_slots[s].zoombiniIdx == (int)i) {
				inSlot = true;
				break;
			}
		}

		if (!inSlot && _puzzleZoombinis[i]->_freeStatus != 0) {
			// Draw in staging area
			const Zoombini *z = _puzzleZoombinis[i];
			int x = stageX + (idx % 8) * 25;
			int y = stageY;

			// Highlight if selected
			if ((int)i == _selectedZoombini) {
				screen->fillRect(Common::Rect(x - 2, y - 2, x + 22, y + 32), 0xFFFF00);
			}

			int baseIdx = 0;
			const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
			if (frame)
				frame->drawToScreen(screen, x, y, lut);

			const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
			for (int feat = 1; feat <= 4; feat++) {
				int featIdx = baseIdx + feat * ZoombiniGfx::kDim2 + features[feat - 1];
				frame = _zoombiniGfx->getFrame(featIdx, 0);
				if (frame)
					frame->drawToScreen(screen, x, y, lut);
			}

			idx++;
		}
	}
}

void WaterslidePuzzle::handleClick(const Common::Point &pos) {
	if (_state != kStateIdle)
		return;

	// Check if clicked on a slot
	for (int i = 0; i < _numSlots; i++) {
		if (_slots[i].hitbox.contains(pos)) {
			debug(2, "WaterslidePuzzle: Clicked slot %d", i);
			clickSlot(i);
			return;
		}
	}

	// Check if clicked on staging area zoombini
	int stageX = 50;
	int stageY = 450;
	int idx = 0;

	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		bool inSlot = false;
		for (int s = 0; s < _numSlots; s++) {
			if (_slots[s].zoombiniIdx == (int)i) {
				inSlot = true;
				break;
			}
		}

		if (!inSlot && _puzzleZoombinis[i]->_freeStatus != 0) {
			int x = stageX + (idx % 8) * 25;
			int y = stageY;
			Common::Rect zoomRect(x, y, x + 20, y + 30);

			if (zoomRect.contains(pos)) {
				_selectedZoombini = i;
				debug(2, "WaterslidePuzzle: Selected zoombini %d", i);
				return;
			}

			idx++;
		}
	}

	debug(3, "WaterslidePuzzle: Click at %d,%d (nothing)", pos.x, pos.y);
}

} // End of namespace Zoombini2
