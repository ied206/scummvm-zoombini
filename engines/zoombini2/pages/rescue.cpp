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

#include "common/debug.h"

#include "zoombini2/pages/rescue.h"
#include "zoombini2/gfx.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// RescuePage — rescue transition (page ID 4 and 9).
// Rescue1: Original: Rescue1__Init_428A00, object size 0x88.
// Rescue2: Original: Rescue2__Init_42ADC0, object size 0x54.
//
// Splits the zoombini pack into groups, shows scrollable grid,
// portal animation, route direction selection, then transitions to map.
//
// Rescue1 resources: bmp/rescue1/ (SELECTOR.RB, PORTE.RB, portal_top.rb,
//   porte_select.rb, CRAMURE.RB, button_left/right.an, ENGRNG.AN,
//   inside_arrow_*.bb, STORM.AN, area.bmt)
// Rescue2 resources: bmp/rescue2/ (selector.rb, porte_select.rb,
//   button_left/right.an, area.bmt)
// ============================================================================

RescuePage::RescuePage(Zoombini2Engine *engine, int rescueNum)
	: Page(engine), _rescueNum(rescueNum),
	  _selector(nullptr), _portal(nullptr), _portalTop(nullptr),
	  _cramure(nullptr), _porteSelect(nullptr),
	  _arrowLeftOff(nullptr), _arrowLeftOn(nullptr),
	  _arrowRightOff(nullptr), _arrowRightOn(nullptr),
	  _buttonUp(nullptr), _buttonDown(nullptr),
	  _gridBaseX(0), _gridBaseY(0),
	  _scrollOffset(0), _selectedZoombini(-1), _readyToDepart(false),
	  _phase(0), _phaseTimer(0), _musicId(-1), _zoombiniGfx(nullptr) {
	_pageId = (rescueNum == 1) ? kPageRescue1 : kPageRescue2;
}

RescuePage::~RescuePage() {
	if (_musicId >= 0)
		_engine->getSoundManager()->stop(_musicId);

	delete _selector;
	delete _portal;
	delete _portalTop;
	delete _cramure;
	delete _porteSelect;
	delete _arrowLeftOff;
	delete _arrowLeftOn;
	delete _arrowRightOff;
	delete _arrowRightOn;
	delete _buttonUp;
	delete _buttonDown;
	delete _zoombiniGfx;
}

void RescuePage::init() {
	debug(1, "RescuePage::init — Rescue%d", _rescueNum);

	Common::String basePath = Common::String::format("bmp/rescue%d/", _rescueNum);

	// Load background — original: Scene__LoadBackground("bmp/rescue{N}/back")
	_engine->getScreen()->fillRect(Common::Rect(kScreenWidth, kScreenHeight), 0);
	BitBlock bg;
	if (bg.load(Common::Path(basePath + "back")))
		bg.drawToSurface(_engine->getScreen(), 0, 0);

	// ------------------------------------------------------------------
	// Rescue1: positions from Rescue1__Init_428A00
	// Rescue2: positions from Rescue2__Init_42ADC0
	// ------------------------------------------------------------------
	if (_rescueNum == 1) {
		// Grid base: this+40=68, this+44=95
		_gridBaseX = 68;
		_gridBaseY = 95;

		// Scroll buttons
		// button_left.an (63x86) at (26,240), rect (26,240,89,326)
		_scrollUpRect = Common::Rect(26, 240, 89, 326);
		// button_right.an (63x79) at (276,240), rect (276,240,339,319)
		_scrollDownRect = Common::Rect(276, 240, 339, 319);

		// Arrow positions (inside_arrow_*.bb)
		_arrowLeftPos = Common::Point(525, 248);
		_arrowRightPos = Common::Point(641, 253);

		// Portal positions
		_portalPos = Common::Point(520, 84);
		_portalTopPos = Common::Point(518, 99);
		_cramurePos = Common::Point(311, 99);

		// Load sprites
		_selector = new RleBlock();
		_selector->loadFromFile(Common::Path(basePath + "SELECTOR.RB"));

		_portal = new RleBlock();
		_portal->loadFromFile(Common::Path(basePath + "PORTE.RB"));

		_portalTop = new RleBlock();
		_portalTop->loadFromFile(Common::Path(basePath + "portal_top.rb"));

		_porteSelect = new RleBlock();
		_porteSelect->loadFromFile(Common::Path(basePath + "porte_select.rb"));

		_cramure = new RleBlock();
		_cramure->loadFromFile(Common::Path(basePath + "CRAMURE.RB"));

		// Inside arrow BitBlocks
		_arrowLeftOff = new BitBlock();
		_arrowLeftOff->loadFromBB(Common::Path(basePath + "inside_arrow_left_off.bb"));

		_arrowLeftOn = new BitBlock();
		_arrowLeftOn->loadFromBB(Common::Path(basePath + "inside_arrow_left_on.bb"));

		_arrowRightOff = new BitBlock();
		_arrowRightOff->loadFromBB(Common::Path(basePath + "inside_arrow_right_off.bb"));

		_arrowRightOn = new BitBlock();
		_arrowRightOn->loadFromBB(Common::Path(basePath + "inside_arrow_right_on.bb"));

		// Scroll button animations
		_buttonUp = new Animation();
		_buttonUp->loadFromFile(Common::Path(basePath + "button_left.an"));

		_buttonDown = new Animation();
		_buttonDown->loadFromFile(Common::Path(basePath + "button_right.an"));

		// Music: "sounds/music/C1-BB02.wav" (looping)
		_musicId = _engine->getSoundManager()->load(true, Common::Path("sounds/music/C1-BB02.wav"), true);
		if (_musicId >= 0)
			_engine->getSoundManager()->play(_musicId);
	} else {
		// Rescue2

		// Grid base: this+16=75, this+20=193
		_gridBaseX = 75;
		_gridBaseY = 193;

		// Scroll buttons
		// button_left.an (53x52) at (20,332), rect (20,332,83,418)
		_scrollUpRect = Common::Rect(20, 332, 83, 418);
		// button_right.an (53x52) at (289,332), rect (289,332,352,411)
		_scrollDownRect = Common::Rect(289, 332, 352, 411);

		// Load sprites
		_selector = new RleBlock();
		_selector->loadFromFile(Common::Path(basePath + "selector.rb"));

		_porteSelect = new RleBlock();
		_porteSelect->loadFromFile(Common::Path(basePath + "porte_select.rb"));

		// Scroll button animations
		_buttonUp = new Animation();
		_buttonUp->loadFromFile(Common::Path(basePath + "button_left.an"));

		_buttonDown = new Animation();
		_buttonDown->loadFromFile(Common::Path(basePath + "button_right.an"));

		// Music: "sounds/music/C2-BB02.wav" (looping)
		_musicId = _engine->getSoundManager()->load(true, Common::Path("sounds/music/C2-BB02.wav"), true);
		if (_musicId >= 0)
			_engine->getSoundManager()->play(_musicId);
	}

	// Build slot grid: 4 columns x 5 rows
	// Original: col spacing = 40px, row spacing = 57px
	// Slot rect: (baseX+col*40+18, baseY+row*57+30) to (baseX+col*40+58, baseY+row*57+87)
	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			int idx = col * kGridRows + row;
			int x = _gridBaseX + col * kSlotWidth;
			int y = _gridBaseY + row * kSlotHeight;
			_slotRects[idx] = Common::Rect(x + 18, y + 30, x + 58, y + 87);
		}
	}

	_phase = 0;
	_phaseTimer = _engine->getGameTickCount();
	_scrollOffset = 0;
	_selectedZoombini = -1;
	_readyToDepart = false;

	// Load zoombini sprites (littleZomb.anm) for grid rendering
	// Original: Zoombini__Zoombini_45BEE0 creates temp with &g_worldData, cellIndex=33
	delete _zoombiniGfx;
	_zoombiniGfx = new ZoombiniGfx();
	if (!_zoombiniGfx->loadFromFile(Common::Path("bmp/zombis/littleZomb.anm"))) {
		warning("RescuePage: Failed to load littleZomb.anm");
		delete _zoombiniGfx;
		_zoombiniGfx = nullptr;
	}
}

void RescuePage::update() {
	if (_phase == 0) {
		// Selecting phase — wait for player to choose route direction
	} else if (_phase == 1) {
		// Departing — auto-transition after brief delay (1.5s)
		// Original: g_zoombiniWalkingFlag=1 → g_skipMode=1 in FrameTick
		uint32 elapsed = _engine->getGameTickCount() - _phaseTimer;
		if (elapsed > 1500) {
			_engine->_maptransSourceWorld = _pageId;
			_engine->requestPageChange(kPageMapTrans);
		}
	}
}

void RescuePage::draw(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	if (_rescueNum == 1) {
		// Draw portal at (520, 84) or hidden at (520, -130) when departing
		if (_portal && _portal->isValid()) {
			int portalY = (_phase == 1) ? -130 : _portalPos.y;
			_portal->drawToScreen(screen, _portalPos.x, portalY, lut);
		}

		// Draw portal top overlay at (518, 99)
		if (_portalTop && _portalTop->isValid())
			_portalTop->drawToScreen(screen, _portalTopPos.x, _portalTopPos.y, lut);

		// Draw cramure at (311, 99)
		if (_cramure && _cramure->isValid())
			_cramure->drawToScreen(screen, _cramurePos.x, _cramurePos.y, lut);

		// Draw inside arrows — show direction selection state
		// Left arrow at (525, 248): on if routeDirection==1, off otherwise
		// Right arrow at (641, 253): on if routeDirection==2, off otherwise
		if (_engine->_routeDirection == 1) {
			if (_arrowLeftOn)
				_arrowLeftOn->drawToSurface(screen, _arrowLeftPos.x, _arrowLeftPos.y);
			if (_arrowRightOff)
				_arrowRightOff->drawToSurface(screen, _arrowRightPos.x, _arrowRightPos.y);
		} else if (_engine->_routeDirection == 2) {
			if (_arrowLeftOff)
				_arrowLeftOff->drawToSurface(screen, _arrowLeftPos.x, _arrowLeftPos.y);
			if (_arrowRightOn)
				_arrowRightOn->drawToSurface(screen, _arrowRightPos.x, _arrowRightPos.y);
		} else {
			if (_arrowLeftOff)
				_arrowLeftOff->drawToSurface(screen, _arrowLeftPos.x, _arrowLeftPos.y);
			if (_arrowRightOff)
				_arrowRightOff->drawToSurface(screen, _arrowRightPos.x, _arrowRightPos.y);
		}
	}

	// Draw scroll buttons at IDA positions
	// Rescue1: button_left at (26,240), button_right at (276,240)
	// Rescue2: button_left at (20,332), button_right at (289,332)
	if (_buttonUp && _buttonUp->getFrameCount() > 0) {
		const RleBlock *frame = _buttonUp->getFrame(0);
		if (frame) {
			if (_rescueNum == 1)
				frame->drawToScreen(screen, 26, 240, lut);
			else
				frame->drawToScreen(screen, 20, 332, lut);
		}
	}

	if (_buttonDown && _buttonDown->getFrameCount() > 0) {
		const RleBlock *frame = _buttonDown->getFrame(0);
		if (frame) {
			if (_rescueNum == 1)
				frame->drawToScreen(screen, 276, 240, lut);
			else
				frame->drawToScreen(screen, 289, 332, lut);
		}
	}

	// Draw zoombini slot grid
	// Original: Rescue1__DrawZoombiniGrid_4273B0 / Rescue2__DrawZoombiniGrid_429870
	// Default cell index 33 (standing pose), features drawn as 5 layered sprites.
	// Clipped to grid area: (gridBaseX, 0, gridBaseX+223, 600).
	if (_zoombiniGfx) {
		static const int kStandingCell = 33;
		int baseIdx = kStandingCell * ZoombiniGfx::kDim1 * ZoombiniGfx::kDim2;
		int clipRight = _gridBaseX + 223;
		int numZoombinis = (int)_engine->_globalZoombinis.size();

		for (int col = 0; col < kGridCols; col++) {
			int colSlotBase = (col + _scrollOffset) * kGridRows;
			for (int row = 0; row < kGridRows; row++) {
				int slotIdx = colSlotBase + row;
				if (slotIdx < 0 || slotIdx >= numZoombinis)
					continue;

				const Zoombini *z = _engine->_globalZoombinis[slotIdx];
				if (!z)
					continue;

				int px = _gridBaseX + col * kSlotWidth + 18;
				int py = _gridBaseY + row * kSlotHeight + 30;

				// Body: [33][0][0]
				const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
				if (frame)
					frame->drawToScreenClipped(screen, px, py,
						_gridBaseX, 0, clipRight, kScreenHeight, lut);

				// Features 1..4 (Hair, Eyes, Nose, Feet)
				const byte features[4] = {
					z->_featureA, z->_featureB, z->_featureC, z->_featureD
				};
				for (int slot = 1; slot <= 4; slot++) {
					int featIdx = baseIdx + slot * ZoombiniGfx::kDim2 + features[slot - 1];
					frame = _zoombiniGfx->getFrame(featIdx, 0);
					if (frame)
						frame->drawToScreenClipped(screen, px, py,
						_gridBaseX, 0, clipRight, kScreenHeight, lut);
				}

				// Draw selector if this zoombini is selected
				if (slotIdx == _selectedZoombini && _selector) {
					_selector->drawToScreen(screen, px - 10, py - 10, lut);
				}
			}
		}
	}
}

void RescuePage::handleClick(const Common::Point &pos) {
	if (_phase != 0)
		return;

	// Scroll up/left button
	if (_scrollUpRect.contains(pos)) {
		if (_scrollOffset > 0) {
			_scrollOffset--;
			debug(2, "RescuePage: Scroll up/left, offset=%d", _scrollOffset);
		}
		return;
	}

	// Scroll down/right button
	if (_scrollDownRect.contains(pos)) {
		_scrollOffset++;
		debug(2, "RescuePage: Scroll down/right, offset=%d", _scrollOffset);
		return;
	}

	if (_rescueNum == 1) {
		// Direction selection — Rescue1 arrow click areas from IDA
		// Left direction: x in (519..617), y in (139..313)
		// Original: Rescue1__HandleInputAndAnimate_427CE0 at 0x427E2B
		if (pos.x > 519 && pos.x < 617 && pos.y > 139 && pos.y < 313) {
			debug(1, "RescuePage: Route left (Rescue1)");
			_engine->_zoombiniWalkingFlag = true;
			_engine->_routeDirection = 1;
			_phase = 1;
			_phaseTimer = _engine->getGameTickCount();
			return;
		}

		// Right direction: x in (624..723), y in (139..313)
		// Original at 0x427E9F
		if (pos.x > 624 && pos.x < 723 && pos.y > 139 && pos.y < 313) {
			debug(1, "RescuePage: Route right (Rescue1)");
			_engine->_zoombiniWalkingFlag = true;
			_engine->_routeDirection = 2;
			_phase = 1;
			_phaseTimer = _engine->getGameTickCount();
			return;
		}
	}

	// Grid slot click — select a zoombini from the grid
	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			int x = _gridBaseX + col * kSlotWidth;
			int y = _gridBaseY + row * kSlotHeight;
			Common::Rect slotHit(x + 24, y + 30, x + 64, y + 87);
			if (slotHit.contains(pos)) {
				int slotIdx = (col + _scrollOffset) * kGridRows + row;
				if (slotIdx >= 0 && slotIdx < (int)_engine->_globalZoombinis.size()) {
					_selectedZoombini = slotIdx;
					debug(2, "RescuePage: Selected zoombini %d at slot %d", slotIdx, slotIdx);
				} else {
					_selectedZoombini = -1;
				}
				return;
			}
		}
	}
}

} // End of namespace Zoombini2
