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

#ifndef TRINKET_OMPP_NE_RESOURCE_H
#define TRINKET_OMPP_NE_RESOURCE_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/path.h"
#include "common/ptr.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace OM {

/** OM resource-only NE libraries map their own numeric types to four-character tags. */
class NEResourceLibrary {
public:
	NEResourceLibrary() = default;
	bool load(const Common::Path &path);
	Common::SeekableReadStream *open(const Common::String &tag, uint id);

private:
	NEResourceLibrary(const NEResourceLibrary &) = delete;
	NEResourceLibrary &operator=(const NEResourceLibrary &) = delete;
	struct Entry {
		uint32 offset, size;
	};
	Common::ScopedPtr<Common::SeekableReadStream> _stream;
	Common::HashMap<uint32, Entry> _resources;
	Common::HashMap<Common::String, uint> _types;
	bool readTable(Common::SeekableReadStream &stream);
	bool loadTags(Common::SeekableReadStream &stream);
};

} // End of namespace OM
} // End of namespace Trinket

#endif
