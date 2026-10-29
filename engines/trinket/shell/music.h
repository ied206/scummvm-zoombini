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
	/** Connect shared background-music playback to the mixer and resource provider. */
	BackgroundMusic(Audio::Mixer &mixer, const OM::ResourceProvider &resources) : _resources(resources), _sound(mixer) {}
	/** Select a resource and volume, starting playback when music is enabled. */
	void start(const Common::String &resource, uint volume);
	/** Change the selected resource while retaining the current volume. */
	void setResource(const Common::String &resource);
	/** Stop the current music stream without changing its selected resource. */
	void stop() { _sound.stop(); }
	/** Apply a zero-to-one-hundred volume to the selected music resource. */
	void setVolume(uint volume) { start(_resource, volume); }
	/** Enable or disable music playback while retaining the selected resource. */
	void setEnabled(bool enabled);
	/** Return the selected music resource path. */
	const Common::String &resource() const { return _resource; }
	/** Return the selected music volume from zero through one hundred. */
	uint volume() const { return _volume; }
	/** Return whether music playback is enabled. */
	bool isEnabled() const { return _enabled; }
	/** Return whether the current music stream is active. */
	bool isPlaying() const { return _sound.isPlaying(); }
	/** Stop music and restore the default resource and playback preferences. */
	void reset();

private:
	/** Provider used to open background-music resources. */
	const OM::ResourceProvider &_resources;
	/** Mixer handle used for shared background-music playback. */
	OM::Sound _sound;
	/** Currently selected music resource path. */
	Common::String _resource;
	/** Selected music volume from zero through one hundred. */
	uint _volume = 100;
	/** Whether the selected background music may play. */
	bool _enabled = true;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
