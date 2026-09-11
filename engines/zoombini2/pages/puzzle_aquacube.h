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

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class RleBlock;
class Animation;

/**
 * Aqua Cube (Route1-3)
 *
 * Use the levers to move the magic ball through the cube and rescue Zoombinis.
 */
class PuzzleAquacube : public PuzzleBase {
public:
	/** Construct the Aqua Cube puzzle for @p vm. */
	PuzzleAquacube(Zoombini2Engine *vm);
	/** Release graph sprites and animations. */
	~PuzzleAquacube() override;

	/** Load the graph selected by level and place all puzzle actors. */
	void init() override;
	/** Advance ball, match, penalty, and warp phases. */
	void onUpdate() override;
	/** Draw the cube graph, actors, controls, and remaining-step display. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw the layered cube and node sprites. */
	void onRenderScene(ManagedSurface32 *screen) override;
	/** Draw the moving ball. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Draw direction controls, indicators, and visual effects. */
	void onRenderForeground(ManagedSurface32 *screen) override;
	/** Start a direction move or toggle the warp control. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;

private:
	/** One cube-graph vertex with adjacency, display, and occupant state. */
	struct GraphNode {
		/** Adjacent vertex indices, with `-1` marking a missing edge. */
		int adj[4];
		/** Signed 32-bit screen position of this vertex. */
		Common::Point32 pos;
		/** Vertex role identifying occupants, the ball start, or a Fleen. */
		int state;
		/** Number of Zoombinis assigned to this vertex. */
		int occupantCount;
		/** Puzzle-roster indices assigned to this vertex. */
		int occupants[3];
		/** Binary graph coordinates used to resolve directional movement. */
		int dirValues[4];
		/** Fleen visual variant, or zero when no Fleen occupies the vertex. */
		int fleenType;
	};

	/** Runtime phase of the Aqua Cube interaction. */
	enum GameState {
		/** Wait for directional input. */
		kStateIdle,
		/** Animate the ball along a graph edge. */
		kStateBallMoving,
		/** Resolve the destination vertex. */
		kStateMatchCheck,
		/** Hold briefly after releasing Zoombinis. */
		kStateZoombiniFreed,
		/** Apply the penalty for reaching a Fleen. */
		kStateFleenHit,
		/** Collect a sequence of warp directions. */
		kStateWarpPlanning,
		/** Execute the collected warp directions. */
		kStateWarpExecuting,
		/** Stop accepting input after success or failure. */
		kStateDone
	};

	/** Level in the range one through four. */
	int _level = 1;

	/** Number of active vertices in @ref PuzzleAquacube::_nodes. */
	int _numNodes = 8;
	/** Graph storage sized for the largest level. */
	GraphNode _nodes[16];
	/** Vertex currently occupied by the ball. */
	int _ballNode = 0;
	/** Destination vertex while the ball is moving. */
	int _targetNode = -1;

	/** Current screen position of the moving ball. */
	Common::Point32 _ballPos = Common::Point32();
	/** Ball position at the start of the current move. */
	Common::Point32 _ballStartPos = Common::Point32();
	/** Ball position at the end of the current move. */
	Common::Point32 _ballEndPos = Common::Point32();
	/** Time at which the current ball movement began. */
	uint32 _moveStartTime = 0;
	/** Duration of one graph-edge movement in milliseconds. */
	static const uint32 kMoveAnimDuration = 680;

	/** Number of Zoombinis placed on the graph for this level. */
	int _numZoombinisToPlace = 3;
	/** Initial movement allowance for this level. */
	int _totalSteps = 6;
	/** Number of Fleen obstacles placed on the graph. */
	int _numFleens = 0;
	/** Number of ball movements already consumed. */
	int _stepsUsed = 0;
	/** Maximum number of ball movements allowed. */
	int _maxSteps = 6;

	/** Offset applied when drawing Zoombinis at graph vertices. */
	Common::Point32 _zoombiniOffset = Common::Point32(-2, 6);
	/** Offset applied when drawing Fleens at graph vertices. */
	Common::Point32 _fleenOffset = Common::Point32(10, 6);
	/** Offset applied when drawing graph-vertex markers. */
	Common::Point32 _nodeOffset = Common::Point32();

	/** Whether the selected level exposes the warp control. */
	bool _warpAvailable = false;
	/** Whether direction clicks are currently building a warp sequence. */
	bool _warpActive = false;
	/** Planned sequence of direction indices. */
	Common::Array<int> _warpQueue;
	/** Index of the warp movement currently being executed. */
	int _warpQueueIdx = 0;

	/** Direction cursor light. */
	RleBlock *_lightImage = nullptr;
	/** Three cube layers drawn behind and around the actors. */
	RleBlock *_cubeImage[3] = {};
	/** Enabled joystick visual. */
	RleBlock *_manetteOnImage = nullptr;
	/** Disabled joystick visual. */
	RleBlock *_manetteOffImage = nullptr;
	/** Normal ball visual. */
	RleBlock *_ballImage = nullptr;
	/** Enlarged ball visual used during movement effects. */
	RleBlock *_ballBigImage = nullptr;
	/** Active movement-counter mark. */
	RleBlock *_shotsOnImage = nullptr;
	/** Consumed movement-counter mark. */
	RleBlock *_shotsOffImage = nullptr;
	/** Red direction indicator. */
	RleBlock *_lightRedImage = nullptr;
	/** Inactive direction indicator. */
	RleBlock *_lightGreyImage = nullptr;
	/** Active warp-control visual. */
	RleBlock *_warpOnImage = nullptr;
	/** Available warp-control visual. */
	RleBlock *_warpOffImage = nullptr;
	/** Disabled warp-control visual. */
	RleBlock *_warpDisableImage = nullptr;
	/** Warp timing effect. */
	Animation *_warpTimerAnim = nullptr;
	/** Bubble effects used at occupied graph vertices. */
	RleBlock *_bubbleImage[3] = {};
	/** Fleen visuals indexed by obstacle variant. */
	RleBlock *_fleenImage[4] = {};
	/** Flare effects used while releasing occupants. */
	Animation *_flareAnims[2] = {};

	/** Current interaction phase. */
	GameState _gameState = kStateIdle;
	/** Number of Zoombinis successfully released. */
	int _freedCount = 0;

	/** Direction labels for the eight-node cube. */
	static const char kGraph1DirLabels[8][4];
	/** Direction labels for the sixteen-node cube. */
	static const char kGraph2DirLabels[16][4];

	/** Populate the graph topology selected by level. */
	void loadGraph();
	/** Assign puzzle-roster entries to graph vertices. */
	void placeZoombinis();
	/** Place the level-selected number of Fleen obstacles. */
	void placeFleens();
	/** Select the graph vertex at which the ball begins. */
	void placeBallStart();
	/** Begin moving the ball toward @p targetNode. */
	void startBallMove(int targetNode);
	/** Commit the current movement and begin destination resolution. */
	void finishBallMove();
	/** Find an eight-node graph vertex by its three binary coordinates. */
	int findNodeByDirValues3(int a, int b, int c) const;
	/** Find a sixteen-node graph vertex by its four binary coordinates. */
	int findNodeByDirValues4(int a, int b, int c, int d) const;
	/** Release every Zoombini assigned to vertex @p nodeIdx. */
	void freeZoombini(int nodeIdx);
	/** Return the number of puzzle-roster entries already released. */
	int countFreeZoombinis() const;
	/** Load all graphics and animations owned by the puzzle. */
	void loadResources();

	/** Music handle used while Aqua Cube is active. */
	int _musicId = -1;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_AQUACUBE_H
