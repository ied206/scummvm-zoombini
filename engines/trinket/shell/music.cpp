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

#include "trinket/shell/music.h"

namespace Trinket {
namespace Shell {

void BackgroundMusic::start(const Common::String &resource, uint volume) {
	if (_resource != resource) {
		stop();
		_resource = resource;
	}
	_volume = volume;
	if (!_enabled || _resource.empty())
		return;
	if (_sound.isPlaying())
		_sound.setVolume(_volume);
	else
		_sound.play(_resources, _resource, Audio::Mixer::kMusicSoundType, 0, _volume);
}

void BackgroundMusic::setResource(const Common::String &resource) {
	if (_resource != resource || !_sound.isPlaying()) {
		stop();
		start(resource, _volume);
	}
}

void BackgroundMusic::setEnabled(bool enabled) {
	if (_enabled == enabled)
		return;
	_enabled = enabled;
	if (!_enabled || _resource.empty())
		stop();
	else
		start(_resource, _volume);
}

void BackgroundMusic::reset() {
	stop();
	_resource.clear();
	_volume = 100;
	_enabled = true;
}

} // namespace Shell
} // namespace Trinket
