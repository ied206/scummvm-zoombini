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

#include "trinket/core/animation.h"

#include "common/endian.h"

namespace Trinket {
namespace Core {

AnimationPlayer::AnimationPlayer() = default;

bool AnimationPlayer::start(const Animation &animation, uint sequence, int repeat) {
	if (animation.celCount() <= sequence)
		return false;
	const Common::Array<Animation::Step> &steps = animation.sequence(sequence);
	uint first = 0;
	while (first < steps.size() && steps[first].picture < 0)
		first += 1;
	if (first == steps.size())
		return false;
	_animation = &animation;
	_steps = steps;
	_fps = animation.fps();
	_position = steps[0].picture == -4 ? 1 : 0;
	_picture = steps[first].picture;
	_picturePosition = first;
	_offset = Common::Point(steps[first].x, steps[first].y);
	_repeat = repeat;
	_deadline = 0;
	_state = kPlaying01;
	return true;
}

bool AnimationPlayer::replaceSequence(const Common::Array<Animation::Step> &steps) {
	if (!_animation || steps.empty())
		return false;
	bool hasPicture = false;
	for (uint i = 0; i < steps.size(); i++) {
		if (steps[i].picture == -1)
			break;
		if (0 <= steps[i].picture) {
			if (_animation->frameCount() <= static_cast<uint>(steps[i].picture))
				return false;
			hasPicture = true;
		}
	}
	if (!hasPicture)
		return false;
	_steps = steps;
	setFrame(1);
	return true;
}

void AnimationPlayer::insertSequenceEntry(uint position, const Animation::Step &step) {
	if (!_animation || lastSequencePosition() < position)
		return;
	_steps.insert_at(position, step);
	if (position <= _position)
		_position += 1;
}

void AnimationPlayer::removeSequenceEntry(uint position) {
	if (!_animation || lastSequencePosition() <= position)
		return;
	_steps.remove_at(position);
	if (position < _position)
		_position -= 1;
}

void AnimationPlayer::pause(bool paused) {
	if (paused && _state == kPlaying01) {
		_state = kPaused02;
	} else if (!paused && _state == kPaused02) {
		_state = kPlaying01;
		_deadline = 0;
	}
}

uint AnimationPlayer::lastSequencePosition() const {
	if (!_animation)
		return 0;
	const Common::Array<Animation::Step> &steps = _steps;
	uint position = 0;
	while (position + 1 < steps.size() && steps[position].picture != -1)
		position += 1;
	return position;
}

void AnimationPlayer::setFrame(int frame) {
	if (!_animation || frame <= 0)
		return;
	const Common::Array<Animation::Step> &steps = _steps;
	uint last = lastSequencePosition();
	uint position = 0;
	if (0 <= steps[position].picture)
		frame -= 1;
	while (frame && position < last) {
		position += 1;
		if (0 <= steps[position].picture)
			frame -= 1;
	}
	while (position && steps[position].picture < 0)
		position -= 1;
	_position = position;
	if (0 <= steps[position].picture) {
		_picturePosition = position;
		_picture = steps[position].picture;
		_offset = Common::Point(steps[position].x, steps[position].y);
	}
	_state = kPaused02;
}

uint AnimationPlayer::getFrame() const {
	if (!_animation)
		return 0;
	const Common::Array<Animation::Step> &steps = _steps;
	uint frame = 0;
	for (uint i = 0; i <= _position && i < steps.size(); i++) {
		if (0 <= steps[i].picture)
			frame += 1;
	}
	return frame;
}

uint AnimationPlayer::getNumberOfFrames() const {
	if (!_animation)
		return 0;
	const Common::Array<Animation::Step> &steps = _steps;
	uint count = 0;
	for (uint i = 0; i < steps.size(); i++) {
		if (steps[i].picture == -1)
			break;
		if (0 <= steps[i].picture)
			count += 1;
	}
	return count;
}

uint AnimationPlayer::displayedFrame() const {
	uint frame = 0;
	for (uint i = 0; i <= _picturePosition && i < _steps.size(); i++) {
		if (0 <= _steps[i].picture)
			frame += 1;
	}
	return frame;
}

bool AnimationPlayer::update(uint32 time, Common::Array<uint16> &sounds) {
	if (!_animation || _state != kPlaying01 || (_deadline && static_cast<int32>(time - _deadline) < 0))
		return true;
	const Common::Array<Animation::Step> &steps = _steps;
	if (steps.size() <= _position) {
		_state = kStopped00;
		return true;
	}
	uint duration = 0;
	if (_fps)
		duration = 1000 / _fps;
	else if (0 <= steps[_position].picture)
		duration = static_cast<uint16>(steps[_position].duration);
	if (!_deadline)
		_deadline = time;
	_deadline += duration;
	if (static_cast<int32>(time - _deadline) > static_cast<int32>(duration / 2))
		_deadline = time + duration;
	// A malformed command-only loop must not stall the engine event loop.
	for (uint commands = 0; commands <= steps.size() * 2; commands++) {
		if (steps.size() <= _position)
			break;
		const Animation::Step &step = steps[_position];
		if (0 <= step.picture) {
			_picturePosition = _position;
			_picture = step.picture;
			_offset = Common::Point(step.x, step.y);
			if (_state == kPlaying01)
				_position += 1;
			return true;
		}
		if (step.picture == -101) {
			sounds.push_back(static_cast<uint16>(step.duration));
			_position += 1;
		} else if (step.picture == -5) {
			_position += 1;
		} else if (step.picture == -4) {
			if (!_repeat) {
				_state = kPaused02;
				if (_pauseListener && !_pauseListener->animationPaused())
					return false;
			}
			// The callback may have selected a picture or replaced the sequence.
			if (_position < _steps.size() && _steps[_position].picture < 0)
				_position += 1;
		} else if (step.picture == -1 && !_repeat) {
			_state = kStopped00;
			_position = 0;
			return true;
		} else if (step.picture == -1 || step.picture == -2) {
			if (_repeat == 1) {
				_repeat = 0;
				_state = kPaused02;
				if (_pauseListener && !_pauseListener->animationPaused())
					return false;
				while (_position && steps[_position].picture < 0)
					_position -= 1;
				return true;
			}
			if (0 < _repeat)
				_repeat -= 1;
			_position = 0;
		} else {
			// Script commands require the owning script object's callback.
			_state = kPaused02;
			return true;
		}
	}
	_state = kPaused02;
	return true;
}

bool Animation::contains(uint32 offset, uint32 size) const {
	return offset <= _data.size() && size <= _data.size() - offset;
}

uint16 Animation::readUint16(uint32 offset) const {
	return _bigEndian ? READ_BE_UINT16(&_data[offset]) : READ_LE_UINT16(&_data[offset]);
}

uint32 Animation::readUint32(uint32 offset) const {
	return _bigEndian ? READ_BE_UINT32(&_data[offset]) : READ_LE_UINT32(&_data[offset]);
}

bool Animation::load(Common::SeekableReadStream &stream) {
	_hasPalette = false;
	_pictures.clear();
	_sequences.clear();
	if (stream.size() < 10 || stream.size() > 64 * 1024 * 1024)
		return false;
	_data.resize(stream.size());
	if (stream.read(&_data[0], _data.size()) != _data.size())
		return false;
	_bigEndian = _data[5] != 0;
	int cels = static_cast<int16>(readUint16(0));
	int pictures = static_cast<int16>(readUint16(2));
	_fps = _data[4];
	_version = readUint16(8);
	if (cels <= 0 || pictures <= 0 || 1 < _data[5] || 2 < _version)
		return false;
	uint32 pos = 10;
	if (!contains(pos, pictures * 4 + cels * 4))
		return false;
	for (int i = 0; i < pictures; i++) {
		_pictures.push_back(readUint32(pos));
		pos += 4;
	}
	int x = _data[pos] | ((_data[pos + 2] & 15) << 8);
	int y = _data[pos + 1] | ((_data[pos + 2] & 240) << 4);
	if (2048 < x)
		x = 2048 - x;
	if (2048 < y)
		y = 2048 - y;
	_origin = Common::Point(x, y);
	pos += cels * 4;
	for (int i = 0; i < cels; i++) {
		if (!contains(pos, 2))
			return false;
		int count = static_cast<int16>(readUint16(pos));
		pos += 2;
		if (count < 0 || !contains(pos, count * 8))
			return false;
		Common::Array<Step> sequence;
		for (int j = 0; j < count; j++) {
			Step step;
			step.x = static_cast<int16>(readUint16(pos));
			step.y = static_cast<int16>(readUint16(pos + 2));
			step.picture = static_cast<int16>(readUint16(pos + 4));
			step.duration = static_cast<int16>(readUint16(pos + 6));
			if (pictures <= step.picture)
				return false;
			sequence.push_back(step);
			pos += 8;
		}
		_sequences.push_back(sequence);
	}
	_pictureBase = pos;
	for (uint i = 0; i < _pictures.size(); i++) {
		if (_data.size() - pos <= _pictures[i])
			return false;
	}
	return true;
}

bool Animation::decode(uint frame, Graphics::Surface &surface, const byte *palette) const {
	if (!palette && _hasPalette)
		palette = _palette;
	if (_pictures.size() <= frame)
		return false;
	uint32 base = _pictureBase + _pictures[frame];
	if (!contains(base, 6))
		return false;
	uint type = readUint16(base);
	int width = static_cast<int16>(readUint16(base + 2));
	int height = static_cast<int16>(readUint16(base + 4));
	if (width <= 0 || height <= 0 || 4096 < width || 4096 < height)
		return false;
	if ((type != 2 && type != 4) || (type == 4 && !palette))
		return false;
	if (type == 2 && !contains(base, 8 + height * 4))
		return false;
	surface.free();
	surface.create(width, height, Graphics::PixelFormat::createFormatRGBA32());
	surface.fillRect(Common::Rect(width, height), 0);
	uint32 pos = base + 6;
	for (int y = 0; y < height; y++) {
		if (type == 2) {
			uint32 line = readUint32(base + 8 + y * 4);
			if (_data.size() - base <= line)
				return false;
			pos = base + line;
		}
		int x = 0;
		while (x < width) {
			uint opcode, count;
			byte index = 0;
			if (type == 2) {
				if (!contains(pos, 4))
					return false;
				opcode = readUint16(pos);
				count = readUint16(pos + 2);
				pos += 4;
				if (2 < opcode)
					return false;
			} else {
				if (!contains(pos, 1))
					return false;
				opcode = _data[pos];
				pos += 1;
				count = opcode;
				if (opcode == 255) {
					if (!contains(pos, 1))
						return false;
					index = _data[pos];
					pos += 1;
				}
				if (opcode == 0 || opcode == 255) {
					if (!contains(pos, 1))
						return false;
					count = _data[pos];
					pos += 1;
				}
			}
			if (!count || static_cast<uint>(width - x) < count)
				return false;
			for (uint i = 0; i < count; i++) {
				byte r = 0, g = 0, b = 0, a = 0;
				if (opcode && type == 2) {
					uint bytes = opcode == 2 ? 3 : 2;
					if (!contains(pos, bytes))
						return false;
					uint16 pixel = readUint16(pos);
					r = (pixel >> 10) & 31;
					g = (pixel >> 5) & 31;
					b = pixel & 31;
					r = (r << 3) | (r >> 2);
					g = (g << 3) | (g >> 2);
					b = (b << 3) | (b >> 2);
					a = opcode == 2 ? _data[pos + 2] : 255;
					pos += bytes;
				} else if (opcode) {
					if (opcode != 255) {
						if (!contains(pos, 1))
							return false;
						index = _data[pos];
						pos += 1;
					}
					r = palette[index * 3];
					g = palette[index * 3 + 1];
					b = palette[index * 3 + 2];
					a = 255;
				}
				*static_cast<uint32 *>(surface.getBasePtr(x, y)) = surface.format.ARGBToColor(a, r, g, b);
				x += 1;
			}
		}
	}
	return true;
}

bool Animation::loadPalette(Common::SeekableReadStream &stream) {
	if (stream.size() - stream.pos() != 1536)
		return false;
	for (uint i = 0; i < 768; i++) {
		_palette[i] = stream.readByte();
		stream.readByte();
	}
	_hasPalette = !stream.err();
	return _hasPalette;
}

int Animation::pictureForFrame(uint frame, uint sequence) const {
	if (_sequences.size() <= sequence)
		return -1;
	const Common::Array<Step> &steps = _sequences[sequence];
	for (uint i = 0; i < steps.size(); i++) {
		if (steps[i].picture == -1)
			break;
		if (0 <= steps[i].picture) {
			if (!frame)
				return steps[i].picture;
			frame -= 1;
		}
	}
	return -1;
}

} // namespace Core
} // namespace Trinket
