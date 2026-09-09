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
 * Cube-graph puzzle that moves a ball between nodes to release Zoombinis.
 *
 * The lower difficulties use an eight-node graph, while the upper difficulties
 * use a sixteen-node graph. Higher levels add Fleens and expose the warp control.
 */
class AquacubePuzzle : public PuzzlePage {
public:
	/** Construct the Aqua Cube puzzle for @p engine. */
	AquacubePuzzle(Zoombini2Engine *engine);
	/** Release graph sprites and animations. */
	~AquacubePuzzle() override;

	/** Load the graph selected by difficulty and place all puzzle actors. */
	void init() override;
	/** Advance ball, match, penalty, and warp phases. */
	void update() override;
	/** Draw the cube graph, actors, controls, and remaining-step display. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Start a direction move or toggle the warp control. */
	void handleClick(const Common::Point &pos) override;

private:
	/** One cube-graph vertex with adjacency, display, and occupant state. */
	struct GraphNode {
		/** Adjacent vertex indices, with `-1` marking a missing edge. */
		int adj[4];
		/** Signed 32-bit screen position of this vertex. */
		Common::Point32 position;
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

	/** Difficulty level in the range one through four. */
	int _difficulty;

	/** Number of active vertices in @ref AquacubePuzzle::_nodes. */
	int _numNodes;
	/** Graph storage sized for the largest difficulty. */
	GraphNode _nodes[16];
	/** Vertex currently occupied by the ball. */
	int _ballNode;
	/** Destination vertex while the ball is moving. */
	int _targetNode;

	/** Current screen position of the moving ball. */
	Common::Point32 _ballPosition;
	/** Ball position at the start of the current move. */
	Common::Point32 _ballStartPosition;
	/** Ball position at the end of the current move. */
	Common::Point32 _ballEndPosition;
	/** Time at which the current ball movement began. */
	uint32 _moveStartTime;
	/** Duration of one graph-edge movement in milliseconds. */
	static const uint32 kMoveAnimDuration = 680;

	/** Number of Zoombinis placed on the graph for this difficulty. */
	int _numZoombinisToPlace;
	/** Initial movement allowance for this difficulty. */
	int _totalSteps;
	/** Number of Fleen obstacles placed on the graph. */
	int _numFleens;
	/** Number of ball movements already consumed. */
	int _stepsUsed;
	/** Maximum number of ball movements allowed. */
	int _maxSteps;

	/** Offset applied when drawing Zoombinis at graph vertices. */
	Common::Point32 _zoombiniOffset;
	/** Offset applied when drawing Fleens at graph vertices. */
	Common::Point32 _fleenOffset;
	/** Offset applied when drawing graph-vertex markers. */
	Common::Point32 _nodeOffset;

	/** Whether the selected difficulty exposes the warp control. */
	bool _warpAvailable;
	/** Whether direction clicks are currently building a warp sequence. */
	bool _warpActive;
	/** Planned sequence of direction indices. */
	Common::Array<int> _warpQueue;
	/** Index of the warp movement currently being executed. */
	int _warpQueueIdx;

	/** Direction cursor light. */
	RleBlock *_lightGfx;
	/** Three cube layers drawn behind and around the actors. */
	RleBlock *_cubeGfx[3];
	/** Enabled joystick visual. */
	RleBlock *_manetteOnGfx;
	/** Disabled joystick visual. */
	RleBlock *_manetteOffGfx;
	/** Normal ball visual. */
	RleBlock *_ballGfx;
	/** Enlarged ball visual used during movement effects. */
	RleBlock *_ballBigGfx;
	/** Active movement-counter mark. */
	RleBlock *_shotsOnGfx;
	/** Consumed movement-counter mark. */
	RleBlock *_shotsOffGfx;
	/** Red direction indicator. */
	RleBlock *_lightRedGfx;
	/** Inactive direction indicator. */
	RleBlock *_lightGreyGfx;
	/** Active warp-control visual. */
	RleBlock *_warpOnGfx;
	/** Available warp-control visual. */
	RleBlock *_warpOffGfx;
	/** Disabled warp-control visual. */
	RleBlock *_warpDisableGfx;
	/** Warp timing effect. */
	Animation *_warpTimerAnim;
	/** Bubble effects used at occupied graph vertices. */
	RleBlock *_bubbleGfx[3];
	/** Fleen visuals indexed by obstacle variant. */
	RleBlock *_fleenGfx[4];
	/** Flare effects used while releasing occupants. */
	Animation *_flareAnims[2];

	/** Current interaction phase. */
	GameState _gameState;
	/** Number of Zoombinis successfully released. */
	int _freedCount;

	/** Direction labels for the eight-node cube. */
	static const char kGraph1DirLabels[8][4];
	/** Direction labels for the sixteen-node cube. */
	static const char kGraph2DirLabels[16][4];

	/** Populate the graph topology selected by difficulty. */
	void loadGraph();
	/** Assign puzzle-roster entries to graph vertices. */
	void placeZoombinis();
	/** Place the difficulty-selected number of Fleen obstacles. */
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
	int _musicId;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_AQUACUBE_H
