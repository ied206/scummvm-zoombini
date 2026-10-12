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

#include "trinket/zoombini3_saves.h"

#include "common/memstream.h"
#include "common/ptr.h"
#include "trinket/zoombini3_world.h"

namespace Trinket {
namespace Zoombini3 {

Saves::Saves(Common::SaveFileManager &manager, const Common::String &target) : _manager(manager), _prefix(target + ".trinket-"), _registryFile(_prefix + "launcher-users.xml"), _saveFiles(manager, target) {
}

bool Saves::load() {
	return _saveFiles.load() && _registry.load(_manager, _registryFile);
}

Common::StringArray Saves::files() const {
	Common::StringArray result;
	for (const Common::String &file : _manager.listSavefiles(_prefix + "*")) {
		if (file.hasPrefix(_prefix))
			result.push_back(file);
	}
	return result;
}

bool Saves::read(Common::SeekableReadStream &stream, Bytes &bytes) {
	if (stream.size() < 0 || 1024 * 1024 < stream.size() || !stream.seek(0))
		return false;
	bytes.resize(stream.size());
	return bytes.empty() || (stream.read(&bytes[0], bytes.size()) == bytes.size() && !stream.err());
}

bool Saves::write(const Common::String &file, const Bytes &bytes) {
	Common::ScopedPtr<Common::OutSaveFile> stream(_manager.openForSaving(file));
	if (!stream)
		return false;
	if (!bytes.empty())
		stream->write(&bytes[0], bytes.size());
	stream->finalize();
	return !stream->err();
}

bool Saves::begin() {
	_before.clear();
	_rollbackFailed = false;
	uint32 size = 0;
	for (const Common::String &file : files()) {
		Common::ScopedPtr<Common::InSaveFile> stream(_manager.openForLoading(file));
		Bytes bytes;
		if (!stream || !read(*stream, bytes) || 64 * 1024 * 1024 - size < bytes.size())
			return false;
		size += bytes.size();
		_before[file] = bytes;
	}
	return true;
}

bool Saves::finish(bool success) {
	if (success)
		success = _saveFiles.store() && _registry.store(_manager, _registryFile);
	if (!success) {
		// SaveFileManager has no multi-file transaction. Restore the pre-operation image on an I/O failure.
		for (const Common::String &file : files()) {
			if (!_before.contains(file) && !_manager.removeSavefile(file))
				_rollbackFailed = true;
		}
		for (auto it = _before.begin(); it != _before.end(); ++it) {
			Common::ScopedPtr<Common::InSaveFile> current(_manager.openForLoading(it->_key));
			Bytes bytes;
			bool unchanged = current && read(*current, bytes) && bytes == it->_value;
			current.reset();
			if (!unchanged && !write(it->_key, it->_value))
				_rollbackFailed = true;
		}
		load();
	}
	_before.clear();
	return success;
}

bool Saves::validName(const Common::String &name) {
	if (name.empty() || 21 < name.size() || name.hasSuffix("_Practice"))
		return false;
	// Keep names representable by the US launcher's single-byte text storage.
	for (uint i = 0; i < name.size(); i++) {
		byte c = name[i];
		if (c < 32 || 126 < c)
			return false;
	}
	return true;
}

bool Saves::validSlot(uint player, uint slot) const {
	return player < players().size() && 1 <= slot && slot <= players()[player].count;
}

bool Saves::rename(uint player, const Common::String &name, bool clone) {
	if (players().size() <= player || !validName(name) || players()[player].name.hasSuffix("_Practice"))
		return false;
	Common::String old = players()[player].name;
	for (const auto &record : players()) {
		if (record.name == name || record.name == name + "_Practice")
			return false;
	}
	for (const auto &user : _registry.users) {
		if (user.name == name)
			return false;
	}
	if ((clone && 99 <= _registry.users.size()) || !begin())
		return false;
	uint count = players().size();
	for (uint i = 0; i < count; i++) {
		if (players()[i].name != old && players()[i].name != old + "_Practice")
			continue;
		Common::String replacement = name;
		if (players()[i].name != old)
			replacement += "_Practice";
		if (!clone) {
			_saveFiles.players()[i].name = replacement;
			continue;
		}
		Shell::SaveFiles::Player source = players()[i];
		int destination = _saveFiles.add(replacement);
		if (destination < 0)
			return finish(false);
		source.id = players()[destination].id;
		source.name = replacement;
		_saveFiles.players()[destination] = source;
		for (uint slot = 1; slot <= source.count; slot++) {
			auto data = _before.find(stateName(i, slot));
			if (data == _before.end() || _before.contains(stateName(destination, slot)) || !write(stateName(destination, slot), data->_value))
				return finish(false);
		}
	}
	bool found = false;
	if (!clone) {
		for (auto &user : _registry.users) {
			if (user.name == old) {
				user.name = name;
				found = true;
			}
		}
	}
	if (!found) {
		if (99 <= _registry.users.size() || 2147483647u <= _registry.nextID)
			return finish(false);
		Launcher::Registry::User user;
		user.id = _registry.nextID;
		_registry.nextID += 1;
		user.name = name;
		_registry.users.push_back(user);
	}
	return finish(true);
}

bool Saves::remove(uint player) {
	if (players().size() <= player || players()[player].name.hasSuffix("_Practice") || !begin())
		return false;
	Common::String name = players()[player].name;
	if (!_saveFiles.remove(player))
		return finish(false);
	for (int i = _registry.users.size() - 1; 0 <= i; i--) {
		if (_registry.users[i].name == name)
			_registry.users.remove_at(i);
	}
	return finish(true);
}

bool Saves::select(uint player, uint slot) {
	if (!validSlot(player, slot) || !begin())
		return false;
	_saveFiles.players()[player].current = slot;
	return finish(true);
}

bool Saves::importSlot(uint player, uint slot, Common::SeekableReadStream &stream) {
	Bytes bytes;
	World world;
	if (!validSlot(player, slot) || !read(stream, bytes) || !stream.seek(0) || !world.read(stream) || stream.pos() != stream.size())
		return false;
	if (!begin())
		return false;
	if (!write(stateName(player, slot), bytes))
		return finish(false);
	_saveFiles.players()[player].locations[slot - 1] = world.currentLocation();
	return finish(true);
}

bool Saves::exportSlot(uint player, uint slot, Common::WriteStream &stream) {
	if (!validSlot(player, slot))
		return false;
	Common::ScopedPtr<Common::InSaveFile> source(_manager.openForLoading(stateName(player, slot)));
	Bytes bytes;
	if (!source || !read(*source, bytes))
		return false;
	return (bytes.empty() || stream.write(&bytes[0], bytes.size()) == bytes.size()) && !stream.err();
}

} // namespace Zoombini3
} // namespace Trinket
