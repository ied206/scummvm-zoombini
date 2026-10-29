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

#ifndef TRINKET_SHELL_SCENE_H
#define TRINKET_SHELL_SCENE_H

#include "common/events.h"
#include "common/hash-str.h"
#include "common/hashmap.h"
#include "trinket/core/animation.h"
#include "trinket/core/trinket.h"
#include "trinket/ompp/canvas.h"
#include "trinket/ompp/resource.h"
#include "trinket/ompp/resource_cache.h"

namespace Trinket {

/** Trinket Shell: Z3 trinket.dll, Reader Rabbit shelldll.dll, and launcher TLCLLShelldll.dll. */
namespace Shell {

class Scene;

class Sprite : public Core::Trinket, private Core::AnimationPlayer::PauseListener {
public:
	class UpdateListener {
	public:
		/** Allow destruction through the sprite-update interface. */
		virtual ~UpdateListener() {}
		/** Notify the listener before the sprite advances at the supplied millisecond time. */
		virtual void spriteUpdating(Sprite &sprite, uint32 time) = 0;
	};
	class AnimationListener {
	public:
		/** Allow destruction through the animation-listener interface. */
		virtual ~AnimationListener() {}
		/** Notify the listener that the sprite's requested playback completed. */
		virtual void animationFinished(Sprite &sprite) = 0;
	};
	/** Create an inactive sprite and declare its animation and pointer events. */
	Sprite();
	/** Release sprite playback state while retaining the scene's cached animation pictures. */
	~Sprite() override = default;
	/** Remove the sprite from its scene before deleting it. */
	void dispose() override;
	/** Nonzero input action returned when this sprite is activated. */
	int action = 0;
	/** Zero-based animation picture index selected for drawing. */
	uint frame = 0;
	/** Whether pointer presses select button-state pictures. */
	bool button = false;
	/** Whether the pointer currently holds this button down. */
	bool pressed = false;
	/** Whether pointer hover selects a separate button picture. */
	bool flyButton = false;
	/** Whether this sprite is currently under the pointer. */
	bool hovered = false;
	/** Whether hit testing uses the full bounds instead of picture transparency. */
	bool solid = false;
	/** Whether the explicit hit rectangle determines pointer hits. */
	bool boxTouchy = false;
	/** Whether pointer movement is reported while interacting with the sprite. */
	bool tracking = false;
	/** Game-pixel rectangle restricting the sprite's movement. */
	Common::Rect corral;
	/** Whether scene scrolling translates this sprite. */
	bool scrolling = false;
	/**
	 * Optional display recolor, applied only while the canvas enables color-blind assistance.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 */
	OM::Canvas::ColorRemap colorRemap = OM::Canvas::ColorRemap::kNone;
	/**
	 * Source-local recolor overrides for selected parts of a multicolored image.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 */
	Common::Array<OM::Canvas::ColorRemapRegion> colorRemapRegions;

	/** Dispatch the mouse-down link instead of a button's release hit. */
	bool actionOnMouseDown = false;
	/** Animation resource supplying the sprite's picture data and sequences. */
	const Core::Animation *asset = nullptr;
	/** Whether scene cleanup should also delete this sprite. */
	bool managed = false;
	/** Whether drawing reflects the sprite horizontally. */
	bool mirrored = false;
	/** Horizontal mirror axis in game pixels. */
	int mirrorX = 0;
	/** One-based button frame selected during hover. */
	uint hoverFrame = 3;
	/** One-based button frame selected while input is disabled. */
	uint disabledFrame = 2;
	/** Current picture rectangle in game pixels. */
	Common::Rect bounds;
	/** Explicit game-pixel rectangle used for box-based hit testing. */
	Common::Rect hitRect;
	/** Advance animation and pointer feedback, appending triggered sound IDs. */
	void update(uint32 time, Common::Array<uint16> &sounds);
	/** Set the callback invoked before each sprite update. */
	void setUpdateListener(UpdateListener *listener) { _updateListener = listener; }
	/** Start an authored sequence with the supplied repeat count. */
	bool startAnimation(int repeat = 0, uint sequence = 0);
	/** Seek to a one-based picture frame and synchronize the displayed sprite. */
	void setFrame(int number);
	/** Return the one-based frame at the animation's current sequence position. */
	uint getFrame() const { return _player.getFrame(); }
	/** Return the one-based frame whose picture is currently displayed. */
	uint displayedFrame() const { return _player.displayedFrame(); }
	/** Return the number of picture frames in the selected sequence. */
	uint getNumberOfFrames() const { return _player.getNumberOfFrames(); }
	/** Override the playback rate, using entry durations when the rate is zero. */
	void setFPS(uint fps) { _player.setFPS(fps); }
	/** Change the remaining playback repeat count. */
	void setRepeat(int repeat) { _player.setRepeat(repeat); }
	/** Set the callback for authored animation pause markers. */
	void setPauseListener(Core::AnimationPlayer::PauseListener *listener) { _player.setPauseListener(listener); }
	/** Reverse the picture order of the current playback sequence. */
	bool reverseSequence();
	/** Install replacement playback entries and synchronize the selected frame. */
	bool replaceSequence(const Common::Array<Core::Animation::Step> &steps);
	/** Choose whether playback holds the last picture instead of disappearing. */
	void setPauseOnLastFrame(bool pauseAtEnd);
	/** Return whether playback is configured to hold its last picture. */
	bool pausesOnLastFrame() const { return _pauseOnLastFrame; }
	/** Set the callback notified when the requested playback finishes. */
	void setAnimationListener(AnimationListener *listener) { _animListener = listener; }
	/** Detach the supplied callback if it is still registered. */
	void removeAnimationListener(AnimationListener &listener) {
		if (_animListener == &listener)
			_animListener = nullptr;
	}
	/** Resume script-requested animation from its current position. */
	void playAnimation();
	/** Stop animation and synchronize the resulting sprite state. */
	void stopAnimation();
	/** Pause or resume animation while retaining the displayed picture. */
	void pauseAnimation(bool paused) {
		_explicitlyPaused = paused;
		_player.pause(paused);
	}
	/** Return whether animation is enabled and its player is advancing. */
	bool isRunning() const { return _animated && _player.isRunning(); }
	/** Return the current picture width in pixels, decoding on first use. */
	int width() override { return cachePicture() ? _picture->w : 0; }
	/** Return the current picture height in pixels, decoding on first use. */
	int height() override { return cachePicture() ? _picture->h : 0; }
	/** Read sprite animation, interaction or inherited geometry properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign sprite animation, interaction or inherited geometry properties. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch sprite playback, frame-selection and button script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Create a sprite with the same resource, geometry and interaction settings. */
	Core::ScriptableObject *clone() const override;
	/** Check whether the sprite accepts a hit at the supplied game-pixel position. */
	bool covers(Common::Point point);
	/** Return and clear a sound resource queued by pointer interaction. */
	bool pollClickSound(Common::String &resource);
	/** Start button feedback at a millisecond timestamp, optionally simulating a pointer press. */
	void buttonPressed(uint32 time, bool synthetic = false);
	/** Select hover feedback and queue its configured sound. */
	void flyOn();

private:
	/** Script event name for hit. */
	static constexpr const char *kEventHit = "hit";
	/** Script event name for anim Done. */
	static constexpr const char *kEventAnimDone = "animDone";
	/** Script event name for paused. */
	static constexpr const char *kEventPaused = "paused";
	/** Script event name for start Tracking. */
	static constexpr const char *kEventStartTracking = "startTracking";
	/** Script event name for end Tracking. */
	static constexpr const char *kEventEndTracking = "endTracking";
	/** Script property name for user Data. */
	static constexpr const char *kPropertyUserData = "userData";
	/** Script property name for enabled. */
	static constexpr const char *kPropertyEnabled = "enabled";
	/** Script property name for is Solid. */
	static constexpr const char *kPropertyIsSolid = "isSolid";
	/** Script property name for box Touchy. */
	static constexpr const char *kPropertyBoxTouchy = "boxTouchy";
	/** Script property name for tracking. */
	static constexpr const char *kPropertyTracking = "tracking";
	/** Script property name for click Sound ID. */
	static constexpr const char *kPropertyClickSoundID = "clickSoundID";
	/** Script property name for fly On Sound ID. */
	static constexpr const char *kPropertyFlyOnSoundID = "flyOnSoundID";
	/** Script property name for play Delay. */
	static constexpr const char *kPropertyPlayDelay = "playDelay";
	/** Script property name for frame. */
	static constexpr const char *kPropertyFrame = "frame";
	/** Script property name for frame Count. */
	static constexpr const char *kPropertyFrameCount = "frameCount";
	/** Script property name for pause On Last Frame. */
	static constexpr const char *kPropertyPauseOnLastFrame = "pauseOnLastFrame";
	/** Script property name for click Controller FXID. */
	static constexpr const char *kPropertyClickControllerFXID = "clickControllerFXID";
	/** Script property name for fly On Controller FXID. */
	static constexpr const char *kPropertyFlyOnControllerFXID = "flyOnControllerFXID";
	/** Script property name for corral. */
	static constexpr const char *kPropertyCorral = "corral";
	/** Script property name for repeat Count. */
	static constexpr const char *kPropertyRepeatCount = "repeatCount";
	/** Script property name for z. */
	static constexpr const char *kPropertyZ = "z";
	/** Script method name for set Fly Off Rect. */
	static constexpr const char *kMethodSetFlyOffRect = "setFlyOffRect";
	/** Script method name for default. */
	static constexpr const char *kMethodDefault = "default";
	/** Script method name for forever. */
	static constexpr const char *kMethodForever = "forever";
	/** Script method name for play. */
	static constexpr const char *kMethodPlay = "play";
	/** Script method name for resume. */
	static constexpr const char *kMethodResume = "resume";
	/** Script method name for pause. */
	static constexpr const char *kMethodPause = "pause";
	/** Script identifier for -1. */
	static constexpr const char *k1Name = "-1";

	friend class Scene;
	/** Scene containing this sprite, or null while detached. */
	Scene *_scene = nullptr;
	/** Callback invoked before the sprite advances. */
	UpdateListener *_updateListener = nullptr;
	/** Callback notified when script-requested animation finishes. */
	AnimationListener *_animListener = nullptr;
	/** Sequence position, timing and control-marker state for animation playback. */
	Core::AnimationPlayer _player;
	/** Whether the sprite has active animation-player state. */
	bool _animated = false;
	/** Whether playback was requested through the script interface. */
	bool _scriptPlaying = false;
	/** Whether playback should retain its final picture. */
	bool _pauseOnLastFrame = false;
	/** Whether a script pause should prevent automatic playback resumption. */
	bool _explicitlyPaused = false;
	/** Picture offset already applied to the sprite's game-pixel position. */
	Common::Point _animOffset;
	/** Current decoded picture displayed by the sprite. */
	const Graphics::Surface *_picture = nullptr;
	/** Last picture index used to refresh bounds and button state. */
	int _cachedFrame = -1;
	/** Arbitrary script text attached to the sprite. */
	Common::String _userData;
	/** Resource name played when the sprite is pressed, or minus one when unset. */
	Common::String _clickSoundID = "-1";
	/** Resource name played on pointer entry, or minus one when unset. */
	Common::String _flyOnSoundID = "-1";
	/** Sound resource waiting to be consumed by the scene's input handler. */
	Common::String _pendingSound;
	/** Sound-controller effect ID used for press feedback, or minus one when unset. */
	int _clickControllerFXID = -1;
	/** Sound-controller effect ID used for hover feedback, or minus one when unset. */
	int _flyOnControllerFXID = -1;
	/** Milliseconds to retain simulated button-press feedback. */
	uint32 _playDelay = 250;
	/** Millisecond timestamp when the current press feedback began. */
	uint32 _pressTime = 0;
	/** Whether button feedback was triggered without a held physical button. */
	bool _syntheticPress = false;
	/** Whether a scripted click still needs to emit its input action. */
	bool _clickPending = false;
	/** Resolve the current state to a shared picture for rendering and hit testing. */
	bool cachePicture();
	/** Apply the player's picture and offset to sprite geometry and visibility. */
	void syncAnimation();
	/** Dispatch pause and completion callbacks while checking whether the sprite survives them. */
	bool animationPaused() override;
	/** Prevent accidental copies of this stateful object. */
	Sprite(const Sprite &) = delete;
	/** Prevent accidental copies of this stateful object. */
	Sprite &operator=(const Sprite &) = delete;
};

/**
 * Load scene images and animations, display sprites and route input; the game dispatches callbacks.
 * @ref Trinket::OM::ResourceCache retains source bytes; sprites using the same path share an animation asset.
 * Each sprite keeps its own position and playback state.
 */
class Scene {
public:
	/** Create a scene that loads images and animations from the supplied resource provider. */
	explicit Scene(const OM::ResourceProvider &resources) : _resources(resources) {}
	/** Clear sprites, decoded resources and background pixels. */
	~Scene();
	/** Remove scene sprites and reset the background, input and scrolling state. */
	void clear();
	/** Load a background resource and replace the displayed background on success. */
	bool setBackground(const Common::String &path);
	/** Return a cached animation, valid until scene clear or unused-resource collection. */
	const Core::Animation *loadAnimation(const Common::String &path);
	/** Load or reuse an animation and register a new @ref Trinket::Shell::Sprite for drawing and input. */
	Sprite *add(const Common::String &path, int z, int action = 0, bool button = false);
	/** Detach a sprite from drawing and input lists. */
	void remove(Sprite *sprite);
	/** Change a sprite's depth and reinsert it in drawing order. */
	void setDepth(Sprite *sprite, int z);
	/** Dispatch pointer input within the supplied depth range and return the triggered action. */
	int event(const Common::Event &event, int minimumZ = INT_MIN, int maximumZ = INT_MAX);
	/** Return the action whose sprite is currently showing press feedback. */
	int pressFeedback() const { return _pressFeedback; }
	/** Draw the background followed by visible sprites in depth order. */
	void draw(OM::Canvas &canvas);
	/** Draw the background with the current horizontal scroll offset. */
	void drawBackground(OM::Canvas &canvas);
	/** Draw visible sprites within an inclusive depth range. */
	void drawSprites(OM::Canvas &canvas, int minimumZ, int maximumZ);
	/** Return the topmost input action at a game-pixel point, or zero if none is hit. */
	int hitTest(Common::Point point, int minimumZ = INT_MIN, int maximumZ = INT_MAX) const;
	/** Advance sprites at a millisecond timestamp and collect their triggered sounds. */
	void update(uint32 time, Common::Array<uint16> &sounds);
	/** Return the topmost eligible sprite at a game-pixel point, or null if none is hit. */
	Sprite *hitSprite(Common::Point point, int minimumZ = INT_MIN, int maximumZ = INT_MAX, bool requireAction = false) const;
	/** Discard cached animation resources no longer referenced by scene sprites. */
	void uncacheUnusedResources();
	/** Return the supplied script object if it is one of this scene's sprites. */
	Sprite *resolveSprite(const Core::ScriptableObject *object) const;
	/** Set the horizontal background offset in game pixels. */
	void setScrollX(int x) { _scrollX = x; }
	/** Return the active background width in pixels, or zero without a background. */
	int backgroundWidth() const { return _background ? _background->surface.w : 0; }
	/** Describe retained payloads, retained-asset decode work and lifetime request counters. */
	Common::String cacheDescription() const;

private:
	/** Script identifier for z. */
	static constexpr const char *kZName = "z";

	/** Extension of packed 16-bit background images. */
	static constexpr const char *kBackgroundExtension = ".bk16";
	/** Extension of animation sequences. */
	static constexpr const char *kAnimationExtension = ".ao";
	/** Extension of external animation palettes. */
	static constexpr const char *kPaletteExtension = ".rgb";

	friend class Sprite;
	/** Scene resource cache used to resolve backgrounds and animations. */
	OM::ResourceCache _resources;
	/** Sprites in drawing and input depth order. */
	Common::Array<Sprite *> _sprites;
	/** Cached animation assets keyed by animation and default-palette resource identities. */
	Common::HashMap<Common::String, Core::Animation *> _assets;
	/** Decoded background pixels and their resource identity. */
	struct Background {
		Graphics::Surface surface;        ///< Decoded background pixels used for scene drawing.
		~Background() { surface.free(); } ///< Release the pixel allocation.
	};
	/** Decoded backgrounds retained until scene clear or unused-resource collection. */
	Common::HashMap<Common::String, Common::SharedPtr<Background>> _backgrounds;
	/** Active background, retained independently of unused entries. */
	Common::SharedPtr<Background> _background;
	/** Successful background decodes over this scene instance's lifetime. */
	uint64 _bgDecodes = 0;
	/** Repeated background requests served without decoding. */
	uint64 _backgroundHits = 0;
	/** Input action selected when the current pointer press began. */
	int _pressedAction = 0;
	/** Action whose sprite currently displays pressed feedback. */
	int _pressFeedback = 0;
	/** Horizontal background offset in game pixels. */
	int _scrollX = 0;
	/** Whether the scene is traversing its sprite update list. */
	bool _updating = false;
	/** Index of the next sprite to update, adjusted when callbacks remove sprites. */
	uint _nextUpdate = 0;
	/** Insert a sprite at its drawing depth without changing the order of equal-depth peers. */
	void insert(Sprite *sprite);
	/** Prevent accidental copies of this stateful object. */
	Scene(const Scene &) = delete;
	/** Prevent accidental copies of this stateful object. */
	Scene &operator=(const Scene &) = delete;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
