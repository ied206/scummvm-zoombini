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
#include "common/ptr.h"
#include "common/textconsole.h"

#include <math.h>

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_snowboard.h"
#include "zoombini2/random.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleSnowboard::kPageName;
constexpr const char *PuzzleSnowboard::kBackgroundPath;
constexpr const char *PuzzleSnowboard::kMusicPath;
constexpr const char *PuzzleSnowboard::kTraitFormat;
constexpr const char *PuzzleSnowboard::kPatFormat;
constexpr const char *PuzzleSnowboard::kL4PatFormat;
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

constexpr Common::Point32 PuzzleSnowboard::kStartPositions[8];

constexpr Common::Point32 PuzzleSnowboard::kObstaclePositions[kLaneCount];
constexpr Common::Point32 PuzzleSnowboard::kL4ObstaclePositions[kL4LaneCount];

constexpr PuzzleSnowboard::PathPoint PuzzleSnowboard::kExitPoints[kEndpointCount];
constexpr PuzzleSnowboard::PathPoint PuzzleSnowboard::kL4ExitPoints[kL4EndpointCount];

constexpr Common::Point32 PuzzleSnowboard::kEasyHints[3];

constexpr Common::Point32 PuzzleSnowboard::kHardHints[4];
constexpr Common::Point32 PuzzleSnowboard::kL4Hints[6];

PuzzleSnowboard::PuzzleSnowboard(Zoombini2Engine *vm)
	: PuzzleBase(vm, PageId::kPageSnowboard_10) {
	_initialBackgroundPath = kBackgroundPath;
}

PuzzleSnowboard::~PuzzleSnowboard() {
	delete _l4TrailOverlay;
	if (SoundManager *sound = _vm->getSoundManager()) {
		sound->unload(_boardReadySound);
		sound->unload(_rideSound);
		sound->unload(_obstacleHitSound);
		sound->unload(_obstacleRevealSound);
		sound->unload(_obstacleHideSound);
	}
	delete _boardAnim;
	delete _engineAnim;
	for (int i = 0; i < ARRAYSIZE(_obstacleAnims); i++)
		delete _obstacleAnims[i];
	finishPuzzleRoster(_vm->_state->_rescue2Storage);
}

void PuzzleSnowboard::loadGraphics() {
	for (int trait = 0; trait < ZmbTrait::kTraitKindCount; trait++) {
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
	for (int i = 0; i < ARRAYSIZE(_obstacleAnims); i++) {
		_obstacleAnims[i] = new Animation(_vm);
		const Common::Path path(Common::String::format(kDecorFormat, kDecorIds[i]));
		if (!_obstacleAnims[i]->loadFromFile(path)) {
			delete _obstacleAnims[i];
			_obstacleAnims[i] = nullptr;
		}
	}
	for (int i = 0; i < ARRAYSIZE(kSurfIds); i++) {
		const Common::String path = Common::String::format(kSurfFormat, kSurfIds[i]);
		if (!_vm->_gfx->loadPageRleBlock(path))
			warning("Snowboard: missing rider board '%s'", path.c_str());
	}
}

void PuzzleSnowboard::drawL4Trail(ManagedSurface32 *screen) {
	if (!_l4TrailOverlay) {
		_l4TrailOverlay = new ManagedSurface32(Size32(screen->w, screen->h), screen->format);

		const uint32 transparentKey = screen->format.ARGBToColor(0, 0, 0, 0);
		_l4TrailOverlay->fillRect(Common::Rect32(0, 0, _l4TrailOverlay->w, _l4TrailOverlay->h), transparentKey);

		// Omit the upper approach in the HARD PAT routes and draw only the slope below y=70.
		const Common::Rect32 sampleBounds(0, 70, _l4TrailOverlay->w, _l4TrailOverlay->h);
		const uint32 trailColor = screen->format.ARGBToColor(128, 255, 255, 255);
		for (int lane = 0; lane < kL4LaneCount; lane++) {
			const Common::Path filename(Common::String::format(kL4PatFormat, lane + kMaxRuleCount));
			Common::ScopedPtr<PathObject> path(_vm->loadPagePATPath(filename));
			if (!path)
				continue;
			for (uint sIdx = 0; sIdx < path->segments.size(); sIdx++)
				_vm->_gfx->drawCubicBezierCurve(_l4TrailOverlay, *path->segments[sIdx], trailColor, 5, sampleBounds);
		}
	}
	// The sentinel disables color-key matching; the overlay's alpha channel controls transparency.
	const uint32 transBlitKey = screen->format.ARGBToColor(255, 255, 255, 255);
	screen->transBlitFrom(*_l4TrailOverlay, transBlitKey, false, 255);
}

void PuzzleSnowboard::drawL4Legend(ManagedSurface32 *screen) const {
	drawTraitHint(screen, _tree[0], false, kL4Hints[0]);
	drawTraitHint(screen, _tree[1], false, kL4Hints[1]);
	drawTraitHint(screen, _tree[4], false, kL4Hints[2]);
	drawTraitHint(screen, _tree[0], true, kL4Hints[3]);
	drawTraitHint(screen, _tree[1], true, kL4Hints[4]);
	drawTraitHint(screen, _tree[4], true, kL4Hints[5]);
}

byte PuzzleSnowboard::pickPresentValue(ZmbTrait::TraitKind traitIndex) {
	for (;;) {
		const byte value = static_cast<byte>(_vm->_rnd->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1);
		for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
			if (_puzzleZoombinis[i] && _puzzleZoombinis[i]->getTraits().getValue(traitIndex) == value)
				return value;
		}
	}
}

int PuzzleSnowboard::classifyZoombini(const ZoombiniRunner *zoombini) const {
	int nodeIndex = 0;
	while (nodeIndex < _ruleCount) {
		const TreeNode &node = _tree[nodeIndex];
		const byte value = zoombini->getTraits().getValue(node.traitIndex);
		const byte alternate = _puzzleLevel == 4 ? zoombini->getTraits().getValue(node.alternateTraitIndex) : value;
		const bool matches = value == node.primaryValue || (3 <= _puzzleLevel && alternate == node.alternateValue);
		nodeIndex = 2 * nodeIndex + (matches ? 1 : 2);
	}
	return nodeIndex - _ruleCount;
}

void PuzzleSnowboard::countLanes(Common::Array<byte> &counts) const {
	counts.resize(_laneCount);
	for (int lane = 0; lane < _laneCount; lane++)
		counts[lane] = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _puzzleZoombinis[i];
		if (zoombini && zoombini->isInputEnabled())
			counts[classifyZoombini(zoombini)] += 1;
	}
}

void PuzzleSnowboard::generateTree() {
	if (_puzzleZoombinis.empty())
		return;
	if (_puzzleLevel == 4) {
		generateL4Tree();
		return;
	}
	Common::Array<byte> laneCounts(_laneCount);
	for (int attempt = 0; attempt < 5000; attempt++) {
		_tree[0].traitIndex = static_cast<ZmbTrait::TraitKind>(_vm->_rnd->getRandomNumber(ZmbTrait::kTraitKindCount - 1));
		_tree[0].primaryValue = pickPresentValue(_tree[0].traitIndex);
		if (_puzzleLevel == 3) {
			do {
				_tree[0].alternateValue = pickPresentValue(_tree[0].traitIndex);
			} while (_tree[0].alternateValue == _tree[0].primaryValue);
		}
		do {
			_tree[1].traitIndex = static_cast<ZmbTrait::TraitKind>(_vm->_rnd->getRandomNumber(ZmbTrait::kTraitKindCount - 1));
		} while (_tree[1].traitIndex == _tree[0].traitIndex);
		_tree[1].primaryValue = pickPresentValue(_tree[1].traitIndex);
		if (_puzzleLevel == 3) {
			do {
				_tree[1].alternateValue = pickPresentValue(_tree[1].traitIndex);
			} while (_tree[1].alternateValue == _tree[1].primaryValue);
		}
		_tree[2] = _tree[1];
		countLanes(laneCounts);
		int score = 10;
		for (int lane = 0; lane < kLaneCount; lane++) {
			switch (laneCounts[lane]) {
			case 0:
				score -= 3;
				break;
			case 1:
				score -= 1;
				break;
			default:
				if (5 < laneCounts[lane])
					score += 5 - laneCounts[lane];
				break;
			}
		}
		if (7 <= score)
			break;
	}
}

bool PuzzleSnowboard::hasRepeatedL4Rule() const {
	return _tree[0].primaryValue == _tree[1].primaryValue &&
		   ((_tree[0].traitIndex == _tree[1].traitIndex) != _tree[0].alternateValue) == _tree[1].alternateValue &&
		   _tree[0].alternateTraitIndex == _tree[1].alternateTraitIndex && _tree[0].primaryValue == _tree[3].primaryValue &&
		   ((_tree[0].traitIndex == _tree[3].traitIndex) != _tree[0].alternateValue) == _tree[3].alternateValue &&
		   _tree[0].alternateTraitIndex == _tree[3].alternateTraitIndex && _tree[1].primaryValue == _tree[3].primaryValue &&
		   ((_tree[1].traitIndex == _tree[3].traitIndex) != _tree[1].alternateValue) == _tree[3].alternateValue &&
		   _tree[1].alternateTraitIndex == _tree[3].alternateTraitIndex;
}

void PuzzleSnowboard::generateL4Candidate() {
	do {
		for (int depth = 0; depth < 3; depth++) {
			TreeNode &node = _tree[(1 << depth) - 1];
			do {
				node.traitIndex = static_cast<ZmbTrait::TraitKind>(drawL4Value(ZmbTrait::kTraitKindCount));
				node.alternateTraitIndex = static_cast<ZmbTrait::TraitKind>(drawL4Value(ZmbTrait::kTraitKindCount));
				node.primaryValue = static_cast<byte>(drawL4Value(ZmbTrait::kTraitValueCount) + 1);
				node.alternateValue = static_cast<byte>(drawL4Value(ZmbTrait::kTraitValueCount) + 1);
			} while (node.traitIndex == node.alternateTraitIndex);
		}
		_tree[2] = _tree[1];
		for (int i = 4; i < kMaxRuleCount; i++)
			_tree[i] = _tree[3];
	} while (hasRepeatedL4Rule());
}

int PuzzleSnowboard::drawL4Value(int count) {
	const int sample = _vm->_rnd->getRandomNumber(0x7FFF);
	return count * sample / 0x8000;
}

void PuzzleSnowboard::generateL4Tree() {
	TreeNode best[kMaxRuleCount] = {};
	Common::Array<byte> laneCounts(_laneCount);
	int bestScore = -3 * kL4LaneCount;
	const int maximumScore = 10 - 3 * _laneCount + 2 * MIN(_laneCount, static_cast<int>(_puzzleZoombinis.size()));
	for (int attempt = 0; attempt < 5000; attempt++) {
		generateL4Candidate();
		countLanes(laneCounts);
		int score = 10;
		for (int lane = 0; lane < _laneCount; lane++) {
			switch (laneCounts[lane]) {
			case 0:
				score -= 3;
				break;
			case 1:
				score -= 1;
				break;
			default:
				if (5 < laneCounts[lane])
					score += 5 - laneCounts[lane];
				break;
			}
		}
		if (bestScore < score) {
			bestScore = score;
			for (int i = 0; i < _ruleCount; i++)
				best[i] = _tree[i];
		}
		if (maximumScore <= score)
			break;
	}
	for (int i = 0; i < _ruleCount; i++)
		_tree[i] = best[i];
}

const Common::Point32 &PuzzleSnowboard::obstaclePosition(int lane) const {
	return _puzzleLevel == 4 ? kL4ObstaclePositions[lane] : kObstaclePositions[lane];
}

void PuzzleSnowboard::init() {
	PuzzleBase::init();
	startPageMusic(Common::Path(kMusicPath));
	_laneCount = _puzzleLevel == 4 ? kL4LaneCount : kLaneCount;
	_obstacleVisible.resize(_laneCount);
	_obstacleHit.resize(_laneCount);
	_obstacleHiding.resize(_laneCount);
	_obstacleHitAnimating.resize(_laneCount);
	_obstacleRevealStart.resize(_laneCount);
	_obstacleHideStart.resize(_laneCount);
	_obstacleHitStart.resize(_laneCount);
	for (int lane = 0; lane < _laneCount; lane++) {
		_obstacleVisible[lane] = false;
		_obstacleHit[lane] = false;
		_obstacleHiding[lane] = false;
		_obstacleHitAnimating[lane] = false;
		_obstacleRevealStart[lane] = 0;
		_obstacleHideStart[lane] = 0;
		_obstacleHitStart[lane] = 0;
	}
	_ruleCount = _puzzleLevel == 4 ? kMaxRuleCount : kRuleCount;
	_collisionQuota = _puzzleLevel == 1 ? 2 : 4;
	loadGraphics();
	loadAreaMask(Common::Path(kAreaMaskPath));
	if (SoundManager *sound = _vm->getSoundManager()) {
		_boardReadySound = sound->load(false, Common::Path(kBoardReadySoundPath), false);
		_rideSound = sound->load(false, Common::Path(kRideSoundPath), false);
		_obstacleHitSound = sound->load(false, Common::Path(kObstacleHitSoundPath), false);
		_obstacleRevealSound = sound->load(false, Common::Path(kObstacleRevealSoundPath), false);
		_obstacleHideSound = sound->load(false, Common::Path(kObstacleHideSoundPath), false);
	}
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _puzzleZoombinis[i];
		if (!zoombini)
			continue;
		zoombini->setDefaultAnimation(ZmbAnimationKind::kBasic);
		zoombini->setPosition(kStartPositions[i % 8]);
		zoombini->setInputEnabled(true);
		zoombini->setHidden(false);
		zoombini->setCanAdvanceFromPage(false);
	}
	generateTree();
	if (_vm->inJourneyMode())
		_vm->_state->registerPageVisit(_pageId, 1);
	debug(1, "Snowboard: level=%d lanes=%d nodes=%d", _puzzleLevel, _laneCount, _ruleCount);
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i])
			debug(1, "Snowboard: waiting rider=%u lane=%d", i, classifyZoombini(_puzzleZoombinis[i]) + 1);
	}
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
	if (!zoombini || !zoombini->isInputEnabled())
		return;
	_targetRouteCode = classifyZoombini(zoombini) + _ruleCount;
	const Common::Path path(Common::String::format(_puzzleLevel == 4 ? kL4PatFormat : kPatFormat, _targetRouteCode));
	if (!zoombini->loadMovementPathFromPAT(_vm, path)) {
		warning("Snowboard: missing ride path '%s'", path.toString().c_str());
		return;
	}
	_activeRunnerIndex = zoombiniIndex;
	debug(1, "Snowboard: board rider=%d route=%d", zoombiniIndex, _targetRouteCode);
	_boardReady = false;
	_onExitPath = false;
	const uint32 now = _vm->getTotalPlayTime();
	_boardFacingSector = 8;
	_boardFacingDelay = 100;
	for (int lane = 0; lane < _laneCount; lane++)
		_obstacleHit[lane] = false;
	zoombini->setInputEnabled(false);
	zoombini->startMovement(now);
	zoombini->resetAnimation();
	bool hasSelectableRider = false;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i] && _puzzleZoombinis[i]->isInputEnabled()) {
			hasSelectableRider = true;
			break;
		}
	}
	if (!hasSelectableRider) {
		_boardCoverAnimationStart = now;
		_boardCoverActive = true;
	}
	if (SoundManager *sound = _vm->getSoundManager())
		sound->playWithVolume(_rideSound, sound->getSfxVolume());
}

void PuzzleSnowboard::chooseOpenLane() {
	Common::Array<byte> laneCounts(_laneCount);
	countLanes(laneCounts);
	int available = 0;
	for (int lane = 0; lane < _laneCount; lane++)
		available += laneCounts[lane];
	if (!available) {
		_obstacleChangePending = false;
		return;
	}
	int openLane;
	do {
		openLane = _vm->_rnd->getRandomNumber(_laneCount - 1);
	} while (laneCounts[openLane] == 0);
	const uint32 now = _vm->getTotalPlayTime();
	for (int lane = 0; lane < _laneCount; lane++) {
		const bool visible = lane != openLane;
		if (visible == _obstacleVisible[lane])
			continue;
		_obstacleVisible[lane] = visible;
		if (visible) {
			_obstacleHiding[lane] = false;
			_obstacleRevealStart[lane] = now;
			if (SoundManager *sound = _vm->getSoundManager())
				sound->playWithVolume(_obstacleRevealSound, sound->getSfxVolume());
		} else {
			_obstacleHiding[lane] = true;
			_obstacleHideStart[lane] = now;
			if (SoundManager *sound = _vm->getSoundManager())
				sound->playWithVolume(_obstacleHideSound, sound->getSfxVolume());
		}
	}
	_obstacleChangePending = false;
	debug(1, "Snowboard: open lane=%d", openLane + 1);
}

void PuzzleSnowboard::checkObstacleCollision(ZoombiniRunner *zoombini) {
	if (_collisionSpeechPending)
		return;
	for (int lane = 0; lane < _laneCount; lane++) {
		if (!_obstacleVisible[lane] || _obstacleHit[lane])
			continue;
		const Common::Point32 &position = obstaclePosition(lane);
		if (position.x < zoombini->getScreenPosition().x && zoombini->getScreenPosition().x < position.x + 50 &&
			position.y < zoombini->getScreenPosition().y && zoombini->getScreenPosition().y < position.y + 50) {
			_obstacleHit[lane] = true;
			_obstacleHitAnimating[lane] = true;
			_obstacleHitStart[lane] = _vm->getTotalPlayTime();
			if (SoundManager *sound = _vm->getSoundManager())
				sound->playWithVolume(_obstacleHitSound, sound->getSfxVolume());
			const int variant = _vm->_rnd->getRandomNumber(2) + 1;
			Common::String speechPath;
			if (_collisionCount == _collisionQuota)
				speechPath = Common::String::format(kQuotaSpeechFormat, variant);
			else if (_collisionCount == _collisionQuota - 1)
				speechPath = Common::String::format(kNearQuotaSpeechFormat, variant);
			else {
				// The restored eight lanes share the four surviving collision voice families in adjacent pairs.
				const int speechLane = _puzzleLevel == 4 ? lane / 2 : lane;
				speechPath = Common::String::format(kCollisionSpeechFormat, speechLane + 1, variant);
			}
			enqueueSpeech(speechPath, true);
			_collisionCount += 1;
			debug(1, "Snowboard: collision lane=%d count=%d", lane + 1, _collisionCount);
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
	SoundManager *sound = _vm->getSoundManager();
	if (!sound)
		return;
	sound->queueSpeech(Common::Path(path));
	if (collision)
		_collisionSpeechPending = true;
}

bool PuzzleSnowboard::hasActiveHitAnimation(uint32 now) const {
	const Animation *animation = _obstacleAnims[kHitAnimIndex];
	if (!animation)
		return false;
	const uint32 duration = animation->getFrameCount() * 50;
	for (int lane = 0; lane < _laneCount; lane++) {
		if (_obstacleHitAnimating[lane] && now - _obstacleHitStart[lane] < duration)
			return true;
	}
	return false;
}

void PuzzleSnowboard::startExitPath(ZoombiniRunner *zoombini) {
	const uint32 now = _vm->getTotalPlayTime();
	if (SoundManager *sound = _vm->getSoundManager()) {
		sound->stop(_rideSound);
		sound->playWithVolume(_boardReadySound, sound->getSfxVolume());
	}
	_generatorAnimationStart = now;
	_generatorActive = true;
	const PathPoint *points = _puzzleLevel == 4 ? kL4ExitPoints : kExitPoints;
	const int pointCount = _puzzleLevel == 4 ? kL4EndpointCount : kEndpointCount;
	for (int i = 0; i < pointCount; i++) {
		if (_usedExitPoint[i] || points[i].routeCode != _targetRouteCode)
			continue;
		_usedExitPoint[i] = true;
		const Common::Point32 start = zoombini->getScreenPosition();
		const Common::Point32 end(points[i].x, points[i].y);
		const int dx = end.x - start.x;
		const int dy = end.y - start.y;
		PathObject *path = new PathObject(_vm);
		path->appendSegment(start, Common::Point32(start.x + dx / 3, start.y + dy / 3),
							Common::Point32(end.x - dx / 3, end.y - dy / 3), end, 10, 0);
		zoombini->startMovement(path, now);
		zoombini->startDirectionTrackedAnimation(now);
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
	zoombini->setCanAdvanceFromPage(true);
	zoombini->setInputEnabled(false);
	_activeRunnerIndex = -1;
	_onExitPath = false;
	_ridesCompleted += 1;
	int remaining = 0;
	int successful = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (!_puzzleZoombinis[i])
			continue;
		if (!_puzzleZoombinis[i]->canAdvanceFromPage())
			remaining += 1;
		else if (_puzzleZoombinis[i]->canAdvanceFromPage())
			successful += 1;
	}
	if (remaining == 0 || _collisionQuota < _collisionCount) {
		_finished = true;
		_boardCoverActive = false;
		_obstacleChangePending = false;
		_finishAnimationStart = _vm->getTotalPlayTime();
		enqueueSpeech(successful == 8 ? kSuccessSpeechPath : kFailureSpeechPath, false);
		_vm->restartGoBlink();
	} else if (2 <= _ridesCompleted) {
		_obstacleChangePending = true;
	}
	debug(1, "Snowboard: completed=%d successful=%d remaining=%d collisions=%d", _ridesCompleted, successful, remaining, _collisionCount);
}

void PuzzleSnowboard::onUpdate() {
	const uint32 now = _vm->getTotalPlayTime();
	SoundManager *sound = _vm->getSoundManager();
	if (_collisionSpeechPending && (!sound || !sound->hasPendingSpeech()))
		_collisionSpeechPending = false;
	if (_goTransitionPending && (!sound || !sound->hasPendingSpeech())) {
		_goTransitionPending = false;
		_vm->_mapTransitSrcPageId = PageId::kPageSnowboard_10;
		_vm->requestPageChange(PageId::kPageMapTrans_22);
	}
	if (_boardCoverActive && _boardAnim && static_cast<uint32>(_boardAnim->getFrameCount() * 100) <= now - _boardCoverAnimationStart)
		_boardCoverActive = false;
	if (_generatorActive && 1000 <= now - _generatorAnimationStart) {
		_generatorActive = false;
		_boardReady = true;
	}
	if (_obstacleChangePending && !_finished && !_collisionSpeechPending && !hasActiveHitAnimation(now))
		chooseOpenLane();
	if (_finished) {
		for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
			ZoombiniRunner *zoombini = _puzzleZoombinis[i];
			if (zoombini)
				zoombini->tryStartCelebrationAnimation(ZmbAnimationKind::kCelebration, *_vm->_rnd, now, _vm->getFrameDeltaMs(),
													   _vm->getLogicPacingHz());
		}
	}
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i])
			_puzzleZoombinis[i]->updateAnimation(now);
	}
	if (_activeRunnerIndex == -1 || _puzzleZoombinis.size() <= static_cast<uint>(_activeRunnerIndex)) {
		if (!_dropTargets.empty())
			_dropTargets[0].occupied = false;
		return;
	}
	ZoombiniRunner *zoombini = _puzzleZoombinis[_activeRunnerIndex];
	if (!zoombini || !zoombini->hasMovementPath())
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
	if (_puzzleLevel == 4)
		drawL4Trail(screen);
}

void PuzzleSnowboard::drawTraitHint(ManagedSurface32 *screen, const TreeNode &node, bool alternate, const Common::Point32 &pos) const {
	const int trait = static_cast<int>(_puzzleLevel == 4 && alternate ? node.alternateTraitIndex : node.traitIndex);
	const byte value = alternate ? node.alternateValue : node.primaryValue;
	if (trait < 0 || ZmbTrait::kTraitKindCount <= trait || value < 1 || ZmbTrait::kTraitValueCount < value)
		return;
	const Common::String key = Common::String::format(kTraitFormat, trait + 1, value);
	if (trait == static_cast<int>(ZmbTrait::TraitKind::kNose01))
		_vm->_gfx->drawPageNoseTraitSprite(screen, key, pos, value);
	else
		_vm->_gfx->drawPageRleBlock(screen, key, pos);
}

void PuzzleSnowboard::updateBoardFacing(ZoombiniRunner *zoombini) {
	if (_boardFacingDelay <= 2) {
		_boardFacingDelay += 1;
		return;
	}
	const Common::Point32 previousScreenPosition = zoombini->getPreviousScreenPosition();
	const int deltaX = previousScreenPosition.x - zoombini->getScreenPosition().x;
	const int deltaY = previousScreenPosition.y - zoombini->getScreenPosition().y;
	const double distance = sqrt(static_cast<double>(deltaX) * deltaX + static_cast<double>(deltaY) * deltaY);
	if (distance == 0.0)
		return;
	int direction = static_cast<int>(acos(CLIP(static_cast<double>(deltaX) / distance, -1.0, 1.0)) * 180.0 * 0.31831926) / 20;
	if (zoombini->getScreenPosition().y < previousScreenPosition.y)
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
	zoombini->setAnimationCell(kFacingCells[_boardFacingSector]);
	_boardFacingDelay = 0;
}

void PuzzleSnowboard::drawObstacle(ManagedSurface32 *screen, int lane, uint32 now) const {
	const Common::Point32 &position = obstaclePosition(lane);
	if (_finished) {
		const Animation *finish = _obstacleAnims[kFinishAnimIndex];
		const Animation *hide = _obstacleAnims[kHideAnimIndex];
		const uint32 elapsed = now - _finishAnimationStart;
		const uint32 finishDuration = finish ? static_cast<uint32>(finish->getFrameCount()) * 50 : 0;
		if (finish && elapsed < finishDuration) {
			_vm->_gfx->drawAnimationFrame(screen, finish, elapsed / 50, position);
		} else if (hide && elapsed - finishDuration < static_cast<uint32>(hide->getFrameCount()) * 50) {
			_vm->_gfx->drawAnimationFrame(screen, hide, (elapsed - finishDuration) / 50, position);
		}
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
	const uint32 now = _vm->getTotalPlayTime();
	if (_generatorActive && _engineAnim && 0 < _engineAnim->getFrameCount()) {
		const int frame = ((now - _generatorAnimationStart) / 100) % 5 % _engineAnim->getFrameCount();
		_vm->_gfx->drawAnimationFrame(screen, _engineAnim, frame, Common::Point32(153, 0));
	}
	if (_boardReady)
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSurfFormat, kSurfIds[0]), Common::Point32(300, 1));
	switch (_puzzleLevel) {
	case 4:
		if (_finished)
			drawL4Legend(screen);
		break;
	case 3:
		drawTraitHint(screen, _tree[0], false, kHardHints[0]);
		drawTraitHint(screen, _tree[0], true, kHardHints[1]);
		for (int branch = 0; branch < 2; branch++) {
			const Common::Point32 offset(branch * 70, 0);
			drawTraitHint(screen, _tree[1], false, kHardHints[2] + offset);
			drawTraitHint(screen, _tree[1], true, kHardHints[3] + offset);
		}
		break;
	default:
		drawTraitHint(screen, _tree[0], false, kEasyHints[0]);
		drawTraitHint(screen, _tree[1], false, kEasyHints[1]);
		drawTraitHint(screen, _tree[1], false, kEasyHints[2]);
		break;
	}
	if (_puzzleLevel != 1 && !_finished)
		_vm->_gfx->drawPageRleBlock(screen, kBoardPath, Common::Point32(580, 58));
	if (_boardCoverActive && _puzzleLevel != 1 && !_finished && _boardAnim && 0 < _boardAnim->getFrameCount()) {
		const int frame = (now - _boardCoverAnimationStart) / 100;
		if (frame < _boardAnim->getFrameCount())
			_vm->_gfx->drawAnimationFrame(screen, _boardAnim, frame, Common::Point32(580, 58));
	}
	for (int lane = 0; lane < _laneCount; lane++) {
		if (_finished && !_obstacleVisible[lane])
			continue;
		if (!_obstacleVisible[lane] && !_obstacleHiding[lane])
			continue;
		drawObstacle(screen, lane, now);
	}
}

void PuzzleSnowboard::onRenderActors(ManagedSurface32 *screen) {
	if (_activeRunnerIndex != -1 && static_cast<uint>(_activeRunnerIndex) < _puzzleZoombinis.size() && !_onExitPath) {
		const ZoombiniRunner *zoombini = _puzzleZoombinis[_activeRunnerIndex];
		if (zoombini) {
			_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSurfFormat, kSurfIds[8 - _boardFacingSector]),
										Common::Point32(zoombini->getScreenPosition().x - 5, zoombini->getScreenPosition().y + 10));
		}
	}
	renderZoombinis(screen);
}

bool PuzzleSnowboard::canUseGoButton() const {
	return _vm->_zoombiniWalkingFlag;
}

bool PuzzleSnowboard::onGoButtonPressed() {
	if (_vm->inPracticeMode())
		return true;
	if (_goTransitionPending)
		return false;
	int freeCount = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i] && !_puzzleZoombinis[i]->canAdvanceFromPage())
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
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i])
			_puzzleZoombinis[i]->advanceAnimationAfterDraw();
	}
}

EventHandleResult PuzzleSnowboard::onLButtonUp(const Common::Point &pos) {
	ZmbHoldDropInput input;
	if (!_finished && _activeRunnerIndex == -1)
		input = ZmbHoldDropInput::kLeftButtonRelease;
	else
		input = ZmbHoldDropInput::kMoveOnly;
	const ZmbDropResult result = ZoombiniRunner::handleHoldAndDrop(_vm, _puzzleZoombinis, Common::Point32(pos.x, pos.y),
																   input, ZmbAnimationKind::kPickup, &_dropTargets, getAreaMask());
	switch (result) {
	case ZmbDropResult::kIgnored00:
		return EventHandleResult::kPassthrough;
	default:
		return EventHandleResult::kConsumed;
	}
}

EventHandleResult PuzzleSnowboard::onMouseMove(const Common::Point &pos) {
	const ZmbDropResult result = ZoombiniRunner::handleHoldAndDrop(_vm, _puzzleZoombinis, Common::Point32(pos.x, pos.y), ZmbHoldDropInput::kMoveOnly,
																   ZmbAnimationKind::kPickup, &_dropTargets, getAreaMask());
	switch (result) {
	case ZmbDropResult::kIgnored00:
		return EventHandleResult::kPassthrough;
	default:
		return EventHandleResult::kConsumed;
	}
}

Common::String PuzzleSnowboard::debugGetAnswer() const {
	static constexpr const char *kForkNames[] = {
		"First",
		"Second",
		"Third",
	};
	Common::String answer = debugAnswerHeader();
	answer += "\n  [*] Fork rules (top to bottom):\n";
	for (int depth = 0; (1 << depth) - 1 < _ruleCount; depth++) {
		const TreeNode &rule = _tree[(1 << depth) - 1];
		answer += Common::String::format("    %-6s fork: LEFT if any trait matches:\n", kForkNames[depth]);
		answer += Common::String::format("      - %s: %s\n", ZmbTrait::debugTraitName(rule.traitIndex),
										 ZmbTrait::debugTraitValueName(rule.traitIndex, rule.primaryValue));
		if (3 <= _puzzleLevel) {
			const ZmbTrait::TraitKind alternate = _puzzleLevel == 4 ? rule.alternateTraitIndex : rule.traitIndex;
			answer += Common::String::format("      - %s: %s\n", ZmbTrait::debugTraitName(alternate),
											 ZmbTrait::debugTraitValueName(alternate, rule.alternateValue));
		}
	}
	return answer;
}

PuzzleChanceInfo PuzzleSnowboard::debugGetChances() const {
	return PuzzleChanceInfo(PuzzleChanceInfo::Type::kMistake, _collisionQuota + 1, _collisionCount, "obstacle contact");
}

bool PuzzleSnowboard::debugCanSetChances() const {
	if (_debugFinishPending || _finished || _goTransitionPending || _collisionQuota < _collisionCount)
		return false;
	for (const ZoombiniRunner *zoombini : _puzzleZoombinis) {
		if (zoombini && !zoombini->canAdvanceFromPage())
			return true;
	}
	return false;
}

bool PuzzleSnowboard::debugSetChances(int remaining) {
	if (!debugCanSetChances() || remaining < 1 || _collisionQuota + 1 < remaining)
		return false;
	_collisionCount = _collisionQuota + 1 - remaining;
	return true;
}

} // End of namespace Zoombini2
