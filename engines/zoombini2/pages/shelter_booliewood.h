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

#ifndef ZOOMBINI2_PAGES_SHELTER_BOOLIEWOOD_H
#define ZOOMBINI2_PAGES_SHELTER_BOOLIEWOOD_H

#include "zoombini2/pages/shelter_base.h"

namespace Zoombini2 {

class BitBlock;
class ZoombiniGfx;

/**
 * Booliewood is the arrival shelter displaying the rescued community.
 * Leaving through the Map button returns to the adventure map.
 */
class BooliewoodPage : public ShelterPage {
public:
	BooliewoodPage(Zoombini2Engine *engine);
	~BooliewoodPage() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	bool hasGoButton() const override { return false; }

private:
	// --- Constants ---
	static const int kNumSeats = 23;

	// --- State ---
	int _musicId;           // Sound manager music handle

	BitBlock *_background;
	ZoombiniGfx *_zoombiniGfx;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_BOOLIEWOOD_H
