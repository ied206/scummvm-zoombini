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
	SaveFiles(Common::SaveFileManager &manager, const Common::String &target) : Shell::SaveFiles(manager, target) {}
	static Shell::SaveFiles *create(Common::SaveFileManager &manager, const Common::String &target) { return new SaveFiles(manager, target); }
	uint maxSlots() const override { return 1; }

private:
	bool readRegistry(Common::SeekableReadStream *stream, Common::Array<Player> &players) override;
	bool writeRegistry(Common::WriteStream &stream, const Common::Array<Player> &players) const override;
	Common::String slotFileName(uint32 id, uint slot) const override;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
