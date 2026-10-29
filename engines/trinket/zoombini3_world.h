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
	/** Number of character token records retained by the world. */
	static constexpr uint kTokenCount = 240;
	/** Maximum number of characters in an active party. */
	static constexpr uint kPartyLimit = 12;
	struct Token {
		/** Current world location assigned to this character token. */
		int32 location;
		/** Character attribute codes retained in native save order. */
		int32 attributes[6];
	};
	struct Property {
		/** Property name in the world's string-value table. */
		Common::String name;
		/** Stored text representation of the property value. */
		Common::String value;
	};
	/** Initialize native world fields and game property defaults. */
	World();
	/** Reset serialized fields to their initial native values. */
	void resetNative();
	/** Clear visit and activity progress while retaining persistent configuration fields. */
	void resetGame();
	/** Return the activity's automatic-leveling enable value. */
	int autoEnabled(uint activity) const { return activity < 7 ? _activities[activity][0] : 0; }
	/** Set automatic leveling for a valid activity index. */
	bool setAutoEnabled(uint activity, int32 enabled);
	/** Reset native fields and the world's property table. */
	void reset();
	/** Reset the world and initialize a new practice session. */
	void resetPractice();
	/** Set initial locations, activity progress and audio defaults for a new session. */
	void initializeNewGame(bool practice);
	/** Apply game or practice initialization to the current world. */
	void initializeGame(bool practice);
	/** Decode native world state and its bounded property table from a save stream. */
	bool read(Common::SeekableReadStream &stream);
	/** Serialize native world state and its property table to a save stream. */
	bool write(Common::WriteStream &stream) const;
	/** Find the next token assigned to a location after the supplied token index. */
	int nextToken(int previous, int location) const;
	/** Count token records currently assigned to a location. */
	uint countTokens(int location) const;
	/** Return the maintained token counter for a scene or activity. */
	int32 tokenCount(int location) const;
	/** Assign a maintained token counter for a valid scene or activity. */
	bool setTokenCount(int location, int32 count);
	/** Return the minimum party size configured for a scene or activity. */
	int minTokens(int location) const;
	/** Set the minimum party size for a valid scene or activity. */
	bool setMinTokens(int location, int32 count);
	/** Recalculate party-size requirements from current activity progress. */
	void updateMinimumPartySizes();
	/** Return the highest difficulty recorded for an activity. */
	int highestLevel(uint activity) const;
	/** Return whether spoken help is disabled. */
	bool isHelpAudioOff() const;
	/** Enable or disable spoken help through the saved property table. */
	void setHelpAudioOff(bool off);
	/** Return the saved background-music volume. */
	int musicVolume() const { return integerProperty("backgroundMusicVolume"); }
	/** Return whether saved preferences disable background music. */
	bool isBackgroundMusicOff() const { return integerProperty("isBackgroundMusicOff") != 0; }
	/** Return the saved dialogue volume. */
	int dialogueVolume() const { return integerProperty("dialogVolume"); }
	/** Return the saved sound-effects volume. */
	int effectsVolume() const { return integerProperty("sfxVolume"); }
	/** Store the background-music volume preference. */
	void setMusicVolume(int volume) { setIntegerProperty("backgroundMusicVolume", volume); }
	/** Store the dialogue volume preference. */
	void setDialogueVolume(int volume) { setIntegerProperty("dialogVolume", volume); }
	/** Store the sound-effects volume preference. */
	void setEffectsVolume(int volume) { setIntegerProperty("sfxVolume", volume); }
	/** Return whether the ScummVM-specific saved mute preference is set. */
	bool isSoundMuted() const { return integerProperty("scummvmSoundMuted") != 0; }
	/** Store the ScummVM-specific mute preference. */
	void setSoundMuted(bool muted) { setIntegerProperty("scummvmSoundMuted", muted ? 1 : 0); }
	/** Create a recruited character token from four selected appearance traits. */
	bool recruit(const int traits[4]);
	/** Return the world's currently selected location. */
	int currentLocation() const { return _location; }
	/** Set the world's currently selected location. */
	void setLocation(Location location) { _location = location; }
	/** Return a character token's stored location and attributes. */
	const Token &token(uint index) const { return _tokens[index]; }
	/** Return the activity's current difficulty, or zero for an invalid index. */
	int level(uint activity) const { return activity < 7 ? _activities[activity][11] : 0; }
	/** Assign a supported difficulty to a valid activity. */
	bool setLevel(uint activity, uint level);
	/** Return the dataset selected for an activity and difficulty. */
	int dataset(uint activity, uint level) const;
	/** Store a dataset selection for a valid activity and difficulty. */
	bool setDataset(uint activity, uint level, int dataset);
	/** Move a valid character token to a destination location. */
	bool moveToken(uint token, int destination = 0);
	/** Assign a valid character token's selected attribute. */
	bool setTokenAttribute(uint token, uint attribute, int32 value);
	/** Add to the activity's played-character counter. */
	void addTokensPlayed(uint activity, uint count);
	/** Add to the activity's successful-character counter. */
	void addTokensPassed(uint activity, uint count);
	/** Record a correct activity guess and update its progress counters. */
	bool correctGuess(uint activity);
	/** Update automatic difficulty selection from success and failure counts. */
	void autoLevel(uint activity, uint successes, uint failures);
	/** Read a saved property as an integer, using zero when conversion fails. */
	int integerProperty(const char *name) const;
	/** Store an integer as a text property. */
	void setIntegerProperty(const char *name, int value);
	/** Read a named text property and report whether it exists. */
	bool stringProperty(const Common::String &name, Common::String &value) const;
	/** Insert or replace a named text property. */
	void setStringProperty(const Common::String &name, const Common::String &value);
	/** Return the recorded number of visits to a typed location. */
	uint visitedCount(Location location) const;
	/** Return the visit counter associated with a valid location code. */
	int32 visitCount(int location) const;
	/** Assign the visit counter for a valid location code. */
	bool setVisitedCount(int location, int32 count);
	/** Record a visit to the supplied world location. */
	void visit(Location location);
	/** Return the recorded seen-state value for a movie index. */
	int32 seenMovie(uint movie) const { return movie < ARRAYSIZE(_seenMovies) ? _seenMovies[movie] : 0; }
	/** Assign a valid movie's recorded seen-state value. */
	bool setSeenMovie(uint movie, int32 value);
	/** Mark a movie as seen and report whether its state changed. */
	bool markMovieSeen(uint movie) {
		return setSeenMovie(movie, 1);
	}

private:
	/** Format for script names or values: Z3A%uHighestLevel. */
	static constexpr const char *kZ3AUnsignedIndexHighestLevelFormat = "Z3A%uHighestLevel";
	/** Script identifier for is Background Music Off. */
	static constexpr const char *kIsBackgroundMusicOffName = "isBackgroundMusicOff";
	/** Script identifier for is Printer Off. */
	static constexpr const char *kIsPrinterOffName = "isPrinterOff";
	/** Script identifier for been Leveled. */
	static constexpr const char *kBeenLeveledName = "beenLeveled";
	/** Script identifier for are Background Animations Off. */
	static constexpr const char *kAreBackgroundAnimationsOffName = "areBackgroundAnimationsOff";
	/** Script identifier for is Closed Captioning On. */
	static constexpr const char *kIsClosedCaptioningOnName = "isClosedCaptioningOn";
	/** Script identifier for is Controller FXOff. */
	static constexpr const char *kIsControllerFXOffName = "isControllerFXOff";
	/** Script identifier for completed Game. */
	static constexpr const char *kCompletedGameName = "completedGame";
	/** Script identifier for z3 A1 Level1 DS1 Done. */
	static constexpr const char *kZ3A1Level1DS1DoneName = "Z3A1Level1DS1Done";
	/** Format for script names or values: Z3A1DS%uDone. */
	static constexpr const char *kZ3A1DSUnsignedIndexDoneFormat = "Z3A1DS%uDone";
	/** XML element or attribute name for pLAYER END. */
	static constexpr const char *kXMLPLAYEREND = "PLAYER_END";
	/** Script identifier for is Help Audio Off. */
	static constexpr const char *kIsHelpAudioOffName = "isHelpAudioOff";

	/** Current location written to native world saves. */
	int32 _location;
	/** Serialized global state slots, including the world's visit counter. */
	int32 _flags[2];
	/** Scene visit counts, minimum party sizes and token counters. */
	int32 _scenes[3][3];
	/** Per-activity native progress, difficulty, dataset and history fields. */
	int32 _activities[7][32];
	/** Seen-state values indexed by authored movie. */
	int32 _seenMovies[10];
	/** Character token locations and attribute records. */
	Token _tokens[kTokenCount];
	/** Additional native scalar retained with the serialized world state. */
	int32 _additional;
	/** Named text properties storing script and game preferences. */
	Common::Array<Property> _properties;
	/** Read a bounded property line, rejecting NUL, carriage returns and missing terminators. */
	static bool readLine(Common::SeekableReadStream &stream, Common::String &line);
	/** Read the fixed native world-state fields before the property table. */
	bool readState(Common::SeekableReadStream &stream);
	/** Supply missing audio preferences when importing older save properties. */
	void restoreLegacyAudioSettings();
	/** Read one packed history byte from an activity's integer fields. */
	static byte historyEntry(const int32 *activity, uint index);
	/** Replace one packed history byte without altering neighboring entries. */
	static void setHistoryEntry(int32 *activity, uint index, byte value);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
