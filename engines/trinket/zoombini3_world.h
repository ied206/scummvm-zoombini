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

#ifndef TRINKET_ZOOMBINI3_WORLD_H
#define TRINKET_ZOOMBINI3_WORLD_H

#include "common/array.h"
#include "common/str.h"
#include "common/stream.h"

namespace Trinket {
namespace Zoombini3 {

enum Location {
	kCatapult01 = 1,
	kGlyph02 = 2,
	kPlanetarium03 = 3,
	kGreenhouse04 = 4,
	kGarden05 = 5,
	kCorral06 = 6,
	kBarn07 = 7,
	kRecruitment11 = 11,
	kMap12 = 12,
	kIsland13 = 13,
};

class World {
public:
	static constexpr uint kTokenCount = 240;
	static constexpr uint kPartyLimit = 12;
	struct Token {
		int32 location;
		int32 attributes[6];
	};
	struct Property {
		Common::String name, value;
	};
	World();
	void resetNative();
	void resetGame();
	int autoEnabled(uint activity) const { return activity < 7 ? _activities[activity][0] : 0; }
	bool setAutoEnabled(uint activity, int32 enabled);
	void reset();
	void resetPractice();
	void initializeNewGame(bool practice);
	void initializeGame(bool practice);
	bool read(Common::SeekableReadStream &stream);
	bool write(Common::WriteStream &stream) const;
	int nextToken(int previous, int location) const;
	uint countTokens(int location) const;
	int32 tokenCount(int location) const;
	bool setTokenCount(int location, int32 count);
	int minTokens(int location) const;
	bool setMinTokens(int location, int32 count);
	void updateMinimumPartySizes();
	int highestLevel(uint activity) const;
	bool isHelpAudioOff() const;
	void setHelpAudioOff(bool off);
	int musicVolume() const { return integerProperty("backgroundMusicVolume"); }
	bool isBackgroundMusicOff() const { return integerProperty("isBackgroundMusicOff") != 0; }
	int dialogueVolume() const { return integerProperty("dialogVolume"); }
	int effectsVolume() const { return integerProperty("sfxVolume"); }
	void setMusicVolume(int volume) { setIntegerProperty("backgroundMusicVolume", volume); }
	void setDialogueVolume(int volume) { setIntegerProperty("dialogVolume", volume); }
	void setEffectsVolume(int volume) { setIntegerProperty("sfxVolume", volume); }
	bool isSoundMuted() const { return integerProperty("scummvmSoundMuted") != 0; }
	void setSoundMuted(bool muted) { setIntegerProperty("scummvmSoundMuted", muted ? 1 : 0); }
	bool recruit(const int traits[4]);
	int currentLocation() const { return _location; }
	void setLocation(Location location) { _location = location; }
	const Token &token(uint index) const { return _tokens[index]; }
	int level(uint activity) const { return activity < 7 ? _activities[activity][11] : 0; }
	bool setLevel(uint activity, uint level);
	int dataset(uint activity, uint level) const;
	bool setDataset(uint activity, uint level, int dataset);
	bool moveToken(uint token, int destination = 0);
	bool setTokenAttribute(uint token, uint attribute, int32 value);
	void addTokensPlayed(uint activity, uint count);
	void addTokensPassed(uint activity, uint count);
	bool correctGuess(uint activity);
	void autoLevel(uint activity, uint successes, uint failures);
	int integerProperty(const char *name) const;
	void setIntegerProperty(const char *name, int value);
	bool stringProperty(const Common::String &name, Common::String &value) const;
	void setStringProperty(const Common::String &name, const Common::String &value);
	uint visitedCount(Location location) const;
	int32 visitCount(int location) const;
	bool setVisitedCount(int location, int32 count);
	void visit(Location location);
	int32 seenMovie(uint movie) const { return movie < ARRAYSIZE(_seenMovies) ? _seenMovies[movie] : 0; }
	bool setSeenMovie(uint movie, int32 value);
	bool markMovieSeen(uint movie) {
		return setSeenMovie(movie, 1);
	}

private:
	int32 _location;
	int32 _flags[2];
	int32 _scenes[3][3];
	int32 _activities[7][32];
	int32 _seenMovies[10];
	Token _tokens[kTokenCount];
	int32 _additional;
	Common::Array<Property> _properties;
	static bool readLine(Common::SeekableReadStream &stream, Common::String &line);
	bool readState(Common::SeekableReadStream &stream);
	void restoreLegacyAudioSettings();
	static byte historyEntry(const int32 *activity, uint index);
	static void setHistoryEntry(int32 *activity, uint index, byte value);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
