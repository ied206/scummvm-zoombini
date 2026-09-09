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

#ifndef ZOOMBINI2_PAGES_PUZZLE_WATERSLIDE_H
#define ZOOMBINI2_PAGES_PUZZLE_WATERSLIDE_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * Pipes of Paloo pairs Zoombinis that share a selected visible feature.
 *
 * The generated pairs and matching strategy depend on difficulty. Correctly
 * paired Zoombinis slide away together, while incorrect placements are returned.
 */
class WaterslidePuzzle : public PuzzlePage {
public:
	/** Construct Pipes of Paloo for @p engine. */
	WaterslidePuzzle(Zoombini2Engine *engine);
	/** Release pipe, indicator, and decoration resources. */
	~WaterslidePuzzle() override;

	/** Load the pipe layout, build slots, and generate trait pairs. */
	void init() override;
	/** Advance movement, match feedback, sliding, and rejection phases. */
	void update() override;
	/** Draw pipes, slots, trait indicators, decorations, and Zoombinis. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Move the selected Zoombini into the clicked slot. */
	void handleClick(const Common::Point &pos) override;

private:
	/** Occupancy and feedback state of one pipe slot. */
	enum SlotState {
		/** No Zoombini occupies this slot. */
		kSlotEmpty,
		/** A Zoombini is waiting for a partner. */
		kSlotOccupied,
		/** The occupants formed a correct pair. */
		kSlotMatched,
		/** The occupants formed an incorrect pair. */
		kSlotRejected
	};

	/** Runtime phase of the Pipes of Paloo interaction. */
	enum PuzzleState {
		/** Complete initial pair and slot setup. */
		kStateInit,
		/** Wait for a Zoombini or slot selection. */
		kStateIdle,
		/** Move a selected Zoombini into a slot. */
		kStateZoombiniMoving,
		/** Compare both occupants of a pair of slots. */
		kStateCheckingMatch,
		/** Move a correct pair down the waterslide. */
		kStateSliding,
		/** Return an incorrect pair from its slots. */
		kStateRejecting,
		/** Stop accepting input after completion. */
		kStateDone
	};

	/** One clickable pipe slot and its current occupant. */
	struct Slot {
		/** Screen position. */
		Common::Point32 position;
		/** Clickable area. */
		Common::Rect hitbox;
		/** Current occupancy or feedback state. */
		SlotState state;
		/** Puzzle-roster index in this slot, or `-1` when empty. */
		int zoombiniIdx;
		/** Partner slot index, or `-1` when unpaired. */
		int pairSlot;
	};

	/** Correct pairing and its shared feature value. */
	struct TraitPair {
		/** First puzzle-roster index. */
		int zoombiniA;
		/** Second puzzle-roster index. */
		int zoombiniB;
		/** Visible feature index shared by the pair. */
		int featureAxis;
		/** Feature value shared by the pair. */
		int sharedValue;
		/** Whether this pair has been placed correctly. */
		bool matched;
	};

	/** Load pipe, indicator, and decoration resources. */
	void loadResources();
	/** Return visible feature @p axis from @p zoombini. */
	static byte getFeature(const ZoombiniState *zoombini, int axis);
	/** Initialize slot geometry and partner relationships. */
	void setupSlots();
	/** Select the difficulty-specific pairing algorithm. */
	void computePairs();
	/** Generate the difficulty-one pair layout. */
	void computePairsDiff1();
	/** Generate the difficulty-two pair layout. */
	void computePairsDiff2();
	/** Generate the upper-difficulty pair layout. */
	void computePairsDiff3();

	/** Dispatch a click on slot @p slotIdx. */
	void clickSlot(int slotIdx);
	/** Place puzzle-roster entry @p zoombiniIdx into slot @p slotIdx. */
	void moveZoombiniToSlot(int zoombiniIdx, int slotIdx);
	/** Return whether the occupants of @p slotA and @p slotB form a generated pair. */
	bool checkPairMatch(int slotA, int slotB);
	/** Release and animate the correct pair in @p slotA and @p slotB. */
	void slideDownPair(int slotA, int slotB);
	/** Reject the incorrect pair in @p slotA and @p slotB. */
	void rejectPair(int slotA, int slotB);

	/** Release puzzle-roster entry @p zoombiniIdx. */
	void freeZoombini(int zoombiniIdx);
	/** Return the number of puzzle-roster entries already released. */
	int countFreeZoombinis() const;

	/** Draw pipe segments using their current feedback colors. */
	void drawPipes(Graphics::ManagedSurface *screen);
	/** Draw every active slot and occupant marker. */
	void drawSlots(Graphics::ManagedSurface *screen);
	/** Draw the feature icons for generated pairs. */
	void drawTraitIndicators(Graphics::ManagedSurface *screen);
	/** Draw the fountain, tree, valve, and cascades. */
	void drawDecorations(Graphics::ManagedSurface *screen);
	/** Draw waiting and placed Zoombinis. */
	void drawZoombinis(Graphics::ManagedSurface *screen);

	/** Current interaction phase. */
	PuzzleState _state;
	/** Number of Zoombinis already released. */
	int _freedCount;
	/** Selected puzzle-roster index, or `-1` when none is selected. */
	int _selectedZoombini;
	/** Selected slot index, or `-1` when none is selected. */
	int _selectedSlot;
	/** Time at which the current phase began. */
	uint32 _stateTimer;

	/** Maximum number of pipe slots. */
	static const int kMaxSlots = 16;
	/** Maximum number of generated pairs. */
	static const int kMaxPairs = 8;
	/** Pipe-slot runtime state. */
	Slot _slots[kMaxSlots];
	/** Number of active entries in @ref WaterslidePuzzle::_slots. */
	int _numSlots;

	/** Generated correct pair definitions. */
	TraitPair _pairs[kMaxPairs];
	/** Number of active entries in @ref WaterslidePuzzle::_pairs. */
	int _numPairs;
	/** Number of generated pairs already matched. */
	int _matchedPairs;

	/** Trait indicators indexed by visible feature. */
	RleBlock *_traitGfx[4];

	/** Blue horizontal pipe segment. */
	RleBlock *_pipeBlueHoriz;
	/** Gray horizontal pipe segment. */
	RleBlock *_pipeGreyHoriz;
	/** Red horizontal pipe segment. */
	RleBlock *_pipeRedHoriz;
	/** Blue large pipe segment. */
	RleBlock *_pipeBlueBigone;
	/** Gray large pipe segment. */
	RleBlock *_pipeGreyBigone;
	/** Red large pipe segment. */
	RleBlock *_pipeRedBigone;

	/** Blue match indicator. */
	RleBlock *_pastilleBlue;
	/** Gray inactive match indicator. */
	RleBlock *_pastilleGrey;

	/** Neutral edge visual. */
	RleBlock *_edgeNeutre;

	/** Fountain decoration animation. */
	Animation *_blueFountainAnim;
	/** Tree decoration animation. */
	Animation *_littleTreeAnim;
	/** Valve Master animation. */
	Animation *_valveAnim;
	/** First cascade animation. */
	Animation *_cascade1Anim;
	/** Second cascade animation. */
	Animation *_cascade2Anim;

	/** Music handle used while Pipes of Paloo is active. */
	int _musicId;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_WATERSLIDE_H
