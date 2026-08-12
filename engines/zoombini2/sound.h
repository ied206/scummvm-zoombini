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

#ifndef ZOOMBINI2_SOUND_H
#define ZOOMBINI2_SOUND_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/path.h"
#include "common/array.h"

#include "audio/mixer.h"

namespace Audio {
class AudioStream;
class RewindableAudioStream;
}

namespace Zoombini2 {

/**
 * Volume range constants.
 * Original MSS range: 0-127.
 * Original percent mode: 0-100.
 */
const int kMaxVolumeMSS = 127;
const int kMaxVolumePercent = 100;

/**
 * Maximum simultaneous sample playback slots per buffer.
 * Original: CSaianSoundBuffer supports 5 sample handles.
 */
const int kMaxSampleSlots = 5;

/**
 * Sound buffer — wraps a single sound resource.
 *
 * Corresponds to CSaianSoundBuffer (0x30 = 48 bytes).
 * Supports both in-memory samples (SFX) and disk streams (music/speech).
 */
struct SoundBuffer {
	int id;
	Common::Path path;
	bool isStream;
	bool loop;
	int volume;
	Audio::SoundHandle handles[kMaxSampleSlots];
	Audio::SoundHandle streamHandle;
};

/**
 * SoundManager — high-level sound manager.
 *
 * Corresponds to CSaianSound (~2068 bytes, global at 0x571DE8).
 * Wraps Miles Sound System AIL API via ScummVM Audio::Mixer.
 *
 * Sound path convention:
 *   '#' prefix  -> CD-ROM path (streamed)
 *   no prefix   -> install directory (RAM)
 *   './' prefix -> relative path
 */
class SoundManager {
public:
	SoundManager(Audio::Mixer *mixer);
	~SoundManager();

	int load(bool isStream, const Common::Path &filename, bool loop);
	void unload(int id);
	void unloadAll();

	void play(int id);
	void playWithVolume(int id, int volume);
	void playLoop(int id);
	void stop(int id);
	void pause(int id);
	void resume(int id);

	bool isPlaying(int id) const;

	void setVolume(int id, int volume);
	void setVolumeAll(int volume);

	void mute();
	void unmute();

	void pauseAll();
	void resumeAll();

	// Volume globals
	int _volumeSFX;     // 0x571DF0: SFX volume
	int _volumeMusic;   // 0x571DF4: Music volume
	int _volumeSpeech;  // 0x571DF8: Speech volume

private:
	Audio::Mixer *_mixer;
	Common::Array<SoundBuffer *> _buffers;
	int _nextId;
	int _muteRefCount; // >0 = muted

	SoundBuffer *findBuffer(int id) const;
	Common::Path resolvePath(const Common::Path &filename) const;

	/**
	 * Normalize volume to mixer range.
	 * Original: NormalizeVolume at 0x46C463
	 */
	byte normalizeVolume(int volume) const;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_SOUND_H
