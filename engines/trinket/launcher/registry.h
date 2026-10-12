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

#ifndef TRINKET_LAUNCHER_REGISTRY_H
#define TRINKET_LAUNCHER_REGISTRY_H

#include "common/array.h"
#include "common/savefile.h"

namespace Trinket {
namespace Launcher {

/** Launcher identity records; no graphics or game instance is needed to edit them. */
class Registry {
public:
	struct User {
		uint id;
		Common::String name;
	};
	Common::Array<User> users;
	uint nextID = 1;
	bool musicEnabled = true;
	bool soundEnabled = true;
	bool updatesAutomatic = true;
	bool load(Common::SaveFileManager &manager, const Common::String &file);
	bool store(Common::SaveFileManager &manager, const Common::String &file) const;
	static bool less(const User &left, const User &right) { return left.name < right.name; }

private:
	static bool parseID(const Common::String &value, uint &id);
};

} // End of namespace Launcher
} // End of namespace Trinket

#endif
