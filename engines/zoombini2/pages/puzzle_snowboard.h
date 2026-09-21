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

#ifndef ZOOMBINI2_PAGES_PUZZLE_SNOWBOARD_H
#define ZOOMBINI2_PAGES_PUZZLE_SNOWBOARD_H

#include "common/rect.h"

#include "zoombini2/pages/puzzle_base.h"
#include "zoombini2/scripts.h"
#include "zoombini2/state.h"

namespace Zoombini2 {

class Animation;

/**
 * Snowboard Gulch (Route4-1)
 *
 * Send each Zoombini down the trail that matches its visible traits.
 */
class PuzzleSnowboard : public PuzzleBase {
public:
	/** Construct Snowboard Gulch for @p vm. */
	PuzzleSnowboard(Zoombini2Engine *vm);
	/** Release the lane, trait, board, scenery, and active-path resources. */
	~PuzzleSnowboard() override;

	/** Generate the decision tree and place the puzzle roster at the start. */
	void init() override;
	/** Advance the current ride, obstacle checks, and exit path. */
	void onUpdate() override;
	/** Restore the primary page layer before drawing the trail. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw the board, scenery, decision hints, and visible obstacles. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Draw the current board rider and the puzzle roster. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Advance Zoombini animation frames after they have been drawn. */
	void onActorsRendered() override;
	/** Handle release at @p pos and start a ride when the board receives the rider. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Update a dragged Zoombini as the pointer moves to @p pos. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Queue the original Go speech before leaving a saved game. */
	bool onGoButtonPressed() override;
	/** Return whether the ride sequence has completed enough successful exits to leave. */
	bool canUseGoButton() const override;
	/** Describe the generated decision tree and classified rider routes for the console. */
	Common::String debugGetAnswer() const override;
	/** Report remaining collision tolerance as the puzzle's chance count. */
	PuzzleChanceInfo debugGetChances() const override;
	/** Return whether the console can alter collision tolerance between rides. */
	bool debugCanSetChances() const override;
	/** Set collision tolerance and synchronize the completed/Go state for the console. */
	bool debugSetChances(int remaining) override;

private:
	/** Music, trait icons, PAT ride paths, board art, scenery, and obstacle art. */
	static constexpr const char *kMusicPath = "#sounds/music/01-BS06.wav";
	static constexpr const char *kTraitFormat = "bmp/snowboard/traits/%d-%d";
	static constexpr const char *kPatFormat = "bmp/snowboard/pat/easy/%d.pat";
	static constexpr const char *kBoardPath = "bmp/snowboard/BOARD01";
	static constexpr const char *kBoardAnimPath = "bmp/snowboard/BOARD";
	static constexpr const char *kEngineAnimPath = "bmp/snowboard/ENGINE";
	static constexpr const char *kDecorFormat = "bmp/snowboard/N1So-%d";
	static constexpr const char *kSurfFormat = "bmp/snowboard/SURF/SN%d";
	static constexpr int kSurfIds[9] = {
		66,
		36,
		33,
		23,
		22,
		12,
		11,
		41,
		44,
	};
	/** Area mask, board and obstacle effects, collision/completion speech, and celebration animation. */
	static constexpr const char *kAreaMaskPath = "bmp/snowboard/AREA.BMT";
	static constexpr const char *kBoardReadySoundPath = "sounds/fx/01-BB02.wav";
	static constexpr const char *kRideSoundPath = "sounds/fx/01-BS01.wav";
	static constexpr const char *kObstacleHitSoundPath = "sounds/fx/01-BS02.wav";
	static constexpr const char *kObstacleRevealSoundPath = "sounds/fx/01-BS03.wav";
	static constexpr const char *kObstacleHideSoundPath = "sounds/fx/01-BS04.wav";
	static constexpr const char *kCollisionSpeechFormat = "sounds/SWB13%d-%d.wav";
	static constexpr const char *kNearQuotaSpeechFormat = "sounds/SWB14-%d.wav";
	static constexpr const char *kQuotaSpeechFormat = "sounds/SWB15-%d.wav";
	static constexpr const char *kSuccessSpeechPath = "sounds/8-E1.wav";
	static constexpr const char *kFailureSpeechPath = "sounds/8-E2.wav";
	static constexpr const char *kPerfectGoSpeechFormat = "sounds/WLD11.%d.wav";
	static constexpr const char *kCaveGoSpeechPath = "sounds/DW-Cave.wav";
	static constexpr const char *kCelebrationAnimationPath = "bmp/zombis/attente/attenteZomb.anm";
	/** Counts for physical lanes, decision tests, exit slots, and animation-bank frame roles. */
	static constexpr int kLaneCount = 4;
	static constexpr int kRuleCount = 3;
	static constexpr int kEndpointCount = 33;
	static constexpr int kRevealAnimIndex = 0;
	static constexpr int kHitAnimIndex = 1;
	static constexpr int kFinishAnimIndex = 2;
	static constexpr int kHideAnimIndex = 3;
	static constexpr int kIdleAnimIndex = 4;

	/** One trait test in the four-leaf decision tree. */
	struct TreeNode {
		/** Trait index tested by this node. */
		ZmbTrait::TraitIndex traitIndex;
		/** Primary value that selects the matching branch. */
		byte primaryValue;
		/** Second matching value used at difficulty 3. */
		byte alternateValue;
	};

	/** Exit position and route code for one completed ride. */
	struct PathPoint {
		/** Destination screen X coordinate. */
		int x;
		/** Destination screen Y coordinate. */
		int y;
		/** Decision-tree leaf code associated with this point. */
		int routeCode;
	};

	/** One page-local speech request and whether it gates another collision. */
	struct SpeechEntry {
		/** Resource path and collision gate associated with one queued speech item. */
		Common::String path;
		bool collision;
	};

	/** Starting positions for the eight-member route party. */
	static constexpr Common::Point32 kStartPositions[8] = {
		Common::Point32(7, 128),
		Common::Point32(27, 167),
		Common::Point32(66, 111),
		Common::Point32(39, 59),
		Common::Point32(103, 80),
		Common::Point32(135, 121),
		Common::Point32(179, 82),
		Common::Point32(218, 101),
	};
	/** Four physical obstacle positions on the lower trail. */
	static constexpr Common::Point32 kObstaclePositions[kLaneCount] = {
		Common::Point32(163, 366),
		Common::Point32(295, 398),
		Common::Point32(428, 402),
		Common::Point32(551, 362),
	};
	/** Candidate endpoints for classified riders. */
	static constexpr PathPoint kExitPoints[kEndpointCount] = {
		{41, 417, 3},
		{77, 462, 3},
		{81, 511, 3},
		{76, 416, 3},
		{122, 511, 3},
		{122, 461, 3},
		{227, 531, 4},
		{269, 539, 4},
		{325, 509, 4},
		{210, 498, 4},
		{245, 488, 4},
		{284, 500, 4},
		{511, 539, 5},
		{548, 520, 5},
		{424, 520, 5},
		{469, 524, 5},
		{518, 500, 5},
		{460, 496, 5},
		{731, 377, 6},
		{736, 424, 6},
		{733, 459, 6},
		{681, 377, 6},
		{687, 463, 6},
		{690, 421, 6},
		{810, 470, 6},
		{810, 470, 6},
		{810, 470, 6},
		{810, 470, 6},
		{810, 470, 6},
		{810, 470, 6},
		{810, 470, 6},
		{810, 470, 6},
		{810, 470, 6},
	};
	/** Trait-hint positions at difficulties 1 and 2. */
	static constexpr Common::Point32 kEasyHints[3] = {
		Common::Point32(648, 123),
		Common::Point32(619, 188),
		Common::Point32(689, 188),
	};
	/** Trait-hint positions at difficulty 3. */
	static constexpr Common::Point32 kHardHints[4] = {
		Common::Point32(648, 111),
		Common::Point32(642, 141),
		Common::Point32(617, 176),
		Common::Point32(613, 206),
	};

	/** Capture the dropped rider after a release over the board target. */
	static void onBoardDrop(void *context, int targetIndex, int zoombiniIndex);
	/** Start the PAT ride for puzzle-roster entry @p zoombiniIndex. */
	void captureZoombini(int zoombiniIndex);
	/** Load trait icons, board visuals, scenery, and obstacle animations. */
	void loadGraphics();
	/** Generate the decision tree for the selected difficulty. */
	void generateTree();
	/** Pick a value for @p traitIndex that occurs in the active roster. */
	byte pickPresentValue(ZmbTrait::TraitIndex traitIndex);
	/** Count currently selectable Zoombinis in each classified lane. */
	void countLanes(byte (&counts)[kLaneCount]) const;
	/** Classify @p zoombini and return its destination lane. */
	int classifyZoombini(const ZoombiniRunner *zoombini) const;
	/** Hide an obstacle in one lane that still has a selectable rider. */
	void chooseOpenLane();
	/** Append the exit curve for @p zoombini's classified route. */
	void startExitPath(ZoombiniRunner *zoombini);
	/** Mark @p zoombini successful after its exit path finishes. */
	void finishRide(ZoombiniRunner *zoombini);
	/** Count contact between @p zoombini and each visible obstacle. */
	void checkObstacleCollision(ZoombiniRunner *zoombini);
	/** Queue and advance collision or completion speech. */
	void enqueueSpeech(const Common::String &path, bool collision);
	void pumpSpeechQueue();
	/** Whether a collision reaction still blocks an obstacle change. */
	bool hasActiveHitAnimation(uint32 now) const;
	/** Advance the board pose toward the active rider's path direction. */
	void updateBoardFacing(ZoombiniRunner *zoombini);
	/** Draw a visible obstacle's introduction or idle sequence. */
	void drawObstacle(ManagedSurface32 *screen, int lane, uint32 now) const;
	/** Draw the value of @p node at @p pos, choosing its alternate when requested. */
	void drawTraitHint(ManagedSurface32 *screen, const TreeNode &node, bool alternate, const Common::Point32 &pos) const;

	/** Generated root and child trait tests for the current party. */
	TreeNode _tree[kRuleCount] = {};
	/** Board drop region used when a dragged rider is released. */
	Common::Array<ZmbDropTarget> _dropTargets;
	/** Shared celebration grid for successful riders after completion. */
	const ZoombiniAnimation *_celebrationAnimation = nullptr;
	/** One-shot board cover after the final rider is captured. */
	Animation *_boardAnim = nullptr;
	/** Lift engine animation at the upper station. */
	Animation *_engineAnim = nullptr;
	/** Loaded obstacle and scenery animations. */
	Animation *_obstacleAnims[5] = {};
	/** Whether each physical obstacle is visible. */
	bool _obstacleVisible[kLaneCount] = {};
	/** Whether the active rider has already contacted each obstacle. */
	bool _obstacleHit[kLaneCount] = {};
	/** Whether a lane is playing its disappear sequence. */
	bool _obstacleHiding[kLaneCount] = {};
	/** Whether a lane is playing its squash and recovery sequence. */
	bool _obstacleHitAnimating[kLaneCount] = {};
	/** A lane change waits for collision speech and squash animations. */
	bool _obstacleChangePending = false;
	/** Exit points already assigned to completed rides. */
	bool _usedExitPoint[kEndpointCount] = {};
	/** Collision threshold after which no further riders are accepted. */
	int _collisionQuota = 2;
	/** Number of visible-obstacle contacts during this puzzle instance. */
	int _collisionCount = 0;
	/** Contacts since the last two-hit obstacle change request. */
	int _hitsSinceRetarget = 0;
	/** Number of riders that have completed an exit path. */
	int _ridesCompleted = 0;
	/** Puzzle-roster index on the active board, or -1 between rides. */
	int _activeRunnerIndex = -1;
	/** Leaf code used to choose the active rider's exit path. */
	int _targetRouteCode = -1;
	/** Whether the active rider has entered the one-segment exit path. */
	bool _onExitPath = false;
	/** Whether the page has stopped accepting board drops. */
	bool _finished = false;
	/** Whether the station has a board available for the next rider. */
	bool _boardReady = true;
	/** One-shot board-cover sequence after the last rider is captured. */
	uint32 _boardCoverAnimationStart = 0;
	bool _boardCoverActive = false;
	/** Tick at which the board generator began its current one-shot cycle. */
	uint32 _generatorAnimationStart = 0;
	/** Whether the board generator is playing its one-shot cycle. */
	bool _generatorActive = false;
	/** Current board-facing sector, from 0 through 8. */
	int _boardFacingSector = 8;
	/** Number of movement updates since the last board-facing adjustment. */
	int _boardFacingDelay = 100;
	/** Tick at which each visible obstacle began its introduction. */
	uint32 _obstacleRevealStart[kLaneCount] = {};
	/** Tick at which each obstacle began hiding or reacting to a hit. */
	uint32 _obstacleHideStart[kLaneCount] = {};
	uint32 _obstacleHitStart[kLaneCount] = {};
	/** Tick at which the page began its completion animation. */
	uint32 _finishAnimationStart = 0;
	/** Page-local speech playback and queued paths. */
	Common::Array<SpeechEntry> _speechQueue;
	uint _nextSpeechIndex = 0;
	int _speechSoundId = -1;
	bool _activeSpeechIsCollision = false;
	bool _collisionSpeechPending = false;
	/** Whether Go is waiting for its page speech before transition. */
	bool _goTransitionPending = false;
	/** Sound handles for board creation, riding, obstacle contact, and lane changes. */
	int _boardReadySound = -1;
	int _rideSound = -1;
	int _obstacleHitSound = -1;
	int _obstacleRevealSound = -1;
	int _obstacleHideSound = -1;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_SNOWBOARD_H
