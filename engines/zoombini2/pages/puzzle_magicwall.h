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
#include "common/hashmap.h"
#include "common/rect.h"
#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_base.h"

namespace Common {
class SeekableReadStream;
}

namespace Zoombini2 {

class Animation;
class AnimationRunner;
struct PathObject;
struct RGBColor;

class Random;

/** Resource-defined beetle permutations and weighted puzzle generation. */
class MagicWallBoard {
public:
	/** Original color ordinals shared by beetles, target vertices and gate lamps. */
	enum VertexColor : uint {
		kVertexColor00_Blue = 0,
		kVertexColor01_Green = 1,
		kVertexColor02_Navy = 2,
		kVertexColor03_Orange = 3,
		kVertexColor04_Purple = 4,
		kVertexColor05_Red = 5,
		kVertexColor06_Rose = 6,
		kVertexColor07_Turquoise = 7,
		kVertexColor08_Violet = 8,
		kVertexColor09_Yellow = 9,
		/** Neutral lamp for an unmatched vertex; no beetle uses this ordinal. */
		kVertexColor10_Stone = 10,
	};

	struct Edge {
		/** Source vertex index in one tablet rule. */
		int from = 0;
		/** Destination vertex index in one tablet rule. */
		int to = 0;
		/** Whether applying this edge exchanges the beetles at both endpoints. */
		bool swap = false;
	};
	/** Vertex transfers and swaps performed by one tablet. */
	typedef Common::Array<Edge> Rule;
	/** Ordered tablet rules; each resource candidate contains one such set. */
	typedef Common::Array<Rule> RuleSet;
	/** One resource-defined board layout and the rules offered on its tablets. */
	struct LayoutDef {
		/** Puzzle level for which this layout can be selected. */
		int puzzleLevel = 0;
		/**
		 * Raw relative weight w; W is the sum of weights for layouts with the same puzzle level.
		 * Nominal selection chance is w / W (100 * w / W percent); weights 2, 3, 5 give 20%, 30%, 50%.
		 * Zero weight is never selected; W == 0 prevents board generation.
		 * The original PRNG's modulo reduction may bias these nominal probabilities.
		 */
		int selectionWeight = 0;
		/** Beetle vertex locations in the layout's local coordinate system. */
		Common::Array<Common::Point32> vertexCoords;
		/**
		 * Candidate tablet-rule sets in three independent choice groups.
		 * Generation selects one set from each group and combines their tablet rules.
		 */
		Common::Array<RuleSet> ruleSetCandidates[3];
	};
	/** Read bounded layout records, retaining complete records before damaged trailing data. */
	bool load(Common::SeekableReadStream &stream);
	/** Choose a weighted layout for @p puzzleLevel and construct its initial scrambled board. */
	bool generate(int puzzleLevel, Random &random);
	/** Apply the player's selected tablet rule to the current beetle positions. */
	void apply(int ruleIndex);
	/** Return whether the beetle of @p vertexColor occupies its matching vertex. */
	bool matched(VertexColor vertexColor) const;
	/** Return whether every beetle needed by @p gate is at its matching vertex. */
	bool gateMatched(int gate) const;
	/** Return the selected resource layout index. */
	int layoutIndex() const { return _layoutIndex; }
	/** Return the selected gate and tablet layout. */
	const LayoutDef &layout() const { return _layouts[_layoutIndex]; }
	/** Return all layouts loaded from the maze resource. */
	const Common::Array<LayoutDef> &layouts() const { return _layouts; }
	/** Return the selected logical gate and tablet rules. */
	const RuleSet &rules() const { return _rules; }
	/** Return the current vertex occupied by each beetle. */
	const Common::Array<int> &positions() const { return _positions; }
	/** Return the initial vertex occupied by each beetle after board generation. */
	const Common::Array<int> &initialPositions() const { return _initialPositions; }
	/** Return the retained sequence of tablet moves used during generation. */
	const Common::Array<int> &generationSequence() const { return _generationSequence; }
	/** Distinguish an exhausted reachable state space from an interrupted search. */
	enum class SolutionResult {
		/** A solution sequence was found. */
		kSolved,
		/** The reachable state space contains no solution. */
		kUnreachable,
		/** The search limit was reached before the state space was exhausted. */
		kLimit,
		/** The starting board is not a valid permutation. */
		kInvalid,
	};
	/** Find a shortest player-rule sequence from the current board without changing it. */
	SolutionResult debugAnswerSolver(Common::Array<int> &sequence, uint &visited) const;
	/** Describe the selected layout's cycle and exchange strategy. */
	const char *debugLayoutAdvice() const;
	/** Describe the selected tablet's role in the layout strategy. */
	const char *debugTabletRole(uint rule) const;
	/** Return the nontrivial cycles of one tablet in vertex-index order, following its actual movement direction. */
	Common::Array<Common::Array<int>> debugTabletPaths(uint rule) const;
	/** Return where each vertex's beetle goes after one simultaneous tablet press. */
	Common::Array<int> debugRuleMapping(uint rule) const;

private:
	/** Read a count only when it fits @p maximum and the resource's trusted bounds. */
	static bool readCount(Common::SeekableReadStream &stream, int &count, int maximum);
	/** Decode one structurally complete layout record from the resource stream. */
	static bool readLayout(Common::SeekableReadStream &stream, LayoutDef &layout);
	/** Apply a whole rule once per beetle, or sequentially when constructing a scramble. */
	static void applyRule(const Rule &rule, Common::Array<int> &positions, bool simultaneous);
	/** Return whether @p positions has every beetle at its matching point. */
	static bool isIdentity(const Common::Array<int> &positions);
	/** Search the internal construction space for a non-identity scramble. */
	int solve(const Common::Array<int> &positions, int depth) const;
	/** Pack a permutation and reject repeated or out-of-range vertex indices. */
	static bool debugEncodePositions(const Common::Array<int> &positions, uint64 &state);
	/** Return the dense permutation index of a packed board. */
	static uint debugPermutationRank(uint64 state, uint count);
	/** Return whether a tablet transfers beetles between the selected vertex group and its complement. */
	static bool debugCrossesGroup(const Common::Array<int> &mapping, uint group);
	/** Build an initial board by applying selected rules sequentially. */
	void scramble(Random &random);
	/** All decoded layouts, selected layout index, active rules, and current beetle positions. */
	Common::Array<LayoutDef> _layouts;
	/** Index of the selected resource layout, or -1 before generation. */
	int _layoutIndex = -1;
	/** Tablet operations selected for the current board. */
	RuleSet _rules;
	/** Current vertex occupied by each beetle color. */
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
	/** Release the vertex surface, beetle paths, page animations, and loaded sounds. */
	~PuzzleMagicWall() override;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	/** Return the 1-based route containing this puzzle. */
	int getRouteNumber() const override { return kRouteNumber; }
	/** Return the 1-based puzzle number within its route. */
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }
	/** Load resources, generate a beetle board, and arrange the puzzle roster. */
	void init() override;
	/** Advance tablet, gate, beetle, runner, and speech transitions. */
	void onUpdate() override;
	/** Restore the puzzle's primary background layer. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw tablets, connections, the vertex surface, beetles, lights, and gates. */
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
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = "Beetle Bug Alley";
	/** 1-based route number used by the puzzle console. */
	static constexpr int kRouteNumber = 2;
	/** 1-based puzzle number within the route used by the puzzle console. */
	static constexpr int kRoutePuzzleIdx = 1;
	/** Beetle Bug Alley board background loaded into the first page layer. */
	static constexpr const char *kBackgroundPath = "#bmp/magic_wall/magic wall";
	/** ZTL board data containing weighted layouts, vertex coordinates, and candidate tablet-rule sets. */
	static constexpr const char *kLayoutPath = "bmp/magic_wall/default.ztl";
	/** Looping Beetle Bug Alley soundtrack. */
	static constexpr const char *kMusicPath = "#sounds/music/06-BB01.wav";
	/** Path format for colored vertex markers on the board and tablets. */
	static constexpr const char *kDotFormat = "bmp/magic_wall/DOT-%s";
	/** Path format for the colored beetle sprite assigned to each vertex color. */
	static constexpr const char *kBugFormat = "bmp/magic_wall/bug_c_%s";
	/** Path format for one of eight beetle-facing direction overlays. */
	static constexpr const char *kDirectionFormat = "bmp/magic_wall/bug_%d";
	/** Path format for matched-color lamps. */
	static constexpr const char *kLightFormat = "bmp/magic_wall/mini-light-%s";
	/** Neutral stone lamp drawn for a vertex whose beetle does not match. */
	static constexpr const char *kStoneLightPath = "bmp/magic_wall/mini-light-pierre";
	/** RLE image drawn as the frame for each rule tablet. */
	static constexpr const char *kTabletPath = "bmp/magic_wall/mini-map";
	/** RLE vertex marker drawn on a rule tablet. */
	static constexpr const char *kTabletDotPath = "bmp/magic_wall/mini-map-dot";
	/** Lever animation played when the player submits the current beetle positions. */
	static constexpr const char *kLeverPath = "bmp/magic_wall/le_vier";
	/** BitBlock glow drawn over the lever when every beetle matches its vertex. */
	static constexpr const char *kLeverGlowPath = "bmp/magic_wall/le_vier_luisant";
	/** Animation path format for the four lettered gates opening and closing. */
	static constexpr const char *kGateFormat = "bmp/magic_wall/porte-%c";
	/** BitBlock wall and entrance artwork drawn behind runners at each gate. */
	static constexpr const char *kGateBackFormat = "bmp/magic_wall/porte-%csingle";
	/** PAT route format for a rescued Zoombini to pass through a matched gate and leave. */
	static constexpr const char *kExitFormat = "bmp/magic_wall/PAT/EXIT%d.PAT";
	/** PAT route format for a Zoombini to move from its roster position to a matched gate. */
	static constexpr const char *kMoveFormat = "bmp/magic_wall/PAT/BOUGE%d.PAT";
	/** Looped sound played while beetles move along a tablet rule. */
	static constexpr const char *kBugLoopPath = "sounds/fx/06-BB02.wav";
	/** Sound played when a matched gate opens for a rescue. */
	static constexpr const char *kGateSoundPath = "sounds/fx/06-BS03.wav";
	/** Sound played when the lever submits the current board. */
	static constexpr const char *kLeverSoundPath = "sounds/fx/06-BS05.wav";
	/** Retreat speech played before leaving with four or more Zoombinis still in the roster. */
	static constexpr const char *kRetreatSpeechPath = "sounds/DW-Cave.wav";
	/** One of four randomized praise lines played after all Zoombinis are rescued. */
	static constexpr const char *kPerfectSpeechFormat = "sounds/wld11.%d.wav";
	/** Resource filename suffixes keyed by original beetle-color identity. */
	typedef Common::HashMap<MagicWallBoard::VertexColor, const char *, Common::Hash<uint>> VertexColorNameMap;
	/** Build the resource suffix mapping for the ten beetle identities. */
	static VertexColorNameMap createVertexColorNames();
	/** Return the immutable resource suffix mapping reused across page instances. */
	static const VertexColorNameMap &getVertexColorNames();
	/** Upper-range channels of the shaded vertex discs, avoiding clipped highlights after recoloring. */
	static constexpr byte kDotSourceBrightness[10] = {164, 154, 168, 154, 156, 220, 172, 140, 126, 128};
	/** Peak channels at the centers of the ten source beetle sprites. */
	static constexpr byte kBugSourceBrightness[10] = {228, 180, 130, 208, 157, 201, 197, 200, 213, 208};
	/** Replacement RGB colors keyed by beetle and target-vertex identity. */
	typedef Common::HashMap<MagicWallBoard::VertexColor, RGBColor, Common::Hash<uint>> VertexColorPalette;
	/** Build the red-green assistance palette without treating color ordinals as array offsets. */
	static VertexColorPalette createRedGreenVertexColorPalette();
	/** Select a display RGB color for a beetle identity, leaving the neutral stone lamp unchanged. */
	static bool vertexColorAssistRGB(ColorAssistMode mode, MagicWallBoard::VertexColor vertexColor, RGBColor &displayColor);
	/** Draw one identity-colored RLE resource with the current display palette. */
	void drawVertexColorSprite(ManagedSurface32 *screen, const char *format, MagicWallBoard::VertexColor vertexColor,
							   const Common::Point32 &pos, byte sourceBrightness) const;
	/** Screen locations for tablets, gates, roster runners, and gate lights. */
	static constexpr Common::Point32 kTabletPositions[5] = {
		{350, 400},
		{415, 417},
		{482, 425},
		{553, 440},
		{616, 455},
	};
	/** Page-space anchors in pixels for the four gate animations. */
	static constexpr Common::Point32 kGatePositions[4] = {
		{76, 290},
		{129, 288},
		{180, 288},
		{228, 285},
	};
	/** Waiting Zoombini anchors in page pixels, indexed by party slot. */
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
	/** Page-space anchors in pixels for the gate indicator artwork. */
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
		/** Current beetle anchor in page pixels. */
		Common::Point32 position;
		/** Beetle movement path released on replacement or page teardown; nullptr while stationary. */
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
	/** Convert a layout vertex to page-space coordinates. */
	Common::Point32 vertexPosition(int index) const;
	/** Convert a layout vertex to an inset position on the selected tablet. */
	Common::Point32 tabletPoint(int tablet, int point, int inset) const;
	/** Clear and redraw the current layout's vertex patches without changing the background surface. */
	void rebuildVertexSurface();
	/** Composite the cached vertex patches above connections and below beetles, refreshing changed display colors. */
	void drawVertices(ManagedSurface32 *screen);
	/** Draw each tablet's background and inset rule diagram. */
	void drawTablets(ManagedSurface32 *screen);
	/** Draw the connection ripples belonging to the selected tablet rule. */
	void drawConnections(ManagedSurface32 *screen, int index);
	/** Draw colored beetles and their facing indicators at their current maze positions. */
	void drawBeetles(ManagedSurface32 *screen);
	/** Draw the animated connection ripple between page-pixel anchors at the supplied phase. */
	void drawRipple(ManagedSurface32 *screen, Common::Point32 from, const Common::Point32 &to, int phase);
	/** Select a beetle direction frame from a point-to-point movement vector. */
	static int directionIndex(const Common::Point32 &from, const Common::Point32 &to);
	/** Return whether @p pos lies in a page-space rectangle. */
	static bool inside(const Common::Point &pos, int x, int y, int width, int height);
	/** Clear the active gate runner after its opening animation completes. */
	static void gateDone(void *context, AnimationRunner *runner);

	/** Resource-defined board, tablet rules and current beetle permutation. */
	MagicWallBoard _board;
	/** Beetles on the current board, one per layout vertex: L1/L2 use 5-8, L3 uses 6-9, and L4 uses 8-9. */
	Common::Array<Beetle> _beetles;
	/** Transparent page-sized vertex patches that mask connections, released with the page and rebuilt for each layout. */
	ManagedSurface32 *_vertexSurface = nullptr;
	/** Color assistance mode represented by the cached vertex surface. */
	ColorAssistMode _vertexSurfaceColorAssistMode = ColorAssistMode::kOriginal;
	/** Animation data for the four exit gates, released with the page. */
	Animation *_gateAnimations[4] = {};
	/** Submission-lever animation data released with the page. */
	Animation *_leverAnimation = nullptr;
	/** Gate playback state, indexed by gate. */
	AnimationRunner *_gateRunners[4] = {};
	/** Playback state for the submission lever. */
	AnimationRunner *_leverRunner = nullptr;

	/** A runner is retired only after its exit path reaches the upper side of the gate. */
	bool _exited[8] = {};
	/** Accepted submissions move from the first board to the second, then finish the puzzle. */
	enum class Phase {
		kFirstBoard,
		kSecondBoard,
		kFinished
	};
	/** Whether the current board has finished initializing. */
	bool _ready = false;
	/** Whether the first board has succeeded and a replacement board is due. */
	bool _nextPuzzlePending = false;
	/** Whether the current page state permits departure through Go. */
	bool _canDepart = false;
	/** Whether a tablet press is armed for the current pointer gesture. */
	bool _buttonArmed = false;
	/** Whether departure is waiting for queued narration. */
	bool _speechPending = false;
	/** Current board and departure stage. */
	Phase _phase = Phase::kFirstBoard;
	/** Tablet under the pointer, or -1 when no tablet is hovered. */
	int _hoveredTablet = -1;
	/** Tablet rule currently being animated, or -1 when none is active. */
	int _movingRule = -1;
	/** Current ripple presentation phase around the rule animation. */
	int _ripplePhase = 0;
	/** Latest pointer location used for tablet hover feedback. */
	Common::Point _pointer;
	/** Loaded looping beetle sound handle, or -1 when unavailable. */
	int _bugSound = -1;
	/** Loaded gate sound handle, or -1 when unavailable. */
	int _gateSound = -1;
	/** Loaded lever sound handle, or -1 when unavailable. */
	int _leverSound = -1;
};

} // End of namespace Zoombini2

#endif
