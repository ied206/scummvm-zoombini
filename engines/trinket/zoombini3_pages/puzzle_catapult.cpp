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
 */

#include "trinket/zoombini3_pages/puzzle_catapult.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "trinket/ompp/xml.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace Zoombini3 {

constexpr uint CatapultRules::kDatasetCount;
constexpr uint CatapultRules::kPartyLimit;

CatapultRules::CatapultRules() = default;

bool CatapultRules::configure(uint level, uint partySize, uint dataset, int overrideDataset) {
	if (3 < level || !partySize || kPartyLimit < partySize || kDatasetCount <= dataset ||
		overrideDataset < -1 || static_cast<int>(kDatasetCount) <= overrideDataset)
		return false;
	if (0 <= overrideDataset) {
		dataset = overrideDataset;
	} else if (!level) {
		level = 1;
		dataset = 8;
	}
	static constexpr uint primary[] = {
		0,
		0,
		0,
		0,
		0,
		1,
		1,
		1,
		1,
		2,
		2,
		2,
		2,
	};
	static constexpr int count[] = {
		2,
		3,
		-3,
		-3,
		4,
		3,
		-3,
		-3,
		4,
		2,
		-3,
		-3,
		4,
	};
	static constexpr uint paddle[] = {
		4,
		2,
		4,
		4,
		4,
		3,
		4,
		4,
		1,
		1,
		1,
		1,
		1,
	};
	static constexpr uint drive[] = {
		2,
		1,
		2,
		2,
		2,
		2,
		2,
		2,
		0,
		0,
		0,
		0,
		0,
	};
	_level = level;
	_partySize = partySize;
	_dataset = dataset;
	_primaryFamily = primary[dataset];
	_paddleFamily = level == 1 ? 0 : paddle[dataset];
	_driveFamily = level == 1 ? 0 : drive[dataset];
	_primaryCount = count[dataset] == -3 ? 3 : count[dataset];
	_primaryPhaseDivisor = count[dataset] == -3 ? 4 : _primaryCount;
	return true;
}

uint CatapultRules::primaryStartFrame(uint index, uint frameCount) const {
	static constexpr uint phase[] = {
		6,
		6,
		6,
		0,
		6,
		9,
		8,
		26,
		8,
		10,
		10,
		20,
		10,
	};
	return frameCount / _primaryPhaseDivisor * index + phase[_dataset] + 1;
}

uint CatapultRules::secondaryStartFrame(uint index, uint frameCount) const {
	static constexpr uint phase[] = {
		89,
		0,
		0,
		4,
	};
	uint step = frameCount / 6;
	return step / 2 + step * index + phase[_level] + 1;
}

bool CatapultRules::releasesPrimaryLatch(uint frame) const {
	static constexpr uint lower[] = {
		25,
		55,
		85,
	};
	static constexpr uint upper[] = {
		29,
		59,
		89,
	};
	return lower[_primaryFamily] < frame && frame < upper[_primaryFamily];
}

bool CatapultRules::canReceiveFromChute(uint frame) const {
	static constexpr uint transfer[] = {
		29,
		59,
		89,
	};
	return frame == transfer[_primaryFamily];
}

bool CatapultRules::canReceiveFromPrimary(uint frame) const {
	static constexpr uint lower[] = {
		4,
		154,
		153,
		77,
	};
	static constexpr uint upper[] = {
		77,
		162,
		162,
		84,
	};
	return lower[_level] <= frame && frame <= upper[_level];
}

uint CatapultRules::primaryTransportFrame() const {
	static constexpr uint first[] = {
		1,
		3,
		5,
	};
	return first[_primaryFamily];
}

int CatapultRules::secondaryTransportFrame(uint catcherFrame) const {
	return static_cast<int>(catcherFrame) - (_level == 3 ? 80 : 157) + 1;
}

bool CatapultRules::paddleIsOpen(uint frame, uint frameCount) const {
	static constexpr uint intervals[3][2][5] = {
		{{1, 1, 1, 1, 1}, {14, 6, 32, 37, 3}},
		{{31, 36, 102, 1, 68}, {47, 72, 108, 37, 144}},
		{{65, 36, 102, 1, 68}, {72, 72, 108, 37, 144}}};
	for (uint i = 0; i < ARRAYSIZE(intervals); i++) {
		int first = intervals[i][0][_paddleFamily];
		int last = intervals[i][1][_paddleFamily];
		if (_level != 3) {
			int reversedFirst = static_cast<int>(frameCount) - last + 1;
			last = static_cast<int>(frameCount) - first + 1;
			first = reversedFirst;
		}
		if (first <= static_cast<int>(frame) && static_cast<int>(frame) <= last)
			return true;
	}
	return false;
}

int CatapultRules::paddleHeight() const {
	static constexpr int height[] = {
		275,
		275,
		330,
		325,
		315,
	};
	static constexpr int offset[] = {
		0,
		0,
		-30,
		-15,
		-15,
	};
	return height[_paddleFamily] + (_level == 3 ? 0 : offset[_paddleFamily]);
}

int CatapultRules::flightVelocity() const {
	return _level == 3 ? 90 : 60;
}

bool CatapultRules::chuteCollision(const Common::Rect &first, const Common::Rect &second) {
	if (!first.intersects(second))
		return false;
	int x = first.left + first.width() / 2 - second.left - second.width() / 2;
	int y = first.top + first.height() / 2 - second.top - second.height() / 2;
	int radius = (first.width() + first.height()) / 4 + (second.width() + second.height()) / 4 + 1;
	return x * x + y * y <= radius * radius;
}

int CatapultRules::datasetForDraw(uint draw, const bool *seen) {
	if (!draw || kDatasetCount * 5 < draw)
		return -1;
	for (uint i = 0; i < kDatasetCount; i++) {
		uint boundary = 5 * (i + 1);
		if ((draw < boundary && !seen[i]) || draw == boundary)
			return i;
	}
	return -1;
}

CatapultMotion::CatapultMotion() = default;

void CatapultMotion::startWalk(int xVelocity, int yVelocity) {
	_walking = true;
	_walkX = xVelocity;
	_walkY = yVelocity;
}

void CatapultMotion::startBounce(int velocity, int acceleration, int decay, const Common::Rect &position, uint32 time) {
	_bouncing = true;
	_bounceVelocity = velocity;
	_currentBounceVelocity = position.bottom < _bounds.bottom ? 0 : velocity;
	_acceleration = acceleration;
	_decay = decay;
	_bounceTime = time;
	_bounceOriginY = position.top;
	_previousBounceY = position.top;
}

void CatapultMotion::pause(uint32 time) {
	_paused = true;
	_pauseTime = time;
}

void CatapultMotion::resume(uint32 time) {
	_lastUpdate = time;
	if (_bouncing)
		_bounceTime += static_cast<double>(time) - _pauseTime;
	_paused = false;
}

void CatapultMotion::update(Common::Rect &position, uint32 time, Listener &listener) {
	uint32 elapsed = _lastUpdate ? time - _lastUpdate : 8;
	if (!elapsed)
		return;
	_lastUpdate = time;
	if (_paused || (!_walking && !_bouncing))
		return;
	double rawDelta = elapsed * 0.001;
	double total = rawDelta;
	for (uint i = ARRAYSIZE(_samples) - 1; 0 < i; i--) {
		_samples[i] = _samples[i - 1] == 0 ? rawDelta : _samples[i - 1];
		total += _samples[i];
	}
	_samples[0] = rawDelta;
	double delta = total * 0.1;
	if (0.5 < delta) {
		delta = 0.5;
		if (_bouncing)
			startBounce(_bounceVelocity, _acceleration, _decay, position, time);
	}
	double dx = _remainderX;
	double dy = _remainderY;
	if (_bouncing) {
		double seconds = (static_cast<double>(time) - _bounceTime) * 0.001;
		double y = _bounceOriginY - _currentBounceVelocity * seconds + _acceleration * seconds * seconds;
		dy += y - _previousBounceY;
		_previousBounceY = y;
	}
	if (_walking) {
		dx += _walkX * delta;
		dy += _walkY * delta;
	}
	int moveX = static_cast<int>(dx);
	int moveY = static_cast<int>(dy);
	_remainderX = dx - moveX;
	_remainderY = dy - moveY;
	position.translate(moveX, moveY);
	if (_bouncing && _bounds.bottom <= position.bottom) {
		position.translate(0, _bounds.bottom - position.bottom);
		_bounceVelocity -= _decay;
		if (_bounceVelocity < 0) {
			_bounceVelocity = 0;
			_bouncing = false;
			listener.motionBoundary(*this, position, kBounceStopped, time);
		} else {
			listener.motionBoundary(*this, position, kBounce, time);
		}
		_currentBounceVelocity = _bounceVelocity;
		_bounceTime = time;
		_bounceOriginY = position.top;
		_previousBounceY = position.top;
	}
	if (_walking) {
		// Horizontal callbacks may change bounds; vertical comparisons use the saved position.
		Common::Rect moved = position;
		if (_bounds.right < moved.right) {
			position.translate(_bounds.right - position.right, 0);
			listener.motionBoundary(*this, position, kRight, time);
		} else if (moved.left < _bounds.left) {
			position.translate(_bounds.left - position.left, 0);
			listener.motionBoundary(*this, position, kLeft, time);
		}
		if (_bounds.bottom < moved.bottom) {
			position.translate(0, _bounds.bottom - position.bottom);
			listener.motionBoundary(*this, position, kBottom, time);
		} else if (moved.top < _bounds.top) {
			position.translate(0, _bounds.top - position.top);
			listener.motionBoundary(*this, position, kTop, time);
		}
	}
	_pauseTime = time;
}

Common::String CatapultMachine::debugAnswer() const {
	Common::String result = Common::String::format("Catapult: level %u, dataset %u, machine %s\n",
												   _rules.level(), _rules.dataset(), machineRunning() ? "running" : "stopped");
	for (uint i = 0; i < _rules.primaryCount(); i++) {
		uint frame = _primary[i]->getFrame();
		result += Common::String::format("Primary %u: frame %u, chute entry %s, transport frame %u\n",
										 i + 1, frame, _rules.canReceiveFromChute(frame) ? "open" : "closed", _rules.primaryTransportFrame());
	}
	if (_paddle)
		result += Common::String::format("Paddle: frame %u/%u, landing %s\n", _paddle->getFrame(), _paddle->getNumberOfFrames(),
										 _rules.paddleIsOpen(_paddle->getFrame(), _paddle->getNumberOfFrames()) ? "open" : "closed");
	result += "Timing puzzle: test the current cycle with unlimited mud; boulders move the party. Frame values are a snapshot.\n";
	return result;
}

Common::String CatapultMachine::debugChance() const {
	uint supply = 0;
	for (uint i = 0; i < _objects.size(); i++)
		if (_objects[i]->rock && _objects[i]->stage == kSupply && _objects[i]->sprite->enabled)
			supply += 1;
	return Common::String::format("Boulders: %u unresolved, %u in supply, %u in flight or machinery. Mud: unlimited.\n",
								  _unresolved, supply, _unresolved - supply);
}

constexpr const char *CatapultMachine::kAnimation;
constexpr const char *CatapultMachine::kDiscAnimation;

CatapultMachine::Object::Object(CatapultMachine &owner, Shell::Sprite *image, Stage state, bool isRock) : machine(owner), sprite(image), stage(state), rock(isRock) {
	sprite->setPauseListener(this);
	sprite->setUpdateListener(this);
}

Common::Rect CatapultMachine::Object::position() const {
	return Common::Rect(sprite->x, sprite->y, sprite->x + sprite->width(), sprite->y + sprite->height());
}

bool CatapultMachine::Object::animationPaused() {
	return machine.complete(*this);
}

void CatapultMachine::Object::spriteUpdating(Shell::Sprite &image, uint32 time) {
	if (stage != kMissFlight && stage != kFlight)
		return;
	Common::Rect box = position();
	motion.update(box, time, *this);
	image.x = box.left;
	image.y = box.top;
}

void CatapultMachine::Object::motionBoundary(CatapultMotion &movement, Common::Rect &box,
											 CatapultMotion::Boundary boundary, uint32 time) {
	machine.land(*this, box, boundary, time);
}

CatapultMachine::CatapultMachine(Shell::Scene &scene) : _scene(scene) {
	static const char *const events[] = {
		"correct",
		"incorrect",
		"finishedCorrect",
		"finishedIncorrect",
		"tokenPickedUp",
	};
	for (uint i = 0; i < ARRAYSIZE(events); i++)
		addEvent(events[i]);
}

CatapultMachine::CatapultMachine(Shell::Scene &scene, const CatapultRules &rules) : CatapultMachine(scene) {
	_rules = rules;
	_unresolved = rules.partySize();
}

bool CatapultMachine::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (method == "setupGame" && arguments.size() == 3) {
		CatapultRules rules;
		if (!rules.configure(arguments[0].integer(), arguments[1].integer(), arguments[2].integer()))
			return false;
		clearRound();
		_rules = rules;
		_unresolved = rules.partySize();
		return load();
	}
	if (method == "startRound" && arguments.empty())
		return true;
	if ((method == "pauseRound" || method == "finishRound") && arguments.empty()) {
		pause(callTime());
		if (method == "finishRound")
			emitEvent("finishedCorrect");
		return true;
	}
	if (method == "resumeRound" && arguments.empty()) {
		resume(callTime());
		return true;
	}
	return MultiTrinket::play(method, arguments, result);
}

CatapultMachine::~CatapultMachine() {
	clearRound();
}

void CatapultMachine::clearRound() {
	while (!_objects.empty())
		remove(_objects.back());
	for (uint i = 0; i < _owned.size(); i++)
		_scene.remove(_owned[i]);
	_owned.clear();
	_gears.clear();
	_chute.clear();
	_sounds.clear();
	_results.clear();
	for (uint i = 0; i < ARRAYSIZE(_primary); i++) {
		_primary[i] = nullptr;
		_occupied[i] = false;
	}
	for (uint i = 0; i < ARRAYSIZE(_secondary); i++)
		_secondary[i] = nullptr;
	_paddle = _indicator = _trigger = _mudSplash = _switch = nullptr;
	_running = _paused = true;
	_chuteMoving = _missDone = _secondaryDone = _missSplashDone = _waterSplashDone = _resourceError = false;
}

Shell::Sprite *CatapultMachine::add(uint id, int depth, bool gear) {
	bool disc = (10425 <= id && id <= 10429) || id == 10431 || id == 10432 || id == 10456 || id == 10457 ||
				id == 10463 || id == 10472 || id == 10473 || id == 10474;
	Shell::Sprite *sprite = _scene.add(Common::String::format(disc ? kDiscAnimation : kAnimation, id), depth);
	if (!sprite || !sprite->startAnimation(gear ? -1 : 0)) {
		if (sprite)
			_scene.remove(sprite);
		_resourceError = true;
		return nullptr;
	}
	sprite->setFPS(12);
	sprite->setFrame(1);
	_owned.push_back(sprite);
	addTrinket(*sprite);
	if (gear) {
		_gears.push_back(sprite);
		sprite->pauseAnimation(false);
	}
	return sprite;
}

CatapultMachine::Object *CatapultMachine::addObject(uint id, int depth, Stage stage, bool rock) {
	Shell::Sprite *sprite = add(id, depth);
	if (!sprite)
		return nullptr;
	Object *object = new Object(*this, sprite, stage, rock);
	_objects.push_back(object);
	return object;
}

void CatapultMachine::remove(Object *object) {
	if (!object)
		return;
	for (uint i = 0; i < _objects.size(); i++) {
		if (_objects[i] == object) {
			_objects.remove_at(i);
			break;
		}
	}
	for (uint i = 0; i < _chute.size(); i++) {
		if (_chute[i] == object) {
			_chute.remove_at(i);
			break;
		}
	}
	for (uint i = 0; i < _owned.size(); i++) {
		if (_owned[i] == object->sprite) {
			_owned.remove_at(i);
			break;
		}
	}
	for (uint i = 0; i < ARRAYSIZE(_rocks); i++) {
		if (_rocks[i] == object)
			_rocks[i] = nullptr;
	}
	Object **slots[] = {
		&_mud,
		&_miss,
		&_mudFlight,
		&_rockFlight,
		&_missSplash,
		&_waterSplash,
	};
	for (uint i = 0; i < ARRAYSIZE(slots); i++) {
		if (*slots[i] == object)
			*slots[i] = nullptr;
	}
	_scene.remove(object->sprite);
	delete object;
}

void CatapultMachine::createMudSupply() {
	_mud = addObject(10401, 4, kSupply, false);
	if (!_mud)
		return;
	_mudOrigin = Common::Point(_mud->sprite->x, _mud->sprite->y);
	_mud->sprite->x = 44;
	_mud->sprite->y = 31;
	_mud->sprite->action = kInsertMud01;
	_mud->sprite->actionOnMouseDown = true;
}

bool CatapultMachine::load() {
	for (uint i = 0; i < ARRAYSIZE(_rocks); i++) {
		_rocks[i] = addObject(10402, 3, kSupply, true);
		if (!_rocks[i])
			return false;
		Shell::Sprite *sprite = _rocks[i]->sprite;
		_rockOrigin = Common::Point(sprite->x, sprite->y);
		sprite->x = 90 + 21 * i;
		sprite->y = 12;
		sprite->visible = i < _rules.partySize();
		sprite->enabled = sprite->visible;
		sprite->action = kInsertRock02;
		sprite->actionOnMouseDown = true;
	}
	createMudSupply();
	Shell::Sprite *decoration = add(10401, 3);
	if (!decoration)
		return false;
	decoration->x = 52;
	decoration->y = -7;
	decoration = add(10401, 3);
	if (!decoration)
		return false;
	decoration->x = 47;
	decoration->y = 12;
	_switch = add(10433, 17);
	if (!_switch)
		return false;
	_switch->action = kToggle03;
	uint family = _rules.primaryFamily();
	static constexpr uint catcher[] = {
		10460,
		10462,
		10463,
	};
	static constexpr uint spoke[] = {
		10470,
		10471,
		10472,
	};
	static constexpr uint coupling[] = {
		10419,
		10453,
		10454,
	};
	for (uint i = 0; i < _rules.primaryCount(); i++) {
		_primary[i] = add(catcher[family], 6, true);
		if (!_primary[i])
			return false;
		_primary[i]->setFrame(_rules.primaryStartFrame(i, _primary[i]->getNumberOfFrames()));
		_primary[i]->pauseAnimation(false);
	}
	for (uint i = 0; i < 4; i++) {
		Shell::Sprite *sprite = add(spoke[family], 9, true);
		if (!sprite)
			return false;
		sprite->setFrame(sprite->getNumberOfFrames() / 4 * i + 1);
		sprite->pauseAnimation(false);
	}
	if (!add(10413 + family, 8, true))
		return false;
	if (_rules.level() != 3) {
		if (!add(coupling[family], 7, true))
			return false;
		if (family != 2) {
			static constexpr int originX[] = {
				207,
				224,
			};
			static constexpr int originY[] = {
				247,
				267,
			};
			for (uint i = 0; i < 4; i++) {
				Shell::Sprite *sprite = add(10471, 8, true);
				if (!sprite)
					return false;
				sprite->x = originX[family];
				sprite->y = originY[family];
				if (!sprite->reverseSequence())
					return false;
				sprite->setFrame(sprite->getNumberOfFrames() / 4 * i + 1);
				sprite->pauseAnimation(false);
			}
		}
	}
	uint secondaryFamily = _rules.level() == 3 ? 1 : 0;
	for (uint i = 0; i < 6; i++) {
		_secondary[i] = add(10456 + secondaryFamily, 9, true);
		if (!_secondary[i])
			return false;
		_secondary[i]->setFrame(_rules.secondaryStartFrame(i, _secondary[i]->getNumberOfFrames()));
		_secondary[i]->pauseAnimation(false);
	}
	if (!add(secondaryFamily ? 10455 : 10420, 6, true) || !add(10473 + secondaryFamily, 10, true))
		return false;
	if (_rules.level() == 3) {
		static constexpr uint shoulder[] = {
			2,
			3,
			2,
			2,
			2,
			1,
			1,
			1,
			0,
			4,
			4,
			4,
			4,
		};
		if (!add(10434, 11, true) || !add(10435 + shoulder[_rules.dataset()], 12, true))
			return false;
	}
	static constexpr int offsetY[] = {
		0,
		0,
		-30,
		-15,
		-15,
	};
	static constexpr uint driveFrame[] = {
		1,
		5,
		5,
	};
	static constexpr uint paddleFrame[] = {
		25,
		1,
		21,
		25,
		1,
	};
	int shiftX = _rules.level() == 3 ? 0 : -30;
	int shiftY = _rules.level() == 3 ? 0 : offsetY[_rules.paddleFamily()];
	Shell::Sprite *drive = add(10476 + _rules.driveFamily(), 7, true);
	_indicator = add(10479 + _rules.driveFamily(), 13);
	_paddle = add(10425 + _rules.paddleFamily(), 8, true);
	if (!drive || !_indicator || !_paddle)
		return false;
	Shell::Sprite *reversed[] = {
		drive,
		_paddle,
	};
	for (uint i = 0; i < ARRAYSIZE(reversed); i++) {
		reversed[i]->x += shiftX;
		reversed[i]->y += shiftY;
		if (_rules.level() != 3 && !reversed[i]->reverseSequence())
			return false;
	}
	drive->setFrame(driveFrame[_rules.driveFamily()]);
	_paddle->setFrame(paddleFrame[_rules.paddleFamily()]);
	drive->pauseAnimation(false);
	_paddle->pauseAnimation(false);
	_indicator->x += shiftX + (_rules.paddleFamily() < 2 ? 7 : 0);
	_indicator->y += shiftY + (_rules.paddleFamily() < 2 ? -10 : 0);
	_trigger = add(10423, 16);
	_mudSplash = add(10452, 17);
	if (!_trigger || !_mudSplash || _resourceError)
		return false;
	_mudSplash->setPauseOnLastFrame(true);
	_mudSplash->visible = false;
	return true;
}

void CatapultMachine::applyMachinePause(bool paused) {
	for (uint i = 0; i < _gears.size(); i++)
		_gears[i]->pauseAnimation(paused);
	for (uint i = 0; i < _objects.size(); i++) {
		if (_objects[i]->stage == kPrimary || _objects[i]->stage == kSecondary)
			_objects[i]->sprite->pauseAnimation(paused);
	}
}

void CatapultMachine::pause(uint32 time) {
	_paused = true;
	_chuteMoving = false;
	applyMachinePause(true);
	for (uint i = 0; i < _chute.size(); i++)
		_chute[i]->sprite->pauseAnimation(true);
	Object *flights[] = {
		_miss,
		_mudFlight,
		_rockFlight,
	};
	for (uint i = 0; i < ARRAYSIZE(flights); i++) {
		if (flights[i])
			flights[i]->motion.pause(time);
	}
}

void CatapultMachine::resume(uint32 time) {
	_paused = false;
	if (_running)
		applyMachinePause(false);
	Object *flights[] = {
		_miss,
		_mudFlight,
		_rockFlight,
	};
	for (uint i = 0; i < ARRAYSIZE(flights); i++) {
		if (flights[i])
			flights[i]->motion.resume(time);
	}
}

void CatapultMachine::action(int action) {
	if (_paused || _resourceError)
		return;
	if (action == kToggle03) {
		_running = !_running;
		_switch->setFrame(_running ? 1 : 2);
		applyMachinePause(!_running);
		_sounds.push_back(kResSound10407_Launch);
		return;
	}
	if (action != kInsertMud01 && action != kInsertRock02)
		return;
	if (!_chute.empty() && _chute.back()->sprite->getFrame() < 4)
		return;
	Object *object = _mud;
	if (action == kInsertRock02) {
		object = nullptr;
		for (uint i = 0; i < _rules.partySize(); i++) {
			if (!_rocks[i] || !_rocks[i]->sprite->enabled)
				break;
			object = _rocks[i];
		}
	}
	if (!object)
		return;
	Common::Point origin = object->rock ? _rockOrigin : _mudOrigin;
	object->sprite->x = origin.x;
	object->sprite->y = origin.y;
	if (!object->rock)
		createMudSupply();
	_scene.setDepth(object->sprite, 3);
	object->sprite->setPauseOnLastFrame(true);
	object->sprite->enabled = false;
	object->stage = kChute;
	if (object->rock)
		object->sprite->pauseAnimation(false);
	_chute.push_back(object);
	_sounds.push_back(kResSound10411_PaddleHit);
}

void CatapultMachine::transferPrimary(Object *object) {
	Object *carried = addObject((object->rock ? 10446 : 10440) + _rules.primaryFamily(), 5, kPrimary, object->rock);
	if (!carried)
		return;
	carried->sprite->setPauseOnLastFrame(true);
	carried->sprite->setFrame(_rules.primaryTransportFrame());
	_sounds.push_back(kResSound10413_Trigger);
	pause(_time);
	resume(_time);
	remove(object);
}

bool CatapultMachine::transferSecondary(Object *object) {
	uint variant = _rules.level() == 3 ? 1 : 0;
	Object *carried = addObject((object->rock ? 10449 : 10443) + variant, 5, kSecondary, object->rock);
	if (!carried)
		return true;
	carried->sprite->setPauseOnLastFrame(true);
	bool captured = false;
	for (uint i = 0; i < ARRAYSIZE(_secondary); i++) {
		uint frame = _secondary[i]->getFrame();
		if (!_rules.canReceiveFromPrimary(frame))
			continue;
		carried->sprite->setFrame(_rules.secondaryTransportFrame(frame));
		_sounds.push_back(kResSound10414_Miss);
		captured = true;
		break;
	}
	if (captured) {
		pause(_time);
		resume(_time);
	} else {
		startFlight(carried, true);
	}
	remove(object);
	return false;
}

void CatapultMachine::startFlight(Object *object, bool missed) {
	Object **slot = &_miss;
	if (!missed)
		slot = object->rock ? &_rockFlight : &_mudFlight;
	remove(*slot);
	*slot = object;
	object->stage = missed ? kMissFlight : kFlight;
	object->paddleLatched = false;
	object->landing = kWater;
	object->motion.setBounds(Common::Rect(800, missed ? 560 : 520));
	object->motion.startBounce(200, 500, 0, object->position(), _time);
	object->motion.startWalk(missed ? 80 : _rules.flightVelocity(), 0);
	_scene.setDepth(object->sprite, 5);
}

bool CatapultMachine::complete(Object &object) {
	if (object.stage == kPrimary)
		return transferSecondary(&object);
	if (object.stage == kSecondary)
		startFlight(&object, false);
	else if (object.stage == kMissSplash)
		_missSplashDone = true;
	else if (object.stage == kWaterSplash)
		_waterSplashDone = true;
	return true;
}

void CatapultMachine::land(Object &object, Common::Rect &position, CatapultMotion::Boundary boundary, uint32 time) {
	if (boundary == CatapultMotion::kRight && object.stage == kFlight && object.rock && object.landing == kNoLanding) {
		object.paddleLatched = false;
		_secondaryDone = true;
		return;
	}
	if (boundary != CatapultMotion::kBounce)
		return;
	if (object.stage == kMissFlight) {
		_missDone = true;
		return;
	}
	if (object.landing == kNoLanding)
		return;
	if (object.landing == kPaddle) {
		object.paddleLatched = true;
		object.landing = kTrigger;
		object.motion.setBounds(Common::Rect(800, 465));
		object.motion.startBounce(200, 500, 0, position, time);
		object.motion.startWalk(780 - position.left, 0);
		_scene.setDepth(object.sprite, 17);
		_sounds.push_back(kResSound10415_Impact);
	} else if (object.landing == kTrigger && object.rock) {
		object.landing = kNoLanding;
		_trigger->setFrame(2);
		_sounds.push_back(kResSound10404_Splash);
	} else {
		object.paddleLatched = false;
		_secondaryDone = true;
		if (object.landing == kTrigger) {
			_mudSplash->setFrame(1);
			_mudSplash->x = position.left;
			_mudSplash->y = position.top + 10;
			_mudSplash->visible = true;
			_mudSplash->pauseAnimation(false);
			_sounds.push_back(kResSound10403_MudDrop);
		} else {
			_sounds.push_back(object.rock ? kResSound10405_RockSplash : kResSound10412_MudSplash);
			showWaterSplash(false);
		}
	}
}

void CatapultMachine::showWaterSplash(bool missed) {
	Object **slot = missed ? &_missSplash : &_waterSplash;
	if (missed)
		_missSplashDone = false;
	else
		_waterSplashDone = false;
	if (!*slot) {
		*slot = addObject(missed ? 10431 : 10432, 1, missed ? kMissSplash : kWaterSplash, false);
		if (!*slot)
			return;
		(*slot)->sprite->setPauseOnLastFrame(true);
	}
	(*slot)->sprite->setFrame(1);
	(*slot)->sprite->pauseAnimation(false);
}

void CatapultMachine::resolveRock(bool correct) {
	if (_unresolved)
		_unresolved -= 1;
	Result result = {correct, _unresolved == 0};
	emitEvent(correct ? (result.finished ? "finishedCorrect" : "correct") : (result.finished ? "finishedIncorrect" : "incorrect"));
	if (_resultListener)
		_resultListener->catapultResult(result, _time);
	else
		_results.push_back(result);
}

void CatapultMachine::updateRound() {
	_chuteMoving = false;
	if (!_paused) {
		for (uint i = 0; i < _chute.size(); i++) {
			Shell::Sprite *sprite = _chute[i]->sprite;
			if (i && CatapultRules::chuteCollision(_chute[i]->position(), _chute[i - 1]->position())) {
				sprite->pauseAnimation(true);
			} else if (sprite->getFrame() < sprite->getNumberOfFrames()) {
				sprite->pauseAnimation(false);
				_chuteMoving = true;
			}
		}
		if (_running && !_chute.empty()) {
			Object *head = _chute[0];
			if (head->sprite->getFrame() == head->sprite->getNumberOfFrames()) {
				for (uint i = 0; i < _rules.primaryCount(); i++) {
					uint frame = _primary[i]->getFrame();
					if (_rules.releasesPrimaryLatch(frame))
						_occupied[i] = false;
					if (_rules.canReceiveFromChute(frame) && !_occupied[i]) {
						_occupied[i] = true;
						transferPrimary(head);
						break;
					}
				}
			}
		}
		bool open = _rules.paddleIsOpen(_paddle->getFrame(), _paddle->getNumberOfFrames());
		if (open != (_indicator->getFrame() == 1)) {
			_indicator->setFrame(open ? 1 : 2);
			_sounds.push_back(open ? kResSound10417_GateOpen : kResSound10416_GateClosed);
		}
		Object *flights[] = {
			_mudFlight,
			_rockFlight,
		};
		for (uint i = 0; i < ARRAYSIZE(flights); i++) {
			Object *object = flights[i];
			if (!object || object->paddleLatched || _rules.paddleHeight() <= object->sprite->y)
				continue;
			object->motion.setBounds(Common::Rect(open ? 900 : 800, open ? _rules.paddleHeight() : 520));
			object->landing = open ? kPaddle : kWater;
		}
	}
	if (_missDone && _miss) {
		bool rock = _miss->rock;
		remove(_miss);
		_missDone = false;
		if (rock)
			resolveRock(false);
		_sounds.push_back(rock ? kResSound10405_RockSplash : kResSound10412_MudSplash);
		showWaterSplash(true);
	}
	if (_secondaryDone) {
		if (_mudFlight) {
			remove(_mudFlight);
			_secondaryDone = false;
		}
		// Both tracks share this flag; mud consumes it before the rock branch.
		if (_secondaryDone && _rockFlight) {
			bool failed = _rockFlight->motion.walkXVelocity() < 200;
			remove(_rockFlight);
			_secondaryDone = false;
			if (failed)
				resolveRock(false);
		}
	}
	if (_trigger->getFrame() == 2) {
		resolveRock(true);
		_trigger->setFrame(1);
	}
	if (_missSplashDone) {
		remove(_missSplash);
		_missSplashDone = false;
	}
	if (_waterSplashDone) {
		remove(_waterSplash);
		_waterSplashDone = false;
	}
}

void CatapultMachine::update(uint32 time) {
	_time = time;
	if (_resourceError || !_trigger)
		return;
	updateRound();
	_scene.update(time, _sounds);
}

bool CatapultMachine::pollSound(uint16 &sound) {
	if (_sounds.empty())
		return false;
	sound = _sounds[0];
	_sounds.remove_at(0);
	return true;
}

bool CatapultMachine::pollResult(Result &result) {
	if (_results.empty())
		return false;
	result = _results[0];
	_results.remove_at(0);
	return true;
}

} // namespace Zoombini3
} // namespace Trinket
