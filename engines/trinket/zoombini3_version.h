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

#ifndef TRINKET_ZOOMBINI3_VERSION_H
#define TRINKET_ZOOMBINI3_VERSION_H

#include "trinket/version.h"

namespace Trinket {
namespace Zoombini3 {

/** Original Windows game component versions for the Z3-10 release family. */
struct OriginalVersions {
	static constexpr RuntimeVersions kWindowsGame = {
		"Z3-10 Windows game",
		{"ompp32x.dll", "4.1b", {2002, 9, 15, 6, 47, 28}},
		{"tcx.dll", "1.1b", {2002, 9, 15, 8, 38, 32}},
		{"trinket.dll", nullptr, {}},
		{"PuzzleEngine.dll", nullptr, {}},
		{"Zoombinis Island Odyssey.exe", "1.0", {}}};
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
