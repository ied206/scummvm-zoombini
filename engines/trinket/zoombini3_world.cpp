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

#include "trinket/zoombini3_world.h"

namespace Trinket {
namespace Zoombini3 {

constexpr uint World::kTokenCount;
constexpr uint World::kPartyLimit;

World::World() {
	reset();
}

void World::reset() {
	resetNative();
	initializeNewGame(false);
}

void World::resetNative() {
	_location = kRecruitment11;
	memset(_flags, 0, sizeof(_flags));
	// Native scene limits/counts are assigned later by the new-game script.
	// Keep their pre-script representation deterministic instead of heap contents.
	memset(_scenes, 0, sizeof(_scenes));
	memset(_activities, 0, sizeof(_activities));
	memset(_seenMovies, 0, sizeof(_seenMovies));
	_additional = 0;
	_properties.clear();
	for (uint i = 0; i < 7; i++) {
		_activities[i][0] = 1;
		_activities[i][7] = _activities[i][8] = _activities[i][9] = 1;
		_activities[i][11] = 1;
		_activities[i][27] = 1;
		Property property;
		property.name = Common::String::format("Z3A%uHighestLevel", i + 1);
		property.value = "0";
		_properties.push_back(property);
	}
	setMusicVolume(0);
	setDialogueVolume(0);
	setEffectsVolume(0);
	setIntegerProperty("isBackgroundMusicOff", 0);
	setIntegerProperty("isPrinterOff", 0);
	setIntegerProperty("beenLeveled", 0);
	setIntegerProperty("areBackgroundAnimationsOff", 0);
	setIntegerProperty("isClosedCaptioningOn", 0);
	setIntegerProperty("isControllerFXOff", 0);
	setIntegerProperty("completedGame", 0);
	setHelpAudioOff(false);
	setIntegerProperty("Z3A1Level1DS1Done", 0);
	for (uint i = 1; i <= 13; i++) {
		Common::String name = Common::String::format("Z3A1DS%uDone", i);
		setIntegerProperty(name.c_str(), 0);
	}
	for (uint i = 0; i < kTokenCount; i++) {
		_tokens[i].location = kRecruitment11;
		for (uint j = 0; j < 6; j++)
			_tokens[i].attributes[j] = -1;
	}
}

bool World::readLine(Common::SeekableReadStream &stream, Common::String &line) {
	line.clear();
	while (stream.pos() < stream.size() && line.size() < 1024) {
		byte c = stream.readByte();
		if (c == '\n')
			return !stream.err();
		if (!c || c == '\r')
			return false;
		line += static_cast<char>(c);
	}
	return false;
}

void World::resetGame() {
	memset(_flags, 0, sizeof(_flags));
	for (uint i = 0; i < ARRAYSIZE(_scenes); i++)
		_scenes[i][0] = 0;
	for (uint i = 0; i < ARRAYSIZE(_activities); i++) {
		int32 *state = _activities[i];
		for (uint j = 1; j <= 5; j++)
			state[j] = 0;
		for (uint j = 12; j <= 14; j++)
			state[j] = 0;
		state[22] = state[23] = state[28] = 0;
	}
	_additional = 0;
	_location = kRecruitment11;
}

bool World::setAutoEnabled(uint activity, int32 enabled) {
	if (7 <= activity)
		return false;
	_activities[activity][0] = enabled;
	return true;
}

void World::resetPractice() {
	resetNative();
	initializeNewGame(true);
}

void World::initializeNewGame(bool practice) {
	_location = practice ? kMap12 : kRecruitment11;
	_scenes[0][1] = 1;
	_scenes[0][2] = practice ? 0 : kTokenCount;
	_scenes[1][1] = -1;
	_scenes[1][2] = 0;
	_scenes[2][1] = 1;
	_scenes[2][2] = 0;
	for (uint i = 0; i < 7; i++) {
		_activities[i][27] = 1;
		_activities[i][28] = practice ? kPartyLimit : 0;
		Common::String name = Common::String::format("Z3A%uHighestLevel", i + 1);
		setIntegerProperty(name.c_str(), practice ? 3 : 1);
		if (practice)
			setLevel(i, 1);
	}
	if (!practice) {
		_activities[3][27] = kPartyLimit;
		_activities[5][27] = kPartyLimit;
		_activities[6][27] = 2;
	}
	setMusicVolume(50);
	setDialogueVolume(100);
	setEffectsVolume(80);
	setIntegerProperty("Z3A1Level1DS1Done", 0);
	for (uint i = 1; i <= 13; i++) {
		Common::String name = Common::String::format("Z3A1DS%uDone", i);
		setIntegerProperty(name.c_str(), 0);
	}
}

void World::initializeGame(bool practice) {
	// Normal games restore each activity's highest reached level at game entry.
	// Practice retains the independently selected level stored in its world.
	if (!practice) {
		for (uint i = 0; i < 7; i++)
			setLevel(i, highestLevel(i));
	}
}

bool World::read(Common::SeekableReadStream &stream) {
	World candidate;
	if (!candidate.readState(stream))
		return false;
	*this = candidate;
	return true;
}

bool World::readState(Common::SeekableReadStream &stream) {
	if (stream.size() - stream.pos() < 7708)
		return false;
	_location = stream.readSint32LE();
	for (uint i = 0; i < 2; i++)
		_flags[i] = stream.readSint32LE();
	for (uint i = 0; i < 3; i++)
		for (uint j = 0; j < 3; j++)
			_scenes[i][j] = stream.readSint32LE();
	for (uint i = 0; i < 7; i++)
		for (uint j = 0; j < 32; j++)
			_activities[i][j] = stream.readSint32LE();
	for (uint i = 0; i < 10; i++)
		_seenMovies[i] = stream.readSint32LE();
	for (uint i = 0; i < kTokenCount; i++) {
		_tokens[i].location = stream.readSint32LE();
		for (uint j = 0; j < 6; j++)
			_tokens[i].attributes[j] = stream.readSint32LE();
	}
	_additional = stream.readSint32LE();
	_properties.clear();
	while (!stream.err()) {
		Property property;
		if (!readLine(stream, property.name))
			return false;
		if (property.name == "PLAYER_END") {
			restoreLegacyAudioSettings();
			return true;
		}
		if (property.name.empty() || !readLine(stream, property.value) || 4096 <= _properties.size())
			return false;
		_properties.push_back(property);
	}
	return false;
}

void World::restoreLegacyAudioSettings() {
	// Early ScummVM samples saved only these seven properties, omitting all audio settings.
	// Restore new-game volumes for that format without changing native defaults or an explicit mute.
	if (_properties.size() != 7)
		return;
	for (uint i = 0; i < 7; i++) {
		if (_properties[i].name != Common::String::format("Z3A%uHighestLevel", i + 1))
			return;
	}
	setMusicVolume(50);
	setDialogueVolume(100);
	setEffectsVolume(80);
}

bool World::write(Common::WriteStream &stream) const {
	stream.writeSint32LE(_location);
	for (uint i = 0; i < 2; i++)
		stream.writeSint32LE(_flags[i]);
	for (uint i = 0; i < 3; i++)
		for (uint j = 0; j < 3; j++)
			stream.writeSint32LE(_scenes[i][j]);
	for (uint i = 0; i < 7; i++)
		for (uint j = 0; j < 32; j++)
			stream.writeSint32LE(_activities[i][j]);
	for (uint i = 0; i < 10; i++)
		stream.writeSint32LE(_seenMovies[i]);
	for (uint i = 0; i < kTokenCount; i++) {
		stream.writeSint32LE(_tokens[i].location);
		for (uint j = 0; j < 6; j++)
			stream.writeSint32LE(_tokens[i].attributes[j]);
	}
	stream.writeSint32LE(_additional);
	for (uint i = 0; i < _properties.size(); i++) {
		stream.writeString(_properties[i].name + "\n");
		stream.writeString(_properties[i].value + "\n");
	}
	stream.writeString("PLAYER_END\n");
	return !stream.err();
}

int World::nextToken(int previous, int location) const {
	if (previous < -1 || static_cast<int>(kTokenCount) <= previous)
		return -1;
	for (uint i = previous + 1; i < kTokenCount; i++) {
		if (_tokens[i].location == location)
			return i;
	}
	return -1;
}

uint World::countTokens(int location) const {
	return MAX<int32>(0, tokenCount(location));
}

int32 World::tokenCount(int location) const {
	if (1 <= location && location <= 7)
		return _activities[location - 1][28];
	if (11 <= location && location <= 13)
		return _scenes[location - 11][2];
	return 0;
}

bool World::setTokenCount(int location, int32 count) {
	if (1 <= location && location <= 7)
		_activities[location - 1][28] = count;
	else if (11 <= location && location <= 13)
		_scenes[location - 11][2] = count;
	else
		return false;
	return true;
}

int World::minTokens(int location) const {
	if (1 <= location && location <= 7)
		return _activities[location - 1][27];
	if (11 <= location && location <= 13)
		return _scenes[location - 11][1];
	return 0;
}

bool World::setMinTokens(int location, int32 count) {
	if (1 <= location && location <= 7)
		_activities[location - 1][27] = count;
	else if (11 <= location && location <= 13)
		_scenes[location - 11][1] = count;
	else
		return false;
	return true;
}

void World::updateMinimumPartySizes() {
	uint available = countTokens(kRecruitment11);
	for (int location = kCatapult01; location <= kGreenhouse04; location++)
		available += countTokens(location);
	if (available < kPartyLimit)
		_activities[kGreenhouse04 - 1][27] = 1;
	for (int location = kGarden05; location <= kCorral06; location++)
		available += countTokens(location);
	if (available < kPartyLimit)
		_activities[kCorral06 - 1][27] = 1;
}

uint World::visitedCount(Location location) const {
	return MAX<int32>(0, visitCount(location));
}

int32 World::visitCount(int location) const {
	if (kRecruitment11 <= location && location <= kIsland13)
		return _scenes[location - kRecruitment11][0];
	if (kCatapult01 <= location && location <= kBarn07)
		return _activities[location - kCatapult01][23];
	if (location == 9)
		return _flags[0];
	return 0;
}

bool World::setVisitedCount(int location, int32 count) {
	if (kRecruitment11 <= location && location <= kIsland13)
		_scenes[location - kRecruitment11][0] = count;
	else if (kCatapult01 <= location && location <= kBarn07)
		_activities[location - kCatapult01][23] = count;
	else if (location == 9)
		_flags[0] = count;
	else
		return false;
	return true;
}

bool World::setSeenMovie(uint movie, int32 value) {
	if (ARRAYSIZE(_seenMovies) <= movie)
		return false;
	_seenMovies[movie] = value;
	return true;
}

void World::visit(Location location) {
	if (kRecruitment11 <= location && location <= kIsland13)
		_scenes[location - kRecruitment11][0] += 1;
	else if (kCatapult01 <= location && location <= kBarn07)
		_activities[location - kCatapult01][23] += 1;
}

bool World::isHelpAudioOff() const {
	return integerProperty("isHelpAudioOff") != 0;
}

int World::integerProperty(const char *name) const {
	Common::String value;
	return stringProperty(name, value) ? atoi(value.c_str()) : 0;
}

bool World::stringProperty(const Common::String &name, Common::String &value) const {
	// Restoring repeated property pairs leaves the last assignment in effect.
	for (uint i = _properties.size(); 0 < i; i--) {
		if (_properties[i - 1].name == name) {
			value = _properties[i - 1].value;
			return true;
		}
	}
	return false;
}

void World::setHelpAudioOff(bool off) {
	setIntegerProperty("isHelpAudioOff", off ? 1 : 0);
}

void World::setIntegerProperty(const char *name, int value) {
	setStringProperty(name, Common::String::format("%d", value));
}

void World::setStringProperty(const Common::String &name, const Common::String &value) {
	for (uint i = _properties.size(); 0 < i; i--) {
		if (_properties[i - 1].name == name) {
			_properties[i - 1].value = value;
			return;
		}
	}
	Property property;
	property.name = name;
	property.value = value;
	_properties.push_back(property);
}

int World::highestLevel(uint activity) const {
	Common::String name = Common::String::format("Z3A%uHighestLevel", activity + 1);
	for (uint i = 0; i < _properties.size(); i++) {
		if (_properties[i].name == name) {
			if (_properties[i].value == "3")
				return 3;
			if (_properties[i].value == "2")
				return 2;
		}
	}
	return 1;
}

bool World::recruit(const int traits[4]) {
	for (uint i = 0; i < 4; i++) {
		if (traits[i] < 0 || 4 < traits[i])
			return false;
	}
	int index = nextToken(-1, kRecruitment11);
	if (index < 0)
		return false;
	Token &token = _tokens[index];
	token.attributes[0] = 0;
	for (uint i = 0; i < 4; i++)
		token.attributes[i + 1] = traits[i];
	token.location = kCatapult01;
	_scenes[0][2] -= 1;
	_activities[0][28] += 1;
	return true;
}

bool World::setLevel(uint activity, uint level) {
	if (7 <= activity || level < 1 || 3 < level)
		return false;
	if (_activities[activity][11] != static_cast<int>(level)) {
		for (uint i = 1; i <= 5; i++)
			_activities[activity][i] = 0;
		_activities[activity][15] = 0;
	}
	_activities[activity][11] = level;
	return true;
}

int World::dataset(uint activity, uint level) const {
	return activity < 7 && 1 <= level && level <= 3 ? _activities[activity][6 + level] : 0;
}

bool World::setDataset(uint activity, uint level, int dataset) {
	if (7 <= activity || level < 1 || 3 < level)
		return false;
	_activities[activity][6 + level] = dataset;
	return true;
}

bool World::setTokenAttribute(uint token, uint attribute, int32 value) {
	if (kTokenCount <= token || ARRAYSIZE(_tokens[token].attributes) <= attribute)
		return false;
	_tokens[token].attributes[attribute] = value;
	return true;
}

bool World::moveToken(uint index, int destination) {
	if (kTokenCount <= index)
		return false;
	int source = _tokens[index].location;
	int next = 0;
	if (kCatapult01 <= source && source < kBarn07)
		next = source + 1;
	else if (source == kBarn07)
		next = kIsland13;
	else if (source == kRecruitment11)
		next = kCatapult01;
	if (destination && destination != next) {
		if (!(kCatapult01 <= destination && destination <= kBarn07) &&
			!(kRecruitment11 <= destination && destination <= kIsland13))
			return false;
		next = destination;
		// An explicit reroute clears traits throughout the world, including other parties.
		for (uint i = 0; i < kTokenCount; i++)
			for (uint j = 0; j < 6; j++)
				_tokens[i].attributes[j] = -1;
	}
	if (!next)
		return false;
	_tokens[index].location = next;
	if (kCatapult01 <= source && source <= kBarn07)
		_activities[source - kCatapult01][28] -= 1;
	else if (kRecruitment11 <= source && source <= kIsland13)
		_scenes[source - kRecruitment11][2] -= 1;
	if (kCatapult01 <= next && next <= kBarn07)
		_activities[next - kCatapult01][28] += 1;
	else
		_scenes[next - kRecruitment11][2] += 1;
	return true;
}

void World::addTokensPlayed(uint activity, uint count) {
	if (activity < 7) {
		_activities[activity][29] += count;
		_activities[activity][31] += 1;
	}
}

void World::addTokensPassed(uint activity, uint count) {
	if (activity < 7)
		_activities[activity][30] += count;
}

byte World::historyEntry(const int32 *activity, uint index) {
	return static_cast<uint32>(activity[1 + index / 4]) >> ((index % 4) * 8);
}

void World::setHistoryEntry(int32 *activity, uint index, byte value) {
	uint shift = (index % 4) * 8;
	uint32 word = activity[1 + index / 4];
	activity[1 + index / 4] = (word & ~(255U << shift)) | (static_cast<uint32>(value) << shift);
}

bool World::correctGuess(uint activity) {
	if (7 <= activity)
		return false;
	int32 *state = _activities[activity];
	if (state[11] < 1 || 3 < state[11] || state[15] < 0 || 20 < state[15])
		return false;
	state[6] += 1;
	state[10] += 1;
	state[15 + state[11]] += 1;
	state[18 + state[11]] += 1;
	if (state[15] == 20) {
		for (uint i = 0; i < 19; i++)
			setHistoryEntry(state, i, historyEntry(state, i + 1));
		// Full history retains its final entry and writes into the following count byte.
		setHistoryEntry(state, 20, 1);
	} else {
		setHistoryEntry(state, state[15], 1);
		state[15] += 1;
	}
	return true;
}

void World::autoLevel(uint activity, uint successes, uint failures) {
	if (7 <= activity || !successes || !failures)
		return;
	int32 *state = _activities[activity];
	if (state[11] < 1 || 3 < state[11] || state[15] < 0 || 20 < state[15])
		return;
	uint correct = 0, incorrect = 0;
	for (int i = state[15] - 1; 0 <= i; i--) {
		if (historyEntry(state, i)) {
			correct += 1;
			if (successes <= correct)
				break;
			incorrect = 0;
		} else {
			incorrect += 1;
			if (failures <= incorrect)
				break;
			correct = 0;
		}
	}
	if (successes <= correct || failures <= incorrect) {
		bool advance = successes <= correct;
		state[23 + state[11]] = advance ? 1 : 2;
		if (state[0]) {
			if (advance && state[11] < 3)
				state[11] += 1;
			else if (!advance && 1 < state[11])
				state[11] -= 1;
		}
		for (uint i = 1; i <= 5; i++)
			state[i] = 0;
		state[15] = 0;
	} else if (static_cast<int>(MIN(successes, failures)) <= state[15]) {
		state[23 + state[11]] = 3;
	}
}

} // namespace Zoombini3
} // namespace Trinket
