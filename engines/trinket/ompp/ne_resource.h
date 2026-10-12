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

/**
 * Read resource-only NE containers used by RR1Cloud9 and the TLC launcher.
 * Each library maps its own numeric types to four-character OM tags.
 * Its embedded table maps numeric type/ID pairs to file spans using the table's alignment shift.
 * Unlike @ref Trinket::Shell::IndexedResources, no separate INX or BUL filename prefix is involved.
 * @ref Trinket::OM::NEResourceLibrary::open returns stored bytes; the game's resource provider handles wrappers such as NC1.
 *
 * @note @ref Common::NEResources limits each type to 256 entries; RR1Cloud9's r1ca1.rsc has 484 LIPS entries.
 * NE name-table records contain a type-name string followed by a resource-name string, both null-terminated.
 * RR1Cloud9 stores tags such as ASEQ in the type-name field and leaves the resource-name field empty for numeric IDs.
 * The shared reader assumes an empty type-name field, skips its first byte, and treats the following string as an ID alias.
 * It therefore misreads these named types. The original OM loader uses Windows resource APIs to resolve them.
 * Both table readers are private and non-virtual, so inheritance cannot replace those incompatible steps.
 * This reader indexes the complete numeric table and maps the libraries' named types to their numeric ordinals.
 *
 * FIXME: Integrate this into @ref Common::NEResources once these incompatibilities are addressed.
 */
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
