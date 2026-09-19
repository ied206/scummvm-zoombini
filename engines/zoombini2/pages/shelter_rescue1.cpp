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
#include "zoombini2/scripts.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *ShelterRescueSite1::kBackgroundPath;
constexpr const char *ShelterRescueSite1::kSelectorPath;
constexpr const char *ShelterRescueSite1::kPortalPath;
constexpr const char *ShelterRescueSite1::kPortalTopPath;
constexpr const char *ShelterRescueSite1::kPorteSelectorPath;
constexpr const char *ShelterRescueSite1::kCramurePath;
constexpr const char *ShelterRescueSite1::kArrowLeftOffPath;
constexpr const char *ShelterRescueSite1::kArrowLeftOnPath;
constexpr const char *ShelterRescueSite1::kArrowRightOffPath;
constexpr const char *ShelterRescueSite1::kArrowRightOnPath;
constexpr const char *ShelterRescueSite1::kScrollLeftPath;
constexpr const char *ShelterRescueSite1::kScrollRightPath;
constexpr const char *ShelterRescueSite1::kMusicPath;
constexpr const char *ShelterRescueSite1::kLittleZombAnimationPath;
constexpr const char *ShelterRescueSite1::kPickupZombAnimationPath;
constexpr const char *ShelterRescueSite1::kIdleZombAnimationPath;
constexpr const char *ShelterRescueSite1::kAreaMaskPath;
constexpr const char *ShelterRescueSite1::kMissingArrivalsSpeechFormat;
constexpr const char *ShelterRescueSite1::kFirstArrivalSpeechPath;
constexpr const char *ShelterRescueSite1::kFirstArrivalFollowupSpeechPath;
constexpr const char *ShelterRescueSite1::kFirstDepartureSpeechPath;
constexpr const char *ShelterRescueSite1::kReadyDepartureSpeechPath;
constexpr const char *ShelterRescueSite1::kIncompleteDepartureSpeechPath;

constexpr Common::Point32 ShelterRescueSite1::kRosterGridBasePos;
constexpr Common::Point32 ShelterRescueSite1::kRosterButtonUpPos;
constexpr Common::Point32 ShelterRescueSite1::kRosterButtonDownPos;

constexpr Common::Point32 ShelterRescueSite1::kSeatPositions[kDepartureSeatCount];
constexpr Common::Point32 ShelterRescueSite1::kFloorPositions[kDepartureSeatCount];

ShelterRescueSite1::ShelterRescueSite1(Zoombini2Engine *vm)
	: ShelterRescueSiteBase(vm) {
	_pageId = kPageRescue1;
}

ShelterRescueSite1::~ShelterRescueSite1() {
	saveRescueRoster(getRescueBoard());

	_vm->setHoverCursorActive(false);
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

	if (!_vm->_gfx->loadBackground(Common::Path(kBackgroundPath)))
		warning("ShelterRescueSite1: Failed to load background");
	_vm->getScreen()->fillRect(Common::Rect32(ManagedSurface32::kScreenSize.width, ManagedSurface32::kScreenSize.height), 0);
	_vm->_gfx->drawBackground(_vm->getScreen(), Common::Point32(0, 0));
	configureRosterLayout(kRosterGridBasePos, _rosterScrollUpRect, _rosterScrollDownRect, kRosterButtonUpPos, kRosterButtonDownPos);

	_arrowLeftPos = Common::Point32(525, 248);
	_arrowRightPos = Common::Point32(641, 253);
	_portalPos = Common::Point32(520, 84);
	_portalTopPos = Common::Point32(518, 99);
	_cramurePos = Common::Point32(311, 99);

	loadSelector(kSelectorPath);

	_portal = new RleBlock(_vm);
	_portal->loadFromFile(Common::Path(kPortalPath));

	_portalTop = new RleBlock(_vm);
	_portalTop->loadFromFile(Common::Path(kPortalTopPath));

	loadPorteSelector(kPorteSelectorPath);

	_cramure = new RleBlock(_vm);
	_cramure->loadFromFile(Common::Path(kCramurePath));

	_arrowLeftOff = new BitBlock(_vm);
	_arrowLeftOff->loadFromBB(Common::Path(kArrowLeftOffPath));

	_arrowLeftOn = new BitBlock(_vm);
	_arrowLeftOn->loadFromBB(Common::Path(kArrowLeftOnPath));

	_arrowRightOff = new BitBlock(_vm);
	_arrowRightOff->loadFromBB(Common::Path(kArrowRightOffPath));

	_arrowRightOn = new BitBlock(_vm);
	_arrowRightOn->loadFromBB(Common::Path(kArrowRightOnPath));

	loadScrollButtons(kScrollLeftPath, kScrollRightPath);
	startMusic(kMusicPath);
	resetRescueState();
	initRescueRoster();

	_littleZombAnimation = _vm->loadZoombiniAnimation(Common::Path(kLittleZombAnimationPath), 50);
	if (!_littleZombAnimation)
		warning("ShelterRescueSite1: Failed to load littleZomb.anm");
	_pickupZombAnimation = _vm->loadZoombiniAnimation(Common::Path(kPickupZombAnimationPath), 100);
	if (!_pickupZombAnimation)
		warning("ShelterRescueSite1: Failed to load pris.anm");
	_idleZombAnimation = _vm->loadZoombiniAnimation(Common::Path(kIdleZombAnimationPath), 50);
	if (!_idleZombAnimation)
		warning("ShelterRescueSite1: Failed to load attenteZomb2.anm");

	loadAreaMask(Common::Path(kAreaMaskPath));

	buildDropTargets();
	refillBoardingRoster();
	loadRosterAnimation();
}

void ShelterRescueSite1::initRescueRoster() {
	GameState *state = _vm->getGameState();
	BoardRecord **board = state->_rescue1Board;
	prepareWaitingBoard(board);
	_scrollRow = GameState::findBoardScrollRow(board);
	_vm->_routeDirection = RouteBranch::kNone00;
	if (_vm->_isSavedGame) {
		state->_hasReachedRescue1 = 1;
		const int oldArrivalCount = state->_rescue1ArrivalCount;
		state->_rescue1ArrivalCount += _vm->_globalZoombinis.size();
		const int arrivalCount = state->_rescue1ArrivalCount;
		const bool hadVisit = state->hasPageVisit(kPageRescue1, 1);
		state->registerPageVisit(kPageRescue1);
		if (arrivalCount < 8) {
			if (hadVisit)
				enqueueSpeech(Common::String::format(kMissingArrivalsSpeechFormat, 8 - arrivalCount));
			else {
				enqueueSpeech(kFirstArrivalSpeechPath);
				enqueueSpeech(kFirstArrivalFollowupSpeechPath);
			}
		} else if (oldArrivalCount < 8) {
			enqueueSpeech(kFirstDepartureSpeechPath);
		} else if (countBoardMembers(board) + static_cast<int>(_vm->_globalZoombinis.size()) >= 8) {
			enqueueSpeech(kReadyDepartureSpeechPath);
		} else {
			enqueueSpeech(kIncompleteDepartureSpeechPath);
		}
	}
	_shipVisible = 8 <= state->_rescue1ArrivalCount;
}

void ShelterRescueSite1::refillBoardingRoster() {
	GameState::refillFromBoard(getRescueBoard(), _vm->_globalZoombinis, 8);
	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _vm->_globalZoombinis[i];
		zoombini->_puzzleStatus = 0;
		zoombini->_inputEnabled = 1;
		zoombini->_dragging = false;
		zoombini->_idleAnimationEnabled = true;
		zoombini->_hidden = false;
		zoombini->setDefaultAnimation(_littleZombAnimation, 33);
		if (i < kDepartureSeatCount) {
			// Seated members stand 36 pixels above the seat-rect origin, matching the seat drop snap.
			zoombini->setPosition(Common::Point32(kSeatPositions[i].x, kSeatPositions[i].y - 36));
			_dropTargets[kSeatTargetBase + i].occupied = true;
			_dropTargets[kSeatTargetBase + i].zoombiniIndex = static_cast<int>(i);
		} else if (i - kDepartureSeatCount < kDepartureSeatCount) {
			zoombini->setPosition(kFloorPositions[i - kDepartureSeatCount]);
		} else {
			zoombini->setPosition(kFloorPositions[kDepartureSeatCount - 1]);
		}
	}
	refreshGridOccupancy();
}

void ShelterRescueSite1::buildDropTargets() {
	_dropTargets.clear();
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 5; row++) {
			ZoombiniDropTarget target;
			target.rect = Common::Rect32(kRosterGridBasePos.x + col * 40 + 18, kRosterGridBasePos.y + row * 57 + 30,
										 kRosterGridBasePos.x + col * 40 + 58, kRosterGridBasePos.y + row * 57 + 87);
			target.occupied = false;
			target.callback = gridDropCallback;
			target.callbackContext = this;
			target.zoombiniIndex = -1;
			_dropTargets.push_back(target);
		}
	}
	for (int seat = 0; seat < kDepartureSeatCount; seat++) {
		ZoombiniDropTarget target;
		target.rect = Common::Rect32(kSeatPositions[seat].x, kSeatPositions[seat].y,
									 kSeatPositions[seat].x + 61, kSeatPositions[seat].y + 23);
		target.occupied = false;
		target.callback = seatDropCallback;
		target.callbackContext = this;
		target.zoombiniIndex = -1;
		_dropTargets.push_back(target);
	}
}

void ShelterRescueSite1::refreshGridOccupancy() {
	BoardRecord *const *board = getRescueBoard();
	for (int record = 0; record < kGridTargetCount; record++) {
		const int boardIndex = (_scrollRow + record / 5) * kBoardCols + record % 5;
		const bool occupied = 0 <= boardIndex && boardIndex < kBoardRows * kBoardCols && board[boardIndex];
		_dropTargets[record].occupied = occupied;
		_dropTargets[record].zoombiniIndex = occupied ? 0 : -1;
	}
}

int ShelterRescueSite1::getGridRecordBoardIndex(int recordIndex) const {
	return (_scrollRow + recordIndex / 5) * kBoardCols + recordIndex % 5;
}

bool ShelterRescueSite1::seatsFullyOccupied() const {
	for (int seat = 0; seat < kDepartureSeatCount; seat++) {
		if (!_dropTargets[kSeatTargetBase + seat].occupied)
			return false;
	}
	return true;
}

ZoombiniRunner *ShelterRescueSite1::getDraggedZoombini() const {
	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		if (_vm->_globalZoombinis[i]->_dragging)
			return _vm->_globalZoombinis[i];
	}
	return nullptr;
}

bool ShelterRescueSite1::isDepartingMember(const ZoombiniRunner *zoombini) const {
	for (int seat = 0; seat < kDepartureSeatCount; seat++) {
		const ZoombiniDropTarget &target = _dropTargets[kSeatTargetBase + seat];
		if (target.occupied && 0 <= target.zoombiniIndex && static_cast<uint>(target.zoombiniIndex) < _vm->_globalZoombinis.size() &&
			_vm->_globalZoombinis[target.zoombiniIndex] == zoombini)
			return true;
	}
	return false;
}

void ShelterRescueSite1::gridDropCallback(void *context, int targetIndex, int zoombiniIndex) {
	ShelterRescueSite1 *page = static_cast<ShelterRescueSite1 *>(context);
	if (!page || targetIndex < 0 || kGridTargetCount <= targetIndex)
		return;
	ZoombiniDropTarget &target = page->_dropTargets[targetIndex];
	if (!target.occupied)
		return;
	if (0 <= zoombiniIndex && static_cast<uint>(zoombiniIndex) < page->_vm->_globalZoombinis.size())
		page->_vm->_globalZoombinis[zoombiniIndex]->setPosition(Common::Point32(target.rect.left - 8, target.rect.top - 15));
	page->captureToBoard(targetIndex, zoombiniIndex);
}

void ShelterRescueSite1::seatDropCallback(void *context, int targetIndex, int zoombiniIndex) {
	ShelterRescueSite1 *page = static_cast<ShelterRescueSite1 *>(context);
	if (!page || targetIndex < kSeatTargetBase || kDropTargetCount <= targetIndex)
		return;
	if (zoombiniIndex < 0 || page->_vm->_globalZoombinis.size() <= static_cast<uint>(zoombiniIndex))
		return;
	const ZoombiniDropTarget &target = page->_dropTargets[targetIndex];
	page->_vm->_globalZoombinis[zoombiniIndex]->setPosition(Common::Point32(target.rect.left, target.rect.top - 36));
}

void ShelterRescueSite1::captureToBoard(int recordIndex, int zoombiniIndex) {
	if (zoombiniIndex < 0 || _vm->_globalZoombinis.size() <= static_cast<uint>(zoombiniIndex))
		return;
	const int boardIndex = getGridRecordBoardIndex(recordIndex);
	BoardRecord **board = getRescueBoard();
	if (boardIndex < 0 || kBoardRows * kBoardCols <= boardIndex)
		return;
	ZoombiniRunner *zoombini = _vm->_globalZoombinis[zoombiniIndex];
	delete board[boardIndex];
	board[boardIndex] = new BoardRecord();
	board[boardIndex]->store(*zoombini);
	delete zoombini;
	_vm->_globalZoombinis.remove_at(zoombiniIndex);
	for (uint record = 0; record < _dropTargets.size(); record++) {
		if (_dropTargets[record].zoombiniIndex > zoombiniIndex)
			_dropTargets[record].zoombiniIndex -= 1;
	}
	refreshGridOccupancy();
}

bool ShelterRescueSite1::hasBoardCellsInRows(int firstRow, int lastRow) const {
	BoardRecord *const *board = getRescueBoard();
	for (int row = firstRow; row <= lastRow; row++) {
		if (row < 0 || kBoardRows <= row)
			continue;
		for (int col = 0; col < kBoardCols; col++) {
			if (board[row * kBoardCols + col])
				return true;
		}
	}
	return false;
}

bool ShelterRescueSite1::materializeFromBoard(int gridCol, int gridRow, const Common::Point &pointerPos) {
	BoardRecord *const *board = getRescueBoard();
	const int boardIndex = (_scrollRow + gridCol) * kBoardCols + gridRow;
	if (boardIndex < 0 || kBoardRows * kBoardCols <= boardIndex || !board[boardIndex])
		return false;
	const uint32 tick = _vm->getGameTickCount();
	ZoombiniRunner *zoombini = new ZoombiniRunner();
	zoombini->setTraits(board[boardIndex]->getTraits());
	Common::strlcpy(zoombini->_name, board[boardIndex]->_name, sizeof(zoombini->_name));
	zoombini->setDefaultAnimation(_littleZombAnimation, 33);
	zoombini->setPosition(Common::Point32(kRosterGridBasePos.x + gridCol * 40 + 24, kRosterGridBasePos.y + gridRow * 57 + 30));
	zoombini->_dragOrigin = zoombini->_screenPos;
	zoombini->_dragOffset = Common::Point32(pointerPos.x - zoombini->_screenPos.x - 3, pointerPos.y - zoombini->_screenPos.y - 10);
	zoombini->setPosition(Common::Point32(zoombini->_screenPos.x + zoombini->_dragOffset.x, zoombini->_screenPos.y + zoombini->_dragOffset.y));
	zoombini->_dragging = true;
	zoombini->startAnimation(_pickupZombAnimation, 33, tick);
	_vm->_globalZoombinis.push_back(zoombini);
	delete board[boardIndex];
	getRescueBoard()[boardIndex] = nullptr;
	refreshGridOccupancy();
	return true;
}

void ShelterRescueSite1::triggerScrollUp() {
	if (0 < _scrollRow && hasBoardCellsInRows(0, _scrollRow)) {
		_scrollPhase = kScrollPhaseUp04;
		_scrollPixelsLeft = kScrollPixelLength;
	}
}

void ShelterRescueSite1::triggerScrollDown() {
	if (_scrollRow + 4 < kBoardRows && hasBoardCellsInRows(_scrollRow + 3, kBoardRows - 1)) {
		_scrollPhase = kScrollPhaseDown06;
		_scrollPixelsLeft = kScrollPixelLength;
	}
}

void ShelterRescueSite1::stepScrollAnimation() {
	_scrollPixelsLeft -= kScrollPixelStep;
	if (_scrollPhase == kScrollPhaseUp04) {
		_scrollBgX -= kScrollPixelStep;
		if (_scrollBgX < 0)
			_scrollBgX = 223;
	} else {
		_scrollBgX += kScrollPixelStep;
		if (223 < _scrollBgX)
			_scrollBgX = 0;
	}
	if (_scrollPixelsLeft <= 0) {
		if (_scrollPhase == kScrollPhaseUp04)
			_scrollRow -= 1;
		else
			_scrollRow += 1;
		_scrollPhase = kScrollIdle;
		refreshGridOccupancy();
	}
}

void ShelterRescueSite1::updateHoverCursor() {
	const Common::Point32 mousePos = _vm->getMousePos();
	BoardRecord *const *board = getRescueBoard();
	bool hover = false;
	for (int col = 0; col < 4 && !hover; col++) {
		for (int row = 0; row < 5 && !hover; row++) {
			const int left = kRosterGridBasePos.x + col * 40;
			const int top = kRosterGridBasePos.y + row * 57;
			if (mousePos.x <= left + 24 || left + 64 <= mousePos.x || mousePos.y <= top + 30 || top + 87 <= mousePos.y)
				continue;
			const int boardIndex = (_scrollRow + col) * kBoardCols + row;
			if (0 <= boardIndex && boardIndex < kBoardRows * kBoardCols && board[boardIndex])
				hover = true;
		}
	}
	_vm->setHoverCursorActive(hover);
}

void ShelterRescueSite1::onUpdate() {
	pumpSpeechQueue();
	updateHoverCursor();
	refreshGridOccupancy();
	if (_scrollPhase != kScrollIdle)
		stepScrollAnimation();
	if (_departurePrompted) {
		_departurePrompted = false;
		_vm->_mapTransitionSourcePageId = static_cast<PageId>(_pageId);
		_vm->requestPageChange(kPageMapTrans);
	}
}

bool ShelterRescueSite1::canUseGoButton() const {
	return seatsFullyOccupied() && (_vm->_routeDirection == RouteBranch::kLeft01 || _vm->_routeDirection == RouteBranch::kRight02);
}

BoardRecord **ShelterRescueSite1::getRescueBoard() const {
	return _vm->getGameState()->_rescue1Board;
}

void ShelterRescueSite1::buildBoardingDrawOrder(Common::Array<uint> &order, ZoombiniRunner *&draggedZoombini) const {
	order.clear();
	draggedZoombini = nullptr;
	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		if (_vm->_globalZoombinis[i]->_dragging) {
			draggedZoombini = _vm->_globalZoombinis[i];
			continue;
		}
		order.push_back(i);
	}
	for (uint i = 0; i < order.size(); i++) {
		for (uint j = i + 1; j < order.size(); j++) {
			if (_vm->_globalZoombinis[order[j]]->_screenPos.y < _vm->_globalZoombinis[order[i]]->_screenPos.y)
				SWAP(order[i], order[j]);
		}
	}
}

void ShelterRescueSite1::drawBoardingActives(ManagedSurface32 *screen) const {
	Common::Array<uint> order;
	ZoombiniRunner *draggedZoombini = nullptr;
	buildBoardingDrawOrder(order, draggedZoombini);
	for (uint i = 0; i < order.size(); i++) {
		ZoombiniRunner *zoombini = _vm->_globalZoombinis[order[i]];
		if (zoombini->_hidden)
			continue;
		zoombini->tryStartIdleAnimation(_idleZombAnimation, *_vm->_rnd, _vm->getGameTickCount());
		_vm->_gfx->drawZoombiniRunner(screen, zoombini);
		zoombini->advanceAnimationAfterDraw();
	}
	if (draggedZoombini && !draggedZoombini->_hidden) {
		_vm->_gfx->drawZoombiniRunner(screen, draggedZoombini);
		draggedZoombini->advanceAnimationAfterDraw();
	}
}

void ShelterRescueSite1::drawWaitingBoard(ManagedSurface32 *screen, int pixelShiftX) const {
	if (!_littleZombAnimation)
		return;
	BoardRecord *const *board = getRescueBoard();
	int startCol = 0;
	if (0 < _scrollRow)
		startCol = -1;
	int maxCol = 6;
	if (_scrollRow + 5 >= kBoardRows)
		maxCol = 5;
	if (_scrollRow + 4 >= kBoardRows)
		maxCol = 4;
	const Common::Rect32 clip(kRosterGridBasePos.x, 0, kRosterGridBasePos.x + 223, ManagedSurface32::kScreenSize.height);
	for (int col = startCol; col < maxCol; col++) {
		for (int row = 0; row < 5; row++) {
			const int boardIndex = (_scrollRow + col) * kBoardCols + row;
			if (boardIndex < 0 || kBoardRows * kBoardCols <= boardIndex || !board[boardIndex])
				continue;
			_vm->_gfx->drawZoombini(screen, _littleZombAnimation, board[boardIndex]->getTraits(),
								   Common::Point32(kRosterGridBasePos.x + col * 40 + 18 + pixelShiftX, kRosterGridBasePos.y + row * 57 + 30), 33, 0, &clip);
		}
	}
}

void ShelterRescueSite1::onRenderContent(ManagedSurface32 *screen) {
	// Recompose every frame because the site and actors are redrawn even when unchanged.
	// Restore their underlying pixels before blending translucent edges or moving sprites.
	_vm->_gfx->drawBackground(screen, Common::Point32(0, 0));
	onRenderSite(screen);

	if (_scrollPhase != kScrollIdle && _vm->_gfx->hasBackground()) {
		const int shift = (_scrollPhase == kScrollPhaseUp04 ? 1 : -1) * (kScrollPixelLength - _scrollPixelsLeft);
		_vm->_gfx->fillRect(screen, Common::Rect32(kRosterGridBasePos.x, 0, kRosterGridBasePos.x + 223, 600), 0);
		_vm->_gfx->drawBackgroundSubRect(screen, Common::Point32(kRosterGridBasePos.x, 0), Common::Rect(_scrollBgX, 0, _scrollBgX + 223, 600));
		drawWaitingBoard(screen, shift);
	} else {
		drawWaitingBoard(screen, 0);
	}
	// The original draws the door-selection overlay over the waiting grid on every grid update, beneath the scroll controls.
	if (RleBlock *porteSelect = getPorteSelector())
		porteSelect->drawToScreen(screen, kRosterGridBasePos, _vm->getAlphaLUT());

	// The original shows frame 1 on an idle button and frame 0 only while that
	// button's own scroll animation is running.
	if (_buttonUp && 0 < _buttonUp->getFrameCount()) {
		const int frame = (1 < _buttonUp->getFrameCount()) ? ((_scrollPhase == kScrollPhaseUp04) ? 0 : 1) : 0;
		const RleBlock *frameBlock = _buttonUp->getFrame(frame);
		if (frameBlock)
			frameBlock->drawToScreen(screen, _buttonUpPos, _vm->getAlphaLUT());
	}

	if (_buttonDown && 0 < _buttonDown->getFrameCount()) {
		const int frame = (1 < _buttonDown->getFrameCount()) ? ((_scrollPhase == kScrollPhaseDown06) ? 0 : 1) : 0;
		const RleBlock *frameBlock = _buttonDown->getFrame(frame);
		if (frameBlock)
			frameBlock->drawToScreen(screen, _buttonDownPos, _vm->getAlphaLUT());
	}

	drawBoardingActives(screen);
}

void ShelterRescueSite1::onRenderSite(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();
	// The original draws the closed stone door at the portal position, and slides it up off-screen once the ship arrives.
	if (_portal && _portal->isValid()) {
		if (_shipVisible)
			_portal->drawToScreenClipped(screen, Common::Point32(_portalPos.x, -130), Common::Rect32(0, 100, 800, 600), lut);
		else
			_portal->drawToScreen(screen, _portalPos, lut);
	}

	if (_portalTop && _portalTop->isValid())
		_portalTop->drawToScreen(screen, _portalTopPos, lut);
	// The original never draws the cramure sprite; it only sizes a background snapshot. Drawing it here would smear smoke streaks over the machine.

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

EventHandleResult ShelterRescueSite1::onLButtonDown(const Common::Point &pos) {
	// The original picks members up on the pressed frame, so start the drag here.
	// Releases only finish a drag already in progress through onLButtonUp.
	const ZoombiniInputResult inputResult = ZoombiniRunner::handlePointerInput(_vm->_globalZoombinis, Common::Point32(pos.x, pos.y), true,
																			   _pickupZombAnimation, _vm->getGameTickCount(), &_dropTargets, getAreaMask());
	return inputResult != ZoombiniInputResult::kIgnored00 ? EventHandleResult::kConsumed : EventHandleResult::kPassthrough;
}

EventHandleResult ShelterRescueSite1::onLButtonUp(const Common::Point &pos) {
	if (_departurePrompted)
		return EventHandleResult::kConsumed;
	refreshGridOccupancy();
	const uint32 tick = _vm->getGameTickCount();
	const ZoombiniInputResult inputResult = ZoombiniRunner::handlePointerInput(_vm->_globalZoombinis, Common::Point32(pos.x, pos.y), true,
																			   _pickupZombAnimation, tick, &_dropTargets, getAreaMask());
	if (inputResult != ZoombiniInputResult::kIgnored00)
		return EventHandleResult::kConsumed;
	if (_scrollPhase != kScrollIdle || getDraggedZoombini())
		return EventHandleResult::kPassthrough;

	if (seatsFullyOccupied()) {
		if (519 < pos.x && pos.x < 617 && 139 < pos.y && pos.y < 313) {
			debug(1, "ShelterRescueSite1: Route left");
			_vm->_zoombiniWalkingFlag = true;
			_vm->_routeDirection = RouteBranch::kLeft01;
			_departurePrompted = true;
			return EventHandleResult::kConsumed;
		}

		if (624 < pos.x && pos.x < 723 && 139 < pos.y && pos.y < 313) {
			debug(1, "ShelterRescueSite1: Route right");
			_vm->_zoombiniWalkingFlag = true;
			_vm->_routeDirection = RouteBranch::kRight02;
			_departurePrompted = true;
			return EventHandleResult::kConsumed;
		}
	}

	if (_rosterScrollUpRect.contains(pos)) {
		triggerScrollUp();
		return EventHandleResult::kConsumed;
	}

	if (_rosterScrollDownRect.contains(pos)) {
		triggerScrollDown();
		return EventHandleResult::kConsumed;
	}

	BoardRecord *const *board = getRescueBoard();
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 5; row++) {
			const Common::Rect slotHit(kRosterGridBasePos.x + col * 40 + 24, kRosterGridBasePos.y + row * 57 + 30,
									   kRosterGridBasePos.x + col * 40 + 64, kRosterGridBasePos.y + row * 57 + 87);
			if (slotHit.contains(pos)) {
				const int boardIndex = (_scrollRow + col) * kBoardCols + row;
				if (0 <= boardIndex && boardIndex < kBoardRows * kBoardCols && board[boardIndex])
					materializeFromBoard(col, row, pos);
				return EventHandleResult::kConsumed;
			}
		}
	}
	return EventHandleResult::kPassthrough;
}

EventHandleResult ShelterRescueSite1::onMouseMove(const Common::Point &pos) {
	const ZoombiniInputResult result = ZoombiniRunner::handlePointerInput(_vm->_globalZoombinis, Common::Point32(pos.x, pos.y), false,
																		  _pickupZombAnimation, _vm->getGameTickCount(), &_dropTargets, getAreaMask());
	return result == ZoombiniInputResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

} // End of namespace Zoombini2
