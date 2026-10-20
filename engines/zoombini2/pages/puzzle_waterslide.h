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

#ifndef ZOOMBINI2_PAGES_PUZZLE_WATERSLIDE_H
#define ZOOMBINI2_PAGES_PUZZLE_WATERSLIDE_H

#include "zoombini2/pages/puzzle_base.h"
#include "zoombini2/scripts.h"

namespace Zoombini2 {

/** Arrange matching traits along the pipes, then release the connected party with the valve. */
class PuzzleWaterslide : public PuzzleBase {
public:
	/** Construct the Water Slide pipe-matching puzzle for @p vm. */
	PuzzleWaterslide(Zoombini2Engine *vm);
	/** Release active decoration and valve animation runners. */
	~PuzzleWaterslide() override;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	/** Return the 1-based route containing this puzzle. */
	int getRouteNumber() const override { return kRouteNumber; }
	/** Return the 1-based puzzle number within its route. */
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }
	/** Generate the level board, load resources, and arrange the party. */
	void init() override;
	/** Advance the valve, cascade, and Zoombini discharge sequence. */
	void onUpdate() override;
	/** Restore the background before drawing pipes and puzzle decorations. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw pipe connections, labels, the valve, and the waterfall. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Draw waiting, picked-up, and draining Zoombinis. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Advance Zoombini animation frames after the actor pass. */
	void onActorsRendered() override;
	/** Start holding a Zoombini or arm the valve at @p pos. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Drop the held Zoombini into a compatible slot at @p pos. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Update target hover feedback while a Zoombini is held. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Return to the map immediately or after retreat speech. */
	bool onGoButtonPressed() override;
	/** Return whether every required connection has resolved. */
	bool canUseGoButton() const override;
	/** Format the generated placement solution for the puzzle console. */
	Common::String debugGetAnswer() const override;
	/** Report generated graph and current connections for the puzzle console. */
	Common::String debugGetChanceDetails() const override;

private:
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = "Pipes of Paloo";
	/** 1-based route number used by the puzzle console. */
	static constexpr int kRouteNumber = 1;
	/** 1-based puzzle number within the route used by the puzzle console. */
	static constexpr int kRoutePuzzleIdx = 2;
	/** Pipes of Paloo background loaded into the first page layer. */
	static constexpr const char *kBackgroundPath = "#bmp/waterslide/waterslides";
	/** Background music played while Pipes of Paloo is active. */
	static constexpr const char *kMusicPath = "#sounds/music/02-BS01.wav";
	/** Trait icon format for the numbered Zoombini traits shown beside pipe endpoints. */
	static constexpr const char *kTraitFormat = "bmp/waterslide/traits/%d";
	/** Horizontal pipe segment selected by its color directory. */
	static constexpr const char *kHorizontalFormat = "bmp/waterslide/pipes - %s/pipe - horizontal";
	/** Large blue entrance/exit pipe art selected by the internal level number. */
	static constexpr const char *kLargePipeFormat = "bmp/waterslide/pipes - blue/pipe - lev%d_bigone";
	/** Large-pipe selector art, selected by pipe color and arm number. */
	static constexpr const char *kSelectorFormat = "bmp/waterslide/pipes - %s/pipe_bigone_selector_%02d";
	/** Hard-mode pipe segment art, selected by color and graph edge type. */
	static constexpr const char *kHardPipeFormat = "bmp/waterslide/pipes - %s/pipe - hard %02d";
	/** Directory suffixes for neutral gray and connected blue horizontal pipes. */
	static constexpr const char *kPipeColors[2] = {
		"grey",
		"blue",
	};
	/** Directory suffixes for neutral gray and connected red hard-mode pipes. */
	static constexpr const char *kHardPipeColors[2] = {
		"grey",
		"red",
	};
	/** Small diagonal connector segments used to draw the easy/medium pipe graph. */
	static constexpr const char *kMiniDiagonalPath = "bmp/waterslide/pipes - blue/pipe - mini racord diagon";
	/** Small horizontal connector segments used to draw the easy/medium pipe graph. */
	static constexpr const char *kMiniHorizontalPath = "bmp/waterslide/pipes - blue/pipe - mini horizontal";
	/** Red outlet artwork at the end of the hard-mode pipe graph. */
	static constexpr const char *kOutletPath = "bmp/waterslide/pipes - red/truc_rouge";
	/** Gray connector marker drawn beneath each pipe endpoint. */
	static constexpr const char *kPastillePath = "bmp/waterslide/pastilles grey";
	/** Neutral edge artwork used for unconnected pipe links. */
	static constexpr const char *kEdgePath = "bmp/waterslide/edge neutre";
	/** Animated fountain scenery beside the board. */
	static constexpr const char *kFountainPath = "bmp/waterslide/blue funtain";
	/** Animated tree scenery beside the board. */
	static constexpr const char *kTreePath = "bmp/waterslide/little tree";
	/** Animated valve that starts the discharge sequence when clicked. */
	static constexpr const char *kValvePath = "bmp/waterslide/mr valve master";
	/** Cascade animation format; levels below 3 use variant 1, level 3 and higher use variant 2. */
	static constexpr const char *kCascadeFormat = "bmp/waterslide/pipe - cascade %d";
	/** Area mask used to constrain Zoombini hold-and-drop hit testing. */
	static constexpr const char *kAreaPath = "bmp/waterslide/area.bmt";
	/** Sound-effects path format used for the six Waterslide effect identifiers. */
	static constexpr const char *kSoundFormat = "sounds/fx/%s.wav";
	/** Praise speech played when all eight expected connections are made. */
	static constexpr const char *kPraisePath = "sounds/wsl31.wav";
	/** Alternate praise speech played when the board is completed with fewer connections. */
	static constexpr const char *kPartialPraisePath = "sounds/wsl31alt.wav";
	/** Speech queued when the player leaves before starting the discharge. */
	static constexpr const char *kRetreatSpeech = "sounds/DW-Zville.wav";
	/** Perfect-clear Go speech format; the argument selects a voice variant. */
	static constexpr const char *kGoSpeechFormat = "sounds/wld11.%d.wav";

	enum Phase {
		/** Accept player placements and update graph connections. */
		kInteractive00 = 0,
		/** Animate the opened valve before water begins flowing. */
		kValve01 = 1,
		/** Send connected Zoombinis through the cascade one at a time. */
		kDischarge02 = 2,
		/** All discharge work is complete and Go may leave the puzzle. */
		kFinished03 = 3,
	};
	/** Generated endpoints name board slots; a negative axis denotes an unlabeled connection. */
	struct Edge {
		/** First endpoint slot of this graph connection. */
		int a = -1;
		/** Second endpoint slot of this graph connection. */
		int b = -1;
		/** Trait axis used by this connection; -1 denotes an unlabeled edge. */
		int axis = -1;
		/** Whether the currently placed pair satisfies this connection. */
		bool connected = false;
	};
	/** Candidate pair returned while generating one trait-labeled graph edge. */
	struct Pair {
		/** First endpoint slot of this candidate pair. */
		int a = -1;
		/** Second endpoint slot of this graph connection. */
		int b = -1;
		/** Trait axis used by this connection; -1 denotes an unlabeled edge. */
		int axis = -1;
	};
	/** Screen placement and art type for one possible graph edge. */
	struct GraphPlacement {
		/** First board-slot endpoint of the pipe. */
		int a;
		/** Second board-slot endpoint of the pipe. */
		int b;
		/** Pipe artwork variant selected for this edge. */
		int type;
		/** Trait-label x coordinate in page pixels. */
		int labelX;
		/** Trait-label y coordinate in page pixels. */
		int labelY;
	};
	/** Adjacency candidates used to construct a connected sixteen-slot graph. */
	static constexpr int kNeighbors[16][5] = {
		{1, 6, 8, 4, -1},
		{0, 9, 2, -1, -1},
		{1, 6, 10, 3, -1},
		{2, 7, 11, -1, -1},
		{0, 5, 12, -1, -1},
		{4, 6, 13, -1, -1},
		{5, 0, 2, 7, 14},
		{6, 3, 15, -1, -1},
		{0, 9, 12, -1, -1},
		{8, 10, 15, 1, -1},
		{9, 14, 11, 2, -1},
		{10, 3, 15, -1, -1},
		{8, 4, -1, -1, -1},
		{5, 14, -1, -1, -1},
		{13, 10, 6, -1, -1},
		{9, 7, 11, -1, -1},
	};
	/** Waiting positions for the party and visual placements for all drawable graph edges. */
	static constexpr Common::Point32 kWaitingPositions[16] = {
		{131, 312},
		{158, 369},
		{180, 414},
		{177, 469},
		{162, 518},
		{114, 355},
		{122, 405},
		{129, 457},
		{112, 509},
		{69, 339},
		{70, 397},
		{81, 463},
		{61, 516},
		{28, 475},
		{37, 423},
		{31, 367},
	};
	/** Pipe artwork and label anchors for each drawable endpoint pair. */
	static constexpr GraphPlacement kGraphPlacements[26] = {
		{0, 6, 1, 542, 380},
		{9, 15, 1, 252, 272},
		{0, 1, 0, 597, 383},
		{1, 2, 0, 562, 322},
		{2, 3, 0, 522, 242},
		{4, 5, 0, 512, 455},
		{5, 6, 0, 472, 374},
		{6, 7, 0, 433, 291},
		{8, 9, 0, 385, 381},
		{9, 10, 0, 343, 299},
		{10, 11, 0, 308, 226},
		{13, 14, 0, 257, 362},
		{4, 12, 2, 403, 481},
		{0, 8, 2, 464, 433},
		{5, 13, 2, 331, 403},
		{1, 9, 2, 417, 353},
		{6, 14, 2, 392, 314},
		{7, 15, 2, 359, 239},
		{2, 10, 2, 470, 270},
		{3, 11, 2, 373, 190},
		{0, 4, 3, 565, 465},
		{8, 12, 3, 367, 455},
		{10, 14, 3, 304, 288},
		{11, 15, 3, 247, 214},
		{3, 7, 3, 445, 225},
		{2, 6, 3, 494, 303},
	};

	/** Load page art, sounds, interaction targets, and Zoombini animations. */
	void loadResources();
	/** Construct a deterministic party when the page runs without a saved roster. */
	void createDemoParty();
	/** Generate the easy-level pipe graph and its trait-placement constraints. */
	void generateEasy();
	/** Generate the medium-level pipe graph and its trait-placement constraints. */
	void generateMedium();
	/** Generate the hard-level pipe graph and its trait-placement constraints. */
	void generateHard();
	/** Build a graph and return its root slot, optionally selecting it randomly. */
	int generateGraph(bool randomRoot);
	/** Return a usable common trait axis for two generated Zoombinis. */
	int sharedAxis(int first, int second, bool rejectLast);
	/** Find a still-available partner for @p source and record it in @p pair. */
	bool findPair(int source, Common::Array<bool> &available, Pair &pair);
	/** Return generated Zoombini @p generatedIndex's value on trait @p axis. */
	int trait(int generatedIndex, int axis) const;
	/** Add one logical connection and its screen placement to the board graph. */
	void addEdge(int a, int b, int axis);
	/** Create droppable slot targets for the current graph. */
	void setupTargets();
	/** Return the page-pixel anchor for a graph slot. */
	static Common::Point32 graphPosition(int slot);
	/** Return the artwork placement matching an edge's unordered endpoints, or nullptr. */
	static const GraphPlacement *graphPlacement(const Edge &edge);
	/** Update each edge's satisfied flag after a slot assignment changes. */
	void evaluateConnections();
	/** Return whether both occupants of @p edge match its required trait axis. */
	bool matches(const Edge &edge) const;
	/** Return the slot at a zero-based display row and column, or -1 for a gap. */
	int debugCellSlot(int row, int column) const;
	/** Start the valve animation and the following cascade sequence. */
	void activateValve();
	/** Start the next eligible Zoombini's drain animation. */
	void dischargeNext();
	/** Play the indexed pipe or valve sound effect. */
	void playSound(int sound);
	/** Return the number of roster members not yet eligible to leave the puzzle. */
	int countFreeZoombinis() const;
	/** Draw the pipe graph, its occupants and trait labels. */
	void drawBoard(ManagedSurface32 *screen) const;
	/** Draw and advance the tree, fountain and active cascade animations. */
	void drawDecorations(ManagedSurface32 *screen);
	/** Update puzzle state after a Zoombini is assigned to a graph slot. */
	static void onSlotChanged(void *context, int slot, int zoombini);
	/** Hide the rider after its suction animation completes. */
	static void onAspirationComplete(void *context, ZoombiniRunner *zoombini);
	/** Hide the current cascade rider and clear the held-roster index. */
	static void onCascadeComplete(void *context, AnimationRunner *runner);

	/** Current interaction, discharge or departure stage. */
	Phase _phase = kInteractive00;
	/** Number of graph edges satisfied by the current placement. */
	int _connectionCount = 0;
	/** Puzzle-roster index currently held by the pointer, or -1 when none is held. */
	int _heldZoombini = -1;
	/** All levels keep one eligibility flag per party member, up to 16. */
	Common::Array<bool> _eligible;
	/** Active flags: L1 uses party size, up to 16; L2-L4 use all 16 slots. */
	Common::Array<bool> _activeSlot;
	/** Each active slot names a distinct participant in the intended solution; inactive slots contain -1. */
	Common::Array<int> _solution;
	/** All levels keep one generation-order entry per party member, up to 16. */
	Common::Array<int> _generationOrder;
	/** Graph connections for the generated board. */
	Common::Array<Edge> _edges;
	/** Targets: L1 uses party size, up to 16; L2-L4 use all 16 slots. */
	Common::Array<ZmbDropTarget> _targets;
	/** Valve/cascade placement, discharge timing, and deferred Go/speech state. */
	Common::Point32 _valvePos;
	/** Current cascade presentation anchor in page pixels. */
	Common::Point32 _cascadePos;
	/** Gameplay tick in milliseconds when water discharge began. */
	uint32 _dischargeStart = 0;
	/** Whether departure is waiting for the current speech or animation. */
	bool _goPending = false;
	/** Indexed pipe and valve effects. */
	int _sounds[6] = {
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
	};

	/** Decorative and valve animation resources plus their active runners. */
	Animation *_tree = nullptr;
	/** Fountain animation data released with the page. */
	Animation *_fountain = nullptr;
	/** Valve animation data released with the page. */
	Animation *_valve = nullptr;
	/** Water-cascade animation data released with the page. */
	Animation *_cascade = nullptr;
	/** Playback state for the valve animation. */
	AnimationRunner *_valveRunner = nullptr;
	/** Playback state for the cascade animation. */
	AnimationRunner *_cascadeRunner = nullptr;
	/** Playback state for the tree animation. */
	AnimationRunner *_treeRunner = nullptr;
	/** Playback state for the fountain animation. */
	AnimationRunner *_fountainRunner = nullptr;
};

} // End of namespace Zoombini2

#endif
