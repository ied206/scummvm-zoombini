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
#include "zoombini2/state.h"

namespace Zoombini2 {

class RleBlock;
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
	/** Release lane, trait, board, and scenery resources. */
	~PuzzleSnowboard() override;

	/** Generate the decision tree and assign the current puzzle roster to lanes. */
	void init() override;
	/** Advance the current snowboarder and completion state. */
	void onUpdate() override;
	/** Draw the board, scenery, decision hints, and current snowboarder. */
	void onRenderScene(ManagedSurface32 *screen) override;
	/** Draw lane occupants and the current sliding actor. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Restore the page background. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Ignore clicks because lane traversal advances automatically. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;

private:
	/** One generated trait test in the lane-classification tree. */
	struct TreeNode {
		/** Trait index tested by this node. */
		ZmbTrait::TraitIndex traitIndex;
		/** Primary value that selects the matching branch. */
		byte matchVal1;
		/** Optional second matching value used at higher levels. */
		byte matchVal2;
	};

	/** Generate the decision tree for the current level. */
	void generateTree();

	/** Classify @p z and return its destination lane. */
	int classifyZoombini(const ZoombiniState *z) const;

	/** Classify every puzzle-roster entry and store its lane. */
	void assignZoombinisToLanes();

	/** Load the lane, board, trait, and scenery resources. */
	void loadLaneGraphics();

	/** Draw a trait-value hint at @p pos. */
	void drawTraitIcon(ManagedSurface32 *screen, ZmbTrait::TraitIndex traitIndex, int value, const Common::Point32 &pos);

	/** Generated internal decision-tree nodes. */
	Common::Array<TreeNode> _tree;
	/** Number of internal nodes, with one more destination leaf than this value. */
	int _treeDepth;
	/** Number of destination lanes. */
	int _numLanes;

	/** Destination lane indexed by puzzle-roster entry. */
	Common::Array<int> _laneAssignments;

	/** Trait-value hint visuals. */
	RleBlock *_traitIcons[ZmbTrait::kTraitCount][ZmbTrait::kTraitValueCount];
	/** Static board visual. */
	BitBlock *_boardBitmap;
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
