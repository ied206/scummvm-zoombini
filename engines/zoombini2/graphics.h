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

#ifndef ZOOMBINI2_GRAPHICS_H
#define ZOOMBINI2_GRAPHICS_H

#include "common/array.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/stream.h"

#include "graphics/managed_surface.h"

namespace Zoombini2 {

/**
 * Owns an uncompressed RGBA bitmap and an optional alpha mask.
 *
 * A BitBlock may be loaded from a color BMP, a color-and-alpha BMP pair, or
 * the game's cached BB format. Drawing clips source pixels to the destination
 * surface and preserves the destination outside the covered region.
 */
class BitBlock {
public:
	/** Construct an empty bitmap. */
	BitBlock();
	/** Release the owned pixel and alpha buffers. */
	~BitBlock();

	/** Load a mandatory color BMP and an optional alpha-mask BMP. */
	bool loadFromBMPPair(const Common::Path &colorPath, const Common::Path &alphaPath);
	/** Load a 24-bit color BMP without a separate alpha mask. */
	bool loadFromBMP(const Common::Path &colorPath);
	/** Load the cached BB representation at @p bbPath. */
	bool loadFromBB(const Common::Path &bbPath);

	/**
	 * Load a cached or source bitmap for @p basePath.
	 *
	 * The loader appends `.bb` first and falls back to `.bmp` when the cached
	 * representation is unavailable or invalid.
	 */
	bool load(const Common::Path &basePath);

	/** Allocate a zeroed bitmap and optionally a fully transparent alpha mask. */
	void createEmpty(int width, int height, bool withAlpha);

	/** Draw the full bitmap without alpha blending. */
	void drawToSurface(Graphics::ManagedSurface *dst, int x, int y) const;
	/** Draw @p srcRect from this bitmap without alpha blending. */
	void drawSubRect(Graphics::ManagedSurface *dst, int x, int y, const Common::Rect &srcRect) const;
	/** Draw the full bitmap through @p alphaLUT when an alpha mask is present. */
	void drawAlphaBlend(Graphics::ManagedSurface *dst, int x, int y, const byte alphaLUT[256][256]) const;

	/** Return the bitmap width in pixels. */
	int getWidth() const { return _width; }
	/** Return the bitmap height in pixels. */
	int getHeight() const { return _height; }
	/** Return whether this bitmap owns a separate alpha mask. */
	bool hasAlpha() const { return _alphaMap != nullptr; }
	/** Return the borrowed RGBA pixel buffer. */
	const byte *getPixels() const { return _pixels; }
	/** Return the borrowed alpha buffer, or nullptr when no mask is loaded. */
	const byte *getAlpha() const { return _alphaMap; }

private:
	/** Bitmap width in pixels. */
	int _width;
	/** Bitmap height in pixels. */
	int _height;
	/** Owned RGBA pixels with four bytes per pixel. */
	byte *_pixels;
	/** Owned one-byte-per-pixel alpha values, or nullptr. */
	byte *_alphaMap;

	/** Decode a 24-bit bottom-up BMP from @p stream. */
	bool loadColorBMP(Common::SeekableReadStream *stream);
	/** Decode an 8-bit alpha-mask BMP from @p stream. */
	bool loadAlphaBMP(Common::SeekableReadStream *stream);
};

/**
 * Owns one RLE-compressed sprite frame in the engine's drawing format.
 *
 * The frame header stores its dimensions and encoded byte count. Encoded spans
 * contain screen-relative coordinates, a pixel count, an opaque-or-alpha mode,
 * and expanded four-byte pixel records. Drawing clips malformed or off-screen
 * spans instead of writing outside the destination surface.
 */
class RleBlock {
public:
	/** Construct an empty RLE frame. */
	RleBlock();
	/** Release the owned RLE data. */
	~RleBlock();

	/** Load a complete RB record from @p stream. */
	bool loadFromStream(Common::SeekableReadStream *stream);
	/** Open and load a complete RB record from @p path. */
	bool loadFromFile(const Common::Path &path);
	/** Append `.rb` to @p basePath and load the resulting file. */
	bool load(const Common::Path &basePath);
	/** Load an AN frame whose header is followed by an inner size and RLE data. */
	bool loadHeaderAndData(Common::SeekableReadStream *stream, uint32 dataSize);
	/** Load an ANM frame whose header is followed directly by its RLE data. */
	bool loadAnmFrame(Common::SeekableReadStream *stream);

	/** Draw this frame at @p x and @p y, clipped to the destination surface. */
	void drawToScreen(Graphics::ManagedSurface *dst, int x, int y, const byte alphaLUT[256][256]) const;
	/** Draw this frame inside the supplied screen-space clipping rectangle. */
	void drawToScreenClipped(Graphics::ManagedSurface *dst, int x, int y, int clipLeft, int clipTop, int clipRight, int clipBottom,
							 const byte alphaLUT[256][256]) const;

	/** Return the frame width in pixels. */
	int getWidth() const { return _width; }
	/** Return the frame height in pixels. */
	int getHeight() const { return _height; }
	/** Return whether encoded frame data has been loaded. */
	bool isValid() const { return _rleData != nullptr; }

private:
	/** Frame width in pixels. */
	int32 _width;
	/** Frame height in pixels. */
	int32 _height;
	/** Number of bytes in @ref RleBlock::_rleData before in-place expansion. */
	uint32 _dataSize;
	/** Owned RLE span data using four bytes per encoded pixel after expansion. */
	byte *_rleData;
	/** Trailing header value retained with the decoded frame. */
	int32 _field20;

	/** Expand three-byte encoded pixels to the four-byte drawing representation. */
	void expand3to4bpp();
};

/** Owns the ordered RLE frames decoded from one AN animation file. */
class Animation {
public:
	/** Construct an animation without frames. */
	Animation();
	/** Release all owned frames. */
	~Animation();

	/** Load the complete frame sequence from @p path. */
	bool loadFromFile(const Common::Path &path);

	/** Return the number of loaded frames. */
	int getFrameCount() const { return _frames.size(); }
	/** Return a borrowed frame, or nullptr when @p index is outside the sequence. */
	const RleBlock *getFrame(int index) const;

private:
	/** Ordered frame sequence owned by this animation. */
	Common::Array<RleBlock *> _frames;
};

/**
 * Completion callback invoked by a non-looping @ref AnimationPlayer.
 *
 * @param userData Caller-owned context supplied to the player.
 * @param animId Caller-supplied identifier for the completed animation.
 */
typedef void (*AnimationCallback)(void *userData, int animId);

/**
 * Advances and draws a borrowed @ref Animation at a fixed frame interval.
 *
 * A player may loop or stop on the final frame. Non-looping playback invokes
 * its optional completion callback exactly when an update crosses the end of
 * the sequence. The player does not own the animation or callback context.
 */
class AnimationPlayer {
public:
	/** Construct an idle player with a 100-millisecond default frame interval. */
	AnimationPlayer();
	/** Destroy the animation player. */
	~AnimationPlayer();

	/** Select a borrowed animation and reset playback to its first frame. */
	void setAnimation(const Animation *animation);

	/**
	 * Start from the first frame and initialize timing on the next update.
	 *
	 * @param frameDelayMs Milliseconds assigned to each frame.
	 * @param loop Whether playback wraps at the end of the sequence.
	 * @param callback Optional callback for non-looping completion.
	 * @param userData Caller-owned context passed to @p callback.
	 * @param animId Caller-supplied identifier passed to @p callback.
	 */
	void play(uint32 frameDelayMs, bool loop, AnimationCallback callback = nullptr, void *userData = nullptr, int animId = 0);

	/**
	 * Start from the first frame using @p tickCount as the timing origin.
	 *
	 * @param tickCount Current engine tick count.
	 * @param frameDelayMs Milliseconds assigned to each frame.
	 * @param loop Whether playback wraps at the end of the sequence.
	 * @param callback Optional callback for non-looping completion.
	 * @param userData Caller-owned context passed to @p callback.
	 * @param animId Caller-supplied identifier passed to @p callback.
	 */
	void playAt(uint32 tickCount, uint32 frameDelayMs, bool loop, AnimationCallback callback = nullptr, void *userData = nullptr, int animId = 0);

	/** Stop playback and select the first frame without firing the callback. */
	void stop();
	/** Suspend frame advancement without changing the selected frame. */
	void pause();
	/** Allow a paused player to advance again. */
	void resume();

	/**
	 * Advance the selected frame to @p tickCount.
	 *
	 * @return true only when non-looping playback completes during this call.
	 */
	bool update(uint32 tickCount);

	/** Draw the selected frame when an animation is available. */
	void draw(Graphics::ManagedSurface *dst, int x, int y, const byte alphaLUT[256][256]) const;

	/** Return the selected zero-based frame index. */
	int getCurrentFrame() const { return _currentFrame; }
	/** Return whether playback is active and not paused. */
	bool isPlaying() const { return _playing && !_paused; }
	/** Return whether non-looping playback reached its final frame. */
	bool isFinished() const { return _finished; }
	/** Clamp and select @p frame when an animation with frames is available. */
	void setFrame(int frame);

private:
	/** Borrowed animation selected by @ref AnimationPlayer::setAnimation. */
	const Animation *_animation;
	/** Selected zero-based frame index. */
	int _currentFrame;
	/** Milliseconds assigned to each frame. */
	uint32 _frameDelayMs;
	/** Tick used as the origin for the next advancement calculation. */
	uint32 _lastFrameTime;
	/** Whether playback has started and has not stopped or completed. */
	bool _playing;
	/** Whether playback is temporarily suspended. */
	bool _paused;
	/** Whether advancement wraps after the last frame. */
	bool _loop;
	/** Whether non-looping playback has reached the final frame. */
	bool _finished;
	/** Optional non-owning completion callback. */
	AnimationCallback _callback;
	/** Caller-owned context passed to @ref AnimationPlayer::_callback. */
	void *_userData;
	/** Caller-supplied identifier passed to @ref AnimationPlayer::_callback. */
	int _animId;
};

/**
 * Owns the fixed three-dimensional sprite-frame grid loaded from an ANM file.
 *
 * The first index selects one of 100 movement or animation cells. The second
 * selects the body or one of four feature layers, and the third selects the
 * base image or one of five feature values.
 */
class ZoombiniGraphics {
public:
	/** Number of movement and animation cells. */
	static const int kDim0 = 100;
	/** Number of sprite layers per cell. */
	static const int kDim1 = 5;
	/** Number of base-or-feature variants per layer. */
	static const int kDim2 = 6;
	/** Total number of independently framed grid entries. */
	static const int kCellCount = kDim0 * kDim1 * kDim2;

	/** Construct an empty sprite grid. */
	ZoombiniGraphics();
	/** Release every frame owned by the sprite grid. */
	~ZoombiniGraphics();

	/** Load all @ref ZoombiniGraphics::kCellCount grid entries from @p path. */
	bool loadFromFile(const Common::Path &path);
	/** Return a borrowed frame, or nullptr when either index is invalid. */
	const RleBlock *getFrame(int cellIndex, int frameIndex) const;
	/** Return the number of frames in @p cellIndex, or zero for an invalid cell. */
	int getFrameCount(int cellIndex) const;

private:
	/** Owns the frames assigned to one sprite-grid entry. */
	struct Cell {
		/** Ordered frames stored in this grid entry. */
		Common::Array<RleBlock *> frames;
		/** Release every owned frame. */
		~Cell();
	};

	/** Fixed sprite grid indexed by cell, layer, and feature value. */
	Cell _cells[kCellCount];
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_GRAPHICS_H
