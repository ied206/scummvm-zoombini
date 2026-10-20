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
#include "common/hashmap.h"
#include "common/str.h"
#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_base.h"
#include "zoombini2/scripts.h"
#include "zoombini2/state.h"

namespace Zoombini2 {

class Random;
class Gfx;

/** Rules and simultaneous bubble movement for Mystic Marsh. */
class MysticMarshGrid {
public:
	/** Authored grid layout codes grouped by difficulty; values index @ref MysticMarshGrid::kLayouts. */
	enum GridLayout : int {
		kGridL1LayoutA_00 = 0, ///< First L1 board template.
		kGridL1LayoutB_01 = 1, ///< Second L1 board template.
		kGridL2LayoutA_02 = 2, ///< First L2 board template.
		kGridL2LayoutB_03 = 3, ///< Second L2 board template.
		kGridL2LayoutC_04 = 4, ///< Third L2 board template.
		kGridL2LayoutD_05 = 5, ///< Fourth L2 board template.
		kGridL3LayoutA_06 = 6, ///< First L3 board template.
		kGridL3LayoutB_07 = 7, ///< Second L3 board template.
		kGridL4LayoutA_08 = 8  ///< Restored L4 board template.
	};

	/** One-based background and matching area-mask resource identities. */
	enum BackgroundKind : int {
		kBackgroundL1LayoutA_01 = 1, ///< L1 layout A background and area mask.
		kBackgroundL1LayoutB_02 = 2, ///< L1 layout B background and area mask.
		kBackgroundL2Layouts_03 = 3, ///< Shared background and area mask for L2 layouts A-D.
		kBackgroundL3LayoutA_04 = 4, ///< L3 layout A background and area mask.
		kBackgroundL3LayoutB_05 = 5, ///< L3 layout B background and area mask.
		kBackgroundL4LayoutA_06 = 6  ///< Restored L4 layout A; its background artwork is missing.
	};

	/**
	 * Cell codes shared by grid routing and symbol rendering.
	 * Types 2 through 61 select symbol suffixes in order.
	 * TRIGGER1-7 are star switches; TS_SPOT1-7 are magnets that hold bubbles until pushed or triggered.
	 */
	enum CellType {
		kCellEmptyType00 = 0,
		kCellFollowIncomingDirectionType01 = 1,
		kCellSymbolSDiv1Type02 = 2,
		kCellSymbolSDiv2Type03 = 3,
		kCellSymbolSDiv3Type04 = 4,
		kCellSymbolSDiv4Type05 = 5,
		kCellSymbolCDiv1Type06 = 6,
		kCellSymbolCDiv2Type07 = 7,
		kCellSymbolCDiv3Type08 = 8,
		kCellSymbolCDiv4Type09 = 9,
		kCellSymbolRdCyDiv2Type10 = 10,
		kCellSymbolDrCyDiv2Type11 = 11,
		kCellSymbolUdCyDiv2Type12 = 12,
		kCellSymbolDuCyDiv2Type13 = 13,
		kCellSymbolLrCyDiv2Type14 = 14,
		kCellSymbolRlCyDiv2Type15 = 15,
		kCellSymbolOtDiv1Type16 = 16,
		kCellSymbolOtDiv2Type17 = 17,
		kCellSymbolOtDiv3Type18 = 18,
		kCellSymbolOtDiv4Type19 = 19,
		kCellSymbolOldOtDiv1Type20 = 20,
		kCellSymbolOldOtDiv2Type21 = 21,
		kCellSymbolOldOtDiv3Type22 = 22,
		kCellSymbolOldOtDiv4Type23 = 23,
		kCellSymbolCcRotatorType24 = 24,
		kCellSymbolCRotatorType25 = 25,
		kCellSymbolLdElbowType26 = 26,
		kCellSymbolLuElbowType27 = 27,
		kCellSymbolLdConvergerType28 = 28,
		kCellSymbolTrigger1Type29 = 29,
		kCellSymbolTrigger2Type30 = 30,
		kCellSymbolTrigger3Type31 = 31,
		kCellSymbolTrigger4Type32 = 32,
		kCellSymbolTrigger5Type33 = 33,
		kCellSymbolTrigger6Type34 = 34,
		kCellSymbolTrigger7Type35 = 35,
		kCellSymbolUrTcyDiv2Type36 = 36,
		kCellSymbolRuTcyDiv2Type37 = 37,
		kCellSymbolRdTcyDiv2Type38 = 38,
		kCellSymbolDrTcyDiv2Type39 = 39,
		kCellSymbolLrTcyDiv2Type40 = 40,
		kCellSymbolRlTcyDiv2Type41 = 41,
		kCellSymbolLlTcyDiv3Type42 = 42,
		kCellSymbolLuTcyDiv3Type43 = 43,
		kCellSymbolLdTcyDiv3Type44 = 44,
		kCellSymbolRrTcyDiv3Type45 = 45,
		kCellSymbolRuTcyDiv3Type46 = 46,
		kCellSymbolRdTcyDiv3Type47 = 47,
		kCellSymbolUuTcyDiv3Type48 = 48,
		kCellSymbolUlTcyDiv3Type49 = 49,
		kCellSymbolUrTcyDiv3Type50 = 50,
		kCellSymbolTsSpot1Type51 = 51,
		kCellSymbolTsSpot2Type52 = 52,
		kCellSymbolTsSpot3Type53 = 53,
		kCellSymbolTsSpot4Type54 = 54,
		kCellSymbolTsSpot5Type55 = 55,
		kCellSymbolTsSpot6Type56 = 56,
		kCellSymbolTsSpot7Type57 = 57,
		kCellSymbolTourbiType58 = 58,
		kCellSymbolEdgeType59 = 59,
		kCellSymbolEntry1Type60 = 60,
		kCellSymbolEntry2Type61 = 61,
		kCellExitType62 = 62
	};

private:
	/** Direction codes used by cell outputs and bubble movement. */
	enum Direction {
		kLeftDirection00 = 0,
		kRightDirection01 = 1,
		kUpDirection02 = 2,
		kDownDirection03 = 3,
		kNoFixedDirection04 = 4
	};

public:
	/** Number of logical columns in each grid row. */
	static constexpr int kColumns = 16;
	/** Number of logical rows in each grid column. */
	static constexpr int kRows = 12;
	/** Number of cells in the column-major grid buffers. */
	static constexpr int kCellCount = kColumns * kRows;
	struct Cell {
		/** Device code selecting this cell's routing behavior and symbol artwork. */
		int type;
		/** Direction code used when this cell routes or retains a bubble. */
		int direction;
		/** Star cell-type code that activates this device, when it has a linked trigger. */
		int trigger;
		/** One-based trait selector used by a conditional device. */
		int trait;
		/** Trait value required by this predicate. */
		int value;
		/** Generated auxiliary cell state retained in answer-state comparisons. */
		int state;
		/** Initialize the device code, routing direction, binding and predicate fields. */
		constexpr Cell(int t = kCellEmptyType00, int d = kNoFixedDirection04, int g = 0, int f = 0, int v = 0, int s = 0)
			: type(t), direction(d), trigger(g), trait(f), value(v), state(s) {}
	};
	/** A Zoombini position before or after one simultaneous grid tick. */
	struct Occupant {
		/** Zero-based puzzle-roster index, or -1 when this cell is empty. */
		int index = -1;
		/** Direction carried by this occupant; -1 when no direction is assigned. */
		int direction = -1;
	};
	/** One resolved simultaneous move from source cell to destination cell. */
	struct Move {
		/** Zero-based puzzle-roster index of the moving Zoombini. */
		int index;
		/** Source cell index, stored as column * kRows + row. */
		int from;
		/** Destination cell index, stored as column * kRows + row. */
		int to;
		/** Record one actor's resolved source and destination cells. */
		Move(int z, int a, int b) : index(z), from(a), to(b) {}
	};
	/** Roles used by generated predicates and their reference actors. */
	enum class GenerationKeyRole {
		kRouteFilter,
		kCrossFilter,
		kSingleton,
		kCondition,
		kPrimaryReference,
		kSecondaryReference,
		kRemainingFiltersReference,
		kConstrained,
		kPair,
	};
	struct GenerationKey {
		/** Purpose served by this actor in the retained generation construction. */
		GenerationKeyRole role;
		/** Zero-based index into the puzzle's Zoombini roster. */
		int actor;
		/** Grid cell index, stored as column * kRows + row. */
		int cell;
		/** Retain one generator role and its actor/cell association. */
		GenerationKey(GenerationKeyRole r, int a, int c) : role(r), actor(a), cell(c) {}
	};
	/** Construction records retained with the selected board, independently of live device changes. */
	struct GenerationInfo {
		/** Actor, cell and role records from the selected generation attempt. */
		Common::Array<GenerationKey> keys;
		/** Candidate-selection records retained from the successful generation branch. */
		Common::Array<int> selectionOrder;
		/** Roster indices assigned to the upper route during L1 board generation. */
		Common::Array<int> upper;
		/** Roster indices assigned to the lower route during L1 board generation. */
		Common::Array<int> lower;
		/** Roster indices retained as the L2 board generator's selected group. */
		Common::Array<int> group;
		/** Selected subgroup of the L2 board generator's actor group. */
		Common::Array<int> subset;
		/** Roster selection order retained from the L4 board generator. */
		Common::Array<int> l4Order;
		/** L4 primary, secondary and pair predicates, each stored as trait/value. */
		int l4Features[3][2] = {};
		/** Roster indices used as the two reference actors during L3 board generation. */
		int referenceActors[2] = {
			-1,
			-1,
		};
		/** Roster indices used to establish the two primary L3 board predicates. */
		int primaryFilterActors[2] = {
			-1,
			-1,
		};
		/** Whether retry recovery restored the selected filter values. */
		bool restoredFilterValues = false;
	};
	/** Return retained generation data and the immutable initial device layout. */
	const GenerationInfo &generationInfo() const { return _generation; }
	/** Return the immutable initial device state at a column-major cell index. */
	const Cell &initialCell(int cellIndex) const { return _initialCells[cellIndex]; }
	/** Generate rules using the party and the shared random stream. */
	void init(const Common::Array<ZmbTrait> &party, int level, Random &random);
	/** Place one roster entry in a crater. */
	bool place(int cellIndex, int zoombiniIndex);
	/** Advance one complete grid step and replace the result queues. */
	void tick();
	/** Return the mutable device state at a column-major cell index. */
	const Cell &cell(int index) const { return _cells[index]; }
	/** Return the selected grid layout, including the L4 template. */
	GridLayout layout() const { return _layout; }
	/** Return the background resource index corresponding to the selected layout. */
	BackgroundKind background() const;
	/** Return source/destination movements produced by the last simultaneous grid tick. */
	const Common::Array<Move> &moves() const { return _moves; }
	/** Return roster indices that reached an exit during the last grid tick. */
	const Common::Array<int> &exits() const { return _exits; }
	/** Return roster indices lost to whirlpools during the last grid tick. */
	const Common::Array<int> &lost() const { return _lost; }
	/** Return roster indices removed by collisions during the last grid tick. */
	const Common::Array<int> &collisions() const { return _collisions; }
	/** Return whether the last grid tick rotated a device. */
	bool turned() const { return _turned; }
	/** Return whether the last grid tick captured a rider on a magnet. */
	bool caught() const { return _caught; }
	/** Return whether the last grid tick produced a held-bubble release signal. */
	bool released() const { return _released; }
	/** A waiting bubble at the end of a verified launch. */
	struct AnswerStop {
		/** Zero-based index into the puzzle's Zoombini roster. */
		int actor;
		/** Column-major cell index where this actor remains after the launch settles. */
		int cell;
		/** Record the actor and cell retained at a stationary launch boundary. */
		AnswerStop(int a, int c) : actor(a), cell(c) {}
	};
	/** One launch from the initial board; wait for all bubbles to stop before the next launch. */
	struct AnswerLaunch {
		/** Zero-based index into the puzzle's Zoombini roster. */
		int actor;
		/** Column-major crater cell index from which this actor is launched. */
		int cell;
		/** Distinct star cell indices visited by this launch's chain, in first-visit order. */
		Common::Array<int> stars;
		/** Distinct magnet cell indices that released a rider during this launch, in first-release order. */
		Common::Array<int> releasedMagnets;
		/** Actor/cell pairs still on the grid after this launch's movement chain settles. */
		Common::Array<AnswerStop> waiting;
		/** Number of exit arrivals produced by this launch and its resulting chain. */
		int crossed = 0;
		/** Begin a launch record with empty event lists and no exit arrivals. */
		AnswerLaunch(int a, int c) : actor(a), cell(c) {}
	};
	/** Find and replay-check an initial-board answer without consuming random numbers or changing live state. */
	bool debugAnswerSolver(Common::Array<AnswerLaunch> &answer, bool &fromGeneration) const;

private:
	/** Return whether @p type identifies one of the seven numbered star switches. */
	static bool isStarType(int type);
	/** Return whether @p type identifies one of the seven numbered holding magnets. */
	static bool isMagnetType(int type);
	/** Search only launches separated by stationary grid states, with a bounded tick budget. */
	bool searchAnswer(uint used, uint exited, const Common::Array<int> &order, const Common::Array<int> &candidates, Common::Array<AnswerLaunch> &answer, Common::HashMap<Common::String, bool> &visited, int &budget) const;
	/** Return answer candidates in generation-aware order, deferring reference actors when needed. */
	Common::Array<int> answerActorOrder() const;
	/** Encode device states, occupants and used/exited actor masks for answer-search deduplication. */
	Common::String answerStateKey(uint used, uint exited) const;
	/** Return whether two nonzero roster indices behave identically at the initial board's trait filters. */
	bool equivalentAnswerActors(int first, int second) const;
	/** Append all 32 bits of an integer as a fixed-length alphabetic state-key component. */
	static void appendAnswerValue(Common::String &key, int value);
	/** Replay until stationary within the tick budget; update exits and optional events, rejecting losses and collisions. */
	bool settleAnswer(uint &exited, int &budget, AnswerLaunch *explanation = nullptr);
	/** Compare mutable devices and occupants to detect a stationary answer-replay state. */
	bool sameAnswerState(const MysticMarshGrid &other) const;
	/** Restore the initial board and empty occupants for isolated answer simulation. */
	void resetAnswerState();
	/** Trait/value predicate used while selecting generator actor groups. */
	struct Feature {
		/** One-based trait selector for this generator predicate. */
		int trait;
		/** Trait value required by this predicate. */
		int value;
		/** Initialize a trait/value predicate, or an unset predicate by default. */
		Feature(int t = 0, int v = 0) : trait(t), value(v) {}
	};
	/** Device templates referenced by the authored layout tables. */
	static const Cell *cellTemplates() {
		static constexpr Cell kCellTemplates[] = {
			{kCellEmptyType00, kLeftDirection00, 0, 0, 0, 0},
			{kCellFollowIncomingDirectionType01, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolSDiv2Type03, kRightDirection01, 0, 0, 0, 0},
			{kCellSymbolCDiv4Type09, kDownDirection03, 0, 0, 0, 0},
			{kCellSymbolTsSpot2Type52, kNoFixedDirection04, kCellSymbolTrigger2Type30, 0, 0, 0},
			{kCellSymbolTrigger1Type29, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolSDiv4Type05, kDownDirection03, 0, 0, 0, 0},
			{kCellSymbolTrigger2Type30, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolEntry1Type60, kRightDirection01, 0, 0, 0, 0},
			{kCellSymbolUdCyDiv2Type12, kUpDirection02, 0, 0, 0, 0},
			{kCellSymbolTourbiType58, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolOtDiv2Type17, kRightDirection01, 0, 0, 0, 0},
			{kCellSymbolTsSpot1Type51, kNoFixedDirection04, kCellSymbolTrigger1Type29, 0, 0, 0},
			{kCellExitType62, kLeftDirection00, 0, 0, 0, 0},
			{kCellSymbolTrigger3Type31, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolCDiv3Type08, kUpDirection02, 0, 0, 0, 0},
			{kCellSymbolTsSpot3Type53, kNoFixedDirection04, kCellSymbolTrigger3Type31, 0, 0, 0},
			{kCellSymbolSDiv3Type04, kUpDirection02, 0, 0, 0, 0},
			{kCellSymbolOtDiv4Type19, kDownDirection03, 0, 0, 0, 0},
			{kCellSymbolOtDiv3Type18, kUpDirection02, 0, 0, 0, 0},
			{kCellSymbolCDiv2Type07, kRightDirection01, 0, 0, 0, 0},
			{kCellSymbolTrigger4Type32, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolTsSpot5Type55, kNoFixedDirection04, kCellSymbolTrigger5Type33, 0, 0, 0},
			{kCellSymbolTsSpot6Type56, kNoFixedDirection04, kCellSymbolTrigger6Type34, 0, 0, 0},
			{kCellSymbolTsSpot4Type54, kNoFixedDirection04, kCellSymbolTrigger4Type32, 0, 0, 0},
			{kCellSymbolLrCyDiv2Type14, kLeftDirection00, 0, 0, 0, 0},
			{kCellSymbolTrigger5Type33, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolTrigger6Type34, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolTsSpot7Type57, kNoFixedDirection04, kCellSymbolTrigger7Type35, 0, 0, 0},
			{kCellSymbolTrigger7Type35, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolSDiv1Type02, kLeftDirection00, 0, 0, 0, 0},
			{kCellSymbolCDiv1Type06, kLeftDirection00, 0, 0, 0, 0},
			{kCellSymbolDrCyDiv2Type11, kDownDirection03, 0, 0, 0, 0},
			{kCellSymbolTourbiType58, kLeftDirection00, 0, 0, 0, 0},
			{kCellSymbolCcRotatorType24, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolCRotatorType25, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolEntry2Type61, kDownDirection03, 0, 0, 0, 0},
			{kCellSymbolOtDiv1Type16, kLeftDirection00, 0, 0, 0, 0},
			{kCellSymbolRdTcyDiv2Type38, kRightDirection01, kCellSymbolTrigger3Type31, 0, 0, 0},
			{kCellSymbolRdCyDiv2Type10, kRightDirection01, 0, 0, 0, 0},
			{kCellSymbolUrTcyDiv2Type36, kUpDirection02, kCellSymbolTrigger2Type30, 0, 0, 0},
			{kCellSymbolLuElbowType27, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolTsSpot2Type52, kNoFixedDirection04, kCellSymbolTrigger1Type29, 0, 0, 0},
			{kCellSymbolLdConvergerType28, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolRdTcyDiv3Type47, kNoFixedDirection04, kCellSymbolTrigger5Type33, 0, 0, 0},
			{kCellSymbolLlTcyDiv3Type42, kLeftDirection00, kCellSymbolTrigger1Type29, 0, 0, 0},
			{kCellSymbolLrTcyDiv2Type40, kLeftDirection00, kCellSymbolTrigger6Type34, 0, 0, 0},
			{kCellSymbolLdElbowType26, kNoFixedDirection04, 0, 0, 0, 0},
			{kCellSymbolCDiv2Type07, kNoFixedDirection04, 1, 0, 0, 0},
		};
		return kCellTemplates;
	}
	/** Template indices for each authored layout, indexed by layout, row and column. */
	static constexpr byte kLayouts[9][kRows][kColumns] = {
		{
			// kGridL1LayoutA_00
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
			// kGridL1LayoutB_01
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
			// kGridL2LayoutA_02
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
			// kGridL2LayoutB_03
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
			// kGridL2LayoutC_04
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
			// kGridL2LayoutD_05
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
			// kGridL3LayoutA_06
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
			// kGridL3LayoutB_07
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
		{
			// kGridL4LayoutA_08
			{0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 0, 1, 41, 1, 42, 1, 31, 14, 1, 34, 1, 14, 13, 0},
			{0, 0, 0, 1, 5, 1, 1, 1, 1, 1, 1, 16, 1, 1, 0, 0},
			{0, 0, 0, 1, 24, 6, 1, 1, 1, 1, 18, 31, 1, 10, 0, 0},
			{0, 0, 0, 1, 43, 30, 1, 1, 1, 1, 1, 29, 1, 1, 0, 0},
			{0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 44, 13, 0},
			{0, 0, 0, 1, 21, 1, 1, 2, 1, 1, 1, 43, 1, 1, 0, 0},
			{0, 0, 8, 1, 20, 1, 1, 17, 45, 1, 46, 28, 1, 10, 0, 0},
			{0, 0, 0, 1, 22, 17, 1, 1, 30, 1, 1, 27, 1, 1, 0, 0},
			{0, 0, 0, 1, 47, 1, 26, 48, 1, 23, 1, 41, 1, 1, 0, 0},
			{0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
			{0, 0, 8, 1, 1, 1, 1, 1, 1, 1, 17, 1, 1, 1, 0, 0},
		},
	};
	/** Return the zero-based cell-array index for a column and row, in column-major order. */
	static int toCellIndex(int column, int row) { return column * kRows + row; }
	/** Draw a generation value from zero up to, but excluding, @p count. */
	int randomBelow(int count);
	/** Return a party member's trait value for a 1-based feature selector, or zero for invalid input. */
	int trait(int zoombini, int feature) const;
	/** Test whether a party member satisfies the selected trait/value predicate. */
	bool matches(int zoombini, Feature feature) const;
	/** Replace the board's device states with the selected authored layout. */
	void loadLayout(GridLayout layout);
	/** Remove the device at the supplied grid column and row. */
	void clearCell(int column, int row);
	/** Assign a trait/value predicate to the device at the supplied grid column and row. */
	void setFeature(int column, int row, Feature feature);
	/** Record one valid party actor with its generation role and associated cell. */
	void recordKeyActor(GenerationKeyRole role, int actor, int cell);
	/** Record all party actors matching the generated trait predicate at the specified cell. */
	void recordKeyMatches(GenerationKeyRole role, int column, int row);
	/** Count trait/value occurrences among the supplied roster indices. */
	void tally(const Common::Array<int> &group, int counts[5][6]) const;
	/** Return the last trait/value found exactly once in the group, or an unset predicate. */
	Feature singleton(const Common::Array<int> &group) const;
	/** Return the most frequent group trait/value whose count is below @p limit, or an unset predicate. */
	Feature popular(const Common::Array<int> &group, int limit) const;
	/** Randomly select a party trait/value absent from the group, or return an unset predicate. */
	Feature unusedFeature(const Common::Array<int> &group);
	/** Select and mark an unused candidate roster index, or return -1 when none remain. */
	int pick(const Common::Array<int> &candidates, Common::Array<int> &used);
	/** Select and mark an unused actor satisfying the requested matches or exclusions, or return -1. */
	int select(Common::Array<int> &used, Feature a, bool matchA, Feature b = Feature(), bool matchB = true, Feature c = Feature(), bool matchC = true);
	/** Select one of the two L1 layouts and generate its board, retaining the first score-12 candidate or the best of 30 attempts. */
	void generateL1Board();
	/** Generate the L2 board, selecting one of its four layouts after deriving the party's trait predicates. */
	void generateL2Board();
	/** Generate the L3 board, selecting one of its two layouts after choosing reference actors and trait values. */
	void generateL3Board();
	/** Generate the L4 board on its fixed layout using the current party. */
	void generateL4Board();
	/** Populate one L1 board candidate on @p layout and return its generation score; 12 accepts the candidate. */
	int generateL1BoardCandidate(GridLayout layout);
	/** Correct the L4 conditional arrow after generation, before retaining the initial board. */
	void correctL4Arrow();
	/** Sample an actor and predicate, preferring a trait/value held by only one party member. */
	int pickL4Singleton(Feature &feature);
	/** Select a predicate excluding the reference actor; return true when it matches two actors instead of the singleton fallback. */
	bool pickL4Constrained(int excludedActor, Feature &feature);
	/** Select a predicate disjoint from the supplied predicates, preferring a pair of actors over one actor. */
	Feature pickL4Pair(Feature primary, Feature secondary, int excludedActor);
	/** Sample and mark an unused actor with a unique complete trait tuple, or return -1. */
	int pickUnique(Common::Array<int> &used);
	/** Select and mark an unused actor differing from @p first in at least @p minimum traits, or return -1. */
	int pickDifferent(int first, Common::Array<int> &used, int minimum);
	/** Resolve one occupant's attempted movement from the supplied cell and direction. */
	void move(int cellIndex, int direction);
	/** Remove the two colliding occupants and record their roster indices in the collision results. */
	void removeCollision(int first, int second);
	/** Rotate devices whose numbered star trigger is active for this grid tick. */
	void rotateCells(const bool triggers[7]);

	/** Borrowed generator RNG; answer replay does not consume it. */
	Random *_random = nullptr;
	/** Traits indexed by the puzzle roster indices carried by occupants. */
	Common::Array<ZmbTrait> _party;
	/** Mutable device states indexed by column * kRows + row. */
	Cell _cells[kCellCount];
	/** Device layout retained before play for answer construction and replay. */
	Cell _initialCells[kCellCount];
	/** Construction records from the selected generation attempt. */
	GenerationInfo _generation;
	/** Whether moves are being replayed for an answer without updating live presentation state. */
	bool _answerSimulation = false;
	/** Occupants before the current simultaneous grid step, indexed by cell. */
	Occupant _before[kCellCount];
	/** Resolved occupants after the current simultaneous grid step, indexed by cell. */
	Occupant _after[kCellCount];
	/** Per-roster movement flags for the current simultaneous grid step. */
	Common::Array<int> _moved;
	/** Resolved source/destination movements produced by the current grid step. */
	Common::Array<Move> _moves;
	/** Roster indices that reached exit cells during the current grid step. */
	Common::Array<int> _exits;
	/** Roster indices lost to whirlpools during the current grid step. */
	Common::Array<int> _lost;
	/** Roster indices removed by bubble collisions during the current grid step. */
	Common::Array<int> _collisions;
	/** Selected authored grid layout, including the L4 template. */
	GridLayout _layout = kGridL1LayoutA_00;
	/** Whether any device changed direction during the current grid step. */
	bool _turned = false;
	/** Whether a magnet captured a new occupant during the current grid step. */
	bool _caught = false;
	/** Release signal produced by the held-bubble processing passes. */
	bool _released = false;
};

/**
 * Draw L4 background artwork and cell outlines into a background surface.
 *
 * Background resource for MysticMarsh L4 is deleted form the final release.
 * - The maze layout logic is intact, so we derive a maze grid from it.
 * - Compose a hypothetical background for L4 based on a maze grid.
 */
class MysticMarshL4Background {
public:
	/** Column-major cell index of the restored L4 top-row reject marker and its outline. */
	static constexpr int kRejectVisualCellIdx = 8 * MysticMarshGrid::kRows;
	/**
	 * Reconstruct L4 background from a L1 layout resource.
	 * Retain shared grid lighting, repair obsolete edges, and light the new outlines.
	 *
	 * @param[in,out] surface Background surface receiving the restored artwork.
	 * @param[in] cells Generated grid cells used to select visible outlines.
	 * @param[in] gfx Graphics interface used to draw the outlines as one AA line batch.
	 */
	static void compose(ManagedSurface32 *surface, const MysticMarshGrid::Cell *cells, const Gfx &gfx);
	/** Return the page-pixel anchor for a column-major grid cell index. */
	static Common::Point32 cellPosition(int index);

private:
	/** Warm grid-halo color shared by removal estimates and newly lit edges. */
	static constexpr RGBColor kGlowColor = RGBColor(255, 232, 163);
	/** Largest supported grid-mask radius in pixels, used by the halo kernels in this background pass. */
	static constexpr int kMaxGridSpreadRadius = 14;
	/** Find the connected bright L1 grid in the complete source artwork; return false when its seed is absent. */
	static bool findBakedGrid(const ManagedSurface32 *surface, Common::Array<byte> &mask);
	/** Replace only baked opaque strokes with adjacent unmasked colors before drawing the single final grid core. */
	static void removeBakedGridCore(ManagedSurface32 *surface, const Common::Array<byte> &bakedGrid);
	/** Mark water while excluding shore-colored terrain connected to the image boundary. */
	static void findWaterMask(const ManagedSurface32 *surface, Common::Array<byte> &water);
	/**
	 * Expand a screen-sized mask by maximum coverage, using a solid disk or a soft grid halo of @p radius pixels.
	 * @pre @p radius is nonnegative and does not exceed @ref MysticMarshL4Background::kMaxGridSpreadRadius.
	 */
	static void spreadGridMask(const Common::Array<byte> &mask, Common::Array<byte> &spread, int width, int height, int radius, bool glow);
	/** Repair only water in @p repair, using water-only center samples and relaxation that never crosses @p water boundaries. */
	static void repairUncoveredGrid(ManagedSurface32 *surface, const Common::Array<byte> &repair, const Common::Array<byte> &bakedGrid, const Common::Array<byte> &bakedGlow, const Common::Array<byte> &water);
	/** Add only the missing halo contribution; repaired pixels no longer contain a baked halo. */
	static void applyGridGlow(ManagedSurface32 *surface, const Common::Array<byte> &bakedGlow, const Common::Array<byte> &gridGlow, const Common::Array<byte> &repair);
	/** Return whether this level 4 grid cell is assigned a visible outline. */
	static bool isCellBorderVisible(int column, int row);
	/** Return whether the device type is a grid entry or exit portal. */
	static bool isPortalCellType(int type);
	/**
	 * Bake white outlines for the restored L4 layout using the common two-pixel square pen.
	 *
	 * @param[in,out] surface Background surface receiving the composited outlines.
	 * @param[in] cells Generated grid cells used to select visible outlines.
	 * @param[in] gfx Graphics interface that merges line coverage and composites once.
	 */
	static void drawGrid(ManagedSurface32 *surface, const MysticMarshGrid::Cell *cells, const Gfx &gfx);
};

/** Bubble Bumpers: simultaneous bubble paths through a mutable device grid. */
class PuzzleMysticMarsh : public PuzzleBase {
public:
	/** Construct Bubble Bumpers for @p vm. */
	explicit PuzzleMysticMarsh(Zoombini2Engine *vm);
	/** Release bubble paths and page-local animation runners. */
	~PuzzleMysticMarsh() override;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	/** Return the 1-based route containing this puzzle. */
	int getRouteNumber() const override { return kRouteNumber; }
	/** Return the 1-based puzzle number within its route. */
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }
	/** Generate a grid, load resources, and place the selectable roster. */
	void init() override;
	/** Place a dropped Zoombini in a crater or update the held-rider pointer. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Update the held Zoombini position and crater drop-target hover state. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Start retreat speech or return through the map transition. */
	bool onGoButtonPressed() override;
	/** Return whether enough grid outcomes allow departure. */
	bool canUseGoButton() const override;
	/** Explain the release-chain strategy and a verified launch sequence for the initial board. */
	Common::String debugGetAnswer() const override;
	/** Report that Bubble Bumpers has no numeric chance counter. */
	PuzzleChanceInfo debugGetChances() const override { return PuzzleChanceInfo(PuzzleChanceInfo::Type::kAmorphous); }
	/** Explain that Bubble Bumpers outcomes do not consume a numeric chance counter. */
	Common::String debugGetChanceDetails() const override;

protected:
	/** Advance grid ticks, bubble paths, effects, speech, and completion state. */
	void onUpdate() override;
	/** Restore the selected background, including the baked restored L4 artwork and grid. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Clear uncovered pixels when restored L4 background artwork is incomplete. */
	bool needsScreenClear() const override { return _bgKind == MysticMarshGrid::kBackgroundL4LayoutA_06; }
	/** Draw grid devices, matching symbols and short-lived crater or whirlpool effects. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Draw the puzzle roster in its current waiting or travelling states. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Advance each Zoombini's animation after its current frame has been drawn. */
	void onActorsRendered() override;
	/** Draw active bubble shells over their riders. */
	void onRenderForeground(ManagedSurface32 *screen) override;

private:
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = "Bubble Bumpers";
	/** 1-based route number used by the puzzle console. */
	static constexpr int kRouteNumber = 3;
	/** 1-based puzzle number within the route used by the puzzle console. */
	static constexpr int kRoutePuzzleIdx = 1;
	/** Initial Bubble Bumpers background, replaced after the generated layout selects its board. */
	static constexpr const char *kBackgroundPath = "#bmp/mystic_marsh/background1";
	/** Looping Bubble Bumpers soundtrack. */
	static constexpr const char *kMusicPath = "#sounds/music/04-BS01.wav";
	/** Path format for the board backdrop selected by the generated layout. */
	static constexpr const char *kBackgroundFormat = "#bmp/mystic_marsh/background%d";
	/** Existing backdrop used when the L4 layout selects missing background 6. */
	static constexpr MysticMarshGrid::BackgroundKind kL4FallbackBackgroundIndex = MysticMarshGrid::kBackgroundL1LayoutA_01;
	/** Displayed whirlpool symbol type; the marked cell retains its logical type. */
	static constexpr int kRejectCellType = MysticMarshGrid::kCellSymbolTourbiType58;
	/** Path format for the drop-area mask corresponding to the generated background index. */
	static constexpr const char *kAreaFormat = "bmp/mystic_marsh/area%d.bmt";
	/** Path format for the RLE device and tile symbols selected by grid-cell type. */
	static constexpr const char *kSymbolFormat = "bmp/mystic_marsh/symbols/%s";
	/** Path format for RLE icons identifying a numbered Zoombini feature and value. */
	static constexpr const char *kTraitFormat = "bmp/mystic_marsh/traits/%d-%d";
	/** RLE crater artwork drawn beneath Zoombinis entering the bubble grid. */
	static constexpr const char *kCraterPath = "bmp/mystic_marsh/crater";
	/** Animation played over a crater while a Zoombini enters the grid. */
	static constexpr const char *kCraterAnimationPath = "bmp/mystic_marsh/BubbleCrater";
	/** Animation over a whirlpool when a bubble and its rider are lost. */
	static constexpr const char *kWhirlpoolPath = "bmp/mystic_marsh/symbols/tourbi_anim";
	/** RLE bubble sprite drawn along each active movement path. */
	static constexpr const char *kBubblePath = "bmp/mystic_marsh/bubble1";
	/** Path format for launch, collision, turn, catch, release, and whirlpool sound effects. */
	static constexpr const char *kSfxFormat = "sounds/fx/04-BS%02d.wav";
	/** Path format for one of five randomized perfect-completion praise lines. */
	static constexpr const char *kGoSpeechFormat = "sounds/WLD11.%d.wav";
	/** Retreat speech played before leaving with four or more Zoombinis still in the roster. */
	static constexpr const char *kCaveSpeech = "sounds/DW-Cave.wav";
	/** Completion speech queued after the party has escaped the marsh. */
	static constexpr const char *kCompleteSpeech = "sounds/8-E1.wav";
	/** Suffixes used by the 60 cell-type RLE symbols loaded from the symbols directory. */
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
	/** Page-space exit positions in pixels, indexed by background variant and party slot. */
	static constexpr Common::Point32 kExitPositions[6][8] = {
		{Common::Point32(589, 10), Common::Point32(646, 10), Common::Point32(705, 12), Common::Point32(611, 58), Common::Point32(671, 59), Common::Point32(735, 52), Common::Point32(734, 105), Common::Point32(734, 172)},
		{Common::Point32(589, 10), Common::Point32(646, 10), Common::Point32(705, 12), Common::Point32(611, 58), Common::Point32(671, 59), Common::Point32(735, 52), Common::Point32(734, 105), Common::Point32(734, 172)},
		{Common::Point32(745, 135), Common::Point32(713, 97), Common::Point32(746, 69), Common::Point32(701, 46), Common::Point32(744, 14), Common::Point32(704, 1), Common::Point32(653, 1), Common::Point32(580, 1)},
		{Common::Point32(643, 545), Common::Point32(691, 545), Common::Point32(732, 545), Common::Point32(747, 513), Common::Point32(718, 479), Common::Point32(750, 465), Common::Point32(689, 452), Common::Point32(725, 433)},
		{Common::Point32(631, 44), Common::Point32(685, 61), Common::Point32(741, 91), Common::Point32(740, 152), Common::Point32(694, 195), Common::Point32(741, 218), Common::Point32(740, 280), Common::Point32(738, 331)},
		{Common::Point32(613, 7), Common::Point32(670, 7), Common::Point32(729, 5), Common::Point32(689, 52), Common::Point32(743, 49), Common::Point32(731, 95), Common::Point32(741, 141), Common::Point32(739, 186)},
	};

	struct Bubble {
		/** Whether this bubble currently has an active visual representation. */
		bool active = false;
		/** Anchor position in unscrolled page pixels. */
		Common::Point32 position;
		/** Bubble movement path released on replacement or page teardown; nullptr while stationary. */
		PathObject *path = nullptr;
	};
	/** A selectable crater and its page-space location. */
	struct Slot {
		/** Column-major grid cell index of this selectable crater. */
		int cell;
		/** Anchor position in unscrolled page pixels. */
		Common::Point32 position;
	};
	/** One short-lived crater or whirlpool visual effect. */
	struct Effect {
		/** Anchor position in unscrolled page pixels. */
		Common::Point32 position;
		/** Gameplay tick in milliseconds when this visual effect began. */
		uint32 start;
	};
	/**
	 * Upper-bank rectangles above the shared lower bank, grouped by background.
	 * Used for optional L1-L3 area-mask expansion.
	 * Restored L4 always uses its rectangles to generate the base mask, independently of the option.
	 * Rectangles leave the sidebar, grid and crater bounds clear.
	 * Each background has 15 slots. When fewer rectangles are specified, the remaining slots
	 * default to empty rectangles and are skipped by initAreaMaskExpansion().
	 */
	static constexpr Common::Rect32 kAreaMaskExpansionUpperBankRects[6][15] = {
		{
			// L1 layout A, background 1.
			Common::Rect32(Common::Point32(0, 234), 72, 25),
			Common::Rect32(Common::Point32(0, 259), 144, 29),
			Common::Rect32(Common::Point32(0, 288), 152, 23),
			Common::Rect32(Common::Point32(0, 311), 160, 31),
			Common::Rect32(Common::Point32(0, 342), 176, 31),
			Common::Rect32(Common::Point32(0, 373), 184, 31),
			Common::Rect32(Common::Point32(0, 404), 192, 31),
			Common::Rect32(Common::Point32(0, 435), 264, 14),
			Common::Rect32(Common::Point32(0, 449), 272, 1),
			Common::Rect32(Common::Point32(0, 450), 280, 5),
		},
		{
			// L1 layout B, background 2.
			Common::Rect32(Common::Point32(0, 234), 72, 25),
			Common::Rect32(Common::Point32(0, 259), 136, 23),
			Common::Rect32(Common::Point32(0, 282), 152, 29),
			Common::Rect32(Common::Point32(0, 311), 160, 31),
			Common::Rect32(Common::Point32(0, 342), 176, 31),
			Common::Rect32(Common::Point32(0, 373), 184, 31),
			Common::Rect32(Common::Point32(0, 404), 192, 31),
			Common::Rect32(Common::Point32(0, 435), 280, 20),
		},
		{
			// L2 layouts A-D, background 3.
			Common::Rect32(Common::Point32(0, 238), 80, 52),
			Common::Rect32(Common::Point32(0, 290), 128, 10),
			Common::Rect32(Common::Point32(0, 300), 104, 52),
			Common::Rect32(Common::Point32(0, 352), 144, 10),
			Common::Rect32(Common::Point32(0, 362), 120, 52),
			Common::Rect32(Common::Point32(0, 414), 160, 2),
			Common::Rect32(Common::Point32(0, 416), 192, 19),
			Common::Rect32(Common::Point32(0, 435), 264, 14),
			Common::Rect32(Common::Point32(0, 449), 272, 1),
			Common::Rect32(Common::Point32(0, 450), 280, 5),
		},
		{
			// L3 layout A, background 4.
			// Extend the upper-left bank, narrowing beside the upper crater.
			Common::Rect32(Common::Point32(0, 0), 128, 50),
			Common::Rect32(Common::Point32(0, 50), 64, 56),
			Common::Rect32(Common::Point32(0, 106), 128, 132),
			Common::Rect32(Common::Point32(0, 238), 40, 52),
			Common::Rect32(Common::Point32(104, 238), 24, 36),
			Common::Rect32(Common::Point32(0, 290), 64, 21),
			Common::Rect32(Common::Point32(0, 311), 72, 51),
			Common::Rect32(Common::Point32(0, 362), 80, 28),
			Common::Rect32(Common::Point32(0, 390), 96, 14),
			Common::Rect32(Common::Point32(0, 404), 104, 34),
			Common::Rect32(Common::Point32(0, 438), 136, 6),
			Common::Rect32(Common::Point32(0, 444), 200, 3),
			Common::Rect32(Common::Point32(0, 447), 240, 2),
			Common::Rect32(Common::Point32(0, 449), 272, 1),
			Common::Rect32(Common::Point32(0, 450), 280, 5),
		},
		{
			// L3 layout B, background 5.
			Common::Rect32(Common::Point32(0, 274), 48, 47),
			Common::Rect32(Common::Point32(0, 321), 72, 41),
			Common::Rect32(Common::Point32(0, 362), 80, 28),
			Common::Rect32(Common::Point32(0, 390), 96, 14),
			Common::Rect32(Common::Point32(0, 404), 104, 31),
			Common::Rect32(Common::Point32(0, 435), 136, 8),
			Common::Rect32(Common::Point32(0, 443), 144, 1),
			Common::Rect32(Common::Point32(0, 444), 200, 3),
			Common::Rect32(Common::Point32(0, 447), 240, 2),
			Common::Rect32(Common::Point32(0, 449), 272, 1),
			Common::Rect32(Common::Point32(0, 450), 280, 5),
		},
		{
			// Restored L4 layout A, background 6.
			Common::Rect32(Common::Point32(0, 242), 136, 79),
			Common::Rect32(Common::Point32(0, 321), 160, 1),
			Common::Rect32(Common::Point32(0, 322), 200, 32),
			Common::Rect32(Common::Point32(0, 354), 216, 19),
			Common::Rect32(Common::Point32(0, 373), 224, 20),
			Common::Rect32(Common::Point32(0, 393), 176, 52),
			Common::Rect32(Common::Point32(248, 437), 16, 8),
			Common::Rect32(Common::Point32(0, 445), 264, 4),
			Common::Rect32(Common::Point32(0, 449), 272, 1),
			Common::Rect32(Common::Point32(0, 450), 280, 5),
		},
	};
	/** Lower-bank rectangles shared by optional L1-L3 expansion and the always-generated restored L4 mask, leaving the sidebar clear. */
	static constexpr Common::Rect32 kAreaMaskExpansionLowerBankRects[12] = {
		Common::Rect32(Common::Point32(0, 455), 312, 17),
		Common::Rect32(Common::Point32(48, 472), 264, 13),
		Common::Rect32(Common::Point32(48, 485), 280, 4),
		Common::Rect32(Common::Point32(48, 489), 288, 2),
		Common::Rect32(Common::Point32(48, 491), 296, 5),
		Common::Rect32(Common::Point32(48, 496), 312, 6),
		Common::Rect32(Common::Point32(48, 502), 320, 6),
		Common::Rect32(Common::Point32(48, 508), 312, 6),
		Common::Rect32(Common::Point32(48, 514), 296, 4),
		Common::Rect32(Common::Point32(48, 518), 288, 3),
		Common::Rect32(Common::Point32(48, 521), 264, 6),
		Common::Rect32(Common::Point32(48, 527), 240, 53),
	};
	/** Select the allowed area-mask rectangles and generate the always-enabled restored L4 base mask. */
	void initAreaMaskExpansion();
	/** Enable or clear additional L1-L3 area-mask rectangles when the option changes; restored L4 is unaffected. */
	void refreshAreaMaskExpansion();
	/** Page-pixel rectangles added to the loaded L1-L3 mask when expansion is enabled, or used as the restored L4 base mask. */
	Common::Array<Common::Rect32> _areaMaskExpansionRects;
	/** Whether the area mask is expanded by L1-L3 additions or the always-enabled restored L4 base mask. */
	bool _areaMaskExpanded = false;
	/** Forward a crater drop from the slot manager to this puzzle instance. */
	static void slotDropCallback(void *context, int slotIndex, int zoombiniIndex);
	/** Launch the selected roster actor from the crater assigned to the drop slot. */
	void placeZoombini(int slotIndex, int zoombiniIndex);
	/** Allocate a bubble path between page-pixel anchors using the supplied raw Q10 step. */
	PathObject *createPath(const Common::Point32 &from, const Common::Point32 &to, int step) const;
	/** Resolve one grid tick and start the resulting bubble paths, exit arrivals and losses. */
	void advanceGrid(uint32 now);
	/** Move an exiting rider into the successful party and start its exit animation. */
	void freeZoombini(int index, uint32 now);
	/** Remove a rider after a whirlpool or collision and start the corresponding loss effect. */
	void loseZoombini(int index, bool whirlpool, uint32 now);
	/** Return the console label for a generated key actor's role. */
	static const char *debugKeyRoleName(MysticMarshGrid::GenerationKeyRole role);
	/** Summarize the layout strategy, final release and waiting rule for a replay-checked answer. */
	Common::String debugAnswerStrategy(const Common::Array<MysticMarshGrid::AnswerLaunch> &launches) const;
	/** Play a loaded indexed effect at the current sound-effect volume. */
	void playSfx(int index);
	/** Queue a named speech resource for playback through the sound manager. */
	void enqueueSpeech(const Common::String &name);
	/** Load the page's symbol artwork, animations, and indexed sound effects. */
	void loadResources();
	/** Draw a trigger or matching spot with the same assisted color for its group. */
	void drawColorSymbol(ManagedSurface32 *screen, int symbolIndex, const Common::Point32 &pos) const;
	/** One missing-coverage sample along the left fringe of a cell plate. */
	struct CellEdgePixel {
		/** Pixel position relative to the original symbol origin. */
		Common::Point32 position;
		/** Straight color of the adjacent plate edge, without its pictogram. */
		RGBColor color;
		/** Additional opacity required to match the adjacent plate edge. */
		byte alpha;
	};
	/** Cache missing plate coverage without modifying the shared sprite. */
	void cacheCellEdgePixels(int symbolIndex, const RleBlock *sprite);
	/** Fill a symbol's narrow left fringe, clipped to the existing white grid's inner edge. */
	void fillCellEdge(ManagedSurface32 *screen, int symbolIndex, const Common::Point32 &pos) const;
	/** Page-owned plate samples indexed by the current symbol, including rotating states. */
	Common::Array<CellEdgePixel> _cellEdgePixels[60];
	/** Cache the answer for this puzzle's initial board until the page is initialized again. */
	mutable Common::String _initialAnswer;

	/** Gameplay grid retaining device states, occupants and traversal results. */
	MysticMarshGrid _grid;
	/** Snapshot of logical cell states used by the current render pass. */
	MysticMarshGrid::Cell _drawCells[MysticMarshGrid::kCellCount];
	/** Selectable crater cells and their page-space anchors. */
	Common::Array<Slot> _slots;
	/** Crater drop-target geometry and placement callbacks, parallel to _slots. */
	Common::Array<ZmbDropTarget> _dropTargets;
	/** Moving bubble presentation state indexed by puzzle-roster entry. */
	Common::Array<Bubble> _bubbles;
	/** Active crater and whirlpool effects with their presentation start times. */
	Common::Array<Effect> _effects;
	/** One-based background variant selected by the generated layout. */
	MysticMarshGrid::BackgroundKind _bgKind = MysticMarshGrid::kBackgroundL1LayoutA_01;
	/** Number of Zoombinis that have reached the far side of the marsh. */
	int _freed = 0;
	/** Roster index currently walking into a crater, or -1 when no placement is active. */
	int _placingZoombini = -1;
	/** Crater slot receiving the current placement, or -1 when none is active. */
	int _placingSlot = -1;
	/** Gameplay tick in milliseconds when the current crater placement began. */
	uint32 _placementStart = 0;
	/** Gameplay tick in milliseconds of the most recent grid update. */
	uint32 _lastTick = 0;
	/** Gameplay tick in milliseconds after which crater input becomes available. */
	uint32 _unlockTime = 0;
	/** Whether all puzzle actors have finished crossing or have been hidden. */
	bool _finished = false;
	/** Whether departure is waiting for retreat speech to finish. */
	bool _goPending = false;
	/** Bubbling crater-entrance animation released with the page. */
	Animation *_craterAnimation = nullptr;
	/** Whirlpool-entry animation for bubbles and their riders, released with the page. */
	Animation *_whirlpoolAnimation = nullptr;
	/** Loaded sound-effect handles indexed by the page's event sound slots. */
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
