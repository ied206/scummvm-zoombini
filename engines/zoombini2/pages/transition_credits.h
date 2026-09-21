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

#ifndef ZOOMBINI2_PAGES_TRANSITION_CREDITS_H
#define ZOOMBINI2_PAGES_TRANSITION_CREDITS_H

#include "zoombini2/pages/transition_base.h"

namespace Zoombini2 {

/**
 * End credits presentation with a pause, vertical scroll, and end hold in retail releases.
 * The demo presents a static background until the player releases the mouse button.
 * Dismissing the presentation or reaching the scroll endpoint exits the game.
 */
class TransitionCredits : public TransitionBase {
public:
	/** Bind the credits presentation to @p vm. */
	TransitionCredits(Zoombini2Engine *vm);

	/** Load resources and start the initial hold period. */
	void init() override;
	/** Advance initial hold, vertical scrolling, and final hold. */
	void onUpdate() override;
	/** Redraw the credits only when their scroll position changes. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Finish retail credits when the player presses the mouse button. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Finish demo credits when the player releases the mouse button. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;

private:
	/** Resource paths used by the credits presentation. */
	static constexpr const char *kBackgroundPath = "#bmp/credits/credits";
	static constexpr const char *kMusicPath = "#sounds/music/ZMR-Transition.wav";
	static constexpr const char *kDemoBackgroundPath = "bmp/credits/background";
	static constexpr const char *kDemoMusicPath = "sounds/music/01niv12.wav";

	/** Gameplay deadline for the active hold period. */
	uint32 _endTime = 0;
	/** Current vertical scroll offset. */
	float _scrollY = 0.0f;
	/** Largest vertical scroll offset exposed by the bitmap. */
	int _maxScrollY = 0;
	/** Whether the credits are currently scrolling. */
	bool _scrollActive = false;
	/** Whether @ref TransitionCredits::draw must update the screen. */
	bool _redrawNeeded = false;
	/** Gameplay tick used to calculate the next scroll delta. */
	uint32 _lastUpdateTime = 0;
	/** Whether the presentation remains in its initial hold. */
	bool _initialWait = false;

	/** Whether the credits have requested engine exit. */
	bool _finished = false;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_TRANSITION_CREDITS_H
