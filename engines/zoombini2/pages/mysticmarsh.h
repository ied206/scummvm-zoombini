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

#ifndef ZOOMBINI2_PAGES_MYSTICMARSH_H
#define ZOOMBINI2_PAGES_MYSTICMARSH_H

#include <vector>

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle.h"

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * MysticMarshPuzzle — Swamp symbol grid puzzle (ID 5).
 *
 * Original: MysticMarsh__Init_426770, object size 0xC268, vtable at 0x4803F4.
 *
 * Mechanics:
 *   - 16×12 grid of cells with pipe/symbol tiles
 *   - Zoombinis placed in BubbleCrater slots on the grid
 *   - Symbol tiles define paths that connect craters
 *   - Zoombinis must match traits along connected paths
 *   - Correct matches free the zoombinis through the marsh
 *
 * Grid cell types:
 *   - 0, 1: Empty/border
 *   - 2-59: Symbol tile (from the 60-entry symbol table)
 *   - 60, 61: BubbleCrater (clickable zoombini slot)
 *   - 62: Special marker
 *
 * Symbol categories:
 *   - S_DIV1-4: Straight dividers
 *   - C_DIV1-4: Curved dividers
 *   - *_CY_DIV2: Cycle dividers
 *   - *_TCY_DIV2/3: T-cycle dividers
 *   - TRIGGER1-7: Triggers
 *   - TS_SPOT1-7: Spots
 *   - TOURBI: Whirlpool animation
 *   - EDGE, ENTRY1, ENTRY2: Boundaries
 *   - LD_CONVERGER: Convergence point
 *
 * Difficulty: 4 levels (GenerateRules picks Level1A/1B, 2, 3, or 4).
 * Background: 6 variants selected by difficulty sub-level.
 */
class MysticMarshPuzzle : public PuzzlePage {
public:
	MysticMarshPuzzle(Zoombini2Engine *engine);
	~MysticMarshPuzzle() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

	// === Constants ===
	static const int kGridCols = 16;
	static const int kGridRows = 12;
	static const int kMaxCells = kGridCols * kGridRows;  // 192
	static const int kMaxSlots = 10;   // Max BubbleCrater slots
	static const int kNumSymbols = 60; // Symbol types in table
	static const int kCellSize = 24;   // Original cell stride in bytes

private:

	// === Grid cell ===
	struct GridCell {
		int type;         // Cell type (0=empty, 2-59=symbol, 60/61=crater, 62=marker)
		int symbolIdx;    // Index into _symbolGfx[] for types 2-59
		int x, y;         // Screen position
	};

	// === Slot (BubbleCrater position) ===
	struct Slot {
		int cellCol;      // Grid column
		int cellRow;      // Grid row
		int x, y;         // Screen position
		Common::Rect hitbox;  // Clickable area (43×50)
		int zoombiniIdx;  // Placed zoombini index (-1 = empty)
		bool occupied;
	};

	// === Puzzle state ===
	enum State {
		kStateInit,
		kStateIdle,
		kStateLaunching,       // Zoombini moving through grid
		kStateFreeing,         // Zoombini reached exit correctly
		kStatePopping,         // Zoombini collided and popped
		kStateDone
	};

	// === Active Zoombini state ===
	struct ActiveZoombini {
		int zoombiniIdx;    // Index in _puzzleZoombinis
		int cellCol;        // Current grid column
		int cellRow;        // Current grid row
		int targetX, targetY; // Interpolated screen position
		uint32 moveStartTime;
	};

	// === Resource loading ===
	void loadResources();
	void loadSymbols();
	void loadTraits();
	void loadBubbles();

	// === Grid setup ===
	void setupGrid();
	void buildSlots();
	void generateRules();

	// === Gameplay ===
	void launchZoombini(int entranceIdx);
	void moveZoombini();
	void freeZoombini(int zoombiniIdx);
	int countFreeZoombinis() const;

	// === Drawing ===
	void drawGrid(Graphics::ManagedSurface *screen);
	void drawSlots(Graphics::ManagedSurface *screen);
	void drawZoombinis(Graphics::ManagedSurface *screen);

	// === State ===
	State _state;
	int _freedCount;
	int _selectedZoombini;
	int _difficulty;          // Game difficulty (1-4)
	int _bgIndex;             // Background variant (1-6)
	int _numSlots;
	Slot _slots[kMaxSlots];

	// --- Launch Sequence ---
	int _currentSequenceIdx;   // Which Zoombini in the pack is next to launch
	std::vector<int> _targetSequence; // The correct order of Zoombinis for the exit

	// --- Active Zoombini ---
	ActiveZoombini _activeZ;
	bool _hasActiveZ;

	// === Grid data ===
	GridCell _grid[kMaxCells];
	
	// === Graphics ===
	RleBlock *_craterGfx;
	RleBlock *_symbolGfx[kNumSymbols];       // Symbol sprites
	RleBlock *_traitGfx[4][5];               // Trait icons [feature][value]
	RleBlock *_bubbleGfx[3];                  // 3 bubble types
	Animation *_bubbleCraterAnim;             // BubbleCrater animation
	Animation *_tourbiAnim;                   // Whirlpool animation

	int _musicId;  // BGM: sounds/music/04-BS01.wav
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_MYSTICMARSH_H
