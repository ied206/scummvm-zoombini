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

#include "trinket/ompp/ne_resource.h"

#include "common/endian.h"
#include "common/fs.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/substream.h"

namespace Trinket {
namespace OM {

bool NEResourceLibrary::load(const Common::Path &path) {
	_resources.clear();
	_types.clear();
	_stream.reset(Common::FSNode(path).createReadStream());
	if (!_stream || _stream->size() < 64 || 64 * 1024 * 1024 < _stream->size() || !readTable(*_stream)) {
		_stream.reset();
		_resources.clear();
		_types.clear();
		return false;
	}
	return true;
}

bool NEResourceLibrary::readTable(Common::SeekableReadStream &stream) {
	// OM libraries use numeric IDs and four-character tags in the NE name table.
	// Game libraries may contain more than 256 resources of a single type.
	if (stream.readByte() != 'M' || stream.readByte() != 'Z')
		return false;
	stream.seek(60);
	uint32 ne = stream.readUint32LE();
	if (65535 < ne || stream.size() - 40 < ne)
		return false;
	stream.seek(ne);
	if (stream.readByte() != 'N' || stream.readByte() != 'E')
		return false;
	stream.seek(ne + 36);
	uint32 table = ne + stream.readUint16LE();
	if (stream.size() - 2 < table)
		return false;
	stream.seek(table);
	uint shift = stream.readUint16LE();
	if (16 < shift)
		return false;
	bool terminated = false;
	uint total = 0;
	while (stream.pos() + 2 <= stream.size()) {
		uint type = stream.readUint16LE();
		if (!type) {
			terminated = true;
			break;
		}
		if (!(type & 0x8000) || stream.size() - stream.pos() < 6)
			return false;
		uint count = stream.readUint16LE();
		stream.skip(4);
		if (65536 < total + count || stream.size() - stream.pos() < count * 12)
			return false;
		total += count;
		for (uint i = 0; i < count; i++) {
			uint32 offset = static_cast<uint32>(stream.readUint16LE()) << shift;
			uint32 size = static_cast<uint32>(stream.readUint16LE()) << shift;
			stream.skip(2);
			uint id = stream.readUint16LE();
			if (!(id & 0x8000) || stream.size() < offset || stream.size() - offset < size)
				return false;
			stream.skip(4);
			uint32 key = (static_cast<uint32>(type & 0x7fff) << 16) | (id & 0x7fff);
			if (_resources.contains(key))
				return false;
			_resources[key] = {offset, size};
			if ((type & 0x7fff) == 15) {
				int64 position = stream.pos();
				Common::SeekableSubReadStream names(&stream, offset, offset + size);
				if (!loadTags(names) || !stream.seek(position))
					return false;
			}
		}
	}
	return terminated && !stream.err() && !_types.empty();
}

bool NEResourceLibrary::loadTags(Common::SeekableReadStream &names) {
	if (names.size() < 2)
		return false;
	while (names.pos() + 2 <= names.size()) {
		uint length = names.readUint16LE();
		if (!length)
			break;
		if (length < 8 || names.size() - names.pos() < length - 2)
			return false;
		uint type = names.readUint16LE() & 0x7fff;
		names.readUint16LE();
		Common::String tag;
		for (uint j = 6; j < length; j++) {
			char c = names.readByte();
			if (c)
				tag += c;
		}
		if (tag.size() != 4 || (_types.contains(tag) && _types[tag] != type))
			return false;
		_types[tag] = type;
	}
	return !names.err();
}

Common::SeekableReadStream *NEResourceLibrary::open(const Common::String &tag, uint id) {
	if (!_types.contains(tag) || 32767 < id)
		return nullptr;
	uint type = _types[tag];
	auto entry = _resources.find((static_cast<uint32>(type) << 16) | id);
	if (entry == _resources.end() || !_stream || !_stream->seek(entry->_value.offset))
		return nullptr;
	if (!entry->_value.size)
		return new Common::MemoryReadStream(nullptr, 0);
	return _stream->readStream(entry->_value.size);
}

} // namespace OM
} // namespace Trinket
