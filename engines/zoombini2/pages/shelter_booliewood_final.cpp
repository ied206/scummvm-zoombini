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
#include "zoombini2/pages/shelter_booliewood_final.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

const int BooliewoodFinalPage::kDancingBooliePositions[kDancingBoolieCount][2] = {
	{100, 530},
	{300, 530},
	{400, 530},
};
const int BooliewoodFinalPage::kDecorativeZoombiniPositions[kDecorativeZoombiniCount][2] = {
	{500, 550},
	{200, 550},
};
const int BooliewoodFinalPage::kDecorativeZoombiniCells[kDecorativeZoombiniCount] = {77, 99};

BooliewoodFinalPage::BooliewoodFinalPage(Zoombini2Engine *engine)
	: ShelterPage(engine), _background(nullptr), _fullBigBool(nullptr), _revealBigBool(nullptr), _dancingBoolie(nullptr), _boolDance(nullptr),
	  _revealFlare1(nullptr), _revealFlare2(nullptr), _zoombiniGfx(nullptr), _walkingZoombiniGfx(nullptr), _animationStartTime(0), _openingSpeechFinished(false),
	  _closingSpeechTime(0), _revealStarted(false), _revealCellReady(false), _revealRow(0), _revealColumn(0), _musicId(-1),
	  _openingSpeechId(-1), _closingSpeechId(-1), _nextAmbientTime(0) {
	_pageId = kPageFinal;
	for (int i = 0; i < kFireworkCount; i++) {
		_fireworks[i].animation = nullptr;
		_fireworks[i].collisionRect = Common::Rect(1000, 1000, 1001, 1001);
		_fireworks[i].startY = 0;
		_fireworks[i].timeStep = 0;
		_fireworks[i].frame = 0;
		_fireworks[i].nextFrameTime = 0;
		_fireworks[i].active = true;
	}
	for (int i = 0; i < kAmbientSoundCount; i++)
		_ambientSoundIds[i] = -1;
	for (int i = 0; i < kDecorativeZoombiniCount; i++) {
		_decorativeAnimationFrames[i] = 0;
		_decorativeNextFrameTimes[i] = 0;
	}
}

BooliewoodFinalPage::~BooliewoodFinalPage() {
	_engine->clearGlobalZoombinis();

	SoundManager *sound = _engine->getSoundManager();
	if (sound) {
		if (0 <= _musicId) {
			sound->stop(_musicId);
			sound->unload(_musicId);
		}
		if (0 <= _openingSpeechId)
			sound->unload(_openingSpeechId);
		if (0 <= _closingSpeechId)
			sound->unload(_closingSpeechId);
		for (int i = 0; i < kAmbientSoundCount; i++) {
			if (0 <= _ambientSoundIds[i])
				sound->unload(_ambientSoundIds[i]);
		}
	}

	delete _background;
	delete _fullBigBool;
	delete _revealBigBool;
	delete _dancingBoolie;
	delete _boolDance;
	delete _revealFlare1;
	delete _revealFlare2;
	delete _zoombiniGfx;
	delete _walkingZoombiniGfx;
	for (int i = 0; i < kFireworkCount; i++)
		delete _fireworks[i].animation;

	GameState *state = _engine->getGameState();
	if (state)
		_engine->writeGameSave(state->_playerName);
}

void BooliewoodFinalPage::init() {
	debug(1, "BooliewoodFinalPage::init");
	_engine->clearGlobalZoombinis();

	_background = new BitBlock();
	if (!_background->load(Common::Path("bmp/final/big BOOL"))) {
		warning("BooliewoodFinalPage: Failed to load big BOOL background");
		delete _background;
		_background = nullptr;
	}
	_fullBigBool = new BitBlock();
	if (!_fullBigBool->load(Common::Path("bmp/final/thefullbigbool"))) {
		warning("BooliewoodFinalPage: Failed to load thefullbigbool overlay");
		delete _fullBigBool;
		_fullBigBool = nullptr;
	}
	_revealBigBool = new RleBlock();
	if (!_revealBigBool->loadFromFile(Common::Path("bmp/final/therealbigbool.rb"))) {
		delete _revealBigBool;
		_revealBigBool = nullptr;
	}

	_dancingBoolie = new Animation();
	if (!_dancingBoolie->loadFromFile(Common::Path("bmp/final/dancing_boolie.an"))) {
		delete _dancingBoolie;
		_dancingBoolie = nullptr;
	}
	_boolDance = new Animation();
	if (!_boolDance->loadFromFile(Common::Path("bmp/final/booldance.an"))) {
		delete _boolDance;
		_boolDance = nullptr;
	}
	_revealFlare1 = new Animation();
	if (!_revealFlare1->loadFromFile(Common::Path("bmp/aquacube/flare1.an"))) {
		delete _revealFlare1;
		_revealFlare1 = nullptr;
	}
	_revealFlare2 = new Animation();
	if (!_revealFlare2->loadFromFile(Common::Path("bmp/aquacube/flare2.an"))) {
		delete _revealFlare2;
		_revealFlare2 = nullptr;
	}

	const char *fireworkPaths[kFireworkCount] = {
		"bmp/final/FWBLUE.an",
		"bmp/final/FWGREEN.an",
		"bmp/final/FWRED.an",
	};
	const uint32 now = _engine->getGameTickCount();
	for (int i = 0; i < kFireworkCount; i++) {
		FireworkState &firework = _fireworks[i];
		firework.animation = new Animation();
		if (!firework.animation->loadFromFile(Common::Path(fireworkPaths[i]))) {
			delete firework.animation;
			firework.animation = nullptr;
			firework.active = false;
		}
		firework.frame = 0;
		firework.nextFrameTime = now + 40;
	}

	_zoombiniGfx = new ZoombiniGraphics();
	if (!_zoombiniGfx->loadFromFile(Common::Path("bmp/zombis/littleZomb.anm"))) {
		warning("BooliewoodFinalPage: Failed to load littleZomb.anm");
		delete _zoombiniGfx;
		_zoombiniGfx = nullptr;
	}
	_walkingZoombiniGfx = new ZoombiniGraphics();
	if (!_walkingZoombiniGfx->loadFromFile(Common::Path("bmp/zombis/attente/attenteZomb.anm"))) {
		warning("BooliewoodFinalPage: Failed to load attenteZomb.anm");
		delete _walkingZoombiniGfx;
		_walkingZoombiniGfx = nullptr;
	}
	createDecorativeZoombinis();

	_animationStartTime = now;
	_openingSpeechFinished = false;
	_closingSpeechTime = 0;
	_revealStarted = false;
	_revealCellReady = true;
	_revealRow = 0;
	_revealColumn = 0;

	SoundManager *sound = _engine->getSoundManager();
	if (sound) {
		_musicId = sound->load(true, Common::Path("sounds/music/Booliewood_Finale.wav"), true);
		if (0 <= _musicId) {
			sound->playLoop(_musicId);
			sound->setVolume(_musicId, sound->_volumeMusic);
		}
		_openingSpeechId = sound->load(false, Common::Path("sounds/Fin11.wav"), false);
		if (0 <= _openingSpeechId)
			sound->playWithVolume(_openingSpeechId, sound->_volumeSpeech);
		_closingSpeechId = sound->load(false, Common::Path("sounds/INT11.17.wav"), false);
		_ambientSoundIds[0] = sound->load(true, Common::Path("sounds/zbv42.5.wav"), false);
		for (int i = 1; i < kAmbientSoundCount; i++) {
			const Common::String path = Common::String::format("sounds/blw22.%d.wav", i);
			_ambientSoundIds[i] = sound->load(true, Common::Path(path), false);
		}
	}
	scheduleNextAmbient(now);
}

void BooliewoodFinalPage::update() {
	const uint32 now = _engine->getGameTickCount();
	SoundManager *sound = _engine->getSoundManager();
	if (!_openingSpeechFinished && (!sound || _openingSpeechId < 0 || !sound->isPlaying(_openingSpeechId))) {
		_openingSpeechFinished = true;
		_closingSpeechTime = now + 60000;
	}
	if (_closingSpeechTime != 0 && _closingSpeechTime < now) {
		if (sound && 0 <= _closingSpeechId)
			sound->playWithVolume(_closingSpeechId, sound->_volumeSpeech);
		_closingSpeechTime = 0;
	}

	updateDecorativeZoombinis(now);
	updateFireworks(now);
	if (_nextAmbientTime != 0 && _nextAmbientTime < now) {
		playRandomAmbient();
		scheduleNextAmbient(now);
	}
}

void BooliewoodFinalPage::draw(Graphics::ManagedSurface *screen) {
	if (_background)
		_background->drawToSurface(screen, 0, 0);
	if (_fullBigBool)
		_fullBigBool->drawToSurface(screen, 225, 34);

	const uint32 elapsed = _engine->getGameTickCount() - _animationStartTime;
	if (_boolDance && 0 < _boolDance->getFrameCount())
		drawAnimation(_boolDance, static_cast<int>(elapsed / 200) % _boolDance->getFrameCount(), 151, 31, screen);
	if (_dancingBoolie && 0 < _dancingBoolie->getFrameCount()) {
		const int frame = static_cast<int>(elapsed / 100) % _dancingBoolie->getFrameCount();
		for (int i = 0; i < kDancingBoolieCount; i++)
			drawAnimation(_dancingBoolie, frame, kDancingBooliePositions[i][0], kDancingBooliePositions[i][1], screen);
	}
	for (int i = 0; i < kFireworkCount; i++) {
		const FireworkState &firework = _fireworks[i];
		if (firework.active)
			drawAnimation(firework.animation, firework.frame, firework.collisionRect.left, firework.collisionRect.top, screen);
	}
	for (int i = 0; i < kDecorativeZoombiniCount && i < static_cast<int>(_engine->_globalZoombinis.size()); i++)
		drawZoombini(*_engine->_globalZoombinis[i], i, screen);
}

void BooliewoodFinalPage::handleClick(const Common::Point &pos) {
	(void)pos;
	_engine->requestPageChange(kPageMenuOptions);
}

void BooliewoodFinalPage::createDecorativeZoombinis() {
	for (int i = 0; i < kDecorativeZoombiniCount; i++) {
		ZoombiniState *zoombini = new ZoombiniState();
		zoombini->setFeatures(static_cast<byte>(_engine->getRandom()->getRandomNumber(4) + 1),
							  static_cast<byte>(_engine->getRandom()->getRandomNumber(4) + 1),
							  static_cast<byte>(_engine->getRandom()->getRandomNumber(4) + 1),
							  static_cast<byte>(_engine->getRandom()->getRandomNumber(4) + 1));
		zoombini->_position = Common::Point32(kDecorativeZoombiniPositions[i][0], kDecorativeZoombiniPositions[i][1]);
		zoombini->_activeFlag = 0;
		zoombini->_stateByte6C = 0;
		zoombini->_zoombiniIndex = kDecorativeZoombiniCells[i];
		_decorativeAnimationFrames[i] = 0;
		_decorativeNextFrameTimes[i] = 0;
		_engine->_globalZoombinis.push_back(zoombini);
	}
}

void BooliewoodFinalPage::updateDecorativeZoombinis(uint32 now) {
	for (int i = 0; i < kDecorativeZoombiniCount && i < static_cast<int>(_engine->_globalZoombinis.size()); i++) {
		ZoombiniState *zoombini = _engine->_globalZoombinis[i];
		if (zoombini->_stateByte6C == 1) {
			if (_decorativeNextFrameTimes[i] < now) {
				_decorativeAnimationFrames[i] += 1;
				_decorativeNextFrameTimes[i] = now + 50;
				if (11 <= _decorativeAnimationFrames[i])
					_decorativeAnimationFrames[i] = 1;
			}
		}
		if (zoombini->_stateByte6C == 0 && _engine->getRandom()->getRandomNumber(99) == 10) {
			zoombini->_stateByte6C = 1;
			zoombini->_zoombiniIndex = 33;
			_decorativeAnimationFrames[i] = 1;
			_decorativeNextFrameTimes[i] = now + 50;
		}
		if (zoombini->_stateByte6C == 1 && _engine->getRandom()->getRandomNumber(49) == 10) {
			zoombini->_stateByte6C = 0;
			zoombini->_zoombiniIndex = kDecorativeZoombiniCells[i];
			_decorativeAnimationFrames[i] = 0;
			_decorativeNextFrameTimes[i] = 0;
		}
	}
}

void BooliewoodFinalPage::updateFireworks(uint32 now) {
	for (int i = 0; i < kFireworkCount; i++)
		advanceFireworkAnimation(_fireworks[i], now);
	if (!_fireworks[0].active) {
		resetFirework(0, now);
		return;
	}
	moveFirework(_fireworks[0]);
	if (!_fireworks[1].active) {
		resetFirework(1, now);
		return;
	}
	moveFirework(_fireworks[1]);
	if (!_fireworks[2].active) {
		resetFirework(2, now);
		return;
	}
	moveFirework(_fireworks[2]);
}

void BooliewoodFinalPage::advanceFireworkAnimation(FireworkState &firework, uint32 now) {
	if (!firework.active || !firework.animation)
		return;
	while (firework.nextFrameTime <= now) {
		firework.frame += 1;
		firework.nextFrameTime += 40;
		if (firework.animation->getFrameCount() <= firework.frame) {
			firework.active = false;
			return;
		}
	}
}

void BooliewoodFinalPage::resetFirework(int fireworkIndex, uint32 now) {
	FireworkState &firework = _fireworks[fireworkIndex];
	if (!firework.animation)
		return;
	int x;
	int y;
	do {
		x = _engine->getRandom()->getRandomNumber(749);
		y = _engine->getRandom()->getRandomNumber(299);
	} while (!isFireworkPositionFree(x, y));
	firework.collisionRect = Common::Rect(x, y, x + 60, y + 59);
	firework.startY = y;
	firework.timeStep = 0;
	firework.frame = 0;
	firework.nextFrameTime = now + 40;
	firework.active = true;
}

void BooliewoodFinalPage::moveFirework(FireworkState &firework) {
	const double time = static_cast<double>(firework.timeStep);
	const int y = static_cast<int>(static_cast<double>(firework.startY) - 2.0 * time + 0.06 * time * time);
	firework.timeStep += 1;
	firework.collisionRect.top = y;
	firework.collisionRect.right = firework.collisionRect.left + 60;
	firework.collisionRect.bottom = y + 59;
}

bool BooliewoodFinalPage::isFireworkPositionFree(int x, int y) const {
	Common::Rect fixedRects[2] = {Common::Rect(153, 3, 647, 416), Common::Rect(0, 387, 800, 600)};
	for (int i = 0; i < kFireworkCount; i++) {
		const Common::Rect &rect = _fireworks[i].collisionRect;
		if (pointInsidePaddedRect(x, y, rect) || pointInsidePaddedRect(x + 60, y, rect) ||
			pointInsidePaddedRect(x, y + 59, rect) || pointInsidePaddedRect(x + 60, y + 59, rect))
			return false;
	}
	for (int i = 0; i < 2; i++) {
		const Common::Rect &rect = fixedRects[i];
		if (pointInsidePaddedRect(x, y, rect) || pointInsidePaddedRect(x + 60, y, rect) ||
			pointInsidePaddedRect(x, y + 59, rect) || pointInsidePaddedRect(x + 60, y + 59, rect))
			return false;
	}
	return true;
}

bool BooliewoodFinalPage::pointInsidePaddedRect(int x, int y, const Common::Rect &rect) {
	return rect.left - 15 < x && x < rect.right + 15 && rect.top - 15 < y && y < rect.bottom + 15;
}

void BooliewoodFinalPage::scheduleNextAmbient(uint32 now) {
	_nextAmbientTime = now + 1000 * (_engine->getRandom()->getRandomNumber(9) + 10);
}

void BooliewoodFinalPage::playRandomAmbient() {
	SoundManager *sound = _engine->getSoundManager();
	if (!sound)
		return;
	const int index = _engine->getRandom()->getRandomNumber(kAmbientSoundCount - 1);
	if (0 <= _ambientSoundIds[index])
		sound->playWithVolume(_ambientSoundIds[index], sound->_volumeSpeech);
}

void BooliewoodFinalPage::drawAnimation(const Animation *animation, int frameIndex, int x, int y, Graphics::ManagedSurface *screen) const {
	if (!animation || animation->getFrameCount() <= 0)
		return;
	const RleBlock *frame = animation->getFrame(frameIndex % animation->getFrameCount());
	if (frame)
		frame->drawToScreen(screen, x, y, _engine->getAlphaLUT());
}

void BooliewoodFinalPage::drawZoombini(const ZoombiniState &zoombini, int decorativeIndex, Graphics::ManagedSurface *screen) const {
	const ZoombiniGraphics *graphics = zoombini._stateByte6C == 1 ? _walkingZoombiniGfx : _zoombiniGfx;
	if (!graphics)
		return;
	const int baseIndex = zoombini._zoombiniIndex * ZoombiniGraphics::kDim1 * ZoombiniGraphics::kDim2;
	const int animationFrame = _decorativeAnimationFrames[decorativeIndex];
	int frameIndex = graphics->getFrameCount(baseIndex) == 1 ? 0 : animationFrame;
	const RleBlock *frame = graphics->getFrame(baseIndex, frameIndex);
	if (frame)
		frame->drawToScreen(screen, zoombini._position.x, zoombini._position.y, _engine->getAlphaLUT());
	const byte features[kNumFeatures] = {zoombini._featureA, zoombini._featureB, zoombini._featureC, zoombini._featureD};
	for (int layer = 1; layer <= kNumFeatures; layer++) {
		const int featureIndex = baseIndex + layer * ZoombiniGraphics::kDim2 + features[layer - 1];
		frameIndex = graphics->getFrameCount(featureIndex) == 1 ? 0 : animationFrame;
		frame = graphics->getFrame(featureIndex, frameIndex);
		if (frame)
			frame->drawToScreen(screen, zoombini._position.x, zoombini._position.y, _engine->getAlphaLUT());
	}
}

} // End of namespace Zoombini2
