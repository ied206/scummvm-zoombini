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
	/** Release loaded sounds and page-managed animations. */
	~PuzzleSnowboard() override;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	/** Return the 1-based route containing this puzzle. */
	int getRouteNumber() const override { return kRouteNumber; }
	/** Return the 1-based puzzle number within its route. */
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }

	/** Generate the decision tree and place the puzzle roster at the start. */
	void init() override;
	/** Advance the current ride, obstacle checks, and exit path. */
	void onUpdate() override;
	/** Restore the primary page layer, including the baked restored L4 trails. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw the board, scenery, decision hints, and visible obstacles. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Draw the current board rider and the puzzle roster. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Advance Zoombini animation frames after they have been drawn. */
	void onActorsRendered() override;
	/** Handle release at @p pos and start a ride when the board receives the rider. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Update a held Zoombini as the pointer moves to @p pos. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Queue the original Go speech before leaving journey mode. */
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
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = "Snowboard Gulch";
	/** 1-based route number used by the puzzle console. */
	static constexpr int kRouteNumber = 4;
	/** 1-based puzzle number within the route used by the puzzle console. */
	static constexpr int kRoutePuzzleIdx = 1;
	/** Snowboard Gulch slope background loaded into the first page layer. */
	static constexpr const char *kBackgroundPath = "#bmp/snowboard/snowboard-EASY";
	/** Looping Snowboard Gulch soundtrack. */
	static constexpr const char *kMusicPath = "#sounds/music/01-BS06.wav";
	/** Path format for the RLE icon of a feature and one of its values. */
	static constexpr const char *kTraitFormat = "bmp/snowboard/traits/%d-%d";
	/** PAT route format for the hill ride selected by a decision-tree leaf code. */
	static constexpr const char *kPatFormat = "bmp/snowboard/pat/easy/%d.pat";
	/** PAT route format for level 4 hill paths selected by their route code. */
	static constexpr const char *kL4PatFormat = "bmp/snowboard/pat/hard/%d.pat";
	/** RLE cover drawn over the finish area from L2, L3 and L4. */
	static constexpr const char *kBoardPath = "bmp/snowboard/BOARD01";
	/** One-shot animation that covers the finish area during the ride sequence. */
	static constexpr const char *kBoardAnimPath = "bmp/snowboard/BOARD";
	/** Lift-engine animation shown while the upper station prepares a rider. */
	static constexpr const char *kEngineAnimPath = "bmp/snowboard/ENGINE";
	/** Path format for numbered trail-obstacle animations.
	 * IDs 1, 3, 4, 5, and 6 provide reveal, hit, completion, hide, and idle states.
	 */
	static constexpr const char *kDecorFormat = "bmp/snowboard/N1So-%d";
	/** Path format for the parked snowboard and the rider-facing board sprites. */
	static constexpr const char *kSurfFormat = "bmp/snowboard/SURF/SN%d";
	/** Resource IDs indexed by parked-board state or rider-facing sector. */
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
	/** One-bit mask that limits where a held Zoombini can be dropped onto the board. */
	static constexpr const char *kAreaMaskPath = "bmp/snowboard/AREA.BMT";
	/** Sound played when the lift board is ready for another rider. */
	static constexpr const char *kBoardReadySoundPath = "sounds/fx/01-BB02.wav";
	/** Sound played when a Zoombini starts down the hill. */
	static constexpr const char *kRideSoundPath = "sounds/fx/01-BS01.wav";
	/** Sound played on contact with a trail obstacle. */
	static constexpr const char *kObstacleHitSoundPath = "sounds/fx/01-BS02.wav";
	/** Sound played when a lane obstacle becomes visible. */
	static constexpr const char *kObstacleRevealSoundPath = "sounds/fx/01-BS03.wav";
	/** Sound played when a lane obstacle is hidden. */
	static constexpr const char *kObstacleHideSoundPath = "sounds/fx/01-BS04.wav";
	/** Collision speech path format, selected by lane number and one of three voice variants. */
	static constexpr const char *kCollisionSpeechFormat = "sounds/SWB13%d-%d.wav";
	/** Speech path format used one collision before the allowed quota is reached. */
	static constexpr const char *kNearQuotaSpeechFormat = "sounds/SWB14-%d.wav";
	/** Speech path format used when the collision quota is reached. */
	static constexpr const char *kQuotaSpeechFormat = "sounds/SWB15-%d.wav";
	/** Arrival speech queued when all eight riders reach the finish. */
	static constexpr const char *kSuccessSpeechPath = "sounds/8-E1.wav";
	/** Arrival speech queued when fewer than eight riders reach the finish. */
	static constexpr const char *kFailureSpeechPath = "sounds/8-E2.wav";
	/** One of five randomized praise lines queued for a perfect journey departure. */
	static constexpr const char *kPerfectGoSpeechFormat = "sounds/WLD11.%d.wav";
	/** Retreat speech queued before leaving with four or more riders unresolved. */
	static constexpr const char *kCaveGoSpeechPath = "sounds/DW-Cave.wav";
	/** L1-L3 physical lane count. */
	static constexpr int kLaneCount = 4;
	/** Number of lanes used by L4. */
	static constexpr int kL4LaneCount = 8;
	/** Number of trait-test nodes in the standard decision tree. */
	static constexpr int kRuleCount = 3;
	/** Number of entries in the standard ride endpoint table. */
	static constexpr int kEndpointCount = 33;
	/** Maximum number of trait-test nodes, including the L4 tree. */
	static constexpr int kMaxRuleCount = 7;
	/** Number of entries in the ㅣ4 endpoint table. */
	static constexpr int kL4EndpointCount = 48;
	/** Obstacle animation slot used when a hidden obstacle is revealed. */
	static constexpr int kRevealAnimIndex = 0;
	/** Obstacle animation slot used for a collision reaction. */
	static constexpr int kHitAnimIndex = 1;
	/** Obstacle animation slot used after a completed ride. */
	static constexpr int kFinishAnimIndex = 2;
	/** Obstacle animation slot used when an obstacle is concealed. */
	static constexpr int kHideAnimIndex = 3;
	/** Obstacle animation slot used while waiting for a rider. */
	static constexpr int kIdleAnimIndex = 4;

	/** One trait test, or two different trait tests joined by OR. */
	struct TreeNode {
		/** Trait index tested by this node. */
		ZmbTrait::TraitKind traitIndex;
		/** Second trait tested at level 4. */
		ZmbTrait::TraitKind alternateTraitIndex;
		/** Primary value that selects the matching branch. */
		byte primaryValue;
		/** Second matching value used at difficulties 3 and 4. */
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
	/** Retained eight-route anchors ordered by ascending PAT leaf code. */
	static constexpr Common::Point32 kL4ObstaclePositions[kL4LaneCount] = {
		Common::Point32(92, 347),
		Common::Point32(128, 443),
		Common::Point32(241, 427),
		Common::Point32(323, 432),
		Common::Point32(405, 437),
		Common::Point32(491, 415),
		Common::Point32(580, 373),
		Common::Point32(612, 283),
	};
	/**
	 * Retained exit slots for L4, including its offscreen overflow positions.
	 * These exit points were excavated from the Prerelease US Demo.
	 */
	static constexpr PathPoint kL4ExitPoints[kL4EndpointCount] = {
		{21, 341, 7},
		{26, 387, 7},
		{51, 366, 7},
		{-51, 366, 7},
		{-51, 366, 7},
		{-51, 366, 7},
		{75, 491, 8},
		{76, 448, 8},
		{115, 483, 8},
		{76, 650, 8},
		{76, 650, 8},
		{76, 650, 8},
		{191, 514, 9},
		{240, 533, 9},
		{219, 506, 9},
		{240, 650, 9},
		{240, 650, 9},
		{240, 650, 9},
		{303, 529, 10},
		{352, 516, 10},
		{325, 503, 10},
		{250, 650, 10},
		{250, 650, 10},
		{250, 650, 10},
		{407, 517, 11},
		{457, 503, 11},
		{435, 525, 11},
		{511, 650, 11},
		{511, 650, 11},
		{511, 650, 11},
		{516, 526, 12},
		{566, 512, 12},
		{532, 490, 12},
		{516, 650, 12},
		{516, 650, 12},
		{516, 650, 12},
		{675, 498, 13},
		{712, 469, 13},
		{669, 459, 13},
		{850, 450, 13},
		{850, 450, 13},
		{850, 450, 13},
		{741, 350, 14},
		{741, 308, 14},
		{703, 327, 14},
		{850, 310, 14},
		{850, 310, 14},
		{850, 310, 14},
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
	/** Trait-hint positions at level 3. */
	static constexpr Common::Point32 kHardHints[4] = {
		Common::Point32(648, 111),
		Common::Point32(642, 141),
		Common::Point32(617, 176),
		Common::Point32(613, 206),
	};
	/** Primary depth hints followed by alternate depth hints at level 4. */
	static constexpr Common::Point32 kL4Hints[6] = {
		Common::Point32(648, 111),
		Common::Point32(642, 141),
		Common::Point32(617, 176),
		Common::Point32(613, 206),
		Common::Point32(163, 366),
		Common::Point32(295, 398),
	};
	/** Capture the dropped rider after a release over the board target. */
	static void onBoardDrop(void *context, int targetIndex, int zoombiniIndex);
	/** Start the PAT ride for puzzle-roster entry @p zoombiniIndex. */
	void captureZoombini(int zoombiniIndex);
	/** Load trait icons, board visuals, scenery, and obstacle animations. */
	void loadGraphics();
	/**
	 * Bake the eight restored L4 trails into the available slope background during initialization.
	 *
	 * The HARD background bitmap is missing, so the trails provide a visual guide over the EASY artwork.
	 * A temporary transparent surface keeps overlapping routes at a uniform opacity before composition.
	 */
	void bakeL4Trail();
	/** Draw the six full-size depth hints at the retained demo positions. */
	void drawL4Legend(ManagedSurface32 *screen) const;
	/** Generate the decision tree for the selected level. */
	void generateTree();
	/** Generate and rank bounded candidates for the retained fourth-tier rules. */
	void generateL4Tree();
	/** Generate one original two-trait OR tree in the demo's RNG order. */
	void generateL4Candidate();
	/** Scale a random sample using the demo's range-selection formula. */
	int drawL4Value(int count);
	/** Preserve the demo's nested boolean comparisons in its retry gate. */
	bool hasRepeatedL4Rule() const;
	/** Pick a value for @p traitIndex that occurs in the active roster. */
	byte pickPresentValue(ZmbTrait::TraitKind traitIndex);
	/** Count currently selectable Zoombinis in each classified lane. */
	void countLanes(Common::Array<byte> &counts) const;
	/** Return the authored obstacle position for the selected lane family. */
	const Common::Point32 &obstaclePosition(int lane) const;
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
	/** Queue collision or completion speech through the serialized game queue. */
	void enqueueSpeech(const Common::String &path, bool collision);
	/** Whether a collision reaction still blocks an obstacle change. */
	bool hasActiveHitAnimation(uint32 now) const;
	/** Advance the board pose toward the active rider's path direction. */
	void updateBoardFacing(ZoombiniRunner *zoombini);
	/** Draw a visible obstacle's introduction or idle sequence. */
	void drawObstacle(ManagedSurface32 *screen, int lane, uint32 now) const;
	/** Draw the value of @p node at @p pos, choosing its alternate when requested. */
	void drawTraitHint(ManagedSurface32 *screen, const TreeNode &node, bool alternate, const Common::Point32 &pos) const;

	/** Generated root and child trait tests for the current party. */
	TreeNode _tree[kMaxRuleCount] = {};
	/** Number of lanes enabled by the current difficulty. */
	int _laneCount = kLaneCount;
	/** Number of active trait-test nodes for the current difficulty. */
	int _ruleCount = kRuleCount;
	/** Board drop region used when a held rider is released. */
	Common::Array<ZmbDropTarget> _dropTargets;
	/** One-shot board cover after the final rider is captured. */
	Animation *_boardAnim = nullptr;
	/** Lift engine animation at the upper station. */
	Animation *_engineAnim = nullptr;
	/** Loaded obstacle and scenery animations. */
	Animation *_obstacleAnims[5] = {};
	/** Obstacle visibility by lane: 4 elements for L1-L3 and 8 for L4. */
	Common::Array<bool> _obstacleVisible;
	/** Rider hit state by lane: 4 elements for L1-L3 and 8 for L4. */
	Common::Array<bool> _obstacleHit;
	/** Hide-animation state by lane: 4 elements for L1-L3 and 8 for L4. */
	Common::Array<bool> _obstacleHiding;
	/** Hit-animation state by lane: 4 elements for L1-L3 and 8 for L4. */
	Common::Array<bool> _obstacleHitAnimating;
	/** A lane change waits for collision speech and squash animations. */
	bool _obstacleChangePending = false;
	/** Exit points already assigned to completed rides. */
	bool _usedExitPoint[kL4EndpointCount] = {};
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
	/** Whether the board-cover presentation is currently active. */
	bool _boardCoverActive = false;
	/** Tick at which the board generator began its current one-shot cycle. */
	uint32 _generatorAnimationStart = 0;
	/** Whether the board generator is playing its one-shot cycle. */
	bool _generatorActive = false;
	/** Current board-facing sector, from 0 through 8. */
	int _boardFacingSector = 8;
	/** Number of movement updates since the last board-facing adjustment. */
	int _boardFacingDelay = 100;
	/** Obstacle reveal-start ticks by lane: 4 elements for L1-L3 and 8 for L4. */
	Common::Array<uint32> _obstacleRevealStart;
	/** Obstacle hide-start ticks by lane: 4 elements for L1-L3 and 8 for L4. */
	Common::Array<uint32> _obstacleHideStart;
	/** Obstacle hit-start ticks by lane: 4 elements for L1-L3 and 8 for L4. */
	Common::Array<uint32> _obstacleHitStart;
	/** Tick at which the page began its completion animation. */
	uint32 _finishAnimationStart = 0;
	/** Whether the shared speech queue still gates another collision. */
	bool _collisionSpeechPending = false;
	/** Whether Go is waiting for serialized speech before transition. */
	bool _goTransitionPending = false;
	/** Sound handles for board creation, riding, obstacle contact, and lane changes. */
	int _boardReadySound = -1;
	/** Loaded snowboard riding sound handle, or -1 when unavailable. */
	int _rideSound = -1;
	/** Loaded obstacle collision sound handle, or -1 when unavailable. */
	int _obstacleHitSound = -1;
	/** Loaded obstacle reveal sound handle, or -1 when unavailable. */
	int _obstacleRevealSound = -1;
	/** Loaded obstacle concealment sound handle, or -1 when unavailable. */
	int _obstacleHideSound = -1;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_SNOWBOARD_H
