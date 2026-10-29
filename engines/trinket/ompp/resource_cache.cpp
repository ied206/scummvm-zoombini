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

#include "trinket/ompp/resource_cache.h"

#include "common/memstream.h"

namespace Trinket {
namespace OM {

class CachedReadStream : public Common::MemoryReadStream {
public:
	/** Retain payload ownership while maintaining this stream's own read position. */
	explicit CachedReadStream(const Common::SharedPtr<Common::Array<byte>> &data) : Common::MemoryReadStream(data->empty() ? nullptr : &(*data)[0], data->size()), _data(data) {}

private:
	/** Immutable payload retained even after the cache releases its reference. */
	Common::SharedPtr<Common::Array<byte>> _data;
};

void ResourceCache::clear() {
	_entries.clear();
	_bytes = 0;
}

Common::SeekableReadStream *ResourceCache::open(const Common::String &name) const {
	_statistics.requests += 1;
	Common::String key = cacheKey(name);
	auto found = _entries.find(key);
	if (found != _entries.end()) {
		_statistics.hits += 1;
		return new CachedReadStream(found->_value);
	}
	_statistics.reads += 1;
	Common::ScopedPtr<Common::SeekableReadStream> stream(_source.open(name));
	if (!stream || stream->size() < 0) {
		_statistics.failures += 1;
		return nullptr;
	}
	// A provider may learn the selected common-library alias during the first open.
	key = cacheKey(name);
	found = _entries.find(key);
	if (found != _entries.end())
		return new CachedReadStream(found->_value);
	// Array and stream reads use 32-bit sizes. Larger streams stay with the provider.
	if (UINT32_MAX < static_cast<uint64>(stream->size()))
		return stream.release();
	Data data(new Common::Array<byte>());
	data->resize(stream->size());
	uint32 size = data->size();
	if (size && (stream->read(&(*data)[0], size) != size || stream->err())) {
		_statistics.failures += 1;
		return nullptr;
	}
	if (!key.empty()) {
		_bytes += size;
		_entries[key] = data;
	}
	return new CachedReadStream(data);
}

bool ResourceCache::contains(const Common::String &name) const {
	return _entries.contains(cacheKey(name)) || _source.contains(name);
}

void ResourceCache::release(const Common::String &name) {
	auto found = _entries.find(cacheKey(name));
	if (found == _entries.end())
		return;
	_bytes -= found->_value->size();
	_entries.erase(found);
}

} // namespace OM
} // namespace Trinket
