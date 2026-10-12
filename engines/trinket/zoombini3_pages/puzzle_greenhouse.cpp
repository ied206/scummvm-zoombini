/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/zoombini3_pages/puzzle_greenhouse.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/tokenizer.h"
#include "common/util.h"
#include "trinket/trinket.h"
#include <math.h>

namespace Trinket {
namespace Zoombini3 {

GreenhouseRules::GreenhouseRules() = default;

void GreenhouseRules::shuffle(uint *values, uint count, uint first, Shell::Random &random) {
	for (uint i = 0; i < count; i++)
		values[i] = first + i;
	for (uint i = 0; i < 10 * count; i++) {
		uint a = random.draw() % count;
		uint b = random.draw() % count;
		SWAP(values[a], values[b]);
	}
}

bool GreenhouseRules::loadTemplate(const OM::XMLParser &templates, uint family, uint attribute, int orientation, Shell::Random &random) {
	Common::String path = Common::String::format("Z3A4Data/Template%u", family);
	const Common::Array<OM::XMLParser::Element> &elements = templates.elements();
	uint count = 0;
	for (uint i = 0; i < elements.size(); i++)
		if (elements[i].path == path)
			count = atoi(elements[i].attributes.getValOrDefault("Num").c_str());
	if (!count)
		return false;
	path += Common::String::format("/T%u", random.draw() % count + 1);
	const OM::XMLParser::Element *selected = nullptr;
	for (uint i = 0; i < elements.size(); i++)
		if (elements[i].path == path)
			selected = &elements[i];
	if (!selected)
		return false;
	Common::String beetles = selected->attributes.getValOrDefault("BE");
	if (!beetles.empty()) {
		Common::StringTokenizer tokens(beetles);
		for (uint i = 0; i < 3; i++) {
			if (tokens.empty())
				return false;
			_beetleEntries[i] = atoi(tokens.nextToken().c_str());
			if (_beetleEntries[i] < 0 || 12 <= _beetleEntries[i])
				return false;
		}
	}
	uint values[144];
	for (uint row = 0; row < 12; row++) {
		Common::StringTokenizer tokens(selected->attributes.getValOrDefault(Common::String::format("line%u", row + 1)));
		for (uint column = 0; column < 12; column++) {
			if (tokens.empty())
				return false;
			values[row * 12 + column] = atoi(tokens.nextToken().c_str());
			if (family < values[row * 12 + column])
				return false;
		}
	}
	if (orientation == -1)
		orientation = random.draw() % 4;
	static constexpr uint shifts[] = {
		0,
		2,
		5,
	};
	for (uint row = 0; row < 12; row++) {
		for (uint column = 0; column < 12; column++) {
			uint targetRow = row, targetColumn = column;
			if (orientation == 1 || orientation == 3)
				targetColumn = 11 - column;
			if (orientation == 2 || orientation == 3)
				targetRow = 11 - row;
			if (orientation == 90) {
				targetRow = column;
				targetColumn = 11 - row;
			}
			_cells[targetRow * 12 + targetColumn] += values[row * 12 + column] << shifts[attribute];
		}
	}
	return true;
}

bool GreenhouseRules::generate(uint level, const OM::XMLParser &templates, Shell::Random &random, Common::Point mothSize) {
	if (level < 1 || 3 < level || mothSize.x <= 0 || mothSize.y <= 0 || 155 <= mothSize.x || 350 <= mothSize.y)
		return false;
	*this = GreenhouseRules();
	_level = level;
	for (uint i = 0; i < 12; i++) {
		bool overlaps;
		do {
			int x = random.draw() % (155 - mothSize.x);
			int y = 150 + random.draw() % (350 - mothSize.y);
			_moths[i].supply = Common::Point(x, y);
			Common::Rect bounds(x, y, x + mothSize.x, y + mothSize.y);
			overlaps = false;
			for (uint j = 0; j < i; j++) {
				Common::Point p = _moths[j].supply;
				if (bounds.intersects(Common::Rect(p.x, p.y, p.x + mothSize.x, p.y + mothSize.y)))
					overlaps = true;
			}
		} while (overlaps);
	}
	uint shapes = level == 3 ? 6 : 4;
	uint colors = level == 3 ? 6 : 5;
	if (!loadTemplate(templates, 3, 0, level == 3 ? 90 : -1, random) ||
		!loadTemplate(templates, shapes, 1, -1, random) || !loadTemplate(templates, colors, 2, -1, random))
		return false;
	for (uint i = 0; i < 144; i++) {
		if (!(_cells[i] & 3))
			_cells[i] += random.draw() % 3 + 1;
		if (!(_cells[i] & 28))
			_cells[i] += (random.draw() % shapes + 1) << 2;
		if (!(_cells[i] & 224))
			_cells[i] += (random.draw() % colors + 1) << 5;
		_original[i] = _cells[i];
	}
	if (1 < level) {
		while (_scramble.size() < 20) {
			uint a = random.draw() % 144, b = random.draw() % 144;
			if (a == b || (level == 3 && (_cells[a] & 3) != (_cells[b] & 3)))
				continue;
			SWAP(_cells[a], _cells[b]);
			Swap swap = {a, b};
			_scramble.push_back(swap);
		}
	}
	calculateFlow();
	uint leaves[3], shapeOrder[6], colorOrder[6], unusedOrder[12];
	shuffle(leaves, 3, 1, random);
	shuffle(shapeOrder, shapes, 1, random);
	shuffle(colorOrder, colors, 1, random);
	shuffle(unusedOrder, 12, 0, random);
	uint index = 0;
	if (level != 3) {
		for (uint i = 0; i < 3; i++) {
			_moths[index].trait = leaves[i];
			_moths[index].animation = leaves[i];
			index += 1;
		}
	}
	for (uint i = 0; i < shapes; i++) {
		_moths[index].trait = shapeOrder[i] << 2;
		_moths[index].animation = shapeOrder[i] + 3;
		index += 1;
	}
	for (uint i = 0; i < colors; i++) {
		_moths[index].trait = colorOrder[i] << 5;
		_moths[index].animation = colorOrder[i] + 9;
		index += 1;
	}
	return index == 12;
}

bool GreenhouseRules::matches(uint cell, uint trait) const {
	return (_cells[cell] & 3) == trait || (_cells[cell] & 28) == trait || (_cells[cell] & 224) == trait;
}

bool GreenhouseRules::traversable(uint trait, int column, int row) const {
	if (column < 0 || row < 0 || 12 <= row)
		return false;
	return 12 <= column || (!occupied(row * 12 + column) && matches(row * 12 + column, trait));
}

bool GreenhouseRules::canEnter(uint index, int row) const {
	return index < 12 && _moths[index].state == kSupply00 && traversable(_moths[index].trait, 0, row);
}

void GreenhouseRules::logVisit(Moth &moth, uint32 tick) {
	uint cell = moth.row * 12 + moth.column;
	moth.visits[cell] = tick;
	if (!_swaps) {
		moth.counts[cell] += 1;
		if (moth.counts[cell] == 5)
			moth.stuck = true;
	}
}

bool GreenhouseRules::enter(uint index, int row) {
	if (!canEnter(index, row))
		return false;
	Moth &moth = _moths[index];
	moth.state = kCrossing01;
	moth.column = 0;
	moth.row = row;
	logVisit(moth, 0);
	_mothOccupancy[row * 12] = true;
	return true;
}

GreenhouseRules::Direction GreenhouseRules::nextDirection(uint index, int &column, int &row) const {
	const Moth &moth = _moths[index];
	column = moth.column;
	row = moth.row;
	static constexpr int dx[] = {
		0,
		0,
		0,
		1,
		-1,
	};
	static constexpr int dy[] = {
		0,
		-1,
		1,
		0,
		0,
	};
	static constexpr uint priorities[4][3] = {{3, 4, 2}, {4, 3, 1}, {2, 1, 4}, {1, 2, 3}};
	uint32 oldest = UINT_MAX, visits[5] = {
								  UINT_MAX,
								  UINT_MAX,
								  UINT_MAX,
								  UINT_MAX,
								  UINT_MAX,
							  };
	for (uint direction = 1; direction <= 4; direction++) {
		int x = column + dx[direction], y = row + dy[direction];
		if (traversable(moth.trait, x, y)) {
			visits[direction] = 12 <= x ? 0 : moth.visits[y * 12 + x];
			oldest = MIN(oldest, visits[direction]);
		}
	}
	if (oldest == UINT_MAX)
		return kNone00;
	uint direction = moth.direction;
	if (visits[direction] != oldest) {
		for (uint i = 0; i < 3; i++) {
			uint candidate = priorities[moth.direction - 1][i];
			if (visits[candidate] == oldest) {
				direction = candidate;
				break;
			}
		}
	}
	column += dx[direction];
	row += dy[direction];
	return static_cast<Direction>(direction);
}

bool GreenhouseRules::advance(uint index, uint32 tick) {
	if (12 <= index || _moths[index].state != kCrossing01)
		return false;
	Moth &moth = _moths[index];
	int column, row;
	Direction direction = nextDirection(index, column, row);
	if (direction == kNone00) {
		moth.stuck = true;
		return false;
	}
	if (moth.direction != direction) {
		moth.direction = direction;
		return false;
	}
	_mothOccupancy[moth.row * 12 + moth.column] = false;
	moth.column = column;
	moth.row = row;
	if (12 <= column) {
		moth.state = kPassed02;
		_passed += 1;
	} else {
		logVisit(moth, tick);
		_mothOccupancy[row * 12 + column] = true;
	}
	return true;
}

bool GreenhouseRules::allMothsStuck() const {
	if (_swaps)
		return false;
	for (uint i = 0; i < 12; i++) {
		if (_moths[i].state == kSupply00 || (_moths[i].state == kCrossing01 && !_moths[i].stuck))
			return false;
	}
	return true;
}

bool GreenhouseRules::swapPlants(uint first, uint second) {
	if (_level == 1 || !_swaps || 144 <= first || 144 <= second || first == second || occupied(first) || occupied(second))
		return false;
	SWAP(_cells[first], _cells[second]);
	_swaps -= 1;
	calculateFlow();
	return true;
}

int GreenhouseRules::buildFlow(uint trait, int column, int row, uint *types, FlowNode *nodes, uint &depth, uint &maximum) {
	if (column < 0 || 12 <= column || row < 0 || 12 <= row)
		return -1;
	uint cell = row * 12 + column;
	if (types[cell] != trait)
		return -1;
	types[cell] = 0;
	FlowNode &node = nodes[cell];
	depth += 1;
	node.down = buildFlow(trait, column, row + 1, types, nodes, depth, maximum);
	if (0 <= node.down) {
		maximum = MAX(maximum, depth);
		node.children += 1;
		_flow[cell] |= 1;
	}
	depth -= 1;
	node.left = buildFlow(trait, column - 1, row, types, nodes, depth, maximum);
	if (0 <= node.left) {
		node.children += 1;
		_flow[cell] |= 2;
	}
	node.right = buildFlow(trait, column + 1, row, types, nodes, depth, maximum);
	if (0 <= node.right) {
		node.children += 1;
		_flow[cell] |= 4;
	}
	return cell;
}

bool GreenhouseRules::traverseFlow(int index, uint back, FlowNode *nodes, uint &depth, uint maximum) {
	if (index < 0)
		return false;
	FlowNode &node = nodes[index];
	bool reaches = false;
	if (node.children) {
		uint mask = 0, down = back, left = back, right = back;
		if (0 <= node.down) {
			depth += 1;
			if (traverseFlow(node.down, 8, nodes, depth, maximum)) {
				reaches = true;
				mask |= 1;
				down = nodes[node.down].distance + 1;
			}
			depth -= 1;
		}
		if (traverseFlow(node.left, 4, nodes, depth, maximum)) {
			reaches = true;
			mask |= 2;
			left = nodes[node.left].distance + 1;
		}
		if (traverseFlow(node.right, 2, nodes, depth, maximum)) {
			reaches = true;
			mask |= 4;
			right = nodes[node.right].distance + 1;
		}
		if (mask == 7) {
			node.distance = MAX(down, MIN(left, right));
		} else if (mask) {
			uint distance = UINT_MAX;
			if (mask & 1)
				distance = MIN(distance, down);
			if (mask & 2)
				distance = MIN(distance, left);
			if (mask & 4)
				distance = MIN(distance, right);
			if (distance < down)
				mask &= ~1;
			if (distance < left)
				mask &= ~2;
			if (distance < right)
				mask &= ~4;
			node.distance = distance;
		}
		_flow[index] = reaches ? mask : back;
	} else if (depth == maximum) {
		reaches = true;
	} else {
		_flow[index] = back;
	}
	if (index / 12 == 11)
		_flow[index] = 1;
	return reaches;
}

void GreenhouseRules::calculateFlow() {
	uint types[144];
	FlowNode nodes[144];
	for (uint i = 0; i < 144; i++) {
		types[i] = _cells[i] & 3;
		_flow[i] = 0;
		nodes[i] = {-1, -1, -1, 0, 0};
	}
	for (uint i = 0; i < 144; i++) {
		if (!types[i])
			continue;
		uint depth = 1, maximum = 1;
		int root = buildFlow(types[i], i % 12, i / 12, types, nodes, depth, maximum);
		depth = 1;
		traverseFlow(root, 0, nodes, depth, maximum);
	}
}

bool GreenhouseRules::findPath(uint index, Common::Array<uint> &path) const {
	path.clear();
	if (12 <= index || _moths[index].state == kPassed02)
		return false;
	int previous[144];
	uint queue[144], head = 0, tail = 0;
	for (uint i = 0; i < 144; i++)
		previous[i] = -2;
	const Moth &moth = _moths[index];
	if (moth.state == kCrossing01) {
		uint cell = moth.row * 12 + moth.column;
		queue[tail++] = cell;
		previous[cell] = -1;
	} else {
		for (uint row = 0; row < 12; row++) {
			if (canEnter(index, row)) {
				queue[tail++] = row * 12;
				previous[row * 12] = -1;
			}
		}
	}
	static constexpr int dx[] = {
		1,
		0,
		0,
		-1,
	},
						 dy[] = {
							 0,
							 1,
							 -1,
							 0,
						 };
	while (head < tail) {
		uint cell = queue[head++];
		if (cell % 12 == 11) {
			for (int current = cell; 0 <= current; current = previous[current])
				path.insert_at(0, current);
			return true;
		}
		for (uint direction = 0; direction < 4; direction++) {
			int x = cell % 12 + dx[direction], y = cell / 12 + dy[direction];
			if (x < 0 || 12 <= x || y < 0 || 12 <= y)
				continue;
			uint next = y * 12 + x;
			if (previous[next] == -2 && traversable(moth.trait, x, y)) {
				previous[next] = cell;
				queue[tail++] = next;
			}
		}
	}
	return false;
}

bool GreenhouseRules::findRestoration(Common::Array<Swap> &swaps) const {
	swaps.clear();
	if (_level == 1)
		return true;
	uint board[144];
	memcpy(board, _cells, sizeof(board));
	// The generated solution is retained only for the read-only debug solver.
	for (int i = static_cast<int>(_scramble.size()) - 1; 0 <= i; i--) {
		const Swap &swap = _scramble[i];
		if (board[swap.first] == board[swap.second])
			continue;
		if (occupied(swap.first) || occupied(swap.second))
			break;
		SWAP(board[swap.first], board[swap.second]);
		swaps.push_back(swap);
	}
	if (memcmp(board, _original, sizeof(board)) || _swaps < swaps.size()) {
		swaps.clear();
		memcpy(board, _cells, sizeof(board));
		for (uint i = 0; i < 144; i++) {
			if (board[i] == _original[i])
				continue;
			if (occupied(i))
				return false;
			uint donor = 144;
			for (uint j = i + 1; j < 144; j++) {
				if (board[j] == _original[i] && !occupied(j)) {
					donor = j;
					if (_original[j] == board[i])
						break;
				}
			}
			if (donor == 144)
				return false;
			SWAP(board[i], board[donor]);
			Swap swap = {i, donor};
			swaps.push_back(swap);
		}
	}
	return swaps.size() <= _swaps;
}

const uint GreenhouseMachine::kFlowers[36] = {
	16200,
	16201,
	16202,
	16203,
	16242,
	16243,
	16204,
	16205,
	16206,
	16207,
	16244,
	16245,
	16208,
	16209,
	16210,
	16211,
	16246,
	16247,
	16212,
	16213,
	16214,
	16215,
	16248,
	16249,
	16216,
	16217,
	16218,
	16219,
	16250,
	16251,
	16252,
	16253,
	16254,
	16255,
	16256,
	16257,
};

uint GreenhouseMachine::traitResource(uint index) {
	static constexpr uint resources[] = {
		0,
		16065,
		16066,
		16067,
		16061,
		16062,
		16063,
		16064,
		16084,
		16085,
		16056,
		16057,
		16058,
		16059,
		16060,
		16083,
	};
	return index < ARRAYSIZE(resources) ? resources[index] : 0;
}

constexpr const char *GreenhouseMachine::kAnimation;
constexpr const char *GreenhouseMachine::kTemplate;

GreenhouseMachine::GreenhouseMachine(TrinketEngine &vm, Shell::Scene &scene, uint level) : _vm(vm), _scene(scene), _level(level) {
	addEvent("mothDone");
	addEvent("mothStuck");
}

GreenhouseMachine::~GreenhouseMachine() {
	clearSprites();
}

void GreenhouseMachine::remove(Shell::Sprite *sprite) {
	for (uint i = 0; i < _owned.size(); i++) {
		if (_owned[i] == sprite) {
			_owned.remove_at(i);
			_scene.remove(sprite);
			return;
		}
	}
}

void GreenhouseMachine::clearSprites() {
	while (!_owned.empty())
		remove(_owned.back());
	_beetles.clear();
	for (uint i = 0; i < 144; i++)
		_flowers[i] = _leaves[i] = nullptr;
	for (uint i = 0; i < 12; i++) {
		_pots[i] = nullptr;
		_moths[i] = Moving();
	}
	_wand = _highlight = nullptr;
	_dragged = _selected = _swapSecond = -1;
	_wandHeld = false;
	_sounds.clear();
}

bool GreenhouseMachine::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property == "seed" && arguments.empty()) {
		result = Core::ScriptValue(static_cast<int32>(_seed));
		return true;
	}
	return Core::ScriptableObject::get(property, arguments, result);
}

bool GreenhouseMachine::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property == "wand" && arguments.empty()) {
		if (_wand) {
			_wand->visible = value.integer() != 0;
			if (_wand->visible)
				_wand->setFrame(MIN<uint>(21, 23 - (_rules.remainingSwaps() + 1) * 20 / 30));
		}
		return true;
	}
	return Core::ScriptableObject::set(property, arguments, value);
}

bool GreenhouseMachine::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (method == "start" && arguments.size() == 1) {
		_time = _lastTime = callTime();
		return start(arguments[0].integer());
	}
	if (method == "stop" && arguments.empty()) {
		_running = false;
		return true;
	}
	if ((method == "pause" || method == "resume") && arguments.empty()) {
		pause(method == "pause", callTime());
		return true;
	}
	return Core::ScriptableObject::play(method, arguments, result);
}

Shell::Sprite *GreenhouseMachine::add(uint id, int depth, int frame) {
	Shell::Sprite *sprite = _scene.add(Common::String::format(kAnimation, id), depth);
	if (!sprite) {
		_valid = false;
		return nullptr;
	}
	_owned.push_back(sprite);
	sprite->enabled = false;
	sprite->setFrame(frame);
	return sprite;
}

Common::Rect GreenhouseMachine::bounds(const Shell::Sprite *sprite) {
	if (!sprite || !sprite->visible)
		return Common::Rect();
	return Common::Rect(sprite->x, sprite->y, sprite->x + sprite->bounds.width(), sprite->y + sprite->bounds.height());
}

bool GreenhouseMachine::start(uint32 seed) {
	if (_level < 1 || 3 < _level)
		return false;
	clearSprites();
	_valid = true;
	_paused = false;
	_gameTime = _tick = 0;
	_nextBeetle = 10000;
	OM::XMLParser templates;
	if (!templates.loadFile(Common::Path(kTemplate)) || !templates.parse())
		return false;
	for (uint i = 0; i < 12; i++) {
		_moths[i].body = add(16100, 5);
		if (!_moths[i].body)
			return false;
		_moths[i].body->managed = true;
	}
	if (!seed) {
		TimeDate date;
		g_system->getTimeAndDate(date);
		seed = static_cast<uint32>(Common::DateTime::dateTimeToInt64(date));
		if (ConfMan.hasKey("random_seed"))
			seed = ConfMan.getInt("random_seed");
	}
	_seed = seed;
	_vm.rnd().setSeed(seed);
	Common::Point size(_moths[0].body->width(), _moths[0].body->height());
	if (!_rules.generate(_level, templates, _vm.rnd(), size))
		return false;
	for (uint i = 0; i < 144; i++)
		rebuildPlant(i);
	for (uint i = 0; i < 12; i++) {
		Moving &moth = _moths[i];
		moth.trait = add(traitResource(_rules.moth(i).animation), 6);
		if (!moth.trait)
			return false;
		moth.trait->managed = true;
		moth.body->x = _rules.moth(i).supply.x;
		moth.body->y = _rules.moth(i).supply.y;
		positionTrait(moth);
		_pots[i] = add(16258, 4);
		if (!_pots[i])
			return false;
		_pots[i]->x = 730;
		_pots[i]->y = 90 + 42 * i;
		_pots[i]->visible = true;
	}
	if (1 < _level) {
		_wand = add(16235, 20);
		if (!_wand)
			return false;
		_wand->setFrame(3);
		_wand->visible = false;
	}
	_running = _valid;
	return _valid;
}

void GreenhouseMachine::rebuildPlant(uint index) {
	if (_flowers[index])
		remove(_flowers[index]);
	if (_leaves[index])
		remove(_leaves[index]);
	uint value = _rules.cell(index);
	uint leaves = (value & 3) - 1, shape = ((value >> 2) & 7) - 1, color = ((value >> 5) & 7) - 1;
	Shell::Sprite *leaf = _leaves[index] = add(16220 + leaves, 1);
	Shell::Sprite *flower = _flowers[index] = add(kFlowers[color * 6 + shape], 2);
	if (!leaf || !flower)
		return;
	Common::Point offset = leaf->asset->origin() - flower->asset->origin();
	Common::Rect combined(0, 0, flower->width(), flower->height());
	combined.extend(Common::Rect(offset.x, offset.y, offset.x + leaf->width(), offset.y + leaf->height()));
	Common::Point origin = GreenhouseRules::cellOrigin(index % 12, index / 12);
	flower->x = origin.x + (44 - combined.width()) / 2 - combined.left;
	flower->y = origin.y + (42 - combined.height()) / 2 - combined.top;
	leaf->x = flower->x + offset.x;
	leaf->y = flower->y + offset.y;
}

void GreenhouseMachine::positionTrait(Moving &motion) {
	if (!motion.trait)
		return;
	static constexpr uint frames[] = {
		1,
		4,
		2,
		1,
		3,
	};
	motion.trait->setFrame(frames[motion.direction]);
	motion.trait->x = motion.body->x + (motion.body->width() - motion.trait->width()) / 2;
	motion.trait->y = motion.body->y + (motion.body->height() - motion.trait->height()) / 2;
	motion.trait->visible = motion.body->visible;
}

void GreenhouseMachine::beginMotion(Moving &motion, Common::Point target, uint duration, uint direction) {
	motion.from = Common::Point(motion.body->x, motion.body->y);
	motion.to = target;
	motion.duration = MAX<uint>(duration, 1);
	motion.started = _gameTime;
	motion.direction = direction;
	motion.active = true;
}

void GreenhouseMachine::updateMotion(Moving &motion, bool beetle) {
	uint elapsed = _gameTime - motion.started;
	uint frame = MIN<uint>(elapsed * 12 / 1000, motion.duration * 12 / 1000);
	static constexpr uint offsets[] = {
		0,
		31,
		11,
		1,
		21,
	};
	uint first = beetle ? 1 : offsets[motion.direction];
	motion.body->setFrame(first + frame % 10);
	float progress = MIN<float>(1, static_cast<float>(elapsed) / motion.duration);
	motion.body->x = motion.from.x + static_cast<int>((motion.to.x - motion.from.x) * progress);
	motion.body->y = motion.from.y + static_cast<int>((motion.to.y - motion.from.y) * progress);
	positionTrait(motion);
	if (motion.duration <= elapsed) {
		motion.active = false;
		motion.body->setFrame(first);
		motion.body->x = motion.to.x;
		motion.body->y = motion.to.y;
		positionTrait(motion);
	}
}

void GreenhouseMachine::finishMoth(uint index) {
	Moving &moth = _moths[index];
	moth.done = true;
	_pots[moth.destination]->visible = true;
	_pots[moth.destination]->setFrame(2);
	emitEvent("mothDone");
}

void GreenhouseMachine::updateMoths() {
	for (uint i = 0; i < 12; i++) {
		Moving &motion = _moths[i];
		if (motion.done) {
			if (motion.pollinating) {
				uint elapsed = _gameTime - motion.started;
				motion.body->setFrame(1 + MIN<uint>(9, elapsed * 12 / 1000));
				positionTrait(motion);
				if (10000 / 12 <= elapsed) {
					motion.pollinating = false;
					Common::Point target(799 + motion.body->width(), motion.body->y);
					beginMotion(motion, target, (target.x - motion.body->x) * 25, 3);
				}
			} else if (motion.active) {
				updateMotion(motion, false);
				motion.trait->visible = false;
				if (!motion.active)
					motion.body->visible = false;
			}
			continue;
		}
		if (motion.active) {
			updateMotion(motion, false);
			if (!motion.active && motion.flying) {
				motion.flying = false;
				motion.pollinating = true;
				motion.started = _gameTime;
				playSound(kResSound16240_SwapComplete);
				finishMoth(i);
			}
			continue;
		}
		if (_rules.moth(i).state != GreenhouseRules::kCrossing01)
			continue;
		if (!_rules.advance(i, _tick))
			continue;
		const GreenhouseRules::Moth &moth = _rules.moth(i);
		if (moth.state == GreenhouseRules::kPassed02) {
			motion.destination = _rules.passed() - 1;
			Common::Point target(730, 90 + 42 * motion.destination);
			int dx = target.x - motion.body->x, dy = target.y - motion.body->y;
			uint frames = static_cast<uint>(sqrt(static_cast<double>(dx * dx + dy * dy)) * 0.3);
			uint direction = 3;
			if (dx * 2 <= dy)
				direction = 2;
			if (dy <= -dx * 2)
				direction = 1;
			motion.flying = true;
			beginMotion(motion, target, frames * 1000 / 12, direction);
		} else {
			beginMotion(motion, GreenhouseRules::cellOrigin(moth.column, moth.row), 10000 / 12, moth.direction);
		}
	}
}

void GreenhouseMachine::updateBeetles() {
	if (_level != 3)
		return;
	if (_nextBeetle < _gameTime) {
		uint order[] = {
			0,
			1,
			2,
		};
		for (uint i = 0; i < 30; i++) {
			uint a = _vm.rnd().draw() % 3, b = _vm.rnd().draw() % 3;
			SWAP(order[a], order[b]);
		}
		for (uint i = 0; i < 3; i++) {
			int column = _rules.beetleEntry(order[i]);
			if (_rules.mothOccupied(column))
				continue;
			Beetle beetle;
			beetle.column = column;
			beetle.row = 0;
			beetle.direction = 2;
			beetle.trait = _rules.beetleOccupied(column) ? 0 : _rules.cell(column) & 3;
			beetle.deadline = _gameTime + 500;
			beetle.exiting = false;
			beetle.motion.body = add(16104, 5);
			beetle.motion.direction = 2;
			if (!beetle.motion.body)
				return;
			beetle.motion.body->managed = true;
			beetle.motion.body->x = 165 + 44 * column;
			beetle.motion.body->y = 90;
			if (beetle.trait) {
				_rules.setBeetleOccupied(column, true);
				beetle.motion.trait = add(traitResource(beetle.trait), 6);
				positionTrait(beetle.motion);
			}
			_beetles.push_back(beetle);
			break;
		}
		_nextBeetle = _gameTime + 8000;
	}
	for (uint i = 0; i < _beetles.size();) {
		Beetle &beetle = _beetles[i];
		Moving &motion = beetle.motion;
		if (motion.active) {
			updateMotion(motion, true);
			if (!motion.active && beetle.exiting) {
				remove(motion.body);
				if (motion.trait)
					remove(motion.trait);
				_beetles.remove_at(i);
				continue;
			}
		} else if (beetle.deadline < _gameTime) {
			beetle.deadline = _gameTime + 500;
			uint cell = beetle.row * 12 + beetle.column;
			int x = beetle.column, y = beetle.row;
			uint direction = beetle.direction;
			uint flow = beetle.trait ? _rules.beetleFlow(cell) : 1;
			if (flow == 6)
				flow = _vm.rnd().draw() % 100 < 50 ? 2 : 4;
			if (flow == 1) {
				y += 1;
				direction = 2;
			} else if (flow == 2) {
				x -= 1;
				direction = 4;
			} else if (flow == 4) {
				x += 1;
				direction = 3;
			} else if (flow == 8) {
				y -= 1;
				direction = 1;
			} else {
				i += 1;
				continue;
			}
			if (direction != beetle.direction) {
				beetle.direction = direction;
				i += 1;
				continue;
			}
			if (x < 0 || 12 <= x || y < 0) {
				i += 1;
				continue;
			}
			if (y == 12) {
				if (beetle.trait)
					_rules.setBeetleOccupied(cell, false);
				beetle.exiting = true;
				if (motion.trait)
					motion.trait->visible = false;
				beginMotion(motion, GreenhouseRules::cellOrigin(x, y), 10000 / 12, 2);
			} else {
				uint next = y * 12 + x;
				if (!_rules.mothOccupied(next) && !(_rules.beetleOccupied(next) && beetle.trait)) {
					if (beetle.trait)
						_rules.setBeetleOccupied(cell, false);
					else if (!_rules.beetleOccupied(next)) {
						beetle.trait = _rules.cell(next) & 3;
						motion.trait = add(traitResource(beetle.trait), 6);
					}
					if (beetle.trait)
						_rules.setBeetleOccupied(next, true);
					beetle.column = x;
					beetle.row = y;
					motion.body->visible = !_rules.beetleOccupied(next) || beetle.trait;
					beginMotion(motion, GreenhouseRules::cellOrigin(x, y), 10000 / 12, direction);
				}
			}
		}
		i += 1;
	}
}

int GreenhouseMachine::hitCell(Common::Point point) {
	if (point.x < 165 || 693 <= point.x || point.y < 90 || 594 <= point.y)
		return -1;
	return (point.y - 90) / 42 * 12 + (point.x - 165) / 44;
}

int GreenhouseMachine::hitMoth(Common::Point point) const {
	for (int i = 11; 0 <= i; i--)
		if (_rules.moth(i).state == GreenhouseRules::kSupply00 && bounds(_moths[i].body).contains(point))
			return i;
	return -1;
}

bool GreenhouseMachine::overDraggable(Common::Point point) const {
	return 0 <= hitMoth(point) || (_wand && bounds(_wand).contains(point));
}

void GreenhouseMachine::releaseMoth() {
	if (_dragged < 0)
		return;
	Moving &motion = _moths[_dragged];
	Common::Rect rect = bounds(motion.body);
	int bestRow = -1, bestArea = 0;
	for (uint row = 0; row < 12; row++) {
		Common::Rect cell(165, 90 + 42 * row, 209, 132 + 42 * row);
		if (!rect.intersects(cell))
			continue;
		cell.clip(rect);
		if (bestArea < cell.width() * cell.height()) {
			bestArea = cell.width() * cell.height();
			bestRow = row;
		}
	}
	if (0 <= bestRow && _rules.enter(_dragged, bestRow)) {
		Common::Point point = GreenhouseRules::cellOrigin(0, bestRow);
		motion.body->x = point.x + (44 - motion.body->width()) / 2;
		motion.body->y = point.y + (44 - motion.body->height()) / 2;
		debug(1, "Greenhouse moth %u entered row %d", _dragged + 1, bestRow + 1);
	} else if (0 <= bestRow || !rect.intersects(Common::Rect(0, 150, 155, 500))) {
		motion.body->x = _rules.moth(_dragged).supply.x;
		motion.body->y = _rules.moth(_dragged).supply.y;
	}
	_scene.setDepth(motion.body, 5);
	_scene.setDepth(motion.trait, 6);
	positionTrait(motion);
	_dragged = -1;
	playSound(kResSound16052_WandPickup);
}

void GreenhouseMachine::cancelWand() {
	_selected = _swapSecond = -1;
	_wandHeld = false;
	if (_highlight) {
		remove(_highlight);
		_highlight = nullptr;
	}
	if (_wand) {
		_wand->x = _wand->asset->origin().x;
		_wand->y = _wand->asset->origin().y;
		_wand->visible = _rules.remainingSwaps() != 0;
	}
}

void GreenhouseMachine::selectPlant(uint cell) {
	if (_rules.occupied(cell) || 0 <= _swapSecond)
		return;
	if (_selected == static_cast<int>(cell)) {
		_selected = -1;
		if (_highlight) {
			remove(_highlight);
			_highlight = nullptr;
		}
		return;
	}
	if (_selected < 0) {
		_selected = cell;
		uint value = _rules.cell(cell);
		static constexpr uint highlights[] = {
			16223,
			16226,
			16229,
			16232,
			16236,
			16239,
			16224,
			16227,
			16230,
			16233,
			16237,
			16240,
			16225,
			16228,
			16231,
			16234,
			16238,
			16241,
		};
		uint id = highlights[6 * ((value & 3) - 1) + (((value >> 2) & 7) - 1)];
		_highlight = add(id, 3);
		if (_highlight) {
			Common::Point point = GreenhouseRules::cellOrigin(cell % 12, cell / 12);
			_highlight->x = point.x + (44 - _highlight->width()) / 2;
			_highlight->y = point.y + (42 - _highlight->height()) / 2;
		}
		playSound(kResSound16236_MothStuck);
	} else {
		_swapSecond = cell;
		_swapStarted = _time;
		playSound(kResSound16238_SwapAccepted);
	}
}

void GreenhouseMachine::event(const Common::Event &event) {
	if (_paused || !_running)
		return;
	if (event.type == Common::EVENT_MOUSEMOVE) {
		_mouse = event.mouse;
		if (0 <= _dragged) {
			Moving &motion = _moths[_dragged];
			motion.body->x = _mouse.x - _dragOffset.x;
			motion.body->y = _mouse.y - _dragOffset.y;
			positionTrait(motion);
		} else if (_wandHeld) {
			_wand->x = _mouse.x - _wand->width() / 2;
			_wand->y = _mouse.y - _wand->height() / 2;
		}
	} else if (event.type == Common::EVENT_LBUTTONDOWN) {
		_mouse = event.mouse;
		if (_wandHeld) {
			int cell = hitCell(_mouse);
			if (0 <= cell)
				selectPlant(cell);
			else {
				playSound(kResSound16237_SwapRejected);
				cancelWand();
			}
		} else if (_wand && bounds(_wand).contains(_mouse)) {
			_wandHeld = true;
			playSound(kResSound16235_WandCancel);
		} else {
			_dragged = hitMoth(_mouse);
			if (0 <= _dragged) {
				Moving &motion = _moths[_dragged];
				_dragOffset = _mouse - Common::Point(motion.body->x, motion.body->y);
				_scene.setDepth(motion.body, 1000);
				_scene.setDepth(motion.trait, 1001);
				playSound(kResSound16050_WandUse);
			}
		}
	} else if (event.type == Common::EVENT_LBUTTONUP) {
		releaseMoth();
	} else if (event.type == Common::EVENT_RBUTTONDOWN && _wandHeld) {
		cancelWand();
	}
	return;
}

void GreenhouseMachine::update(uint32 time) {
	_time = time;
	if (!_running)
		return;
	if (!_paused && _selected < 0)
		_gameTime += time - _lastTime;
	_lastTime = time;
	if (!_paused) {
		if (0 <= _swapSecond && 500 <= time - _swapStarted) {
			if (_rules.swapPlants(_selected, _swapSecond)) {
				rebuildPlant(_selected);
				rebuildPlant(_swapSecond);
				playSound(kResSound16239_MothFree);
				debug(1, "Greenhouse swapped cells %d %d; remaining %u", _selected + 1, _swapSecond + 1, _rules.remainingSwaps());
			}
			_selected = _swapSecond = -1;
			if (_highlight) {
				remove(_highlight);
				_highlight = nullptr;
			}
			_wand->setFrame(MIN<uint>(21, 23 - (_rules.remainingSwaps() + 1) * 20 / 30));
			if (!_rules.remainingSwaps()) {
				cancelWand();
				playSound(kResSound16241_NoSwaps);
			}
		}
		if (_selected < 0) {
			_tick += 1;
			updateBeetles();
			// The task tests the previous stuck flags before advancing the moths.
			bool allStuck = _rules.allMothsStuck();
			updateMoths();
			if (allStuck)
				emitEvent("mothStuck");
		}
	}
}

void GreenhouseMachine::pause(bool paused, uint32 time) {
	if (_paused == paused)
		return;
	if (paused) {
		if (0 <= _dragged) {
			Moving &motion = _moths[_dragged];
			motion.body->x = _rules.moth(_dragged).supply.x;
			motion.body->y = _rules.moth(_dragged).supply.y;
			positionTrait(motion);
			_dragged = -1;
		}
		cancelWand();
	}
	_time = _lastTime = time;
	_paused = paused;
}

bool GreenhouseMachine::pollSound(uint16 &sound) {
	if (_sounds.empty())
		return false;
	sound = _sounds.front();
	_sounds.remove_at(0);
	return true;
}

Common::String GreenhouseMachine::debugAnswer() const {
	const GreenhouseRules &rules = this->rules();
	Common::String result = Common::String::format("Greenhouse level %u. Coordinates use the 800x600 game surface.\n", _level);
	GreenhouseRules solution = rules;
	Common::Array<GreenhouseRules::Swap> swaps;
	if (rules.findRestoration(swaps) && !swaps.empty()) {
		result += "Click wand (708,537); apply swaps, then click outside the field to put the wand away.\n";
		for (uint i = 0; i < swaps.size(); i++) {
			Common::Point a = GreenhouseRules::cellOrigin(swaps[i].first % 12, swaps[i].first / 12);
			Common::Point b = GreenhouseRules::cellOrigin(swaps[i].second % 12, swaps[i].second / 12);
			result += Common::String::format("Swap (%d,%d) with (%d,%d)\n", a.x + 22, a.y + 21, b.x + 22, b.y + 21);
			solution.swapPlants(swaps[i].first, swaps[i].second);
		}
	}
	for (uint i = 0; i < 12; i++) {
		if (rules.moth(i).state == GreenhouseRules::kPassed02)
			continue;
		Common::Array<uint> path;
		if (!solution.findPath(i, path)) {
			result += Common::String::format("Moth %u: path blocked; wait for moving occupants or repair its trait path.\n", i + 1);
			continue;
		}
		if (rules.moth(i).state == GreenhouseRules::kSupply00) {
			Common::Rect from = mothBounds(i);
			result += Common::String::format("Moth %u: drag (%d,%d) to (187,%u)\n", i + 1,
											 from.left + from.width() / 2, from.top + from.height() / 2, 111 + 42 * (path[0] / 12));
		}
		result += Common::String::format("Path %u:", i + 1);
		for (uint j = 0; j < path.size(); j++)
			result += Common::String::format(" %u,%u", path[j] % 12 + 1, path[j] / 12 + 1);
		result += "\n";
	}
	return result;
}

} // namespace Zoombini3
} // namespace Trinket
