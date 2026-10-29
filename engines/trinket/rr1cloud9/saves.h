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

#ifndef TRINKET_RR1CLOUD9_SAVES_H
#define TRINKET_RR1CLOUD9_SAVES_H

#include "trinket/launcher/registry.h"
#include "trinket/rr1cloud9/savefiles.h"

namespace Trinket {
namespace RR1Cloud9 {

/** Offline operations on portable RR1Cloud9 saves and their launcher identities. */
class Saves {
public:
	/** Bind the manager to one configuration target's save storage. */
	Saves(Common::SaveFileManager &manager, const Common::String &target);
	/** Load both registries without changing the files. */
	bool load();
	/** Return the RR player registry; each player has one checkpoint. */
	const Common::Array<Shell::SaveFiles::Player> &players() const { return _saveFiles.players(); }
	/** Rename a player in both registries, preserving its IDs and checkpoint. */
	bool rename(uint player, const Common::String &name);
	/** Delete one player's checkpoint and both registry entries. */
	bool remove(uint player);
	/** Validate and replace a portable checkpoint and its registered location. */
	bool importSlot(uint player, Common::SeekableReadStream &stream);
	/** Import a portable checkpoint under a new player and launcher identity. */
	bool importSave(const Common::String &name, Common::SeekableReadStream &stream);
	/** Copy the portable checkpoint to the caller's output stream. */
	bool exportSlot(uint player, Common::WriteStream &stream) const;
	/** Report whether the last failed transaction could not be fully restored. */
	bool rollbackFailed() const { return _rollbackFailed; }

private:
	/** Launcher registry filename after the target prefix. */
	static constexpr const char *kLauncherRegistryFile = ".trinket-launcher-users.xml";
	/** Maximum checkpoint size, including all five flipbooks, in bytes. */
	static constexpr uint32 kMaxFileSize = 16 * 1024 * 1024;
	/** Backend save storage; must outlive the manager. */
	Common::SaveFileManager &_manager;
	/** Filename of this target's launcher identities. */
	Common::String _registryFile;
	/** RR registry codec and checkpoint I/O. */
	SaveFiles _saveFiles;
	/** Launcher identities kept in step with the game registry. */
	Launcher::Registry _registry;
	/** Pre-operation checkpoint and game registry, including absent files. */
	Shell::SaveFiles::Checkpoint _checkpoint;
	/** Pre-operation launcher registry bytes. */
	Common::Array<byte> _launcherBefore;
	/** Whether the launcher registry existed before the transaction. */
	bool _launcherExisted = false;
	/** Whether restoration failed after the last write failure. */
	bool _rollbackFailed = false;
	/** Snapshot only the selected player's checkpoint and the two registries. */
	bool begin(uint player);
	/** Commit registry edits, restoring the checkpoint and both registries on failure. */
	bool finish(bool success);
	/** Write and finalize one file in backend save storage. */
	bool write(const Common::String &filename, const Common::Array<byte> &bytes) const;
	/** Read at most kMaxFileSize bytes from the beginning of a stream. */
	static bool read(Common::SeekableReadStream &stream, Common::Array<byte> &bytes);
	/** Check the RR sign-in length limit and single-byte player name characters. */
	static bool validName(const Common::String &name);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
