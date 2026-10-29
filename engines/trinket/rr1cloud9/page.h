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

#ifndef TRINKET_RR1CLOUD9_PAGE_H
#define TRINKET_RR1CLOUD9_PAGE_H

#include "common/rect.h"
#include "trinket/core/script.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace RR1Cloud9 {

/** Authored scene presentation, exits and their hover cursor resources. */
class Page {
public:
	/** Script name for the CN1 page. */
	static constexpr const char *kPageCN1 = "r1cn1";
	/** Script name for the CN2 page. */
	static constexpr const char *kPageCN2 = "r1cn2";
	/** Script name for the CN3 page. */
	static constexpr const char *kPageCN3 = "r1cn3";
	/** Script name for the CN4 page. */
	static constexpr const char *kPageCN4 = "r1cn4";
	/** Script name for the CA1 page. */
	static constexpr const char *kPageCA1 = "r1ca1";
	/** Script name for the CA2 page. */
	static constexpr const char *kPageCA2 = "r1ca2";
	/** Script name for the CG1 page. */
	static constexpr const char *kPageCG1 = "r1cg1";
	/** Script name for the CG2 page. */
	static constexpr const char *kPageCG2 = "r1cg2";
	/** Script name for the LN2 page. */
	static constexpr const char *kPageLN2 = "r1ln2";
	/** Script name for the LG2 page. */
	static constexpr const char *kPageLG2 = "r1lg2";
	/** Script name for the LA1 page. */
	static constexpr const char *kPageLA1 = "r1la1";
	/** Script name for the LA2 page. */
	static constexpr const char *kPageLA2 = "r1la2";
	/** Script name for the LA3 page. */
	static constexpr const char *kPageLA3 = "r1la3";
	/** Script name for the IN1 page. */
	static constexpr const char *kPageIN1 = "r1in1";
	/** Script name for the IN4 page. */
	static constexpr const char *kPageIN4 = "r1in4";
	/** Script name for the IG1 page. */
	static constexpr const char *kPageIG1 = "r1ig1";
	/** Script name for the IA1 page. */
	static constexpr const char *kPageIA1 = "r1ia1";
	/** Script name for the IA2 page. */
	static constexpr const char *kPageIA2 = "r1ia2";
	/** Script name for the IA3 page. */
	static constexpr const char *kPageIA3 = "r1ia3";
	/** Script name for the RSS page. */
	static constexpr const char *kPageRSS = "r1rss";
	/** Script name for the MAIN page. */
	static constexpr const char *kPageMAIN = "r1main";

	struct Exit {
		/** Game-pixel rectangle that activates the page exit. */
		Common::Rect rect;
		/** Authored page name entered through this exit. */
		Common::String destination;
		/** Cursor resource ID displayed over the exit. */
		int cursor = 0;
	};
	/** Read a compiled page script and extract its authored constants and exit regions. */
	bool load(const OM::ResourceProvider &resources, const Common::String &name);
	/** Resolve an authored constant to text without executing the page script. */
	Common::String text(const Common::String &name) const;
	/** Resolve an authored numeric constant, returning zero when it is unavailable. */
	int number(const Common::String &name) const;
	/** Return the page's authored exit rectangles and destinations. */
	const Common::Array<Exit> &exits() const { return _exits; }
	/** Return the decoded compiled script used for constant lookup. */
	const Core::Script &script() const { return _script; }

private:
	/** Authored data key for Num Prev Locs. */
	static constexpr const char *kNumPrevLocsKey = "kNumPrevLocs";
	/** Authored data key for Previous Locations.%d. */
	static constexpr const char *kPreviousLocationsSignedIndexKey = "kPreviousLocations.%d";
	/** Authored data key for Num Nav Hotspots.%d. */
	static constexpr const char *kNumNavHotspotsSignedIndexKey = "kNumNavHotspots.%d";
	/** Format for script names or values: .%d.%d. */
	static constexpr const char *kSignedIndexSignedIndexFormat = ".%d.%d";
	/** Authored data key for Nav HSX1. */
	static constexpr const char *kNavHSX1Key = "kNavHSX1";
	/** Authored data key for Nav HSY1. */
	static constexpr const char *kNavHSY1Key = "kNavHSY1";
	/** Authored data key for Nav HSX2. */
	static constexpr const char *kNavHSX2Key = "kNavHSX2";
	/** Authored data key for Nav HSY2. */
	static constexpr const char *kNavHSY2Key = "kNavHSY2";
	/** Authored data key for Nav HSCursors.%d. */
	static constexpr const char *kNavHSCursorsSignedIndexKey = "kNavHSCursors.%d";

	/** Extension of compiled page scripts. */
	static constexpr const char *kScriptExtension = ".mps";

	/** Compiled page instructions and variables used as authored data. */
	Core::Script _script;
	/** Exit regions extracted from the page's constants. */
	Common::Array<Exit> _exits;
	/** Resolve a constant through bounded variable indirection. */
	const Core::Script::Variable *constant(const Common::String &name, uint depth = 0) const;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
