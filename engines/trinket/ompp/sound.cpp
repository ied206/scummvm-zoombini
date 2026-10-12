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

#include "trinket/ompp/sound.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/util.h"

namespace Trinket {
namespace OM {

bool Sound::play(const ResourceProvider &resources, const Common::String &path, Audio::Mixer::SoundType type, uint count, uint volume) {
	Common::SeekableReadStream *source = resources.open(path);
	if (!source)
		return false;
	Audio::SeekableAudioStream *wave = Audio::makeWAVStream(source, DisposeAfterUse::YES);
	if (!wave)
		return false;
	stop();
	_mixer.playStream(type, &_handle, Audio::makeLoopingAudioStream(wave, count), -1, MIN<uint>(volume, 100) * 255 / 100);
	return true;
}

} // namespace OM
} // namespace Trinket
