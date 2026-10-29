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

#ifndef TRINKET_ZOOMBINI3_SAVES_H
#define TRINKET_ZOOMBINI3_SAVES_H

#include "common/hash-str.h"
#include "common/hashmap.h"
#include "trinket/launcher/registry.h"
#include "trinket/zoombini3_savefiles.h"

namespace Trinket {
namespace Zoombini3 {

/** Offline save operations, keeping launcher identities and practice companions together. */
class Saves {
public:
	/** Bind offline transactions to one target's save storage. */
	Saves(Common::SaveFileManager &manager, const Common::String &target);
	/** Load the game and launcher registries without rewriting them. */
	bool load();
	/** Return the native game registry, including practice companions. */
	const Common::Array<Shell::SaveFiles::Player> &players() const { return _saveFiles.players(); }
	/** Apply the native codec's player visibility policy to a name. */
	bool isVisible(const Common::String &name) const { return _saveFiles.isVisible(name); }
	/** Return a target-prefixed filename for a registry index and one-based slot. */
	Common::String stateName(uint player, uint slot) const { return _saveFiles.stateName(player, slot); }
	/** Rename or clone a player together with launcher identity and practice saves. */
	bool rename(uint player, const Common::String &name, bool clone);
	/** Delete a player, its slots, its practice companion and launcher identity. */
	bool remove(uint player);
	/** Persist the one-based slot selected for this player. */
	bool select(uint player, uint slot);
	/** Validate and replace one native slot, updating its registered location. */
	bool importSlot(uint player, uint slot, Common::SeekableReadStream &stream);
	/** Validate and import one native save under a new player and launcher identity. */
	bool importSave(const Common::String &name, Common::SeekableReadStream &stream);
	/** Copy one native slot to the caller's output stream. */
	bool exportSlot(uint player, uint slot, Common::WriteStream &stream);
	/** Report an incomplete restoration after a failed write. */
	bool rollbackFailed() const { return _rollbackFailed; }

private:
	/** XML element or attribute name for  Practice. */
	static constexpr const char *kXMLPractice = "_Practice";

	/** Suffix separating target IDs from portable save filenames. */
	static constexpr const char *kSavePrefixSuffix = ".trinket-";
	/** Launcher registry filename relative to the target save prefix. */
	static constexpr const char *kLauncherRegistryFile = "launcher-users.xml";

	/** Uncompressed file bytes retained for rollback. */
	typedef Common::Array<byte> Bytes;
	/** File snapshots indexed by their target-prefixed names. */
	typedef Common::HashMap<Common::String, Bytes> Files;
	/** Save storage supplied by the backend; must outlive this manager. */
	Common::SaveFileManager &_manager;
	/** Prefix restricting transactions to the configured target. */
	Common::String _prefix;
	/** Target-prefixed launcher registry filename. */
	Common::String _registryFile;
	/** Native registry codec and player/slot policies. */
	SaveFiles _saveFiles;
	/** Launcher identities kept in step with game player names. */
	Launcher::Registry _registry;
	/** Pre-operation file images used on failure. */
	Files _before;
	/** Whether any pre-operation image could not be restored. */
	bool _rollbackFailed = false;
	/** Snapshot the target's files before modifying its registries or slots. */
	bool begin();
	/** Commit both registries, or restore their pre-operation file images. */
	bool finish(bool success);
	/** List only files belonging to this target's save prefix. */
	Common::StringArray files() const;
	/** Check the registry index and one-based slot against the player's count. */
	bool validSlot(uint player, uint slot) const;
	/** Write and finalize one uncompressed file image. */
	bool write(const Common::String &file, const Bytes &bytes);
	/** Read a bounded file image from the beginning of a seekable stream. */
	static bool read(Common::SeekableReadStream &stream, Bytes &bytes);
	/** Check the native single-byte name limit and reserved practice suffix. */
	static bool validName(const Common::String &name);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
