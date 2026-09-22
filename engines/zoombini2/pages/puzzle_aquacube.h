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

#ifndef ZOOMBINI2_PAGES_PUZZLE_AQUACUBE_H
#define ZOOMBINI2_PAGES_PUZZLE_AQUACUBE_H

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class Animation;
class AnimationRunner;
struct PathObject;

/** Move through a randomized cube, rescuing its occupants before the movement allowance expires. */
class PuzzleAquacube : public PuzzleBase {
public:
	/** Bind AquaCube puzzle state to @p vm. */
	PuzzleAquacube(Zoombini2Engine *vm);
	/** Release movement paths, page runners, animations, and loaded audio. */
	~PuzzleAquacube() override;
	/** Load page resources and generate the current difficulty's randomized board. */
	void init() override;
	/** Advance movement, warp, rescue, chase, departure, and bubble state. */
	void onUpdate() override;
	/** Restore and draw the shared puzzle background layer. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw cube layers, nodes, controls, and the move or warp displays. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Draw the active Zoombini roster over the cube. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Advance roster animation state after the actor render boundary. */
	void onActorsRendered() override;
	/** Draw bubbles, the moving light, and effect runners over the board. */
	void onRenderForeground(ManagedSurface32 *screen) override;
	/** Select a warp, queue a warp lever, or start a direct lever move on button release. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Enable Go after at least one rescue and after any deferred departure finishes. */
	bool canUseGoButton() const override;
	/** Begin saved-game departure speech when enough unrescued actors remain. */
	bool onGoButtonPressed() override;
	/** Describe the current generated graph, lever mapping, and warp behavior. */
	Common::String debugGetAnswer() const override;
	/** Report completed moves against the current move allowance. */
	PuzzleChanceInfo debugGetChances() const override;
	/** Return whether the move allowance can be changed without interrupting page work. */
	bool debugCanSetChances() const override;
	/** Set the number of moves remaining and finish the puzzle when none remain. */
	bool debugSetChances(int remaining) override;
	/** Report warp and rescue progress alongside the move allowance. */
	Common::String debugGetChanceDetails() const override;

private:
	/** Looping page soundtrack. */
	static constexpr const char *kMusicPath = "#sounds/music/03-BB01.wav";
	/** Glow drawn behind the moving light. */
	static constexpr const char *kLightPath = "bmp/aquacube/light";
	/** Node sprites indexed by easy or medium versus hard board rendering. */
	static constexpr const char *kBallPaths[2] = {
		"bmp/aquacube/ballBIG",
		"bmp/aquacube/ball",
	};
	/** Cube layer path format, parameterized by board kind and layer number. */
	static constexpr const char *kCubeFormat = "bmp/aquacube/kub_%s_%02d";
	/** Lever sprites indexed by off and on state. */
	static constexpr const char *kLeverPaths[2] = {
		"bmp/aquacube/control_manetteOFF",
		"bmp/aquacube/control_manetteON",
	};
	/** Lever indicator sprites indexed by off and on state. */
	static constexpr const char *kIndicatorPaths[2] = {
		"bmp/aquacube/control_manette_lightGREY",
		"bmp/aquacube/control_manette_lightRED",
	};
	/** Move-gauge sprites indexed by unused and completed move state. */
	static constexpr const char *kShotPaths[2] = {
		"bmp/aquacube/control_shotsOFF",
		"bmp/aquacube/control_shotsON",
	};
	/** Warp-button sprites indexed by @ref PuzzleAquacube::WarpButtonState. */
	static constexpr const char *kWarpPaths[3] = {
		"bmp/aquacube/control_warpBUTTON_OFF",
		"bmp/aquacube/control_warpBUTTON_ON",
		"bmp/aquacube/control_warpBUTTON_DISABLE",
	};
	/** Empty timer panel drawn behind the warp animation. */
	static constexpr const char *kTimerEmptyPath = "bmp/aquacube/control_warpTIMER_empty";
	/** Timed warp-planning animation. */
	static constexpr const char *kTimerPath = "bmp/aquacube/control_warpTIMER";
	/** Rescue flare animation path format. */
	static constexpr const char *kFlareFormat = "bmp/aquacube/flare%d";
	/** Cosmetic bubble sprite path format. */
	static constexpr const char *kBubbleFormat = "bmp/aquacube/bubble%d";
	/** Idle Fleen sprite path format. */
	static constexpr const char *kFleenFormat = "bmp/aquacube/fleen/fixe/f%dfixe";
	/** Angry Fleen animation path format. */
	static constexpr const char *kAngryFormat = "bmp/aquacube/fleen/vener/f%dma66";
	/** Fleen chase animation path format. */
	static constexpr const char *kChaseFormat = "bmp/aquacube/fleen/marche/f%dco66";
	/** Small Zoombini animation used on the cube and during the chase. */
	static constexpr const char *kSmallestPath = "bmp/aquacube/smallest/smallest.anm";
	/** Small Zoombini idle animation used after puzzle completion. */
	static constexpr const char *kIdlePath = "bmp/aquacube/smallest/attente/attente.anm";
	/** Sound effects indexed by lever, rescue, warp, move, and Fleen chase start. */
	static constexpr const char *kSoundPaths[5] = {
		"sounds/fx/03-BS01.wav",
		"sounds/fx/03-BS03.wav",
		"sounds/fx/03-BS05.wav",
		"sounds/fx/03-BB02.wav",
		"sounds/fx/FleenChasesZs.wav",
	};
	/** Praise speech indexed by partial and full rescue result. */
	static constexpr const char *kPraisePaths[2] = {
		"sounds/8-E2.wav",
		"sounds/8-E1.wav",
	};
	/** Departure speech used before a saved-game map transition. */
	static constexpr const char *kRetreatPath = "sounds/DW-Zville.wav";

	/** Occupancy state associated with one graph node. */
	enum NodeState {
		/** Node retains a party group, possibly already collected. */
		kOccupied00 = 0,
		/** Node has neither a party group nor a Fleen. */
		kEmpty01 = 1,
		/** Node holds the current light position at board setup. */
		kStart02 = 2,
		/** Node holds one Fleen encounter. */
		kFleen03 = 3
	};
	/** Presentation state selected for the warp button. */
	enum WarpButtonState {
		/** Warp remains available but has not been selected. */
		kWarpButtonOff00 = 0,
		/** Warp planning is active and awaits selected levers. */
		kWarpButtonOn01 = 1,
		/** Warp is unavailable at the current difficulty or quota. */
		kWarpButtonDisabled02 = 2
	};
	/**
	 * Identifies one end of a logical cube axis.
	 *
	 * Node layouts list up/down, left/right, front/back, and outer/inner cube axes in that order.
	 * The very hard layout adds an outer/inner cube axis.
	 * Up, left, front, and outer cube select an axis's randomized orientation.
	 * Down, right, back, and inner cube select its opposite orientation.
	 * The unused label fills the fourth axis slot of three-dimensional layouts.
	 */
	enum NodeCoordinateLabel {
		kUp = 0,
		kDown = 1,
		kLeft = 2,
		kRight = 3,
		kFront = 4,
		kBack = 5,
		kOuter = 6,
		kInner = 7,
		kUnused = 8
	};
	/** Mutable graph state for one visible AquaCube bubble. */
	struct Node {
		/** Neighbor node indexed by logical axis, or -1 when the axis is absent. */
		int adj[4] = {};
		/** Screen anchor of this node's bubble. */
		Common::Point32 pos;
		/** Party, start, empty, or Fleen content at this node. */
		NodeState state = kEmpty01;
		/** Number of active party members represented by this node. */
		int occupantCount = 0;
		/** Indices into @ref PuzzleBase::_puzzleZoombinis for this node's party group. */
		int occupants[3] = {};
		/** Randomized binary coordinate used to resolve graph destinations. */
		int coordinates = 0;
		/** One-based Fleen asset and animation variant for a Fleen node. */
		int fleenType = 0;
	};
	/**
	 * Defines the graph, screen position, and logical coordinate labels of a cube node.
	 *
	 * The labels generate randomized node coordinates.
	 * They are not UI strings or resource names.
	 */
	struct NodeLayout {
		/** Neighbor node indices in logical-axis order. */
		int adj[4];
		/** Screen anchor used to initialize @ref PuzzleAquacube::Node::pos. */
		Common::Point32 pos;
		/** Side labels used to generate the randomized coordinate bits. */
		NodeCoordinateLabel labels[4];
	};
	/** Cosmetic bubble state updated independently of puzzle movement. */
	struct Bubble {
		/** Whether this bubble is visible and moving upward. */
		bool active = false;
		/** Zero-based sprite and vertical-speed variant. */
		int type = 0;
		/** Unoscillated horizontal source position. */
		int originX = 100;
		/** Subpixel horizontal screen position after sine-wave displacement. */
		float x = 100;
		/** Subpixel vertical screen position. */
		float y = 640;
		/** Sine-wave phase in radians. */
		float phase = 0;
	};
	/** Eight-node cube layout used by the first two difficulty tables. */
	static constexpr NodeLayout kEasyNodes[8] = {
		{{1, 3, 4, -1}, {162, 530}, {kDown, kLeft, kFront, kUnused}},
		{{0, 2, 5, -1}, {524, 530}, {kDown, kRight, kFront, kUnused}},
		{{3, 1, 6, -1}, {591, 175}, {kUp, kRight, kFront, kUnused}},
		{{2, 0, 7, -1}, {92, 175}, {kUp, kLeft, kFront, kUnused}},
		{{5, 7, 0, -1}, {238, 350}, {kDown, kLeft, kBack, kUnused}},
		{{4, 6, 1, -1}, {441, 350}, {kDown, kRight, kBack, kUnused}},
		{{7, 5, 2, -1}, {458, 136}, {kUp, kRight, kBack, kUnused}},
		{{6, 4, 3, -1}, {218, 136}, {kUp, kLeft, kBack, kUnused}},
	};
	/** Sixteen-node inner and outer cube layout used by the hard difficulty table. */
	static constexpr NodeLayout kHardNodes[16] = {
		{{1, 3, 12, 4}, {165, 533}, {kDown, kLeft, kFront, kOuter}},
		{{0, 2, 13, 5}, {527, 533}, {kDown, kRight, kFront, kOuter}},
		{{3, 1, 14, 6}, {594, 178}, {kUp, kRight, kFront, kOuter}},
		{{2, 0, 15, 7}, {95, 178}, {kUp, kLeft, kFront, kOuter}},
		{{5, 7, 8, 0}, {275, 403}, {kDown, kLeft, kFront, kInner}},
		{{4, 6, 9, 1}, {406, 403}, {kDown, kRight, kFront, kInner}},
		{{7, 5, 10, 2}, {419, 260}, {kUp, kRight, kFront, kInner}},
		{{6, 4, 11, 3}, {269, 260}, {kUp, kLeft, kFront, kInner}},
		{{9, 11, 4, 12}, {293, 335}, {kDown, kLeft, kBack, kInner}},
		{{8, 10, 5, 13}, {393, 335}, {kDown, kRight, kBack, kInner}},
		{{11, 9, 6, 14}, {403, 223}, {kUp, kRight, kBack, kInner}},
		{{10, 8, 7, 15}, {283, 223}, {kUp, kLeft, kBack, kInner}},
		{{13, 15, 0, 8}, {241, 353}, {kDown, kLeft, kBack, kOuter}},
		{{12, 14, 1, 9}, {444, 353}, {kDown, kRight, kBack, kOuter}},
		{{15, 13, 2, 10}, {461, 139}, {kUp, kRight, kBack, kOuter}},
		{{14, 12, 3, 11}, {221, 139}, {kUp, kLeft, kBack, kOuter}},
	};
	/** Shore positions assigned to party members in rescue order. */
	static constexpr Common::Point32 kRescuePositions[16] = {
		{775, 38},
		{778, 62},
		{760, 40},
		{763, 59},
		{750, 62},
		{744, 41},
		{734, 64},
		{730, 46},
		{722, 63},
		{715, 49},
		{707, 64},
		{695, 53},
		{692, 66},
		{676, 59},
		{674, 66},
		{663, 62},
	};
	/** Panel positions of the four levers in left-to-right order. */
	static constexpr Common::Point32 kLeverPositions[4] = {
		{649, 483},
		{674, 484},
		{698, 483},
		{725, 482},
	};
	/** Panel positions of the four lever indicators in left-to-right order. */
	static constexpr Common::Point32 kIndicatorPositions[4] = {
		{648, 526},
		{676, 527},
		{701, 526},
		{727, 525},
	};
	/** Horizontal positions of the move-gauge dots in completion order. */
	static constexpr int kShotX[11] = {
		647,
		658,
		667,
		676,
		685,
		695,
		705,
		713,
		723,
		730,
		738,
	};

	/** Randomize lever axes and directions, then populate nodes, Fleens, and the active party. */
	void setupBoard();
	/** Load every page-local sprite, animation, and sound resource. */
	void loadResources();
	/** Return the node whose generated binary coordinate equals @p coordinates. */
	int findNode(int coordinates) const;
	/** Mark the generated node at @p coordinates as a Fleen of @p type. */
	void putFleen(int coordinates, int type);
	/** Create and start one cubic movement path between two screen positions. */
	PathObject *makePath(const Common::Point32 &start, const Common::Point32 &end, int speed);
	/** Apply the optional safe-first-move axis remap before a direct lever move. */
	void protectFirstDirectMoveFromFleen(int lever);
	/** Start light movement along @p axis and update the current node immediately. */
	void moveBall(int axis);
	/** Consume a move and process rescue, Fleen, or final-move state at the light's node. */
	void resolveArrival();
	/** Mark the puzzle complete, select praise, and request sidebar Go attention. */
	void finishPuzzle();
	/** Start rescued actors and the Fleen on their escape paths. */
	void beginChase();
	/** Advance, spawn, and retire cosmetic bubbles using elapsed milliseconds. */
	void updateBubbles(uint32 elapsed);
	/** Play the indexed page sound effect when its handle was loaded successfully. */
	void playSound(int index);
	/** Count active party members that have not reached the rescue shore. */
	int countFreeZoombinis() const;
	/** Load one animation, retain it for page teardown, and configure a timed runner. */
	AnimationRunner *loadRunner(const Common::Path &path, int frames, int repeats, uint32 delay);
	/** Reveal a rescued group and start its second flare. */
	static void onFlareComplete(void *context, AnimationRunner *runner);
	/** Start the Fleen chase after its angry animation completes. */
	static void onAngryComplete(void *context, AnimationRunner *runner);
	/** Allow the timer to execute its queued warp levers. */
	static void onTimerComplete(void *context, AnimationRunner *runner);

	/** Selected difficulty table index. */
	int _level = 1;
	/** Number of active graph nodes for the selected board. */
	int _numNodes = 8;
	/** Number of active logical movement axes and levers. */
	int _dimensions = 3;
	/** Number of resolved arrivals allowed before the puzzle completes. */
	int _maxSteps = 6;
	/** Number of warp plans available to the selected difficulty. */
	int _warpQuota = 0;
	/** Index of the node that currently contains the light. */
	int _ballNode = 0;
	/** Number of resolved light arrivals. */
	int _stepsUsed = 0;
	/** Number of warp plans already started. */
	int _warpsUsed = 0;
	/** Number of party members delivered to the rescue shore before any chase reset. */
	int _freedCount = 0;
	/** Zero-based Fleen variant currently moving along the chase path. */
	int _fleenIndex = -1;
	/** Maps left-to-right visible levers to randomized logical axes. */
	int _axisMap[4] = {};
	/** Visual on state for each lever during a direct move or warp plan. */
	bool _leverOn[4] = {};
	/** Queued lever selections consumed in ascending index order by a warp. */
	bool _warpPending[4] = {};
	/** Whether the player is currently selecting levers for a warp. */
	bool _warpPlanning = false;
	/** Whether the expired warp timer is dispatching queued moves. */
	bool _warpExecuting = false;
	/** Whether the puzzle reached its move limit or completed roster processing. */
	bool _finished = false;
	/** Whether saved-game departure speech delays the map transition. */
	bool _goPending = false;
	/** Whether rescued actors still have active shore escape paths. */
	bool _actorsEscaping = false;
	/** Whether the first update must resolve the initial node without consuming a move. */
	bool _pendingInitialArrival = false;
	/** Active graph nodes for the selected difficulty layout. */
	Node _nodes[16];
	/** Cosmetic bubbles updated and drawn independently of graph nodes. */
	Bubble _bubbles[12];
	/** Roster indices revealed when the first rescue flare completes. */
	Common::Array<int> _rescued;
	/** Interpolated light position used by foreground rendering. */
	Common::Point32 _ballPos;
	/** Active path from the previous light node to the current light node. */
	PathObject *_ballPath = nullptr;
	/** Active path that carries the Fleen away after its chase. */
	PathObject *_chasePath = nullptr;
	/** Last game tick used to calculate bubble elapsed time. */
	uint32 _lastTick = 0;
	/** Loaded sound handles indexed like @ref PuzzleAquacube::kSoundPaths. */
	int _sounds[5] = {
		-1,
		-1,
		-1,
		-1,
		-1,
	};
	/** Timed runner that bounds warp-lever selection. */
	AnimationRunner *_timer = nullptr;
	/** First and second rescue flare runners. */
	AnimationRunner *_flare[2] = {};
	/** One angry runner for each Fleen variant. */
	AnimationRunner *_angry[4] = {};
	/** One chase runner for each Fleen variant. */
	AnimationRunner *_chase[4] = {};
	/** Frame data retained until the associated page runners have been released. */
	Common::Array<Animation *> _animations;
	/** Small Zoombini animation used for board placement and fleeing actors. */
	const ZoombiniAnimation *_smallest = nullptr;
	/** Small Zoombini idle animation selected after the puzzle ends. */
	const ZoombiniAnimation *_idle = nullptr;
};

} // End of namespace Zoombini2

#endif
