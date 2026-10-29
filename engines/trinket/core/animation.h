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
#include "common/ptr.h"
#include "common/rect.h"
#include "common/stream.h"
#include "graphics/surface.h"

namespace Trinket {
namespace Core {

/** An AO resource: embedded origin, picture frames and cel sequences. */
class Animation {
public:
	/** One authored sequence entry; picture and duration retain native sentinel values. */
	struct Step {
		Common::Point offset; ///< Frame offset in game pixels.
		int16 picture;  ///< Picture index or native command sentinel.
		int16 duration; ///< Frame delay in milliseconds when FPS is zero, or sound ID when picture is -101.
	};
	/** Construct an empty asset with no cached pictures. */
	Animation() = default;
	/** Replace source data and parsed sequences, invalidating cached pictures. */
	bool load(Common::SeekableReadStream &stream);
	/** Install a resource palette and invalidate pictures decoded with the old palette. */
	bool loadPalette(Common::SeekableReadStream &stream);
	/** Decode a frame into the supplied surface, optionally using a replacement palette. */
	bool decode(uint frame, Graphics::Surface &surface, const byte *palette = nullptr) const;
	/** Return a cached default-palette picture, valid until the animation reloads or is destroyed. */
	const Graphics::Surface *picture(uint frame) const;
	/** Return bytes retained by decoded pictures, excluding allocation overhead. */
	uint64 pictureBytes() const { return _pictureBytes; }
	/** Return requests served by already-decoded pictures. */
	uint64 pictureHits() const { return _pictureHits; }
	/** Return picture decode attempts, including explicit alternate-palette decodes. */
	uint64 decodeCount() const { return _decodeCount; }
	/** Return retained AO source bytes, excluding parsed sequence storage. */
	uint64 sourceBytes() const { return _data.size(); }
	uint frameCount() const { return _pictures.size(); }                            ///< Return the number of stored pictures.
	uint celCount() const { return _sequences.size(); }                             ///< Return the number of authored sequences.
	uint fps() const { return _fps; }                                               ///< Return the authored frames per second.
	Common::Point origin() const { return _origin; }                                ///< Return the asset origin in game pixels.
	const Common::Array<Step> &sequence(uint cel) const { return _sequences[cel]; } ///< Return the authored sequence at a valid index.
	int pictureForFrame(uint frame, uint sequence = 0) const;                       ///< Map a logical frame to its picture or a negative sentinel.

private:
	/** Decoded picture whose pixel storage remains stable when cache entries move. */
	struct Picture {
		Graphics::Surface surface;     ///< Decoded RGBA pixels, unchanged after a successful decode.
		~Picture() { surface.free(); } ///< Free the decoded pixel buffer.
	};
	/** Picture-indexed slots; empty slots have not been successfully decoded. */
	mutable Common::Array<Common::SharedPtr<Picture>> _decodedPics;
	/** Bytes occupied by decoded pixel buffers. */
	mutable uint64 _pictureBytes = 0;
	/** Cumulative successful picture-cache hits. */
	mutable uint64 _pictureHits = 0;
	/** Cumulative picture decode attempts for this animation instance. */
	mutable uint64 _decodeCount = 0;
	Common::Array<byte> _data;                       ///< Source bytes retained for lazy picture decoding.
	Common::Array<uint32> _pictures;                 ///< Picture offsets relative to the picture block.
	Common::Array<Common::Array<Step>> _sequences;   ///< Immutable authored sequence entries.
	Common::Point _origin;                           ///< Asset origin in game pixels.
	uint _fps = 0;                                   ///< Authored frame rate.
	uint _version = 0;                               ///< Original AO format version.
	uint32 _pictureBase = 0;                         ///< Byte offset of the picture block in the source.
	byte _palette[768];                              ///< Default RGB palette, indexed by color then component.
	bool _hasPalette = false;                        ///< Whether the default palette is available.
	bool _bigEndian = false;                         ///< Whether multi-byte fields use big-endian encoding.
	bool contains(uint32 offset, uint32 size) const; ///< Check a source byte range.
	uint16 readUint16(uint32 offset) const;          ///< Read a source word using the asset byte order.
	uint32 readUint32(uint32 offset) const;          ///< Read a source double word using the asset byte order.
	void clearPictures();                            ///< Invalidate default-palette pictures without changing cumulative counters.
};

/** Playback state is independent of the shared AO image data. */
class AnimationPlayer {
public:
	/** Called before processing the picture following an authored pause. */
	class PauseListener {
	public:
		/** Allow destruction through the pause-listener interface. */
		virtual ~PauseListener() {}
		/** Return false if this playback was detached or destroyed by the callback. */
		virtual bool animationPaused() = 0;
	};
	/** Create a stopped player with no animation bound. */
	AnimationPlayer();
	/** Copy the selected sequence and begin playback, returning false if the sequence is invalid. */
	bool start(const Animation &animation, uint sequence, int repeat);
	/** Pause or resume sequence advancement without hiding the current picture. */
	void pause(bool paused);
	/** Advance playback and append triggered sounds; return false if a pause callback detaches the player. */
	bool update(uint32 time, Common::Array<uint16> &sounds);
	/** Set the listener notified by authored pause entries, or detach it with null. */
	void setPauseListener(PauseListener *listener) { _pauseListener = listener; }
	/** Seek to a positive one-based picture frame and pause playback. */
	void setFrame(int frame);
	/** Return the one-based picture frame at the playback position. */
	uint getFrame() const;
	/** Return the one-based frame whose picture is currently displayed. */
	uint displayedFrame() const;
	/** Count picture entries before the sequence terminator. */
	uint getNumberOfFrames() const;
	/** Set the frame rate; zero uses each sequence entry's millisecond duration. */
	void setFPS(uint fps) { _fps = fps; }
	/** Set the remaining repeat count; negative values repeat indefinitely. */
	void setRepeat(int repeat) { _repeat = repeat; }
	/** Return this player's sequence entries, including control markers. */
	const Common::Array<Animation::Step> &sequence() const { return _steps; }
	/** Replace playback entries while retaining the animation, returning false for an invalid sequence. */
	bool replaceSequence(const Common::Array<Animation::Step> &steps);
	/** Insert an entry before a valid sequence position. */
	void insertSequenceEntry(uint position, const Animation::Step &step);
	/** Remove an entry before the sequence terminator. */
	void removeSequenceEntry(uint position);
	/** Return the animation picture index currently selected for drawing. */
	uint picture() const { return _picture; }
	/** Return the current sequence entry's offset in game pixels. */
	Common::Point offset() const { return _offset; }
	/** Return whether playback is actively advancing. */
	bool isRunning() const { return _state == kPlaying01; }
	/** Return whether the current picture remains visible, including while paused. */
	bool isVisible() const { return _state != kStopped00; }
	/** Resume advancement and clear the deadline so the next update runs immediately. */
	void activate() {
		_state = kPlaying01;
		_deadline = 0;
	}
	/** Hide the player and reset its sequence position and timing. */
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
	/** Animation supplying pictures for the copied playback sequence. */
	const Animation *_anim = nullptr;
	/** Sequence entries used by this player, including control and sound entries. */
	Common::Array<Animation::Step> _steps;
	/** Index of the next sequence entry to process. */
	uint _position = 0;
	/** Sequence index that supplied the currently displayed picture. */
	uint _picPos = 0;
	/** Animation picture index used for the current frame. */
	uint _picture = 0;
	/** Playback frames per second, or zero for per-entry durations. */
	uint _fps = 0;
	/** Remaining repeats; negative values request indefinite playback. */
	int _repeat = 0;
	/** Millisecond time when the next sequence entry becomes due. */
	uint32 _deadline = 0;
	/** Current picture displacement in game pixels. */
	Common::Point _offset;
	/** Playback state controlling advancement and picture visibility. */
	State _state = kStopped00;
	/** Listener for authored pause markers, or null if detached. */
	PauseListener *_pauseListener = nullptr;
	/** Return the sequence terminator's index, or the final available entry. */
	uint lastSequencePosition() const;
};

} // End of namespace Core
} // End of namespace Trinket

#endif
