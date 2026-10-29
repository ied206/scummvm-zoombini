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

#ifndef TRINKET_ZOOMBINI3_QUIT_H
#define TRINKET_ZOOMBINI3_QUIT_H

#include "trinket/ompp/sound.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace Zoombini3 {

/** The game's RDialog quit confirmation, including its hover illustrations. */
class QuitDialog {
public:
	enum Result {
		kPending = -1,
		kQuit00 = 0,
		kResume01 = 1,
	};
	/** Connect quit-confirmation sprites and audio to shared resources. */
	QuitDialog(const OM::ResourceProvider &resources, Audio::Mixer &mixer);
	/** Create the quit panel, character feedback and choice buttons. */
	bool load();
	/** Handle quit-confirmation input and return the selected result. */
	Result event(const Common::Event &event);
	/** Check whether a game-pixel point hits a quit-dialog button. */
	bool overButton(Common::Point point) const { return _scene.hitTest(point) != 0; }
	/** Advance and draw quit-confirmation animations at a millisecond timestamp. */
	void draw(OM::Canvas &canvas, uint32 time);

private:
	/** Sound resource IDs used by the quit dialog. */
	enum PageResourceId : int16 {
		kResSoundCommon0001_Click = 1,
	};
	/** Animation path pattern for shared quit-dialog resources. */
	static constexpr const char *kAsset = "/z3common/%u.ao";
	/** Sound path pattern for shared quit-dialog resources. */
	static constexpr const char *kSound = "/z3common/%u.wav";
	/** Provider used to load quit-dialog animations and audio. */
	const OM::ResourceProvider &_resources;
	/** Sprites displayed by the quit-confirmation panel. */
	Shell::Scene _scene;
	/** Character idle animation shown in the confirmation panel. */
	Shell::Sprite *_idle = nullptr;
	/** Panel and character feedback sprites. */
	Shell::Sprite *_graphics[2];
	/** Confirmation and cancellation button sprites. */
	Shell::Sprite *_buttons[2];
	/** Playback state for choice-button feedback. */
	OM::Sound _click;
	/** Playback state for the quit prompt. */
	OM::Sound _voice;
	/** Current hovered button index, or minus one outside the buttons. */
	int _hover = -1;
	/** Update character and button feedback for the current hovered choice. */
	void setHover(int hover);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
