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
 *
 */

#ifndef ZOOMBINI2_SOUND_H
#define ZOOMBINI2_SOUND_H

#include "common/array.h"
#include "common/path.h"
#include "common/scummsys.h"
#include "common/str.h"

#include "audio/mixer.h"

namespace Audio {
class AudioStream;
class RewindableAudioStream;
} // namespace Audio

namespace Zoombini2 {

/** Maximum volume accepted by the game-facing sound API. */
const int kMaxVolumePercent = 100;

/** Number of sample handles reserved for one logical sound. */
const int kMaxSampleSlots = 5;

/** Retains the path, playback policy, volume, and mixer handles for one sound. */
struct SoundBuffer {
	/** Manager-assigned sound identifier. */
	int id;
	/** Resolved loose-file path. */
	Common::Path path;
	/** Whether the caller requested streaming playback. */
	bool isStream;
	/** Whether playback should restart after the final sample. */
	bool loop;
	/** Game-facing volume in the inclusive range 0 through 100. */
	int volume;
	/** Mixer handles reserved for overlapping sample playback. */
	Audio::SoundHandle handles[kMaxSampleSlots];
	/** Mixer handle reserved for streamed playback. */
	Audio::SoundHandle streamHandle;
};

/**
 * Owns logical sound records and routes WAV playback through Audio::Mixer.
 *
 * Loading records a path and playback policy but does not decode audio.
 * Playback opens and decodes the WAV on demand. A leading `#` is removed from
 * disc-style paths, and numbered music paths may resolve to the extracted FX
 * directory when that resource exists.
 */
class SoundManager {
public:
	/** Bind sound playback to the borrowed @p mixer. */
	explicit SoundManager(Audio::Mixer *mixer);
	/** Stop playback and release every owned @ref SoundBuffer. */
	~SoundManager();

	/** Record a sound path and return its manager-assigned identifier. */
	int load(bool isStream, const Common::Path &filename, bool loop);
	/** Stop and release the sound identified by @p id. */
	void unload(int id);
	/** Stop and release every loaded sound. */
	void unloadAll();

	/** Start the sound identified by @p id with its stored volume and loop policy. */
	void play(int id);
	/** Store @p volume and start the sound identified by @p id. */
	void playWithVolume(int id, int volume);
	/** Enable looping and start the sound identified by @p id. */
	void playLoop(int id);
	/** Stop every active handle belonging to @p id. */
	void stop(int id);
	/** Pause every active handle belonging to @p id. */
	void pause(int id);
	/** Resume every paused handle belonging to @p id. */
	void resume(int id);

	/** Return whether any mixer handle belonging to @p id is active. */
	bool isPlaying(int id) const;

	/** Store and apply @p volume to every active handle belonging to @p id. */
	void setVolume(int id, int volume);
	/** Apply @p volume to every loaded sound. */
	void setVolumeAll(int volume);

	/** Increment the nested mute count and mute mixer sound categories on the first request. */
	void mute();
	/** Decrement the nested mute count and unmute mixer sound categories when it reaches zero. */
	void unmute();

	/** Pause all mixer channels. */
	void pauseAll();
	/** Resume all mixer channels. */
	void resumeAll();

	/** Global sound-effect volume in the inclusive range 0 through 100. */
	int _volumeSFX;
	/** Global music volume in the inclusive range 0 through 100. */
	int _volumeMusic;
	/** Global speech volume in the inclusive range 0 through 100. */
	int _volumeSpeech;

private:
	/** Borrowed mixer used by every sound record. */
	Audio::Mixer *_mixer;
	/** Loaded sound records owned by this manager. */
	Common::Array<SoundBuffer *> _buffers;
	/** Identifier assigned to the next loaded sound. */
	int _nextId;
	/** Nested mute-request count. */
	int _muteRefCount;

	/** Return the borrowed sound record for @p id, or nullptr. */
	SoundBuffer *findBuffer(int id) const;
	/** Normalize a game resource name into a SearchMan-relative path. */
	Common::Path resolvePath(const Common::Path &filename) const;
	/** Clamp a percentage and convert it to the Audio::Mixer volume range. */
	byte normalizeVolume(int volume) const;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_SOUND_H
