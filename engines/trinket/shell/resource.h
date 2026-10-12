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

#ifndef TRINKET_SHELL_RESOURCE_H
#define TRINKET_SHELL_RESOURCE_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/path.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace Shell {

/** INX directory and BUL payload integration used by the Z3 runtime family. */
class IndexedResources : public OM::ResourceProvider {
public:
	bool load(const Common::Path &indexPath);
	Common::SeekableReadStream *open(const Common::String &name) const override;
	uint size() const { return _entries.size(); }
	const Common::String &getError() const { return _error; }

private:
	struct Entry {
		Common::String name;
		uint16 bulk;
		uint32 offset;
		uint32 size;
	};
	struct Bulk {
		Common::String name;
		uint32 checksum;
	};
	Common::Array<byte> _index;
	Common::Array<Bulk> _bulks;
	Common::HashMap<uint32, bool> _visitedNodes;
	Common::HashMap<Common::String, Entry> _entries;
	Common::String _error;
	bool parseNode(uint32 offset, const Common::String &prefix, uint depth, bool bulkList);
	bool readName(uint32 offset, Common::String &name) const;
	bool contains(uint32 offset, uint32 size) const;
	bool fail(const char *message);
	static Common::String normalize(const Common::String &name);
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
