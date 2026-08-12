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

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"

#include "zoombini2/sound.h"

namespace Zoombini2 {

SoundManager::SoundManager(Audio::Mixer *mixer) : _mixer(mixer) {
	_nextId = 1;
	_muteRefCount = 0;
	_volumeSFX = 100;
	_volumeMusic = 100;
	_volumeSpeech = 100;
}

SoundManager::~SoundManager() {
	unloadAll();
}

int SoundManager::load(bool isStream, const Common::Path &filename, bool loop) {
	SoundBuffer *buf = new SoundBuffer();
	buf->id = _nextId++;
	buf->path = resolvePath(filename);
	buf->isStream = isStream;
	buf->loop = loop;
	buf->volume = kMaxVolumePercent;
	_buffers.push_back(buf);
	return buf->id;
}

void SoundManager::unload(int id) {
	for (uint i = 0; i < _buffers.size(); i++) {
		if (_buffers[i]->id == id) {
			stop(id);
			delete _buffers[i];
			_buffers.remove_at(i);
			return;
		}
	}
}

void SoundManager::unloadAll() {
	for (uint i = 0; i < _buffers.size(); i++) {
		stop(_buffers[i]->id);
		delete _buffers[i];
	}
	_buffers.clear();
}

void SoundManager::play(int id) {
	SoundBuffer *buf = findBuffer(id);
	if (!buf)
		return;

	Common::File *f = new Common::File();
	if (!f->open(buf->path)) {
		debug(1, "SoundManager::play: Cannot open '%s'", buf->path.toString().c_str());
		delete f;
		return;
	}

	Audio::RewindableAudioStream *stream = Audio::makeWAVStream(f, DisposeAfterUse::YES);
	if (!stream)
		return;

	Audio::AudioStream *audioStream;
	if (buf->loop) {
		audioStream = Audio::makeLoopingAudioStream(stream, 0);
	} else {
		audioStream = stream;
	}

	byte vol = normalizeVolume(buf->volume);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &buf->handles[0], audioStream, -1, vol);
}

void SoundManager::playWithVolume(int id, int volume) {
	setVolume(id, volume);
	play(id);
}

void SoundManager::playLoop(int id) {
	SoundBuffer *buf = findBuffer(id);
	if (buf) {
		buf->loop = true;
		play(id);
	}
}

void SoundManager::stop(int id) {
	SoundBuffer *buf = findBuffer(id);
	if (!buf)
		return;

	for (int i = 0; i < kMaxSampleSlots; i++) {
		if (_mixer->isSoundHandleActive(buf->handles[i]))
			_mixer->stopHandle(buf->handles[i]);
	}
	if (_mixer->isSoundHandleActive(buf->streamHandle))
		_mixer->stopHandle(buf->streamHandle);
}

void SoundManager::pause(int id) {
	SoundBuffer *buf = findBuffer(id);
	if (!buf)
		return;

	for (int i = 0; i < kMaxSampleSlots; i++) {
		if (_mixer->isSoundHandleActive(buf->handles[i]))
			_mixer->pauseHandle(buf->handles[i], true);
	}
	if (_mixer->isSoundHandleActive(buf->streamHandle))
		_mixer->pauseHandle(buf->streamHandle, true);
}

void SoundManager::resume(int id) {
	SoundBuffer *buf = findBuffer(id);
	if (!buf)
		return;

	for (int i = 0; i < kMaxSampleSlots; i++) {
		if (_mixer->isSoundHandleActive(buf->handles[i]))
			_mixer->pauseHandle(buf->handles[i], false);
	}
	if (_mixer->isSoundHandleActive(buf->streamHandle))
		_mixer->pauseHandle(buf->streamHandle, false);
}

bool SoundManager::isPlaying(int id) const {
	SoundBuffer *buf = findBuffer(id);
	if (!buf)
		return false;

	for (int i = 0; i < kMaxSampleSlots; i++) {
		if (_mixer->isSoundHandleActive(buf->handles[i]))
			return true;
	}
	return _mixer->isSoundHandleActive(buf->streamHandle);
}

void SoundManager::setVolume(int id, int volume) {
	SoundBuffer *buf = findBuffer(id);
	if (!buf)
		return;

	buf->volume = volume;
	byte vol = normalizeVolume(volume);

	for (int i = 0; i < kMaxSampleSlots; i++) {
		if (_mixer->isSoundHandleActive(buf->handles[i]))
			_mixer->setChannelVolume(buf->handles[i], vol);
	}
	if (_mixer->isSoundHandleActive(buf->streamHandle))
		_mixer->setChannelVolume(buf->streamHandle, vol);
}

void SoundManager::setVolumeAll(int volume) {
	for (uint i = 0; i < _buffers.size(); i++)
		setVolume(_buffers[i]->id, volume);
}

void SoundManager::mute() {
	_muteRefCount++;
	if (_muteRefCount == 1) {
		_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, true);
		_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, true);
		_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, true);
	}
}

void SoundManager::unmute() {
	if (_muteRefCount > 0)
		_muteRefCount--;
	if (_muteRefCount == 0) {
		_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, false);
		_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, false);
		_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, false);
	}
}

void SoundManager::pauseAll() {
	_mixer->pauseAll(true);
}

void SoundManager::resumeAll() {
	_mixer->pauseAll(false);
}

SoundBuffer *SoundManager::findBuffer(int id) const {
	for (uint i = 0; i < _buffers.size(); i++) {
		if (_buffers[i]->id == id)
			return _buffers[i];
	}
	return nullptr;
}

Common::Path SoundManager::resolvePath(const Common::Path &filename) const {
	// Original path resolution: '#' prefix = CD path, else = install path
	// In ScummVM, all paths are relative to the game data directory
	Common::String str = filename.toString();
	if (!str.empty() && str[0] == '#') {
		str = str.substr(1);
	}

	// Handle music directory mapping:
	// With full game installation, music files are in INSTALL/HD/sounds/music/
	// With extracted data only, BB files are in Data/Sounds/FX/
	// Map sounds/music/XX-BB*.wav to sounds/fx/XX-BB*.wav for fallback
	if (str.hasPrefix("sounds/music/")) {
		Common::String basename = str.substr(13);  // Remove "sounds/music/"
		// Check if this is a numeric-prefix BB file (e.g. "01-BB01.wav")
		if (basename.size() > 6 && basename[2] == '-' && basename[3] == 'B' && basename[4] == 'B') {
			// Try sounds/fx/ path first for extracted data compatibility
			Common::Path fxPath = Common::Path(Common::String::format("sounds/fx/%s", basename.c_str()));
			Common::File testFile;
			if (testFile.open(fxPath)) {
				testFile.close();
				return fxPath;
			}
		}
	}

	return Common::Path(str);
}

/**
 * Normalize volume from game range (0-100) to ScummVM mixer range (0-255).
 * Original NormalizeVolume (0x46C463): converts 0-100% to MSS 0-127.
 * ScummVM uses 0-255 for mixer volume.
 */
byte SoundManager::normalizeVolume(int volume) const {
	if (volume < 0)
		volume = 0;
	if (volume > kMaxVolumePercent)
		volume = kMaxVolumePercent;
	return (byte)((volume * 255) / kMaxVolumePercent);
}

} // End of namespace Zoombini2
