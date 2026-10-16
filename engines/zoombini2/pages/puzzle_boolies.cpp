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

#include "common/textconsole.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_boolies.h"
#include "zoombini2/random.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleBoolies::kPageName;
constexpr const char *PuzzleBoolies::kBackgroundPath;
constexpr const char *PuzzleBoolies::kMusicPath;
constexpr const char *PuzzleBoolies::kFixePath;
constexpr const char *PuzzleBoolies::kFixe2Path;
constexpr const char *PuzzleBoolies::kBallPosPath;
constexpr const char *PuzzleBoolies::kBallNegPath;
constexpr const char *PuzzleBoolies::kPinPath;
constexpr const char *PuzzleBoolies::kPinLightedPath;
constexpr const char *PuzzleBoolies::kBoatPath;
constexpr const char *PuzzleBoolies::kSpotFormat;
constexpr const char *PuzzleBoolies::kBlockerPath;
constexpr const char *PuzzleBoolies::kRollPath;
constexpr const char *PuzzleBoolies::kRoll2Path;
constexpr const char *PuzzleBoolies::kBoolieWalkOnePath;
constexpr const char *PuzzleBoolies::kBoolieWalkTwoPath;
constexpr const char *PuzzleBoolies::kPreviewEntryPath;
constexpr const char *PuzzleBoolies::kFeederPathFormat;
constexpr const char *PuzzleBoolies::kLanePathFormat;
constexpr const char *PuzzleBoolies::kBallExitPathFormat;
constexpr const char *PuzzleBoolies::kJumpPathFormat;
constexpr const char *PuzzleBoolies::kEffectPathFormat;
constexpr const char *PuzzleBoolies::kBoardVoicePathFormat;
constexpr const char *PuzzleBoolies::kCompleteSpeechPath;
constexpr const char *PuzzleBoolies::kRetreatSpeechPath;
constexpr int PuzzleBoolies::kRefillStartX[kSlotCount];
constexpr int PuzzleBoolies::kRefillY[kRowCount];
constexpr int PuzzleBoolies::kPreviewHoldX[kMaxChallengeBalls];
constexpr int PuzzleBoolies::kPreviewHoldY[kMaxChallengeBalls];
constexpr int PuzzleBoolies::kReadyHoldX[kMaxChallengeBalls];
constexpr int PuzzleBoolies::kReadyHoldY[kMaxChallengeBalls];

PuzzleBoolies::PuzzleBoolies(Zoombini2Engine *vm)
	: PuzzleBase(vm, PageId::kPageBoolies_11) {
	_initialBackgroundPath = kBackgroundPath;
}

PuzzleBoolies::~PuzzleBoolies() {
	clearBalls();
	clearJumps();
	delete _blockerAnimation;
	for (int i = 0; i < ARRAYSIZE(_rollAnimations); i++) {
		delete _rollAnimations[i];
		delete _walkAnimations[i];
	}
	SoundManager *soundManager = _vm->getSoundManager();
	if (soundManager) {
		for (int i = 0; i < ARRAYSIZE(_effectIds); i++) {
			if (0 < _effectIds[i])
				soundManager->unload(_effectIds[i]);
		}
		for (int i = 0; i < ARRAYSIZE(_boardVoiceIds); i++) {
			if (0 < _boardVoiceIds[i])
				soundManager->unload(_boardVoiceIds[i]);
		}
	}
	finishPuzzleRoster(_vm->_state->_rescue2Storage);
}

int PuzzleBoolies::getRescuedBooliesPerZoombini(int level) {
	switch (level) {
	case 1:
		return 2;
	case 2:
		return 3;
	case 3:
	case 4:
		return 4;
	default:
		error("PuzzleBoolies::getResucedBooliesPerZoombini: invalid level(%d)", level);
		return 0;
	}
}

void PuzzleBoolies::loadResources() {
	_vm->_gfx->loadPageRleBlock(kFixePath);
	_vm->_gfx->loadPageRleBlock(kFixe2Path);
	_vm->_gfx->loadPageRleBlock(kBallPosPath);
	_vm->_gfx->loadPageRleBlock(kBallNegPath);
	_vm->_gfx->loadPageRleBlock(kPinPath);
	_vm->_gfx->loadPageRleBlock(kPinLightedPath);
	_vm->_gfx->loadPageRleBlock(kBoatPath);
	for (int i = 0; i < 5; i++)
		_vm->_gfx->loadPageRleBlock(Common::String::format(kSpotFormat, i + 1));
	_vm->_gfx->loadPageRleBlock(kBlockerPath);
	_blockerAnimation = new Animation(_vm);
	if (!_blockerAnimation->loadFromFile(Common::Path(kBlockerPath))) {
		delete _blockerAnimation;
		_blockerAnimation = nullptr;
	}
	static constexpr const char *kRollPaths[2] = {
		kRollPath,
		kRoll2Path,
	};
	static constexpr const char *kWalkPaths[2] = {
		kBoolieWalkOnePath,
		kBoolieWalkTwoPath,
	};
	for (int i = 0; i < ARRAYSIZE(_rollAnimations); i++) {
		_rollAnimations[i] = new Animation(_vm);
		if (!_rollAnimations[i]->loadFromFile(Common::Path(kRollPaths[i]))) {
			delete _rollAnimations[i];
			_rollAnimations[i] = nullptr;
		}
		_walkAnimations[i] = new Animation(_vm);
		if (!_walkAnimations[i]->loadFromFile(Common::Path(kWalkPaths[i]))) {
			delete _walkAnimations[i];
			_walkAnimations[i] = nullptr;
		}
	}
	SoundManager *soundManager = _vm->getSoundManager();
	if (soundManager) {
		for (int i = 0; i < ARRAYSIZE(_effectIds); i++) {
			const Common::Path path(Common::String::format(kEffectPathFormat, i + 1));
			_effectIds[i] = soundManager->load(false, path, false);
		}
		for (int i = 0; i < ARRAYSIZE(_boardVoiceIds); i++) {
			const Common::Path path(Common::String::format(kBoardVoicePathFormat, i + 1));
			_boardVoiceIds[i] = soundManager->load(false, path, false);
		}
	}
}

Common::Point32 PuzzleBoolies::getBooliePosition(int row, int slot) {
	static constexpr int kRowY[kRowCount] = {
		92,
		206,
		307,
	};
	return Common::Point32(130 - 32 * slot, kRowY[row]);
}

Common::Point32 PuzzleBoolies::getPreviewHoldPosition(int index) {
	return Common::Point32(kPreviewHoldX[index] + 17, kPreviewHoldY[index] + 35);
}

Common::Point32 PuzzleBoolies::getReadyHoldPosition(int index) {
	return Common::Point32(kReadyHoldX[index] + 17, kReadyHoldY[index] + 35);
}

int PuzzleBoolies::getRowForPoint(const Common::Point &pos) {
	if (pos.x <= 0 || 210 <= pos.x)
		return -1;
	if (94 < pos.y && pos.y < 184)
		return 0;
	if (218 < pos.y && pos.y < 308)
		return 1;
	if (330 < pos.y && pos.y < 420)
		return 2;
	return -1;
}

void PuzzleBoolies::sampleRowValues(byte (&values)[kSlotCount]) {
	for (int slot = 0; slot < kSlotCount; slot++)
		values[slot] = 0;
	int traitCount = 4;
	switch (_puzzleLevel) {
	case 1:
		traitCount = 2;
		break;
	case 2:
		traitCount = 3;
		break;
	default:
		break;
	}
	if (_puzzleLevel == 1) {
		const int value = _vm->_rnd->getRandomNumber(2);
		values[0] = (value & 1) == 0 ? 2 : 1;
		values[1] = value == 2 ? 1 : 2;
	} else {
		bool allOne = true;
		for (int slot = 0; slot < traitCount - 1; slot++) {
			values[slot] = static_cast<byte>(_vm->_rnd->getRandomNumber(1) + 1);
			allOne = allOne && values[slot] == 1;
		}
		values[traitCount - 1] = allOne ? 2 : static_cast<byte>(_vm->_rnd->getRandomNumber(1) + 1);
	}
}

void PuzzleBoolies::generateRow(int row) {
	byte values[kSlotCount];
	sampleRowValues(values);
	for (int slot = 0; slot < kSlotCount; slot++) {
		_boolies[row][slot].value = values[slot];
		_boolies[row][slot].visibleValue = values[slot];
		_boolies[row][slot].jumping = false;
		_boolies[row][slot].removed = values[slot] == 0;
	}
}

int PuzzleBoolies::pickChallengeType() {
	switch (_puzzleLevel) {
	case 1: {
		const int value = _vm->_rnd->getRandomNumber(29);
		if (value < 10)
			return 1;
		if (value < 20)
			return 2;
		return 3;
	}
	case 2:
		return _vm->_rnd->getRandomNumber(3) + 1;
	case 3:
		return _vm->_rnd->getRandomNumber(4) + 1;
	case 4: {
		int value;
		do {
			value = _vm->_rnd->getRandomNumber(8) - 2;
		} while (value == 0);
		return value;
	}
	default:
		return 0;
	}
}

void PuzzleBoolies::clearBallArray(Common::Array<Ball> &balls) {
	for (uint i = 0; i < balls.size(); i++)
		delete balls[i].path;
	balls.clear();
}

void PuzzleBoolies::clearBalls() {
	clearBallArray(_balls);
	clearBallArray(_nextBalls);
	clearBallArray(_followingBalls);
}

void PuzzleBoolies::clearJumps() {
	for (uint i = 0; i < _jumps.size(); i++)
		delete _jumps[i].path;
	_jumps.clear();
}

void PuzzleBoolies::beginRound(uint32 now) {
	clearBalls();
	clearJumps();
	_flips.clear();
	_boardingCheckPending = false;
	_boardingCheckScheduled = false;
	_selectedRow = -1;
	_boardingRow = -1;
	_nextLaneBallIndex = 0;
	_nextPrepared = false;
	_followingPrepared = false;
	_nextFeedingRequested = false;
	_nextFeeding = false;
	_nextChallengeType = 0;
	_readyTurns = 0;
	_currentChallengeReady = false;
	_nextChallengeReady = false;
	_currentChallengeResolved = false;
	_followingChallengeType = 0;
	_challengeType = pickChallengeType();
	const int count = _challengeType < 0 ? -_challengeType : _challengeType;
	for (int i = 0; i < count && i < kMaxChallengeBalls; i++) {
		Ball ball;
		ball.index = i;
		ball.startAt = now + kBallLaunchInterval * i;
		ball.path = _vm->loadPagePATPath(Common::Path(Common::String::format(kFeederPathFormat, i + 1)));
		ball.pos = getPreviewHoldPosition(i);
		if (!ball.path || ball.path->segments.empty()) {
			ball.pos = getReadyHoldPosition(i);
			ball.stage = BallStage::kHeld02;
		}
		_balls.push_back(ball);
	}
	_phase = Phase::kFeeding00;
}

void PuzzleBoolies::prepareNextChallenge(uint32 now) {
	if (_nextPrepared)
		return;
	stagePreviewChallenge(_nextBalls, _nextChallengeType, now);
	_nextPrepared = true;
}

void PuzzleBoolies::prepareFollowingChallenge(uint32 now) {
	if (_followingPrepared)
		return;
	stagePreviewChallenge(_followingBalls, _followingChallengeType, now);
	_followingPrepared = true;
}

void PuzzleBoolies::stagePreviewChallenge(Common::Array<Ball> &balls, int &challengeType, uint32 now) {
	challengeType = pickChallengeType();
	const int count = challengeType < 0 ? -challengeType : challengeType;
	for (int i = 0; i < count && i < kMaxChallengeBalls; i++) {
		Ball ball;
		ball.index = i;
		ball.startAt = now + kBallLaunchInterval * i;
		ball.path = _vm->loadPagePATPath(Common::Path(kPreviewEntryPath));
		if (ball.path && !ball.path->segments.empty()) {
			ball.pos = ball.path->segments[0]->getStartPosition();
			ball.stage = BallStage::kPreviewWaiting05;
		} else {
			ball.pos = getPreviewHoldPosition(i);
			ball.stage = BallStage::kPreviewHeld07;
		}
		balls.push_back(ball);
	}
}

void PuzzleBoolies::startNextFeeder(uint32 now) {
	if (!_nextPrepared || _nextFeeding)
		return;
	_nextFeeding = true;
	for (uint i = 0; i < _nextBalls.size(); i++) {
		Ball &ball = _nextBalls[i];
		delete ball.path;
		ball.path = _vm->loadPagePATPath(Common::Path(Common::String::format(kFeederPathFormat, ball.index + 1)));
		ball.startAt = now + kBallLaunchInterval * ball.index;
		ball.pos = getPreviewHoldPosition(ball.index);
		if (ball.path && !ball.path->segments.empty()) {
			ball.stage = BallStage::kWaiting00;
		} else {
			ball.pos = getReadyHoldPosition(ball.index);
			ball.stage = BallStage::kHeld02;
		}
	}
}

void PuzzleBoolies::promoteNextChallenge(uint32 now) {
	if (!_nextPrepared) {
		prepareNextChallenge(now);
		return;
	}
	if (!_nextFeeding)
		return;
	clearBallArray(_balls);
	for (uint i = 0; i < _nextBalls.size(); i++)
		_balls.push_back(_nextBalls[i]);
	_nextBalls.clear();
	_challengeType = _nextChallengeType;
	_currentChallengeReady = _nextChallengeReady;
	_nextChallengeReady = false;
	_currentChallengeResolved = false;
	_nextChallengeType = _followingChallengeType;
	_nextPrepared = _followingPrepared;
	for (uint i = 0; i < _followingBalls.size(); i++)
		_nextBalls.push_back(_followingBalls[i]);
	_followingBalls.clear();
	_followingChallengeType = 0;
	_followingPrepared = false;
	_nextFeedingRequested = false;
	_nextFeeding = false;
	_selectedRow = -1;
	_nextLaneBallIndex = 0;
	_phase = Phase::kFeeding00;
}

void PuzzleBoolies::playEffect(int soundId) const {
	if (soundId <= 0)
		return;
	SoundManager *soundManager = _vm->getSoundManager();
	if (soundManager)
		soundManager->playWithVolume(soundId, soundManager->getSfxVolume());
}

void PuzzleBoolies::playBoardVoice() {
	const int voiceIndex = _vm->_rnd->getRandomNumber(3);
	playEffect(_boardVoiceIds[voiceIndex]);
}

void PuzzleBoolies::init() {
	PuzzleBase::init();
	startPageMusic(Common::Path(kMusicPath));
	loadResources();
	_blockerStart = _vm->getTotalPlayTime();
	const int rosterCount = static_cast<int>(_puzzleZoombinis.size());
	for (int i = 0; i < rosterCount; i++) {
		ZoombiniRunner *zoombini = _puzzleZoombinis[i];
		if (!zoombini)
			continue;
		zoombini->setDefaultAnimation(ZmbAnimationKind::kBasic);
		zoombini->setRescuedBooliesPerZoombini(getRescuedBooliesPerZoombini(_puzzleLevel));
		zoombini->setCanAdvanceFromPage(false);
		zoombini->setInputEnabled(false);
		zoombini->setHidden(i != 0);
		Common::Point32 position(120, 484);
		if (i == 0)
			position = Common::Point32(115, 495);
		zoombini->setPosition(position);
	}
	for (int row = 0; row < kRowCount; row++)
		generateRow(row);
	byte entryValues[kSlotCount];
	for (int slot = 0; slot < kSlotCount; slot++)
		entryValues[slot] = _boolies[kRowCount - 1][slot].value;
	startRefill(kRowCount - 1, entryValues, _vm->getTotalPlayTime());
	if (_vm->inJourneyMode())
		_vm->_state->registerPageVisit(_pageId, 1);
	switch (_puzzleLevel) {
	case 1:
		_requiredTurns = (rosterCount + 3) / 4 + 2 * rosterCount + 1;
		break;
	case 2:
		_requiredTurns = (rosterCount + 1) / 2 + 3 * rosterCount + 1;
		break;
	case 3:
		_requiredTurns = 4 * rosterCount + 1;
		break;
	case 4:
		_requiredTurns = (rosterCount + 1) / 2 + 3 * rosterCount + 4;
		break;
	}
	_boatX = 80;
	if (rosterCount == 0) {
		_phase = Phase::kFinished05;
		_vm->restartGoBlink();
		return;
	}
	beginRound(_vm->getTotalPlayTime());
}

void PuzzleBoolies::startLaneBall(Ball &ball, uint32 now) {
	if (_selectedRow < 0)
		return;
	delete ball.path;
	ball.path = _vm->loadPagePATPath(Common::Path(Common::String::format(kLanePathFormat, _selectedRow + 2)));
	if (!ball.path || ball.path->segments.empty()) {
		ball.stage = BallStage::kDone04;
		return;
	}
	ball.path->start(now);
	ball.pos = ball.path->segments[0]->getStartPosition();
	ball.stage = BallStage::kLane03;
	const int effectIndex = 2 * _selectedRow + (_challengeType < 0 ? 1 : 0);
	playEffect(_effectIds[effectIndex]);
}

void PuzzleBoolies::advanceBall(Ball &ball, uint32 now) {
	if (ball.stage == BallStage::kWaiting00 && ball.startAt <= now) {
		if (!ball.path) {
			ball.pos = getReadyHoldPosition(ball.index);
			ball.stage = BallStage::kHeld02;
			return;
		}
		ball.path->start(now);
		ball.stage = BallStage::kFeeder01;
	}
	switch (ball.stage) {
	case BallStage::kFeeder01:
		if (!ball.path->advance(now, ball.pos)) {
			ball.pos = getReadyHoldPosition(ball.index);
			ball.stage = BallStage::kHeld02;
		}
		break;
	case BallStage::kLane03: {
		const bool moving = ball.path->advance(now, ball.pos);
		if (!moving || ball.pos.x - 17 <= 180) {
			resolveBall(ball, now);
			delete ball.path;
			ball.path = _vm->loadPagePATPath(Common::Path(Common::String::format(kBallExitPathFormat, _selectedRow + 2)));
			if (ball.path) {
				ball.path->start(now);
				ball.stage = BallStage::kExit08;
			} else {
				ball.stage = BallStage::kDone04;
			}
		}
		break;
	}
	case BallStage::kExit08:
		// The finite exit path keeps requesting the impact sample as voices become available.
		playEffect(_effectIds[6]);
		if (!ball.path->advance(now, ball.pos) || 810 < ball.pos.x - 17)
			ball.stage = BallStage::kDone04;
		break;
	default:
		break;
	}
}

void PuzzleBoolies::advanceNextBalls(uint32 now) {
	advancePreviewBalls(_nextBalls, now);
	advancePreviewBalls(_followingBalls, now);
	if (_nextFeeding) {
		for (uint i = 0; i < _nextBalls.size(); i++)
			advanceBall(_nextBalls[i], now);
		bool allHeld = !_nextBalls.empty();
		for (uint i = 0; i < _nextBalls.size(); i++)
			allHeld = allHeld && _nextBalls[i].stage == BallStage::kHeld02;
		if (allHeld && !_nextChallengeReady) {
			_nextChallengeReady = true;
			_readyTurns += 1;
		}
		if (allHeld && !_followingPrepared && _readyTurns < _requiredTurns)
			prepareFollowingChallenge(now);
		return;
	}
	if (!_nextFeedingRequested)
		return;
	for (uint i = 0; i < _nextBalls.size(); i++) {
		if (_nextBalls[i].stage != BallStage::kPreviewHeld07)
			return;
	}
	startNextFeeder(now);
}

void PuzzleBoolies::advancePreviewBalls(Common::Array<Ball> &balls, uint32 now) {
	for (uint i = 0; i < balls.size(); i++) {
		Ball &ball = balls[i];
		if (ball.stage == BallStage::kPreviewWaiting05 && ball.startAt <= now) {
			ball.path->start(now);
			ball.stage = BallStage::kPreviewEntry06;
		}
		if (ball.stage == BallStage::kPreviewEntry06) {
			const bool walking = ball.path->advance(now, ball.pos);
			const Common::Point32 hold = getPreviewHoldPosition(ball.index);
			if (!walking || hold.x <= ball.pos.x) {
				ball.pos = hold;
				ball.stage = BallStage::kPreviewHeld07;
			}
		}
	}
}

bool PuzzleBoolies::isBlockerLowered() const {
	for (uint i = 0; i < _balls.size(); i++) {
		if (_balls[i].stage == BallStage::kWaiting00 || _balls[i].stage == BallStage::kFeeder01)
			return true;
	}
	for (uint i = 0; i < _nextBalls.size(); i++) {
		if (_nextBalls[i].stage == BallStage::kWaiting00 || _nextBalls[i].stage == BallStage::kFeeder01)
			return true;
	}
	return false;
}

void PuzzleBoolies::resolveBall(const Ball &ball, uint32 now) {
	(void)ball;
	if (_selectedRow < 0)
		return;
	for (int slot = 0; slot < kSlotCount; slot++) {
		Boolie &boolie = _boolies[_selectedRow][slot];
		if (boolie.removed || boolie.jumping)
			break;
		const byte previousValue = boolie.value;
		boolie.value = previousValue == 1 ? 2 : 1;
		Flip flip;
		flip.row = _selectedRow;
		flip.slot = slot;
		flip.fromValue = previousValue;
		flip.toValue = boolie.value;
		if (_flips.empty())
			_flipStart = now;
		_flips.push_back(flip);
		if ((_challengeType < 0 && previousValue == 1) || (0 < _challengeType && previousValue == 2))
			break;
	}
}

void PuzzleBoolies::advanceFlips(uint32 now) {
	if (_flips.empty())
		return;
	const Flip &flip = _flips[0];
	const Animation *animation = _rollAnimations[flip.fromValue - 1];
	const uint32 frameCount = animation && 0 < animation->getFrameCount() ? animation->getFrameCount() : 13;
	if (now - _flipStart < frameCount * kRollFrameTime)
		return;
	_boolies[flip.row][flip.slot].visibleValue = flip.toValue;
	_flips.remove_at(0);
	_flipStart = now;
}

void PuzzleBoolies::startRowJumps(int row, uint32 now) {
	_boardingRow = row;
	for (int replacementRow = 0; replacementRow < kRowCount; replacementRow++)
		sampleRowValues(_replacementValues[replacementRow]);
	for (int slot = 0; slot < kSlotCount; slot++) {
		Boolie &boolie = _boolies[row][slot];
		if (boolie.removed || boolie.jumping)
			continue;
		Jump jump;
		jump.row = row;
		jump.slot = slot;
		jump.value = boolie.value;
		jump.pos = getBooliePosition(row, slot);
		jump.path = _vm->loadPagePATPath(Common::Path(Common::String::format(kJumpPathFormat, row, slot + 1)));
		if (jump.path) {
			_jumps.push_back(jump);
		} else {
			boolie.removed = true;
		}
	}
	if (!_jumps.empty()) {
		startJump(_jumps[0], now);
	} else {
		startBoat(now);
	}
}

void PuzzleBoolies::startJump(Jump &jump, uint32 now) {
	playBoardVoice();
	jump.path->start(now);
	jump.pos = jump.path->segments[0]->getStartPosition();
	jump.started = true;
	_boolies[jump.row][jump.slot].jumping = true;
}

void PuzzleBoolies::advanceJumps(uint32 now) {
	if (_jumps.empty())
		return;
	Jump &jump = _jumps[0];
	if (jump.path->advance(now, jump.pos))
		return;
	_boolies[jump.row][jump.slot].jumping = false;
	_boolies[jump.row][jump.slot].removed = true;
	Passenger passenger;
	passenger.pos = jump.pos;
	passenger.value = jump.value;
	_passengers.push_back(passenger);
	delete jump.path;
	_jumps.remove_at(0);
	if (!_jumps.empty()) {
		startJump(_jumps[0], now);
	} else {
		startBoat(now);
	}
}

void PuzzleBoolies::startRefill(int row, const byte (&values)[kSlotCount], uint32 now) {
	for (int slot = 0; slot < kSlotCount; slot++) {
		_refillValues[slot] = values[slot];
		Boolie &boolie = _boolies[row][slot];
		boolie.value = values[slot];
		boolie.visibleValue = values[slot];
		boolie.jumping = false;
		boolie.removed = true;
	}
	_refillRow = row;
	_refillSlot = 0;
	_refillX = kRefillStartX[0];
	_refillCycleStart = now;
	_refillActive = true;
}

void PuzzleBoolies::advanceRefill(uint32 now) {
	if (!_refillActive || now - _refillCycleStart < kWalkFrameCount * kWalkFrameTime)
		return;
	const int targetX = getBooliePosition(_refillRow, _refillSlot).x;
	if (_refillX < targetX) {
		_refillX += kWalkStepX;
		_refillCycleStart = now;
		return;
	}
	Boolie &boolie = _boolies[_refillRow][_refillSlot];
	boolie.removed = false;
	_refillSlot += 1;
	int traitCount = 4;
	switch (_puzzleLevel) {
	case 1:
		traitCount = 2;
		break;
	case 2:
		traitCount = 3;
		break;
	default:
		break;
	}
	if (traitCount <= _refillSlot) {
		_refillActive = false;
		if (_refillRow == _boardingRow)
			_boardingRow = -1;
		return;
	}
	_refillX = kRefillStartX[_refillSlot];
	_refillCycleStart = now;
}

bool PuzzleBoolies::isRowEmpty(int row) const {
	for (int slot = 0; slot < kSlotCount; slot++) {
		if (!_boolies[row][slot].removed && _boolies[row][slot].value == 2)
			return false;
	}
	return true;
}

void PuzzleBoolies::startBoat(uint32 now) {
	startRefill(_boardingRow, _replacementValues[0], now);
	if (_activeRunnerIndex < static_cast<int>(_puzzleZoombinis.size()) && _puzzleZoombinis[_activeRunnerIndex])
		_puzzleZoombinis[_activeRunnerIndex]->setCanAdvanceFromPage(true);
	_boatState = BoatState::kLeaving01;
	_boatTime = now;
	_returnBoatActive = false;
}

void PuzzleBoolies::finishRound() {
	_currentChallengeResolved = true;
	_boardingCheckPending = true;
	_nextFeedingRequested = _readyTurns < _requiredTurns;
	if (_requiredTurns <= _readyTurns) {
		_phase = Phase::kFinished05;
		if (!isRowEmpty(_selectedRow)) {
			_vm->_zoombiniWalkingFlag = true;
			_vm->restartGoBlink();
		}
		return;
	}
	_phase = Phase::kBetweenRounds02;
}

void PuzzleBoolies::advanceBoat(uint32 now) {
	if (_boatState == BoatState::kReady00)
		return;
	static constexpr double kPixelsPerMillisecond = 800.0 / 4500.0;
	const double distance = (now - _boatTime) * kPixelsPerMillisecond;
	_boatTime = now;
	const bool returning = _returnBoatActive;
	if (_boatState == BoatState::kLeaving01) {
		_boatX = static_cast<int>(_boatX + distance);
		if (_activeRunnerIndex < static_cast<int>(_puzzleZoombinis.size()) && _puzzleZoombinis[_activeRunnerIndex])
			_puzzleZoombinis[_activeRunnerIndex]->setPosition(Common::Point32(_boatX + 35, 495));
		const bool lastRunner = _activeRunnerIndex + 1 == static_cast<int>(_puzzleZoombinis.size());
		if (!lastRunner && !_returnBoatActive && 549 <= _boatX) {
			_returnBoatX = -251;
			_returnBoatActive = true;
			_puzzleZoombinis[_activeRunnerIndex + 1]->setHidden(false);
			_puzzleZoombinis[_activeRunnerIndex + 1]->setPosition(Common::Point32(_returnBoatX + 35, 495));
			_vm->_zoombiniWalkingFlag = true;
		}
		if (810 < _boatX && lastRunner) {
			_boatState = BoatState::kReady00;
			_phase = Phase::kFinished05;
			_vm->_zoombiniWalkingFlag = true;
			_vm->restartGoBlink();
			if (_vm->inJourneyMode()) {
				if (SoundManager *sound = _vm->getSoundManager())
					sound->queueSpeech(Common::Path(kCompleteSpeechPath));
			}
			return;
		}
		if (810 < _boatX) {
			_puzzleZoombinis[_activeRunnerIndex]->setHidden(true);
			_boatState = BoatState::kReturning02;
		}
	}
	if (!returning)
		return;
	_returnBoatX = static_cast<int>(_returnBoatX + distance);
	_puzzleZoombinis[_activeRunnerIndex + 1]->setPosition(Common::Point32(_returnBoatX + 35, 495));
	if (_returnBoatX < 80)
		return;
	_puzzleZoombinis[_activeRunnerIndex]->setHidden(true);
	_activeRunnerIndex += 1;
	_returnBoatActive = false;
	_passengers.clear();
	_boatX = 80;
	_boatState = BoatState::kReady00;
	_puzzleZoombinis[_activeRunnerIndex]->setPosition(Common::Point32(115, 495));
	_vm->_zoombiniWalkingFlag = true;
	if (_phase == Phase::kFinished05) {
		_vm->restartGoBlink();
	}
}

void PuzzleBoolies::advanceBoardingCheck(uint32 now) {
	if (_boardingCheckPending && _flips.empty()) {
		_boardingCheckPending = false;
		_boardingCheckScheduled = true;
		_boardingCheckAt = now + kBoardingCheckDelay;
	}
	if (!_boardingCheckScheduled || now < _boardingCheckAt || !_flips.empty())
		return;
	if (_boardingRow != -1 || !_jumps.empty() || _refillActive || _boatState != BoatState::kReady00)
		return;
	_boardingCheckScheduled = false;
	for (int row = 0; row < kRowCount; row++) {
		if (isRowEmpty(row)) {
			startRowJumps(row, now);
			return;
		}
	}
	if (_phase == Phase::kFinished05) {
		_vm->_zoombiniWalkingFlag = true;
		_vm->restartGoBlink();
	}
}

void PuzzleBoolies::onUpdate() {
	const uint32 now = _vm->getTotalPlayTime();
	if (_goPending) {
		SoundManager *sound = _vm->getSoundManager();
		if (!sound || !sound->hasPendingSpeech()) {
			_vm->_mapTransitSrcPageId = PageId::kPageBoolies_11;
			_vm->requestPageChange(PageId::kPageMapTrans_22);
		}
		return;
	}
	advanceRefill(now);
	advanceBoat(now);
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i])
			_puzzleZoombinis[i]->updateAnimation(now);
	}
	if (_nextPrepared)
		advanceNextBalls(now);
	if (_phase == Phase::kFeeding00 || _phase == Phase::kRolling01) {
		if (_phase == Phase::kRolling01 && _nextLaneBallIndex < _balls.size() && _nextLaneLaunchAt <= now) {
			startLaneBall(_balls[_nextLaneBallIndex], now);
			_nextLaneBallIndex += 1;
			_nextLaneLaunchAt = now + kBallLaunchInterval;
		}
		for (int i = static_cast<int>(_balls.size()) - 1; 0 <= i; i--)
			advanceBall(_balls[i], now);
		if (_phase == Phase::kFeeding00) {
			bool allHeld = !_balls.empty();
			for (uint i = 0; i < _balls.size(); i++)
				allHeld = allHeld && _balls[i].stage == BallStage::kHeld02;
			if (allHeld && !_currentChallengeReady) {
				_currentChallengeReady = true;
				_readyTurns += 1;
			}
			if (allHeld && !_nextPrepared && _readyTurns < _requiredTurns)
				prepareNextChallenge(now);
		} else {
			bool allDone = true;
			for (uint i = 0; i < _balls.size(); i++)
				allDone = allDone && _balls[i].stage == BallStage::kDone04;
			if (allDone) {
				finishRound();
			}
		}
	}
	advanceFlips(now);
	advanceJumps(now);
	advanceBoardingCheck(now);
	if (_phase == Phase::kBetweenRounds02 && _nextFeeding)
		promoteNextChallenge(now);
}

void PuzzleBoolies::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

void PuzzleBoolies::drawPinSections(ManagedSurface32 *screen) const {
	switch (_pinRoute) {
	case 0:
		_vm->_gfx->drawPageRleBlockClipped(screen, kPinPath, Common::Point32(435, 120), Common::Rect32(0, 0, 518, 193));
		_vm->_gfx->drawPageRleBlockClipped(screen, kPinPath, Common::Point32(518, 95), Common::Rect32(0, 0, 601, 168));
		break;
	case 1:
		_vm->_gfx->drawPageRleBlockClipped(screen, kPinPath, Common::Point32(363, 142), Common::Rect32(446, 0, 529, 215));
		_vm->_gfx->drawPageRleBlockClipped(screen, kPinPath, Common::Point32(432, 91), Common::Rect32(515, 0, 598, 164));
		break;
	default:
		_vm->_gfx->drawPageRleBlock(screen, kPinLightedPath, Common::Point32(435, 120));
		_vm->_gfx->drawPageRleBlockClipped(screen, kPinPath, Common::Point32(358, 95), Common::Rect32(524, 0, 690, 168));
		break;
	}
}

void PuzzleBoolies::drawSpotHighlights(ManagedSurface32 *screen) const {
	switch (_hoverRow) {
	case 0:
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 2), Common::Point32(250, 142));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 1), Common::Point32(322, 134));
		break;
	case 1:
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 5), Common::Point32(247, 250));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 3), Common::Point32(309, 242));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 4), Common::Point32(371, 223));
		break;
	case 2:
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 4), Common::Point32(277, 353));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 1), Common::Point32(389, 324));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 5), Common::Point32(469, 257));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 3), Common::Point32(509, 196));
		break;
	default:
		break;
	}
}

void PuzzleBoolies::onRenderContent(ManagedSurface32 *screen) {
	const uint32 now = _vm->getTotalPlayTime();
	for (int row = 0; row < kRowCount; row++) {
		for (int slot = 0; slot < kSlotCount; slot++) {
			const Boolie &boolie = _boolies[row][slot];
			if (boolie.removed || boolie.jumping)
				continue;
			const Common::Point32 pos = getBooliePosition(row, slot);
			if (!_flips.empty() && _flips[0].row == row && _flips[0].slot == slot) {
				const Animation *animation = _rollAnimations[_flips[0].fromValue - 1];
				if (animation && 0 < animation->getFrameCount()) {
					const uint32 frame = (now - _flipStart) / kRollFrameTime;
					_vm->_gfx->drawAnimationFrame(screen, animation, MIN<uint32>(frame, animation->getFrameCount() - 1), pos);
					continue;
				}
			}
			const char *path = kFixe2Path;
			if (boolie.visibleValue == 1)
				path = kFixePath;
			_vm->_gfx->drawPageRleBlock(screen, path, pos);
		}
	}
	for (uint i = 0; i < _jumps.size(); i++) {
		const Jump &jump = _jumps[i];
		if (jump.started) {
			const char *path = kFixe2Path;
			if (jump.value == 1)
				path = kFixePath;
			_vm->_gfx->drawPageRleBlock(screen, path, Common::Point32(jump.pos.x - 35, jump.pos.y - 69));
		}
	}
	if (_refillActive) {
		const byte value = _refillValues[_refillSlot];
		const Animation *animation = _walkAnimations[value - 1];
		if (animation && 0 < animation->getFrameCount()) {
			const uint32 frame = MIN<uint32>((now - _refillCycleStart) / kWalkFrameTime, MIN<uint32>(kWalkFrameCount, animation->getFrameCount()) - 1);
			_vm->_gfx->drawAnimationFrame(screen, animation, frame, Common::Point32(_refillX, kRefillY[_refillRow]));
		} else {
			const char *path = kFixe2Path;
			if (value == 1)
				path = kFixePath;
			_vm->_gfx->drawPageRleBlock(screen, path, Common::Point32(_refillX, kRefillY[_refillRow] - 15));
		}
	}
	drawPinSections(screen);
	drawSpotHighlights(screen);
	for (int i = static_cast<int>(_balls.size()) - 1; 0 <= i; i--) {
		const Ball &ball = _balls[i];
		if (ball.stage != BallStage::kDone04) {
			const char *path = kBallPosPath;
			if (_challengeType <= 0)
				path = kBallNegPath;
			_vm->_gfx->drawPageRleBlock(screen, path, Common::Point32(ball.pos.x - 17, ball.pos.y - 35));
		}
	}
	for (int i = static_cast<int>(_nextBalls.size()) - 1; 0 <= i; i--) {
		const Ball &ball = _nextBalls[i];
		if (ball.stage != BallStage::kPreviewWaiting05 && ball.stage != BallStage::kDone04) {
			const char *path = kBallPosPath;
			if (_nextChallengeType <= 0)
				path = kBallNegPath;
			_vm->_gfx->drawPageRleBlock(screen, path, Common::Point32(ball.pos.x - 17, ball.pos.y - 35));
		}
	}
	for (int i = static_cast<int>(_followingBalls.size()) - 1; 0 <= i; i--) {
		const Ball &ball = _followingBalls[i];
		if (ball.stage != BallStage::kPreviewWaiting05 && ball.stage != BallStage::kDone04) {
			const char *path = kBallPosPath;
			if (_followingChallengeType <= 0)
				path = kBallNegPath;
			_vm->_gfx->drawPageRleBlock(screen, path, Common::Point32(ball.pos.x - 17, ball.pos.y - 35));
		}
	}
	if (isBlockerLowered())
		_vm->_gfx->drawPageRleBlockClipped(screen, kBlockerPath, Common::Point32(198, 38), Common::Rect32(244, 0, 290, 67));
	else
		_vm->_gfx->drawPageRleBlockClipped(screen, kBlockerPath, Common::Point32(244, 38), Common::Rect32(0, 0, 267, 67));
	if (_blockerAnimation && 0 < _blockerAnimation->getFrameCount()) {
		const uint32 frameCount = MIN<uint32>(kBlockerFrameCount, _blockerAnimation->getFrameCount());
		const uint32 elapsed = now - _blockerStart;
		if (elapsed < frameCount * kBlockerFrameTime)
			_vm->_gfx->drawAnimationFrame(screen, _blockerAnimation, elapsed / kBlockerFrameTime, Common::Point32(244, 38));
	}
	_vm->_gfx->drawPageRleBlock(screen, kBoatPath, Common::Point32(_boatX, 410));
	if (_returnBoatActive)
		_vm->_gfx->drawPageRleBlock(screen, kBoatPath, Common::Point32(_returnBoatX, 410));
	for (uint i = 0; i < _passengers.size(); i++) {
		const Passenger &passenger = _passengers[i];
		_vm->_gfx->drawPageRleBlock(screen, passenger.value == 1 ? kFixePath : kFixe2Path,
									Common::Point32(passenger.pos.x + _boatX - 80 - 35, passenger.pos.y - 69));
	}
}

void PuzzleBoolies::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleBoolies::onActorsRendered() {
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i])
			_puzzleZoombinis[i]->advanceAnimationAfterDraw();
	}
}

EventHandleResult PuzzleBoolies::onLButtonDown(const Common::Point &pos) {
	if (_phase != Phase::kFeeding00 || _selectedRow != -1 || _requiredTurns < _readyTurns)
		return EventHandleResult::kPassthrough;
	const bool lastRunner = _activeRunnerIndex + 1 == static_cast<int>(_puzzleZoombinis.size());
	if (lastRunner && (!_jumps.empty() || _boatState != BoatState::kReady00))
		return EventHandleResult::kPassthrough;
	for (uint i = 0; i < _balls.size(); i++) {
		if (_balls[i].stage != BallStage::kHeld02)
			return EventHandleResult::kPassthrough;
	}
	const int row = getRowForPoint(pos);
	if (row < 0 || isRowEmpty(row) || (_refillActive && row == _refillRow))
		return EventHandleResult::kPassthrough;
	_selectedRow = row;
	_pinRoute = row;
	const uint32 now = _vm->getTotalPlayTime();
	_nextLaneBallIndex = 0;
	_nextLaneLaunchAt = now + (row == 2 ? 0 : kFirstBallLaunchDelay);
	_phase = Phase::kRolling01;
	return EventHandleResult::kConsumed;
}

EventHandleResult PuzzleBoolies::onMouseMove(const Common::Point &pos) {
	_hoverRow = _phase == Phase::kFeeding00 ? getRowForPoint(pos) : -1;
	return EventHandleResult::kPassthrough;
}

Common::String PuzzleBoolies::debugBinaryValue(const byte (&values)[kSlotCount], int bitCount, int &number) {
	Common::String binary;
	number = 0;
	for (int slot = bitCount - 1; 0 <= slot; slot -= 1) {
		const int bit = values[slot] == 1 ? 1 : 0;
		binary += bit == 0 ? '0' : '1';
		number = number * 2 + bit;
	}
	return binary;
}

void PuzzleBoolies::debugApplyBalls(Boolie (&values)[kSlotCount], uint ballCount, int challengeType) {
	for (uint i = 0; i < ballCount; i++) {
		for (int slot = 0; slot < kSlotCount; slot++) {
			Boolie &boolie = values[slot];
			if (boolie.removed || boolie.jumping)
				break;
			const byte previous = boolie.value;
			boolie.value = previous == 1 ? 2 : 1;
			if ((challengeType < 0 && previous == 1) || (0 < challengeType && previous == 2))
				break;
		}
	}
}

Common::String PuzzleBoolies::debugRowForecast(const Boolie (&values)[kSlotCount], uint ballCount, int challengeType, int bitCount) {
	Boolie result[kSlotCount];
	memcpy(result, values, sizeof(result));
	debugApplyBalls(result, ballCount, challengeType);
	byte currentValues[kSlotCount];
	byte resultValues[kSlotCount];
	bool cleared = true;
	for (int slot = 0; slot < kSlotCount; slot++) {
		currentValues[slot] = values[slot].value;
		resultValues[slot] = result[slot].value;
		if (!result[slot].removed)
			cleared = cleared && result[slot].value != 2;
	}
	int currentNumber = 0;
	int resultNumber = 0;
	const Common::String currentBinary = debugBinaryValue(currentValues, bitCount, currentNumber);
	const Common::String resultBinary = debugBinaryValue(resultValues, bitCount, resultNumber);
	const int decimalWidth = bitCount < 4 ? 1 : 2;
	return Common::String::format("%s (%*d) -> %s (%*d)%s\n", currentBinary.c_str(), decimalWidth, currentNumber,
								  resultBinary.c_str(), decimalWidth, resultNumber, cleared ? " (boards the boat)" : " (stays)");
}

Common::String PuzzleBoolies::debugGetAnswer() const {
	Common::String answer = debugAnswerHeader();
	answer += "\n  Practice binary addition and subtraction.\n";
	int bitCount = kSlotCount;
	switch (_puzzleLevel) {
	case 1:
		bitCount = 2;
		break;
	case 2:
		bitCount = 3;
		break;
	default:
		break;
	}
	const int goal = (1 << bitCount) - 1;
	Common::String goalBinary;
	for (int slot = 0; slot < bitCount; slot++)
		goalBinary += '1';
	answer += Common::String::format("  Goal: Reach %d (binary %s) on one of lanes.\n", goal, goalBinary.c_str());
	const int remaining = debugGetChances().chancesLeft();
	answer += Common::String::format("\n  [*] Balls (%d chances remaining):\n", remaining);
	answer += Common::String::format("  - Current: %+d\n", _challengeType);
	if (_nextPrepared)
		answer += Common::String::format("  - Next:    %+d\n", _nextChallengeType);
	else
		answer += "  - Next:    pending\n";
	if (_phase == Phase::kFinished05 || _goPending)
		return answer + "  No further row choices are available.\n";
	Boolie rows[kRowCount][kSlotCount];
	memcpy(rows, _boolies, sizeof(rows));
	const bool nextChoice = _phase != Phase::kFeeding00 || _selectedRow != -1;
	int challengeType = _challengeType;
	uint ballCount = _balls.size();
	if (nextChoice) {
		if (_phase == Phase::kRolling01 && 0 <= _selectedRow && _selectedRow < kRowCount) {
			uint pending = 0;
			for (const Ball &ball : _balls) {
				if (ball.stage != BallStage::kExit08 && ball.stage != BallStage::kDone04)
					pending += 1;
			}
			const int signedPending = _challengeType < 0 ? -static_cast<int>(pending) : static_cast<int>(pending);
			answer += Common::String::format("\n  [*] Current result (Row %d; %+d still to hit):\n    ", _selectedRow + 1, signedPending);
			answer += debugRowForecast(rows[_selectedRow], pending, _challengeType, bitCount);
			debugApplyBalls(rows[_selectedRow], pending, _challengeType);
		} else {
			answer += "  Current batch has finished.\n";
		}
		const int nextChances = remaining - (_currentChallengeResolved ? 0 : 1);
		if (nextChances <= 0)
			return answer + "  No further ball batches remain.\n";
		if (!_nextPrepared)
			return answer + "  Next balls have not been generated yet.\n";
		challengeType = _nextChallengeType;
		ballCount = challengeType < 0 ? -challengeType : challengeType;
		answer += "\n  [*] Next row values (after current balls; top to bottom):\n";
	} else {
		answer += "\n  [*] Row values (top to bottom):\n";
	}
	for (int row = 0; row < kRowCount; row++) {
		const byte *incoming = nullptr;
		if (_refillActive && row == _refillRow)
			incoming = _refillValues;
		else if (row == _boardingRow)
			incoming = _replacementValues[0];
		bool empty = true;
		for (int slot = 0; slot < kSlotCount; slot++)
			empty = empty && (rows[row][slot].removed || rows[row][slot].value != 2);
		if (incoming) {
			for (int slot = 0; slot < kSlotCount; slot++) {
				rows[row][slot].value = incoming[slot];
				rows[row][slot].removed = bitCount <= slot;
				rows[row][slot].jumping = false;
			}
		} else if (empty) {
			answer += Common::String::format("    Row %d: incoming\n", row + 1);
			continue;
		}
		answer += Common::String::format("    Row %d: %s", row + 1, incoming ? "incoming; after arrival: " : "");
		answer += debugRowForecast(rows[row], ballCount, challengeType, bitCount);
	}
	return answer;
}

bool PuzzleBoolies::canUseGoButton() const {
	return !_goPending && _vm->_zoombiniWalkingFlag;
}

bool PuzzleBoolies::onGoButtonPressed() {
	if (_vm->inPracticeMode())
		return true;
	if (_goPending)
		return false;
	int remaining = 0;
	for (const ZoombiniRunner *runner : _puzzleZoombinis) {
		if (!runner->canAdvanceFromPage())
			remaining += 1;
	}
	if (remaining < 4)
		return true;
	if (SoundManager *sound = _vm->getSoundManager())
		sound->queueSpeech(Common::Path(kRetreatSpeechPath));
	_goPending = true;
	return false;
}

PuzzleChanceInfo PuzzleBoolies::debugGetChances() const {
	int used = _readyTurns;
	if (_currentChallengeReady && !_currentChallengeResolved)
		used -= 1;
	if (_nextChallengeReady)
		used -= 1;
	return PuzzleChanceInfo(PuzzleChanceInfo::Type::kSubmit, _requiredTurns, used, "ball batch sent");
}

bool PuzzleBoolies::debugCanSetChances() const {
	if (_debugFinishPending || _goPending || _phase != Phase::kFeeding00 || _selectedRow != -1 || debugGetChances().chancesLeft() <= 0)
		return false;
	for (const ZoombiniRunner *zoombini : _puzzleZoombinis) {
		if (zoombini && !zoombini->canAdvanceFromPage())
			return true;
	}
	return false;
}

bool PuzzleBoolies::debugSetChances(int remaining) {
	if (!debugCanSetChances() || remaining < 1 || _requiredTurns < remaining)
		return false;
	_readyTurns = _requiredTurns - remaining;
	if (_currentChallengeReady && !_currentChallengeResolved)
		_readyTurns += 1;
	if (_nextChallengeReady)
		_readyTurns += 1;
	return true;
}

} // End of namespace Zoombini2
