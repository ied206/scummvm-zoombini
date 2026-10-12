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

#ifndef TRINKET_ZOOMBINI3_CHARACTER_H
#define TRINKET_ZOOMBINI3_CHARACTER_H

#include "trinket/ompp/sound.h"
#include "trinket/shell/multitrinket.h"
#include "trinket/shell/scene.h"

namespace Trinket {

class TrinketEngine;
namespace Zoombini3 {

/** Attribute-composed character with pose timers and queued walking. */
class AttributeCharacter : public Shell::MultiTrinket {
public:
	class AnimationResources {
	public:
		virtual ~AnimationResources() {}
		virtual Common::String characterAnimationPath(uint resource) const = 0;
	};
	class WalkListener {
	public:
		virtual ~WalkListener() {}
		virtual void walkFinished(AttributeCharacter &character) = 0;
	};

	AttributeCharacter(TrinketEngine &vm, Shell::Scene &scene);
	~AttributeCharacter() override;
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	int width() override { return bounds().width(); }
	int height() override { return bounds().height(); }
	bool create(uint type, const int32 *traits, uint pose, Common::Point position, int depth, uint32 time);
	bool create(const Core::ScriptArguments &arguments, uint32 time);
	bool setPose(uint pose, uint32 time);
	void setAnimationResources(const AnimationResources *resources) { _animationResources = resources; }
	void addWalkMove(Common::Point destination, bool absolute = true);
	bool walk(bool sound, uint32 time, bool clearPath = true, bool keepPosition = false);
	void setWalkListener(WalkListener *listener) { _walkListener = listener; }
	void removeWalkListener(WalkListener &listener) {
		if (_walkListener == &listener)
			_walkListener = nullptr;
	}
	/** Return whether a walk or feedback animation completed during this update. */
	bool update(uint32 time, Common::Array<uint16> &sounds);
	bool hasResourceError() const { return !_valid; }
	void pause(bool paused, uint32 time);
	void setVisible(bool shown, uint32 time);
	void setDepth(int depth);
	void setDepthFromY(bool usePosition, int originY = 0);
	void stopWalk(bool finishPath);
	bool beginDrag(Common::Point mouse, uint32 time, const Common::Rect &corral = Common::Rect(800, 600));
	void dragTo(Common::Point mouse);

	/** Restore the home position and return the attempted drop anchor. */
	Common::Point endDrag(bool notify = true);
	bool isDragging() const { return _dragging; }
	Common::Point position() const { return _position; }
	void setPosition(Common::Point position) {
		_home = position;
		moveTo(position);
	}
	Common::Rect bounds() const;
	bool isWalking() const { return _walking; }

private:
	static constexpr const char *kAsset = "/z3common/%u.ao";
	static constexpr const char *kSound = "/z3common/%u.wav";
	struct Variant {
		Common::Array<Shell::Sprite *> parts;
	};
	struct Timer {
		uint minimum, maximum;
		uint32 deadline;
		Common::Array<Variant> variants;
		Common::Array<uint> available;
	};
	TrinketEngine &_vm;
	Shell::Scene &_scene;
	OM::Sound _walkSound;
	uint _type = 0, _count = 0;
	int32 _traits[6] = {};
	int _pose = -1, _z = 0, _direction = -1;
	bool _mirror = false, _walkMirror = false, _walking = false, _paused = false, _visible = true, _dragging = false;
	Common::Point _dragHome, _dragOffset, _dropped;
	int _dragDepth = 0;
	Common::Rect _dragCorral;
	bool _dragOutsideX = false, _dragOutsideY = false;
	bool _scriptCorral = false, _isCorralled = false;
	bool _depthFromY = false;
	bool _doWalkSFX = true, _loadFidgetsInBackground = true;
	int _depthBase = 0, _depthOriginY = 0;
	Common::Point _origin, _position, _home, _start, _delta;
	float _distance = 0, _travelled = 0;
	uint32 _lastMove = 0;
	Common::Array<Common::Point> _path;
	Common::Array<bool> _absolutePath;
	WalkListener *_walkListener = nullptr;
	bool _clearWalkPath = true, _keepWalkPosition = false;
	uint _nextPoint = 0;
	Common::Array<Shell::Sprite *> _base, _moving, _dragParts;
	const AnimationResources *_animationResources = nullptr;
	Common::Array<Shell::Sprite *> _feedback;
	bool _feedbackKeepVisible = true;
	Common::Array<Timer> _timers;
	int _activeTimer = -1, _activeVariant = -1;
	bool _valid = true;

	Shell::Sprite *loadPart(uint resource, uint attribute, bool movement = false);
	void clearParts(Common::Array<Shell::Sprite *> &parts);
	void clearTimers(uint32 time);
	void schedule(Timer &timer, uint32 time);
	void addTimer(uint minimum, uint maximum, uint firstBase, int maskAttribute, uint secondBase, uint32 time);
	void startVariant(uint timer, uint variant);
	void finishVariant(uint32 time);
	bool playFeedback(uint resource, int repeat, bool keepVisible, int selector, uint32 time);
	void stopFeedback();
	void moveTo(Common::Point position);
	void beginSegment();
	void updateParts(Common::Array<Shell::Sprite *> &parts, uint32 time, Common::Array<uint16> &sounds);
	static uint32 ticks(uint32 time);
	AttributeCharacter(const AttributeCharacter &) = delete;
	AttributeCharacter &operator=(const AttributeCharacter &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
