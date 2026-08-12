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
	CrazyTurtlePuzzle(Zoombini2Engine *engine);
	~CrazyTurtlePuzzle() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;
	bool canUseGoButton() const override { return false; }

private:
	struct TurtlePlacement {
		int type;
		int x;
		int y;
	};

	static const int kFeatureCount = 4;
	static const int kFeatureValueCount = 5;
	static const int kRuleGroupCount = 2;
	static const int kRuleSlotCount = 5;
	static const int kTurtleCount = 16;
	static const int kZoombiniCount = 16;

	static const TurtlePlacement kTurtlePlacements[kTurtleCount];
	static const Common::Point kZoombiniPositions[kZoombiniCount];
	static const Common::Point kPrimaryIconPositions[kRuleSlotCount];
	static const Common::Point kSecondaryIconPositions[kRuleSlotCount];

	static bool loadAnimationResource(Animation *&resource, const Common::Path &path);
	static bool loadRleResource(RleBlock *&resource, const Common::Path &path);

	void loadResources();
	void generateRules();
	void generateFeatureOrder(int group);
	void activateRandomRuleSlots(bool *slots, int count);
	void placeZoombinis();
	void drawBridgeState(Graphics::ManagedSurface *screen) const;
	void drawRuleHints(Graphics::ManagedSurface *screen) const;
	void drawTurtles(Graphics::ManagedSurface *screen) const;
	void drawMother(Graphics::ManagedSurface *screen) const;
	void drawZoombinis(Graphics::ManagedSurface *screen) const;
	void drawZoombini(Graphics::ManagedSurface *screen, const Zoombini &zoombini) const;

	int _difficulty;
	int _primaryFeature;
	int _secondaryFeature;
	int _ruleValues[kRuleGroupCount][kRuleSlotCount];
	bool _primaryRuleActive[kRuleSlotCount];
	bool _secondaryRuleActive[kRuleSlotCount];
	int _remainingMistakes;
	int _initialMistakes;

	Animation *_turtleIdleAnimations[kFeatureCount];
	Animation *_turtleSpinAnimations[kFeatureCount];
	RleBlock *_turtleFixedGraphics[kFeatureCount];
	Animation *_motherAnimation;
	Animation *_motherSpeechAnimation;
	Animation *_smokeAnimation;
	RleBlock *_motherStartGraphic;
	RleBlock *_motherEndGraphic;
	RleBlock *_bridgeGraphic;
	RleBlock *_collapsedBridgeGraphic;
	RleBlock *_beamGraphic;
	RleBlock *_traitGraphics[kFeatureCount][kFeatureValueCount];

	int _musicId;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_CRAZYTURTLE_H
