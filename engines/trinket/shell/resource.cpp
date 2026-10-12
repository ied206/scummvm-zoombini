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

#include "trinket/shell/resource.h"

#include "common/endian.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/textconsole.h"

namespace Trinket {
namespace Shell {

bool IndexedResources::contains(uint32 offset, uint32 size) const {
	return offset <= _index.size() && size <= _index.size() - offset;
}

bool IndexedResources::fail(const char *message) {
	_error = message;
	return false;
}

Common::String IndexedResources::normalize(const Common::String &name) {
	Common::String result = name;
	result.replace('\\', '/');
	while (result.hasPrefix("/"))
		result.erase(0, 1);
	result.toLowercase();
	return result;
}

bool IndexedResources::readName(uint32 offset, Common::String &name) const {
	name.clear();
	while (contains(offset, 1)) {
		byte c = _index[offset];
		offset += 1;
		if (!c)
			return !name.empty() && name != "." && name != "..";
		if (c < 32 || c == '/' || c == '\\' || c == ':' || 255 <= name.size())
			return false;
		name += static_cast<char>(c);
	}
	return false;
}

bool IndexedResources::parseNode(uint32 offset, const Common::String &prefix, uint depth, bool bulkList) {
	if (32 <= depth || !contains(offset, 4))
		return fail("Invalid INX directory");
	if (_visitedNodes.contains(offset))
		return fail("Repeated INX directory");
	_visitedNodes[offset] = true;
	uint32 count = READ_LE_UINT32(&_index[offset]);
	if (count > (_index.size() - offset - 4) / 4)
		return fail("Truncated INX directory table");
	for (uint32 i = 0; i < count; i++) {
		uint32 record = READ_LE_UINT32(&_index[offset + 4 + i * 4]);
		if (!contains(record, 6))
			return fail("Invalid INX record");
		bool leaf = (_index[record] & 0x80) != 0;
		uint32 nameOffset = !leaf || (_index[record + 1] & 4) ? 6 : 18;
		Common::String name;
		if (!contains(record, nameOffset) || !readName(record + nameOffset, name))
			return fail("Invalid INX name");
		if (!leaf) {
			if (bulkList)
				return fail("Nested INX bulk list");
			bool isBulkList = depth == 0 && name == "*bulkFiles";
			if (!parseNode(READ_LE_UINT32(&_index[record + 2]), prefix + name + "/", depth + 1, isBulkList))
				return false;
		} else {
			if (!contains(record, 18))
				return fail("Truncated INX resource record");
			if (bulkList) {
				Bulk bulk;
				bulk.name = name;
				bulk.checksum = READ_LE_UINT32(&_index[record + 4]);
				uint id = READ_LE_UINT16(&_index[record + 2]);
				if (_bulks.size() <= id)
					_bulks.resize(id + 1);
				if (!_bulks[id].name.empty())
					return fail("Duplicate INX bulk identifier");
				_bulks[id] = bulk;
			} else {
				Entry entry;
				entry.name = name;
				entry.bulk = READ_LE_UINT16(&_index[record + 2]);
				entry.offset = READ_LE_UINT32(&_index[record + 4]);
				entry.size = READ_LE_UINT32(&_index[record + 8]);
				Common::String key = normalize(prefix + name);
				if (_entries.contains(key))
					return fail("Duplicate INX resource");
				_entries[key] = entry;
			}
		}
	}
	return true;
}

bool IndexedResources::load(const Common::Path &indexPath) {
	_index.clear();
	_entries.clear();
	_bulks.clear();
	_visitedNodes.clear();
	_error.clear();
	Common::File file;
	if (!file.open(indexPath))
		return fail("Cannot open INX file");
	if (file.size() < 4 || file.size() > 16 * 1024 * 1024)
		return fail("Invalid INX size");
	_index.resize(file.size());
	if (file.read(&_index[0], _index.size()) != _index.size())
		return fail("Cannot read INX file");
	if (!parseNode(0, "", 0, false)) {
		_entries.clear();
		return false;
	}
	for (auto it = _entries.begin(); it != _entries.end(); ++it) {
		if (_bulks.size() <= it->_value.bulk || _bulks[it->_value.bulk].name.empty() || it->_value.size <= it->_value.name.size()) {
			_entries.clear();
			return fail("Invalid INX bulk reference");
		}
	}
	return true;
}

Common::SeekableReadStream *IndexedResources::open(const Common::String &name) const {
	Common::String key = normalize(name);
	auto it = _entries.find(key);
	if (it == _entries.end())
		return nullptr;
	const Entry &entry = it->_value;
	const Bulk &bulk = _bulks[entry.bulk];
	Common::File file;
	if (!file.open(Common::Path(bulk.name)) || file.size() < 4 || file.readUint32LE() != bulk.checksum)
		return nullptr;
	if (entry.offset < 4 || file.size() < entry.offset || file.size() - entry.offset < entry.size)
		return nullptr;
	file.seek(entry.offset);
	for (uint i = 0; i < entry.name.size(); i++) {
		if (file.readByte() != static_cast<byte>(entry.name[i]))
			return nullptr;
	}
	if (file.readByte() != 0 || file.err())
		return nullptr;
	uint32 size = entry.size - entry.name.size() - 1;
	if (!size)
		return new Common::MemoryReadStream(nullptr, 0);
	Common::ScopedPtr<Common::SeekableReadStream> payload(file.readStream(size));
	if (!payload || payload->size() != size || file.err())
		return nullptr;
	return payload.release();
}

} // namespace Shell
} // namespace Trinket
