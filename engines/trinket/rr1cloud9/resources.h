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

#ifndef TRINKET_RR1CLOUD9_RESOURCES_H
#define TRINKET_RR1CLOUD9_RESOURCES_H

#include "common/ptr.h"
#include "trinket/ompp/ne_resource.h"

namespace Trinket {
namespace RR1Cloud9 {

/** Explicit library names keep page resources with overlapping IDs separate. */
class Resources : public OM::ResourceProvider {
public:
	bool load(Common::Path &gamePath);
	Common::SeekableReadStream *open(const Common::String &name) const override;

private:
	static Common::SeekableReadStream *unpack(Common::SeekableReadStream *stream, const Common::String &tag);
	Common::Array<Common::Path> _directories;
	mutable Common::HashMap<Common::String, Common::SharedPtr<OM::NEResourceLibrary>> _libraries;
	Common::SeekableReadStream *openLibrary(const Common::String &library, const Common::String &tag, uint id) const;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
