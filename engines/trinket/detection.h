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

#ifndef TRINKET_DETECTION_H
#define TRINKET_DETECTION_H

#include "engines/advancedDetector.h"

namespace Trinket {

enum TrinketGameFeatures {
	/**
	 * Zoombinis: Island Odyssey
	 * Runtime family: OM 4.1b and TC 1.1b.
	 */
	GF_Z3_10 = (1 << 0),
	/**
	 * Reader Rabbit 1st Grade: Capers on Cloud Nine!
	 * Runtime family: OM 4.0. TC is unknown.
	 */
	GF_RR1_CLOUD9 = (1 << 1),
};

struct TrinketGameDescription {
	ADGameDescription desc;
	uint32 features;

	AD_GAME_DESCRIPTION_HELPERS(desc);
};

} // End of namespace Trinket

#endif // TRINKET_DETECTION_H
