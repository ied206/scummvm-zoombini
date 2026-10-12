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

#ifndef TRINKET_CORE_ANIMATION_H
#define TRINKET_CORE_ANIMATION_H

#include "common/array.h"
#include "common/rect.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace Trinket {
namespace Core {

/** An AO resource: embedded origin, picture frames and cel sequences. */
class Animation {
public:
	struct Step {
		int16 x, y, picture, duration;
	};
	Animation() = default;
	bool load(Common::SeekableReadStream &stream);
	bool loadPalette(Common::SeekableReadStream &stream);
	bool decode(uint frame, Graphics::Surface &surface, const byte *palette = nullptr) const;
	uint frameCount() const { return _pictures.size(); }
	uint celCount() const { return _sequences.size(); }
	uint fps() const { return _fps; }
	Common::Point origin() const { return _origin; }
	const Common::Array<Step> &sequence(uint cel) const { return _sequences[cel]; }
	int pictureForFrame(uint frame, uint sequence = 0) const;

private:
	Common::Array<byte> _data;
	Common::Array<uint32> _pictures;
	Common::Array<Common::Array<Step>> _sequences;
	Common::Point _origin;
	uint _fps = 0;
	uint _version = 0;
	uint32 _pictureBase = 0;
	byte _palette[768];
	bool _hasPalette = false;
	bool _bigEndian = false;
	bool contains(uint32 offset, uint32 size) const;
	uint16 readUint16(uint32 offset) const;
	uint32 readUint32(uint32 offset) const;
};

/** Playback state is independent of the shared AO image data. */
class AnimationPlayer {
public:
	/** Called before processing the picture following an authored pause. */
	class PauseListener {
	public:
		virtual ~PauseListener() {}
		/** Return false if this playback was detached or destroyed by the callback. */
		virtual bool animationPaused() = 0;
	};
	AnimationPlayer();
	bool start(const Animation &animation, uint sequence, int repeat);
	void pause(bool paused);
	bool update(uint32 time, Common::Array<uint16> &sounds);
	void setPauseListener(PauseListener *listener) { _pauseListener = listener; }
	void setFrame(int frame);
	uint getFrame() const;
	uint displayedFrame() const;
	uint getNumberOfFrames() const;
	void setFPS(uint fps) { _fps = fps; }
	void setRepeat(int repeat) { _repeat = repeat; }
	const Common::Array<Animation::Step> &sequence() const { return _steps; }
	bool replaceSequence(const Common::Array<Animation::Step> &steps);
	void insertSequenceEntry(uint position, const Animation::Step &step);
	void removeSequenceEntry(uint position);
	uint picture() const { return _picture; }
	Common::Point offset() const { return _offset; }
	bool isRunning() const { return _state == kPlaying01; }
	bool isVisible() const { return _state != kStopped00; }
	void activate() {
		_state = kPlaying01;
		_deadline = 0;
	}
	void stop() {
		_state = kStopped00;
		_position = 0;
		_deadline = 0;
	}

private:
	enum State {
		kStopped00 = 0,
		kPlaying01 = 1,
		kPaused02 = 2,
	};
	const Animation *_animation = nullptr;
	Common::Array<Animation::Step> _steps;
	uint _position = 0, _picturePosition = 0, _picture = 0, _fps = 0;
	int _repeat = 0;
	uint32 _deadline = 0;
	Common::Point _offset;
	State _state = kStopped00;
	PauseListener *_pauseListener = nullptr;
	uint lastSequencePosition() const;
};

} // End of namespace Core
} // End of namespace Trinket

#endif
