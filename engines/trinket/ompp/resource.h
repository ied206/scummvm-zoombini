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

#ifndef TRINKET_OMPP_RESOURCE_H
#define TRINKET_OMPP_RESOURCE_H

#include "common/str.h"
#include "common/stream.h"

namespace Trinket {

/** OM foundations: ompp32x.dll in games and ompp32x.tlc in the TLC launcher. */
namespace OM {

/**
 * Supply named resource bytes to consumers such as @ref Trinket::Shell::Scene and @ref Trinket::OM::Sound.
 * Each returned stream is owned by the caller and has an independent read position.
 */
class ResourceProvider {
public:
	virtual ~ResourceProvider() {}
	virtual Common::SeekableReadStream *open(const Common::String &name) const = 0;
};

} // End of namespace OM
} // End of namespace Trinket

#endif
