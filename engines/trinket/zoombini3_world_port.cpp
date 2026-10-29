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

constexpr const char *WorldPort::kWsAutoLevelingAName;
constexpr const char *WorldPort::kWsAutoLevelingBName;
constexpr const char *WorldPort::kWsAutoLevelingXName;
constexpr const char *WorldPort::kWsAutoLevelingYName;
constexpr const char *WorldPort::kZ3A1Name;
constexpr const char *WorldPort::kZ3A2Name;
constexpr const char *WorldPort::kZ3A3Name;
constexpr const char *WorldPort::kZ3A4Name;
constexpr const char *WorldPort::kZ3A5Name;
constexpr const char *WorldPort::kZ3A6Name;
constexpr const char *WorldPort::kZ3A7Name;
constexpr const char *WorldPort::kZ3P1Name;
constexpr const char *WorldPort::kZ3M2Name;
constexpr const char *WorldPort::kZ3V3Name;
constexpr const char *WorldPort::kPropertyCurrentLocation;
constexpr const char *WorldPort::kPropertySeenMovie;
constexpr const char *WorldPort::kPropertyTokenAttribute;
constexpr const char *WorldPort::kPropertyNextToken;
constexpr const char *WorldPort::kPropertyTokenCount;
constexpr const char *WorldPort::kPropertyMinTokens;
constexpr const char *WorldPort::kPropertyVisitedCount;
constexpr const char *WorldPort::kPropertyLocationType;
constexpr const char *WorldPort::kCurrentLevelKey;
constexpr const char *WorldPort::kPropertyCurrentDataset;
constexpr const char *WorldPort::kMethodMoveToken;
constexpr const char *WorldPort::kMethodAddTokensPlayed;
constexpr const char *WorldPort::kMethodAddTokensPassed;
constexpr const char *WorldPort::kMethodCorrectGuess;
constexpr const char *WorldPort::kMethodAutoLevel;

int32 WorldPort::_worksheet[7][4];

void WorldPort::resetWorksheet() {
	static constexpr int32 defaults[] = {
		5,
		6,
		4,
		10,
	};
	for (uint activity = 0; activity < ARRAYSIZE(_worksheet); activity++) {
		for (uint column = 0; column < ARRAYSIZE(defaults); column++)
			_worksheet[activity][column] = defaults[column];
	}
}

int WorldPort::worksheetColumn(const Common::String &property) {
	static constexpr const char *names[] = {
		kWsAutoLevelingAName,
		kWsAutoLevelingBName,
		kWsAutoLevelingXName,
		kWsAutoLevelingYName,
	};
	for (uint i = 0; i < ARRAYSIZE(names); i++) {
		if (property.equals(names[i]))
			return i;
	}
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
	static constexpr const char *names[] = {
		kZ3A1Name,
		kZ3A2Name,
		kZ3A3Name,
		kZ3A4Name,
		kZ3A5Name,
		kZ3A6Name,
		kZ3A7Name,
		kZ3P1Name,
		kZ3M2Name,
		kZ3V3Name,
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
		if (name.equals(names[i])) {
			location = locations[i];
			return true;
		}
	}
	return false;
}

Common::String WorldPort::nameForLocation(int location) {
	static constexpr const char *names[] = {
		kZ3A1Name,
		kZ3A2Name,
		kZ3A3Name,
		kZ3A4Name,
		kZ3A5Name,
		kZ3A6Name,
		kZ3A7Name,
		kZ3P1Name,
		kZ3M2Name,
		kZ3V3Name,
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
	for (uint i = 0; i < ARRAYSIZE(locations); i++) {
		if (location == locations[i])
			return names[i];
	}
	return Common::String();
}

bool WorldPort::activityForLocation(Location location, uint &activity) {
	if (location < kCatapult01 || kBarn07 < location)
		return false;
	activity = location - kCatapult01;
	return true;
}

bool WorldPort::locationArgument(const Core::ScriptArguments &sargs, uint index, Location &location) const {
	if (sargs.size() <= index || sargs[index].toText().empty()) {
		location = static_cast<Location>(_location ? _location : _world.currentLocation());
		return true;
	}
	return locationForName(sargs[index].toText(), location);
}

bool WorldPort::get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	Location location;
	uint activity;
	if (_host && _host->get(property, sargs, result))
		return true;
	if (property.equals(kPropertyCurrentLocation) && sargs.empty()) {
		result = Core::ScriptValue(nameForLocation(_world.currentLocation()));
		return true;
	}
	if (property.equals(kPropertySeenMovie)) {
		if (1 < sargs.size() || (!sargs.empty() && (sargs[0].toInt32() < 0 || 10 <= sargs[0].toInt32())))
			return false;
		result = Core::ScriptValue(_world.seenMovie(sargs.empty() ? 0 : sargs[0].toInt32()));
		return true;
	}
	if (property.equals(kPropertyTokenAttribute)) {
		if (sargs.size() != 2 || sargs[0].toInt32() < 0 || World::kTokenCount <= static_cast<uint>(sargs[0].toInt32()) ||
			sargs[1].toInt32() < 0 || 6 <= sargs[1].toInt32())
			return false;
		result = Core::ScriptValue(_world.token(sargs[0].toInt32()).attributes[sargs[1].toInt32()]);
		return true;
	}
	if (property.equals(kPropertyNextToken)) {
		if (sargs.empty() || !locationArgument(sargs, 1, location))
			return false;
		result = Core::ScriptValue(_world.nextToken(sargs[0].toInt32(), location));
		return true;
	}
	if (!locationArgument(sargs, 0, location))
		return false;
	int column = worksheetColumn(property);
	if (0 <= column) {
		if (!activityForLocation(location, activity))
			return false;
		result = Core::ScriptValue(_worksheet[activity][column]);
	} else if (property.equals(kPropertyTokenCount))
		result = Core::ScriptValue(_world.tokenCount(location));
	else if (property.equals(kPropertyMinTokens))
		result = Core::ScriptValue(_world.minTokens(location));
	else if (property.equals(kPropertyVisitedCount))
		result = Core::ScriptValue(_world.visitCount(location));
	else if (property.equals(kPropertyLocationType)) {
		int type = 3;
		if (kCatapult01 <= location && location <= kBarn07)
			type = 0;
		else if (kRecruitment11 <= location && location <= kIsland13)
			type = 1;
		else if (location == 9)
			type = 2;
		result = Core::ScriptValue(type);
	} else if (property.equals(kCurrentLevelKey) || property.equals(kPropertyCurrentDataset)) {
		if (!activityForLocation(location, activity))
			return false;
		int level = _world.level(activity);
		if (1 < sargs.size() && sargs[1].toInt32() != -1)
			level = sargs[1].toInt32();
		result = Core::ScriptValue(property.equals(kCurrentLevelKey) ? _world.level(activity) : _world.dataset(activity, level));
	} else if (sargs.empty()) {
		Common::String text;
		if (!_world.stringProperty(property, text)) {
			if (!_host) {
				result = Core::ScriptValue(0);
				return true;
			}
			return ScriptableObject::get(property, sargs, result);
		}
		result = Core::ScriptValue::fromText(text);
	} else {
		return ScriptableObject::get(property, sargs, result);
	}
	return true;
}

bool WorldPort::set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) {
	Location location;
	uint activity;
	if (_host && _host->set(property, sargs, value))
		return true;
	if (property.equals(kPropertyCurrentLocation) && sargs.empty()) {
		if (!locationForName(value.toText(), location))
			return false;
		_world.setLocation(location);
		return true;
	}
	if (property.equals(kPropertySeenMovie)) {
		if (1 < sargs.size() || (!sargs.empty() && sargs[0].toInt32() < 0))
			return false;
		return _world.setSeenMovie(sargs.empty() ? 0 : sargs[0].toInt32(), value.toInt32());
	}
	if (property.equals(kPropertyTokenAttribute)) {
		if (sargs.size() != 2 || sargs[0].toInt32() < 0 || sargs[1].toInt32() < 0)
			return false;
		return _world.setTokenAttribute(sargs[0].toInt32(), sargs[1].toInt32(), value.toInt32());
	}
	if (property.equals(kPropertyMinTokens) || property.equals(kPropertyTokenCount) || property.equals(kPropertyVisitedCount)) {
		if (1 < sargs.size() || !locationArgument(sargs, 0, location))
			return false;
		if (property.equals(kPropertyMinTokens))
			return _world.setMinTokens(location, value.toInt32());
		if (property.equals(kPropertyTokenCount))
			return _world.setTokenCount(location, value.toInt32());
		return _world.setVisitedCount(location, value.toInt32());
	}
	int column = worksheetColumn(property);
	if (0 <= column) {
		if (!locationArgument(sargs, 0, location) || !activityForLocation(location, activity))
			return false;
		_worksheet[activity][column] = value.toInt32();
		return true;
	}
	if (property.equals(kCurrentLevelKey) || property.equals(kPropertyCurrentDataset)) {
		if (!locationArgument(sargs, 0, location) || !activityForLocation(location, activity))
			return false;
		if (property.equals(kCurrentLevelKey))
			return _world.setLevel(activity, value.toInt32());
		int level = _world.level(activity);
		if (1 < sargs.size() && sargs[1].toInt32() != -1)
			level = sargs[1].toInt32();
		return _world.setDataset(activity, level, value.toInt32());
	}
	if (!sargs.empty())
		return ScriptableObject::set(property, sargs, value);
	_world.setStringProperty(property, value.toText());
	return true;
}

bool WorldPort::play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	Location location;
	uint activity;
	if (_host && _host->play(method, sargs, result))
		return true;
	if (!method.equals(kMethodMoveToken) && !method.equals(kMethodAddTokensPlayed) && !method.equals(kMethodAddTokensPassed) &&
		!method.equals(kMethodCorrectGuess) && !method.equals(kMethodAutoLevel))
		return ScriptableObject::play(method, sargs, result);
	if (method.equals(kMethodMoveToken)) {
		if (sargs.empty())
			return false;
		int destination = 0;
		if (1 < sargs.size()) {
			if (!locationForName(sargs[1].toText(), location))
				return false;
			destination = location;
		}
		return _world.moveToken(sargs[0].toInt32(), destination);
	}
	uint locationIndex = (method.equals(kMethodAddTokensPlayed) || method.equals(kMethodAddTokensPassed)) ? 1 : 0;
	if (!locationArgument(sargs, locationIndex, location))
		return false;
	if (!activityForLocation(location, activity))
		return true;
	if (method.equals(kMethodAddTokensPlayed) || method.equals(kMethodAddTokensPassed)) {
		if (sargs.empty() || sargs[0].toInt32() < 0)
			return false;
		if (method.equals(kMethodAddTokensPlayed))
			_world.addTokensPlayed(activity, sargs[0].toInt32());
		else
			_world.addTokensPassed(activity, sargs[0].toInt32());
		return true;
	}
	if (method.equals(kMethodCorrectGuess))
		return _world.correctGuess(activity);
	if (method.equals(kMethodAutoLevel)) {
		_world.autoLevel(activity, _worksheet[activity][0], _worksheet[activity][2]);
		return true;
	}
	return ScriptableObject::play(method, sargs, result);
}

} // namespace Zoombini3
} // namespace Trinket
