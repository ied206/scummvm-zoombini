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

#ifndef TRINKET_OMPP_RESOURCE_CACHE_H
#define TRINKET_OMPP_RESOURCE_CACHE_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "common/ptr.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace OM {

/** Scene-lifetime source bytes. Streams own their position and pin immutable data. */
class ResourceCache : public ResourceProvider {
public:
	explicit ResourceCache(const ResourceProvider &source) : _source(source) {}
	Common::SeekableReadStream *open(const Common::String &name) const override;
	void clear();
	uint64 bytes() const { return _bytes; }

private:
	typedef Common::SharedPtr<Common::Array<byte>> Data;
	const ResourceProvider &_source;
	mutable uint64 _bytes = 0;
	mutable Common::HashMap<Common::String, Data> _entries;
};

} // End of namespace OM
} // End of namespace Trinket

#endif
