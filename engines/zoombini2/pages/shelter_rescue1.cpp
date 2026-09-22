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
	saveRescueRoster(getRescueStorage());

	_vm->setHoverCursorActive(false);
}

void ShelterRescueSite1::init() {
	debug(1, "ShelterRescueSite1::init");

	if (!_vm->_gfx->loadBackground(kBackgroundPath))
		warning("ShelterRescueSite1: Failed to load background");
	_vm->getScreen()->fillRect(Common::Rect32(ManagedSurface32::kScreenSize.width, ManagedSurface32::kScreenSize.height), 0);
	_vm->_gfx->drawBackground(_vm->getScreen(), Common::Point32(0, 0));
	configureRosterLayout(kRosterGridBasePos, _rosterScrollUpRect, _rosterScrollDownRect, kRosterButtonUpPos, kRosterButtonDownPos);

	_arrowLeftPos = Common::Point32(525, 248);
	_arrowRightPos = Common::Point32(641, 253);
	_portalPos = Common::Point32(520, 84);
	_portalTopPos = Common::Point32(518, 99);

	loadSelector(kSelectorPath);

	_vm->_gfx->loadPageRleBlock(kPortalPath);
	_vm->_gfx->loadPageRleBlock(kPortalTopPath);

	_vm->_gfx->loadPageRleBlock(kPorteSelectorPath);

	_vm->_gfx->loadPageRleBlock(kCramurePath);
	_vm->_gfx->loadPageBitBlock(kArrowLeftOffPath);
	_vm->_gfx->loadPageBitBlock(kArrowLeftOnPath);
	_vm->_gfx->loadPageBitBlock(kArrowRightOffPath);
	_vm->_gfx->loadPageBitBlock(kArrowRightOnPath);

	loadScrollButtons(kScrollLeftPath, kScrollRightPath);
	startPageMusic(Common::Path(kMusicPath));
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
	GameState *state = _vm->_state;
	StorageRecord **storage = state->_rescue1Storage;
	prepareWaitingStorage(storage);
	_scrollRow = GameState::findStorageScrollRow(storage);
	_vm->_routeDirection = RouteBranch::kNone00;
	if (_vm->_isSavedGame) {
		state->_hasReachedRescue1 = 1;
		const int oldArrivalCount = state->_rescue1ArrivalCount;
		state->_rescue1ArrivalCount += _vm->_state->_activeZoombinis.size();
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
		} else if (8 <= countStorageMembers(storage) + _vm->_state->_activeZoombinis.size()) {
			enqueueSpeech(kReadyDepartureSpeechPath);
		} else {
			enqueueSpeech(kIncompleteDepartureSpeechPath);
		}
	}
	_shipVisible = 8 <= state->_rescue1ArrivalCount;
}

void ShelterRescueSite1::refillBoardingRoster() {
	GameState::refillFromStorage(getRescueStorage(), _vm->_state->_activeZoombinis, 8);
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[i];
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
			ZmbDropTarget target;
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
		ZmbDropTarget target;
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
	StorageRecord *const *storage = getRescueStorage();
	for (int record = 0; record < kGridTargetCount; record++) {
		const int storageIndex = (_scrollRow + record / 5) * kStorageCols + record % 5;
		const bool occupied = 0 <= storageIndex && storageIndex < kStorageRows * kStorageCols && storage[storageIndex];
		_dropTargets[record].occupied = occupied;
		_dropTargets[record].zoombiniIndex = occupied ? 0 : -1;
	}
}

int ShelterRescueSite1::getGridRecordStorageIndex(int recordIndex) const {
	return (_scrollRow + recordIndex / 5) * kStorageCols + recordIndex % 5;
}

bool ShelterRescueSite1::seatsFullyOccupied() const {
	for (int seat = 0; seat < kDepartureSeatCount; seat++) {
		if (!_dropTargets[kSeatTargetBase + seat].occupied)
			return false;
	}
	return true;
}

ZoombiniRunner *ShelterRescueSite1::getDraggedZoombini() const {
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		if (_vm->_state->_activeZoombinis[i]->_dragging)
			return _vm->_state->_activeZoombinis[i];
	}
	return nullptr;
}

bool ShelterRescueSite1::isDepartingMember(const ZoombiniRunner *zoombini) const {
	for (int seat = 0; seat < kDepartureSeatCount; seat++) {
		const ZmbDropTarget &target = _dropTargets[kSeatTargetBase + seat];
		if (target.occupied && 0 <= target.zoombiniIndex && static_cast<uint>(target.zoombiniIndex) < _vm->_state->_activeZoombinis.size() &&
			_vm->_state->_activeZoombinis[target.zoombiniIndex] == zoombini)
			return true;
	}
	return false;
}

void ShelterRescueSite1::gridDropCallback(void *context, int targetIndex, int zoombiniIndex) {
	ShelterRescueSite1 *page = static_cast<ShelterRescueSite1 *>(context);
	if (!page || targetIndex < 0 || kGridTargetCount <= targetIndex)
		return;
	ZmbDropTarget &target = page->_dropTargets[targetIndex];
	if (!target.occupied)
		return;
	if (0 <= zoombiniIndex && static_cast<uint>(zoombiniIndex) < page->_vm->_state->_activeZoombinis.size())
		page->_vm->_state->_activeZoombinis[zoombiniIndex]->setPosition(Common::Point32(target.rect.left - 8, target.rect.top - 15));
	page->captureToStorage(targetIndex, zoombiniIndex);
}

void ShelterRescueSite1::seatDropCallback(void *context, int targetIndex, int zoombiniIndex) {
	ShelterRescueSite1 *page = static_cast<ShelterRescueSite1 *>(context);
	if (!page || targetIndex < kSeatTargetBase || kDropTargetCount <= targetIndex)
		return;
	if (zoombiniIndex < 0 || page->_vm->_state->_activeZoombinis.size() <= static_cast<uint>(zoombiniIndex))
		return;
	const ZmbDropTarget &target = page->_dropTargets[targetIndex];
	page->_vm->_state->_activeZoombinis[zoombiniIndex]->setPosition(Common::Point32(target.rect.left, target.rect.top - 36));
}

void ShelterRescueSite1::captureToStorage(int recordIndex, int zoombiniIndex) {
	if (zoombiniIndex < 0 || _vm->_state->_activeZoombinis.size() <= static_cast<uint>(zoombiniIndex))
		return;
	const int storageIndex = getGridRecordStorageIndex(recordIndex);
	StorageRecord **storage = getRescueStorage();
	if (storageIndex < 0 || kStorageRows * kStorageCols <= storageIndex)
		return;
	ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[zoombiniIndex];
	delete storage[storageIndex];
	storage[storageIndex] = new StorageRecord();
	storage[storageIndex]->store(*zoombini);
	delete zoombini;
	_vm->_state->_activeZoombinis.remove_at(zoombiniIndex);
	for (uint record = 0; record < _dropTargets.size(); record++) {
		if (_dropTargets[record].zoombiniIndex > zoombiniIndex)
			_dropTargets[record].zoombiniIndex -= 1;
	}
	refreshGridOccupancy();
}

bool ShelterRescueSite1::hasStorageCellsInRows(int firstRow, int lastRow) const {
	StorageRecord *const *storage = getRescueStorage();
	for (int row = firstRow; row <= lastRow; row++) {
		if (row < 0 || kStorageRows <= row)
			continue;
		for (int col = 0; col < kStorageCols; col++) {
			if (storage[row * kStorageCols + col])
				return true;
		}
	}
	return false;
}

bool ShelterRescueSite1::materializeFromStorage(int gridCol, int gridRow, const Common::Point &pointerPos) {
	StorageRecord *const *storage = getRescueStorage();
	const int storageIndex = (_scrollRow + gridCol) * kStorageCols + gridRow;
	if (storageIndex < 0 || kStorageRows * kStorageCols <= storageIndex || !storage[storageIndex])
		return false;
	const uint32 tick = _vm->getGameTickCount();
	ZoombiniRunner *zoombini = new ZoombiniRunner();
	zoombini->_inputEnabled = true;
	zoombini->setTraits(storage[storageIndex]->getTraits());
	Common::strlcpy(zoombini->_name, storage[storageIndex]->_name, sizeof(zoombini->_name));
	zoombini->setDefaultAnimation(_littleZombAnimation, 33);
	zoombini->setPosition(Common::Point32(kRosterGridBasePos.x + gridCol * 40 + 24, kRosterGridBasePos.y + gridRow * 57 + 30));
	zoombini->_dragOrigin = zoombini->_screenPos;
	zoombini->_dragOffset = Common::Point32(pointerPos.x - zoombini->_screenPos.x - 3, pointerPos.y - zoombini->_screenPos.y - 10);
	zoombini->setPosition(Common::Point32(zoombini->_screenPos.x + zoombini->_dragOffset.x, zoombini->_screenPos.y + zoombini->_dragOffset.y));
	zoombini->_dragging = true;
	zoombini->startAnimation(_pickupZombAnimation, 33, tick);
	_vm->_state->_activeZoombinis.push_back(zoombini);
	delete storage[storageIndex];
	getRescueStorage()[storageIndex] = nullptr;
	refreshGridOccupancy();
	return true;
}

void ShelterRescueSite1::triggerScrollUp() {
	if (0 < _scrollRow && hasStorageCellsInRows(0, _scrollRow)) {
		_scrollPhase = kScrollPhaseUp04;
		_scrollPixelsLeft = kScrollPixelLength;
	}
}

void ShelterRescueSite1::triggerScrollDown() {
	if (_scrollRow + 4 < kStorageRows && hasStorageCellsInRows(_scrollRow + 3, kStorageRows - 1)) {
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
	StorageRecord *const *storage = getRescueStorage();
	bool hover = false;
	for (int col = 0; col < 4 && !hover; col++) {
		for (int row = 0; row < 5 && !hover; row++) {
			const int left = kRosterGridBasePos.x + col * 40;
			const int top = kRosterGridBasePos.y + row * 57;
			if (mousePos.x <= left + 24 || left + 64 <= mousePos.x || mousePos.y <= top + 30 || top + 87 <= mousePos.y)
				continue;
			const int storageIndex = (_scrollRow + col) * kStorageCols + row;
			if (0 <= storageIndex && storageIndex < kStorageRows * kStorageCols && storage[storageIndex])
				hover = true;
		}
	}
	_vm->setHoverCursorActive(hover);
}

void ShelterRescueSite1::onUpdate() {
	pumpSpeechQueue();
	updateHoverCursor();
	refreshGridOccupancy();
	const uint32 tick = _vm->getGameTickCount();
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++)
		_vm->_state->_activeZoombinis[i]->updateAnimation(tick);
	if (_scrollPhase != kScrollIdle)
		stepScrollAnimation();
	if (_departurePrompted) {
		_departurePrompted = false;
		_vm->_mapTransitionSourcePageId = _pageId;
		_vm->requestPageChange(kPageMapTrans);
	}
}

bool ShelterRescueSite1::canUseGoButton() const {
	return seatsFullyOccupied() && (_vm->_routeDirection == RouteBranch::kLeft01 || _vm->_routeDirection == RouteBranch::kRight02);
}

StorageRecord **ShelterRescueSite1::getRescueStorage() const {
	return _vm->_state->_rescue1Storage;
}

void ShelterRescueSite1::buildBoardingDrawOrder(Common::Array<uint> &order, ZoombiniRunner *&draggedZoombini) const {
	order.clear();
	draggedZoombini = nullptr;
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		if (_vm->_state->_activeZoombinis[i]->_dragging) {
			draggedZoombini = _vm->_state->_activeZoombinis[i];
			continue;
		}
		order.push_back(i);
	}
	ZoombiniRunner::sortDrawOrderByY(_vm->_state->_activeZoombinis, order);
}

void ShelterRescueSite1::drawBoardingActives(ManagedSurface32 *screen) const {
	Common::Array<uint> order;
	ZoombiniRunner *draggedZoombini = nullptr;
	buildBoardingDrawOrder(order, draggedZoombini);
	for (uint i = 0; i < order.size(); i++) {
		ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[order[i]];
		if (zoombini->_hidden)
			continue;
		zoombini->tryStartIdleAnimation(_idleZombAnimation, *_vm->_rnd, _vm->getGameTickCount(), _vm->getFrameDeltaMs(), _vm->getLogicPacingHz());
		_vm->_gfx->drawZoombiniRunner(screen, zoombini);
		zoombini->advanceAnimationAfterDraw();
	}
	if (draggedZoombini && !draggedZoombini->_hidden) {
		_vm->_gfx->drawZoombiniRunner(screen, draggedZoombini);
		draggedZoombini->advanceAnimationAfterDraw();
	}
}

void ShelterRescueSite1::drawWaitingStorage(ManagedSurface32 *screen, int pixelShiftX) const {
	if (!_littleZombAnimation)
		return;
	StorageRecord *const *storage = getRescueStorage();
	int startCol = 0;
	if (0 < _scrollRow)
		startCol = -1;
	int maxCol = 6;
	if (_scrollRow + 5 >= kStorageRows)
		maxCol = 5;
	if (_scrollRow + 4 >= kStorageRows)
		maxCol = 4;
	const Common::Rect32 clip(kRosterGridBasePos.x, 0, kRosterGridBasePos.x + 223, ManagedSurface32::kScreenSize.height);
	for (int col = startCol; col < maxCol; col++) {
		for (int row = 0; row < 5; row++) {
			const int storageIndex = (_scrollRow + col) * kStorageCols + row;
			if (storageIndex < 0 || kStorageRows * kStorageCols <= storageIndex || !storage[storageIndex])
				continue;
			_vm->_gfx->drawZoombini(screen, _littleZombAnimation, storage[storageIndex]->getTraits(),
									Common::Point32(kRosterGridBasePos.x + col * 40 + 18 + pixelShiftX, kRosterGridBasePos.y + row * 57 + 30), 33, 0, &clip);
		}
	}
}

void ShelterRescueSite1::onRenderContent(ManagedSurface32 *screen) {
	// Recompose every frame because the site and actors are redrawn even when unchanged.
	// Restore their underlying pixels before blending translucent edges or moving sprites.
	_vm->_gfx->drawBackground(screen, Common::Point32(0, 0));
	onRenderSite(screen);
	drawRosterBackdrop(screen, _scrollBgX);

	if (_scrollPhase != kScrollIdle) {
		const int shift = (_scrollPhase == kScrollPhaseUp04 ? 1 : -1) * (kScrollPixelLength - _scrollPixelsLeft);
		drawWaitingStorage(screen, shift);
	} else {
		drawWaitingStorage(screen, 0);
	}
	// The original draws the door-selection overlay over the waiting grid on every grid update, beneath the scroll controls.
	_vm->_gfx->drawPageRleBlock(screen, kPorteSelectorPath, kRosterGridBasePos);

	// The original shows frame 1 on an idle button and frame 0 only while that
	// button's own scroll animation is running.
	if (_buttonUp && 0 < _buttonUp->getFrameCount()) {
		int frame = 0;
		if (1 < _buttonUp->getFrameCount() && _scrollPhase != kScrollPhaseUp04)
			frame = 1;
		_vm->_gfx->drawAnimationFrame(screen, _buttonUp, frame, _buttonUpPos);
	}

	if (_buttonDown && 0 < _buttonDown->getFrameCount()) {
		int frame = 0;
		if (1 < _buttonDown->getFrameCount() && _scrollPhase != kScrollPhaseDown06)
			frame = 1;
		_vm->_gfx->drawAnimationFrame(screen, _buttonDown, frame, _buttonDownPos);
	}

	drawBoardingActives(screen);
}

void ShelterRescueSite1::onRenderSite(ManagedSurface32 *screen) {
	// The original draws the closed stone door at the portal position, and slides it up off-screen once the ship arrives.
	if (_shipVisible)
		_vm->_gfx->drawPageRleBlockClipped(screen, kPortalPath, Common::Point32(_portalPos.x, -130), Common::Rect32(0, 100, 800, 600));
	else
		_vm->_gfx->drawPageRleBlock(screen, kPortalPath, _portalPos);

	_vm->_gfx->drawPageRleBlock(screen, kPortalTopPath, _portalTopPos);
	// The cramure sprite is cached for site setup but is not composited over the machine.

	if (_vm->_routeDirection == RouteBranch::kLeft01) {
		_vm->_gfx->drawPageBitBlock(screen, kArrowLeftOnPath, _arrowLeftPos);
		_vm->_gfx->drawPageBitBlock(screen, kArrowRightOffPath, _arrowRightPos);
	} else if (_vm->_routeDirection == RouteBranch::kRight02) {
		_vm->_gfx->drawPageBitBlock(screen, kArrowLeftOffPath, _arrowLeftPos);
		_vm->_gfx->drawPageBitBlock(screen, kArrowRightOnPath, _arrowRightPos);
	} else {
		_vm->_gfx->drawPageBitBlock(screen, kArrowLeftOffPath, _arrowLeftPos);
		_vm->_gfx->drawPageBitBlock(screen, kArrowRightOffPath, _arrowRightPos);
	}
}

EventHandleResult ShelterRescueSite1::onLButtonDown(const Common::Point &) {
	return EventHandleResult::kPassthrough;
}

EventHandleResult ShelterRescueSite1::onLButtonUp(const Common::Point &pos) {
	if (_departurePrompted)
		return EventHandleResult::kConsumed;
	refreshGridOccupancy();
	const uint32 tick = _vm->getGameTickCount();
	const ZmbDropResult inputResult = ZoombiniRunner::handlePointerInput(_vm->_state->_activeZoombinis, Common::Point32(pos.x, pos.y), true,
																		 _pickupZombAnimation, tick, &_dropTargets, getAreaMask());
	if (inputResult != ZmbDropResult::kIgnored00)
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

	StorageRecord *const *storage = getRescueStorage();
	for (int col = 0; col < 4; col++) {
		for (int row = 0; row < 5; row++) {
			const Common::Rect slotHit(kRosterGridBasePos.x + col * 40 + 24, kRosterGridBasePos.y + row * 57 + 30,
									   kRosterGridBasePos.x + col * 40 + 64, kRosterGridBasePos.y + row * 57 + 87);
			if (slotHit.contains(pos)) {
				const int storageIndex = (_scrollRow + col) * kStorageCols + row;
				if (0 <= storageIndex && storageIndex < kStorageRows * kStorageCols && storage[storageIndex])
					materializeFromStorage(col, row, pos);
				return EventHandleResult::kConsumed;
			}
		}
	}
	return EventHandleResult::kPassthrough;
}

EventHandleResult ShelterRescueSite1::onMouseMove(const Common::Point &pos) {
	const ZmbDropResult result = ZoombiniRunner::handlePointerInput(_vm->_state->_activeZoombinis, Common::Point32(pos.x, pos.y), false,
																	_pickupZombAnimation, _vm->getGameTickCount(), &_dropTargets, getAreaMask());
	return result == ZmbDropResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

} // End of namespace Zoombini2
