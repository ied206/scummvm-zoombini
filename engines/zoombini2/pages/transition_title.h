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

#ifndef ZOOMBINI2_PAGES_TRANSITION_TITLE_H
#define ZOOMBINI2_PAGES_TRANSITION_TITLE_H

#include "zoombini2/pages/transition_base.h"

namespace Zoombini2 {

/** Load the title background and wait for input or the timeout. */
class TransitionTitle : public TransitionBase {
public:
	/** Bind the title screen to @p vm. */
	TransitionTitle(Zoombini2Engine *vm);

	/** Load the background, start music, and establish the timeout. */
	void init() override;
	/** Request the sign-in page after input or timeout. */
	void onUpdate() override;
	/** Draw the title background. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Mark the title screen as dismissed by the player. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;

private:
	/** Resource paths used by the title screen. */
	static constexpr const char *kBackgroundPath = "bmp/story_intro/title_screen";
	static constexpr const char *kMusicPath = "#sounds/music/Booliewood_Level1.wav";
	static constexpr const char *kDemoMusicPath = "sounds/music/I_BB1.wav";

	EventHandleResult dismiss();
	/** Whether the player clicked to dismiss the title screen. */
	bool _clicked = false;
	/** Gameplay deadline for automatic dismissal. */
	uint32 _deadline = 0;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_TRANSITION_TITLE_H
