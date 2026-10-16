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

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class Animation;
class AnimationRunner;
struct PathObject;

/** Prepare meals from the Norfs' spoken clues and release the waiting party. */
class PuzzleChezNorf : public PuzzleBase {
public:
	/** Construct the Chez Norf service puzzle for @p vm. */
	PuzzleChezNorf(Zoombini2Engine *vm);
	/** Release active tray paths and animation runners. */
	~PuzzleChezNorf() override;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	/** Return the 1-based route containing this puzzle. */
	int getRouteNumber() const override { return kRouteNumber; }
	/** Return the 1-based puzzle number within its route. */
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }
	/** Select a level layout, load art, and arrange the waiting Zoombinis. */
	void init() override;
	/** Begin or continue the click selected at @p pos. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Commit a food, tray, Norf, or note selection released at @p pos. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Update the food and tray cursor feedback at @p pos. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Return whether at least one released cohort may leave the restaurant. */
	bool canUseGoButton() const override;
	/** Leave immediately or play retreat speech before returning to the map. */
	bool onGoButtonPressed() override;
	/** Keep sidebar controls unavailable while a meal selection or service sequence is active. */
	bool blocksSidebarInteraction() const override;
	/** Describe the active Norfs' meal requirements for the puzzle console. */
	Common::String debugGetAnswer() const override;
	/** Report the remaining tray opportunities as the puzzle's chances. */
	PuzzleChanceInfo debugGetChances() const override;
	/** Return whether the console may change tray opportunities in the ready phase. */
	bool debugCanSetChances() const override;
	/** Synchronize tray availability with console-selected remaining opportunities. */
	bool debugSetChances(int remaining) override;
	/** Describe available trays and waiting Zoombinis for the puzzle console. */
	Common::String debugGetChanceDetails() const override;

private:
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = "Chez Norf";
	/** 1-based route number used by the puzzle console. */
	static constexpr int kRouteNumber = 2;
	/** 1-based puzzle number within the route used by the puzzle console. */
	static constexpr int kRoutePuzzleIdx = 2;
	/** Chez Norf dining-room background loaded into the first page layer. */
	static constexpr const char *kBackgroundPath = "#bmp/chez_norf/baquegund";
	/** Path format for the food, note-symbol, and level-specific order-panel RLE sprites. */
	static constexpr const char *kFoodFormat = "bmp/chez_norf/%s";
	/** Food resource names ordered as three mains, three drinks, then three desserts. */
	static constexpr const char *kFoodNames[9] = {
		"miam_sandwitch",  ///< MainDish - Sandwitch
		"miam_poisson",    ///< MainDish - Fish
		"miam_salade",     ///< MainDish - Salad
		"glouglou_cafe",   ///< Drink - Coffee
		"glouglou_orange", ///< Drink - OrangeJuice
		"glouglou_lait",   ///< Drink - Milk
		"slurp_tarte",     ///< Desert - FruitPie
		"slurp_pasteque",  ///< Desert - Watermelon
		"slurp_glace",     ///< Desert - IceCream
	};
	/** Player note sprites for confirmed, rejected, and uncertain food deductions. */
	static constexpr const char *kMemoSymbolNames[3] = {
		"symb_OK",    ///< 'V'
		"symb_NO",    ///< 'X'
		"symb_MAYBE", ///< '?'
	};
	/** Order-panel RLE sprites indexed by Level 1, 2, and 3. */
	static constexpr const char *kPanelNames[3] = {
		"comande3", ///< Level 1.
		"COMANDE1", ///< Level 2.
		"comande2", ///< Level 3.
	};
	/** Full-size RLE tray drawn for meal assembly and service on Levels 2 and 3. */
	static constexpr const char *kMealTrayPath = "bmp/chez_norf/plato";
	/** Compact RLE tray drawn for meal assembly and service on Level 1. */
	static constexpr const char *kSmallMealTrayPath = "bmp/chez_norf/plato2";
	/** RLE hover or selection highlight for food, trays, and Norfs. */
	static constexpr const char *kHighlightPath = "bmp/chez_norf/highlight";
	/** Static RLE Norf body art shown beneath its table-specific motion animation. */
	static constexpr const char *kNorfPath = "bmp/chez_norf/norf/norfDeBaz";
	/** Path format for the static table-specific Norf cap sprites. */
	static constexpr const char *kCapPath = "bmp/chez_norf/norf/cask/%d/cask2baz";
	/** Animation path format for the shared Norf body motions. */
	static constexpr const char *kNorfAnimationFormat = "bmp/chez_norf/norf/norf%d.an";
	/** Animation path format for each table's matching Norf cap motions. */
	static constexpr const char *kCapAnimationFormat = "bmp/chez_norf/norf/cask/%d/cask%d.an";
	/** PAT path format for Zoombinis leaving after a successful meal service. */
	static constexpr const char *kExitFormat = "bmp/chez_norf/out%d.pat";
	/** PAT route for the waiter who leaves when the player retreats to the map. */
	static constexpr const char *kWaiterExitPath = "bmp/chez_norf/out_special.pat";
	/** Looping restaurant soundtrack used while the puzzle is active. */
	static constexpr const char *kMusicPath = "#sounds/music/07-BB02.wav";
	/** Path format for short food-service and interaction sound effects. */
	static constexpr const char *kSoundFormat = "sounds/fx/07-BS%02d.wav";
	/** Path format for Norf clue speech, selected by puzzle level and generated meal layout. */
	static constexpr const char *kClueFormat = "sounds/7-N%d-%d";
	/** Path format for a Norf's rejected, final-rejection, or accepted meal response. */
	static constexpr const char *kFeedbackFormat = "sounds/7-N%d-G%02d.wav";
	/** Shared completion speech queued when the rescued party departs the restaurant. */
	static constexpr const char *kCompleteSpeechPath = "sounds/CZN31.wav";
	/** Retreat speech played before a journey return to the map. */
	static constexpr const char *kRetreatSpeechPath = "sounds/DW-Cave.wav";

	/** Indices in the paired body/cap animation bank. */
	enum Motion {
		/** Refuse an incorrect meal. */
		kReject00 = 0,
		/** Return to rest after dropping a rejected tray. */
		kRejectReturn01 = 1,
		/** Send the waiter away after the player leaves. */
		kDismiss02 = 2,
		/** Accept a correct meal. */
		kAccept03 = 3,
		/** Idle table gesture. */
		kIdle04 = 4,
		/** First or second direction clue, pointing right. */
		kPointRight05 = 5,
		/** First or second direction clue, pointing left. */
		kPointLeft06 = 6
	};
	/** Success and rejection callbacks advance independently of the flying tray. */
	enum Phase {
		/** Accept food, tray, Norf, and note input. */
		kReady00 = 0,
		/** Tray travels from its source table to the Norf. */
		kThrow01 = 1,
		/** Tray descends to the selected Norf's table. */
		kLand02 = 2,
		/** Rejected tray falls back toward the lower tables. */
		kRejectFall03 = 3,
		/** Wait for accept or rejection animation feedback. */
		kFeedback04 = 4,
		/** Move the released Zoombini cohort to the exit. */
		kRelease05 = 5,
		/** Move the waiter out after the party retreats. */
		kDismiss06 = 6,
		/** All allowed attempts have resolved. */
		kFinished07 = 7
	};
	/** The three food selections carried by a tray. */
	struct Meal {
		/** Main dish, drink, and dessert values; -1 means no selection. */
		int food[3] = {
			-1,
			-1,
			-1,
		}; // Main dish, drink, dessert.
	};
	/** A seated Norf's rule, spoken clues, feedback, and service state. */
	struct Norf {
		/** Required meal for acceptance. */
		Meal answer;
		/** Spoken food clues, with 9 denoting an omitted category. */
		int clue[3] = {
			9,
			9,
			9,
		};
		/** One or two directional gesture indices shown after the spoken clue. */
		int gesture[2] = {};
		/** Whether this Norf has accepted a meal and released a cohort. */
		bool served = false;
		/** Whether the accepted-tray motion is awaiting its completion callback. */
		bool acceptedTray = false;
		/** Whether the rejected-tray motion is awaiting its completion callback. */
		bool rejectedTray = false;
		/** Last accepted meal retained for feedback rendering. */
		Meal accepted;
		/** Last rejected meal retained for feedback rendering. */
		Meal rejected;
	};
	/** One level-layout variant encoded in food, clue, and gesture indices. */
	struct Layout {
		/** Required meals indexed by Norf and food category. */
		byte answers[6][3];
		/** Food clue codes indexed by Norf and the three clue slots. */
		byte clues[6][3];
		/** Directional gesture codes indexed by Norf and gesture slot. */
		byte gestures[6][2];
	};
	/** Four release-matched variants for each of the three selectable levels. */
	static constexpr Layout kLayouts[12] = {
		{
			// Layout 11
			{{0, 3, 9}, {1, 3, 9}, {2, 5, 9}, {2, 3, 9}},
			{{0, 9, 9}, {2, 9, 9}, {3, 9, 9}, {4, 9, 9}},
			{{0, 0}, {1, 0}, {0, 0}, {0, 0}},
		},
		{
			// Layout 12
			{{0, 5, 9}, {2, 3, 9}, {1, 5, 9}, {1, 4, 9}},
			{{0, 9, 9}, {5, 9, 9}, {2, 9, 9}, {4, 9, 9}},
			{{0, 0}, {0, 0}, {2, 0}, {0, 0}},
		},
		{
			// Layout 13
			{{1, 5, 9}, {2, 5, 9}, {1, 5, 9}, {2, 4, 9}},
			{{3, 0, 9}, {1, 9, 9}, {9, 9, 9}, {5, 9, 9}},
			{{0, 0}, {0, 0}, {0, 0}, {0, 0}},
		},
		{
			// Layout 14
			{{0, 5, 9}, {1, 5, 9}, {1, 5, 9}, {2, 5, 9}},
			{{2, 9, 9}, {2, 0, 9}, {1, 9, 9}, {3, 4, 9}},
			{{0, 0}, {0, 0}, {0, 0}, {0, 0}},
		},
		{
			// Layout 21
			{{0, 4, 8}, {1, 4, 6}, {0, 5, 7}, {1, 3, 7}},
			{{0, 8, 9}, {7, 6, 9}, {5, 1, 9}, {3, 4, 9}},
			{{0, 0}, {1, 0}, {3, 0}, {0, 0}},
		},
		{
			// Layout 22
			{{0, 5, 7}, {1, 5, 7}, {2, 3, 6}, {1, 5, 8}},
			{{2, 9, 9}, {1, 4, 9}, {5, 7, 9}, {1, 8, 9}},
			{{0, 0}, {0, 0}, {2, 0}, {0, 0}},
		},
		{
			// Layout 23
			{{2, 3, 8}, {2, 5, 8}, {0, 5, 7}, {0, 4, 8}},
			{{0, 2, 9}, {7, 4, 9}, {6, 5, 9}, {0, 7, 9}},
			{{0, 0}, {1, 0}, {0, 0}, {0, 0}},
		},
		{
			// Layout 24
			{{2, 5, 7}, {2, 3, 8}, {1, 5, 8}, {2, 3, 6}},
			{{4, 9, 9}, {2, 9, 9}, {1, 8, 9}, {0, 6, 5}},
			{{0, 0}, {0, 0}, {0, 0}, {0, 0}},
		},
		{
			// Layout 31
			{{0, 3, 6}, {1, 5, 8}, {2, 3, 8}, {1, 4, 6}, {0, 5, 8}, {1, 5, 8}},
			{{8, 8, 4}, {1, 3, 9}, {7, 2, 9}, {4, 1, 9}, {1, 9, 9}, {5, 9, 9}},
			{{0, 0}, {3, 0}, {0, 0}, {0, 0}, {3, 0}, {3, 0}},
		},
		{
			// Layout 32
			{{2, 4, 7}, {1, 3, 6}, {2, 5, 7}, {2, 4, 7}, {0, 3, 6}, {1, 4, 6}},
			{{2, 7, 8}, {6, 6, 9}, {4, 5, 9}, {0, 9, 9}, {2, 9, 9}, {3, 6, 9}},
			{{0, 0}, {0, 0}, {0, 0}, {0, 0}, {2, 0}, {0, 0}},
		},
		{
			// Layout 33
			{{0, 3, 8}, {0, 5, 6}, {2, 5, 7}, {2, 3, 6}, {2, 3, 6}, {0, 3, 6}},
			{{8, 5, 9}, {9, 9, 9}, {4, 9, 9}, {2, 1, 9}, {0, 9, 9}, {7, 9, 9}},
			{{0, 0}, {0, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}},
		},
		{
			// Layout 34
			{{1, 3, 8}, {2, 4, 8}, {2, 4, 7}, {1, 5, 6}, {1, 3, 6}, {1, 5, 6}},
			{{2, 9, 9}, {3, 1, 9}, {6, 8, 9}, {6, 1, 9}, {5, 9, 9}, {4, 9, 9}},
			{{0, 0}, {0, 0}, {1, 2}, {0, 0}, {0, 0}, {0, 0}},
		},
	};
	struct L4Layout {
		/** Food clue codes indexed by Norf and the three clue slots. */
		byte clues[6][3];
		/** Directional gesture codes indexed by Norf and gesture slot. */
		byte gestures[6][2];
		/** First branch index in the shared level 4 answer table. */
		int firstAnswer;
		/** Number of level 4 answer branches available to this layout. */
		int answerCount;
	};
	/** Four restored level 4 layouts and their answer-table slices. */
	static constexpr L4Layout kL4Layouts[4] = {
		{
			// Layout 41
			{
				{6, 8, 9},
				{1, 9, 9},
				{4, 5, 9},
				{7, 9, 9},
				{3, 9, 9},
				{9, 9, 9},
			},
			{
				{0, 0},
				{0, 0},
				{2, 1},
				{1, 0},
				{0, 0},
				{0, 0},
			},
			0,
			12,
		},
		{
			// Layout 42
			{
				{1, 9, 9},
				{6, 9, 9},
				{4, 9, 9},
				{7, 9, 9},
				{0, 9, 9},
				{2, 9, 9},
			},
			{
				{0, 0},
				{0, 0},
				{0, 0},
				{0, 0},
				{0, 0},
				{0, 0},
			},
			12,
			4,
		},
		{
			// Layout 43
			{
				{9, 9, 9},
				{0, 9, 9},
				{7, 9, 9},
				{5, 9, 9},
				{1, 9, 9},
				{9, 9, 9},
			},
			{
				{0, 0},
				{0, 0},
				{0, 0},
				{0, 0},
				{2, 0},
				{0, 0},
			},
			16,
			16,
		},
		{
			// Layout 44
			{
				{8, 9, 9},
				{9, 9, 9},
				{9, 9, 9},
				{9, 9, 9},
				{5, 9, 9},
				{0, 9, 9},
			},
			{
				{0, 0},
				{0, 0},
				{0, 0},
				{0, 0},
				{0, 0},
				{0, 0},
			},
			32,
			16,
		},
	};
	/** Restored level 4 required meals indexed by answer branch, Norf and food category. */
	static constexpr byte kL4Answers[48][6][3] = {
		{
			// Layout 41, branch 0
			{2, 3, 6},
			{1, 4, 7},
			{0, 3, 8},
			{2, 5, 8},
			{1, 5, 7},
			{0, 3, 7},
		},
		{
			// Layout 41, branch 1
			{0, 3, 6},
			{1, 4, 7},
			{2, 3, 8},
			{0, 5, 8},
			{1, 5, 7},
			{2, 3, 7},
		},
		{
			// Layout 41, branch 2
			{2, 3, 6},
			{1, 4, 7},
			{0, 3, 8},
			{2, 5, 8},
			{1, 3, 7},
			{0, 5, 7},
		},
		{
			// Layout 41, branch 3
			{0, 3, 6},
			{1, 4, 7},
			{2, 3, 8},
			{0, 5, 8},
			{1, 3, 7},
			{2, 5, 7},
		},
		{
			// Layout 41, branch 4
			{2, 3, 6},
			{1, 4, 8},
			{0, 3, 7},
			{2, 3, 8},
			{1, 5, 7},
			{0, 5, 7},
		},
		{
			// Layout 41, branch 5
			{2, 3, 6},
			{1, 4, 8},
			{0, 3, 7},
			{2, 3, 8},
			{1, 5, 7},
			{0, 5, 7},
		},
		{
			// Layout 41, branch 6
			{2, 3, 6},
			{1, 4, 8},
			{0, 3, 8},
			{2, 5, 7},
			{1, 5, 7},
			{0, 3, 7},
		},
		{
			// Layout 41, branch 7
			{0, 3, 6},
			{1, 4, 8},
			{2, 3, 8},
			{0, 5, 7},
			{1, 5, 7},
			{2, 3, 7},
		},
		{
			// Layout 41, branch 8
			{2, 3, 6},
			{1, 4, 8},
			{0, 3, 8},
			{2, 5, 7},
			{1, 3, 7},
			{0, 5, 7},
		},
		{
			// Layout 41, branch 9
			{0, 3, 6},
			{1, 4, 8},
			{2, 3, 8},
			{0, 5, 7},
			{1, 3, 7},
			{2, 5, 7},
		},
		{
			// Layout 41, branch 10
			{2, 3, 6},
			{1, 4, 8},
			{0, 3, 8},
			{2, 3, 7},
			{1, 5, 7},
			{0, 5, 7},
		},
		{
			// Layout 41, branch 11
			{2, 3, 6},
			{1, 4, 8},
			{0, 3, 8},
			{2, 3, 7},
			{1, 5, 7},
			{0, 5, 7},
		},
		{
			// Layout 42, branch 0
			{1, 4, 6},
			{1, 5, 8},
			{1, 3, 7},
			{0, 4, 7},
			{0, 4, 8},
			{0, 4, 6},
		},
		{
			// Layout 42, branch 1
			{0, 4, 6},
			{0, 4, 8},
			{0, 4, 7},
			{1, 5, 7},
			{1, 3, 8},
			{1, 4, 6},
		},
		{
			// Layout 42, branch 2
			{1, 4, 6},
			{1, 3, 8},
			{1, 5, 7},
			{0, 4, 7},
			{0, 4, 8},
			{0, 4, 6},
		},
		{
			// Layout 42, branch 3
			{0, 4, 6},
			{0, 4, 8},
			{0, 4, 7},
			{1, 3, 7},
			{1, 5, 8},
			{1, 4, 6},
		},
		{
			// Layout 43, branch 0
			{1, 3, 6},
			{0, 4, 6},
			{1, 4, 7},
			{1, 5, 6},
			{2, 5, 8},
			{2, 3, 6},
		},
		{
			// Layout 43, branch 1
			{1, 3, 6},
			{0, 4, 6},
			{1, 4, 7},
			{1, 5, 6},
			{2, 3, 6},
			{2, 5, 8},
		},
		{
			// Layout 43, branch 2
			{1, 3, 8},
			{0, 4, 8},
			{1, 4, 7},
			{1, 5, 6},
			{2, 5, 8},
			{2, 3, 6},
		},
		{
			// Layout 43, branch 3
			{1, 3, 8},
			{0, 4, 8},
			{1, 4, 7},
			{1, 5, 6},
			{2, 3, 6},
			{2, 5, 8},
		},
		{
			// Layout 43, branch 4
			{1, 3, 6},
			{0, 4, 6},
			{1, 4, 7},
			{1, 5, 8},
			{2, 5, 6},
			{2, 3, 8},
		},
		{
			// Layout 43, branch 5
			{1, 3, 6},
			{0, 4, 6},
			{1, 4, 7},
			{1, 5, 8},
			{2, 3, 8},
			{2, 5, 6},
		},
		{
			// Layout 43, branch 6
			{1, 3, 8},
			{0, 4, 8},
			{1, 4, 7},
			{1, 5, 8},
			{2, 5, 6},
			{2, 3, 8},
		},
		{
			// Layout 43, branch 7
			{1, 3, 8},
			{0, 4, 8},
			{1, 4, 7},
			{1, 5, 8},
			{2, 3, 8},
			{2, 5, 6},
		},
		{
			// Layout 43, branch 8
			{1, 4, 6},
			{0, 3, 6},
			{1, 3, 7},
			{1, 5, 6},
			{2, 5, 8},
			{2, 4, 6},
		},
		{
			// Layout 43, branch 9
			{1, 4, 6},
			{0, 3, 6},
			{1, 3, 7},
			{1, 5, 6},
			{2, 4, 6},
			{2, 5, 8},
		},
		{
			// Layout 43, branch 10
			{1, 4, 8},
			{0, 3, 8},
			{1, 3, 7},
			{1, 5, 6},
			{2, 5, 8},
			{2, 4, 6},
		},
		{
			// Layout 43, branch 11
			{1, 4, 8},
			{0, 3, 8},
			{1, 3, 7},
			{1, 5, 6},
			{2, 4, 6},
			{2, 5, 8},
		},
		{
			// Layout 43, branch 12
			{1, 4, 6},
			{0, 3, 6},
			{1, 3, 7},
			{1, 5, 8},
			{2, 5, 6},
			{2, 4, 8},
		},
		{
			// Layout 43, branch 13
			{1, 4, 6},
			{0, 3, 6},
			{1, 3, 7},
			{1, 5, 8},
			{2, 4, 8},
			{2, 5, 6},
		},
		{
			// Layout 43, branch 14
			{1, 4, 8},
			{0, 3, 8},
			{1, 3, 7},
			{1, 5, 8},
			{2, 5, 6},
			{2, 4, 8},
		},
		{
			// Layout 43, branch 15
			{1, 4, 8},
			{0, 3, 8},
			{1, 3, 7},
			{1, 5, 8},
			{2, 4, 8},
			{2, 5, 6},
		},
		{
			// Layout 44, branch 0
			{0, 4, 8},
			{1, 3, 6},
			{0, 4, 8},
			{0, 4, 7},
			{0, 5, 8},
			{0, 4, 7},
		},
		{
			// Layout 44, branch 1
			{0, 4, 8},
			{1, 3, 7},
			{0, 4, 8},
			{0, 4, 6},
			{0, 5, 8},
			{0, 4, 6},
		},
		{
			// Layout 44, branch 2
			{0, 4, 8},
			{1, 3, 6},
			{0, 4, 8},
			{0, 4, 8},
			{0, 5, 7},
			{0, 4, 8},
		},
		{
			// Layout 44, branch 3
			{0, 4, 8},
			{1, 3, 7},
			{0, 4, 8},
			{0, 4, 8},
			{0, 5, 6},
			{0, 4, 8},
		},
		{
			// Layout 44, branch 4
			{0, 3, 8},
			{1, 4, 6},
			{0, 3, 8},
			{0, 3, 7},
			{0, 5, 8},
			{0, 3, 7},
		},
		{
			// Layout 44, branch 5
			{0, 3, 8},
			{1, 4, 7},
			{0, 3, 8},
			{0, 3, 6},
			{0, 5, 8},
			{0, 3, 6},
		},
		{
			// Layout 44, branch 6
			{0, 3, 8},
			{1, 4, 6},
			{0, 3, 8},
			{0, 3, 8},
			{0, 5, 7},
			{0, 3, 8},
		},
		{
			// Layout 44, branch 7
			{0, 3, 8},
			{1, 4, 7},
			{0, 3, 8},
			{0, 3, 8},
			{0, 5, 6},
			{0, 3, 8},
		},
		{
			// Layout 44, branch 8
			{0, 4, 8},
			{2, 3, 6},
			{0, 4, 8},
			{0, 4, 7},
			{0, 5, 8},
			{0, 4, 7},
		},
		{
			// Layout 44, branch 9
			{0, 4, 8},
			{2, 3, 7},
			{0, 4, 8},
			{0, 4, 6},
			{0, 5, 8},
			{0, 4, 6},
		},
		{
			// Layout 44, branch 10
			{0, 4, 8},
			{2, 3, 6},
			{0, 4, 8},
			{0, 4, 8},
			{0, 5, 7},
			{0, 4, 8},
		},
		{
			// Layout 44, branch 11
			{0, 4, 8},
			{2, 3, 7},
			{0, 4, 8},
			{0, 4, 8},
			{0, 5, 6},
			{0, 4, 8},
		},
		{
			// Layout 44, branch 12
			{0, 3, 8},
			{2, 4, 6},
			{0, 3, 8},
			{0, 3, 7},
			{0, 5, 8},
			{0, 3, 7},
		},
		{
			// Layout 44, branch 13
			{0, 3, 8},
			{2, 4, 7},
			{0, 3, 8},
			{0, 3, 6},
			{0, 5, 8},
			{0, 3, 6},
		},
		{
			// Layout 44, branch 14
			{0, 3, 8},
			{2, 4, 6},
			{0, 3, 8},
			{0, 3, 8},
			{0, 5, 7},
			{0, 3, 8},
		},
		{
			// Layout 44, branch 15
			{0, 3, 8},
			{2, 4, 7},
			{0, 3, 8},
			{0, 3, 8},
			{0, 5, 6},
			{0, 3, 8},
		},
	};

	/** Screen origins and tray-relative offsets for the nine food sprites. */
	static constexpr Common::Point32 kFoodPositions[9] = {
		{121, 412},
		{85, 415},
		{51, 414},
		{107, 462},
		{81, 445},
		{134, 457},
		{122, 494},
		{150, 494},
		{91, 496},
	};
	/** Per-food drawing offsets in pixels from the tray anchor. */
	static constexpr Common::Point32 kFoodOffsets[9] = {
		{45, 34},
		{43, 33},
		{46, 32},
		{15, 32},
		{17, 18},
		{18, 33},
		{28, 51},
		{25, 53},
		{29, 40},
	};

	/** Advance tray travel, Norf motions, speech, and exiting cohorts. */
	void onUpdate() override;
	/** Draw food, trays, Norfs, and player deduction panels. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Draw page-managed Zoombini runners. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Advance runner frames after the actors are displayed. */
	void onActorsRendered() override;
	/** Draw the flying or falling tray over the regular page layers. */
	void onRenderForeground(ManagedSurface32 *screen) override;
	/** Load puzzle art, paths, animations, music, and effects. */
	void loadResources();
	/** Copy a random layout into the active Norfs and reset their service state. */
	void generateRules();
	/** Process a completed pointer click in page coordinates. */
	void handleClick(const Common::Point32 &pos);
	/** Select the appropriate cursor sprite and hover state. */
	void updateCursor();
	/** Cancel the food and tray currently held by the player. */
	void clearSelection();
	/** Queue a Norf's spoken clue and its optional directional gestures. */
	void sayClue(int index);
	/** Play an interruptible Norf clue outside the serialized narration queue. */
	void playClue(const Common::Path &path);
	/** Add feedback, completion, or departure speech to the serialized game narration queue. */
	void queueSpeech(const Common::Path &path);
	/** Play one indexed service effect. */
	void playSound(int index);
	/** Start a body/cap motion for the Norf seated at @p table. */
	void startMotion(int table, Motion motion);
	/** Dispatch a completed Norf animation to @ref PuzzleChezNorf::finishMotion. */
	static void motionComplete(void *context, AnimationRunner *runner);
	/** Advance service state after @p motion reaches its final frame. */
	void finishMotion(Motion motion);
	/** Launch the completed source tray toward the Norf at @p table. */
	void submit(int table);
	/** Compare the delivered meal with the selected Norf's answer and schedule feedback. */
	void checkMeal();
	/** Send the successful cohort through its exit path. */
	void releaseCohort();
	/** Start the waiter exit after retreat speech completes. */
	void dismissWaiter();
	/** Allocate a transient path for the currently flying tray. */
	void startTrayPath(const Common::Point32 &from, const Common::Point32 &to);
	/** Draw @p meal on a tray whose upper-left position is @p pos. */
	void drawTray(ManagedSurface32 *screen, const Common::Point32 &pos, const Meal &meal);
	/** Draw the console-only answer overlay. */
	void drawDebugOverlay(ManagedSurface32 *screen);
	/** Draw the optional English caption for the currently playing direct clue. */
	void drawClueSubtitle(ManagedSurface32 *screen);
	/** Original selection key and retained English sentence for a Chez Norf clue. */
	struct ClueTextEntry {
		/** Decimal layout code used to select the matching clue sentence. */
		int layout;
		/** One-based Norf number within the selected layout. */
		int norf;
		/** Three food clue codes that select this exact sentence. */
		int clue[3];
		/** Borrowed English caption for this layout, Norf and clue combination. */
		const char *text;
	};
	/** English captions keyed by layout code, one-based Norf number and clue tuple. */
	static constexpr ClueTextEntry kClueTextEntries[401] = {
		{11, 1, {0, 9, 9}, "I'm the only one who wants a sandwich"},
		{11, 1, {1, 9, 9}, "I'm the only one who wants fish"},
		{11, 1, {2, 9, 9}, "I'm the only one who wants a salad"},
		{11, 2, {0, 9, 9}, "Everyone over there wants a sandwich, YUCK ! (points right)"},
		{11, 2, {1, 9, 9}, "Everyone over there wants fish, YUCK ! (points right)"},
		{11, 2, {2, 9, 9}, "Everyone over there wants a salad, YUCK ! (points right)"},
		{11, 3, {3, 9, 9}, "Everyone else wants coffee, YUCK !"},
		{11, 3, {4, 9, 9}, "Everyone else wants orange juice, YUCK !"},
		{11, 3, {5, 9, 9}, "Everyone else wants milk, YUCK !"},
		{11, 4, {3, 9, 9}, "No one wants coffee"},
		{11, 4, {4, 9, 9}, "No one wants orange juice"},
		{11, 4, {5, 9, 9}, "No one wants milk"},
		{12, 1, {0, 9, 9}, "I'm the only one who wants a sandwich"},
		{12, 1, {1, 9, 9}, "I'm the only one who wants fish"},
		{12, 1, {2, 9, 9}, "I'm the only one who wants a salad"},
		{12, 2, {3, 9, 9}, "Both my neighbors want coffee"},
		{12, 2, {4, 9, 9}, "Both my neighbors want orange juice"},
		{12, 2, {5, 9, 9}, "Both my neighbors want milk"},
		{12, 3, {0, 9, 9}, "The only one who wants a sandwich is over there (points left)"},
		{12, 3, {1, 9, 9}, "The only one who wants fish is over there (points left)"},
		{12, 3, {2, 9, 9}, "The only one who wants a salad is over there (points left)"},
		{12, 4, {3, 9, 9}, "I'm the only one who wants coffee"},
		{12, 4, {4, 9, 9}, "I'm the only one who wants orange juice"},
		{12, 4, {5, 9, 9}, "I'm the only one who wants milk"},
		{13, 1, {3, 0, 9}, "No one wants coffee or a sandwich"},
		{13, 1, {3, 1, 9}, "No one wants coffee or fish"},
		{13, 1, {3, 2, 9}, "No one wants coffee or a salad"},
		{13, 1, {4, 0, 9}, "No one wants orange juice or a sandwich"},
		{13, 1, {4, 1, 9}, "No one wants orange juice or fish"},
		{13, 1, {4, 2, 9}, "No one wants orange juice or a salad"},
		{13, 1, {5, 0, 9}, "No one wants milk or a sandwich"},
		{13, 1, {5, 1, 9}, "No one wants milk or fish"},
		{13, 1, {5, 2, 9}, "No one wants milk or a salad"},
		{13, 2, {0, 9, 9}, "Both my neighbors want a sandwich, YUCK !"},
		{13, 2, {1, 9, 9}, "Both my neighbors want fish, YUCK !"},
		{13, 2, {2, 9, 9}, "Both my neighbors want a salad, YUCK !"},
		{13, 3, {9, 9, 9}, "Both my neighbors want the same dish"},
		{13, 4, {3, 9, 9}, "Everyone else wants coffee, YUCK !"},
		{13, 4, {4, 9, 9}, "Everyone else wants orange juice, YUCK !"},
		{13, 4, {5, 9, 9}, "Everyone else wants milk, YUCK !"},
		{14, 1, {0, 9, 9}, "I don't want a sandwich"},
		{14, 1, {1, 9, 9}, "I don't want fish"},
		{14, 1, {2, 9, 9}, "I don't want a salad"},
		{14, 2, {0, 1, 9}, "The person who wants a sandwich is not sitting next to the person who wants fish"},
		{14, 2, {0, 2, 9}, "The person who wants a sandwich is not sitting next to the person who wants a salad"},
		{14, 2, {1, 0, 9}, "The person who wants fish is not sitting next to the person who wants a sandwich"},
		{14, 2, {1, 2, 9}, "The person who wants fish is not sitting next to the person who wants a salad"},
		{14, 2, {2, 0, 9}, "The person who wants a salad is not sitting next to the person who wants a sandwich"},
		{14, 2, {2, 1, 9}, "The person who wants a salad is not sitting next to the person who wants fish"},
		{14, 3, {0, 9, 9}, "One neighbor and I both want a sandwich"},
		{14, 3, {1, 9, 9}, "One neighbor and I both want fish"},
		{14, 3, {2, 9, 9}, "One neighbor and I both want a salad"},
		{14, 4, {3, 4, 9}, "No one wants coffee or orange juice"},
		{14, 4, {3, 5, 9}, "No one wants coffee or milk"},
		{14, 4, {4, 3, 9}, "No one wants orange juice or coffee"},
		{14, 4, {4, 5, 9}, "No one wants orange juice or milk"},
		{14, 4, {5, 3, 9}, "No one wants milk or coffee"},
		{14, 4, {5, 4, 9}, "No one wants milk or orange juice"},
		{21, 1, {0, 6, 9}, "Two of us want a sandwich. I'm the only one who wants fruit pie"},
		{21, 1, {0, 7, 9}, "Two of us want a sandwich. I'm the only one who wants watermelon"},
		{21, 1, {0, 8, 9}, "Two of us want a sandwich. I'm the only one who wants ice cream"},
		{21, 1, {1, 6, 9}, "Two of us want fish. I'm the only one who wants fruit pie"},
		{21, 1, {1, 7, 9}, "Two of us want fish. I'm the only one who wants watermelon"},
		{21, 1, {1, 8, 9}, "Two of us want fish. I'm the only one who wants ice cream"},
		{21, 1, {2, 6, 9}, "Two of us want a salad. I'm the only one who wants fruit pie"},
		{21, 1, {2, 7, 9}, "Two of us want a salad. I'm the only one who wants watermelon"},
		{21, 1, {2, 8, 9}, "Two of us want a salad. I'm the only one who wants ice cream"},
		{21, 2, {6, 7, 9}, "Everyone over there wants fruit pie. I want watermelon (points right)"},
		{21, 2, {6, 8, 9}, "Everyone over there wants fruit pie. I want ice cream (points right)"},
		{21, 2, {7, 6, 9}, "Everyone over there wants watermelon. I want fruit pie (points right)"},
		{21, 2, {7, 8, 9}, "Everyone over there wants watermelon. I want ice cream (points right)"},
		{21, 2, {8, 6, 9}, "Everyone over there wants ice cream. I want fruit pie (points right)"},
		{21, 2, {8, 7, 9}, "Everyone over there wants ice cream. I want watermelon (points right)"},
		{21, 3, {3, 0, 9}, "I'm the only one who wants coffee. Both my neighbors want a sandwich (points both)"},
		{21, 3, {3, 1, 9}, "I'm the only one who wants coffee. Both my neighbors want fish (points both)"},
		{21, 3, {3, 2, 9}, "I'm the only one who wants coffee. Both my neighbors want a salad (points both)"},
		{21, 3, {4, 0, 9}, "I'm the only one who wants orange juice. Both my neighbors want a sandwich (points both)"},
		{21, 3, {4, 1, 9}, "I'm the only one who wants orange juice. Both my neighbors want fish (points both)"},
		{21, 3, {4, 2, 9}, "I'm the only one who wants orange juice. Both my neighbors want a salad (points both)"},
		{21, 3, {5, 0, 9}, "I'm the only one who wants milk. Both my neighbors want a sandwich (points both)"},
		{21, 3, {5, 1, 9}, "I'm the only one who wants milk. Both my neighbors want fish (points both)"},
		{21, 3, {5, 2, 9}, "I'm the only one who wants milk. Both my neighbors want a salad (points both)"},
		{21, 4, {3, 4, 9}, "I want coffee, but two others want orange juice"},
		{21, 4, {3, 5, 9}, "I want coffee, but two others want milk"},
		{21, 4, {4, 3, 9}, "I want orange juice, but two others want coffee"},
		{21, 4, {4, 5, 9}, "I want orange juice, but two others want milk"},
		{21, 4, {5, 3, 9}, "I want milk, but two others want coffee"},
		{21, 4, {5, 4, 9}, "I want milk, but two others want orange juice"},
		{22, 1, {0, 9, 9}, "Someone wants a sandwich, YUCK !"},
		{22, 1, {1, 9, 9}, "Someone wants fish, YUCK !"},
		{22, 1, {2, 9, 9}, "Someone wants a salad, YUCK !"},
		{22, 2, {0, 3, 9}, "Neither of my neighbors want a sandwich, and no one wants coffee"},
		{22, 2, {0, 4, 9}, "Neither of my neighbors want a sandwich, and no one wants orange juice"},
		{22, 2, {0, 5, 9}, "Neither of my neighbors want a sandwich, and no one wants milk"},
		{22, 2, {1, 3, 9}, "Neither of my neighbors want fish, and no one wants coffee"},
		{22, 2, {1, 4, 9}, "Neither of my neighbors want fish, and no one wants orange juice"},
		{22, 2, {1, 5, 9}, "Neither of my neighbors want fish, and no one wants milk"},
		{22, 2, {2, 3, 9}, "Neither of my neighbors want a salad, and no one wants coffee"},
		{22, 2, {2, 4, 9}, "Neither of my neighbors want a salad, and no one wants orange juice"},
		{22, 2, {2, 5, 9}, "Neither of my neighbors want a salad, and no one wants milk"},
		{22, 3, {3, 6, 9}, "Everyone else wants coffee. Everyone over there wants fruit pie, YUCK ! (points left)"},
		{22, 3, {3, 7, 9}, "Everyone else wants coffee. Everyone over there wants watermelon, YUCK ! (points left)"},
		{22, 3, {3, 8, 9}, "Everyone else wants coffee. Everyone over there wants ice cream, YUCK ! (points left)"},
		{22, 3, {4, 6, 9}, "Everyone else wants orange juice. Everyone over there wants fruit pie, YUCK ! (points left)"},
		{22, 3, {4, 7, 9}, "Everyone else wants orange juice. Everyone over there wants watermelon, YUCK ! (points left)"},
		{22, 3, {4, 8, 9}, "Everyone else wants orange juice. Everyone over there wants ice cream, YUCK ! (points left)"},
		{22, 3, {5, 6, 9}, "Everyone else wants milk. Everyone over there wants fruit pie, YUCK ! (points left)"},
		{22, 3, {5, 7, 9}, "Everyone else wants milk. Everyone over there wants watermelon, YUCK ! (points left)"},
		{22, 3, {5, 8, 9}, "Everyone else wants milk. Everyone over there wants ice cream, YUCK ! (points left)"},
		{22, 4, {0, 6, 9}, "Two of us want a sandwich. I'm the only one who wants fruit pie"},
		{22, 4, {0, 7, 9}, "Two of us want a sandwich. I'm the only one who wants watermelon"},
		{22, 4, {0, 8, 9}, "Two of us want a sandwich. I'm the only one who wants ice cream"},
		{22, 4, {1, 6, 9}, "Two of us want fish. I'm the only one who wants fruit pie"},
		{22, 4, {1, 7, 9}, "Two of us want fish. I'm the only one who wants watermelon"},
		{22, 4, {1, 8, 9}, "Two of us want fish. I'm the only one who wants ice cream"},
		{22, 4, {2, 6, 9}, "Two of us want a salad. I'm the only one who wants fruit pie"},
		{22, 4, {2, 7, 9}, "Two of us want a salad. I'm the only one who wants watermelon"},
		{22, 4, {2, 8, 9}, "Two of us want a salad. I'm the only one who wants ice cream"},
		{23, 1, {0, 1, 9}, "The two who wants a sandwich are next to each other, and so are the two who want fish"},
		{23, 1, {0, 2, 9}, "The two who wants a sandwich are next to each other, and so are the two who want a salad"},
		{23, 1, {1, 0, 9}, "The two who wants fish are next to each other, and so are the two who want a sandwich"},
		{23, 1, {1, 2, 9}, "The two who wants fish are next to each other, and so are the two who want a salad"},
		{23, 1, {2, 0, 9}, "The two who wants a salad are next to each other, and so are the two who want a sandwich"},
		{23, 1, {2, 1, 9}, "The two who wants a salad are next to each other, and so are the two who want fish"},
		{23, 2, {6, 3, 9}, "One of them is the only one who wants fruit pie, and the other one is the only one who wants coffee (points right)"},
		{23, 2, {6, 4, 9}, "One of them is the only one who wants fruit pie, and the other one is the only one who wants orange juice (points right)"},
		{23, 2, {6, 5, 9}, "One of them is the only one who wants fruit pie, and the other one is the only one who wants milk (points right)"},
		{23, 2, {7, 3, 9}, "One of them is the only one who wants watermelon, and the other one is the only one who wants coffee (points right)"},
		{23, 2, {7, 4, 9}, "One of them is the only one who wants watermelon, and the other one is the only one who wants orange juice (points right)"},
		{23, 2, {7, 5, 9}, "One of them is the only one who wants watermelon, and the other one is the only one who wants milk (points right)"},
		{23, 2, {8, 3, 9}, "One of them is the only one who wants ice cream, and the other one is the only one who wants coffee (points right)"},
		{23, 2, {8, 4, 9}, "One of them is the only one who wants ice cream, and the other one is the only one who wants orange juice (points right)"},
		{23, 2, {8, 5, 9}, "One of them is the only one who wants ice cream, and the other one is the only one who wants milk (points right)"},
		{23, 3, {6, 3, 9}, "No one wants fruit pie. Only my neighbor and I want coffee"},
		{23, 3, {6, 4, 9}, "No one wants fruit pie. Only my neighbor and I want orange juice"},
		{23, 3, {6, 5, 9}, "No one wants fruit pie. Only my neighbor and I want milk"},
		{23, 3, {7, 3, 9}, "No one wants watermelon. Only my neighbor and I want coffee"},
		{23, 3, {7, 4, 9}, "No one wants watermelon. Only my neighbor and I want orange juice"},
		{23, 3, {7, 5, 9}, "No one wants watermelon. Only my neighbor and I want milk"},
		{23, 3, {8, 3, 9}, "No one wants ice cream. Only my neighbor and I want coffee"},
		{23, 3, {8, 4, 9}, "No one wants ice cream. Only my neighbor and I want orange juice"},
		{23, 3, {8, 5, 9}, "No one wants ice cream. Only my neighbor and I want milk"},
		{23, 4, {0, 6, 9}, "I want a sandwich, but I don't want fruit pie"},
		{23, 4, {0, 7, 9}, "I want a sandwich, but I don't want watermelon"},
		{23, 4, {0, 8, 9}, "I want a sandwich, but I don't want ice cream"},
		{23, 4, {1, 6, 9}, "I want fish, but I don't want fruit pie"},
		{23, 4, {1, 7, 9}, "I want fish, but I don't want watermelon"},
		{23, 4, {1, 8, 9}, "I want fish, but I don't want ice cream"},
		{23, 4, {2, 6, 9}, "I want a salad, but I don't want fruit pie"},
		{23, 4, {2, 7, 9}, "I want a salad, but I don't want watermelon"},
		{23, 4, {2, 8, 9}, "I want a salad, but I don't want ice cream"},
		{24, 1, {3, 9, 9}, "No one has the same drink as their neighbor. No one wants coffee"},
		{24, 1, {4, 9, 9}, "No one has the same drink as their neighbor. No one wants orange juice"},
		{24, 1, {5, 9, 9}, "No one has the same drink as their neighbor. No one wants milk"},
		{24, 2, {0, 9, 9}, "Everyone who wants a sandwich wants a different dessert"},
		{24, 2, {1, 9, 9}, "Everyone who wants fish wants a different dessert"},
		{24, 2, {2, 9, 9}, "Everyone who wants a salad wants a different dessert"},
		{24, 3, {0, 6, 9}, "I'm the only one who wants a sandwich. One of my neighbor and I both want fruit pie"},
		{24, 3, {0, 7, 9}, "I'm the only one who wants a sandwich. One of my neighbor and I both want watermelon"},
		{24, 3, {0, 8, 9}, "I'm the only one who wants a sandwich. One of my neighbor and I both want ice cream"},
		{24, 3, {1, 6, 9}, "I'm the only one who wants fish. One of my neighbor and I both want fruit pie"},
		{24, 3, {1, 7, 9}, "I'm the only one who wants fish. One of my neighbor and I both want watermelon"},
		{24, 3, {1, 8, 9}, "I'm the only one who wants fish. One of my neighbor and I both want ice cream"},
		{24, 3, {2, 6, 9}, "I'm the only one who wants a salad. One of my neighbor and I both want fruit pie"},
		{24, 3, {2, 7, 9}, "I'm the only one who wants a salad. One of my neighbor and I both want watermelon"},
		{24, 3, {2, 8, 9}, "I'm the only one who wants a salad. One of my neighbor and I both want ice cream"},
		{24, 4, {0, 6, 3}, "No one wants a sandwich. I'm the only one who wants fruit pie. I don't want coffee"},
		{24, 4, {0, 6, 4}, "No one wants a sandwich. I'm the only one who wants fruit pie. I don't want orange juice"},
		{24, 4, {0, 6, 5}, "No one wants a sandwich. I'm the only one who wants fruit pie. I don't want milk"},
		{24, 4, {0, 7, 3}, "No one wants a sandwich. I'm the only one who wants watermelon. I don't want coffee"},
		{24, 4, {0, 7, 4}, "No one wants a sandwich. I'm the only one who wants watermelon. I don't want orange juice"},
		{24, 4, {0, 7, 5}, "No one wants a sandwich. I'm the only one who wants watermelon. I don't want milk"},
		{24, 4, {0, 8, 3}, "No one wants a sandwich. I'm the only one who wants ice cream. I don't want coffee"},
		{24, 4, {0, 8, 4}, "No one wants a sandwich. I'm the only one who wants ice cream. I don't want orange juice"},
		{24, 4, {0, 8, 5}, "No one wants a sandwich. I'm the only one who wants ice cream. I don't want milk"},
		{24, 4, {1, 6, 3}, "No one wants fish. I'm the only one who wants fruit pie. I don't want coffee"},
		{24, 4, {1, 6, 4}, "No one wants fish. I'm the only one who wants fruit pie. I don't want orange juice"},
		{24, 4, {1, 6, 5}, "No one wants fish. I'm the only one who wants fruit pie. I don't want milk"},
		{24, 4, {1, 7, 3}, "No one wants fish. I'm the only one who wants watermelon. I don't want coffee"},
		{24, 4, {1, 7, 4}, "No one wants fish. I'm the only one who wants watermelon. I don't want orange juice"},
		{24, 4, {1, 7, 5}, "No one wants fish. I'm the only one who wants watermelon. I don't want milk"},
		{24, 4, {1, 8, 3}, "No one wants fish. I'm the only one who wants ice cream. I don't want coffee"},
		{24, 4, {1, 8, 4}, "No one wants fish. I'm the only one who wants ice cream. I don't want orange juice"},
		{24, 4, {1, 8, 5}, "No one wants fish. I'm the only one who wants ice cream. I don't want milk"},
		{24, 4, {2, 6, 3}, "No one wants a salad. I'm the only one who wants fruit pie. I don't want coffee"},
		{24, 4, {2, 6, 4}, "No one wants a salad. I'm the only one who wants fruit pie. I don't want orange juice"},
		{24, 4, {2, 6, 5}, "No one wants a salad. I'm the only one who wants fruit pie. I don't want milk"},
		{24, 4, {2, 7, 3}, "No one wants a salad. I'm the only one who wants watermelon. I don't want coffee"},
		{24, 4, {2, 7, 4}, "No one wants a salad. I'm the only one who wants watermelon. I don't want orange juice"},
		{24, 4, {2, 7, 5}, "No one wants a salad. I'm the only one who wants watermelon. I don't want milk"},
		{24, 4, {2, 8, 3}, "No one wants a salad. I'm the only one who wants ice cream. I don't want coffee"},
		{24, 4, {2, 8, 4}, "No one wants a salad. I'm the only one who wants ice cream. I don't want orange juice"},
		{24, 4, {2, 8, 5}, "No one wants a salad. I'm the only one who wants ice cream. I don't want milk"},
		{31, 1, {6, 6, 3}, "I'm not one of the four who want fruit pie. No one who wants fruit pie wants coffee"},
		{31, 1, {6, 6, 4}, "I'm not one of the four who want fruit pie. No one who wants fruit pie wants orange juice"},
		{31, 1, {6, 6, 5}, "I'm not one of the four who want fruit pie. No one who wants fruit pie wants milk"},
		{31, 1, {7, 7, 3}, "I'm not one of the four who want watermelon. No one who wants watermelon wants coffee"},
		{31, 1, {7, 7, 4}, "I'm not one of the four who want watermelon. No one who wants watermelon wants orange juice"},
		{31, 1, {7, 7, 5}, "I'm not one of the four who want watermelon. No one who wants watermelon wants milk"},
		{31, 1, {8, 8, 3}, "I'm not one of the four who want ice cream. No one who wants ice cream wants coffee"},
		{31, 1, {8, 8, 4}, "I'm not one of the four who want ice cream. No one who wants ice cream wants orange juice"},
		{31, 1, {8, 8, 5}, "I'm not one of the four who want ice cream. No one who wants ice cream wants milk"},
		{31, 2, {0, 3, 9}, "Neither of my neighbors want a sandwich, but both want coffee (points both)"},
		{31, 2, {0, 4, 9}, "Neither of my neighbors want a sandwich, but both want orange juice (points both)"},
		{31, 2, {0, 5, 9}, "Neither of my neighbors want a sandwich, but both want milk (points both)"},
		{31, 2, {1, 3, 9}, "Neither of my neighbors want fish, but both want coffee (points both)"},
		{31, 2, {1, 4, 9}, "Neither of my neighbors want fish, but both want orange juice (points both)"},
		{31, 2, {1, 5, 9}, "Neither of my neighbors want fish, but both want milk (points both)"},
		{31, 2, {2, 3, 9}, "Neither of my neighbors want a salad, but both want coffee (points both)"},
		{31, 2, {2, 4, 9}, "Neither of my neighbors want a salad, but both want orange juice (points both)"},
		{31, 2, {2, 5, 9}, "Neither of my neighbors want a salad, but both want milk (points both)"},
		{31, 3, {6, 0, 9}, "No one wants fruit pie. I'm the only one who wants a sandwich"},
		{31, 3, {6, 1, 9}, "No one wants fruit pie. I'm the only one who wants fish"},
		{31, 3, {6, 2, 9}, "No one wants fruit pie. I'm the only one who wants a salad"},
		{31, 3, {7, 0, 9}, "No one wants watermelon. I'm the only one who wants a sandwich"},
		{31, 3, {7, 1, 9}, "No one wants watermelon. I'm the only one who wants fish"},
		{31, 3, {7, 2, 9}, "No one wants watermelon. I'm the only one who wants a salad"},
		{31, 3, {8, 0, 9}, "No one wants ice cream. I'm the only one who wants a sandwich"},
		{31, 3, {8, 1, 9}, "No one wants ice cream. I'm the only one who wants fish"},
		{31, 3, {8, 2, 9}, "No one wants ice cream. I'm the only one who wants a salad"},
		{31, 4, {3, 0, 9}, "I want coffee. Three of us want a sandwich"},
		{31, 4, {3, 1, 9}, "I want coffee. Three of us want fish"},
		{31, 4, {3, 2, 9}, "I want coffee. Three of us want salad"},
		{31, 4, {4, 0, 9}, "I want orange juice. Three of us want a sandwich"},
		{31, 4, {4, 1, 9}, "I want orange juice. Three of us want fish"},
		{31, 4, {4, 2, 9}, "I want orange juice. Three of us want salad"},
		{31, 4, {5, 0, 9}, "I want milk. Three of us want a sandwich"},
		{31, 4, {5, 1, 9}, "I want milk. Three of us want fish"},
		{31, 4, {5, 2, 9}, "I want milk. Three of us want salad"},
		{31, 5, {0, 9, 9}, "Both my neighbors want a sandwich, YUCK ! (points both)"},
		{31, 5, {1, 9, 9}, "Both my neighbors want fish, YUCK ! (points both)"},
		{31, 5, {2, 9, 9}, "Both my neighbors want a salad, YUCK ! (points both)"},
		{31, 6, {3, 9, 9}, "One of my neighbors wants coffee, and so does one of their neighbors, and so does someone else (points both)"},
		{31, 6, {4, 9, 9}, "One of my neighbors wants orange juice, and so does one of their neighbors, and so does someone else (points both)"},
		{31, 6, {5, 9, 9}, "One of my neighbors wants milk, and so does one of their neighbors, and so does someone else (points both)"},
		{32, 1, {0, 6, 8}, "Everyone who wants a sandwich also wants fruit pie. No one wants ice cream"},
		{32, 1, {0, 6, 7}, "Everyone who wants a sandwich also wants fruit pie. No one wants watermelon"},
		{32, 1, {0, 7, 6}, "Everyone who wants a sandwich also wants watermelon. No one wants fruit pie"},
		{32, 1, {0, 7, 8}, "Everyone who wants a sandwich also wants watermelon. No one wants ice cream"},
		{32, 1, {0, 8, 6}, "Everyone who wants a sandwich also wants ice cream. No one wants fruit pie"},
		{32, 1, {0, 8, 7}, "Everyone who wants a sandwich also wants ice cream. No one wants watermelon"},
		{32, 1, {1, 6, 8}, "Everyone who wants fish also wants fruit pie. No one wants ice cream"},
		{32, 1, {1, 6, 7}, "Everyone who wants fish also wants fruit pie. No one wants watermelon"},
		{32, 1, {1, 7, 6}, "Everyone who wants fish also wants watermelon. No one wants fruit pie"},
		{32, 1, {1, 7, 8}, "Everyone who wants fish also wants watermelon. No one wants ice cream"},
		{32, 1, {1, 8, 6}, "Everyone who wants fish also wants ice cream. No one wants fruit pie"},
		{32, 1, {1, 8, 7}, "Everyone who wants fish also wants ice cream. No one wants watermelon"},
		{32, 1, {2, 6, 8}, "Everyone who wants a salad also wants fruit pie. No one wants ice cream"},
		{32, 1, {2, 6, 7}, "Everyone who wants a salad also wants fruit pie. No one wants watermelon"},
		{32, 1, {2, 7, 6}, "Everyone who wants a salad also wants watermelon. No one wants fruit pie"},
		{32, 1, {2, 7, 8}, "Everyone who wants a salad also wants watermelon. No one wants ice cream"},
		{32, 1, {2, 8, 6}, "Everyone who wants a salad also wants ice cream. No one wants fruit pie"},
		{32, 1, {2, 8, 7}, "Everyone who wants a salad also wants ice cream. No one wants watermelon"},
		{32, 2, {6, 6, 9}, "I want fruit pie, and the other two who want fruit pie are neighbors"},
		{32, 2, {7, 7, 9}, "I want watermelon, and the other two who want watermelon are neighbors"},
		{32, 2, {8, 8, 9}, "I want ice cream, and the other two who want ice cream are neighbors"},
		{32, 3, {3, 4, 9}, "Three of us want coffee, but I'm the only one who wants orange juice"},
		{32, 3, {3, 5, 9}, "Three of us want coffee, but I'm the only one who wants milk"},
		{32, 3, {4, 3, 9}, "Three of us want orange juice, but I'm the only one who wants coffee"},
		{32, 3, {4, 5, 9}, "Three of us want orange juice, but I'm the only one who wants milk"},
		{32, 3, {5, 3, 9}, "Three of us want milk, but I'm the only one who wants coffee"},
		{32, 3, {5, 4, 9}, "Three of us want milk, but I'm the only one who wants orange juice"},
		{32, 4, {0, 9, 9}, "One of my neighbors is the only one who wants a sandwich"},
		{32, 4, {1, 9, 9}, "One of my neighbors is the only one who wants fish"},
		{32, 4, {2, 9, 9}, "One of my neighbors is the only one who wants a salad"},
		{32, 5, {0, 9, 9}, "All three sandwiches are over there (points left)"},
		{32, 5, {1, 9, 9}, "All three fishes are over there (points left)"},
		{32, 5, {2, 9, 9}, "All three salads are over there (points left)"},
		{32, 6, {3, 6, 9}, "Two of us want coffee with their fruit pie, but not me"},
		{32, 6, {3, 7, 9}, "Two of us want coffee with their watermelon, but not me"},
		{32, 6, {3, 8, 9}, "Two of us want coffee with their ice cream, but not me"},
		{32, 6, {4, 6, 9}, "Two of us want orange juice with their fruit pie, but not me"},
		{32, 6, {4, 7, 9}, "Two of us want orange juice with their watermelon, but not me"},
		{32, 6, {4, 8, 9}, "Two of us want orange juice with their ice cream, but not me"},
		{32, 6, {5, 6, 9}, "Two of us want milk with their fruit pie, but not me"},
		{32, 6, {5, 7, 9}, "Two of us want milk with their watermelon, but not me"},
		{32, 6, {5, 8, 9}, "Two of us want milk with their ice cream, but not me"},
		{33, 1, {6, 3, 9}, "I'm the only one who wants fruit pie. Two want coffee. YUCK !"},
		{33, 1, {6, 4, 9}, "I'm the only one who wants fruit pie. Two want orange juice. YUCK !"},
		{33, 1, {6, 5, 9}, "I'm the only one who wants fruit pie. Two want milk. YUCK !"},
		{33, 1, {7, 3, 9}, "I'm the only one who wants watermelon. Two want coffee. YUCK !"},
		{33, 1, {7, 4, 9}, "I'm the only one who wants watermelon. Two want orange juice. YUCK !"},
		{33, 1, {7, 5, 9}, "I'm the only one who wants watermelon. Two want milk. YUCK !"},
		{33, 1, {8, 3, 9}, "I'm the only one who wants ice cream. Two want coffee. YUCK !"},
		{33, 1, {8, 4, 9}, "I'm the only one who wants ice cream. Two want orange juice. YUCK !"},
		{33, 1, {8, 5, 9}, "I'm the only one who wants ice cream. Two want milk. YUCK !"},
		{33, 2, {9, 9, 9}, "Both my neighbors want different desserts from the rest of us"},
		{33, 3, {3, 9, 9}, "Everyone over there wants the same drink, but no one wants coffee (points right)"},
		{33, 3, {4, 9, 9}, "Everyone over there wants the same drink, but no one wants orange juice (points right)"},
		{33, 3, {5, 9, 9}, "Everyone over there wants the same drink, but no one wants milk (points right)"},
		{33, 4, {0, 1, 9}, "Both of my neighbors want a sandwich, but no one wants fish"},
		{33, 4, {0, 2, 9}, "Both of my neighbors want a sandwich, but no one wants a salad"},
		{33, 4, {1, 0, 9}, "Both of my neighbors want fish, but no one wants a sandwich"},
		{33, 4, {1, 2, 9}, "Both of my neighbors want fish, but no one wants a salad"},
		{33, 4, {2, 0, 9}, "Both of my neighbors want a salad, but no one wants a sandwich"},
		{33, 4, {2, 1, 9}, "Both of my neighbors want a salad, but no one wants fish"},
		{33, 5, {0, 9, 9}, "The two on the ends want a sandwich, and so does someone else"},
		{33, 5, {1, 9, 9}, "The two on the ends want fish, and so does someone else"},
		{33, 5, {2, 9, 9}, "The two on the ends want a salad, and so does someone else"},
		{33, 6, {6, 9, 9}, "Two neighbors want the same meal. They don't want fruit pie"},
		{33, 6, {7, 9, 9}, "Two neighbors want the same meal. They don't want watermelon"},
		{33, 6, {8, 9, 9}, "Two neighbors want the same meal. They don't want ice cream"},
		{34, 1, {0, 9, 9}, "Two want a sandwich. YUCK !"},
		{34, 1, {1, 9, 9}, "Two want fish. YUCK !"},
		{34, 1, {2, 9, 9}, "Two want a salad. YUCK !"},
		{34, 2, {3, 0, 9}, "The two who want coffee also want a sandwich"},
		{34, 2, {3, 1, 9}, "The two who want coffee also want fish"},
		{34, 2, {3, 2, 9}, "The two who want coffee also want a salad"},
		{34, 2, {4, 0, 9}, "The two who want orange juice also want a sandwich"},
		{34, 2, {4, 1, 9}, "The two who want orange juice also want fish"},
		{34, 2, {4, 2, 9}, "The two who want orange juice also want a salad"},
		{34, 2, {5, 0, 9}, "The two who want milk also want a sandwich"},
		{34, 2, {5, 1, 9}, "The two who want milk also want fish"},
		{34, 2, {5, 2, 9}, "The two who want milk also want a salad"},
		{34, 3, {6, 7, 9}, "Everyone over there wants fruit pie. Everyone over there wants watermelon (points right then left)"},
		{34, 3, {6, 8, 9}, "Everyone over there wants fruit pie. Everyone over there wants ice cream (points right then left)"},
		{34, 3, {7, 6, 9}, "Everyone over there wants watermelon. Everyone over there wants fruit pie (points right then left)"},
		{34, 3, {7, 8, 9}, "Everyone over there wants watermelon. Everyone over there wants ice cream (points right then left)"},
		{34, 3, {8, 6, 9}, "Everyone over there wants ice cream. Everyone over there wants fruit pie (points right then left)"},
		{34, 3, {8, 7, 9}, "Everyone over there wants ice cream. Everyone over there wants watermelon (points right then left)"},
		{34, 4, {6, 0, 9}, "Everyone who wants fruit pie also wants a sandwich"},
		{34, 4, {6, 1, 9}, "Everyone who wants fruit pie also wants fish"},
		{34, 4, {6, 2, 9}, "Everyone who wants fruit pie also wants a salad"},
		{34, 4, {7, 0, 9}, "Everyone who wants watermelon also wants a sandwich"},
		{34, 4, {7, 1, 9}, "Everyone who wants watermelon also wants fish"},
		{34, 4, {7, 2, 9}, "Everyone who wants watermelon also wants a salad"},
		{34, 4, {8, 0, 9}, "Everyone who wants ice cream also wants a sandwich"},
		{34, 4, {8, 1, 9}, "Everyone who wants ice cream also wants fish"},
		{34, 4, {8, 2, 9}, "Everyone who wants ice cream also wants a salad"},
		{34, 5, {3, 9, 9}, "Both my neighbors want coffee. YUCK !"},
		{34, 5, {4, 9, 9}, "Both my neighbors want orange juice. YUCK !"},
		{34, 5, {5, 9, 9}, "Both my neighbors want milk. YUCK !"},
		{34, 6, {3, 9, 9}, "The two who want coffee are neighbors"},
		{34, 6, {4, 9, 9}, "The two who want orange juice are neighbors"},
		{34, 6, {5, 9, 9}, "The two who want milk are neighbors"},
		{41, 1, {6, 7, 9}, "I'm the only one who wants fruit pie, and two others want watermelon"},
		{41, 1, {6, 8, 9}, "I'm the only one who wants fruit pie, and two others want ice cream"},
		{41, 1, {7, 6, 9}, "I'm the only one who wants watermelon, and two others want fruit pie"},
		{41, 1, {7, 8, 9}, "I'm the only one who wants watermelon, and two others want ice cream"},
		{41, 1, {8, 6, 9}, "I'm the only one who wants ice cream, and two others want fruit pie"},
		{41, 1, {8, 7, 9}, "I'm the only one who wants ice cream, and two others want watermelon"},
		{41, 2, {0, 9, 9}, "I want a sandwich"},
		{41, 2, {1, 9, 9}, "I want fish"},
		{41, 2, {2, 9, 9}, "I want a salad"},
		{41, 3, {3, 4, 9}, "Someone other there wants coffee. The only one who want orange juice are over there (points left then right)"},
		{41, 3, {3, 5, 9}, "Someone other there wants coffee. The only one who want milk are over there (points left then right)"},
		{41, 3, {4, 3, 9}, "Someone other there wants orange juice. The only one who want coffee are over there (points left then right)"},
		{41, 3, {4, 5, 9}, "Someone other there wants orange juice. The only one who want milk are over there (points left then right)"},
		{41, 3, {5, 3, 9}, "Someone other there wants milk. The only one who want coffee are over there (points left then right)"},
		{41, 3, {5, 4, 9}, "Someone other there wants milk. The only one who want orange juice are over there (points left then right)"},
		{41, 4, {6, 9, 9}, "These two want fruit pie (points right)"},
		{41, 4, {7, 9, 9}, "These two want watermelon (points right)"},
		{41, 4, {8, 9, 9}, "These two want ice cream (points right)"},
		{41, 5, {3, 9, 9}, "Each of the three who want coffee wants a different dessert"},
		{41, 5, {4, 9, 9}, "Each of the three who want orange juice wants a different dessert"},
		{41, 5, {5, 9, 9}, "Each of the three who want milk wants a different dessert"},
		{41, 6, {9, 9, 9}, "Any three in a row want different main dishes from each other"},
		{42, 1, {0, 9, 9}, "Everyone who wants a sandwich wants a different drink and a different dessert, and they are all in a row"},
		{42, 1, {1, 9, 9}, "Everyone who wants fish wants a different drink and a different dessert, and they are all in a row"},
		{42, 1, {2, 9, 9}, "Everyone who wants a salad wants a different drink and a different dessert, and they are all in a row"},
		{42, 2, {6, 9, 9}, "The two on the ends have the same drink and they both have fruit pie"},
		{42, 2, {7, 9, 9}, "The two on the ends have the same drink and they both have watermelon"},
		{42, 2, {8, 9, 9}, "The two on the ends have the same drink and they both have ice cream"},
		{42, 3, {3, 9, 9}, "There are more coffees than any other item"},
		{42, 3, {4, 9, 9}, "There are more orange juices than any other item"},
		{42, 3, {5, 9, 9}, "There are more milk glasses than any other item"},
		{42, 4, {6, 9, 9}, "I want fruit pie and so does one of my neighbors"},
		{42, 4, {7, 9, 9}, "I want watermelon and so does one of my neighbors"},
		{42, 4, {8, 9, 9}, "I want ice cream and so does one of my neighbors"},
		{42, 5, {0, 9, 9}, "Everyone who wants a sandwich wants a different dessert"},
		{42, 5, {1, 9, 9}, "Everyone who wants fish wants a different dessert"},
		{42, 5, {2, 9, 9}, "Everyone who wants a salad wants a different dessert"},
		{42, 6, {0, 9, 9}, "No one wants a sandwich"},
		{42, 6, {1, 9, 9}, "No one wants fish"},
		{42, 6, {2, 9, 9}, "No one wants a salad"},
		{43, 1, {9, 9, 9}, "Everyone has exactly one thing in common with each neighbor"},
		{43, 2, {0, 9, 9}, "I'm the only one who wants a sandwich"},
		{43, 2, {1, 9, 9}, "I'm the only one who wants fish"},
		{43, 2, {2, 9, 9}, "I'm the only one who wants a salad"},
		{43, 3, {6, 9, 9}, "I'm the only one who wants fruit pie"},
		{43, 3, {7, 9, 9}, "I'm the only one who wants watermelon"},
		{43, 3, {8, 9, 9}, "I'm the only one who wants ice cream"},
		{43, 4, {3, 9, 9}, "I want coffee"},
		{43, 4, {4, 9, 9}, "I want orange juice"},
		{43, 4, {5, 9, 9}, "I want milk"},
		{43, 5, {0, 9, 9}, "There are more sandwiches than any other main dishes, and they are all other there (points left)"},
		{43, 5, {1, 9, 9}, "There are more fishes than any other main dishes, and they are all other there (points left)"},
		{43, 5, {2, 9, 9}, "There are more salads than any other main dishes, and they are all other there (points left)"},
		{43, 6, {9, 9, 9}, "The same number of us want coffee, orange juice, and milk"},
		{44, 1, {6, 9, 9}, "If someone doesn't want fruit pie, all their neighbors do"},
		{44, 1, {7, 9, 9}, "If someone doesn't want watermelon, all their neighbors do"},
		{44, 1, {8, 9, 9}, "If someone doesn't want ice cream, all their neighbors do"},
		{44, 2, {9, 9, 9}, "No one else wants anything I want"},
		{44, 3, {9, 9, 9}, "Someone two seats away wants the same meal as I do"},
		{44, 4, {9, 9, 9}, "Someone two seats away wants the same meal as I do"},
		{44, 5, {3, 9, 9}, "I'm the only one who wants coffee"},
		{44, 5, {4, 9, 9}, "I'm the only one who wants orange juice"},
		{44, 5, {5, 9, 9}, "I'm the only one who wants milk"},
		{44, 6, {0, 9, 9}, "There are more sandwiches than any other item"},
		{44, 6, {1, 9, 9}, "There are more fishes than any other item"},
		{44, 6, {2, 9, 9}, "There are more salads than any other item"},
	};
	/** Return the English text for the exact @p layout, @p norf, and @p clue selection. */
	static Common::String getClueText(int layout, int norf, const int clue[3]);
	/** Return whether the required meal categories have all been selected. */
	bool mealComplete(const Meal &meal) const;
	/** Return whether the selected tray may be submitted to @p table. */
	bool canSubmitTo(int table) const;
	/** Return whether a Norf animation is active, optionally ignoring idle animation. */
	bool motionActive(bool includeIdle = true) const;
	/** Return whether the serialized speech queue is still active. */
	bool speechPlaying() const;
	/** Return whether @p pos lies inside a page-space rectangle. */
	static bool inside(const Common::Point32 &pos, int x, int y, int width, int height);
	/** Return the food choice under the page-pixel point, or -1 when none is hit. */
	int foodAt(const Common::Point32 &pos) const;
	/** Return the tray slot under the page-pixel point, or -1 when none is hit. */
	int trayAt(const Common::Point32 &pos) const;
	/** Return the Norf under the page-pixel point, or -1 when none is hit. */
	int norfAt(const Common::Point32 &pos) const;
	/** Return the page-pixel position for a table column at the supplied Y coordinate. */
	static Common::Point32 tablePosition(int index, int y);

	/** Number of active Norf tables in the selected layout. */
	int _tableCount = 4;
	/** Selected decimal layout code, combining difficulty and variant. */
	int _layout = 11;
	/** Total submissions available from the initial reusable trays and spare supply. */
	int _submissionLimit = 8;
	/** Category lookup for the food codes used by clues and tray contents. */
	int _foodValues[10] = {};
	/** Rule, clue and service state indexed by table. */
	Norf _norfs[6];
	/** Current food selections indexed by tray slot. */
	Meal _trays[6];
	/** Whether each table still supplies a reusable tray. */
	bool _trayAvailable[6] = {
		true,
		true,
		true,
		true,
		true,
		true,
	};
	/** Player marks by category, food value, and table. */
	byte _notes[3][3][6] = {};
	/** Number of replacement trays still available. */
	int _traySupply = 5;
	/** Number of submitted trays processed by the page. */
	int _submissions = 0;
	/** Number of Norfs that have accepted their required meal. */
	int _successCount = 0;
	/** Number of Zoombinis still waiting to be served and released. */
	int _remaining = 0;
	/** Food code currently held by the pointer, or -1 when none is held. */
	int _selectedFood = -1;
	/** Tray currently held by the pointer, or -1 when none is held. */
	int _selectedTray = -1;
	/** Norf receiving the currently animated tray, or -1 when none is selected. */
	int _recipient = -1;
	/** Norf whose meal reaction is currently animated. */
	int _animatedNorf = -1;
	/** Norf selected for the current idle gesture, or -1 when none is active. */
	int _idleNorf = -1;
	/** Queued second directional gesture code, or zero when none is pending. */
	int _nextGesture = 0;
	/** Current stage of tray submission and Norf feedback. */
	Phase _phase = kReady00;
	/** Food selections retained for the moving tray. */
	Meal _flyingMeal;
	/** Current moving-tray anchor in page pixels. */
	Common::Point32 _flyingPosition;
	/** Tray path deleted when movement finishes or the page closes; nullptr while stationary. */
	PathObject *_trayPath = nullptr;
	/** Latest pointer location in page pixels. */
	Common::Point32 _pointer;
	/** Page-space pointer location retained for deferred click processing. */
	Common::Point32 _click;
	/** Whether the current pointer press can produce a release action. */
	bool _buttonArmed = false;
	/** Whether a pointer release awaits processing by the page state machine. */
	bool _clickPending = false;
	/** Whether the page currently allows the Go action. */
	bool _canDepart = false;
	/** Whether Go is waiting for retreat narration to complete. */
	bool _departAfterSpeech = false;
	/** Whether the current Norf's dismissal reaction is pending. */
	bool _dismissPending = false;
	/** Whether an accepted meal still needs to release its Zoombini cohort. */
	bool _releasePending = false;
	/** Interruptible clue speech handle, or -1 when no direct clue is playing. */
	int _clueSpeechSound = -1;
	/** Wrapped caption lines for the currently playing clue. */
	Common::Array<Common::U32String> _clueSubtitleLines;
	/** Loaded sound handles indexed by the page's event sound slots. */
	int _sounds[8] = {
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
	};
	/** Body animation data indexed by motion and released with the page. */
	Animation *_bodyAnimations[7] = {};
	/** Head animation data indexed by cap variant and motion, released with the page. */
	Animation *_capAnimations[6][7] = {};
	/** Body animation playback state indexed by scene animation slot. */
	AnimationRunner *_bodyRunners[7] = {};
	/** Head playback state indexed by cap variant and motion. */
	AnimationRunner *_capRunners[6][7] = {};
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_CHEZNORF_H
