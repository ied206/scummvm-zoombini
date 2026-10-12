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

/**
 * Indexed access to resources packed in BUL containers.
 * The INX directory maps virtual paths to BUL identifiers, offsets and stored lengths.
 * Its *bulkFiles directory maps those identifiers to filenames and header checksum values.
 * @ref Trinket::Shell::IndexedResources::load keeps the index in memory; BUL payloads are read on demand.
 * Z3 looks up the full virtual filename; the INX entry selects the BUL, not the first path component.
 * RR1Cloud9 instead resolves library/type/ID tuples through @ref Trinket::RR1Cloud9::Resources.
 * Z3's loose MPS, XML and movie files use their own file loaders.
 */
class IndexedResources : public OM::ResourceProvider {
public:
	/** Read the complete INX tree and build lookup tables for resources and BUL files. */
	bool load(const Common::Path &indexPath);
	/**
	 * Return an owned stream containing only the requested resource payload.
	 * Validate the BUL header value, record bounds and stored filename before skipping the name prefix.
	 * Resource decoding and game-object creation belong to consumers such as @ref Trinket::Shell::Scene.
	 * @return A payload stream, or nullptr if lookup or validation fails.
	 */
	Common::SeekableReadStream *open(const Common::String &name) const override;
	uint size() const { return _entries.size(); }
	const Common::String &getError() const { return _error; }

private:
	struct Entry {
		Common::String name; ///< Leaf filename stored before the payload in the BUL.
		uint16 bulk;         ///< Index into the BUL table built from *bulkFiles.
		uint32 offset;       ///< Start of the stored filename within the BUL.
		uint32 size;         ///< Byte length of the filename, terminating zero and payload together.
	};
	struct Bulk {
		Common::String name; ///< BUL filename opened through the file search system.
		uint32 checksum;     ///< Expected value of the BUL's first four bytes.
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
