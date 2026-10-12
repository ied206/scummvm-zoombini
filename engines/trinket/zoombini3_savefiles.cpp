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

bool SaveFiles::readRegistry(Common::SeekableReadStream *stream, Common::Array<Player> &players) {
	if (!stream) {
		_properties.clear();
		return true;
	}
	Common::String value;
	Common::Array<Common::String> properties;
	if (!line(*stream, value) || value != "Version" || !line(*stream, value) || value != "12")
		return false;
	while (true) {
		if (!line(*stream, value))
			return false;
		if (value == "GLOBAL_END")
			break;
		if (value.empty() || 8192 <= properties.size())
			return false;
		properties.push_back(value);
		if (!line(*stream, value))
			return false;
		properties.push_back(value);
	}
	while (stream->pos() < stream->size()) {
		Player player;
		if (1024 <= players.size() || !line(*stream, player.name) || player.name.empty() || !line(*stream, value))
			return false;
		player.id = 0;
		for (uint i = 0; i < value.size(); i++) {
			if (value[i] < '0' || '9' < value[i] || 100000 < player.id)
				return false;
			player.id = player.id * 10 + value[i] - '0';
		}
		if (!player.id || stream->size() - stream->pos() < 32)
			return false;
		player.count = stream->readUint32LE();
		player.current = stream->readUint32LE();
		if (!player.count || kSlotCount < player.count || !player.current || player.count < player.current)
			return false;
		player.locations.resize(kSlotCount);
		for (uint i = 0; i < kSlotCount; i++)
			player.locations[i] = stream->readSint32LE();
		for (uint i = 0; i < players.size(); i++) {
			if (players[i].id == player.id || players[i].name == player.name)
				return false;
		}
		players.push_back(player);
	}
	if (stream->err())
		return false;
	_properties = properties;
	return true;
}

bool SaveFiles::writeRegistry(Common::WriteStream &stream, const Common::Array<Player> &players) const {
	stream.writeString("Version\n12\n");
	for (uint i = 0; i < _properties.size(); i++)
		stream.writeString(_properties[i] + "\n");
	stream.writeString("GLOBAL_END\n");
	for (uint i = 0; i < players.size(); i++) {
		const Player &player = players[i];
		stream.writeString(player.name + "\n");
		stream.writeString(Common::String::format("%u\n", player.id));
		stream.writeUint32LE(player.count);
		stream.writeUint32LE(player.current);
		for (uint j = 0; j < kSlotCount; j++)
			stream.writeSint32LE(player.locations[j]);
	}
	return !stream.err();
}

Common::String SaveFiles::primaryName(const Common::String &name) const {
	return isVisible(name) ? name : name.substr(0, name.size() - Common::String("_Practice").size());
}

bool SaveFiles::belongsTo(const Common::String &candidate, const Common::String &owner) const {
	return candidate == owner || candidate == owner + "_Practice";
}

Common::String SaveFiles::slotFileName(uint32 id, uint slot) const {
	return Common::String::format("User%03u_%uGZ.dat", id, slot);
}

} // namespace Zoombini3
} // namespace Trinket
