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

#ifndef ZOOMBINI2_PAGES_PUZZLE_MYSTICMARSH_H
#define ZOOMBINI2_PAGES_PUZZLE_MYSTICMARSH_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle_base.h"
#include "zoombini2/scripts.h"

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * Bubble Bumpers (Route3-1)
 *
 * Place Zoombinis in bubble craters to cross the marsh.
 */
class PuzzleMysticMarsh : public PuzzleBase {
public:
	/** Construct Bubble Bumpers for @p vm. */
	PuzzleMysticMarsh(Zoombini2Engine *vm);
	/** Release grid, trait, bubble, and animation resources. */
	~PuzzleMysticMarsh() override;

	/** Load the selected marsh layout and generate its route rules. */
	void init() override;
	/** Advance the active Zoombini through the grid. */
	void onUpdate() override;
	/** Draw the marsh background before grid and Zoombini passes. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw the grid behind active actors. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Leave pointer presses to the common release-driven Zoombini input. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Pick up or place a Zoombini through the common input lifecycle. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Update a Zoombini currently following the pointer. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Apply scheduled Zoombini animation frames after drawing the roster. */
	void onActorsRendered() override;

	/** Number of grid columns. */
	static constexpr int kGridCols = 16;
	/** Number of grid rows. */
	static constexpr int kGridRows = 12;
	/** Total cell capacity of the grid. */
	static constexpr int kMaxCells = kGridCols * kGridRows;
	/** Maximum number of bubble-crater placement slots. */
	static constexpr int kMaxSlots = 10;
	/** Number of symbol visuals available to grid cells. */
	static constexpr int kNumSymbols = 60;
	/** Screen-space size of one grid cell. */
	static constexpr int kCellSize = 24;

private:
	/** Resource paths and formats used by the marsh scene. */
	static constexpr const char *kMusicPath = "#sounds/music/04-BS01.wav";
	static constexpr const char *kBackgroundFormat = "#bmp/mystic_marsh/background%d";
	static constexpr const char *kCraterPath = "bmp/mystic_marsh/crater";
	static constexpr const char *kBubbleCraterPath = "bmp/mystic_marsh/BubbleCrater";
	static constexpr const char *kTourbiPath = "bmp/mystic_marsh/symbols/tourbi_anim";
	static constexpr const char *kTraitFormat = "bmp/mystic_marsh/traits/%d-%d";
	static constexpr const char *kSymbolFormat = "bmp/mystic_marsh/symbols/%s";
	static constexpr const char *kBubbleFormat = "bmp/mystic_marsh/bubble%d";
	static constexpr const char *kPickupZombAnimationPath = "bmp/zombis/pris/pris.anm";
	static constexpr const char *kAreaMaskFormat = "bmp/mystic_marsh/area%d.bmt";
	static constexpr const char *kPlacementSoundPath = "sounds/fx/04-BS03.wav";
	/** Starting positions selected by the six background variants. */
	static const Common::Point32 kStartingPositions[6][8];
	/** Resource names for the symbol slots. */
	static constexpr const char *kSymbolNames[kNumSymbols] = {
		"S_DIV1",
		"S_DIV2",
		"S_DIV3",
		"S_DIV4",
		"C_DIV1",
		"C_DIV2",
		"C_DIV3",
		"C_DIV4",
		"RD_CY_DIV2",
		"DR_CY_DIV2",
		"UD_CY_DIV2",
		"DU_CY_DIV2",
		"LR_CY_DIV2",
		"RL_CY_DIV2",
		"LD_CONVERGER",
		"TRIGGER1",
		"TRIGGER2",
		"TRIGGER3",
		"TRIGGER4",
		"TRIGGER5",
		"TRIGGER6",
		"TRIGGER7",
		"UR_TCY_DIV2",
		"RU_TCY_DIV2",
		"RD_TCY_DIV2",
		"DR_TCY_DIV2",
		"LR_TCY_DIV2",
		"RL_TCY_DIV2",
		"LL_TCY_DIV3",
		"LU_TCY_DIV3",
		"LD_TCY_DIV3",
		"RR_TCY_DIV3",
		"RU_TCY_DIV3",
		"RD_TCY_DIV3",
		"UU_TCY_DIV3",
		"UL_TCY_DIV3",
		"UR_TCY_DIV3",
		"TS_SPOT1",
		"TS_SPOT2",
		"TS_SPOT3",
		"TS_SPOT4",
		"TS_SPOT5",
		"TS_SPOT6",
		"TS_SPOT7",
		"TOURBI",
		"EDGE",
		"ENTRY1",
		"ENTRY2",
		// The remaining resource slots intentionally reuse the straight divider.
		"S_DIV1",
		"S_DIV1",
		"S_DIV1",
		"S_DIV1",
		"S_DIV1",
		"S_DIV1",
		"S_DIV1",
		"S_DIV1",
		"S_DIV1",
		"S_DIV1",
		"S_DIV1",
		"S_DIV1",
	};

	/** One routing-grid cell. */
	struct GridCell {
		/** Cell role identifying empty, symbol, crater, or marker cells. */
		int type = 0;
		/** Index into @ref PuzzleMysticMarsh::_symbolImage for symbol cells. */
		int symbolIdx = 0;
		/** Screen position. */
		Common::Point32 pos = Common::Point32();
	};

	/** One bubble-crater placement position. */
	struct Slot {
		/** Grid column. */
		int cellCol = 0;
		/** Grid row. */
		int cellRow = 0;
		/** Screen position. */
		Common::Point32 pos = Common::Point32();
		/** Zoombini drop area. */
		Common::Rect32 hitbox = Common::Rect32();
	};

	/** Runtime phase of the Bubble Bumpers interaction. */
	enum State {
		/** Complete initial grid setup. */
		kStateInit,
		/** Wait for a Zoombini to be placed in a crater. */
		kStateIdle,
		/** Move the most recently placed Zoombini through the grid. */
		kStateMoving,
		/** Release a Zoombini that reached the correct exit. */
		kStateFreeing,
		/** Play collision feedback for an incorrect route. */
		kStatePopping,
		/** Stop accepting input after completion. */
		kStateDone
	};

	/** Grid position and timing for the currently moving Zoombini. */
	struct ActiveZoombini {
		/** Index into @ref Puzzle::_puzzleZoombinis. */
		int zoombiniIdx = -1;
		/** Current grid column. */
		int cellCol = 0;
		/** Current grid row. */
		int cellRow = 0;
		/** Current screen position. */
		Common::Point32 targetPos = Common::Point32();
		/** Time at which the current cell movement began. */
		uint32 moveStartTime = 0;
	};

	/** Load all marsh resources. */
	void loadResources();
	/** Load grid-symbol visuals. */
	void loadSymbols();
	/** Load Zoombini feature icons. */
	void loadTraits();
	/** Load bubble visuals and animations. */
	void loadBubbles();

	/** Populate the routing grid for the selected background. */
	void setupGrid();
	/** Build placement targets from crater cells. */
	void buildSlots();
	/** Populate the current routing symbols and crater cells. */
	void generateRules();

	/** Apply the marsh-specific grid placement after a common target release. */
	void placeZoombiniAtSlot(int slotIdx, int zoombiniIdx);
	/** Adapt the common target callback to this page. */
	static void slotDropCallback(void *context, int slotIdx, int zoombiniIdx);
	/** Advance the active Zoombini by one routing step. */
	void moveZoombini();
	/** Release puzzle-roster entry @p zoombiniIdx. */
	void freeZoombini(int zoombiniIdx);
	/** Return the number of puzzle-roster entries already released. */
	int countFreeZoombinis() const;

	/** Draw symbols and markers in the routing grid. */
	void drawGrid(ManagedSurface32 *screen);
	/** Draw waiting and active Zoombinis. */
	void onRenderActors(ManagedSurface32 *screen) override;

	/** Current interaction phase. */
	State _state = kStateInit;
	/** Number of Zoombinis already released. */
	int _freedCount = 0;
	/** Level in the range one through four. */
	int _level = 1;
	/** Selected background variant. */
	int _bgIndex = 1;
	/** Number of crater slots populated in @ref PuzzleMysticMarsh::_slots. */
	int _numSlots = 0;
	/** Bubble-crater launch slots. */
	Slot _slots[kMaxSlots] = {};
	/** Shared Zoombini input targets linked to the crater slots. */
	Common::Array<ZoombiniDropTarget> _dropTargets;
	/** Deadline after which all crater targets accept another placement. */
	uint32 _slotUnlockTime = 0;

	/** Runtime state of the currently moving Zoombini. */
	ActiveZoombini _activeZ;
	/** Crater slot playing the current placement animation. */
	int _activeSlotIdx = -1;
	/** Whether @ref PuzzleMysticMarsh::_activeZ is currently valid. */
	bool _hasActiveZ = false;

	/** Routing-grid cells stored in row-major order. */
	GridCell _grid[kMaxCells] = {};

	/** Bubble-crater visual. */
	RleBlock *_craterImage = nullptr;
	/** Grid-symbol visuals. */
	RleBlock *_symbolImage[kNumSymbols] = {};
	/** Feature icons indexed by feature and value. */
	RleBlock *_traitImage[4][5] = {};
	/** Bubble visuals indexed by bubble type. */
	RleBlock *_bubbleImage[3] = {};
	/** Bubble-crater animation. */
	Animation *_bubbleCraterAnim = nullptr;
	/** Whirlpool animation. */
	Animation *_tourbiAnim = nullptr;
	/** Borrowed pickup animation for the common Zoombini input. */
	const ZoombiniAnimation *_pickupZombAnimation = nullptr;

	/** Crater placement sound handle. */
	int _placementSoundId = -1;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_MYSTICMARSH_H
