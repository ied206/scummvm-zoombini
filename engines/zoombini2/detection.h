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

/** Detector GUI option for color-keying solid-color help-sheet bitmaps. */
#define GAMEOPTION_HELP_PAGE_COLOR_KEYING GUIO_GAMEOPTIONS1

namespace Zoombini2 {

/** Release-family flags attached to one detected game description. */
enum GameFeatures {
	GF_NONE = 0, ///< No release-family feature flags.
	/**
	 * Zoombinis: Mountain Rescue - v1.0 family.
	 * - v1.0US
	 * - v1.0NL
	 * - v1.0HE
	 *
	 * Selects gameplay-tick reseeding when a puzzle calls @ref Zoombini2Engine::reseedRandomForV10.
	 */
	GF_Z2_V10 = (1 << 0),
	/**
	 * Zoombinis: Mountain Rescue - v1.1 family.
	 * - v1.1US
	 * - v1.1KR
	 * - v1.1SE
	 * - v1.1PL
	 *
	 * Puzzle generation continues the existing random sequence without reseeding.
	 * @ref Zoombini2Engine::reseedRandomForV10 therefore leaves the random state unchanged.
	 * Except of reseed difference, v1.1 has identical gameplay logics.
	 *
	 * v1.1 enables DirectDraw flip chain mode on some NT5 or later Windows.
	 * The exact version check has a quirk: (5 <= os.major && 0 < os.minor). Windows XP qualifies, though.
	 * Successful flip-chain creation engages vsync; otherwise presentation falls back to blitting-wo-waiting.
	 * ScummVM presentation remains backend-controlled, this flag does not affect any frame presentation.
	 */
	GF_Z2_V11 = (1 << 1),
	/** Help-sheet bitmaps use a solid background color instead of the frame gradient. */
	GF_Z2_SOLID_HELP_PAGES = (1 << 2),
	/** The help frame resource stores a solid gray full-screen canvas. */
	GF_Z2_SOLID_HELP_FRAME = (1 << 3),
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
