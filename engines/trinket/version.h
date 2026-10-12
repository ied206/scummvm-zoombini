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

#ifndef TRINKET_VERSION_H
#define TRINKET_VERSION_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Trinket {

/** Calendar build time with one-based months and no assumed time zone. A zero year means unknown. */
struct BuildDateTime {
	uint16 year;
	byte month, day, hour, minute, second;

	bool isKnown() const { return year != 0; }
	Common::String toString() const {
		if (!isKnown())
			return "unknown";
		return Common::String::format("%04u-%02u-%02u %02u:%02u:%02u",
									  static_cast<uint>(year), static_cast<uint>(month), static_cast<uint>(day),
									  static_cast<uint>(hour), static_cast<uint>(minute), static_cast<uint>(second));
	}
};

/** Original module identity. Null module means absent; null version means unknown.
 * Version labels retain the original spelling rather than imposing a numeric version scheme.
 * The build time is independent of the version and may be unknown for a known version.
 */
struct ComponentVersion {
	const char *module;
	const char *version;
	BuildDateTime builtAt;
};

/** Original reference builds for one application and release, supplied by its subengine.
 * These describe native binaries, not ScummVM's version or detected compatibility capabilities.
 */
struct RuntimeVersions {
	const char *label;
	ComponentVersion om, core, shell, puzzle, application;
};

} // End of namespace Trinket

#endif
