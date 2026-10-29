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
	/** Connect a music checkbox to the options scene. */
	explicit OptionsCheckbox(Shell::Scene &scene) : _scene(scene) {}
	/** Remove the checkbox sprite from the scene. */
	~OptionsCheckbox() { _scene.remove(_sprite); }
	/** Create the checkbox with its action, initial state and practice-mode appearance. */
	bool load(int z, int action, bool on, bool practice);
	/** Handle pointer interaction and report a checkbox state change. */
	uint event(const Common::Event &event);
	/** Return whether the checkbox is selected. */
	bool isOn() const { return _on; }
	/** Return the checkbox sprite's height in pixels, or zero before loading. */
	int height() const { return _sprite ? _sprite->height() : 0; }
	/** Set the selected state and refresh feedback for the current pointer position. */
	void setOn(bool on, Common::Point pointer);

private:
	/** Resource path of the options checkbox animation. */
	static constexpr const char *kAsset = "/z3options/422.ao";
	/** Scene displaying the checkbox sprite. */
	Shell::Scene &_scene;
	/** Sprite whose frame displays checkbox state and pointer feedback. */
	Shell::Sprite *_sprite = nullptr;
	/** Whether the checkbox is selected. */
	bool _on = true;
	/** Whether a pointer press began on this checkbox. */
	bool _tracking = false;
	/** Select the sprite frame for the selected and hover states. */
	void showState(bool hover);
	/** Prevent accidental copies of this stateful object. */
	OptionsCheckbox(const OptionsCheckbox &) = delete;
	/** Prevent accidental copies of this stateful object. */
	OptionsCheckbox &operator=(const OptionsCheckbox &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
