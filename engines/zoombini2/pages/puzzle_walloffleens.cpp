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

#include "zoombini2/pages/puzzle_walloffleens.h"
#include "common/debug.h"
#include "zoombini2/graphics.h"
#include "zoombini2/random.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleWallOfFleens::kMusicPath;
constexpr const char *PuzzleWallOfFleens::kCannonFormat;
constexpr const char *PuzzleWallOfFleens::kMirrorPaths[5];
constexpr const char *PuzzleWallOfFleens::kBallPaths[6];
constexpr const char *PuzzleWallOfFleens::kOverlayPaths[2];
constexpr const char *PuzzleWallOfFleens::kScoreFormat;
constexpr const char *PuzzleWallOfFleens::kRotatePath;
constexpr const char *PuzzleWallOfFleens::kExplodePath;
constexpr const char *PuzzleWallOfFleens::kBallLoadPath;
constexpr const char *PuzzleWallOfFleens::kJumpPath;
constexpr const char *PuzzleWallOfFleens::kFleenPath;
constexpr const char *PuzzleWallOfFleens::kVocif1Path;
constexpr const char *PuzzleWallOfFleens::kVocif2Path;
constexpr const char *PuzzleWallOfFleens::kCannonZombPath;
constexpr const char *PuzzleWallOfFleens::kJumpAnimationPath;
constexpr const char *PuzzleWallOfFleens::kCelebratePath;
constexpr const char *PuzzleWallOfFleens::kSoundPaths[8];
constexpr const char *PuzzleWallOfFleens::kSuccessSpeechPath;
constexpr const char *PuzzleWallOfFleens::kLossSpeechPath;
constexpr const char *PuzzleWallOfFleens::kRetreatSpeechPaths[2];
constexpr const char *PuzzleWallOfFleens::kAmbientFormat;
constexpr const char *PuzzleWallOfFleens::kPerfectGoFormat;

constexpr Common::Point32 PuzzleWallOfFleens::kPanelOrigins[6];
constexpr Common::Point32 PuzzleWallOfFleens::kGridOrigins[5];
constexpr Common::Point32 PuzzleWallOfFleens::kMuzzles[9];

PuzzleWallOfFleens::PuzzleWallOfFleens(Zoombini2Engine *vm) : PuzzleBase(vm, kPageWallOfFleens) {
	for (int i = 0; i < 8; i++)
		_sounds[i] = -1;
	for (int i = 0; i < 3; i++)
		_ambientSounds[i] = -1;
}

PuzzleWallOfFleens::~PuzzleWallOfFleens() {
	delete _projectilePath;
	for (uint i = 0; i < _railPaths.size(); i++)
		delete _railPaths[i];
	delete _rotate;
	delete _explode;
	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		for (int i = 0; i < 8; i++)
			sound->unload(_sounds[i]);
		for (int i = 0; i < 3; i++)
			sound->unload(_ambientSounds[i]);
	}
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		_puzzleZoombinis[i]->clearMovement();
		_puzzleZoombinis[i]->setAnimationCompleteCallback(nullptr);
	}
	finishPuzzleRoster(_vm->_state->_rescue1Storage, _perfectClearEligible);
}

void PuzzleWallOfFleens::loadResources() {
	for (int i = 0; i < 9; i++)
		_vm->_gfx->loadPageRleBlock(Common::String::format(kCannonFormat, i));
	for (int i = 0; i < 5; i++) {
		_vm->_gfx->loadPageRleBlock(kMirrorPaths[i]);
		_vm->_gfx->loadPageRleBlock(Common::String::format(kScoreFormat, i));
	}
	for (int i = 0; i < 6; i++)
		_vm->_gfx->loadPageRleBlock(kBallPaths[i]);
	for (int i = 0; i < 2; i++)
		_vm->_gfx->loadPageRleBlock(kOverlayPaths[i]);
	_rotate = new Animation(_vm);
	if (!_rotate->loadFromFile(Common::Path(kRotatePath))) {
		delete _rotate;
		_rotate = nullptr;
	}
	_explode = new Animation(_vm);
	if (!_explode->loadFromFile(Common::Path(kExplodePath))) {
		delete _explode;
		_explode = nullptr;
	}
	_fleensAnimation = _vm->loadZoombiniAnimation(Common::Path(kFleenPath), 100);
	_vocif1 = _vm->loadZoombiniAnimation(Common::Path(kVocif1Path), 70);
	_vocif2 = _vm->loadZoombiniAnimation(Common::Path(kVocif2Path), 100);
	_cannonAnimation = _vm->loadZoombiniAnimation(Common::Path(kCannonZombPath), 50);
	_jumpAnimation = _vm->loadZoombiniAnimation(Common::Path(kJumpAnimationPath), 70);
	_celebrationAnimation = _vm->loadZoombiniAnimation(Common::Path(kCelebratePath), 50);
	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		for (int i = 0; i < 8; i++)
			_sounds[i] = sound->load(false, Common::Path(kSoundPaths[i]), false);
		static constexpr int kAmbientVariants[3] = {
			2,
			3,
			5,
		};
		for (int i = 0; i < 3; i++)
			_ambientSounds[i] = sound->load(false, Common::Path(Common::String::format(kAmbientFormat, kAmbientVariants[i])), false);
	}
}

void PuzzleWallOfFleens::init() {
	PuzzleBase::init();
	_level = CLIP(_puzzleLevel, 1, 4);
	loadResources();
	startPageMusic(Common::Path(kMusicPath));
	_initialPartyCount = _puzzleZoombinis.size();
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *z = _puzzleZoombinis[i];
		z->clearMovement();
		z->setDefaultAnimation(_zoombiniAnimation);
		z->resetAnimation();
		z->setPosition(Common::Point32(50 + 30 * i, 490));
		z->_hidden = false;
		z->_inputEnabled = false;
		z->_puzzleStatus = 1;
	}
	_ballsLeft = 8;
	if (_level == 1)
		_ballsLeft = 12;
	else if (_level == 3)
		_ballsLeft = 6;
	_railPaths.resize(_ballsLeft);
	_railPositions.resize(_ballsLeft);
	_easyHistory.clear();
	for (int i = 0; i < _ballsLeft; i++)
		_railPositions[i] = Common::Point32(690 + 23 * (_ballsLeft - 1 - i), 424);
	buildGrid();
	loadNextProjectile();
	_nextAmbientTick = _vm->getGameTickCount() + 1000 * (_vm->_rnd->getRandomNumber(15) + 15);
}

void PuzzleWallOfFleens::permute(int *values, int count) {
	for (int i = 0; i < count; i++) {
		bool duplicate;
		do {
			values[i] = _vm->_rnd->getRandomNumber(count - 1);
			duplicate = false;
			for (int j = 0; j < i; j++) {
				if (values[j] == values[i])
					duplicate = true;
			}
		} while (duplicate);
	}
}

void PuzzleWallOfFleens::generateEasyPanel() {
	const int pattern = _vm->_rnd->getRandomNumber(122);
	int slots[6];
	int traits[4];
	int values[4][5];
	permute(slots, 6);
	permute(traits, 4);
	for (int i = 0; i < 4; i++)
		permute(values[i], 5);
	for (int i = 0; i < 6; i++) {
		for (int t = 0; t < 4; t++) {
			const int valueIndex = i == 5 ? 0 : kL1Patterns[pattern][i][t];
			ZmbTrait &tuple = _cells[slots[i]].traits;
			byte *fields[4] = {
				&tuple._feet,
				&tuple._nose,
				&tuple._hair,
				&tuple._eyes,
			};
			*fields[traits[t]] = values[t][valueIndex] + 1;
		}
	}
	_target = slots[5];
	_alternateTarget = slots[0];
}

void PuzzleWallOfFleens::buildGrid() {
	_columns = 12;
	if (_level == 1)
		_columns = 3;
	else if (_level == 2)
		_columns = 9;
	_cellCount = _columns * (_level == 1 ? 2 : 6);
	_cells.resize(_cellCount);
	const Common::Point32 origin = _level == 1 ? kPanelOrigins[_panel] : kGridOrigins[_level];
	for (int i = 0; i < _cellCount; i++) {
		_cells[i] = Cell();
		_cells[i].pos = Common::Point32(origin.x + (i % _columns) * 52, origin.y + (i / _columns) * 68);
	}
	_clickCount = 0;
	if (_level == 1) {
		generateEasyPanel();
	} else {
		for (int i = 0; i < _cellCount; i++) {
			bool duplicate;
			do {
				ZmbTrait &traits = _cells[i].traits;
				traits._feet = _vm->_rnd->getRandomNumber(4) + 1;
				traits._nose = _vm->_rnd->getRandomNumber(4) + 1;
				traits._hair = _vm->_rnd->getRandomNumber(4) + 1;
				traits._eyes = _vm->_rnd->getRandomNumber(4) + 1;
				duplicate = false;
				for (int j = 0; j < i; j++) {
					if (traits == _cells[j].traits)
						duplicate = true;
				}
			} while (duplicate);
		}
		_target = _vm->_rnd->getRandomNumber(_cellCount - 1);
		if (_level == 4) {
			do {
				_alternateTarget = _vm->_rnd->getRandomNumber(_cellCount - 1);
			} while (_target == _alternateTarget);
		}
	}
	debug(2, "WallOfFleens: level=%d panel=%d target=%d alternate=%d balls=%d", _level, _panel, _target, _alternateTarget, _ballsLeft);
}

int PuzzleWallOfFleens::compareTraits(int first, int second) const {
	int score = 0;
	for (int i = 0; i < ZmbTrait::kTraitCount; i++) {
		const ZmbTrait::TraitIndex trait = static_cast<ZmbTrait::TraitIndex>(i);
		if (_cells[first].traits.getValue(trait) == _cells[second].traits.getValue(trait))
			score += 1;
	}
	return score;
}

PathObject *PuzzleWallOfFleens::makeLine(const Common::Point32 &start, const Common::Point32 &end, int step) {
	PathObject *path = new PathObject(_vm);
	const Common::Point32 third((end.x - start.x) / 3, (end.y - start.y) / 3);
	path->appendSegment(start, Common::Point32(start.x + third.x, start.y + third.y),
						Common::Point32(end.x - third.x, end.y - third.y), end, step, 0);
	return path;
}

void PuzzleWallOfFleens::loadNextProjectile() {
	if (_finished || _retreating)
		return;
	const uint32 now = _vm->getGameTickCount();
	if (_ballsLeft == 0) {
		if (_selected != -1 && (_level != 1 || _panel == 5)) {
			const int score = _cells[_selected].score;
			bool finalCatch = score == 4;
			if (_level == 4)
				finalCatch = _caughtTargets == 1 && (score % 10 == 4 || score / 10 == 4);
			if (finalCatch)
				return;
		}
		if (_puzzleZoombinis.size() <= 1) {
			if (_mirrorPhase == MirrorPhase::kNone00)
				startRetreat();
			return;
		}
		_loadedRunner = _puzzleZoombinis.size() - 1;
		ZoombiniRunner *z = _puzzleZoombinis[_loadedRunner];
		PathObject *path = PathObject::loadFromPAT(_vm, Common::Path(kJumpPath));
		if (!path) {
			warning("WallOfFleens: missing required cannon loading path");
			return;
		}
		for (int i = _loadedRunner - 1; 0 <= i; i--) {
			ZoombiniRunner *waiting = _puzzleZoombinis[i];
			waiting->startMovement(makeLine(waiting->_screenPos, _puzzleZoombinis[i + 1]->_screenPos, 7), now);
			waiting->startAnimation(_zoombiniAnimation, 66, now);
		}
		z->startMovement(path, now);
		z->startAnimation(_jumpAnimation, 66, now);
	} else {
		_loadedRunner = -1;
		delete _projectilePath;
		_projectilePath = PathObject::loadFromPAT(_vm, Common::Path(kBallLoadPath));
		if (!_projectilePath) {
			warning("WallOfFleens: missing required ammunition loading path");
			return;
		}
		_projectilePath->start(now);
		_projectilePos = _railPositions[_ballsLeft - 1];
		for (int i = _ballsLeft - 2; 0 <= i; i--) {
			delete _railPaths[i];
			_railPaths[i] = makeLine(_railPositions[i], _railPositions[i + 1], 7);
			_railPaths[i]->start(now);
		}
		playEffect(2);
	}
	_shotPhase = ShotPhase::kLoading00;
	_shotConsumed = false;
}

int PuzzleWallOfFleens::direction(const Common::Point32 &start, const Common::Point32 &end, int sectors) {
	const double dx = end.x - start.x;
	const double dy = start.y - end.y;
	const double distance = sqrt(dx * dx + dy * dy);
	if (distance == 0)
		return 0;
	double angle = acos(dx / distance) * 180.0 / M_PI;
	if (start.y < end.y)
		angle = -angle;
	return static_cast<int>(angle) / (360 / sectors);
}

void PuzzleWallOfFleens::startFlight(const Common::Point32 &start, const Common::Point32 &end, int step) {
	delete _projectilePath;
	_projectilePath = makeLine(start, end, step);
	_projectilePath->start(_vm->getGameTickCount());
	_projectilePos = start;
	static constexpr int kCells[5] = {
		6,
		9,
		8,
		7,
		4,
	};
	static constexpr int kImages[5] = {
		2,
		5,
		4,
		3,
		1,
	};
	const int sector = CLIP(direction(start, end, 10), 0, 4);
	_projectileCell = kCells[sector];
	_projectileImage = kImages[sector];
	if (0 <= _loadedRunner) {
		_projectileRunner.setTraits(_puzzleZoombinis[_loadedRunner]->_traits);
		_projectileRunner.setDefaultAnimation(_cannonAnimation, _projectileCell);
		_projectileRunner._animationCell = _projectileCell;
	}
}

EventHandleResult PuzzleWallOfFleens::onLButtonUp(const Common::Point &pos) {
	if (_finished || _retreating || _shotPhase != ShotPhase::kReady01 || _mirrorPhase != MirrorPhase::kNone00)
		return EventHandleResult::kPassthrough;
	for (int i = 0; i < _cellCount; i++) {
		Cell &cell = _cells[i];
		if (pos.x < cell.pos.x || cell.pos.x + 52 <= pos.x || pos.y < cell.pos.y || cell.pos.y + 68 <= pos.y)
			continue;
		const int previousClicks = _clickCount;
		_clickCount += 1;
		if (cell.tried)
			return EventHandleResult::kConsumed;
		if (_level == 1 && previousClicks == 0 && i == _target)
			_target = _alternateTarget;
		cell.tried = true;
		cell.score = compareTraits(i, _target);
		if (_level == 4)
			cell.score += 10 * compareTraits(i, _alternateTarget);
		_selected = i;
		_targetAngle = CLIP(direction(Common::Point32(470, 550), Common::Point32(cell.pos.x + 26, cell.pos.y + 34), 18), 0, 8);
		_shotPhase = ShotPhase::kAiming02;
		_resetting = false;
		debug(2, "WallOfFleens: shot cell=%d score=%d balls=%d runner=%d", i, cell.score, _ballsLeft, _loadedRunner);
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

void PuzzleWallOfFleens::finishShot() {
	_shotConsumed = true;
	_shotPhase = ShotPhase::kStopped05;
	_resetting = true;
	if (_loadedRunner < 0) {
		_ballsLeft -= 1;
	} else {
		ZoombiniRunner *lost = _puzzleZoombinis[_loadedRunner];
		if (_vm->_isSavedGame)
			GameState::storeInStorage(_vm->_state->_rescue1Storage, *lost);
		for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
			if (_vm->_state->_activeZoombinis[i] == lost) {
				_vm->_state->_activeZoombinis.remove_at(i);
				break;
			}
		}
		_puzzleZoombinis.remove_at(_loadedRunner);
		delete lost;
		_loadedRunner = -1;
	}
	debug(2, "WallOfFleens: shot consumed balls=%d survivors=%u", _ballsLeft, _puzzleZoombinis.size());
	if (!_finished)
		loadNextProjectile();
}

void PuzzleWallOfFleens::onFleenAnimationDone(void *context, ZoombiniRunner *runner) {
	PuzzleWallOfFleens *page = static_cast<PuzzleWallOfFleens *>(context);
	if (page->_mirrorPhase == MirrorPhase::kShouting03 && page->_shoutCount == 0) {
		page->_shoutCount = 1;
		page->playEffect(7);
		runner->startAnimation(page->_vocif1, 55, page->_vm->getGameTickCount());
		runner->setAnimationCompleteCallback(onFleenAnimationDone, page);
	} else if (page->_mirrorPhase == MirrorPhase::kShouting03) {
		page->_mirrorPhase = MirrorPhase::kLeaving04;
		runner->startAnimation(page->_vocif2, 56, page->_vm->getGameTickCount());
		runner->setAnimationCompleteCallback(onFleenAnimationDone, page);
	} else {
		page->finishCatch();
	}
}

void PuzzleWallOfFleens::finishCatch() {
	_perfectClearEligible = true;
	_cells[_selected].empty = true;
	_foundPrimary = _foundPrimary || _cells[_selected].score % 10 == 4;
	_foundSecondary = _foundSecondary || (_level == 4 && _cells[_selected].score / 10 == 4);
	_mirrorPhase = MirrorPhase::kNone00;
	_fleensRunner.resetAnimation();
	_vm->restartGoBlink();
	if (_level == 1 && _panel < 5) {
		for (int i = 0; i < 6; i++)
			_easyHistory.push_back(_cells[i]);
		_panel += 1;
		buildGrid();
		_selected = -1;
	} else if (_level == 4 && _caughtTargets == 0) {
		_caughtTargets += 1;
	} else {
		_finished = true;
		_vm->_zoombiniWalkingFlag = !_puzzleZoombinis.empty();
		debug(2, "WallOfFleens: completed level=%d survivors=%u", _level, _puzzleZoombinis.size());
		queueSpeech(static_cast<int>(_puzzleZoombinis.size()) == _initialPartyCount ? kSuccessSpeechPath : kLossSpeechPath);
	}
	if (!_finished && _shotPhase == ShotPhase::kStopped05)
		loadNextProjectile();
}

void PuzzleWallOfFleens::startRetreat() {
	if (_retreating || _finished || _puzzleZoombinis.empty())
		return;
	_retreating = true;
	debug(2, "WallOfFleens: last survivor retreat");
	_shotPhase = ShotPhase::kStopped05;
	ZoombiniRunner *z = _puzzleZoombinis[0];
	const Common::Point32 end(-10 - z->_spriteSize.width, z->_screenPos.y);
	z->startMovement(makeLine(z->_screenPos, end, 2), _vm->getGameTickCount());
	z->setActiveAnimation(_zoombiniAnimation);
	z->startDirectionTrackedAnimation(_vm->getGameTickCount());
	z->_puzzleStatus = 0;
	_vm->restartGoBlink();
	if (_vm->_isSavedGame) {
		queueSpeech(kRetreatSpeechPaths[0]);
		queueSpeech(kRetreatSpeechPaths[1]);
	}
}

void PuzzleWallOfFleens::playEffect(int index) {
	SoundManager *sound = _vm->getSoundManager();
	if (sound && 0 <= _sounds[index])
		sound->playWithVolume(_sounds[index], sound->_volumeSFX);
}

void PuzzleWallOfFleens::queueSpeech(const Common::String &path) {
	SoundManager *sound = _vm->getSoundManager();
	if (sound)
		sound->queueSpeech(Common::Path(path));
}

void PuzzleWallOfFleens::onUpdate() {
	const uint32 now = _vm->getGameTickCount();
	SoundManager *sound = _vm->getSoundManager();
	if (_goTransitionPending && (!sound || !sound->hasPendingSpeech())) {
		_goTransitionPending = false;
		_vm->_mapTransitionSourcePageId = kPageWallOfFleens;
		_vm->requestPageChange(kPageMapTrans);
	}
	if (!_finished && !_retreating && _mirrorPhase != MirrorPhase::kShouting03 && _mirrorPhase != MirrorPhase::kLeaving04 && _nextAmbientTick < now) {
		const int variant = _vm->_rnd->getRandomNumber(2);
		if (sound)
			sound->play(_ambientSounds[variant]);
		_nextAmbientTick = now + 1000 * (_vm->_rnd->getRandomNumber(15) + 15);
	}
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *z = _puzzleZoombinis[i];
		if (z->_movementPath && !z->advanceMovement(now)) {
			z->clearMovement();
			z->resetAnimation();
			if (static_cast<int>(i) == _loadedRunner && _shotPhase == ShotPhase::kLoading00) {
				z->_hidden = true;
				z->_puzzleStatus = 0;
				_shotPhase = ShotPhase::kReady01;
				_angle = 4;
				playEffect(3);
			}
		}
		if (_finished)
			z->tryStartCelebrationAnimation(_celebrationAnimation, *_vm->_rnd, now, _vm->getFrameDeltaMs(), _vm->getLogicPacingHz());
		z->updateAnimation(now);
	}
	_fleensRunner.updateAnimation(now);
	_projectileRunner.updateAnimation(now);
	for (int i = 0; i < _ballsLeft; i++) {
		if (_railPaths[i] && !_railPaths[i]->advance(now, _railPositions[i])) {
			delete _railPaths[i];
			_railPaths[i] = nullptr;
		}
	}
	if (_shotPhase == ShotPhase::kLoading00 && _loadedRunner < 0 && _projectilePath) {
		if (!_projectilePath->advance(now, _projectilePos)) {
			delete _projectilePath;
			_projectilePath = nullptr;
			_shotPhase = ShotPhase::kReady01;
			_angle = 4;
		}
	}
	if (_shotPhase == ShotPhase::kAiming02) {
		if (_angle == _targetAngle) {
			startFlight(kMuzzles[_angle], _cells[_selected].pos, 14);
			_shotPhase = ShotPhase::kOutbound03;
			playEffect(4);
		} else if (_aimTick < now) {
			_angle += _angle < _targetAngle ? 1 : -1;
			_aimTick = now + 200;
			playEffect(0);
		}
	}
	if (_resetting) {
		if (_angle == 4) {
			_resetting = false;
		} else if (_aimTick < now) {
			_angle += _angle < 4 ? 1 : -1;
			_aimTick = now + 200;
			playEffect(0);
		}
	}
	if ((_shotPhase == ShotPhase::kOutbound03 || _shotPhase == ShotPhase::kRebound04) && _projectilePath) {
		if (!_projectilePath->advance(now, _projectilePos)) {
			if (_shotPhase == ShotPhase::kOutbound03) {
				startFlight(_cells[_selected].pos, Common::Point32(470, -550), 4);
				_shotPhase = ShotPhase::kRebound04;
				_mirrorPhase = MirrorPhase::kRotating01;
				_mirrorTick = now;
				playEffect(5);
				playEffect(1);
			} else {
				delete _projectilePath;
				_projectilePath = nullptr;
				finishShot();
			}
		}
	}
	if (_mirrorPhase == MirrorPhase::kRotating01 && 1140 <= now - _mirrorTick) {
		_cells[_selected].revealed = true;
		if (_level == 4) {
			_cells[_selected].scoreMask = kScoreNone00;
			if (!_foundPrimary)
				_cells[_selected].scoreMask |= kShowPrimaryScore01;
			if (!_foundSecondary)
				_cells[_selected].scoreMask |= kShowSecondaryScore02;
		}
		if (sound)
			sound->stop(_sounds[1]);
		const int score = _cells[_selected].score;
		if (score % 10 == 4 || (_level == 4 && score / 10 == 4)) {
			_mirrorPhase = MirrorPhase::kExploding02;
			_mirrorTick = now;
			playEffect(6);
		} else {
			_mirrorPhase = MirrorPhase::kNone00;
			if (_shotPhase == ShotPhase::kStopped05)
				loadNextProjectile();
		}
	}
	if (_mirrorPhase == MirrorPhase::kExploding02 && 600 <= now - _mirrorTick) {
		_mirrorPhase = MirrorPhase::kShouting03;
		_shoutCount = 0;
		_fleensRunner.setTraits(_cells[_selected].traits);
		_fleensRunner.setDefaultAnimation(_fleensAnimation, 55);
		_fleensRunner.setPosition(_cells[_selected].pos);
		if (_vocif1 && _vocif2) {
			_fleensRunner.startAnimation(_vocif1, 55, now);
			_fleensRunner.setAnimationCompleteCallback(onFleenAnimationDone, this);
		} else {
			finishCatch();
		}
	}
	if (_mirrorPhase == MirrorPhase::kRotating01)
		playEffect(1);
}

void PuzzleWallOfFleens::drawCell(ManagedSurface32 *screen, const Cell &cell, bool active) const {
	if (cell.empty) {
		_vm->_gfx->drawPageRleBlock(screen, kMirrorPaths[4], cell.pos);
		return;
	}
	const bool reacting = active && _selected != -1 && &cell == &_cells[_selected] && _mirrorPhase != MirrorPhase::kNone00;
	if (reacting && _mirrorPhase != MirrorPhase::kRotating01) {
		if (_mirrorPhase == MirrorPhase::kExploding02) {
			_vm->_gfx->drawPageRleBlock(screen, kMirrorPaths[4], cell.pos);
			if (_fleensAnimation)
				_vm->_gfx->drawZoombini(screen, _fleensAnimation, cell.traits, Common::Point32(cell.pos.x + 2, cell.pos.y + 2), 55, 0);
			_vm->_gfx->drawPageRleBlock(screen, kMirrorPaths[3], cell.pos);
		} else {
			_vm->_gfx->drawPageRleBlock(screen, kMirrorPaths[4], cell.pos);
		}
		if (_mirrorPhase == MirrorPhase::kExploding02 && _explode) {
			const int frame = MIN<int>((_vm->getGameTickCount() - _mirrorTick) / 100, 5);
			_vm->_gfx->drawAnimationFrame(screen, _explode, frame, cell.pos);
		}
		return;
	}
	const bool revealed = cell.revealed;
	_vm->_gfx->drawPageRleBlock(screen, kMirrorPaths[revealed ? 2 : 0], cell.pos);
	if (_fleensAnimation)
		_vm->_gfx->drawZoombini(screen, _fleensAnimation, cell.traits, Common::Point32(cell.pos.x + 2, cell.pos.y + 2), 55, 0);
	_vm->_gfx->drawPageRleBlock(screen, kMirrorPaths[revealed ? 3 : 1], cell.pos);
	if (reacting && _rotate) {
		static constexpr int kFrames[21] = {
			0,
			1,
			2,
			3,
			4,
			5,
			4,
			3,
			2,
			1,
			0,
			1,
			2,
			3,
			4,
			5,
			4,
			3,
			2,
			1,
			0,
		};
		const uint32 elapsed = _vm->getGameTickCount() - _mirrorTick;
		int index = elapsed < 440 ? elapsed / 40 : 11 + (elapsed - 440) / 70;
		index = MIN(index, 20);
		_vm->_gfx->drawAnimationFrame(screen, _rotate, kFrames[index], cell.pos);
	} else if (revealed && 0 <= cell.score) {
		if (cell.scoreMask & kShowPrimaryScore01)
			_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kScoreFormat, cell.score % 10), cell.pos);
		if (cell.scoreMask & kShowSecondaryScore02) {
			RleBlock *scoreSprite = _vm->_gfx->loadPageRleBlock(Common::String::format(kScoreFormat, cell.score / 10));
			if (scoreSprite)
				scoreSprite->drawToScreenMirrored(screen, cell.pos, _vm->getAlphaLUT());
		}
	}
}

void PuzzleWallOfFleens::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

void PuzzleWallOfFleens::onRenderContent(ManagedSurface32 *screen) {
	if (_level == 1) {
		for (int panel = 0; panel < 6; panel++) {
			for (int i = 0; i < 6; i++) {
				if (panel < _panel) {
					drawCell(screen, _easyHistory[panel * 6 + i], false);
				} else if (_panel < panel) {
					const Common::Point32 pos(kPanelOrigins[panel].x + (i % 3) * 52, kPanelOrigins[panel].y + (i / 3) * 68);
					_vm->_gfx->drawPageRleBlock(screen, kMirrorPaths[0], pos);
					_vm->_gfx->drawPageRleBlock(screen, kMirrorPaths[1], pos);
				}
			}
		}
	}
	for (int i = 0; i < _cellCount; i++)
		drawCell(screen, _cells[i], true);
	for (int i = 0; i < _ballsLeft; i++) {
		if (i == _ballsLeft - 1 && _loadedRunner < 0 && !_shotConsumed)
			continue;
		_vm->_gfx->drawPageRleBlock(screen, kBallPaths[0], _railPositions[i]);
	}
	_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kCannonFormat, 8 - _angle), Common::Point32(385, 465));
	_vm->_gfx->drawPageRleBlock(screen, kOverlayPaths[0], Common::Point32(475, 431));
	if (_shotPhase == ShotPhase::kLoading00 && _loadedRunner < 0)
		_vm->_gfx->drawPageRleBlock(screen, kBallPaths[0], _projectilePos);
	if (_shotPhase == ShotPhase::kOutbound03 || _shotPhase == ShotPhase::kRebound04) {
		if (_loadedRunner < 0) {
			_vm->_gfx->drawPageRleBlock(screen, kBallPaths[_projectileImage], _projectilePos);
		} else {
			_projectileRunner.setPosition(_projectilePos);
			_vm->_gfx->drawZoombiniRunner(screen, &_projectileRunner);
		}
	}
}

void PuzzleWallOfFleens::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleWallOfFleens::onRenderForeground(ManagedSurface32 *screen) {
	if (_mirrorPhase == MirrorPhase::kShouting03 || _mirrorPhase == MirrorPhase::kLeaving04)
		_vm->_gfx->drawZoombiniRunner(screen, &_fleensRunner);
	if (_angle == 4)
		_vm->_gfx->drawPageRleBlock(screen, kOverlayPaths[1], Common::Point32(449, 465));
}

void PuzzleWallOfFleens::onActorsRendered() {
	for (uint i = 0; i < _puzzleZoombinis.size(); i++)
		_puzzleZoombinis[i]->advanceAnimationAfterDraw();
	_fleensRunner.advanceAnimationAfterDraw();
	_projectileRunner.advanceAnimationAfterDraw();
}

Common::String PuzzleWallOfFleens::debugGetAnswer() const {
	Common::String answer = debugAnswerHeader();
	answer += "\n";
	if (_level == 1)
		answer += Common::String::format("  Panel %d of 6.\n", _panel + 1);
	answer += "  Target mirrors (rows from top, columns from left):\n";
	for (int target = 0; target < (_level == 4 ? 2 : 1); target++) {
		const int index = target == 0 ? _target : _alternateTarget;
		const Cell &cell = _cells[index];
		const char *status = cell.empty ? "caught" : cell.revealed ? "revealed" : "hidden";
		answer += Common::String::format("    %s: row %d, column %d, near (%d, %d)\n", target == 0 ? "Primary" : "Alternate",
										 index / _columns + 1, index % _columns + 1, cell.pos.x, cell.pos.y);
		answer += Common::String::format("      Appearance: %s; status: %s\n", cell.traits.toStr().c_str(), status);
	}
	return answer;
}

Common::String PuzzleWallOfFleens::debugGetChanceDetails() const {
	return Common::String::format("Cannonballs: %d. Party: %u/%d. After cannonballs run out, shots consume Zoombinis down to the last survivor.\n",
								  _ballsLeft, _puzzleZoombinis.size(), _initialPartyCount);
}

void PuzzleWallOfFleens::applyDebugPuzzleCompletion() {
	_perfectClearEligible = true;
	_finished = true;
	_shotPhase = ShotPhase::kStopped05;
	_mirrorPhase = MirrorPhase::kNone00;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		_puzzleZoombinis[i]->_puzzleStatus = 1;
		_puzzleZoombinis[i]->_hidden = false;
	}
	_vm->_zoombiniWalkingFlag = !_puzzleZoombinis.empty();
	_vm->restartGoBlink();
}

bool PuzzleWallOfFleens::canUseGoButton() const {
	return _vm->_zoombiniWalkingFlag;
}

bool PuzzleWallOfFleens::onGoButtonPressed() {
	if (!_vm->_isSavedGame)
		return true;
	if (_goTransitionPending)
		return false;
	const int lost = _initialPartyCount - static_cast<int>(_puzzleZoombinis.size());
	if (lost == 0)
		queueSpeech(Common::String::format(kPerfectGoFormat, _vm->_rnd->getRandomNumber(4) + 1));
	else if (4 <= lost)
		queueSpeech(kRetreatSpeechPaths[1]);
	else
		return true;
	_goTransitionPending = true;
	return false;
}

constexpr byte PuzzleWallOfFleens::kL1Patterns[123][5][4];

} // namespace Zoombini2
