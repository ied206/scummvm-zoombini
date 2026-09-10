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

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * Bubble Bumpers (Route3-1)
 *
 * Choose each Zoombini's entrance and launch order to cross the marsh.
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
	/** Draw the grid, launch slots, and Zoombinis. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw the grid behind active actors. */
	void onRenderScene(ManagedSurface32 *screen) override;
	/** Launch the next Zoombini from the selected entrance. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;

	/** Number of grid columns. */
	static const int kGridCols = 16;
	/** Number of grid rows. */
	static const int kGridRows = 12;
	/** Total cell capacity of the grid. */
	static const int kMaxCells = kGridCols * kGridRows;
	/** Maximum number of bubble-crater launch slots. */
	static const int kMaxSlots = 10;
	/** Number of symbol visuals available to grid cells. */
	static const int kNumSymbols = 60;
	/** Screen-space size of one grid cell. */
	static const int kCellSize = 24;

private:
	/** One routing-grid cell. */
	struct GridCell {
		/** Cell role identifying empty, symbol, crater, or marker cells. */
		int type;
		/** Index into @ref PuzzleMysticMarsh::_symbolImage for symbol cells. */
		int symbolIdx;
		/** Screen position. */
		Common::Point32 pos;
	};

	/** One bubble-crater launch position. */
	struct Slot {
		/** Grid column. */
		int cellCol;
		/** Grid row. */
		int cellRow;
		/** Screen position. */
		Common::Point32 pos;
		/** Clickable launch area. */
		Common::Rect hitbox;
		/** Assigned puzzle-roster index, or `-1` when empty. */
		int zoombiniIdx;
		/** Whether a Zoombini currently occupies this slot. */
		bool occupied;
	};

	/** Runtime phase of the Bubble Bumpers interaction. */
	enum State {
		/** Complete initial grid setup. */
		kStateInit,
		/** Wait for an entrance selection. */
		kStateIdle,
		/** Move the active Zoombini through the grid. */
		kStateLaunching,
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
		int zoombiniIdx;
		/** Current grid column. */
		int cellCol;
		/** Current grid row. */
		int cellRow;
		/** Interpolated screen position. */
		Common::Point32 targetPos;
		/** Time at which the current cell movement began. */
		uint32 moveStartTime;
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
	/** Build launch slots from crater cells. */
	void buildSlots();
	/** Generate the correct entrance sequence. */
	void generateRules();

	/** Launch the next Zoombini from entrance @p entranceIdx. */
	void launchZoombini(int entranceIdx);
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
	State _state;
	/** Number of Zoombinis already released. */
	int _freedCount;
	/** Selected puzzle-roster index, or `-1` when none is selected. */
	int _selectedZoombini;
	/** Level in the range one through four. */
	int _level;
	/** Selected background variant. */
	int _bgIndex;
	/** Number of launch slots populated in @ref PuzzleMysticMarsh::_slots. */
	int _numSlots;
	/** Bubble-crater launch slots. */
	Slot _slots[kMaxSlots];

	/** Index of the next Zoombini in the generated launch sequence. */
	int _currentSequenceIdx;
	/** Correct entrance index for each Zoombini in sequence. */
	Common::Array<int> _targetSequence;

	/** Runtime state of the currently moving Zoombini. */
	ActiveZoombini _activeZ;
	/** Whether @ref PuzzleMysticMarsh::_activeZ is currently valid. */
	bool _hasActiveZ;

	/** Routing-grid cells stored in row-major order. */
	GridCell _grid[kMaxCells];

	/** Bubble-crater visual. */
	RleBlock *_craterImage;
	/** Grid-symbol visuals. */
	RleBlock *_symbolImage[kNumSymbols];
	/** Feature icons indexed by feature and value. */
	RleBlock *_traitImage[4][5];
	/** Bubble visuals indexed by bubble type. */
	RleBlock *_bubbleImage[3];
	/** Bubble-crater animation. */
	Animation *_bubbleCraterAnim;
	/** Whirlpool animation. */
	Animation *_tourbiAnim;

	/** Music handle used while Bubble Bumpers is active. */
	int _musicId;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_MYSTICMARSH_H
