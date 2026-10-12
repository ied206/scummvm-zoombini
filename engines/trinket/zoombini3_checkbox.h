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

#ifndef TRINKET_ZOOMBINI3_CHECKBOX_H
#define TRINKET_ZOOMBINI3_CHECKBOX_H

#include "trinket/shell/scene.h"

namespace Trinket {
namespace Zoombini3 {

/** The ROptions Help Speech control; its scene must outlive it. */
class OptionsCheckbox {
public:
	enum Notification {
		kNone00 = 0,
		kClicked01 = 1,
		kClickSound02 = 2,
	};
	explicit OptionsCheckbox(Shell::Scene &scene) : _scene(scene) {}
	~OptionsCheckbox() { _scene.remove(_sprite); }
	bool load(int z, int action, bool on, bool practice);
	uint event(const Common::Event &event);
	bool isOn() const { return _on; }
	int height() const { return _sprite ? _sprite->height() : 0; }
	void setOn(bool on, Common::Point pointer);

private:
	static constexpr const char *kAsset = "/z3options/422.ao";
	Shell::Scene &_scene;
	Shell::Sprite *_sprite = nullptr;
	bool _on = true, _tracking = false;
	void showState(bool hover);
	OptionsCheckbox(const OptionsCheckbox &) = delete;
	OptionsCheckbox &operator=(const OptionsCheckbox &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
