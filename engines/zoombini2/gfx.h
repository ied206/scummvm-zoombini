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

#ifndef ZOOMBINI2_GFX_H
#define ZOOMBINI2_GFX_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/path.h"
#include "common/stream.h"
#include "common/rect.h"

#include "graphics/managed_surface.h"

namespace Zoombini2 {

/**
 * CBitBlock — uncompressed bitmap sprite.
 *
 * Original struct: +0=alphaMap, +4=pPixels, +8=bufSize, +12=width, +16=height.
 * ReadBMP:  CBitBlock__ReadBMP_4573D0 — reads 24-bit color BMP.
 * ReadAlpha: CBitBlock__ReadAlphaBMP_4571A0 — reads 8-bit alpha mask BMP.
 */
class BitBlock {
public:
	BitBlock();
	~BitBlock();

	bool loadFromBMPPair(const Common::Path &colorPath, const Common::Path &alphaPath);
	bool loadFromBMP(const Common::Path &colorPath);
	bool loadFromBB(const Common::Path &bbPath);

	/**
	 * Unified loader: tries .bb first, then .bmp, mirroring
	 * CBitBlock__Read_457670 which caches BMP→BB.
	 * @param basePath  Path without extension (e.g. "bmp/story_intro/title_screen").
	 */
	bool load(const Common::Path &basePath);

	/**
	 * Create an empty bitmap with given dimensions, optionally with alpha.
	 * Pixels are zeroed; alpha (if enabled) is zeroed (fully transparent).
	 */
	void createEmpty(int width, int height, bool withAlpha);

	void drawToSurface(Graphics::ManagedSurface *dst, int x, int y) const;
	void drawSubRect(Graphics::ManagedSurface *dst, int x, int y,
	                 const Common::Rect &srcRect) const;
	void drawAlphaBlend(Graphics::ManagedSurface *dst, int x, int y,
	                    const byte alphaLUT[256][256]) const;

	int getWidth() const { return _width; }
	int getHeight() const { return _height; }
	bool hasAlpha() const { return _alphaMap != nullptr; }
	const byte *getPixels() const { return _pixels; }
	const byte *getAlpha() const { return _alphaMap; }

private:
	int _width;
	int _height;
	byte *_pixels;    // RGBA pixel data (4 bytes per pixel)
	byte *_alphaMap;  // Alpha channel (1 byte per pixel), may be null

	bool loadColorBMP(Common::SeekableReadStream *stream);
	bool loadAlphaBMP(Common::SeekableReadStream *stream);
};

/**
 * CRleBlock — RLE-compressed sprite.
 *
 * Original: CRleBlock__CRleBlock_45AC90
 *
 * File header (24 bytes):
 *   +0:  DWORD field0    (overwritten with first int16 of data)
 *   +4:  DWORD (pData placeholder, overwritten)
 *   +8:  DWORD dataSize
 *   +12: DWORD width
 *   +16: DWORD height
 *   +20: DWORD field20
 *
 * After the header: 4-byte dataSize (redundant), then RLE data.
 *
 * RLE data layout:
 *   int16 effectiveHeight (first 2 bytes, skipped in draw)
 *   Span blocks until end of data:
 *     int16 x_offset
 *     int16 y_offset
 *     int16 pixel_count
 *     byte  mode (0=opaque, nonzero=alpha)
 *     Pixel data:
 *       mode 0: pixel_count * 4 bytes (R, G, B, pad) [after 3→4 expand]
 *       mode 1: pixel_count * 4 bytes (premultR, premultG, premultB, invAlpha)
 */
class RleBlock {
public:
	RleBlock();
	~RleBlock();

	bool loadFromStream(Common::SeekableReadStream *stream);
	bool loadFromFile(const Common::Path &path);

	/**
	 * Unified loader: appends .rb extension and loads.
	 * @param basePath  Path without extension (e.g. "bmp/map/icon00").
	 */
	bool load(const Common::Path &basePath);

	/** Load only header + data from a stream (used by .an loaders).
	 *  Format: 24-byte header, 4-byte inner dataSize, then RLE data. */
	bool loadHeaderAndData(Common::SeekableReadStream *stream, uint32 dataSize);

	/** Load a frame from a .anm stream (ZoombiniGfx format).
	 *  Format: 24-byte header then exactly _dataSize bytes of RLE data.
	 *  Unlike loadHeaderAndData, does NOT read an extra inner 4-byte size field.
	 *  Original: CompressGfxZomb_Read_45C4D0 per-frame read sequence. */
	bool loadAnmFrame(Common::SeekableReadStream *stream);

	void drawToScreen(Graphics::ManagedSurface *dst, int x, int y,
	                  const byte alphaLUT[256][256]) const;

	/**
	 * Draw RLE sprite to surface with screen-space clip rectangle.
	 * Original: CRleBlock__DrawToScreenClipped_45B410
	 */
	void drawToScreenClipped(Graphics::ManagedSurface *dst, int x, int y,
	                         int clipLeft, int clipTop, int clipRight, int clipBottom,
	                         const byte alphaLUT[256][256]) const;

	int getWidth() const { return _width; }
	int getHeight() const { return _height; }
	bool isValid() const { return _rleData != nullptr; }

private:
	int32 _width;
	int32 _height;
	uint32 _dataSize;
	byte *_rleData;    // RLE data (always 4bpp after expansion)
	int32 _field20;

	void expand3to4bpp();
};

/**
 * CAnimation — animation sequence from .an cache files.
 *
 * Original: CAnimation__Read_455F30
 * Format: DWORD frameCount, per frame: 24-byte header + 4-byte dataSize + RLE data.
 */
class Animation {
public:
	Animation();
	~Animation();

	bool loadFromFile(const Common::Path &path);

	int getFrameCount() const { return _frames.size(); }
	const RleBlock *getFrame(int index) const;

private:
	Common::Array<RleBlock *> _frames;
};

/**
 * Callback function type for animation completion.
 * @param userData  User-provided context pointer
 * @param animId    Animation ID passed when starting playback
 */
typedef void (*AnimationCallback)(void *userData, int animId);

/**
 * AnimationPlayer — timed animation playback with callback support.
 *
 * Manages animation state machine:
 *   - Tracks current frame and elapsed time
 *   - Supports one-shot and looping modes
 *   - Fires callback when one-shot animation completes
 *   - Supports frame delay configuration
 *
 * Usage:
 *   AnimationPlayer player;
 *   player.setAnimation(myAnim);
 *   player.play(100, false, myCallback, this, 42);  // 100ms/frame, one-shot
 *   // In update loop:
 *   player.update(tickCount);
 *   // In draw:
 *   player.draw(screen, x, y, lut);
 */
class AnimationPlayer {
public:
	AnimationPlayer();
	~AnimationPlayer();

	/**
	 * Set the animation to play. Does not start playback.
	 */
	void setAnimation(const Animation *anim);

	/**
	 * Start or restart playback.
	 * @param frameDelayMs   Milliseconds per frame
	 * @param loop           True for looping, false for one-shot
	 * @param callback       Optional callback fired when one-shot completes
	 * @param userData       User context passed to callback
	 * @param animId         ID passed to callback (for identifying which animation)
	 */
	void play(uint32 frameDelayMs, bool loop,
	          AnimationCallback callback = nullptr,
	          void *userData = nullptr, int animId = 0);

	/**
	 * Start playback with tick time for synchronization.
	 */
	void playAt(uint32 tickCount, uint32 frameDelayMs, bool loop,
	            AnimationCallback callback = nullptr,
	            void *userData = nullptr, int animId = 0);

	/**
	 * Stop playback. Does not fire callback.
	 */
	void stop();

	/**
	 * Pause playback.
	 */
	void pause();

	/**
	 * Resume paused playback.
	 */
	void resume();

	/**
	 * Update animation state. Call once per frame.
	 * @param tickCount  Current tick count (from engine)
	 * @return true if animation completed this frame (one-shot only)
	 */
	bool update(uint32 tickCount);

	/**
	 * Draw current frame to screen.
	 */
	void draw(Graphics::ManagedSurface *dst, int x, int y,
	          const byte alphaLUT[256][256]) const;

	/**
	 * Get current frame index.
	 */
	int getCurrentFrame() const { return _currentFrame; }

	/**
	 * Check if animation is currently playing.
	 */
	bool isPlaying() const { return _playing && !_paused; }

	/**
	 * Check if one-shot animation has finished.
	 */
	bool isFinished() const { return _finished; }

	/**
	 * Set frame directly (for manual control).
	 */
	void setFrame(int frame);

private:
	const Animation *_animation;
	int _currentFrame;
	uint32 _frameDelayMs;
	uint32 _lastFrameTime;
	bool _playing;
	bool _paused;
	bool _loop;
	bool _finished;

	AnimationCallback _callback;
	void *_userData;
	int _animId;
};

/**
 * CompressGfxZomb — zoombini sprite graphics from .anm files.
 *
 * Original: CompressGfxZomb_Read_45C4D0
 * 3D array: 100 × 5 × 6 = 3000 cells.
 * Per cell: DWORD frameCount, per frame: DWORD dataSize + 24-byte header + RLE data.
 */
class ZoombiniGfx {
public:
	ZoombiniGfx();
	~ZoombiniGfx();

	bool loadFromFile(const Common::Path &path);

	const RleBlock *getFrame(int cellIndex, int frameIndex) const;
	int getFrameCount(int cellIndex) const;

	static const int kDim0 = 100;
	static const int kDim1 = 5;
	static const int kDim2 = 6;
	static const int kCellCount = kDim0 * kDim1 * kDim2; // 3000

private:
	struct Cell {
		Common::Array<RleBlock *> frames;
		~Cell();
	};

	Cell _cells[kCellCount];
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_GFX_H
