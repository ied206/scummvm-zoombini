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

#ifndef TRINKET_RR1CLOUD9_SAVEFILES_H
#define TRINKET_RR1CLOUD9_SAVEFILES_H

#include "trinket/shell/savefiles.h"

namespace Trinket {
namespace RR1Cloud9 {

/** One portable navigation checkpoint per player; not a native RR save codec. */
class SaveFiles : public Shell::SaveFiles {
public:
	/** Select the Reader Rabbit registry codec and target save prefix. */
	SaveFiles(Common::SaveFileManager &manager, const Common::String &target) : Shell::SaveFiles(manager, target) {}
	/** Construct Reader Rabbit save storage through the shared subengine interface. */
	static Shell::SaveFiles *create(Common::SaveFileManager &manager, const Common::String &target) { return new SaveFiles(manager, target); }
	/** Return the single game slot supported for each player. */
	uint maxSlots() const override { return 1; }

private:
	/** Header written before portable player records. */
	static constexpr const char *kRegistryHeader = "RR1Cloud9 saves\n1\n";
	/** Identifier accepted for the earlier portable profile registry. */
	static constexpr const char *kLegacyRegistryMagic = "RR1Cloud9 profiles";
	/** Identifier of the portable player registry. */
	static constexpr const char *kRegistryMagic = "RR1Cloud9 saves";
	/** Script identifier for version. */
	static constexpr const char *kVersionName = "Version";
	/** Script identifier for 12. */
	static constexpr const char *k12Name = "12";
	/** Script identifier for gLOBAL END. */
	static constexpr const char *kGLOBALENDName = "GLOBAL_END";
	/** Script identifier for 1. */
	static constexpr const char *k1Name = "1";

	/** Checkpoint filename indexed by player ID and slot. */
	static constexpr const char *kSlotFileFormat = "User%03u_%uGZ.dat";

	/** Decode the Reader Rabbit player registry. */
	bool readRegistry(Common::SeekableReadStream *stream, Common::Array<Player> &players) override;
	/** Encode the Reader Rabbit player registry. */
	bool writeRegistry(Common::WriteStream &stream, const Common::Array<Player> &players) const override;
	/** Build the player's single-slot save filename from its persistent ID. */
	Common::String slotFileName(uint32 id, uint slot) const override;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
