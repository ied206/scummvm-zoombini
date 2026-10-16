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

#include "zoombini2/pages/puzzle_magicwall.h"
#include "common/debug.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "zoombini2/graphics.h"
#include "zoombini2/random.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleMagicWall::kPageName;
constexpr const char *PuzzleMagicWall::kBackgroundPath;
constexpr const char *PuzzleMagicWall::kLayoutPath;
constexpr const char *PuzzleMagicWall::kMusicPath;
constexpr const char *PuzzleMagicWall::kDotFormat;
constexpr const char *PuzzleMagicWall::kBugFormat;
constexpr const char *PuzzleMagicWall::kDirectionFormat;
constexpr const char *PuzzleMagicWall::kLightFormat;
constexpr const char *PuzzleMagicWall::kTabletPath;
constexpr const char *PuzzleMagicWall::kTabletDotPath;
constexpr const char *PuzzleMagicWall::kLeverPath;
constexpr const char *PuzzleMagicWall::kLeverGlowPath;
constexpr const char *PuzzleMagicWall::kGateFormat;
constexpr const char *PuzzleMagicWall::kGateBackFormat;
constexpr const char *PuzzleMagicWall::kExitFormat;
constexpr const char *PuzzleMagicWall::kMoveFormat;
constexpr const char *PuzzleMagicWall::kBugLoopPath;
constexpr const char *PuzzleMagicWall::kGateSoundPath;
constexpr const char *PuzzleMagicWall::kLeverSoundPath;
constexpr const char *PuzzleMagicWall::kRetreatSpeechPath;
constexpr const char *PuzzleMagicWall::kPerfectSpeechFormat;
constexpr const char *PuzzleMagicWall::kColors[11];
constexpr byte PuzzleMagicWall::kDotSourceBrightness[10];
constexpr byte PuzzleMagicWall::kBugSourceBrightness[10];
constexpr Common::Point32 PuzzleMagicWall::kTabletPositions[5];
constexpr Common::Point32 PuzzleMagicWall::kGatePositions[4];
constexpr Common::Point32 PuzzleMagicWall::kRosterPositions[8];
constexpr Common::Point32 PuzzleMagicWall::kLightPositions[10];

bool MagicWallMaze::readCount(Common::SeekableReadStream &stream, int &count, int maximum) {
	if (stream.size() - stream.pos() < 2)
		return false;
	count = stream.readSint16LE();
	return !stream.err() && 0 <= count && count <= maximum;
}

bool MagicWallMaze::readLayout(Common::SeekableReadStream &stream, Layout &layout) {
	int count;
	if (!readCount(stream, layout.level, 4) || !readCount(stream, layout.weight, 32767) || !readCount(stream, count, 10) || count == 0)
		return false;
	for (int i = 0; i < count; i++) {
		if (stream.size() - stream.pos() < 4)
			return false;
		const int x = stream.readSint16LE();
		const int y = stream.readSint16LE();
		layout.points.push_back(Common::Point32(x, y));
	}
	for (int group = 0; group < ARRAYSIZE(layout.groups); group++) {
		int variants;
		if (!readCount(stream, variants, 32767) || variants == 0)
			return false;
		for (int v = 0; v < variants; v++) {
			int rules;
			if (!readCount(stream, rules, 5))
				return false;
			Variant variant;
			for (int r = 0; r < rules; r++) {
				int edges;
				if (!readCount(stream, edges, 100))
					return false;
				Rule rule;
				for (int e = 0; e < edges; e++) {
					if (stream.size() - stream.pos() < 6)
						return false;
					Edge edge;
					edge.from = stream.readSint16LE();
					edge.to = stream.readSint16LE();
					const int kind = stream.readSint16LE();
					if (edge.from < 0 || count <= edge.from || edge.to < 0 || count <= edge.to || kind < 0 || 1 < kind)
						return false;
					edge.swap = kind == 1;
					rule.push_back(edge);
				}
				variant.push_back(rule);
			}
			layout.groups[group].push_back(variant);
		}
	}
	return !stream.err();
}

bool MagicWallMaze::load(Common::SeekableReadStream &stream) {
	_layouts.clear();
	_layoutIndex = -1;
	int count;
	if (!readCount(stream, count, 32767))
		return false;
	for (int i = 0; i < count; i++) {
		Layout layout;
		if (!readLayout(stream, layout)) {
			warning("MagicWall: incomplete layout %d; retaining %u complete layouts", i, _layouts.size());
			break;
		}
		_layouts.push_back(layout);
	}
	return !_layouts.empty();
}

bool MagicWallMaze::isIdentity(const Common::Array<int> &positions) {
	for (uint i = 0; i < positions.size(); i++) {
		if (positions[i] != static_cast<int>(i))
			return false;
	}
	return true;
}

void MagicWallMaze::applyRule(const Rule &rule, Common::Array<int> &positions, bool simultaneous) {
	Common::Array<bool> consumed;
	consumed.resize(positions.size());
	for (const Edge &edge : rule) {
		int from = -1;
		int to = -1;
		for (uint i = 0; i < positions.size(); i++) {
			if (positions[i] == edge.from && (!simultaneous || !consumed[i])) {
				from = i;
				if (simultaneous)
					consumed[i] = true;
			}
			if (edge.swap && positions[i] == edge.to && (!simultaneous || !consumed[i])) {
				to = i;
				if (simultaneous)
					consumed[i] = true;
			}
		}
		if (0 <= from)
			positions[from] = edge.to;
		if (edge.swap && 0 <= to)
			positions[to] = edge.from;
	}
}

void MagicWallMaze::scramble(Random &random) {
	_generationSequence.clear();
	int count;
	do {
		count = random.getRandomNumber(6);
	} while (count < 4);
	for (int i = 0; i < count; i++) {
		const int rule = random.getRandomNumber(_rules.size() - 1);
		_generationSequence.push_back(rule);
		applyRule(_rules[rule], _positions, false);
	}
}

int MagicWallMaze::solve(const Common::Array<int> &positions, int depth) const {
	if (isIdentity(positions))
		return 1;
	for (const Rule &rule : _rules) {
		Common::Array<int> next = positions;
		applyRule(rule, next, true);
		if (isIdentity(next))
			return 2;
		if (depth <= 4 && solve(next, depth + 1) == 2)
			return 2;
	}
	return 0;
}

bool MagicWallMaze::generate(int level, Random &random) {
	_generationSequence.clear();
	_initialPositions.clear();
	int totalWeight = 0;
	for (const Layout &layout : _layouts) {
		if (layout.level == level)
			totalWeight += layout.weight;
	}
	if (totalWeight == 0)
		return false;
	int choice = random.getRandomNumber(totalWeight - 1);
	for (uint i = 0; i < _layouts.size(); i++) {
		if (_layouts[i].level != level)
			continue;
		if (choice < _layouts[i].weight) {
			_layoutIndex = i;
			break;
		}
		choice -= _layouts[i].weight;
	}
	const Layout &selected = layout();
	_rules.clear();
	for (int group = 0; group < ARRAYSIZE(selected.groups); group++) {
		const Common::Array<Variant> &variants = selected.groups[group];
		const int variantIndex = random.getRandomNumber(variants.size() - 1);
		const Variant &variant = variants[variantIndex];
		if (!variant.empty() && !variant[0].empty()) {
			for (const Rule &rule : variant) {
				if (!rule.empty())
					_rules.push_back(rule);
			}
		}
	}
	if (_rules.empty() || 5 < _rules.size())
		return false;
	for (Rule &rule : _rules) {
		if (random.getRandomNumber(2) == 0) {
			for (Edge &edge : rule)
				SWAP(edge.from, edge.to);
		}
	}
	_positions.resize(selected.points.size());
	Common::Array<int> candidate;
	Common::Array<int> candidateSequence;
	for (int attempt = 0; attempt < 6; attempt++) {
		for (uint i = 0; i < _positions.size(); i++)
			_positions[i] = i;
		scramble(random);
		const int result = solve(_positions, 0);
		if (result == 0 || result == 2) {
			candidate = _positions;
			candidateSequence = _generationSequence;
		}
		if (result == 0)
			break;
	}
	if (!candidate.empty()) {
		_positions = candidate;
		_generationSequence = candidateSequence;
	}
	_initialPositions = _positions;
	return true;
}

void MagicWallMaze::apply(int ruleIndex) {
	if (0 <= ruleIndex && ruleIndex < static_cast<int>(_rules.size()))
		applyRule(_rules[ruleIndex], _positions, true);
}

bool MagicWallMaze::matched(int color) const {
	return static_cast<int>(_positions.size()) <= color || _positions[color] == color;
}

bool MagicWallMaze::gateMatched(int gate) const {
	return matched(gate) && matched(gate + 4) && matched(gate + 8);
}

bool MagicWallMaze::debugEncodePositions(const Common::Array<int> &positions, uint64 &state) {
	state = 0;
	if (positions.empty() || 10 < positions.size())
		return false;
	uint used = 0;
	for (uint i = 0; i < positions.size(); i++) {
		const int point = positions[i];
		if (point < 0 || positions.size() <= static_cast<uint>(point) || (used & (1U << point)))
			return false;
		used |= 1U << point;
		state |= static_cast<uint64>(point) << (4 * i);
	}
	return true;
}

uint MagicWallMaze::debugPermutationRank(uint64 state, uint count) {
	uint rank = 0;
	uint used = 0;
	for (uint i = 0; i < count; i++) {
		const uint point = (state >> (4 * i)) & 15;
		uint smaller = 0;
		for (uint previous = 0; previous < point; previous++)
			if (!(used & (1U << previous)))
				smaller += 1;
		rank = rank * (count - i) + smaller;
		used |= 1U << point;
	}
	return rank;
}

Common::Array<int> MagicWallMaze::debugRuleMapping(uint rule) const {
	Common::Array<int> mapping;
	for (uint i = 0; i < _positions.size(); i++)
		mapping.push_back(i);
	applyRule(_rules[rule], mapping, true);
	return mapping;
}

Common::Array<Common::Array<int>> MagicWallMaze::debugTabletPaths(uint rule) const {
	Common::Array<Common::Array<int>> paths;
	const Common::Array<int> mapping = debugRuleMapping(rule);
	uint64 encoded;
	if (!debugEncodePositions(mapping, encoded))
		return paths;
	bool seen[10] = {};
	for (uint point = 0; point < mapping.size(); point++) {
		if (seen[point] || mapping[point] == static_cast<int>(point))
			continue;
		Common::Array<int> path;
		int next = point;
		while (!seen[next]) {
			seen[next] = true;
			path.push_back(next);
			next = mapping[next];
		}
		paths.push_back(path);
	}
	return paths;
}

bool MagicWallMaze::debugCrossesGroup(const Common::Array<int> &mapping, uint group) {
	for (uint point = 0; point < mapping.size(); point++)
		if (static_cast<bool>(group & (1U << point)) != static_cast<bool>(group & (1U << mapping[point])))
			return true;
	return false;
}

const char *MagicWallMaze::debugTabletRole(uint rule) const {
	const Common::Array<int> mapping = debugRuleMapping(rule);
	uint moved = 0;
	for (uint point = 0; point < mapping.size(); point++)
		if (mapping[point] != static_cast<int>(point))
			moved |= 1U << point;
	static constexpr uint kLeftSquare = (1U << 0) | (1U << 1) | (1U << 2) | (1U << 3);
	static constexpr uint kInnerSquare = (1U << 2) | (1U << 3) | (1U << 4) | (1U << 5);
	static constexpr uint kOuterSquare = (1U << 0) | (1U << 1) | (1U << 6) | (1U << 7);
	switch (_layoutIndex) {
	case 0:
	case 1:
		return "exchanges the beetles at the two vertices of one edge";
	case 2:
		if (rule == 0)
			return "rotates the left four-vertex cycle toward the bridge";
		if (moved & kLeftSquare)
			return "exchanges a pair across the bridge between the left cycle and right group";
		return "selects the returning beetle by exchanging a pair within the right group";
	case 3:
		if (rule == 0)
			return "rotates the left triangle toward its upper holding-vertex exchange";
		if (rule == 1)
			return "rotates the right triangle toward its upper holding-vertex exchange";
		if (rule == 2)
			return "exchanges the left triangle with its upper holding vertex";
		if (rule == 3)
			return "exchanges the right triangle with its upper holding vertex";
		return "exchanges the upper holding vertices to transfer beetles between sides";
	case 4:
		if (rule == 0)
			return "rotates the outer ring toward the central exchange";
		if (rule == 1)
			return "exchanges the outer ring with the central holding vertex";
		return "reorders the outer ring and central holding vertex together";
	case 5:
	case 9:
	case 10:
	case 13:
		if (rule == 0)
			return "rotates the full ring to position a correction";
		return "changes relative order within the ring after positioning";
	case 6:
		if (rule == 0)
			return "rotates the inner square toward its outer holding-vertex exchanges";
		if (moved & kInnerSquare)
			return "exchanges the inner square with an outer holding vertex";
		return "exchanges outer holding vertices to select a returning beetle";
	case 7:
		if (rule == 0)
			return "rotates triangle 1 toward the bridge";
		if (rule == 1)
			return "rotates triangle 2 toward the bridge";
		return "exchanges a pair between triangles 1 and 2";
	case 8:
		if (rule == 0)
			return "rotates the outer ring toward the bridge";
		if (rule == 1)
			return "rotates the inner ring toward the bridge";
		return "exchanges a pair between the outer and inner rings";
	case 11:
		if (rule == 0)
			return "rotates the left triangle and exchanges the upper holding vertices";
		if (rule == 1)
			return "rotates the right triangle and exchanges the left triangle with its holding vertex";
		return "exchanges the right triangle with its upper holding vertex";
	case 12:
		if (rule == 0)
			return "rotates the square and triangle together; repeat to isolate either group";
		return "transfers beetles between the square and triangle and corrects their order";
	case 14:
		if (rule == 0)
			return moved & kOuterSquare ? "rotates the outer ring for positioning" : "rotates the inner ring for positioning";
		if (rule == 1)
			return "exchanges all four pairs between the outer and inner rings";
		if ((moved & kOuterSquare) && (moved & kInnerSquare))
			return "exchanges one pair between the outer and inner rings";
		return moved & kOuterSquare ? "corrects relative order within the outer ring" : "corrects relative order within the inner ring";
	case 15:
		if (rule == 0)
			return "rotates the upper triangle through its shared vertices";
		if (rule == 1)
			return "rotates the lower-left triangle through its shared vertices";
		if (rule == 2)
			return "rotates the lower-right triangle through its shared vertices";
		return "changes the relative order preserved by the three triangle rotations";
	case 16:
		if (rule == 0)
			return "rotates the upper-left square through its shared middle vertices";
		if (rule == 1)
			return "rotates the upper-right square through its shared middle vertices";
		if (rule == 2)
			return "rotates the lower-left square through its shared middle vertices";
		if (rule == 3)
			return "rotates the lower-right square through its shared middle vertices";
		return "corrects relative order with the extra local exchange";
	case 17:
		if (rule == 0)
			return "rotates the square and five-vertex ring together; repeat to isolate either group";
		if (debugCrossesGroup(mapping, kLeftSquare))
			return "transfers beetles between the square and five-vertex ring";
		return "corrects order within the square and five-vertex ring without exchanging groups";
	case 18:
		if (rule == 0)
			return "rotates the five-vertex ring and triangle together; repeat to isolate either group";
		if (debugCrossesGroup(mapping, kLeftSquare | (1U << 4)))
			return "transfers beetles between the five-vertex ring and triangle";
		return "corrects order within the five-vertex ring and triangle without exchanging groups";
	case 19:
		if (rule == 0)
			return "exchanges columns 1 and 2 to position a correction";
		if (rule == 1)
			return "exchanges columns 2 and 3 to position a correction";
		if (rule == 2)
			return "exchanges rows 1 and 2 to position a correction";
		if (rule == 3)
			return "exchanges rows 2 and 3 to position a correction";
		return "corrects relative order beyond whole-row and whole-column exchanges";
	case 20:
		if (rule < 3)
			return "repositions beetles with a whole-cube pair flip";
		if (rule == 3)
			return "corrects relative order with the local pair exchange";
		return "combines a local three-cycle and pair exchange; repeat to isolate either action";
	case 21:
		if (rule == 0)
			return "rotates all three triangles together without changing group membership";
		return "transfers beetles between the triangles to break their coupled order";
	default:
		return "positions or exchanges beetles within the connected group";
	}
}

MagicWallMaze::SolutionResult MagicWallMaze::debugAnswerSolver(Common::Array<int> &sequence, uint &visited) const {
	static constexpr uint kMaxStates = 500000;
	sequence.clear();
	visited = 0;
	uint64 start;
	if (!debugEncodePositions(_positions, start))
		return SolutionResult::kInvalid;
	const uint count = _positions.size();
	Common::Array<int> identity;
	uint stateCount = 1;
	for (uint i = 0; i < count; i++) {
		identity.push_back(i);
		stateCount *= i + 1;
	}
	uint64 goal;
	debugEncodePositions(identity, goal);
	if (start == goal)
		return SolutionResult::kSolved;
	Common::Array<Common::Array<int>> maps;
	for (uint rule = 0; rule < _rules.size(); rule++) {
		Common::Array<int> mapping = debugRuleMapping(rule);
		uint64 encoded;
		if (!debugEncodePositions(mapping, encoded))
			return SolutionResult::kInvalid;
		maps.push_back(mapping);
	}
	Common::Array<int> parents(stateCount, -1);
	Common::Array<uint> tablets(stateCount, 0);
	Common::Array<uint64> queue;
	queue.push_back(start);
	const uint startRank = debugPermutationRank(start, count);
	parents[startRank] = startRank;
	bool solved = false;
	for (uint head = 0; head < queue.size() && !solved; head++) {
		const uint64 current = queue[head];
		const uint currentRank = debugPermutationRank(current, count);
		visited += 1;
		for (uint rule = 0; rule < maps.size(); rule++) {
			uint64 next = 0;
			for (uint actor = 0; actor < count; actor++) {
				const uint point = (current >> (4 * actor)) & 15;
				next |= static_cast<uint64>(maps[rule][point]) << (4 * actor);
			}
			const uint nextRank = debugPermutationRank(next, count);
			if (0 <= parents[nextRank])
				continue;
			parents[nextRank] = currentRank;
			tablets[nextRank] = rule;
			if (next == goal) {
				solved = true;
				break;
			}
			if (kMaxStates <= queue.size())
				return SolutionResult::kLimit;
			queue.push_back(next);
		}
	}
	if (!solved)
		return SolutionResult::kUnreachable;
	for (uint rank = 0; rank != startRank; rank = parents[rank])
		sequence.push_back(tablets[rank]);
	for (uint i = 0; i < sequence.size() / 2; i++)
		SWAP(sequence[i], sequence[sequence.size() - 1 - i]);
	Common::Array<int> replay = _positions;
	for (int rule : sequence)
		applyRule(_rules[rule], replay, true);
	if (!isIdentity(replay)) {
		sequence.clear();
		return SolutionResult::kInvalid;
	}
	return SolutionResult::kSolved;
}

const char *MagicWallMaze::debugLayoutAdvice() const {
	static constexpr const char *kAdvice[] = {
		"- Two rows, pair exchanges: each tablet swaps beetles across one edge.\n"
		"    - To swap distant beetles and restore intermediate ones, press tablets along a path, "
		"then retrace it without repeating the final edge.",
		"- Five vertices, pair exchanges: rearrange beetles using the exchange edges.\n"
		"    - Press tablets along a path, then retrace it without repeating the final edge "
		"to swap endpoint beetles and restore intermediate ones.",
		"- Left four-vertex cycle and right three-vertex exchange group: rotate a misplaced left beetle to the bridge, "
		"swap it with the right group, then undo the rotation.\n"
		"    - Use right-side swaps to choose the returning beetle.",
		"- Two lower triangles with upper holding vertices: rotate each triangle to its upper exchange vertex.\n"
		"    - The upper vertices bridge the groups.\n"
		"    - Move beetles to the correct side, then sort each triangle and its holding vertex.",
		"- Five-vertex outer cycle with a central holding vertex: rotate an outer beetle to the central exchange, swap, then undo the rotation.\n"
		"    - Use the center as a buffer to reorder the ring; restore the center's beetle once the ring is sorted.",
		"- Five-vertex ring with a pair exchange: rotate to choose the beetles to swap; swaps change their circular order.\n"
		"    - Undo the rotation after each correction, then align the sorted ring with its colored vertices.",
		"- Inner square and outer holding vertices: rotate the square to reach exchanges, "
		"use outer vertices as buffers, then undo the rotation.\n"
		"    - Some variants leave an outer vertex untouched; preserve it while sorting the connected group.",
		"- Two three-vertex cycles joined by an exchange: rotate both to bring the desired pair to the bridge, swap, then undo the rotations.\n"
		"    - Change group membership and circular order this way, then align both cycles.",
		"- Inner and outer four-vertex cycles: rotate each ring independently to the bridge.\n"
		"    - Swap a pair, then undo the rotations to restore other beetles.\n"
		"    - Correct group membership and circular order before final alignment.",
		"- Five-vertex ring with an extra exchange or partial cycle: full rotation only changes alignment.\n"
		"    - Rotate misplaced beetles onto the smaller operation's vertices, apply it, then rotate back.\n"
		"    - Account for every cycle on that tablet.",
		"- Seven-vertex ring with a smaller operation: rotate the desired beetles to its exchange or partial-cycle vertices, "
		"apply it, then rotate back.\n"
		"    - Rotating the whole ring alone cannot change relative order.",
		"- Two triangles with coupled exchanges: the first two tablets each combine a three-cycle with a separate swap.\n"
		"    - Three presses isolate the swap; two isolate the reverse three-cycle.\n"
		"    - Use these actions to transfer and sort beetles.",
		"- Coupled four-vertex and three-vertex cycles: three presses of tablet 1 move only the square; four move only the triangle.\n"
		"    - Use other tablets to transfer beetles between the groups and correct their order.\n"
		"    - Twelve presses restore both.",
		"- Five-vertex ring with partial cycles and exchanges: rotate to position a correction, apply a smaller operation, then rotate back.\n"
		"    - This layout is not selected in normal play.",
		"- Nested squares: one tablet rotates a ring; another swaps all four pairs between rings.\n"
		"    - Swap rings, rotate, then swap back to act on the other ring.\n"
		"    - Use the small pair exchange to correct relative order, then undo positioning moves.",
		"- Three overlapping triangles: shared vertices let beetles move between cycles.\n"
		"    - Rotate a beetle to a shared vertex, move it into the next triangle, then undo the first rotation.\n"
		"    - The extra swap changes the order that three-cycles alone preserve.",
		"- Three-by-three grid of overlapping square rotations: pass beetles between squares through shared middle vertices.\n"
		"    - Position a local correction, apply it, then undo positioning moves; matched beetles may need to move temporarily.",
		"- Coupled four-vertex and five-vertex cycles: five presses of tablet 1 move only the square; four move only the five-vertex ring.\n"
		"    - Use other tablets to transfer beetles between the groups.\n"
		"    - Twenty presses restore both.",
		"- Coupled five-vertex and three-vertex cycles: three presses of tablet 1 move only the ring; five move only the triangle.\n"
		"    - Use other operations to transfer beetles and reorder them.\n"
		"    - Fifteen presses restore both.",
		"- Three-by-three grid with whole-row and whole-column swaps: these preserve row/column structure; "
		"use the extra local operation for arbitrary corrections.\n"
		"    - Position a correction for the local operation, apply it, then undo the row/column swaps.",
		"- Cube with three whole-cube pair flips: press any flip again to undo it.\n"
		"    - Use flips to bring a pair to the local exchange, swap, then undo the flips.\n"
		"    - Separate a bundled three-cycle and swap with two or three presses.",
		"- Three coupled triangles: tablet 1 moves all groups together; three presses restore all, so repetition cannot isolate one group.\n"
		"    - Use other tablets to transfer beetles or break the coupling, then undo positioning moves.",
	};
	if (_layoutIndex < 0 || ARRAYSIZE(kAdvice) <= static_cast<uint>(_layoutIndex))
		return "- Use cycle rotations to position exchanges, then undo the positioning rotations.";
	return kAdvice[_layoutIndex];
}

PuzzleMagicWall::PuzzleMagicWall(Zoombini2Engine *vm)
	: PuzzleBase(vm, PageId::kPageMagicWall_06) {
	_initialBackgroundPath = kBackgroundPath;
}

PuzzleMagicWall::~PuzzleMagicWall() {
	_vm->setCursor(Zoombini2Engine::CursorType::kDefault);
	for (Beetle &beetle : _beetles)
		delete beetle.path;
	for (int i = 0; i < ARRAYSIZE(_gateAnimations); i++)
		delete _gateAnimations[i];
	delete _leverAnimation;
	if (SoundManager *sound = _vm->getSoundManager()) {
		sound->unload(_bugSound);
		sound->unload(_gateSound);
		sound->unload(_leverSound);
	}
	for (ZoombiniRunner *runner : _puzzleZoombinis)
		runner->clearMovement();
	finishPuzzleRoster(_vm->_state->_rescue1Storage);
}

void PuzzleMagicWall::loadResources() {
	Gfx *gfx = _vm->_gfx;
	for (int i = 0; i < ARRAYSIZE(kColors) - 1; i++) {
		gfx->loadPageRleBlock(Common::String::format(kDotFormat, kColors[i]));
		gfx->loadPageRleBlock(Common::String::format(kBugFormat, kColors[i]));
	}
	for (int i = 0; i < ARRAYSIZE(kColors); i++)
		gfx->loadPageRleBlock(Common::String::format(kLightFormat, kColors[i]));
	for (int i = 0; i < 8; i++)
		gfx->loadPageRleBlock(Common::String::format(kDirectionFormat, i + 1));
	gfx->loadPageRleBlock(kTabletPath);
	gfx->loadPageRleBlock(kTabletDotPath);
	gfx->loadPageBitBlock(kLeverGlowPath);
	PageLayer *foreground = gfx->getPageLayerStack()->getLayer(1);
	for (int i = 0; i < ARRAYSIZE(_gateAnimations); i++) {
		gfx->loadPageBitBlock(Common::String::format(kGateBackFormat, 'A' + i));
		_gateAnimations[i] = new Animation(_vm);
		_gateAnimations[i]->loadFromFile(Common::Path(Common::String::format(kGateFormat, 'A' + i)));
		AnimationRunner *runner = foreground->createAnimationRunner(kGatePositions[i], AnimationRunnerMode::kPlayOnce00);
		_gateRunners[i] = runner;
		runner->setAnimation(_gateAnimations[i]);
		for (int frame = 0; frame < 5; frame++)
			runner->addTimedFrame(frame, 50);
		runner->addTimedFrame(5, 2000);
		for (int frame = 4; 0 <= frame; frame--)
			runner->addTimedFrame(frame, 50);
		runner->setCompletionCallback(gateDone, this);
	}
	_leverAnimation = new Animation(_vm);
	_leverAnimation->loadFromFile(Common::Path(kLeverPath));
	_leverRunner = gfx->getPageLayerStack()->getLayer(0)->createAnimationRunner(Common::Point32(279, 362), AnimationRunnerMode::kPlayOnceAndHide02);
	_leverRunner->setAnimation(_leverAnimation);
	static constexpr int kLeverFrames[5] = {
		0,
		1,
		2,
		1,
		0,
	};
	for (int frame : kLeverFrames)
		_leverRunner->addTimedFrame(frame, 100);
	if (SoundManager *sound = _vm->getSoundManager()) {
		_bugSound = sound->load(false, Common::Path(kBugLoopPath), false);
		_gateSound = sound->load(false, Common::Path(kGateSoundPath), false);
		_leverSound = sound->load(false, Common::Path(kLeverSoundPath), false);
	}
}

void PuzzleMagicWall::init() {
	PuzzleBase::init();
	loadResources();
	startPageMusic(Common::Path(kMusicPath));
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *runner = _puzzleZoombinis[i];
		runner->clearMovement();
		runner->setDefaultAnimation(ZmbAnimationKind::kBasic);
		runner->resetAnimation();
		runner->setPosition(kRosterPositions[MIN<uint>(i, 7)]);
		runner->setInputEnabled(false);
		runner->setHidden(false);
		runner->setCanAdvanceFromPage(false);
	}
	Common::SeekableReadStream *stream = _vm->openResourceFile(kLayoutPath);
	const bool loaded = stream && _maze.load(*stream);
	delete stream;
	if (!loaded)
		error("MagicWall: no bounded puzzle layouts in %s", kLayoutPath);
	newPuzzle();
	if (_vm->inJourneyMode())
		_vm->_state->registerPageVisit(_pageId, 1);
}

void PuzzleMagicWall::newPuzzle() {
	_ready = _maze.generate(_puzzleLevel, *_vm->_rnd);
	if (!_ready)
		error("MagicWall: no usable layout for level %d", _puzzleLevel);
	for (Beetle &beetle : _beetles)
		delete beetle.path;
	_beetles.clear();
	_beetles.resize(_maze.positions().size());
	for (uint i = 0; i < _maze.positions().size(); i++) {
		_beetles[i].position = vertexPosition(_maze.positions()[i]) + Common::Point32(5, 8);
		_beetles[i].direction = 0;
	}
	_movingRule = -1;
	_hoveredTablet = -1;
	debug(1, "MagicWall: phase=%d level=%d layout=%d points=%u tablets=%u", static_cast<int>(_phase), _puzzleLevel,
		  _maze.layoutIndex(), _maze.positions().size(), _maze.rules().size());
	for (uint i = 0; i < _maze.positions().size(); i++)
		debug(2, "MagicWall: beetle %u at %d", i, _maze.positions()[i]);
	for (uint i = 0; i < _maze.rules().size(); i++) {
		for (const MagicWallMaze::Edge &edge : _maze.rules()[i])
			debug(2, "MagicWall: rule %u edge %d %d %d", i, edge.from, edge.to, edge.swap);
	}
}

Common::Point32 PuzzleMagicWall::vertexPosition(int index) const {
	const Common::Point32 &point = _maze.layout().points[index];
	return Common::Point32(300 + 40 * point.x, -15 + 40 * point.y);
}

bool PuzzleMagicWall::beetlesMoving() const {
	for (const Beetle &beetle : _beetles) {
		if (beetle.path)
			return true;
	}
	return false;
}

bool PuzzleMagicWall::runnersMoving() const {
	for (const ZoombiniRunner *runner : _puzzleZoombinis) {
		if (runner->hasMovementPath())
			return true;
	}
	return false;
}

bool PuzzleMagicWall::gatesActive() const {
	for (int i = 0; i < ARRAYSIZE(_gateRunners); i++) {
		if (_gateRunners[i] && _gateRunners[i]->isActive())
			return true;
	}
	return false;
}

int PuzzleMagicWall::directionIndex(const Common::Point32 &from, const Common::Point32 &to) {
	const double dx = to.x - from.x;
	const double dy = to.y - from.y;
	if (dx == 0 && dy == 0)
		return 0;
	int angle = static_cast<int>(acos(dx / sqrt(dx * dx + dy * dy)) * 180.0 * 0.31831926);
	if (from.y < to.y)
		angle = -angle;
	return (2 - angle / 45 + 8) % 8;
}

void PuzzleMagicWall::startRule(int index) {
	if (!_ready || _phase == Phase::kFinished || _nextPuzzlePending || gatesActive() || runnersMoving() || beetlesMoving())
		return;
	const Common::Array<int> before = _maze.positions();
	_maze.apply(index);
	const uint32 now = _vm->getTotalPlayTime();
	for (uint i = 0; i < before.size(); i++) {
		if (before[i] == _maze.positions()[i])
			continue;
		Beetle &beetle = _beetles[i];
		const Common::Point32 end = vertexPosition(_maze.positions()[i]);
		const Common::Point32 third((end.x - beetle.position.x) / 3, (end.y - beetle.position.y) / 3);
		beetle.path = new PathObject(_vm);
		beetle.path->appendSegment(beetle.position, beetle.position + third, end - third, end, 10, 0);
		beetle.path->start(now);
		beetle.direction = directionIndex(beetle.position, end);
	}
	_movingRule = index;
	if (SoundManager *sound = _vm->getSoundManager())
		sound->playLoop(_bugSound);
	debug(2, "MagicWall: tablet=%d", index);
}

void PuzzleMagicWall::startRunnerPath(int index, int gate, bool exit) {
	if (index < 0 || 8 <= index || static_cast<int>(_puzzleZoombinis.size()) <= index || _exited[index])
		return;
	const Common::Path path(Common::String::format(exit ? kExitFormat : kMoveFormat, gate + 1));
	PathObject *movement = _vm->loadPagePATPath(path);
	if (!movement)
		error("MagicWall: required roster path is unavailable");
	movement->setStepValueForAllSegments(4);
	ZoombiniRunner *runner = _puzzleZoombinis[index];
	runner->resetAnimation();
	runner->startMovement(movement, _vm->getTotalPlayTime());
	runner->startDirectionTrackedAnimation(_vm->getTotalPlayTime());
	if (exit)
		runner->setCanAdvanceFromPage(true);
}

void PuzzleMagicWall::submit() {
	if (!_ready || gatesActive() || runnersMoving() || beetlesMoving() || _phase == Phase::kFinished)
		return;
	const uint32 frameTick = _vm->getFrameTickCount();
	_leverRunner->start(frameTick);
	SoundManager *sound = _vm->getSoundManager();
	if (sound)
		sound->play(_leverSound);
	bool accepted = false;
	for (int gate = 0; gate < ARRAYSIZE(_gateRunners); gate++) {
		if (!_maze.gateMatched(gate))
			continue;
		const int index = _exited[gate] ? gate + 4 : gate;
		if (static_cast<int>(_puzzleZoombinis.size()) <= index || _exited[index])
			continue;
		startRunnerPath(index, gate, true);
		if (index < 4)
			startRunnerPath(index + 4, gate, false);
		_gateRunners[gate]->start(frameTick);
		if (sound)
			sound->play(_gateSound);
		accepted = true;
	}
	if (accepted) {
		_nextPuzzlePending = _phase == Phase::kFirstBoard;
		if (_phase == Phase::kSecondBoard)
			_phase = Phase::kFinished;
		_hoveredTablet = -1;
	}
	debug(1, "MagicWall: submit phase=%d accepted=%d", static_cast<int>(_phase), accepted);
}

void PuzzleMagicWall::gateDone(void *context, AnimationRunner *runner) {
	(void)runner;
	PuzzleMagicWall *page = static_cast<PuzzleMagicWall *>(context);
	page->_canDepart = true;
	if (page->_phase == Phase::kFinished)
		page->_vm->restartGoBlink();
}

void PuzzleMagicWall::onUpdate() {
	const uint32 now = _vm->getTotalPlayTime();
	if (_speechPending) {
		SoundManager *sound = _vm->getSoundManager();
		if (!sound || !sound->hasPendingSpeech()) {
			_speechPending = false;
			_vm->_mapTransitSrcPageId = PageId::kPageMagicWall_06;
			_vm->requestPageChange(PageId::kPageMapTrans_22);
		}
	}
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *runner = _puzzleZoombinis[i];
		if (runner->hasMovementPath()) {
			if (runner->isMovementFinished()) {
				runner->clearMovement();
				runner->resetAnimation();
				if (runner->getScreenPosition().y < 330) {
					runner->setHidden(true);
					if (i < 8)
						_exited[i] = true;
				}
			} else {
				runner->advanceMovement(now);
			}
		}
		runner->updateAnimation(now);
	}
	if (gatesActive() || runnersMoving()) {
		_vm->setCursor(Zoombini2Engine::CursorType::kDefault);
		return;
	}
	if (_nextPuzzlePending) {
		_phase = Phase::kSecondBoard;
		_nextPuzzlePending = false;
		newPuzzle();
	}
	const bool wasMoving = beetlesMoving();
	for (Beetle &beetle : _beetles) {
		if (beetle.path && !beetle.path->advance(now, beetle.position)) {
			delete beetle.path;
			beetle.path = nullptr;
			if (SoundManager *sound = _vm->getSoundManager())
				sound->stopAll(_bugSound);
		}
	}
	if (wasMoving && !beetlesMoving())
		_movingRule = -1;
	_hoveredTablet = tabletAt(_pointer);
	const bool hover = _phase != Phase::kFinished && !_nextPuzzlePending && !beetlesMoving() &&
					   (0 <= _hoveredTablet || inside(_pointer, 279, 341, 40, 70));
	_vm->setCursor(hover ? Zoombini2Engine::CursorType::kInteractive : Zoombini2Engine::CursorType::kDefault);
	if (0 <= _hoveredTablet || beetlesMoving()) {
		_ripplePhase += 7;
		if (255 < _ripplePhase)
			_ripplePhase = 0;
	}
}

void PuzzleMagicWall::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

Common::Point32 PuzzleMagicWall::tabletPoint(int tablet, int point, int inset) const {
	const Common::Point32 &p = _maze.layout().points[point];
	const int firstX = _maze.layout().points[0].x;
	const Common::Point32 origin = kTabletPositions[tablet];
	return Common::Point32(origin.x + 3 * p.x + inset,
						   origin.y + 3 * p.y + inset - static_cast<int>(3 * (p.x - firstX) * -0.5));
}

void PuzzleMagicWall::drawRipple(ManagedSurface32 *screen, Common::Point32 from, const Common::Point32 &to, int phase) {
	const int dx = ABS(to.x - from.x);
	const int dy = ABS(to.y - from.y);
	const int sx = from.x < to.x ? 1 : -1;
	const int sy = from.y < to.y ? 1 : -1;
	const int steps = MAX(dx, dy);
	int errorTerm = steps / 2;
	for (int step = 0; step < steps; step++) {
		phase -= 3;
		if (phase < 0)
			phase = 255;
		if (dy <= dx) {
			errorTerm += dy;
			if (dx <= errorTerm) {
				errorTerm -= dx;
				from.y += sy;
			}
			from.x += sx;
		} else {
			errorTerm += dx;
			if (dy <= errorTerm) {
				errorTerm -= dy;
				from.x += sx;
			}
			from.y += sy;
		}
		const uint32 intensity = 255 - phase % 255;
		const uint32 color = screen->format.RGBToColor(intensity, intensity, intensity);
		_vm->_gfx->fillRect(screen, Common::Rect32(from.x, from.y, from.x + 1, from.y + 1), color);
	}
}

void PuzzleMagicWall::drawTablets(ManagedSurface32 *screen) {
	for (uint i = 0; i < _maze.rules().size(); i++) {
		_vm->_gfx->drawPageRleBlock(screen, kTabletPath, kTabletPositions[i] - Common::Point32(5, 5));
		if (_nextPuzzlePending || _phase == Phase::kFinished)
			continue;
		for (const MagicWallMaze::Edge &edge : _maze.rules()[i])
			drawRipple(screen, tabletPoint(i, edge.from, 12), tabletPoint(i, edge.to, 12), 255);
		for (uint point = 0; point < _maze.layout().points.size(); point++)
			_vm->_gfx->drawPageRleBlock(screen, kTabletDotPath, tabletPoint(i, point, 10));
	}
}

void PuzzleMagicWall::drawConnections(ManagedSurface32 *screen, int index) {
	if (index < 0 || static_cast<int>(_maze.rules().size()) <= index)
		return;
	for (const MagicWallMaze::Edge &edge : _maze.rules()[index]) {
		Common::Point32 from = vertexPosition(edge.from) + Common::Point32(25, 25);
		Common::Point32 to = vertexPosition(edge.to) + Common::Point32(25, 25);
		if (!edge.swap) {
			drawRipple(screen, from, to, _ripplePhase);
			continue;
		}
		drawRipple(screen, to, from, _ripplePhase);
		if (from.x == to.x) {
			from.x += 10;
			to.x += 10;
		} else if (from.y == to.y) {
			from.y += 10;
			to.y += 10;
		} else {
			const float initialToX = to.x;
			const float initialToY = to.y;
			float fx = from.x;
			float fy = from.y;
			float tx = to.x;
			float ty = to.y;
			const float stepY = -((ty - fy) / (tx - fx)) * 0.01f;
			do {
				fx += 0.01f;
				fy += stepY;
				tx += 0.01f;
				ty += stepY;
			} while (sqrt((initialToX - tx) * (initialToX - tx) + (initialToY - ty) * (initialToY - ty)) < 10.0);
			from = Common::Point32(static_cast<int>(fx), static_cast<int>(fy));
			to = Common::Point32(static_cast<int>(tx), static_cast<int>(ty));
		}
		drawRipple(screen, from, to, _ripplePhase);
	}
}

void PuzzleMagicWall::drawBeetles(ManagedSurface32 *screen) {
	if (_phase == Phase::kFinished || _nextPuzzlePending)
		return;
	for (uint i = 0; i < _maze.positions().size(); i++) {
		drawColorSprite(screen, kBugFormat, i, _beetles[i].position, kBugSourceBrightness[i]);
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kDirectionFormat, _beetles[i].direction + 1), _beetles[i].position);
	}
}

bool PuzzleMagicWall::colorAssistRGB(Zoombini2MetaEngine::ColorAssistMode mode, uint color, RGBColor &displayColor) {
	if (10 <= color || mode == Zoombini2MetaEngine::ColorAssistMode::kOriginal)
		return false;
	if (mode == Zoombini2MetaEngine::ColorAssistMode::kEnhancedDistinction) {
		if (color == 6) {
			displayColor = RGBColor(240, 32, 205);
			return true;
		}
		if (color == 7) {
			displayColor = RGBColor(178, 255, 212);
			return true;
		}
		return false;
	}
	static constexpr RGBColor kRedGreenColors[10] = {
		RGBColor(0, 64, 255),
		RGBColor(65, 217, 141),
		RGBColor(42, 91, 140),
		RGBColor(178, 89, 0),
		RGBColor(115, 115, 115),
		RGBColor(100, 40, 32),
		RGBColor(0, 217, 217),
		RGBColor(0, 191, 255),
		RGBColor(0, 108, 217),
		RGBColor(217, 217, 65),
	};
	displayColor = kRedGreenColors[color];
	return true;
}

void PuzzleMagicWall::drawColorSprite(ManagedSurface32 *screen, const char *format, uint color, const Common::Point32 &pos, byte sourceBrightness) const {
	const Common::String key = Common::String::format(format, kColors[color]);
	RGBColor displayColor;
	if (colorAssistRGB(_vm->getColorAssistMode(), color, displayColor)) {
		const RleBlock *sprite = _vm->_gfx->loadPageRleBlock(key);
		if (sprite)
			sprite->drawToScreenRecolored(screen, pos, displayColor, _vm->getAlphaLUT(), nullptr, sourceBrightness);
	} else {
		_vm->_gfx->drawPageRleBlock(screen, key, pos);
	}
}

void PuzzleMagicWall::onRenderContent(ManagedSurface32 *screen) {
	if (!_ready)
		return;
	if (_phase != Phase::kFinished) {
		drawTablets(screen);
		if (!_nextPuzzlePending)
			drawConnections(screen, beetlesMoving() ? _movingRule : _hoveredTablet);
		for (uint i = 0; i < _maze.positions().size(); i++) {
			const Common::Point32 pos = vertexPosition(i);
			if (!_backgroundPath.empty())
				_vm->_gfx->drawPageBitBlockSubRect(screen, _backgroundPath, pos, Common::Rect(pos.x, pos.y, pos.x + 50, pos.y + 50));
			drawColorSprite(screen, kDotFormat, i, vertexPosition(i), kDotSourceBrightness[i]);
		}
		drawBeetles(screen);
	}
	bool allMatched = true;
	for (uint i = 0; i < _maze.positions().size(); i++) {
		const bool lit = _phase != Phase::kFinished && _maze.matched(i);
		if (lit)
			drawColorSprite(screen, kLightFormat, i, kLightPositions[i] - Common::Point32(6, 5), 255);
		else
			_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kLightFormat, kColors[10]), kLightPositions[i] - Common::Point32(6, 5));
		if (!_maze.matched(i))
			allMatched = false;
	}
	if (allMatched && _phase != Phase::kFinished && !_leverRunner->isActive())
		_vm->_gfx->drawPageBitBlock(screen, kLeverGlowPath, Common::Point32(279, 362));
	for (int i = 0; i < ARRAYSIZE(_gateRunners); i++) {
		if (_gateRunners[i]->isActive())
			_vm->_gfx->drawPageBitBlock(screen, Common::String::format(kGateBackFormat, 'A' + i), kGatePositions[i]);
	}
}

void PuzzleMagicWall::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleMagicWall::onActorsRendered() {
	for (ZoombiniRunner *runner : _puzzleZoombinis)
		runner->advanceAnimationAfterDraw();
}

void PuzzleMagicWall::onRenderForeground(ManagedSurface32 *screen) {
	_vm->_gfx->getPageLayerStack()->getLayer(1)->drawAndUpdate(screen);
	if (_ready && !_nextPuzzlePending && _phase != Phase::kFinished && gatesActive())
		drawBeetles(screen);
}

bool PuzzleMagicWall::inside(const Common::Point &pos, int x, int y, int width, int height) {
	return x < pos.x && pos.x < x + width && y < pos.y && pos.y < y + height;
}

int PuzzleMagicWall::tabletAt(const Common::Point &pos) const {
	if (!_ready || _phase == Phase::kFinished || _nextPuzzlePending)
		return -1;
	for (uint i = 0; i < _maze.rules().size(); i++) {
		if (inside(pos, kTabletPositions[i].x, kTabletPositions[i].y, 50, 50))
			return i;
	}
	return -1;
}

EventHandleResult PuzzleMagicWall::onLButtonDown(const Common::Point &pos) {
	_pointer = pos;
	_buttonArmed = tabletAt(pos) != -1 || inside(pos, 279, 341, 40, 70);
	return _buttonArmed ? EventHandleResult::kConsumed : EventHandleResult::kPassthrough;
}

EventHandleResult PuzzleMagicWall::onLButtonUp(const Common::Point &pos) {
	_pointer = pos;
	if (!_buttonArmed)
		return EventHandleResult::kPassthrough;
	_buttonArmed = false;
	const int tablet = tabletAt(pos);
	if (0 <= tablet)
		startRule(tablet);
	else if (inside(pos, 279, 341, 40, 70))
		submit();
	return EventHandleResult::kConsumed;
}

EventHandleResult PuzzleMagicWall::onMouseMove(const Common::Point &pos) {
	_pointer = pos;
	return EventHandleResult::kPassthrough;
}

bool PuzzleMagicWall::blocksSidebarInteraction() const {
	return gatesActive() || runnersMoving() || beetlesMoving() || _speechPending;
}

bool PuzzleMagicWall::canUseGoButton() const {
	return (_canDepart || _vm->_zoombiniWalkingFlag) && !blocksSidebarInteraction();
}

bool PuzzleMagicWall::onGoButtonPressed() {
	if (_speechPending)
		return false;
	if (_vm->inPracticeMode())
		return true;
	int remaining = 0;
	for (const ZoombiniRunner *runner : _puzzleZoombinis) {
		if (!runner->canAdvanceFromPage())
			remaining += 1;
	}
	Common::String path;
	if (4 <= remaining)
		path = kRetreatSpeechPath;
	else if (remaining == 0)
		path = Common::String::format(kPerfectSpeechFormat, _vm->_rnd->getRandomNumber(4) + 1);
	else
		return true;
	SoundManager *sound = _vm->getSoundManager();
	if (!sound)
		return true;
	sound->queueSpeech(Common::Path(path));
	_speechPending = true;
	return false;
}

Common::String PuzzleMagicWall::debugGetAnswer() const {
	Common::String answer = debugAnswerHeader();
	if (!_ready)
		return answer + "\n  Board resources are unavailable.\n";
	answer += "\n  Goal: move each beetle onto the vertex of its own color.\n";
	uint layoutNumber = 0;
	uint layoutCount = 0;
	for (uint i = 0; i < _maze.layouts().size(); i++) {
		const MagicWallMaze::Layout &layout = _maze.layouts()[i];
		if (layout.level != _maze.layout().level || layout.weight == 0)
			continue;
		layoutCount += 1;
		if (static_cast<int>(i) == _maze.layoutIndex())
			layoutNumber = layoutCount;
	}
	answer += Common::String::format("\n  [*] Layout (%02u/%02u) strategy\n    %s\n", layoutNumber, layoutCount, _maze.debugLayoutAdvice());
	if (_phase == Phase::kFinished)
		return answer + "  Both boards are complete; use Go.\n";
	if (_nextPuzzlePending || gatesActive() || runnersMoving() || beetlesMoving())
		return answer + "  Wait for the current movement or board change, then request the answer again.\n";
	Common::Array<int> sequence;
	uint visited = 0;
	const MagicWallMaze::SolutionResult result = _maze.debugAnswerSolver(sequence, visited);
	switch (result) {
	case MagicWallMaze::SolutionResult::kSolved:
		if (sequence.empty()) {
			answer += "    The beetles already match their vertices.\n";
		} else {
			answer += Common::String::format("\n  [*] Searched solution of the current board (%u presses):\n", sequence.size());
			answer += "    Press tablets (left to right, 1-based-index):\n";
			answer += "    - ";
			for (uint step = 0; step < sequence.size(); step++) {
				if (0 < step)
					answer += ", ";
				answer += Common::String::format("%d", sequence[step] + 1);
			}
			answer += "\n";
		}
		break;
	case MagicWallMaze::SolutionResult::kUnreachable:
		answer += "  No solution is reachable from this board setup.\n";
		break;
	case MagicWallMaze::SolutionResult::kLimit:
		answer += "  The search reached its budget limit; this does not prove the board unsolvable.\n";
		break;
	default:
		return answer + "  The board or a tablet is not a valid permutation; no solution can be certified.\n";
	}
	answer += "\n  [*] Tablet Paths (left-to-right):\n";
	uint pathIndex = 0;
	for (uint rule = 0; rule < _maze.rules().size(); rule++) {
		answer += Common::String::format("    Tablet %u: %s\n", rule + 1, _maze.debugTabletRole(rule));
		for (const Common::Array<int> &path : _maze.debugTabletPaths(rule)) {
			answer += Common::String::format("      Path %c (", static_cast<char>('A' + pathIndex));
			for (uint vertex = 0; vertex < path.size(); vertex++) {
				if (0 < vertex)
					answer += "-";
				answer += Common::String::format("%02d", path[vertex]);
			}
			answer += Common::String::format("): %u-press cycle\n", path.size());
			pathIndex += 1;
		}
	}
	return answer;
}

Common::String PuzzleMagicWall::debugGetChanceDetails() const {
	return "Tablet presses and rejected submissions are unlimited.\n";
}

} // End of namespace Zoombini2
