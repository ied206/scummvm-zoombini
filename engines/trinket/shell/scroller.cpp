/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/shell/scroller.h"

#include "common/algorithm.h"

namespace Trinket {
namespace Shell {

Scroller::Scroller(Scene &scene, const Common::Rect &viewport, int edgeWidth) : _scene(scene), _viewport(viewport), _edgeWidth(edgeWidth), _mouseX(viewport.left + viewport.width() / 2) {
	assert(!viewport.isEmpty() && 0 <= edgeWidth && edgeWidth <= viewport.width() / 2);
}

void Scroller::start(uint32 time) {
	stop();
	_active = true;
	_offset = _direction = 0;
	_maximum = MAX(0, _scene.backgroundWidth() - _viewport.width());
	_lastTime = time;
	_scene.setScrollX(0);
}

void Scroller::stop() {
	for (uint i = 0; i < _trinkets.size(); i++)
		_trinkets[i]->removeObserver(*this);
	_trinkets.clear();
	_active = _frozen = false;
}

void Scroller::add(Core::Trinket &trinket) {
	for (uint i = 0; i < _trinkets.size(); i++)
		if (_trinkets[i] == &trinket)
			return;
	trinket.addObserver(*this);
	_trinkets.push_back(&trinket);
}

void Scroller::objectDestroyed(Core::ScriptableObject &object) {
	for (uint i = 0; i < _trinkets.size(); i++)
		if (_trinkets[i] == &object) {
			_trinkets.remove_at(i);
			return;
		}
}

void Scroller::event(const Common::Event &event) {
	if (event.type == Common::EVENT_MOUSEMOVE)
		_mouseX = event.mouse.x;
}

void Scroller::freeze(bool frozen, uint32 time) {
	if (_frozen == frozen)
		return;
	_frozen = frozen;
	if (frozen)
		_freezeTime = time;
	else
		_lastTime += time - _freezeTime;
}

void Scroller::update(uint32 time) {
	if (!_active || _frozen)
		return;
	int direction = 0;
	if (_mouseX < _viewport.left + _edgeWidth)
		direction = -1;
	else if (_viewport.right - _edgeWidth < _mouseX)
		direction = 1;
	if (!direction) {
		_direction = 0;
		return;
	}
	if (direction != _direction)
		_lastTime = time;
	_direction = direction;
	int distance = static_cast<int32>(static_cast<double>(static_cast<int32>(time - _lastTime)) * 0.001 * kSpeed);
	if (!distance)
		return;
	int offset = CLIP(_offset + direction * distance, 0, _maximum);
	int translation = _offset - offset;
	_offset = offset;
	_lastTime = time;
	_scene.setScrollX(offset);
	Core::ScriptArguments arguments;
	for (uint i = 0; i < _trinkets.size(); i++) {
		Core::Trinket &trinket = *_trinkets[i];
		trinket.set("x", arguments, Core::ScriptValue(trinket.x + translation));
		Common::Rect bounds(trinket.x, trinket.y, trinket.x + trinket.width(), trinket.y + trinket.height());
		trinket.visible = bounds.intersects(_viewport);
	}
}

} // namespace Shell
} // namespace Trinket
