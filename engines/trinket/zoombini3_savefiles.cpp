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

#include "trinket/zoombini3_savefiles.h"

namespace Trinket {
namespace Zoombini3 {

constexpr const char *SaveFiles::kGlobalPropertiesEnd;
constexpr const char *SaveFiles::kRegistryHeader;
constexpr const char *SaveFiles::kVersionName;
constexpr const char *SaveFiles::k12Name;
constexpr const char *SaveFiles::kGLOBALENDName;
constexpr const char *SaveFiles::kPracticeSuffix;

constexpr const char *SaveFiles::kSlotFileFormat;

bool SaveFiles::readRegistry(Common::SeekableReadStream *stream, Common::Array<Player> &players) {
	if (!stream) {
		_properties.clear();
		_macPadding.clear();
		return true;
	}
	const int64 start = stream->pos();
	const bool mac = _platform == Common::kPlatformMacintosh;
	Common::Array<Player> candidate;
	if (readRegistryFormat(*stream, candidate, mac)) {
		players = candidate;
		return true;
	}
	// Earlier ScummVM Mac targets also wrote the Windows registry layout.
	stream->clearErr();
	if (!stream->seek(start))
		return false;
	candidate.clear();
	if (!readRegistryFormat(*stream, candidate, !mac))
		return false;
	players = candidate;
	return true;
}

bool SaveFiles::readRegistryFormat(Common::SeekableReadStream &stream, Common::Array<Player> &players, bool mac) {
	Common::String value;
	Common::Array<Common::String> properties;
	Common::HashMap<uint32, uint16> padding;
	if (!line(stream, value) || !value.equals(kVersionName) || !line(stream, value) || !value.equals(k12Name))
		return false;
	while (true) {
		if (!line(stream, value))
			return false;
		if (value.equals(kGLOBALENDName))
			break;
		if (value.empty() || 8192 <= properties.size())
			return false;
		properties.push_back(value);
		if (!line(stream, value))
			return false;
		properties.push_back(value);
	}
	while (stream.pos() < stream.size()) {
		Player player;
		if (1024 <= players.size() || !line(stream, player.name) || player.name.empty() || !line(stream, value))
			return false;
		player.id = 0;
		for (uint i = 0; i < value.size(); i++) {
			if (value[i] < '0' || '9' < value[i] || 100000 < player.id)
				return false;
			player.id = player.id * 10 + value[i] - '0';
		}
		const uint recordSize = mac ? 16 : 32;
		if (!player.id || stream.size() - stream.pos() < recordSize)
			return false;
		if (mac) {
			player.count = stream.readUint32BE();
			player.current = stream.readUint32BE();
		} else {
			player.count = stream.readUint32LE();
			player.current = stream.readUint32LE();
		}
		if (!player.count || kSlotCount < player.count || !player.current || player.count < player.current)
			return false;
		player.locations.resize(kSlotCount);
		for (uint i = 0; i < kSlotCount; i++) {
			if (mac)
				player.locations[i] = stream.readByte();
			else
				player.locations[i] = stream.readSint32LE();
		}
		if (mac)
			padding[player.id] = stream.readUint16BE();
		for (uint i = 0; i < players.size(); i++) {
			if (players[i].id == player.id || players[i].name.equals(player.name))
				return false;
		}
		players.push_back(player);
	}
	if (stream.err())
		return false;
	_properties = properties;
	_macPadding = padding;
	return true;
}

bool SaveFiles::writeRegistry(Common::WriteStream &stream, const Common::Array<Player> &players) const {
	stream.writeString(kRegistryHeader);
	for (uint i = 0; i < _properties.size(); i++)
		stream.writeString(_properties[i] + "\n");
	stream.writeString(kGlobalPropertiesEnd);
	for (uint i = 0; i < players.size(); i++) {
		const Player &player = players[i];
		stream.writeString(player.name + "\n");
		stream.writeString(Common::String::format("%u\n", player.id));
		if (_platform == Common::kPlatformMacintosh) {
			stream.writeUint32BE(player.count);
			stream.writeUint32BE(player.current);
			for (uint j = 0; j < kSlotCount; j++)
				stream.writeByte(static_cast<byte>(player.locations[j]));
			const auto padding = _macPadding.find(player.id);
			stream.writeUint16BE(padding == _macPadding.end() ? 0 : padding->_value);
		} else {
			stream.writeUint32LE(player.count);
			stream.writeUint32LE(player.current);
			for (uint j = 0; j < kSlotCount; j++)
				stream.writeSint32LE(player.locations[j]);
		}
	}
	return !stream.err();
}

Common::String SaveFiles::primaryName(const Common::String &name) const {
	return isVisible(name) ? name : name.substr(0, name.size() - Common::String(kPracticeSuffix).size());
}

bool SaveFiles::belongsTo(const Common::String &candidate, const Common::String &owner) const {
	return candidate.equals(owner) || candidate.equals(owner + kPracticeSuffix);
}

Common::String SaveFiles::slotFileName(uint32 id, uint slot) const {
	return Common::String::format(kSlotFileFormat, id, slot);
}

} // namespace Zoombini3
} // namespace Trinket
