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

#ifndef ZOOMBINI2_DETECTION_H
#define ZOOMBINI2_DETECTION_H

#include "engines/advancedDetector.h"

namespace Zoombini2 {

/** Release-family flags attached to one detected game description. */
enum GameFeatures {
	GF_NONE = 0, ///< No release-family feature flags.
	/**
	 * Zoombinis: Mountain Rescue - v1.0 family.
	 * - v1.0US
	 * - v1.0NL
	 * - v1.0HE
	 */
	GF_Z2_V10 = (1 << 0),
	/**
	 * Zoombinis: Mountain Rescue - v1.1 family.
	 * - v1.1US
	 * - v1.1KR
	 * - v1.1SE
	 * - v1.1PL
	 *
	 * Gameplay is identical to the base v1.0 family, except RNG reseeding on every puzzle loading.
	 * No new puzzle rule or level change has been established.
	 */
	GF_Z2_V11 = (1 << 1),
};

/** Detection record combining ScummVM metadata with Z2 release-family flags. */
struct Zoombini2GameDescription {
	/** Standard advanced-detector description. */
	ADGameDescription desc;
	/** Bitwise combination of @ref GameFeatures values. */
	uint32 features;

	AD_GAME_DESCRIPTION_HELPERS(desc);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_DETECTION_H
