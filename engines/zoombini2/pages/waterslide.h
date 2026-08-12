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

#ifndef ZOOMBINI2_PAGES_WATERSLIDE_H
#define ZOOMBINI2_PAGES_WATERSLIDE_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle.h"

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * WaterslidePuzzle — Trait pair matching puzzle (ID 2).
 *
 * Original: Waterslide__Init_43B490. Object size 0xBE58 (48728 bytes).
 *
 * Mechanics:
 *   - Match pairs of zoombinis that share the same trait value
 *   - Feature axis chosen randomly (hair, eyes, nose, feet)
 *   - Click slots to position zoombinis
 *   - Correct pairs slide down the waterslide together
 *   - Incorrect matches get rejected
 *
 * Resources (bmp/waterslide/):
 *   - TRAITS/1-4.rb: Feature icons for matching indicators
 *   - pipes - blue/grey/red/: Pipe graphics (different colors for state)
 *   - blue fountain.an, little tree.an: Decorative animations
 *   - Mr Valve Master.an: Character animation
 *   - Pastilles*.rb: Match indicators
 *   - Area.bmt: Clickable region definitions
 *
 * Difficulty variants:
 *   - Diff 1 (0x438790): Simple random pairing
 *   - Diff 2 (0x4399D0): Different pairing variant
 *   - Diff 3+ (0x435BA0): Complex bipartite graph matching
 *
 * vtable at 0x480418:
 *   +0: CheckFreeZoombinis (0x43B8E0)
 *   +4: nullsub (blocking model)
 *   +8: Destructor (0x43B350)
 */
class WaterslidePuzzle : public PuzzlePage {
public:
	WaterslidePuzzle(Zoombini2Engine *engine);
	~WaterslidePuzzle() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	// --- Slot Types ---
	enum SlotState {
		kSlotEmpty,
		kSlotOccupied,
		kSlotMatched,
		kSlotRejected
	};

	// --- Puzzle States ---
	enum PuzzleState {
		kStateInit,
		kStateIdle,
		kStateZoombiniMoving,
		kStateCheckingMatch,
		kStateSliding,
		kStateRejecting,
		kStateDone
	};

	// --- Slot Info ---
	struct Slot {
		int x, y;                  // Position
		Common::Rect hitbox;       // Clickable region
		SlotState state;           // Current state
		int zoombiniIdx;           // Zoombini in this slot (-1 if empty)
		int pairSlot;              // Paired slot index (-1 if none)
	};

	// --- Trait Pair ---
	struct TraitPair {
		int zoombiniA;             // First zoombini index
		int zoombiniB;             // Second zoombini index
		int featureAxis;           // Which feature they match on (0-3)
		int sharedValue;           // The shared trait value
		bool matched;              // Have they been correctly placed
	};

	// --- Internal Methods ---
	void loadResources();
	void setupSlots();
	void computePairs();
	void computePairsDiff1();
	void computePairsDiff2();
	void computePairsDiff3();

	void clickSlot(int slotIdx);
	void moveZoombiniToSlot(int zoombiniIdx, int slotIdx);
	bool checkPairMatch(int slotA, int slotB);
	void slideDownPair(int slotA, int slotB);
	void rejectPair(int slotA, int slotB);

	void freeZoombini(int zoombiniIdx);
	int countFreeZoombinis() const;

	void drawPipes(Graphics::ManagedSurface *screen);
	void drawSlots(Graphics::ManagedSurface *screen);
	void drawTraitIndicators(Graphics::ManagedSurface *screen);
	void drawDecorations(Graphics::ManagedSurface *screen);
	void drawZoombinis(Graphics::ManagedSurface *screen);

	// --- State ---
	PuzzleState _state;
	int _freedCount;
	int _selectedZoombini;
	int _selectedSlot;
	uint32 _stateTimer;

	// --- Slots (8 pair positions = 16 slots) ---
	static const int kMaxSlots = 16;
	static const int kMaxPairs = 8;
	Slot _slots[kMaxSlots];
	int _numSlots;

	// --- Trait Pairs ---
	TraitPair _pairs[kMaxPairs];
	int _numPairs;
	int _matchedPairs;

	// --- Graphics Resources ---
	// Trait icons (4 features)
	RleBlock *_traitGfx[4];

	// Pipes (3 colors: blue, grey, red)
	RleBlock *_pipeBlueHoriz;
	RleBlock *_pipeGreyHoriz;
	RleBlock *_pipeRedHoriz;
	RleBlock *_pipeBlueBigone;
	RleBlock *_pipeGreyBigone;
	RleBlock *_pipeRedBigone;

	// Pastilles (match indicators)
	RleBlock *_pastilleBlue;
	RleBlock *_pastilleGrey;

	// Edge graphics
	RleBlock *_edgeNeutre;

	// Decorative animations
	Animation *_blueFountainAnim;
	Animation *_littleTreeAnim;
	Animation *_valveAnim;
	Animation *_cascade1Anim;
	Animation *_cascade2Anim;

	int _musicId;  // BGM: sounds/music/02-BS01.wav
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_WATERSLIDE_H
