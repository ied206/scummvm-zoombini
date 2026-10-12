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

#ifndef TRINKET_RR1CLOUD9_RSS_SKY_RIDE_H
#define TRINKET_RR1CLOUD9_RSS_SKY_RIDE_H

#include "trinket/ompp/sound.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** Shared slide/catapult transition, including its two simultaneous voices. */
class SkyRide {
public:
	explicit SkyRide(PageScene &host) : _host(host) {}
	~SkyRide();
	bool enter();
	void update(uint32 time);
	void pause(bool paused, uint32 elapsed);
	Common::String destination() const;

private:
	PageScene &_host;
	Shell::Sprite *_animation = nullptr;
	OM::Sound *_voices[2] = {};
	int _route = 1;
	bool _started = false, _waiting = false, _ready = false;
	uint32 _start = 0;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
