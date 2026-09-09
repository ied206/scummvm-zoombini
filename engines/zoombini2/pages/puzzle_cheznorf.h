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

#ifndef ZOOMBINI2_PAGES_PUZZLE_CHEZNORF_H
#define ZOOMBINI2_PAGES_PUZZLE_CHEZNORF_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * Chez Norf is the restaurant activity on the left route to Rescue Site II.
 * The internal ChezNorf name identifies the bmp/chez_norf resources.
 */
class ChezNorfPuzzle : public PuzzlePage {
public:
	/** Construct Chez Norf for @p engine. */
	ChezNorfPuzzle(Zoombini2Engine *engine);
	/** Release food, table, and Norf resources. */
	~ChezNorfPuzzle() override;

	/** Load the restaurant and generate the difficulty-selected clue layout. */
	void init() override;
	/** Advance serving and answer-feedback phases. */
	void update() override;
	/** Draw the clue board, tables, meals, Norf, and seated Zoombinis. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Select food or serve the pending order to a table. */
	void handleClick(const Common::Point &pos) override;

	/** Maximum table capacity used by the hardest layout. */
	static const int kMaxTables = 6;
	/** Number of food categories. */
	static const int kNumFoodCategories = 3;
	/** Number of choices within each food category. */
	static const int kFoodsPerCategory = 3;
	/** Total number of selectable food items. */
	static const int kTotalFoods = 9;

	/** Horizontal distance between adjacent tables. */
	static const int kTableSpacing = 85;

private:
	/** Selectable food item IDs grouped by course. */
	enum FoodItem {
		/** No food item is selected. */
		kFoodNone = -1,
		/** Ice cream from the dessert category. */
		kFoodGlace = 0,
		/** Watermelon from the dessert category. */
		kFoodPasteque = 1,
		/** Pie from the dessert category. */
		kFoodTarte = 2,
		/** Fish from the main-dish category. */
		kFoodPoisson = 3,
		/** Salad from the main-dish category. */
		kFoodSalade = 4,
		/** Sandwich from the main-dish category. */
		kFoodSandwitch = 5,
		/** Coffee from the drink category. */
		kFoodCafe = 6,
		/** Milk from the drink category. */
		kFoodLait = 7,
		/** Orange drink from the drink category. */
		kFoodOrange = 8
	};

	/** Runtime phase of the Chez Norf interaction. */
	enum State {
		/** Complete generated table setup. */
		kStateInit,
		/** Wait for food and table selection. */
		kStateIdle,
		/** Play the serving animation. */
		kStateServing,
		/** Place the order at the selected table. */
		kStateFoodServed,
		/** Compare the served order with the table answer. */
		kStateMatching,
		/** Play correct-answer feedback and release the diner. */
		kStateCorrect,
		/** Play wrong-answer feedback. */
		kStateWrong,
		/** Stop accepting input after all diners are released. */
		kStateDone
	};

	/** One restaurant table and its current order. */
	struct TableSlot {
		/** Table position. */
		Common::Point32 position;
		/** Clickable table area. */
		Common::Rect hitbox;
		/** Assigned puzzle-roster index, or `-1` when empty. */
		int zoombiniIdx;
		/** Selected dessert choice, or `-1` when none is present. */
		int foodSlurp;
		/** Selected main-dish choice, or `-1` when none is present. */
		int foodMiam;
		/** Selected drink choice, or `-1` when none is present. */
		int foodGlouglou;
		/** Whether the pending order has been served. */
		bool served;
		/** Whether the assigned Zoombini has been released. */
		bool completed;
	};

	/** Required food-category values for one table. */
	struct FoodAnswer {
		/** Required dessert, or the wildcard value. */
		int slurp;
		/** Required main dish, or the wildcard value. */
		int miam;
		/** Required drink, or the wildcard value. */
		int glouglou;
	};

	/** Load all restaurant graphics. */
	void loadResources();

	/** Generate the food-value permutation used by the current puzzle. */
	void generateFoodVals();
	/** Assign each table answer from the chosen clue template. */
	void setTableAnswersByTemplate();
	/** Build the colored-dot clue grid from the generated answers. */
	void generateFoodGrid();

	/** Return the table at @p pos, or `-1` when none is hit. */
	int findTableAtPos(const Common::Point &pos) const;
	/** Return the food item at @p pos, or @ref ChezNorfPuzzle::kFoodNone. */
	int findFoodAtPos(const Common::Point &pos) const;
	/** Serve the pending course selections to table @p tableIdx. */
	void serveFoodToTable(int tableIdx);
	/** Return whether table @p tableIdx matches its generated answer. */
	bool checkFoodMatch(int tableIdx);
	/** Release the Zoombini seated at table @p tableIdx. */
	void freeZoombini(int tableIdx);
	/** Return the number of table occupants already released. */
	int countFreeZoombinis() const;

	/** Draw the colored-dot clue board and food selectors. */
	void drawFoodBoard(Graphics::ManagedSurface *screen);
	/** Draw the active table layout. */
	void drawTables(Graphics::ManagedSurface *screen);
	/** Draw served orders at their tables. */
	void drawPlates(Graphics::ManagedSurface *screen);
	/** Draw Norf and current answer feedback. */
	void drawNorf(Graphics::ManagedSurface *screen);
	/** Draw every seated Zoombini that has not been released. */
	void drawZoombinis(Graphics::ManagedSurface *screen);

	/** Current interaction phase. */
	State _state;
	/** Number of tables enabled for the selected difficulty. */
	int _numTables;
	/** Difficulty level consumed by this page. */
	int _difficulty;
	/** Number of Zoombinis already released. */
	int _freedCount;
	/** Currently active table, or `-1` when none is active. */
	int _currentTable;
	/** Most recently selected food item. */
	int _selectedFood;
	/** Number of incorrect orders submitted. */
	int _wrongCount;
	/** Difficulty-dependent incorrect-order allowance. */
	int _maxAttempts;
	/** Number of feature attributes represented in the clue layout. */
	int _clueAttrCount;
	/** Current Norf feedback visual state. */
	int _norfState;

	/** Generated item order, grouped into dessert, main-dish, and drink ranges. */
	int _foodVals[9];
	/** Identifier of the generated clue template. */
	int _templateId;

	/** Pending dessert index within its category, or `-1`. */
	int _pendingSlurp;
	/** Pending main-dish index within its category, or `-1`. */
	int _pendingMiam;
	/** Pending drink index within its category, or `-1`. */
	int _pendingGlouglou;

	/** Table runtime state. */
	TableSlot _tables[kMaxTables];
	/** Correct answer corresponding to each table. */
	FoodAnswer _answers[kMaxTables];

	/** Clue symbols indexed by category section, column, and row. */
	int _foodGrid[3][6][4];

	/** Correct-answer feedback symbol. */
	RleBlock *_symbOK;
	/** Incorrect-answer feedback symbol. */
	RleBlock *_symbNO;
	/** Partial-match feedback symbol. */
	RleBlock *_symbMaybe;

	/** Full-size served-order platter. */
	RleBlock *_plato;
	/** Small served-order platter. */
	RleBlock *_platoMini;

	/** Dessert item visuals. */
	RleBlock *_slurpGfx[3];

	/** Main-dish item visuals. */
	RleBlock *_miamGfx[3];

	/** Drink item visuals. */
	RleBlock *_glouglouGfx[3];

	/** Order-panel visuals. */
	RleBlock *_comandeGfx[3];

	/** Default Norf visual. */
	RleBlock *_norfDefault;
	/** Selection highlight visual. */
	RleBlock *_highlightGfx;

	/** Music handle used while Chez Norf is active. */
	int _musicId;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_CHEZNORF_H
