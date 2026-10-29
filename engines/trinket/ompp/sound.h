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

/** Load a sound resource and control its playback through the mixer. */
class Sound {
public:
	/** Bind a playback handle to the mixer. */
	explicit Sound(Audio::Mixer &mixer) : _mixer(mixer) {}
	/** Stop playback before releasing the sound wrapper. */
	~Sound() { stop(); }
	/** Decode a resource and submit its repetitions at a zero-to-one-hundred volume, returning false on failure. */
	bool play(const ResourceProvider &resources, const Common::String &path, Audio::Mixer::SoundType type, uint count = 1, uint volume = 100);
	/** Stop playback on this sound's mixer handle. */
	void stop() { _mixer.stopHandle(_handle); }
	/** Pause or resume this sound's mixer handle. */
	void pause(bool paused) { _mixer.pauseHandle(_handle, paused); }
	/** Set this sound's channel volume, clamped to zero through one hundred. */
	void setVolume(uint volume) { _mixer.setChannelVolume(_handle, MIN<uint>(volume, 100) * 255 / 100); }
	/** Return whether the mixer still has an active stream for this sound. */
	bool isPlaying() const { return _mixer.isSoundHandleActive(_handle); }

private:
	/** Mixer used to play and control this sound. */
	Audio::Mixer &_mixer;
	/** Mixer handle identifying this sound's current playback. */
	Audio::SoundHandle _handle;
	/** Prevent accidental copies of this stateful object. */
	Sound(const Sound &) = delete;
	/** Prevent accidental copies of this stateful object. */
	Sound &operator=(const Sound &) = delete;
};

} // End of namespace OM
} // End of namespace Trinket

#endif
