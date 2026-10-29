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
	/** Connect an audio-volume slider to the options scene. */
	explicit OptionsSlider(Shell::Scene &scene);
	/** Remove slider track, thumb and label sprites. */
	~OptionsSlider();
	/** Create a channel's slider with its depth and input-action range. */
	bool load(Channel channel, int z, int actionBase);
	/** Set the slider value as a zero-to-one-hundred percentage. */
	void setPercentage(int percentage);
	/** Return the selected volume percentage. */
	int percentage() const;
	/** Handle slider dragging and timed hover feedback. */
	uint event(const Common::Event &event, uint32 time);
	/** Report a newly elapsed hover delay once for the current control. */
	bool hoverExpired(uint32 time);
	/** Return whether the pointer is currently dragging the thumb. */
	bool isTracking() const { return _tracking; }

private:
	/** Animation path pattern for volume slider resources. */
	static constexpr const char *kAsset = "/z3options/%u.ao";
	/** Milliseconds before a slider hover prompt becomes due. */
	static constexpr uint kHoverDelay = 2000;
	/** Scene displaying the slider sprites. */
	Shell::Scene &_scene;
	/** Sprite displaying the slider track. */
	Shell::Sprite *_track = nullptr;
	/** Sprite displaying the draggable slider thumb. */
	Shell::Sprite *_thumb = nullptr;
	/** Sprite identifying the audio channel controlled by the slider. */
	Shell::Sprite *_label = nullptr;
	/** Minimum thumb position along the slider track, in pixels. */
	int _minimum = 0;
	/** Maximum thumb position along the slider track, in pixels. */
	int _maximum = 0;
	/** Current thumb position along the slider track, in pixels. */
	int _position = 0;
	/** Pointer displacement from the thumb position during dragging. */
	int _dragOffset = 0;
	/** Input action whose timed hover feedback is being tracked. */
	int _hoverAction = 0;
	/** Millisecond timestamp when the current hover began. */
	uint32 _hoverStart = 0;
	/** Whether the slider thumb is currently being dragged. */
	bool _tracking = false;
	/** Whether feedback for the current hover has already been reported. */
	bool _hoverConsumed = false;
	/** Remove all slider sprites and reset interaction state. */
	void clear();
	/** Clamp the thumb position and update its sprite. */
	void setPosition(int position);
	/** Begin or clear timed hover feedback for a control action. */
	void setHover(int action, uint32 time);
	/** Prevent accidental copies of this stateful object. */
	OptionsSlider(const OptionsSlider &) = delete;
	/** Prevent accidental copies of this stateful object. */
	OptionsSlider &operator=(const OptionsSlider &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
