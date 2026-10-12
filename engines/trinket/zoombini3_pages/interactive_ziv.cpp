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

#include "trinket/zoombini3_pages/interactive_ziv.h"

#include "common/algorithm.h"
#include "common/file.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace Zoombini3 {

constexpr const char *ZIVMachine::kAnimation;

ZIVMachine::ZIVMachine(Shell::Scene &scene, Shell::Scroller &scroller, uint32 time) : _scene(scene), _scroller(scroller), _cullTime(time + 200) {
	_scroller.start(time);
}

ZIVMachine::~ZIVMachine() {
	_scroller.stop();
	clearAnimations();
}

void ZIVMachine::clearAnimations() {
	for (uint i = 0; i < _animations.size(); i++)
		_scene.remove(_animations[i]);
	_ambient.clear();
	_animations.clear();
	_pausedAnimations.clear();
}

bool ZIVMachine::addAnimation(uint id, bool cull, int repeat) {
	Shell::Sprite *sprite = _scene.add(Common::String::format(kAnimation, id), 1);
	if (!sprite)
		return false;
	if (!sprite->startAnimation(repeat)) {
		_scene.remove(sprite);
		return false;
	}
	sprite->enabled = false;
	sprite->movable = false;
	_scroller.add(*sprite);
	switch (id) {
	case 22008:
		sprite->x = 2390;
		break;
	case 22011:
		sprite->x = 2160;
		break;
	case 22056:
		sprite->x = 2130;
		break;
	case 22015:
		sprite->x = 2230;
		break;
	case 22030:
		sprite->x = 2280;
		break;
	case 22038:
		sprite->x = 2220;
		break;
	case 22021:
		sprite->x = 2325;
		break;
	case 22029:
		sprite->x = 2055;
		break;
	case 22037:
		sprite->x = 2240;
		break;
	default:
		break;
	}
	if (cull)
		_ambient.push_back(sprite);
	_animations.push_back(sprite);
	return true;
}

bool ZIVMachine::setup(uint tokens, uint32 time) {
	clearAnimations();
	static constexpr uint decorative[] = {
		22008,
		22011,
		22013,
		22054,
		22055,
		22056,
		22012,
	};
	static constexpr uint ambient[] = {
		22014,
		22015,
		22016,
		22017,
		22042,
	};
	for (uint i = 0; i < ARRAYSIZE(decorative); i++)
		if (!addAnimation(decorative[i], false))
			return false;
	for (uint i = 0; i < ARRAYSIZE(ambient); i++)
		if (!addAnimation(ambient[i], true))
			return false;
	uint population = MAX<uint>(1, tokens / 10);
	if (population == 24)
		population = 25;
	for (uint i = 0; i < population; i++)
		if (!addAnimation(22017 + i, i < 24, i < 24 ? -1 : 0))
			return false;
	_cullTime = time + 200;
	return true;
}

bool ZIVMachine::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if ((method == "pauseRound" || method == "resumeRound") && arguments.empty()) {
		pause(method == "pauseRound");
		return true;
	}
	if (method == "setupGame" && arguments.size() == 1 && 0 <= arguments[0].integer() &&
		static_cast<uint>(arguments[0].integer()) <= World::kTokenCount) {
		_valid = setup(arguments[0].integer(), callTime());
		return _valid;
	}
	return Shell::MultiTrinket::play(method, arguments, result);
}

void ZIVMachine::pause(bool paused) {
	if (_paused == paused)
		return;
	_paused = paused;
	if (paused) {
		for (uint i = 0; i < _animations.size(); i++)
			if (_animations[i]->isRunning()) {
				_pausedAnimations.push_back(_animations[i]);
				_animations[i]->pauseAnimation(true);
			}
	} else {
		for (uint i = 0; i < _pausedAnimations.size(); i++)
			_pausedAnimations[i]->pauseAnimation(false);
		_pausedAnimations.clear();
	}
}

void ZIVMachine::update(uint32 time) {
	if (_paused || static_cast<int32>(time) < static_cast<int32>(_cullTime))
		return;
	_cullTime = time + 200;
	for (uint i = 0; i < _ambient.size(); i++) {
		Shell::Sprite &sprite = *_ambient[i];
		if (900 <= sprite.x || sprite.x + sprite.width() <= -100) {
			if (sprite.isRunning())
				sprite.pauseAnimation(true);
		} else if (!sprite.isRunning()) {
			sprite.pauseAnimation(false);
		}
	}
}

} // namespace Zoombini3
} // namespace Trinket
