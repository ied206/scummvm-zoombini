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

#ifndef TRINKET_SHELL_MUSIC_H
#define TRINKET_SHELL_MUSIC_H

#include "trinket/ompp/sound.h"

namespace Trinket {
namespace Shell {

/** Shared background music survives replacement of the game world and scene. */
class BackgroundMusic {
public:
	BackgroundMusic(Audio::Mixer &mixer, const OM::ResourceProvider &resources) : _resources(resources), _sound(mixer) {}
	void start(const Common::String &resource, uint volume);
	void setResource(const Common::String &resource);
	void stop() { _sound.stop(); }
	void setVolume(uint volume) { start(_resource, volume); }
	void setEnabled(bool enabled);
	const Common::String &resource() const { return _resource; }
	uint volume() const { return _volume; }
	bool isEnabled() const { return _enabled; }
	bool isPlaying() const { return _sound.isPlaying(); }
	void reset();

private:
	const OM::ResourceProvider &_resources;
	OM::Sound _sound;
	Common::String _resource;
	uint _volume = 100;
	bool _enabled = true;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
