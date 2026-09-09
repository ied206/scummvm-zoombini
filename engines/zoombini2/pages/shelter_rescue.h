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

#ifndef ZOOMBINI2_PAGES_SHELTER_RESCUE_H
#define ZOOMBINI2_PAGES_SHELTER_RESCUE_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/shelter_base.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;
class Animation;
class ZoombiniGraphics;
class ZoombiniState;

/**
 * Rescue Sites I and II store waiting Zoombinis and assemble parties of eight.
 * Site I selects one of two routes; Site II continues toward Snowboard Gulch.
 * Each site loads its own bmp/rescue1 or bmp/rescue2 resources.
 */
class RescuePage : public ShelterPage {
public:
	/** Construct rescue site @p rescueNum for @p engine. */
	RescuePage(Zoombini2Engine *engine, int rescueNum);
	/** Release the site resources and temporary departure roster. */
	~RescuePage() override;

	/** Load the selected rescue site and restore its waiting roster. */
	void init() override;
	/** Advance the selection or departure phase. */
	void update() override;
	/** Draw the waiting roster, site controls, and departure effect. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Select a visible Zoombini or activate the site's route controls. */
	void handleClick(const Common::Point &pos) override;
	/** Return whether the site has assembled the required party of eight. */
	bool hasFullDepartureParty() const { return _readyToDepart; }

private:
	/** Rescue site variant, either one or two. */
	int _rescueNum;

	/** Selection marker for a visible Zoombini slot. */
	RleBlock *_selector;
	/** Portal body used by rescue site one. */
	RleBlock *_portal;
	/** Portal foreground used by rescue site one. */
	RleBlock *_portalTop;
	/** Rescue-site-one foreground overlay. */
	RleBlock *_cramure;
	/** Route selector used by rescue site one. */
	RleBlock *_porteSelect;
	/** Inactive left route arrow used by rescue site one. */
	BitBlock *_arrowLeftOff;
	/** Highlighted left route arrow used by rescue site one. */
	BitBlock *_arrowLeftOn;
	/** Inactive right route arrow used by rescue site one. */
	BitBlock *_arrowRightOff;
	/** Highlighted right route arrow used by rescue site one. */
	BitBlock *_arrowRightOn;
	/** Upward roster scroll animation. */
	Animation *_buttonUp;
	/** Downward roster scroll animation. */
	Animation *_buttonDown;

	/** Signed screen origin of the visible roster grid. */
	Common::Point32 _gridBasePosition;

	/** Number of columns in the visible roster grid. */
	static const int kGridCols = 4;
	/** Number of rows in the visible roster grid. */
	static const int kGridRows = 5;
	/** Width of one visible roster slot. */
	static const int kSlotWidth = 40;
	/** Height of one visible roster slot. */
	static const int kSlotHeight = 57;
	/** Hit-test rectangles for the visible roster page. */
	Common::Rect _slotRects[kGridCols * kGridRows];

	/** Hit-test rectangle for scrolling toward earlier roster entries. */
	Common::Rect _scrollUpRect;
	/** Hit-test rectangle for scrolling toward later roster entries. */
	Common::Rect _scrollDownRect;

	/** Left route-arrow draw position at rescue site one. */
	Common::Point _arrowLeftPos;
	/** Right route-arrow draw position at rescue site one. */
	Common::Point _arrowRightPos;

	/** Portal-body draw position at rescue site one. */
	Common::Point _portalPos;
	/** Portal-foreground draw position at rescue site one. */
	Common::Point _portalTopPos;
	/** Foreground-overlay draw position at rescue site one. */
	Common::Point _cramurePos;

	/** First waiting-roster entry shown in the visible grid. */
	int _scrollOffset;
	/** Selected waiting-roster entry, or `-1` when none is selected. */
	int _selectedZoombini;
	/** Whether the departure roster contains at least eight Zoombinis. */
	bool _readyToDepart;

	/** Current site phase, with zero selecting and one departing. */
	int _phase;
	/** Time at which the current phase began. */
	uint32 _phaseTimer;

	/** Music handle used while the rescue site is active. */
	int _musicId;

	/** Little-Zoombini animation cells used in the roster grid. */
	ZoombiniGraphics *_zoombiniGfx;
	/** Runtime party chosen to leave this rescue site. */
	Common::Array<ZoombiniState *> _departureRoster;
	/** Restore the waiting roster and populate the departure party. */
	void initRescueRoster();
	/** Persist the waiting roster and selected departure party. */
	void saveRescueRoster();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_RESCUE_H
