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
#include "common/algorithm.h"
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
constexpr const char *PuzzleMagicWall::kStoneLightPath;
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
constexpr byte PuzzleMagicWall::kDotSourceBrightness[10];
constexpr byte PuzzleMagicWall::kBugSourceBrightness[10];
constexpr Common::Point32 PuzzleMagicWall::kTabletPositions[5];
constexpr Common::Point32 PuzzleMagicWall::kGatePositions[4];
constexpr Common::Point32 PuzzleMagicWall::kRosterPositions[8];
constexpr Common::Point32 PuzzleMagicWall::kLightPositions[10];

bool MagicWallBoard::readCount(Common::SeekableReadStream &stream, int &count, int maximum) {
	if (stream.size() - stream.pos() < 2)
		return false;
	count = stream.readSint16LE();
	return !stream.err() && 0 <= count && count <= maximum;
}

bool MagicWallBoard::readLayout(Common::SeekableReadStream &stream, LayoutDef &layout) {
	int vertexCount;
	if (!readCount(stream, layout.puzzleLevel, 4) || !readCount(stream, layout.selectionWeight, 32767) ||
		!readCount(stream, vertexCount, 10) || vertexCount == 0)
		return false;
	for (int i = 0; i < vertexCount; i++) {
		if (stream.size() - stream.pos() < 4)
			return false;
		const int x = stream.readSint16LE();
		const int y = stream.readSint16LE();
		layout.vertexCoords.push_back(Common::Point32(x, y));
	}
	for (int choiceGroup = 0; choiceGroup < ARRAYSIZE(layout.ruleSetCandidates); choiceGroup++) {
		int candidateCount;
		if (!readCount(stream, candidateCount, 32767) || candidateCount == 0)
			return false;
		for (int i = 0; i < candidateCount; i++) {
			int ruleCount;
			if (!readCount(stream, ruleCount, 5))
				return false;
			RuleSet ruleSet;
			for (int j = 0; j < ruleCount; j++) {
				int edgeCount;
				if (!readCount(stream, edgeCount, 100))
					return false;
				Rule rule;
				for (int k = 0; k < edgeCount; k++) {
					if (stream.size() - stream.pos() < 6)
						return false;
					Edge edge;
					edge.from = stream.readSint16LE();
					edge.to = stream.readSint16LE();
					const int kind = stream.readSint16LE();
					if (edge.from < 0 || vertexCount <= edge.from || edge.to < 0 || vertexCount <= edge.to || kind < 0 || 1 < kind)
						return false;
					edge.swap = kind == 1;
					rule.push_back(edge);
				}
				ruleSet.push_back(rule);
			}
			layout.ruleSetCandidates[choiceGroup].push_back(ruleSet);
		}
	}
	return !stream.err();
}

bool MagicWallBoard::load(Common::SeekableReadStream &stream) {
	_layouts.clear();
	_layoutIndex = -1;
	int count;
	if (!readCount(stream, count, 32767))
		return false;
	for (int i = 0; i < count; i++) {
		LayoutDef layout;
		if (!readLayout(stream, layout)) {
			warning("MagicWall: incomplete layout %d; retaining %u complete layouts", i, _layouts.size());
			break;
		}
		_layouts.push_back(layout);
	}
	return !_layouts.empty();
}

bool MagicWallBoard::isIdentity(const Common::Array<int> &positions) {
	for (uint i = 0; i < positions.size(); i++) {
		if (positions[i] != static_cast<int>(i))
			return false;
	}
	return true;
}

void MagicWallBoard::applyRule(const Rule &rule, Common::Array<int> &positions, bool simultaneous) {
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

void MagicWallBoard::scramble(Random &random) {
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

int MagicWallBoard::solve(const Common::Array<int> &positions, int depth) const {
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

bool MagicWallBoard::generate(int puzzleLevel, Random &random) {
	_generationSequence.clear();
	_initialPositions.clear();
	int totalWeight = 0;
	for (const LayoutDef &layout : _layouts) {
		if (layout.puzzleLevel == puzzleLevel)
			totalWeight += layout.selectionWeight;
	}
	if (totalWeight == 0)
		return false;
	int choice = random.getRandomNumber(totalWeight - 1);
	for (uint i = 0; i < _layouts.size(); i++) {
		if (_layouts[i].puzzleLevel != puzzleLevel)
			continue;
		if (choice < _layouts[i].selectionWeight) {
			_layoutIndex = i;
			break;
		}
		choice -= _layouts[i].selectionWeight;
	}
	const LayoutDef &selectedLayout = layout();
	_rules.clear();
	for (int choiceGroup = 0; choiceGroup < ARRAYSIZE(selectedLayout.ruleSetCandidates); choiceGroup++) {
		const Common::Array<RuleSet> &candidates = selectedLayout.ruleSetCandidates[choiceGroup];
		const int candidateIndex = random.getRandomNumber(candidates.size() - 1);
		const RuleSet &selectedRuleSet = candidates[candidateIndex];
		if (!selectedRuleSet.empty() && !selectedRuleSet[0].empty()) {
			for (const Rule &rule : selectedRuleSet) {
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
	_positions.resize(selectedLayout.vertexCoords.size());
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

void MagicWallBoard::apply(int ruleIndex) {
	if (0 <= ruleIndex && ruleIndex < static_cast<int>(_rules.size()))
		applyRule(_rules[ruleIndex], _positions, true);
}

bool MagicWallBoard::matched(VertexColor vertexColor) const {
	return _positions.size() <= vertexColor || _positions[vertexColor] == static_cast<int>(vertexColor);
}

bool MagicWallBoard::gateMatched(int gate) const {
	return matched(static_cast<VertexColor>(gate)) &&
		   matched(static_cast<VertexColor>(gate + 4)) &&
		   matched(static_cast<VertexColor>(gate + 8));
}

bool MagicWallBoard::debugEncodePositions(const Common::Array<int> &positions, uint64 &state) {
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

uint MagicWallBoard::debugPermutationRank(uint64 state, uint count) {
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

Common::Array<int> MagicWallBoard::debugRuleMapping(uint rule) const {
	Common::Array<int> mapping;
	for (uint i = 0; i < _positions.size(); i++)
		mapping.push_back(i);
	applyRule(_rules[rule], mapping, true);
	return mapping;
}

Common::Array<Common::Array<int>> MagicWallBoard::debugTabletPaths(uint rule) const {
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

bool MagicWallBoard::debugCrossesGroup(const Common::Array<int> &mapping, uint group) {
	for (uint point = 0; point < mapping.size(); point++)
		if (static_cast<bool>(group & (1U << point)) != static_cast<bool>(group & (1U << mapping[point])))
			return true;
	return false;
}

const char *MagicWallBoard::debugTabletRole(uint rule) const {
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

MagicWallBoard::SolutionResult MagicWallBoard::debugAnswerSolver(Common::Array<int> &sequence, uint &visited) const {
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

const char *MagicWallBoard::debugLayoutAdvice() const {
	static constexpr const char *kAdvice[] = {
		"- The vertices form two rows. Each tablet swaps the beetles at the two ends of one edge.\n"
		"    - To swap distant beetles and restore intermediate ones, press tablets along a path, "
		"then retrace it without repeating the final edge.",
		"- The layout has five vertices. Each tablet swaps the beetles at the two ends of one edge.\n"
		"    - Press tablets along a path, then retrace it without repeating the final edge "
		"to swap endpoint beetles and restore intermediate ones.",
		"- The layout has a four-vertex cycle on the left and a three-vertex exchange group on the right.\n"
		"    - Rotate a misplaced left beetle to the bridge, "
		"swap it with the right group, then undo the rotation.\n"
		"    - Use right-side swaps to choose the returning beetle.",
		"- The layout has two lower triangles, each with an upper holding vertex.\n"
		"    - Rotate each triangle to bring a beetle to its upper exchange vertex.\n"
		"    - The upper vertices bridge the groups.\n"
		"    - Move beetles to the correct side, then sort each triangle and its holding vertex.",
		"- The layout has a five-vertex outer cycle and a central holding vertex.\n"
		"    - Rotate an outer beetle to the central exchange, swap, then undo the rotation.\n"
		"    - Use the center as a buffer to reorder the ring; restore the center's beetle once the ring is sorted.",
		"- The layout has a five-vertex ring and a tablet that swaps a pair on the ring.\n"
		"    - Rotate the ring to choose the beetles to swap. Swaps change their circular order.\n"
		"    - Undo the rotation after each correction, then align the sorted ring with its colored vertices.",
		"- The layout has an inner square and outer holding vertices. Rotate the square to reach exchanges, "
		"use outer vertices as buffers, then undo the rotation.\n"
		"    - Some variants leave an outer vertex untouched; preserve it while sorting the connected group.",
		"- The layout has two three-vertex cycles connected by a pair exchange.\n"
		"    - Rotate both cycles to bring the desired pair to the bridge, swap, then undo the rotations.\n"
		"    - Change group membership and circular order this way, then align both cycles.",
		"- The layout has inner and outer four-vertex cycles. Rotate each ring independently to the bridge.\n"
		"    - Swap a pair, then undo the rotations to restore other beetles.\n"
		"    - Correct group membership and circular order before final alignment.",
		"- The layout has a five-vertex ring with an extra exchange or partial cycle. Rotating the whole ring only changes alignment.\n"
		"    - Rotate misplaced beetles onto the smaller operation's vertices, apply it, then rotate back.\n"
		"    - Account for every cycle on that tablet.",
		"- The layout has a seven-vertex ring and an operation that moves a smaller group.\n"
		"    - Rotate the desired beetles to that operation's exchange or partial-cycle vertices, "
		"apply it, then rotate back.\n"
		"    - Rotating the whole ring alone cannot change relative order.",
		"- The layout has two triangles. Each of the first two tablets rotates one triangle and swaps a separate pair at the same time.\n"
		"    - Press one of these tablets three times to swap only the pair, or twice to rotate only its triangle in reverse.\n"
		"    - Use these actions to transfer and sort beetles.",
		"- Tablet 1 rotates a four-vertex square and a three-vertex triangle at the same time.\n"
		"    - Press tablet 1 three times to move only the square, or four times to move only the triangle.\n"
		"    - Use other tablets to transfer beetles between the groups and correct their order.\n"
		"    - Pressing tablet 1 twelve times restores both groups.",
		"- The layout has a five-vertex ring with partial cycles and exchanges.\n"
		"    - Rotate the ring to position a correction, apply a smaller operation, then rotate back.\n"
		"    - This layout is not selected in normal play.",
		"- The layout has nested squares. One tablet rotates a ring; another swaps all four pairs between the rings.\n"
		"    - Swap rings, rotate, then swap back to act on the other ring.\n"
		"    - Use the small pair exchange to correct relative order, then undo positioning moves.",
		"- The layout has three overlapping triangles. Their shared vertices let beetles move between cycles.\n"
		"    - Rotate a beetle to a shared vertex, move it into the next triangle, then undo the first rotation.\n"
		"    - The extra swap changes the order that three-cycles alone preserve.",
		"- The vertices form a three-by-three grid with overlapping square rotations.\n"
		"    - Pass beetles between squares through their shared middle vertices.\n"
		"    - Position a local correction, apply it, then undo positioning moves; matched beetles may need to move temporarily.",
		"- Tablet 1 rotates a four-vertex square and a five-vertex ring at the same time.\n"
		"    - Press tablet 1 five times to move only the square, or four times to move only the five-vertex ring.\n"
		"    - Use other tablets to transfer beetles between the groups.\n"
		"    - Pressing tablet 1 twenty times restores both groups.",
		"- Tablet 1 rotates a five-vertex ring and a three-vertex triangle at the same time.\n"
		"    - Press tablet 1 three times to move only the ring, or five times to move only the triangle.\n"
		"    - Use other operations to transfer beetles and reorder them.\n"
		"    - Pressing tablet 1 fifteen times restores both groups.",
		"- The vertices form a three-by-three grid. Swapping whole rows or columns preserves the row/column structure; "
		"use the extra local operation for arbitrary corrections.\n"
		"    - Position a correction for the local operation, apply it, then undo the row/column swaps.",
		"- The layout is a cube with three tablets that flip pairs across the whole cube. Press the same tablet again to undo its flips.\n"
		"    - Use flips to bring a pair to the local exchange, swap, then undo the flips.\n"
		"    - If a tablet combines a three-cycle and a separate swap, press it twice to isolate the reverse three-cycle, "
		"or three times to isolate the swap.",
		"- The layout has three triangles. Tablet 1 rotates all three at the same time.\n"
		"    - Pressing tablet 1 three times restores all groups, so repeating that tablet cannot move just one group.\n"
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
	delete _vertexSurface;
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
	const VertexColorNameMap &vertexColorNames = getVertexColorNames();
	Common::Array<MagicWallBoard::VertexColor> vertexColors;
	for (VertexColorNameMap::const_iterator entry = vertexColorNames.begin(); entry != vertexColorNames.end(); entry++)
		vertexColors.push_back(entry->_key);
	// Preserve ordinal load order so page-resource handles stay stable.
	Common::sort(vertexColors.begin(), vertexColors.end());
	for (MagicWallBoard::VertexColor vertexColor : vertexColors) {
		gfx->loadPageRleBlock(Common::String::format(kDotFormat, vertexColorNames[vertexColor]));
		gfx->loadPageRleBlock(Common::String::format(kBugFormat, vertexColorNames[vertexColor]));
	}
	for (MagicWallBoard::VertexColor vertexColor : vertexColors)
		gfx->loadPageRleBlock(Common::String::format(kLightFormat, vertexColorNames[vertexColor]));
	gfx->loadPageRleBlock(kStoneLightPath);
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
	const bool loaded = stream && _board.load(*stream);
	delete stream;
	if (!loaded)
		error("MagicWall: no bounded puzzle layouts in %s", kLayoutPath);
	newPuzzle();
	if (_vm->inJourneyMode())
		_vm->_state->registerPageVisit(_pageId, 1);
}

void PuzzleMagicWall::newPuzzle() {
	_ready = _board.generate(_puzzleLevel, *_vm->_rnd);
	if (!_ready)
		error("MagicWall: no usable layout for level %d", _puzzleLevel);
	for (Beetle &beetle : _beetles)
		delete beetle.path;
	_beetles.clear();
	_beetles.resize(_board.positions().size());
	for (uint i = 0; i < _board.positions().size(); i++) {
		_beetles[i].position = vertexPosition(_board.positions()[i]) + Common::Point32(5, 8);
		_beetles[i].direction = 0;
	}
	_movingRule = -1;
	_hoveredTablet = -1;
	rebuildVertexSurface();
	debug(1, "MagicWall: phase=%d level=%d layout=%d points=%u tablets=%u", static_cast<int>(_phase), _puzzleLevel,
		  _board.layoutIndex(), _board.positions().size(), _board.rules().size());
	for (uint i = 0; i < _board.positions().size(); i++)
		debug(2, "MagicWall: beetle %u at %d", i, _board.positions()[i]);
	for (uint i = 0; i < _board.rules().size(); i++) {
		for (const MagicWallBoard::Edge &edge : _board.rules()[i])
			debug(2, "MagicWall: rule %u edge %d %d %d", i, edge.from, edge.to, edge.swap);
	}
}

Common::Point32 PuzzleMagicWall::vertexPosition(int index) const {
	const Common::Point32 &point = _board.layout().vertexCoords[index];
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
	const Common::Array<int> before = _board.positions();
	_board.apply(index);
	const uint32 now = _vm->getTotalPlayTime();
	for (uint i = 0; i < before.size(); i++) {
		if (before[i] == _board.positions()[i])
			continue;
		Beetle &beetle = _beetles[i];
		const Common::Point32 end = vertexPosition(_board.positions()[i]);
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
		if (!_board.gateMatched(gate))
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

void PuzzleMagicWall::rebuildVertexSurface() {
	if (!_vertexSurface)
		_vertexSurface = _vm->_gfx->createSurface(Gfx::kScreenSize);
	const uint32 transparentKey = _vertexSurface->format.ARGBToColor(0, 0, 0, 0);
	_vertexSurface->fillRect(Common::Rect32(0, 0, _vertexSurface->w, _vertexSurface->h), transparentKey);
	for (uint i = 0; i < _board.positions().size(); i++) {
		const MagicWallBoard::VertexColor vertexColor = static_cast<MagicWallBoard::VertexColor>(i);
		const Common::Point32 pos = vertexPosition(i);
		// Keep the background under each disc to mask connections and retain the RLE edge blend.
		_vm->_gfx->drawBackgroundSubRect(_vertexSurface, pos, Common::Rect(pos.x, pos.y, pos.x + 50, pos.y + 50));
		drawVertexColorSprite(_vertexSurface, kDotFormat, vertexColor, pos, kDotSourceBrightness[vertexColor]);
	}
	_vertexSurfaceColorAssistMode = _vm->getColorAssistMode();
}

void PuzzleMagicWall::drawVertices(ManagedSurface32 *screen) {
	if (!_vertexSurface || _vertexSurfaceColorAssistMode != _vm->getColorAssistMode())
		rebuildVertexSurface();
	// The sentinel disables color-key matching; the surface alpha channel controls transparency.
	const uint32 transBlitKey = screen->format.ARGBToColor(255, 255, 255, 255);
	screen->transBlitFrom(*_vertexSurface, transBlitKey, false, 255);
}

Common::Point32 PuzzleMagicWall::tabletPoint(int tablet, int point, int inset) const {
	const Common::Point32 &p = _board.layout().vertexCoords[point];
	const int firstX = _board.layout().vertexCoords[0].x;
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
	for (uint i = 0; i < _board.rules().size(); i++) {
		_vm->_gfx->drawPageRleBlock(screen, kTabletPath, kTabletPositions[i] - Common::Point32(5, 5));
		if (_nextPuzzlePending || _phase == Phase::kFinished)
			continue;
		for (const MagicWallBoard::Edge &edge : _board.rules()[i])
			drawRipple(screen, tabletPoint(i, edge.from, 12), tabletPoint(i, edge.to, 12), 255);
		for (uint point = 0; point < _board.layout().vertexCoords.size(); point++)
			_vm->_gfx->drawPageRleBlock(screen, kTabletDotPath, tabletPoint(i, point, 10));
	}
}

void PuzzleMagicWall::drawConnections(ManagedSurface32 *screen, int index) {
	if (index < 0 || static_cast<int>(_board.rules().size()) <= index)
		return;
	for (const MagicWallBoard::Edge &edge : _board.rules()[index]) {
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
	for (uint i = 0; i < _board.positions().size(); i++) {
		const MagicWallBoard::VertexColor vertexColor = static_cast<MagicWallBoard::VertexColor>(i);
		drawVertexColorSprite(screen, kBugFormat, vertexColor, _beetles[i].position, kBugSourceBrightness[vertexColor]);
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kDirectionFormat, _beetles[i].direction + 1), _beetles[i].position);
	}
}

PuzzleMagicWall::VertexColorNameMap PuzzleMagicWall::createVertexColorNames() {
	VertexColorNameMap names;
	names[MagicWallBoard::kVertexColor00_Blue] = "blue";
	names[MagicWallBoard::kVertexColor01_Green] = "green";
	names[MagicWallBoard::kVertexColor02_Navy] = "navy";
	names[MagicWallBoard::kVertexColor03_Orange] = "orange";
	names[MagicWallBoard::kVertexColor04_Purple] = "purple";
	names[MagicWallBoard::kVertexColor05_Red] = "red";
	names[MagicWallBoard::kVertexColor06_Rose] = "rose";
	names[MagicWallBoard::kVertexColor07_Turquoise] = "turquoise";
	names[MagicWallBoard::kVertexColor08_Violet] = "violet";
	names[MagicWallBoard::kVertexColor09_Yellow] = "yellow";
	return names;
}

const PuzzleMagicWall::VertexColorNameMap &PuzzleMagicWall::getVertexColorNames() {
	static const VertexColorNameMap names = createVertexColorNames();
	return names;
}

PuzzleMagicWall::VertexColorPalette PuzzleMagicWall::createRedGreenVertexColorPalette() {
	VertexColorPalette palette;
	palette[MagicWallBoard::kVertexColor00_Blue] = RGBColor(0, 64, 255);
	palette[MagicWallBoard::kVertexColor01_Green] = RGBColor(65, 217, 141);
	palette[MagicWallBoard::kVertexColor02_Navy] = RGBColor(42, 91, 140);
	palette[MagicWallBoard::kVertexColor03_Orange] = RGBColor(178, 89, 0);
	palette[MagicWallBoard::kVertexColor04_Purple] = RGBColor(115, 115, 115);
	palette[MagicWallBoard::kVertexColor05_Red] = RGBColor(100, 40, 32);
	palette[MagicWallBoard::kVertexColor06_Rose] = RGBColor(0, 217, 217);
	palette[MagicWallBoard::kVertexColor07_Turquoise] = RGBColor(0, 191, 255);
	palette[MagicWallBoard::kVertexColor08_Violet] = RGBColor(0, 108, 217);
	palette[MagicWallBoard::kVertexColor09_Yellow] = RGBColor(217, 217, 65);
	return palette;
}

bool PuzzleMagicWall::vertexColorAssistRGB(ColorAssistMode mode, MagicWallBoard::VertexColor vertexColor, RGBColor &displayColor) {
	switch (mode) {
	case ColorAssistMode::kOriginal:
	default:
		return false;
	case ColorAssistMode::kEnhancedDistinction:
		if (vertexColor == MagicWallBoard::kVertexColor06_Rose) {
			displayColor = RGBColor(240, 32, 205);
			return true;
		}
		if (vertexColor == MagicWallBoard::kVertexColor07_Turquoise) {
			displayColor = RGBColor(178, 255, 212);
			return true;
		}
		return false;
	case ColorAssistMode::kRedGreenBlindAssist: {
		static const VertexColorPalette palette = createRedGreenVertexColorPalette();
		const VertexColorPalette::const_iterator entry = palette.find(vertexColor);
		if (entry == palette.end())
			return false;
		displayColor = entry->_value;
		return true;
	}
	}
}

void PuzzleMagicWall::drawVertexColorSprite(ManagedSurface32 *screen, const char *format, MagicWallBoard::VertexColor vertexColor,
											const Common::Point32 &pos, byte sourceBrightness) const {
	const VertexColorNameMap &vertexColorNames = getVertexColorNames();
	const VertexColorNameMap::const_iterator entry = vertexColorNames.find(vertexColor);
	if (entry == vertexColorNames.end()) {
		warning("MagicWall: no resource suffix for vertex color %u", static_cast<uint>(vertexColor));
		return;
	}
	const Common::String key = Common::String::format(format, entry->_value);
	RGBColor displayColor;
	if (vertexColorAssistRGB(_vm->getColorAssistMode(), vertexColor, displayColor)) {
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
		drawVertices(screen);
		drawBeetles(screen);
	}
	bool allMatched = true;
	for (uint i = 0; i < _board.positions().size(); i++) {
		const MagicWallBoard::VertexColor vertexColor = static_cast<MagicWallBoard::VertexColor>(i);
		const bool lit = _phase != Phase::kFinished && _board.matched(vertexColor);
		if (lit)
			drawVertexColorSprite(screen, kLightFormat, vertexColor, kLightPositions[i] - Common::Point32(6, 5), 255);
		else
			_vm->_gfx->drawPageRleBlock(screen, kStoneLightPath, kLightPositions[i] - Common::Point32(6, 5));
		if (!_board.matched(vertexColor))
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
	for (uint i = 0; i < _board.rules().size(); i++) {
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
	for (uint i = 0; i < _board.layouts().size(); i++) {
		const MagicWallBoard::LayoutDef &layout = _board.layouts()[i];
		if (layout.puzzleLevel != _board.layout().puzzleLevel || layout.selectionWeight == 0)
			continue;
		layoutCount += 1;
		if (static_cast<int>(i) == _board.layoutIndex())
			layoutNumber = layoutCount;
	}
	answer += Common::String::format("\n  [*] Layout (%02u/%02u) strategy\n    %s\n", layoutNumber, layoutCount, _board.debugLayoutAdvice());
	if (_phase == Phase::kFinished)
		return answer + "  Both boards are complete; use Go.\n";
	if (_nextPuzzlePending || gatesActive() || runnersMoving() || beetlesMoving())
		return answer + "  Wait for the current movement or board change, then request the answer again.\n";
	Common::Array<int> sequence;
	uint visited = 0;
	const MagicWallBoard::SolutionResult result = _board.debugAnswerSolver(sequence, visited);
	switch (result) {
	case MagicWallBoard::SolutionResult::kSolved:
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
	case MagicWallBoard::SolutionResult::kUnreachable:
		answer += "  No solution is reachable from this board setup.\n";
		break;
	case MagicWallBoard::SolutionResult::kLimit:
		answer += "  The search reached its budget limit; this does not prove the board unsolvable.\n";
		break;
	default:
		return answer + "  The board or a tablet is not a valid permutation; no solution can be certified.\n";
	}
	answer += "\n  [*] Tablet Paths (left-to-right):\n";
	uint pathIndex = 0;
	for (uint rule = 0; rule < _board.rules().size(); rule++) {
		answer += Common::String::format("    Tablet %u: %s\n", rule + 1, _board.debugTabletRole(rule));
		for (const Common::Array<int> &path : _board.debugTabletPaths(rule)) {
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
