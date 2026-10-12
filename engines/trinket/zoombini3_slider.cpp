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

#include "trinket/zoombini3_slider.h"

namespace Trinket {
namespace Zoombini3 {

constexpr const char *OptionsSlider::kAsset;
constexpr uint OptionsSlider::kHoverDelay;

OptionsSlider::OptionsSlider(Shell::Scene &scene) : _scene(scene) {
}

OptionsSlider::~OptionsSlider() {
	clear();
}

void OptionsSlider::clear() {
	_scene.remove(_thumb);
	_scene.remove(_track);
	_scene.remove(_label);
	_track = _thumb = _label = nullptr;
	_tracking = false;
	_hoverAction = _dragOffset = 0;
	_minimum = _maximum = _position = 0;
}

bool OptionsSlider::load(Channel channel, int z, int actionBase) {
	clear();
	if (channel < kMusic00 || kEffects02 < channel || actionBase <= 0 || actionBase == INT_MAX)
		return false;
	static constexpr uint tracks[] = {
		426,
		429,
		440,
	};
	static constexpr uint labels[] = {
		425,
		428,
		432,
	};
	_track = _scene.add(Common::String::format(kAsset, tracks[channel]), z + 1, actionBase);
	_thumb = _scene.add(Common::String::format(kAsset, 427), z + 2, actionBase + 1);
	_label = _scene.add(Common::String::format(kAsset, labels[channel]), z);
	if (!_track || !_thumb || !_label) {
		clear();
		return false;
	}
	Shell::Sprite *parts[] = {
		_track,
		_thumb,
		_label,
	};
	for (uint i = 0; i < ARRAYSIZE(parts); i++) {
		parts[i]->managed = true;
		parts[i]->setFrame(1);
	}
	int width = _thumb->width(), height = _thumb->height();
	_minimum = _track->x + 10;
	_maximum = _track->x + _track->width() - width - (channel == kMusic00 ? 8 : 10);
	if (_maximum <= _minimum || width <= 0 || height <= 0) {
		clear();
		return false;
	}
	_position = _minimum;
	_thumb->x = _position;
	_thumb->y = _track->y - 8;
	// Hover artwork moves and grows; coverage remains the normal-frame rectangle.
	_thumb->hitRect = Common::Rect(_position, _thumb->y, _position + width, _thumb->y + height);
	return true;
}

void OptionsSlider::setPosition(int position) {
	if (!_thumb)
		return;
	position = CLIP(position, _minimum, _maximum);
	int delta = position - _position;
	_thumb->x += delta;
	_thumb->hitRect.translate(delta, 0);
	_position = position;
}

void OptionsSlider::setPercentage(int percentage) {
	setPosition(_minimum + CLIP(percentage, 0, 100) * (_maximum - _minimum) / 100);
}

int OptionsSlider::percentage() const {
	return _minimum < _maximum ? 100 * (_position - _minimum) / (_maximum - _minimum) : 0;
}

void OptionsSlider::setHover(int action, uint32 time) {
	if (_hoverAction == action)
		return;
	_hoverAction = action;
	_hoverStart = time;
	_hoverConsumed = false;
	uint frame = action ? 2 : 1;
	_track->setFrame(frame);
	_thumb->setFrame(frame);
	_label->setFrame(frame);
}

uint OptionsSlider::event(const Common::Event &event, uint32 time) {
	if (!_thumb || (event.type != Common::EVENT_MOUSEMOVE && event.type != Common::EVENT_LBUTTONDOWN && event.type != Common::EVENT_LBUTTONUP))
		return kNone00;
	int hit = _scene.hitTest(event.mouse);
	if (hit != _track->action && hit != _thumb->action)
		hit = 0;
	if (!_tracking)
		setHover(hit, time);
	if (event.type == Common::EVENT_LBUTTONDOWN) {
		if (hit == _thumb->action) {
			_tracking = true;
			_dragOffset = event.mouse.x - _position;
			_hoverConsumed = true;
		} else if (hit == _track->action) {
			setPosition(event.mouse.x);
			return kValueChanged01 | kActivated02;
		}
	} else if (_tracking && event.type == Common::EVENT_MOUSEMOVE) {
		setPosition(event.mouse.x - _dragOffset);
		// The native value link fires even when clamping leaves the value unchanged.
		return kValueChanged01;
	} else if (_tracking && event.type == Common::EVENT_LBUTTONUP) {
		_tracking = false;
		_dragOffset = 0;
		setHover(hit, time);
		return kActivated02;
	}
	return kNone00;
}

bool OptionsSlider::hoverExpired(uint32 time) {
	if (!_hoverAction || _hoverConsumed || time - _hoverStart < kHoverDelay)
		return false;
	_hoverConsumed = true;
	return true;
}

} // namespace Zoombini3
} // namespace Trinket
