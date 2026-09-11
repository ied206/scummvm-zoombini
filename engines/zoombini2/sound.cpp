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
#include "common/memstream.h"
#include "common/substream.h"
#include "common/system.h"

#include "audio/audiostream.h"
#include "audio/decoders/raw.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/wave_types.h"
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
}

SoundManager::~SoundManager() {
	unloadAll();
}

int SoundManager::load(bool isStream, const Common::Path &filename, bool loop) {
	SoundBuffer *buf = new SoundBuffer();
	buf->id = _nextId;
	_nextId += 1;
	buf->path = resolveCompatibilityPath(filename);
	buf->isStream = isStream;
	buf->loop = loop;
	buf->category = classifySound(filename);
	buf->volume = kMaxVolumePercent;
	buf->usesCategoryVolume = true;
	if (!isStream) {
		Common::SeekableReadStream *file = _vm->openResourceFile(buf->path.toString('/'));
		if (!file) {
			warning("SoundManager::load: Cannot retain sample '%s'", buf->path.toString().c_str());
		} else {
			const int64 fileSize = file->size();
			if (0 < fileSize && fileSize <= UINT32_MAX) {
				buf->sampleData.resize(static_cast<uint32>(fileSize));
				const uint32 bytesRead = file->read(buf->sampleData.data(), static_cast<uint32>(fileSize));
				if (bytesRead != fileSize) {
					warning("SoundManager::load: Short read while retaining sample '%s' (expected %lld bytes, found %u)", buf->path.toString().c_str(),
							static_cast<long long int>(fileSize), bytesRead);
					buf->sampleData.clear();
				}
			} else {
				warning("SoundManager::load: Invalid sample size for '%s' (%lld bytes)", buf->path.toString().c_str(), static_cast<long long int>(fileSize));
			}
			delete file;
		}
	}
	_buffers.push_back(buf);
	return buf->id;
}

void SoundManager::unload(int id) {
	for (uint i = 0; i < _buffers.size(); i++) {
		if (_buffers[i]->id == id) {
			releasePlayback(*_buffers[i]);
			delete _buffers[i];
			_buffers.remove_at(i);
			return;
		}
	}
}

void SoundManager::unloadAll() {
	for (uint i = 0; i < _buffers.size(); i++) {
		releasePlayback(*_buffers[i]);
		delete _buffers[i];
	}
	_buffers.clear();
}

void SoundManager::play(int id) {
	SoundBuffer *buf = findBuffer(id);
	if (!buf)
		return;

	Audio::SoundHandle *handle = &buf->streamHandle;
	int sampleSlot = -1;
	if (!buf->isStream) {
		for (int i = 0; i < kMaxSampleSlots; i++) {
			if (!_mixer->isSoundHandleActive(buf->handles[i])) {
				handle = &buf->handles[i];
				sampleSlot = i;
				break;
			}
		}
		if (sampleSlot < 0 || buf->sampleData.empty())
			return;
	}

	Common::SeekableReadStream *f;
	if (buf->isStream) {
		f = _vm->openResourceFile(buf->path.toString('/'));
		if (!f) {
			debug(1, "SoundManager::play: Cannot open '%s'", buf->path.toString().c_str());
			return;
		}
	} else {
		f = new Common::MemoryReadStream(buf->sampleData.data(), static_cast<uint32>(buf->sampleData.size()));
	}

	Audio::RewindableAudioStream *stream = makeAudioStream(f, buf->path);
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

	const byte volume = buf->usesCategoryVolume ? static_cast<byte>(Audio::Mixer::kMaxChannelVolume) : normalizeVolume(buf->volume);
	if (buf->isStream && _mixer->isSoundHandleActive(buf->streamHandle))
		_mixer->stopHandle(buf->streamHandle);
	_mixer->playStream(getMixerSoundType(buf->category), handle, audioStream, -1, volume);
	if (buf->isStream)
		buf->streamPaused = false;
	else
		buf->handlesPaused[sampleSlot] = false;
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

	if (buf->isStream) {
		if (!buf->streamPaused && _mixer->isSoundHandleActive(buf->streamHandle)) {
			_mixer->pauseHandle(buf->streamHandle, true);
			buf->streamPaused = true;
		}
		return;
	}

	for (int i = 0; i < kMaxSampleSlots; i++) {
		if (_mixer->isSoundHandleActive(buf->handles[i]))
			_mixer->stopHandle(buf->handles[i]);
		buf->handlesPaused[i] = false;
	}
}

void SoundManager::pause(int id) {
	SoundBuffer *buf = findBuffer(id);
	if (!buf)
		return;

	if (buf->isStream) {
		if (!buf->streamPaused && _mixer->isSoundHandleActive(buf->streamHandle)) {
			_mixer->pauseHandle(buf->streamHandle, true);
			buf->streamPaused = true;
		}
		return;
	}

	for (int i = 0; i < kMaxSampleSlots; i++) {
		if (!buf->handlesPaused[i] && _mixer->isSoundHandleActive(buf->handles[i])) {
			_mixer->pauseHandle(buf->handles[i], true);
			buf->handlesPaused[i] = true;
		}
	}
}

void SoundManager::resume(int id) {
	SoundBuffer *buf = findBuffer(id);
	if (!buf)
		return;

	if (buf->isStream) {
		if (buf->streamPaused) {
			if (_mixer->isSoundHandleActive(buf->streamHandle))
				_mixer->pauseHandle(buf->streamHandle, false);
			buf->streamPaused = false;
		}
		return;
	}

	for (int i = 0; i < kMaxSampleSlots; i++) {
		if (buf->handlesPaused[i]) {
			if (_mixer->isSoundHandleActive(buf->handles[i]))
				_mixer->pauseHandle(buf->handles[i], false);
			buf->handlesPaused[i] = false;
		}
	}
}

bool SoundManager::isPlaying(int id) const {
	SoundBuffer *buf = findBuffer(id);
	if (!buf)
		return false;

	if (buf->isStream)
		return !buf->streamPaused && _mixer->isSoundHandleActive(buf->streamHandle);

	for (int i = 0; i < kMaxSampleSlots; i++) {
		if (!buf->handlesPaused[i] && _mixer->isSoundHandleActive(buf->handles[i]))
			return true;
	}
	return false;
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
	_muteRefCount += 1;
	if (_muteRefCount == 1) {
		_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, true);
		_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, true);
		_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, true);
	}
}

void SoundManager::unmute() {
	if (0 < _muteRefCount)
		_muteRefCount -= 1;
	if (_muteRefCount == 0) {
		_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, false);
		_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, false);
		_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, false);
	}
}

void SoundManager::pauseAll() {
	for (uint i = 0; i < _buffers.size(); i++)
		pause(_buffers[i]->id);
}

void SoundManager::resumeAll() {
	for (uint i = 0; i < _buffers.size(); i++)
		resume(_buffers[i]->id);
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

void SoundManager::releasePlayback(SoundBuffer &buffer) {
	for (int i = 0; i < kMaxSampleSlots; i++) {
		if (_mixer->isSoundHandleActive(buffer.handles[i]))
			_mixer->stopHandle(buffer.handles[i]);
		buffer.handlesPaused[i] = false;
	}
	if (_mixer->isSoundHandleActive(buffer.streamHandle))
		_mixer->stopHandle(buffer.streamHandle);
	buffer.streamPaused = false;
}

Audio::RewindableAudioStream *SoundManager::makeAudioStream(Common::SeekableReadStream *stream, const Common::Path &path) {
	const int64 initialPos = stream->pos();
	int dataSize;
	int rate;
	byte flags;
	uint16 wavType;
	if (!Audio::loadWAVFromStream(*stream, dataSize, rate, flags, &wavType)) {
		delete stream;
		return nullptr;
	}

	if (wavType != Audio::kWaveFormatPCM) {
		stream->seek(initialPos);
		return Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	}

	const int channels = (flags & Audio::FLAG_STEREO) ? 2 : 1;
	const int bytesPerSample = (flags & Audio::FLAG_24BITS) ? 3 : ((flags & Audio::FLAG_16BITS) ? 2 : 1);
	const int sampleFrameSize = channels * bytesPerSample;
	const int64 dataOffset = stream->pos();
	if (dataSize < 0 || stream->size() < dataOffset) {
		warning("SoundManager::makeAudioStream: Invalid PCM data bounds in '%s'", path.toString().c_str());
		delete stream;
		return nullptr;
	}

	const int64 availableDataSize = stream->size() - dataOffset;
	if (dataSize % sampleFrameSize == 0 && dataSize <= availableDataSize) {
		stream->seek(initialPos);
		return Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	}

	if (availableDataSize < dataSize) {
		warning("SoundManager::makeAudioStream: Truncated PCM data in '%s' (declared %d bytes, found %lld)",
				path.toString().c_str(), dataSize, static_cast<long long int>(availableDataSize));
	}

	const int64 boundedDataSize = MIN<int64>(dataSize, availableDataSize);
	const int64 completeDataSize = boundedDataSize - boundedDataSize % sampleFrameSize;
	if (completeDataSize == 0) {
		warning("SoundManager::makeAudioStream: No complete PCM sample frames in '%s'", path.toString().c_str());
		delete stream;
		return nullptr;
	}

	static constexpr int64 kMaxSubstreamPosition = UINT32_MAX;
	if (kMaxSubstreamPosition < dataOffset || kMaxSubstreamPosition - dataOffset < completeDataSize) {
		warning("SoundManager::makeAudioStream: PCM data range is too large in '%s'", path.toString().c_str());
		delete stream;
		return nullptr;
	}

	debug(2, "SoundManager::makeAudioStream: Ignoring %lld incomplete PCM bytes in '%s'",
		  static_cast<long long int>(boundedDataSize - completeDataSize), path.toString().c_str());
	const uint32 dataStart = static_cast<uint32>(dataOffset);
	const uint32 dataEnd = static_cast<uint32>(dataOffset + completeDataSize);
	Common::SeekableReadStream *dataStream = new Common::SeekableSubReadStream(stream, dataStart, dataEnd, DisposeAfterUse::YES);
	return Audio::makeRawStream(dataStream, rate, flags);
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
