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

#ifndef TRINKET_ZOOMBINI3_SAVEFILES_H
#define TRINKET_ZOOMBINI3_SAVEFILES_H

#include "trinket/shell/savefiles.h"

namespace Trinket {
namespace Zoombini3 {

/** Z3 Version 12 registry, six save slots and hidden practice companions. */
class SaveFiles : public Shell::SaveFiles {
public:
	/** Select the Zoombinis registry codec and target save prefix. */
	SaveFiles(Common::SaveFileManager &manager, const Common::String &target) : Shell::SaveFiles(manager, target) {}
	/** Construct Zoombinis save storage through the shared subengine interface. */
	static Shell::SaveFiles *create(Common::SaveFileManager &manager, const Common::String &target) { return new SaveFiles(manager, target); }
	/** Return the number of game slots supported for one player. */
	uint maxSlots() const override { return kSlotCount; }
	/** Hide practice-only registry names from ordinary player lists. */
	bool isVisible(const Common::String &name) const override { return !name.hasSuffix("_Practice"); }
	/** Remove a practice suffix to obtain the player's base name. */
	Common::String primaryName(const Common::String &name) const override;
	/** Return the reserved registry name used for direct practice startup. */
	Common::String bootName() const override { return "Boot_Practice"; }

private:
	/** Line ending the global registry properties. */
	static constexpr const char *kGlobalPropertiesEnd = "GLOBAL_END\n";
	/** Header written before global registry properties. */
	static constexpr const char *kRegistryHeader = "Version\n12\n";
	/** Script identifier for version. */
	static constexpr const char *kVersionName = "Version";
	/** Script identifier for 12. */
	static constexpr const char *k12Name = "12";
	/** Script identifier for gLOBAL END. */
	static constexpr const char *kGLOBALENDName = "GLOBAL_END";
	/** Suffix identifying  Practice. */
	static constexpr const char *kPracticeSuffix = "_Practice";

	/** Checkpoint filename indexed by player ID and slot. */
	static constexpr const char *kSlotFileFormat = "User%03u_%uGZ.dat";

	/** Number of native game slots available to each player. */
	static constexpr uint kSlotCount = 6;
	/** Additional registry lines preserved while users and slots are edited. */
	Common::Array<Common::String> _properties;
	/** Decode the native user registry and its retained property lines. */
	bool readRegistry(Common::SeekableReadStream *stream, Common::Array<Player> &players) override;
	/** Encode users, slots and retained registry property lines. */
	bool writeRegistry(Common::WriteStream &stream, const Common::Array<Player> &players) const override;
	/** Build a native slot filename from the player ID and one-based slot. */
	Common::String slotFileName(uint32 id, uint slot) const override;
	/** Check whether a name is the primary or practice entry for the same player. */
	bool belongsTo(const Common::String &candidate, const Common::String &owner) const override;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
