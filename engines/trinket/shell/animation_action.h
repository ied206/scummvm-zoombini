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

#ifndef TRINKET_SHELL_ANIMATION_ACTION_H
#define TRINKET_SHELL_ANIMATION_ACTION_H

#include "trinket/shell/action.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace Shell {

/** Owns a temporary animation for one action execution. */
class AnimationAction : public Action, private Sprite::AnimationListener, private Core::ScriptableObject::Observer {
public:
	AnimationAction(ActionSystem &system, Scene &scene, const Common::String &resource, Common::Point position,
					int depth, int repeat, bool interruptible) : Action(system, interruptible), _scene(scene), _resource(resource), _position(position), _depth(depth), _repeat(repeat) {}
	~AnimationAction() override;
	void start() override;
	void halt() override;
	void pause() override;
	void resume() override;
	Core::ScriptableObject *clone() const override {
		return new AnimationAction(_system, _scene, _resource, _position, _depth, _repeat, isInterruptible());
	}

private:
	Scene &_scene;
	Common::String _resource;
	Common::Point _position;
	int _depth, _repeat;
	Sprite *_animation = nullptr;
	void removeAnimation();
	void animationFinished(Sprite &) override;
	void objectDestroyed(Core::ScriptableObject &) override { _animation = nullptr; }
};

/** Plays an existing animation from its current authored position. */
class PlayAnimationAction : public Action, private Sprite::AnimationListener, private Core::ScriptableObject::Observer {
public:
	PlayAnimationAction(ActionSystem &system, Scene &scene, const Core::ScriptValue &target, bool interruptible) : Action(system, interruptible), _scene(scene), _target(target) {}
	~PlayAnimationAction() override;
	void start() override;
	void halt() override;
	void pause() override;
	void resume() override;
	Core::ScriptableObject *clone() const override { return new PlayAnimationAction(_system, _scene, _target, isInterruptible()); }

private:
	Scene &_scene;
	Core::ScriptValue _target;
	Sprite *_animation = nullptr;
	Sprite *target() const { return _scene.resolveSprite(_system.resolve(_target)); }
	void detach();
	void animationFinished(Sprite &sprite) override;
	void objectDestroyed(Core::ScriptableObject &) override { _animation = nullptr; }
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
