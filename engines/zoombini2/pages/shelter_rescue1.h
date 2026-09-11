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
	/** Origin of the Rescue Site I waiting-roster grid. */
	static constexpr Common::Point32 kRosterGridBasePos = Common::Point32(68, 95);
	/** Position of the upward Rescue Site I roster-scroll animation. */
	static constexpr Common::Point32 kRosterButtonUpPos = Common::Point32(26, 240);
	/** Position of the downward Rescue Site I roster-scroll animation. */
	static constexpr Common::Point32 kRosterButtonDownPos = Common::Point32(276, 240);

	/**
	 * Immutable Rescue Site I layout rectangles are instance members because
	 * Common::Rect32 requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Hit rectangle for upward Rescue Site I roster scrolling. */
	const Common::Rect32 _rosterScrollUpRect = Common::Rect32(26, 240, 89, 326);
	/** Hit rectangle for downward Rescue Site I roster scrolling. */
	const Common::Rect32 _rosterScrollDownRect = Common::Rect32(276, 240, 339, 319);

	/** Restore the Rescue Site I visit and departure-roster state. */
	void initRescueRoster();

	/** Portal body. */
	RleBlock *_portal = nullptr;
	/** Portal foreground. */
	RleBlock *_portalTop = nullptr;
	/** Foreground overlay. */
	RleBlock *_cramure = nullptr;
	/** Inactive left route arrow. */
	BitBlock *_arrowLeftOff = nullptr;
	/** Highlighted left route arrow. */
	BitBlock *_arrowLeftOn = nullptr;
	/** Inactive right route arrow. */
	BitBlock *_arrowRightOff = nullptr;
	/** Highlighted right route arrow. */
	BitBlock *_arrowRightOn = nullptr;
	/** Left route-arrow draw position. */
	Common::Point32 _arrowLeftPos = Common::Point32();
	/** Right route-arrow draw position. */
	Common::Point32 _arrowRightPos = Common::Point32();
	/** Portal-body draw position. */
	Common::Point32 _portalPos = Common::Point32();
	/** Portal-foreground draw position. */
	Common::Point32 _portalTopPos = Common::Point32();
	/** Foreground-overlay draw position. */
	Common::Point32 _cramurePos = Common::Point32();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_RESCUE1_H
