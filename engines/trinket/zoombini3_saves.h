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
	Saves(Common::SaveFileManager &manager, const Common::String &target);
	bool load();
	const Common::Array<Shell::SaveFiles::Player> &players() const { return _saveFiles.players(); }
	Common::String stateName(uint player, uint slot) const { return _saveFiles.stateName(player, slot); }
	bool rename(uint player, const Common::String &name, bool clone);
	bool remove(uint player);
	bool select(uint player, uint slot);
	bool importSlot(uint player, uint slot, Common::SeekableReadStream &stream);
	bool exportSlot(uint player, uint slot, Common::WriteStream &stream);
	bool rollbackFailed() const { return _rollbackFailed; }

private:
	typedef Common::Array<byte> Bytes;
	typedef Common::HashMap<Common::String, Bytes> Files;
	Common::SaveFileManager &_manager;
	Common::String _prefix, _registryFile;
	SaveFiles _saveFiles;
	Launcher::Registry _registry;
	Files _before;
	bool _rollbackFailed = false;
	bool begin();
	bool finish(bool success);
	Common::StringArray files() const;
	bool validSlot(uint player, uint slot) const;
	bool write(const Common::String &file, const Bytes &bytes);
	static bool read(Common::SeekableReadStream &stream, Bytes &bytes);
	static bool validName(const Common::String &name);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
