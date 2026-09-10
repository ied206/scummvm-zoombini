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

#include "zoombini2/graphics.h"
#include "zoombini2/pages/shelter_rescue1.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

ShelterRescueSite1::ShelterRescueSite1(Zoombini2Engine *vm)
	: ShelterRescueSiteBase(vm), _portal(nullptr), _portalTop(nullptr), _cramure(nullptr), _arrowLeftOff(nullptr), _arrowLeftOn(nullptr),
	  _arrowRightOff(nullptr), _arrowRightOn(nullptr), _arrowLeftPos(), _arrowRightPos(), _portalPos(), _portalTopPos(), _cramurePos() {
	_pageId = kPageRescue1;
}

ShelterRescueSite1::~ShelterRescueSite1() {
	saveRescueRoster(getRescueBoard());

	delete _portal;
	delete _portalTop;
	delete _cramure;
	delete _arrowLeftOff;
	delete _arrowLeftOn;
	delete _arrowRightOff;
	delete _arrowRightOn;
}

void ShelterRescueSite1::init() {
	debug(1, "ShelterRescueSite1::init");

	loadBackground("#bmp/rescue1/background");
	configureRosterLayout(Common::Point32(68, 95), Common::Rect(26, 240, 89, 326), Common::Rect(276, 240, 339, 319), Common::Point32(26, 240),
						  Common::Point32(276, 240));

	_arrowLeftPos = Common::Point32(525, 248);
	_arrowRightPos = Common::Point32(641, 253);
	_portalPos = Common::Point32(520, 84);
	_portalTopPos = Common::Point32(518, 99);
	_cramurePos = Common::Point32(311, 99);

	loadSelector("bmp/rescue1/SELECTOR.RB");

	_portal = new RleBlock();
	_portal->loadFromFile(Common::Path("bmp/rescue1/PORTE.RB"));

	_portalTop = new RleBlock();
	_portalTop->loadFromFile(Common::Path("bmp/rescue1/portal_top.rb"));

	loadPorteSelector("bmp/rescue1/porte_select.rb");

	_cramure = new RleBlock();
	_cramure->loadFromFile(Common::Path("bmp/rescue1/CRAMURE.RB"));

	_arrowLeftOff = new BitBlock();
	_arrowLeftOff->loadFromBB(Common::Path("bmp/rescue1/inside_arrow_left_off.bb"));

	_arrowLeftOn = new BitBlock();
	_arrowLeftOn->loadFromBB(Common::Path("bmp/rescue1/inside_arrow_left_on.bb"));

	_arrowRightOff = new BitBlock();
	_arrowRightOff->loadFromBB(Common::Path("bmp/rescue1/inside_arrow_right_off.bb"));

	_arrowRightOn = new BitBlock();
	_arrowRightOn->loadFromBB(Common::Path("bmp/rescue1/inside_arrow_right_on.bb"));

	loadScrollButtons("bmp/rescue1/button_left.an", "bmp/rescue1/button_right.an");
	startMusic("#sounds/music/C1-BB02.wav");
	resetRescueState();
	initRescueRoster();
	loadRosterAnimation();
}

void ShelterRescueSite1::initRescueRoster() {
	GameState *state = _vm->getGameState();
	BoardRecord **board = state->_rescue1Board;
	prepareWaitingBoard(board);
	_vm->_routeDirection = RouteBranch::kNone00;
	if (_vm->_isSavedGame) {
		state->_hasReachedRescue1 = 1;
		state->_rescue1ArrivalCount += _vm->_globalZoombinis.size();
		state->registerPageVisit(kPageRescue1);
	}
	refillDepartureRoster(board);
}

bool ShelterRescueSite1::canUseGoButton() const {
	return hasFullDepartureParty() && (_vm->_routeDirection == RouteBranch::kLeft01 || _vm->_routeDirection == RouteBranch::kRight02);
}

BoardRecord **ShelterRescueSite1::getRescueBoard() const {
	return _vm->getGameState()->_rescue1Board;
}

void ShelterRescueSite1::onRenderSite(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();
	if (_portal && _portal->isValid()) {
		const int portalY = isDeparting() ? -130 : _portalPos.y;
		_portal->drawToScreen(screen, Common::Point32(_portalPos.x, portalY), lut);
	}

	if (_portalTop && _portalTop->isValid())
		_portalTop->drawToScreen(screen, _portalTopPos, lut);
	if (_cramure && _cramure->isValid())
		_cramure->drawToScreen(screen, _cramurePos, lut);

	if (_vm->_routeDirection == RouteBranch::kLeft01) {
		if (_arrowLeftOn)
			_arrowLeftOn->drawToSurface(screen, _arrowLeftPos);
		if (_arrowRightOff)
			_arrowRightOff->drawToSurface(screen, _arrowRightPos);
	} else if (_vm->_routeDirection == RouteBranch::kRight02) {
		if (_arrowLeftOff)
			_arrowLeftOff->drawToSurface(screen, _arrowLeftPos);
		if (_arrowRightOn)
			_arrowRightOn->drawToSurface(screen, _arrowRightPos);
	} else {
		if (_arrowLeftOff)
			_arrowLeftOff->drawToSurface(screen, _arrowLeftPos);
		if (_arrowRightOff)
			_arrowRightOff->drawToSurface(screen, _arrowRightPos);
	}
}

EventHandleResult ShelterRescueSite1::onSiteLButtonDown(const Common::Point &pos) {
	if (hasFullDepartureParty() && 519 < pos.x && pos.x < 617 && 139 < pos.y && pos.y < 313) {
		debug(1, "ShelterRescueSite1: Route left");
		_vm->_zoombiniWalkingFlag = true;
		_vm->_routeDirection = RouteBranch::kLeft01;
		beginDeparture();
		return EventHandleResult::kConsumed;
	}

	if (hasFullDepartureParty() && 624 < pos.x && pos.x < 723 && 139 < pos.y && pos.y < 313) {
		debug(1, "ShelterRescueSite1: Route right");
		_vm->_zoombiniWalkingFlag = true;
		_vm->_routeDirection = RouteBranch::kRight02;
		beginDeparture();
		return EventHandleResult::kConsumed;
	}

	return EventHandleResult::kPassthrough;
}

} // End of namespace Zoombini2
