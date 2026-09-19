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
#include "zoombini2/pages/shelter_final.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *ShelterFinal::kBackgroundPath;
constexpr const char *ShelterFinal::kFullBigBoolPath;
constexpr const char *ShelterFinal::kRevealBigBoolPath;
constexpr const char *ShelterFinal::kDancingBooliePath;
constexpr const char *ShelterFinal::kBoolDancePath;
constexpr const char *ShelterFinal::kRevealFlare1Path;
constexpr const char *ShelterFinal::kRevealFlare2Path;
constexpr const char *ShelterFinal::kFireworkPaths[kFireworkCount];
constexpr const char *ShelterFinal::kZoombiniAnimationPath;
constexpr const char *ShelterFinal::kWalkingZoombiniAnimationPath;
constexpr const char *ShelterFinal::kMusicPath;
constexpr const char *ShelterFinal::kOpeningSpeechPath;
constexpr const char *ShelterFinal::kClosingSpeechPath;
constexpr const char *ShelterFinal::kFirstAmbientSoundPath;
constexpr const char *ShelterFinal::kAmbientSoundFormat;

constexpr Common::Point32 ShelterFinal::kDancingBooliePos[kDancingBoolieCount];
constexpr Common::Point32 ShelterFinal::kDecorativeZoombiniPos[kDecorativeZoombiniCount];
constexpr int ShelterFinal::kDecorativeZoombiniCells[kDecorativeZoombiniCount];

ShelterFinal::ShelterFinal(Zoombini2Engine *vm)
	: ShelterBase(vm) {
	_pageId = kPageFinal;
}

ShelterFinal::~ShelterFinal() {
	_vm->_state->clearActiveZoombinis();

	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
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
	for (int i = 0; i < kFireworkCount; i++)
		delete _fireworks[i].animation;

	GameState *state = _vm->_state;
	if (state)
		_vm->writeGameSave(state->_playerName);
}

void ShelterFinal::init() {
	debug(1, "BooliewoodFinalPage::init");
	_vm->_state->clearActiveZoombinis();

	_background = new BitBlock(_vm);
	if (!_background->load(Common::Path(kBackgroundPath))) {
		warning("BooliewoodFinalPage: Failed to load big BOOL background");
		delete _background;
		_background = nullptr;
	}
	_fullBigBool = new BitBlock(_vm);
	if (!_fullBigBool->load(Common::Path(kFullBigBoolPath))) {
		warning("BooliewoodFinalPage: Failed to load thefullbigbool overlay");
		delete _fullBigBool;
		_fullBigBool = nullptr;
	}
	_revealBigBool = new RleBlock(_vm);
	if (!_revealBigBool->loadFromFile(Common::Path(kRevealBigBoolPath))) {
		delete _revealBigBool;
		_revealBigBool = nullptr;
	}

	_dancingBoolie = new Animation(_vm);
	if (!_dancingBoolie->loadFromFile(Common::Path(kDancingBooliePath))) {
		delete _dancingBoolie;
		_dancingBoolie = nullptr;
	}
	_boolDance = new Animation(_vm);
	if (!_boolDance->loadFromFile(Common::Path(kBoolDancePath))) {
		delete _boolDance;
		_boolDance = nullptr;
	}
	_revealFlare1 = new Animation(_vm);
	if (!_revealFlare1->loadFromFile(Common::Path(kRevealFlare1Path))) {
		delete _revealFlare1;
		_revealFlare1 = nullptr;
	}
	_revealFlare2 = new Animation(_vm);
	if (!_revealFlare2->loadFromFile(Common::Path(kRevealFlare2Path))) {
		delete _revealFlare2;
		_revealFlare2 = nullptr;
	}

	const uint32 now = _vm->getGameTickCount();
	for (int i = 0; i < kFireworkCount; i++) {
		FireworkState &firework = _fireworks[i];
		firework.animation = new Animation(_vm);
		if (!firework.animation->loadFromFile(Common::Path(kFireworkPaths[i]))) {
			delete firework.animation;
			firework.animation = nullptr;
			firework.active = false;
		}
		firework.frame = 0;
		firework.nextFrameTime = now + 40;
	}

	_zoombiniAnimation = _vm->loadZoombiniAnimation(Common::Path(kZoombiniAnimationPath), 50);
	if (!_zoombiniAnimation)
		warning("BooliewoodFinalPage: Failed to load littleZomb.anm");
	_walkingZoombiniAnimation = _vm->loadZoombiniAnimation(Common::Path(kWalkingZoombiniAnimationPath), 50);
	if (!_walkingZoombiniAnimation)
		warning("BooliewoodFinalPage: Failed to load attenteZomb.anm");
	createDecorativeZoombinis();

	_animationStartTime = now;
	_openingSpeechFinished = false;
	_closingSpeechTime = 0;
	_revealStarted = false;
	_revealCellReady = true;
	_revealRow = 0;
	_revealColumn = 0;

	startPageMusic(Common::Path(kMusicPath));
	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		_openingSpeechId = sound->load(false, Common::Path(kOpeningSpeechPath), false);
		if (0 <= _openingSpeechId)
			sound->playWithVolume(_openingSpeechId, sound->_volumeSpeech);
		_closingSpeechId = sound->load(false, Common::Path(kClosingSpeechPath), false);
		_ambientSoundIds[0] = sound->load(true, Common::Path(kFirstAmbientSoundPath), false);
		for (int i = 1; i < kAmbientSoundCount; i++) {
			const Common::String path = Common::String::format(kAmbientSoundFormat, i);
			_ambientSoundIds[i] = sound->load(true, Common::Path(path), false);
		}
	}
	scheduleNextAmbient(now);
}

void ShelterFinal::onUpdate() {
	const uint32 now = _vm->getGameTickCount();
	SoundManager *sound = _vm->getSoundManager();
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

void ShelterFinal::onRenderContent(ManagedSurface32 *screen) {
	if (_background)
		_background->drawToSurface(screen, Common::Point32(0, 0));
	if (_fullBigBool)
		_fullBigBool->drawToSurface(screen, Common::Point32(225, 34));

	const uint32 elapsed = _vm->getGameTickCount() - _animationStartTime;
	if (_boolDance && 0 < _boolDance->getFrameCount())
		drawAnimation(_boolDance, static_cast<int>(elapsed / 200) % _boolDance->getFrameCount(), Common::Point32(151, 31), screen);
	if (_dancingBoolie && 0 < _dancingBoolie->getFrameCount()) {
		const int frame = static_cast<int>(elapsed / 100) % _dancingBoolie->getFrameCount();
		for (int i = 0; i < kDancingBoolieCount; i++)
			drawAnimation(_dancingBoolie, frame, kDancingBooliePos[i], screen);
	}
	for (int i = 0; i < kFireworkCount; i++) {
		const FireworkState &firework = _fireworks[i];
		if (firework.active)
			drawAnimation(firework.animation, firework.frame,
						  Common::Point32(firework.collisionRect.left, firework.collisionRect.top), screen);
	}
}

void ShelterFinal::onRenderActors(ManagedSurface32 *screen) {
	Common::Array<uint> order;
	for (uint i = 0; i < kDecorativeZoombiniCount && i < _vm->_state->_activeZoombinis.size(); i++)
		order.push_back(i);
	ZoombiniRunner::sortDrawOrderByY(_vm->_state->_activeZoombinis, order);
	for (uint i = 0; i < order.size(); i++)
		drawZoombini(*_vm->_state->_activeZoombinis[order[i]], screen);
}

void ShelterFinal::onActorsRendered() {
	for (uint i = 0; i < kDecorativeZoombiniCount && i < _vm->_state->_activeZoombinis.size(); i++)
		_vm->_state->_activeZoombinis[i]->advanceAnimationAfterDraw();
	updateDecorativeZoombiniRunnersAfterDraw(_vm->getGameTickCount());
}

EventHandleResult ShelterFinal::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	_vm->requestPageChange(kPageMenuOptions);
	return EventHandleResult::kConsumed;
}

void ShelterFinal::createDecorativeZoombinis() {
	for (uint i = 0; i < kDecorativeZoombiniCount; i++) {
		ZoombiniRunner *zoombini = new ZoombiniRunner();
		const byte nose = static_cast<byte>(_vm->_rnd->getRandomNumber(4) + 1);
		const byte eyes = static_cast<byte>(_vm->_rnd->getRandomNumber(4) + 1);
		const byte hair = static_cast<byte>(_vm->_rnd->getRandomNumber(4) + 1);
		const byte feet = static_cast<byte>(_vm->_rnd->getRandomNumber(4) + 1);
		zoombini->setTraits(ZmbTrait(feet, nose, hair, eyes));
		zoombini->setPosition(kDecorativeZoombiniPos[i]);
		zoombini->setDefaultAnimation(_zoombiniAnimation, kDecorativeZoombiniCells[i]);
		zoombini->_inputEnabled = false;
		_vm->_state->_activeZoombinis.push_back(zoombini);
	}
}

void ShelterFinal::updateDecorativeZoombinis(uint32 now) {
	for (uint i = 0; i < kDecorativeZoombiniCount && i < _vm->_state->_activeZoombinis.size(); i++)
		_vm->_state->_activeZoombinis[i]->updateAnimation(now);
}

void ShelterFinal::updateDecorativeZoombiniRunnersAfterDraw(uint32 now) {
	for (uint i = 0; i < kDecorativeZoombiniCount && i < _vm->_state->_activeZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[i];
		if (!zoombini->_animationActive && _vm->_rnd->getRandomNumber(99) == 10)
			zoombini->startAnimation(_walkingZoombiniAnimation, 33, now);
		if (zoombini->_animationActive && _vm->_rnd->getRandomNumber(49) == 10) {
			zoombini->resetAnimation();
			zoombini->_animationCell = kDecorativeZoombiniCells[i];
		}
	}
}

void ShelterFinal::updateFireworks(uint32 now) {
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

void ShelterFinal::advanceFireworkAnimation(FireworkState &firework, uint32 now) {
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

void ShelterFinal::resetFirework(int fireworkIndex, uint32 now) {
	FireworkState &firework = _fireworks[fireworkIndex];
	if (!firework.animation)
		return;
	Common::Point32 pos;
	do {
		pos.x = _vm->_rnd->getRandomNumber(749);
		pos.y = _vm->_rnd->getRandomNumber(299);
	} while (!isFireworkPositionFree(pos));
	firework.collisionRect = Common::Rect(pos.x, pos.y, pos.x + 60, pos.y + 59);
	firework.startPos = pos;
	firework.timeStep = 0;
	firework.frame = 0;
	firework.nextFrameTime = now + 40;
	firework.active = true;
}

void ShelterFinal::moveFirework(FireworkState &firework) {
	const double time = static_cast<double>(firework.timeStep);
	const int y = static_cast<int>(static_cast<double>(firework.startPos.y) - 2.0 * time + 0.06 * time * time);
	firework.timeStep += 1;
	firework.collisionRect.top = y;
	firework.collisionRect.right = firework.collisionRect.left + 60;
	firework.collisionRect.bottom = y + 59;
}

bool ShelterFinal::isFireworkPositionFree(const Common::Point32 &pos) const {
	const Common::Point32 corners[4] = {
		pos,
		Common::Point32(pos.x + 60, pos.y),
		Common::Point32(pos.x, pos.y + 59),
		Common::Point32(pos.x + 60, pos.y + 59)};
	Common::Rect fixedRects[2] = {Common::Rect(153, 3, 647, 416), Common::Rect(0, 387, 800, 600)};
	for (int i = 0; i < kFireworkCount; i++) {
		const Common::Rect &rect = _fireworks[i].collisionRect;
		for (int corner = 0; corner < 4; corner++) {
			if (pointInsidePaddedRect(corners[corner], rect))
				return false;
		}
	}
	for (int i = 0; i < 2; i++) {
		const Common::Rect &rect = fixedRects[i];
		for (int corner = 0; corner < 4; corner++) {
			if (pointInsidePaddedRect(corners[corner], rect))
				return false;
		}
	}
	return true;
}

bool ShelterFinal::pointInsidePaddedRect(const Common::Point32 &pos, const Common::Rect &rect) {
	return rect.left - 15 < pos.x && pos.x < rect.right + 15 && rect.top - 15 < pos.y && pos.y < rect.bottom + 15;
}

void ShelterFinal::scheduleNextAmbient(uint32 now) {
	_nextAmbientTime = now + 1000 * (_vm->_rnd->getRandomNumber(9) + 10);
}

void ShelterFinal::playRandomAmbient() {
	SoundManager *sound = _vm->getSoundManager();
	if (!sound)
		return;
	const int index = _vm->_rnd->getRandomNumber(kAmbientSoundCount - 1);
	if (0 <= _ambientSoundIds[index])
		sound->playWithVolume(_ambientSoundIds[index], sound->_volumeSpeech);
}

void ShelterFinal::drawAnimation(const Animation *animation, int frameIndex, const Common::Point32 &pos, ManagedSurface32 *screen) const {
	if (!animation || animation->getFrameCount() <= 0)
		return;
	const RleBlock *frame = animation->getFrame(frameIndex % animation->getFrameCount());
	if (frame)
		frame->drawToScreen(screen, pos, _vm->getAlphaLUT());
}

void ShelterFinal::drawZoombini(const ZoombiniRunner &zoombini, ManagedSurface32 *screen) const {
	_vm->_gfx->drawZoombiniRunner(screen, &zoombini);
}

} // End of namespace Zoombini2
