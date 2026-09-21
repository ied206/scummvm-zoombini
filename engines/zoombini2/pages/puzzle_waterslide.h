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
	PuzzleWaterslide(Zoombini2Engine *vm);
	~PuzzleWaterslide() override;
	void init() override;
	void onUpdate() override;
	void onRenderBackground(ManagedSurface32 *screen) override;
	void onRenderContent(ManagedSurface32 *screen) override;
	void onRenderActors(ManagedSurface32 *screen) override;
	void onActorsRendered() override;
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	bool onGoButtonPressed() override;
	bool canUseGoButton() const override;
	Common::String debugGetAnswer() const override;
	Common::String debugGetChanceDetails() const override;

private:
	static constexpr const char *kMusicPath = "#sounds/music/02-BS01.wav";
	static constexpr const char *kTraitFormat = "bmp/waterslide/traits/%d";
	static constexpr const char *kHorizontalFormat = "bmp/waterslide/pipes - %s/pipe - horizontal";
	static constexpr const char *kLargePipeFormat = "bmp/waterslide/pipes - blue/pipe - lev%d_bigone";
	static constexpr const char *kSelectorFormat = "bmp/waterslide/pipes - %s/pipe_bigone_selector_%02d";
	static constexpr const char *kHardPipeFormat = "bmp/waterslide/pipes - %s/pipe - hard %02d";
	static constexpr const char *kPipeColors[2] = {
		"grey",
		"blue",
	};
	static constexpr const char *kHardPipeColors[2] = {
		"grey",
		"red",
	};
	static constexpr const char *kMiniDiagonalPath = "bmp/waterslide/pipes - blue/pipe - mini racord diagon";
	static constexpr const char *kMiniHorizontalPath = "bmp/waterslide/pipes - blue/pipe - mini horizontal";
	static constexpr const char *kOutletPath = "bmp/waterslide/pipes - red/truc_rouge";
	static constexpr const char *kPastillePath = "bmp/waterslide/pastilles grey";
	static constexpr const char *kEdgePath = "bmp/waterslide/edge neutre";
	static constexpr const char *kFountainPath = "bmp/waterslide/blue funtain";
	static constexpr const char *kTreePath = "bmp/waterslide/little tree";
	static constexpr const char *kValvePath = "bmp/waterslide/mr valve master";
	static constexpr const char *kCascadeFormat = "bmp/waterslide/pipe - cascade %d";
	static constexpr const char *kAreaPath = "bmp/waterslide/area.bmt";
	static constexpr const char *kPickupPath = "bmp/zombis/pris/pris.anm";
	static constexpr const char *kIdlePath = "bmp/zombis/attente/attenteZomb.anm";
	static constexpr const char *kAspirationPath = "bmp/zombis/aspiration/aspiration.anm";
	static constexpr const char *kSoundFormat = "sounds/fx/%s.wav";
	static constexpr const char *kPraisePath = "sounds/wsl31.wav";
	static constexpr const char *kPartialPraisePath = "sounds/wsl31alt.wav";
	static constexpr const char *kRetreatSpeech = "sounds/DW-Zville.wav";
	static constexpr const char *kGoSpeechFormat = "sounds/wld11.%d.wav";

	enum Phase {
		kInteractive00 = 0,
		kValve01 = 1,
		kDischarge02 = 2,
		kFinished03 = 3,
	};
	/** Generated endpoints name board slots; a negative axis denotes an unlabeled connection. */
	struct Edge {
		int a = -1;
		int b = -1;
		int axis = -1;
		bool connected = false;
	};
	struct Pair {
		int a = -1;
		int b = -1;
		int axis = -1;
	};
	struct GraphPlacement {
		int a, b, type, labelX, labelY;
	};
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

	void loadResources();
	void generateEasy();
	void generateMedium();
	void generateHard();
	int generateGraph(bool randomRoot);
	int sharedAxis(int first, int second, bool rejectLast);
	bool findPair(int source, bool *available, Pair &pair);
	int trait(int generatedIndex, int axis) const;
	void addEdge(int a, int b, int axis);
	void setupTargets();
	static Common::Point32 graphPosition(int slot);
	static const GraphPlacement *graphPlacement(const Edge &edge);
	void evaluateConnections();
	bool matches(const Edge &edge) const;
	bool debugPlacementMatches(int slot, int actor, const int *assignment) const;
	bool debugFindPlacement(int *assignment, uint32 used, int &budget) const;
	void activateValve();
	void dischargeNext();
	void playSound(int sound);
	int countFreeZoombinis() const;
	void drawBoard(ManagedSurface32 *screen) const;
	void drawDecorations(ManagedSurface32 *screen);
	static void onSlotChanged(void *context, int slot, int zoombini);
	static void onAspirationComplete(void *context, ZoombiniRunner *zoombini);
	static void onCascadeComplete(void *context, AnimationRunner *runner);

	int _level = 1;
	Phase _phase = kInteractive00;
	int _connectionCount = 0;
	int _heldZoombini = -1;
	bool _eligible[16] = {};
	bool _activeSlot[16] = {};
	int _solution[16] = {};
	Common::Array<int> _generationOrder;
	Common::Array<Edge> _edges;
	Common::Array<ZoombiniDropTarget> _targets;
	Common::Point32 _valvePos;
	Common::Point32 _cascadePos;
	uint32 _dischargeStart = 0;
	bool _goPending = false;
	int _goSpeech = -1;
	int _praiseSpeech = -1;
	int _sounds[6] = {
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
	};

	Animation *_tree = nullptr;
	Animation *_fountain = nullptr;
	Animation *_valve = nullptr;
	Animation *_cascade = nullptr;
	AnimationRunner *_valveRunner = nullptr;
	AnimationRunner *_cascadeRunner = nullptr;
	AnimationRunner *_treeRunner = nullptr;
	AnimationRunner *_fountainRunner = nullptr;
	const ZoombiniAnimation *_pickup = nullptr;
	const ZoombiniAnimation *_idle = nullptr;
	const ZoombiniAnimation *_aspiration = nullptr;
};

} // End of namespace Zoombini2

#endif
