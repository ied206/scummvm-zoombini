/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_SHELL_SCROLLER_H
#define TRINKET_SHELL_SCROLLER_H

#include "trinket/core/trinket.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace Shell {

/** Scroll registered trinkets when the pointer reaches a scene edge. */
class Scroller : private Core::ScriptableObject::Observer {
public:
	/** Configure horizontal edge scrolling for a scene viewport. */
	Scroller(Scene &scene, const Common::Rect &viewport, int edgeWidth);
	/** Stop scrolling and detach from tracked scene objects. */
	~Scroller() { stop(); }
	/** Reset the scroll offset and begin edge tracking at a millisecond timestamp. */
	void start(uint32 time);
	/** Stop scrolling and stop tracking scene objects. */
	void stop();
	/** Track an object whose position and visibility follow horizontal scrolling. */
	void add(Core::Trinket &trinket);
	/** Record pointer motion used to choose the scrolling direction. */
	void event(const Common::Event &event);
	/** Advance horizontal scrolling at the supplied millisecond timestamp. */
	void update(uint32 time);
	/** Freeze or unfreeze scrolling while excluding the paused interval from motion. */
	void freeze(bool frozen, uint32 time);
	/** Return the horizontal background offset in game pixels. */
	int offset() const { return _offset; }
	/** Return the largest horizontal offset allowed by the background width. */
	int maximum() const { return _maximum; }

private:
	/** Script property name for x. */
	static constexpr const char *kPropertyX = "x";

	/** Horizontal scrolling speed in game pixels per second. */
	static constexpr int kSpeed = 200;
	/** Scene whose background and objects are scrolled. */
	Scene &_scene;
	/** Visible game-pixel rectangle used for clipping and edge detection. */
	const Common::Rect _viewport;
	/** Width in pixels of the left and right scrolling zones. */
	const int _edgeWidth;
	/** Tracked objects translated and clipped when the background scrolls. */
	Common::Array<Core::Trinket *> _trinkets;
	/** Current horizontal background offset in game pixels. */
	int _offset = 0;
	/** Maximum horizontal background offset in game pixels. */
	int _maximum = 0;
	/** Horizontal scrolling direction: left, stopped or right. */
	int _direction = 0;
	/** Most recent pointer x coordinate in game pixels. */
	int _mouseX;
	/** Millisecond timestamp used to calculate the next scrolling step. */
	uint32 _lastTime = 0;
	/** Millisecond timestamp when scrolling was frozen. */
	uint32 _freezeTime = 0;
	/** Whether edge scrolling is enabled for this scene. */
	bool _active = false;
	/** Whether active scrolling is temporarily suspended. */
	bool _frozen = false;
	/** Remove a destroyed object from the scrolling list. */
	void objectDestroyed(Core::ScriptableObject &object) override;
	/** Prevent accidental copies of this stateful object. */
	Scroller(const Scroller &) = delete;
	/** Prevent accidental copies of this stateful object. */
	Scroller &operator=(const Scroller &) = delete;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
