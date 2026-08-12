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

#ifndef ZOOMBINI2_PAGES_FINAL_H
#define ZOOMBINI2_PAGES_FINAL_H

#include "zoombini2/pages/page.h"

namespace Zoombini2 {

class BitBlock;
class Animation;

/**
 * FinalPage — final celebration (page ID 23).
 * Object size: 0x54 (84 bytes).
 * Init: Final__Init_418740
 *
 * Shows dancing boolies, fireworks, plays celebration music,
 * then transitions to credits.
 */
class FinalPage : public Page {
public:
	FinalPage(Zoombini2Engine *engine);
	~FinalPage() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	BitBlock *_background;
	BitBlock *_fullBigBool;
	Animation *_dancingBoolie;
	Animation *_boolDance;
	Animation *_fwBlue;
	Animation *_fwRed;
	Animation *_fwGreen;
	int _animFrame;
	uint32 _lastFrameTime;
	uint32 _startTime;
	bool _finished;
	bool _speechPlayed; // "fin11" speech audio
	int _musicId;       // BGM: sounds/music/Booliewood_Finale.wav

	// Ambient sound system (IDA: this+0 to this+6 in object layout)
	// zbv425.wav + blw22.1-6.wav = 7 sounds
	static const int kAmbientSoundCount = 7;
	int _ambientSoundIds[kAmbientSoundCount];
	uint32 _nextAmbientTime;  // When to play next ambient sound

	void scheduleNextAmbient();
	void playRandomAmbient();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_FINAL_H
