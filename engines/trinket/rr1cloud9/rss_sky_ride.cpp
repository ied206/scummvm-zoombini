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

#include "trinket/rr1cloud9/rss_sky_ride.h"

#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

SkyRide::~SkyRide() {
	for (auto *voice : _voices)
		delete voice;
}

bool SkyRide::enter() {
	_route = _host._world.get("previousLocation") == 10 ? 2 : 1;
	uint resource = _host._page->number(Common::String::format("kAnimID.%d", _route));
	_animation = _host._scene.add(_host.characterResource(resource, Shell::Character::Resources::kAnimation), _host._page->number("kAmbientZ"));
	if (!_animation)
		return false;
	for (auto &voice : _voices)
		voice = new OM::Sound(_host._vm.mixer());
	return true;
}

void SkyRide::update(uint32 time) {
	if (!_started) {
		_animation->startAnimation(1);
		const char *keys[] = {
			"kSamSpchID",
			"kRRSpchID",
		};
		for (uint i = 0; i < 2; i++) {
			int id = _host._page->number(Common::String::format("%s.%d", keys[i], _route));
			_voices[i]->play(_host._vm.resources(), _host.characterResource(id, Shell::Character::Resources::kSound),
							 Audio::Mixer::kSpeechSoundType, 1, _host._page->number("kAmbientVolume"));
		}
		_started = true;
		return;
	}
	if (_animation->isRunning() || _voices[0]->isPlaying() || _voices[1]->isPlaying())
		return;
	if (!_waiting) {
		_start = time;
		_waiting = true;
	}
	_ready = 1 <= time - _start;
}

void SkyRide::pause(bool paused, uint32 elapsed) {
	_animation->pauseAnimation(paused);
	for (auto *voice : _voices)
		voice->pause(paused);
	if (!paused && _waiting)
		_start += elapsed;
}

Common::String SkyRide::destination() const {
	return _ready ? _host._page->text(Common::String::format("kPreviousLocations.%d", _route)) : Common::String();
}
} // namespace RR1Cloud9
} // namespace Trinket
