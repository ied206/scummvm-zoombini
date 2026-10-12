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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_RR1CLOUD9_LG2_CATAPULT_H
#define TRINKET_RR1CLOUD9_LG2_CATAPULT_H

#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** Feeding the catapult's guardian and launching back to Cloud Nine. */
class Catapult {
public:
	explicit Catapult(PageScene &host) : _host(host) {}
	bool enter();
	void advance(uint32 time);
	void update(uint32 time);
	bool click(Common::Point point);
	bool donate(uint item);
	void pause(bool paused, uint32 elapsed);
	bool busy() const { return _phase != kIdle; }
	Common::String destination() const;

private:
	enum Phase {
		kIdle,
		kFeedWait,
		kFeedSpeech,
		kEating,
		kSpeech,
		kExitSpeech,
		kExiting,
		kExitReady,
	};
	PageScene &_host;
	Shell::Sprite *_bowl = nullptr, *_sandwich = nullptr, *_eating = nullptr, *_sleep = nullptr;
	Shell::Sprite *_clump = nullptr, *_clumpAnim = nullptr, *_catapult = nullptr, *_exit = nullptr;
	Phase _phase = kIdle;
	bool _sleeping = false, _clock = false, _ambientPlaying = false, _snoring = false;
	uint32 _start = 0, _idle = 0, _ambientStart = 0, _sleepStart = 0;
	uint _ambientDelay = 1000;
	int number(const Common::String &key) const;
	void line(const char *actors, const char *sounds, int index = 0);
	void group(const char *actors, const char *sounds, const char *count, int index = 0);
	void ride();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
