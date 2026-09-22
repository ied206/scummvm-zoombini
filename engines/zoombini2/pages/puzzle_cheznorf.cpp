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

#include "zoombini2/pages/puzzle_cheznorf.h"
#include "common/debug.h"
#include "zoombini2/graphics.h"
#include "zoombini2/random.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleChezNorf::kFoodFormat;
constexpr const char *PuzzleChezNorf::kFoodNames[9];
constexpr const char *PuzzleChezNorf::kSymbolNames[3];
constexpr const char *PuzzleChezNorf::kPanelNames[3];
constexpr const char *PuzzleChezNorf::kPlatePath;
constexpr const char *PuzzleChezNorf::kSmallPlatePath;
constexpr const char *PuzzleChezNorf::kMiniPlatePath;
constexpr const char *PuzzleChezNorf::kHighlightPath;
constexpr const char *PuzzleChezNorf::kNorfPath;
constexpr const char *PuzzleChezNorf::kCapPath;
constexpr const char *PuzzleChezNorf::kNorfAnimationFormat;
constexpr const char *PuzzleChezNorf::kCapAnimationFormat;
constexpr const char *PuzzleChezNorf::kExitFormat;
constexpr const char *PuzzleChezNorf::kWaiterExitPath;
constexpr const char *PuzzleChezNorf::kMusicPath;
constexpr const char *PuzzleChezNorf::kSoundFormat;
constexpr const char *PuzzleChezNorf::kClueFormat;
constexpr const char *PuzzleChezNorf::kFeedbackFormat;
constexpr const char *PuzzleChezNorf::kCompleteSpeechPath;
constexpr const char *PuzzleChezNorf::kRetreatSpeechPath;

constexpr Common::Point32 PuzzleChezNorf::kFoodPositions[9];
constexpr Common::Point32 PuzzleChezNorf::kFoodOffsets[9];

constexpr PuzzleChezNorf::Layout PuzzleChezNorf::kLayouts[12];

PuzzleChezNorf::PuzzleChezNorf(Zoombini2Engine *vm) : PuzzleBase(vm, kPageChezNorf) {}

PuzzleChezNorf::~PuzzleChezNorf() {
	clearSelection();
	_vm->setHoverCursorActive(false);
	delete _trayPath;
	for (int motion = 0; motion < 7; motion++) {
		delete _bodyAnimations[motion];
		for (int table = 0; table < _tableCount; table++)
			delete _capAnimations[table][motion];
	}
	if (SoundManager *sound = _vm->getSoundManager()) {
		sound->unload(_clueSpeechSound);
		for (int id : _sounds)
			sound->unload(id);
	}
	for (ZoombiniRunner *runner : _puzzleZoombinis) {
		runner->clearMovement();
		runner->resetAnimation();
	}
	finishPuzzleRoster(_vm->_state->_rescue1Storage);
}

Common::Point32 PuzzleChezNorf::tablePosition(int index, int y) {
	return Common::Point32(205 + 85 * index, y);
}

void PuzzleChezNorf::init() {
	PuzzleBase::init();
	_level = CLIP(_puzzleLevel, 1, 3);
	_tableCount = _level == 3 ? 6 : 4;
	_traySupply = 6 - _level;
	_remaining = static_cast<int>(_puzzleZoombinis.size());
	generateRules();
	loadResources();
	startPageMusic(Common::Path(kMusicPath));
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *runner = _puzzleZoombinis[i];
		runner->clearMovement();
		runner->setDefaultAnimation(_zoombiniAnimation);
		runner->resetAnimation();
		runner->setPosition(Common::Point32(100 + 50 * i, i == 0 ? 530 : 610));
		runner->_inputEnabled = false;
		runner->_hidden = i != 0;
		runner->_puzzleStatus = 0;
	}
	debug(1, "ChezNorf: level=%d layout=%d tables=%d", _level, _layout, _tableCount);
	for (int i = 0; i < _tableCount; i++) {
		debug(2, "ChezNorf: norf=%d answer=%d,%d,%d clue=%d,%d,%d", i,
			  _norfs[i].answer.food[0], _norfs[i].answer.food[1], _norfs[i].answer.food[2],
			  _norfs[i].clue[0], _norfs[i].clue[1], _norfs[i].clue[2]);
	}
}

void PuzzleChezNorf::generateRules() {
	const int variant = _vm->_rnd->getRandomNumber(3);
	_layout = 10 * _level + variant + 1;
	for (int category = 0; category < 3; category++) {
		const int first = category * 3;
		_foodValues[first] = first + _vm->_rnd->getRandomNumber(2);
		do {
			_foodValues[first + 1] = first + _vm->_rnd->getRandomNumber(2);
		} while (_foodValues[first + 1] == _foodValues[first]);
		do {
			_foodValues[first + 2] = first + _vm->_rnd->getRandomNumber(2);
		} while (_foodValues[first + 2] == _foodValues[first] || _foodValues[first + 2] == _foodValues[first + 1]);
	}
	_foodValues[9] = 9;
	const Layout &layout = kLayouts[4 * (_level - 1) + variant];
	for (int i = 0; i < _tableCount; i++) {
		for (int course = 0; course < 3; course++) {
			_norfs[i].answer.food[course] = _foodValues[layout.answers[i][course]];
			_norfs[i].clue[course] = _foodValues[layout.clues[i][course]];
		}
		for (int j = 0; j < 2; j++)
			_norfs[i].gesture[j] = layout.gestures[i][j];
	}
}

void PuzzleChezNorf::loadResources() {
	Gfx *gfx = _vm->_gfx;
	for (int i = 0; i < 9; i++)
		gfx->loadPageRleBlock(Common::String::format(kFoodFormat, kFoodNames[i]));
	for (int i = 0; i < 3; i++)
		gfx->loadPageRleBlock(Common::String::format(kFoodFormat, kSymbolNames[i]));
	gfx->loadPageRleBlock(Common::String::format(kFoodFormat, kPanelNames[_level - 1]));
	gfx->loadPageRleBlock(_level == 1 ? kSmallPlatePath : kPlatePath);
	gfx->loadPageRleBlock(kMiniPlatePath);
	gfx->loadPageRleBlock(kHighlightPath);
	gfx->loadPageRleBlock(kNorfPath);
	static constexpr int kMotionResources[7] = {
		3,
		32,
		5,
		4,
		1,
		6,
		7,
	};
	static constexpr int kFrameCounts[7] = {
		14,
		5,
		20,
		20,
		20,
		20,
		20,
	};
	PageLayer *layer = gfx->getPageLayerStack()->getLayer(1);
	for (int motion = 0; motion < 7; motion++) {
		_bodyAnimations[motion] = new Animation(_vm);
		_bodyAnimations[motion]->loadFromFile(Common::Path(Common::String::format(kNorfAnimationFormat, kMotionResources[motion])));
		AnimationRunner *runner = layer->createAnimationRunner(Common::Point32(), AnimationRunnerMode::kPlayOnce00);
		_bodyRunners[motion] = runner;
		runner->setAnimation(_bodyAnimations[motion]);
		for (int frame = 0; frame < kFrameCounts[motion]; frame++) {
			int image = frame;
			if (motion == kIdle04 && 10 <= frame)
				image = 19 - frame;
			runner->addTimedFrame(image, 100);
		}
		runner->setCompletionCallback(motionComplete, this);
	}
	for (int table = 0; table < _tableCount; table++) {
		gfx->loadPageRleBlock(Common::String::format(kCapPath, table + 1));
		for (int motion = 0; motion < 7; motion++) {
			_capAnimations[table][motion] = new Animation(_vm);
			_capAnimations[table][motion]->loadFromFile(Common::Path(Common::String::format(kCapAnimationFormat, table + 1, kMotionResources[motion])));
			AnimationRunner *runner = layer->createAnimationRunner(tablePosition(table, 205), AnimationRunnerMode::kPlayOnce00);
			_capRunners[table][motion] = runner;
			runner->setAnimation(_capAnimations[table][motion]);
			for (int frame = 0; frame < kFrameCounts[motion]; frame++)
				runner->addTimedFrame(frame, 100);
		}
	}
	static constexpr int kSoundNumbers[8] = {
		1,
		2,
		3,
		4,
		5,
		6,
		7,
		10,
	};
	if (SoundManager *sound = _vm->getSoundManager()) {
		for (int i = 0; i < 8; i++)
			_sounds[i] = sound->load(false, Common::Path(Common::String::format(kSoundFormat, kSoundNumbers[i])), false);
	}
}

bool PuzzleChezNorf::inside(const Common::Point32 &pos, int x, int y, int width, int height) {
	return x < pos.x && pos.x < x + width && y < pos.y && pos.y < y + height;
}

int PuzzleChezNorf::foodAt(const Common::Point32 &pos) const {
	for (int i = 0; i < (_level == 1 ? 6 : 9); i++) {
		const Size32 size = _vm->_gfx->getPageRleBlockSize(Common::String::format(kFoodFormat, kFoodNames[i]));
		if (size.width && size.height && inside(pos, kFoodPositions[i].x, kFoodPositions[i].y, size.width, size.height))
			return i;
	}
	return -1;
}

int PuzzleChezNorf::trayAt(const Common::Point32 &pos) const {
	const Size32 size = _vm->_gfx->getPageRleBlockSize(_level == 1 ? kSmallPlatePath : kPlatePath);
	if (!size.width || !size.height)
		return -1;
	for (int i = 0; i < _tableCount; i++) {
		if (_trayAvailable[i] && inside(pos, tablePosition(i, 500).x, 500, size.width, size.height))
			return i;
	}
	return -1;
}

int PuzzleChezNorf::norfAt(const Common::Point32 &pos) const {
	for (int i = 0; i < _tableCount; i++) {
		if (inside(pos, tablePosition(i, 205).x, 205, 85, 82))
			return i;
	}
	return -1;
}

bool PuzzleChezNorf::mealComplete(const Meal &meal) const {
	return 0 <= meal.food[0] && 0 <= meal.food[1] && (_level == 1 || 0 <= meal.food[2]);
}

bool PuzzleChezNorf::canSubmitTo(int table) const {
	if (_phase != kReady00 || _departAfterSpeech || _releasePending || _dismissPending)
		return false;
	return 0 <= _selectedTray && 0 <= table && table < _tableCount && !speechPlaying() && !motionActive(false) && !_norfs[table].served;
}

bool PuzzleChezNorf::motionActive(bool includeIdle) const {
	for (int motion = 0; motion < 7; motion++) {
		if ((includeIdle || motion != kIdle04) && _bodyRunners[motion] && _bodyRunners[motion]->isActive())
			return true;
	}
	return false;
}

bool PuzzleChezNorf::speechPlaying() const {
	SoundManager *sound = _vm->getSoundManager();
	return sound && (sound->isPlaying(_clueSpeechSound) || sound->hasPendingSpeech());
}

void PuzzleChezNorf::playSound(int index) {
	if (SoundManager *sound = _vm->getSoundManager())
		sound->play(_sounds[index]);
}

void PuzzleChezNorf::playClue(const Common::Path &path) {
	if (SoundManager *sound = _vm->getSoundManager()) {
		sound->unload(_clueSpeechSound);
		_clueSpeechSound = sound->load(true, path, false);
		sound->playWithVolume(_clueSpeechSound, sound->_volumeSpeech);
	}
}

void PuzzleChezNorf::queueSpeech(const Common::Path &path) {
	if (SoundManager *sound = _vm->getSoundManager())
		sound->queueSpeech(path);
}

void PuzzleChezNorf::sayClue(int index) {
	Common::String path = Common::String::format(kClueFormat, index + (_level == 3 ? 1 : 3), _layout);
	for (int value : _norfs[index].clue) {
		if (value < 9)
			path += Common::String::format("-%c%d", 'A' + value / 3, value % 3 + 1);
	}
	if (_layout == 13 && index == 2)
		path += "-A0";
	if (_layout == 33 && index == 1)
		path += "-C0";
	path += ".wav";
	playClue(Common::Path(path));
	if (_idleNorf == index && _bodyRunners[kIdle04]->isActive()) {
		_bodyRunners[kIdle04]->stop();
		_capRunners[index][kIdle04]->stop();
	}
	const int gesture = _norfs[index].gesture[0];
	_nextGesture = _norfs[index].gesture[1];
	if (gesture == 1 || gesture == 2)
		startMotion(index, gesture == 1 ? kPointRight05 : kPointLeft06);
	debug(2, "ChezNorf: clue %s", path.c_str());
}

void PuzzleChezNorf::clearSelection() {
	_selectedFood = -1;
	_selectedTray = -1;
	_vm->setPageCursorSprite(nullptr);
}

void PuzzleChezNorf::updateCursor() {
	const bool available = _phase != kRelease05 && _phase != kDismiss06 && _phase != kFinished07 && !_departAfterSpeech;
	const int tray = trayAt(_pointer);
	_vm->setHoverCursorActive(available && (0 <= foodAt(_pointer) || 0 <= norfAt(_pointer) || (0 <= tray && mealComplete(_trays[tray]))));
	if (0 <= _selectedFood)
		_vm->setPageCursorSprite(_vm->_gfx->loadPageRleBlock(Common::String::format(kFoodFormat, kFoodNames[_selectedFood])));
	else if (0 <= _selectedTray && _phase == kReady00)
		_vm->setPageCursorSprite(_vm->_gfx->loadPageRleBlock(kMiniPlatePath));
	else
		_vm->setPageCursorSprite(nullptr);
}

EventHandleResult PuzzleChezNorf::onMouseMove(const Common::Point &pos) {
	_pointer = Common::Point32(pos);
	return EventHandleResult::kPassthrough;
}

EventHandleResult PuzzleChezNorf::onLButtonDown(const Common::Point &pos) {
	_pointer = Common::Point32(pos);
	_buttonArmed = true;
	return EventHandleResult::kPassthrough;
}

EventHandleResult PuzzleChezNorf::onLButtonUp(const Common::Point &pos) {
	_pointer = Common::Point32(pos);
	if (_buttonArmed) {
		_click = _pointer;
		_clickPending = true;
		if (canSubmitTo(norfAt(_pointer)))
			_vm->setPageCursorSprite(nullptr);
	}
	_buttonArmed = false;
	return EventHandleResult::kPassthrough;
}

void PuzzleChezNorf::handleClick(const Common::Point32 &pos) {
	if (_phase == kRelease05 || _phase == kDismiss06 || _phase == kFinished07 || _departAfterSpeech)
		return;
	const int norf = norfAt(pos);
	if (_phase == kReady00 && 0 <= norf) {
		if (0 <= _selectedTray) {
			if (canSubmitTo(norf))
				submit(norf);
			return;
		}
		if (_selectedFood == -1 && !_bodyRunners[kReject00]->isActive() && !_bodyRunners[kRejectReturn01]->isActive() &&
			!_bodyRunners[kAccept03]->isActive()) {
			sayClue(norf);
			return;
		}
	}
	const int food = foodAt(pos);
	if (_selectedTray == -1 && 0 <= food) {
		_selectedFood = food;
		static constexpr int kSelectionSounds[3] = {
			3,
			2,
			4,
		};
		playSound(kSelectionSounds[food / 3]);
		return;
	}
	if (_phase == kReady00 && 0 <= _selectedTray && inside(pos, 200, 503, 564, 82)) {
		clearSelection();
		return;
	}
	const int tray = trayAt(pos);
	if (0 <= tray && 0 <= _selectedFood) {
		_trays[tray].food[_selectedFood / 3] = _selectedFood;
		playSound(6);
		clearSelection();
		return;
	}
	if (0 <= tray && _selectedFood == -1 && _selectedTray == -1 && mealComplete(_trays[tray])) {
		_selectedTray = tray;
		return;
	}
	if (0 <= _selectedFood && inside(pos, 29, 399, 167, 131)) {
		clearSelection();
		return;
	}
	static constexpr int kTop[3] = {
		49,
		105,
		162,
	};
	static constexpr int kBottom[3] = {
		97,
		160,
		219,
	};
	static constexpr int kStride[3] = {
		17,
		16,
		19,
	};
	for (int section = 0; section < (_level == 1 ? 2 : 3); section++) {
		if (!inside(pos, 49, kTop[section], 87, kBottom[section] - kTop[section]))
			continue;
		const int column = (pos.x - 49) / 14;
		const int row = (pos.y - kTop[section]) / kStride[section];
		if (column < _tableCount && row < 3) {
			_notes[section][row][column] = (_notes[section][row][column] + 1) % 4;
			playSound(5);
		}
	}
}

void PuzzleChezNorf::startMotion(int table, Motion motion) {
	for (int i = 0; i < 7; i++) {
		if (i == kIdle04 && _idleNorf != table && (motion == kPointRight05 || motion == kPointLeft06))
			continue;
		_bodyRunners[i]->stop();
		for (int j = 0; j < _tableCount; j++)
			_capRunners[j][i]->stop();
	}
	if (motion == kIdle04)
		_idleNorf = table;
	else
		_animatedNorf = table;
	const uint32 now = _vm->getFrameTickCount();
	_bodyRunners[motion]->startAt(tablePosition(table, 205), now);
	_capRunners[table][motion]->startAt(tablePosition(table, 205), now);
}

void PuzzleChezNorf::motionComplete(void *context, AnimationRunner *runner) {
	PuzzleChezNorf *page = static_cast<PuzzleChezNorf *>(context);
	for (int i = 0; i < 7; i++) {
		if (page->_bodyRunners[i] == runner) {
			page->finishMotion(static_cast<Motion>(i));
			return;
		}
	}
}

void PuzzleChezNorf::finishMotion(Motion motion) {
	switch (motion) {
	case kReject00:
		_phase = kRejectFall03;
		_norfs[_recipient].rejectedTray = false;
		playSound(1);
		startMotion(_recipient, kRejectReturn01);
		break;
	case kRejectReturn01:
		_dismissPending = _submissions == 8;
		break;
	case kAccept03:
		_releasePending = true;
		break;
	case kPointRight05:
	case kPointLeft06:
		if (_nextGesture == 1 || _nextGesture == 2) {
			const int next = _nextGesture;
			_nextGesture = 0;
			startMotion(_animatedNorf, next == 1 ? kPointRight05 : kPointLeft06);
		}
		break;
	default:
		break;
	}
}

void PuzzleChezNorf::startTrayPath(const Common::Point32 &from, const Common::Point32 &to) {
	delete _trayPath;
	_trayPath = new PathObject(_vm);
	const int dy = (to.y - from.y) / 3;
	_trayPath->appendSegment(from, Common::Point32(from.x, from.y + dy), Common::Point32(to.x, to.y - dy), to, 6, 0);
	_trayPath->start(_vm->getGameTickCount());
	_flyingPosition = from;
}

void PuzzleChezNorf::submit(int table) {
	if (_bodyRunners[kIdle04]->isActive()) {
		_bodyRunners[kIdle04]->stop();
		_capRunners[_idleNorf][kIdle04]->stop();
	}
	_recipient = table;
	_flyingMeal = _trays[_selectedTray];
	startTrayPath(tablePosition(_selectedTray, 500), tablePosition(_selectedTray, -200));
	_submissions += 1;
	_traySupply -= 1;
	if (_traySupply <= 0)
		_trayAvailable[_selectedTray] = false;
	playSound(0);
	_phase = kThrow01;
	_vm->setPageCursorSprite(nullptr);
	debug(1, "ChezNorf: submit=%d source=%d norf=%d meal=%d,%d,%d", _submissions, _selectedTray, table,
		  _flyingMeal.food[0], _flyingMeal.food[1], _flyingMeal.food[2]);
}

void PuzzleChezNorf::checkMeal() {
	Norf &norf = _norfs[_recipient];
	bool accepted = true;
	for (int course = 0; course < 3; course++) {
		if (_flyingMeal.food[course] != norf.answer.food[course] &&
			!(course == 2 && _flyingMeal.food[course] == -1 && norf.answer.food[course] == 9))
			accepted = false;
	}
	_trays[_selectedTray] = Meal();
	clearSelection();
	_phase = kFeedback04;
	if (accepted) {
		norf.served = true;
		norf.acceptedTray = true;
		norf.accepted = _flyingMeal;
		startMotion(_recipient, kAccept03);
	} else {
		startMotion(_recipient, kReject00);
	}
	int feedback = 1;
	if (accepted)
		feedback = 3;
	else if (_submissions == 8)
		feedback = 2;
	queueSpeech(Common::Path(Common::String::format(kFeedbackFormat, _recipient + 7 - _tableCount, feedback)));
	debug(1, "ChezNorf: result=%s submissions=%d", accepted ? "accepted" : "rejected", _submissions);
}

void PuzzleChezNorf::releaseCohort() {
	const int count = _level == 3 && 2 <= _successCount ? 1 : 2;
	const uint32 now = _vm->getGameTickCount();
	for (int i = 0; i < count && 0 < _remaining; i++) {
		const int index = _remaining - 1;
		const Common::Path path(index == 0 ? Common::String(kWaiterExitPath) : Common::String::format(kExitFormat, i + 1));
		PathObject *movement = PathObject::loadFromPAT(_vm, path);
		if (!movement)
			error("ChezNorf: required exit path is unavailable: %s", path.toString().c_str());
		ZoombiniRunner *runner = _puzzleZoombinis[index];
		runner->_hidden = false;
		runner->_puzzleStatus = 1;
		runner->startMovement(movement, now);
		runner->startDirectionTrackedAnimation(now);
		_remaining -= 1;
	}
	_phase = kRelease05;
	if (_remaining == 0) {
		_vm->restartGoBlink();
		queueSpeech(Common::Path(kCompleteSpeechPath));
	}
	debug(1, "ChezNorf: release ordinal=%d count=%d remaining=%d", _successCount, count, _remaining);
}

void PuzzleChezNorf::dismissWaiter() {
	_phase = kDismiss06;
	startMotion(_recipient, kDismiss02);
	clearSelection();
	if (_puzzleZoombinis.empty())
		return;
	ZoombiniRunner *runner = _puzzleZoombinis[0];
	const Common::Point32 start = runner->_screenPos;
	const int dy = (630 - start.y) / 3;
	PathObject *path = new PathObject(_vm);
	path->appendSegment(start, Common::Point32(start.x, start.y + dy), Common::Point32(start.x, 630 - dy), Common::Point32(start.x, 630), 2, 0);
	runner->startMovement(path, _vm->getGameTickCount());
	runner->startAnimation(_zoombiniAnimation, 22, _vm->getGameTickCount());
	_canDepart = 0 < _successCount;
	if (_canDepart)
		_vm->restartGoBlink();
}

void PuzzleChezNorf::onUpdate() {
	const uint32 now = _vm->getGameTickCount();
	SoundManager *sound = _vm->getSoundManager();
	if (_departAfterSpeech && (!sound || !sound->hasPendingSpeech())) {
		_vm->_returningFromPuzzle = true;
		_vm->_mapTransitionSourcePageId = kPageChezNorf;
		_vm->requestPageChange(kPageMapTrans);
	}
	for (ZoombiniRunner *runner : _puzzleZoombinis)
		runner->updateAnimation(now);
}

void PuzzleChezNorf::onActorsRendered() {
	const uint32 now = _vm->getGameTickCount();
	for (ZoombiniRunner *runner : _puzzleZoombinis)
		runner->advanceAnimationAfterDraw();
	if (_releasePending) {
		_releasePending = false;
		releaseCohort();
	}
	if (_dismissPending && _phase == kReady00) {
		_dismissPending = false;
		dismissWaiter();
	}
	if (_phase == kRelease05 || _phase == kDismiss06) {
		bool moving = false;
		for (ZoombiniRunner *runner : _puzzleZoombinis) {
			if (!runner->_movementPath)
				continue;
			if (runner->_movementPath->finished) {
				runner->clearMovement();
				runner->resetAnimation();
				runner->_hidden = true;
			} else {
				runner->advanceMovement(now, Common::Point32(0, _phase == kRelease05 ? -15 : 0));
				moving = true;
			}
		}
		if (!moving && _phase == kRelease05) {
			_successCount += 1;
			_canDepart = true;
			const bool terminal = _remaining == 0 || _submissions == 8;
			_phase = terminal ? kFinished07 : kReady00;
			if (terminal)
				_vm->restartGoBlink();
		}
	} else {
		if (_phase == kReady00 && !motionActive() && now % 20 == 0) {
			const int table = _vm->_rnd->getRandomNumber(_tableCount - 1);
			if (!_norfs[table].served)
				startMotion(table, kIdle04);
		}
		if (_phase == kRejectFall03) {
			_flyingPosition.y += 20;
			if (360 < _flyingPosition.y) {
				_flyingPosition.y = 360;
				_norfs[_recipient].rejectedTray = true;
				_norfs[_recipient].rejected = _flyingMeal;
				_phase = kReady00;
			}
		}
		if (_trayPath) {
			if (_trayPath->finished) {
				delete _trayPath;
				_trayPath = nullptr;
				if (_phase == kThrow01) {
					_phase = kLand02;
					startTrayPath(tablePosition(_recipient, -300), tablePosition(_recipient, 260));
				} else {
					playSound(7);
					checkMeal();
				}
			} else {
				_trayPath->advance(now, _flyingPosition);
			}
		}
		if (_clickPending)
			handleClick(_click);
	}
	_clickPending = false;
	updateCursor();
}

void PuzzleChezNorf::drawTray(ManagedSurface32 *screen, const Common::Point32 &pos, const Meal &meal) {
	_vm->_gfx->drawPageRleBlock(screen, _level == 1 ? kSmallPlatePath : kPlatePath, pos);
	static constexpr int kDrawOrder[3] = {
		1,
		0,
		2,
	};
	for (int course : kDrawOrder) {
		const int food = meal.food[course];
		if (0 <= food && food < 9)
			_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kFoodFormat, kFoodNames[food]), pos + kFoodOffsets[food]);
	}
}

void PuzzleChezNorf::onRenderContent(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
	_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kFoodFormat, kPanelNames[_level - 1]), Common::Point32());
	for (int i = 0; i < (_level == 1 ? 6 : 9); i++)
		_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kFoodFormat, kFoodNames[i]), kFoodPositions[i]);
	static constexpr int kNoteTop[3] = {
		47,
		104,
		164,
	};
	for (int section = 0; section < (_level == 1 ? 2 : 3); section++) {
		for (int row = 0; row < 3; row++) {
			for (int column = 0; column < _tableCount; column++) {
				const int mark = _notes[section][row][column];
				if (0 < mark)
					_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kFoodFormat, kSymbolNames[mark - 1]),
												Common::Point32(51 + 14 * column, kNoteTop[section] + 15 * row));
			}
		}
	}
	for (int i = 0; i < _tableCount; i++) {
		const bool idle = i == _idleNorf && _bodyRunners[kIdle04]->isActive();
		const bool reacting = i == _animatedNorf && motionActive(false);
		if (!idle && !reacting) {
			_vm->_gfx->drawPageRleBlock(screen, kNorfPath, tablePosition(i, 205));
			_vm->_gfx->drawPageRleBlock(screen, Common::String::format(kCapPath, i + 1), tablePosition(i, 205));
		}
		if (_trayAvailable[i]) {
			if ((_phase == kThrow01 || _phase == kLand02) && _selectedTray == i)
				drawTray(screen, tablePosition(i, 500), Meal());
			else
				drawTray(screen, tablePosition(i, 500), _trays[i]);
		}
		if (_norfs[i].acceptedTray)
			drawTray(screen, tablePosition(i, 260), _norfs[i].accepted);
		if (_norfs[i].rejectedTray)
			drawTray(screen, tablePosition(i, 360), _norfs[i].rejected);
	}
}

void PuzzleChezNorf::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleChezNorf::onRenderForeground(ManagedSurface32 *screen) {
	if (_phase == kThrow01 || _phase == kLand02 || _phase == kRejectFall03 ||
		(_phase == kFeedback04 && !_norfs[_recipient].served))
		drawTray(screen, _flyingPosition, _flyingMeal);
	_vm->_gfx->getPageLayerStack()->getLayer(1)->drawAndUpdate(screen);
	if (0 <= _selectedFood) {
		const int tray = trayAt(_pointer);
		if (0 <= tray) {
			static constexpr int kHighlightOffsets[3][2] = {
				{50, 42},
				{12, 42},
				{26, 58},
			};
			const int course = _selectedFood / 3;
			const Common::Point32 pos = tablePosition(tray, 500) + Common::Point32(kHighlightOffsets[course][0] - 16, kHighlightOffsets[course][1] - 13);
			_vm->_gfx->drawPageRleBlock(screen, kHighlightPath, pos);
		}
	}
	if (_vm->showChezNorfDebugOverlay())
		drawDebugOverlay(screen);
}

void PuzzleChezNorf::drawDebugOverlay(ManagedSurface32 *screen) {
	Gfx *gfx = _vm->_gfx;
	if (!gfx->hasTextFont(Gfx::TextColor::kGreen02) && !gfx->loadTextFont(Gfx::TextColor::kGreen02))
		return;
	static constexpr const char *kLabels[10] = {
		"Swich",
		"Fish",
		"Salad",
		"tea",
		"Ornge",
		"Milk",
		"Pie",
		"Melon",
		"IC",
		"Rien",
	};
	const Common::String header = Common::String::format("%d - %d %d %d - %d %d %d - %d %d %d", _layout,
														 _foodValues[0], _foodValues[1], _foodValues[2], _foodValues[3], _foodValues[4], _foodValues[5], _foodValues[6], _foodValues[7], _foodValues[8]);
	gfx->drawText(screen, Gfx::TextColor::kGreen02, Common::Point32(100, 0), header);
	for (int table = 0; table < _tableCount; table++) {
		int clues = 0;
		for (int course = 0; course < 3; course++) {
			gfx->drawText(screen, Gfx::TextColor::kGreen02, tablePosition(table, 50 + 30 * course), kLabels[_norfs[table].answer.food[course]]);
			if (_norfs[table].clue[course] != 9)
				clues += 1;
		}
		gfx->drawText(screen, Gfx::TextColor::kGreen02, tablePosition(table, 140), Common::String::format("%d", clues));
	}
}

bool PuzzleChezNorf::blocksSidebarInteraction() const {
	return 0 <= _selectedFood || 0 <= _selectedTray || (_phase != kReady00 && _phase != kFinished07 && _phase != kDismiss06) || _departAfterSpeech;
}

bool PuzzleChezNorf::canUseGoButton() const {
	return _canDepart && !blocksSidebarInteraction();
}

bool PuzzleChezNorf::onGoButtonPressed() {
	if (_departAfterSpeech)
		return false;
	if (_vm->_isSavedGame && 4 <= _remaining) {
		queueSpeech(Common::Path(kRetreatSpeechPath));
		_departAfterSpeech = true;
		return false;
	}
	return true;
}

Common::String PuzzleChezNorf::debugGetAnswer() const {
	static constexpr const char *foods[] = {
		"sandwich",
		"fish",
		"salad",
		"coffee",
		"orange juice",
		"milk",
		"pie",
		"watermelon",
		"ice cream",
		"none",
	};
	Common::String answer = debugAnswerHeader();
	for (int i = 0; i < _tableCount; i++) {
		const Norf &norf = _norfs[i];
		const char *servedSuffix = "";
		if (norf.served)
			servedSuffix = " (served)";
		answer += Common::String::format("Norf %d (left to right): %s, %s, %s%s\n", i + 1,
										 foods[norf.answer.food[0]], foods[norf.answer.food[1]], foods[norf.answer.food[2]], servedSuffix);
	}
	return answer;
}

PuzzleChanceInfo PuzzleChezNorf::debugGetChances() const {
	return PuzzleChanceInfo(PuzzleChanceInfo::Type::kSubmit, 8, _submissions, "meal submission");
}

bool PuzzleChezNorf::debugCanSetChances() const {
	return !_debugFinishPending && _phase == kReady00 && !_departAfterSpeech && !_releasePending && !_dismissPending && !motionActive(false);
}

bool PuzzleChezNorf::debugSetChances(int remaining) {
	if (!debugCanSetChances() || remaining < 0 || 8 < remaining)
		return false;
	clearSelection();
	_submissions = 8 - remaining;
	const int available = MIN(_tableCount, remaining);
	_traySupply = remaining - available + 1;
	for (int i = 0; i < _tableCount; i++) {
		_trayAvailable[i] = i < available;
		if (!_trayAvailable[i])
			_trays[i] = Meal();
	}
	if (!remaining) {
		if (_recipient < 0)
			_recipient = 0;
		dismissWaiter();
	}
	return true;
}

Common::String PuzzleChezNorf::debugGetChanceDetails() const {
	int trays = 0;
	for (int i = 0; i < _tableCount; i++)
		if (_trayAvailable[i])
			trays += 1;
	return Common::String::format("Tray positions: %d; spare trays: %d; waiting Zoombinis: %d. Setting chances also synchronizes tray availability.\n",
								  trays, MAX(0, _traySupply - 1), _remaining);
}

void PuzzleChezNorf::applyDebugPuzzleCompletion() {
	_remaining = 0;
	_phase = kFinished07;
	_canDepart = true;
	clearSelection();
	delete _trayPath;
	_trayPath = nullptr;
	for (AnimationRunner *runner : _bodyRunners)
		runner->stop();
	for (int i = 0; i < _tableCount; i++) {
		for (AnimationRunner *runner : _capRunners[i])
			runner->stop();
	}
}

} // End of namespace Zoombini2
