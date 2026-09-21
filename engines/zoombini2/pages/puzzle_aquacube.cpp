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

#include "zoombini2/pages/puzzle_aquacube.h"
#include "common/debug.h"
#include "zoombini2/graphics.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"
#include <cmath>

namespace Zoombini2 {

constexpr const char *PuzzleAquacube::kMusicPath;
constexpr const char *PuzzleAquacube::kLightPath;
constexpr const char *PuzzleAquacube::kBallPaths[2];
constexpr const char *PuzzleAquacube::kCubeFormat;
constexpr const char *PuzzleAquacube::kLeverPaths[2];
constexpr const char *PuzzleAquacube::kIndicatorPaths[2];
constexpr const char *PuzzleAquacube::kShotPaths[2];
constexpr const char *PuzzleAquacube::kWarpPaths[3];
constexpr const char *PuzzleAquacube::kTimerEmptyPath;
constexpr const char *PuzzleAquacube::kTimerPath;
constexpr const char *PuzzleAquacube::kFlareFormat;
constexpr const char *PuzzleAquacube::kBubbleFormat;
constexpr const char *PuzzleAquacube::kFleenFormat;
constexpr const char *PuzzleAquacube::kAngryFormat;
constexpr const char *PuzzleAquacube::kChaseFormat;
constexpr const char *PuzzleAquacube::kSmallestPath;
constexpr const char *PuzzleAquacube::kIdlePath;
constexpr const char *PuzzleAquacube::kSoundPaths[5];
constexpr const char *PuzzleAquacube::kPraisePaths[2];
constexpr const char *PuzzleAquacube::kRetreatPath;

constexpr PuzzleAquacube::NodeLayout PuzzleAquacube::kEasyNodes[8];
constexpr PuzzleAquacube::NodeLayout PuzzleAquacube::kHardNodes[16];
constexpr Common::Point32 PuzzleAquacube::kRescuePositions[16];
constexpr Common::Point32 PuzzleAquacube::kLeverPositions[4];
constexpr Common::Point32 PuzzleAquacube::kIndicatorPositions[4];
constexpr int PuzzleAquacube::kShotX[11];

PuzzleAquacube::PuzzleAquacube(Zoombini2Engine *vm) : PuzzleBase(vm, kPageAquacube) {}

PuzzleAquacube::~PuzzleAquacube() {
	delete _ballPath;
	delete _chasePath;
	delete _timer;
	for (AnimationRunner *runner : _flare)
		delete runner;
	for (AnimationRunner *runner : _angry)
		delete runner;
	for (AnimationRunner *runner : _chase)
		delete runner;
	for (Animation *animation : _animations)
		delete animation;
	for (ZoombiniRunner *actor : _puzzleZoombinis)
		actor->clearMovement();
	if (SoundManager *sound = _vm->getSoundManager()) {
		for (int handle : _sounds)
			if (0 <= handle)
				sound->unload(handle);
		if (0 <= _praiseSpeech)
			sound->unload(_praiseSpeech);
		if (0 <= _goSpeech)
			sound->unload(_goSpeech);
	}
	finishPuzzleRoster(nullptr);
}

void PuzzleAquacube::init() {
	PuzzleBase::init();
	_level = CLIP(_puzzleLevel, 1, 4);
	_numNodes = _level < 3 ? 8 : 16;
	_dimensions = _level < 3 ? 3 : 4;
	_maxSteps = _level < 3 ? 6 : 11;
	_warpQuota = MIN(_level - 1, 2);
	loadResources();
	setupBoard();
	_lastTick = _vm->getGameTickCount();
	startPageMusic(Common::Path(kMusicPath));
	debug(1, "AquaCube: level=%d nodes=%d party=%u start=%d warps=%d", _level, _numNodes, _puzzleZoombinis.size(), _ballNode, _warpQuota);
}

AnimationRunner *PuzzleAquacube::loadRunner(const Common::Path &path, int frames, int repeats, uint32 delay) {
	Animation *animation = new Animation(_vm);
	animation->loadFromFile(path);
	_animations.push_back(animation);
	AnimationRunner *runner = new AnimationRunner(_vm, Common::Point32(), AnimationRunnerMode::kPlayOnce00);
	runner->setAnimation(animation);
	for (int repeat = 0; repeat < repeats; repeat++)
		for (int frame = 0; frame < frames; frame++)
			runner->addTimedFrame(frame, delay);
	return runner;
}

void PuzzleAquacube::loadResources() {
	Gfx *gfx = _vm->_gfx;
	gfx->loadPageRleBlock(kLightPath);
	gfx->loadPageRleBlock(kBallPaths[_level < 3 ? 0 : 1]);
	for (int i = 0; i < 3; i++) {
		gfx->loadPageRleBlock(Common::String::format(kCubeFormat, _level < 3 ? "easy" : "hard", i + 1));
		gfx->loadPageRleBlock(kWarpPaths[i]);
		gfx->loadPageRleBlock(Common::String::format(kBubbleFormat, i + 1));
	}
	for (int i = 0; i < 2; i++) {
		gfx->loadPageRleBlock(kLeverPaths[i]);
		gfx->loadPageRleBlock(kIndicatorPaths[i]);
		gfx->loadPageRleBlock(kShotPaths[i]);
		_flare[i] = loadRunner(Common::Path(Common::String::format(kFlareFormat, i + 1)), 8, 1, 80);
	}
	_flare[0]->setCompletionCallback(&onFlareComplete, this);
	gfx->loadPageRleBlock(kTimerEmptyPath);
	_timer = loadRunner(Common::Path(kTimerPath), 7, 1, 857);
	_timer->setCompletionCallback(&onTimerComplete, this);
	for (int i = 0; i < 4; i++) {
		gfx->loadPageRleBlock(Common::String::format(kFleenFormat, i + 1));
		_angry[i] = loadRunner(Common::Path(Common::String::format(kAngryFormat, i + 1)), 11, 2, 80);
		_angry[i]->setCompletionCallback(&onAngryComplete, this);
		_chase[i] = loadRunner(Common::Path(Common::String::format(kChaseFormat, i + 1)), 11, 2, 80);
	}
	_smallest = _vm->loadZoombiniAnimation(Common::Path(kSmallestPath), 10);
	_idle = _vm->loadZoombiniAnimation(Common::Path(kIdlePath), 50);
	if (SoundManager *sound = _vm->getSoundManager())
		for (int i = 0; i < 5; i++)
			_sounds[i] = sound->load(true, Common::Path(kSoundPaths[i]), false);
}

int PuzzleAquacube::findNode(int coordinates) const {
	for (int i = 0; i < _numNodes; i++)
		if (_nodes[i].coordinates == coordinates)
			return i;
	error("AquaCube: missing generated coordinate %d", coordinates);
}

void PuzzleAquacube::putFleen(int coordinates, int type) {
	Node &node = _nodes[findNode(coordinates)];
	node.state = kFleen03;
	node.fleenType = type;
}

void PuzzleAquacube::setupBoard() {
	bool used[4] = {};
	int flips[4] = {};
	for (int button = 0; button < _dimensions; button++) {
		int axis;
		do {
			axis = _vm->_rnd->getRandomNumber(_dimensions - 1);
		} while (used[axis]);
		used[axis] = true;
		_axisMap[button] = axis;
		flips[axis] = _vm->_rnd->getRandomNumber(1);
	}
	const NodeLayout *layout = _level < 3 ? kEasyNodes : kHardNodes;
	for (int i = 0; i < _numNodes; i++) {
		Node &node = _nodes[i];
		node = Node();
		node.pos = Common::Point32(layout[i].x, layout[i].y);
		for (int axis = 0; axis < 4; axis++)
			node.adj[axis] = layout[i].adj[axis];
		for (int axis = 0; axis < _dimensions; axis++) {
			const char label = layout[i].labels[axis];
			const bool first = label == 'U' || label == 'L' || label == 'F' || label == 'X';
			node.coordinates = node.coordinates * 2 + (first ? flips[axis] : 1 - flips[axis]);
		}
	}
	int start = 0;
	if (_level < 3) {
		putFleen(7, 1);
		if (_level == 2)
			start = 1 << _vm->_rnd->getRandomNumber(2);
	} else {
		putFleen(15, 1);
		const int single = 1 << _vm->_rnd->getRandomNumber(3);
		putFleen(15 ^ single, 2);
		putFleen(single, 3);
		static constexpr int candidates[8] = {
			14,
			13,
			11,
			7,
			1,
			2,
			4,
			8,
		};
		int extra;
		do {
			extra = candidates[_vm->_rnd->getRandomNumber(7)];
		} while (_nodes[findNode(extra)].state == kFleen03);
		putFleen(extra, 4);
		if (_level == 4) {
			do {
				start = 1 << _vm->_rnd->getRandomNumber(3);
			} while (_nodes[findNode(start)].state == kFleen03);
		}
	}
	_ballNode = findNode(start);
	_nodes[_ballNode].state = kStart02;
	_ballPos = _nodes[_ballNode].pos;
	_pendingInitialArrival = _level == 4;
	const Common::Point32 offset = _level < 3 ? Common::Point32(-2, 6) : Common::Point32(-5, 5);
	int slot = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		while (_nodes[slot].state != kEmpty01 && _nodes[slot].state != kOccupied00)
			slot = (slot + 1) % _numNodes;
		Node &node = _nodes[slot];
		node.state = kOccupied00;
		node.occupants[node.occupantCount] = i;
		ZoombiniRunner *actor = _puzzleZoombinis[i];
		actor->clearMovement();
		actor->_savedAnimation = _zoombiniAnimation;
		actor->resetAnimation();
		actor->_activeAnimation = _smallest;
		actor->_hidden = false;
		actor->_inputEnabled = false;
		actor->_puzzleStatus = 0;
		actor->_exitComplete = false;
		actor->setPosition(node.pos + offset + Common::Point32(6 * (3 - node.occupantCount), 0));
		node.occupantCount += 1;
		slot = (slot + 1) % _numNodes;
	}
}

PathObject *PuzzleAquacube::makePath(const Common::Point32 &start, const Common::Point32 &end, int speed) {
	const Common::Point32 third((end.x - start.x) / 3, (end.y - start.y) / 3);
	PathObject *path = new PathObject(_vm);
	path->appendSegment(start, start + third, end - third, end, speed, 0);
	path->start(_vm->getGameTickCount());
	return path;
}

void PuzzleAquacube::playSound(int index) {
	if (SoundManager *sound = _vm->getSoundManager())
		if (0 <= _sounds[index])
			sound->playWithVolume(_sounds[index], sound->_volumeSFX);
}

void PuzzleAquacube::moveBall(int axis) {
	const int next = _nodes[_ballNode].adj[axis];
	_ballPath = makePath(_nodes[_ballNode].pos, _nodes[next].pos, 7);
	_ballNode = next;
	playSound(3);
}

void PuzzleAquacube::finishPuzzle() {
	if (_finished)
		return;
	_finished = true;
	if (_freedCount) {
		_vm->_zoombiniWalkingFlag = true;
		if (SoundManager *sound = _vm->getSoundManager()) {
			const bool allRescued = _freedCount == static_cast<int>(_puzzleZoombinis.size());
			_praiseSpeech = sound->load(true, Common::Path(kPraisePaths[allRescued ? 1 : 0]), false);
			if (0 <= _praiseSpeech)
				sound->playWithVolume(_praiseSpeech, sound->_volumeSpeech);
		}
	}
	_vm->restartGoBlink();
}

void PuzzleAquacube::resolveArrival() {
	playSound(3);
	_stepsUsed += 1;
	for (bool &on : _leverOn)
		on = false;
	// The last move ends the puzzle before its destination is resolved.
	if (_stepsUsed == _maxSteps)
		finishPuzzle();
	Node &node = _nodes[_ballNode];
	if (node.state == kOccupied00) {
		_rescued.clear();
		for (int i = 0; i < node.occupantCount; i++) {
			const int index = node.occupants[i];
			ZoombiniRunner *actor = _puzzleZoombinis[index];
			actor->setPosition(kRescuePositions[_freedCount]);
			_freedCount += 1;
			actor->_puzzleStatus = 1;
			actor->_hidden = true;
			actor->_exitComplete = true;
			_vm->_zoombiniWalkingFlag = true;
			_rescued.push_back(index);
			_flare[0]->startAt(actor->_screenPos - Common::Point32(90, 90), _vm->getGameTickCount());
			playSound(1);
		}
		node.occupantCount = 0;
	} else if (node.state == kFleen03) {
		_vm->_zoombiniWalkingFlag = false;
		node.state = kEmpty01;
		_fleenIndex = node.fleenType - 1;
		_angry[_fleenIndex]->startAt(Common::Point32(573, 67), _vm->getGameTickCount());
		playSound(1);
	}
	debug(1, "AquaCube arrival: node=%d steps=%d rescued=%d warps=%d finished=%d", _ballNode, _stepsUsed, _freedCount, _warpsUsed, _finished);
}

void PuzzleAquacube::onFlareComplete(void *context, AnimationRunner *runner) {
	PuzzleAquacube *page = static_cast<PuzzleAquacube *>(context);
	for (int index : page->_rescued)
		page->_puzzleZoombinis[index]->_hidden = false;
	page->_flare[1]->startAt(runner->getPosition(), page->_vm->getGameTickCount());
}

void PuzzleAquacube::onTimerComplete(void *context, AnimationRunner *runner) {
	(void)runner;
	static_cast<PuzzleAquacube *>(context)->_warpExecuting = true;
}

void PuzzleAquacube::onAngryComplete(void *context, AnimationRunner *runner) {
	(void)runner;
	static_cast<PuzzleAquacube *>(context)->beginChase();
}

void PuzzleAquacube::beginChase() {
	for (ZoombiniRunner *actor : _puzzleZoombinis) {
		if (actor->_screenPos.y < 100) {
			actor->clearMovement();
			actor->_movementPath = makePath(actor->_screenPos, actor->_screenPos + Common::Point32(150, 0), 3);
			actor->startAnimation(_smallest, 33, _vm->getGameTickCount());
			actor->_tracksMovementDirection = false;
			actor->_puzzleStatus = 0;
			_actorsEscaping = true;
		}
	}
	_vm->_zoombiniWalkingFlag = false;
	_chasePath = makePath(Common::Point32(573, 67), Common::Point32(805, 52), 2);
}

void PuzzleAquacube::updateBubbles(uint32 elapsed) {
	for (Bubble &bubble : _bubbles) {
		if (bubble.active) {
			if (bubble.y < 135) {
				bubble.active = false;
			} else {
				bubble.y -= elapsed * (bubble.type == 0 ? 0.05 : 0.025);
				bubble.x = std::sin(bubble.phase) * 5.0 + bubble.originX;
				bubble.phase += elapsed * 0.0025;
				if (6.283 < bubble.phase)
					bubble.phase = 0;
			}
		} else if (_vm->_rnd->getRandomNumber(599) < 2) {
			bubble.active = true;
			bubble.originX = _vm->_rnd->getRandomNumber(769);
			bubble.type = _vm->_rnd->getRandomNumber(2);
			bubble.y = 600 - 100 * bubble.type;
			while (400 < bubble.y && 600 < bubble.originX)
				bubble.originX = _vm->_rnd->getRandomNumber(769);
			bubble.x = bubble.originX;
			bubble.phase = 0;
		}
	}
}

void PuzzleAquacube::onUpdate() {
	const uint32 tick = _vm->getGameTickCount();
	SoundManager *sound = _vm->getSoundManager();
	if (_goPending && (_goSpeech < 0 || !sound || !sound->isPlaying(_goSpeech))) {
		_vm->_mapTransitionSourcePageId = kPageAquacube;
		_vm->requestPageChange(kPageMapTrans);
		return;
	}
	if (_finished)
		for (ZoombiniRunner *actor : _puzzleZoombinis)
			if (actor->_puzzleStatus == 1 && !actor->_animationActive && _vm->_rnd->getRandomNumber(19) == 1)
				actor->startAnimation(_idle, 33, tick);
	updateBubbles(tick - _lastTick);
	_lastTick = tick;
	if (_pendingInitialArrival) {
		_pendingInitialArrival = false;
		_stepsUsed -= 1;
		resolveArrival();
	}
	if (_chasePath) {
		if (_chasePath->finished) {
			delete _chasePath;
			_chasePath = nullptr;
			_freedCount = 0;
		} else {
			Common::Point32 pos;
			_chasePath->advance(tick, pos);
			if (!_chase[_fleenIndex]->isActive()) {
				_chase[_fleenIndex]->start(tick);
				playSound(4);
			}
			_chase[_fleenIndex]->setPosition(pos);
		}
	}
	_actorsEscaping = false;
	for (ZoombiniRunner *actor : _puzzleZoombinis) {
		if (actor->_movementPath) {
			if (actor->_movementPath->finished) {
				actor->clearMovement();
				actor->_hidden = true;
			} else {
				Common::Point32 pos;
				actor->_movementPath->advance(tick, pos);
				actor->setPosition(pos);
				_actorsEscaping = true;
			}
		}
		actor->updateAnimation(tick);
	}
	if (_ballPath) {
		if (_ballPath->finished) {
			delete _ballPath;
			_ballPath = nullptr;
			if (sound && 0 <= _sounds[3])
				sound->stop(_sounds[3]);
			if (!_warpExecuting)
				resolveArrival();
		} else {
			_ballPath->advance(tick, _ballPos);
		}
	}
	if (!_ballPath && _warpExecuting) {
		int button = 0;
		while (button < _dimensions && !_warpPending[button])
			button += 1;
		if (button < _dimensions) {
			_warpPending[button] = false;
			moveBall(_axisMap[button]);
		} else {
			_warpExecuting = false;
			_warpPlanning = false;
			resolveArrival();
		}
	}
	bool allProcessed = true;
	for (const ZoombiniRunner *actor : _puzzleZoombinis)
		if (!actor->_exitComplete)
			allProcessed = false;
	if (allProcessed)
		finishPuzzle();
}

void PuzzleAquacube::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

void PuzzleAquacube::onRenderContent(ManagedSurface32 *screen) {
	Gfx *gfx = _vm->_gfx;
	const Common::Point32 origin(96, 152);
	const char *cubeKind = _level < 3 ? "easy" : "hard";
	gfx->drawPageRleBlock(screen, Common::String::format(kCubeFormat, cubeKind, 3), origin);
	gfx->drawPageRleBlock(screen, Common::String::format(kCubeFormat, cubeKind, 2), origin);
	if (3 <= _level)
		for (int i = 8; i < 12; i++)
			gfx->drawPageRleBlock(screen, kBallPaths[1], _nodes[i].pos);
	gfx->drawPageRleBlock(screen, Common::String::format(kCubeFormat, cubeKind, 1), origin);
	for (int i = 0; i < _numNodes; i++) {
		if (_level < 3 || i < 8 || 12 <= i)
			gfx->drawPageRleBlock(screen, kBallPaths[_level < 3 ? 0 : 1], _nodes[i].pos);
		if (_nodes[i].state == kFleen03)
			gfx->drawPageRleBlock(screen, Common::String::format(kFleenFormat, _nodes[i].fleenType), _nodes[i].pos + Common::Point32(10, 6));
	}
	for (int i = 0; i < _dimensions; i++) {
		gfx->drawPageRleBlock(screen, kLeverPaths[_leverOn[i] ? 1 : 0], kLeverPositions[i]);
		gfx->drawPageRleBlock(screen, kIndicatorPaths[_leverOn[i] ? 1 : 0], kIndicatorPositions[i]);
	}
	for (int i = 0; i < _maxSteps; i++)
		gfx->drawPageRleBlock(screen, kShotPaths[i < _stepsUsed ? 1 : 0], Common::Point32(kShotX[i], 565));
	if (1 < _level) {
		WarpButtonState warpState = kWarpButtonDisabled02;
		if (_warpPlanning)
			warpState = kWarpButtonOn01;
		else if (_warpsUsed < _warpQuota)
			warpState = kWarpButtonOff00;
		gfx->drawPageRleBlock(screen, kWarpPaths[warpState], Common::Point32(641, 541));
		gfx->drawPageRleBlock(screen, kTimerEmptyPath, Common::Point32(666, 544));
		gfx->drawAndUpdateAnimationRunner(screen, _timer, _vm->getGameTickCount(), 0, ManagedSurface32::kScreenSize.width);
	}
}

void PuzzleAquacube::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleAquacube::onActorsRendered() {
	for (ZoombiniRunner *actor : _puzzleZoombinis)
		actor->advanceAnimationAfterDraw();
}

void PuzzleAquacube::onRenderForeground(ManagedSurface32 *screen) {
	Gfx *gfx = _vm->_gfx;
	for (const Bubble &bubble : _bubbles)
		if (bubble.active)
			gfx->drawPageRleBlock(screen, Common::String::format(kBubbleFormat, bubble.type + 1), Common::Point32(static_cast<int>(bubble.x), static_cast<int>(bubble.y)));
	gfx->drawPageRleBlock(screen, kLightPath, _ballPos - Common::Point32(60, 50));
	const uint32 tick = _vm->getGameTickCount();
	for (AnimationRunner *runner : _angry)
		gfx->drawAndUpdateAnimationRunner(screen, runner, tick, 0, ManagedSurface32::kScreenSize.width);
	for (AnimationRunner *runner : _chase)
		gfx->drawAndUpdateAnimationRunner(screen, runner, tick, 0, ManagedSurface32::kScreenSize.width);
	for (AnimationRunner *runner : _flare)
		gfx->drawAndUpdateAnimationRunner(screen, runner, tick, 0, ManagedSurface32::kScreenSize.width);
}

EventHandleResult PuzzleAquacube::onLButtonUp(const Common::Point &pos) {
	if (_finished || _ballPath || _chasePath || _actorsEscaping)
		return EventHandleResult::kPassthrough;
	for (AnimationRunner *runner : _angry)
		if (runner->isActive())
			return EventHandleResult::kPassthrough;
	const Size32 warpButtonSize = _vm->_gfx->getPageRleBlockSize(kWarpPaths[1]);
	if (!_warpPlanning && _warpsUsed < _warpQuota && 0 < warpButtonSize.width && 0 < warpButtonSize.height &&
		641 < pos.x && pos.x < 641 + warpButtonSize.width && 541 < pos.y && pos.y < 541 + warpButtonSize.height) {
		for (bool &pending : _warpPending)
			pending = false;
		_warpPlanning = true;
		_warpsUsed += 1;
		_timer->startAt(Common::Point32(666, 544), _vm->getGameTickCount());
		playSound(2);
		return EventHandleResult::kConsumed;
	}
	for (int i = 0; i < _dimensions; i++) {
		const Common::Point32 &lever = kLeverPositions[i];
		if (lever.x < pos.x && pos.x < lever.x + 17 && lever.y < pos.y && pos.y < lever.y + 41) {
			playSound(0);
			_leverOn[i] = true;
			if (_warpPlanning)
				_warpPending[i] = true;
			else
				moveBall(_axisMap[i]);
			return EventHandleResult::kConsumed;
		}
	}
	return EventHandleResult::kPassthrough;
}

int PuzzleAquacube::countFreeZoombinis() const {
	int count = 0;
	for (const ZoombiniRunner *actor : _puzzleZoombinis)
		if (actor->_puzzleStatus == 0)
			count += 1;
	return count;
}

bool PuzzleAquacube::canUseGoButton() const {
	return !_goPending && _vm->_zoombiniWalkingFlag;
}

bool PuzzleAquacube::onGoButtonPressed() {
	if (!_vm->_isSavedGame || countFreeZoombinis() < 4)
		return true;
	if (_goPending)
		return false;
	if (SoundManager *sound = _vm->getSoundManager()) {
		_goSpeech = sound->load(true, Common::Path(kRetreatPath), false);
		if (0 <= _goSpeech)
			sound->playWithVolume(_goSpeech, sound->_volumeSpeech);
	}
	_goPending = true;
	return false;
}

Common::String PuzzleAquacube::debugGetAnswer() const {
	Common::String answer = debugAnswerHeader();
	answer += Common::String::format("Ball: node %d. Levers below are numbered left to right.\n", _ballNode + 1);
	for (int i = 0; i < _numNodes; i++) {
		const Node &node = _nodes[i];
		answer += Common::String::format("Node %d (%d,%d): %s", i + 1, node.pos.x, node.pos.y,
										 node.state == kFleen03 ? "FLEEN" : "safe");
		for (int occupant = 0; occupant < node.occupantCount; occupant++)
			answer += Common::String::format("; %s", debugActorDescription(node.occupants[occupant]).c_str());
		answer += "\n";
		for (int lever = 0; lever < _dimensions; lever++)
			answer += Common::String::format("  Lever %d -> node %d\n", lever + 1, node.adj[_axisMap[lever]] + 1);
	}
	answer += "A warp visits selected levers in ascending order and resolves only its final destination, using one move.\n";
	return answer;
}

PuzzleChanceInfo PuzzleAquacube::debugGetChances() const {
	return PuzzleChanceInfo(PuzzleChanceInfo::Type::kSubmit, _maxSteps, _stepsUsed, "completed move");
}

bool PuzzleAquacube::debugCanSetChances() const {
	return !_debugFinishPending && !_finished && !_goPending && !_ballPath && !_warpPlanning && !_actorsEscaping && !_pendingInitialArrival;
}

bool PuzzleAquacube::debugSetChances(int remaining) {
	if (!debugCanSetChances() || remaining < 0 || _maxSteps < remaining)
		return false;
	_stepsUsed = _maxSteps - remaining;
	if (!remaining)
		finishPuzzle();
	return true;
}

Common::String PuzzleAquacube::debugGetChanceDetails() const {
	return Common::String::format("Warps: %d/%d remaining. Rescued: %d. Finished: %s.\n", _warpQuota - _warpsUsed, _warpQuota,
								  _freedCount, _finished ? "yes" : "no");
}

} // End of namespace Zoombini2
