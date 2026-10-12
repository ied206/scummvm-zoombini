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

/** Shared edge-scrolling task with borrowed registered trinkets. */
class Scroller : private Core::ScriptableObject::Observer {
public:
	Scroller(Scene &scene, const Common::Rect &viewport, int edgeWidth);
	~Scroller() { stop(); }
	void start(uint32 time);
	void stop();
	void add(Core::Trinket &trinket);
	void event(const Common::Event &event);
	void update(uint32 time);
	void freeze(bool frozen, uint32 time);
	int offset() const { return _offset; }
	int maximum() const { return _maximum; }

private:
	static constexpr int kSpeed = 200;
	Scene &_scene;
	const Common::Rect _viewport;
	const int _edgeWidth;
	Common::Array<Core::Trinket *> _trinkets;
	int _offset = 0, _maximum = 0, _direction = 0, _mouseX;
	uint32 _lastTime = 0, _freezeTime = 0;
	bool _active = false, _frozen = false;
	void objectDestroyed(Core::ScriptableObject &object) override;
	Scroller(const Scroller &) = delete;
	Scroller &operator=(const Scroller &) = delete;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
