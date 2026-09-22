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
		/** Source and destination point indices in one tablet rule. */
		int from = 0;
		int to = 0;
		/** Whether applying this edge exchanges the beetles at both endpoints. */
		bool swap = false;
	};
	/** One tablet operation and its selectable variants. */
	typedef Common::Array<Edge> Rule;
	typedef Common::Array<Rule> Variant;
	/** One resource-defined board layout and the rules offered on its tablets. */
	struct Layout {
		/** Difficulty branch and weighted random-selection value. */
		int difficulty = 0;
		int weight = 0;
		/** Beetle point locations and the three rule groups read from the layout resource. */
		Common::Array<Common::Point32> points;
		Common::Array<Variant> groups[3];
	};
	/** Read bounded layout records, retaining complete records before damaged trailing data. */
	bool load(Common::SeekableReadStream &stream);
	/** Choose a weighted layout for @p difficulty and construct its initial scrambled board. */
	bool generate(int difficulty, Random &random);
	/** Apply the player's selected tablet rule to the current beetle positions. */
	void apply(int ruleIndex);
	/** Return whether the beetle of @p color occupies its matching endpoint. */
	bool matched(int color) const;
	/** Return whether every beetle needed by @p gate is at its matching endpoint. */
	bool gateMatched(int gate) const;
	/** Return the selected resource layout and its available rule groups. */
	int layoutIndex() const { return _layoutIndex; }
	const Layout &layout() const { return _layouts[_layoutIndex]; }
	const Common::Array<Layout> &layouts() const { return _layouts; }
	const Variant &rules() const { return _rules; }
	const Common::Array<int> &positions() const { return _positions; }
	const Common::Array<int> &initialPositions() const { return _initialPositions; }
	const Common::Array<int> &generationSequence() const { return _generationSequence; }
	/** Find a bounded player-rule sequence to solve the current board for the console. */
	bool debugSolution(Common::Array<int> &sequence) const;

private:
	/** Read a count only when it fits @p maximum and the resource's trusted bounds. */
	static bool readCount(Common::SeekableReadStream &stream, int &count, int maximum);
	/** Decode one structurally complete layout record from the resource stream. */
	static bool readLayout(Common::SeekableReadStream &stream, Layout &layout);
	/** Apply a whole rule once per beetle, or sequentially when constructing a scramble. */
	static void applyRule(const Rule &rule, Common::Array<int> &positions, bool simultaneous);
	/** Return whether @p positions has every beetle at its matching point. */
	static bool isIdentity(const Common::Array<int> &positions);
	/** Search the internal construction space for a non-identity scramble. */
	int solve(const Common::Array<int> &positions, int depth) const;
	/** Search player-rule moves within @p budget and append a solution to @p sequence. */
	bool debugSolve(const Common::Array<int> &positions, int depth, int &budget, Common::Array<int> &sequence) const;
	/** Build an initial board by applying selected rules sequentially. */
	void scramble(Random &random);
	/** All decoded layouts, selected layout index, active rules, and current beetle positions. */
	Common::Array<Layout> _layouts;
	int _layoutIndex = -1;
	Variant _rules;
	Common::Array<int> _positions;
	/** Sequential construction operations for the selected initial board, not a player solution. */
	Common::Array<int> _generationSequence;
	/** Current board before player input, retained for debug output and validation. */
	Common::Array<int> _initialPositions;
};

/** Beetle Bug Alley: stone tablets permute beetles to unlock four doors. */
class PuzzleMagicWall : public PuzzleBase {
public:
	/** Construct Beetle Bug Alley for @p vm. */
	explicit PuzzleMagicWall(Zoombini2Engine *vm);
	/** Release transient beetle paths and animation runners. */
	~PuzzleMagicWall() override;
	/** Load resources, generate a beetle board, and arrange the puzzle roster. */
	void init() override;
	/** Advance tablet, gate, beetle, runner, and speech transitions. */
	void onUpdate() override;
	/** Restore the puzzle's primary background layer. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw tablets, connections, beetles, lights, and gates. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Draw runners travelling through opened gates. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Advance runner animation frames after the actor pass. */
	void onActorsRendered() override;
	/** Draw moving-tablet ripple effects over the board. */
	void onRenderForeground(ManagedSurface32 *screen) override;
	/** Arm tablet interaction at @p pos. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Apply an armed tablet rule when the button is released at @p pos. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Update the tablet hover state at @p pos. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Return whether a completed gate transfer allows departure. */
	bool canUseGoButton() const override;
	/** Begin retreat speech or return through the map transition. */
	bool onGoButtonPressed() override;
	/** Prevent sidebar actions during active tablet, gate, or runner movement. */
	bool blocksSidebarInteraction() const override;
	/** Format the current board and a bounded solution for the puzzle console. */
	Common::String debugGetAnswer() const override;
	/** Report the active layout and generated scramble history for the console. */
	Common::String debugGetChanceDetails() const override;

private:
	/** Resource-defined tablet layout and the page's background music. */
	static constexpr const char *kLayoutPath = "bmp/magic_wall/default.ztl";
	static constexpr const char *kMusicPath = "#sounds/music/06-BB01.wav";
	/** Formats for beetle endpoints, colored beetles, direction markers, lights, and gates. */
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
	/** Runner exit/movement paths, page effects, and departure speech. */
	static constexpr const char *kExitFormat = "bmp/magic_wall/PAT/EXIT%d.PAT";
	static constexpr const char *kMoveFormat = "bmp/magic_wall/PAT/BOUGE%d.PAT";
	static constexpr const char *kBugLoopPath = "sounds/fx/06-BB02.wav";
	static constexpr const char *kGateSoundPath = "sounds/fx/06-BS03.wav";
	static constexpr const char *kLeverSoundPath = "sounds/fx/06-BS05.wav";
	static constexpr const char *kRetreatSpeechPath = "sounds/DW-Cave.wav";
	static constexpr const char *kPerfectSpeechFormat = "sounds/wld11.%d.wav";
	/** Resource color suffixes for the ten beetles plus the stone marker. */
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
	/** Screen locations for tablets, gates, roster runners, and gate lights. */
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
		/** Current point-derived screen position and active movement path. */
		Common::Point32 position;
		PathObject *path = nullptr;
		/** Facing frame selected from the prior and next tablet points. */
		int direction = 0;
	};

	/** Load static art, gate animations, paths, music, and effects. */
	void loadResources();
	/** Generate the next board after a successful first submission. */
	void newPuzzle();
	/** Apply tablet rule @p index and animate every affected beetle. */
	void startRule(int index);
	/** Open matched gates and move successful runners through them. */
	void submit();
	/** Start runner @p index on a gate path, optionally using its exit path. */
	void startRunnerPath(int index, int gate, bool exit);
	/** Return whether all gate animations are open and matched. */
	bool gatesActive() const;
	/** Return whether any puzzle Zoombini is following a gate path. */
	bool runnersMoving() const;
	/** Return whether any beetle is following a tablet-generated path. */
	bool beetlesMoving() const;
	/** Return the tablet index under @p pos, or -1. */
	int tabletAt(const Common::Point &pos) const;
	/** Convert a layout point or tablet-local point to page-space coordinates. */
	Common::Point32 dotPosition(int index) const;
	Common::Point32 tabletPoint(int tablet, int point, int inset) const;
	/** Draw tablet art, rule connections, moving beetles, and their ripple. */
	void drawTablets(ManagedSurface32 *screen);
	void drawConnections(ManagedSurface32 *screen, int index);
	void drawBeetles(ManagedSurface32 *screen);
	void drawRipple(ManagedSurface32 *screen, Common::Point32 from, const Common::Point32 &to, int phase);
	/** Select a beetle direction frame from a point-to-point movement vector. */
	static int directionIndex(const Common::Point32 &from, const Common::Point32 &to);
	/** Return whether @p pos lies in a page-space rectangle. */
	static bool inside(const Common::Point &pos, int x, int y, int width, int height);
	/** Clear the active gate runner after its opening animation completes. */
	static void gateDone(void *context, AnimationRunner *runner);

	/** Resource-defined board, displayed beetles, and gate/lever animation state. */
	MagicWallMaze _maze;
	Beetle _beetles[10];
	Animation *_gateAnimations[4] = {};
	Animation *_leverAnimation = nullptr;
	AnimationRunner *_gateRunners[4] = {};
	AnimationRunner *_leverRunner = nullptr;

	/** A runner is retired only after its exit path reaches the upper side of the gate. */
	bool _exited[8] = {};
	/** Accepted submissions move from the first board to the second, then finish the puzzle. */
	enum class Phase {
		kFirstBoard,
		kSecondBoard,
		kFinished
	};
	/** Board readiness, deferred refresh, Go availability, and mouse-button state. */
	bool _ready = false;
	bool _nextPuzzlePending = false;
	bool _canDepart = false;
	bool _buttonArmed = false;
	bool _speechPending = false;
	Phase _phase = Phase::kFirstBoard;
	/** Hovered tablet, currently animating rule, and ripple animation frame. */
	int _hoveredTablet = -1;
	int _movingRule = -1;
	int _ripplePhase = 0;
	/** Latest pointer location used for tablet hover feedback. */
	Common::Point _pointer;
	/** Looping beetle, gate, and lever mixer handles. */
	int _bugSound = -1;
	int _gateSound = -1;
	int _leverSound = -1;
};

} // End of namespace Zoombini2

#endif
