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

#ifndef ZOOMBINI2_PAGES_SHELTER_RESCUE2_H
#define ZOOMBINI2_PAGES_SHELTER_RESCUE2_H

#include "zoombini2/pages/shelter_base.h"

namespace Zoombini2 {

/** Rescue Site II waiting shelter before Route4. */
class ShelterRescueSite2 : public ShelterRescueSiteBase {
public:
	/** Create Rescue Site II with page identifier nine. */
	explicit ShelterRescueSite2(Zoombini2Engine *vm);
	/** Persist the Rescue Site II waiting roster. */
	~ShelterRescueSite2() override;

	/** Load Rescue Site II resources and restore its waiting roster. */
	void init() override;
	/** Require exactly eight departing Zoombinis. */
	bool canUseGoButton() const override;

protected:
	/** Return the Rescue Site II waiting board. */
	BoardRecord **getRescueBoard() const override;

private:
	/** Restore the Rescue Site II visit and departure-roster state. */
	void initRescueRoster();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_RESCUE2_H
