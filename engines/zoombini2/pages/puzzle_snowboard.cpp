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
#include "common/textconsole.h"

#include <math.h>

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_snowboard.h"
#include "zoombini2/random.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleSnowboard::kMusicPath;
constexpr const char *PuzzleSnowboard::kTraitFormat;
constexpr const char *PuzzleSnowboard::kPatFormat;
constexpr const char *PuzzleSnowboard::kBoardPath;
constexpr const char *PuzzleSnowboard::kBoardAnimPath;
constexpr const char *PuzzleSnowboard::kEngineAnimPath;
constexpr const char *PuzzleSnowboard::kDecorFormat;
constexpr const char *PuzzleSnowboard::kSurfFormat;
constexpr int PuzzleSnowboard::kSurfIds[9];
constexpr const char *PuzzleSnowboard::kAreaMaskPath;
constexpr const char *PuzzleSnowboard::kBoardReadySoundPath;
constexpr const char *PuzzleSnowboard::kRideSoundPath;
constexpr const char *PuzzleSnowboard::kObstacleHitSoundPath;
constexpr const char *PuzzleSnowboard::kObstacleRevealSoundPath;
constexpr const char *PuzzleSnowboard::kObstacleHideSoundPath;
constexpr const char *PuzzleSnowboard::kCollisionSpeechFormat;
constexpr const char *PuzzleSnowboard::kNearQuotaSpeechFormat;
constexpr const char *PuzzleSnowboard::kQuotaSpeechFormat;
constexpr const char *PuzzleSnowboard::kSuccessSpeechPath;
constexpr const char *PuzzleSnowboard::kFailureSpeechPath;
constexpr const char *PuzzleSnowboard::kPerfectGoSpeechFormat;
constexpr const char *PuzzleSnowboard::kCaveGoSpeechPath;
constexpr const char *PuzzleSnowboard::kCelebrationAnimationPath;

constexpr Common::Point32 PuzzleSnowboard::kStartPositions[8];

constexpr Common::Point32 PuzzleSnowboard::kObstaclePositions[kLaneCount];

constexpr PuzzleSnowboard::PathPoint PuzzleSnowboard::kExitPoints[kEndpointCount];

constexpr Common::Point32 PuzzleSnowboard::kEasyHints[3];

constexpr Common::Point32 PuzzleSnowboard::kHardHints[4];

PuzzleSnowboard::PuzzleSnowboard(Zoombini2Engine *vm) : PuzzleBase(vm, kPageSnowboard) {
}

PuzzleSnowboard::~PuzzleSnowboard() {
	if (SoundManager *sound = _vm->getSoundManager()) {
		sound->unload(_boardReadySound);
		sound->unload(_rideSound);
		sound->unload(_obstacleHitSound);
		sound->unload(_obstacleRevealSound);
		sound->unload(_obstacleHideSound);
		if (0 <= _speechSoundId) {
			sound->stop(_speechSoundId);
			sound->unload(_speechSoundId);
		}
	}
	delete _boardAnim;
	delete _engineAnim;
	for (int index = 0; index < 5; index++)
		delete _obstacleAnims[index];
	finishPuzzleRoster(_vm->_state->_rescue2Board);
}

void PuzzleSnowboard::loadGraphics() {
	for (int trait = 0; trait < ZmbTrait::kTraitCount; trait++) {
		for (int value = 0; value < ZmbTrait::kTraitValueCount; value++) {
			const Common::String path = Common::String::format(kTraitFormat, trait + 1, value + 1);
			_vm->_gfx->loadPageRleBlock(path);
		}
	}
	_vm->_gfx->loadPageRleBlock(kBoardPath);
	_boardAnim = new Animation(_vm);
	if (!_boardAnim->loadFromFile(Common::Path(kBoardAnimPath))) {
		delete _boardAnim;
		_boardAnim = nullptr;
	}
	_engineAnim = new Animation(_vm);
	if (!_engineAnim->loadFromFile(Common::Path(kEngineAnimPath))) {
		delete _engineAnim;
		_engineAnim = nullptr;
	}
	static constexpr int kDecorIds[5] = {
		1,
		3,
		4,
		5,
		6,
	};
	for (int index = 0; index < 5; index++) {
		_obstacleAnims[index] = new Animation(_vm);
		const Common::Path path(Common::String::format(kDecorFormat, kDecorIds[index]));
		if (!_obstacleAnims[index]->loadFromFile(path)) {
			delete _obstacleAnims[index];
			_obstacleAnims[index] = nullptr;
		}
	}
	for (int index = 0; index < 9; index++) {
		const Common::String path = Common::String::format(kSurfFormat, kSurfIds[index]);
		if (!_vm->_gfx->loadPageRleBlock(path))
			warning("Snowboard: missing rider board '%s'", path.c_str());
	}
}

byte PuzzleSnowboard::pickPresentValue(ZmbTrait::TraitIndex traitIndex) {
	for (;;) {
		const byte value = static_cast<byte>(_vm->_rnd->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1);
		for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
			if (_puzzleZoombinis[index] && _puzzleZoombinis[index]->_traits.getValue(traitIndex) == value)
				return value;
		}
	}
}

int PuzzleSnowboard::classifyZoombini(const ZoombiniRunner *zoombini) const {
	int nodeIndex = 0;
	while (nodeIndex < kRuleCount) {
		const TreeNode &node = _tree[nodeIndex];
		const byte value = zoombini->_traits.getValue(node.traitIndex);
		const bool matches = value == node.primaryValue || (_puzzleLevel == 3 && value == node.alternateValue);
		nodeIndex = 2 * nodeIndex + (matches ? 1 : 2);
	}
	return nodeIndex - kRuleCount;
}

void PuzzleSnowboard::countLanes(byte (&counts)[kLaneCount]) const {
	for (int lane = 0; lane < kLaneCount; lane++)
		counts[lane] = 0;
	for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
		const ZoombiniRunner *zoombini = _puzzleZoombinis[index];
		if (zoombini && zoombini->_inputEnabled)
			counts[classifyZoombini(zoombini)] += 1;
	}
}

void PuzzleSnowboard::generateTree() {
	if (_puzzleZoombinis.empty())
		return;
	for (int attempt = 0; attempt < 5000; attempt++) {
		_tree[0].traitIndex = static_cast<ZmbTrait::TraitIndex>(_vm->_rnd->getRandomNumber(ZmbTrait::kTraitCount - 1));
		_tree[0].primaryValue = pickPresentValue(_tree[0].traitIndex);
		if (_puzzleLevel == 3) {
			do {
				_tree[0].alternateValue = pickPresentValue(_tree[0].traitIndex);
			} while (_tree[0].alternateValue == _tree[0].primaryValue);
		}
		do {
			_tree[1].traitIndex = static_cast<ZmbTrait::TraitIndex>(_vm->_rnd->getRandomNumber(ZmbTrait::kTraitCount - 1));
		} while (_tree[1].traitIndex == _tree[0].traitIndex);
		_tree[1].primaryValue = pickPresentValue(_tree[1].traitIndex);
		if (_puzzleLevel == 3) {
			do {
				_tree[1].alternateValue = pickPresentValue(_tree[1].traitIndex);
			} while (_tree[1].alternateValue == _tree[1].primaryValue);
		}
		_tree[2] = _tree[1];
		byte laneCounts[kLaneCount];
		countLanes(laneCounts);
		int score = 10;
		for (int lane = 0; lane < kLaneCount; lane++) {
			if (laneCounts[lane] == 0)
				score -= 3;
			else if (laneCounts[lane] == 1)
				score -= 1;
			else if (5 < laneCounts[lane])
				score += 5 - laneCounts[lane];
		}
		if (7 <= score)
			break;
	}
}

void PuzzleSnowboard::init() {
	PuzzleBase::init();
	startPageMusic(Common::Path(kMusicPath));
	if (_puzzleLevel < 1 || 3 < _puzzleLevel) {
		warning("Snowboard: unsupported difficulty %d for this page", _puzzleLevel);
		_finished = true;
		return;
	}
	_collisionQuota = _puzzleLevel == 1 ? 2 : 4;
	loadGraphics();
	_celebrationAnimation = _vm->loadZoombiniAnimation(Common::Path(kCelebrationAnimationPath), 50);
	loadAreaMask(Common::Path(kAreaMaskPath));
	if (SoundManager *sound = _vm->getSoundManager()) {
		_boardReadySound = sound->load(false, Common::Path(kBoardReadySoundPath), false);
		_rideSound = sound->load(false, Common::Path(kRideSoundPath), false);
		_obstacleHitSound = sound->load(false, Common::Path(kObstacleHitSoundPath), false);
		_obstacleRevealSound = sound->load(false, Common::Path(kObstacleRevealSoundPath), false);
		_obstacleHideSound = sound->load(false, Common::Path(kObstacleHideSoundPath), false);
	}
	for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
		ZoombiniRunner *zoombini = _puzzleZoombinis[index];
		if (!zoombini)
			continue;
		zoombini->setDefaultAnimation(_zoombiniAnimation);
		zoombini->setPosition(kStartPositions[index % 8]);
		zoombini->_inputEnabled = true;
		zoombini->_hidden = false;
		zoombini->_puzzleStatus = 0;
	}
	generateTree();
	ZmbDropTarget boardTarget;
	boardTarget.rect = Common::Rect32(290, 5, 670, 85);
	boardTarget.callback = &PuzzleSnowboard::onBoardDrop;
	boardTarget.callbackContext = this;
	_dropTargets.push_back(boardTarget);
}

/* static */
void PuzzleSnowboard::onBoardDrop(void *context, int targetIndex, int zoombiniIndex) {
	(void)targetIndex;
	static_cast<PuzzleSnowboard *>(context)->captureZoombini(zoombiniIndex);
}

void PuzzleSnowboard::captureZoombini(int zoombiniIndex) {
	if (_finished || _activeRunnerIndex != -1 || zoombiniIndex < 0 || _puzzleZoombinis.size() <= static_cast<uint>(zoombiniIndex))
		return;
	ZoombiniRunner *zoombini = _puzzleZoombinis[zoombiniIndex];
	if (!zoombini || !zoombini->_inputEnabled)
		return;
	_targetRouteCode = classifyZoombini(zoombini) + kRuleCount;
	const Common::Path path(Common::String::format(kPatFormat, _targetRouteCode));
	PathObject *ridePath = PathObject::loadFromPAT(_vm, path);
	if (!ridePath) {
		warning("Snowboard: missing ride path '%s'", path.toString().c_str());
		return;
	}
	_activeRunnerIndex = zoombiniIndex;
	_boardReady = false;
	_onExitPath = false;
	const uint32 now = _vm->getGameTickCount();
	_boardFacingSector = 8;
	_boardFacingDelay = 100;
	for (int lane = 0; lane < kLaneCount; lane++)
		_obstacleHit[lane] = false;
	zoombini->_inputEnabled = false;
	zoombini->startMovement(ridePath, now);
	zoombini->startDirectionTrackedAnimation(now);
	bool hasSelectableRider = false;
	for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
		if (_puzzleZoombinis[index] && _puzzleZoombinis[index]->_inputEnabled) {
			hasSelectableRider = true;
			break;
		}
	}
	if (!hasSelectableRider) {
		_boardCoverAnimationStart = now;
		_boardCoverActive = true;
	}
	if (SoundManager *sound = _vm->getSoundManager())
		sound->playWithVolume(_rideSound, sound->_volumeSFX);
}

void PuzzleSnowboard::chooseOpenLane() {
	byte laneCounts[kLaneCount];
	countLanes(laneCounts);
	int available = 0;
	for (int lane = 0; lane < kLaneCount; lane++)
		available += laneCounts[lane];
	if (!available) {
		_obstacleChangePending = false;
		return;
	}
	int openLane;
	do {
		openLane = _vm->_rnd->getRandomNumber(kLaneCount - 1);
	} while (laneCounts[openLane] == 0);
	const uint32 now = _vm->getGameTickCount();
	for (int lane = 0; lane < kLaneCount; lane++) {
		const bool visible = lane != openLane;
		if (visible == _obstacleVisible[lane])
			continue;
		_obstacleVisible[lane] = visible;
		if (visible) {
			_obstacleHiding[lane] = false;
			_obstacleRevealStart[lane] = now;
			if (SoundManager *sound = _vm->getSoundManager())
				sound->playWithVolume(_obstacleRevealSound, sound->_volumeSFX);
		} else {
			_obstacleHiding[lane] = true;
			_obstacleHideStart[lane] = now;
			if (SoundManager *sound = _vm->getSoundManager())
				sound->playWithVolume(_obstacleHideSound, sound->_volumeSFX);
		}
	}
	_obstacleChangePending = false;
}

void PuzzleSnowboard::checkObstacleCollision(ZoombiniRunner *zoombini) {
	if (_collisionSpeechPending)
		return;
	for (int lane = 0; lane < kLaneCount; lane++) {
		if (!_obstacleVisible[lane] || _obstacleHit[lane])
			continue;
		const Common::Point32 &position = kObstaclePositions[lane];
		if (position.x < zoombini->_screenPos.x && zoombini->_screenPos.x < position.x + 50 &&
			position.y < zoombini->_screenPos.y && zoombini->_screenPos.y < position.y + 50) {
			_obstacleHit[lane] = true;
			_obstacleHitAnimating[lane] = true;
			_obstacleHitStart[lane] = _vm->getGameTickCount();
			if (SoundManager *sound = _vm->getSoundManager())
				sound->playWithVolume(_obstacleHitSound, sound->_volumeSFX);
			const int variant = _vm->_rnd->getRandomNumber(2) + 1;
			Common::String speechPath;
			if (_collisionCount == _collisionQuota)
				speechPath = Common::String::format(kQuotaSpeechFormat, variant);
			else if (_collisionCount == _collisionQuota - 1)
				speechPath = Common::String::format(kNearQuotaSpeechFormat, variant);
			else
				speechPath = Common::String::format(kCollisionSpeechFormat, lane + 1, variant);
			enqueueSpeech(speechPath, true);
			_collisionCount += 1;
			_hitsSinceRetarget += 1;
			if (_hitsSinceRetarget == 2) {
				_hitsSinceRetarget = 0;
				_obstacleChangePending = true;
			}
			break;
		}
	}
}

void PuzzleSnowboard::enqueueSpeech(const Common::String &path, bool collision) {
	SpeechEntry entry;
	entry.path = path;
	entry.collision = collision;
	_speechQueue.push_back(entry);
	if (collision)
		_collisionSpeechPending = true;
	pumpSpeechQueue();
}

void PuzzleSnowboard::pumpSpeechQueue() {
	SoundManager *sound = _vm->getSoundManager();
	if (!sound) {
		_speechSoundId = -1;
		_nextSpeechIndex = _speechQueue.size();
		_collisionSpeechPending = false;
		return;
	}
	if (0 <= _speechSoundId) {
		if (sound->isPlaying(_speechSoundId))
			return;
		sound->unload(_speechSoundId);
		_speechSoundId = -1;
		if (_activeSpeechIsCollision)
			_collisionSpeechPending = false;
	}
	while (_nextSpeechIndex < _speechQueue.size()) {
		const SpeechEntry &entry = _speechQueue[_nextSpeechIndex];
		_nextSpeechIndex += 1;
		_speechSoundId = sound->load(true, Common::Path(entry.path), false);
		if (0 <= _speechSoundId) {
			_activeSpeechIsCollision = entry.collision;
			sound->playWithVolume(_speechSoundId, sound->_volumeSpeech);
			return;
		}
		if (entry.collision)
			_collisionSpeechPending = false;
	}
}

bool PuzzleSnowboard::hasActiveHitAnimation(uint32 now) const {
	const Animation *animation = _obstacleAnims[kHitAnimIndex];
	if (!animation)
		return false;
	const uint32 duration = animation->getFrameCount() * 50;
	for (int lane = 0; lane < kLaneCount; lane++) {
		if (_obstacleHitAnimating[lane] && now - _obstacleHitStart[lane] < duration)
			return true;
	}
	return false;
}

void PuzzleSnowboard::startExitPath(ZoombiniRunner *zoombini) {
	const uint32 now = _vm->getGameTickCount();
	if (SoundManager *sound = _vm->getSoundManager()) {
		sound->stop(_rideSound);
		sound->playWithVolume(_boardReadySound, sound->_volumeSFX);
	}
	_generatorAnimationStart = now;
	_generatorActive = true;
	for (int index = 0; index < kEndpointCount; index++) {
		if (_usedExitPoint[index] || kExitPoints[index].routeCode != _targetRouteCode)
			continue;
		_usedExitPoint[index] = true;
		const Common::Point32 start = zoombini->_screenPos;
		const Common::Point32 end(kExitPoints[index].x, kExitPoints[index].y);
		const int dx = end.x - start.x;
		const int dy = end.y - start.y;
		PathObject *path = new PathObject(_vm);
		path->appendSegment(start, Common::Point32(start.x + dx / 3, start.y + dy / 3),
							Common::Point32(end.x - dx / 3, end.y - dy / 3), end, 10, 0);
		zoombini->startMovement(path, now);
		_onExitPath = true;
		return;
	}
	warning("Snowboard: no exit point for route %d", _targetRouteCode);
	finishRide(zoombini);
}

void PuzzleSnowboard::finishRide(ZoombiniRunner *zoombini) {
	zoombini->clearMovement();
	zoombini->resetAnimation();
	_vm->_zoombiniWalkingFlag = true;
	zoombini->_puzzleStatus = 1;
	zoombini->_inputEnabled = false;
	_activeRunnerIndex = -1;
	_onExitPath = false;
	_ridesCompleted += 1;
	int remaining = 0;
	int successful = 0;
	for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
		if (!_puzzleZoombinis[index])
			continue;
		if (_puzzleZoombinis[index]->_puzzleStatus == 0)
			remaining += 1;
		else if (_puzzleZoombinis[index]->_puzzleStatus == 1)
			successful += 1;
	}
	if (remaining == 0 || _collisionQuota < _collisionCount) {
		_finished = true;
		_boardCoverActive = false;
		_obstacleChangePending = false;
		_finishAnimationStart = _vm->getGameTickCount();
		enqueueSpeech(successful == 8 ? kSuccessSpeechPath : kFailureSpeechPath, false);
		_vm->restartGoBlink();
	} else if (2 <= _ridesCompleted) {
		_obstacleChangePending = true;
	}
}

void PuzzleSnowboard::onUpdate() {
	const uint32 now = _vm->getGameTickCount();
	pumpSpeechQueue();
	if (_goTransitionPending && _speechSoundId < 0 && _nextSpeechIndex == _speechQueue.size()) {
		_goTransitionPending = false;
		_vm->_mapTransitionSourcePageId = kPageSnowboard;
		_vm->requestPageChange(kPageMapTrans);
	}
	if (_boardCoverActive && _boardAnim && static_cast<uint32>(_boardAnim->getFrameCount() * 100) <= now - _boardCoverAnimationStart)
		_boardCoverActive = false;
	if (_generatorActive && 1000 <= now - _generatorAnimationStart) {
		_generatorActive = false;
		_boardReady = true;
	}
	if (_obstacleChangePending && !_finished && !_collisionSpeechPending && !hasActiveHitAnimation(now))
		chooseOpenLane();
	if (_finished && _celebrationAnimation) {
		for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
			ZoombiniRunner *zoombini = _puzzleZoombinis[index];
			if (zoombini && zoombini->_puzzleStatus == 1 && !zoombini->_animationActive && _vm->_rnd->getRandomNumber(19) == 1)
				zoombini->startAnimation(_celebrationAnimation, 33, now, ZoombiniRunner::AnimationCompletionPolicy::kBypassCallbackAndCorrection01);
		}
	}
	for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
		if (_puzzleZoombinis[index])
			_puzzleZoombinis[index]->updateAnimation(now);
	}
	if (_activeRunnerIndex == -1 || _puzzleZoombinis.size() <= static_cast<uint>(_activeRunnerIndex)) {
		if (!_dropTargets.empty())
			_dropTargets[0].occupied = false;
		return;
	}
	ZoombiniRunner *zoombini = _puzzleZoombinis[_activeRunnerIndex];
	if (!zoombini || !zoombini->_movementPath)
		return;
	const bool moving = zoombini->advanceMovement(now, _onExitPath ? Common::Point32() : Common::Point32(20, 30));
	if (!_onExitPath) {
		updateBoardFacing(zoombini);
		checkObstacleCollision(zoombini);
	}
	if (!moving) {
		if (_onExitPath)
			finishRide(zoombini);
		else
			startExitPath(zoombini);
	}
}

void PuzzleSnowboard::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

void PuzzleSnowboard::drawTraitHint(ManagedSurface32 *screen, const TreeNode &node, bool alternate, const Common::Point32 &pos) const {
	const int trait = static_cast<int>(node.traitIndex);
	const byte value = alternate ? node.alternateValue : node.primaryValue;
	if (0 <= trait && trait < ZmbTrait::kTraitCount && 1 <= value && value <= ZmbTrait::kTraitValueCount)
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kTraitFormat, trait + 1, value), pos);
}

void PuzzleSnowboard::updateBoardFacing(ZoombiniRunner *zoombini) {
	if (_boardFacingDelay <= 2) {
		_boardFacingDelay += 1;
		return;
	}
	const int deltaX = zoombini->_previousScreenPos.x - zoombini->_screenPos.x;
	const int deltaY = zoombini->_previousScreenPos.y - zoombini->_screenPos.y;
	const double distance = sqrt(static_cast<double>(deltaX) * deltaX + static_cast<double>(deltaY) * deltaY);
	if (distance == 0.0)
		return;
	int direction = static_cast<int>(acos(CLIP(static_cast<double>(deltaX) / distance, -1.0, 1.0)) * 180.0 * 0.31831926) / 20;
	if (zoombini->_screenPos.y < zoombini->_previousScreenPos.y)
		direction = -direction;
	if (_boardFacingSector < direction)
		_boardFacingSector += 1;
	else if (direction < _boardFacingSector)
		_boardFacingSector -= 1;
	_boardFacingSector = CLIP(_boardFacingSector, 0, 8);
	static constexpr int kFacingCells[9] = {
		44,
		41,
		11,
		12,
		22,
		23,
		33,
		36,
		66,
	};
	zoombini->_animationCell = kFacingCells[_boardFacingSector];
	_boardFacingDelay = 0;
}

void PuzzleSnowboard::drawObstacle(ManagedSurface32 *screen, int lane, uint32 now) const {
	const Common::Point32 &position = kObstaclePositions[lane];
	if (_finished) {
		const Animation *finish = _obstacleAnims[kFinishAnimIndex];
		if (finish && now - _finishAnimationStart < static_cast<uint32>(finish->getFrameCount() * 50))
			_vm->_gfx->drawAnimationFrame(screen, finish, (now - _finishAnimationStart) / 50, position);
		return;
	}
	if (_obstacleHiding[lane]) {
		const Animation *hide = _obstacleAnims[kHideAnimIndex];
		if (hide && now - _obstacleHideStart[lane] < static_cast<uint32>(hide->getFrameCount() * 50))
			_vm->_gfx->drawAnimationFrame(screen, hide, (now - _obstacleHideStart[lane]) / 50, position);
		return;
	}
	if (_obstacleHitAnimating[lane]) {
		const Animation *hit = _obstacleAnims[kHitAnimIndex];
		if (hit && now - _obstacleHitStart[lane] < static_cast<uint32>(hit->getFrameCount() * 50)) {
			_vm->_gfx->drawAnimationFrame(screen, hit, (now - _obstacleHitStart[lane]) / 50, position);
			return;
		}
	}
	const uint32 elapsed = now - _obstacleRevealStart[lane];
	const Animation *intro = _obstacleAnims[kRevealAnimIndex];
	if (intro && intro->getFrameCount() != 0 && elapsed < static_cast<uint32>(intro->getFrameCount() * 50)) {
		const int frame = elapsed / 50;
		_vm->_gfx->drawAnimationFrame(screen, intro, frame, position);
		return;
	}
	const Animation *idle = _obstacleAnims[kIdleAnimIndex];
	if (!idle || idle->getFrameCount() == 0)
		return;
	const uint32 introDuration = intro ? intro->getFrameCount() * 50 : 0;
	const uint32 firstDuration = static_cast<uint32>(lane + 1) * 100;
	const uint32 cycleDuration = firstDuration + (idle->getFrameCount() - 1) * 100;
	const uint32 idleElapsed = (elapsed - introDuration) % cycleDuration;
	const int frame = idleElapsed < firstDuration ? 0 : 1 + (idleElapsed - firstDuration) / 100;
	_vm->_gfx->drawAnimationFrame(screen, idle, frame, position);
}

void PuzzleSnowboard::onRenderContent(ManagedSurface32 *screen) {
	const uint32 now = _vm->getGameTickCount();
	if (_generatorActive && _engineAnim && 0 < _engineAnim->getFrameCount()) {
		const int frame = ((now - _generatorAnimationStart) / 100) % 5 % _engineAnim->getFrameCount();
		_vm->_gfx->drawAnimationFrame(screen, _engineAnim, frame, Common::Point32(153, 0));
	}
	if (_boardReady)
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSurfFormat, kSurfIds[0]), Common::Point32(300, 1));
	if (_puzzleLevel == 3) {
		drawTraitHint(screen, _tree[0], false, kHardHints[0]);
		drawTraitHint(screen, _tree[0], true, kHardHints[1]);
		for (int branch = 0; branch < 2; branch++) {
			const Common::Point32 offset(branch * 70, 0);
			drawTraitHint(screen, _tree[1], false, kHardHints[2] + offset);
			drawTraitHint(screen, _tree[1], true, kHardHints[3] + offset);
		}
	} else {
		drawTraitHint(screen, _tree[0], false, kEasyHints[0]);
		drawTraitHint(screen, _tree[1], false, kEasyHints[1]);
		drawTraitHint(screen, _tree[1], false, kEasyHints[2]);
	}
	if (_puzzleLevel != 1 && !_finished)
		_vm->_gfx->drawPageRleBlock(screen, kBoardPath, Common::Point32(580, 58));
	if (_boardCoverActive && _puzzleLevel != 1 && !_finished && _boardAnim && 0 < _boardAnim->getFrameCount()) {
		const int frame = (now - _boardCoverAnimationStart) / 100;
		if (frame < _boardAnim->getFrameCount())
			_vm->_gfx->drawAnimationFrame(screen, _boardAnim, frame, Common::Point32(580, 58));
	}
	for (int lane = 0; lane < kLaneCount; lane++) {
		if (_finished && !_obstacleVisible[lane])
			continue;
		if (!_obstacleVisible[lane] && !_obstacleHiding[lane])
			continue;
		drawObstacle(screen, lane, now);
	}
}

void PuzzleSnowboard::onRenderActors(ManagedSurface32 *screen) {
	if (_activeRunnerIndex != -1 && _puzzleZoombinis.size() > static_cast<uint>(_activeRunnerIndex) && !_onExitPath) {
		const ZoombiniRunner *zoombini = _puzzleZoombinis[_activeRunnerIndex];
		if (zoombini) {
			_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSurfFormat, kSurfIds[8 - _boardFacingSector]),
										Common::Point32(zoombini->_screenPos.x - 5, zoombini->_screenPos.y + 10));
		}
	}
	renderZoombinis(screen);
}

bool PuzzleSnowboard::canUseGoButton() const {
	return _vm->_zoombiniWalkingFlag;
}

bool PuzzleSnowboard::onGoButtonPressed() {
	if (!_vm->_isSavedGame)
		return true;
	if (_goTransitionPending)
		return false;
	int freeCount = 0;
	for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
		if (_puzzleZoombinis[index] && _puzzleZoombinis[index]->_puzzleStatus == 0)
			freeCount += 1;
	}
	if (freeCount == 0) {
		const int variant = _vm->_rnd->getRandomNumber(4) + 1;
		enqueueSpeech(Common::String::format(kPerfectGoSpeechFormat, variant), false);
	} else if (4 <= freeCount) {
		enqueueSpeech(kCaveGoSpeechPath, false);
	} else {
		return true;
	}
	_goTransitionPending = true;
	return false;
}

void PuzzleSnowboard::onActorsRendered() {
	for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
		if (_puzzleZoombinis[index])
			_puzzleZoombinis[index]->advanceAnimationAfterDraw();
	}
}

EventHandleResult PuzzleSnowboard::onLButtonUp(const Common::Point &pos) {
	const bool acceptRelease = !_finished && _activeRunnerIndex == -1;
	const ZmbDropResult result = ZoombiniRunner::handlePointerInput(_puzzleZoombinis, Common::Point32(pos.x, pos.y),
																		  acceptRelease, _zoombiniAnimation, _vm->getGameTickCount(), &_dropTargets, getAreaMask());
	return result == ZmbDropResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

EventHandleResult PuzzleSnowboard::onMouseMove(const Common::Point &pos) {
	const ZmbDropResult result = ZoombiniRunner::handlePointerInput(_puzzleZoombinis, Common::Point32(pos.x, pos.y), false,
																		  _zoombiniAnimation, _vm->getGameTickCount(), &_dropTargets, getAreaMask());
	return result == ZmbDropResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

Common::String PuzzleSnowboard::debugGetAnswer() const {
	Common::String answer = debugAnswerHeader();
	for (int lane = 0; lane < kLaneCount; lane++)
		answer += Common::String::format("Lane %d, obstacle (%d,%d): %s\n", lane + 1, kObstaclePositions[lane].x,
										 kObstaclePositions[lane].y, _obstacleVisible[lane] ? "blocked" : "open");
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		const ZoombiniRunner *actor = _puzzleZoombinis[i];
		const int lane = classifyZoombini(actor);
		const char *obstacleState = "open";
		if (_obstacleVisible[lane])
			obstacleState = "blocked";
		const char *inputState = "";
		if (actor->_inputEnabled)
			inputState = " - waiting";
		answer += Common::String::format("%s -> lane %d (%s)%s\n", debugActorDescription(i).c_str(), lane + 1,
										 obstacleState, inputState);
	}
	return answer + "Recheck after a lane change; safe riders depend on the current obstacle positions.\n";
}

PuzzleChanceInfo PuzzleSnowboard::debugGetChances() const {
	return PuzzleChanceInfo(PuzzleChanceInfo::Type::kMistake, _collisionQuota + 1, _collisionCount, "obstacle contact");
}

bool PuzzleSnowboard::debugCanSetChances() const {
	return !_debugFinishPending && !_finished && !_goTransitionPending && _activeRunnerIndex == -1 && !_collisionSpeechPending;
}

bool PuzzleSnowboard::debugSetChances(int remaining) {
	if (!debugCanSetChances() || remaining < 0 || _collisionQuota + 1 < remaining)
		return false;
	_collisionCount = _collisionQuota + 1 - remaining;
	if (!remaining) {
		_finished = true;
		_boardCoverActive = false;
		_obstacleChangePending = false;
		_finishAnimationStart = _vm->getGameTickCount();
		enqueueSpeech(kFailureSpeechPath, false);
		_vm->restartGoBlink();
	}
	return true;
}

} // End of namespace Zoombini2
