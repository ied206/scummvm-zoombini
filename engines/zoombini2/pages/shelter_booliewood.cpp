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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/debug.h"
#include "common/str.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/shelter_booliewood.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

const ShelterBooliewood::SeatDefinition ShelterBooliewood::kSeatDefinitions[kNumSeats] = {
	{Common::Point32(148, 459), 402, 0},
	{Common::Point32(168, 479), 422, 1},
	{Common::Point32(598, 490), 958, 1},
	{Common::Point32(1014, 383), 1113, 1},
	{Common::Point32(1060, 481), 1124, 2},
	{Common::Point32(1123, 383), 1224, 0},
	{Common::Point32(1144, 403), 1215, 1},
	{Common::Point32(1246, 467), 1316, 2},
	{Common::Point32(1283, 487), 1343, 5},
	{Common::Point32(1440, 427), 1490, 1},
	{Common::Point32(1543, 480), 1620, 2},
	{Common::Point32(1623, 447), 1689, 1},
	{Common::Point32(1856, 468), 2021, 2},
	{Common::Point32(2101, 480), 2348, 2},
	{Common::Point32(2468, 431), 2564, 1},
	{Common::Point32(2620, 475), 2775, 1},
	{Common::Point32(2764, 490), 2869, 1},
	{Common::Point32(2888, 475), 2977, 2},
	{Common::Point32(3047, 450), 3121, 0},
	{Common::Point32(3121, 410), 3313, 1},
	{Common::Point32(3254, 475), 3335, 0},
	{Common::Point32(3416, 530), 3783, 1},
	{Common::Point32(3854, 415), 3973, 0},
};

const char *const ShelterBooliewood::kCrowdPattern[27] = {
	"00000111111111100000",
	"00011111111111111000",
	"00111111111111111100",
	"01111111111111111100",
	"01111111111111111100",
	"11111222111222111000",
	"11112222212222211000",
	"11112212212212211000",
	"01112212212212211000",
	"01112222212222211000",
	"00111222211222111100",
	"00111111111111111100",
	"00011111111112211110",
	"00001222222222211110",
	"00001122222222111111",
	"00011112222221111111",
	"00011111222211111111",
	"00111111111111111111",
	"00111111111111111110",
	"00111111111111111100",
	"00000222000222000000",
	"00000222000222100000",
	"00001111101111110000",
	"00012211101112221000",
	"00122211101112221100",
	"01111111000111111110",
	"01111110000001111110",
};

ShelterBooliewood::ShelterBooliewood(Zoombini2Engine *vm)
	: ShelterBase(vm), _scrollX(0), _developmentStage(1), _background(nullptr), _zoombiniAnimation(nullptr), _walkingZoombiniAnimation(nullptr),
	  _contentMarker(nullptr), _pascontentMarker(nullptr), _walkingAnimation(nullptr), _waitingAnimation(nullptr), _musicId(-1), _introSpeechId(-1),
	  _nextAmbientSpeechTime(0), _ambientSpeechEnabled(false) {
	_pageId = kPageBooliewood;
	for (int i = 0; i < kAttractionCount; i++) {
		_attractions[i].animation = nullptr;
		_attractions[i].pos = Common::Point32();
		_attractions[i].frame = 0;
		_attractions[i].nextFrameTime = 0;
		_attractions[i].active = false;
	}
	for (int i = 0; i < kCrowdActorCount; i++) {
		_crowdActors[i].path = nullptr;
		_crowdActors[i].nextWalkTime = 0;
		_crowdActors[i].nextFrameTime = 0;
		_crowdActors[i].frame = 0;
		_crowdActors[i].walking = false;
	}
	for (int i = 0; i < kAmbientSpeechCount; i++)
		_ambientSpeechIds[i] = -1;
	resetSeats();
}

ShelterBooliewood::~ShelterBooliewood() {
	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		if (0 <= _musicId) {
			sound->stop(_musicId);
			sound->unload(_musicId);
		}
		if (0 <= _introSpeechId)
			sound->unload(_introSpeechId);
		for (int i = 0; i < kAmbientSpeechCount; i++) {
			if (0 <= _ambientSpeechIds[i])
				sound->unload(_ambientSpeechIds[i]);
		}
	}

	for (int i = 0; i < kAttractionCount; i++)
		delete _attractions[i].animation;
	for (int i = 0; i < kCrowdActorCount; i++)
		delete _crowdActors[i].path;
	delete _background;
	delete _contentMarker;
	delete _pascontentMarker;
	delete _walkingAnimation;
	delete _waitingAnimation;
	_vm->clearGlobalZoombinis();
}

void ShelterBooliewood::init() {
	debug(1, "BooliewoodPage::init");
	GameState *state = _vm->getGameState();
	const bool firstVisit = !state->hasPageVisit(kPageBooliewood, 1);
	state->_hasReachedBooliewood = 1;
	state->registerPageVisit(kPageBooliewood, 1);

	const int rescuedTotal = state->_rescuedBoolieCount;
	_developmentStage = 4;
	if (rescuedTotal < 300)
		_developmentStage = 3;
	if (rescuedTotal < 200)
		_developmentStage = 2;
	if (rescuedTotal < 100)
		_developmentStage = 2;
	if (rescuedTotal < 50)
		_developmentStage = 1;

	_background = new BitBlock();
	if (!_background->load(Common::Path("#bmp/booliewood/background"))) {
		warning("BooliewoodPage: Failed to load background");
		delete _background;
		_background = nullptr;
	}

	_zoombiniAnimation = _vm->loadZoombiniAnimation(Common::Path("bmp/zombis/littleZomb.anm"));
	if (!_zoombiniAnimation)
		warning("BooliewoodPage: Failed to load littleZomb.anm");
	_walkingZoombiniAnimation = _vm->loadZoombiniAnimation(Common::Path("bmp/zombis/attente/attenteZomb.anm"));
	if (!_walkingZoombiniAnimation)
		warning("BooliewoodPage: Failed to load attenteZomb.anm");

	_contentMarker = new RleBlock();
	if (!_contentMarker->loadFromFile(Common::Path("bmp/booliewood/piti_bool/content.rb"))) {
		delete _contentMarker;
		_contentMarker = nullptr;
	}
	_pascontentMarker = new RleBlock();
	if (!_pascontentMarker->loadFromFile(Common::Path("bmp/booliewood/piti_bool/pascontent.rb"))) {
		delete _pascontentMarker;
		_pascontentMarker = nullptr;
	}

	const uint32 now = _vm->getGameTickCount();
	resetSeats();
	buildSeatedCommunity(now);
	loadAttractions(now);
	loadCrowdActors(now);

	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		_musicId = sound->load(true, Common::Path("#sounds/music/Booliewood_Level1.wav"), true);
		if (0 <= _musicId) {
			sound->playLoop(_musicId);
			sound->setVolume(_musicId, sound->_volumeMusic);
		}
		if (firstVisit) {
			_introSpeechId = sound->load(false, Common::Path("sounds/zbv21.2.wav"), false);
			if (0 <= _introSpeechId) {
				sound->playWithVolume(_introSpeechId, sound->_volumeSpeech);
			}
		}
		for (int i = 0; i < kAmbientSpeechCount; i++) {
			const Common::String path = Common::String::format("sounds/blw22.%d.wav", i + 1);
			_ambientSpeechIds[i] = sound->load(false, Common::Path(path), false);
		}
	}

	_ambientSpeechEnabled = 32 <= rescuedTotal;
	if (_ambientSpeechEnabled)
		scheduleAmbientSpeech(now);
}

void ShelterBooliewood::onUpdate() {
	const uint32 now = _vm->getGameTickCount();
	updateScroll();
	updateAttractions(now);
	updateSeatedAnimations(now);
	updateCrowdActors(now);
	if (_ambientSpeechEnabled && _nextAmbientSpeechTime <= now) {
		playAmbientSpeech();
		scheduleAmbientSpeech(now);
	}
}

void ShelterBooliewood::onRenderBackground(ManagedSurface32 *screen) {
	drawBackground(screen);
}

void ShelterBooliewood::onRenderScene(ManagedSurface32 *screen) {
	drawAttractions(screen);
	drawRescuedCrowd(screen);
}

void ShelterBooliewood::onRenderActors(ManagedSurface32 *screen) {
	drawSeatedCommunity(screen);
}

void ShelterBooliewood::onRenderForeground(ManagedSurface32 *screen) {
	drawCrowdActors(screen);
}

void ShelterBooliewood::resetSeats() {
	for (int i = 0; i < kNumSeats; i++) {
		Seat &seat = _seats[i];
		const SeatDefinition &definition = kSeatDefinitions[i];
		seat.nextPos = definition.initialPos;
		seat.maximumX = definition.maximumX;
		seat.rowKind = definition.rowKind;
		seat.assignedCount = 0;
		seat.full = false;
	}
}

bool ShelterBooliewood::assignSeat(Common::Point32 &pos) {
	int order[kNumSeats];
	for (int i = 0; i < kNumSeats; i++)
		order[i] = i;

	int successfulSwaps = 0;
	while (successfulSwaps < 100) {
		const int first = _vm->getRandom()->getRandomNumber(kNumSeats - 1);
		const int second = _vm->getRandom()->getRandomNumber(kNumSeats - 1);
		if (first == second)
			continue;
		const int temporary = order[first];
		order[first] = order[second];
		order[second] = temporary;
		successfulSwaps += 1;
	}

	for (int i = 0; i < kNumSeats; i++) {
		Seat &seat = _seats[order[i]];
		if (seat.full)
			continue;
		pos = seat.nextPos;
		seat.assignedCount += 1;
		seat.nextPos.x += 35;
		seat.full = seat.maximumX < seat.nextPos.x;
		return true;
	}
	return false;
}

void ShelterBooliewood::buildSeatedCommunity(uint32 now) {
	GameState *state = _vm->getGameState();
	const int incomingCount = static_cast<int>(_vm->_globalZoombinis.size());
	for (int i = 0; i < incomingCount; i++) {
		ZoombiniState *zoombini = _vm->_globalZoombinis[i];
		Common::Point32 pos;
		if (assignSeat(pos))
			zoombini->setPosition(pos);
		zoombini->setDefaultAnimation(_zoombiniAnimation, kSeatedZoombiniCell);
		zoombini->_inputEnabled = false;
	}

	int historicalCount = state->_completedZoombiniCount - incomingCount;
	if (historicalCount < 0)
		historicalCount = 0;
	if (180 < historicalCount)
		historicalCount = 180;
	int walkingQuota = historicalCount / 5;
	for (int i = 0; i < historicalCount && static_cast<int>(_vm->_globalZoombinis.size()) < kMaximumVisibleZoombinis; i++) {
		Common::Point32 pos;
		if (!assignSeat(pos))
			break;
		ZoombiniState *zoombini = createHistoricalZoombini(state->_completedTraitHashes[i]);
		zoombini->setPosition(pos);
		zoombini->setDefaultAnimation(_zoombiniAnimation, kSeatedZoombiniCell);
		zoombini->_inputEnabled = false;
		_vm->_globalZoombinis.push_back(zoombini);
		if (walkingQuota != 0 && _vm->getRandom()->getRandomNumber(1) != 0) {
			zoombini->startAnimation(_walkingZoombiniAnimation, kSeatedZoombiniCell, now, 50, true);
			walkingQuota -= 1;
		}
	}
}

void ShelterBooliewood::updateSeatedAnimations(uint32 now) {
	int count = static_cast<int>(_vm->_globalZoombinis.size());
	if (kMaximumVisibleZoombinis < count)
		count = kMaximumVisibleZoombinis;
	for (int i = 0; i < count; i++)
		_vm->_globalZoombinis[i]->updateAnimation(now);
}

ZoombiniState *ShelterBooliewood::createHistoricalZoombini(int32 traitHash) {
	ZoombiniState *zoombini = new ZoombiniState();
	zoombini->setTraits(ZmbTrait::fromHash(static_cast<uint16>(traitHash)));
	return zoombini;
}

void ShelterBooliewood::loadAttractions(uint32 now) {
	const char *paths[kAttractionCount] = {
		"bmp/booliewood/atraction_ourson_rail1.an", "bmp/booliewood/atraction_ourson_rail2.an", "bmp/booliewood/atraction_ourson_rail3.an",
		"bmp/booliewood/atraction_horror_move_lev3.an", "bmp/booliewood/atraction_horror_cils_lev3.an",
		"bmp/booliewood/atraction_horror_fire_lev2.an", "bmp/booliewood/atraction_mushroom_lights.an"};
	const Common::Point32 attractionPos[kAttractionCount] = {
		Common::Point32(1734, 195), Common::Point32(1829, 79), Common::Point32(2197, 90), Common::Point32(670, 347),
		Common::Point32(656, 231), Common::Point32(990, 271), Common::Point32(2639, 167)};
	const int minimumStages[kAttractionCount] = {1, 1, 1, 4, 3, 2, 2};
	for (int i = 0; i < kAttractionCount; i++) {
		AttractionState &attraction = _attractions[i];
		attraction.pos = attractionPos[i];
		attraction.frame = 0;
		attraction.active = minimumStages[i] <= _developmentStage && (i == 0 || 3 <= i);
		if (minimumStages[i] <= _developmentStage) {
			attraction.animation = new Animation();
			if (!attraction.animation->loadFromFile(Common::Path(paths[i]))) {
				delete attraction.animation;
				attraction.animation = nullptr;
				attraction.active = false;
			}
		}
		attraction.nextFrameTime = now + getAttractionFrameDelay(i, 0);
	}
}

void ShelterBooliewood::updateAttractions(uint32 now) {
	for (int i = 0; i < kAttractionCount; i++) {
		AttractionState &attraction = _attractions[i];
		if (!attraction.active || !attraction.animation || attraction.nextFrameTime > now)
			continue;
		attraction.frame += 1;
		if (attraction.animation->getFrameCount() <= attraction.frame) {
			if (i < 3) {
				attraction.active = false;
				const int nextRail = i == 2 ? 0 : i + 1;
				AttractionState &next = _attractions[nextRail];
				if (next.animation) {
					next.frame = 0;
					next.active = true;
					next.nextFrameTime = now + getAttractionFrameDelay(nextRail, 0);
				}
				continue;
			}
			attraction.frame = 0;
		}
		attraction.nextFrameTime = now + getAttractionFrameDelay(i, attraction.frame);
	}
}

uint32 ShelterBooliewood::getAttractionFrameDelay(int attractionIndex, int frame) {
	if (attractionIndex == 3)
		return frame == 0 ? 1000 : 100;
	if (3 < attractionIndex)
		return 300;
	return 100;
}

void ShelterBooliewood::loadCrowdActors(uint32 now) {
	_walkingAnimation = new Animation();
	if (!_walkingAnimation->loadFromFile(Common::Path("bmp/boolies/marche.an"))) {
		delete _walkingAnimation;
		_walkingAnimation = nullptr;
	}
	_waitingAnimation = new Animation();
	if (!_waitingAnimation->loadFromFile(Common::Path("bmp/boolies/attend.an"))) {
		delete _waitingAnimation;
		_waitingAnimation = nullptr;
	}

	for (int i = 0; i < kCrowdActorCount; i++) {
		CrowdActorState &actor = _crowdActors[i];
		const Common::String path = Common::String::format("bmp/booliewood/path%d.pat", i + 1);
		actor.path = PathObject::loadFromPAT(Common::Path(path));
		if (actor.path && !actor.path->segments.empty()) {
			const CurveSegment *first = actor.path->segments[0];
			const Common::Point32 pathPos = first->getStartPosition();
			actor.waitPos = Common::Point32(pathPos.x - 40, pathPos.y - 70);
		} else {
			actor.waitPos = Common::Point32();
		}
		actor.pos = actor.waitPos;
		actor.nextWalkTime = now + _vm->getRandom()->getRandomNumber(3999);
		actor.nextFrameTime = now + getWaitingFrameDelay(0);
		actor.frame = 0;
		actor.walking = false;
	}
}

void ShelterBooliewood::updateCrowdActors(uint32 now) {
	for (int i = 0; i < kCrowdActorCount; i++) {
		CrowdActorState &actor = _crowdActors[i];
		if (!actor.path)
			continue;

		if (!actor.walking && actor.nextWalkTime <= now) {
			actor.walking = true;
			actor.frame = 0;
			actor.nextFrameTime = now + 40;
			actor.path->start(now);
		}

		if (actor.walking) {
			Common::Point32 pathPos;
			if (actor.path->advance(now, pathPos)) {
				actor.pos = Common::Point32(pathPos.x - 30, pathPos.y - 50);
				if (_walkingAnimation && actor.nextFrameTime <= now) {
					actor.frame += 1;
					if (_walkingAnimation->getFrameCount() <= actor.frame)
						actor.frame = 0;
					actor.nextFrameTime = now + 40;
				}
			} else {
				actor.walking = false;
				actor.pos = actor.waitPos;
				actor.frame = 0;
				actor.nextFrameTime = now + getWaitingFrameDelay(0);
				actor.nextWalkTime = now + 1000 + _vm->getRandom()->getRandomNumber(3999);
			}
		} else if (_waitingAnimation && actor.nextFrameTime <= now) {
			actor.frame += 1;
			if (_waitingAnimation->getFrameCount() <= actor.frame)
				actor.frame = 0;
			actor.nextFrameTime = now + getWaitingFrameDelay(actor.frame);
		}
	}
}

uint32 ShelterBooliewood::getWaitingFrameDelay(int frame) {
	return frame == 12 ? 500 : 90;
}

EventHandleResult ShelterBooliewood::onLButtonDown(const Common::Point &pos) {
	if (760 < pos.x)
		_pendingScrollDelta = 300;
	else if (pos.x < 30 && pos.y < 475)
		_pendingScrollDelta = -300;
	else
		return EventHandleResult::kPassthrough;
	return EventHandleResult::kConsumed;
}

void ShelterBooliewood::updateScroll() {
	const Common::Point32 mousePos = _vm->getMousePos();
	int delta = _pendingScrollDelta;
	_pendingScrollDelta = 0;
	if (delta != 0) {
		// A click supplies the complete scroll step for this frame.
	} else if (760 < mousePos.x) {
		delta = 30;
	} else if (mousePos.x < 30 && mousePos.y < 475) {
		delta = -30;
	}
	_scrollX += delta;
	while (_scrollX < 0)
		_scrollX += kSceneWidth;
	while (kSceneWidth <= _scrollX)
		_scrollX -= kSceneWidth;
}

void ShelterBooliewood::scheduleAmbientSpeech(uint32 now) {
	_nextAmbientSpeechTime = now + 8000 + _vm->getRandom()->getRandomNumber(29999);
}

void ShelterBooliewood::playAmbientSpeech() {
	SoundManager *sound = _vm->getSoundManager();
	if (!sound)
		return;
	const int index = _vm->getRandom()->getRandomNumber(kAmbientSpeechCount - 1);
	if (0 <= _ambientSpeechIds[index])
		sound->playWithVolume(_ambientSpeechIds[index], sound->_volumeSpeech);
}

void ShelterBooliewood::drawBackground(ManagedSurface32 *screen) const {
	if (!_background)
		return;
	const int width = _background->getWidth();
	if (width <= kScreenWidth) {
		_background->drawToSurface(screen, Common::Point32(0, 0));
		return;
	}
	const int origin = _scrollX % width;
	const int tailWidth = width - origin;
	if (kScreenWidth <= tailWidth) {
		_background->drawSubRect(screen, Common::Point32(0, 0), Common::Rect(origin, 0, origin + kScreenWidth, kScreenHeight));
	} else {
		_background->drawSubRect(screen, Common::Point32(0, 0), Common::Rect(origin, 0, width, kScreenHeight));
		_background->drawSubRect(screen, Common::Point32(tailWidth, 0), Common::Rect(0, 0, kScreenWidth - tailWidth, kScreenHeight));
	}
}

void ShelterBooliewood::drawAnimationInScene(const Animation *animation, int frameIndex, const Common::Point32 &pos, ManagedSurface32 *screen) const {
	if (!animation || animation->getFrameCount() <= 0)
		return;
	const int normalizedFrame = frameIndex % animation->getFrameCount();
	drawRleInScene(animation->getFrame(normalizedFrame), pos, screen);
}

void ShelterBooliewood::drawRleInScene(const RleBlock *frame, const Common::Point32 &pos, ManagedSurface32 *screen) const {
	if (!frame)
		return;
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();
	const int baseX = pos.x - _scrollX;
	frame->drawToScreen(screen, Common::Point32(baseX - kSceneWidth, pos.y), lut);
	frame->drawToScreen(screen, Common::Point32(baseX, pos.y), lut);
	frame->drawToScreen(screen, Common::Point32(baseX + kSceneWidth, pos.y), lut);
}

void ShelterBooliewood::drawAttractions(ManagedSurface32 *screen) const {
	for (int i = 0; i < kAttractionCount; i++) {
		const AttractionState &attraction = _attractions[i];
		if (attraction.active)
			drawAnimationInScene(attraction.animation, attraction.frame, attraction.pos, screen);
	}
}

void ShelterBooliewood::drawRescuedCrowd(ManagedSurface32 *screen) const {
	const int rescuedTotal = _vm->getGameState()->_rescuedBoolieCount;
	int drawn = 0;
	for (int row = 26; 0 <= row && drawn < rescuedTotal; row--) {
		const int y = 474 - (26 - row) * 16;
		for (int column = 0; column < 20 && drawn < rescuedTotal; column++) {
			const char marker = kCrowdPattern[row][column];
			if (marker == '0')
				continue;
			const RleBlock *frame = marker == '1' ? _contentMarker : _pascontentMarker;
			drawRleInScene(frame, Common::Point32(3450 + column * 16, y), screen);
			drawn += 1;
		}
	}
}

void ShelterBooliewood::drawSeatedCommunity(ManagedSurface32 *screen) const {
	if (!_zoombiniAnimation)
		return;
	int count = static_cast<int>(_vm->_globalZoombinis.size());
	if (kMaximumVisibleZoombinis < count)
		count = kMaximumVisibleZoombinis;
	int order[kMaximumVisibleZoombinis];
	for (int i = 0; i < count; i++) {
		order[i] = i;
		int insert = i;
		while (0 < insert && _vm->_globalZoombinis[order[insert]]->_screenPos.y < _vm->_globalZoombinis[order[insert - 1]]->_screenPos.y) {
			const int temporary = order[insert];
			order[insert] = order[insert - 1];
			order[insert - 1] = temporary;
			insert -= 1;
		}
	}
	for (int i = 0; i < count; i++) {
		const int visibleIndex = order[i];
		const ZoombiniState &zoombini = *_vm->_globalZoombinis[visibleIndex];
		const int animationFrame = zoombini._animationActive ? zoombini._animationFrame : 0;
		drawZoombiniInScene(zoombini, zoombini._activeAnimation, zoombini._animationCell, animationFrame, zoombini._screenPos, screen);
	}
}

void ShelterBooliewood::drawCrowdActors(ManagedSurface32 *screen) const {
	for (int i = 0; i < kCrowdActorCount; i++) {
		const CrowdActorState &actor = _crowdActors[i];
		const Animation *animation = actor.walking ? _walkingAnimation : _waitingAnimation;
		drawAnimationInScene(animation, actor.frame, actor.pos, screen);
	}
}

void ShelterBooliewood::drawZoombiniInScene(const ZoombiniState &zoombini, const ZoombiniAnimation *animation, int cell, int animationFrame,
										 const Common::Point32 &pos, ManagedSurface32 *screen) const {
	if (!animation)
		return;
	const int baseX = pos.x - _scrollX;
	for (int copy = -1; copy <= 1; copy++)
		animation->drawZoombini(screen, zoombini._traits, Common::Point32(baseX + copy * kSceneWidth, pos.y), cell, animationFrame, _vm->getAlphaLUT());
}

} // End of namespace Zoombini2
