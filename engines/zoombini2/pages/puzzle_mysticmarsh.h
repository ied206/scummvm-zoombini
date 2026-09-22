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

#ifndef ZOOMBINI2_PAGES_PUZZLE_MYSTICMARSH_H
#define ZOOMBINI2_PAGES_PUZZLE_MYSTICMARSH_H

#include "common/array.h"
#include "zoombini2/pages/puzzle_base.h"
#include "zoombini2/scripts.h"
#include "zoombini2/state.h"

namespace Zoombini2 {

class Random;

/** Rules and simultaneous bubble movement for Mystic Marsh. */
class MysticMarshGrid {
public:
	/** Grid dimensions in logical device cells. */
	static constexpr int kColumns = 16;
	static constexpr int kRows = 12;
	static constexpr int kCellCount = kColumns * kRows;
	struct Cell {
		/** Template type, output direction, trigger number, trait test, value, and mutable visual state. */
		int type;
		int direction;
		int trigger;
		int trait;
		int value;
		int state;
		constexpr Cell(int t = 0, int d = 4, int g = 0, int f = 0, int v = 0, int s = 0)
			: type(t), direction(d), trigger(g), trait(f), value(v), state(s) {}
	};
	/** A Zoombini position before or after one simultaneous grid tick. */
	struct Occupant {
		/** Puzzle-roster index and direction used by its pending move. */
		int index = -1;
		int direction = -1;
	};
	/** One resolved simultaneous move from source cell to destination cell. */
	struct Move {
		int index;
		int from;
		int to;
		Move(int z, int a, int b) : index(z), from(a), to(b) {}
	};
	/** Construction records retained with the selected board, independently of live device changes. */
	struct GenerationInfo {
		/** Candidate-selection records retained from the successful generation branch. */
		Common::Array<int> selectionOrder;
		Common::Array<int> upper;
		Common::Array<int> lower;
		Common::Array<int> group;
		Common::Array<int> subset;
		/** Actors used as generator references and primary filter examples. */
		int referenceActors[2] = {
			-1,
			-1,
		};
		int primaryFilterActors[2] = {
			-1,
			-1,
		};
		/** Whether retry recovery restored the selected filter values. */
		bool restoredFilterValues = false;
	};
	/** Return retained generation data and the immutable initial device layout. */
	const GenerationInfo &generationInfo() const { return _generation; }
	const Cell &initialCell(int cellIndex) const { return _initialCells[cellIndex]; }
	/** Generate rules using the party and the shared random stream. */
	void init(const Common::Array<ZmbTrait> &party, int difficulty, Random &random);
	/** Place one roster entry in a crater. */
	bool place(int cellIndex, int zoombiniIndex);
	/** Advance one complete grid step and replace the result queues. */
	void tick();
	/** Return current cell state, selected layout, and its matching background index. */
	const Cell &cell(int index) const { return _cells[index]; }
	int layout() const { return _layout; }
	int background() const;
	/** Return results produced by the last simultaneous tick. */
	const Common::Array<Move> &moves() const { return _moves; }
	const Common::Array<int> &exits() const { return _exits; }
	const Common::Array<int> &lost() const { return _lost; }
	const Common::Array<int> &collisions() const { return _collisions; }
	bool turned() const { return _turned; }
	bool caught() const { return _caught; }
	bool released() const { return _released; }

private:
	struct Feature {
		/** One trait/value predicate used while selecting generator groups. */
		int trait;
		int value;
		Feature(int t = 0, int v = 0) : trait(t), value(v) {}
	};
	/** Decoded cell templates and layout-indexed template maps. */
	static const Cell kCellTemplates[];
	static constexpr byte kLayouts[8][kRows][kColumns] = {
		{
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 1, 2, 3, 3, 2, 3, 1, 4, 4, 5, 6, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 7, 1, 0, 0},
			{0, 8, 1, 9, 10, 1, 1, 11, 12, 1, 1, 1, 2, 1, 13, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 14, 1, 0, 0},
			{0, 0, 1, 2, 15, 2, 15, 1, 1, 16, 16, 16, 17, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 10, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
		},
		{
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 2, 3, 3, 2, 3, 1, 1, 18, 5, 18, 18, 10, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 8, 9, 10, 1, 1, 2, 12, 1, 2, 2, 2, 2, 1, 13, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 2, 15, 2, 15, 1, 1, 1, 19, 19, 19, 19, 10, 0, 0},
			{0, 0, 1, 1, 1, 1, 10, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
		},
		{
			{0, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 2, 1, 13},
			{0, 0, 2, 1, 1, 1, 1, 1, 1, 18, 18, 1, 1, 1, 6, 0},
			{0, 0, 1, 1, 1, 2, 1, 1, 1, 1, 1, 6, 1, 1, 6, 0},
			{0, 0, 20, 10, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
			{0, 0, 5, 1, 1, 7, 1, 1, 1, 1, 1, 1, 14, 21, 1, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 2, 22, 23, 1, 16, 24, 10, 0},
			{0, 8, 12, 1, 1, 4, 10, 1, 20, 1, 1, 1, 20, 1, 17, 0},
			{0, 0, 17, 1, 25, 17, 1, 1, 1, 26, 27, 1, 1, 1, 1, 0},
			{0, 8, 28, 29, 17, 1, 1, 17, 1, 30, 30, 2, 19, 19, 17, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
			{0, 8, 1, 1, 1, 1, 1, 1, 17, 1, 1, 1, 1, 1, 1, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		},
		{
			{0, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 2, 1, 13},
			{0, 0, 2, 1, 1, 1, 1, 1, 1, 18, 18, 1, 1, 1, 6, 0},
			{0, 0, 1, 1, 1, 2, 1, 1, 1, 1, 1, 6, 1, 1, 1, 0},
			{0, 0, 20, 10, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
			{0, 0, 5, 1, 1, 7, 1, 1, 1, 1, 1, 1, 14, 21, 1, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 2, 22, 23, 1, 16, 24, 10, 0},
			{0, 8, 12, 1, 1, 4, 10, 1, 20, 1, 1, 1, 1, 20, 17, 0},
			{0, 0, 17, 1, 25, 17, 1, 1, 1, 26, 27, 1, 1, 1, 1, 0},
			{0, 8, 28, 29, 17, 1, 1, 17, 1, 30, 30, 2, 19, 19, 17, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
			{0, 8, 1, 1, 1, 1, 1, 1, 17, 1, 1, 1, 1, 1, 1, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		},
		{
			{0, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 2, 1, 13},
			{0, 0, 2, 1, 1, 1, 1, 1, 1, 18, 18, 1, 1, 1, 6, 0},
			{0, 0, 1, 1, 1, 2, 1, 1, 1, 1, 1, 6, 1, 1, 1, 0},
			{0, 0, 1, 10, 1, 31, 1, 1, 1, 1, 20, 1, 1, 1, 6, 0},
			{0, 0, 5, 1, 1, 7, 1, 1, 1, 1, 1, 1, 14, 21, 1, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 2, 22, 23, 1, 16, 24, 10, 0},
			{0, 8, 12, 1, 1, 4, 10, 1, 20, 1, 1, 1, 1, 1, 17, 0},
			{0, 0, 17, 1, 25, 17, 1, 1, 1, 26, 27, 1, 1, 1, 1, 0},
			{0, 8, 28, 29, 17, 1, 1, 17, 1, 30, 30, 2, 19, 19, 17, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
			{0, 8, 1, 1, 1, 1, 1, 1, 17, 1, 1, 1, 1, 1, 1, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		},
		{
			{0, 0, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 2, 2, 1, 13},
			{0, 0, 2, 1, 1, 1, 1, 1, 1, 18, 18, 1, 1, 1, 6, 0},
			{0, 0, 1, 1, 1, 2, 1, 1, 1, 1, 1, 6, 1, 1, 1, 0},
			{0, 0, 1, 10, 1, 31, 1, 1, 1, 20, 1, 1, 1, 1, 6, 0},
			{0, 0, 5, 1, 1, 7, 1, 1, 1, 1, 1, 1, 14, 21, 1, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 2, 22, 23, 1, 16, 24, 10, 0},
			{0, 8, 12, 1, 1, 4, 10, 1, 20, 1, 1, 1, 1, 1, 17, 0},
			{0, 0, 17, 1, 25, 17, 1, 1, 1, 26, 27, 1, 1, 1, 1, 0},
			{0, 8, 28, 29, 17, 1, 1, 17, 1, 30, 30, 2, 19, 19, 17, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
			{0, 8, 1, 1, 1, 1, 1, 1, 17, 1, 1, 1, 1, 1, 1, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
		},
		{
			{0, 0, 8, 32, 18, 18, 1, 1, 1, 1, 1, 1, 1, 1, 33, 0},
			{0, 0, 0, 1, 2, 2, 1, 1, 34, 1, 1, 34, 1, 1, 6, 0},
			{0, 0, 0, 1, 1, 1, 1, 1, 15, 1, 1, 3, 1, 1, 1, 0},
			{0, 0, 0, 1, 1, 1, 1, 1, 3, 1, 1, 3, 1, 1, 1, 0},
			{0, 0, 0, 11, 1, 34, 20, 20, 35, 1, 1, 35, 31, 31, 34, 0},
			{0, 0, 0, 11, 1, 17, 1, 1, 5, 1, 1, 1, 1, 1, 1, 0},
			{36, 0, 0, 1, 6, 30, 1, 14, 37, 1, 1, 1, 1, 1, 1, 0},
			{1, 2, 38, 10, 1, 34, 31, 31, 35, 1, 1, 35, 31, 31, 34, 0},
			{12, 1, 1, 1, 1, 1, 1, 1, 15, 1, 1, 3, 1, 1, 6, 0},
			{4, 4, 7, 1, 1, 1, 1, 1, 15, 1, 1, 3, 1, 1, 1, 0},
			{4, 4, 1, 1, 1, 17, 1, 1, 34, 1, 1, 34, 1, 1, 30, 0},
			{2, 17, 2, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 13},
		},
		{
			{0, 0, 1, 10, 2, 1, 1, 34, 1, 1, 34, 1, 1, 6, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 3, 1, 1, 15, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 15, 1, 1, 15, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 34, 31, 31, 35, 1, 1, 35, 31, 31, 34, 0, 0},
			{0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 10, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 11, 1, 34, 31, 20, 35, 1, 1, 35, 20, 20, 34, 0, 0},
			{36, 36, 11, 1, 17, 1, 1, 15, 1, 1, 11, 7, 1, 2, 13, 0},
			{1, 1, 1, 1, 1, 1, 1, 15, 1, 1, 15, 1, 1, 1, 0, 0},
			{1, 39, 17, 1, 17, 1, 1, 34, 30, 5, 34, 1, 1, 30, 0, 0},
			{12, 2, 1, 1, 1, 1, 1, 19, 19, 10, 1, 1, 1, 1, 0, 0},
			{2, 16, 40, 16, 16, 16, 1, 1, 14, 1, 1, 1, 1, 1, 13, 0},
		},
	};
	/** Convert coordinates, sample generation values, and inspect party traits. */
	static int index(int column, int row) { return column * kRows + row; }
	int randomBelow(int count);
	int trait(int zoombini, int feature) const;
	bool matches(int zoombini, Feature feature) const;
	/** Load/reset layout cells and assign a generated trait predicate to one device. */
	void loadLayout(int layout);
	void clearCell(int column, int row);
	void setFeature(int column, int row, Feature feature);
	/** Count party features and select predicates/actors for the difficulty generator. */
	void tally(const Common::Array<int> &group, int counts[5][6]) const;
	Feature singleton(const Common::Array<int> &group) const;
	Feature popular(const Common::Array<int> &group, int limit) const;
	Feature unusedFeature(const Common::Array<int> &group);
	int pick(const Common::Array<int> &candidates, Common::Array<int> &used);
	int select(Common::Array<int> &used, Feature a, bool matchA, Feature b = Feature(), bool matchB = true, Feature c = Feature(), bool matchC = true);
	/** Generate each difficulty branch and select distinct party actors as needed. */
	int generateEasy(int layout);
	void generateMedium();
	void generateHard();
	int pickUnique(Common::Array<int> &used);
	int pickDifferent(int first, Common::Array<int> &used, int minimum);
	/** Resolve a move, remove a collision pair, and rotate triggered devices. */
	void move(int cellIndex, int direction);
	void removeCollision(int first, int second);
	void rotateCells(const bool triggers[7]);

	/** Shared generator RNG, party traits, live cells, and their initial snapshot. */
	Random *_random = nullptr;
	Common::Array<ZmbTrait> _party;
	Cell _cells[kCellCount];
	Cell _initialCells[kCellCount];
	/** Generation record and before/after occupant buffers used for simultaneous ticks. */
	GenerationInfo _generation;
	Occupant _before[kCellCount];
	Occupant _after[kCellCount];
	/** Moved, exited, lost, and collided roster indices plus their resolved moves. */
	Common::Array<int> _moved;
	Common::Array<Move> _moves;
	Common::Array<int> _exits;
	Common::Array<int> _lost;
	Common::Array<int> _collisions;
	/** Selected layout and tick-level signal flags returned to the page renderer. */
	int _layout = 0;
	bool _turned = false;
	bool _caught = false;
	bool _released = false;
};

/** Bubble Bumpers: simultaneous bubble paths through a mutable device grid. */
class PuzzleMysticMarsh : public PuzzleBase {
public:
	/** Construct Bubble Bumpers for @p vm. */
	explicit PuzzleMysticMarsh(Zoombini2Engine *vm);
	/** Release bubble paths and page-local animation runners. */
	~PuzzleMysticMarsh() override;
	/** Generate a grid, load resources, and place the selectable roster. */
	void init() override;
	/** Place a dropped Zoombini in a crater or update the held-rider pointer. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Start retreat speech or return through the map transition. */
	bool onGoButtonPressed() override;
	/** Return whether enough grid outcomes allow departure. */
	bool canUseGoButton() const override;
	/** Mark all remaining party members free for console completion. */
	void applyDebugPuzzleCompletion() override;
	/** Describe retained generation intent and a bounded current-state forecast. */
	Common::String debugGetAnswer() const override;
	PuzzleChanceInfo debugGetChances() const override { return PuzzleChanceInfo(PuzzleChanceInfo::Type::kAmorphous); }
	Common::String debugGetChanceDetails() const override;

protected:
	/** Advance grid ticks, bubble paths, effects, speech, and completion state. */
	void onUpdate() override;
	void onRenderBackground(ManagedSurface32 *screen) override;
	void onRenderContent(ManagedSurface32 *screen) override;
	void onRenderActors(ManagedSurface32 *screen) override;
	void onActorsRendered() override;
	void onRenderForeground(ManagedSurface32 *screen) override;

private:
	/** Music, layout background/mask, device symbols, trait icons, and bubble art. */
	static constexpr const char *kMusicPath = "#sounds/music/04-BS01.wav";
	static constexpr const char *kBackgroundFormat = "#bmp/mystic_marsh/background%d";
	static constexpr const char *kAreaFormat = "bmp/mystic_marsh/area%d.bmt";
	static constexpr const char *kSymbolFormat = "bmp/mystic_marsh/symbols/%s";
	static constexpr const char *kTraitFormat = "bmp/mystic_marsh/traits/%d-%d";
	static constexpr const char *kCraterPath = "bmp/mystic_marsh/crater";
	static constexpr const char *kCraterAnimationPath = "bmp/mystic_marsh/BubbleCrater";
	static constexpr const char *kWhirlpoolPath = "bmp/mystic_marsh/symbols/tourbi_anim";
	static constexpr const char *kBubblePath = "bmp/mystic_marsh/bubble1";
	static constexpr const char *kFloatPath = "bmp/zombis/flotte/flotte.anm";
	static constexpr const char *kPickupPath = "bmp/zombis/pris/pris.anm";
	static constexpr const char *kCelebratePath = "bmp/zombis/attente/attenteZomb.anm";
	/** Effects, departure speech, completion speech, and symbol resource names. */
	static constexpr const char *kSfxFormat = "sounds/fx/04-BS%02d.wav";
	static constexpr const char *kGoSpeechFormat = "sounds/WLD11.%d.wav";
	static constexpr const char *kCaveSpeech = "sounds/DW-Cave.wav";
	static constexpr const char *kCompleteSpeech = "sounds/8-E1.wav";
	static constexpr const char *kSymbolNames[60] = {
		"S_DIV1",
		"S_DIV2",
		"S_DIV3",
		"S_DIV4",
		"C_DIV1",
		"C_DIV2",
		"C_DIV3",
		"C_DIV4",
		"RD_CY_DIV2",
		"DR_CY_DIV2",
		"UD_CY_DIV2",
		"DU_CY_DIV2",
		"LR_CY_DIV2",
		"RL_CY_DIV2",
		"OT_DIV1",
		"OT_DIV2",
		"OT_DIV3",
		"OT_DIV4",
		"OLD_OT_DIV1",
		"OLD_OT_DIV2",
		"OLD_OT_DIV3",
		"OLD_OT_DIV4",
		"CC_ROTATOR",
		"C_ROTATOR",
		"LD_ELBOW",
		"LU_ELBOW",
		"LD_CONVERGER",
		"TRIGGER1",
		"TRIGGER2",
		"TRIGGER3",
		"TRIGGER4",
		"TRIGGER5",
		"TRIGGER6",
		"TRIGGER7",
		"UR_TCY_DIV2",
		"RU_TCY_DIV2",
		"RD_TCY_DIV2",
		"DR_TCY_DIV2",
		"LR_TCY_DIV2",
		"RL_TCY_DIV2",
		"LL_TCY_DIV3",
		"LU_TCY_DIV3",
		"LD_TCY_DIV3",
		"RR_TCY_DIV3",
		"RU_TCY_DIV3",
		"RD_TCY_DIV3",
		"UU_TCY_DIV3",
		"UL_TCY_DIV3",
		"UR_TCY_DIV3",
		"TS_SPOT1",
		"TS_SPOT2",
		"TS_SPOT3",
		"TS_SPOT4",
		"TS_SPOT5",
		"TS_SPOT6",
		"TS_SPOT7",
		"TOURBI",
		"EDGE",
		"ENTRY1",
		"ENTRY2",
	};
	/** Page-space starting and exit positions selected by the generated layout. */
	static constexpr Common::Point32 kStartingPositions[6][8] = {
		{Common::Point32(127, 397), Common::Point32(176, 411), Common::Point32(232, 411), Common::Point32(99, 437), Common::Point32(151, 449), Common::Point32(210, 447), Common::Point32(117, 484), Common::Point32(186, 490)},
		{Common::Point32(127, 397), Common::Point32(176, 411), Common::Point32(232, 411), Common::Point32(99, 437), Common::Point32(151, 449), Common::Point32(210, 447), Common::Point32(117, 484), Common::Point32(186, 490)},
		{Common::Point32(127, 397), Common::Point32(176, 411), Common::Point32(232, 411), Common::Point32(99, 437), Common::Point32(151, 449), Common::Point32(210, 447), Common::Point32(117, 484), Common::Point32(186, 490)},
		{Common::Point32(52, 458), Common::Point32(103, 463), Common::Point32(148, 463), Common::Point32(200, 475), Common::Point32(53, 514), Common::Point32(90, 514), Common::Point32(132, 520), Common::Point32(175, 513)},
		{Common::Point32(106, 452), Common::Point32(155, 465), Common::Point32(211, 465), Common::Point32(78, 491), Common::Point32(131, 503), Common::Point32(189, 501), Common::Point32(95, 538), Common::Point32(164, 543)},
		{Common::Point32(127, 397), Common::Point32(176, 411), Common::Point32(232, 411), Common::Point32(99, 437), Common::Point32(151, 449), Common::Point32(210, 447), Common::Point32(117, 484), Common::Point32(186, 490)},
	};
	static constexpr Common::Point32 kExitPositions[6][8] = {
		{Common::Point32(589, 10), Common::Point32(646, 10), Common::Point32(705, 12), Common::Point32(611, 58), Common::Point32(671, 59), Common::Point32(735, 52), Common::Point32(734, 105), Common::Point32(734, 172)},
		{Common::Point32(589, 10), Common::Point32(646, 10), Common::Point32(705, 12), Common::Point32(611, 58), Common::Point32(671, 59), Common::Point32(735, 52), Common::Point32(734, 105), Common::Point32(734, 172)},
		{Common::Point32(745, 135), Common::Point32(713, 97), Common::Point32(746, 69), Common::Point32(701, 46), Common::Point32(744, 14), Common::Point32(704, 1), Common::Point32(653, 1), Common::Point32(580, 1)},
		{Common::Point32(643, 545), Common::Point32(691, 545), Common::Point32(732, 545), Common::Point32(747, 513), Common::Point32(718, 479), Common::Point32(750, 465), Common::Point32(689, 452), Common::Point32(725, 433)},
		{Common::Point32(631, 44), Common::Point32(685, 61), Common::Point32(741, 91), Common::Point32(740, 152), Common::Point32(694, 195), Common::Point32(741, 218), Common::Point32(740, 280), Common::Point32(738, 331)},
		{Common::Point32(613, 7), Common::Point32(670, 7), Common::Point32(729, 5), Common::Point32(689, 52), Common::Point32(743, 49), Common::Point32(731, 95), Common::Point32(741, 141), Common::Point32(739, 186)},
	};

	struct Bubble {
		/** Visibility, current position, and path for one moving bubble. */
		bool active = false;
		Common::Point32 position;
		PathObject *path = nullptr;
	};
	/** A selectable crater and its page-space location. */
	struct Slot {
		int cell;
		Common::Point32 position;
	};
	/** One short-lived crater or whirlpool visual effect. */
	struct Effect {
		Common::Point32 position;
		uint32 start;
	};
	/** Convert a grid cell or drag/drop callback into a page placement. */
	static Common::Point32 cellPosition(int index);
	static void slotDropCallback(void *context, int slotIndex, int zoombiniIndex);
	void placeZoombini(int slotIndex, int zoombiniIndex);
	/** Create bubble path segments, resolve grid results, and free or lose riders. */
	PathObject *createPath(const Common::Point32 &from, const Common::Point32 &to, int step) const;
	void advanceGrid(uint32 now);
	void freeZoombini(int index, uint32 now);
	void loseZoombini(int index, bool whirlpool, uint32 now);
	/** Play indexed effects, serialize speech, load resources, and format console diagnostics. */
	void playSfx(int index);
	void enqueueSpeech(const Common::String &name);
	void loadResources();
	Common::String debugGroup(const char *label, const Common::Array<int> &actors) const;
	Common::String debugGeneration() const;

	/** Logical grid and its cell snapshot for the current render frame. */
	MysticMarshGrid _grid;
	MysticMarshGrid::Cell _drawCells[MysticMarshGrid::kCellCount];
	/** Crater slots, associated drop targets, moving bubbles, effects, and speech queue. */
	Common::Array<Slot> _slots;
	Common::Array<ZmbDropTarget> _dropTargets;
	Common::Array<Bubble> _bubbles;
	Common::Array<Effect> _effects;
	/** Background variant, released count, active placement, tick/lock timing, and Go state. */
	int _backgroundIndex = 1;
	int _freed = 0;
	int _placingZoombini = -1;
	int _placingSlot = -1;
	uint32 _placementStart = 0;
	uint32 _lastTick = 0;
	uint32 _unlockTime = 0;
	bool _finished = false;
	bool _goPending = false;
	/** Device/bubble visual resources, Zoombini grids, and indexed effect handles. */
	Animation *_craterAnimation = nullptr;
	Animation *_whirlpoolAnimation = nullptr;
	const ZoombiniAnimation *_floatAnimation = nullptr;
	const ZoombiniAnimation *_pickupAnimation = nullptr;
	const ZoombiniAnimation *_celebrateAnimation = nullptr;
	int _sounds[9] = {
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
	};
};

} // End of namespace Zoombini2

#endif
