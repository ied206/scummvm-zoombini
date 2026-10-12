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
	explicit CachedReadStream(const Common::SharedPtr<Common::Array<byte>> &data) : Common::MemoryReadStream(data->empty() ? nullptr : &(*data)[0], data->size()), _data(data) {}

private:
	Common::SharedPtr<Common::Array<byte>> _data;
};

void ResourceCache::clear() {
	_entries.clear();
	_bytes = 0;
}

Common::SeekableReadStream *ResourceCache::open(const Common::String &name) const {
	Common::String key = name;
	key.toLowercase();
	key.replace('\\', '/');
	while (key.hasPrefix("/"))
		key.erase(0, 1);
	auto found = _entries.find(key);
	if (found != _entries.end())
		return new CachedReadStream(found->_value);
	Common::ScopedPtr<Common::SeekableReadStream> stream(_source.open(name));
	if (!stream || stream->size() < 0)
		return nullptr;
	// Array and stream reads use 32-bit sizes. Larger streams stay with the provider.
	if (0xFFFFFFFFULL < static_cast<uint64>(stream->size()))
		return stream.release();
	Data data(new Common::Array<byte>());
	data->resize(stream->size());
	uint32 size = data->size();
	if (size && (stream->read(&(*data)[0], size) != size || stream->err()))
		return nullptr;
	_bytes += size;
	_entries[key] = data;
	return new CachedReadStream(data);
}

} // namespace OM
} // namespace Trinket
