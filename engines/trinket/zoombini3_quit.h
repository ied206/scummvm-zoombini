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
	QuitDialog(const OM::ResourceProvider &resources, Audio::Mixer &mixer);
	bool load();
	Result event(const Common::Event &event);
	bool overButton(Common::Point point) const { return _scene.hitTest(point) != 0; }
	void draw(OM::Canvas &canvas, uint32 time);

private:
	/** Sound resource IDs owned by the quit dialog. */
	enum PageResourceId : int16 {
		kResSoundCommon0001_Click = 1,
	};
	static constexpr const char *kAsset = "/z3common/%u.ao";
	static constexpr const char *kSound = "/z3common/%u.wav";
	const OM::ResourceProvider &_resources;
	Shell::Scene _scene;
	Shell::Sprite *_idle = nullptr, *_graphics[2], *_buttons[2];
	OM::Sound _click, _voice;
	int _hover = -1;
	void setHover(int hover);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
