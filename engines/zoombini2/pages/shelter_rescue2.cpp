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

#include "zoombini2/pages/shelter_rescue2.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

ShelterRescueSite2::ShelterRescueSite2(Zoombini2Engine *vm) : ShelterRescueSiteBase(vm) {
	_pageId = kPageRescue2;
}

ShelterRescueSite2::~ShelterRescueSite2() {
	saveRescueRoster(getRescueBoard());
}

void ShelterRescueSite2::init() {
	debug(1, "ShelterRescueSite2::init");

	loadBackground("#bmp/rescue2/background");
	configureRosterLayout(Common::Point32(75, 193), Common::Rect(20, 332, 83, 418), Common::Rect(289, 332, 352, 411), Common::Point32(20, 332),
						  Common::Point32(289, 332));
	loadSelector("bmp/rescue2/selector.rb");
	loadPorteSelector("bmp/rescue2/porte_select.rb");
	loadScrollButtons("bmp/rescue2/button_left.an", "bmp/rescue2/button_right.an");
	startMusic("#sounds/music/C2-BB02.wav");
	resetRescueState();
	initRescueRoster();
	loadRosterAnimation();
}

void ShelterRescueSite2::initRescueRoster() {
	GameState *state = _vm->getGameState();
	BoardRecord **board = state->_rescue2Board;
	prepareWaitingBoard(board);
	if (_vm->_isSavedGame)
		state->_hasReachedRescue2 = 1;
	refillDepartureRoster(board);
	state->registerPageVisit(kPageRescue2);
}

bool ShelterRescueSite2::canUseGoButton() const {
	return hasFullDepartureParty();
}

BoardRecord **ShelterRescueSite2::getRescueBoard() const {
	return _vm->getGameState()->_rescue2Board;
}

} // End of namespace Zoombini2
