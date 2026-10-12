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
		virtual ~UpdateListener() {}
		virtual void spriteUpdating(Sprite &sprite, uint32 time) = 0;
	};
	class AnimationListener {
	public:
		virtual ~AnimationListener() {}
		virtual void animationFinished(Sprite &sprite) = 0;
	};
	Sprite();
	~Sprite() override { _picture.free(); }
	void dispose() override;
	int action = 0;
	uint frame = 0;
	bool button = false, pressed = false;
	bool flyButton = false, hovered = false;
	bool solid = false, boxTouchy = false, tracking = false;
	Common::Rect corral;
	bool scrolling = false;

	/** Dispatch the mouse-down link instead of a button's release hit. */
	bool actionOnMouseDown = false;
	const Core::Animation *asset = nullptr;
	bool managed = false, mirrored = false;
	int mirrorX = 0;
	uint hoverFrame = 3;
	uint disabledFrame = 2;
	Common::Rect bounds;
	Common::Rect hitRect;
	void update(uint32 time, Common::Array<uint16> &sounds);
	void setUpdateListener(UpdateListener *listener) { _updateListener = listener; }
	bool startAnimation(int repeat = 0, uint sequence = 0);
	void setFrame(int number);
	uint getFrame() const { return _player.getFrame(); }
	uint displayedFrame() const { return _player.displayedFrame(); }
	uint getNumberOfFrames() const { return _player.getNumberOfFrames(); }
	void setFPS(uint fps) { _player.setFPS(fps); }
	void setRepeat(int repeat) { _player.setRepeat(repeat); }
	void setPauseListener(Core::AnimationPlayer::PauseListener *listener) { _player.setPauseListener(listener); }
	bool reverseSequence();
	bool replaceSequence(const Common::Array<Core::Animation::Step> &steps);
	void setPauseOnLastFrame(bool pauseAtEnd);
	bool pausesOnLastFrame() const { return _pauseOnLastFrame; }
	void setAnimationListener(AnimationListener *listener) { _animationListener = listener; }
	void removeAnimationListener(AnimationListener &listener) {
		if (_animationListener == &listener)
			_animationListener = nullptr;
	}
	void playAnimation();
	void stopAnimation();
	void pauseAnimation(bool paused) {
		_explicitlyPaused = paused;
		_player.pause(paused);
	}
	bool isRunning() const { return _animated && _player.isRunning(); }
	int width() override { return cachePicture() ? _picture.w : 0; }
	int height() override { return cachePicture() ? _picture.h : 0; }
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	Core::ScriptableObject *clone() const override;
	bool covers(Common::Point point);
	bool pollClickSound(Common::String &resource);
	void buttonPressed(uint32 time, bool synthetic = false);
	void flyOn();

private:
	friend class Scene;
	Scene *_scene = nullptr;
	UpdateListener *_updateListener = nullptr;
	AnimationListener *_animationListener = nullptr;
	Core::AnimationPlayer _player;
	bool _animated = false;
	bool _scriptPlaying = false, _pauseOnLastFrame = false, _explicitlyPaused = false;
	Common::Point _animationOffset;
	Graphics::Surface _picture;
	int _cachedFrame = -1;
	Common::String _userData;
	Common::String _clickSoundID = "-1", _flyOnSoundID = "-1", _pendingSound;
	int _clickControllerFXID = -1, _flyOnControllerFXID = -1;
	uint32 _playDelay = 250, _pressTime = 0;
	bool _syntheticPress = false, _clickPending = false;
	bool cachePicture();
	void syncAnimation();
	bool animationPaused() override;
	Sprite(const Sprite &) = delete;
	Sprite &operator=(const Sprite &) = delete;
};

/** Owns scene images and visual objects; callbacks are dispatched by the game. */
class Scene {
public:
	explicit Scene(const OM::ResourceProvider &resources) : _resources(resources) {}
	~Scene();
	void clear();
	bool setBackground(const Common::String &path);
	Sprite *add(const Common::String &path, int z, int action = 0, bool button = false);
	void remove(Sprite *sprite);
	void setDepth(Sprite *sprite, int z);
	int event(const Common::Event &event, int minimumZ = INT_MIN, int maximumZ = INT_MAX);
	int pressFeedback() const { return _pressFeedback; }
	void draw(OM::Canvas &canvas);
	void drawBackground(OM::Canvas &canvas);
	void drawSprites(OM::Canvas &canvas, int minimumZ, int maximumZ);
	int hitTest(Common::Point point, int minimumZ = INT_MIN, int maximumZ = INT_MAX) const;
	void update(uint32 time, Common::Array<uint16> &sounds);
	Sprite *hitSprite(Common::Point point, int minimumZ = INT_MIN, int maximumZ = INT_MAX, bool requireAction = false) const;
	void uncacheUnusedResources();
	Sprite *resolveSprite(const Core::ScriptableObject *object) const;
	void setScrollX(int x) { _scrollX = x; }
	int backgroundWidth() const { return _background.w; }

private:
	friend class Sprite;
	OM::ResourceCache _resources;
	Common::Array<Sprite *> _sprites;
	Common::HashMap<Common::String, Core::Animation *> _assets;
	Graphics::Surface _background;
	int _pressedAction = 0;
	int _pressFeedback = 0;
	int _scrollX = 0;
	bool _updating = false;
	uint _nextUpdate = 0;
	void insert(Sprite *sprite);
	Scene(const Scene &) = delete;
	Scene &operator=(const Scene &) = delete;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
