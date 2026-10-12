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

#include "trinket/zoombini3_checkbox.h"

namespace Trinket {
namespace Zoombini3 {

constexpr const char *OptionsCheckbox::kAsset;

bool OptionsCheckbox::load(int z, int action, bool on, bool practice) {
	_scene.remove(_sprite);
	_sprite = nullptr;
	_tracking = false;
	if (action <= 0)
		return false;
	_sprite = _scene.add(kAsset, z, action);
	if (!_sprite)
		return false;
	_sprite->managed = true;
	_on = on;
	showState(false);
	if (practice)
		_sprite->y -= _sprite->height();
	return true;
}

void OptionsCheckbox::showState(bool hover) {
	if (_sprite)
		_sprite->setFrame((_on ? 1 : 3) + (hover ? 1 : 0));
}

void OptionsCheckbox::setOn(bool on, Common::Point pointer) {
	_on = on;
	showState(false);
	if (_sprite)
		showState(_scene.hitTest(pointer) == _sprite->action);
}

uint OptionsCheckbox::event(const Common::Event &event) {
	if (!_sprite || (event.type != Common::EVENT_MOUSEMOVE && event.type != Common::EVENT_LBUTTONDOWN && event.type != Common::EVENT_LBUTTONUP))
		return kNone00;
	bool inside = _scene.hitTest(event.mouse) == _sprite->action;
	if (event.type == Common::EVENT_LBUTTONDOWN) {
		if (inside) {
			_tracking = true;
			return kClickSound02;
		}
	} else if (event.type == Common::EVENT_LBUTTONUP && _tracking) {
		_tracking = false;
		if (inside) {
			setOn(!_on, event.mouse);
			return kClicked01;
		}
	} else if (event.type == Common::EVENT_MOUSEMOVE && _tracking) {
		if (!inside) {
			showState(false);
		} else if (_sprite->getFrame() == 1 || _sprite->getFrame() == 3) {
			// Native tracking restarts the click sound when moving over a normal frame.
			return kClickSound02;
		}
		return kNone00;
	}
	if (!_tracking)
		showState(inside);
	return kNone00;
}

} // namespace Zoombini3
} // namespace Trinket
