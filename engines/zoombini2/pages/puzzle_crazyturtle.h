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

#ifndef ZOOMBINI2_PAGES_PUZZLE_CRAZYTURTLE_H
#define ZOOMBINI2_PAGES_PUZZLE_CRAZYTURTLE_H

#include "common/rect.h"

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * Turtle Hurdle (Route1-1)
 *
 * Place the Zoombinis on the baby turtles in the required order.
 */
class PuzzleCrazyTurtle : public PuzzleBase {
public:
	/** Construct Turtle Hurdle for @p vm. */
	PuzzleCrazyTurtle(Zoombini2Engine *vm);
	/** Release turtle, bridge, and trait resources. */
	~PuzzleCrazyTurtle() override;

	/** Load the dock and generate the level-selected ordering rules. */
	void init() override;
	/** Advance the common puzzle and active turtle animations. */
	void onUpdate() override;
	/** Place the next Zoombini on the selected turtle. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Report that this puzzle does not use the common Go button. */
	bool canUseGoButton() const override { return false; }

private:
	/** Restore the dock background. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw dock condition, rule hints, turtles, and mother. */
	void onRenderScene(ManagedSurface32 *screen) override;
	/** Draw the party in common depth order. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Turtle resource type and draw position. */
	struct TurtlePlacement {
		/** Index of the turtle animation and fixed visual. */
		int type;
		/** Screen position of the fixed visual. */
		Common::Point32 pos;
	};

	/** Number of visible Zoombini features. */
	static const int kFeatureCount = 4;
	/** Number of values for each visible feature. */
	static const int kFeatureValueCount = 5;
	/** Number of simultaneous ordering-rule groups. */
	static const int kRuleGroupCount = 2;
	/** Number of positions in each ordering rule. */
	static const int kRuleSlotCount = 5;
	/** Number of turtles spanning the dock. */
	static const int kTurtleCount = 16;
	/** Maximum party size used by this puzzle. */
	static const int kZoombiniCount = 16;

	/** Turtle types and screen positions. */
	static const TurtlePlacement kTurtlePlacements[kTurtleCount];
	/** Screen positions used for Zoombinis standing on turtles. */
	static const Common::Point32 kZoombiniPos[kZoombiniCount];
	/** Draw positions for primary-rule feature icons. */
	static const Common::Point32 kPrimaryIconPos[kRuleSlotCount];
	/** Draw positions for secondary-rule feature icons. */
	static const Common::Point32 kSecondaryIconPos[kRuleSlotCount];

	/** Load an animation from @p path and report whether it succeeded. */
	bool loadAnimationResource(Animation *&resource, const Common::Path &path);
	/** Load an RLE sprite from @p path and report whether it succeeded. */
	bool loadRleResource(RleBlock *&resource, const Common::Path &path);

	/** Load all turtle, dock, mother, and trait resources. */
	void loadResources();
	/** Select active features and generate their ordering rules. */
	void generateRules();
	/** Generate the value order for rule @p group. */
	void generateFeatureOrder(int group);
	/** Enable @p count random entries in @p slots. */
	void activateRandomRuleSlots(bool *slots, int count);
	/** Position the puzzle roster before interaction begins. */
	void placeZoombinis();
	/** Draw the intact or damaged dock for the remaining mistake count. */
	void drawBridgeState(ManagedSurface32 *screen) const;
	/** Draw the active feature-order hints. */
	void drawRuleHints(ManagedSurface32 *screen) const;
	/** Draw each turtle in its current animation state. */
	void drawTurtles(ManagedSurface32 *screen) const;
	/** Draw the mother turtle and feedback effects. */
	void drawMother(ManagedSurface32 *screen) const;

	/** Level in the range one through four. */
	int _level = 1;
	/** Feature index used by the primary ordering rule. */
	int _primaryFeature = 0;
	/** Feature index used by the secondary ordering rule. */
	int _secondaryFeature = 1;
	/** Feature-value ordering for each rule group. */
	int _ruleValues[kRuleGroupCount][kRuleSlotCount] = {};
	/** Active hint positions in the primary rule. */
	bool _primaryRuleActive[kRuleSlotCount] = {};
	/** Active hint positions in the secondary rule. */
	bool _secondaryRuleActive[kRuleSlotCount] = {};
	/** Incorrect placements still allowed before the dock collapses. */
	int _remainingMistakes = 0;
	/** Initial incorrect-placement allowance for the selected level. */
	int _initialMistakes = 0;

	/** Idle turtle animations indexed by turtle type. */
	Animation *_turtleIdleAnimations[kFeatureCount] = {};
	/** Feedback turtle animations indexed by turtle type. */
	Animation *_turtleSpinAnimations[kFeatureCount] = {};
	/** Fixed turtle visuals indexed by turtle type. */
	RleBlock *_turtleFixedImages[kFeatureCount] = {};
	/** Mother turtle animation. */
	Animation *_motherAnimation = nullptr;
	/** Mother turtle speech animation. */
	Animation *_motherSpeechAnimation = nullptr;
	/** Dock-damage smoke animation. */
	Animation *_smokeAnimation = nullptr;
	/** Mother turtle's starting visual. */
	RleBlock *_motherStartImage = nullptr;
	/** Mother turtle's ending visual. */
	RleBlock *_motherEndImage = nullptr;
	/** Intact dock visual. */
	RleBlock *_bridgeImage = nullptr;
	/** Collapsed dock visual. */
	RleBlock *_collapsedBridgeImage = nullptr;
	/** Individual dock-beam visual. */
	RleBlock *_beamImage = nullptr;
	/** Feature-value hint visuals. */
	RleBlock *_traitImages[kFeatureCount][kFeatureValueCount] = {};

	/** Music handle used while Turtle Hurdle is active. */
	int _musicId = -1;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_CRAZYTURTLE_H
