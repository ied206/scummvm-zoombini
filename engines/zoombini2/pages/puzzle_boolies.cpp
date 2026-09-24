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

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_boolies.h"
#include "zoombini2/random.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

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

PuzzleBoolies::PuzzleBoolies(Zoombini2Engine *vm) : PuzzleBase(vm, kPageBoolies) {
}

PuzzleBoolies::~PuzzleBoolies() {
	clearBalls();
	clearJumps();
	delete _blockerAnimation;
	for (int index = 0; index < 2; index++) {
		delete _rollAnimations[index];
		delete _walkAnimations[index];
	}
	SoundManager *soundManager = _vm->getSoundManager();
	if (soundManager) {
		for (int index = 0; index < 7; index++) {
			if (0 < _effectIds[index])
				soundManager->unload(_effectIds[index]);
		}
		for (int index = 0; index < 4; index++) {
			if (0 < _boardVoiceIds[index])
				soundManager->unload(_boardVoiceIds[index]);
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
	for (int index = 0; index < 5; index++)
		_vm->_gfx->loadPageRleBlock(Common::String::format(kSpotFormat, index + 1));
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
	for (int index = 0; index < 2; index++) {
		_rollAnimations[index] = new Animation(_vm);
		if (!_rollAnimations[index]->loadFromFile(Common::Path(kRollPaths[index]))) {
			delete _rollAnimations[index];
			_rollAnimations[index] = nullptr;
		}
		_walkAnimations[index] = new Animation(_vm);
		if (!_walkAnimations[index]->loadFromFile(Common::Path(kWalkPaths[index]))) {
			delete _walkAnimations[index];
			_walkAnimations[index] = nullptr;
		}
	}
	SoundManager *soundManager = _vm->getSoundManager();
	if (soundManager) {
		for (int index = 0; index < 7; index++) {
			const Common::Path path(Common::String::format(kEffectPathFormat, index + 1));
			_effectIds[index] = soundManager->load(false, path, false);
		}
		for (int index = 0; index < 4; index++) {
			const Common::Path path(Common::String::format(kBoardVoicePathFormat, index + 1));
			_boardVoiceIds[index] = soundManager->load(false, path, false);
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
	if (_puzzleLevel == 1)
		traitCount = 2;
	else if (_puzzleLevel == 2)
		traitCount = 3;
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
	for (uint index = 0; index < balls.size(); index++)
		delete balls[index].path;
	balls.clear();
}

void PuzzleBoolies::clearBalls() {
	clearBallArray(_balls);
	clearBallArray(_nextBalls);
	clearBallArray(_followingBalls);
}

void PuzzleBoolies::clearJumps() {
	for (uint index = 0; index < _jumps.size(); index++)
		delete _jumps[index].path;
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
	_followingChallengeType = 0;
	_challengeType = pickChallengeType();
	const int count = _challengeType < 0 ? -_challengeType : _challengeType;
	for (int index = 0; index < count && index < kMaxChallengeBalls; index++) {
		Ball ball;
		ball.index = index;
		ball.startAt = now + kBallLaunchInterval * index;
		ball.path = PathObject::loadFromPAT(_vm, Common::Path(Common::String::format(kFeederPathFormat, index + 1)));
		ball.pos = getPreviewHoldPosition(index);
		if (!ball.path || ball.path->segments.empty()) {
			ball.pos = getReadyHoldPosition(index);
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
	for (int index = 0; index < count && index < kMaxChallengeBalls; index++) {
		Ball ball;
		ball.index = index;
		ball.startAt = now + kBallLaunchInterval * index;
		ball.path = PathObject::loadFromPAT(_vm, Common::Path(kPreviewEntryPath));
		if (ball.path && !ball.path->segments.empty()) {
			ball.pos = ball.path->segments[0]->getStartPosition();
			ball.stage = BallStage::kPreviewWaiting05;
		} else {
			ball.pos = getPreviewHoldPosition(index);
			ball.stage = BallStage::kPreviewHeld07;
		}
		balls.push_back(ball);
	}
}

void PuzzleBoolies::startNextFeeder(uint32 now) {
	if (!_nextPrepared || _nextFeeding)
		return;
	_nextFeeding = true;
	for (uint index = 0; index < _nextBalls.size(); index++) {
		Ball &ball = _nextBalls[index];
		delete ball.path;
		ball.path = PathObject::loadFromPAT(_vm, Common::Path(Common::String::format(kFeederPathFormat, ball.index + 1)));
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
	for (uint index = 0; index < _nextBalls.size(); index++)
		_balls.push_back(_nextBalls[index]);
	_nextBalls.clear();
	_challengeType = _nextChallengeType;
	_nextChallengeType = _followingChallengeType;
	_nextPrepared = _followingPrepared;
	for (uint index = 0; index < _followingBalls.size(); index++)
		_nextBalls.push_back(_followingBalls[index]);
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
		soundManager->playWithVolume(soundId, soundManager->_volumeSFX);
}

void PuzzleBoolies::playBoardVoice() {
	const int voiceIndex = _vm->_rnd->getRandomNumber(3);
	playEffect(_boardVoiceIds[voiceIndex]);
}

void PuzzleBoolies::init() {
	PuzzleBase::init();
	startPageMusic(Common::Path(kMusicPath));
	if (_puzzleLevel < 1 || 4 < _puzzleLevel) {
		warning("Boolies: unsupported difficulty %d", _puzzleLevel);
		_phase = Phase::kFinished05;
		return;
	}
	loadResources();
	_blockerStart = _vm->getGameTickCount();
	const int rosterCount = static_cast<int>(_puzzleZoombinis.size());
	for (int index = 0; index < rosterCount; index++) {
		ZoombiniRunner *zoombini = _puzzleZoombinis[index];
		if (!zoombini)
			continue;
		zoombini->setDefaultAnimation(_zoombiniAnimation);
		zoombini->_rescuedBooliesPerZoombini = getRescuedBooliesPerZoombini(_puzzleLevel);
		zoombini->_puzzleStatus = 0;
		zoombini->_inputEnabled = false;
		zoombini->_hidden = index != 0;
		Common::Point32 position(120, 484);
		if (index == 0)
			position = Common::Point32(115, 495);
		zoombini->setPosition(position);
	}
	for (int row = 0; row < kRowCount; row++)
		generateRow(row);
	byte entryValues[kSlotCount];
	for (int slot = 0; slot < kSlotCount; slot++)
		entryValues[slot] = _boolies[kRowCount - 1][slot].value;
	startRefill(kRowCount - 1, entryValues, _vm->getGameTickCount());
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
	beginRound(_vm->getGameTickCount());
}

void PuzzleBoolies::startLaneBall(Ball &ball, uint32 now) {
	if (_selectedRow < 0)
		return;
	delete ball.path;
	ball.path = PathObject::loadFromPAT(_vm, Common::Path(Common::String::format(kLanePathFormat, _selectedRow + 2)));
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
	if (ball.stage == BallStage::kFeeder01) {
		if (!ball.path->advance(now, ball.pos)) {
			ball.pos = getReadyHoldPosition(ball.index);
			ball.stage = BallStage::kHeld02;
		}
	} else if (ball.stage == BallStage::kLane03) {
		const bool moving = ball.path->advance(now, ball.pos);
		if (!moving || ball.pos.x - 17 <= 180) {
			resolveBall(ball, now);
			delete ball.path;
			ball.path = PathObject::loadFromPAT(_vm, Common::Path(Common::String::format(kBallExitPathFormat, _selectedRow + 2)));
			if (ball.path) {
				ball.path->start(now);
				ball.stage = BallStage::kExit08;
			} else {
				ball.stage = BallStage::kDone04;
			}
		}
	} else if (ball.stage == BallStage::kExit08) {
		// The finite exit path keeps requesting the impact sample as voices become available.
		playEffect(_effectIds[6]);
		if (!ball.path->advance(now, ball.pos) || 810 < ball.pos.x - 17)
			ball.stage = BallStage::kDone04;
	}
}

void PuzzleBoolies::advanceNextBalls(uint32 now) {
	advancePreviewBalls(_nextBalls, now);
	advancePreviewBalls(_followingBalls, now);
	if (_nextFeeding) {
		for (uint index = 0; index < _nextBalls.size(); index++)
			advanceBall(_nextBalls[index], now);
		bool allHeld = !_nextBalls.empty();
		for (uint index = 0; index < _nextBalls.size(); index++)
			allHeld = allHeld && _nextBalls[index].stage == BallStage::kHeld02;
		const int pendingTurn = _phase == Phase::kRolling01 ? 1 : 0;
		if (allHeld && !_followingPrepared && _completedTurns + pendingTurn + 1 < _requiredTurns)
			prepareFollowingChallenge(now);
		return;
	}
	if (!_nextFeedingRequested)
		return;
	for (uint index = 0; index < _nextBalls.size(); index++) {
		if (_nextBalls[index].stage != BallStage::kPreviewHeld07)
			return;
	}
	startNextFeeder(now);
}

void PuzzleBoolies::advancePreviewBalls(Common::Array<Ball> &balls, uint32 now) {
	for (uint index = 0; index < balls.size(); index++) {
		Ball &ball = balls[index];
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
	for (uint index = 0; index < _balls.size(); index++) {
		if (_balls[index].stage == BallStage::kWaiting00 || _balls[index].stage == BallStage::kFeeder01)
			return true;
	}
	for (uint index = 0; index < _nextBalls.size(); index++) {
		if (_nextBalls[index].stage == BallStage::kWaiting00 || _nextBalls[index].stage == BallStage::kFeeder01)
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
		jump.path = PathObject::loadFromPAT(_vm, Common::Path(Common::String::format(kJumpPathFormat, row, slot + 1)));
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
	if (_puzzleLevel == 1)
		traitCount = 2;
	else if (_puzzleLevel == 2)
		traitCount = 3;
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
		_puzzleZoombinis[_activeRunnerIndex]->_puzzleStatus = 1;
	_boatState = BoatState::kLeaving01;
	_boatTime = now;
	_returnBoatActive = false;
}

void PuzzleBoolies::finishRound() {
	_completedTurns += 1;
	_boardingCheckPending = true;
	if (_requiredTurns < _completedTurns) {
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
			_puzzleZoombinis[_activeRunnerIndex + 1]->_hidden = false;
			_puzzleZoombinis[_activeRunnerIndex + 1]->setPosition(Common::Point32(_returnBoatX + 35, 495));
			_vm->_zoombiniWalkingFlag = true;
		}
		if (810 < _boatX && lastRunner) {
			_boatState = BoatState::kReady00;
			_phase = Phase::kFinished05;
			_vm->_zoombiniWalkingFlag = true;
			_vm->restartGoBlink();
			if (_vm->_isSavedGame) {
				if (SoundManager *sound = _vm->getSoundManager())
					sound->queueSpeech(Common::Path(kCompleteSpeechPath));
			}
			return;
		}
		if (810 < _boatX) {
			_puzzleZoombinis[_activeRunnerIndex]->_hidden = true;
			_boatState = BoatState::kReturning02;
		}
	}
	if (!returning)
		return;
	_returnBoatX = static_cast<int>(_returnBoatX + distance);
	_puzzleZoombinis[_activeRunnerIndex + 1]->setPosition(Common::Point32(_returnBoatX + 35, 495));
	if (_returnBoatX < 80)
		return;
	_puzzleZoombinis[_activeRunnerIndex]->_hidden = true;
	_activeRunnerIndex += 1;
	_returnBoatActive = false;
	_passengers.clear();
	_boatX = 80;
	_boatState = BoatState::kReady00;
	_puzzleZoombinis[_activeRunnerIndex]->setPosition(Common::Point32(115, 495));
	_vm->_zoombiniWalkingFlag = true;
	if (_requiredTurns < _completedTurns) {
		_phase = Phase::kFinished05;
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
	const uint32 now = _vm->getGameTickCount();
	if (_goPending) {
		SoundManager *sound = _vm->getSoundManager();
		if (!sound || !sound->hasPendingSpeech()) {
			_vm->_mapTransitionSourcePageId = kPageBoolies;
			_vm->requestPageChange(kPageMapTrans);
		}
		return;
	}
	advanceRefill(now);
	advanceBoat(now);
	for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
		if (_puzzleZoombinis[index])
			_puzzleZoombinis[index]->updateAnimation(now);
	}
	if (_nextPrepared)
		advanceNextBalls(now);
	if (_phase == Phase::kFeeding00 || _phase == Phase::kRolling01) {
		if (_phase == Phase::kRolling01 && _nextLaneBallIndex < _balls.size() && _nextLaneLaunchAt <= now) {
			startLaneBall(_balls[_nextLaneBallIndex], now);
			_nextLaneBallIndex += 1;
			_nextLaneLaunchAt = now + kBallLaunchInterval;
		}
		for (int index = static_cast<int>(_balls.size()) - 1; 0 <= index; index--)
			advanceBall(_balls[index], now);
		if (_phase == Phase::kFeeding00) {
			bool allHeld = !_balls.empty();
			for (uint index = 0; index < _balls.size(); index++)
				allHeld = allHeld && _balls[index].stage == BallStage::kHeld02;
			if (allHeld && !_nextPrepared && _completedTurns + 1 < _requiredTurns)
				prepareNextChallenge(now);
		} else {
			bool allDone = true;
			for (uint index = 0; index < _balls.size(); index++)
				allDone = allDone && _balls[index].stage == BallStage::kDone04;
			if (allDone) {
				_nextFeedingRequested = true;
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
	if (_pinRoute == 0) {
		_vm->_gfx->drawPageRleBlockClipped(screen, kPinPath, Common::Point32(435, 120), Common::Rect32(0, 0, 518, 193));
		_vm->_gfx->drawPageRleBlockClipped(screen, kPinPath, Common::Point32(518, 95), Common::Rect32(0, 0, 601, 168));
	} else if (_pinRoute == 1) {
		_vm->_gfx->drawPageRleBlockClipped(screen, kPinPath, Common::Point32(363, 142), Common::Rect32(446, 0, 529, 215));
		_vm->_gfx->drawPageRleBlockClipped(screen, kPinPath, Common::Point32(432, 91), Common::Rect32(515, 0, 598, 164));
	} else {
		_vm->_gfx->drawPageRleBlock(screen, kPinLightedPath, Common::Point32(435, 120));
		_vm->_gfx->drawPageRleBlockClipped(screen, kPinPath, Common::Point32(358, 95), Common::Rect32(524, 0, 690, 168));
	}
}

void PuzzleBoolies::drawSpotHighlights(ManagedSurface32 *screen) const {
	if (_hoverRow == 0) {
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 2), Common::Point32(250, 142));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 1), Common::Point32(322, 134));
	} else if (_hoverRow == 1) {
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 5), Common::Point32(247, 250));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 3), Common::Point32(309, 242));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 4), Common::Point32(371, 223));
	} else if (_hoverRow == 2) {
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 4), Common::Point32(277, 353));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 1), Common::Point32(389, 324));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 5), Common::Point32(469, 257));
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kSpotFormat, 3), Common::Point32(509, 196));
	}
}

void PuzzleBoolies::onRenderContent(ManagedSurface32 *screen) {
	const uint32 now = _vm->getGameTickCount();
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
	for (uint index = 0; index < _jumps.size(); index++) {
		const Jump &jump = _jumps[index];
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
	for (int index = static_cast<int>(_balls.size()) - 1; 0 <= index; index--) {
		const Ball &ball = _balls[index];
		if (ball.stage != BallStage::kDone04) {
			const char *path = kBallPosPath;
			if (_challengeType <= 0)
				path = kBallNegPath;
			_vm->_gfx->drawPageRleBlock(screen, path, Common::Point32(ball.pos.x - 17, ball.pos.y - 35));
		}
	}
	for (int index = static_cast<int>(_nextBalls.size()) - 1; 0 <= index; index--) {
		const Ball &ball = _nextBalls[index];
		if (ball.stage != BallStage::kPreviewWaiting05 && ball.stage != BallStage::kDone04) {
			const char *path = kBallPosPath;
			if (_nextChallengeType <= 0)
				path = kBallNegPath;
			_vm->_gfx->drawPageRleBlock(screen, path, Common::Point32(ball.pos.x - 17, ball.pos.y - 35));
		}
	}
	for (int index = static_cast<int>(_followingBalls.size()) - 1; 0 <= index; index--) {
		const Ball &ball = _followingBalls[index];
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
	for (uint index = 0; index < _passengers.size(); index++) {
		const Passenger &passenger = _passengers[index];
		_vm->_gfx->drawPageRleBlock(screen, passenger.value == 1 ? kFixePath : kFixe2Path,
									Common::Point32(passenger.pos.x + _boatX - 80 - 35, passenger.pos.y - 69));
	}
}

void PuzzleBoolies::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleBoolies::onActorsRendered() {
	for (uint index = 0; index < _puzzleZoombinis.size(); index++) {
		if (_puzzleZoombinis[index])
			_puzzleZoombinis[index]->advanceAnimationAfterDraw();
	}
}

EventHandleResult PuzzleBoolies::onLButtonDown(const Common::Point &pos) {
	if (_phase != Phase::kFeeding00 || _selectedRow != -1 || _requiredTurns < _completedTurns)
		return EventHandleResult::kPassthrough;
	const bool lastRunner = _activeRunnerIndex + 1 == static_cast<int>(_puzzleZoombinis.size());
	if (lastRunner && (!_jumps.empty() || _boatState != BoatState::kReady00))
		return EventHandleResult::kPassthrough;
	for (uint index = 0; index < _balls.size(); index++) {
		if (_balls[index].stage != BallStage::kHeld02)
			return EventHandleResult::kPassthrough;
	}
	const int row = getRowForPoint(pos);
	if (row < 0 || isRowEmpty(row) || (_refillActive && row == _refillRow))
		return EventHandleResult::kPassthrough;
	_selectedRow = row;
	_pinRoute = row;
	const uint32 now = _vm->getGameTickCount();
	_nextLaneBallIndex = 0;
	_nextLaneLaunchAt = now + (row == 2 ? 0 : kFirstBallLaunchDelay);
	_phase = Phase::kRolling01;
	return EventHandleResult::kConsumed;
}

EventHandleResult PuzzleBoolies::onMouseMove(const Common::Point &pos) {
	_hoverRow = _phase == Phase::kFeeding00 ? getRowForPoint(pos) : -1;
	return EventHandleResult::kPassthrough;
}

Common::String PuzzleBoolies::debugGetAnswer() const {
	Common::String answer = debugAnswerHeader();
	answer += "\n";
	answer += Common::String::format("  Current balls: %+d. Rows run from top to bottom.\n", _challengeType);
	if (_nextPrepared)
		answer += Common::String::format("  Next balls: %+d.\n", _nextChallengeType);
	if (_phase != Phase::kFeeding00 || _selectedRow != -1)
		return answer + "  Balls are moving. Query again before choosing another row.\n";
	answer += "  Result of choosing each row:\n";
	for (int row = 0; row < kRowCount; row++) {
		if (isRowEmpty(row) || (_refillActive && row == _refillRow)) {
			answer += Common::String::format("    Row %d: unavailable\n", row + 1);
			continue;
		}
		byte values[kSlotCount];
		answer += Common::String::format("    Row %d:", row + 1);
		for (int slot = 0; slot < kSlotCount; slot++) {
			values[slot] = _boolies[row][slot].value;
			if (!_boolies[row][slot].removed)
				answer += Common::String::format(" %d", values[slot]);
		}
		for (uint ball = 0; ball < _balls.size(); ball++) {
			for (int slot = 0; slot < kSlotCount; slot++) {
				if (_boolies[row][slot].removed || _boolies[row][slot].jumping)
					break;
				const byte previous = values[slot];
				values[slot] = previous == 1 ? 2 : 1;
				if ((_challengeType < 0 && previous == 1) || (0 < _challengeType && previous == 2))
					break;
			}
		}
		answer += " ->";
		bool cleared = true;
		for (int slot = 0; slot < kSlotCount; slot++) {
			if (_boolies[row][slot].removed)
				continue;
			answer += Common::String::format(" %d", values[slot]);
			cleared = cleared && values[slot] != 2;
		}
		answer += cleared ? " (boards the boat)\n" : " (stays)\n";
	}
	return answer;
}

bool PuzzleBoolies::canUseGoButton() const {
	return !_goPending && _vm->_zoombiniWalkingFlag;
}

bool PuzzleBoolies::onGoButtonPressed() {
	if (!_vm->_isSavedGame)
		return true;
	if (_goPending)
		return false;
	int remaining = 0;
	for (const ZoombiniRunner *runner : _puzzleZoombinis) {
		if (runner->_puzzleStatus == 0)
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
	return PuzzleChanceInfo(PuzzleChanceInfo::Type::kSubmit, _requiredTurns + 1, _completedTurns, "resolved challenge");
}

bool PuzzleBoolies::debugCanSetChances() const {
	return !_debugFinishPending && _phase == Phase::kFeeding00 && _selectedRow == -1;
}

bool PuzzleBoolies::debugSetChances(int remaining) {
	if (!debugCanSetChances() || remaining < 0 || _requiredTurns + 1 < remaining)
		return false;
	_completedTurns = _requiredTurns + 1 - remaining;
	if (!remaining) {
		_phase = Phase::kFinished05;
		_vm->_zoombiniWalkingFlag = true;
		_vm->restartGoBlink();
	}
	return true;
}

} // End of namespace Zoombini2
