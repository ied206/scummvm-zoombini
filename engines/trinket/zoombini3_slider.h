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

#ifndef TRINKET_ZOOMBINI3_SLIDER_H
#define TRINKET_ZOOMBINI3_SLIDER_H

#include "trinket/shell/scene.h"

namespace Trinket {
namespace Zoombini3 {

/** ROptions volume control. The containing scene must outlive the control. */
class OptionsSlider {
public:
	enum Channel {
		kMusic00 = 0,
		kDialogue01 = 1,
		kEffects02 = 2,
	};
	enum Notification {
		kNone00 = 0,
		kValueChanged01 = 1,
		kActivated02 = 2,
	};
	explicit OptionsSlider(Shell::Scene &scene);
	~OptionsSlider();
	bool load(Channel channel, int z, int actionBase);
	void setPercentage(int percentage);
	int percentage() const;
	uint event(const Common::Event &event, uint32 time);
	bool hoverExpired(uint32 time);
	bool isTracking() const { return _tracking; }

private:
	static constexpr const char *kAsset = "/z3options/%u.ao";
	static constexpr uint kHoverDelay = 2000;
	Shell::Scene &_scene;
	Shell::Sprite *_track = nullptr, *_thumb = nullptr, *_label = nullptr;
	int _minimum = 0, _maximum = 0, _position = 0, _dragOffset = 0, _hoverAction = 0;
	uint32 _hoverStart = 0;
	bool _tracking = false, _hoverConsumed = false;
	void clear();
	void setPosition(int position);
	void setHover(int action, uint32 time);
	OptionsSlider(const OptionsSlider &) = delete;
	OptionsSlider &operator=(const OptionsSlider &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
