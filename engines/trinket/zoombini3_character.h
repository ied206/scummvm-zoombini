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
		/** Allow destruction through the character-animation resource interface. */
		virtual ~AnimationResources() {}
		/** Resolve a character animation resource ID to its virtual path. */
		virtual Common::String characterAnimationPath(uint resource) const = 0;
	};
	class WalkListener {
	public:
		/** Allow destruction through the walk-completion interface. */
		virtual ~WalkListener() {}
		/** Notify the caller that the character's queued walk completed. */
		virtual void walkFinished(AttributeCharacter &character) = 0;
	};

	/** Connect composed character sprites to engine resources and a scene. */
	AttributeCharacter(TrinketEngine &vm, Shell::Scene &scene);
	/** Stop walking audio and remove pose, movement and feedback sprites. */
	~AttributeCharacter() override;
	/** Read character traits, pose, movement and inherited properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign supported trait, placement and movement properties. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch pose, walking, dragging and feedback script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Return the combined visible-part width in game pixels. */
	int width() override { return bounds().width(); }
	/** Return the combined visible-part height in game pixels. */
	int height() override { return bounds().height(); }
	/** Build the character's trait parts and initial pose from explicit settings or script arguments. */
	bool create(uint type, const int32 *traits, uint pose, Common::Point position, int depth, uint32 time);
	/** Build the character's trait parts and initial pose from explicit settings or script arguments. */
	bool create(const Core::ScriptArguments &sargs, uint32 time);
	/** Replace pose parts and recreate their idle timers at a millisecond timestamp. */
	bool setPose(uint pose, uint32 time);
	/** Select an alternate animation path resolver, or restore default paths with null. */
	void setAnimationResources(const AnimationResources *resources) { _animRes = resources; }
	/** Append an absolute destination or relative displacement to the walk path. */
	void addWalkMove(Common::Point destination, bool absolute = true);
	/** Begin the queued walk with the requested sound and path-completion policy. */
	bool walk(bool sound, uint32 time, bool clearPath = true, bool keepPosition = false);
	/** Set the callback notified when the queued walk finishes. */
	void setWalkListener(WalkListener *listener) { _walkListener = listener; }
	/** Detach the supplied walk callback if it is still registered. */
	void removeWalkListener(WalkListener &listener) {
		if (_walkListener == &listener)
			_walkListener = nullptr;
	}
	/** Return whether a walk or feedback animation completed during this update. */
	bool update(uint32 time, Common::Array<uint16> &sounds);
	/** Return whether a required character resource failed to load. */
	bool hasResourceError() const { return !_valid; }
	/** Pause or resume movement and idle timers at a millisecond timestamp. */
	void pause(bool paused, uint32 time);
	/** Show or hide the character's parts and update idle timing. */
	void setVisible(bool shown, uint32 time);
	/** Apply a drawing depth to the character's part sprites. */
	void setDepth(int depth);
	/** Choose whether drawing depth follows vertical position relative to an origin. */
	void setDepthFromY(bool usePosition, int originY = 0);
	/** Stop movement, optionally finishing the remaining path. */
	void stopWalk(bool finishPath);
	/** Begin a pointer drag within a game-pixel corral at a millisecond timestamp. */
	bool beginDrag(Common::Point mouse, uint32 time, const Common::Rect &corral = Common::Rect(800, 600));
	/** Move the dragged character according to the latest pointer position. */
	void dragTo(Common::Point mouse);

	/** Restore the home position and return the attempted drop anchor. */
	Common::Point endDrag(bool notify = true);
	/** Return whether the character is currently following a pointer drag. */
	bool isDragging() const { return _dragging; }
	/** Return the character anchor in game pixels. */
	Common::Point position() const { return _position; }
	/** Move the character anchor and realign its part sprites. */
	void setPosition(Common::Point position) {
		_home = position;
		moveTo(position);
	}
	/** Return the combined visible-part rectangle in game pixels. */
	Common::Rect bounds() const;
	/** Return whether the queued path is currently advancing. */
	bool isWalking() const { return _walking; }

private:
	/** Script event name for finished. */
	static constexpr const char *kEventFinished = "finished";
	/** Script event name for dropped. */
	static constexpr const char *kEventDropped = "dropped";
	/** Script property name for real X. */
	static constexpr const char *kPropertyRealX = "realX";
	/** Script property name for real Y. */
	static constexpr const char *kPropertyRealY = "realY";
	/** Script property name for dropped X. */
	static constexpr const char *kPropertyDroppedX = "droppedX";
	/** Script property name for dropped Y. */
	static constexpr const char *kPropertyDroppedY = "droppedY";
	/** Script property name for is Corralled. */
	static constexpr const char *kPropertyIsCorralled = "isCorralled";
	/** Script property name for do Walk SFX. */
	static constexpr const char *kPropertyDoWalkSFX = "doWalkSFX";
	/** Script property name for do ZUpdates. */
	static constexpr const char *kPropertyDoZUpdates = "doZUpdates";
	/** Script property name for z Base Y. */
	static constexpr const char *kPropertyZBaseY = "zBaseY";
	/** Script property name for load Fidgets In Bkgd. */
	static constexpr const char *kPropertyLoadFidgetsInBkgd = "loadFidgetsInBkgd";
	/** Script method name for play Anim. */
	static constexpr const char *kMethodPlayAnim = "playAnim";
	/** Script method name for set Home Point. */
	static constexpr const char *kMethodSetHomePoint = "setHomePoint";
	/** Script method name for force Drop. */
	static constexpr const char *kMethodForceDrop = "forceDrop";
	/** Script method name for add Walk Move. */
	static constexpr const char *kMethodAddWalkMove = "addWalkMove";
	/** Script method name for walk. */
	static constexpr const char *kMethodWalk = "walk";
	/** Script method name for stop Action. */
	static constexpr const char *kMethodStopAction = "stopAction";
	/** Script method name for stop. */
	static constexpr const char *kMethodStop = "stop";
	/** Script method name for pause. */
	static constexpr const char *kMethodPause = "pause";
	/** Script method name for resume. */
	static constexpr const char *kMethodResume = "resume";
	/** Script property name for x. */
	static constexpr const char *kPropertyX = "x";
	/** Script property name for y. */
	static constexpr const char *kPropertyY = "y";
	/** Script property name for visible. */
	static constexpr const char *kPropertyVisible = "visible";
	/** Script property name for z. */
	static constexpr const char *kPropertyZ = "z";

	/** Default animation path pattern for shared character resources. */
	static constexpr const char *kAsset = "/z3common/%u.ao";
	/** Sound path pattern for shared character resource IDs. */
	static constexpr const char *kSound = "/z3common/%u.wav";
	struct Variant {
		/** Attribute-part sprites forming one idle-animation variant. */
		Common::Array<Shell::Sprite *> parts;
	};
	struct Timer {
		/** Inclusive minimum delay between idle attempts, in seconds. */
		uint minimum;
		/** Exclusive maximum delay between idle attempts, in seconds. */
		uint maximum;
		/** Next idle attempt in sixty-per-second ticks. */
		uint32 deadline;
		/** Idle-animation variants available to this timer. */
		Common::Array<Variant> variants;
		/** Variant indices remaining in the non-repeating selection pool. */
		Common::Array<uint> available;
	};
	/** Engine supplying character resources and the shared random stream. */
	TrinketEngine &_vm;
	/** Scene displaying the character's component sprites. */
	Shell::Scene &_scene;
	/** Playback state for the character's walking sound. */
	OM::Sound _walkSound;
	/** Character family selecting the part count and resource layout. */
	uint _type = 0;
	/** Number of attribute parts used by the selected character family. */
	uint _count = 0;
	/** Attribute codes used to select the character's visual parts. */
	int32 _traits[6] = {};
	/** Current pose code, or minus one before pose initialization. */
	int _pose = -1;
	/** Current drawing depth applied to component sprites. */
	int _z = 0;
	/** Directional animation variant selected for the current walk segment. */
	int _direction = -1;
	/** Whether the current character pose is mirrored horizontally. */
	bool _mirror = false;
	/** Whether walking parts require horizontal mirroring. */
	bool _walkMirror = false;
	/** Whether a queued walk is active. */
	bool _walking = false;
	/** Whether movement and timed animations are paused. */
	bool _paused = false;
	/** Whether the character's active parts should be drawn. */
	bool _visible = true;
	/** Whether pointer movement currently controls the character. */
	bool _dragging = false;
	/** Character anchor recorded before dragging began. */
	Common::Point _dragHome;
	/** Pointer displacement from the drag anchor in pixels. */
	Common::Point _dragOffset;
	/** Most recent character drop position in game pixels. */
	Common::Point _dropped;
	/** Drawing depth restored after a drag. */
	int _dragDepth = 0;
	/** Game-pixel rectangle constraining the current drag. */
	Common::Rect _dragCorral;
	/** Whether the pointer crossed the drag corral's horizontal bounds. */
	bool _dragOutsideX = false;
	/** Whether the pointer crossed the drag corral's vertical bounds. */
	bool _dragOutsideY = false;
	/** Whether scripts supplied an explicit drag corral. */
	bool _scriptCorral = false;
	/** Whether corral constraints apply to character movement. */
	bool _isCorralled = false;
	/** Whether drawing depth is derived from vertical position. */
	bool _depthFromY = false;
	/** Whether the current walk uses its footstep sound. */
	bool _doWalkSFX = true;
	/** Stored script preference for background loading of idle animations. */
	bool _loadFidgetsBg = true;
	/** Base drawing depth used by position-derived depth calculation. */
	int _depthBase = 0;
	/** Vertical origin used by position-derived depth calculation. */
	int _depthOriginY = 0;
	/** Reference anchor encoded by the component animation resources. */
	Common::Point _origin;
	/** Current character anchor in game pixels. */
	Common::Point _position;
	/** Home position used by character placement and return behavior. */
	Common::Point _home;
	/** Starting game-pixel position of the current walk segment. */
	Common::Point _start;
	/** Game-pixel displacement of the current walk segment. */
	Common::Point _delta;
	/** Length of the current walk segment in pixels. */
	float _distance = 0;
	/** Distance already covered within the current segment, in pixels. */
	float _travelled = 0;
	/** Millisecond timestamp of the previous movement step. */
	uint32 _lastMove = 0;
	/** Queued walk destinations or displacements in game pixels. */
	Common::Array<Common::Point> _path;
	/** Whether each corresponding path point is absolute rather than relative. */
	Common::Array<bool> _absolutePath;
	/** Callback notified when the queued walk completes. */
	WalkListener *_walkListener = nullptr;
	/** Whether completing the walk clears its queued path. */
	bool _clearWalkPath = true;
	/** Whether walk completion retains the final character position. */
	bool _keepWalkPosition = false;
	/** Index of the next queued walk point. */
	uint _nextPoint = 0;
	/** Sprites displaying the resting pose. */
	Common::Array<Shell::Sprite *> _base;
	/** Sprites displaying the current directional walk. */
	Common::Array<Shell::Sprite *> _moving;
	/** Sprites displaying the drag pose. */
	Common::Array<Shell::Sprite *> _dragParts;
	/** Optional resource resolver used instead of the default animation path. */
	const AnimationResources *_animRes = nullptr;
	/** Sprites displaying a scripted feedback animation. */
	Common::Array<Shell::Sprite *> _feedback;
	/** Whether feedback parts remain visible after playback completes. */
	bool _feedbackKeepVisible = true;
	/** Idle schedules, animation variants and non-repeat pools for the current pose. */
	Common::Array<Timer> _timers;
	/** Index of the timer whose idle animation is active, or minus one. */
	int _activeTimer = -1;
	/** Index of the active idle variant within its timer, or minus one. */
	int _activeVariant = -1;
	/** Whether the current character setup and required resources are valid. */
	bool _valid = true;

	/** Load an attribute-specific animation part, optionally as a movement sprite. */
	Shell::Sprite *loadPart(uint resource, uint attribute, bool movement = false);
	/** Remove all sprites in a component-part group. */
	void clearParts(Common::Array<Shell::Sprite *> &parts);
	/** Stop active idle variants and reset pose timer state. */
	void clearTimers(uint32 time);
	/** Draw the timer's next idle deadline from its second-based limits. */
	void schedule(Timer &timer, uint32 time);
	/** Build a pose timer and its attribute-dependent animation variants. */
	void addTimer(uint minimum, uint maximum, uint firstBase, int maskAttribute, uint secondBase, uint32 time);
	/** Show one timer variant and hide the parts it replaces. */
	void startVariant(uint timer, uint variant);
	/** Restore the resting pose and reschedule the completed idle animation. */
	void finishVariant(uint32 time);
	/** Load and begin a scripted feedback animation with its repeat and visibility policy. */
	bool playFeedback(uint resource, int repeat, bool keepVisible, int selector, uint32 time);
	/** Stop feedback playback and restore the appropriate pose parts. */
	void stopFeedback();
	/** Move the character anchor and all currently displayed parts. */
	void moveTo(Common::Point position);
	/** Resolve the next queued walk point and initialize its motion distance. */
	void beginSegment();
	/** Advance a component sprite group and append triggered sound IDs. */
	void updateParts(Common::Array<Shell::Sprite *> &parts, uint32 time, Common::Array<uint16> &sounds);
	/** Convert a millisecond timestamp to sixty-per-second idle timer ticks. */
	static uint32 ticks(uint32 time);
	/** Prevent accidental copies of this stateful object. */
	AttributeCharacter(const AttributeCharacter &) = delete;
	/** Prevent accidental copies of this stateful object. */
	AttributeCharacter &operator=(const AttributeCharacter &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
