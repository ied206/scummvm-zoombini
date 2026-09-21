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

#ifndef ZOOMBINI2_PAGES_PUZZLE_MAGICWALL_H
#define ZOOMBINI2_PAGES_PUZZLE_MAGICWALL_H

#include "common/array.h"
#include "common/rect.h"
#include "zoombini2/pages/puzzle_base.h"

namespace Common {
class SeekableReadStream;
}

namespace Zoombini2 {

class Animation;
class AnimationRunner;
struct PathObject;

class Random;

/** Resource-defined beetle permutations and weighted puzzle generation. */
class MagicWallMaze {
public:
	struct Edge {
		int from = 0;
		int to = 0;
		bool swap = false;
	};
	typedef Common::Array<Edge> Rule;
	typedef Common::Array<Rule> Variant;
	struct Layout {
		int difficulty = 0;
		int weight = 0;
		Common::Array<Common::Point32> points;
		Common::Array<Variant> groups[3];
	};
	/** Read bounded layout records, retaining complete records before damaged trailing data. */
	bool load(Common::SeekableReadStream &stream);
	bool generate(int difficulty, Random &random);
	void apply(int ruleIndex);
	bool matched(int color) const;
	bool gateMatched(int gate) const;
	int layoutIndex() const { return _layoutIndex; }
	const Layout &layout() const { return _layouts[_layoutIndex]; }
	const Common::Array<Layout> &layouts() const { return _layouts; }
	const Variant &rules() const { return _rules; }
	const Common::Array<int> &positions() const { return _positions; }
	const Common::Array<int> &initialPositions() const { return _initialPositions; }
	const Common::Array<int> &generationSequence() const { return _generationSequence; }
	bool debugSolution(Common::Array<int> &sequence) const;

private:
	static bool readCount(Common::SeekableReadStream &stream, int &count, int maximum);
	static bool readLayout(Common::SeekableReadStream &stream, Layout &layout);
	/** Apply a whole rule once per beetle, or sequentially when constructing a scramble. */
	static void applyRule(const Rule &rule, Common::Array<int> &positions, bool simultaneous);
	static bool isIdentity(const Common::Array<int> &positions);
	int solve(const Common::Array<int> &positions, int depth) const;
	bool debugSolve(const Common::Array<int> &positions, int depth, int &budget, Common::Array<int> &sequence) const;
	void scramble(Random &random);
	Common::Array<Layout> _layouts;
	int _layoutIndex = -1;
	Variant _rules;
	Common::Array<int> _positions;
	/** Sequential construction operations for the selected initial board, not a player solution. */
	Common::Array<int> _generationSequence;
	Common::Array<int> _initialPositions;
};

/** Beetle Bug Alley: stone tablets permute beetles to unlock four doors. */
class PuzzleMagicWall : public PuzzleBase {
public:
	explicit PuzzleMagicWall(Zoombini2Engine *vm);
	~PuzzleMagicWall() override;
	void init() override;
	void onUpdate() override;
	void onRenderBackground(ManagedSurface32 *screen) override;
	void onRenderContent(ManagedSurface32 *screen) override;
	void onRenderActors(ManagedSurface32 *screen) override;
	void onActorsRendered() override;
	void onRenderForeground(ManagedSurface32 *screen) override;
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	bool canUseGoButton() const override;
	bool onGoButtonPressed() override;
	bool blocksSidebarInteraction() const override;
	Common::String debugGetAnswer() const override;
	Common::String debugGetChanceDetails() const override;

private:
	static constexpr const char *kLayoutPath = "bmp/magic_wall/default.ztl";
	static constexpr const char *kMusicPath = "#sounds/music/06-BB01.wav";
	static constexpr const char *kDotFormat = "bmp/magic_wall/DOT-%s";
	static constexpr const char *kBugFormat = "bmp/magic_wall/bug_c_%s";
	static constexpr const char *kDirectionFormat = "bmp/magic_wall/bug_%d";
	static constexpr const char *kLightFormat = "bmp/magic_wall/mini-light-%s";
	static constexpr const char *kTabletPath = "bmp/magic_wall/mini-map";
	static constexpr const char *kTabletDotPath = "bmp/magic_wall/mini-map-dot";
	static constexpr const char *kLeverPath = "bmp/magic_wall/le_vier";
	static constexpr const char *kLeverGlowPath = "bmp/magic_wall/le_vier_luisant";
	static constexpr const char *kGateFormat = "bmp/magic_wall/porte-%c";
	static constexpr const char *kGateBackFormat = "bmp/magic_wall/porte-%csingle";
	static constexpr const char *kExitFormat = "bmp/magic_wall/PAT/EXIT%d.PAT";
	static constexpr const char *kMoveFormat = "bmp/magic_wall/PAT/BOUGE%d.PAT";
	static constexpr const char *kBugLoopPath = "sounds/fx/06-BB02.wav";
	static constexpr const char *kGateSoundPath = "sounds/fx/06-BS03.wav";
	static constexpr const char *kLeverSoundPath = "sounds/fx/06-BS05.wav";
	static constexpr const char *kRetreatSpeechPath = "sounds/DW-Cave.wav";
	static constexpr const char *kPerfectSpeechFormat = "sounds/wld11.%d.wav";
	static constexpr const char *kColors[11] = {
		"blue",
		"green",
		"navy",
		"orange",
		"purple",
		"red",
		"rose",
		"turquoise",
		"violet",
		"yellow",
		"pierre",
	};
	static constexpr Common::Point32 kTabletPositions[5] = {
		{350, 400},
		{415, 417},
		{482, 425},
		{553, 440},
		{616, 455},
	};
	static constexpr Common::Point32 kGatePositions[4] = {
		{76, 290},
		{129, 288},
		{180, 288},
		{228, 285},
	};
	static constexpr Common::Point32 kRosterPositions[8] = {
		{31, 442},
		{77, 458},
		{124, 465},
		{171, 473},
		{12, 478},
		{60, 499},
		{109, 515},
		{158, 524},
	};
	static constexpr Common::Point32 kLightPositions[10] = {
		{82, 346},
		{130, 335},
		{184, 336},
		{243, 346},
		{103, 347},
		{151, 336},
		{205, 336},
		{265, 346},
		{93, 326},
		{142, 315},
	};

	struct Beetle {
		Common::Point32 position;
		PathObject *path = nullptr;
		int direction = 0;
	};

	void loadResources();
	void newPuzzle();
	void startRule(int index);
	void submit();
	void startRunnerPath(int index, int gate, bool exit);
	bool gatesActive() const;
	bool runnersMoving() const;
	bool beetlesMoving() const;
	int tabletAt(const Common::Point &pos) const;
	Common::Point32 dotPosition(int index) const;
	Common::Point32 tabletPoint(int tablet, int point, int inset) const;
	void drawTablets(ManagedSurface32 *screen);
	void drawConnections(ManagedSurface32 *screen, int index);
	void drawBeetles(ManagedSurface32 *screen);
	void drawRipple(ManagedSurface32 *screen, Common::Point32 from, const Common::Point32 &to, int phase);
	static int directionIndex(const Common::Point32 &from, const Common::Point32 &to);
	static bool inside(const Common::Point &pos, int x, int y, int width, int height);
	static void gateDone(void *context, AnimationRunner *runner);

	MagicWallMaze _maze;
	Beetle _beetles[10];
	Animation *_gateAnimations[4] = {};
	Animation *_leverAnimation = nullptr;
	AnimationRunner *_gateRunners[4] = {};
	AnimationRunner *_leverRunner = nullptr;

	// A runner is retired only after its exit path reaches the upper side of the gate.
	bool _exited[8] = {};
	bool _ready = false;
	bool _nextPuzzlePending = false;
	bool _canDepart = false;
	bool _buttonArmed = false;
	bool _speechPending = false;
	// The first accepted submission starts a new board; the second finishes the puzzle.
	int _phase = 0;
	int _hoveredTablet = -1;
	int _movingRule = -1;
	int _ripplePhase = 0;
	Common::Point _pointer;
	int _bugSound = -1;
	int _gateSound = -1;
	int _leverSound = -1;
	int _speechSound = -1;
};

} // End of namespace Zoombini2

#endif
