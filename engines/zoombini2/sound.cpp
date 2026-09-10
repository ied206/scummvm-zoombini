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
#include "common/system.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "audio/mixer.h"

#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

/** Convert stereo samples to one mono sample per source frame. */
class SoundManager::MonoAudioStream : public Audio::AudioStream {
public:
	explicit MonoAudioStream(Audio::AudioStream *parent) : _parent(parent) {
	}

	~MonoAudioStream() override {
		delete _parent;
	}

	int readBuffer(int16 *buffer, const int numSamples) override {
		if (!_parent->isStereo())
			return _parent->readBuffer(buffer, numSamples);

		int samplesRead = 0;
		while (samplesRead < numSamples && !_parent->endOfData()) {
			int16 stereoFrame[2];
			const int frameSamples = _parent->readBuffer(stereoFrame, ARRAYSIZE(stereoFrame));
			if (frameSamples < 0)
				return samplesRead == 0 ? frameSamples : samplesRead;
			if (frameSamples < static_cast<int>(ARRAYSIZE(stereoFrame)))
				break;
			buffer[samplesRead] = static_cast<int16>((static_cast<int32>(stereoFrame[0]) + static_cast<int32>(stereoFrame[1])) / 2);
			samplesRead += 1;
		}
		return samplesRead;
	}

	bool isStereo() const override { return false; }
	int getRate() const override { return _parent->getRate(); }
	bool endOfData() const override { return _parent->endOfData(); }
	bool endOfStream() const override { return _parent->endOfStream(); }

private:
	Audio::AudioStream *_parent;
};

SoundManager::SoundManager(Zoombini2Engine *vm, Audio::Mixer *mixer) : _vm(vm), _mixer(mixer) {
	_nextId = 1;
	_muteRefCount = 0;
	_stereoOutputEnabled = false;
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
	buf->path = resolveCompatibilityPath(filename);
	buf->isStream = isStream;
	buf->loop = loop;
	buf->category = classifySound(filename);
	buf->volume = kMaxVolumePercent;
	buf->usesCategoryVolume = true;
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

	Common::SeekableReadStream *f = _vm->openResourceFile(buf->path.toString('/'));
	if (!f) {
		debug(1, "SoundManager::play: Cannot open '%s'", buf->path.toString().c_str());
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
	if (!_stereoOutputEnabled && audioStream->isStereo())
		audioStream = new MonoAudioStream(audioStream);

	Audio::SoundHandle *handle = &buf->streamHandle;
	if (!buf->isStream) {
		handle = nullptr;
		for (int i = 0; i < kMaxSampleSlots; i++) {
			if (!_mixer->isSoundHandleActive(buf->handles[i])) {
				handle = &buf->handles[i];
				break;
			}
		}
		if (!handle) {
			delete audioStream;
			return;
		}
	}

	const byte volume = buf->usesCategoryVolume ? static_cast<byte>(Audio::Mixer::kMaxChannelVolume) : normalizeVolume(buf->volume);
	_mixer->playStream(getMixerSoundType(buf->category), handle, audioStream, -1, volume);
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

	buf->volume = CLIP(volume, 0, kMaxVolumePercent);
	buf->usesCategoryVolume = buf->volume == getCategoryVolume(buf->category);
	const byte channelVolume = buf->usesCategoryVolume ? static_cast<byte>(Audio::Mixer::kMaxChannelVolume) : normalizeVolume(buf->volume);

	for (int i = 0; i < kMaxSampleSlots; i++) {
		if (_mixer->isSoundHandleActive(buf->handles[i]))
			_mixer->setChannelVolume(buf->handles[i], channelVolume);
	}
	if (_mixer->isSoundHandleActive(buf->streamHandle))
		_mixer->setChannelVolume(buf->streamHandle, channelVolume);
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

void SoundManager::setVolumeSettings(int music, int sfx, int speech) {
	_volumeMusic = CLIP(music, 0, kMaxVolumePercent);
	_volumeSFX = CLIP(sfx, 0, kMaxVolumePercent);
	_volumeSpeech = CLIP(speech, 0, kMaxVolumePercent);
}

SoundBuffer *SoundManager::findBuffer(int id) const {
	for (uint i = 0; i < _buffers.size(); i++) {
		if (_buffers[i]->id == id)
			return _buffers[i];
	}
	return nullptr;
}

Common::Path SoundManager::resolveCompatibilityPath(const Common::Path &filename) const {
	if (_vm->hasResource(filename.toString('/')))
		return filename;

	// Some extracted-data layouts retain the numbered background tracks in the CD sound-effects directory.
	const Common::String str = filename.toString('/');
	const Common::String installedMusicPrefix("#sounds/music/");
	if (str.hasPrefix(installedMusicPrefix)) {
		const Common::String basename = str.substr(installedMusicPrefix.size());
		// Check if this is a numeric-prefix BB file (e.g. "01-BB01.wav")
		if (basename.size() > 6 && basename[2] == '-' && basename[3] == 'B' && basename[4] == 'B') {
			const Common::Path fxPath(Common::String::format("sounds/fx/%s", basename.c_str()));
			if (_vm->hasResource(fxPath.toString('/')))
				return fxPath;
		}
	}

	return filename;
}

SoundCategory SoundManager::classifySound(const Common::Path &filename) {
	Common::String path = filename.toString('/');
	if (!path.empty() && path[0] == '#')
		path = path.substr(1);
	else if (path.hasPrefixIgnoreCase("Data/"))
		path = path.substr(5);
	path.toLowercase();
	if (path.hasPrefix("sounds/music/"))
		return SoundCategory::kMusic00;
	if (path.hasPrefix("sounds/fx/") || path == "sounds/blip.wav")
		return SoundCategory::kSFX01;
	if (path.hasPrefix("sounds/"))
		return SoundCategory::kSpeech02;
	return SoundCategory::kSFX01;
}

Audio::Mixer::SoundType SoundManager::getMixerSoundType(SoundCategory category) {
	switch (category) {
	case SoundCategory::kMusic00:
		return Audio::Mixer::kMusicSoundType;
	case SoundCategory::kSpeech02:
		return Audio::Mixer::kSpeechSoundType;
	case SoundCategory::kSFX01:
	default:
		return Audio::Mixer::kSFXSoundType;
	}
}

int SoundManager::getCategoryVolume(SoundCategory category) const {
	switch (category) {
	case SoundCategory::kMusic00:
		return _volumeMusic;
	case SoundCategory::kSpeech02:
		return _volumeSpeech;
	case SoundCategory::kSFX01:
	default:
		return _volumeSFX;
	}
}

/** Normalize the game's zero-to-one-hundred volume into the mixer range. */
byte SoundManager::normalizeVolume(int volume) {
	return static_cast<byte>((CLIP(volume, 0, kMaxVolumePercent) * Audio::Mixer::kMaxChannelVolume) / kMaxVolumePercent);
}

} // End of namespace Zoombini2
