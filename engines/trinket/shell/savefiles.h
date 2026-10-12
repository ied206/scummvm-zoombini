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

#ifndef TRINKET_SHELL_SAVEFILES_H
#define TRINKET_SHELL_SAVEFILES_H

#include "common/array.h"
#include "common/savefile.h"

namespace Trinket {
namespace Shell {

/** Save storage transactions; registry codecs and player policies belong to each game. */
class SaveFiles {
public:
	struct Player {
		Common::String name;
		uint32 id, count, current;
		Common::Array<int32> locations;
	};

	/** Pre-entry images of the affected slot and registry, including absent files. */
	class Checkpoint {
	private:
		struct File {
			Common::String name;
			Common::Array<byte> bytes;
			bool existed = false;
		};
		Common::Array<File> _files;
		friend class SaveFiles;
	};
	SaveFiles(Common::SaveFileManager &manager, const Common::String &target) : _manager(manager), _prefix(target + ".trinket-") {}
	virtual ~SaveFiles() {}
	virtual uint maxSlots() const = 0;
	virtual bool isVisible(const Common::String &name) const { return true; }
	virtual Common::String primaryName(const Common::String &name) const { return name; }
	virtual Common::String bootName() const { return "Boot"; }
	bool load();
	bool store() const;
	bool storeLocation(uint index, int location, bool forceStore = false);
	bool checkpoint(uint index, uint slot, Checkpoint &checkpoint) const;
	bool restore(const Checkpoint &checkpoint) const;
	int add(const Common::String &name, int initialLocation = 0);
	bool remove(uint index);
	Common::String stateName(uint index, uint slot) const;
	Common::Array<Player> &players() { return _players; }
	const Common::Array<Player> &players() const { return _players; }

protected:
	virtual bool readRegistry(Common::SeekableReadStream *stream, Common::Array<Player> &players) = 0;
	virtual bool writeRegistry(Common::WriteStream &stream, const Common::Array<Player> &players) const = 0;
	virtual Common::String slotFileName(uint32 id, uint slot) const = 0;
	virtual bool belongsTo(const Common::String &candidate, const Common::String &owner) const { return candidate == owner; }
	static bool line(Common::SeekableReadStream &stream, Common::String &value);

private:
	Common::SaveFileManager &_manager;
	Common::String _prefix;
	Common::Array<Player> _players;
	// Allow the bounded property tables accepted by the registry and World readers.
	static constexpr uint32 kMaxCheckpointFileSize = 16 * 1024 * 1024;
	bool readCheckpointFile(Checkpoint::File &file) const;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
