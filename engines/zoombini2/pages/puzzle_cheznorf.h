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
	/** Select a difficulty layout, load art, and arrange the waiting Zoombinis. */
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
	/** Mark every waiting Zoombini released for the puzzle-console completion command. */
	void applyDebugPuzzleCompletion() override;
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
	/** Format one main dish, drink, dessert, or note-symbol sprite by resource name. */
	static constexpr const char *kFoodFormat = "bmp/chez_norf/%s";
	/** Food resource names ordered as three mains, three drinks, then three desserts. */
	static constexpr const char *kFoodNames[9] = {
		"miam_sandwitch",
		"miam_poisson",
		"miam_salade",
		"glouglou_cafe",
		"glouglou_orange",
		"glouglou_lait",
		"slurp_tarte",
		"slurp_pasteque",
		"slurp_glace",
	};
	/** Player note sprites for confirmed, rejected, and uncertain food deductions. */
	static constexpr const char *kSymbolNames[3] = {
		"symb_OK",
		"symb_NO",
		"symb_MAYBE",
	};
	/** Three note-panel sprites, one for each meal category. */
	static constexpr const char *kPanelNames[3] = {
		"comande3",
		"COMANDE1",
		"comande2",
	};
	/** Full, cursor, and miniature tray sprites used while assembling and serving meals. */
	static constexpr const char *kPlatePath = "bmp/chez_norf/plato";
	static constexpr const char *kSmallPlatePath = "bmp/chez_norf/plato2";
	static constexpr const char *kMiniPlatePath = "bmp/chez_norf/plato_mini";
	/** Highlight drawn over the selected food, tray, or Norf. */
	static constexpr const char *kHighlightPath = "bmp/chez_norf/highlight";
	/** Static Norf body and cap art, indexed by table and motion. */
	static constexpr const char *kNorfPath = "bmp/chez_norf/norf/norfDeBaz";
	static constexpr const char *kCapPath = "bmp/chez_norf/norf/cask/%d/cask2baz";
	static constexpr const char *kNorfAnimationFormat = "bmp/chez_norf/norf/norf%d.an";
	static constexpr const char *kCapAnimationFormat = "bmp/chez_norf/norf/cask/%d/cask%d.an";
	/** Paths used by released cohorts and the waiter dismissal. */
	static constexpr const char *kExitFormat = "bmp/chez_norf/out%d.pat";
	static constexpr const char *kWaiterExitPath = "bmp/chez_norf/out_special.pat";
	/** Restaurant music, effects, clues, response speech, completion, and retreat audio. */
	static constexpr const char *kMusicPath = "#sounds/music/07-BB02.wav";
	static constexpr const char *kSoundFormat = "sounds/fx/07-BS%02d.wav";
	static constexpr const char *kClueFormat = "sounds/7-N%d-%d";
	static constexpr const char *kFeedbackFormat = "sounds/7-N%d-G%02d.wav";
	static constexpr const char *kCompleteSpeechPath = "sounds/CZN31.wav";
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
		/** Temporary flags consumed by the corresponding motion-complete callback. */
		bool acceptedTray = false;
		bool rejectedTray = false;
		/** Last accepted and rejected meals retained for feedback rendering. */
		Meal accepted;
		Meal rejected;
	};
	/** One difficulty-layout variant encoded in food, clue, and gesture indices. */
	struct Layout {
		/** Required meals, spoken clues, and directional gestures for up to six tables. */
		byte answers[6][3];
		byte clues[6][3];
		byte gestures[6][2];
	};
	/** Four release-matched variants for each of the three selectable difficulties. */
	static constexpr Layout kLayouts[12] = {
		{// Layout 11
		 {{0, 3, 9}, {1, 3, 9}, {2, 5, 9}, {2, 3, 9}},
		 {{0, 9, 9}, {2, 9, 9}, {3, 9, 9}, {4, 9, 9}},
		 {{0, 0}, {1, 0}, {0, 0}, {0, 0}},},
		{// Layout 12
		 {{0, 5, 9}, {2, 3, 9}, {1, 5, 9}, {1, 4, 9}},
		 {{0, 9, 9}, {5, 9, 9}, {2, 9, 9}, {4, 9, 9}},
		 {{0, 0}, {0, 0}, {2, 0}, {0, 0}}},
		{// Layout 13
		 {{1, 5, 9}, {2, 5, 9}, {1, 5, 9}, {2, 4, 9}},
		 {{3, 0, 9}, {1, 9, 9}, {9, 9, 9}, {5, 9, 9}},
		 {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},
		{// Layout 14
		 {{0, 5, 9}, {1, 5, 9}, {1, 5, 9}, {2, 5, 9}},
		 {{2, 9, 9}, {2, 0, 9}, {1, 9, 9}, {3, 4, 9}},
		 {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},
		{// Layout 21
		 {{0, 4, 8}, {1, 4, 6}, {0, 5, 7}, {1, 3, 7}},
		 {{0, 8, 9}, {7, 6, 9}, {5, 1, 9}, {3, 4, 9}},
		 {{0, 0}, {1, 0}, {3, 0}, {0, 0}}},
		{// Layout 22
		 {{0, 5, 7}, {1, 5, 7}, {2, 3, 6}, {1, 5, 8}},
		 {{2, 9, 9}, {1, 4, 9}, {5, 7, 9}, {1, 8, 9}},
		 {{0, 0}, {0, 0}, {2, 0}, {0, 0}}},
		{// Layout 23
		 {{2, 3, 8}, {2, 5, 8}, {0, 5, 7}, {0, 4, 8}},
		 {{0, 2, 9}, {7, 4, 9}, {6, 5, 9}, {0, 7, 9}},
		 {{0, 0}, {1, 0}, {0, 0}, {0, 0}}},
		{// Layout 24
		 {{2, 5, 7}, {2, 3, 8}, {1, 5, 8}, {2, 3, 6}},
		 {{4, 9, 9}, {2, 9, 9}, {1, 8, 9}, {0, 6, 5}},
		 {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},
		{// Layout 31
		 {{0, 3, 6}, {1, 5, 8}, {2, 3, 8}, {1, 4, 6}, {0, 5, 8}, {1, 5, 8}},
		 {{8, 8, 4}, {1, 3, 9}, {7, 2, 9}, {4, 1, 9}, {1, 9, 9}, {5, 9, 9}},
		 {{0, 0}, {3, 0}, {0, 0}, {0, 0}, {3, 0}, {3, 0}}},
		{// Layout 32
		 {{2, 4, 7}, {1, 3, 6}, {2, 5, 7}, {2, 4, 7}, {0, 3, 6}, {1, 4, 6}},
		 {{2, 7, 8}, {6, 6, 9}, {4, 5, 9}, {0, 9, 9}, {2, 9, 9}, {3, 6, 9}},
		 {{0, 0}, {0, 0}, {0, 0}, {0, 0}, {2, 0}, {0, 0}}},
		{// Layout 33
		 {{0, 3, 8}, {0, 5, 6}, {2, 5, 7}, {2, 3, 6}, {2, 3, 6}, {0, 3, 6}},
		 {{8, 5, 9}, {9, 9, 9}, {4, 9, 9}, {2, 1, 9}, {0, 9, 9}, {7, 9, 9}},
		 {{0, 0}, {0, 0}, {1, 0}, {0, 0}, {0, 0}, {0, 0}}},
		{// Layout 34
		 {{1, 3, 8}, {2, 4, 8}, {2, 4, 7}, {1, 5, 6}, {1, 3, 6}, {1, 5, 6}},
		 {{2, 9, 9}, {3, 1, 9}, {6, 8, 9}, {6, 1, 9}, {5, 9, 9}, {4, 9, 9}},
		 {{0, 0}, {0, 0}, {1, 2}, {0, 0}, {0, 0}, {0, 0}}}};
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
	/** Start a non-looping speech resource and retain its mixer handle. */
	void playSpeech(const Common::Path &path);
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
	/** Return whether the required meal categories have all been selected. */
	bool mealComplete(const Meal &meal) const;
	/** Return whether the selected tray may be submitted to @p table. */
	bool canSubmitTo(int table) const;
	/** Return whether a Norf animation is active, optionally ignoring idle animation. */
	bool motionActive(bool includeIdle = true) const;
	/** Return whether queued speech still occupies the speech sound handle. */
	bool speechPlaying() const;
	/** Return whether @p pos lies inside a page-space rectangle. */
	static bool inside(const Common::Point32 &pos, int x, int y, int width, int height);
	/** Return the food, tray, or Norf index under @p pos, or -1 when absent. */
	int foodAt(const Common::Point32 &pos) const;
	int trayAt(const Common::Point32 &pos) const;
	int norfAt(const Common::Point32 &pos) const;
	static Common::Point32 tablePosition(int index, int y);

	/** Selected difficulty, active table count, and chosen layout identifier. */
	int _level = 1;
	int _tableCount = 4;
	int _layout = 11;
	/** Food-category lookup plus all active Norf rules and source trays. */
	int _foodValues[10] = {};
	Norf _norfs[6];
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
	/** Remaining reusable trays, submitted meals, released cohorts, and waiting Zoombinis. */
	int _traySupply = 5;
	int _submissions = 0;
	int _successCount = 0;
	int _remaining = 0;
	/** Food, tray, Norf, animation, idle-Norf, and chained-gesture selection state. */
	int _selectedFood = -1;
	int _selectedTray = -1;
	int _recipient = -1;
	int _animatedNorf = -1;
	int _idleNorf = -1;
	int _nextGesture = 0;
	/** Current service phase and the meal, position, and path of its flying tray. */
	Phase _phase = kReady00;
	Meal _flyingMeal;
	Common::Point32 _flyingPosition;
	PathObject *_trayPath = nullptr;
	/** Latest pointer/click positions and input arming state. */
	Common::Point32 _pointer;
	Common::Point32 _click;
	bool _buttonArmed = false;
	bool _clickPending = false;
	/** Go availability and deferred transitions waiting on speech or animation. */
	bool _canDepart = false;
	bool _departAfterSpeech = false;
	bool _dismissPending = false;
	bool _releasePending = false;
	/** Current speech handle and the indexed service-effect handles. */
	int _speechSound = -1;
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
	/** Shared body/cap animation resources and their active runners, indexed by motion and cap. */
	Animation *_bodyAnimations[7] = {};
	Animation *_capAnimations[6][7] = {};
	AnimationRunner *_bodyRunners[7] = {};
	AnimationRunner *_capRunners[6][7] = {};
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_CHEZNORF_H
