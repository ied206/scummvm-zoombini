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

#ifndef ZOOMBINI2_PAGES_TITLE_H
#define ZOOMBINI2_PAGES_TITLE_H

#include "zoombini2/pages/page.h"

namespace Zoombini2 {

class BitBlock;

/**
 * Load the title background and wait for input or the timeout.
 */
class TitleScreen : public Page {
public:
	TitleScreen(Zoombini2Engine *engine);
	~TitleScreen() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	BitBlock *_background;
	bool _clicked;
	uint32 _deadline;
	int _musicId;            // BGM: sounds/music/Booliewood_Level1.wav
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_TITLE_H
