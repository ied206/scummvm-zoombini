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

/** Script wrapper borrowing the active persisted World. */
class WorldPort : public Core::ScriptableObject {
public:
	class Host {
	public:
		virtual ~Host() {}
		virtual bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) = 0;
		virtual bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) = 0;
		virtual bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) = 0;
	};
	class Factory : public Core::ObjectRegistry::Factory {
	public:
		Factory(World &world, Host &host) : _world(world), _host(host) {}
		Core::ScriptableObject *create(const Core::ScriptArguments &) override { return new WorldPort(_world, _host); }

	private:
		World &_world;
		Host &_host;
	};
	WorldPort(World &world, Host &host) : _world(world), _host(&host) {}
	WorldPort(World &world, Location location) : _world(world), _location(location) {}
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	void setAutoLevelThresholds(uint successes, uint failures);
	static void resetWorksheet();
	static bool locationForName(const Common::String &name, Location &location);
	static Common::String nameForLocation(int location);

private:
	World &_world;
	Host *_host = nullptr;
	int _location = 0;
	static int32 _worksheet[7][4];
	static int worksheetColumn(const Common::String &property);
	bool locationArgument(const Core::ScriptArguments &arguments, uint index, Location &location) const;
	static bool activityForLocation(Location location, uint &activity);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
