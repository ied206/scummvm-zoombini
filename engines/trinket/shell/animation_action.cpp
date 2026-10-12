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

#include "trinket/shell/animation_action.h"

namespace Trinket {
namespace Shell {

AnimationAction::~AnimationAction() {
	removeAnimation();
}

void AnimationAction::removeAnimation() {
	if (_animation) {
		_animation->removeAnimationListener(*this);
		_animation->removeObserver(*this);
		_scene.remove(_animation);
		_animation = nullptr;
	}
}

void AnimationAction::start() {
	ActionSystem::Scope scope(_system);
	removeAnimation();
	Action::start();
	if (_resource.empty()) {
		finished();
		return;
	}
	_animation = _scene.add(_resource, _depth);
	if (!_animation || !_animation->startAnimation(_repeat))
		error("Could not load action animation %s", _resource.c_str());
	_animation->enabled = false;
	if (_position.x != 11111)
		_animation->x = _position.x;
	if (_position.y != 11111)
		_animation->y = _position.y;
	_animation->addObserver(*this);
	_animation->setAnimationListener(this);
	_animation->playAnimation();
}

void AnimationAction::halt() {
	ActionSystem::Scope scope(_system);
	Action::halt();
	removeAnimation();
}

void AnimationAction::pause() {
	Action::pause();
	if (_animation)
		_animation->pauseAnimation(true);
}

void AnimationAction::resume() {
	Action::resume();
	if (_animation)
		_animation->pauseAnimation(false);
}

void AnimationAction::animationFinished(Sprite &) {
	ActionSystem::Scope scope(_system);
	halt();
	finished();
}

PlayAnimationAction::~PlayAnimationAction() {
	detach();
}

void PlayAnimationAction::detach() {
	if (_animation) {
		_animation->removeAnimationListener(*this);
		_animation->removeObserver(*this);
		_animation = nullptr;
	}
}

void PlayAnimationAction::start() {
	ActionSystem::Scope scope(_system);
	detach();
	_animation = target();
	Action::start();
	if (!_animation) {
		finished();
		return;
	}
	_animation->addObserver(*this);
	_animation->setAnimationListener(this);
	_animation->playAnimation();
}

void PlayAnimationAction::halt() {
	ActionSystem::Scope scope(_system);
	Action::halt();
	Sprite *animation = target();
	detach();
	if (animation) {
		animation->setAnimationListener(nullptr);
		if (animation->pausesOnLastFrame())
			animation->setFrame(animation->getNumberOfFrames());
		else
			animation->stopAnimation();
	}
}

void PlayAnimationAction::pause() {
	Action::pause();
	Sprite *animation = target();
	if (animation)
		animation->pauseAnimation(true);
}

void PlayAnimationAction::resume() {
	Action::resume();
	Sprite *animation = target();
	if (animation)
		animation->pauseAnimation(false);
}

void PlayAnimationAction::animationFinished(Sprite &) {
	ActionSystem::Scope scope(_system);
	detach();
	finished();
}

} // namespace Shell
} // namespace Trinket
