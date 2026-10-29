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

/**
 * Cache immutable source bytes for use by @ref Trinket::Shell::Scene.
 * Streams have independent read positions over the same immutable cached bytes.
 * Clearing the cache releases its references while already-open streams retain their data.
 */
class ResourceCache : public ResourceProvider {
public:
	/** Cumulative requests and provider reads for this cache instance. */
	struct Statistics {
		uint64 requests = 0; ///< Calls to open, including missing resources.
		uint64 hits = 0; ///< Requests served from retained bytes.
		uint64 reads = 0; ///< Attempts to open the underlying provider.
		uint64 failures = 0; ///< Failed opens or incomplete reads.
	};
	/** Create a byte cache using a provider that must remain valid while the cache is used. */
	explicit ResourceCache(const ResourceProvider &source) : _source(source) {}
	/** Return an independent stream sharing cached immutable bytes. */
	Common::SeekableReadStream *open(const Common::String &name) const override;
	/** Preserve the source provider's resource identity and search-path rules. */
	Common::String cacheKey(const Common::String &name) const override { return _source.cacheKey(name); }
	/** Check retained bytes first, otherwise ask the underlying provider. */
	bool contains(const Common::String &name) const override;
	/** Remove one cached payload without invalidating streams that still read it. */
	void release(const Common::String &name);
	/** Release retained bytes without invalidating open streams or resetting counters. */
	void clear();
	/** Return payload bytes retained by this cache, excluding released open streams. */
	uint64 bytes() const { return _bytes; }
	/** Return cumulative counters, including requests before clear(). */
	const Statistics &statistics() const { return _statistics; }

private:
	typedef Common::SharedPtr<Common::Array<byte>> Data;
	/** Resource provider queried when requested bytes are not cached. */
	const ResourceProvider &_source;
	/** Total payload size in the current entry table. */
	mutable uint64 _bytes = 0;
	/** Normalized names mapped to shared payload storage. */
	mutable Common::HashMap<Common::String, Data> _entries;
	/** Cumulative request counters, independent of retained entries. */
	mutable Statistics _statistics;
};

} // End of namespace OM
} // End of namespace Trinket

#endif
