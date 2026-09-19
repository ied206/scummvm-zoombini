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
#include "zoombini2/scripts.h"

namespace Zoombini2 {

class RleBlock;
class ZoombiniAnimation;

/** Rescue Site II waiting shelter before Route4. */
class ShelterRescueSite2 : public ShelterRescueSiteBase {
public:
	/** Create Rescue Site II with page identifier nine. */
	explicit ShelterRescueSite2(Zoombini2Engine *vm);
	/** Persist the Rescue Site II waiting roster. */
	~ShelterRescueSite2() override;

	/** Load Rescue Site II resources and restore its waiting roster. */
	void init() override;
	/** Drain pending releases, animate the roster scroll, and refresh occupancy. */
	void onUpdate() override;
	/** Draw the scroll controls, waiting board, and boarding actives. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Leave button presses pending until the release event. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Route common pickup and drop first, then scroll and boarding clicks. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Follow a held Zoombini with the pointer. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Require exactly eight seated Zoombinis. */
	bool canUseGoButton() const override;

protected:
	/** Return the Rescue Site II waiting board. */
	BoardRecord **getRescueBoard() const override;
	/** Return whether @p zoombini occupies a departure seat. */
	bool isDepartingMember(const ZoombiniRunner *zoombini) const override;

private:
	/** Resource paths used by the scene and arrival speech. */
	static constexpr const char *kBackgroundPath = "#bmp/rescue2/background";
	static constexpr const char *kSelectorPath = "bmp/rescue2/selector.rb";
	static constexpr const char *kPorteSelectorPath = "bmp/rescue2/porte_select.rb";
	static constexpr const char *kScrollLeftPath = "bmp/rescue2/button_left.an";
	static constexpr const char *kScrollRightPath = "bmp/rescue2/button_right.an";
	static constexpr const char *kMusicPath = "#sounds/music/C2-BB02.wav";
	static constexpr const char *kLittleZombAnimationPath = "bmp/zombis/littleZomb.anm";
	static constexpr const char *kPickupZombAnimationPath = "bmp/zombis/pris/pris.anm";
	static constexpr const char *kIdleZombAnimationPath = "bmp/zombis/attente2/attenteZomb2.anm";
	static constexpr const char *kAreaMaskPath = "bmp/rescue2/area.bmt";
	static constexpr const char *kMissingArrivalsSpeechFormat = "sounds/BC222.%u.wav";
	static constexpr const char *kReadyDepartureSpeechPath = "sounds/BC213.11.wav";

	/** Number of waiting-grid drop records mirroring the visible board cells. */
	static constexpr int kGridTargetCount = 20;
	/** First drop-record index of the departure seats. */
	static constexpr int kSeatTargetBase = 20;
	/** Number of departure seats scanned by the gate. */
	static constexpr int kDepartureSeatCount = 8;
	/**
	 * Drop records accepted by @ref ZoombiniRunner::handlePointerInput.
	 *
	 * Twenty waiting-grid records plus eight departure-seat rects built from
	 * the original seat table, matching the 28-record original allocation.
	 */
	static constexpr int kDropTargetCount = 28;
	/** Departure-seat screen positions from the original seat table. */
	static constexpr Common::Point32 kSeatPositions[kDepartureSeatCount] = {
		Common::Point32(527, 502),
		Common::Point32(590, 503),
		Common::Point32(656, 492),
		Common::Point32(720, 474),
		Common::Point32(482, 563),
		Common::Point32(539, 563),
		Common::Point32(601, 563),
		Common::Point32(665, 554),
	};
	/** Waiting-floor screen positions from the original floor table. */
	static constexpr Common::Point32 kFloorPositions[kDepartureSeatCount] = {
		Common::Point32(170, 540),
		Common::Point32(150, 540),
		Common::Point32(130, 540),
		Common::Point32(110, 540),
		Common::Point32(90, 540),
		Common::Point32(70, 540),
		Common::Point32(50, 540),
		Common::Point32(30, 540),
	};
	/** No pending roster release. */
	static constexpr int kNoPendingRelease = -1;
	/** Resting scroll phase with no animation in progress. */
	static constexpr int kScrollIdle = -1;
	/** Scroll phase toward earlier board rows. */
	static constexpr int kScrollPhaseLeft04 = 4;
	/** Scroll phase toward later board rows. */
	static constexpr int kScrollPhaseRight06 = 6;
	/** Scroll animation length in pixels, matching one grid column. */
	static constexpr int kScrollPixelLength = 40;
	/** Scroll animation step in pixels per frame. */
	static constexpr int kScrollPixelStep = 4;

	/** Origin of the Rescue Site II waiting-roster grid. */
	static constexpr Common::Point32 kRosterGridBasePos = Common::Point32(75, 193);
	/** Position of the upward Rescue Site II roster-scroll animation. */
	static constexpr Common::Point32 kRosterButtonUpPos = Common::Point32(20, 332);
	/** Position of the downward Rescue Site II roster-scroll animation. */
	static constexpr Common::Point32 kRosterButtonDownPos = Common::Point32(289, 332);

	/**
	 * Immutable Rescue Site II layout rectangles are instance members because
	 * Common::Rect32 requires runtime construction and ScummVM prohibits global C++ constructors.
	 */
	/** Hit rectangle for upward Rescue Site II roster scrolling. */
	const Common::Rect32 _rosterScrollUpRect = Common::Rect32(20, 332, 83, 418);
	/** Hit rectangle for downward Rescue Site II roster scrolling. */
	const Common::Rect32 _rosterScrollDownRect = Common::Rect32(289, 332, 352, 411);

	/** Restore the Rescue Site II visit and departure-roster state. */
	void initRescueRoster();
	/** Refill boarding actives from the board and seat the first eight. */
	void refillBoardingRoster();
	/** Build the waiting-grid and departure-seat drop records. */
	void buildDropTargets();
	/** Refresh waiting-grid record occupancy from the visible board cells. */
	void refreshGridOccupancy();
	/** Return the board cell index for a grid record. */
	int getGridRecordBoardIndex(int recordIndex) const;
	/** Return whether all eight departure seats are occupied. */
	bool seatsFullyOccupied() const;
	/** Return the currently held Zoombini, if any. */
	ZoombiniRunner *getDraggedZoombini() const;
	/**
	 * Store a grid-dropped Zoombini in the board and schedule deferred roster
	 * removal, which runs on the next update like the original release index.
	 */
	void captureToBoard(int recordIndex, int zoombiniIndex);
	/** Erase the pending-release roster member without further index fixup. */
	void releasePendingZoombini();
	/** Materialize a held boarding active from a nonempty grid cell. */
	bool materializeFromBoard(int gridCol, int gridRow, const Common::Point &pointerPos);
	/** Attempt a leftward roster scroll and flash the scroll button. */
	void triggerScrollLeft();
	/** Attempt a rightward roster scroll and flash the scroll button. */
	void triggerScrollRight();
	/** Return whether any board cell in rows @p firstRow through @p lastRow is occupied. */
	bool hasBoardCellsInRows(int firstRow, int lastRow) const;
	/** Advance the roster scroll animation by one frame. */
	void stepScrollAnimation();
	/** Present the interactive cursor while hovering a nonempty grid cell. */
	void updateHoverCursor();
	/** Draw the waiting-board pictures with the current scroll shift. */
	void drawWaitingBoard(ManagedSurface32 *screen, int pixelShiftX) const;
	/** Collect the visible boarding draw order with the held Zoombini last. */
	void buildBoardingDrawOrder(Common::Array<uint> &order, ZoombiniRunner *&draggedZoombini) const;
	/** Draw the seated and waiting boarding actives. */
	void drawBoardingActives(ManagedSurface32 *screen) const;
	/** Snap a dropped Zoombini into a waiting-grid cell. */
	static void gridDropCallback(void *context, int targetIndex, int zoombiniIndex);
	/** Snap a dropped Zoombini into a departure seat. */
	static void seatDropCallback(void *context, int targetIndex, int zoombiniIndex);

	/** Shared little-Zoombini grid borrowed for boarding actives. */
	const ZoombiniAnimation *_littleZombAnimation = nullptr;
	/** Shared pickup grid borrowed for boarding actives. */
	const ZoombiniAnimation *_pickupZombAnimation = nullptr;
	/** Shared idle grid borrowed for boarding actives. */
	const ZoombiniAnimation *_idleZombAnimation = nullptr;
	/** Waiting-grid and departure-seat drop records. */
	Common::Array<ZoombiniDropTarget> _dropTargets;
	/** First board row shown in the visible roster grid. */
	int _scrollRow = 0;
	/** Roster scroll phase: idle, scrolling left, or scrolling right. */
	int _scrollPhase = kScrollIdle;
	/** Background scroll pixel with original wraparound. */
	int _scrollBgX = 0;
	/** Remaining scroll animation pixels. */
	int _scrollPixelsLeft = 0;
	/** Roster index with a deferred removal pending, or -1 for none. */
	int _pendingReleaseIndex = kNoPendingRelease;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_RESCUE2_H
