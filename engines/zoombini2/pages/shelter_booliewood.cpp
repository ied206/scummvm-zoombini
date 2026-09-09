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
#include "zoombini2/path.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

const int BooliewoodPage::kSeatDefinitions[kNumSeats][4] = {
	{148, 459, 402, 0}, {168, 479, 422, 1}, {598, 490, 958, 1}, {1014, 383, 1113, 1}, {1060, 481, 1124, 2}, {1123, 383, 1224, 0},
	{1144, 403, 1215, 1}, {1246, 467, 1316, 2}, {1283, 487, 1343, 5}, {1440, 427, 1490, 1}, {1543, 480, 1620, 2}, {1623, 447, 1689, 1},
	{1856, 468, 2021, 2}, {2101, 480, 2348, 2}, {2468, 431, 2564, 1}, {2620, 475, 2775, 1}, {2764, 490, 2869, 1}, {2888, 475, 2977, 2},
	{3047, 450, 3121, 0}, {3121, 410, 3313, 1}, {3254, 475, 3335, 0}, {3416, 530, 3783, 1}, {3854, 415, 3973, 0}
};

const char *const BooliewoodPage::kCrowdPattern[27] = {
	"00000111111111100000", "00011111111111111000", "00111111111111111100", "01111111111111111100", "01111111111111111100",
	"11111222111222111000", "11112222212222211000", "11112212212212211000", "01112212212212211000", "01112222212222211000",
	"00111222211222111100", "00111111111111111100", "00011111111112211110", "00001222222222211110", "00001122222222111111",
	"00011112222221111111", "00011111222211111111", "00111111111111111111", "00111111111111111110", "00111111111111111100",
	"00000222000222000000", "00000222000222100000", "00001111101111110000", "00012211101112221000", "00122211101112221100",
	"01111111000111111110", "01111110000001111110"
};

BooliewoodPage::BooliewoodPage(Zoombini2Engine *engine)
	: ShelterPage(engine), _scrollX(0), _developmentStage(1), _background(nullptr), _zoombiniGfx(nullptr), _walkingZoombiniGfx(nullptr), _contentMarker(nullptr),
	  _pascontentMarker(nullptr), _walkingAnimation(nullptr), _waitingAnimation(nullptr), _musicId(-1), _introSpeechId(-1),
	  _nextAmbientSpeechTime(0), _ambientSpeechEnabled(false) {
	_pageId = kPageBooliewood;
	for (int i = 0; i < kAttractionCount; i++) {
		_attractions[i].animation = nullptr;
		_attractions[i].worldX = 0;
		_attractions[i].y = 0;
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
	for (int i = 0; i < kMaximumVisibleZoombinis; i++) {
		_seatedWalking[i] = false;
		_seatedAnimationFrames[i] = 0;
		_seatedNextFrameTimes[i] = 0;
	}
	resetSeats();
}

BooliewoodPage::~BooliewoodPage() {
	SoundManager *sound = _engine->getSoundManager();
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
	delete _zoombiniGfx;
	delete _walkingZoombiniGfx;
	delete _contentMarker;
	delete _pascontentMarker;
	delete _walkingAnimation;
	delete _waitingAnimation;
	_engine->clearGlobalZoombinis();
}

void BooliewoodPage::init() {
	debug(1, "BooliewoodPage::init");
	GameState *state = _engine->getGameState();
	const bool firstVisit = !state->isWorldVisitedAtDiff(kPageBooliewood, 1);
	state->_stateByteC = 1;
	state->registerWorldVisit(kPageBooliewood, 1);

	const int rescuedTotal = state->_counterDword;
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
	if (!_background->load(Common::Path("bmp/booliewood/background"))) {
		warning("BooliewoodPage: Failed to load background");
		delete _background;
		_background = nullptr;
	}

	_zoombiniGfx = new ZoombiniGraphics();
	if (!_zoombiniGfx->loadFromFile(Common::Path("bmp/zombis/littleZomb.anm"))) {
		warning("BooliewoodPage: Failed to load littleZomb.anm");
		delete _zoombiniGfx;
		_zoombiniGfx = nullptr;
	}
	_walkingZoombiniGfx = new ZoombiniGraphics();
	if (!_walkingZoombiniGfx->loadFromFile(Common::Path("bmp/zombis/attente/attenteZomb.anm"))) {
		warning("BooliewoodPage: Failed to load attenteZomb.anm");
		delete _walkingZoombiniGfx;
		_walkingZoombiniGfx = nullptr;
	}

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

	const uint32 now = _engine->getGameTickCount();
	resetSeats();
	buildSeatedCommunity(now);
	loadAttractions(now);
	loadCrowdActors(now);

	SoundManager *sound = _engine->getSoundManager();
	if (sound) {
		_musicId = sound->load(true, Common::Path("sounds/music/Booliewood_Level1.wav"), true);
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

void BooliewoodPage::update() {
	const uint32 now = _engine->getGameTickCount();
	updateScroll();
	updateAttractions(now);
	updateSeatedAnimations(now);
	updateCrowdActors(now);
	if (_ambientSpeechEnabled && _nextAmbientSpeechTime <= now) {
		playAmbientSpeech();
		scheduleAmbientSpeech(now);
	}
}

void BooliewoodPage::draw(Graphics::ManagedSurface *screen) {
	drawBackground(screen);
	drawAttractions(screen);
	drawRescuedCrowd(screen);
	drawSeatedCommunity(screen);
	drawCrowdActors(screen);
}

void BooliewoodPage::resetSeats() {
	for (int i = 0; i < kNumSeats; i++) {
		Seat &seat = _seats[i];
		seat.initialX = kSeatDefinitions[i][0];
		seat.y = kSeatDefinitions[i][1];
		seat.maximumX = kSeatDefinitions[i][2];
		seat.rowKind = kSeatDefinitions[i][3];
		seat.assignedCount = 0;
		seat.nextX = seat.initialX;
		seat.full = false;
	}
}

bool BooliewoodPage::assignSeat(Common::Point32 &position) {
	int order[kNumSeats];
	for (int i = 0; i < kNumSeats; i++)
		order[i] = i;

	int successfulSwaps = 0;
	while (successfulSwaps < 100) {
		const int first = _engine->getRandom()->getRandomNumber(kNumSeats - 1);
		const int second = _engine->getRandom()->getRandomNumber(kNumSeats - 1);
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
		position = Common::Point32(seat.nextX, seat.y);
		seat.assignedCount += 1;
		seat.nextX += 35;
		seat.full = seat.maximumX < seat.nextX;
		return true;
	}
	return false;
}

void BooliewoodPage::buildSeatedCommunity(uint32 now) {
	GameState *state = _engine->getGameState();
	for (int i = 0; i < kMaximumVisibleZoombinis; i++) {
		_seatedWalking[i] = false;
		_seatedAnimationFrames[i] = 0;
		_seatedNextFrameTimes[i] = 0;
	}
	const int incomingCount = static_cast<int>(_engine->_globalZoombinis.size());
	for (int i = 0; i < incomingCount; i++) {
		ZoombiniState *zoombini = _engine->_globalZoombinis[i];
		Common::Point32 position;
		if (assignSeat(position))
			zoombini->_position = position;
		zoombini->_activeFlag = 0;
		zoombini->_zoombiniIndex = kSeatedZoombiniCell;
	}

	int historicalCount = state->_statC - incomingCount;
	if (historicalCount < 0)
		historicalCount = 0;
	if (180 < historicalCount)
		historicalCount = 180;
	int walkingQuota = historicalCount / 5;
	for (int i = 0; i < historicalCount && static_cast<int>(_engine->_globalZoombinis.size()) < kMaximumVisibleZoombinis; i++) {
		Common::Point32 position;
		if (!assignSeat(position))
			break;
		ZoombiniState *zoombini = createHistoricalZoombini(state->_extendedState[i]);
		zoombini->_position = position;
		zoombini->_activeFlag = 0;
		zoombini->_zoombiniIndex = kSeatedZoombiniCell;
		_engine->_globalZoombinis.push_back(zoombini);
		const int visibleIndex = static_cast<int>(_engine->_globalZoombinis.size()) - 1;
		if (walkingQuota != 0 && _engine->getRandom()->getRandomNumber(1) != 0) {
			zoombini->_stateByte6C = 1;
			_seatedWalking[visibleIndex] = true;
			_seatedAnimationFrames[visibleIndex] = 1;
			_seatedNextFrameTimes[visibleIndex] = now + 50;
			walkingQuota -= 1;
		}
	}
}

void BooliewoodPage::updateSeatedAnimations(uint32 now) {
	int count = static_cast<int>(_engine->_globalZoombinis.size());
	if (kMaximumVisibleZoombinis < count)
		count = kMaximumVisibleZoombinis;
	for (int i = 0; i < count; i++) {
		if (!_seatedWalking[i])
			continue;
		if (_seatedNextFrameTimes[i] < now) {
			_seatedAnimationFrames[i] += 1;
			_seatedNextFrameTimes[i] = now + 50;
			if (11 <= _seatedAnimationFrames[i])
				_seatedAnimationFrames[i] = 1;
		}
	}
}

ZoombiniState *BooliewoodPage::createHistoricalZoombini(int32 featureHash) {
	int32 value = featureHash;
	const byte featureD = static_cast<byte>(value % 8);
	value /= 8;
	const byte featureC = static_cast<byte>(value % 8);
	value /= 8;
	const byte featureB = static_cast<byte>(value % 8);
	value /= 8;
	const byte featureA = static_cast<byte>(value % 8);
	ZoombiniState *zoombini = new ZoombiniState();
	zoombini->setFeatures(featureA, featureB, featureC, featureD);
	return zoombini;
}

void BooliewoodPage::loadAttractions(uint32 now) {
	const char *paths[kAttractionCount] = {
		"bmp/booliewood/atraction_ourson_rail1.an", "bmp/booliewood/atraction_ourson_rail2.an", "bmp/booliewood/atraction_ourson_rail3.an",
		"bmp/booliewood/atraction_horror_move_lev3.an", "bmp/booliewood/atraction_horror_cils_lev3.an",
		"bmp/booliewood/atraction_horror_fire_lev2.an", "bmp/booliewood/atraction_mushroom_lights.an"
	};
	const int positions[kAttractionCount][2] = {{1734, 195}, {1829, 79}, {2197, 90}, {670, 347}, {656, 231}, {990, 271}, {2639, 167}};
	const int minimumStages[kAttractionCount] = {1, 1, 1, 4, 3, 2, 2};
	for (int i = 0; i < kAttractionCount; i++) {
		AttractionState &attraction = _attractions[i];
		attraction.worldX = positions[i][0];
		attraction.y = positions[i][1];
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

void BooliewoodPage::updateAttractions(uint32 now) {
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

uint32 BooliewoodPage::getAttractionFrameDelay(int attractionIndex, int frame) {
	if (attractionIndex == 3)
		return frame == 0 ? 1000 : 100;
	if (3 < attractionIndex)
		return 300;
	return 100;
}

void BooliewoodPage::loadCrowdActors(uint32 now) {
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
			actor.waitPosition = Common::Point32((first->p0.x >> 10) - 40, (first->p0.y >> 10) - 70);
		} else {
			actor.waitPosition = Common::Point32();
		}
		actor.position = actor.waitPosition;
		actor.nextWalkTime = now + _engine->getRandom()->getRandomNumber(3999);
		actor.nextFrameTime = now + getWaitingFrameDelay(0);
		actor.frame = 0;
		actor.walking = false;
	}
}

void BooliewoodPage::updateCrowdActors(uint32 now) {
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
			Common::Point32 pathPosition;
			if (actor.path->advance(now, pathPosition)) {
				actor.position = Common::Point32(pathPosition.x - 30, pathPosition.y - 50);
				if (_walkingAnimation && actor.nextFrameTime <= now) {
					actor.frame += 1;
					if (_walkingAnimation->getFrameCount() <= actor.frame)
						actor.frame = 0;
					actor.nextFrameTime = now + 40;
				}
			} else {
				actor.walking = false;
				actor.position = actor.waitPosition;
				actor.frame = 0;
				actor.nextFrameTime = now + getWaitingFrameDelay(0);
				actor.nextWalkTime = now + 1000 + _engine->getRandom()->getRandomNumber(3999);
			}
		} else if (_waitingAnimation && actor.nextFrameTime <= now) {
			actor.frame += 1;
			if (_waitingAnimation->getFrameCount() <= actor.frame)
				actor.frame = 0;
			actor.nextFrameTime = now + getWaitingFrameDelay(actor.frame);
		}
	}
}

uint32 BooliewoodPage::getWaitingFrameDelay(int frame) {
	return frame == 12 ? 500 : 90;
}

void BooliewoodPage::updateScroll() {
	const Common::Point mouse = _engine->getMousePos();
	int delta = 0;
	if (760 < mouse.x) {
		delta = _engine->isMouseClicked() ? 300 : 30;
	} else if (mouse.x < 30 && mouse.y < 475) {
		delta = _engine->isMouseClicked() ? -300 : -30;
	}
	_scrollX += delta;
	while (_scrollX < 0)
		_scrollX += kWorldWidth;
	while (kWorldWidth <= _scrollX)
		_scrollX -= kWorldWidth;
}

void BooliewoodPage::scheduleAmbientSpeech(uint32 now) {
	_nextAmbientSpeechTime = now + 8000 + _engine->getRandom()->getRandomNumber(29999);
}

void BooliewoodPage::playAmbientSpeech() {
	SoundManager *sound = _engine->getSoundManager();
	if (!sound)
		return;
	const int index = _engine->getRandom()->getRandomNumber(kAmbientSpeechCount - 1);
	if (0 <= _ambientSpeechIds[index])
		sound->playWithVolume(_ambientSpeechIds[index], sound->_volumeSpeech);
}

void BooliewoodPage::drawBackground(Graphics::ManagedSurface *screen) const {
	if (!_background)
		return;
	const int width = _background->getWidth();
	if (width <= kScreenWidth) {
		_background->drawToSurface(screen, 0, 0);
		return;
	}
	const int origin = _scrollX % width;
	const int tailWidth = width - origin;
	if (kScreenWidth <= tailWidth) {
		_background->drawSubRect(screen, 0, 0, Common::Rect(origin, 0, origin + kScreenWidth, kScreenHeight));
	} else {
		_background->drawSubRect(screen, 0, 0, Common::Rect(origin, 0, width, kScreenHeight));
		_background->drawSubRect(screen, tailWidth, 0, Common::Rect(0, 0, kScreenWidth - tailWidth, kScreenHeight));
	}
}

void BooliewoodPage::drawAnimationAtWorld(const Animation *animation, int frameIndex, int worldX, int y, Graphics::ManagedSurface *screen) const {
	if (!animation || animation->getFrameCount() <= 0)
		return;
	const int normalizedFrame = frameIndex % animation->getFrameCount();
	drawRleAtWorld(animation->getFrame(normalizedFrame), worldX, y, screen);
}

void BooliewoodPage::drawRleAtWorld(const RleBlock *frame, int worldX, int y, Graphics::ManagedSurface *screen) const {
	if (!frame)
		return;
	const byte (*lut)[256] = _engine->getAlphaLUT();
	const int baseX = worldX - _scrollX;
	frame->drawToScreen(screen, baseX - kWorldWidth, y, lut);
	frame->drawToScreen(screen, baseX, y, lut);
	frame->drawToScreen(screen, baseX + kWorldWidth, y, lut);
}

void BooliewoodPage::drawAttractions(Graphics::ManagedSurface *screen) const {
	for (int i = 0; i < kAttractionCount; i++) {
		const AttractionState &attraction = _attractions[i];
		if (attraction.active)
			drawAnimationAtWorld(attraction.animation, attraction.frame, attraction.worldX, attraction.y, screen);
	}
}

void BooliewoodPage::drawRescuedCrowd(Graphics::ManagedSurface *screen) const {
	const int rescuedTotal = _engine->getGameState()->_counterDword;
	int drawn = 0;
	for (int row = 26; 0 <= row && drawn < rescuedTotal; row--) {
		const int y = 474 - (26 - row) * 16;
		for (int column = 0; column < 20 && drawn < rescuedTotal; column++) {
			const char marker = kCrowdPattern[row][column];
			if (marker == '0')
				continue;
			const RleBlock *frame = marker == '1' ? _contentMarker : _pascontentMarker;
			drawRleAtWorld(frame, 3450 + column * 16, y, screen);
			drawn += 1;
		}
	}
}

void BooliewoodPage::drawSeatedCommunity(Graphics::ManagedSurface *screen) const {
	if (!_zoombiniGfx)
		return;
	int count = static_cast<int>(_engine->_globalZoombinis.size());
	if (kMaximumVisibleZoombinis < count)
		count = kMaximumVisibleZoombinis;
	int order[kMaximumVisibleZoombinis];
	for (int i = 0; i < count; i++) {
		order[i] = i;
		int insert = i;
		while (0 < insert && _engine->_globalZoombinis[order[insert]]->_position.y < _engine->_globalZoombinis[order[insert - 1]]->_position.y) {
			const int temporary = order[insert];
			order[insert] = order[insert - 1];
			order[insert - 1] = temporary;
			insert -= 1;
		}
	}
	for (int i = 0; i < count; i++) {
		const int visibleIndex = order[i];
		const ZoombiniState &zoombini = *_engine->_globalZoombinis[visibleIndex];
		const ZoombiniGraphics *graphics = _seatedWalking[visibleIndex] ? _walkingZoombiniGfx : _zoombiniGfx;
		drawZoombiniAtWorld(zoombini, graphics, kSeatedZoombiniCell, _seatedAnimationFrames[visibleIndex], zoombini._position.x,
						 zoombini._position.y, screen);
	}
}

void BooliewoodPage::drawCrowdActors(Graphics::ManagedSurface *screen) const {
	for (int i = 0; i < kCrowdActorCount; i++) {
		const CrowdActorState &actor = _crowdActors[i];
		const Animation *animation = actor.walking ? _walkingAnimation : _waitingAnimation;
		drawAnimationAtWorld(animation, actor.frame, actor.position.x, actor.position.y, screen);
	}
}

void BooliewoodPage::drawZoombiniAtWorld(const ZoombiniState &zoombini, const ZoombiniGraphics *graphics, int cell, int animationFrame, int worldX, int y,
										 Graphics::ManagedSurface *screen) const {
	if (!graphics)
		return;
	const int baseIndex = cell * ZoombiniGraphics::kDim1 * ZoombiniGraphics::kDim2;
	int frameIndex = graphics->getFrameCount(baseIndex) == 1 ? 0 : animationFrame;
	const RleBlock *frame = graphics->getFrame(baseIndex, frameIndex);
	drawRleAtWorld(frame, worldX, y, screen);
	const byte features[kNumFeatures] = {zoombini._featureA, zoombini._featureB, zoombini._featureC, zoombini._featureD};
	for (int layer = 1; layer <= kNumFeatures; layer++) {
		const int featureIndex = baseIndex + layer * ZoombiniGraphics::kDim2 + features[layer - 1];
		frameIndex = graphics->getFrameCount(featureIndex) == 1 ? 0 : animationFrame;
		frame = graphics->getFrame(featureIndex, frameIndex);
		drawRleAtWorld(frame, worldX, y, screen);
	}
}

} // End of namespace Zoombini2
