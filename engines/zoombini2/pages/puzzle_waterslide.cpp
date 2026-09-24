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

#include "zoombini2/pages/puzzle_waterslide.h"
#include "zoombini2/graphics.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleWaterslide::kMusicPath;
constexpr const char *PuzzleWaterslide::kTraitFormat;
constexpr const char *PuzzleWaterslide::kHorizontalFormat;
constexpr const char *PuzzleWaterslide::kLargePipeFormat;
constexpr const char *PuzzleWaterslide::kSelectorFormat;
constexpr const char *PuzzleWaterslide::kHardPipeFormat;
constexpr const char *PuzzleWaterslide::kPipeColors[2];
constexpr const char *PuzzleWaterslide::kHardPipeColors[2];
constexpr const char *PuzzleWaterslide::kMiniDiagonalPath;
constexpr const char *PuzzleWaterslide::kMiniHorizontalPath;
constexpr const char *PuzzleWaterslide::kOutletPath;
constexpr const char *PuzzleWaterslide::kPastillePath;
constexpr const char *PuzzleWaterslide::kEdgePath;
constexpr const char *PuzzleWaterslide::kFountainPath;
constexpr const char *PuzzleWaterslide::kTreePath;
constexpr const char *PuzzleWaterslide::kValvePath;
constexpr const char *PuzzleWaterslide::kCascadeFormat;
constexpr const char *PuzzleWaterslide::kAreaPath;
constexpr const char *PuzzleWaterslide::kPickupPath;
constexpr const char *PuzzleWaterslide::kIdlePath;
constexpr const char *PuzzleWaterslide::kAspirationPath;
constexpr const char *PuzzleWaterslide::kSoundFormat;
constexpr const char *PuzzleWaterslide::kPraisePath;
constexpr const char *PuzzleWaterslide::kPartialPraisePath;
constexpr const char *PuzzleWaterslide::kRetreatSpeech;
constexpr const char *PuzzleWaterslide::kGoSpeechFormat;

constexpr Common::Point32 PuzzleWaterslide::kWaitingPositions[16];
constexpr int PuzzleWaterslide::kNeighbors[16][5];
constexpr PuzzleWaterslide::GraphPlacement PuzzleWaterslide::kGraphPlacements[26];

PuzzleWaterslide::PuzzleWaterslide(Zoombini2Engine *vm) : PuzzleBase(vm, kPageWaterslide) {}

PuzzleWaterslide::~PuzzleWaterslide() {
	delete _valveRunner;
	delete _cascadeRunner;
	delete _treeRunner;
	delete _fountainRunner;
	delete _tree;
	delete _fountain;
	delete _valve;
	delete _cascade;
	if (SoundManager *sound = _vm->getSoundManager()) {
		for (int handle : _sounds)
			if (0 <= handle)
				sound->unload(handle);
	}
	finishPuzzleRoster(nullptr);
}

void PuzzleWaterslide::init() {
	if (_vm->isDemo())
		createDemoParty();
	PuzzleBase::init();
	_level = CLIP(_puzzleLevel, 1, 4);
	const int count = _puzzleZoombinis.size();
	const int slots = _level == 1 ? count : 16;
	_solution.resize(slots);
	_eligible.resize(count);
	for (int i = 0; i < slots; i++)
		_solution[i] = -1;
	Common::Array<bool> used;
	used.resize(count);
	while (static_cast<int>(_generationOrder.size()) < count) {
		const int index = _vm->_rnd->getRandomNumber(count - 1);
		if (!used[index]) {
			used[index] = true;
			_generationOrder.push_back(index);
		}
	}
	if (0 < count) {
		if (_level == 1)
			generateEasy();
		else if (_level == 2)
			generateMedium();
		else
			generateHard();
	}
	loadResources();
	setupTargets();
	for (int i = 0; i < count; i++) {
		ZoombiniRunner *actor = _puzzleZoombinis[i];
		actor->setDefaultAnimation(_zoombiniAnimation);
		actor->resetAnimation();
		actor->setPosition(kWaitingPositions[i]);
		actor->_inputEnabled = true;
		actor->_puzzleStatus = 0;
		actor->_hidden = false;
	}
	startPageMusic(Common::Path(kMusicPath));
	if (_vm->_isSavedGame)
		_vm->_state->registerPageVisit(kPageWaterslide, 2);
}

void PuzzleWaterslide::createDemoParty() {
	_vm->_isSavedGame = false;
	_vm->_state->_level = 1;
	_puzzleLevel = 1;
	_vm->_zoombiniWalkingFlag = false;
	_vm->_state->clearActiveZoombinis();
	for (int index = 0; index < 16; index++) {
		const byte hair = static_cast<byte>(_vm->_rnd->getRandomNumber(4) + 1);
		const byte eyes = static_cast<byte>(_vm->_rnd->getRandomNumber(4) + 1);
		const byte nose = static_cast<byte>(_vm->_rnd->getRandomNumber(4) + 1);
		const byte feet = static_cast<byte>(_vm->_rnd->getRandomNumber(4) + 1);
		ZoombiniRunner *actor = new ZoombiniRunner();
		actor->setTraits(ZmbTrait(feet, nose, hair, eyes));
		actor->_animationCell = 33;
		_vm->_state->_activeZoombinis.push_back(actor);
	}
	for (ZoombiniRunner *actor : _vm->_state->_activeZoombinis) {
		const Common::String name = GameState::generateZoombiniName(*_vm->_rnd);
		Common::strlcpy(actor->_name, name.c_str(), sizeof(actor->_name));
	}
}

void PuzzleWaterslide::loadResources() {
	Gfx *gfx = _vm->_gfx;
	for (int axis = 0; axis < 4; axis++)
		gfx->loadPageRleBlock(Common::String::format(kTraitFormat, axis + 1));
	gfx->loadPageRleBlock(kEdgePath);
	gfx->loadPageRleBlock(kPastillePath);
	for (int color = 0; color < 2; color++) {
		const char *name = kPipeColors[color];
		if (_level < 3) {
			gfx->loadPageRleBlock(Common::String::format(kHorizontalFormat, name));
			if (_level == 2)
				for (int arm = 0; arm < 5; arm++)
					gfx->loadPageRleBlock(Common::String::format(kSelectorFormat, name, arm + 1));
		} else {
			for (int type = 0; type < 4; type++)
				gfx->loadPageRleBlock(Common::String::format(kHardPipeFormat, kHardPipeColors[color], type + 1));
		}
	}
	if (_level < 3) {
		gfx->loadPageRleBlock(Common::String::format(kLargePipeFormat, _level));
		gfx->loadPageRleBlock(kMiniDiagonalPath);
		gfx->loadPageRleBlock(kMiniHorizontalPath);
	} else {
		gfx->loadPageRleBlock(kOutletPath);
	}
	_tree = new Animation(_vm);
	_tree->loadFromFile(Common::Path(kTreePath));
	_fountain = new Animation(_vm);
	_fountain->loadFromFile(Common::Path(kFountainPath));
	_valve = new Animation(_vm);
	_valve->loadFromFile(Common::Path(kValvePath));
	_cascade = new Animation(_vm);
	_cascade->loadFromFile(Common::Path(Common::String::format(kCascadeFormat, _level < 3 ? 1 : 2)));
	_valvePos = _level < 3 ? Common::Point32(470, 139) : Common::Point32(660, 501);
	_cascadePos = _level < 3 ? Common::Point32(505, 139) : Common::Point32(689, 505);
	_valveRunner = new AnimationRunner(_vm, _valvePos, AnimationRunnerMode::kPlayOnceAndHide02);
	_valveRunner->setAnimation(_valve);
	_valveRunner->setHitRect(Common::Rect32(_valvePos.x, _valvePos.y, _valvePos.x + 55, _valvePos.y + 42));
	static constexpr int valveFrames[6] = {
		0,
		1,
		2,
		2,
		1,
		0,
	};
	int valveCycles = 1;
	if (_level == 1)
		valveCycles = 4;
	else if (_level == 2)
		valveCycles = 2;
	for (int cycle = 0; cycle < valveCycles; cycle++) {
		for (int frame : valveFrames)
			_valveRunner->addTimedFrame(frame, 60);
	}
	_cascadeRunner = new AnimationRunner(_vm, _cascadePos, AnimationRunnerMode::kPlayOnce00);
	_cascadeRunner->setAnimation(_cascade);
	_cascadeRunner->addTimedFrame(0, 700);
	for (int frame = 1; frame < 10; frame++)
		_cascadeRunner->addTimedFrame(frame, 60);
	_cascadeRunner->setCompletionCallback(&onCascadeComplete, this);
	_treeRunner = new AnimationRunner(_vm, Common::Point32(605, 95), AnimationRunnerMode::kRandomIdle05);
	_treeRunner->setAnimation(_tree);
	_fountainRunner = new AnimationRunner(_vm, Common::Point32(21, 85), AnimationRunnerMode::kLoop01);
	_fountainRunner->setAnimation(_fountain);
	for (int frame = 0; frame < 4; frame++) {
		_treeRunner->addTimedFrame(frame, 200);
		_fountainRunner->addTimedFrame(frame, 200);
	}
	_treeRunner->addTimedFrame(3, 2000);
	_pickup = _vm->loadZoombiniAnimation(Common::Path(kPickupPath), 100);
	_idle = _vm->loadZoombiniAnimation(Common::Path(kIdlePath), 50);
	_aspiration = _vm->loadZoombiniAnimation(Common::Path(kAspirationPath), 50);
	loadAreaMask(Common::Path(kAreaPath));
	if (SoundManager *sound = _vm->getSoundManager()) {
		static constexpr const char *ids[6] = {
			"02-BB03",
			"02-BB02",
			"02-BS02",
			"02-BS03",
			"02-BS04",
			"02-BS05",
		};
		for (int i = 0; i < 6; i++)
			_sounds[i] = sound->load(false, Common::Path(Common::String::format(kSoundFormat, ids[i])), false);
	}
}

int PuzzleWaterslide::trait(int generatedIndex, int axis) const {
	return _puzzleZoombinis[_generationOrder[generatedIndex]]->_traits.getValue(static_cast<ZmbTrait::TraitIndex>(axis));
}

int PuzzleWaterslide::sharedAxis(int first, int second, bool rejectLast) {
	int visited = 0;
	while (visited != 15) {
		const int axis = _vm->_rnd->getRandomNumber(3);
		visited |= 1 << axis;
		if (trait(first, axis) == trait(second, axis))
			return rejectLast && visited == 15 ? -1 : axis;
	}
	return -1;
}

bool PuzzleWaterslide::findPair(int source, Common::Array<bool> &available, Pair &pair) {
	if (source < 0)
		return false;
	for (int candidate = 0; candidate < static_cast<int>(_generationOrder.size()); candidate++) {
		if (candidate == source || !available[candidate])
			continue;
		const int axis = sharedAxis(source, candidate, true);
		if (axis < 0)
			continue;
		pair.a = candidate;
		pair.b = source;
		pair.axis = axis;
		available[candidate] = false;
		available[source] = false;
		return true;
	}
	return false;
}

void PuzzleWaterslide::addEdge(int a, int b, int axis) {
	Edge edge;
	edge.a = a;
	edge.b = b;
	edge.axis = axis;
	_edges.push_back(edge);
}

void PuzzleWaterslide::generateEasy() {
	const int count = _generationOrder.size();
	if (count == 1) {
		_solution[0] = _generationOrder[0];
		return;
	}
	Common::Array<Pair> pairs;
	Common::Array<int> remaining, unmatched;
	const bool greedyPairing = _vm->isDemo() || _vm->useGreedyWaterslidePairing();
	if (greedyPairing) {
		for (int attempt = 0; attempt < 10; attempt++) {
			pairs.clear();
			Common::Array<bool> available;
			available.resize(count);
			for (int i = 0; i < count; i++)
				available[i] = true;
			int source = 0;
			int left = count;
			while (source < count && 1 < left) {
				for (int candidate = 0; candidate < count && source < count; candidate++) {
					if (candidate == source || !available[candidate] || !available[source])
						continue;
					const int axis = sharedAxis(source, candidate, true);
					if (0 <= axis) {
						Pair pair;
						pair.a = candidate;
						pair.b = source;
						pair.axis = axis;
						pairs.push_back(pair);
						available[candidate] = available[source] = false;
						left -= 2;
					}
					source += 1;
					while (source < count && !available[source])
						source += 1;
				}
			}
			remaining.clear();
			for (int i = 0; i < count; i++)
				if (available[i])
					remaining.push_back(i);
			if (left <= 1)
				break;
			if (_vm->isDemo() && attempt < 9 && !remaining.empty()) {
				const int unmatchedActor = _generationOrder[remaining[0]];
				_generationOrder.remove_at(remaining[0]);
				_generationOrder.insert_at(0, unmatchedActor);
			}
		}
	} else {
		for (int attempt = 0; attempt < 9; attempt++) {
			pairs.clear();
			remaining.clear();
			for (int i = 0; i < count; i++)
				remaining.push_back(i);
			for (int promote : unmatched) {
				for (uint i = 0; i < remaining.size(); i++) {
					if (remaining[i] == promote) {
						remaining.remove_at(i);
						break;
					}
				}
				remaining.insert_at(0, promote);
			}
			unmatched.clear();
			while (1 < remaining.size()) {
				const int source = remaining[0];
				bool matched = false;
				for (uint candidate = 1; candidate < remaining.size(); candidate++) {
					int axes[4] = {
						0,
						1,
						2,
						3,
					};
					for (int i = 0; i < 400; i++) {
						const int first = _vm->_rnd->getRandomNumber(3);
						const int second = _vm->_rnd->getRandomNumber(3);
						SWAP(axes[first], axes[second]);
					}
					for (int axis : axes) {
						if (trait(source, axis) != trait(remaining[candidate], axis))
							continue;
						Pair pair;
						pair.a = remaining[candidate];
						pair.b = source;
						pair.axis = axis;
						pairs.push_back(pair);
						remaining.remove_at(candidate);
						matched = true;
						break;
					}
					if (matched)
						break;
				}
				remaining.remove_at(0);
				if (!matched)
					unmatched.push_back(source);
			}
			if (static_cast<int>(pairs.size()) == count / 2)
				break;
		}
		for (int index : unmatched)
			remaining.push_back(index);
	}
	while (!remaining.empty()) {
		Pair pair;
		pair.b = remaining[0];
		remaining.remove_at(0);
		if (!remaining.empty()) {
			pair.a = remaining[0];
			remaining.remove_at(0);
		}
		pairs.push_back(pair);
	}
	if (!greedyPairing && !pairs.empty()) {
		const int first = _vm->_rnd->getRandomNumber(pairs.size() - 1);
		const int second = _vm->_rnd->getRandomNumber(pairs.size() - 1);
		SWAP(pairs[first], pairs[second]);
	}
	const int rows = (count + 1) / 2;
	for (int row = 0; row < count / 2; row++) {
		const Pair &pair = pairs[count / 2 - 1 - row];
		addEdge(row, rows + row, pair.axis);
		if (0 <= pair.a)
			_solution[row] = _generationOrder[pair.a];
		if (0 <= pair.b)
			_solution[rows + row] = _generationOrder[pair.b];
	}
	// Fill unlabeled positions from the actors not used by the displayed pair rules.
	Common::Array<bool> assigned;
	assigned.resize(count);
	for (int i = 0; i < count; i++)
		if (0 <= _solution[i])
			assigned[_solution[i]] = true;
	for (int i = 0; i < count; i++) {
		if (0 <= _solution[i])
			continue;
		for (int actor = 0; actor < count; actor++) {
			if (assigned[actor])
				continue;
			_solution[i] = actor;
			assigned[actor] = true;
			break;
		}
	}
}

void PuzzleWaterslide::generateMedium() {
	const int count = _generationOrder.size();
	Pair records[3][5];
	int root = 0;
	for (int attempt = 0; attempt < 100; attempt++) {
		Common::Array<bool> available;
		available.resize(count);
		for (int i = 0; i < count; i++)
			available[i] = true;
		for (int layer = 0; layer < 3; layer++)
			for (int arm = 0; arm < 5; arm++)
				records[layer][arm] = Pair();
		root = _vm->_rnd->getRandomNumber(count - 1);
		available[root] = false;
		int errors = 0;
		int generated = 0;
		for (int layer = 2; 0 <= layer; layer -= 1) {
			for (int arm = 0; arm < (count + 1) / 3 && generated < count; arm++) {
				const int source = layer == 2 ? root : records[layer + 1][arm].a;
				if (!findPair(source, available, records[layer][arm]))
					errors += 1;
				generated += 1;
			}
		}
		if (!errors)
			break;
	}
	_solution[15] = _generationOrder[root];
	for (int arm = 0; arm < 5; arm++) {
		for (int depth = 0; depth < 3; depth++) {
			const int slot = arm + depth * 5;
			if (count <= 1 + arm * 3 + depth)
				continue;
			const Pair &pair = records[2 - depth][arm];
			addEdge(depth == 0 ? 15 : slot - 5, slot, pair.axis);
			if (0 <= pair.a)
				_solution[slot] = _generationOrder[pair.a];
		}
	}
}

int PuzzleWaterslide::generateGraph(bool randomRoot) {
	const int count = _generationOrder.size();
	int nodes[16];
	Common::Array<bool> available;
	available.resize(count);
	for (int i = 0; i < 16; i++) {
		nodes[i] = -1;
		_solution[i] = -1;
	}
	for (int i = 0; i < count; i++)
		available[i] = true;
	_edges.clear();
	const int root = randomRoot ? _vm->_rnd->getRandomNumber(count - 1) : 0;
	nodes[0] = root;
	available[root] = false;
	int errors = 0;
	for (int placed = 1; placed < count; placed++) {
		bool assigned = false;
		for (int candidate = 0; candidate < count && !assigned; candidate++) {
			if (!available[candidate])
				continue;
			for (int parent = 0; parent < 16 && !assigned; parent++) {
				if (nodes[parent] < 0)
					continue;
				const int axis = sharedAxis(candidate, nodes[parent], false);
				if (axis < 0)
					continue;
				int tried = 0;
				while (tried != 31) {
					const int neighbor = _vm->_rnd->getRandomNumber(4);
					tried |= 1 << neighbor;
					const int slot = kNeighbors[parent][neighbor];
					if (slot < 0 || 0 <= nodes[slot])
						continue;
					nodes[slot] = candidate;
					available[candidate] = false;
					addEdge(parent, slot, axis);
					assigned = true;
					break;
				}
			}
		}
		while (!assigned) {
			const int slot = _vm->_rnd->getRandomNumber(15);
			if (0 <= nodes[slot])
				continue;
			for (int neighbor = 0; neighbor < 5; neighbor++) {
				const int parent = kNeighbors[slot][neighbor];
				if (parent < 0 || nodes[parent] < 0)
					continue;
				int candidate = 0;
				while (!available[candidate])
					candidate += 1;
				nodes[slot] = candidate;
				available[candidate] = false;
				addEdge(slot, parent, -1);
				errors += 1;
				assigned = true;
				break;
			}
		}
	}
	for (int slot = 0; slot < 16; slot++)
		if (0 <= nodes[slot])
			_solution[slot] = _generationOrder[nodes[slot]];
	return errors;
}

void PuzzleWaterslide::generateHard() {
	const int count = _generationOrder.size();
	if (count == 1) {
		_solution[0] = _generationOrder[0];
		return;
	}
	Common::Array<int> degrees;
	degrees.resize(count);
	for (int source = 0; source < count; source++) {
		Common::Array<bool> available;
		available.resize(count);
		for (int i = 0; i < count; i++)
			available[i] = true;
		for (int i = 0; i < count; i++) {
			Pair pair;
			if (findPair(source, available, pair))
				degrees[source] += 1;
		}
	}
	Common::Array<int> sorted;
	for (int i = 0; i < count; i++) {
		int lowest = 0;
		for (int candidate = 1; candidate < count; candidate++)
			if (degrees[candidate] < degrees[lowest])
				lowest = candidate;
		sorted.push_back(_generationOrder[lowest]);
		degrees[lowest] = 2000;
	}
	_generationOrder = sorted;
	if (!generateGraph(false) || !generateGraph(true))
		return;
	int bestErrors = 1000;
	Common::Array<Edge> best;
	int bestSolution[16];
	for (int attempt = 0; attempt < 10; attempt++) {
		const int errors = generateGraph(false);
		if (errors < bestErrors) {
			bestErrors = errors;
			best = _edges;
			for (int i = 0; i < 16; i++)
				bestSolution[i] = _solution[i];
		}
	}
	_edges = best;
	for (int i = 0; i < 16; i++)
		_solution[i] = bestSolution[i];
}

Common::Point32 PuzzleWaterslide::graphPosition(int slot) {
	static constexpr int baseX[4] = {
		310,
		220,
		100,
		10,
	};
	const int row = (15 - slot) % 4;
	return Common::Point32(baseX[slot / 4] + 40 * row, 80 * row - (slot / 4 % 2 == 0 ? 50 : 0));
}

const PuzzleWaterslide::GraphPlacement *PuzzleWaterslide::graphPlacement(const Edge &edge) {
	for (const GraphPlacement &placement : kGraphPlacements)
		if (placement.a == MIN(edge.a, edge.b) && placement.b == MAX(edge.a, edge.b))
			return &placement;
	return nullptr;
}

void PuzzleWaterslide::setupTargets() {
	const int count = _puzzleZoombinis.size();
	const int slots = _level == 1 ? count : 16;
	_targets.resize(slots);
	_activeSlot.resize(slots);
	for (int slot = 0; slot < slots; slot++) {
		Common::Point32 pos;
		if (_level == 1) {
			const int rows = (count + 1) / 2;
			const bool right = slot < rows;
			int row = slot - rows;
			int baseX = 243;
			int baseY = 130;
			if (right) {
				row = slot;
				baseX = 345;
				baseY = 129;
			}
			pos = Common::Point32(baseX + row * 28, baseY + row * 60);
			_activeSlot[slot] = true;
		} else if (_level == 2) {
			if (slot == 15)
				pos = Common::Point32(254, 363);
			else
				pos = Common::Point32(333 + slot / 5 * 100 + slot % 5 * 28, 250 + slot % 5 * 60);
			_activeSlot[slot] = slot == 15 || 1 + slot % 5 * 3 + slot / 5 < count;
		} else {
			pos = graphPosition(slot) + Common::Point32(163, 227);
			_activeSlot[slot] = slot == 0;
			for (const Edge &edge : _edges)
				if (edge.a == slot || edge.b == slot)
					_activeSlot[slot] = true;
		}
		if (!_activeSlot[slot])
			pos = Common::Point32(1000, 1000);
		ZmbDropTarget &target = _targets[slot];
		target.rect = Common::Rect32(pos.x, pos.y, pos.x + 60, pos.y + 60);
		target.callback = &onSlotChanged;
		target.callbackContext = this;
	}
}

bool PuzzleWaterslide::matches(const Edge &edge) const {
	const ZmbDropTarget &a = _targets[edge.a];
	const ZmbDropTarget &b = _targets[edge.b];
	if (!a.occupied || !b.occupied)
		return false;
	if (edge.axis < 0)
		return true;
	const ZmbTrait::TraitIndex axis = static_cast<ZmbTrait::TraitIndex>(edge.axis);
	return _puzzleZoombinis[a.zoombiniIndex]->_traits.getValue(axis) == _puzzleZoombinis[b.zoombiniIndex]->_traits.getValue(axis);
}

void PuzzleWaterslide::evaluateConnections() {
	if (_phase != kInteractive00)
		return;
	for (uint i = 0; i < _eligible.size(); i++)
		_eligible[i] = false;
	const int previous = _connectionCount;
	_connectionCount = 0;
	bool reached[16] = {};
	const int root = _level == 2 ? 15 : 0;
	if (!_targets.empty())
		reached[root] = _targets[root].occupied;
	for (Edge &edge : _edges)
		edge.connected = matches(edge);
	int mediumCount = 0;
	if (_level == 2) {
		bool valid[3][5] = {};
		for (const Edge &edge : _edges) {
			valid[edge.b / 5][edge.b % 5] = edge.connected;
			if (edge.connected)
				mediumCount += 1;
		}
		for (int arm = 0; arm < 5; arm++) {
			if (!valid[0][arm] || !valid[1][arm]) {
				if (_targets[arm + 10].occupied)
					mediumCount -= 1;
				if (_targets[arm + 5].occupied)
					mediumCount -= 1;
			}
			if (!valid[0][arm]) {
				if (_targets[arm + 5].occupied)
					mediumCount -= 1;
				if (_targets[arm].occupied)
					mediumCount -= 1;
			}
		}
	}
	if (1 < _level) {
		for (int pass = 0; pass < 16; pass++) {
			for (const Edge &edge : _edges) {
				if (edge.connected && (reached[edge.a] || reached[edge.b]))
					reached[edge.a] = reached[edge.b] = true;
			}
		}
	}
	for (Edge &edge : _edges) {
		edge.connected = edge.connected && (_level == 1 || reached[edge.a]);
		if (edge.connected) {
			_connectionCount += 1;
			_eligible[_targets[edge.a].zoombiniIndex] = true;
			_eligible[_targets[edge.b].zoombiniIndex] = true;
		}
	}
	if (1 < _level && reached[root])
		_eligible[_targets[root].zoombiniIndex] = true;
	if (_level == 1 && _targets.size() % 2 && _targets[_targets.size() / 2].occupied)
		_eligible[_targets[_targets.size() / 2].zoombiniIndex] = true;
	if (_level == 2)
		_connectionCount = mediumCount;
	if (previous < _connectionCount)
		playSound(_sounds[2]);
	else if (_connectionCount < previous)
		playSound(_sounds[3]);
}

void PuzzleWaterslide::onSlotChanged(void *context, int slot, int zoombini) {
	PuzzleWaterslide *page = static_cast<PuzzleWaterslide *>(context);
	const Common::Rect32 &rect = page->_targets[slot].rect;
	const Common::Point32 offset = page->_level < 3 ? Common::Point32(-5, -32) : Common::Point32(9, -25);
	page->_puzzleZoombinis[zoombini]->setPosition(Common::Point32(rect.left, rect.top) + offset);
	page->evaluateConnections();
}

void PuzzleWaterslide::playSound(int handle) {
	if (0 <= handle && _vm->getSoundManager())
		_vm->getSoundManager()->playWithVolume(handle, _vm->getSoundManager()->_volumeSFX);
}

void PuzzleWaterslide::activateValve() {
	if (_phase != kInteractive00)
		return;
	bool any = false;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++)
		any = any || _eligible[i];
	if (!any)
		return;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		_puzzleZoombinis[i]->_inputEnabled = false;
		if (_eligible[i])
			_puzzleZoombinis[i]->_puzzleStatus = 1;
	}
	_phase = kValve01;
	_valveRunner->setHitTestEnabled(false);
	playSound(_sounds[4]);
}

void PuzzleWaterslide::dischargeNext() {
	Common::Array<int> order;
	order.resize(_targets.size());
	int size = 0;
	if (_vm->isDemo()) {
		for (int slot = 0; slot < 16; slot++)
			order[slot] = slot;
		size = 16;
	} else if (_level == 1) {
		const int rows = (_targets.size() + 1) / 2;
		for (int row = 0; row < rows; row++) {
			order[size] = row;
			size += 1;
			if (rows + row < static_cast<int>(_targets.size())) {
				order[size] = rows + row;
				size += 1;
			}
		}
	} else if (_level == 2) {
		order[size] = 15;
		size += 1;
		for (int arm = 0; arm < 5; arm++)
			for (int depth = 0; depth < 3; depth++) {
				order[size] = arm + 5 * depth;
				size += 1;
			}
	} else {
		for (int i = 0; i < 16; i++)
			order[i] = i;
		size = 16;
	}
	for (int i = 0; i < size; i++) {
		ZmbDropTarget &target = _targets[order[i]];
		if (!target.occupied || !_eligible[target.zoombiniIndex])
			continue;
		target.occupied = false;
		_heldZoombini = target.zoombiniIndex;
		_vm->_zoombiniWalkingFlag = true;
		_eligible[_heldZoombini] = false;
		ZoombiniRunner *actor = _puzzleZoombinis[_heldZoombini];
		actor->setPosition(actor->_screenPos - Common::Point32(1, 1));
		_dischargeStart = _vm->getGameTickCount();
		actor->startAnimation(_aspiration, 33, _dischargeStart);
		actor->setAnimationCompleteCallback(&onAspirationComplete, this);
		_cascadeRunner->startAt(_cascadePos, _dischargeStart);
		playSound(_sounds[1]);
		playSound(_sounds[5]);
		return;
	}
	_phase = kFinished03;
	_vm->restartGoBlink();
}

void PuzzleWaterslide::onAspirationComplete(void *context, ZoombiniRunner *zoombini) {
	(void)context;
	zoombini->_hidden = true;
}

void PuzzleWaterslide::onCascadeComplete(void *context, AnimationRunner *runner) {
	(void)runner;
	PuzzleWaterslide *page = static_cast<PuzzleWaterslide *>(context);
	if (0 <= page->_heldZoombini)
		page->_puzzleZoombinis[page->_heldZoombini]->_hidden = true;
	page->_heldZoombini = -1;
}

void PuzzleWaterslide::onUpdate() {
	const uint32 tick = _vm->getGameTickCount();
	SoundManager *sound = _vm->getSoundManager();
	if (_goPending && (!sound || !sound->hasPendingSpeech())) {
		_vm->_mapTransitionSourcePageId = kPageWaterslide;
		_vm->requestPageChange(kPageMapTrans);
		return;
	}
	if (_phase == kValve01 && (_sounds[4] < 0 || !sound || !sound->isPlaying(_sounds[4]))) {
		if (sound && 3 <= _connectionCount)
			sound->queueSpeech(Common::Path(8 <= _connectionCount ? kPraisePath : kPartialPraisePath));
		_phase = kDischarge02;
	}
	if (_phase == kDischarge02) {
		// Optional cascade art must not prevent the next actor from leaving.
		if (0 <= _heldZoombini && !_cascade->getFrameCount() && 1300 < tick - _dischargeStart)
			onCascadeComplete(this, _cascadeRunner);
		if (_heldZoombini < 0)
			dischargeNext();
	}
	for (ZoombiniRunner *actor : _puzzleZoombinis) {
		if (actor->_puzzleStatus == 1 && !actor->_animationActive && _vm->_rnd->getRandomNumber(19) == 1)
			actor->startAnimation(_idle, 33, tick);
	}
	for (ZoombiniRunner *actor : _puzzleZoombinis)
		actor->updateAnimation(tick);
}

void PuzzleWaterslide::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

void PuzzleWaterslide::drawBoard(ManagedSurface32 *screen) const {
	Gfx *gfx = _vm->_gfx;
	if (_level == 1) {
		const int rows = (_targets.size() + 1) / 2;
		gfx->drawPageRleBlock(screen, Common::String::format(kLargePipeFormat, _level), Common::Point32(136, 102));
		gfx->drawPageRleBlock(screen, Common::String::format(kHorizontalFormat, kPipeColors[1]), Common::Point32(418, 149));
		for (int row = rows - 2; 0 <= row; row -= 1)
			gfx->drawPageRleBlock(screen, kMiniDiagonalPath, Common::Point32(378 + 28 * row, 150 + 60 * row));
		const int pairs = _targets.size() / 2;
		Common::Point32 end;
		if (_targets.size() % 2)
			end = Common::Point32(376 + 28 * pairs, 150 + 60 * pairs);
		else
			end = Common::Point32(348 + 28 * pairs, 90 + 60 * pairs);
		gfx->drawPageRleBlock(screen, kMiniHorizontalPath, end);
		for (uint row = 0; row < _edges.size(); row++) {
			const char *pipeColor = kPipeColors[0];
			if (_edges[row].connected)
				pipeColor = kPipeColors[1];
			gfx->drawPageRleBlock(screen, Common::String::format(kHorizontalFormat, pipeColor), Common::Point32(268 + 28 * row, 148 + 60 * row));
		}
	} else if (_level == 2) {
		gfx->drawPageRleBlock(screen, Common::String::format(kLargePipeFormat, _level), Common::Point32(116, 150));
		for (const Edge &edge : _edges) {
			const int arm = edge.b % 5;
			const char *pipeColor = kPipeColors[0];
			if (edge.connected)
				pipeColor = kPipeColors[1];
			if (edge.a == 15)
				gfx->drawPageRleBlock(screen, Common::String::format(kSelectorFormat, pipeColor, arm + 1), Common::Point32(280, 270));
			else
				gfx->drawPageRleBlock(screen, Common::String::format(kHorizontalFormat, pipeColor), Common::Point32(345 + edge.a / 5 * 100 + arm * 30, 265 + arm * 60));
		}
	} else {
		static constexpr int offsetX[4] = {
			188,
			191,
			198,
			198,
		};
		static constexpr int offsetY[4] = {
			245,
			248,
			250,
			205,
		};
		for (const Edge &edge : _edges) {
			const GraphPlacement *placement = graphPlacement(edge);
			if (!placement)
				continue;
			const int type = placement->type;
			const Common::Point32 pos = graphPosition(MAX(edge.a, edge.b)) + Common::Point32(offsetX[type], offsetY[type]);
			const char *pipeColor = kHardPipeColors[0];
			if (edge.connected)
				pipeColor = kHardPipeColors[1];
			gfx->drawPageRleBlock(screen, Common::String::format(kHardPipeFormat, pipeColor, type + 1), pos);
		}
		gfx->drawPageRleBlock(screen, Common::String::format(kHardPipeFormat, kHardPipeColors[1], 1), Common::Point32(625, 430));
		gfx->drawPageRleBlock(screen, kOutletPath, Common::Point32(432, 517));
	}
	for (uint slot = 0; slot < _targets.size(); slot++) {
		if (!_activeSlot[slot])
			continue;
		Common::Point32 pos;
		if (_level == 1) {
			const int rows = (_targets.size() + 1) / 2;
			int row = slot - rows;
			int baseX = 248;
			if (slot < static_cast<uint>(rows)) {
				row = slot;
				baseX = 348;
			}
			pos = Common::Point32(baseX + 28 * row, 137 + 60 * row);
		} else if (_level == 2) {
			if (slot == 15)
				pos = Common::Point32(260, 370);
			else
				pos = Common::Point32(338 + slot / 5 * 100 + slot % 5 * 28, 257 + slot % 5 * 60);
		} else {
			pos = graphPosition(slot) + Common::Point32(183, 240);
		}
		gfx->drawPageRleBlock(screen, kEdgePath, pos);
	}
	static const Common::Point32 rootLabels[5] = {
		{311, 313},
		{315, 347},
		{344, 376},
		{364, 411},
		{391, 454},
	};
	for (uint i = 0; i < _edges.size(); i++) {
		const Edge &edge = _edges[i];
		if (edge.axis < 0)
			continue;
		Common::Point32 pos;
		if (_level == 1) {
			pos = Common::Point32(305 + 28 * i, 140 + 60 * i);
		} else if (_level == 2) {
			if (edge.a == 15)
				pos = rootLabels[edge.b];
			else
				pos = Common::Point32(390 + edge.a / 5 * 100 + edge.b % 5 * 30, 260 + edge.b % 5 * 60);
		} else {
			const GraphPlacement *placement = graphPlacement(edge);
			if (!placement)
				continue;
			pos = Common::Point32(placement->labelX, placement->labelY);
		}
		gfx->drawPageRleBlock(screen, kPastillePath, pos);
		gfx->drawPageRleBlock(screen, Common::String::format(kTraitFormat, edge.axis + 1), pos);
	}
}

void PuzzleWaterslide::drawDecorations(ManagedSurface32 *screen) {
	const uint32 tick = _vm->getGameTickCount();
	_vm->_gfx->drawAndUpdateAnimationRunner(screen, _treeRunner, tick, 0, ManagedSurface32::kScreenSize.width);
	_vm->_gfx->drawAndUpdateAnimationRunner(screen, _fountainRunner, tick, 0, ManagedSurface32::kScreenSize.width);
	if (_cascadeRunner->isActive())
		_vm->_gfx->drawAndUpdateAnimationRunner(screen, _cascadeRunner, tick, 0, ManagedSurface32::kScreenSize.width);
	else
		_vm->_gfx->drawAnimationFrame(screen, _cascade, 0, _cascadePos);
	// The valve overlaps the cascade's left edge and must remain in front of it.
	if (!_valveRunner->isActive())
		_vm->_gfx->drawAnimationFrame(screen, _valve, 0, _valvePos);
	_vm->_gfx->drawAndUpdateAnimationRunner(screen, _valveRunner, tick, 0, ManagedSurface32::kScreenSize.width);
}

void PuzzleWaterslide::onRenderContent(ManagedSurface32 *screen) {
	drawBoard(screen);
	drawDecorations(screen);
}

void PuzzleWaterslide::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleWaterslide::onActorsRendered() {
	for (ZoombiniRunner *actor : _puzzleZoombinis)
		actor->advanceAnimationAfterDraw();
}

EventHandleResult PuzzleWaterslide::onLButtonDown(const Common::Point &pos) {
	if (_phase != kInteractive00 || !_valveRunner->containsHitPoint(Common::Point32(pos.x, pos.y)))
		return EventHandleResult::kPassthrough;
	if (!_valveRunner->isActive()) {
		activateValve();
		_valveRunner->start(_vm->getGameTickCount());
	}
	return EventHandleResult::kConsumed;
}

EventHandleResult PuzzleWaterslide::onLButtonUp(const Common::Point &pos) {
	if (_phase != kInteractive00)
		return EventHandleResult::kPassthrough;
	if (_valveRunner->containsHitPoint(Common::Point32(pos.x, pos.y))) {
		return EventHandleResult::kConsumed;
	}
	const ZmbDropResult result = ZoombiniRunner::handlePointerInput(_puzzleZoombinis, Common::Point32(pos.x, pos.y), true,
																	_pickup, _vm->getGameTickCount(), &_targets, getAreaMask());
	return result == ZmbDropResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

EventHandleResult PuzzleWaterslide::onMouseMove(const Common::Point &pos) {
	if (_phase != kInteractive00)
		return EventHandleResult::kPassthrough;
	const ZmbDropResult result = ZoombiniRunner::handlePointerInput(_puzzleZoombinis, Common::Point32(pos.x, pos.y), false,
																	_pickup, _vm->getGameTickCount(), &_targets, getAreaMask());
	return result == ZmbDropResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

int PuzzleWaterslide::countFreeZoombinis() const {
	int count = 0;
	for (const ZoombiniRunner *actor : _puzzleZoombinis)
		if (actor->_puzzleStatus == 0)
			count += 1;
	return count;
}

bool PuzzleWaterslide::canUseGoButton() const {
	return _vm->isDemo() || (!_goPending && _vm->_zoombiniWalkingFlag);
}

bool PuzzleWaterslide::onGoButtonPressed() {
	if (_vm->isDemo()) {
		_vm->quitGame();
		return false;
	}
	if (!_vm->_isSavedGame)
		return true;
	if (_goPending)
		return false;
	const int freeCount = countFreeZoombinis();
	if (0 < freeCount && freeCount < 4)
		return true;
	Common::String speech = kRetreatSpeech;
	if (!freeCount) {
		const int variant = _vm->_rnd->getRandomNumber(4) + 1;
		speech = Common::String::format(kGoSpeechFormat, variant);
	}
	if (SoundManager *sound = _vm->getSoundManager())
		sound->queueSpeech(Common::Path(speech));
	_goPending = true;
	return false;
}

bool PuzzleWaterslide::debugPlacementMatches(int slot, int actor, const Common::Array<int> &assignment) const {
	for (const Edge &edge : _edges) {
		if (edge.axis < 0 || (edge.a != slot && edge.b != slot))
			continue;
		const int other = assignment[edge.a == slot ? edge.b : edge.a];
		if (other < 0)
			continue;
		const ZmbTrait::TraitIndex axis = static_cast<ZmbTrait::TraitIndex>(edge.axis);
		if (_puzzleZoombinis[actor]->_traits.getValue(axis) != _puzzleZoombinis[other]->_traits.getValue(axis))
			return false;
	}
	return true;
}

bool PuzzleWaterslide::debugFindPlacement(Common::Array<int> &assignment, uint32 used, int &budget) const {
	if (budget <= 0)
		return false;
	budget -= 1;
	int bestSlot = -1;
	int bestCount = 17;
	uint32 bestCandidates = 0;
	for (uint slot = 0; slot < assignment.size(); slot++) {
		if (!_activeSlot[slot] || 0 <= assignment[slot])
			continue;
		int count = 0;
		uint32 candidates = 0;
		for (uint actor = 0; actor < _puzzleZoombinis.size(); actor++) {
			if (!(used & (1U << actor)) && debugPlacementMatches(slot, actor, assignment)) {
				candidates |= 1U << actor;
				count += 1;
			}
		}
		if (!count)
			return false;
		if (count < bestCount) {
			bestSlot = slot;
			bestCount = count;
			bestCandidates = candidates;
		}
	}
	if (bestSlot < 0)
		return true;
	for (uint actor = 0; actor < _puzzleZoombinis.size(); actor++) {
		if (!(bestCandidates & (1U << actor)))
			continue;
		assignment[bestSlot] = actor;
		if (debugFindPlacement(assignment, used | (1U << actor), budget))
			return true;
		assignment[bestSlot] = -1;
	}
	return false;
}

Common::String PuzzleWaterslide::debugGetAnswer() const {
	Common::String answer = debugAnswerHeader();
	answer += "\n";
	Common::Array<int> assignment;
	assignment.resize(_activeSlot.size());
	uint32 used = 0;
	bool generatedValid = true;
	for (uint slot = 0; slot < assignment.size(); slot++) {
		assignment[slot] = _activeSlot[slot] ? _solution[slot] : -1;
		if (!_activeSlot[slot])
			continue;
		const int actor = assignment[slot];
		if (actor < 0 || static_cast<int>(_puzzleZoombinis.size()) <= actor || (used & (1U << actor))) {
			generatedValid = false;
		} else {
			used |= 1U << actor;
		}
	}
	if (generatedValid) {
		for (uint slot = 0; slot < assignment.size(); slot++)
			if (_activeSlot[slot] && !debugPlacementMatches(slot, assignment[slot], assignment))
				generatedValid = false;
	}
	bool hasPlacement = generatedValid;
	if (generatedValid) {
		answer += "  Place Zoombinis at these pipe mouths (validated generation placement):\n";
	} else {
		for (uint slot = 0; slot < assignment.size(); slot++)
			assignment[slot] = -1;
		int budget = 200000;
		hasPlacement = debugFindPlacement(assignment, 0, budget);
		if (hasPlacement)
			answer += "  Place Zoombinis at these pipe mouths (searched replacement):\n";
		else
			answer += "  No complete placement found within the debugger search limit.\n";
	}
	if (hasPlacement) {
		for (uint slot = 0; slot < assignment.size(); slot++) {
			if (!_activeSlot[slot])
				continue;
			const Common::Rect32 &rect = _targets[slot].rect;
			answer += Common::String::format("    Near (%d, %d): %s\n", rect.left, rect.top, debugActorDescription(assignment[slot]).c_str());
		}
	}
	answer += "  Pipe connections (trait shared by both ends):\n";
	for (const Edge &edge : _edges) {
		const char *axis = "any trait";
		if (0 <= edge.axis)
			axis = ZmbTrait::debugTraitName(static_cast<ZmbTrait::TraitIndex>(edge.axis));
		const Common::Rect32 &first = _targets[edge.a].rect;
		const Common::Rect32 &second = _targets[edge.b].rect;
		answer += Common::String::format("    (%d, %d) <-> (%d, %d): %s\n", first.left, first.top, second.left, second.top, axis);
	}
	return answer;
}

Common::String PuzzleWaterslide::debugGetChanceDetails() const {
	return Common::String::format("Placements are unlimited before releasing the valve. Connected: %d; phase: %d.\n", _connectionCount, _phase);
}

} // End of namespace Zoombini2
