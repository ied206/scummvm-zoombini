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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "trinket/rr1cloud9/saves.h"

#include "common/ptr.h"
#include "trinket/rr1cloud9/game.h"

namespace Trinket {
namespace RR1Cloud9 {

constexpr const char *Saves::kLauncherRegistryFile;

Saves::Saves(Common::SaveFileManager &manager, const Common::String &target)
	: _manager(manager), _registryFile(target + kLauncherRegistryFile), _saveFiles(manager, target) {}

bool Saves::load() {
	return _saveFiles.load() && _registry.load(_manager, _registryFile);
}

bool Saves::read(Common::SeekableReadStream &stream, Common::Array<byte> &bytes) {
	if (stream.size() < 0 || kMaxFileSize < stream.size() || !stream.seek(0))
		return false;
	bytes.resize(stream.size());
	return (bytes.empty() || stream.read(bytes.data(), bytes.size()) == bytes.size()) && !stream.err();
}

bool Saves::write(const Common::String &filename, const Common::Array<byte> &bytes) const {
	Common::ScopedPtr<Common::OutSaveFile> stream(_manager.openForSaving(filename));
	if (!stream)
		return false;
	if (!bytes.empty())
		stream->write(bytes.data(), bytes.size());
	stream->finalize();
	return !stream->err();
}

bool Saves::begin(uint player) {
	if (players().size() <= player || !_saveFiles.checkpoint(player, 1, _checkpoint))
		return false;
	_launcherBefore.clear();
	_launcherExisted = !_manager.listSavefiles(_registryFile).empty();
	if (!_launcherExisted)
		return true;
	Common::ScopedPtr<Common::InSaveFile> stream(_manager.openForLoading(_registryFile));
	return stream && read(*stream, _launcherBefore);
}

bool Saves::finish(bool success) {
	if (success)
		success = _saveFiles.store() && _registry.store(_manager, _registryFile);
	if (!success) {
		_rollbackFailed = !_saveFiles.restore(_checkpoint);
		Common::ScopedPtr<Common::InSaveFile> current(_manager.openForLoading(_registryFile));
		Common::Array<byte> bytes;
		const bool unchanged = current && read(*current, bytes) && bytes == _launcherBefore;
		current.reset();
		if (_launcherExisted) {
			if (!unchanged && !write(_registryFile, _launcherBefore))
				_rollbackFailed = true;
		} else if (!_manager.listSavefiles(_registryFile).empty() && !_manager.removeSavefile(_registryFile)) {
			_rollbackFailed = true;
		}
		if (!load())
			_rollbackFailed = true;
	}
	return success;
}

bool Saves::validName(const Common::String &name) {
	if (name.empty() || 16 < name.size())
		return false;
	for (char character : name) {
		if (static_cast<byte>(character) < 32 || 126 < static_cast<byte>(character))
			return false;
	}
	return true;
}

bool Saves::rename(uint player, const Common::String &name) {
	_rollbackFailed = false;
	if (players().size() <= player || !validName(name))
		return false;
	const Common::String oldName = players()[player].name;
	if (name.equals(oldName))
		return true;
	for (const auto &record : players()) {
		if (record.name.equals(name))
			return false;
	}
	for (const auto &user : _registry.users) {
		if (user.name.equals(name))
			return false;
	}
	if (!begin(player))
		return false;
	_saveFiles.players()[player].name = name;
	// Direct sign-in can create a game player without a Learning Launcher identity.
	for (auto &user : _registry.users) {
		if (user.name.equals(oldName))
			user.name = name;
	}
	return finish(true);
}

bool Saves::remove(uint player) {
	_rollbackFailed = false;
	if (!begin(player))
		return false;
	const Common::String name = players()[player].name;
	if (!_saveFiles.remove(player))
		return finish(false);
	for (int i = _registry.users.size() - 1; 0 <= i; i--) {
		if (_registry.users[i].name.equals(name))
			_registry.users.remove_at(i);
	}
	return finish(true);
}

bool Saves::importSlot(uint player, Common::SeekableReadStream &stream) {
	_rollbackFailed = false;
	Common::Array<byte> bytes;
	World world;
	world.reset();
	int location = 1;
	Shell::Game::SoundSettings sound = {};
	bool arrivalPending = false;
	if (players().size() <= player || !read(stream, bytes) || !stream.seek(0) ||
		!Game::readSave(stream, location, sound, arrivalPending, world) || stream.pos() != stream.size() || !begin(player))
		return false;
	if (!write(_saveFiles.stateName(player, 1), bytes))
		return finish(false);
	_saveFiles.players()[player].locations[0] = location;
	return finish(true);
}

bool Saves::exportSlot(uint player, Common::WriteStream &stream) const {
	if (players().size() <= player)
		return false;
	Common::ScopedPtr<Common::InSaveFile> source(_manager.openForLoading(_saveFiles.stateName(player, 1)));
	Common::Array<byte> bytes;
	if (!source || !read(*source, bytes))
		return false;
	return (bytes.empty() || stream.write(bytes.data(), bytes.size()) == bytes.size()) && !stream.err();
}

bool Saves::importSave(const Common::String &name, Common::SeekableReadStream &stream) {
	_rollbackFailed = false;
	if (!validName(name) || 99 <= _registry.users.size() || 99 <= players().size() || INT32_MAX <= _registry.nextID)
		return false;
	for (const auto &user : _registry.users) {
		if (user.name.equals(name))
			return false;
	}
	const int player = _saveFiles.add(name);
	if (player < 0)
		return false;
	if (!_manager.listSavefiles(_saveFiles.stateName(player, 1)).empty()) {
		load();
		return false;
	}
	Launcher::Registry::User user;
	user.id = _registry.nextID;
	_registry.nextID += 1;
	user.name = name;
	_registry.users.push_back(user);
	if (importSlot(player, stream))
		return true;
	load();
	return false;
}

} // End of namespace RR1Cloud9
} // End of namespace Trinket
