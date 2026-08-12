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

#include <vector>

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class RleBlock;
class Animation;

/**
 * AquacubePuzzle - Aqua Cube: Cube graph navigation puzzle (World ID 3).
 *
 * Object size: 0x2C4 (708) bytes.
 *
 * Core mechanics:
 *   - Player controls a ball on a 3D cube graph (8 or 16 nodes)
 *   - Difficulty 1-2: 8-node cube (3 edges per vertex), 3 zoombinis
 *   - Difficulty 3-4: 16-node double cube (4 edges per vertex), 4 zoombinis
 *   - 4 direction buttons control ball movement between adjacent vertices
 *   - Zoombinis trapped at vertices; freed when ball reaches their node
 *   - Fleens are obstacles at certain vertices (diff 2+)
 *   - Step counter limits total moves (6 for diff 1-2, 11 for diff 3-4)
 *   - Warp button provides teleport moves (diff 2+)
 *   - Goal: Free at least 4 zoombinis
 *
 * Node structure: 72 bytes (18 DWORDs) each. See Aquacube-Mechanics.md KB.
 *
 * Blocking model: All logic runs in callbacks, vtable Tick is null.
 */
class AquacubePuzzle : public PuzzlePage {
public:
	AquacubePuzzle(Zoombini2Engine *engine);
	~AquacubePuzzle() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	/**
	 * Node links, screen position, occupants, and direction values.
	 */
	struct GraphNode {
		int adj[4];           ///< DWORD 0-3: Adjacent node indices (-1 = none)
		int x, y;             ///< DWORD 4-5: Screen position
		int state;            ///< DWORD 6: 0=zoombini, 1=empty, 2=ballStart, 3=fleen
		int occupantCount;    ///< DWORD 7: Number of zoombinis placed at this node
		int occupants[3];     ///< DWORD 8-10: Zoombini indices in puzzle list
		int dirValues[4];     ///< DWORD 13-16: Binary direction coords (0 or 1)
		int fleenType;        ///< DWORD 17 (byte 68): Fleen variant 0=none, 1-4
	};

	enum GameState {
		kStateIdle,           ///< Waiting for player direction input
		kStateBallMoving,     ///< Ball animating along graph edge
		kStateMatchCheck,     ///< Checking zoombini match at destination
		kStateZoombiniFreed,  ///< Zoombini freed — brief celebration
		kStateFleenHit,       ///< Ball hit a fleen — penalty
		kStateWarpPlanning,   ///< Planning a sequence of warp moves
		kStateWarpExecuting,  ///< Executing a planned warp sequence
		kStateDone            ///< Puzzle complete or failed
	};

	// Difficulty level (1-4)
	int _difficulty;

	// Graph
	int _numNodes;                    ///< 8 (diff 1-2) or 16 (diff 3-4)
	GraphNode _nodes[16];             ///< Static graph data
	int _ballNode;                    ///< Current ball position (node index)
	int _targetNode;                  ///< Destination during ball movement

	// Ball animation
	int _ballX, _ballY;              ///< Current ball screen position
	int _ballStartX, _ballStartY;    ///< Start of movement
	int _ballEndX, _ballEndY;        ///< End of movement
	uint32 _moveStartTime;
	static const uint32 kMoveAnimDuration = 680;

	// Difficulty parameters
	int _numZoombinisToPlace;         ///< 3 (diff 1-2) or 4 (diff 3-4)
	int _totalSteps;                  ///< 6 (diff 1-2) or 11 (diff 3-4)
	int _numFleens;                   ///< 0, 1, or 2
	int _stepsUsed;                   ///< Current step counter
	int _maxSteps;                    ///< Total allowed steps

	// Draw offsets for node sprites
	int _zoombiniOffX, _zoombiniOffY; ///< Offset for drawing zoombinis at nodes
	int _fleenOffX, _fleenOffY;       ///< Offset for drawing fleens at nodes
	int _nodeOffX, _nodeOffY;         ///< Offset for drawing node circles

	// Warp mechanics
	bool _warpAvailable;              ///< True if warp is available (diff > 1)
	bool _warpActive;                 ///< Warp mode toggled on
	std::vector<int> _warpQueue;      ///< Planned sequence of directions
	int _warpQueueIdx;                ///< Current move being executed in warp queue


	// Graphics
	RleBlock *_lightGfx;              ///< cursor light
	RleBlock *_cubeGfx[3];           ///< cube layers (3 layers)
	RleBlock *_manetteOnGfx;          ///< joystick ON
	RleBlock *_manetteOffGfx;         ///< joystick OFF
	RleBlock *_ballGfx;               ///< small ball
	RleBlock *_ballBigGfx;            ///< large ball
	RleBlock *_shotsOnGfx;            ///< shot counter ON
	RleBlock *_shotsOffGfx;           ///< shot counter OFF
	RleBlock *_lightRedGfx;           ///< direction light RED
	RleBlock *_lightGreyGfx;          ///< direction light GREY
	RleBlock *_warpOnGfx;             ///< warp ON
	RleBlock *_warpOffGfx;            ///< warp OFF
	RleBlock *_warpDisableGfx;        ///< warp DISABLE
	Animation *_warpTimerAnim;        ///< warp timer animation
	RleBlock *_bubbleGfx[3];          ///< bubble effects
	RleBlock *_fleenGfx[4];           ///< fleen fixed sprites (1-4)
	Animation *_flareAnims[2];        ///< FLARE1.AN, FLARE2.AN visual effects

	// State
	GameState _gameState;
	int _freedCount;                  ///< Number of zoombinis successfully freed

	// Direction labels for the graph (encoded in static data at byte offset 44)
	static const char kGraph1DirLabels[8][4];   ///< 8-node cube
	static const char kGraph2DirLabels[16][4];  ///< 16-node double cube

	// Private methods —
	void loadGraph();                 ///< Load static graph data from tables
	void placeZoombinis();            ///< Place zoombinis on graph (Init loop)
	void placeFleens();               ///< Place fleens on graph (diff 2+)
	void placeBallStart();            ///< Find ball start position
	void startBallMove(int targetNode);
	void finishBallMove();
	int findNodeByDirValues3(int a, int b, int c) const;
	int findNodeByDirValues4(int a, int b, int c, int d) const;
	void freeZoombini(int nodeIdx);
	int countFreeZoombinis() const;
	void loadResources();

	int _musicId;  // BGM: sounds/music/03-BB01.wav
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_AQUACUBE_H
