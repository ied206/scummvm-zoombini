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
	PuzzleChezNorf(Zoombini2Engine *vm);
	~PuzzleChezNorf() override;
	void init() override;
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	bool canUseGoButton() const override;
	bool onGoButtonPressed() override;
	bool blocksSidebarInteraction() const override;
	void applyDebugPuzzleCompletion() override;
	Common::String debugGetAnswer() const override;
	PuzzleChanceInfo debugGetChances() const override;
	bool debugCanSetChances() const override;
	bool debugSetChances(int remaining) override;
	Common::String debugGetChanceDetails() const override;

private:
	static constexpr const char *kFoodFormat = "bmp/chez_norf/%s";
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
	static constexpr const char *kSymbolNames[3] = {
		"symb_NO",
		"symb_OK",
		"symb_MAYBE",
	};
	static constexpr const char *kPanelNames[3] = {
		"comande3",
		"COMANDE1",
		"comande2",
	};
	static constexpr const char *kPlatePath = "bmp/chez_norf/plato";
	static constexpr const char *kSmallPlatePath = "bmp/chez_norf/plato2";
	static constexpr const char *kMiniPlatePath = "bmp/chez_norf/plato_mini";
	static constexpr const char *kHighlightPath = "bmp/chez_norf/highlight";
	static constexpr const char *kNorfPath = "bmp/chez_norf/norf/norfDeBaz";
	static constexpr const char *kCapPath = "bmp/chez_norf/norf/cask/%d/cask2baz";
	static constexpr const char *kNorfAnimationFormat = "bmp/chez_norf/norf/norf%d.an";
	static constexpr const char *kCapAnimationFormat = "bmp/chez_norf/norf/cask/%d/cask%d.an";
	static constexpr const char *kExitFormat = "bmp/chez_norf/out%d.pat";
	static constexpr const char *kWaiterExitPath = "bmp/chez_norf/out_special.pat";
	static constexpr const char *kMusicPath = "#sounds/music/07-BB02.wav";
	static constexpr const char *kSoundFormat = "sounds/fx/07-BS%02d.wav";
	static constexpr const char *kClueFormat = "sounds/7-N%d-%d";
	static constexpr const char *kFeedbackFormat = "sounds/7-N%d-G%02d.wav";
	static constexpr const char *kCompleteSpeechPath = "sounds/CZN31.wav";
	static constexpr const char *kRetreatSpeechPath = "sounds/DW-Cave.wav";

	/** Indices in the paired body/cap animation bank. */
	enum Motion {
		kReject00 = 0,
		kRejectReturn01 = 1,
		kDismiss02 = 2,
		kAccept03 = 3,
		kIdle04 = 4,
		kPointRight05 = 5,
		kPointLeft06 = 6
	};
	/** Success and rejection callbacks advance independently of the flying tray. */
	enum Phase {
		kReady00 = 0,
		kThrow01 = 1,
		kLand02 = 2,
		kRejectFall03 = 3,
		kFeedback04 = 4,
		kRelease05 = 5,
		kDismiss06 = 6,
		kFinished07 = 7
	};
	struct Meal {
		int food[3] = {
			-1,
			-1,
			-1,
		}; // Main dish, drink, dessert.
	};
	struct Norf {
		Meal answer;
		int clue[3] = {
			9,
			9,
			9,
		};
		int gesture[2] = {};
		bool served = false;
		bool acceptedTray = false;
		bool rejectedTray = false;
		Meal accepted;
		Meal rejected;
	};
	struct Layout {
		byte answers[6][3];
		byte clues[6][3];
		byte gestures[6][2];
	};
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

	void onUpdate() override;
	void onRenderContent(ManagedSurface32 *screen) override;
	void onRenderActors(ManagedSurface32 *screen) override;
	void onActorsRendered() override;
	void onRenderForeground(ManagedSurface32 *screen) override;
	void loadResources();
	void generateRules();
	void handleClick(const Common::Point32 &pos);
	void updateCursor();
	void clearSelection();
	void sayClue(int index);
	void playSpeech(const Common::Path &path);
	void playSound(int index);
	void startMotion(int table, Motion motion);
	static void motionComplete(void *context, AnimationRunner *runner);
	void finishMotion(Motion motion);
	void submit(int table);
	void checkMeal();
	void releaseCohort();
	void dismissWaiter();
	void startTrayPath(const Common::Point32 &from, const Common::Point32 &to);
	void drawTray(ManagedSurface32 *screen, const Common::Point32 &pos, const Meal &meal);
	void drawDebugOverlay(ManagedSurface32 *screen);
	bool mealComplete(const Meal &meal) const;
	bool motionActive(bool includeIdle = true) const;
	bool speechPlaying() const;
	static bool inside(const Common::Point32 &pos, int x, int y, int width, int height);
	int foodAt(const Common::Point32 &pos) const;
	int trayAt(const Common::Point32 &pos) const;
	int norfAt(const Common::Point32 &pos) const;
	static Common::Point32 tablePosition(int index, int y);

	int _level = 1;
	int _tableCount = 4;
	int _layout = 11;
	int _foodValues[10] = {};
	Norf _norfs[6];
	Meal _trays[6];
	bool _trayAvailable[6] = {
		true,
		true,
		true,
		true,
		true,
		true,
	};
	byte _notes[3][3][6] = {};
	int _traySupply = 5;
	int _submissions = 0;
	int _successCount = 0;
	int _remaining = 0;
	int _selectedFood = -1;
	int _selectedTray = -1;
	int _recipient = -1;
	int _animatedNorf = -1;
	int _idleNorf = -1;
	int _nextGesture = 0;
	Phase _phase = kReady00;
	Meal _flyingMeal;
	Common::Point32 _flyingPosition;
	PathObject *_trayPath = nullptr;
	Common::Point32 _pointer;
	Common::Point32 _click;
	bool _buttonArmed = false;
	bool _clickPending = false;
	bool _canDepart = false;
	bool _departAfterSpeech = false;
	bool _dismissPending = false;
	bool _releasePending = false;
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
	Animation *_bodyAnimations[7] = {};
	Animation *_capAnimations[6][7] = {};
	AnimationRunner *_bodyRunners[7] = {};
	AnimationRunner *_capRunners[6][7] = {};
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_CHEZNORF_H
