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

#ifndef ZOOMBINI2_PAGES_CREDITS_H
#define ZOOMBINI2_PAGES_CREDITS_H

#include "zoombini2/pages/page.h"

namespace Zoombini2 {

class BitBlock;

/**
 * CreditsPage — end credits scroll (page ID 16).
 * Original: Credits__Init_417E60 (0x417E60), object size 0x20 (32 bytes).
 *
 * Loads bmp/credits/credits.bmp and scrolls it vertically at 30px/sec.
 * Waits 4 seconds before starting scroll, then holds 10 seconds at bottom.
 * Click or skip at any point transitions immediately to kPageTitleScreen.
 * BGM: sounds/music/ZMR-Transition.wav (looped).
 *
 * Object layout (matching original 0x20-byte struct):
 *   offset  0 (uint32): endTime — initial 4s wait timer, reused as 10s end timer
 *   offset  8 (float):  scrollY — current vertical scroll position
 *   offset 12 (int):    maxScrollY — image_height - 600 (scroll endpoint)
 *   offset 16 (bool):   scrollActive — true while scrolling in progress
 *   offset 17 (bool):   redrawNeeded — set when scroll position changes
 *   offset 20 (uint32): lastUpdateTime — for delta-time scroll calculation
 *   offset 24 (bool):   initialWait — true during initial 4-second pause
 *   offset 28 (ptr):    background — credits bitmap (tall image to scroll)
 */
class CreditsPage : public Page {
public:
	CreditsPage(Zoombini2Engine *engine, bool quitAfterCredits = false);
	~CreditsPage() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

	// Credits uses conditional drawing via _redrawNeeded — never needs screen clear.
	// The default false from Page is correct, so no override needed.

private:
	// Original Credits struct fields (offset-mapped):
	uint32    _endTime;        // offset  0: initial/end wait timer
	float     _scrollY;        // offset  8: current scroll Y position
	int       _maxScrollY;     // offset 12: scroll endpoint (bitmap_height - 600)
	bool      _scrollActive;   // offset 16: true while scrolling
	bool      _redrawNeeded;   // offset 17: set each frame when scrolling
	uint32    _lastUpdateTime; // offset 20: delta-time base
	bool      _initialWait;    // offset 24: true during 4-sec initial pause
	BitBlock *_background;     // offset 28: tall credits bitmap

	int  _musicId; // BGM sound handle (-1 = none)
	bool _finished; // true when done (click/skip/natural end) — signal transition
	bool _quitAfterCredits; // true when launched from the quit button
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_CREDITS_H
