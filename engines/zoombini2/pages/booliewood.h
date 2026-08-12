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

#ifndef ZOOMBINI2_PAGES_BOOLIEWOOD_H
#define ZOOMBINI2_PAGES_BOOLIEWOOD_H

#include "zoombini2/pages/puzzle.h"

namespace Zoombini2 {

class RleBlock;

/**
 * BooliewoodPage — Animated celebration hub (ID 12).
 *
 * Original: Booliewood__Init_40E600, object size 0x24 (36 bytes).
 * NOT a puzzle — this is an animated hub/celebration world.
 *
 * Mechanics:
 *   - Zoombinis sit in 23 theater-style seats
 *   - 17 animated boolies walk along predefined paths
 *   - Background animations play based on difficulty level
 *   - Crowd speech plays periodically ("blw22.N")
 *   - Background music from "sounds/music/booliewood"
 *   - First-visit speech: "zbv212"
 *
 * Difficulty determines animation set:
 *   - Level >= 1: mushroom lights, ourson rail anims
 *   - Level >= 2: fire, horror cils anims
 *   - Level >= 3: horror move, eyelashes anims
 *   - Level >= 4: horror fire large anim
 *
 * Simplified: Display background, place zoombinis in seating,
 * auto-advance after delay or click.
 *
 * Resources:
 *   bmp/booliewood/background (background)
 *   bmp/booliewood/atraction_*.an (animations)
 *   bmp/booliewood/path1-17.pat (boolie walking paths)
 *   bmp/booliewood/piti_bool/ (boolie sprites)
 *   bmp/boolies/boolies_march.anm, boolies_attente.anm
 *   sounds/music/booliewood (background music)
 */
class BooliewoodPage : public PuzzlePage {
public:
	BooliewoodPage(Zoombini2Engine *engine);
	~BooliewoodPage() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	// --- Constants ---
	static const int kNumSeats = 23;
	static const int kAutoAdvanceMs = 15000; // 15 seconds for hub scene

	// --- State ---
	int _difficulty;        // 1-4, determines animation set
	bool _transitioning;    // True when transitioning out
	int _musicId;           // Sound manager music handle

	// Two overlay sprites (from Init: this+3, this+4)
	RleBlock *_overlay1;
	RleBlock *_overlay2;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_BOOLIEWOOD_H
