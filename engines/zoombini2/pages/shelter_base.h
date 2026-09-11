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

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/interactive_base.h"

namespace Zoombini2 {

/** Party assembly, storage, and arrival hubs with direct access to the map. */
class ShelterBase : public InteractiveBase {
public:
	/** Bind a shelter page to @p vm. */
	explicit ShelterBase(Zoombini2Engine *vm) : InteractiveBase(vm) {}
	/** Return whether the shared sidebar is visible. */
	bool hasSidebar() const override { return true; }
	/** Identify this page as a shelter for global page policy. */
	bool isShelter() const override { return true; }
};

class Animation;
class RleBlock;
class ZoombiniAnimation;
class ZoombiniState;
struct BoardRecord;

/** Common waiting-roster, scrolling, and departure lifecycle for both rescue sites. */
class ShelterRescueSiteBase : public ShelterBase {
public:
	/** Release the common rescue-site resources. */
	~ShelterRescueSiteBase() override;

	/** Advance the departure phase. */
	void onUpdate() override;
	/** Draw the site-owned foreground followed by the common waiting roster. */
	void onRenderScene(ManagedSurface32 *screen) override;
	/** Dispatch common scrolling and roster selection around site-owned controls. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Return whether the departure roster contains exactly eight Zoombinis. */
	bool hasFullDepartureParty() const { return _readyToDepart; }

protected:
	/** Bind a concrete rescue site to @p vm. */
	explicit ShelterRescueSiteBase(Zoombini2Engine *vm);

	/** Load and draw the persistent site background. */
	void loadBackground(const char *path);
	/** Configure the site-specific roster and scroll-control geometry. */
	void configureRosterLayout(const Common::Point32 &gridBasePos, const Common::Rect32 &scrollUpRect, const Common::Rect32 &scrollDownRect, const Common::Point32 &buttonUpPos, const Common::Point32 &buttonDownPos);
	/** Load the selection marker used by the waiting roster. */
	void loadSelector(const char *path);
	/** Load the door-selection overlay retained by the site. */
	void loadPorteSelector(const char *path);
	/** Load the two roster scroll animations. */
	void loadScrollButtons(const char *buttonUpPath, const char *buttonDownPath);
	/** Start the site-owned looping music. */
	void startMusic(const char *path);
	/** Reset common selection and departure state after presentation resources load. */
	void resetRescueState();
	/** Clear an unvisited waiting board and restore its saved scroll position. */
	void prepareWaitingBoard(BoardRecord **board);
	/** Refill the eight-member departure roster from @p board. */
	void refillDepartureRoster(BoardRecord **board);
	/** Persist every non-departing member in @p board and write the active save. */
	void saveRescueRoster(BoardRecord **board);
	/** Load the shared little-Zoombini animation through the engine cache. */
	void loadRosterAnimation();
	/** Begin the delayed map transition used by Rescue Site I's branch controls. */
	void beginDeparture();
	/** Return whether the delayed departure phase is active. */
	bool isDeparting() const { return _phase == 1; }

	/** Return the concrete site's waiting board. */
	virtual BoardRecord **getRescueBoard() const = 0;
	/** Draw site-specific layers before the common scroll controls and waiting roster. */
	virtual void onRenderSite(ManagedSurface32 *screen);
	/** Handle site-specific controls before common waiting-roster selection. */
	virtual EventHandleResult onSiteLButtonDown(const Common::Point &pos);

private:
	/** Number of columns in the visible roster grid. */
	static const int kGridCols = 4;
	/** Number of rows in the visible roster grid. */
	static const int kGridRows = 5;
	/** Width of one visible roster slot. */
	static const int kSlotWidth = 40;
	/** Height of one visible roster slot. */
	static const int kSlotHeight = 57;

	/** Selection marker for a visible Zoombini slot. */
	RleBlock *_selector = nullptr;
	/** Door-selection overlay retained by both rescue sites. */
	RleBlock *_porteSelect = nullptr;
	/** Upward roster scroll animation. */
	Animation *_buttonUp = nullptr;
	/** Downward roster scroll animation. */
	Animation *_buttonDown = nullptr;

	/** Signed screen origin of the visible roster grid. */
	Common::Point32 _gridBasePos = Common::Point32();
	/** Hit-test rectangles for the visible roster page. */
	Common::Rect32 _slotRects[kGridCols * kGridRows] = {};
	/** Hit-test rectangle for scrolling toward earlier roster entries. */
	Common::Rect32 _scrollUpRect = Common::Rect32();
	/** Hit-test rectangle for scrolling toward later roster entries. */
	Common::Rect32 _scrollDownRect = Common::Rect32();
	/** Draw position for the earlier-entry scroll animation. */
	Common::Point32 _buttonUpPos = Common::Point32();
	/** Draw position for the later-entry scroll animation. */
	Common::Point32 _buttonDownPos = Common::Point32();

	/** First waiting-roster entry shown in the visible grid. */
	int _scrollOffset = 0;
	/** Selected waiting-roster entry, or `-1` when none is selected. */
	int _selectedZoombini = -1;
	/** Whether the departure roster contains exactly eight Zoombinis. */
	bool _readyToDepart = false;
	/** Current site phase, with zero selecting and one departing. */
	int _phase = 0;
	/** Time at which the current phase began. */
	uint32 _phaseTimer = 0;
	/** Music handle used while the rescue site is active. */
	int _musicId = -1;
	/** Borrowed immutable sprite grid owned by the engine cache. */
	const ZoombiniAnimation *_zoombiniAnimation = nullptr;
	/** Runtime party chosen to leave this rescue site. */
	Common::Array<ZoombiniState *> _departureRoster;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_BASE_H
