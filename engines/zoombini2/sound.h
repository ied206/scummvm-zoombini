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

namespace Common {
class SeekableReadStream;
}

namespace Audio {
class AudioStream;
class RewindableAudioStream;
} // namespace Audio

namespace Zoombini2 {

class Zoombini2Engine;

/** Maximum volume accepted by the game-facing sound API. */
const int kMaxVolumePercent = 100;

/** Number of sample handles reserved for one logical sound. */
const int kMaxSampleSlots = 5;

/** Logical mixer category derived from a loose-file resource path. */
enum class SoundCategory {
	kMusic00 = 0,
	kSFX01 = 1,
	kSpeech02 = 2
};

/** Retains the path, playback policy, volume, and mixer handles for one sound. */
struct SoundBuffer {
	/** Manager-assigned sound identifier. */
	int id;
	/** Original logical resource name, including any installed-root marker. */
	Common::Path path;
	/** Complete encoded file image retained for non-stream playback. */
	Common::Array<byte> sampleData;
	/** Whether the caller requested streaming playback. */
	bool isStream;
	/** Whether playback should restart after the final sample. */
	bool loop;
	/** Mixer category controlling this sound. */
	SoundCategory category;
	/** Game-facing volume in the inclusive range 0 through 100. */
	int volume;
	/** Whether the channel should use the complete volume of its mixer category. */
	bool usesCategoryVolume;
	/** Mixer handles reserved for overlapping sample playback. */
	Audio::SoundHandle handles[kMaxSampleSlots];
	/** Whether each active sample handle is paused. */
	bool handlesPaused[kMaxSampleSlots] = {};
	/** Mixer handle reserved for streamed playback. */
	Audio::SoundHandle streamHandle;
	/** Whether the active streamed handle is paused. */
	bool streamPaused = false;
};

/**
 * Manages logical sound records and routes WAV playback through Audio::Mixer.
 *
 * The original engine uses Miles Sound System for digital audio. This class
 * reproduces the sample and stream lifecycle required by the game on top of
 * ScummVM's @ref Audio::Mixer.
 *
 * Loading retains non-stream file images for overlapping sample playback.
 * Streamed sounds are opened and decoded on demand through the engine resource
 * resolver. Numbered installed-music paths may resolve to the extracted CD
 * sound-effects directory when that compatibility resource exists.
 */
class SoundManager {
public:
	/** Bind sound playback to the borrowed @p vm and @p mixer. */
	SoundManager(Zoombini2Engine *vm, Audio::Mixer *mixer);
	/** Stop playback and release every retained @ref SoundBuffer. */
	~SoundManager();

	/** Record a streamed sound or retain a non-stream file image and return its manager-assigned identifier. */
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
	/** Stop sample handles or preserve and pause the streamed handle belonging to @p id. */
	void stop(int id);
	/** Pause every active handle belonging to @p id. */
	void pause(int id);
	/** Resume every paused handle belonging to @p id. */
	void resume(int id);

	/** Return whether any handle belonging to @p id is actively playing rather than paused. */
	bool isPlaying(int id) const;

	/** Store and apply @p volume to every active handle belonging to @p id. */
	void setVolume(int id, int volume);
	/** Apply @p volume to every loaded sound. */
	void setVolumeAll(int volume);

	/** Increment the nested mute count and mute mixer sound categories on the first request. */
	void mute();
	/** Decrement the nested mute count and unmute mixer sound categories when it reaches zero. */
	void unmute();

	/** Pause every active handle associated with a loaded sound. */
	void pauseAll();
	/** Resume every paused handle associated with a loaded sound. */
	void resumeAll();

	/** Synchronize the game-facing percentages with ScummVM's mixer settings. */
	void setVolumeSettings(int music, int sfx, int speech);
	/** Select stereo input streams or downmix them to mono before playback. */
	void setStereoOutputEnabled(bool enabled) { _stereoOutputEnabled = enabled; }

	/** Global sound-effect volume in the inclusive range 0 through 100. */
	int _volumeSFX = 100;
	/** Global music volume in the inclusive range 0 through 100. */
	int _volumeMusic = 100;
	/** Global speech volume in the inclusive range 0 through 100. */
	int _volumeSpeech = 100;

private:
	class MonoAudioStream;

	/** Borrowed vm that resolves original logical resource names. */
	Zoombini2Engine *_vm;
	/** Borrowed mixer used by every sound record. */
	Audio::Mixer *_mixer;
	/** Loaded sound records retained for the game instance. */
	Common::Array<SoundBuffer *> _buffers;
	/** Identifier assigned to the next loaded sound. */
	int _nextId = 1;
	/** Nested mute-request count. */
	int _muteRefCount = 0;
	/** Whether newly started stereo WAV streams retain both channels. */
	bool _stereoOutputEnabled = false;

	/** Return the borrowed sound record for @p id, or nullptr. */
	SoundBuffer *findBuffer(int id) const;
	/** Stop and release every mixer handle retained by @p buffer. */
	void releasePlayback(SoundBuffer &buffer);
	/** Decode a WAV while retaining only complete PCM sample frames. */
	static Audio::RewindableAudioStream *makeAudioStream(Common::SeekableReadStream *stream, const Common::Path &path);
	/** Select an optional extracted-data compatibility alternative without resolving a physical path. */
	Common::Path resolveCompatibilityPath(const Common::Path &filename) const;
	/** Classify @p filename before any music-path fallback is applied. */
	static SoundCategory classifySound(const Common::Path &filename);
	/** Convert a logical sound category to its mixer sound type. */
	static Audio::Mixer::SoundType getMixerSoundType(SoundCategory category);
	/** Return the current game-facing percentage for @p category. */
	int getCategoryVolume(SoundCategory category) const;
	/** Clamp a percentage and convert it to the Audio::Mixer volume range. */
	static byte normalizeVolume(int volume);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_SOUND_H
