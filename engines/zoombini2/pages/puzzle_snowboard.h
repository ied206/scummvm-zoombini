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

namespace Zoombini2 {

class RleBlock;
class Animation;

/**
 * Snowboard Gulch routes Zoombinis to lanes through a generated decision tree.
 *
 * Each internal node tests a visible feature. Matching values take one branch,
 * nonmatching values take the other, and the reached leaf selects a lane.
 */
class SnowboardPuzzle : public PuzzlePage {
public:
	/** Construct Snowboard Gulch for @p engine. */
	SnowboardPuzzle(Zoombini2Engine *engine);
	/** Release lane, trait, board, and scenery resources. */
	~SnowboardPuzzle() override;

	/** Generate the decision tree and assign the current puzzle roster to lanes. */
	void init() override;
	/** Advance the current snowboarder and completion state. */
	void update() override;
	/** Draw the board, scenery, decision hints, and current snowboarder. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Ignore clicks because lane traversal advances automatically. */
	void handleClick(const Common::Point &pos) override;

private:
	/** One generated feature test in the lane-classification tree. */
	struct TreeNode {
		/** Feature index tested by this node. */
		byte featureIdx;
		/** Primary value that selects the matching branch. */
		byte matchVal1;
		/** Optional second matching value used at higher difficulty. */
		byte matchVal2;
	};

	/** Generate the decision tree for the current difficulty. */
	void generateTree();

	/** Classify @p z and return its destination lane. */
	int classifyZoombini(const ZoombiniState *z) const;

	/** Classify every puzzle-roster entry and store its lane. */
	void assignZoombinisToLanes();

	/** Load the lane, board, trait, and scenery resources. */
	void loadLaneGraphics();

	/** Draw a feature-value hint at @p position. */
	void drawTraitIcon(Graphics::ManagedSurface *screen, int feature, int value, const Common::Point32 &position);

	/** Generated internal decision-tree nodes. */
	Common::Array<TreeNode> _tree;
	/** Number of internal nodes, with one more destination leaf than this value. */
	int _treeDepth;
	/** Number of destination lanes. */
	int _numLanes;

	/** Destination lane indexed by puzzle-roster entry. */
	Common::Array<int> _laneAssignments;

	/** Feature-value hint visuals. */
	RleBlock *_traitIcons[4][5];
	/** Static board visual. */
	BitBlock *_boardGfx;
	/** Animated board sequence. */
	Animation *_boardAnim;
	/** Lift engine animation. */
	Animation *_engineAnim;
	/** Background scenery animations. */
	Animation *_decorAnims[5];

	/** Puzzle-roster index currently sliding. */
	int _currentZoombini;
	/** Current frame of the active board animation. */
	int _animFrame;
	/** Time at which the animation last advanced. */
	uint32 _lastFrameTime;

	/** Runtime phase of the automatic snowboard sequence. */
	enum State {
		/** Complete initial assignment before starting the sequence. */
		kStateInit,
		/** Animate Zoombinis through their assigned lanes. */
		kStateSliding,
		/** Stop after every assigned Zoombini has completed. */
		kStateDone
	};
	/** Current automatic sequence phase. */
	State _state;

	/** Music handle used while Snowboard Gulch is active. */
	int _musicId;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_SNOWBOARD_H
