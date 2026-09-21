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

#ifndef ZOOMBINI2_PAGES_PUZZLE_BOOLIES_H
#define ZOOMBINI2_PAGES_PUZZLE_BOOLIES_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class Animation;
class PathObject;

/**
 * Boolie Boggle (Route4-2)
 *
 * Use pinballs to make each Boolie group happy and ready to board a boat.
 */
class PuzzleBoolies : public PuzzleBase {
public:
	/** Construct Boolie Boggle for @p vm. */
	PuzzleBoolies(Zoombini2Engine *vm);
	/** Release animations and active paths; sprites remain in the graphics page cache. */
	~PuzzleBoolies() override;

	/** Load the three Boolie groups and place the active Zoombini in the boat. */
	void init() override;
	/** Advance challenge balls, Boolie jumps, and boat travel. */
	void onUpdate() override;
	/** Restore the primary page layer before drawing puzzle content. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw the Boolie groups, balls, pin sections, lights, and boat. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Draw the Zoombini riding in the boat. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Advance Zoombini animation frames after they have been drawn. */
	void onActorsRendered() override;
	/** Select a Boolie row at @p pos while a challenge awaits input. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Highlight the selectable Boolie row under @p pos. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Return whether at least one Boolie group has boarded and may depart. */
	bool canUseGoButton() const override;
	/** Describe active rows and the current/next ball cohorts for the puzzle console. */
	Common::String debugGetAnswer() const override;
	/** Report remaining challenges as the puzzle's chance count. */
	PuzzleChanceInfo debugGetChances() const override;
	/** Return whether the console may change challenge count while the board is idle. */
	bool debugCanSetChances() const override;
	/** Set remaining challenges and synchronize the generated turn threshold. */
	bool debugSetChances(int remaining) override;

	/** Return the rescued-Boolie credit assigned to each party member at @p level. */
	static int getRescuedBooliesPerZoombini(int level);

private:
	/** Number of Boolie ledges. */
	static constexpr int kRowCount = 3;
	/** Maximum number of Boolies on one ledge. */
	static constexpr int kSlotCount = 4;
	/** Maximum number of balls in one challenge. */
	static constexpr int kMaxChallengeBalls = 6;
	/** Music played while this page is active. */
	static constexpr const char *kMusicPath = "#sounds/music/09-BB01.wav";
	/** Boolie and ball sprites in the release-matched resource tree. */
	static constexpr const char *kFixePath = "bmp/boolies/FIXE";
	static constexpr const char *kFixe2Path = "bmp/boolies/FIXE2";
	static constexpr const char *kBallPosPath = "bmp/boolies/BALL_POS";
	static constexpr const char *kBallNegPath = "bmp/boolies/BALL_NEG";
	static constexpr const char *kPinPath = "bmp/boolies/PIN";
	static constexpr const char *kPinLightedPath = "bmp/boolies/pin_lighted";
	static constexpr const char *kBoatPath = "bmp/boolies/BATEAU";
	static constexpr const char *kSpotFormat = "bmp/boolies/SPOT%02d";
	static constexpr const char *kBlockerPath = "bmp/boolies/BLOCKER";
	static constexpr const char *kRollPath = "bmp/boolies/ROLL";
	static constexpr const char *kRoll2Path = "bmp/boolies/ROLL2";
	static constexpr const char *kBoolieWalkOnePath = "bmp/boolies/MARCHE";
	static constexpr const char *kBoolieWalkTwoPath = "bmp/boolies/MARCHE2";
	/** Paths used to feed, route, and board balls and Boolies. */
	static constexpr const char *kPreviewEntryPath = "bmp/boolies/b_boolies1.pat";
	static constexpr const char *kFeederPathFormat = "bmp/boolies/b_boolies1bis_%d.pat";
	static constexpr const char *kLanePathFormat = "bmp/boolies/b_boolies%d.pat";
	static constexpr const char *kJumpPathFormat = "bmp/boolies/Jump%d_%d.pat";
	/** Row and polarity select effects 01 through 06; effect 07 accompanies a hit. */
	static constexpr const char *kEffectPathFormat = "sounds/fx/09-BS%02d.wav";
	/** Four Boolie voices used as members start boarding paths. */
	static constexpr const char *kBoardVoicePathFormat = "sounds/blp15.%d.wav";
	/** Duration of each frame in the original Boolie roll sequence. */
	static constexpr uint32 kRollFrameTime = 30;
	/** Duration of each frame in the opening blocker sequence. */
	static constexpr uint32 kBlockerFrameTime = 70;
	/** Number of frames played by the opening blocker sequence. */
	static constexpr uint32 kBlockerFrameCount = 3;
	/** Launch, boarding-check, replacement-walk timing, and replacement screen offsets. */
	static constexpr uint32 kBallLaunchInterval = 600;
	static constexpr uint32 kFirstBallLaunchDelay = 30;
	static constexpr uint32 kBoardingCheckDelay = 2000;
	static constexpr uint32 kWalkFrameTime = 40;
	static constexpr uint32 kWalkFrameCount = 11;
	static constexpr int kWalkStepX = 30;
	static constexpr int kRefillStartX[kSlotCount] = {
		-20,
		-22,
		-24,
		-26,
	};
	static constexpr int kRefillY[kRowCount] = {
		107,
		221,
		322,
	};
	/** Original top-left holding positions for the preview and ready ball groups. */
	static constexpr int kPreviewHoldX[kMaxChallengeBalls] = {
		220,
		187,
		154,
		121,
		88,
		55,
	};
	static constexpr int kPreviewHoldY[kMaxChallengeBalls] = {
		25,
		27,
		28,
		27,
		23,
		17,
	};
	static constexpr int kReadyHoldX[kMaxChallengeBalls] = {
		601,
		633,
		663,
		680,
		674,
		651,
	};
	static constexpr int kReadyHoldY[kMaxChallengeBalls] = {
		56,
		49,
		39,
		32,
		15,
		9,
	};

	/** Runtime phase of the Boolie challenge and boat transfer. */
	enum class Phase {
		/** Feed balls and accept one row selection. */
		kFeeding00 = 0,
		/** Move the selected challenge balls through the row. */
		kRolling01 = 1,
		/** Delay the next challenge after an uncleared row. */
		kBetweenRounds02 = 2,
		/** Carry the successful group away from the ledges. */
		kBoatLeaving03 = 3,
		/** Bring the boat back for the next Zoombini. */
		kBoatReturning04 = 4,
		/** Stop accepting row input and enable the Go control. */
		kFinished05 = 5
	};

	/** Position of one challenge ball in its feeder and row paths. */
	enum class BallStage {
		/** Wait for the ball's scheduled feeder start. */
		kWaiting00 = 0,
		/** Follow the upper feeder path. */
		kFeeder01 = 1,
		/** Wait at the feeder endpoint for a row selection. */
		kHeld02 = 2,
		/** Follow the selected row path. */
		kLane03 = 3,
		/** Finish the row path and stop drawing the ball. */
		kDone04 = 4,
		/** Wait in the cave for this next-challenge preview ball's entry. */
		kPreviewWaiting05 = 5,
		/** Follow the cave-to-preview path. */
		kPreviewEntry06 = 6,
		/** Remain at the left preview position until the current turn ends. */
		kPreviewHeld07 = 7
	};

	/** One Boolie's current value and ledge visibility. */
	struct Boolie {
		/** Value 1 or 2 in an active slot; zero in an unused slot. */
		byte value = 0;
		/** Value currently shown after queued rolling animations. */
		byte visibleValue = 0;
		/** Whether its jump path is currently running. */
		bool jumping = false;
		/** Whether the ledge slot is unused or has completed its jump. */
		bool removed = false;
	};

	/** One queued visible transition after a ball changes a Boolie value. */
	struct Flip {
		/** Row/slot and source/destination portrait values for one queued visible roll. */
		int row = 0;
		int slot = 0;
		byte fromValue = 0;
		byte toValue = 0;
	};

	/** One ball in the current signed challenge. */
	struct Ball {
		/** Current feeder or row path, released after the challenge. */
		PathObject *path = nullptr;
		/** Position in the indexed feeder and preview holding arrays. */
		int index = 0;
		/** Current screen position. */
		Common::Point32 pos;
		/** Earliest game tick at which the feeder path may start. */
		uint32 startAt = 0;
		/** Current path stage. */
		BallStage stage = BallStage::kWaiting00;
	};

	/** A Boolie traveling from one ledge slot into the boat. */
	struct Jump {
		/** Row-specific path, released when the jump finishes. */
		PathObject *path = nullptr;
		/** Current screen position. */
		Common::Point32 pos;
		/** Source row. */
		int row = 0;
		/** Source slot on that row. */
		int slot = 0;
		/** Portrait selected by this Boolie's value. */
		byte value = 1;
		/** Whether this jump has started after earlier group members. */
		bool started = false;
	};

	/** A Boolie retained at its landing position while the boat moves. */
	struct Passenger {
		/** Landing position in the boat's initial screen placement. */
		Common::Point32 pos;
		/** Portrait selected by this Boolie's value. */
		byte value = 1;
	};

	/** Return the row hit by @p pos, or -1 outside the three row regions. */
	static int getRowForPoint(const Common::Point &pos);
	/** Return the screen position of @p slot on @p row. */
	static Common::Point32 getBooliePosition(int row, int slot);
	/** Return the center position of a ball held behind the left blocker. */
	static Common::Point32 getPreviewHoldPosition(int index);
	/** Return the center position of a ball waiting at the right feeder end. */
	static Common::Point32 getReadyHoldPosition(int index);
	/** Load the page's sprites, animations, and sound effects. */
	void loadResources();
	/** Generate the active values on @p row for the selected difficulty. */
	void generateRow(int row);
	/** Draw the original difficulty-specific random values for one four-slot row. */
	void sampleRowValues(byte (&values)[kSlotCount]);
	/** Schedule a new signed ball challenge at game tick @p now. */
	void beginRound(uint32 now);
	/** Stage the following challenge at the left preview holding positions. */
	void prepareNextChallenge(uint32 now);
	/** Start the prepared preview cohort across the upper feeder. */
	void startNextFeeder(uint32 now);
	/** Replace the completed cohort with the already prepared one. */
	void promoteNextChallenge(uint32 now);
	/** Choose the signed challenge type for the selected difficulty. */
	int pickChallengeType();
	/** Release the previous challenge's ball paths. */
	void clearBalls();
	/** Release every path in @p balls and empty the array. */
	static void clearBallArray(Common::Array<Ball> &balls);
	/** Release any Boolie jump paths still in progress. */
	void clearJumps();
	/** Play a page effect through the configured effects volume. */
	void playEffect(int soundId) const;
	/** Play one randomly selected boarding voice through the original SFX volume. */
	void playBoardVoice();
	/** Move @p ball from its feeder endpoint onto the selected row at @p now. */
	void startLaneBall(Ball &ball, uint32 now);
	/** Advance @p ball through its feeder or row path at @p now. */
	void advanceBall(Ball &ball, uint32 now);
	/** Advance cave entry and feeder movement for the following challenge. */
	void advanceNextBalls(uint32 now);
	/** Return whether the upper gate must be lowered for scheduled feeder balls. */
	bool isBlockerLowered() const;
	/** Apply the completed @p ball to the selected Boolie row at @p now. */
	void resolveBall(const Ball &ball, uint32 now);
	/** Complete queued portrait rolls one at a time. */
	void advanceFlips(uint32 now);
	/** Start the selected row's jumps after every ball and portrait roll finishes. */
	void startRowJumps(uint32 now);
	/** Advance one Boolie jump and start the next at @p now. */
	void advanceJumps(uint32 now);
	/** Advance the selected row's replacement walkers one animation cycle. */
	void advanceRefill(uint32 now);
	/** End the selected challenge, starting the boat or the next round. */
	void finishRound(uint32 now);
	/** Mark the active Zoombini successful and start boat departure at @p now. */
	void startBoat(uint32 now);
	/** Move the boat and choose the next Zoombini at @p now. */
	void advanceBoat(uint32 now);
	/** Return whether @p row contains no Boolie with value 2. */
	bool isRowEmpty(int row) const;
	/** Draw the pin deflector sections and selected-row light. */
	void drawPinSections(ManagedSurface32 *screen) const;
	/** Draw lights for the row under the pointer. */
	void drawSpotHighlights(ManagedSurface32 *screen) const;

	/** Current values and ledge state of the three Boolie groups. */
	Boolie _boolies[kRowCount][kSlotCount] = {};
	/** Balls in the current challenge and their active paths. */
	Common::Array<Ball> _balls;
	/** Following challenge, visible at the left while the current balls wait or roll. */
	Common::Array<Ball> _nextBalls;
	/** Pending visible transitions in impact order. */
	Common::Array<Flip> _flips;
	/** Group members waiting to jump or currently jumping. */
	Common::Array<Jump> _jumps;
	/** Completed jump landings retained for the current boat trip. */
	Common::Array<Passenger> _passengers;
	/** Opening blocker transition, played once. */
	Animation *_blockerAnimation = nullptr;
	/** Original rolling portraits for values 1 and 2. */
	Animation *_rollAnimations[2] = {};
	/** Eleven-frame walking animations for incoming value-1 and value-2 Boolies. */
	Animation *_walkAnimations[2] = {};
	/** Row and sign effects followed by the hit effect. */
	int _effectIds[7] = {};
	/** Four random boarding voices. */
	int _boardVoiceIds[4] = {};
	/** Replacement values generated when a row becomes ready to board. */
	byte _replacementValues[kRowCount][kSlotCount] = {};
	/** Start tick of the current visible roll. */
	uint32 _flipStart = 0;
	/** Start tick of the opening blocker transition. */
	uint32 _blockerStart = 0;
	/** Whether the selected row has started boarding. */
	bool _rowJumpsStarted = false;
	/** Whether the post-roll boarding check has been scheduled. */
	bool _boardingCheckScheduled = false;
	/** Deadline for the scheduled post-roll boarding check. */
	uint32 _boardingCheckAt = 0;
	/** Whether one replacement Boolie is walking from the left edge. */
	bool _refillActive = false;
	/** Replacement walker source row/slot, screen X, and animation-cycle start tick. */
	int _refillRow = -1;
	int _refillSlot = 0;
	int _refillX = 0;
	uint32 _refillCycleStart = 0;
	/** Current challenge or boat-transfer phase. */
	Phase _phase = Phase::kFeeding00;
	/** Tick when the current timed phase began. */
	uint32 _phaseTime = 0;
	/** Puzzle-roster entry currently riding in the boat. */
	int _activeRunnerIndex = 0;
	/** Selected row, or -1 before the player has chosen one. */
	int _selectedRow = -1;
	/** Persistent clipped PIN route selected by the most recent lane click. */
	int _pinRoute = 0;
	/** Index and deadline of the next ball to launch into the selected lane. */
	uint _nextLaneBallIndex = 0;
	/** Deadline for launching the next ball onto the selected lane. */
	uint32 _nextLaneLaunchAt = 0;
	/** Row currently highlighted by the pointer, or -1. */
	int _hoverRow = -1;
	/** Signed type and ball count of the current challenge. */
	int _challengeType = 0;
	/** Signed type selected for the following preview cohort. */
	int _nextChallengeType = 0;
	/** Whether the following challenge has been selected and staged. */
	bool _nextPrepared = false;
	/** Whether the following challenge should cross the upper feeder. */
	bool _nextFeedingRequested = false;
	/** Whether the following cohort has started its upper feeder paths. */
	bool _nextFeeding = false;
	/** Number of resolved challenges. */
	int _completedTurns = 0;
	/** Turn threshold checked after each completed challenge. */
	int _requiredTurns = 0;
	/** Current horizontal screen position of the boat sprite. */
	int _boatX = 80;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_BOOLIES_H
