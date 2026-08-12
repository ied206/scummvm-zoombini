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

#ifndef ZOOMBINI2_PAGES_SHELTER_BASE_H
#define ZOOMBINI2_PAGES_SHELTER_BASE_H

#include "zoombini2/pages/interactive_base.h"

namespace Zoombini2 {

/** Party assembly, storage, and arrival hubs with direct access to the map. */
class ShelterPage : public InteractivePage {
public:
	explicit ShelterPage(Zoombini2Engine *engine) : InteractivePage(engine) {}
	bool hasSidebar() const override { return true; }
	bool isShelter() const override { return true; }
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_BASE_H
