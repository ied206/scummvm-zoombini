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
 * Turtle Hurdle: place Zoombinis in the order required by the turtles.
 * Incorrect placements damage the dock and strand any remaining Zoombinis.
 * The internal CrazyTurtle name identifies the bmp/crazy_turtle resources.
 */
class CrazyTurtlePuzzle : public PuzzlePage {
public:
	/** Construct Turtle Hurdle for @p engine. */
	CrazyTurtlePuzzle(Zoombini2Engine *engine);
	/** Release turtle, bridge, and trait resources. */
	~CrazyTurtlePuzzle() override;

	/** Load the dock and generate the difficulty-selected ordering rules. */
	void init() override;
	/** Advance the common puzzle and active turtle animations. */
	void update() override;
	/** Draw the dock condition, rule hints, turtles, mother, and Zoombinis. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Place the next Zoombini on the selected turtle. */
	void handleClick(const Common::Point &pos) override;
	/** Report that this puzzle does not use the common Go button. */
	bool canUseGoButton() const override { return false; }

private:
	/** Turtle resource type and draw position. */
	struct TurtlePlacement {
		/** Index of the turtle animation and fixed visual. */
		int type;
		/** Horizontal draw position. */
		int x;
		/** Vertical draw position. */
		int y;
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
	static const Common::Point kZoombiniPositions[kZoombiniCount];
	/** Draw positions for primary-rule feature icons. */
	static const Common::Point kPrimaryIconPositions[kRuleSlotCount];
	/** Draw positions for secondary-rule feature icons. */
	static const Common::Point kSecondaryIconPositions[kRuleSlotCount];

	/** Load an animation from @p path and report whether it succeeded. */
	static bool loadAnimationResource(Animation *&resource, const Common::Path &path);
	/** Load an RLE sprite from @p path and report whether it succeeded. */
	static bool loadRleResource(RleBlock *&resource, const Common::Path &path);

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
	void drawBridgeState(Graphics::ManagedSurface *screen) const;
	/** Draw the active feature-order hints. */
	void drawRuleHints(Graphics::ManagedSurface *screen) const;
	/** Draw each turtle in its current animation state. */
	void drawTurtles(Graphics::ManagedSurface *screen) const;
	/** Draw the mother turtle and feedback effects. */
	void drawMother(Graphics::ManagedSurface *screen) const;
	/** Draw all currently placed Zoombinis. */
	void drawZoombinis(Graphics::ManagedSurface *screen) const;
	/** Draw @p zoombini at its runtime position. */
	void drawZoombini(Graphics::ManagedSurface *screen, const ZoombiniState &zoombini) const;

	/** Difficulty level in the range one through four. */
	int _difficulty;
	/** Feature index used by the primary ordering rule. */
	int _primaryFeature;
	/** Feature index used by the secondary ordering rule. */
	int _secondaryFeature;
	/** Feature-value ordering for each rule group. */
	int _ruleValues[kRuleGroupCount][kRuleSlotCount];
	/** Active hint positions in the primary rule. */
	bool _primaryRuleActive[kRuleSlotCount];
	/** Active hint positions in the secondary rule. */
	bool _secondaryRuleActive[kRuleSlotCount];
	/** Incorrect placements still allowed before the dock collapses. */
	int _remainingMistakes;
	/** Initial incorrect-placement allowance for the selected difficulty. */
	int _initialMistakes;

	/** Idle turtle animations indexed by turtle type. */
	Animation *_turtleIdleAnimations[kFeatureCount];
	/** Feedback turtle animations indexed by turtle type. */
	Animation *_turtleSpinAnimations[kFeatureCount];
	/** Fixed turtle visuals indexed by turtle type. */
	RleBlock *_turtleFixedGraphics[kFeatureCount];
	/** Mother turtle animation. */
	Animation *_motherAnimation;
	/** Mother turtle speech animation. */
	Animation *_motherSpeechAnimation;
	/** Dock-damage smoke animation. */
	Animation *_smokeAnimation;
	/** Mother turtle's starting visual. */
	RleBlock *_motherStartGraphic;
	/** Mother turtle's ending visual. */
	RleBlock *_motherEndGraphic;
	/** Intact dock visual. */
	RleBlock *_bridgeGraphic;
	/** Collapsed dock visual. */
	RleBlock *_collapsedBridgeGraphic;
	/** Individual dock-beam visual. */
	RleBlock *_beamGraphic;
	/** Feature-value hint visuals. */
	RleBlock *_traitGraphics[kFeatureCount][kFeatureValueCount];

	/** Music handle used while Turtle Hurdle is active. */
	int _musicId;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_CRAZYTURTLE_H
