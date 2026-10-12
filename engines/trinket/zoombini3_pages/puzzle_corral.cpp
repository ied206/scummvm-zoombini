/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/zoombini3_pages/puzzle_corral.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "trinket/shell/character_action.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace Zoombini3 {

CorralRules::CorralRules() = default;

Common::Point CorralRules::slotPosition(uint slot) {
	static constexpr int x[] = {
		68,
		137,
		203,
		265,
		100,
		170,
		28,
		91,
		144,
		41,
		100,
		78,
		210,
		206,
		270,
		257,
		325,
		512,
		622,
		557,
		644,
		635,
		295,
		537,
		380,
		500,
		428,
		421,
		355,
		435,
		284,
		330,
		304,
		493,
		546,
		555,
		420,
	};
	static constexpr int y[] = {
		122,
		110,
		130,
		105,
		180,
		190,
		239,
		248,
		230,
		315,
		318,
		368,
		306,
		370,
		390,
		462,
		480,
		474,
		439,
		383,
		400,
		323,
		175,
		180,
		148,
		160,
		200,
		438,
		425,
		416,
		266,
		270,
		297,
		260,
		305,
		270,
		322,
	};
	return slot < ARRAYSIZE(x) ? Common::Point(x[slot], y[slot]) : Common::Point();
}

bool CorralRules::generate(uint level, uint party, Shell::Random &random, Generator *generator) {
	if (level < 1 || 3 < level || party < 1 || 12 < party)
		return false;
	_level = level;
	_party = party;
	_turns = level == 1 ? 11 : 8;
	memset(_projectors, -1, sizeof(_projectors));
	bool products[64] = {};
	uint positions[12];
	for (uint i = 0; i < 12; i++)
		positions[i] = i;
	uint repeated[3] = {};
	int previous[3] = {
		-1,
		-1,
		-1,
	};
	static constexpr int fruitX[] = {
		180,
		220,
		260,
		300,
		340,
		380,
		420,
		460,
		500,
		540,
		580,
		625,
	};
	for (uint i = 0; i < party; i++) {
		Fruit &fruit = _fruits[i];
		Zerble &zerble = _zerbles[i];
		uint product;
		do {
			product = 1;
			for (uint j = 0; j < 3; j++) {
				fruit.traits[j] = random.draw() % 4;
				product *= fruit.traits[j] + 1;
			}
		} while (products[product - 1]);
		// Distinct products, not merely distinct triples, are reserved by the original.
		products[product - 1] = true;
		fruit.consumed = false;
		fruit.projector = -1;
		zerble.feet = random.draw() % 3;
		zerble.tail = random.draw() % 3;
		uint choice = random.draw() % (12 - i);
		zerble.slot = positions[choice];
		for (uint j = choice; j + 1 < 12 - i; j++)
			positions[j] = positions[j + 1];
		for (uint j = 0; j < 3; j++) {
			if (previous[j] == static_cast<int>(fruit.traits[j]))
				repeated[j] += 1;
			if ((j == 0 && 9 <= repeated[j]) || (j != 0 && repeated[j] == 9))
				fruit.traits[j] = (fruit.traits[j] + 1) % 4;
			previous[j] = fruit.traits[j];
		}
		fruit.supply = Common::Point(fruitX[i], 535 + 2 * (random.draw() % 15));
		zerble.position = slotPosition(zerble.slot);
		zerble.position.x += 2 * (random.draw() % 15) - 15;
		zerble.position.y += 2 * (random.draw() % 15) - 15;
		zerble.fed = false;
		if (generator) {
			if (!generator->corralGenerated(i))
				return false;
		} else {
			// Character construction schedules its blink and idle timers here.
			random.draw();
			random.draw();
		}
	}
	_axes[0] = random.draw() % 3;
	uint remaining[2], count = 0;
	for (uint i = 0; i < 3; i++)
		if (i != _axes[0]) {
			remaining[count] = i;
			count += 1;
		}
	_axes[1] = remaining[random.draw() % 2];
	for (uint i = 0; i < 3; i++)
		if (i != _axes[0] && i != _axes[1])
			_axes[2] = i;
	random.draw();
	return true;
}

bool CorralRules::remove(uint fruit) {
	if (_party <= fruit || _fruits[fruit].projector < 0)
		return false;
	_projectors[_fruits[fruit].projector] = -1;
	_fruits[fruit].projector = -1;
	// Removing a beam changes its display without relocating the herd.
	return true;
}

bool CorralRules::project(uint fruit, uint projector, Shell::Random &random) {
	if (_party <= fruit || projectorCount() <= projector || _turns < 1 || _fruits[fruit].consumed || 0 <= _projectors[projector])
		return false;
	CorralRules previous = *this;
	remove(fruit);
	_projectors[projector] = fruit;
	_fruits[fruit].projector = projector;
	_turns -= 1;
	if (!relocate(random)) {
		// An overfull light region must not stall the engine in a rejection loop.
		*this = previous;
		return false;
	}
	return true;
}

bool CorralRules::matches(uint fruit, uint zerble) const {
	if (_party <= fruit || _party <= zerble || _fruits[fruit].consumed || _zerbles[zerble].fed)
		return false;
	for (uint i = 0; i < projectorCount(); i++)
		if (_fruits[fruit].traits[_axes[i]] != _fruits[zerble].traits[_axes[i]])
			return false;
	return true;
}

bool CorralRules::feed(uint fruit, uint zerble) {
	if (_party <= fruit || _party <= zerble || _fruits[fruit].consumed || _zerbles[zerble].fed)
		return false;
	bool correct = matches(fruit, zerble);
	remove(fruit);
	_fruits[fruit].consumed = true;
	if (correct)
		_zerbles[zerble].fed = true;
	return correct;
}

uint CorralRules::region(uint zerble) const {
	uint result = 0;
	for (uint i = 0; i < projectorCount(); i++)
		if (0 <= _projectors[i] && _fruits[_projectors[i]].traits[_axes[i]] == _fruits[zerble].traits[_axes[i]])
			result |= 1 << i;
	return result;
}

void CorralRules::allowedSlots(uint zerble, Common::Array<uint> &slots) const {
	slots.clear();
	uint match = region(zerble);
	if (!match) {
		for (uint i = 0; i < 12; i++)
			slots.push_back(i);
		return;
	}
	static constexpr uint masks[] = {
		1,
		2,
		4,
		3,
		5,
		6,
		7,
	};
	static constexpr uint first[] = {
		12,
		17,
		22,
		27,
		30,
		33,
		36,
	},
						  counts[] = {
							  5,
							  5,
							  5,
							  3,
							  3,
							  3,
							  1,
						  };
	uint active = 0;
	for (uint i = 0; i < 3; i++)
		if (0 <= _projectors[i])
			active |= 1 << i;
	for (uint group = 0; group < ARRAYSIZE(masks); group++) {
		if ((masks[group] & active) != match)
			continue;
		for (uint i = 0; i < counts[group]; i++)
			slots.push_back(first[group] + i);
	}
}

bool CorralRules::relocate(Shell::Random &random) {
	bool occupied[37] = {};
	Common::Array<uint> slots[12];
	for (uint i = 0; i < _party; i++) {
		allowedSlots(i, slots[i]);
		bool retained = false;
		for (uint j = 0; j < slots[i].size(); j++)
			if (static_cast<int>(slots[i][j]) == _zerbles[i].slot)
				retained = true;
		if (retained)
			occupied[_zerbles[i].slot] = true;
		else
			_zerbles[i].slot = -1;
	}
	for (uint i = 0; i < _party; i++) {
		if (0 <= _zerbles[i].slot)
			continue;
		bool available = false;
		for (uint j = 0; j < slots[i].size(); j++)
			if (!occupied[slots[i][j]])
				available = true;
		if (!available)
			return false;
		uint slot;
		do {
			slot = slots[i][random.draw() % slots[i].size()];
		} while (occupied[slot]);
		occupied[slot] = true;
		_zerbles[i].slot = slot;
		if (!_zerbles[i].fed)
			_zerbles[i].position = slotPosition(slot);
	}
	return true;
}

uint CorralRules::fruitRemaining() const {
	uint count = 0;
	for (uint i = 0; i < _party; i++)
		if (!_fruits[i].consumed)
			count += 1;
	return count;
}

uint CorralRules::fedCount() const {
	uint count = 0;
	for (uint i = 0; i < _party; i++)
		if (_zerbles[i].fed)
			count += 1;
	return count;
}

void CorralRules::findAnswer(Common::Array<Feeding> &plan) const {
	plan.clear();
	bool assigned[12] = {};
	for (uint i = 0; i < _party; i++) {
		if (_fruits[i].consumed)
			continue;
		for (uint j = 0; j < _party; j++) {
			if (assigned[j] || !matches(i, j))
				continue;
			Feeding item = {i, j};
			plan.push_back(item);
			assigned[j] = true;
			break;
		}
	}
}

constexpr const char *CorralMachine::kAnimation;
constexpr const char *CorralMachine::kBeamAnimation;
constexpr const char *CorralMachine::kCommonAnimation;

CorralMachine::CorralMachine(TrinketEngine &vm, Shell::Scene &scene) : _vm(vm), _scene(scene) {
	const char *events[] = {
		"correct",
		"outOfTurns",
		"finished",
		"tokenPickedUp",
	};
	for (uint i = 0; i < ARRAYSIZE(events); i++)
		addEvent(events[i]);
}

CorralMachine::~CorralMachine() {
	clearSprites();
}

void CorralMachine::clearSprites() {
	clearTrinkets();
	for (uint i = 0; i < ARRAYSIZE(_zerbles); i++) {
		delete _zerbles[i];
		_zerbles[i] = nullptr;
	}
	for (uint i = 0; i < _owned.size(); i++)
		_scene.remove(_owned[i]);
	_owned.clear();
	for (uint i = 0; i < 3; i++)
		_sniff[i] = _feedback[i] = nullptr;
}

void CorralMachine::removeSprite(Shell::Sprite *sprite) {
	if (!sprite)
		return;
	removeTrinket(*sprite);
	for (uint i = 0; i < _owned.size(); i++)
		if (_owned[i] == sprite) {
			_owned.remove_at(i);
			break;
		}
	_scene.remove(sprite);
}

Shell::Sprite *CorralMachine::add(uint id, int depth, bool common) {
	Shell::Sprite *sprite = _scene.add(Common::String::format(common ? kCommonAnimation : kAnimation, id), depth);
	if (!sprite) {
		_valid = false;
		warning("Corral could not load animation %u (common %d)", id, common);
		return nullptr;
	}
	sprite->setFrame(1);
	sprite->enabled = false;
	addTrinket(*sprite);
	_owned.push_back(sprite);
	return sprite;
}

bool CorralMachine::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (!arguments.empty())
		return MultiTrinket::get(property, arguments, result);
	if (property == "remainingTurns")
		result = Core::ScriptValue(_rules.turns());
	else if (property == "currentZerbleFeet")
		result = Core::ScriptValue(static_cast<int32>(_lastExited < 0 ? 0 : _rules.zerble(_lastExited).feet));
	else if (property == "currentZerbleTail")
		result = Core::ScriptValue(static_cast<int32>(_lastExited < 0 ? 0 : _rules.zerble(_lastExited).tail));
	else
		return MultiTrinket::get(property, arguments, result);
	return true;
}

bool CorralMachine::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property != "remainingTurns" || !arguments.empty())
		return MultiTrinket::set(property, arguments, value);
	_rules.setTurns(value.integer());
	return true;
}

bool CorralMachine::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	_time = callTime();
	if (method == "setupGame" && arguments.size() == 2)
		return setup(arguments[0].integer(), arguments[1].integer());
	if (!arguments.empty())
		return MultiTrinket::play(method, arguments, result);
	if (method == "startRound") {
		_demonstrating = true;
		_blinkDeadline = _time + 200;
		_blinkCount = 8;
		return true;
	}
	if (method == "finishRound") {
		pause(true, _time);
		emitEvent("finished");
		return true;
	}
	if (method == "pauseRound" || method == "resumeRound") {
		pause(method == "pauseRound", _time);
		return true;
	}
	return MultiTrinket::play(method, arguments, result);
}

bool CorralMachine::corralGenerated(uint index) {
	const CorralRules::Zerble &zerble = _rules.zerble(index);
	int32 traits[] = {
		0,
		static_cast<int32>(zerble.feet),
		static_cast<int32>(zerble.tail),
	};
	_zerbles[index] = new AttributeCharacter(_vm, _scene);
	if (!_zerbles[index]->create(1, traits, 0, zerble.position, 3, _time))
		return false;
	addTrinket(*_zerbles[index]);
	return true;
}

bool CorralMachine::setup(uint level, uint party) {
	clearSprites();
	_level = level;
	_valid = true;
	_selected = _feeding = _feedingFruit = _lastExited = _hover = _hoverTray = -1;
	_blinkCount = 0;
	_blinkDeadline = 0;
	_demonstrating = false;
	_fruitRemaining = _zerblesRemaining = party;
	memset(_moving, 0, sizeof(_moving));
	memset(_exiting, 0, sizeof(_exiting));
	if (!_rules.generate(_level, party, _vm.rnd(), this))
		return false;
	debug(2, "Corral herd created");
	add(20111, 1);
	add(20114, 500);
	_beams = _scene.add(kBeamAnimation, 1);
	if (!_beams) {
		_valid = false;
		return false;
	}
	addTrinket(*_beams);
	_owned.push_back(_beams);
	_beams->enabled = false;
	_beams->setFrame(1);
	static constexpr int offsetX[] = {
		-200,
		200,
		20,
	},
						 offsetY[] = {
							 20,
							 20,
							 -180,
						 };
	static constexpr int trayX[] = {
		27,
		698,
		600,
	},
						 trayY[] = {
							 528,
							 528,
							 0,
						 };
	for (uint i = 0; i < 3; i++) {
		_projectors[i] = add(20104 + i, 1);
		_trays[i] = add(20107 + i, 2);
		if (!_valid)
			return false;
		_trays[i]->x = trayX[i];
		_trays[i]->y = trayY[i];
		for (uint j = 0; j < 5; j++) {
			_clues[i][j] = add(20120 + j, j < 3 ? 3 : 2);
			if (!_valid)
				return false;
			_clues[i][j]->x += offsetX[i];
			_clues[i][j]->y += offsetY[i];
		}
	}
	for (uint i = 0; i < party; i++) {
		FruitView &view = _fruits[i];
		view.body = add(20101, 11);
		view.leaf = add(20102, 10);
		if (!_valid)
			return false;
		const CorralRules::Fruit &fruit = _rules.fruit(i);
		view.body->setFrame(1 + fruit.traits[1] + 4 * fruit.traits[0]);
		view.leaf->setFrame(1 + fruit.traits[2]);
		Common::Rect body(view.body->x, view.body->y, view.body->x + view.body->width(), view.body->y + view.body->height());
		Common::Rect leaf(view.leaf->x, view.leaf->y, view.leaf->x + view.leaf->width(), view.leaf->y + view.leaf->height());
		body.extend(leaf);
		view.bodyOffset = Common::Point(view.body->x - body.left, view.body->y - body.top);
		view.leafOffset = Common::Point(view.leaf->x - body.left, view.leaf->y - body.top);
		view.bounds = Common::Rect(body.width(), body.height());
		view.home = fruit.supply;
		positionFruit(i, view.home);
	}
	updateProjectors();
	debug(1, "Corral ready: level %u, party %u, charges %d", _level, party, _rules.turns());
	return _valid;
}

void CorralMachine::positionFruit(uint index, Common::Point point) {
	FruitView &view = _fruits[index];
	view.position = point;
	view.body->x = point.x + view.bodyOffset.x;
	view.body->y = point.y + view.bodyOffset.y;
	view.leaf->x = point.x + view.leafOffset.x;
	view.leaf->y = point.y + view.leafOffset.y;
}

Common::Rect CorralMachine::fruitBounds(uint index) const {
	Common::Rect bounds = _fruits[index].bounds;
	bounds.translate(_fruits[index].position.x, _fruits[index].position.y);
	return bounds;
}

int CorralMachine::hitFruit(Common::Point point) const {
	for (int i = _rules.party() - 1; 0 <= i; i--)
		if (!_rules.fruit(i).consumed && fruitBounds(i).contains(point))
			return i;
	return -1;
}

void CorralMachine::updateProjectors() {
	uint mask = 0;
	for (uint i = 0; i < 3; i++) {
		bool active = i < _rules.projectorCount();
		_projectors[i]->visible = _trays[i]->visible = active;
		int fruit = _rules.projectedFruit(i);
		_projectors[i]->setFrame(0 <= fruit ? 2 : 1);
		for (uint j = 0; j < 5; j++)
			_clues[i][j]->visible = false;
		if (!active || fruit < 0)
			continue;
		mask |= 1 << i;
		if (_level == 3)
			continue;
		const CorralRules::Fruit &value = _rules.fruit(fruit);
		uint body = _rules.axis(i) == 0 ? 1 : _rules.axis(i) == 1 ? 0
																  : 2;
		uint leaf = _rules.axis(i) == 2 ? 3 : 4;
		_clues[i][body]->setFrame(1 + value.traits[1] + 4 * value.traits[0]);
		_clues[i][leaf]->setFrame(1 + value.traits[2]);
		_clues[i][body]->visible = _clues[i][leaf]->visible = true;
	}
	_beams->visible = mask != 0;
	if (mask)
		_beams->setFrame(mask);
}

void CorralMachine::moveZerbles() {
	for (uint i = 0; i < _rules.party(); i++) {
		if (_rules.zerble(i).fed || _zerbles[i]->position() == _rules.zerble(i).position)
			continue;
		_zerbles[i]->stopWalk(false);
		_zerbles[i]->addWalkMove(_rules.zerble(i).position);
		_zerbles[i]->walk(false, _time);
		_moving[i] = true;
	}
}

void CorralMachine::updateBlink() {
	if (!_blinkCount)
		return;
	if (!_blinkDeadline)
		_blinkDeadline = _time + 200;
	if (static_cast<int32>(_time - _blinkDeadline) < 0)
		return;
	_blinkCount -= 1;
	_blinkDeadline = _time + (_demonstrating ? 800 : 200);
	updateProjectors();
	if (_demonstrating) {
		bool on = _blinkCount && !(_blinkCount % 2);
		for (uint i = 0; i < _rules.projectorCount(); i++) {
			_trays[i]->setFrame(on ? 2 : 1);
			if (on)
				_projectors[i]->setFrame(2);
		}
		if (on) {
			_beams->setFrame(_level == 1 ? 3 : 7);
			_beams->visible = true;
			playEffect(kResSound20114_FruitPickup);
		}
	} else if (_blinkCount % 2) {
		_beams->visible = false;
		for (uint i = 0; i < 3; i++) {
			_projectors[i]->setFrame(1);
			for (uint j = 0; j < 5; j++)
				_clues[i][j]->visible = false;
		}
	}
}

void CorralMachine::drop() {
	uint index = _selected;
	stopHover();
	_selected = -1;
	playEffect(kResSound20116_FruitDrop);
	Common::Rect bounds = fruitBounds(index);
	Common::Point center(bounds.left + bounds.width() / 2, bounds.top + bounds.height() / 2);
	for (uint i = 0; i < _rules.projectorCount(); i++) {
		if (!_trays[i]->bounds.contains(center))
			continue;
		if (_rules.project(index, i, _vm.rnd())) {
			positionFruit(index, _fruits[index].position - Common::Point(0, 15));
			updateProjectors();
			playEffect(3 <= _rules.turns() ? kResSound20112_RejectEarly : kResSound20113_RejectLate);
			if (_rules.turns() < 3) {
				_demonstrating = false;
				_blinkCount = 7;
				_blinkDeadline = _time + 200;
			}
			if (!_rules.turns())
				emitEvent("outOfTurns");
			moveZerbles();
			debug(1, "Corral projection %u: fruit %u, charges %u", i + 1, index + 1, _rules.turns());
		} else
			positionFruit(index, _fruits[index].home);
		return;
	}
	if (100 < bounds.left && bounds.left < 700 && 500 < bounds.top) {
		_fruits[index].home = _fruits[index].position;
		return;
	}
	for (uint i = 0; i < _rules.party(); i++) {
		if (!_rules.zerble(i).fed && !_moving[i] && bounds.intersects(_zerbles[i]->bounds())) {
			beginFeeding(index, i);
			return;
		}
	}
	positionFruit(index, _fruits[index].home);
}

void CorralMachine::stopHover() {
	for (uint i = 0; i < 3; i++) {
		if (_sniff[i])
			removeSprite(_sniff[i]);
		_sniff[i] = nullptr;
		_trays[i]->setFrame(1);
	}
	if (0 <= _hover)
		_zerbles[_hover]->setVisible(true, _time);
	_hover = _hoverTray = -1;
}

void CorralMachine::updateHover() {
	if (_selected < 0)
		return;
	Common::Rect bounds = fruitBounds(_selected);
	if (0 <= _hover && !bounds.intersects(_hoverBounds))
		stopHover();
	if (_hover < 0) {
		for (uint i = 0; i < _rules.party(); i++) {
			if (_rules.zerble(i).fed || _moving[i] || !bounds.intersects(_zerbles[i]->bounds()))
				continue;
			_hover = i;
			_hoverBounds = _zerbles[i]->bounds();
			_zerbles[i]->setVisible(false, _time);
			const CorralRules::Zerble &zerble = _rules.zerble(i);
			uint traits[] = {
				0,
				zerble.feet,
				zerble.tail,
			};
			static constexpr uint layers[] = {
				1,
				3,
				2,
			};
			Common::Point point = _zerbles[i]->position();
			for (uint j = 0; j < 3; j++) {
				_sniff[j] = add(3160 + j * 10 + traits[j], 3 + layers[j], true);
				if (!_sniff[j])
					continue;
				_sniff[j]->managed = true;
				_sniff[j]->x += point.x - 395;
				_sniff[j]->y += point.y - 302;
				_sniff[j]->startAnimation(-1);
				_sniff[j]->pauseAnimation(j != 0);
			}
			break;
		}
	}
	int tray = -1;
	if (0 < _rules.turns())
		for (uint i = 0; i < _rules.projectorCount(); i++)
			if (bounds.intersects(_trays[i]->bounds)) {
				tray = i;
				break;
			}
	if (tray != _hoverTray) {
		if (0 <= _hoverTray)
			_trays[_hoverTray]->setFrame(1);
		if (0 <= tray) {
			_trays[tray]->setFrame(2);
			playEffect(kResSound20117_TrayHover);
		}
		_hoverTray = tray;
	}
}

void CorralMachine::beginFeeding(uint fruit, uint zerble) {
	_feeding = zerble;
	_feedingFruit = fruit;
	_correct = _rules.matches(fruit, zerble);
	_zerbles[zerble]->setVisible(false, _time);
	const CorralRules::Zerble &value = _rules.zerble(zerble);
	uint traits[] = {
		0,
		value.feet,
		value.tail,
	};
	static constexpr uint layers[] = {
		1,
		3,
		2,
	};
	Common::Point point = _zerbles[zerble]->position();
	for (uint i = 0; i < 3; i++) {
		_feedback[i] = add((_correct ? 3100 : 3130) + i * 10 + traits[i], 3 + layers[i], true);
		if (!_feedback[i]) {
			warning("Corral feedback resource missing");
			return;
		}
		_feedback[i]->managed = true;
		_feedback[i]->x += point.x - 395;
		_feedback[i]->y += point.y - 302;
		_feedback[i]->startAnimation(1);
		_feedback[i]->pauseAnimation(i != 0);
	}
}

void CorralMachine::finishFeeding() {
	uint index = _feeding, fruit = _feedingFruit;
	for (uint i = 0; i < 3; i++) {
		removeSprite(_feedback[i]);
		_feedback[i] = nullptr;
	}
	_zerbles[index]->setVisible(true, _time);
	_rules.feed(fruit, index);
	_fruits[fruit].body->visible = _fruits[fruit].leaf->visible = false;
	_feeding = _feedingFruit = -1;
	if (_correct) {
		_zerbles[index]->addWalkMove(Common::Point(700, 270));
		_zerbles[index]->addWalkMove(Common::Point(850, 270));
		_zerbles[index]->walk(true, _time);
		_moving[index] = _exiting[index] = true;
		playEffect(kResSound2726_CommonGo);
		emitEvent("correct");
	} else {
		playEffect(kResSound20400_Projection);
		_fruitRemaining -= 1;
		if (!_fruitRemaining)
			emitEvent("finished");
	}
	debug(1, "Corral fruit %u %s by zerble %u: fed %u, fruit remaining %u", fruit + 1, _correct ? "accepted" : "rejected", index + 1,
		  _rules.fedCount(), _rules.fruitRemaining());
}

void CorralMachine::updateFeedback(Common::Array<uint16> &sounds) {
	if (_feeding < 0 || !_feedback[0])
		return;
	_feedback[0]->update(_time, sounds);
	for (uint i = 1; i < 3; i++)
		if (_feedback[i])
			_feedback[i]->setFrame(_feedback[0]->displayedFrame());
	if (!_feedback[0]->isRunning())
		finishFeeding();
}

void CorralMachine::event(const Common::Event &event, uint32 time) {
	_time = time;
	if (_paused)
		return;
	if (0 <= _feeding)
		return;
	if (event.type == Common::EVENT_MOUSEMOVE && 0 <= _selected) {
		positionFruit(_selected, event.mouse - _grab);
		updateHover();
	}
	if (event.type == Common::EVENT_LBUTTONDOWN && _selected < 0) {
		_selected = hitFruit(event.mouse);
		if (0 <= _selected) {
			_demonstrating = false;
			_blinkCount = 0;
			for (uint i = 0; i < 3; i++)
				_trays[i]->setFrame(1);
			_pickup = _fruits[_selected].position;
			_grab = event.mouse - _pickup;
			_rules.remove(_selected);
			updateProjectors();
			playEffect(kResSound20115_ProjectorReady);
			emitEvent("tokenPickedUp");
		}
	}
	if (event.type == Common::EVENT_LBUTTONUP && 0 <= _selected)
		drop();
	return;
}

void CorralMachine::update(uint32 time) {
	_time = time;
	if (!_paused) {
		Common::Array<uint16> sounds;
		for (uint i = 0; i < _rules.party(); i++) {
			if (static_cast<int>(i) != _feeding && static_cast<int>(i) != _hover)
				_zerbles[i]->update(time, sounds);
			if (_moving[i] && !_zerbles[i]->isWalking()) {
				_moving[i] = false;
				if (_exiting[i]) {
					_exiting[i] = false;
					_lastExited = i;
					_zerbles[i]->setVisible(false, time);
					_fruitRemaining -= 1;
					_zerblesRemaining -= 1;
					if (!_fruitRemaining || !_zerblesRemaining)
						emitEvent("finished");
				}
			}
		}
		updateFeedback(sounds);
		if (_sniff[0]) {
			_sniff[0]->update(time, sounds);
			for (uint i = 1; i < 3; i++)
				if (_sniff[i])
					_sniff[i]->setFrame(_sniff[0]->displayedFrame());
		}
		updateBlink();
		for (uint i = 0; i < sounds.size(); i++)
			playEffect(sounds[i]);
	}
}

void CorralMachine::pause(bool paused, uint32 time) {
	if (_paused == paused)
		return;
	_paused = paused;
	for (uint i = 0; i < _rules.party(); i++)
		_zerbles[i]->pause(paused, time);
	if (_feedback[0])
		_feedback[0]->pauseAnimation(paused);
	if (_sniff[0])
		_sniff[0]->pauseAnimation(paused);
}

bool CorralMachine::overDraggable(Common::Point point) const {
	return _feeding < 0 && 0 <= hitFruit(point);
}

bool CorralMachine::pollSound(uint16 &sound) {
	if (_sounds.empty())
		return false;
	sound = _sounds[0];
	_sounds.remove_at(0);
	return true;
}

Common::String CorralMachine::debugAnswer() const {
	static const char *const names[] = {
		"shape",
		"color",
		"leaf",
	};
	Common::String result = Common::String::format("Corral level %u: left=%s, right=%s", _level, names[_rules.axis(0)], names[_rules.axis(1)]);
	if (1 < _level)
		result += Common::String::format(", top=%s", names[_rules.axis(2)]);
	result += ". Coordinates use the 800x600 game surface.\n";
	for (uint i = 0; i < _rules.projectorCount(); i++) {
		const Common::Rect &tray = _trays[i]->bounds;
		result += Common::String::format("Projector %u: drop at (%d,%d), fruit %d\n", i + 1,
										 tray.left + tray.width() / 2, tray.top + tray.height() / 2, _rules.projectedFruit(i) + 1);
	}
	if (0 <= _feeding)
		return result + "Wait for feeding feedback.\n";
	Common::Array<CorralRules::Feeding> plan;
	_rules.findAnswer(plan);
	for (uint i = 0; i < plan.size(); i++) {
		Common::Rect fruit = fruitBounds(plan[i].fruit), zerble = _zerbles[plan[i].zerble]->bounds();
		result += Common::String::format("Fruit %u: drag (%d,%d) to (%d,%d), zerble %u%s\n", plan[i].fruit + 1,
										 fruit.left + fruit.width() / 2, fruit.top + fruit.height() / 2, zerble.left + zerble.width() / 2, zerble.top + zerble.height() / 2,
										 plan[i].zerble + 1, _moving[plan[i].zerble] ? " (moving; wait)" : "");
	}
	return result;
}

} // namespace Zoombini3
} // namespace Trinket
