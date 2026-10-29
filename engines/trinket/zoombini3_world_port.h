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

#ifndef TRINKET_ZOOMBINI3_WORLD_PORT_H
#define TRINKET_ZOOMBINI3_WORLD_PORT_H

#include "trinket/core/object.h"
#include "trinket/zoombini3_world.h"

namespace Trinket {
namespace Zoombini3 {

/** Expose saved world properties and tokens to MPS scripts. */
class WorldPort : public Core::ScriptableObject {
public:
	class Host {
	public:
		/** Allow destruction through the world-port extension interface. */
		virtual ~Host() {}
		/** Read a game-specific world property not handled by the shared port. */
		virtual bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) = 0;
		/** Assign a game-specific world property not handled by the shared port. */
		virtual bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) = 0;
		/** Dispatch a game-specific world method not handled by the shared port. */
		virtual bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) = 0;
	};
	class Factory : public Core::ObjectRegistry::Factory {
	public:
		/** Connect world-port creation to the saved world and game-specific property host. */
		Factory(World &world, Host &host) : _world(world), _host(host) {}
		/** Create a script-facing port for global world state. */
		Core::ScriptableObject *create(const Core::ScriptArguments &) override { return new WorldPort(_world, _host); }

	private:
		/** World state exposed by newly created ports. */
		World &_world;
		/** Session interface handling game-specific property and method extensions. */
		Host &_host;
	};
	/** Expose global world state or bind the port to a particular location. */
	WorldPort(World &world, Host &host) : _world(world), _host(&host) {}
	/** Expose global world state or bind the port to a particular location. */
	WorldPort(World &world, Location location) : _world(world), _location(location) {}
	/** Read world, activity, location, token or worksheet properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign supported world, activity, token or worksheet values. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch world progression, recruitment and scene-control script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Set success and failure thresholds used by automatic difficulty changes. */
	void setAutoLevelThresholds(uint successes, uint failures);
	/** Clear the shared per-activity worksheet counters. */
	static void resetWorksheet();
	/** Resolve an authored location name to its location code. */
	static bool locationForName(const Common::String &name, Location &location);
	/** Return the authored script name for a location code. */
	static Common::String nameForLocation(int location);

private:
	/** Script identifier for ws Auto Leveling A. */
	static constexpr const char *kWsAutoLevelingAName = "wsAutoLevelingA";
	/** Script identifier for ws Auto Leveling B. */
	static constexpr const char *kWsAutoLevelingBName = "wsAutoLevelingB";
	/** Script identifier for ws Auto Leveling X. */
	static constexpr const char *kWsAutoLevelingXName = "wsAutoLevelingX";
	/** Script identifier for ws Auto Leveling Y. */
	static constexpr const char *kWsAutoLevelingYName = "wsAutoLevelingY";
	/** Script identifier for z3 A1. */
	static constexpr const char *kZ3A1Name = "Z3A1";
	/** Script identifier for z3 A2. */
	static constexpr const char *kZ3A2Name = "Z3A2";
	/** Script identifier for z3 A3. */
	static constexpr const char *kZ3A3Name = "Z3A3";
	/** Script identifier for z3 A4. */
	static constexpr const char *kZ3A4Name = "Z3A4";
	/** Script identifier for z3 A5. */
	static constexpr const char *kZ3A5Name = "Z3A5";
	/** Script identifier for z3 A6. */
	static constexpr const char *kZ3A6Name = "Z3A6";
	/** Script identifier for z3 A7. */
	static constexpr const char *kZ3A7Name = "Z3A7";
	/** Script identifier for z3 P1. */
	static constexpr const char *kZ3P1Name = "Z3P1";
	/** Script identifier for z3 M2. */
	static constexpr const char *kZ3M2Name = "Z3M2";
	/** Script identifier for z3 V3. */
	static constexpr const char *kZ3V3Name = "Z3V3";
	/** Script property name for current Location. */
	static constexpr const char *kPropertyCurrentLocation = "currentLocation";
	/** Script property name for seen Movie. */
	static constexpr const char *kPropertySeenMovie = "seenMovie";
	/** Script property name for token Attribute. */
	static constexpr const char *kPropertyTokenAttribute = "tokenAttribute";
	/** Script property name for next Token. */
	static constexpr const char *kPropertyNextToken = "nextToken";
	/** Script property name for token Count. */
	static constexpr const char *kPropertyTokenCount = "tokenCount";
	/** Script property name for min Tokens. */
	static constexpr const char *kPropertyMinTokens = "minTokens";
	/** Script property name for visited Count. */
	static constexpr const char *kPropertyVisitedCount = "visitedCount";
	/** Script property name for location Type. */
	static constexpr const char *kPropertyLocationType = "locationType";
	/** World-state key for current Level. */
	static constexpr const char *kCurrentLevelKey = "currentLevel";
	/** Script property name for current Dataset. */
	static constexpr const char *kPropertyCurrentDataset = "currentDataset";
	/** Script method name for move Token. */
	static constexpr const char *kMethodMoveToken = "moveToken";
	/** Script method name for add Tokens Played. */
	static constexpr const char *kMethodAddTokensPlayed = "addTokensPlayed";
	/** Script method name for add Tokens Passed. */
	static constexpr const char *kMethodAddTokensPassed = "addTokensPassed";
	/** Script method name for correct Guess. */
	static constexpr const char *kMethodCorrectGuess = "correctGuess";
	/** Script method name for auto Level. */
	static constexpr const char *kMethodAutoLevel = "autoLevel";

	/** Saved world state exposed to scripts by this port. */
	World &_world;
	/** Optional session interface for game-specific property and method extensions. */
	Host *_host = nullptr;
	/** Location code used by a scene-specific port, or zero for the global port. */
	int _location = 0;
	/** Shared per-activity worksheet values indexed by activity and column. */
	static int32 _worksheet[7][4];
	/** Map a worksheet property name to its stored column index. */
	static int worksheetColumn(const Common::String &property);
	/** Resolve a location argument or use the port's configured location. */
	bool locationArgument(const Core::ScriptArguments &sargs, uint index, Location &location) const;
	/** Map an activity location code to its zero-based activity index. */
	static bool activityForLocation(Location location, uint &activity);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
