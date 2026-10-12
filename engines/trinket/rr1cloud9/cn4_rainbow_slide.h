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

#ifndef TRINKET_RR1CLOUD9_CN4_RAINBOW_SLIDE_H
#define TRINKET_RR1CLOUD9_CN4_RAINBOW_SLIDE_H

#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** Carrie's wagon exchange and the entrance to the rainbow slide. */
class RainbowSlide {
public:
	explicit RainbowSlide(PageScene &host) : _host(host) {}
	bool enter();
	void update(uint32 time);
	bool click(Common::Point point);
	bool donate(uint item);
	void pause(bool paused, uint32 elapsed);
	bool busy() const { return _phase != kIdle; }
	Common::String destination() const;

private:
	enum Phase {
		kIdle,
		kDeliveryWait,
		kDeliverySpeech,
		kCollect,
		kExitSpeech,
		kExitAnimation,
		kExitReady,
	};
	PageScene &_host;
	Shell::Sprite *_bands = nullptr, *_wagon = nullptr, *_walk = nullptr;
	int _state = 1;
	Phase _phase = kIdle;
	uint32 _start = 0, _idle = 0;
	bool _clock = false;
	int number(const Common::String &key) const;
	void setState(int state);
	void group(const char *characters, const char *speech, int count, int index = 0);
	void carrieClick();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
