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

#include "trinket/shell/savefiles.h"

#include "common/ptr.h"
#include "common/textconsole.h"

namespace Trinket {
namespace Shell {

bool SaveFiles::line(Common::SeekableReadStream &stream, Common::String &value) {
	value.clear();
	while (stream.pos() < stream.size() && value.size() < 1024) {
		byte c = stream.readByte();
		if (c == '\n')
			return !stream.err();
		if (c < 32)
			return false;
		value += static_cast<char>(c);
	}
	return false;
}

bool SaveFiles::load() {
	Common::ScopedPtr<Common::InSaveFile> stream(_manager.openForLoading(_prefix + "users.dat"));
	if (!stream && !_manager.listSavefiles(_prefix + "users.dat").empty())
		return false;
	Common::Array<Player> players;
	if (!readRegistry(stream.get(), players))
		return false;
	_players = players;
	return true;
}

bool SaveFiles::store() const {
	Common::ScopedPtr<Common::OutSaveFile> stream(_manager.openForSaving(_prefix + "users.dat"));
	if (!stream || !writeRegistry(*stream, _players))
		return false;
	stream->finalize();
	return !stream->err();
}

bool SaveFiles::storeLocation(uint index, int location, bool forceStore) {
	if (_players.size() <= index)
		return false;
	Player &player = _players[index];
	if (!player.current || player.count < player.current || player.locations.size() < player.count)
		return false;
	int previousLocation = player.locations[player.current - 1];
	// Ordinary saves update the registry only when the slot's displayed location changes.
	if (!forceStore && previousLocation == location)
		return true;
	player.locations[player.current - 1] = location;
	if (store())
		return true;
	player.locations[player.current - 1] = previousLocation;
	return false;
}

bool SaveFiles::readCheckpointFile(Checkpoint::File &file) const {
	file.bytes.clear();
	file.existed = false;
	Common::ScopedPtr<Common::InSaveFile> stream(_manager.openForLoading(file.name));
	if (!stream)
		return _manager.listSavefiles(file.name).empty();
	if (stream->size() < 0 || kMaxCheckpointFileSize < stream->size())
		return false;
	file.bytes.resize(stream->size());
	if ((!file.bytes.empty() && stream->read(file.bytes.data(), file.bytes.size()) != file.bytes.size()) || stream->err())
		return false;
	file.existed = true;
	return true;
}

bool SaveFiles::checkpoint(uint index, uint slot, Checkpoint &checkpoint) const {
	checkpoint._files.clear();
	if (_players.size() <= index || !slot || _players[index].locations.size() < slot)
		return false;
	Checkpoint::File state, registry;
	state.name = stateName(index, slot);
	registry.name = _prefix + "users.dat";
	for (Checkpoint::File *file : {&state, &registry}) {
		if (!readCheckpointFile(*file)) {
			warning("Trinket: could not checkpoint save file %s before game entry", file->name.c_str());
			return false;
		}
	}
	checkpoint._files.push_back(state);
	checkpoint._files.push_back(registry);
	return true;
}

bool SaveFiles::restore(const Checkpoint &checkpoint) const {
	bool success = true;
	for (const Checkpoint::File &file : checkpoint._files) {
		Checkpoint::File current;
		current.name = file.name;
		if (readCheckpointFile(current) && current.existed == file.existed && current.bytes == file.bytes)
			continue;
		bool restored = false;
		if (file.existed) {
			Common::ScopedPtr<Common::OutSaveFile> stream(_manager.openForSaving(file.name));
			if (stream) {
				if (!file.bytes.empty())
					stream->write(file.bytes.data(), file.bytes.size());
				stream->finalize();
				restored = !stream->err();
			}
		} else {
			restored = _manager.listSavefiles(file.name).empty() || _manager.removeSavefile(file.name);
		}
		if (!restored) {
			warning("Trinket: could not restore save file %s after failed game entry", file.name.c_str());
			success = false;
		}
	}
	return success;
}

int SaveFiles::add(const Common::String &name, int initialLocation) {
	if (name.empty() || 80 < name.size() || 1024 <= _players.size())
		return -1;
	for (uint i = 0; i < name.size(); i++) {
		if (static_cast<byte>(name[i]) < 32)
			return -1;
	}
	uint id = 1;
	for (uint i = 0; i < _players.size(); i++) {
		if (_players[i].name == name)
			return -1;
	}
	for (uint i = 0; i < _players.size();) {
		if (_players[i].id == id) {
			id += 1;
			i = 0;
		} else {
			i += 1;
		}
	}
	Player player;
	player.name = name;
	player.id = id;
	player.count = player.current = 1;
	player.locations.resize(maxSlots(), initialLocation);
	_players.push_back(player);
	return _players.size() - 1;
}

Common::String SaveFiles::stateName(uint index, uint slot) const {
	return _prefix + slotFileName(_players[index].id, slot);
}

bool SaveFiles::remove(uint index) {
	if (_players.size() <= index)
		return false;
	Common::Array<Player> previous = _players;
	Common::Array<Common::String> files;
	Common::String name = _players[index].name;
	for (int i = _players.size() - 1; 0 <= i; i--) {
		if (belongsTo(_players[i].name, name)) {
			for (uint slot = 1; slot <= _players[i].count; slot++)
				files.push_back(stateName(i, slot));
			_players.remove_at(i);
		}
	}
	if (!store()) {
		_players = previous;
		return false;
	}
	bool success = true;
	for (uint i = 0; i < files.size(); i++) {
		if (!_manager.listSavefiles(files[i]).empty() && !_manager.removeSavefile(files[i]))
			success = false;
	}
	return success;
}

} // namespace Shell
} // namespace Trinket
