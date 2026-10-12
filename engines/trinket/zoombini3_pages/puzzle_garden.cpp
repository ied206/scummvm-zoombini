/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/zoombini3_pages/puzzle_garden.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "trinket/shell/character_action.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace Zoombini3 {

GardenRules::GardenRules() = default;

bool GardenRules::generate(uint level, uint party, Shell::Random &random) {
	if (level < 1 || 3 < level || party < 1 || 12 < party)
		return false;
	_level = level;
	_party = party;
	_passed = 0;
	_chances = level == 1 ? 5 : 10;
	for (uint i = 0; i < 12; i++) {
		_plants[i].hole = -1;
		for (uint j = 0; j < 4; j++)
			_plants[i].trait[j] = random.draw() % 4;
	}
	_axes[0] = random.draw() % 4;
	uint remaining[3], count = 0;
	for (uint i = 0; i < 4; i++)
		if (i != _axes[0]) {
			remaining[count] = i;
			count += 1;
		}
	_axes[1] = remaining[random.draw() % 3];
	count = 0;
	for (uint i = 0; i < 4; i++)
		if (i != _axes[0] && i != _axes[1]) {
			remaining[count] = i;
			count += 1;
		}
	_axes[2] = remaining[random.draw() % 2];
	if (level < 3)
		_axes[2] = _axes[0];
	if (level < 2)
		_axes[1] = _axes[0];
	_ignored = static_cast<uint16>(~((15 << (4 * _axes[0])) | (15 << (4 * _axes[1])) | (15 << (4 * _axes[2]))));
	for (uint i = 0; i < 64; i++) {
		_allowed[i] = 65535;
		_population[i] = 0;
	}
	return true;
}

uint16 GardenRules::value(uint plant) const {
	uint16 result = 0;
	for (uint i = 0; i < 4; i++)
		result |= 1 << (4 * i + _plants[plant].trait[i]);
	return result;
}

bool GardenRules::accepts(uint plant, uint hole) const {
	if (_party <= plant || holeCount() <= hole || _plants[plant].hole != -1 || !_chances)
		return false;
	uint16 intersection = _ignored | (value(plant) & _allowed[hole]);
	for (uint i = 0; i < 4; i++)
		if (!(intersection & (15 << (4 * i))))
			return false;
	return true;
}

bool GardenRules::place(uint plant, uint hole) {
	if (!accepts(plant, hole))
		return false;
	uint16 accepted = value(plant) | _ignored;
	_allowed[hole] = accepted;
	_population[hole] += 1;
	_plants[plant].hole = hole;
	_passed += 1;
	uint16 exclude = _ignored | ~accepted;
	for (uint i = 0; i < holeCount(); i++) {
		if (_population[i])
			continue;
		uint16 allowed = _allowed[i] & exclude;
		for (uint axis = 0; axis < _level; axis++) {
			uint divisor = axis == 0 ? 1 : axis == 1 ? 4
													 : 16;
			if ((i / divisor) % 4 == (hole / divisor) % 4) {
				uint16 mask = 15 << (4 * _axes[axis]);
				allowed = (allowed & ~mask) | (accepted & mask);
			}
		}
		_allowed[i] = allowed;
	}
	return true;
}

void GardenRules::reject() {
	if (_chances)
		_chances -= 1;
}

bool GardenRules::findAnswer(Common::Array<Placement> &plan) const {
	plan.clear();
	GardenRules trial = *this;
	for (uint i = 0; i < _party; i++) {
		if (0 <= _plants[i].hole)
			continue;
		bool found = false;
		for (uint hole = 0; hole < holeCount(); hole++) {
			if (!trial.place(i, hole))
				continue;
			Placement item = {i, hole};
			plan.push_back(item);
			found = true;
			break;
		}
		if (!found) {
			plan.clear();
			return false;
		}
	}
	return true;
}

Common::Point GardenRules::holePosition(uint level, uint hole) {
	uint column = hole % 4, row = (hole / 4) % 4, section = hole / 16;
	if (level == 1)
		return Common::Point(200 * column, 300);
	if (level == 2)
		return Common::Point(60 + 10 * row + (160 - 5 * row) * column, 390 - 90 * row);
	int x = section % 2 ? 390 + 80 * column - 2 * row : 30 + 80 * column + 2 * row;
	if (2 <= section)
		x += section % 2 ? -8 : 8;
	return Common::Point(x, (section < 2 ? 435 : 235) - 45 * row);
}

constexpr const char *GardenMachine::kAnimation;

GardenMachine::GardenMachine(TrinketEngine &vm, Shell::Scene &scene) : _vm(vm), _scene(scene) {
	const char *events[] = {
		"correct",
		"incorrectFilled",
		"incorrectEmpty",
		"finished",
		"tokenPickedUp",
		"snapDone",
	};
	for (uint i = 0; i < ARRAYSIZE(events); i++)
		addEvent(events[i]);
}

GardenMachine::~GardenMachine() {
	clearSprites();
}

void GardenMachine::clearSprites() {
	clearTrinkets();
	for (uint i = 0; i < _owned.size(); i++)
		_scene.remove(_owned[i]);
	_owned.clear();
}

Shell::Sprite *GardenMachine::add(uint id, int depth, int frame) {
	Shell::Sprite *sprite = _scene.add(Common::String::format(kAnimation, id), depth);
	if (!sprite) {
		_valid = false;
		return nullptr;
	}
	sprite->setFrame(frame);
	sprite->enabled = false;
	addTrinket(*sprite);
	_owned.push_back(sprite);
	return sprite;
}

bool GardenMachine::setup(uint level, uint party) {
	clearSprites();
	_level = level;
	_party = party;
	_selected = _rejected = -1;
	_returning = false;
	_valid = true;
	if (!_rules.generate(level, party, _vm.rnd()))
		return false;
	static constexpr uint holes[] = {
		18173,
		18172,
		18150,
	};
	for (uint i = 0; i < _rules.holeCount(); i++) {
		Shell::Sprite *hole = _holes[i] = add(holes[_level - 1], 1);
		if (!hole)
			return false;
		Common::Point point = GardenRules::holePosition(_level, i);
		hole->x = point.x;
		hole->y = point.y;
	}
	static constexpr uint large[] = {
		18177,
		18175,
		18176,
		18174,
	};
	static constexpr uint small[] = {
		18167,
		18155,
		18159,
		18151,
	};
	static constexpr int depths[] = {
		3,
		0,
		2,
		1,
	};
	for (uint i = 0; i < 12; i++) {
		PlantView &view = _plants[i];
		Common::Rect combined;
		for (uint j = 0; j < 4; j++) {
			Shell::Sprite *part = view.parts[j] = add((_level == 3 ? small : large)[j], 20 + depths[j], _rules.plant(i).trait[j] + 1);
			if (!part)
				return false;
			part->visible = i < _party;
			Common::Point origin(part->x, part->y);
			view.offsets[j] = origin;
			Common::Rect bounds(origin.x, origin.y, origin.x + part->width(), origin.y + part->height());
			if (j == 0)
				combined = bounds;
			else
				combined.extend(bounds);
		}
		for (uint j = 0; j < 4; j++)
			view.offsets[j] -= Common::Point(combined.left, combined.top);
		view.bounds = Common::Rect(combined.width(), combined.height());
		view.home = Common::Point(20 + 50 * i, _level == 3 ? 520 : 500);
		view.depth = 20;
		positionPlant(i, view.home);
	}
	_sun = add(18171, 1);
	if (!_sun)
		return false;
	updateSun();
	return true;
}

bool GardenMachine::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property != "remainingRejections" || !arguments.empty())
		return MultiTrinket::get(property, arguments, result);
	result = Core::ScriptValue(static_cast<int32>(_rules.chances()));
	return true;
}

bool GardenMachine::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property != "remainingRejections" || !arguments.empty())
		return MultiTrinket::set(property, arguments, value);
	_rules.setChances(value.integer());
	return true;
}

bool GardenMachine::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	_time = callTime();
	if (method == "setupGame" && arguments.size() == 2)
		return setup(arguments[0].integer(), arguments[1].integer());
	if (!arguments.empty())
		return MultiTrinket::play(method, arguments, result);
	if (method == "startRound")
		return true;
	if (method == "finishRound") {
		pause(true, _time);
		emitEvent("finished");
		return true;
	}
	if (method == "pauseRound" || method == "resumeRound") {
		pause(method == "pauseRound", _time);
		return true;
	}
	if (method == "snapAnswer") {
		snapAnswer();
		return true;
	}
	return MultiTrinket::play(method, arguments, result);
}

void GardenMachine::positionPlant(uint index, Common::Point point) {
	PlantView &view = _plants[index];
	view.position = point;
	for (uint j = 0; j < 4; j++) {
		view.parts[j]->x = point.x + view.offsets[j].x;
		view.parts[j]->y = point.y + view.offsets[j].y;
	}
}

int GardenMachine::hitPlant(Common::Point point) const {
	int result = -1, depth = -1;
	for (uint i = 0; i < _party; i++) {
		Common::Rect bounds = _plants[i].bounds;
		bounds.translate(_plants[i].position.x, _plants[i].position.y);
		if (depth <= _plants[i].depth && bounds.contains(point)) {
			result = i;
			depth = _plants[i].depth;
		}
	}
	return result;
}

int GardenMachine::hitHole(const Common::Rect &plantBounds) const {
	int result = -1;
	int bestArea = 0;
	for (uint i = 0; i < _rules.holeCount(); i++) {
		if (!plantBounds.intersects(_holes[i]->bounds))
			continue;
		Common::Rect overlap = plantBounds;
		overlap.clip(_holes[i]->bounds);
		int area = overlap.width() * overlap.height();
		// Equal overlaps retain the first registered hole.
		if (bestArea < area) {
			bestArea = area;
			result = i;
		}
	}
	return result;
}

void GardenMachine::updateSun() {
	int frame = MAX<int>(1, _sun->getNumberOfFrames() - _rules.chances());
	_sun->setFrame(frame);
}

void GardenMachine::plantAccepted(uint index, uint hole) {
	uint count = _rules.population(hole) - 1;
	static constexpr int horizontal[] = {
		25,
		15,
		15,
	},
						 vertical[] = {
							 40,
							 25,
							 5,
						 };
	int offsetY = -vertical[_level - 1];
	if (_party / 2 < count) {
		count -= _party / 2;
		offsetY /= 2;
	}
	int offsetX = count % 2 ? -static_cast<int>((count + 1) / 2) : static_cast<int>(count / 2);
	offsetX *= horizontal[_level - 1];
	Common::Rect bounds = _holes[hole]->bounds;
	positionPlant(index, Common::Point(bounds.left + (bounds.width() - _plants[index].bounds.width()) / 2 + offsetX,
									   bounds.top + (bounds.height() - _plants[index].bounds.height()) / 2 + offsetY));
	emitEvent(_rules.passed() == _party ? "finished" : "correct");
}

void GardenMachine::dropPlant() {
	if (_selected < 0)
		return;
	uint index = _selected;
	PlantView &view = _plants[index];
	Common::Rect bounds = view.bounds;
	bounds.translate(view.position.x, view.position.y);
	int hole = hitHole(bounds);
	_selected = -1;
	playEffect(kResSound18157_PlantDrop);
	if (hole < 0) {
		if (view.position.x < 600 && 480 < view.position.y)
			view.home = view.position;
		else
			positionPlant(index, _pickup);
		return;
	}
	if (_rules.place(index, hole)) {
		plantAccepted(index, hole);
		return;
	}
	_rejected = index;
	// Remaining capacity uses the native twelve-plant comparison.
	emitEvent(_party - _rules.population(hole) == 12 ? "incorrectEmpty" : "incorrectFilled");
}

void GardenMachine::event(const Common::Event &event) {
	if (_paused || 0 <= _rejected || !_party)
		return;
	if (event.type == Common::EVENT_MOUSEMOVE && 0 <= _selected)
		positionPlant(_selected, event.mouse - _grab);
	if (event.type == Common::EVENT_LBUTTONDOWN && _selected < 0) {
		int index = hitPlant(event.mouse);
		if (0 <= index) {
			PlantView &view = _plants[index];
			_depth += 1;
			view.depth = _depth;
			static constexpr int depths[] = {
				3,
				0,
				2,
				1,
			};
			for (uint j = 0; j < 4; j++)
				_scene.setDepth(view.parts[j], view.depth * 4 + depths[j]);
			if (_rules.plant(index).hole < 0 && _rules.chances()) {
				_selected = index;
				_pickup = view.position;
				_grab = event.mouse - view.position;
				playEffect(kResSound18156_PlantPickup);
				emitEvent("tokenPickedUp");
			}
		}
	}
	if (event.type == Common::EVENT_LBUTTONUP && 0 <= _selected)
		dropPlant();
}

void GardenMachine::snapAnswer() {
	if (_rejected < 0 || _returning)
		return;
	_returning = true;
	_returnTime = _time;
	_returnStart = _plants[_rejected].position;
	playEffect(kResSound18257_Return);
}

void GardenMachine::update(uint32 time) {
	_time = time;
	if (_paused || !_returning)
		return;
	// Curve PPS measures percent of the parameter interval per second.
	float elapsed = static_cast<float>(time - _returnTime) / 1000;
	float t = MIN<float>(1, elapsed), u = 1 - t;
	int firstX = _returnStart.x + (_pickup.x - _returnStart.x) / 3;
	int secondX = _returnStart.x + 2 * (_pickup.x - _returnStart.x) / 3;
	positionPlant(_rejected, Common::Point(u * u * u * _returnStart.x + 3 * u * u * t * firstX + 3 * u * t * t * secondX + t * t * t * _pickup.x,
										   u * u * u * _returnStart.y + t * t * t * _pickup.y));
	if (1 < elapsed) {
		_rules.reject();
		uint frame = _sun->getFrame() + 1;
		if (frame <= _sun->getNumberOfFrames())
			_sun->setFrame(frame);
		else
			_sun->visible = false;
		_rejected = -1;
		_returning = false;
		emitEvent("snapDone");
	}
}

void GardenMachine::pause(bool paused, uint32 time) {
	if (_paused == paused)
		return;
	_paused = paused;
	if (paused)
		_pauseTime = time;
	else
		_returnTime += time - _pauseTime;
}

bool GardenMachine::overDraggable(Common::Point point) const {
	int index = hitPlant(point);
	return _rejected < 0 && _rules.chances() && 0 <= index && _rules.plant(index).hole < 0;
}

bool GardenMachine::pollSound(uint16 &sound) {
	if (_sounds.empty())
		return false;
	sound = _sounds[0];
	_sounds.remove_at(0);
	return true;
}

Common::String GardenMachine::debugAnswer() const {
	static const char *const names[] = {
		"flower",
		"stem",
		"leaf",
		"root",
	};
	Common::String result = Common::String::format("Garden level %u: columns=%s", _level, names[_rules.axis(0)]);
	if (1 < _level)
		result += Common::String::format(", rows=%s", names[_rules.axis(1)]);
	if (2 < _level)
		result += Common::String::format(", plots=%s", names[_rules.axis(2)]);
	result += ". Coordinates use the 800x600 game surface.\n";
	if (_rejected >= 0)
		return result + "Wait for the rejected plant to return.\n";
	Common::Array<GardenRules::Placement> plan;
	if (!_rules.findAnswer(plan))
		return result + "No remaining legal completion with the current chances.\n";
	for (uint i = 0; i < plan.size(); i++) {
		const PlantView &view = _plants[plan[i].plant];
		Common::Rect hole = _holes[plan[i].hole]->bounds;
		result += Common::String::format("Plant %u: drag (%d,%d) to (%d,%d), hole %u\n", plan[i].plant + 1,
										 view.position.x + view.bounds.width() / 2, view.position.y + view.bounds.height() / 2,
										 hole.left + hole.width() / 2, hole.top + hole.height() / 2, plan[i].hole + 1);
	}
	return result;
}

} // namespace Zoombini3
} // namespace Trinket
