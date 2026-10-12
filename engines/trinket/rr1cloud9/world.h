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

#ifndef TRINKET_RR1CLOUD9_WORLD_H
#define TRINKET_RR1CLOUD9_WORLD_H

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/stream.h"
#include "trinket/rr1cloud9/artwork.h"

namespace Trinket {
namespace RR1Cloud9 {

/** Persistent story properties; visual objects and pending speech remain scene-owned. */
class World {
public:
	int get(const Common::String &key) const { return _values.getValOrDefault(key, 0); }
	void set(const Common::String &key, int value) { _values[key] = value; }
	int visits(const Common::String &page) const { return get("visit:" + page); }
	void enter(const Common::String &page) { set("visit:" + page, visits(page) + 1); }
	void recordAssessment(const Common::String &activity, int level, int outcome, int up, int down, int streak);
	Artwork artwork;
	void reset() {
		artwork.clear();
		_values.clear();
		set("R1CN1State", 1);
		set("R1CG1State", 1);
	}
	bool load(Common::ReadStream &stream);
	bool save(Common::WriteStream &stream) const;

private:
	Common::HashMap<Common::String, int> _values;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
