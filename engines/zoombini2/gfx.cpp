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
#include "common/memstream.h"
#include "common/textconsole.h"

#include "zoombini2/gfx.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// BitBlock
// ============================================================================

BitBlock::BitBlock() : _width(0), _height(0), _pixels(nullptr), _alphaMap(nullptr) {
}

BitBlock::~BitBlock() {
	delete[] _pixels;
	delete[] _alphaMap;
}

/**
 * Load a color+alpha BMP pair.
 * Original: CBitBlockPair__Load_457550 uses ReadBMP + ReadAlphaBMP.
 */
bool BitBlock::loadFromBMPPair(const Common::Path &colorPath, const Common::Path &alphaPath) {
	Common::File colorFile;
	if (!colorFile.open(colorPath)) {
		warning("BitBlock: cannot open color BMP '%s'", colorPath.toString().c_str());
		return false;
	}
	if (!loadColorBMP(&colorFile))
		return false;

	Common::File alphaFile;
	if (alphaFile.open(alphaPath)) {
		if (!loadAlphaBMP(&alphaFile)) {
			warning("BitBlock: failed to load alpha BMP '%s'", alphaPath.toString().c_str());
		}
	}

	return true;
}

bool BitBlock::loadFromBMP(const Common::Path &colorPath) {
	Common::File colorFile;
	if (!colorFile.open(colorPath)) {
		debug(3, "BitBlock: cannot open BMP '%s'", colorPath.toString().c_str());
		return false;
	}
	return loadColorBMP(&colorFile);
}

/**
 * Load from .bb cached BitBlock format — CBitBlock__Read_457670.
 *
 * Format:
 *   - 20-byte header: field0(4) + pPixels_placeholder(4) + bufSize(4) + width(4) + height(4)
 *   - 4-byte data size (= 3 * width * height)
 *   - Raw BGR pixel data (3 bytes per pixel, top-to-bottom)
 */
bool BitBlock::loadFromBB(const Common::Path &bbPath) {
	Common::File f;
	if (!f.open(bbPath)) {
		debug(3, "BitBlock: cannot open BB '%s'", bbPath.toString().c_str());
		return false;
	}

	// Read 20-byte header
	/* int32 field0 = */ f.readSint32LE();
	/* int32 pPixelsPlaceholder = */ f.readSint32LE();
	/* uint32 bufSize = */ f.readUint32LE();
	_width = f.readSint32LE();
	_height = f.readSint32LE();

	if (_width <= 0 || _height <= 0) {
		warning("BitBlock: invalid BB dimensions %dx%d", _width, _height);
		return false;
	}

	// Read 4-byte data size
	uint32 dataSize = f.readUint32LE();
	uint32 expectedSize = (uint32)(_width * _height) * 3;
	if (dataSize != expectedSize)
		dataSize = expectedSize;

	// Read BGR pixel data and convert to RGBA
	delete[] _pixels;
	_pixels = new byte[_width * _height * 4];

	byte *bgrBuf = new byte[dataSize];
	if (f.read(bgrBuf, dataSize) != dataSize) {
		warning("BitBlock: failed to read BB pixel data");
		delete[] bgrBuf;
		return false;
	}

	// Expand 3bpp BGR to 4bpp RGBA (original: 3-to-4 expansion in 32bpp mode)
	const byte *src = bgrBuf;
	byte *dst = _pixels;
	int pixelCount = _width * _height;
	for (int i = 0; i < pixelCount; i++) {
		dst[0] = src[2]; // R
		dst[1] = src[1]; // G
		dst[2] = src[0]; // B
		dst[3] = 255;    // A
		src += 3;
		dst += 4;
	}
	delete[] bgrBuf;

	return true;
}

/**
 * Unified loader — CBitBlock__Read_457670.
 * Tries .bb extension first, then falls back to .bmp.
 */
bool BitBlock::load(const Common::Path &basePath) {
	// Try .bb first (cached binary format)
	Common::Path bbPath(basePath.toString() + ".bb");
	if (loadFromBB(bbPath))
		return true;

	// Fall back to .bmp (standard Windows bitmap)
	Common::Path bmpPath(basePath.toString() + ".bmp");
	return loadFromBMP(bmpPath);
}

void BitBlock::createEmpty(int width, int height, bool withAlpha) {
	delete[] _pixels;
	delete[] _alphaMap;
	_width = width;
	_height = height;
	_pixels = new byte[width * height * 4]();
	_alphaMap = withAlpha ? new byte[width * height]() : nullptr;
}

/**
 * Read 24-bit color BMP — CBitBlock__ReadBMP_4573D0.
 * Standard 14-byte file header + 40-byte info header.
 * Rows are bottom-up (flipped), padded to 4 bytes.
 */
bool BitBlock::loadColorBMP(Common::SeekableReadStream *stream) {
	// Read 14-byte BITMAPFILEHEADER
	byte bmpFileHeader[14];
	if (stream->read(bmpFileHeader, 14) != 14) {
		warning("BitBlock: failed to read BMP file header");
		return false;
	}

	// Read 40-byte BITMAPINFOHEADER
	byte bmpInfoHeader[40];
	if (stream->read(bmpInfoHeader, 40) != 40) {
		warning("BitBlock: failed to read BMP info header");
		return false;
	}

	_width = READ_LE_INT32(bmpInfoHeader + 4);
	_height = READ_LE_INT32(bmpInfoHeader + 8);
	int bitsPerPixel = READ_LE_INT16(bmpInfoHeader + 14);

	if (_width <= 0 || _height <= 0) {
		warning("BitBlock: invalid BMP dimensions %dx%d", _width, _height);
		return false;
	}

	if (bitsPerPixel != 24) {
		warning("BitBlock: unsupported BMP bpp %d (expected 24)", bitsPerPixel);
		return false;
	}

	delete[] _pixels;
	_pixels = new byte[_width * _height * 4];

	int rowPadding = _width % 4;  // Original uses width % 4 for 24-bit BMP
	byte padBuf[4];

	// Read rows bottom-up, store top-down
	for (int row = _height - 1; row >= 0; row--) {
		byte *dstRow = _pixels + row * _width * 4;
		for (int col = 0; col < _width; col++) {
			byte bgr[3];
			if (stream->read(bgr, 3) != 3) {
				warning("BitBlock: failed to read pixel data");
				return false;
			}
			// BMP is BGR, convert to RGBA
			dstRow[col * 4 + 0] = bgr[2]; // R
			dstRow[col * 4 + 1] = bgr[1]; // G
			dstRow[col * 4 + 2] = bgr[0]; // B
			dstRow[col * 4 + 3] = 255;    // A
		}
		if (rowPadding > 0) {
			stream->read(padBuf, rowPadding);
		}
	}

	return true;
}

/**
 * Read 8-bit alpha mask BMP — CBitBlock__ReadAlphaBMP_4571A0.
 * 14-byte file header + 44-byte info header (V4) + palette + 8-bit rows.
 */
bool BitBlock::loadAlphaBMP(Common::SeekableReadStream *stream) {
	// Read 14-byte BITMAPFILEHEADER
	byte bmpFileHeader[14];
	if (stream->read(bmpFileHeader, 14) != 14)
		return false;

	// Read 44-byte info header (BITMAPV4 or extended)
	byte bmpInfoHeader[44];
	if (stream->read(bmpInfoHeader, 44) != 44)
		return false;

	int alphaWidth = READ_LE_INT32(bmpInfoHeader + 4);
	int alphaHeight = READ_LE_INT32(bmpInfoHeader + 8);
	int alphaBpp = READ_LE_INT16(bmpInfoHeader + 14);
	int clrUsed = READ_LE_INT32(bmpInfoHeader + 32);

	if (alphaWidth != _width || alphaHeight != _height) {
		warning("BitBlock: alpha BMP size %dx%d doesn't match color %dx%d",
		        alphaWidth, alphaHeight, _width, _height);
		return false;
	}

	// Read and discard palette
	int paletteEntries = clrUsed ? (clrUsed - 1) : 255;
	if (paletteEntries > 0) {
		byte *palette = new byte[paletteEntries * 4];
		stream->read(palette, paletteEntries * 4);
		delete[] palette;
	}

	// Calculate padding for 8-bit rows
	int rowPad;
	switch (alphaWidth % 4) {
	case 1: rowPad = 3; break;
	case 2: rowPad = 2; break;
	case 3: rowPad = 1; break;
	default: rowPad = 0; break;
	}

	// Read rows top-down into temp buffer, then flip
	byte *tempBuf = new byte[alphaWidth * alphaHeight];
	byte padBuf[4];
	byte *ptr = tempBuf;
	for (int row = 0; row < alphaHeight; row++) {
		stream->read(ptr, alphaWidth);
		ptr += alphaWidth;
		if (rowPad > 0)
			stream->read(padBuf, rowPad);
	}

	// Flip bottom-up to top-down
	delete[] _alphaMap;
	_alphaMap = new byte[alphaWidth * alphaHeight];
	for (int row = 0; row < alphaHeight; row++) {
		memcpy(_alphaMap + row * alphaWidth,
		       tempBuf + (alphaHeight - 1 - row) * alphaWidth,
		       alphaWidth);
	}
	delete[] tempBuf;

	return true;
}

/**
 * Draw bitmap to surface (opaque copy) — CBitBlock__DrawToScreen_456BE0.
 */
void BitBlock::drawToSurface(Graphics::ManagedSurface *dst, int x, int y) const {
	if (!_pixels)
		return;

	const Graphics::PixelFormat &fmt = dst->format;
	for (int row = 0; row < _height; row++) {
		int dy = y + row;
		if (dy < 0 || dy >= dst->h)
			continue;

		for (int col = 0; col < _width; col++) {
			int dx = x + col;
			if (dx < 0 || dx >= dst->w)
				continue;

			const byte *src = _pixels + (row * _width + col) * 4;
			uint32 color = fmt.ARGBToColor(255, src[0], src[1], src[2]);
			*((uint32 *)dst->getBasePtr(dx, dy)) = color;
		}
	}
}

/**
 * Draw sub-rectangle — CBitBlock__DrawSubRect_456D40.
 */
void BitBlock::drawSubRect(Graphics::ManagedSurface *dst, int x, int y,
                           const Common::Rect &srcRect) const {
	if (!_pixels)
		return;

	const Graphics::PixelFormat &fmt = dst->format;
	for (int row = srcRect.top; row < srcRect.bottom && row < _height; row++) {
		int dy = y + (row - srcRect.top);
		if (dy < 0 || dy >= dst->h)
			continue;

		for (int col = srcRect.left; col < srcRect.right && col < _width; col++) {
			int dx = x + (col - srcRect.left);
			if (dx < 0 || dx >= dst->w)
				continue;

			const byte *src = _pixels + (row * _width + col) * 4;
			uint32 color = fmt.ARGBToColor(255, src[0], src[1], src[2]);
			*((uint32 *)dst->getBasePtr(dx, dy)) = color;
		}
	}
}

/**
 * Draw with per-pixel alpha blending — CBitBlock__DrawAlphaBlend_456EE0.
 */
void BitBlock::drawAlphaBlend(Graphics::ManagedSurface *dst, int x, int y,
                              const byte alphaLUT[256][256]) const {
	if (!_pixels || !_alphaMap)
		return;

	for (int row = 0; row < _height; row++) {
		int dy = y + row;
		if (dy < 0 || dy >= dst->h)
			continue;

		for (int col = 0; col < _width; col++) {
			int dx = x + col;
			if (dx < 0 || dx >= dst->w)
				continue;

			byte alpha = _alphaMap[row * _width + col];
			if (alpha == 0)
				continue;

			const byte *src = _pixels + (row * _width + col) * 4;
			byte *dstPixel = (byte *)dst->getBasePtr(dx, dy);

			if (alpha == 255) {
				dstPixel[0] = src[2]; // B
				dstPixel[1] = src[1]; // G
				dstPixel[2] = src[0]; // R
				dstPixel[3] = 255;    // A
			} else {
				byte invAlpha = 255 - alpha;
				dstPixel[0] = alphaLUT[alpha][src[2]] + alphaLUT[invAlpha][dstPixel[0]];
				dstPixel[1] = alphaLUT[alpha][src[1]] + alphaLUT[invAlpha][dstPixel[1]];
				dstPixel[2] = alphaLUT[alpha][src[0]] + alphaLUT[invAlpha][dstPixel[2]];
				dstPixel[3] = 255;
			}
		}
	}
}

// ============================================================================
// RleBlock
// ============================================================================

RleBlock::RleBlock() : _width(0), _height(0), _dataSize(0), _rleData(nullptr), _field20(0) {
}

RleBlock::~RleBlock() {
	free(_rleData);
}

/**
 * Load from standalone .rl/.rb file — CRleBlock__CRleBlock_45AC90.
 * File: [24-byte header] [4-byte dataSize] [dataSize bytes RLE data].
 */
bool RleBlock::loadFromStream(Common::SeekableReadStream *stream) {
	// Read 24-byte header
	uint32 field0 = stream->readUint32LE();
	stream->readUint32LE();  // pData placeholder (ignored)
	_dataSize = stream->readUint32LE();
	_width = stream->readSint32LE();
	_height = stream->readSint32LE();
	_field20 = stream->readSint32LE();

	// Read separate data size (should match _dataSize)
	uint32 fileDataSize = stream->readUint32LE();

	// Use the larger of the two sizes for safety
	uint32 allocSize = MAX(_dataSize, fileDataSize);

	_rleData = (byte *)malloc(allocSize);
	if (!_rleData) {
		warning("RleBlock: malloc failed for %u bytes", allocSize);
		return false;
	}

	if ((uint32)stream->read(_rleData, allocSize) != allocSize) {
		warning("RleBlock: failed to read RLE data (%u bytes)", allocSize);
		free(_rleData);
		_rleData = nullptr;
		return false;
	}

	// Expand 3bpp to 4bpp (original calls this when g_graphicsModeFlag == 0)
	expand3to4bpp();

	return true;
}

bool RleBlock::loadFromFile(const Common::Path &path) {
	Common::File f;
	if (!f.open(path)) {
		debug(3, "RleBlock: cannot open '%s'", path.toString().c_str());
		return false;
	}
	return loadFromStream(&f);
}

/**
 * Unified loader: appends .rb extension and loads.
 * Original: CRleBlock__CRleBlock_45AC90 expects .rb files.
 */
bool RleBlock::load(const Common::Path &basePath) {
	Common::Path rbPath(basePath.toString() + ".rb");
	return loadFromFile(rbPath);
}

/**
 * Load from a .an/.anm stream where header and data are separate.
 * The caller has already read the data size; we read 24-byte header + data.
 * Format: [24-byte header] [4-byte dataSize] [data]
 */
bool RleBlock::loadHeaderAndData(Common::SeekableReadStream *stream, uint32 dataSize) {
	// Read 24-byte header
	stream->readUint32LE();  // field0 (will be set from data)
	stream->readUint32LE();  // pData placeholder
	_dataSize = stream->readUint32LE();
	_width = stream->readSint32LE();
	_height = stream->readSint32LE();
	_field20 = stream->readSint32LE();

	// Read and skip the 4-byte dataSize field after header (same as loadFromStream)
	uint32 fileDataSize = stream->readUint32LE();

	// Use the provided dataSize for allocation (matches header _dataSize)
	uint32 readSize = MAX(_dataSize, MAX(dataSize, fileDataSize));

	_rleData = (byte *)malloc(readSize);
	if (!_rleData)
		return false;

	if ((uint32)stream->read(_rleData, readSize) != readSize) {
		free(_rleData);
		_rleData = nullptr;
		return false;
	}

	expand3to4bpp();
	return true;
}

/**
 * Convert RLE data from 3-byte to 4-byte per pixel format.
 * Original: CRleBlock__Expand3to4bpp_45AF90.
 *
 * Mode 0 (opaque): 3 bytes/pixel → 4 bytes/pixel (RGB + pad)
 * Mode 1 (alpha):  already 4 bytes/pixel, copied as-is.
 */
void RleBlock::expand3to4bpp() {
	if (!_rleData || _dataSize < 2)
		return;

	// Allocate expanded buffer (worst case 2x)
	byte *newData = (byte *)malloc(_dataSize * 2);
	if (!newData)
		return;

	const byte *src = _rleData;
	const byte *srcEnd = _rleData + _dataSize;
	byte *dst = newData;

	// Copy first 2 bytes (effective height marker)
	memcpy(dst, src, 2);
	src += 2;
	dst += 2;

	while (src < srcEnd) {
		// Copy span header: x_offset(2), y_offset(2), pixel_count(2)
		if (src + 6 > srcEnd)
			break;
		memcpy(dst, src, 6);
		int16 pixelCount = READ_LE_INT16(src + 4);
		src += 6;
		dst += 6;

		// Mode byte
		if (src >= srcEnd)
			break;
		byte mode = *src;
		*dst = mode;
		src++;
		dst++;

		if (mode != 0) {
			// Mode 1 (alpha): already 4 bytes per pixel, copy as-is
			uint32 copySize = pixelCount * 4;
			if (src + copySize > srcEnd)
				break;
			memcpy(dst, src, copySize);
			src += copySize;
			dst += copySize;
		} else {
			// Mode 0 (opaque): expand 3 bytes → 4 bytes per pixel
			for (int i = 0; i < pixelCount; i++) {
				if (src + 3 > srcEnd)
					break;
				dst[0] = src[0]; // R
				dst[1] = src[1]; // G
				dst[2] = src[2]; // B
				dst[3] = 0;      // padding
				src += 3;
				dst += 4;
			}
		}
	}

	uint32 newSize = (uint32)(dst - newData);
	free(_rleData);
	_rleData = newData;
	_dataSize = newSize;
}

/**
 * Draw RLE sprite to screen with alpha blending.
 * Original: CRleBlock__DrawToScreen_45B220.
 *
 * Iterates spans from RLE data (starting at offset +2).
 * Mode 0: opaque copy (4 bytes per pixel after expand).
 * Mode 1: alpha-blended using premultiplied source + LUT.
 */
void RleBlock::drawToScreen(Graphics::ManagedSurface *dst, int x, int y,
                            const byte alphaLUT[256][256]) const {
	if (!_rleData || _dataSize < 2)
		return;

	const byte *ptr = _rleData + 2;  // Skip effectiveHeight
	const byte *end = _rleData + _dataSize;

	while (ptr < end) {
		if (ptr + 7 > end)
			break;

		int16 xOff = READ_LE_INT16(ptr);
		int16 yOff = READ_LE_INT16(ptr + 2);
		int16 pixelCount = READ_LE_INT16(ptr + 4);
		byte mode = ptr[6];
		ptr += 7;

		int screenX = x + xOff;
		int screenY = y + yOff;

		if (screenY < 0 || screenY >= dst->h || pixelCount <= 0) {
			// Skip pixel data
			if (mode != 0) {
				ptr += pixelCount * 4;
			} else {
				ptr += pixelCount * 4;
			}
			continue;
		}

		if (mode == 0) {
			// Opaque mode: direct copy (4bpp after expand)
			int startCol = 0;
			int endCol = pixelCount;

			// Clip left
			if (screenX < 0) {
				startCol = -screenX;
				screenX = 0;
			}
			// Clip right
			if (screenX + (endCol - startCol) > dst->w) {
				endCol = startCol + (dst->w - screenX);
			}

			if (startCol < endCol) {
				const byte *srcPixel = ptr + startCol * 4;
				byte *dstPixel = (byte *)dst->getBasePtr(screenX, screenY);

				for (int i = startCol; i < endCol; i++) {
					dstPixel[0] = srcPixel[0]; // B (ScummVM BGRA)
					dstPixel[1] = srcPixel[1]; // G
					dstPixel[2] = srcPixel[2]; // R
					dstPixel[3] = 255;         // A
					srcPixel += 4;
					dstPixel += 4;
				}
			}
			ptr += pixelCount * 4;
		} else {
			// Alpha mode: premultiplied alpha blend
			int startCol = 0;
			int endCol = pixelCount;

			if (screenX < 0) {
				startCol = -screenX;
				screenX = 0;
			}
			if (screenX + (endCol - startCol) > dst->w) {
				endCol = startCol + (dst->w - screenX);
			}

			if (startCol < endCol) {
				const byte *srcPixel = ptr + startCol * 4;
				byte *dstPixel = (byte *)dst->getBasePtr(screenX, screenY);

				for (int i = startCol; i < endCol; i++) {
					byte invAlpha = srcPixel[3];
					// Premultiplied alpha compositing:
					// dest = src_premult + LUT[invAlpha][dest]
					dstPixel[0] = srcPixel[0] + alphaLUT[invAlpha][dstPixel[0]];
					dstPixel[1] = srcPixel[1] + alphaLUT[invAlpha][dstPixel[1]];
					dstPixel[2] = srcPixel[2] + alphaLUT[invAlpha][dstPixel[2]];
					dstPixel[3] = 255;
					srcPixel += 4;
					dstPixel += 4;
				}
			}
			ptr += pixelCount * 4;
		}
	}
}

void RleBlock::drawToScreenClipped(Graphics::ManagedSurface *dst, int x, int y,
                                   int clipLeft, int clipTop, int clipRight, int clipBottom,
                                   const byte alphaLUT[256][256]) const {
	// Original: CRleBlock__DrawToScreenClipped_45B410
	if (!_rleData || _dataSize < 2)
		return;

	const byte *ptr = _rleData + 2;
	const byte *end = _rleData + _dataSize;

	while (ptr < end) {
		if (ptr + 7 > end)
			break;

		int16 xOff = READ_LE_INT16(ptr);
		int16 yOff = READ_LE_INT16(ptr + 2);
		int16 pixelCount = READ_LE_INT16(ptr + 4);
		byte mode = ptr[6];
		ptr += 7;

		int screenX = x + xOff;
		int screenY = y + yOff;

		// Clip vertically against clip rect and screen bounds
		if (screenY < clipTop || screenY >= clipBottom ||
		    screenY < 0 || screenY >= dst->h || pixelCount <= 0) {
			ptr += pixelCount * 4;
			continue;
		}

		// Clip horizontally against clip rect
		int startCol = 0;
		int endCol = pixelCount;

		if (screenX + endCol <= clipLeft || screenX >= clipRight) {
			ptr += pixelCount * 4;
			continue;
		}

		if (screenX < clipLeft) {
			startCol = clipLeft - screenX;
			screenX = clipLeft;
		}
		if (screenX + (endCol - startCol) > clipRight) {
			endCol = startCol + (clipRight - screenX);
		}

		// Also clip to screen bounds
		if (screenX < 0) {
			startCol += -screenX;
			screenX = 0;
		}
		if (screenX + (endCol - startCol) > dst->w) {
			endCol = startCol + (dst->w - screenX);
		}

		if (startCol < endCol) {
			const byte *srcPixel = ptr + startCol * 4;
			byte *dstPixel = (byte *)dst->getBasePtr(screenX, screenY);

			if (mode == 0) {
				for (int i = startCol; i < endCol; i++) {
					dstPixel[0] = srcPixel[0];
					dstPixel[1] = srcPixel[1];
					dstPixel[2] = srcPixel[2];
					dstPixel[3] = 255;
					srcPixel += 4;
					dstPixel += 4;
				}
			} else {
				for (int i = startCol; i < endCol; i++) {
					byte invAlpha = srcPixel[3];
					dstPixel[0] = srcPixel[0] + alphaLUT[invAlpha][dstPixel[0]];
					dstPixel[1] = srcPixel[1] + alphaLUT[invAlpha][dstPixel[1]];
					dstPixel[2] = srcPixel[2] + alphaLUT[invAlpha][dstPixel[2]];
					dstPixel[3] = 255;
					srcPixel += 4;
					dstPixel += 4;
				}
			}
		}
		ptr += pixelCount * 4;
	}
}

// ============================================================================
// Animation
// ============================================================================

Animation::Animation() {
}

Animation::~Animation() {
	for (uint i = 0; i < _frames.size(); i++)
		delete _frames[i];
}

/**
 * Load animation from .an cache file — CAnimation__Read_455F30.
 * Format: DWORD frameCount, per frame: 24-byte header + 4-byte dataSize + data.
 */
bool Animation::loadFromFile(const Common::Path &path) {
	Common::File f;
	if (!f.open(path)) {
		debug(3, "Animation: cannot open '%s'", path.toString().c_str());
		return false;
	}

	uint32 frameCount = f.readUint32LE();
	if (frameCount > 10000) {
		warning("Animation: suspicious frame count %u in '%s'", frameCount,
		        path.toString().c_str());
		return false;
	}

	_frames.reserve(frameCount);

	for (uint32 i = 0; i < frameCount; i++) {
		// Each frame in .an: [24-byte header] [4-byte dataSize] [data]
		// Read 24-byte header into temp
		uint32 headerPos = f.pos();
		byte headerBuf[24];
		if (f.read(headerBuf, 24) != 24) {
			warning("Animation: failed to read frame %u header", i);
			return false;
		}

		uint32 dataSize = f.readUint32LE();

		// Seek back to header start and let RleBlock read header + data
		f.seek(headerPos);

		RleBlock *frame = new RleBlock();
		if (!frame->loadHeaderAndData(&f, dataSize)) {
			warning("Animation: failed to load frame %u", i);
			delete frame;
			return false;
		}

		_frames.push_back(frame);
	}

	debug(3, "Animation: loaded %u frames from '%s'", frameCount,
	      path.toString().c_str());
	return true;
}

const RleBlock *Animation::getFrame(int index) const {
	if (index >= 0 && index < (int)_frames.size())
		return _frames[index];
	return nullptr;
}

// ============================================================================
// AnimationPlayer
// ============================================================================

AnimationPlayer::AnimationPlayer()
	: _animation(nullptr),
	  _currentFrame(0),
	  _frameDelayMs(100),
	  _lastFrameTime(0),
	  _playing(false),
	  _paused(false),
	  _loop(false),
	  _finished(false),
	  _callback(nullptr),
	  _userData(nullptr),
	  _animId(0) {
}

AnimationPlayer::~AnimationPlayer() {
}

void AnimationPlayer::setAnimation(const Animation *anim) {
	_animation = anim;
	_currentFrame = 0;
	_playing = false;
	_finished = false;
}

void AnimationPlayer::play(uint32 frameDelayMs, bool loop,
                           AnimationCallback callback,
                           void *userData, int animId) {
	_frameDelayMs = frameDelayMs;
	_loop = loop;
	_callback = callback;
	_userData = userData;
	_animId = animId;
	_currentFrame = 0;
	_lastFrameTime = 0;  // Will be set on first update
	_playing = true;
	_paused = false;
	_finished = false;
}

void AnimationPlayer::playAt(uint32 tickCount, uint32 frameDelayMs, bool loop,
                             AnimationCallback callback,
                             void *userData, int animId) {
	_frameDelayMs = frameDelayMs;
	_loop = loop;
	_callback = callback;
	_userData = userData;
	_animId = animId;
	_currentFrame = 0;
	_lastFrameTime = tickCount;
	_playing = true;
	_paused = false;
	_finished = false;
}

void AnimationPlayer::stop() {
	_playing = false;
	_paused = false;
	_currentFrame = 0;
}

void AnimationPlayer::pause() {
	_paused = true;
}

void AnimationPlayer::resume() {
	_paused = false;
}

bool AnimationPlayer::update(uint32 tickCount) {
	if (!_animation || !_playing || _paused || _finished)
		return false;

	// Initialize lastFrameTime on first update
	if (_lastFrameTime == 0)
		_lastFrameTime = tickCount;

	int frameCount = _animation->getFrameCount();
	if (frameCount <= 0) {
		_finished = true;
		return true;
	}

	// Calculate elapsed time and advance frames
	uint32 elapsed = tickCount - _lastFrameTime;
	int framesToAdvance = elapsed / _frameDelayMs;

	if (framesToAdvance > 0) {
		_lastFrameTime += framesToAdvance * _frameDelayMs;
		_currentFrame += framesToAdvance;

		if (_loop) {
			_currentFrame %= frameCount;
		} else if (_currentFrame >= frameCount) {
			_currentFrame = frameCount - 1;
			_finished = true;
			_playing = false;

			// Fire completion callback
			if (_callback) {
				_callback(_userData, _animId);
			}
			return true;
		}
	}

	return false;
}

void AnimationPlayer::draw(Graphics::ManagedSurface *dst, int x, int y,
                           const byte alphaLUT[256][256]) const {
	if (!_animation)
		return;

	const RleBlock *frame = _animation->getFrame(_currentFrame);
	if (frame) {
		frame->drawToScreen(dst, x, y, alphaLUT);
	}
}

void AnimationPlayer::setFrame(int frame) {
	if (_animation) {
		int frameCount = _animation->getFrameCount();
		if (frameCount > 0) {
			_currentFrame = CLIP(frame, 0, frameCount - 1);
		}
	}
}

// ============================================================================
// ZoombiniGfx
// ============================================================================

ZoombiniGfx::Cell::~Cell() {
	for (uint i = 0; i < frames.size(); i++)
		delete frames[i];
}

ZoombiniGfx::ZoombiniGfx() {
}

ZoombiniGfx::~ZoombiniGfx() {
}

/**
 * Load from .anm file — CompressGfxZomb_Read_45C4D0.
 * 3000 cells (100 × 5 × 6). Per cell: DWORD frameCount,
 * per frame: DWORD dataSize + 24-byte header + data.
 */
bool RleBlock::loadAnmFrame(Common::SeekableReadStream *stream) {
	// .anm frame format (CompressGfxZomb_Read_45C4D0):
	//   24-byte CRleBlock header  (fields: unused, pData, dataSize, width, height, field20)
	//   N bytes RLE data          (N = _dataSize from header, NOT a separate size field)
	// This differs from .an format which has an inner DWORD size between header and data.
	stream->readUint32LE(); // field0 (unused placeholder)
	stream->readUint32LE(); // pData  (unused placeholder)
	_dataSize  = stream->readUint32LE();
	_width     = stream->readSint32LE();
	_height    = stream->readSint32LE();
	_field20   = stream->readSint32LE();

	if (stream->err() || stream->eos()) {
		warning("RleBlock::loadAnmFrame: EOF/error reading header");
		return false;
	}

	_rleData = (byte *)malloc(_dataSize);
	if (!_rleData)
		return false;

	if ((uint32)stream->read(_rleData, _dataSize) != _dataSize) {
		free(_rleData);
		_rleData = nullptr;
		return false;
	}

	expand3to4bpp();
	return true;
}

bool ZoombiniGfx::loadFromFile(const Common::Path &path) {
	Common::File f;
	if (!f.open(path)) {
		warning("ZoombiniGfx: cannot open '%s'", path.toString().c_str());
		return false;
	}

	int cellIndex = 0;
	for (int d0 = 0; d0 < kDim0; d0++) {
		for (int d1 = 0; d1 < kDim1; d1++) {
			for (int d2 = 0; d2 < kDim2; d2++) {
				uint32 frameCount = f.readUint32LE();
				if (f.eos() || f.err()) {
					warning("ZoombiniGfx: unexpected EOF at cell %d", cellIndex);
					return false;
				}

				Cell &cell = _cells[cellIndex];
				cell.frames.reserve(frameCount);

				for (uint32 fr = 0; fr < frameCount; fr++) {
					// .anm per-frame: DWORD outer_dataSize (for malloc hint) +
					// 24-byte header + _dataSize bytes RLE data.
					// The outer size is read here; loadAnmFrame reads the header
					// and then exactly _dataSize (from header+8) bytes of data.
					f.readUint32LE(); // outer dataSize — consumed but not used

					RleBlock *frame = new RleBlock();
					if (!frame->loadAnmFrame(&f)) {
						warning("ZoombiniGfx: failed cell %d frame %u", cellIndex, fr);
						delete frame;
						return false;
					}

					cell.frames.push_back(frame);
				}

				cellIndex++;
			}
		}
	}

	debug(2, "ZoombiniGfx: loaded %d cells from '%s'",
	      kCellCount, path.toString().c_str());
	return true;
}

const RleBlock *ZoombiniGfx::getFrame(int cellIndex, int frameIndex) const {
	if (cellIndex < 0 || cellIndex >= kCellCount)
		return nullptr;
	const Cell &cell = _cells[cellIndex];
	if (frameIndex < 0 || frameIndex >= (int)cell.frames.size())
		return nullptr;
	return cell.frames[frameIndex];
}

int ZoombiniGfx::getFrameCount(int cellIndex) const {
	if (cellIndex < 0 || cellIndex >= kCellCount)
		return 0;
	return _cells[cellIndex].frames.size();
}

} // End of namespace Zoombini2
