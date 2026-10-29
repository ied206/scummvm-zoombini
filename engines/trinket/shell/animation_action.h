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

/** Play a temporary animation for one queued action. */
class AnimationAction : public Action, private Sprite::AnimationListener, private Core::ScriptableObject::Observer {
public:
	/** Configure a temporary sprite's resource, position, depth and repeats. */
	AnimationAction(ActionSystem &system, Scene &scene, const Common::String &resource, Common::Point position,
					int depth, int repeat, bool interruptible) : Action(system, interruptible), _scene(scene), _resource(resource), _position(position), _depth(depth), _repeat(repeat) {}
	/** Remove the temporary sprite and detach its completion callbacks. */
	~AnimationAction() override;
	/** Create the temporary sprite and begin its animation. */
	void start() override;
	/** Stop the action and remove its temporary sprite. */
	void halt() override;
	/** Pause the temporary sprite's animation. */
	void pause() override;
	/** Resume the temporary sprite's animation. */
	void resume() override;
	/** Create an inactive action with the same animation settings. */
	Core::ScriptableObject *clone() const override {
		return new AnimationAction(_system, _scene, _resource, _position, _depth, _repeat, isInterruptible());
	}

private:
	/** Scene that displays the temporary sprite. */
	Scene &_scene;
	/** Animation resource path loaded when the action starts. */
	Common::String _resource;
	/** Initial sprite position in game pixels. */
	Common::Point _position;
	/** Sprite depth within the scene's drawing order. */
	int _depth;
	/** Repeat count passed to the sprite's animation player. */
	int _repeat;
	/** Temporary sprite currently playing, or null before creation and after removal. */
	Sprite *_anim = nullptr;
	/** Detach callbacks and remove the temporary sprite from the scene. */
	void removeAnimation();
	/** Remove the temporary sprite and finish the action. */
	void animationFinished(Sprite &) override;
	/** Clear the sprite pointer if another operation destroys it. */
	void objectDestroyed(Core::ScriptableObject &) override { _anim = nullptr; }
};

/** Plays an existing animation from its current authored position. */
class PlayAnimationAction : public Action, private Sprite::AnimationListener, private Core::ScriptableObject::Observer {
public:
	/** Configure playback of an existing sprite resolved from a script value. */
	PlayAnimationAction(ActionSystem &system, Scene &scene, const Core::ScriptValue &target, bool interruptible) : Action(system, interruptible), _scene(scene), _target(target) {}
	/** Detach callbacks from the existing target sprite. */
	~PlayAnimationAction() override;
	/** Resolve the target sprite and resume its animation. */
	void start() override;
	/** Stop playback and detach from the target sprite. */
	void halt() override;
	/** Pause the target sprite's animation. */
	void pause() override;
	/** Resume the target sprite's animation. */
	void resume() override;
	/** Create another inactive action referring to the same scripted target. */
	Core::ScriptableObject *clone() const override { return new PlayAnimationAction(_system, _scene, _target, isInterruptible()); }

private:
	/** Scene used to validate and resolve the target sprite. */
	Scene &_scene;
	/** Script value identifying the existing animation sprite. */
	Core::ScriptValue _target;
	/** Sprite currently watched for animation completion. */
	Sprite *_anim = nullptr;
	/** Resolve the current script target to a scene sprite, or return null. */
	Sprite *target() const { return _scene.resolveSprite(_system.resolve(_target)); }
	/** Remove animation and destruction callbacks from the target sprite. */
	void detach();
	/** Detach from the completed sprite and finish the action. */
	void animationFinished(Sprite &sprite) override;
	/** Clear the sprite pointer when the target is destroyed. */
	void objectDestroyed(Core::ScriptableObject &) override { _anim = nullptr; }
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
