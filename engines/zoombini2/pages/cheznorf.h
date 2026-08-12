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

#ifndef ZOOMBINI2_PAGES_CHEZNORF_H
#define ZOOMBINI2_PAGES_CHEZNORF_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle.h"

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * ChezNorfPuzzle — Restaurant food-matching puzzle (ID 8).
 *
 * Original: ChezNorf__Init_412C30, object size 0x33C (828 bytes).
 * vtable at 0x4803AC. Non-blocking model (per-frame Update).
 *
 * Mechanics:
 *   - Norf character runs a restaurant with 4-6 food tables
 *   - Food board on the left shows a 3-section grid of colored dots
 *   - Each zoombini needs the correct combination of 3 food items
 *   - Food categories: Slurp (dessert), Miam (main dish), Glouglou (drink)
 *   - Each category has 3 options = 9 total food items
 *   - Player drags food to zoombini plates at tables
 *   - Norf gives OK/NO/MAYBE feedback via animations and speech
 *   - Correct matches free the zoombini; wrong matches penalize
 *
 * Difficulty:
 *   - Mode 1-2: 4 tables, Mode 3: 6 tables
 *   - Each difficulty has 4 random clue-template variants
 *   - Tolerance for wrong guesses varies by difficulty
 *
 * Resource paths (from IDA):
 *   bmp/chez_norf/baquegund.bmp (background - loaded by base)
 *   bmp/chez_norf/symb_OK.rb, symb_NO.rb, symb_MAYBE.rb
 *   bmp/chez_norf/plato.rb, plato2.rb, plato_mini.rb
 *   bmp/chez_norf/slurp_{glace,pasteque,tarte}.rb
 *   bmp/chez_norf/miam_{poisson,salade,sandwitch}.rb
 *   bmp/chez_norf/glouglou_{cafe,lait,orange}.rb
 *   bmp/chez_norf/comande{1,2,3}.rb
 *   bmp/chez_norf/highlight.rb
 *   bmp/chez_norf/norf/norfDeBaz.rb
 *   bmp/chez_norf/norf/norf{1,3,32,4,5,6,7}.an
 *   bmp/chez_norf/norf/cask/{1-6}/cask{1-7,32,2baz}.{an,rb}
 */
class ChezNorfPuzzle : public PuzzlePage {
public:
	ChezNorfPuzzle(Zoombini2Engine *engine);
	~ChezNorfPuzzle() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

	// === Constants ===
	static const int kMaxTables = 6;
	static const int kNumFoodCategories = 3;  // Slurp, Miam, Glouglou
	static const int kFoodsPerCategory = 3;   // 3 items per category
	static const int kTotalFoods = 9;         // 3 × 3

	// Table spacing from IDA Init
	static const int kTableStartX = 205;
	static const int kTableSpacing = 85;
	static const int kTablePlateY = 500;

private:
	// === Food item IDs ===
	enum FoodItem {
		kFoodNone = -1,
		// Slurp (dessert)
		kFoodGlace = 0,
		kFoodPasteque = 1,
		kFoodTarte = 2,
		// Miam (main dish)
		kFoodPoisson = 3,
		kFoodSalade = 4,
		kFoodSandwitch = 5,
		// Glouglou (drink)
		kFoodCafe = 6,
		kFoodLait = 7,
		kFoodOrange = 8
	};

	// === Puzzle state machine ===
	enum State {
		kStateInit,
		kStateIdle,          // Waiting for player input
		kStateServing,       // Norf serving food animation
		kStateFoodServed,    // Food placed, checking match
		kStateMatching,      // Checking food answer
		kStateCorrect,       // Correct match — exit animation
		kStateWrong,         // Wrong match — reject
		kStateDone           // All zoombinis freed
	};

	// === Table slot ===
	struct TableSlot {
		int x, y;                  // Table position
		Common::Rect hitbox;       // Clickable area
		int zoombiniIdx;           // Assigned zoombini (-1 = empty)
		int foodSlurp;             // Selected slurp item (-1 = none)
		int foodMiam;              // Selected miam item (-1 = none)
		int foodGlouglou;          // Selected glouglou item (-1 = none)
		bool served;               // Food has been served
		bool completed;            // Zoombini freed from this slot
	};

	// === Correct answer for a table ===
	struct FoodAnswer {
		int slurp;     // Correct slurp (0-2, 9=wildcard)
		int miam;      // Correct miam (0-2, 9=wildcard)
		int glouglou;  // Correct glouglou (0-2, 9=wildcard)
	};

	// === Resource loading ===
	void loadResources();

	// === Grid / clue generation ===
	void generateFoodVals();
	void setTableAnswersByTemplate();
	void generateFoodGrid();

	// === Gameplay ===
	int findTableAtPos(const Common::Point &pos) const;
	int findFoodAtPos(const Common::Point &pos) const;
	void serveFoodToTable(int tableIdx);
	bool checkFoodMatch(int tableIdx);
	void freeZoombini(int tableIdx);
	int countFreeZoombinis() const;

	// === Drawing ===
	void drawFoodBoard(Graphics::ManagedSurface *screen);
	void drawTables(Graphics::ManagedSurface *screen);
	void drawPlates(Graphics::ManagedSurface *screen);
	void drawNorf(Graphics::ManagedSurface *screen);
	void drawZoombinis(Graphics::ManagedSurface *screen);

	// === State ===
	State _state;
	int _numTables;              // 4 or 6 depending on difficulty
	int _difficulty;             // Game difficulty (1-3)
	int _freedCount;             // Number of freed zoombinis
	int _currentTable;           // Currently active table (-1 = none)
	int _selectedFood;           // Currently selected food item (legacy, replaced by pending)
	int _wrongCount;             // Number of wrong guesses
	int _maxAttempts;            // Max attempts (passes) per difficulty
	int _clueAttrCount;          // Number of clue attributes shown
	int _norfState;              // Current Norf animation state

	// === Generated food values (from GenerateClueLayout_453C80) ===
	// Indices: [0..2]=slurp (0-2), [3..5]=miam (3-5), [6..8]=glouglou (6-8)
	int _foodVals[9];
	int _templateId;             // Chosen template ID (11-14, 21-24, 31-34)

	// === Player's pending food selection (one per category) ===
	int _pendingSlurp;           // Selected slurp item index within slurp foods (0-2), or -1
	int _pendingMiam;            // Selected miam item index within miam foods (0-2), or -1
	int _pendingGlouglou;        // Selected glouglou item index within glouglou foods (0-2), or -1

	// === Table data ===
	TableSlot _tables[kMaxTables];
	FoodAnswer _answers[kMaxTables];

	// === Food board grid (3 sections of dots) ===
	// Grid values: 0=empty, 1/2/3 = dot colors (mapped to symb types)
	int _foodGrid[3][6][4];      // [section][col][row] — from DrawBoard analysis

	// === Graphics resources ===
	// Food symbols (feedback)
	RleBlock *_symbOK;
	RleBlock *_symbNO;
	RleBlock *_symbMaybe;

	// Plates
	RleBlock *_plato;            // Full platter
	RleBlock *_platoMini;        // Mini platter

	// Slurp category (dessert) - 3 items
	RleBlock *_slurpGfx[3];     // glace, pasteque, tarte

	// Miam category (main dish) - 3 items
	RleBlock *_miamGfx[3];      // poisson, salade, sandwitch

	// Glouglou category (drink) - 3 items
	RleBlock *_glouglouGfx[3];  // cafe, lait, orange

	// Command/order backgrounds
	RleBlock *_comandeGfx[3];   // comande1, comande2, comande3

	// Norf character
	RleBlock *_norfDefault;      // norfDeBaz
	RleBlock *_highlightGfx;     // highlight

	int _musicId;  // BGM: sounds/music/07-BB02.wav
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_CHEZNORF_H
