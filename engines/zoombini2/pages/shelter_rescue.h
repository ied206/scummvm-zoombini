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
class ZoombiniGfx;
class Zoombini;

/**
 * Rescue Sites I and II store waiting Zoombinis and assemble parties of eight.
 * Site I selects one of two routes; Site II continues toward Snowboard Gulch.
 * Each site loads its own bmp/rescue1 or bmp/rescue2 resources.
 */
class RescuePage : public ShelterPage {
public:
	RescuePage(Zoombini2Engine *engine, int rescueNum);
	~RescuePage() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;
	bool hasFullDepartureParty() const { return _readyToDepart; }

private:
	int _rescueNum;          // 1 or 2

	// Sprite resources — Rescue1 loads all; Rescue2 loads a subset
	RleBlock *_selector;
	RleBlock *_portal;        // Rescue1 only: PORTE.RB
	RleBlock *_portalTop;     // Rescue1 only: portal_top.rb
	RleBlock *_cramure;       // Rescue1 only: CRAMURE.RB
	RleBlock *_porteSelect;   // porte_select.rb
	BitBlock *_arrowLeftOff;  // Rescue1 only: inside_arrow_left_off.bb
	BitBlock *_arrowLeftOn;   // Rescue1 only: inside_arrow_left_on.bb
	BitBlock *_arrowRightOff; // Rescue1 only: inside_arrow_right_off.bb
	BitBlock *_arrowRightOn;  // Rescue1 only: inside_arrow_right_on.bb
	Animation *_buttonUp;     // button_left.an (scroll up / scroll left)
	Animation *_buttonDown;   // button_right.an (scroll down / scroll right)

	// Grid layout from IDA: Rescue1 base (68,95), Rescue2 base (75,193)
	int _gridBaseX;
	int _gridBaseY;

	// Zoombini slot grid: 4 columns × 5 rows = 20 slots
	static const int kGridCols = 4;
	static const int kGridRows = 5;
	static const int kSlotWidth = 40;
	static const int kSlotHeight = 57;
	Common::Rect _slotRects[kGridCols * kGridRows];

	// Scroll button rects from IDA
	Common::Rect _scrollUpRect;   // Rescue1: (26,240,89,326)  Rescue2: (20,332,83,418)
	Common::Rect _scrollDownRect; // Rescue1: (276,240,339,319) Rescue2: (289,332,352,411)

	// Arrow positions from IDA (Rescue1 only)
	Common::Point _arrowLeftPos;  // (525, 248)
	Common::Point _arrowRightPos; // (641, 253)

	// Portal positions from IDA (Rescue1 only)
	Common::Point _portalPos;     // (520, 84) normal / (520, -130) departing
	Common::Point _portalTopPos;  // (518, 99)
	Common::Point _cramurePos;    // (311, 99)

	int _scrollOffset;
	int _selectedZoombini;
	bool _readyToDepart;     // true when 8+ zoombinis collected

	int _phase;              // 0=selecting, 1=departing
	uint32 _phaseTimer;

	int _musicId;

	// Zoombini sprite graphics (littleZomb.anm) for grid rendering
	ZoombiniGfx *_zoombiniGfx;
	Common::Array<Zoombini *> _departureRoster;
	void initRescueRoster();
	void saveRescueRoster();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_RESCUE_H
