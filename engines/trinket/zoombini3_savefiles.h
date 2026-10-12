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
	SaveFiles(Common::SaveFileManager &manager, const Common::String &target) : Shell::SaveFiles(manager, target) {}
	static Shell::SaveFiles *create(Common::SaveFileManager &manager, const Common::String &target) { return new SaveFiles(manager, target); }
	uint maxSlots() const override { return kSlotCount; }
	bool isVisible(const Common::String &name) const override { return !name.hasSuffix("_Practice"); }
	Common::String primaryName(const Common::String &name) const override;
	Common::String bootName() const override { return "Boot_Practice"; }

private:
	static constexpr uint kSlotCount = 6;
	Common::Array<Common::String> _properties;
	bool readRegistry(Common::SeekableReadStream *stream, Common::Array<Player> &players) override;
	bool writeRegistry(Common::WriteStream &stream, const Common::Array<Player> &players) const override;
	Common::String slotFileName(uint32 id, uint slot) const override;
	bool belongsTo(const Common::String &candidate, const Common::String &owner) const override;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
