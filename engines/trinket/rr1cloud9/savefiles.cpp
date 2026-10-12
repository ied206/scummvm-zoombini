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

#include "trinket/rr1cloud9/savefiles.h"

namespace Trinket {
namespace RR1Cloud9 {

bool SaveFiles::readRegistry(Common::SeekableReadStream *stream, Common::Array<Player> &players) {
	if (!stream)
		return true;
	Common::String value;
	if (!line(*stream, value))
		return false;
	// Import the first navigation prototype's registry without changing its checkpoint files.
	bool legacy = value == "Version";
	if (legacy) {
		if (!line(*stream, value) || value != "12" || !line(*stream, value) || value != "GLOBAL_END")
			return false;
	} else if ((value != "RR1Cloud9 saves" && value != "RR1Cloud9 profiles") || !line(*stream, value) || value != "1") {
		return false;
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
		if (!player.id || stream->size() - stream->pos() < (legacy ? 32 : 4))
			return false;
		player.count = player.current = 1;
		if (legacy && (stream->readUint32LE() != 1 || stream->readUint32LE() != 1))
			return false;
		player.locations.push_back(stream->readSint32LE());
		if (legacy)
			stream->skip(20);
		for (const Player &other : players) {
			if (other.id == player.id || other.name == player.name)
				return false;
		}
		players.push_back(player);
	}
	return !stream->err();
}

bool SaveFiles::writeRegistry(Common::WriteStream &stream, const Common::Array<Player> &players) const {
	stream.writeString("RR1Cloud9 saves\n1\n");
	for (const Player &player : players) {
		stream.writeString(player.name + "\n");
		stream.writeString(Common::String::format("%u\n", player.id));
		stream.writeSint32LE(player.locations[0]);
	}
	return !stream.err();
}

Common::String SaveFiles::slotFileName(uint32 id, uint slot) const {
	return Common::String::format("User%03u_%uGZ.dat", id, slot);
}

} // namespace RR1Cloud9
} // namespace Trinket
