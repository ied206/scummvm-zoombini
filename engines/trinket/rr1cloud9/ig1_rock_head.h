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

#ifndef TRINKET_RR1CLOUD9_IG1_ROCK_HEAD_H
#define TRINKET_RR1CLOUD9_IG1_ROCK_HEAD_H

#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** Ladder, summit conversation and the hat-for-hoop exchange. */
class RockHead {
public:
	explicit RockHead(PageScene &host) : _host(host) {}
	bool enter();
	void update(uint32 time);
	bool click(Common::Point point);
	bool donate(uint item);
	void resume(uint32 elapsed) {
		_start += elapsed;
		_idle += elapsed;
	}
	bool busy() const { return _phase != kIdle; }

private:
	enum Phase {
		kIdle,
		kArrival,
		kReaderClimb,
		kSamClimb,
		kLadderWait,
		kHatWait,
		kHatSpeech,
		kCollect,
		kSpeech,
	};
	PageScene &_host;
	Shell::Sprite *_ladder = nullptr, *_hat = nullptr, *_hoop = nullptr;
	Phase _phase = kIdle;
	int _state = 1, _pose = 1, _moreClicks = 1;
	uint32 _start = 0, _idle = 0;
	bool _clock = false, _firstVisit = false;
	int number(const Common::String &key) const;
	void line(const Common::String &actors, const Common::String &sounds);
	void group(const Common::String &actors, const Common::String &sounds, int count);
	bool character(const char *name, const char *prefix, int pose);
	bool companions(int pose);
	bool rock();
	void entrySpeech();
	void mainSpeech();
	void rockSpeech();
	void climb();
	void hoopClick();
	bool hotspot(const char *prefix, const Common::String &suffix, Common::Point point) const;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
