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

#ifndef ZOOMBINI2_PAGES_SHELTER_RESCUE1_H
#define ZOOMBINI2_PAGES_SHELTER_RESCUE1_H

#include "zoombini2/pages/shelter_base.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;

/** Rescue Site I waiting shelter and Route2/Route3 branch selector. */
class ShelterRescueSite1 : public ShelterRescueSiteBase {
public:
	/** Create Rescue Site I with page identifier four. */
	explicit ShelterRescueSite1(Zoombini2Engine *vm);
	/** Persist the Rescue Site I roster and release its route-selection resources. */
	~ShelterRescueSite1() override;

	/** Load Rescue Site I resources and restore its waiting roster. */
	void init() override;
	/** Require eight departing Zoombinis and a valid route branch. */
	bool canUseGoButton() const override;

protected:
	/** Return the Rescue Site I waiting board. */
	BoardRecord **getRescueBoard() const override;
	/** Draw the Rescue Site I portal, foreground, and branch arrows. */
	void onRenderSite(ManagedSurface32 *screen) override;
	/** Handle the two Rescue Site I branch hotspots. */
	EventHandleResult onSiteLButtonDown(const Common::Point &pos) override;

private:
	/** Restore the Rescue Site I visit and departure-roster state. */
	void initRescueRoster();

	/** Portal body. */
	RleBlock *_portal;
	/** Portal foreground. */
	RleBlock *_portalTop;
	/** Foreground overlay. */
	RleBlock *_cramure;
	/** Inactive left route arrow. */
	BitBlock *_arrowLeftOff;
	/** Highlighted left route arrow. */
	BitBlock *_arrowLeftOn;
	/** Inactive right route arrow. */
	BitBlock *_arrowRightOff;
	/** Highlighted right route arrow. */
	BitBlock *_arrowRightOn;
	/** Left route-arrow draw position. */
	Common::Point32 _arrowLeftPos;
	/** Right route-arrow draw position. */
	Common::Point32 _arrowRightPos;
	/** Portal-body draw position. */
	Common::Point32 _portalPos;
	/** Portal-foreground draw position. */
	Common::Point32 _portalTopPos;
	/** Foreground-overlay draw position. */
	Common::Point32 _cramurePos;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_RESCUE1_H
