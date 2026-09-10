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
#include "zoombini2/pages/puzzle_waterslide.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// PuzzleWaterslide - trait pair matching puzzle.
//
// Core mechanics:
//   - Zoombinis must pair up based on shared traits
//   - A random trait axis is chosen (feet/nose/hair/eyes)
//   - Two zoombinis with matching trait value can slide together
//   - Click to place zoombinis in slots
//   - Correct pairs slide down the waterslide
//   - Wrong pairs get rejected
//
// Algorithm (Level 1):
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

byte PuzzleWaterslide::getTrait(const ZoombiniState *z, ZmbTrait::TraitIndex axis) {
	return z ? z->_traits.getValue(axis) : 0;
}

// Slot positions (8 pairs = 16 slots, arranged in 2 columns)
// Left column (slots 0-7), Right column (slots 8-15)
static const Common::Point32 kSlotPos[16] = {
	// Left column
	{150, 100},
	{150, 150},
	{150, 200},
	{150, 250},
	{150, 300},
	{150, 350},
	{150, 400},
	{150, 450},
	// Right column
	{450, 100},
	{450, 150},
	{450, 200},
	{450, 250},
	{450, 300},
	{450, 350},
	{450, 400},
	{450, 450},
};

// Pipe connection positions (between slot pairs)
static const Common::Point32 kPipePos[8] = {
	Common::Point32(250, 100),
	Common::Point32(250, 150),
	Common::Point32(250, 200),
	Common::Point32(250, 250),
	Common::Point32(250, 300),
	Common::Point32(250, 350),
	Common::Point32(250, 400),
	Common::Point32(250, 450),
};

PuzzleWaterslide::PuzzleWaterslide(Zoombini2Engine *vm)
	: PuzzleBase(vm, kPageWaterSlide),
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
		_traitImage[i] = nullptr;
	}

	for (int i = 0; i < kMaxSlots; i++) {
		_slots[i].pos = Common::Point32();
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

PuzzleWaterslide::~PuzzleWaterslide() {
	if (_musicId >= 0) {
		SoundManager *snd = _vm->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	// Free trait graphics
	for (int i = 0; i < 4; i++) {
		delete _traitImage[i];
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

void PuzzleWaterslide::init() {
	// Call base init for background and zoombini loading
	PuzzleBase::init();

	// Start the Pipes of Paloo music.
	if (SoundManager *snd = _vm->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("#sounds/music/02-BS01.wav"), true);
		if (_musicId >= 0) {
			snd->playLoop(_musicId);
			snd->setVolume(_musicId, snd->_volumeMusic);
		}
	}

	int level = CLIP(_vm->getGameState()->_level, 1, 3);
	debug(1, "WaterslidePuzzle::init - level %d", level);

	// Load resources
	loadResources();

	// Setup slots
	setupSlots();

	// Compute trait pairs based on level.
	computePairs();

	_freedCount = 0;
	_matchedPairs = 0;
	_selectedZoombini = -1;
	_selectedSlot = -1;
	_state = kStateIdle;
	_stateTimer = _vm->getGameTickCount();
}

void PuzzleWaterslide::loadResources() {
	// Load trait icons (4 features)
	for (int i = 0; i < 4; i++) {
		Common::Path traitPath(Common::String::format("bmp/waterslide/traits/%d", i + 1));
		_traitImage[i] = new RleBlock();
		if (!_traitImage[i]->loadFromFile(traitPath)) {
			delete _traitImage[i];
			_traitImage[i] = nullptr;
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

void PuzzleWaterslide::setupSlots() {
	// Setup 16 slots (8 pairs)
	_numSlots = 16;

	for (int i = 0; i < _numSlots; i++) {
		_slots[i].pos = kSlotPos[i];
		_slots[i].hitbox = Common::Rect(
			static_cast<int16>(_slots[i].pos.x - kSlotHitSize / 2),
			static_cast<int16>(_slots[i].pos.y - kSlotHitSize / 2),
			static_cast<int16>(_slots[i].pos.x + kSlotHitSize / 2),
			static_cast<int16>(_slots[i].pos.y + kSlotHitSize / 2));
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

void PuzzleWaterslide::computePairs() {
	int level = CLIP(_vm->getGameState()->_level, 1, 3);

	switch (level) {
	case 1:
		computePairsLevel1();
		break;
	case 2:
		computePairsLevel2();
		break;
	default:
		computePairsLevel3();
		break;
	}

	debug(2, "WaterslidePuzzle: Computed %d pairs (level %d)", _numPairs, level);
}

void PuzzleWaterslide::computePairsLevel1() {
	if (_vm->useGreedyWaterslidePairing())
		computePairsLevel1Greedy();
	else
		computePairsLevel1Matching();
}

void PuzzleWaterslide::clearPairs() {
	_numPairs = 0;
	for (int i = 0; i < kMaxPairs; i++) {
		_pairs[i].zoombiniA = -1;
		_pairs[i].zoombiniB = -1;
		_pairs[i].traitAxis = ZmbTrait::TraitIndex::kFeet00;
		_pairs[i].sharedValue = 0;
		_pairs[i].matched = false;
	}
}

void PuzzleWaterslide::addPair(int zoombiniA, int zoombiniB, ZmbTrait::TraitIndex traitAxis, int sharedValue) {
	if (kMaxPairs <= _numPairs)
		return;
	_pairs[_numPairs].zoombiniA = zoombiniA;
	_pairs[_numPairs].zoombiniB = zoombiniB;
	_pairs[_numPairs].traitAxis = traitAxis;
	_pairs[_numPairs].sharedValue = sharedValue;
	_pairs[_numPairs].matched = false;
	_numPairs += 1;
}

bool PuzzleWaterslide::findSharedTrait(int zoombiniA, int zoombiniB, ZmbTrait::TraitIndex &traitAxis, int &sharedValue) {
	int traitOrder[ZmbTrait::kTraitCount] = {0, 1, 2, 3};
	for (int i = ZmbTrait::kTraitCount - 1; 0 < i; i--) {
		const int swapIndex = _vm->getRandom()->getRandomNumber(i);
		SWAP(traitOrder[i], traitOrder[swapIndex]);
	}

	for (int i = 0; i < ZmbTrait::kTraitCount; i++) {
		const ZmbTrait::TraitIndex candidateAxis = static_cast<ZmbTrait::TraitIndex>(traitOrder[i]);
		const byte candidateValue = getTrait(_puzzleZoombinis[zoombiniA], candidateAxis);
		if (candidateValue == getTrait(_puzzleZoombinis[zoombiniB], candidateAxis)) {
			traitAxis = candidateAxis;
			sharedValue = candidateValue;
			return true;
		}
	}
	return false;
}

void PuzzleWaterslide::computePairsLevel1Matching() {
	TraitPair bestPairs[kMaxPairs];
	int bestPairCount = 0;
	const int rosterCount = MIN<int>(_puzzleZoombinis.size(), kMaxPairs * 2);
	const int requiredPairCount = rosterCount / 2;
	Common::Array<int> priorityOrder;
	for (int i = 0; i < rosterCount; i++)
		priorityOrder.push_back(i);

	for (int attempt = 0; attempt < 9 && bestPairCount < requiredPairCount; attempt++) {
		clearPairs();
		Common::Array<int> available = priorityOrder;
		Common::Array<int> unmatched;
		while (1 < available.size() && _numPairs < kMaxPairs) {
			const int zoombiniA = available[0];
			int matchedIndex = -1;
			ZmbTrait::TraitIndex traitAxis = ZmbTrait::TraitIndex::kFeet00;
			int sharedValue = 0;
			for (uint candidateIndex = 1; candidateIndex < available.size(); candidateIndex++) {
				if (findSharedTrait(zoombiniA, available[candidateIndex], traitAxis, sharedValue)) {
					matchedIndex = static_cast<int>(candidateIndex);
					break;
				}
			}
			if (0 <= matchedIndex) {
				addPair(zoombiniA, available[matchedIndex], traitAxis, sharedValue);
				available.remove_at(matchedIndex);
			} else {
				unmatched.push_back(zoombiniA);
			}
			available.remove_at(0);
		}
		if (!available.empty())
			unmatched.push_back(available[0]);

		if (bestPairCount < _numPairs) {
			bestPairCount = _numPairs;
			for (int i = 0; i < bestPairCount; i++)
				bestPairs[i] = _pairs[i];
		}
		if (bestPairCount < requiredPairCount) {
			priorityOrder = unmatched;
			for (int rosterIndex = 0; rosterIndex < rosterCount; rosterIndex++) {
				bool alreadyPrioritized = false;
				for (uint i = 0; i < priorityOrder.size(); i++) {
					if (priorityOrder[i] == rosterIndex) {
						alreadyPrioritized = true;
						break;
					}
				}
				if (!alreadyPrioritized)
					priorityOrder.push_back(rosterIndex);
			}
		}
	}

	clearPairs();
	_numPairs = bestPairCount;
	for (int i = 0; i < _numPairs; i++)
		_pairs[i] = bestPairs[i];
}

void PuzzleWaterslide::computePairsLevel1Greedy() {
	TraitPair bestPairs[kMaxPairs];
	int bestPairCount = 0;
	const int requiredPairCount = MIN<int>(_puzzleZoombinis.size() / 2, kMaxPairs);

	for (int attempt = 0; attempt < 10 && bestPairCount < requiredPairCount; attempt++) {
		clearPairs();
		Common::Array<int> available;
		for (uint i = 0; i < _puzzleZoombinis.size() && i < static_cast<uint>(kMaxPairs * 2); i++)
			available.push_back(static_cast<int>(i));
		if (0 < attempt) {
			for (int i = static_cast<int>(available.size()) - 1; 0 < i; i--) {
				const int swapIndex = _vm->getRandom()->getRandomNumber(i);
				SWAP(available[i], available[swapIndex]);
			}
		}

		while (1 < available.size() && _numPairs < kMaxPairs) {
			const int zoombiniA = available[0];
			int matchedIndex = -1;
			ZmbTrait::TraitIndex traitAxis = ZmbTrait::TraitIndex::kFeet00;
			int sharedValue = 0;
			for (uint candidateIndex = 1; candidateIndex < available.size(); candidateIndex++) {
				if (findSharedTrait(zoombiniA, available[candidateIndex], traitAxis, sharedValue)) {
					matchedIndex = static_cast<int>(candidateIndex);
					break;
				}
			}
			if (0 <= matchedIndex) {
				addPair(zoombiniA, available[matchedIndex], traitAxis, sharedValue);
				available.remove_at(matchedIndex);
			}
			available.remove_at(0);
		}

		if (bestPairCount < _numPairs) {
			bestPairCount = _numPairs;
			for (int i = 0; i < bestPairCount; i++)
				bestPairs[i] = _pairs[i];
		}
	}

	clearPairs();
	_numPairs = bestPairCount;
	for (int i = 0; i < _numPairs; i++)
		_pairs[i] = bestPairs[i];
}

void PuzzleWaterslide::computePairsLevel2() {
	// Similar to Level 1 but with a different iteration order.
	computePairsLevel1Matching(); // Placeholder - uses the default level-one matching algorithm for now.
}

void PuzzleWaterslide::computePairsLevel3() {
	// The current upper-level path reuses the level-one pairing algorithm.
	computePairsLevel1Matching();
}

void PuzzleWaterslide::clickSlot(int slotIdx) {
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

void PuzzleWaterslide::moveZoombiniToSlot(int zoombiniIdx, int slotIdx) {
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

bool PuzzleWaterslide::checkPairMatch(int slotA, int slotB) {
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

void PuzzleWaterslide::slideDownPair(int slotA, int slotB) {
	debug(2, "WaterslidePuzzle: Sliding pair slots %d and %d", slotA, slotB);

	_slots[slotA].state = kSlotMatched;
	_slots[slotB].state = kSlotMatched;

	// Free both zoombinis
	freeZoombini(_slots[slotA].zoombiniIdx);
	freeZoombini(_slots[slotB].zoombiniIdx);

	_matchedPairs++;
	_state = kStateSliding;
	_stateTimer = _vm->getGameTickCount();
}

void PuzzleWaterslide::rejectPair(int slotA, int slotB) {
	debug(2, "WaterslidePuzzle: Rejecting pair slots %d and %d", slotA, slotB);

	_slots[slotA].state = kSlotRejected;
	_slots[slotB].state = kSlotRejected;
	_state = kStateRejecting;
	_stateTimer = _vm->getGameTickCount();
}

void PuzzleWaterslide::freeZoombini(int zoombiniIdx) {
	if (zoombiniIdx < 0 || zoombiniIdx >= (int)_puzzleZoombinis.size())
		return;

	_puzzleZoombinis[zoombiniIdx]->_puzzleStatus = 0;
	_freedCount++;

	debug(1, "WaterslidePuzzle: Freed zoombini %d (total: %d)", zoombiniIdx, _freedCount);
}

int PuzzleWaterslide::countFreeZoombinis() const {
	int count = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_puzzleStatus == 0)
			count++;
	}
	return count;
}

void PuzzleWaterslide::onUpdate() {
	uint32 now = _vm->getGameTickCount();
	uint32 elapsed = now - _stateTimer;

	switch (_state) {
	case kStateInit:
		break;

	case kStateIdle:
		// Wait for player clicks
		break;

	case kStateZoombiniMoving:
		// Wait for the selected Zoombini's slot movement to finish.
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
			_vm->_returningFromPuzzle = true;
			_vm->_mapTransitionSourcePageId = kPageWaterSlide;
			_vm->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void PuzzleWaterslide::onRenderBackground(ManagedSurface32 *screen) {
	// Draw background
	if (_background)
		_background->drawToSurface(screen, Common::Point32(0, 0));

}

void PuzzleWaterslide::onRenderScene(ManagedSurface32 *screen) {
	// Draw decorations
	drawDecorations(screen);

	// Draw pipes
	drawPipes(screen);

	// Draw slots
	drawSlots(screen);

	// Draw trait indicators
	drawTraitIndicators(screen);


}

void PuzzleWaterslide::drawPipes(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	// Draw pipes connecting slot pairs
	for (int i = 0; i < 8; i++) {
		int leftSlot = i;
		int rightSlot = i + 8;

		RleBlock *pipeImage = nullptr;

		// Determine pipe color based on slot states
		if (_slots[leftSlot].state == kSlotMatched || _slots[rightSlot].state == kSlotMatched) {
			pipeImage = _pipeBlueHoriz; // Blue for matched
		} else if (_slots[leftSlot].state == kSlotRejected || _slots[rightSlot].state == kSlotRejected) {
			pipeImage = _pipeRedHoriz; // Red for rejected
		} else {
			pipeImage = _pipeGreyHoriz; // Grey for neutral
		}

		if (pipeImage) {
			pipeImage->drawToScreen(screen, kPipePos[i], lut);
		}
	}
}

void PuzzleWaterslide::drawSlots(ManagedSurface32 *screen) {
	// Draw slot indicators/pastilles
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	for (int i = 0; i < _numSlots; i++) {
		const Slot &slot = _slots[i];

		RleBlock *image = nullptr;
		if (slot.state == kSlotOccupied || slot.state == kSlotMatched) {
			image = _pastilleBlue;
		} else {
			image = _pastilleGrey;
		}

		if (image) {
			image->drawToScreen(screen, Common::Point32(slot.pos.x - 15, slot.pos.y - 15), lut);
		} else {
			// Fallback: draw circle
			uint32 color = (slot.state == kSlotEmpty) ? 0x808080 : 0x0000FF;
			screen->fillRect(
				Common::Rect(
					static_cast<int16>(slot.pos.x - 10), static_cast<int16>(slot.pos.y - 10),
					static_cast<int16>(slot.pos.x + 10), static_cast<int16>(slot.pos.y + 10)),
				color);
		}
	}
}

void PuzzleWaterslide::drawTraitIndicators(ManagedSurface32 *screen) {
	// Draw trait icons near pairs to show which trait they match on.
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	for (int i = 0; i < _numPairs; i++) {
		if (_pairs[i].matched)
			continue;

		const int axis = static_cast<int>(_pairs[i].traitAxis);
		if (axis < 0 || ZmbTrait::kTraitCount <= axis)
			continue;

		RleBlock *traitImage = _traitImage[axis];
		if (!traitImage)
			continue;

		// Draw near the corresponding pipe
		const Common::Point32 pipePos = kPipePos[i];
		traitImage->drawToScreen(screen, Common::Point32(pipePos.x + 50, pipePos.y - 10), lut);
	}
}

void PuzzleWaterslide::drawDecorations(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();
	uint32 now = _vm->getGameTickCount();

	// Draw fountain animation
	if (_blueFountainAnim) {
		int frame = (now / 100) % _blueFountainAnim->getFrameCount();
		const RleBlock *frameImage = _blueFountainAnim->getFrame(frame);
		if (frameImage)
			frameImage->drawToScreen(screen, Common::Point32(550, 50), lut);
	}

	// Draw tree animation
	if (_littleTreeAnim) {
		int frame = (now / 150) % _littleTreeAnim->getFrameCount();
		const RleBlock *frameImage = _littleTreeAnim->getFrame(frame);
		if (frameImage)
			frameImage->drawToScreen(screen, Common::Point32(50, 100), lut);
	}

	// Draw valve master
	if (_valveAnim) {
		int frame = (now / 120) % _valveAnim->getFrameCount();
		const RleBlock *frameImage = _valveAnim->getFrame(frame);
		if (frameImage)
			frameImage->drawToScreen(screen, Common::Point32(300, 50), lut);
	}
}

void PuzzleWaterslide::onRenderActors(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	if (!_zoombiniAnimation)
		return;

	// Draw zoombinis in slots
	for (int i = 0; i < _numSlots; i++) {
		const Slot &slot = _slots[i];
		if (slot.zoombiniIdx < 0)
			continue;

		const ZoombiniState *z = _puzzleZoombinis[slot.zoombiniIdx];
		const Common::Point32 pos(slot.pos.x - 15, slot.pos.y - 20);

		// Draw zoombini
		_zoombiniAnimation->drawZoombini(screen, z->_traits, pos, 0, 0, lut);
	}

	// Draw unplaced zoombinis in a staging area
	static const Common::Point32 kStagePos(50, 450);
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

		if (!inSlot && _puzzleZoombinis[i]->_puzzleStatus != 0) {
			// Draw in staging area
			const ZoombiniState *z = _puzzleZoombinis[i];
			const Common::Point32 pos(kStagePos.x + (idx % 8) * 25, kStagePos.y);

			// Highlight if selected
			if ((int)i == _selectedZoombini) {
				screen->fillRect(
					Common::Rect(
						static_cast<int16>(pos.x - 2), static_cast<int16>(pos.y - 2),
						static_cast<int16>(pos.x + 22), static_cast<int16>(pos.y + 32)),
					0xFFFF00);
			}

			_zoombiniAnimation->drawZoombini(screen, z->_traits, pos, 0, 0, lut);

			idx++;
		}
	}
}

EventHandleResult PuzzleWaterslide::onLButtonDown(const Common::Point &pos) {
	if (_state != kStateIdle)
		return EventHandleResult::kPassthrough;

	// Check if clicked on a slot
	for (int i = 0; i < _numSlots; i++) {
		if (_slots[i].hitbox.contains(pos)) {
			debug(2, "WaterslidePuzzle: Clicked slot %d", i);
			clickSlot(i);
			return EventHandleResult::kConsumed;
		}
	}

	// Check if clicked on staging area zoombini
	static const Common::Point32 kStagePos(50, 450);
	int idx = 0;

	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		bool inSlot = false;
		for (int s = 0; s < _numSlots; s++) {
			if (_slots[s].zoombiniIdx == (int)i) {
				inSlot = true;
				break;
			}
		}

		if (!inSlot && _puzzleZoombinis[i]->_puzzleStatus != 0) {
			const Common::Point32 stagePos(kStagePos.x + (idx % 8) * 25, kStagePos.y);
			Common::Rect zoomRect(
				static_cast<int16>(stagePos.x), static_cast<int16>(stagePos.y),
				static_cast<int16>(stagePos.x + 20), static_cast<int16>(stagePos.y + 30));

			if (zoomRect.contains(pos)) {
				_selectedZoombini = i;
				debug(2, "WaterslidePuzzle: Selected zoombini %d", i);
				return EventHandleResult::kConsumed;
			}

			idx++;
		}
	}

	debug(3, "WaterslidePuzzle: Click at %d,%d (nothing)", pos.x, pos.y);
	return EventHandleResult::kPassthrough;
}

} // End of namespace Zoombini2
