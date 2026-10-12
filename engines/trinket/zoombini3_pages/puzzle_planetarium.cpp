/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/zoombini3_pages/puzzle_planetarium.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "trinket/trinket.h"
#include <math.h>

namespace Trinket {
namespace Zoombini3 {

const int PlanetariumRules::kMoonHiddenHours[28][2] = {
	{13, 23}, {13, 23}, {14, 23}, {15, 23}, {16, 23}, {17, 23}, {18, 23}, {19, 23}, {19, 23}, {20, 23}, {21, 23}, {22, 23}, {23, 23}, {-1, -1}, {-1, -1}, {-1, -1}, {13, 13}, {13, 14}, {13, 15}, {13, 16}, {13, 17}, {13, 17}, {13, 18}, {13, 19}, {13, 20}, {13, 21}, {13, 22}, {13, 23}};

PlanetariumRules::PlanetariumRules() = default;

int PlanetariumRules::randomExcept(Shell::Random &random, uint bound, int excluded) {
	int value = random.draw() % bound;
	return value == excluded ? (value + 1) % bound : value;
}

void PlanetariumRules::addPieces(uint exhibit, Denomination type, const uint *values, uint count) {
	uint position = 0;
	for (uint i = 0; i < _pieces.size(); i++)
		if (_pieces[i].exhibit == exhibit && _pieces[i].type == type)
			position += 1;
	for (uint i = 0; i < count; i++) {
		Piece piece = {values[i], exhibit, position + i, type, false, true};
		_pieces.push_back(piece);
	}
}

bool PlanetariumRules::generate(uint level, Shell::Random &random) {
	if (level < 1 || 3 < level)
		return false;
	*this = PlanetariumRules();
	_level = level;
	_day[0] = _day[1] = -1;
	static constexpr uint dayValues[] = {
		1,
		4,
		5,
		5,
		7,
		10,
		10,
	};
	static constexpr uint hourValues[] = {
		4,
		5,
		5,
		7,
		10,
		10,
	};
	static constexpr uint specialValues[] = {
		4,
		5,
		7,
		7,
		10,
		10,
	};
	static constexpr uint helpers[] = {
		1,
		2,
	};
	if (_level == 3) {
		_targetDay = random.draw() % 28;
		for (uint i = 0; i < 2; i++) {
			_day[i] = randomExcept(random, 28, _targetDay);
			addPieces(i, kDay01, dayValues, ARRAYSIZE(dayValues));
		}
	} else if (_level == 2) {
		_day[0] = _day[1] = 7 * (random.draw() % 4);
	}
	_targetHour = random.draw() % 24;
	for (uint i = 0; i < 2; i++) {
		int low = showMoon() ? kMoonHiddenHours[_day[i]][0] : -1;
		int high = showMoon() ? kMoonHiddenHours[_day[i]][1] : -1;
		if (i == 1 && showMoon() && low != -1) {
			int span = high - low + 1;
			int hour = random.draw() % (24 - span);
			if (low <= hour && hour <= high) {
				hour = (hour + span) % 24;
				while (hour == _targetHour || (low <= hour && hour <= high))
					hour = (hour + 1) % 24;
			}
			_hour[i] = hour;
		} else {
			_hour[i] = randomExcept(random, 24, _targetHour);
		}
		if (_level == 1)
			addPieces(i, kHour00, helpers, ARRAYSIZE(helpers));
		int distance = (_targetHour - _hour[i] + 24) % 24;
		addPieces(i, kHour00, distance == 18 || distance == 23 ? specialValues : hourValues, ARRAYSIZE(hourValues));
	}
	return true;
}

bool PlanetariumRules::canUse(uint index) const {
	if (_pieces.size() <= index)
		return false;
	const Piece &piece = _pieces[index];
	return !piece.consumed && piece.enabled && (!advancing(piece.exhibit) || _mode[piece.exhibit] == piece.type);
}

void PlanetariumRules::disablePieces(uint exhibit, Denomination type) {
	for (uint i = 0; i < _pieces.size(); i++)
		if (_pieces[i].exhibit == exhibit && _pieces[i].type == type)
			_pieces[i].enabled = false;
}

bool PlanetariumRules::use(uint index) {
	if (!canUse(index))
		return false;
	Piece &piece = _pieces[index];
	piece.consumed = true;
	_mode[piece.exhibit] = piece.type;
	_remaining[piece.exhibit] += piece.value;
	int projected = (piece.type == kHour00 ? _hour[piece.exhibit] : _day[piece.exhibit]) + _remaining[piece.exhibit];
	int period = piece.type == kHour00 ? 24 : 28;
	if (period <= projected)
		projected -= period;
	if (projected == (piece.type == kHour00 ? _targetHour : _targetDay))
		disablePieces(piece.exhibit, piece.type);
	return true;
}

bool PlanetariumRules::hasTurns() const {
	for (uint i = 0; i < _pieces.size(); i++)
		if (!_pieces[i].consumed && _pieces[i].enabled)
			return true;
	return false;
}

bool PlanetariumRules::solved(uint exhibit, Denomination type) const {
	return type == kHour00 ? _hour[exhibit] == _targetHour : _level == 3 && _day[exhibit] == _targetDay;
}

void PlanetariumRules::checkTarget(uint exhibit, Denomination type, Common::Array<Result> &results) {
	Result result = {exhibit, type, solved(exhibit, type), false};
	if (result.correct) {
		result.hitTarget = !advancing(exhibit) || _mode[exhibit] != type;
		if (type == kDay01 && result.hitTarget)
			disablePieces(exhibit, type);
	}
	results.push_back(result);
}

bool PlanetariumRules::step(uint exhibit, Common::Array<Result> &results) {
	if (2 <= exhibit || !advancing(exhibit))
		return false;
	_remaining[exhibit] -= 1;
	if (_mode[exhibit] == kHour00) {
		_hour[exhibit] = (_hour[exhibit] + 1) % 24;
		if (_hour[exhibit] == 0 && 0 <= _day[exhibit]) {
			_day[exhibit] = (_day[exhibit] + 1) % 28;
			if (_level == 3 && _day[exhibit] == _targetDay)
				checkTarget(exhibit, kDay01, results);
		}
		checkTarget(exhibit, kHour00, results);
	} else {
		_day[exhibit] = (_day[exhibit] + 1) % 28;
		checkTarget(exhibit, kDay01, results);
	}
	return true;
}

bool PlanetariumRules::findAnswer(uint exhibit, Denomination type, Common::Array<uint> &pieces) const {
	pieces.clear();
	if (2 <= exhibit || (type == kDay01 && _level != 3) || advancing(exhibit))
		return false;
	if (solved(exhibit, type))
		return true;
	Common::Array<uint> candidates;
	for (uint i = 0; i < _pieces.size(); i++)
		if (_pieces[i].exhibit == exhibit && _pieces[i].type == type && canUse(i))
			candidates.push_back(i);
	uint period = type == kHour00 ? 24 : 28;
	int start = type == kHour00 ? _hour[exhibit] : _day[exhibit];
	int target = type == kHour00 ? _targetHour : _targetDay;
	uint bestMask = 0, bestTotal = UINT_MAX;
	for (uint mask = 1; mask < (1u << candidates.size()); mask++) {
		uint total = 0;
		for (uint i = 0; i < candidates.size(); i++)
			if (mask & (1u << i))
				total += _pieces[candidates[i]].value;
		if ((start + total) % period != static_cast<uint>(target) || bestTotal <= total)
			continue;
		bestMask = mask;
		bestTotal = total;
	}
	for (uint i = 0; i < candidates.size(); i++)
		if (bestMask & (1u << i))
			pieces.push_back(candidates[i]);
	return bestMask != 0;
}

bool PlanetariumRules::applyAnswer(PlanetariumRules &state, const Common::Array<uint> &pieces) {
	for (uint i = 0; i < pieces.size(); i++) {
		uint exhibit = state.piece(pieces[i]).exhibit;
		if (!state.use(pieces[i]))
			return false;
		Common::Array<Result> results;
		while (state.advancing(exhibit))
			state.step(exhibit, results);
	}
	return true;
}

bool PlanetariumRules::findCompleteAnswer(Common::Array<uint> &pieces) const {
	pieces.clear();
	if (!_level || advancing(0) || advancing(1))
		return false;
	PlanetariumRules state = *this;
	for (uint exhibit = 0; exhibit < 2; exhibit++) {
		Common::Array<uint> bills;
		if (_level == 3) {
			for (uint i = 0; i < state.pieceCount(); i++)
				if (state.piece(i).exhibit == exhibit && state.piece(i).type == kDay01 && state.canUse(i))
					bills.push_back(i);
		}
		bool found = false;
		for (uint mask = 0; mask < (1u << bills.size()) && !found; mask++) {
			PlanetariumRules trial = state;
			Common::Array<uint> prefix, hours, days;
			for (uint i = 0; i < bills.size(); i++)
				if (mask & (1u << i))
					prefix.push_back(bills[i]);
			if (!applyAnswer(trial, prefix) || !trial.findAnswer(exhibit, kHour00, hours) || !applyAnswer(trial, hours))
				continue;
			if (_level == 3 && (!trial.findAnswer(exhibit, kDay01, days) || !applyAnswer(trial, days)))
				continue;
			state = trial;
			pieces.push_back(prefix);
			pieces.push_back(hours);
			pieces.push_back(days);
			found = true;
		}
		if (!found) {
			pieces.clear();
			return false;
		}
	}
	if (!state.solved(0, kHour00) || !state.solved(1, kHour00) ||
		(_level == 3 && (!state.solved(0, kDay01) || !state.solved(1, kDay01)))) {
		pieces.clear();
		return false;
	}
	return true;
}

constexpr const char *PlanetariumMachine::kAnimation;
constexpr const char *PlanetariumMachine::kCDAnimation;
const int PlanetariumMachine::kMoonAngles[28] = {
	180,
	167,
	154,
	141,
	128,
	115,
	102,
	90,
	77,
	64,
	51,
	38,
	25,
	12,
	0,
	347,
	335,
	321,
	308,
	295,
	282,
	270,
	257,
	244,
	231,
	218,
	205,
	192,
};
const Common::Point PlanetariumMachine::kHourPositions[2][9] = {
	{{197, 475}, {235, 475}, {275, 475}, {197, 518}, {235, 518}, {275, 518}, {197, 556}, {235, 556}, {275, 556}},
	{{495, 475}, {535, 475}, {578, 475}, {495, 518}, {535, 518}, {578, 518}, {495, 556}, {535, 556}, {578, 556}}};
const Common::Point PlanetariumMachine::kDayPositions[2][9] = {
	{{16, 475}, {74, 475}, {130, 475}, {16, 518}, {74, 518}, {130, 518}, {36, 556}, {0, 0}, {0, 0}},
	{{628, 475}, {688, 475}, {746, 475}, {628, 518}, {688, 518}, {746, 518}, {666, 556}, {0, 0}, {0, 0}}};

PlanetariumMachine::PlanetariumMachine(Shell::Scene &scene, Shell::Random &random, int depth) : _scene(scene), _random(random), _depth(depth) {
	addEvent("pickedUp");
	addEvent("dropped");
	addEvent("solved");
	addEvent("wrong");
}

PlanetariumMachine::~PlanetariumMachine() {
	clearPuzzle();
}

void PlanetariumMachine::clearPuzzle() {
	for (uint i = 0; i < _owned.size(); i++)
		_scene.remove(_owned[i]);
	_owned.clear();
	_pieces.clear();
	_pausedSprites.clear();
	_sounds.clear();
	_earth = _earthMoon = _sky = _sun = nullptr;
	for (uint i = 0; i < ARRAYSIZE(_moon); i++)
		_moon[i] = nullptr;
	for (uint i = 0; i < ARRAYSIZE(_skyFront); i++)
		_skyFront[i] = nullptr;
	for (uint i = 0; i < ARRAYSIZE(_slots); i++) {
		_slots[i] = nullptr;
		_covers[i] = Cover();
	}
	for (uint i = 0; i < 2; i++) {
		_stepTime[i] = 0;
		_waiting[i] = false;
	}
	_dragged = -1;
}

bool PlanetariumMachine::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property == "isSolved" && arguments.size() == 2 && 0 <= arguments[0].integer() && arguments[0].integer() < 2 &&
		0 <= arguments[1].integer() && arguments[1].integer() < 2) {
		result = Core::ScriptValue(static_cast<int32>(_rules.solved(arguments[0].integer(),
																	static_cast<PlanetariumRules::Denomination>(arguments[1].integer()))));
		return true;
	}
	if (!arguments.empty())
		return Core::ScriptableObject::get(property, arguments, result);
	if (property == "lastSolvedExhibit")
		result = Core::ScriptValue(static_cast<int32>(_lastResult.exhibit));
	else if (property == "lastSolvedDenomType")
		result = Core::ScriptValue(static_cast<int32>(_lastResult.type));
	else if (property == "lastSolvedByHittingTarget")
		result = Core::ScriptValue(static_cast<int32>(_lastResult.hitTarget));
	else if (property == "turnsLeft")
		result = Core::ScriptValue(static_cast<int32>(_rules.hasTurns()));
	else if (property == "isExhibitAdvancing")
		result = Core::ScriptValue(static_cast<int32>(_rules.advancing(0) || _rules.advancing(1)));
	else if (property == "displayText")
		result = Core::ScriptValue(static_cast<int32>(_displayText));
	else if (property == "debugMode")
		result = Core::ScriptValue(static_cast<int32>(_debugMode));
	else if (property == "pieceGrabAOCursorID")
		result = _grabCursor;
	else if (property == "pieceSpotAOCursorID")
		result = _spotCursor;
	else
		return Core::ScriptableObject::get(property, arguments, result);
	return true;
}

bool PlanetariumMachine::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (!arguments.empty())
		return Core::ScriptableObject::set(property, arguments, value);
	if (property == "enabled")
		_enabled = value.integer() != 0;
	else if (property == "displayText")
		_displayText = value.integer() != 0;
	else if (property == "debugMode")
		_debugMode = value.integer() != 0;
	else if (property == "pieceGrabAOCursorID")
		_grabCursor = value;
	else if (property == "pieceSpotAOCursorID")
		_spotCursor = value;
	else
		return Core::ScriptableObject::set(property, arguments, value);
	return true;
}

bool PlanetariumMachine::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if ((method == "pause" || method == "resume") && arguments.empty()) {
		pause(method == "pause", callTime());
		return true;
	}
	if (method == "forceDrop" && arguments.empty()) {
		forceDrop();
		return true;
	}
	if (method == "clearPuzzle" && arguments.empty()) {
		clearPuzzle();
		return true;
	}
	if (method == "generatePuzzle" && 2 <= arguments.size() && arguments.size() <= 4) {
		bool targetHour = arguments[0].integer() != 0, targetDay = arguments[1].integer() != 0;
		bool showMoon = 2 < arguments.size() && arguments[2].integer() != 0;
		bool helpers = 3 < arguments.size() && arguments[3].integer() != 0;
		uint level = targetDay ? 3 : (showMoon ? 2 : 1);
		if (!targetHour || showMoon != (1 < level) || helpers != (level == 1))
			return false;
		clearPuzzle();
		_valid = true;
		return _rules.generate(level, _random) && load();
	}
	return Core::ScriptableObject::play(method, arguments, result);
}

Shell::Sprite *PlanetariumMachine::add(uint id, int depth, bool cd, int frame) {
	Shell::Sprite *sprite = _scene.add(Common::String::format(cd ? kCDAnimation : kAnimation, id), _depth + depth - 10);
	if (!sprite) {
		_valid = false;
		return nullptr;
	}
	_owned.push_back(sprite);
	sprite->enabled = false;
	sprite->setFrame(frame);
	return sprite;
}

bool PlanetariumMachine::load() {
	add(14053, 0);
	_earth = add(14052, 11, true);
	_sky = add(14001, 0, true);
	_sun = add(14055, 10);
	_skyFront[0] = add(14003, 13);
	_skyFront[1] = add(14004, 13);
	add(14002, 13);
	add(14002, 13, false, 2);
	if (_rules.showMoon()) {
		_earthMoon = add(14054, 8, true);
		for (uint i = 0; i < 28; i++) {
			_moon[i] = add(14056 + i, 10);
			if (_moon[i])
				_moon[i]->visible = false;
		}
	}
	const int hour = (_rules.targetHour() + 6) % 24;
	add(14015, 10);
	add(14016, 11, false, hour % 12 ? hour % 12 : 12);
	add(14017, 10, false, hour < 12 ? 1 : 2);
	if (_rules.level() == 3) {
		Shell::Sprite *calendar = add(14023, 10);
		Shell::Sprite *mark = add(14024, 11);
		if (calendar && mark) {
			mark->x = calendar->x + 21 + (_rules.targetDay() % 7) * (mark->width() + 3);
			mark->y = calendar->y + 15 + (_rules.targetDay() / 7) * (mark->height() + 1);
		}
	}
	static constexpr uint slotIDs[] = {
		14011,
		14014,
		14012,
		14090,
	};
	static constexpr uint openIDs[] = {
		14029,
		14033,
		14031,
		14035,
	};
	static constexpr uint closeIDs[] = {
		14030,
		14034,
		14032,
		14036,
	};
	for (uint exhibit = 0; exhibit < 2; exhibit++) {
		for (uint type = 0; type < (_rules.level() == 3 ? 2u : 1u); type++) {
			const uint index = exhibit * 2 + type;
			_slots[index] = add(slotIDs[index], 14);
			Shell::Sprite *face = add(type ? 14025 : 14018, 10);
			Shell::Sprite *mark = add(type ? 14026 : 14019, 11);
			if (!face || !mark)
				return false;
			if (exhibit) {
				const int dx = (type ? 576 : 524) - face->x;
				const int dy = (type ? 19 : 30) - face->y;
				face->x += dx;
				face->y += dy;
				mark->x += dx;
				mark->y += dy;
			}
			if (type) {
				mark->x = face->x + 8 + (_rules.targetDay() % 7) * (mark->width() + 2);
				mark->y = face->y + 14 + (_rules.targetDay() / 7) * mark->height();
			} else {
				mark->setFrame(hour % 12 ? hour % 12 : 12);
			}
			_covers[index].opening = add(openIDs[index], 12);
			_covers[index].closing = add(closeIDs[index], 12);
			if (_covers[index].closing)
				_covers[index].closing->visible = false;
		}
	}
	static constexpr uint values[2][6] = {{1, 2, 4, 5, 7, 10}, {1, 4, 5, 7, 10, 0}};
	for (uint i = 0; i < _rules.pieceCount(); i++) {
		const PlanetariumRules::Piece &piece = _rules.piece(i);
		uint frame = 0;
		while (frame < 6 && values[piece.type][frame] != piece.value)
			frame += 1;
		Shell::Sprite *sprite = add(piece.type == PlanetariumRules::kHour00 ? 14010 : 14013, 10, false, frame + 1);
		if (!sprite)
			return false;
		Common::Point position = piece.type == PlanetariumRules::kHour00 ? kHourPositions[piece.exhibit][piece.position] : kDayPositions[piece.exhibit][piece.position];
		// Setting the composite position moves its bounds to the supply coordinate.
		sprite->x = position.x;
		sprite->y = position.y;
		_scene.setDepth(sprite, 10 + sprite->y + sprite->height() - 470);
		_pieces.push_back(sprite);
	}
	if (!_valid)
		return false;
	updateExhibit(0, 0);
	updateExhibit(1, 0);
	return true;
}

float PlanetariumMachine::hourAngle(float hour) {
	float angle = hour * 15 + 180;
	return 360 <= angle ? angle - 360 : angle;
}

Common::Point PlanetariumMachine::celestialPosition(float angle) {
	const float radians = angle * 3.1415927f * 0.0055555557f;
	return Common::Point(static_cast<int>(cos(radians) * 141 + 607), static_cast<int>(sin(radians) * 225 + 386));
}

void PlanetariumMachine::centerAt(Shell::Sprite *sprite, Common::Point position) {
	sprite->x = position.x - sprite->width() / 2;
	sprite->y = position.y - sprite->height() / 2;
}

void PlanetariumMachine::updateExhibit(uint exhibit, float fraction) {
	const bool advancingDay = _rules.advancing(exhibit) && _rules.advancingType(exhibit) == PlanetariumRules::kDay01;
	float hour = _rules.hour(exhibit) + fraction * (advancingDay ? 24 : 1);
	if (24 <= hour)
		hour -= 24;
	int day = _rules.day(exhibit);
	if (!exhibit) {
		_earth->setFrame(1 + static_cast<int>(hour * 4) % 96);
		if (_earthMoon) {
			int offset = static_cast<int>(hour) / 4;
			int frame = 1 + day * 6 + offset + (advancingDay ? static_cast<int>(fraction * 6) : 0);
			_earthMoon->setFrame((frame - 1) % 168 + 1);
			_scene.setDepth(_earthMoon, 15 <= day ? 12 : 8);
		}
		return;
	}
	uint skyFrame = static_cast<int>(hour) == 0 || static_cast<int>(hour) == 23 ? 2 : (hour < 11 ? 1 : (hour < 13 ? 3 : 4));
	_sky->setFrame(skyFrame);
	_skyFront[0]->setFrame(skyFrame);
	_skyFront[1]->setFrame(skyFrame);
	float angle = hourAngle(hour);
	centerAt(_sun, celestialPosition(angle));
	_sun->visible = angle <= 15 || 165 <= angle;
	if (0 <= day) {
		if (advancingDay && hour < _rules.hour(exhibit))
			day = (day + 1) % 28;
		for (uint i = 0; i < 28; i++)
			_moon[i]->visible = false;
		Shell::Sprite *moon = _moon[day];
		angle = hour * 15 + kMoonAngles[day];
		if (360 <= angle)
			angle -= 360;
		int frame = 1;
		if (static_cast<int>(hour) == 12)
			frame = 2 + static_cast<int>((hour - 12) * 6);
		else if (static_cast<int>(hour) == 23)
			frame = 8 - static_cast<int>((hour - 23) * 6);
		moon->setFrame(frame);
		centerAt(moon, celestialPosition(angle));
		moon->visible = 12 <= hour && (angle <= 15 || 165 <= angle);
	}
}

uint PlanetariumMachine::stepDuration(uint exhibit) const {
	if (_rules.advancingType(exhibit) == PlanetariumRules::kDay01)
		return exhibit ? 2500 : 750;
	return exhibit ? 500 : 1000 / (_earth->asset->fps() / 4);
}

void PlanetariumMachine::beginStep(uint exhibit, uint32 time) {
	_stepTime[exhibit] = time;
	_waiting[exhibit] = false;
	if (_rules.advancingType(exhibit) == PlanetariumRules::kDay01)
		_sounds.push_back(exhibit ? kResSound14055_ExhibitC : kResSound14052_ExhibitA);
}

void PlanetariumMachine::openCover(uint index, bool closeAfter) {
	Cover &cover = _covers[index];
	cover.closeAfterOpening = closeAfter;
	if (!cover.opening || cover.state < 2)
		return;
	cover.closing->visible = false;
	cover.opening->visible = true;
	cover.opening->setFrame(1);
	cover.opening->setRepeat(1);
	cover.opening->pauseAnimation(false);
	cover.state = 0;
}

void PlanetariumMachine::closeCover(uint index) {
	Cover &cover = _covers[index];
	if (!cover.opening || 2 <= cover.state)
		return;
	cover.opening->visible = false;
	cover.closing->visible = true;
	cover.closing->setFrame(1);
	cover.closing->setRepeat(1);
	cover.closing->pauseAnimation(false);
	cover.state = 2;
}

void PlanetariumMachine::updateCovers() {
	for (uint i = 0; i < 4; i++) {
		Cover &cover = _covers[i];
		if (!cover.opening)
			continue;
		if (cover.state == 0 && !cover.opening->isRunning()) {
			cover.state = 1;
			if (cover.closeAfterOpening)
				closeCover(i);
		} else if (cover.state == 2 && !cover.closing->isRunning()) {
			cover.state = 3;
			cover.closing->visible = false;
			cover.opening->setFrame(1);
			cover.opening->visible = true;
		}
	}
}

void PlanetariumMachine::update(uint32 time) {
	_time = time;
	_scene.update(time, _sounds);
	if (_paused)
		return;
	updateCovers();
	for (uint exhibit = 0; exhibit < 2; exhibit++) {
		if (!_rules.advancing(exhibit))
			continue;
		uint elapsed = time - _stepTime[exhibit];
		if (_waiting[exhibit]) {
			if (elapsed < 200)
				continue;
			beginStep(exhibit, time);
			elapsed = 0;
		}
		const uint duration = stepDuration(exhibit);
		updateExhibit(exhibit, MIN<float>(1, static_cast<float>(elapsed) / duration));
		if (elapsed < duration)
			continue;
		Common::Array<PlanetariumRules::Result> results;
		_rules.step(exhibit, results);
		_sounds.push_back(exhibit ? kResSound14084_ExhibitD : kResSound14054_ExhibitB);
		if (_rules.level() == 3 && _rules.hour(exhibit) == 0 && _rules.day(exhibit) != _rules.targetDay())
			closeCover(exhibit * 2 + 1);
		for (uint i = 0; i < results.size(); i++) {
			if (results[i].correct) {
				openCover(exhibit * 2 + results[i].type, !results[i].hitTarget);
				_lastResult = results[i];
			}
			emitEvent(results[i].correct ? "solved" : "wrong");
		}
		updateExhibit(exhibit, 0);
		_stepTime[exhibit] = time;
		_waiting[exhibit] = _rules.advancing(exhibit);
	}
}

Common::Rect PlanetariumMachine::pieceBounds(uint index) const {
	if (_pieces.size() <= index)
		return Common::Rect();
	Shell::Sprite *sprite = _pieces[index];
	return Common::Rect(sprite->x, sprite->y, sprite->x + sprite->width(), sprite->y + sprite->height());
}

Common::Rect PlanetariumMachine::slotBounds(uint index) const {
	if (_rules.pieceCount() <= index)
		return Common::Rect();
	const PlanetariumRules::Piece &piece = _rules.piece(index);
	Shell::Sprite *slot = _slots[piece.exhibit * 2 + piece.type];
	return Common::Rect(slot->x, slot->y, slot->x + slot->width(), slot->y + slot->height());
}

int PlanetariumMachine::hitPiece(Common::Point point) const {
	if (!_enabled || _paused)
		return -1;
	int best = -1;
	for (uint i = 0; i < _pieces.size(); i++) {
		if (!_rules.piece(i).consumed && _rules.piece(i).enabled && pieceBounds(i).contains(point) &&
			(best < 0 || _pieces[best]->z <= _pieces[i]->z))
			best = i;
	}
	return best;
}

void PlanetariumMachine::updateSlot() {
	for (uint i = 0; i < 4; i++)
		if (_slots[i])
			_slots[i]->setFrame(1);
	if (0 <= _dragged && _rules.canUse(_dragged) && pieceBounds(_dragged).intersects(slotBounds(_dragged))) {
		const PlanetariumRules::Piece &piece = _rules.piece(_dragged);
		_slots[piece.exhibit * 2 + piece.type]->setFrame(2);
	}
}

void PlanetariumMachine::drop() {
	if (_dragged < 0)
		return;
	const uint index = _dragged;
	const PlanetariumRules::Piece piece = _rules.piece(index);
	Shell::Sprite *sprite = _pieces[index];
	bool advancing = _rules.advancing(piece.exhibit);
	if (pieceBounds(index).intersects(slotBounds(index)) && _rules.use(index)) {
		debug(1, "Planetarium consumed piece %u: exhibit %u type %u value %u", index + 1, piece.exhibit, piece.type, piece.value);
		sprite->visible = false;
		_sounds.push_back(piece.type == PlanetariumRules::kHour00 ? kResSound14011_HourPlace : kResSound14087_DayPlace);
		if (!advancing)
			beginStep(piece.exhibit, _time);
	} else {
		const Common::Rect supply(piece.exhibit ? 490 : 0, 470, piece.exhibit ? 800 : 320, 598);
		if (!pieceBounds(index).intersects(supply)) {
			sprite->x = _oldPosition.x;
			sprite->y = _oldPosition.y;
			_sounds.push_back(kResSound14085_ExhibitE);
		} else {
			sprite->x = CLIP<int>(sprite->x, supply.left, supply.right - sprite->width());
			sprite->y = CLIP<int>(sprite->y, supply.top, supply.bottom - sprite->height());
			_sounds.push_back(piece.type == PlanetariumRules::kHour00 ? kResSound14010_HourMove : kResSound14086_DayMove);
		}
		_scene.setDepth(sprite, 10 + sprite->y + sprite->height() - 470);
	}
	_dragged = -1;
	updateSlot();
	emitEvent("dropped");
}

void PlanetariumMachine::forceDrop() {
	drop();
}

void PlanetariumMachine::event(const Common::Event &event, uint32 time) {
	_time = time;
	if (_paused)
		return;
	if (event.type == Common::EVENT_LBUTTONDOWN && _dragged < 0) {
		_dragged = hitPiece(event.mouse);
		if (0 <= _dragged) {
			Shell::Sprite *sprite = _pieces[_dragged];
			_oldPosition = Common::Point(sprite->x, sprite->y);
			_grabOffset = event.mouse - _oldPosition;
			_scene.setDepth(sprite, 10000);
			_sounds.push_back(_rules.piece(_dragged).type == PlanetariumRules::kHour00 ? kResSound14010_HourMove : kResSound14086_DayMove);
			updateSlot();
			emitEvent("pickedUp");
		}
	} else if (0 <= _dragged && (event.type == Common::EVENT_MOUSEMOVE || event.type == Common::EVENT_LBUTTONUP)) {
		_pieces[_dragged]->x = event.mouse.x - _grabOffset.x;
		_pieces[_dragged]->y = event.mouse.y - _grabOffset.y;
		updateSlot();
		if (event.type == Common::EVENT_LBUTTONUP)
			drop();
	}
}

void PlanetariumMachine::pause(bool paused, uint32 time) {
	if (_paused == paused)
		return;
	_time = time;
	if (paused) {
		_pauseTime = time;
		for (uint i = 0; i < _owned.size(); i++) {
			if (_owned[i]->isRunning()) {
				_pausedSprites.push_back(_owned[i]);
				_owned[i]->pauseAnimation(true);
			}
		}
	} else {
		for (uint i = 0; i < 2; i++)
			_stepTime[i] += time - _pauseTime;
		for (uint i = 0; i < _pausedSprites.size(); i++)
			_pausedSprites[i]->pauseAnimation(false);
		_pausedSprites.clear();
	}
	_paused = paused;
}

bool PlanetariumMachine::pollSound(uint16 &sound) {
	if (_sounds.empty())
		return false;
	sound = _sounds.front();
	_sounds.remove_at(0);
	return true;
}

constexpr const char *Incubator::kAnimation;
const Common::Point Incubator::kPupaPositions[2][6][4] = {
	{{{384, 474}, {435, 472}, {401, 479}, {418, 477}}, {{376, 495}, {382, 515}, {392, 530}, {390, 505}}, {{434, 494}, {428, 508}, {418, 521}, {414, 537}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}, {{0, 0}, {0, 0}, {0, 0}, {0, 0}}},
	{{{377, 465}, {391, 474}, {0, 0}, {0, 0}}, {{357, 494}, {371, 505}, {0, 0}, {0, 0}}, {{373, 538}, {388, 530}, {0, 0}, {0, 0}}, {{418, 531}, {429, 541}, {0, 0}, {0, 0}}, {{436, 500}, {450, 512}, {0, 0}, {0, 0}}, {{419, 459}, {423, 476}, {0, 0}, {0, 0}}}};

Incubator::Incubator(TrinketEngine &vm, Shell::Scene &scene, int depth, uint rows) : _vm(vm), _scene(scene), _rows(rows) {
	z = depth;
}

Incubator::~Incubator() {
	for (uint i = 0; i < _owned.size(); i++)
		_scene.remove(_owned[i]);
}

Shell::Sprite *Incubator::add(uint id, int depth, int frame) {
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

void Incubator::remove(Shell::Sprite *sprite) {
	for (uint i = 0; i < _owned.size(); i++) {
		if (_owned[i] == sprite) {
			_owned.remove_at(i);
			_scene.remove(sprite);
			return;
		}
	}
}

bool Incubator::load() {
	if (_rows != 3 && _rows != 6)
		return false;
	add(_rows == 6 ? 14041 : 14037, z);
	for (uint row = 0; row < _rows; row++)
		_doors[row] = add((_rows == 6 ? 14042 : 14038) + row, z + 2);
	return _valid;
}

bool Incubator::setPopulation(uint row, uint count) {
	if (_rows <= row || 4 < count)
		return false;
	for (uint i = 0; i < _population[row]; i++)
		remove(_pupae[row][i].sprite);
	_population[row] = count;
	_transformed[row] = 0;
	for (uint i = 0; i < count; i++) {
		Pupa &pupa = _pupae[row][i];
		pupa = Pupa();
		pupa.sprite = add(14048, z + 1, _vm.rnd().draw() % 4 + 1);
		if (!pupa.sprite)
			return false;
		Common::Point position = kPupaPositions[_rows == 6 ? 1 : 0][row][i];
		pupa.sprite->x = position.x - pupa.sprite->width() / 2;
		pupa.sprite->y = position.y - pupa.sprite->height() / 2;
	}
	return true;
}

bool Incubator::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if ((property == "pupaeCount" || property == "pupaeLeftToTransform") && arguments.size() == 1 &&
		0 <= arguments[0].integer() && static_cast<uint>(arguments[0].integer()) < _rows) {
		uint row = arguments[0].integer();
		result = Core::ScriptValue(static_cast<int32>(property == "pupaeCount" ? _population[row] : _population[row] - _transformed[row]));
		return true;
	}
	if (property == "doorCount" && arguments.empty()) {
		result = Core::ScriptValue(static_cast<int32>(_rows));
		return true;
	}
	return Core::Trinket::get(property, arguments, result);
}

bool Incubator::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property == "pupaeCount" && arguments.size() == 1 && 0 <= arguments[0].integer() && 0 <= value.integer())
		return setPopulation(arguments[0].integer(), value.integer());
	return Core::Trinket::set(property, arguments, value);
}

bool Incubator::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (method == "transformPupae" && arguments.size() == 2 && 0 <= arguments[0].integer() && 0 <= arguments[1].integer()) {
		_time = callTime();
		transform(arguments[0].integer(), arguments[1].integer());
		return _valid;
	}
	if ((method == "pause" || method == "resume") && arguments.empty()) {
		pause(method == "pause", callTime());
		return true;
	}
	return Core::Trinket::play(method, arguments, result);
}

void Incubator::transform(uint row, uint count) {
	if (_rows <= row)
		return;
	count = MIN<uint>(count, _population[row] - _transformed[row]);
	const uint first = _transformed[row];
	_transformed[row] += count;
	for (uint i = first; i < _transformed[row]; i++) {
		Pupa &pupa = _pupae[row][i];
		remove(pupa.sprite);
		pupa.sprite = add(14051, z);
		if (!pupa.sprite)
			return;
		Common::Point position = kPupaPositions[_rows == 6 ? 1 : 0][row][i];
		pupa.sprite->x = position.x - pupa.sprite->width() / 2;
		pupa.sprite->y = position.y - pupa.sprite->height() / 2;
		pupa.sprite->pauseAnimation(false);
		pupa.state = 1;
	}
	if (count) {
		Batch batch = {row, first, _transformed[row], _time};
		_batches.push_back(batch);
	}
	_doors[row]->setFrame(1);
	_doors[row]->setRepeat(1);
	_doors[row]->pauseAnimation(false);
	_doorState[row] = 1;
}

void Incubator::update(uint32 time) {
	_time = time;
	if (_paused)
		return;
	for (uint row = 0; row < _rows; row++) {
		Shell::Sprite *door = _doors[row];
		if (_doorState[row] == 1 && !door->isRunning() && _transformed[row] < _population[row]) {
			door->reverseSequence();
			door->setFrame(1);
			door->setRepeat(1);
			door->pauseAnimation(false);
			_doorState[row] = 2;
		} else if (_doorState[row] == 2 && !door->isRunning()) {
			door->reverseSequence();
			door->setFrame(1);
			_doorState[row] = 0;
		}
	}
	for (uint i = 0; i < _batches.size();) {
		Batch &batch = _batches[i];
		if (500 < _time - batch.time && batch.first < batch.end) {
			Pupa &pupa = _pupae[batch.row][batch.first];
			pupa.state = 2;
			pupa.started = _time;
			pupa.start = Common::Point(pupa.sprite->x, pupa.sprite->y);
			pupa.phase = static_cast<float>(_vm.rnd().draw() * 6.2831855f * 0.000030518509);
			pupa.amplitude = static_cast<float>(_vm.rnd().draw() * 2.0f * 0.000030518509 + 8.0);
			pupa.frequency = static_cast<float>(_vm.rnd().draw() * 0.0040000002f * 0.000030518509 + 0.0080000004);
			int sign = (_vm.rnd().draw() % 2) ? 1 : -1;
			pupa.destination = Common::Point(800, pupa.start.y + sign * (_vm.rnd().draw() % 60));
			Common::Point delta = pupa.destination - pupa.start;
			pupa.distance = sqrt(static_cast<float>(delta.x * delta.x + delta.y * delta.y));
			_scene.setDepth(pupa.sprite, z + 3);
			batch.first += 1;
			batch.time = _time;
		}
		if (batch.first == batch.end)
			_batches.remove_at(i);
		else
			i += 1;
	}
	for (uint row = 0; row < _rows; row++) {
		for (uint i = 0; i < _transformed[row]; i++) {
			Pupa &pupa = _pupae[row][i];
			if (pupa.state != 2)
				continue;
			const uint elapsed = _time - pupa.started;
			float progress = MIN<float>(1, elapsed * 0.2f / pupa.distance);
			Common::Point delta = pupa.destination - pupa.start;
			pupa.sprite->x = pupa.start.x + static_cast<int>(delta.x * progress);
			pupa.sprite->y = pupa.start.y + static_cast<int>(delta.y * progress + sin(pupa.phase + elapsed * pupa.frequency) * pupa.amplitude);
			if (1 <= progress) {
				remove(pupa.sprite);
				pupa.sprite = nullptr;
				pupa.state = 3;
			}
		}
	}
}

void Incubator::pause(bool paused, uint32 time) {
	if (_paused == paused)
		return;
	_paused = paused;
	if (paused) {
		_pauseTime = time;
		for (uint i = 0; i < _owned.size(); i++) {
			if (_owned[i]->isRunning()) {
				_pausedSprites.push_back(_owned[i]);
				_owned[i]->pauseAnimation(true);
			}
		}
	} else {
		uint elapsed = time - _pauseTime;
		for (uint i = 0; i < _batches.size(); i++)
			_batches[i].time += elapsed;
		for (uint row = 0; row < _rows; row++)
			for (uint i = 0; i < _population[row]; i++)
				_pupae[row][i].started += elapsed;
		for (uint i = 0; i < _pausedSprites.size(); i++)
			_pausedSprites[i]->pauseAnimation(false);
		_pausedSprites.clear();
	}
}

Common::String PlanetariumMachine::debugAnswer(uint level) const {
	const PlanetariumRules &rules = this->rules();
	uint hour = (rules.targetHour() + 6) % 24;
	Common::String text = Common::String::format("Planetarium: level %u, target %02u:00", level, hour);
	if (level == 3)
		text += Common::String::format(", day %d", rules.targetDay() + 1);
	text += ". Wait for each piece before the next.\n";
	Common::Array<uint> answer;
	if (!rules.findCompleteAnswer(answer))
		return text + "No complete plan in this state; wait for advancing exhibits or restart if the supply is exhausted.\n";
	for (uint i = 0; i < answer.size(); i++) {
		uint index = answer[i];
		const PlanetariumRules::Piece &piece = rules.piece(index);
		Common::Rect from = pieceBounds(index), to = slotBounds(index);
		text += Common::String::format("Piece %u: %s %u, exhibit %u; drag (%d,%d) to (%d,%d)\n", index + 1,
									   piece.type == PlanetariumRules::kHour00 ? "hours" : "days", piece.value, piece.exhibit + 1,
									   from.left + 10, from.top + 10, to.left + to.width() / 2, to.top + to.height() / 2);
	}
	return text;
}

} // namespace Zoombini3
} // namespace Trinket
