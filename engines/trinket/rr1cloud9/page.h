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

#ifndef TRINKET_RR1CLOUD9_PAGE_H
#define TRINKET_RR1CLOUD9_PAGE_H

#include "common/rect.h"
#include "trinket/core/script.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace RR1Cloud9 {

/** Authored presentation and exits for the navigation-only runtime. */
class Page {
public:
	struct Exit {
		Common::Rect rect;
		Common::String destination;
	};
	bool load(const OM::ResourceProvider &resources, const Common::String &name);
	Common::String text(const Common::String &name) const;
	int number(const Common::String &name) const;
	const Common::Array<Exit> &exits() const { return _exits; }
	const Core::Script &script() const { return _script; }

private:
	Core::Script _script;
	Common::Array<Exit> _exits;
	const Core::Script::Variable *constant(const Common::String &name) const;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
