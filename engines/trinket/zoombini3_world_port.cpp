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

#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

int32 WorldPort::_worksheet[7][4];

void WorldPort::resetWorksheet() {
	static constexpr int32 defaults[] = {
		5,
		6,
		4,
		10,
	};
	for (uint activity = 0; activity < ARRAYSIZE(_worksheet); activity++)
		for (uint column = 0; column < ARRAYSIZE(defaults); column++)
			_worksheet[activity][column] = defaults[column];
}

int WorldPort::worksheetColumn(const Common::String &property) {
	static const char *const names[] = {
		"wsAutoLevelingA",
		"wsAutoLevelingB",
		"wsAutoLevelingX",
		"wsAutoLevelingY",
	};
	for (uint i = 0; i < ARRAYSIZE(names); i++)
		if (property == names[i])
			return i;
	return -1;
}

void WorldPort::setAutoLevelThresholds(uint successes, uint failures) {
	uint activity;
	Location location = static_cast<Location>(_location ? _location : _world.currentLocation());
	if (activityForLocation(location, activity)) {
		_worksheet[activity][0] = successes;
		_worksheet[activity][2] = failures;
	}
}

bool WorldPort::locationForName(const Common::String &name, Location &location) {
	static const char *const names[] = {
		"Z3A1",
		"Z3A2",
		"Z3A3",
		"Z3A4",
		"Z3A5",
		"Z3A6",
		"Z3A7",
		"Z3P1",
		"Z3M2",
		"Z3V3",
	};
	static constexpr Location locations[] = {
		kCatapult01,
		kGlyph02,
		kPlanetarium03,
		kGreenhouse04,
		kGarden05,
		kCorral06,
		kBarn07,
		kRecruitment11,
		kMap12,
		kIsland13,
	};
	for (uint i = 0; i < ARRAYSIZE(names); i++) {
		if (name == names[i]) {
			location = locations[i];
			return true;
		}
	}
	return false;
}

Common::String WorldPort::nameForLocation(int location) {
	static const char *const names[] = {
		"Z3A1",
		"Z3A2",
		"Z3A3",
		"Z3A4",
		"Z3A5",
		"Z3A6",
		"Z3A7",
		"Z3P1",
		"Z3M2",
		"Z3V3",
	};
	static constexpr int locations[] = {
		kCatapult01,
		kGlyph02,
		kPlanetarium03,
		kGreenhouse04,
		kGarden05,
		kCorral06,
		kBarn07,
		kRecruitment11,
		kMap12,
		kIsland13,
	};
	for (uint i = 0; i < ARRAYSIZE(locations); i++)
		if (location == locations[i])
			return names[i];
	return Common::String();
}

bool WorldPort::activityForLocation(Location location, uint &activity) {
	if (location < kCatapult01 || kBarn07 < location)
		return false;
	activity = location - kCatapult01;
	return true;
}

bool WorldPort::locationArgument(const Core::ScriptArguments &arguments, uint index, Location &location) const {
	if (arguments.size() <= index || arguments[index].text().empty()) {
		location = static_cast<Location>(_location ? _location : _world.currentLocation());
		return true;
	}
	return locationForName(arguments[index].text(), location);
}

bool WorldPort::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	Location location;
	uint activity;
	if (_host && _host->get(property, arguments, result))
		return true;
	if (property == "currentLocation" && arguments.empty()) {
		result = Core::ScriptValue(nameForLocation(_world.currentLocation()));
		return true;
	}
	if (property == "seenMovie") {
		if (1 < arguments.size() || (!arguments.empty() && (arguments[0].integer() < 0 || 10 <= arguments[0].integer())))
			return false;
		result = Core::ScriptValue(_world.seenMovie(arguments.empty() ? 0 : arguments[0].integer()));
		return true;
	}
	if (property == "tokenAttribute") {
		if (arguments.size() != 2 || arguments[0].integer() < 0 || World::kTokenCount <= static_cast<uint>(arguments[0].integer()) ||
			arguments[1].integer() < 0 || 6 <= arguments[1].integer())
			return false;
		result = Core::ScriptValue(_world.token(arguments[0].integer()).attributes[arguments[1].integer()]);
		return true;
	}
	if (property == "nextToken") {
		if (arguments.empty() || !locationArgument(arguments, 1, location))
			return false;
		result = Core::ScriptValue(_world.nextToken(arguments[0].integer(), location));
		return true;
	}
	if (!locationArgument(arguments, 0, location))
		return false;
	int column = worksheetColumn(property);
	if (0 <= column) {
		if (!activityForLocation(location, activity))
			return false;
		result = Core::ScriptValue(_worksheet[activity][column]);
	} else if (property == "tokenCount")
		result = Core::ScriptValue(_world.tokenCount(location));
	else if (property == "minTokens")
		result = Core::ScriptValue(_world.minTokens(location));
	else if (property == "visitedCount")
		result = Core::ScriptValue(_world.visitCount(location));
	else if (property == "locationType") {
		int type = 3;
		if (kCatapult01 <= location && location <= kBarn07)
			type = 0;
		else if (kRecruitment11 <= location && location <= kIsland13)
			type = 1;
		else if (location == 9)
			type = 2;
		result = Core::ScriptValue(type);
	} else if (property == "currentLevel" || property == "currentDataset") {
		if (!activityForLocation(location, activity))
			return false;
		int level = _world.level(activity);
		if (1 < arguments.size() && arguments[1].integer() != -1)
			level = arguments[1].integer();
		result = Core::ScriptValue(property == "currentLevel" ? _world.level(activity) : _world.dataset(activity, level));
	} else if (arguments.empty()) {
		Common::String text;
		if (!_world.stringProperty(property, text)) {
			if (!_host) {
				result = Core::ScriptValue(0);
				return true;
			}
			return ScriptableObject::get(property, arguments, result);
		}
		result = Core::ScriptValue::fromText(text);
	} else {
		return ScriptableObject::get(property, arguments, result);
	}
	return true;
}

bool WorldPort::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	Location location;
	uint activity;
	if (_host && _host->set(property, arguments, value))
		return true;
	if (property == "currentLocation" && arguments.empty()) {
		if (!locationForName(value.text(), location))
			return false;
		_world.setLocation(location);
		return true;
	}
	if (property == "seenMovie") {
		if (1 < arguments.size() || (!arguments.empty() && arguments[0].integer() < 0))
			return false;
		return _world.setSeenMovie(arguments.empty() ? 0 : arguments[0].integer(), value.integer());
	}
	if (property == "tokenAttribute") {
		if (arguments.size() != 2 || arguments[0].integer() < 0 || arguments[1].integer() < 0)
			return false;
		return _world.setTokenAttribute(arguments[0].integer(), arguments[1].integer(), value.integer());
	}
	if (property == "minTokens" || property == "tokenCount" || property == "visitedCount") {
		if (1 < arguments.size() || !locationArgument(arguments, 0, location))
			return false;
		if (property == "minTokens")
			return _world.setMinTokens(location, value.integer());
		if (property == "tokenCount")
			return _world.setTokenCount(location, value.integer());
		return _world.setVisitedCount(location, value.integer());
	}
	int column = worksheetColumn(property);
	if (0 <= column) {
		if (!locationArgument(arguments, 0, location) || !activityForLocation(location, activity))
			return false;
		_worksheet[activity][column] = value.integer();
		return true;
	}
	if (property == "currentLevel" || property == "currentDataset") {
		if (!locationArgument(arguments, 0, location) || !activityForLocation(location, activity))
			return false;
		if (property == "currentLevel")
			return _world.setLevel(activity, value.integer());
		int level = _world.level(activity);
		if (1 < arguments.size() && arguments[1].integer() != -1)
			level = arguments[1].integer();
		return _world.setDataset(activity, level, value.integer());
	}
	if (!arguments.empty())
		return ScriptableObject::set(property, arguments, value);
	_world.setStringProperty(property, value.text());
	return true;
}

bool WorldPort::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	Location location;
	uint activity;
	if (_host && _host->play(method, arguments, result))
		return true;
	if (method != "moveToken" && method != "addTokensPlayed" && method != "addTokensPassed" &&
		method != "correctGuess" && method != "autoLevel")
		return ScriptableObject::play(method, arguments, result);
	if (method == "moveToken") {
		if (arguments.empty())
			return false;
		int destination = 0;
		if (1 < arguments.size()) {
			if (!locationForName(arguments[1].text(), location))
				return false;
			destination = location;
		}
		return _world.moveToken(arguments[0].integer(), destination);
	}
	uint locationIndex = (method == "addTokensPlayed" || method == "addTokensPassed") ? 1 : 0;
	if (!locationArgument(arguments, locationIndex, location))
		return false;
	if (!activityForLocation(location, activity))
		return true;
	if (method == "addTokensPlayed" || method == "addTokensPassed") {
		if (arguments.empty() || arguments[0].integer() < 0)
			return false;
		if (method == "addTokensPlayed")
			_world.addTokensPlayed(activity, arguments[0].integer());
		else
			_world.addTokensPassed(activity, arguments[0].integer());
		return true;
	}
	if (method == "correctGuess")
		return _world.correctGuess(activity);
	if (method == "autoLevel") {
		_world.autoLevel(activity, _worksheet[activity][0], _worksheet[activity][2]);
		return true;
	}
	return ScriptableObject::play(method, arguments, result);
}

} // namespace Zoombini3
} // namespace Trinket
