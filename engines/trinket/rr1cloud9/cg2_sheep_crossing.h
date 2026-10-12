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

#ifndef TRINKET_RR1CLOUD9_CG2_SHEEP_CROSSING_H
#define TRINKET_RR1CLOUD9_CG2_SHEEP_CROSSING_H

#include "trinket/ompp/sound.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** Whistle-controlled sheep crossing between the palace and rainbow slide. */
class SheepCrossing {
public:
	explicit SheepCrossing(PageScene &host) : _host(host) {}
	~SheepCrossing() { delete _sound; }
	bool enter();
	void update(uint32 time);
	bool click(Common::Point point);
	bool donate(uint item);
	void pause(bool paused, uint32 elapsed);
	bool busy() const { return _phase != kIdle; }
	Common::String destination() const { return _destination; }

private:
	enum Phase {
		kIdle,
		kWait,
		kBlow,
		kStand,
		kSpeech,
	};
	PageScene &_host;
	Common::Array<Shell::Sprite *> _walking, _standing;
	Shell::Sprite *_whistle = nullptr;
	OM::Sound *_sound = nullptr;
	Common::String _destination;
	Phase _phase = kIdle;
	int _state = 1;
	bool _moving = true, _clock = false;
	uint32 _start = 0, _idle = 0;
	int number(const Common::String &key) const;
	int increment(const char *key);
	void group(const char *actors, const char *sounds, const char *count, const Common::String &suffix);
	void talk(bool sheep);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
