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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TRINKET_LAUNCHER_VERSION_H
#define TRINKET_LAUNCHER_VERSION_H

#include "trinket/version.h"

namespace Trinket {
namespace Launcher {

/** Original installed TLC launcher builds, independent of game component versions. */
struct OriginalVersions {
	static constexpr RuntimeVersions kUSInstalled = {
		"Z3-v1.0US installed TLC launcher",
		{"ompp32x.tlc", "4.01d", {2002, 2, 5, 11, 58, 47}},
		{"TLCLLTrinketcore.dll", "1.00a", {2002, 2, 11, 18, 29, 12}},
		{"TLCLLShelldll.dll", nullptr, {}},
		{nullptr, nullptr, {}},
		{"TLCLauncher.exe", "1.012", {}}};
};

} // End of namespace Launcher
} // End of namespace Trinket

#endif
