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
		/** Player name stored in the game registry. */
		Common::String name;
		/** Persistent player identifier used in slot filenames. */
		uint32 id;
		/** Number of game slots recorded for this player. */
		uint32 count;
		/** One-based currently selected slot. */
		uint32 current;
		/** Saved location for each slot, indexed from zero. */
		Common::Array<int32> locations;
	};

	/** Pre-entry images of the affected slot and registry, including absent files. */
	class Checkpoint {
	private:
		struct File {
			/** Save-manager filename captured by this checkpoint. */
			Common::String name;
			/** File contents retained for rollback. */
			Common::Array<byte> bytes;
			/** Whether the file existed before the operation began. */
			bool existed = false;
		};
		/** Slot and registry file snapshots needed to roll back an operation. */
		Common::Array<File> _files;
		friend class SaveFiles;
	};
	/** Select the save manager and target-specific filename prefix. */
	SaveFiles(Common::SaveFileManager &manager, const Common::String &target) : _manager(manager), _prefix(target + ".trinket-") {}
	/** Allow destruction through the game-specific save-storage interface. */
	virtual ~SaveFiles() {}
	/** Return the maximum number of game slots supported for one player. */
	virtual uint maxSlots() const = 0;
	/** Return whether a registry player name should appear in launcher lists. */
	virtual bool isVisible(const Common::String &name) const { return true; }
	/** Return the base player name used to group related registry entries. */
	virtual Common::String primaryName(const Common::String &name) const { return name; }
	/** Return the reserved player name used for direct puzzle startup. */
	virtual Common::String bootName() const { return "Boot"; }
	/** Read the game-specific player registry from the save manager. */
	bool load();
	/** Write the current player registry through its game-specific codec. */
	bool store() const;
	/** Update the selected player's current-slot location and optionally force a registry write. */
	bool storeLocation(uint index, int location, bool forceStore = false);
	/** Capture the affected slot and registry before a save operation. */
	bool checkpoint(uint index, uint slot, Checkpoint &checkpoint) const;
	/** Restore captured files and remove files that were absent before the operation. */
	bool restore(const Checkpoint &checkpoint) const;
	/** Add a named player at the initial location and return its registry index, or minus one on failure. */
	int add(const Common::String &name, int initialLocation = 0);
	/** Remove a player and the slot files associated with that registry entry. */
	bool remove(uint index);
	/** Return the target-prefixed filename for a player's one-based slot. */
	Common::String stateName(uint index, uint slot) const;
	/** Return the current player registry in its stored order. */
	Common::Array<Player> &players() { return _players; }
	/** Return the current player registry in its stored order. */
	const Common::Array<Player> &players() const { return _players; }

protected:
	/** Decode the game-specific registry into the supplied player list. */
	virtual bool readRegistry(Common::SeekableReadStream *stream, Common::Array<Player> &players) = 0;
	/** Encode the supplied player list in the game's registry format. */
	virtual bool writeRegistry(Common::WriteStream &stream, const Common::Array<Player> &players) const = 0;
	/** Build the game-specific slot filename from a persistent player ID and one-based slot. */
	virtual Common::String slotFileName(uint32 id, uint slot) const = 0;
	/** Check whether a registry name belongs to the same player-name group. */
	virtual bool belongsTo(const Common::String &candidate, const Common::String &owner) const { return candidate.equals(owner); }
	/** Read one text line without retaining its line-ending characters. */
	static bool line(Common::SeekableReadStream &stream, Common::String &value);

private:
	/** Player registry filename relative to the target save prefix. */
	static constexpr const char *kRegistryFile = "users.dat";

	/** ScummVM save manager used for registry and slot files. */
	Common::SaveFileManager &_manager;
	/** Target-specific prefix added to all game save filenames. */
	Common::String _prefix;
	/** Decoded player registry, including each player's current slot and saved locations. */
	Common::Array<Player> _players;
	// Allow the bounded property tables accepted by the registry and World readers.
	/** Maximum number of bytes retained for one rollback file. */
	static constexpr uint32 kMaxCheckpointFileSize = 16 * 1024 * 1024;
	/** Capture one existing save file, rejecting files above the checkpoint size limit. */
	bool readCheckpointFile(Checkpoint::File &file) const;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
