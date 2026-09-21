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

#include "zoombini2/pages/puzzle_mysticmarsh.h"
#include "common/debug.h"
#include "zoombini2/graphics.h"
#include "zoombini2/random.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

int MysticMarshGrid::randomBelow(int count) {
	assert(0 < count);
	return _random->getRandomNumber(count - 1);
}

int MysticMarshGrid::trait(int zoombini, int feature) const {
	if (zoombini < 0 || static_cast<int>(_party.size()) <= zoombini || feature < 1 || 4 < feature)
		return 0;
	return _party[zoombini].getValue(static_cast<ZmbTrait::TraitIndex>(feature - 1));
}

bool MysticMarshGrid::matches(int zoombini, Feature feature) const {
	return trait(zoombini, feature.trait) == feature.value;
}

int MysticMarshGrid::background() const {
	if (_layout < 2)
		return _layout + 1;
	if (_layout < 6)
		return 3;
	return _layout - 2;
}

void MysticMarshGrid::loadLayout(int layout) {
	_layout = layout;
	for (int col = 0; col < kColumns; col++) {
		for (int row = 0; row < kRows; row++)
			_cells[index(col, row)] = kCellTemplates[kLayouts[layout][row][col]];
	}
}

void MysticMarshGrid::clearCell(int column, int row) {
	Cell &target = _cells[index(column, row)];
	target.type = 1;
	target.direction = 4;
	target.trigger = 0;
	target.trait = 0;
	target.value = 0;
}

void MysticMarshGrid::setFeature(int column, int row, Feature feature) {
	Cell &target = _cells[index(column, row)];
	target.trait = feature.trait;
	target.value = feature.value;
}

void MysticMarshGrid::tally(const Common::Array<int> &group, int counts[5][6]) const {
	memset(counts, 0, 5 * 6 * sizeof(int));
	for (uint i = 0; i < group.size(); i++) {
		for (int feature = 1; feature <= 4; feature++)
			counts[feature][trait(group[i], feature)] += 1;
	}
}

MysticMarshGrid::Feature MysticMarshGrid::singleton(const Common::Array<int> &group) const {
	int counts[5][6];
	tally(group, counts);
	Feature result;
	for (int f = 1; f <= 4; f++) {
		for (int v = 1; v <= 5; v++) {
			if (counts[f][v] == 1)
				result = Feature(f, v);
		}
	}
	return result;
}

MysticMarshGrid::Feature MysticMarshGrid::popular(const Common::Array<int> &group, int limit) const {
	int counts[5][6];
	tally(group, counts);
	int best = 0;
	Feature result;
	for (int f = 1; f <= 4; f++) {
		for (int v = 1; v <= 5; v++) {
			if (best < counts[f][v] && counts[f][v] < limit) {
				best = counts[f][v];
				result = Feature(f, v);
			}
		}
	}
	return result;
}

MysticMarshGrid::Feature MysticMarshGrid::unusedFeature(const Common::Array<int> &group) {
	int counts[5][6];
	tally(group, counts);
	bool available[5][6] = {};
	bool any = false;
	for (uint i = 0; i < _party.size(); i++) {
		for (int f = 1; f <= 4; f++) {
			const int v = trait(i, f);
			if (counts[f][v] == 0) {
				available[f][v] = true;
				any = true;
			}
		}
	}
	if (!any)
		return Feature();
	Feature result;
	do {
		result.trait = randomBelow(4) + 1;
		result.value = randomBelow(5) + 1;
	} while (!available[result.trait][result.value]);
	return result;
}

int MysticMarshGrid::pick(const Common::Array<int> &candidates, Common::Array<int> &used) {
	bool any = false;
	for (uint i = 0; i < candidates.size(); i++) {
		if (used[candidates[i]] == 0)
			any = true;
	}
	if (!any)
		return -1;
	int selected;
	do {
		selected = candidates[randomBelow(candidates.size())];
	} while (used[selected] != 0);
	used[selected] = 1;
	return selected;
}

int MysticMarshGrid::select(Common::Array<int> &used, Feature a, bool matchA, Feature b, bool matchB, Feature c, bool matchC) {
	Common::Array<int> candidates;
	for (uint i = 0; i < _party.size(); i++) {
		if (matches(i, a) == matchA && (b.trait == 0 || matches(i, b) == matchB) && (c.trait == 0 || matches(i, c) == matchC))
			candidates.push_back(i);
	}
	return pick(candidates, used);
}

void MysticMarshGrid::init(const Common::Array<ZmbTrait> &party, int difficulty, Random &randomSource) {
	_party = party;
	_random = &randomSource;
	_generation = GenerationInfo();
	_moved.resize(party.size());
	for (int i = 0; i < kCellCount; i++) {
		_before[i] = Occupant();
		_after[i] = Occupant();
	}
	if (party.empty()) {
		loadLayout(0);
		memcpy(_initialCells, _cells, sizeof(_cells));
		return;
	}
	if (difficulty == 1) {
		const int layout = randomBelow(2);
		Cell best[kCellCount];
		GenerationInfo bestGeneration;
		int bestScore = -1;
		for (int attempt = 0; attempt < 30; attempt++) {
			const int score = generateEasy(layout);
			if (score == 12) {
				memcpy(_initialCells, _cells, sizeof(_cells));
				return;
			}
			if (attempt == 0 || bestScore < score) {
				bestScore = score;
				memcpy(best, _cells, sizeof(best));
				bestGeneration = _generation;
			}
		}
		memcpy(_cells, best, sizeof(best));
		_generation = bestGeneration;
	} else if (difficulty == 2) {
		generateMedium();
	} else {
		generateHard();
	}
	memcpy(_initialCells, _cells, sizeof(_cells));
}

int MysticMarshGrid::generateEasy(int layout) {
	_generation = GenerationInfo();
	loadLayout(layout);
	Common::Array<int> used;
	used.resize(_party.size(), 0);
	Common::Array<int> lower;
	Common::Array<int> upper;
	// The original leaves fifth-value entries and the cross-filter flag uninitialized.
	// Use false for those indeterminate values instead of reading unrelated stack memory.
	bool available[5][6] = {};
	for (uint i = 0; i < _party.size(); i++) {
		for (int f = 1; f <= 4; f++) {
			if (trait(i, f) < 5)
				available[f][trait(i, f)] = true;
		}
	}
	Feature upperFilter;
	Feature lowerFilter;
	int score = 10;
	for (;;) {
		bool any = false;
		for (int f = 1; f <= 4; f++) {
			for (int v = 1; v <= 5; v++)
				any = any || available[f][v];
		}
		if (!any) {
			score -= 1;
			break;
		}
		do {
			upperFilter.trait = randomBelow(4) + 1;
			upperFilter.value = randomBelow(5) + 1;
		} while (!available[upperFilter.trait][upperFilter.value]);
		available[upperFilter.trait][upperFilter.value] = false;
		any = false;
		for (int f = 1; f <= 4; f++) {
			for (int v = 1; v <= 5; v++)
				any = any || available[f][v];
		}
		if (!any) {
			score -= 1;
			break;
		}
		do {
			lowerFilter.trait = randomBelow(4) + 1;
			lowerFilter.value = randomBelow(5) + 1;
		} while (!available[lowerFilter.trait][lowerFilter.value]);
		bool overlap = false;
		for (uint i = 0; i < _party.size(); i++)
			overlap = overlap || (matches(i, upperFilter) && matches(i, lowerFilter));
		if (!overlap)
			break;
	}
	Feature upperCross;
	Feature lowerCross;
	bool distinct = false;
	do {
		upperCross.trait = randomBelow(4) + 1;
		upperCross.value = randomBelow(5) + 1;
		if (upperCross.trait != upperFilter.trait && upperCross.trait != lowerFilter.trait)
			distinct = true;
	} while (upperCross.value == lowerFilter.value && !distinct);
	lowerCross.trait = randomBelow(4) + 1;
	lowerCross.value = randomBelow(5) + 1;
	int turn = randomBelow(2);
	const int shift = layout == 0 ? 0 : -1;
	Cell &entry = _cells[index(3 + shift, 5)];
	entry.type = 12 + turn;
	entry.direction = 2 + turn;
	entry.state = 1;
	int assigned = 0;
	int stalled = 0;
	while (assigned < static_cast<int>(_party.size())) {
		int upperToLower = 0;
		int lowerToUpper = 0;
		for (uint i = 0; i < upper.size(); i++) {
			if (matches(upper[i], lowerCross))
				upperToLower += 1;
		}
		for (uint i = 0; i < lower.size(); i++) {
			if (matches(lower[i], upperCross) && !matches(lower[i], lowerCross))
				lowerToUpper += 1;
		}
		int selected = -1;
		if (turn == 0) {
			selected = select(used, lowerFilter, true, upperCross, true, lowerCross, false);
			if (selected < 0)
				selected = select(used, lowerFilter, true);
			if (selected < 0 && lowerToUpper < upperToLower)
				selected = select(used, upperCross, true, lowerCross, false);
			if (selected < 0)
				selected = select(used, upperFilter, false);
			if (0 <= selected)
				lower.push_back(selected);
		} else {
			selected = select(used, upperFilter, true, lowerCross, true);
			if (selected < 0)
				selected = select(used, upperFilter, true);
			if (selected < 0 && upperToLower < lowerToUpper)
				selected = select(used, upperCross, true, lowerCross, false);
			if (selected < 0)
				selected = select(used, lowerFilter, false);
			if (0 <= selected)
				upper.push_back(selected);
		}
		if (selected < 0) {
			// A repeated filter cannot make progress for this party; retry the candidate layout.
			if (2 <= stalled)
				return -1;
			stalled += 1;
			Feature replacement;
			for (int f = 1; f <= 4; f++) {
				for (int v = 1; v <= 5; v++) {
					for (uint i = 0; i < _party.size(); i++) {
						if (trait(i, f) == v)
							replacement = Feature(f, v);
					}
				}
			}
			if (turn == 0)
				upperFilter = replacement;
			else
				lowerFilter = replacement;
		} else {
			_generation.selectionOrder.push_back(selected);
			assigned += 1;
			stalled = 0;
		}
		turn = 1 - turn;
	}
	_generation.upper = upper;
	_generation.lower = lower;
	Common::Array<int> lowerArrivals;
	int lowerToUpper = 0;
	int upperToLower = 0;
	for (uint i = 0; i < lower.size(); i++) {
		if (matches(lower[i], upperCross) && !matches(lower[i], lowerCross))
			lowerToUpper += 1;
		else
			lowerArrivals.push_back(lower[i]);
	}
	for (uint i = 0; i < upper.size(); i++) {
		if (matches(upper[i], lowerCross)) {
			upperToLower += 1;
			lowerArrivals.push_back(upper[i]);
		}
	}
	const Feature special = singleton(lowerArrivals);
	setFeature(4 + shift, 3, upperFilter);
	setFeature(4 + shift, 7, lowerFilter);
	setFeature(5 + shift, 3, upperCross);
	setFeature(6 + shift, 7, lowerCross);
	setFeature(7 + shift, 3, special);
	if (upperFilter.trait == 0) {
		clearCell(4 + shift, 3);
		score -= 2;
	}
	if (lowerFilter.trait == 0) {
		clearCell(4 + shift, 7);
		score -= 2;
	}
	if (special.trait == 0) {
		_cells[index(7 + shift, 3)].type = 19;
		_cells[index(7 + shift, 3)].direction = 3;
		score -= 2;
	}
	const int lowerCount = lower.size() + upperToLower - lowerToUpper;
	const int upperCount = upper.size() + lowerToUpper - upperToLower;
	if (layout == 0) {
		if (4 < lowerCount || lowerCount <= 3)
			clearCell(9, 3);
		if (4 < lowerCount || lowerCount <= 2) {
			clearCell(10, 3);
			clearCell(12, 4);
		}
		if (4 < lowerCount || lowerCount <= 1) {
			clearCell(11, 3);
			clearCell(8, 5);
		}
		if (4 < upperCount || upperCount <= 3)
			clearCell(9, 7);
		if (4 < upperCount || upperCount <= 2)
			clearCell(10, 7);
		if (4 < upperCount || upperCount <= 1) {
			clearCell(11, 7);
			clearCell(12, 6);
		}
	} else {
		if (4 < lowerCount) {
			clearCell(13, 3);
			_cells[index(13, 3)].type = 5;
			_cells[index(13, 3)].direction = 3;
		}
		if (lowerCount <= 3)
			clearCell(9, 3);
		if (lowerCount <= 2)
			clearCell(11, 3);
		if (lowerCount <= 1) {
			clearCell(12, 3);
			clearCell(10, 3);
			clearCell(7, 5);
		}
		if (4 < upperCount) {
			clearCell(12, 7);
			_cells[index(12, 7)].type = 4;
			_cells[index(12, 7)].direction = 2;
		}
		if (upperCount <= 3)
			clearCell(9, 7);
		if (upperCount <= 2)
			clearCell(10, 7);
		if (upperCount <= 1)
			clearCell(11, 7);
	}
	if (4 < lowerCount)
		score -= 2;
	if (4 < upperCount)
		score -= 2;
	if (upperCount == lowerCount)
		score += 2;
	return score;
}

void MysticMarshGrid::generateMedium() {
	const int count = _party.size();
	const int half = (count + 1) / 2;
	const int quarter = (count - half) / 2;
	const int eighth = (half - quarter) / 2;
	Common::Array<int> all;
	Common::Array<int> used;
	for (int i = 0; i < count; i++) {
		all.push_back(i);
		used.push_back(0);
	}
	const Feature first = popular(all, half);
	Common::Array<int> group;
	for (int i = 0; i < count; i++) {
		if (matches(i, first)) {
			group.push_back(i);
			used[i] = 1;
		}
	}
	while (static_cast<int>(group.size()) < half) {
		const int selected = randomBelow(count);
		if (used[selected] == 0) {
			used[selected] = 1;
			group.push_back(selected);
		}
	}
	// Subsequent selections inspect exactly the first half of the group.
	group.resize(half);
	Feature second = popular(group, quarter);
	if (second.trait == 0 || quarter == 0)
		second = unusedFeature(group);
	Common::Array<int> subset;
	used.clear();
	used.resize(half, 0);
	for (int i = 0; i < half; i++) {
		if (matches(group[i], second) && static_cast<int>(subset.size()) < quarter) {
			subset.push_back(i);
			used[i] = 1;
		}
	}
	while (static_cast<int>(subset.size()) < quarter) {
		const int selected = randomBelow(half);
		if (used[selected] == 0)
			subset.push_back(selected);
	}
	// The nested selection retains indices into the first group.
	Feature third = popular(subset, eighth);
	if (third.trait == 0 || eighth == 0)
		third = unusedFeature(subset);
	_generation.group = group;
	_generation.subset = subset;
	const int variant = randomBelow(4);
	loadLayout(variant + 2);
	setFeature(8, 6, first);
	setFeature(variant < 2 ? 2 : 5, 3, second);
	static constexpr int kThirdColumns[4] = {
		12,
		13,
		10,
		9,
	};
	const int thirdColumn = kThirdColumns[variant];
	const int thirdRow = variant < 2 ? 6 : 3;
	setFeature(thirdColumn, thirdRow, third);
	_cells[index(thirdColumn, thirdRow)].type = 7;
	_cells[index(thirdColumn, thirdRow)].direction = 1;
	_cells[index(4, 7)].type = 14;
	_cells[index(4, 7)].direction = 0;
	_cells[index(4, 7)].state = 1;
	static constexpr int kRemovals[4][6][8] = {
		{
			{13, 4, 13, 5, -1, -1, -1, -1},
			{5, 4, 5, 6, 13, 0, 13, 8},
			{10, 5, 10, 7, -1, -1, -1, -1},
			{10, 1, 10, 8, 2, 4, 2, 6},
			{12, 4, 12, 5, -1, -1, -1, -1},
			{2, 8, 3, 8, 12, 8, 12, 0},
		},
		{
			{12, 0, 12, 4, 12, 5, 12, 8},
			{5, 4, 5, 6, -1, -1, -1, -1},
			{10, 5, 10, 7, -1, -1, -1, -1},
			{10, 1, 10, 8, 2, 4, 2, 6},
			{13, 4, 13, 5, -1, -1, -1, -1},
			{2, 8, 3, 8, 13, 8, 13, 0},
		},
		{
			{13, 4, 13, 5, -1, -1, -1, -1},
			{13, 0, 13, 8, 5, 4, 5, 6},
			{9, 5, 9, 7, -1, -1, -1, -1},
			{2, 4, 2, 6, 9, 1, 9, 8},
			{12, 4, 12, 5, -1, -1, -1, -1},
			{2, 8, 3, 8, 12, 8, 12, 0},
		},
		{
			{13, 4, 13, 5, -1, -1, -1, -1},
			{13, 0, 13, 8, 5, 4, 5, 6},
			{10, 5, 10, 7, -1, -1, -1, -1},
			{2, 4, 2, 6, 10, 1, 10, 8},
			{12, 4, 12, 5, -1, -1, -1, -1},
			{2, 8, 3, 8, 12, 8, 12, 0},
		},
	};
	for (int threshold = 7; 2 <= threshold; threshold -= 1) {
		if (count <= threshold) {
			for (int i = 0; i < 8; i += 2) {
				const int *pair = &kRemovals[variant][7 - threshold][i];
				if (0 <= pair[0])
					clearCell(pair[0], pair[1]);
			}
		}
	}
}

int MysticMarshGrid::pickUnique(Common::Array<int> &used) {
	const int selected = randomBelow(_party.size());
	if (used[selected] != 0)
		return -1;
	for (uint i = 0; i < _party.size(); i++) {
		if (static_cast<int>(i) != selected && _party[i] == _party[selected]) {
			used[selected] = 2;
			return -1;
		}
	}
	used[selected] = 1;
	return selected;
}

int MysticMarshGrid::pickDifferent(int first, Common::Array<int> &used, int minimum) {
	if (first < 0)
		return -1;
	Common::Array<int> candidates;
	for (uint i = 0; i < _party.size(); i++) {
		int differences = 0;
		for (int f = 1; f <= 4; f++) {
			if (trait(i, f) != trait(first, f))
				differences += 1;
		}
		if (used[i] == 0 && minimum <= differences)
			candidates.push_back(i);
	}
	return pick(candidates, used);
}

void MysticMarshGrid::generateHard() {
	Common::Array<int> used;
	Common::Array<int> tried;
	Common::Array<int> visited;
	used.resize(_party.size(), 0);
	tried.resize(_party.size(), 0);
	visited.resize(_party.size(), 0);
	int first = -1;
	int second = -1;
	int feature[4] = {};
	int values[4][5] = {};
	int bestValues[4] = {};
	int best = 50;
	int bestActors[2] = {
		-1,
		-1,
	};
	for (;;) {
		for (uint i = 0; i < _party.size(); i++) {
			used[i] = 0;
			tried[i] = 0;
		}
		bool exhausted = false;
		do {
			first = pickUnique(tried);
			second = pickDifferent(first, used, 2);
			exhausted = true;
			for (uint i = 0; i < tried.size(); i++)
				exhausted = exhausted && tried[i] != 0;
		} while (!exhausted && second < 0);
		for (uint i = 0; i < tried.size(); i++)
			tried[i] = 0;
		if (exhausted && second < 0) {
			do {
				first = pickUnique(tried);
				second = pickDifferent(first, used, 1);
				exhausted = true;
				for (uint i = 0; i < tried.size(); i++)
					exhausted = exhausted && tried[i] != 0;
			} while (!exhausted && second < 0);
		}
		if (second < 0) {
			first = randomBelow(_party.size());
			second = randomBelow(_party.size());
			feature[0] = randomBelow(4) + 1;
			feature[1] = randomBelow(4) + 1;
			values[0][0] = trait(first, feature[0]);
			values[1][0] = values[0][0];
			values[0][1] = trait(second, feature[0]);
			values[1][1] = values[0][1];
			break;
		}
		static constexpr int kTraitOrder[4] = {
			3,
			4,
			1,
			2,
		};
		int found = 0;
		for (int i = 0; i < 4 && found < 2; i++) {
			const int f = kTraitOrder[i];
			if (trait(first, f) != trait(second, f)) {
				feature[found] = f;
				found += 1;
			}
		}
		if (found < 2) {
			do {
				feature[1] = randomBelow(4) + 1;
			} while (feature[1] == feature[0]);
		}
		for (int i = 0; i < 2; i++) {
			values[i][0] = trait(first, feature[i]);
			values[i][1] = trait(second, feature[i]);
		}
		visited[first] = 1;
		visited[second] = 1;
		int crossMatches = 0;
		for (uint i = 0; i < _party.size(); i++) {
			if (trait(i, feature[0]) == values[0][0] && trait(i, feature[1]) == values[1][1])
				crossMatches += 1;
		}
		if (crossMatches == 0)
			break;
		if (crossMatches < best) {
			best = crossMatches;
			bestActors[0] = first;
			bestActors[1] = second;
			bestValues[0] = values[0][0];
			bestValues[1] = values[0][1];
			bestValues[2] = values[1][0];
			bestValues[3] = values[1][1];
		}
		bool allVisited = true;
		for (uint i = 0; i < visited.size(); i++)
			allVisited = allVisited && visited[i] != 0;
		if (allVisited) {
			_generation.restoredFilterValues = true;
			_generation.primaryFilterActors[0] = bestActors[0];
			_generation.primaryFilterActors[1] = bestActors[1];
			values[0][0] = bestValues[0];
			values[0][1] = bestValues[1];
			values[1][0] = bestValues[2];
			values[1][1] = bestValues[3];
			break;
		}
	}
	_generation.referenceActors[0] = first;
	_generation.referenceActors[1] = second;
	if (!_generation.restoredFilterValues) {
		_generation.primaryFilterActors[0] = first;
		_generation.primaryFilterActors[1] = second;
	}
	do {
		feature[2] = randomBelow(4) + 1;
		feature[3] = randomBelow(4) + 1;
	} while (feature[0] == feature[2] || feature[0] == feature[3] || feature[1] == feature[2] ||
			 feature[1] == feature[3] || feature[2] == feature[3]);
	for (int i = 2; i < 4; i++) {
		values[i][0] = trait(first, feature[i]);
		const int other = trait(second, feature[i]);
		if (other != values[i][0])
			values[i][1] = other;
	}
	for (int f = 0; f < 4; f++) {
		if (values[f][0] == 0)
			values[f][0] = randomBelow(5) + 1;
		for (int v = 1; v < 5; v++) {
			if (v == 1 && values[f][v] != 0)
				continue;
			bool distinct;
			do {
				values[f][v] = randomBelow(5) + 1;
				distinct = true;
				for (int previous = 0; previous < v; previous++)
					distinct = distinct && values[f][previous] != values[f][v];
			} while (!distinct);
		}
	}
	loadLayout(randomBelow(2) + 6);
	if (_layout == 6) {
		_cells[index(3, 0)].state = 2;
		_cells[index(2, 7)].state = 1;
	} else {
		_cells[index(1, 9)].state = 1;
		_cells[index(2, 11)].state = 1;
	}
	static constexpr int kHardFeatures[2][18][5] = {
		{
			{12, 4, 6, 0, 0},
			{8, 3, 9, 0, 1},
			{11, 2, 9, 0, 2},
			{13, 4, 6, 0, 3},
			{12, 7, 6, 0, 4},
			{11, 3, 9, 1, 0},
			{8, 2, 8, 1, 1},
			{13, 7, 6, 1, 2},
			{11, 8, 9, 1, 3},
			{11, 9, 9, 1, 4},
			{8, 8, 8, 2, 2},
			{8, 9, 8, 2, 3},
			{6, 7, 6, 2, 4},
			{7, 7, 6, 3, 2},
			{6, 4, 7, 3, 3},
			{7, 4, 7, 3, 4},
			{-1, 0, 0, 0, 0},
			{-1, 0, 0, 0, 0},
		},
		{
			{7, 2, 8, 0, 0},
			{5, 6, 6, 0, 1},
			{5, 3, 6, 0, 2},
			{7, 1, 9, 0, 3},
			{10, 2, 8, 0, 4},
			{6, 3, 6, 1, 0},
			{6, 6, 7, 1, 1},
			{10, 1, 8, 1, 2},
			{11, 3, 6, 1, 3},
			{12, 3, 6, 1, 4},
			{11, 6, 7, 2, 2},
			{12, 6, 7, 2, 3},
			{10, 8, 8, 2, 4},
			{7, 7, 8, 3, 2},
			{7, 8, 8, 3, 3},
			{-1, 0, 0, 0, 0},
			{-1, 0, 0, 0, 0},
			{-1, 0, 0, 0, 0},
		},
	};
	for (int i = 0; i < 18; i++) {
		const int *rule = kHardFeatures[_layout - 6][i];
		if (rule[0] < 0)
			continue;
		Cell &target = _cells[index(rule[0], rule[1])];
		target.type = rule[2];
		target.direction = rule[2] - 6;
		target.trait = feature[rule[3]];
		target.value = values[rule[3]][rule[4]];
	}
	static constexpr int kRemove[2][4][2] = {
		{
			{1, 9},
			{1, 10},
			{0, 10},
			{0, 9},
		},
		{
			{5, 11},
			{4, 11},
			{3, 11},
			{1, 11},
		},
	};
	for (int i = 0; i < 4; i++) {
		if (static_cast<int>(_party.size()) <= 7 - i)
			clearCell(kRemove[_layout - 6][i][0], kRemove[_layout - 6][i][1]);
	}
}

bool MysticMarshGrid::place(int cellIndex, int zoombiniIndex) {
	if (cellIndex < 0 || kCellCount <= cellIndex || zoombiniIndex < 0 || static_cast<int>(_party.size()) <= zoombiniIndex)
		return false;
	if (_cells[cellIndex].type != 60 && _cells[cellIndex].type != 61)
		return false;
	Occupant &target = _after[cellIndex];
	if (target.index == -1)
		target.index = zoombiniIndex;
	else
		target.index += 1000 * zoombiniIndex;
	target.direction = _cells[cellIndex].direction;
	return true;
}

void MysticMarshGrid::move(int cellIndex, int direction) {
	const Occupant &source = _before[cellIndex];
	if (source.index < 0 || static_cast<int>(_party.size()) <= source.index)
		return;
	int column = cellIndex / kRows;
	int row = cellIndex % kRows;
	switch (direction) {
	case 0:
		column -= 1;
		break;
	case 1:
		column += 1;
		break;
	case 2:
		row -= 1;
		break;
	case 3:
		row += 1;
		break;
	default:
		break;
	}
	_moved[source.index] = 1;
	if (column < 0 || kColumns <= column || row < 0 || kRows <= row) {
		warning("Mystic Marsh: route leaves the bounded grid at %d,%d", column, row);
		_lost.push_back(source.index);
		return;
	}
	Occupant &target = _after[index(column, row)];
	if (target.index == -1)
		target.index = source.index;
	else
		target.index += 1000 * source.index;
	target.direction = direction;
}

void MysticMarshGrid::removeCollision(int first, int second) {
	_collisions.push_back(first);
	_collisions.push_back(second);
	for (int i = 0; i < kCellCount; i++) {
		if (_before[i].index == first || _before[i].index == second)
			_before[i].index = -1;
		if (_after[i].index == first || _after[i].index == second)
			_after[i].index = -1;
	}
}

void MysticMarshGrid::rotateCells(const bool triggers[7]) {
	static constexpr int kNextType[41] = {
		11,
		10,
		13,
		12,
		15,
		14,
		16,
		17,
		18,
		19,
		20,
		21,
		22,
		23,
		24,
		25,
		26,
		27,
		28,
		29,
		30,
		31,
		32,
		33,
		34,
		35,
		37,
		36,
		39,
		38,
		41,
		40,
		43,
		44,
		42,
		47,
		45,
		46,
		50,
		48,
		49,
	};
	static constexpr int kNextDirection[41] = {
		3,
		1,
		3,
		2,
		1,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		1,
		2,
		3,
		1,
		1,
		0,
		2,
		3,
		0,
		3,
		1,
		2,
		1,
		2,
		0,
	};
	for (int i = 0; i < kCellCount; i++) {
		Cell &target = _cells[i];
		const bool cyclic = 10 <= target.type && target.type <= 15 && _before[i].index != -1;
		const bool linked = 36 <= target.type && target.type <= 50 && 29 <= target.trigger && target.trigger <= 35 && triggers[target.trigger - 29];
		if (cyclic || linked) {
			const int type = target.type;
			target.type = kNextType[type - 10];
			target.direction = kNextDirection[type - 10];
			_turned = true;
		}
	}
}

void MysticMarshGrid::tick() {
	_moves.clear();
	_exits.clear();
	_lost.clear();
	_collisions.clear();
	_turned = false;
	_caught = false;
	_released = false;
	bool triggers[7] = {};
	for (uint i = 0; i < _moved.size(); i++)
		_moved[i] = 0;
	for (int i = 0; i < kCellCount; i++) {
		_before[i] = _after[i];
		_after[i] = Occupant();
	}
	for (int i = 0; i < kCellCount; i++) {
		const Occupant &occupant = _before[i];
		if (occupant.index == -1)
			continue;
		Cell &current = _cells[i];
		const int type = current.type;
		int direction = occupant.direction;
		if (type == 1 || (20 <= type && type <= 23) || (29 <= type && type <= 35) || type == 60 || type == 61) {
			move(i, direction);
		} else if ((2 <= type && type <= 5) || (10 <= type && type <= 19) || (36 <= type && type <= 50)) {
			move(i, current.direction);
			if (16 <= type && type <= 19) {
				current.type += 4;
				current.direction = 4;
			}
		} else if (6 <= type && type <= 9) {
			if (trait(occupant.index, current.trait) == current.value)
				direction = current.direction;
			move(i, direction);
		} else if (24 <= type && type <= 27 && 0 <= direction && direction < 4) {
			static constexpr int kTurns[4][4] = {
				{2, 3, 1, 0},
				{3, 2, 0, 1},
				{2, 3, 0, 1},
				{3, 2, 1, 0},
			};
			move(i, kTurns[type - 24][direction]);
		} else if (type == 28) {
			if (direction == 0)
				direction = 2;
			else if (direction == 1)
				direction = 3;
			move(i, direction);
		}
	}
	for (int i = 0; i < kCellCount; i++) {
		if (_before[i].index != -1 && 29 <= _cells[i].type && _cells[i].type <= 35)
			triggers[_cells[i].type - 29] = true;
	}
	bool released;
	do {
		released = false;
		for (int i = 0; i < kCellCount; i++) {
			const int z = _before[i].index;
			if (z < 0 || static_cast<int>(_moved.size()) <= z || _moved[z] != 0 || _cells[i].type < 51 || 57 < _cells[i].type)
				continue;
			const int trigger = _cells[i].trigger;
			if (_after[i].index != -1) {
				move(i, _after[i].direction);
				released = true;
			} else if (29 <= trigger && trigger <= 35 && triggers[trigger - 29]) {
				move(i, _before[i].direction);
				released = true;
			} else {
				released = false;
			}
		}
		_released = _released || released;
	} while (released);
	for (int i = 0; i < kCellCount; i++) {
		const int z = _before[i].index;
		if (z == -1)
			continue;
		if (_cells[i].type == 62)
			_exits.push_back(z);
		if (_cells[i].type == 58) {
			if (1000 <= z) {
				_lost.push_back(z % 1000);
				_lost.push_back(z / 1000);
			} else {
				_lost.push_back(z);
			}
		}
	}
	for (int i = 0; i < kCellCount; i++) {
		const int z = _after[i].index;
		if (1000 <= z && _cells[i].type != 58) {
			_after[i].index = -1;
			removeCollision(z % 1000, z / 1000);
		}
	}
	for (int i = 0; i < kCellCount; i++) {
		const int next = _after[i].index;
		const int previous = _before[i].index;
		if (next < 0 || previous < 0 || next == previous)
			continue;
		for (int j = 0; j < kCellCount; j++) {
			if (_before[j].index == next && _after[j].index == previous) {
				removeCollision(next, previous);
				break;
			}
		}
	}
	rotateCells(triggers);
	for (int i = 0; i < kCellCount; i++) {
		if (_cells[i].type == 62 || _cells[i].type == 58)
			_before[i].index = -1;
		const int z = _before[i].index;
		if (0 <= z && z < static_cast<int>(_moved.size()) && _moved[z] == 0)
			_after[i] = _before[i];
		if (51 <= _cells[i].type && _cells[i].type <= 57 && _after[i].index != -1 && _before[i].index != _after[i].index)
			_caught = true;
	}
	for (int i = 0; i < kCellCount; i++) {
		const int z = _before[i].index;
		if (z == -1)
			continue;
		for (int j = 0; j < kCellCount; j++) {
			const int target = _after[j].index;
			if (target == z || (1000 < target && (target / 1000 == z || target % 1000 == z))) {
				_moves.push_back(Move(z, i, j));
				break;
			}
		}
	}
}

// Immutable marsh cells and row-major layout maps.
const MysticMarshGrid::Cell MysticMarshGrid::kCellTemplates[] = {
	{0, 0, 0, 0, 0, 0},
	{1, 4, 0, 0, 0, 0},
	{3, 1, 0, 0, 0, 0},
	{9, 3, 0, 0, 0, 0},
	{52, 4, 30, 0, 0, 0},
	{29, 4, 0, 0, 0, 0},
	{5, 3, 0, 0, 0, 0},
	{30, 4, 0, 0, 0, 0},
	{60, 1, 0, 0, 0, 0},
	{12, 2, 0, 0, 0, 0},
	{58, 4, 0, 0, 0, 0},
	{17, 1, 0, 0, 0, 0},
	{51, 4, 29, 0, 0, 0},
	{62, 0, 0, 0, 0, 0},
	{31, 4, 0, 0, 0, 0},
	{8, 2, 0, 0, 0, 0},
	{53, 4, 31, 0, 0, 0},
	{4, 2, 0, 0, 0, 0},
	{19, 3, 0, 0, 0, 0},
	{18, 2, 0, 0, 0, 0},
	{7, 1, 0, 0, 0, 0},
	{32, 4, 0, 0, 0, 0},
	{55, 4, 33, 0, 0, 0},
	{56, 4, 34, 0, 0, 0},
	{54, 4, 32, 0, 0, 0},
	{14, 0, 0, 0, 0, 0},
	{33, 4, 0, 0, 0, 0},
	{34, 4, 0, 0, 0, 0},
	{57, 4, 35, 0, 0, 0},
	{35, 4, 0, 0, 0, 0},
	{2, 0, 0, 0, 0, 0},
	{6, 0, 0, 0, 0, 0},
	{11, 3, 0, 0, 0, 0},
	{58, 0, 0, 0, 0, 0},
	{24, 4, 0, 0, 0, 0},
	{25, 4, 0, 0, 0, 0},
	{61, 3, 0, 0, 0, 0},
	{16, 0, 0, 0, 0, 0},
	{38, 1, 31, 0, 0, 0},
	{10, 1, 0, 0, 0, 0},
	{36, 2, 30, 0, 0, 0},
};

constexpr byte MysticMarshGrid::kLayouts[8][kRows][kColumns];

constexpr const char *PuzzleMysticMarsh::kMusicPath;
constexpr const char *PuzzleMysticMarsh::kBackgroundFormat;
constexpr const char *PuzzleMysticMarsh::kAreaFormat;
constexpr const char *PuzzleMysticMarsh::kSymbolFormat;
constexpr const char *PuzzleMysticMarsh::kTraitFormat;
constexpr const char *PuzzleMysticMarsh::kCraterPath;
constexpr const char *PuzzleMysticMarsh::kCraterAnimationPath;
constexpr const char *PuzzleMysticMarsh::kWhirlpoolPath;
constexpr const char *PuzzleMysticMarsh::kBubblePath;
constexpr const char *PuzzleMysticMarsh::kFloatPath;
constexpr const char *PuzzleMysticMarsh::kPickupPath;
constexpr const char *PuzzleMysticMarsh::kCelebratePath;
constexpr const char *PuzzleMysticMarsh::kSfxFormat;
constexpr const char *PuzzleMysticMarsh::kGoSpeechFormat;
constexpr const char *PuzzleMysticMarsh::kCaveSpeech;
constexpr const char *PuzzleMysticMarsh::kCompleteSpeech;

PuzzleMysticMarsh::PuzzleMysticMarsh(Zoombini2Engine *vm) : PuzzleBase(vm, kPageMysticMarsh) {
}

PuzzleMysticMarsh::~PuzzleMysticMarsh() {
	for (uint i = 0; i < _bubbles.size(); i++)
		delete _bubbles[i].path;
	delete _craterAnimation;
	delete _whirlpoolAnimation;
	if (SoundManager *sound = _vm->getSoundManager()) {
		for (int i = 0; i < 9; i++) {
			if (0 <= _sounds[i])
				sound->unload(_sounds[i]);
		}
		if (0 <= _speechSound)
			sound->unload(_speechSound);
	}
	finishPuzzleRoster(_vm->_state->_rescue1Board);
}

Common::Point32 PuzzleMysticMarsh::cellPosition(int index) {
	return Common::Point32(44 * (index / MysticMarshGrid::kRows) + 10 * (index % MysticMarshGrid::kRows), 31 * (index % MysticMarshGrid::kRows) + 1);
}

void PuzzleMysticMarsh::init() {
	PuzzleBase::init();
	Common::Array<ZmbTrait> party;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++)
		party.push_back(_puzzleZoombinis[i]->_traits);
	_grid.init(party, CLIP(_puzzleLevel, 1, 3), *_vm->_rnd);
	for (int i = 0; i < MysticMarshGrid::kCellCount; i++)
		_drawCells[i] = _grid.cell(i);
	_backgroundIndex = _grid.background();
	loadPrimaryLayerBackground(Common::Path(Common::String::format(kBackgroundFormat, _backgroundIndex)));
	loadAreaMask(Common::Path(Common::String::format(kAreaFormat, _backgroundIndex)));
	loadResources();
	startPageMusic(Common::Path(kMusicPath));
	_bubbles.resize(party.size());
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _puzzleZoombinis[i];
		zoombini->clearMovement();
		zoombini->setDefaultAnimation(_zoombiniAnimation);
		zoombini->setPosition(kStartingPositions[_backgroundIndex - 1][i % 8]);
		zoombini->_inputEnabled = true;
		zoombini->_puzzleStatus = 0;
		zoombini->_hidden = false;
		zoombini->_dragging = false;
	}
	for (int i = 0; i < MysticMarshGrid::kCellCount; i++) {
		const int type = _grid.cell(i).type;
		if (type != 60 && type != 61)
			continue;
		Slot slot;
		slot.cell = i;
		const Common::Point32 position = cellPosition(i);
		slot.position = Common::Point32(position.x - 25, position.y - 10);
		_slots.push_back(slot);
		ZoombiniDropTarget target;
		target.rect = Common::Rect32(position.x - 8, position.y + 52, position.x + 35, position.y + 102);
		target.callback = slotDropCallback;
		target.callbackContext = this;
		_dropTargets.push_back(target);
	}
	_lastTick = _vm->getGameTickCount();
	debug(1, "Mystic Marsh: difficulty=%d layout=%d background=%d party=%u craters=%u", _puzzleLevel, _grid.layout(), _backgroundIndex,
		  _puzzleZoombinis.size(), _slots.size());
}

void PuzzleMysticMarsh::loadResources() {
	for (int i = 0; i < 60; i++) {
		_vm->_gfx->loadPageRleBlock(Common::String::format(kSymbolFormat, kSymbolNames[i]));
	}
	for (int f = 0; f < 4; f++) {
		for (int v = 0; v < 5; v++) {
			_vm->_gfx->loadPageRleBlock(Common::String::format(kTraitFormat, f + 1, v + 1));
		}
	}
	_vm->_gfx->loadPageRleBlock(kCraterPath);
	_vm->_gfx->loadPageRleBlock(kBubblePath);
	_craterAnimation = new Animation(_vm);
	_craterAnimation->loadFromFile(Common::Path(kCraterAnimationPath));
	_whirlpoolAnimation = new Animation(_vm);
	_whirlpoolAnimation->loadFromFile(Common::Path(kWhirlpoolPath));
	_floatAnimation = _vm->loadZoombiniAnimation(Common::Path(kFloatPath), 190);
	_pickupAnimation = _vm->loadZoombiniAnimation(Common::Path(kPickupPath), 100);
	_celebrateAnimation = _vm->loadZoombiniAnimation(Common::Path(kCelebratePath), 50);
	if (SoundManager *sound = _vm->getSoundManager()) {
		for (int i = 2; i <= 8; i++)
			_sounds[i] = sound->load(false, Common::Path(Common::String::format(kSfxFormat, i)), false);
	}
}

PathObject *PuzzleMysticMarsh::createPath(const Common::Point32 &from, const Common::Point32 &to, int step) const {
	const Common::Point32 delta((to.x - from.x) / 3, (to.y - from.y) / 3);
	PathObject *path = new PathObject(_vm);
	path->appendSegment(from, Common::Point32(from.x + delta.x, from.y + delta.y), Common::Point32(to.x - delta.x, to.y - delta.y), to, step, 0);
	return path;
}

void PuzzleMysticMarsh::slotDropCallback(void *context, int slotIndex, int zoombiniIndex) {
	PuzzleMysticMarsh *page = static_cast<PuzzleMysticMarsh *>(context);
	if (page && 0 <= slotIndex && slotIndex < static_cast<int>(page->_slots.size()) && page->_dropTargets[slotIndex].occupied)
		page->placeZoombini(slotIndex, zoombiniIndex);
}

void PuzzleMysticMarsh::placeZoombini(int slotIndex, int zoombiniIndex) {
	const uint32 now = _vm->getGameTickCount();
	_unlockTime = now + 4000;
	for (uint i = 0; i < _dropTargets.size(); i++) {
		_dropTargets[i].occupied = true;
		_dropTargets[i].zoombiniIndex = -1;
	}
	if (_placingZoombini != -1 || !_grid.place(_slots[slotIndex].cell, zoombiniIndex))
		return;
	_placingZoombini = zoombiniIndex;
	_placingSlot = slotIndex;
	_placementStart = now;
	ZoombiniRunner *zoombini = _puzzleZoombinis[zoombiniIndex];
	zoombini->_inputEnabled = false;
	const Common::Point32 &position = _slots[slotIndex].position;
	zoombini->setPosition(Common::Point32(position.x + 10, position.y + 30));
	playSfx(3);
}

void PuzzleMysticMarsh::playSfx(int index) {
	SoundManager *sound = _vm->getSoundManager();
	if (sound && 0 <= _sounds[index])
		sound->playWithVolume(_sounds[index], sound->_volumeSFX);
}

void PuzzleMysticMarsh::freeZoombini(int index, uint32 now) {
	if (index < 0 || static_cast<int>(_bubbles.size()) <= index)
		return;
	Bubble &bubble = _bubbles[index];
	delete bubble.path;
	bubble.path = nullptr;
	bubble.active = false;
	ZoombiniRunner *zoombini = _puzzleZoombinis[index];
	zoombini->_inputEnabled = false;
	zoombini->_puzzleStatus = 1;
	zoombini->setActiveAnimation(_zoombiniAnimation);
	zoombini->startMovement(createPath(zoombini->_screenPos, kExitPositions[_backgroundIndex - 1][_freed % 8], 7), now);
	zoombini->startDirectionTrackedAnimation(now);
	_freed += 1;
	playSfx(4);
}

void PuzzleMysticMarsh::loseZoombini(int index, bool whirlpool, uint32 now) {
	if (index < 0 || static_cast<int>(_bubbles.size()) <= index)
		return;
	Bubble &bubble = _bubbles[index];
	if (whirlpool) {
		_effects.clear();
		Effect effect;
		effect.position = Common::Point32(bubble.position.x + 10, bubble.position.y - 45);
		effect.start = now;
		_effects.push_back(effect);
	}
	delete bubble.path;
	bubble.path = nullptr;
	bubble.active = false;
	_puzzleZoombinis[index]->_hidden = true;
	_puzzleZoombinis[index]->_inputEnabled = false;
	if (whirlpool)
		playSfx(8);
}

void PuzzleMysticMarsh::advanceGrid(uint32 now) {
	const Common::Array<MysticMarshGrid::Move> &moves = _grid.moves();
	for (uint i = 0; i < moves.size(); i++) {
		const MysticMarshGrid::Move &move = moves[i];
		if (move.index < 0 || static_cast<int>(_bubbles.size()) <= move.index)
			continue;
		Bubble &bubble = _bubbles[move.index];
		if (!bubble.active)
			continue;
		const Common::Point32 from = cellPosition(move.from);
		const Common::Point32 to = cellPosition(move.to);
		delete bubble.path;
		bubble.path = createPath(Common::Point32(from.x - 7, from.y + 30), Common::Point32(to.x - 7, to.y + 30), 14);
		bubble.path->start(now);
	}
	if (_grid.turned())
		playSfx(5);
	if (_grid.caught())
		playSfx(6);
	if (_grid.released())
		playSfx(7);
	for (uint i = 0; i < _grid.lost().size(); i++)
		loseZoombini(_grid.lost()[i], true, now);
	for (uint i = 0; i < _grid.exits().size(); i++)
		freeZoombini(_grid.exits()[i], now);
	for (uint i = 0; i < _grid.collisions().size(); i++) {
		if (i % 2 == 0)
			playSfx(4);
		loseZoombini(_grid.collisions()[i], false, now);
	}
	_grid.tick();
}

void PuzzleMysticMarsh::onUpdate() {
	const uint32 now = _vm->getGameTickCount();
	// Cell changes become visible after the frame which advances the grid.
	for (int i = 0; i < MysticMarshGrid::kCellCount; i++)
		_drawCells[i] = _grid.cell(i);
	pumpSpeech();
	if (_goPending && _speechSound < 0 && _speechQueue.empty()) {
		_goPending = false;
		_vm->_mapTransitionSourcePageId = kPageMysticMarsh;
		_vm->requestPageChange(kPageMapTrans);
	}
	if (_unlockTime != 0 && _unlockTime < now) {
		for (uint i = 0; i < _dropTargets.size(); i++)
			_dropTargets[i].occupied = false;
		_unlockTime = 0;
	}
	bool available = false;
	int resolved = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _puzzleZoombinis[i];
		available = available || zoombini->_inputEnabled;
		if (zoombini->_puzzleStatus == 1 || zoombini->_hidden)
			resolved += 1;
		if (zoombini->_movementPath && !zoombini->advanceMovement(now)) {
			zoombini->clearMovement();
			zoombini->resetAnimation();
		}
		zoombini->updateAnimation(now);
	}
	if (!_finished && resolved == static_cast<int>(_puzzleZoombinis.size())) {
		_finished = true;
		_vm->restartGoBlink();
		if (0 < _freed)
			enqueueSpeech(kCompleteSpeech);
	}
	if (!available && _vm->_zoombiniWalkingFlag && 0 < _freed) {
		for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
			ZoombiniRunner *zoombini = _puzzleZoombinis[i];
			if (zoombini->_puzzleStatus == 1 && !zoombini->_animationActive && _vm->_rnd->getRandomNumber(19) == 1)
				zoombini->startAnimation(_celebrateAnimation, 33, now);
		}
	}
	if (_placingZoombini != -1) {
		if (_placementStart + 1000 < now) {
			Bubble &bubble = _bubbles[_placingZoombini];
			bubble.active = true;
			bubble.position = _slots[_placingSlot].position;
			_puzzleZoombinis[_placingZoombini]->setActiveAnimation(_floatAnimation);
			_puzzleZoombinis[_placingZoombini]->startAnimation(_floatAnimation, 33, now);
			_placingZoombini = -1;
		} else {
			_puzzleZoombinis[_placingZoombini]->_screenPos.y -= (now - _lastTick) / 30;
		}
	}
	bool moving = false;
	for (uint i = 0; i < _bubbles.size(); i++) {
		Bubble &bubble = _bubbles[i];
		if (!bubble.active || !bubble.path)
			continue;
		// A path completing this frame still postpones the next grid step.
		moving = true;
		if (!bubble.path->advance(now, bubble.position)) {
			delete bubble.path;
			bubble.path = nullptr;
		}
		_puzzleZoombinis[i]->_screenPos = Common::Point32(bubble.position.x + 9, bubble.position.y + 10);
	}
	if (!moving && _placingZoombini == -1 && !_finished)
		advanceGrid(now);
	for (uint i = 0; i < _effects.size();) {
		if (_effects[i].start + 900 <= now)
			_effects.remove_at(i);
		else
			i += 1;
	}
	_lastTick = now;
}

void PuzzleMysticMarsh::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

void PuzzleMysticMarsh::onRenderContent(ManagedSurface32 *screen) {
	for (int i = 0; i < MysticMarshGrid::kCellCount; i++) {
		const MysticMarshGrid::Cell &cell = _drawCells[i];
		if (cell.type < 2 || 60 <= cell.type)
			continue;
		const Common::Point32 position = cellPosition(i);
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSymbolFormat, kSymbolNames[cell.type - 2]),
									Common::Point32(position.x + 3, position.y + 61));
		if (6 <= cell.type && cell.type <= 9 && 1 <= cell.trait && cell.trait <= 4 && 1 <= cell.value && cell.value <= 5)
			_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kTraitFormat, cell.trait, cell.value),
										Common::Point32(position.x + 9, position.y + 62));
	}
	for (uint i = 0; i < _slots.size(); i++) {
		const Slot &slot = _slots[i];
		_vm->_gfx->drawPageRleBlock(screen, kCraterPath, Common::Point32(slot.position.x, slot.position.y + 10));
	}
	const uint32 now = _vm->getGameTickCount();
	if (0 <= _placingSlot && now - _placementStart < 1100 && _craterAnimation && 0 < _craterAnimation->getFrameCount()) {
		const int frame = MIN<int>((now - _placementStart) / 100, _craterAnimation->getFrameCount() - 1);
		_vm->_gfx->drawAnimationFrame(screen, _craterAnimation, frame, _slots[_placingSlot].position);
	}
	if (_whirlpoolAnimation && 0 < _whirlpoolAnimation->getFrameCount()) {
		for (uint i = 0; i < _effects.size(); i++) {
			const Effect &effect = _effects[i];
			const int frame = MIN<int>((now - effect.start) / 100, _whirlpoolAnimation->getFrameCount() - 1);
			_vm->_gfx->drawAnimationFrame(screen, _whirlpoolAnimation, frame, effect.position);
		}
	}
}

void PuzzleMysticMarsh::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleMysticMarsh::onActorsRendered() {
	for (uint i = 0; i < _puzzleZoombinis.size(); i++)
		_puzzleZoombinis[i]->advanceAnimationAfterDraw();
}

void PuzzleMysticMarsh::onRenderForeground(ManagedSurface32 *screen) {
	for (uint i = 0; i < _bubbles.size(); i++) {
		if (_bubbles[i].active)
			_vm->_gfx->drawPageRleBlock(screen, kBubblePath, _bubbles[i].position);
	}
}

EventHandleResult PuzzleMysticMarsh::onLButtonUp(const Common::Point &pos) {
	const ZoombiniInputResult result = ZoombiniRunner::handlePointerInput(_puzzleZoombinis, Common::Point32(pos.x, pos.y), true,
																		  _pickupAnimation, _vm->getGameTickCount(), &_dropTargets, getAreaMask());
	return result == ZoombiniInputResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

EventHandleResult PuzzleMysticMarsh::onMouseMove(const Common::Point &pos) {
	const ZoombiniInputResult result = ZoombiniRunner::handlePointerInput(_puzzleZoombinis, Common::Point32(pos.x, pos.y), false,
																		  _pickupAnimation, _vm->getGameTickCount(), &_dropTargets, getAreaMask());
	return result == ZoombiniInputResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

void PuzzleMysticMarsh::enqueueSpeech(const Common::String &name) {
	_speechQueue.push_back(name);
	pumpSpeech();
}

void PuzzleMysticMarsh::pumpSpeech() {
	SoundManager *sound = _vm->getSoundManager();
	if (!sound) {
		_speechSound = -1;
		_speechQueue.clear();
		return;
	}
	if (0 <= _speechSound) {
		if (sound->isPlaying(_speechSound))
			return;
		sound->unload(_speechSound);
		_speechSound = -1;
	}
	while (!_speechQueue.empty()) {
		_speechSound = sound->load(true, Common::Path(_speechQueue[0]), false);
		_speechQueue.remove_at(0);
		if (0 <= _speechSound) {
			sound->playWithVolume(_speechSound, sound->_volumeSpeech);
			return;
		}
	}
}

bool PuzzleMysticMarsh::onGoButtonPressed() {
	if (!_vm->_isSavedGame)
		return true;
	if (_goPending)
		return true;
	int unresolved = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_puzzleStatus == 0)
			unresolved += 1;
	}
	if (4 <= unresolved)
		enqueueSpeech(kCaveSpeech);
	else if (unresolved == 0) {
		const int variant = _vm->_rnd->getRandomNumber(4) + 1;
		enqueueSpeech(Common::String::format(kGoSpeechFormat, variant));
	} else {
		return true;
	}
	_goPending = true;
	return false;
}

void PuzzleMysticMarsh::applyDebugPuzzleCompletion() {
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _puzzleZoombinis[i];
		zoombini->clearMovement();
		zoombini->_puzzleStatus = 1;
		zoombini->_hidden = false;
		zoombini->_inputEnabled = false;
		zoombini->setDefaultAnimation(_zoombiniAnimation);
		zoombini->setPosition(kExitPositions[_backgroundIndex - 1][i % 8]);
		delete _bubbles[i].path;
		_bubbles[i].path = nullptr;
		_bubbles[i].active = false;
	}
	_placingZoombini = -1;
	_finished = true;
	_freed = _puzzleZoombinis.size();
}

Common::String PuzzleMysticMarsh::debugGroup(const char *label, const Common::Array<int> &actors) const {
	Common::String report = Common::String::format("%s:", label);
	for (int actor : actors)
		report += Common::String::format(" %d", actor + 1);
	return report + "\n";
}

Common::String PuzzleMysticMarsh::debugGeneration() const {
	const MysticMarshGrid::GenerationInfo &info = _grid.generationInfo();
	Common::String report = Common::String::format("Generation layout: %d. Construction records below are not a timed launch solution.\n", _grid.layout() + 1);
	if (_puzzleLevel == 1) {
		report += debugGroup("Selection order", info.selectionOrder);
		report += debugGroup("Upper-path assignment", info.upper);
		report += debugGroup("Lower-path assignment", info.lower);
	} else if (_puzzleLevel == 2) {
		report += debugGroup("Selected first group (Zoombini indices)", info.group);
		report += debugGroup("Nested selection (positions within that group)", info.subset);
		report += debugGroup("Third-filter input (these same numbers are used as roster indices)", info.subset);
	} else {
		for (int i = 0; i < 2; i++)
			report += Common::String::format("Reference %d: %s\n", i + 1, debugActorDescription(info.referenceActors[i]).c_str());
		if (info.restoredFilterValues) {
			report += "Primary filter values were restored from a better candidate; its reference actors were:\n";
			for (int i = 0; i < 2; i++)
				report += "  " + debugActorDescription(info.primaryFilterActors[i]) + "\n";
		}
	}
	for (uint i = 0; i < _puzzleZoombinis.size(); i++)
		report += debugActorDescription(i) + "\n";
	static constexpr const char *directions[] = {
		"left",
		"right",
		"up",
		"down",
		"unchanged",
	};
	static constexpr const char *traits[] = {
		"feet",
		"nose",
		"hair",
		"eyes",
	};
	for (int i = 0; i < MysticMarshGrid::kCellCount; i++) {
		const MysticMarshGrid::Cell &initial = _grid.initialCell(i);
		const bool filter = 6 <= initial.type && initial.type <= 9 && 1 <= initial.trait && initial.trait <= 4;
		if (!filter && !initial.trigger)
			continue;
		const MysticMarshGrid::Cell &current = _grid.cell(i);
		const Common::Point32 pos = cellPosition(i);
		report += Common::String::format("Device column %d row %d (%d,%d): initial %s; direction %s; trigger %d\n",
										 i / MysticMarshGrid::kRows + 1, i % MysticMarshGrid::kRows + 1, pos.x, pos.y,
										 2 <= initial.type && initial.type < 62 ? kSymbolNames[initial.type - 2] : "other",
										 directions[CLIP(initial.direction, 0, 4)], initial.trigger);
		if (filter) {
			const ZmbTrait::TraitIndex axis = static_cast<ZmbTrait::TraitIndex>(initial.trait - 1);
			report += Common::String::format("  Filter: %s = %s; matching Zoombinis:", traits[initial.trait - 1],
											 ZmbTrait::debugTraitValueName(axis, initial.value));
			for (uint actor = 0; actor < _puzzleZoombinis.size(); actor++)
				if (_puzzleZoombinis[actor]->_traits.getValue(axis) == initial.value)
					report += Common::String::format(" %u", actor + 1);
			report += "\n";
		}
		report += Common::String::format("  Current type %d; direction %s; state %d (initial %d).\n", current.type,
										 directions[CLIP(current.direction, 0, 4)], current.state, initial.state);
	}
	return report;
}

Common::String PuzzleMysticMarsh::debugGetAnswer() const {
	Common::String answer = debugAnswerHeader();
	answer += debugGeneration();
	answer += "Supplementary predictions add one Zoombini to a copy of the current grid, with no later launches.\n";
	answer += "A waiting bubble may require another launch to release it; unresolved does not mean an invalid choice. Recheck after each placement.\n";
	for (uint actor = 0; actor < _puzzleZoombinis.size(); actor++) {
		if (!_puzzleZoombinis[actor]->_inputEnabled)
			continue;
		answer += debugActorDescription(actor) + ":\n";
		for (uint slot = 0; slot < _slots.size(); slot++) {
			MysticMarshGrid simulation = _grid;
			const char *result = "unresolved after 384 grid steps (may require another bubble)";
			if (!simulation.place(_slots[slot].cell, actor)) {
				result = "occupied";
			} else {
				for (int step = 0; step < 384; step++) {
					simulation.tick();
					bool resolved = false;
					for (int index : simulation.exits())
						if (index == static_cast<int>(actor)) {
							result = "exit";
							resolved = true;
						}
					for (int index : simulation.lost())
						if (index == static_cast<int>(actor)) {
							result = "whirlpool";
							resolved = true;
						}
					for (int index : simulation.collisions())
						if (index == static_cast<int>(actor)) {
							result = "collision";
							resolved = true;
						}
					if (resolved)
						break;
				}
			}
			answer += Common::String::format("  Crater %u (%d,%d): %s\n", slot + 1, _slots[slot].position.x, _slots[slot].position.y, result);
		}
	}
	return answer;
}

Common::String PuzzleMysticMarsh::debugGetChanceDetails() const {
	int available = 0;
	int active = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_inputEnabled)
			available += 1;
		if (_bubbles[i].active)
			active += 1;
	}
	return Common::String::format("No separate retry counter. Waiting: %d; active bubbles: %d; freed: %d.\n", available, active, _freed);
}

constexpr const char *PuzzleMysticMarsh::kSymbolNames[60];

constexpr Common::Point32 PuzzleMysticMarsh::kStartingPositions[6][8];

constexpr Common::Point32 PuzzleMysticMarsh::kExitPositions[6][8];

} // End of namespace Zoombini2
