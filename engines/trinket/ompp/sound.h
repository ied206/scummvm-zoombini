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

#ifndef TRINKET_OMPP_SOUND_H
#define TRINKET_OMPP_SOUND_H

#include "audio/mixer.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace OM {

/** Owns one sound's playback handle; the mixer owns the submitted decoded stream. */
class Sound {
public:
	explicit Sound(Audio::Mixer &mixer) : _mixer(mixer) {}
	~Sound() { stop(); }
	bool play(const ResourceProvider &resources, const Common::String &path, Audio::Mixer::SoundType type, uint count = 1, uint volume = 100);
	void stop() { _mixer.stopHandle(_handle); }
	void pause(bool paused) { _mixer.pauseHandle(_handle, paused); }
	void setVolume(uint volume) { _mixer.setChannelVolume(_handle, MIN<uint>(volume, 100) * 255 / 100); }
	bool isPlaying() const { return _mixer.isSoundHandleActive(_handle); }

private:
	Audio::Mixer &_mixer;
	Audio::SoundHandle _handle;
	Sound(const Sound &) = delete;
	Sound &operator=(const Sound &) = delete;
};

} // End of namespace OM
} // End of namespace Trinket

#endif
