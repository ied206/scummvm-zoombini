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

#ifndef ZOOMBINI2_PAGES_SNOWBOARD_H
#define ZOOMBINI2_PAGES_SNOWBOARD_H

#include "zoombini2/pages/puzzle.h"

namespace Zoombini2 {

class RleBlock;
class Animation;

/**
 * SnowboardPuzzle — Binary decision tree classifier puzzle.
 *
 * Original: Snowboard__Init_42D620, object size 0x1E0, vtable 0x480400.
 *
 * Mechanics: Routes zoombinis to snowboard lanes via a binary decision tree.
 * Each tree node checks zoombini feature[node.featureIdx] against node.matchVal.
 * Match → left child (2i+1), No match → right child (2i+2).
 * Leaf index determines the snowboard lane assignment.
 *
 * Difficulty scaling:
 *   - Diff 1-2: Single match value per node
 *   - Diff 3: Two match values per node (accept either)
 */
class SnowboardPuzzle : public PuzzlePage {
public:
	SnowboardPuzzle(Zoombini2Engine *engine);
	~SnowboardPuzzle() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	/**
	 * Decision tree node structure (5 bytes in original).
	 * Original: tree stored at this+264, stride 5 bytes.
	 */
	struct TreeNode {
		byte featureIdx;   // Which feature to check (0-3: hair, eyes, nose, feet)
		byte matchVal1;    // Value to match for left branch
		byte matchVal2;    // Second match value (difficulty 3 only)
	};

	// Generate the decision tree for current difficulty
	void generateTree();

	// Traverse tree for a zoombini, returns lane index (0 to numLanes-1)
	int classifyZoombini(const Zoombini *z) const;

	// Assign all zoombinis to lanes
	void assignZoombinisToLanes();

	// Load lane graphics
	void loadLaneGraphics();

	// Draw trail icon for feature match
	void drawTraitIcon(Graphics::ManagedSurface *screen, int feature, int value, int x, int y);

	// Decision tree data
	Common::Array<TreeNode> _tree;
	int _treeDepth;           // Number of internal nodes (leaf count = depth + 1)
	int _numLanes;            // Number of destination lanes

	// Lane assignments: which zoombinis go to which lane
	Common::Array<int> _laneAssignments;  // Index by zoombini, value is lane

	// Graphics
	RleBlock *_traitIcons[4][5];   // Feature icons [feature 0-3][value 0-4]
	BitBlock *_boardGfx;            // Static board sprite (BOARD01.RB)
	Animation *_boardAnim;          // Animated board (BOARD.AN)
	Animation *_engineAnim;         // Engine/lift animation (ENGINE.AN)
	Animation *_decorAnims[5];      // Background/scenery animations (N1So-1,3,4,5,6)

	// Puzzle state
	int _currentZoombini;     // Which zoombini is currently sliding
	int _animFrame;           // Current animation frame
	uint32 _lastFrameTime;    // For animation timing

	enum State {
		kStateInit,
		kStateSliding,
		kStateDone
	};
	State _state;

	int _musicId;  // BGM: sounds/music/01-BS06.wav
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SNOWBOARD_H
