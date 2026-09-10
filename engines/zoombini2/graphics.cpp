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
#include "common/ptr.h"
#include "common/textconsole.h"

#include "zoombini2/graphics.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

void ManagedSurface32::fillRect(const Common::Rect32 &rect, uint32 color) {
	if (!rect.isValidRect())
		return;

	Common::Rect32 clip32 = rect;
	clip32.clip(w, h);
	if (clip32.isEmpty())
		return;
	Common::Rect clip16 = Common::Rect(clip32.left, clip32.top, clip32.right, clip32.bottom);
	Graphics::ManagedSurface::fillRect(clip16, color);
}

void ManagedSurface32::blitFrom(const Graphics::ManagedSurface &src, const Common::Point32 &destPos) {
	const int32 srcRight = destPos.x + src.w;
	const int32 srcBottom = destPos.y + src.h;
	if (srcRight <= 0 || srcBottom <= 0 || w <= destPos.x || h <= destPos.y)
		return;

	const int16 destLeft = destPos.x < 0 ? 0 : static_cast<int16>(destPos.x);
	const int16 destTop = destPos.y < 0 ? 0 : static_cast<int16>(destPos.y);
	const int16 destRight = srcRight < w ? static_cast<int16>(srcRight) : w;
	const int16 destBottom = srcBottom < h ? static_cast<int16>(srcBottom) : h;
	const int16 srcLeft = static_cast<int16>(destLeft - destPos.x);
	const int16 srcTop = static_cast<int16>(destTop - destPos.y);

	Graphics::ManagedSurface::blitFrom(src,
									   Common::Rect(srcLeft, srcTop, srcLeft + destRight - destLeft, srcTop + destBottom - destTop),
									   Common::Rect(destLeft, destTop, destRight, destBottom));
}

AlphaBlendLUT::AlphaBlendLUT() {
	// Fill the complete table once. Blended pixels reuse these products for
	// every color channel, avoiding multiplication and division in the inner
	// drawing loops.
	//
	// The shift is deliberate: it produces floor(factor * value / 256), which
	// is the renderer's exact byte-based rule rather than a /255 blend.
	for (int factor = 0; factor < kValueCount; factor++) {
		for (int value = 0; value < kValueCount; value++)
			_values[factor][value] = static_cast<byte>((factor * value) >> 8);
	}
}

BitBlock::BitBlock() : _width(0), _height(0), _pixels(nullptr), _alphaMap(nullptr) {
}

BitBlock::~BitBlock() {
	delete[] _pixels;
	delete[] _alphaMap;
}

/** Load a color BMP and separate alpha BMP into one drawable block. */
bool BitBlock::loadFromColorAlphaBMP(const Common::Path &colorPath, const Common::Path &alphaPath) {
	Zoombini2Engine *vm = static_cast<Zoombini2Engine *>(g_engine);
	Common::ScopedPtr<Common::SeekableReadStream> colorStream(vm ? vm->openResourceFile(colorPath.toString('/')) : nullptr);
	if (!colorStream) {
		warning("BitBlock: cannot open color BMP '%s'", colorPath.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &colorFile = *colorStream;

	BitBlock loaded;
	if (!loaded.loadColorBMP(&colorFile))
		return false;

	Common::ScopedPtr<Common::SeekableReadStream> alphaStream(vm ? vm->openResourceFile(alphaPath.toString('/')) : nullptr);
	if (!alphaStream) {
		warning("BitBlock: cannot open alpha BMP '%s'", alphaPath.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &alphaFile = *alphaStream;
	if (!loaded.loadAlphaBMP(&alphaFile)) {
		warning("BitBlock: failed to load alpha BMP '%s'", alphaPath.toString().c_str());
		return false;
	}

	swapData(loaded);

	return true;
}

bool BitBlock::loadFromColorBMP(const Common::Path &colorPath) {
	Zoombini2Engine *vm = static_cast<Zoombini2Engine *>(g_engine);
	Common::ScopedPtr<Common::SeekableReadStream> colorStream(vm ? vm->openResourceFile(colorPath.toString('/')) : nullptr);
	if (!colorStream) {
		debug(3, "BitBlock: cannot open BMP '%s'", colorPath.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &colorFile = *colorStream;
	BitBlock loaded;
	if (!loaded.loadColorBMP(&colorFile))
		return false;

	swapData(loaded);
	return true;
}

/**
 * Load the cached `.bb` BitBlock format.
 *
 * Format:
 *   - 20-byte header: field0(4) + pPixels_placeholder(4) + bufSize(4) + width(4) + height(4)
 *   - 4-byte data size (= 3 * width * height)
 *   - Raw BGR pixel data (3 bytes per pixel, top-to-bottom)
 */
bool BitBlock::loadFromBB(const Common::Path &bbPath) {
	Zoombini2Engine *vm = static_cast<Zoombini2Engine *>(g_engine);
	Common::ScopedPtr<Common::SeekableReadStream> stream(vm ? vm->openResourceFile(bbPath.toString('/')) : nullptr);
	if (!stream) {
		debug(3, "BitBlock: cannot open BB '%s'", bbPath.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &f = *stream;

	/* int32 alphaMapPlaceholder = */ f.readSint32LE();
	/* int32 pixelsPlaceholder = */ f.readSint32LE();
	const uint32 bufferSize = f.readUint32LE();
	const int32 width = f.readSint32LE();
	const int32 height = f.readSint32LE();
	const uint32 dataSize = f.readUint32LE();

	if (f.err() || f.eos() || width <= 0 || height <= 0) {
		warning("BitBlock: invalid BB header in '%s'", bbPath.toString().c_str());
		return false;
	}

	const uint64 pixelCount64 = static_cast<uint64>(width) * static_cast<uint64>(height);
	const uint64 expectedSize64 = pixelCount64 * 3;
	if (0x7FFFFFFFU < pixelCount64 || 0xFFFFFFFFU < expectedSize64) {
		warning("BitBlock: BB dimensions overflow in '%s'", bbPath.toString().c_str());
		return false;
	}
	const uint32 expectedSize = static_cast<uint32>(expectedSize64);
	const int64 availableSize64 = f.size() - f.pos();
	if (availableSize64 < 0) {
		warning("BitBlock: invalid BB stream pos in '%s'", bbPath.toString().c_str());
		return false;
	}
	const uint32 readableSize = static_cast<uint32>(MIN<uint64>(expectedSize, static_cast<uint64>(availableSize64)));
	const uint32 recoveredPixelCount = readableSize / 3;
	const uint32 recoveredSize = recoveredPixelCount * 3;

	byte *bgrBuffer = recoveredSize ? new byte[recoveredSize] : nullptr;
	if (recoveredSize && f.read(bgrBuffer, recoveredSize) != recoveredSize) {
		warning("BitBlock: failed to read recoverable BB pixel data in '%s'", bbPath.toString().c_str());
		delete[] bgrBuffer;
		return false;
	}
	if (bufferSize != expectedSize || dataSize != expectedSize)
		warning("BitBlock: inconsistent BB size fields in '%s'; using declared dimensions", bbPath.toString().c_str());
	if (recoveredSize != expectedSize)
		warning("BitBlock: salvaged %u of %u complete pixels from truncated BB '%s'", recoveredPixelCount, static_cast<uint32>(pixelCount64),
				bbPath.toString().c_str());
	if (expectedSize64 < static_cast<uint64>(availableSize64))
		warning("BitBlock: ignoring %u trailing bytes in BB '%s'", static_cast<uint32>(availableSize64 - expectedSize64), bbPath.toString().c_str());

	const int pixelCount = static_cast<int>(pixelCount64);
	byte *pixels = new byte[pixelCount * 4];
	for (int i = 0; i < pixelCount; i++) {
		pixels[i * 4] = 0;
		pixels[i * 4 + 1] = 0;
		pixels[i * 4 + 2] = 0;
		pixels[i * 4 + 3] = 255;
	}
	const byte *src = bgrBuffer;
	byte *dst = pixels;
	for (uint32 i = 0; i < recoveredPixelCount; i++) {
		dst[0] = src[2];
		dst[1] = src[1];
		dst[2] = src[0];
		dst[3] = 255;
		src += 3;
		dst += 4;
	}
	delete[] bgrBuffer;

	delete[] _pixels;
	delete[] _alphaMap;
	_width = width;
	_height = height;
	_pixels = pixels;
	_alphaMap = nullptr;

	return true;
}

/**
 * Load a cached bit block with a bitmap fallback.
 * Tries .bb extension first, then falls back to .bmp.
 */
bool BitBlock::load(const Common::Path &basePath) {
	const Common::String pathString = basePath.toString();
	if (pathString.hasSuffixIgnoreCase(".bb"))
		return loadFromBB(basePath);

	Common::Path sourcePath(basePath);
	Common::Path cachePath(basePath);
	if (pathString.hasSuffixIgnoreCase(".bmp") || pathString.hasSuffixIgnoreCase(".bmt")) {
		cachePath.removeExtension();
		cachePath = cachePath.append(".bb");
	} else {
		cachePath = cachePath.append(".bb");
		sourcePath = sourcePath.append(".bmp");
	}

	if (loadFromBB(cachePath))
		return true;
	return loadFromColorBMP(sourcePath);
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
 * Read a 24-bit color BMP.
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

	const int32 width = READ_LE_INT32(bmpInfoHeader + 4);
	const int32 height = READ_LE_INT32(bmpInfoHeader + 8);
	const int bitsPerPixel = READ_LE_INT16(bmpInfoHeader + 14);

	if (width <= 0 || height <= 0) {
		warning("BitBlock: invalid BMP dimensions %dx%d", width, height);
		return false;
	}

	if (bitsPerPixel != 24) {
		warning("BitBlock: unsupported BMP bpp %d (expected 24)", bitsPerPixel);
		return false;
	}

	const uint64 pixelCount64 = static_cast<uint64>(width) * static_cast<uint64>(height);
	if (0x3FFFFFFFU < pixelCount64) {
		warning("BitBlock: BMP dimensions are too large");
		return false;
	}
	byte *pixels = new byte[static_cast<uint32>(pixelCount64) * 4];

	const int rowPadding = width % 4;
	byte padBuf[4];

	for (int row = height - 1; 0 <= row; row--) {
		byte *dstRow = pixels + row * width * 4;
		for (int col = 0; col < width; col++) {
			byte bgr[3];
			if (stream->read(bgr, 3) != 3) {
				warning("BitBlock: failed to read pixel data");
				delete[] pixels;
				return false;
			}
			dstRow[col * 4 + 0] = bgr[2];
			dstRow[col * 4 + 1] = bgr[1];
			dstRow[col * 4 + 2] = bgr[0];
			dstRow[col * 4 + 3] = 255;
		}
		if (0 < rowPadding && stream->read(padBuf, rowPadding) != static_cast<uint32>(rowPadding)) {
			delete[] pixels;
			return false;
		}
	}

	delete[] _pixels;
	delete[] _alphaMap;
	_width = width;
	_height = height;
	_pixels = pixels;
	_alphaMap = nullptr;

	return true;
}

/**
 * Read an 8-bit alpha-mask BMP.
 * The fixed read covers the 14-byte file header, 40-byte info header, and the
 * first four-byte palette entry. The remaining palette precedes the rows.
 */
bool BitBlock::loadAlphaBMP(Common::SeekableReadStream *stream) {
	// Read 14-byte BITMAPFILEHEADER
	byte bmpFileHeader[14];
	if (stream->read(bmpFileHeader, 14) != 14)
		return false;

	// Read the 40-byte info header and first palette entry.
	byte bmpInfoHeader[44];
	if (stream->read(bmpInfoHeader, 44) != 44)
		return false;

	int alphaWidth = READ_LE_INT32(bmpInfoHeader + 4);
	int alphaHeight = READ_LE_INT32(bmpInfoHeader + 8);
	const int alphaBpp = READ_LE_INT16(bmpInfoHeader + 14);
	const int clrUsed = READ_LE_INT32(bmpInfoHeader + 32);

	if (alphaBpp != 8 || alphaWidth != _width || alphaHeight != _height) {
		warning("BitBlock: alpha BMP size %dx%d doesn't match color %dx%d",
				alphaWidth, alphaHeight, _width, _height);
		return false;
	}

	// Read and discard palette
	int paletteEntries = clrUsed ? (clrUsed - 1) : 255;
	if (paletteEntries > 0) {
		byte *palette = new byte[paletteEntries * 4];
		if (stream->read(palette, paletteEntries * 4) != static_cast<uint32>(paletteEntries * 4)) {
			delete[] palette;
			return false;
		}
		delete[] palette;
	}

	// Calculate padding for 8-bit rows
	int rowPad;
	switch (alphaWidth % 4) {
	case 1:
		rowPad = 3;
		break;
	case 2:
		rowPad = 2;
		break;
	case 3:
		rowPad = 1;
		break;
	default:
		rowPad = 0;
		break;
	}

	// Read rows top-down into temp buffer, then flip
	byte *tempBuf = new byte[alphaWidth * alphaHeight];
	byte padBuf[4];
	byte *ptr = tempBuf;
	for (int row = 0; row < alphaHeight; row++) {
		if (stream->read(ptr, alphaWidth) != static_cast<uint32>(alphaWidth)) {
			delete[] tempBuf;
			return false;
		}
		ptr += alphaWidth;
		if (0 < rowPad && stream->read(padBuf, rowPad) != static_cast<uint32>(rowPad)) {
			delete[] tempBuf;
			return false;
		}
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

void BitBlock::swapData(BitBlock &other) {
	SWAP(_width, other._width);
	SWAP(_height, other._height);
	SWAP(_pixels, other._pixels);
	SWAP(_alphaMap, other._alphaMap);
}

byte BitBlock::blendChannel(byte src, byte destPtr, byte mask) {
	// This path uses the separate-mask bitmap rule, not the RLE LUT rule.
	// The source bitmap already supplies the source contribution, while the
	// destination is retained according to the mask's inverse.
	const int result = src + (destPtr * (255 - mask)) / 255;
	return static_cast<byte>(MIN(result, 255));
}

/**
 * Draw the bitmap to a surface with opaque copying.
 */
void BitBlock::drawToSurface(Graphics::ManagedSurface *dst, const Common::Point32 &pos) const {
	if (!_pixels)
		return;

	const Graphics::PixelFormat &fmt = dst->format;
	for (int row = 0; row < _height; row++) {
		int dy = pos.y + row;
		if (dy < 0 || dst->h <= dy)
			continue;

		for (int col = 0; col < _width; col++) {
			int dx = pos.x + col;
			if (dx < 0 || dst->w <= dx)
				continue;

			const byte *src = _pixels + (row * _width + col) * 4;
			uint32 color = fmt.ARGBToColor(255, src[0], src[1], src[2]);
			*static_cast<uint32 *>(dst->getBasePtr(dx, dy)) = color;
		}
	}
}

/**
 * Draw a source subrectangle to a surface.
 */
void BitBlock::drawSubRect(Graphics::ManagedSurface *dst, const Common::Point32 &pos,
						   const Common::Rect &srcRect) const {
	if (!_pixels)
		return;

	const Graphics::PixelFormat &fmt = dst->format;
	for (int row = srcRect.top; row < srcRect.bottom && row < _height; row++) {
		int dy = pos.y + (row - srcRect.top);
		if (dy < 0 || dst->h <= dy)
			continue;

		for (int col = srcRect.left; col < srcRect.right && col < _width; col++) {
			int dx = pos.x + (col - srcRect.left);
			if (dx < 0 || dst->w <= dx)
				continue;

			const byte *src = _pixels + (row * _width + col) * 4;
			uint32 color = fmt.ARGBToColor(255, src[0], src[1], src[2]);
			*static_cast<uint32 *>(dst->getBasePtr(dx, dy)) = color;
		}
	}
}

/**
 * Draw with per-pixel alpha blending.
 */
void BitBlock::drawAlphaBlend(Graphics::ManagedSurface *dst, const Common::Point32 &pos) const {
	if (!_pixels || !_alphaMap)
		return;

	for (int row = 0; row < _height; row++) {
		int dy = pos.y + row;
		if (dy < 0 || dst->h <= dy)
			continue;

		for (int col = 0; col < _width; col++) {
			int dx = pos.x + col;
			if (dx < 0 || dst->w <= dx)
				continue;

			byte alpha = _alphaMap[row * _width + col];
			if (alpha == 0)
				continue;

			const byte *src = _pixels + (row * _width + col) * 4;
			byte *dstPixel = static_cast<byte *>(dst->getBasePtr(dx, dy));

			dstPixel[0] = blendChannel(src[2], dstPixel[0], alpha);
			dstPixel[1] = blendChannel(src[1], dstPixel[1], alpha);
			dstPixel[2] = blendChannel(src[0], dstPixel[2], alpha);
			dstPixel[3] = 255;
		}
	}
}

/**
 * Draw a separate bitmap and alpha mask with the RLE encoder's blend rule.
 *
 * The bitmap is not premultiplied in memory, so this method creates the two
 * terms explicitly: `scale(mask, source)` and
 * `scale(255 - mask, destination)`.
 */
void BitBlock::drawRleMaskBlend(Graphics::ManagedSurface *dst, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const {
	if (!_pixels || !_alphaMap)
		return;

	for (int row = 0; row < _height; row++) {
		const int destY = pos.y + row;
		if (destY < 0 || dst->h <= destY)
			continue;
		for (int column = 0; column < _width; column++) {
			const int destX = pos.x + column;
			if (destX < 0 || dst->w <= destX)
				continue;

			const byte mask = _alphaMap[row * _width + column];
			if (mask == 0)
				continue;
			const byte *src = _pixels + (row * _width + column) * 4;
			byte *destPtr = static_cast<byte *>(dst->getBasePtr(destX, destY));
			if (mask == 255) {
				destPtr[0] = src[2];
				destPtr[1] = src[1];
				destPtr[2] = src[0];
			} else {
				// `_pixels` is an unpremultiplied bitmap. Scale its source
				// channels by the mask, and scale the destination by its inverse.
				const byte invAlpha = 255 - mask;
				const int srcBlue = alphaLUT.scale(mask, src[2]);
				const int srcGreen = alphaLUT.scale(mask, src[1]);
				const int srcRed = alphaLUT.scale(mask, src[0]);
				const int destBlue = alphaLUT.scale(invAlpha, destPtr[0]);
				const int destGreen = alphaLUT.scale(invAlpha, destPtr[1]);
				const int destRed = alphaLUT.scale(invAlpha, destPtr[2]);
				destPtr[0] = static_cast<byte>(MIN(srcBlue + destBlue, 255));
				destPtr[1] = static_cast<byte>(MIN(srcGreen + destGreen, 255));
				destPtr[2] = static_cast<byte>(MIN(srcRed + destRed, 255));
			}
			destPtr[3] = 255;
		}
	}
}

RleBlock::RleBlock() : _width(0), _height(0), _dataSize(0), _rleData(nullptr), _field20(0) {
}

RleBlock::~RleBlock() {
	free(_rleData);
}

void RleBlock::swapData(RleBlock &other) {
	SWAP(_width, other._width);
	SWAP(_height, other._height);
	SWAP(_dataSize, other._dataSize);
	SWAP(_rleData, other._rleData);
	SWAP(_field20, other._field20);
}

/**
 * Load one serialized RLE record from a standalone `.rb` resource.
 * File: [24-byte header] [4-byte dataSize] [dataSize bytes RLE data].
 */
bool RleBlock::loadFromStream(Common::SeekableReadStream *stream) {
	/* uint32 effectiveHeight = */ stream->readUint32LE();
	/* uint32 dataPlaceholder = */ stream->readUint32LE();
	const uint32 headerSize = stream->readUint32LE();
	const int32 width = stream->readSint32LE();
	const int32 height = stream->readSint32LE();
	const int32 field20 = stream->readSint32LE();
	const uint32 externalSize = stream->readUint32LE();

	if (stream->err() || headerSize != externalSize || width <= 0 || height <= 0 || headerSize < 2) {
		warning("RleBlock: invalid RB header");
		return false;
	}
	const int64 availableSize64 = stream->size() - stream->pos();
	if (availableSize64 < 2) {
		warning("RleBlock: missing RLE data");
		return false;
	}
	const uint32 readableSize = static_cast<uint32>(MIN<uint64>(headerSize, static_cast<uint64>(availableSize64)));

	byte *encodedData = static_cast<byte *>(malloc(readableSize));
	if (!encodedData) {
		warning("RleBlock: malloc failed for %u bytes", readableSize);
		return false;
	}
	if (stream->read(encodedData, readableSize) != readableSize) {
		warning("RleBlock: failed to read RLE data (%u bytes)", readableSize);
		free(encodedData);
		return false;
	}
	if (readableSize != headerSize)
		warning("RleBlock: salvaging %u of %u encoded bytes", readableSize, headerSize);

	byte *expandedData = nullptr;
	uint32 expandedSize = 0;
	bool salvagedSpans = false;
	if (!expand3to4bpp(encodedData, readableSize, expandedData, expandedSize, salvagedSpans)) {
		warning("RleBlock: malformed RLE span data");
		free(encodedData);
		return false;
	}
	free(encodedData);
	if (salvagedSpans)
		warning("RleBlock: discarded a malformed RLE tail after complete spans");

	free(_rleData);
	_width = width;
	_height = height;
	_dataSize = expandedSize;
	_rleData = expandedData;
	_field20 = field20;

	return true;
}

bool RleBlock::loadFromFile(const Common::Path &path) {
	Common::Path resolvedPath(path);
	const Common::String pathString = path.toString();
	if (!pathString.hasSuffixIgnoreCase(".rb")) {
		if (pathString.hasSuffixIgnoreCase(".bmp") || pathString.hasSuffixIgnoreCase(".bmt"))
			resolvedPath.removeExtension();
		resolvedPath = resolvedPath.append(".rb");
	}

	Zoombini2Engine *vm = static_cast<Zoombini2Engine *>(g_engine);
	Common::ScopedPtr<Common::SeekableReadStream> stream(vm ? vm->openResourceFile(resolvedPath.toString('/')) : nullptr);
	if (!stream) {
		debug(3, "RleBlock: cannot open '%s'", resolvedPath.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &f = *stream;
	RleBlock loaded;
	if (!loaded.loadFromStream(&f))
		return false;
	if (f.pos() != f.size())
		warning("RleBlock: ignoring %u trailing bytes in '%s'", static_cast<uint32>(f.size() - f.pos()), resolvedPath.toString().c_str());
	swapData(loaded);
	return true;
}

bool RleBlock::load(const Common::Path &basePath) {
	return loadFromFile(basePath);
}

bool RleBlock::loadAnmFrame(Common::SeekableReadStream *stream, uint32 outerSize) {
	/* uint32 effectiveHeight = */ stream->readUint32LE();
	/* uint32 dataPlaceholder = */ stream->readUint32LE();
	const uint32 headerSize = stream->readUint32LE();
	const int32 width = stream->readSint32LE();
	const int32 height = stream->readSint32LE();
	const int32 field20 = stream->readSint32LE();

	if (stream->err() || headerSize != outerSize || width <= 0 || height <= 0 || headerSize < 2) {
		warning("RleBlock: invalid ANM frame header");
		return false;
	}
	const int64 availableSize64 = stream->size() - stream->pos();
	if (availableSize64 < 2) {
		warning("RleBlock: missing ANM frame data");
		return false;
	}
	const uint32 readableSize = static_cast<uint32>(MIN<uint64>(headerSize, static_cast<uint64>(availableSize64)));

	byte *encodedData = static_cast<byte *>(malloc(readableSize));
	if (!encodedData)
		return false;
	if (stream->read(encodedData, readableSize) != readableSize) {
		free(encodedData);
		return false;
	}
	if (readableSize != headerSize)
		warning("RleBlock: salvaging %u of %u ANM encoded bytes", readableSize, headerSize);

	byte *expandedData = nullptr;
	uint32 expandedSize = 0;
	bool salvagedSpans = false;
	if (!expand3to4bpp(encodedData, readableSize, expandedData, expandedSize, salvagedSpans)) {
		free(encodedData);
		return false;
	}
	free(encodedData);
	if (salvagedSpans)
		warning("RleBlock: discarded a malformed ANM RLE tail after complete spans");

	free(_rleData);
	_width = width;
	_height = height;
	_dataSize = expandedSize;
	_rleData = expandedData;
	_field20 = field20;
	return true;
}

bool RleBlock::expand3to4bpp(const byte *srcData, uint32 srcSize, byte *&expandedData, uint32 &expandedSize, bool &salvaged) {
	expandedData = nullptr;
	expandedSize = 0;
	salvaged = false;
	if (!srcData || srcSize < 2)
		return false;

	const byte *src = srcData + 2;
	const byte *srcEnd = srcData + srcSize;
	uint64 requiredSize = 2;
	while (src < srcEnd) {
		if (static_cast<size_t>(srcEnd - src) < 7) {
			salvaged = true;
			break;
		}
		const int16 pixelCount = READ_LE_INT16(src + 4);
		const byte mode = src[6];
		if (pixelCount < 0 || 1 < mode) {
			salvaged = true;
			break;
		}
		src += 7;
		const uint32 encodedPixelSize = mode == 1 ? 4 : 3;
		const uint32 availablePixelCount = MIN<uint32>(pixelCount, static_cast<size_t>(srcEnd - src) / encodedPixelSize);
		src += availablePixelCount * encodedPixelSize;
		requiredSize += 7 + availablePixelCount * 4;
		if (0xFFFFFFFFU < requiredSize)
			return false;
		if (availablePixelCount != static_cast<uint32>(pixelCount)) {
			salvaged = true;
			break;
		}
	}

	expandedSize = static_cast<uint32>(requiredSize);
	expandedData = static_cast<byte *>(malloc(expandedSize));
	if (!expandedData)
		return false;

	src = srcData;
	byte *dst = expandedData;
	memcpy(dst, src, 2);
	src += 2;
	dst += 2;
	while (src < srcEnd) {
		if (static_cast<uint32>(srcEnd - src) < 7)
			break;
		const int16 pixelCount = READ_LE_INT16(src + 4);
		const byte mode = src[6];
		if (pixelCount < 0 || 1 < mode)
			break;
		const uint32 encodedPixelSize = mode == 1 ? 4 : 3;
		const uint32 availablePixelCount = MIN<uint32>(pixelCount, static_cast<size_t>(srcEnd - src - 7) / encodedPixelSize);
		memcpy(dst, src, 7);
		WRITE_LE_UINT16(dst + 4, availablePixelCount);
		src += 7;
		dst += 7;

		if (mode == 1) {
			const uint32 copySize = availablePixelCount * 4;
			memcpy(dst, src, copySize);
			src += copySize;
			dst += copySize;
		} else {
			for (uint32 i = 0; i < availablePixelCount; i++) {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				dst[3] = 0;
				src += 3;
				dst += 4;
			}
		}
		if (availablePixelCount != static_cast<uint32>(pixelCount))
			break;
	}
	return true;
}

byte RleBlock::blendChannel(byte src, byte dest, byte invAlpha, const AlphaBlendLUT &alphaLUT) {
	// Mode-1 RLE source channels are already premultiplied by opacity. The
	// fourth byte is inverse alpha, so the LUT supplies only the destination
	// contribution before the two terms are added and clamped.
	const int destPart = alphaLUT.scale(invAlpha, dest);
	return static_cast<byte>(MIN(static_cast<int>(src) + destPart, 255));
}

/**
 * Draw the RLE spans with opaque copying or lookup-table alpha blending.
 *
 * Span data begins after the two-byte resource prefix.
 */
void RleBlock::drawToScreen(Graphics::ManagedSurface *dst, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const {
	if (!_rleData || _dataSize < 2)
		return;

	const byte *ptr = _rleData + 2; // Skip effectiveHeight
	const byte *end = _rleData + _dataSize;

	while (ptr < end) {
		if (ptr + 7 > end)
			break;

		int16 xOff = READ_LE_INT16(ptr);
		int16 yOff = READ_LE_INT16(ptr + 2);
		int16 pixelCount = READ_LE_INT16(ptr + 4);
		byte mode = ptr[6];
		ptr += 7;
		if (pixelCount < 0 || 1 < mode || static_cast<uint32>(end - ptr) < static_cast<uint32>(pixelCount) * 4)
			return;

		int screenX = pos.x + xOff;
		int screenY = pos.y + yOff;

		if (screenY < 0 || dst->h <= screenY) {
			ptr += pixelCount * 4;
			continue;
		}

		if (mode == 0) {
			// Opaque mode: direct copy (4bpp after expand)
			int startCol = 0;
			int endCol = pixelCount;

			// Clip left
			if (screenX < 0) {
				startCol = -1 * screenX;
				screenX = 0;
			}
			// Clip right
			if (screenX + (endCol - startCol) > dst->w) {
				endCol = startCol + (dst->w - screenX);
			}

			if (startCol < endCol) {
				const byte *srcPixel = ptr + startCol * 4;
				byte *dstPixel = static_cast<byte *>(dst->getBasePtr(screenX, screenY));

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
			// Mode 1 stores premultiplied BGR followed by inverse alpha.
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
				byte *dstPixel = static_cast<byte *>(dst->getBasePtr(screenX, screenY));

				for (int i = startCol; i < endCol; i++) {
					const byte invAlpha = srcPixel[3];
					dstPixel[0] = blendChannel(srcPixel[0], dstPixel[0], invAlpha, alphaLUT);
					dstPixel[1] = blendChannel(srcPixel[1], dstPixel[1], invAlpha, alphaLUT);
					dstPixel[2] = blendChannel(srcPixel[2], dstPixel[2], invAlpha, alphaLUT);
					dstPixel[3] = 255;
					srcPixel += 4;
					dstPixel += 4;
				}
			}
			ptr += pixelCount * 4;
		}
	}
}

void RleBlock::drawToScreenClipped(Graphics::ManagedSurface *dst, const Common::Point32 &pos, const Common::Rect32 &clip, const AlphaBlendLUT &alphaLUT) const {
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
		if (pixelCount < 0 || 1 < mode || static_cast<uint32>(end - ptr) < static_cast<uint32>(pixelCount) * 4)
			return;

		int screenX = pos.x + xOff;
		int screenY = pos.y + yOff;

		// Clip vertically against clip rect and screen bounds
		if (screenY < clip.top || clip.bottom <= screenY || screenY < 0 || dst->h <= screenY) {
			ptr += pixelCount * 4;
			continue;
		}

		// Clip horizontally against clip rect
		int startCol = 0;
		int endCol = pixelCount;

		if (screenX + endCol <= clip.left || clip.right <= screenX) {
			ptr += pixelCount * 4;
			continue;
		}

		if (screenX < clip.left) {
			startCol = clip.left - screenX;
			screenX = clip.left;
		}
		if (screenX + (endCol - startCol) > clip.right) {
			endCol = startCol + (clip.right - screenX);
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
			byte *dstPixel = static_cast<byte *>(dst->getBasePtr(screenX, screenY));

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
				// Mode 1 stores premultiplied BGR followed by inverse alpha.
				for (int i = startCol; i < endCol; i++) {
					const byte invAlpha = srcPixel[3];
					dstPixel[0] = blendChannel(srcPixel[0], dstPixel[0], invAlpha, alphaLUT);
					dstPixel[1] = blendChannel(srcPixel[1], dstPixel[1], invAlpha, alphaLUT);
					dstPixel[2] = blendChannel(srcPixel[2], dstPixel[2], invAlpha, alphaLUT);
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
 * Load an animation from an `.an` cache file.
 * Format: DWORD frameCount, per frame: 24-byte header + 4-byte dataSize + data.
 */
bool Animation::loadFromFile(const Common::Path &path) {
	Common::Path resolvedPath(path);
	const Common::String pathString = path.toString();
	if (!pathString.hasSuffixIgnoreCase(".an")) {
		if (pathString.hasSuffixIgnoreCase(".bmp") || pathString.hasSuffixIgnoreCase(".bmt"))
			resolvedPath.removeExtension();
		resolvedPath = resolvedPath.append(".an");
	}

	Zoombini2Engine *vm = static_cast<Zoombini2Engine *>(g_engine);
	Common::ScopedPtr<Common::SeekableReadStream> stream(vm ? vm->openResourceFile(resolvedPath.toString('/')) : nullptr);
	if (!stream) {
		debug(3, "Animation: cannot open '%s'", resolvedPath.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &f = *stream;

	const uint32 frameCount = f.readUint32LE();
	if (f.err() || f.eos() || 1024 < frameCount) {
		warning("Animation: suspicious frame count %u in '%s'", frameCount,
				resolvedPath.toString().c_str());
		return false;
	}

	Common::Array<RleBlock *> loadedFrames;
	loadedFrames.reserve(frameCount);
	bool complete = true;

	for (uint32 i = 0; i < frameCount; i++) {
		RleBlock *frame = new RleBlock();
		if (!frame->loadFromStream(&f)) {
			warning("Animation: stopped after %u of %u frames in '%s'", i, frameCount, resolvedPath.toString().c_str());
			delete frame;
			complete = false;
			break;
		}
		loadedFrames.push_back(frame);
	}

	if (f.pos() != f.size()) {
		warning("Animation: ignoring %u unparsed bytes in '%s'", static_cast<uint32>(f.size() - f.pos()), resolvedPath.toString().c_str());
		complete = false;
	}

	for (uint frameIndex = 0; frameIndex < _frames.size(); frameIndex++)
		delete _frames[frameIndex];
	_frames.clear();
	const uint loadedFrameCount = loadedFrames.size();
	_frames.swap(loadedFrames);

	debug(3, "Animation: loaded %u of %u frames from '%s'%s", loadedFrameCount, frameCount, resolvedPath.toString().c_str(),
		  complete ? "" : " with salvage warnings");
	return true;
}

const RleBlock *Animation::getFrame(int index) const {
	if (0 <= index && index < static_cast<int>(_frames.size()))
		return _frames[index];
	return nullptr;
}

// ============================================================================
// Fixed32
// ============================================================================

Fixed32 Fixed32::fromInt(int32 value) {
	return fromRawBits(static_cast<uint32>(value) << kFractionalBits);
}

Fixed32 Fixed32::fromFloat(Float32 value) {
	static constexpr Float32 kMinimum = -2097152.0f;
	static constexpr Float32 kMaximumExclusive = 2097152.0f;
	static constexpr Float32 kScale = 1 << kFractionalBits;
	assert(kMinimum <= value && value < kMaximumExclusive);
	return Fixed32(static_cast<int32>(value * kScale));
}

int32 Fixed32::toInt() const {
	return arithmeticShiftRight(static_cast<uint32>(_rawValue));
}

Float32 Fixed32::toFloat() const {
	static constexpr Float32 kScale = 1 << kFractionalBits;
	return static_cast<Float32>(_rawValue) / kScale;
}

Fixed32 Fixed32::operator*(const Fixed32 &right) const {
	const uint32 productLow = static_cast<uint32>(_rawValue) * static_cast<uint32>(right._rawValue);
	return Fixed32(arithmeticShiftRight(productLow));
}

Fixed32 Fixed32::operator/(const Fixed32 &right) const {
	assert(right._rawValue != 0);
	const int64 dividend = static_cast<int64>(_rawValue) * (1 << kFractionalBits);
	const int64 quotient = dividend / static_cast<int64>(right._rawValue);
	return fromRawBits(static_cast<uint32>(quotient));
}

Fixed32 Fixed32::operator-(const Fixed32 &right) const {
	return fromRawBits(static_cast<uint32>(_rawValue) - static_cast<uint32>(right._rawValue));
}

Fixed32 Fixed32::operator+(const Fixed32 &right) const {
	return fromRawBits(static_cast<uint32>(_rawValue) + static_cast<uint32>(right._rawValue));
}

void Fixed32::operator+=(const Fixed32 &right) {
	*this = *this + right;
}

void Fixed32::operator-=(const Fixed32 &right) {
	*this = *this - right;
}

void Fixed32::operator*=(const Fixed32 &right) {
	*this = *this * right;
}

void Fixed32::operator/=(const Fixed32 &right) {
	*this = *this / right;
}

bool Fixed32::operator>(const Fixed32 &right) const {
	return right._rawValue < _rawValue;
}

bool Fixed32::operator==(const Fixed32 &right) const {
	return _rawValue == right._rawValue;
}

bool Fixed32::operator!=(const Fixed32 &right) const {
	return !(*this == right);
}

Common::String Fixed32::toString() const {
	static constexpr double kScale = 1 << kFractionalBits;
	Common::String result = Common::String::format("%.10f", static_cast<double>(_rawValue) / kScale);

	while (result.lastChar() == '0')
		result.deleteLastChar();
	if (result.lastChar() == '.')
		result.deleteLastChar();

	return result;
}

Fixed32 Fixed32::fromRawBits(uint32 value) {
	static constexpr uint32 kMaxPositive = 0x7FFFFFFFU;

	if (value <= kMaxPositive)
		return Fixed32(static_cast<int32>(value));

	return Fixed32(-1 - static_cast<int32>(~value));
}

int32 Fixed32::arithmeticShiftRight(uint32 value) {
	static constexpr uint32 kSignBit = 0x80000000U;
	static constexpr int32 kNegativeBias = 1 << (32 - kFractionalBits);
	const uint32 shifted = value >> kFractionalBits;

	if ((value & kSignBit) == 0)
		return static_cast<int32>(shifted);

	return static_cast<int32>(shifted) - kNegativeBias;
}

bool PointFixed32::operator==(const PointFixed32 &right) const {
	return x == right.x && y == right.y;
}

bool PointFixed32::operator!=(const PointFixed32 &right) const {
	return !(*this == right);
}

PointFixed32 PointFixed32::operator+(const PointFixed32 &right) const {
	return PointFixed32(x + right.x, y + right.y);
}

PointFixed32 PointFixed32::operator-(const PointFixed32 &right) const {
	return PointFixed32(x - right.x, y - right.y);
}

PointFixed32 PointFixed32::operator*(const Fixed32 &right) const {
	return PointFixed32(x * right, y * right);
}

PointFixed32 PointFixed32::operator*(int32 right) const {
	return *this * Fixed32::fromInt(right);
}

PointFixed32 PointFixed32::operator/(const Fixed32 &right) const {
	return PointFixed32(x / right, y / right);
}

PointFixed32 PointFixed32::operator/(int32 right) const {
	return *this / Fixed32::fromInt(right);
}

void PointFixed32::operator+=(const PointFixed32 &right) {
	x += right.x;
	y += right.y;
}

void PointFixed32::operator-=(const PointFixed32 &right) {
	x -= right.x;
	y -= right.y;
}

void PointFixed32::operator*=(const Fixed32 &right) {
	x *= right;
	y *= right;
}

void PointFixed32::operator/=(const Fixed32 &right) {
	x /= right;
	y /= right;
}

Fixed32 PointFixed32::sqrDist(const PointFixed32 &right) const {
	static constexpr uint64 kFractionMask = (1ULL << Fixed32::kFractionalBits) - 1;
	const uint64 xDifference = rawMagnitudeDifference(x, right.x);
	const uint64 yDifference = rawMagnitudeDifference(y, right.y);
	const uint64 xSquared = xDifference * xDifference;
	const uint64 ySquared = yDifference * yDifference;
	const uint64 fractionalCarry = ((xSquared & kFractionMask) + (ySquared & kFractionMask)) >> Fixed32::kFractionalBits;
	const uint64 distanceRaw = (xSquared >> Fixed32::kFractionalBits) + (ySquared >> Fixed32::kFractionalBits) + fractionalCarry;
	return Fixed32::fromRawBits(static_cast<uint32>(distanceRaw));
}

Common::String PointFixed32::toString() const {
	return Common::String::format("%s, %s", x.toString().c_str(), y.toString().c_str());
}

uint64 PointFixed32::rawMagnitudeDifference(const Fixed32 &left, const Fixed32 &right) {
	const int64 difference = static_cast<int64>(left._rawValue) - static_cast<int64>(right._rawValue);
	if (difference < 0)
		return static_cast<uint64>(-difference);
	return static_cast<uint64>(difference);
}

PointFixed32 operator*(const Fixed32 &left, const PointFixed32 &right) {
	return right * left;
}

PointFixed32 operator*(int32 left, const PointFixed32 &right) {
	return right * left;
}

// ============================================================================
// CurveSegment - runtime-selected cubic Bezier numeric representation.
// ============================================================================

class CurveSegment::Evaluator {
public:
	virtual ~Evaluator() {}
	virtual void init(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1, const Common::Point32 &end,
					  int stepValue, int waitInitial) = 0;
	virtual void computeCoeffs() = 0;
	virtual bool evaluate(uint32 tickCount, Common::Point32 &outPosition) = 0;
	virtual Common::Point32 getPosition() const = 0;
	virtual void setStartTime(uint32 tickCount) = 0;
	virtual uint32 getStartTime() const = 0;
	virtual void setWaitRemaining(int waitRemaining) = 0;
	virtual int getWaitRemaining() const = 0;
	virtual void setParameter(Float32 parameter) = 0;
	virtual Float32 getParameter() const = 0;
};

struct CurveSegment::FixedNumeric {
	typedef Fixed32 Scalar;
	typedef PointFixed32 Point;

	static Scalar fromInt(int32 value) { return Fixed32::fromInt(value); }
	static Scalar fromRaw(int32 value) { return Fixed32::fromRaw(value); }
	static Scalar fromFloat(Float32 value) { return Fixed32::fromFloat(value); }
	static int32 toInt(const Scalar &value) { return value.toInt(); }
	static Float32 toFloat(const Scalar &value) { return value.toFloat(); }
	static bool greater(const Scalar &left, const Scalar &right) { return left > right; }
};

struct CurveSegment::FloatNumeric {
	typedef Float32 Scalar;

	struct Point {
		Scalar x;
		Scalar y;

		Point() : x(0.0f), y(0.0f) {}
		Point(Scalar xValue, Scalar yValue) : x(xValue), y(yValue) {}

		Point operator+(const Point &right) const { return Point(x + right.x, y + right.y); }
		Point operator-(const Point &right) const { return Point(x - right.x, y - right.y); }
		Point operator*(Scalar right) const { return Point(x * right, y * right); }
	};

	static Scalar fromInt(int32 value) { return static_cast<Scalar>(value); }
	static Scalar fromRaw(int32 value) { return static_cast<Scalar>(value) / 1024.0f; }
	static Scalar fromFloat(Float32 value) { return value; }
	static int32 toInt(Scalar value) {
		int32 result = static_cast<int32>(value);
		if (value < static_cast<Scalar>(result))
			result -= 1;
		return result;
	}
	static Float32 toFloat(Scalar value) { return value; }
	static bool greater(Scalar left, Scalar right) { return right < left; }
};

template<typename Numeric>
class CurveSegment::TypedEvaluator : public CurveSegment::Evaluator {
public:
	typedef typename Numeric::Scalar Scalar;
	typedef typename Numeric::Point Point;

	TypedEvaluator()
		: _start(), _control0(), _control1(), _end(), _cubic(), _quadratic(), _linear(), _parameter(), _position(), _step(), _waitInitial(0),
		  _waitRemaining(0), _startTime(0) {
	}

	void init(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1, const Common::Point32 &end,
			  int stepValue, int waitInitial) override {
		_start = Point(Numeric::fromInt(start.x), Numeric::fromInt(start.y));
		_control0 = Point(Numeric::fromInt(control0.x), Numeric::fromInt(control0.y));
		_control1 = Point(Numeric::fromInt(control1.x), Numeric::fromInt(control1.y));
		_end = Point(Numeric::fromInt(end.x), Numeric::fromInt(end.y));
		_cubic = Point();
		_quadratic = Point();
		_linear = Point();
		_parameter = Scalar();
		_position = _start;
		_step = Numeric::fromRaw(stepValue);
		_waitInitial = waitInitial;
		_waitRemaining = waitInitial;
		_startTime = 0;
	}

	void computeCoeffs() override {
		_parameter = Scalar();
		const Scalar three = Numeric::fromInt(3);
		_linear = (_control0 - _start) * three;
		_quadratic = (_control1 - _control0) * three - _linear;
		_cubic = _end - _start - _linear - _quadratic;
		_waitRemaining = _waitInitial;
	}

	bool evaluate(uint32 tickCount, Common::Point32 &outPosition) override {
		const Scalar completionThreshold = Numeric::fromRaw(950);
		if (!Numeric::greater(_parameter, completionThreshold)) {
			const uint32 elapsed = tickCount - _startTime;
			const int32 ticks = static_cast<int32>(elapsed / 5u);
			_parameter = _step * Numeric::fromInt(ticks);
			calculatePosition();
			outPosition = getPosition();
			return true;
		}

		if (0 < _waitRemaining) {
			_waitRemaining -= 1;
			return true;
		}

		return false;
	}

	Common::Point32 getPosition() const override {
		return Common::Point32(Numeric::toInt(_position.x), Numeric::toInt(_position.y));
	}

	void setStartTime(uint32 tickCount) override { _startTime = tickCount; }
	uint32 getStartTime() const override { return _startTime; }
	void setWaitRemaining(int waitRemaining) override { _waitRemaining = waitRemaining; }
	int getWaitRemaining() const override { return _waitRemaining; }
	void setParameter(Float32 parameter) override {
		_parameter = Numeric::fromFloat(parameter);
		calculatePosition();
	}
	Float32 getParameter() const override { return Numeric::toFloat(_parameter); }

private:
	Point _start;
	Point _control0;
	Point _control1;
	Point _end;
	Point _cubic;
	Point _quadratic;
	Point _linear;
	Scalar _parameter;
	Point _position;
	Scalar _step;
	int _waitInitial;
	int _waitRemaining;
	uint32 _startTime;

	void calculatePosition() {
		const Scalar squaredParameter = _parameter * _parameter;
		const Scalar cubedParameter = squaredParameter * _parameter;
		_position = _start + _linear * _parameter + _quadratic * squaredParameter + _cubic * cubedParameter;
	}
};

CurveSegment::CurveSegment(bool useFloatingPoint)
	: _evaluator(createEvaluator(useFloatingPoint)), _start(), _control0(), _control1(), _end(), _stepValue(0), _waitInitial(0), _initialized(false),
	  _coefficientsReady(false), _useFloatingPoint(useFloatingPoint) {
}

CurveSegment::~CurveSegment() {
	delete _evaluator;
}

void CurveSegment::init(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1, const Common::Point32 &end,
						int stepValue, int waitInitial) {
	_start = start;
	_control0 = control0;
	_control1 = control1;
	_end = end;
	_stepValue = stepValue;
	_waitInitial = waitInitial;
	_initialized = true;
	_coefficientsReady = false;
	_evaluator->init(start, control0, control1, end, stepValue, waitInitial);
}

void CurveSegment::computeCoeffs() {
	assert(_initialized);
	_evaluator->computeCoeffs();
	_coefficientsReady = true;
}

bool CurveSegment::evaluate(uint32 tickCount, Common::Point32 &outPosition) {
	assert(_coefficientsReady);
	return _evaluator->evaluate(tickCount, outPosition);
}

Common::Point32 CurveSegment::getStartPosition() const {
	return _start;
}

Common::Point32 CurveSegment::getEndPosition() const {
	return _end;
}

Common::Point32 CurveSegment::getPosition() const {
	return _initialized ? _evaluator->getPosition() : Common::Point32();
}

void CurveSegment::setStartTime(uint32 tickCount) {
	_evaluator->setStartTime(tickCount);
}

void CurveSegment::setFloatingPointMode(bool useFloatingPoint) {
	if (_useFloatingPoint == useFloatingPoint)
		return;
	_useFloatingPoint = useFloatingPoint;
	rebuildEvaluator();
}

CurveSegment::Evaluator *CurveSegment::createEvaluator(bool useFloatingPoint) {
	if (useFloatingPoint)
		return new TypedEvaluator<FloatNumeric>();
	return new TypedEvaluator<FixedNumeric>();
}

void CurveSegment::rebuildEvaluator() {
	const uint32 previousStartTime = _evaluator->getStartTime();
	const int previousWaitRemaining = _evaluator->getWaitRemaining();
	const Float32 previousParameter = _evaluator->getParameter();

	delete _evaluator;
	_evaluator = createEvaluator(_useFloatingPoint);
	if (!_initialized)
		return;

	_evaluator->init(_start, _control0, _control1, _end, _stepValue, _waitInitial);
	_evaluator->setStartTime(previousStartTime);
	if (_coefficientsReady) {
		_evaluator->computeCoeffs();
		_evaluator->setStartTime(previousStartTime);
		_evaluator->setWaitRemaining(previousWaitRemaining);
		_evaluator->setParameter(previousParameter);
	}
}

// ============================================================================
// PathObject - Bezier path composed of chained CurveSegments.
// ============================================================================

PathObject::PathObject()
	: currentSegment(0), looping(false), finished(false),
	  endPos(), startTime(0) {
}

PathObject::~PathObject() {
	for (uint i = 0; i < segments.size(); i++)
		delete segments[i];
}

void PathObject::appendSegment(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1, const Common::Point32 &end,
						   int stepValue, int waitInitial) {
	Zoombini2Engine *vm = static_cast<Zoombini2Engine *>(g_engine);
	CurveSegment *segment = new CurveSegment(vm && vm->useFloatingPointPaths());
	segment->init(start, control0, control1, end, stepValue, waitInitial);
	segments.push_back(segment);
}

/**
 * Load a `.PAT` Bezier path file.
 *
 * Format:
 *   FIRST=coord:x0,y0,cx0,cy0,cx1,cy1,x1,y1 step:S wait:W
 *   NEXT_N=coord:cx0,cy0,cx1,cy1,x1,y1 step:S wait:W
 *
 * NEXT segments chain from the previous segment's endpoint and retain their own
 * step and wait values.
 */
PathObject *PathObject::loadFromPAT(const Common::Path &path) {
	Zoombini2Engine *vm = static_cast<Zoombini2Engine *>(g_engine);
	Common::ScopedPtr<Common::SeekableReadStream> stream(vm ? vm->openResourceFile(path.toString('/')) : nullptr);
	if (!stream) {
		warning("PathObject: cannot open PAT '%s'", path.toString().c_str());
		return nullptr;
	}
	Common::SeekableReadStream &f = *stream;

	PathObject *obj = new PathObject();

	// Read FIRST line
	Common::String line = f.readLine();
	if (line.hasPrefix("FIRST=coord:")) {
		int x0, y0, cx0, cy0, cx1, cy1, x1, y1, stepVal, waitVal;
		if (sscanf(line.c_str(),
				   "FIRST=coord:%d,%d,%d,%d,%d,%d,%d,%d step:%d wait:%d",
				   &x0, &y0, &cx0, &cy0, &cx1, &cy1, &x1, &y1,
				   &stepVal, &waitVal) >= 10) {
			obj->appendSegment(Common::Point32(x0, y0), Common::Point32(cx0, cy0), Common::Point32(cx1, cy1), Common::Point32(x1, y1), stepVal, waitVal);
		}
	}

	// Read NEXT lines (start with 'N')
	while (!f.eos()) {
		line = f.readLine();
		if (line.empty() || line[0] != 'N')
			break;

		// Parse the continuation coordinates, speed, and endpoint wait.
		int ext, ncx0, ncy0, ncx1, ncy1, nx1, ny1, stepVal, waitVal;
		// Skip the leading 'N' and parse the remaining `EXT` record.
		if (sscanf(line.c_str() + 1,
				   "EXT_%d=coord:%d,%d,%d,%d,%d,%d step:%d wait:%d",
				   &ext, &ncx0, &ncy0, &ncx1, &ncy1, &nx1, &ny1, &stepVal, &waitVal) >= 9) {
			CurveSegment *prev = obj->segments.back();
			// P0 comes from previous segment's P1
			obj->appendSegment(prev->getEndPosition(), Common::Point32(ncx0, ncy0), Common::Point32(ncx1, ncy1), Common::Point32(nx1, ny1), stepVal, waitVal);
		}
	}

	if (obj->segments.empty()) {
		delete obj;
		return nullptr;
	}

	// Set endpoint from last segment
	CurveSegment *last = obj->segments.back();
	obj->endPos = last->getEndPosition();

	return obj;
}

/**
 * Start path evaluation. Sets startTime on all segments and computes
 * coefficients for the first segment.
 */
void PathObject::start(uint32 tickCount) {
	synchronizeNumericMode();
	currentSegment = 0;
	finished = false;
	startTime = tickCount;

	for (uint i = 0; i < segments.size(); i++)
		segments[i]->setStartTime(tickCount);

	if (!segments.empty())
		segments[0]->computeCoeffs();
}

/**
 * Advance the path evaluation. Returns true if still walking,
 * false if the entire path is complete.
 * @p outPos receives the current screen pos.
 */
bool PathObject::advance(uint32 tickCount, Common::Point32 &outPos) {
	synchronizeNumericMode();
	if (finished) {
		outPos = endPos;
		return false;
	}

	if (currentSegment >= static_cast<int>(segments.size())) {
		finished = true;
		outPos = endPos;
		return false;
	}

	CurveSegment *seg = segments[currentSegment];
	if (seg->evaluate(tickCount, outPos))
		return true;

	// Advance after the current segment completes.
	currentSegment += 1;
	if (static_cast<int>(segments.size()) <= currentSegment) {
		// All segments done
		finished = true;
		outPos = endPos;
		return false;
	}

	// Compute coefficients for the next segment and evaluate it
	CurveSegment *next = segments[currentSegment];
	next->computeCoeffs();
	next->setStartTime(tickCount);
	next->evaluate(tickCount, outPos);
	return true;
}

void PathObject::synchronizeNumericMode() {
	Zoombini2Engine *vm = static_cast<Zoombini2Engine *>(g_engine);
	const bool useFloatingPoint = vm && vm->useFloatingPointPaths();
	for (uint i = 0; i < segments.size(); i++)
		segments[i]->setFloatingPointMode(useFloatingPoint);
}

// ============================================================================
// ZoombiniAnimation
// ============================================================================

ZoombiniAnimation::Cell::~Cell() {
	for (uint i = 0; i < frames.size(); i++)
		delete frames[i];
}

ZoombiniAnimation::ZoombiniAnimation() {
}

ZoombiniAnimation::~ZoombiniAnimation() {
}

bool ZoombiniAnimation::loadFromFile(const Common::Path &path) {
	Zoombini2Engine *vm = static_cast<Zoombini2Engine *>(g_engine);
	Common::ScopedPtr<Common::SeekableReadStream> stream(vm ? vm->openResourceFile(path.toString('/')) : nullptr);
	if (!stream) {
		warning("ZoombiniAnimation: cannot open '%s'", path.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &f = *stream;

	Cell loadedCells[kCellCount];
	int cellIndex = 0;
	uint32 recoveredFrameCount = 0;
	bool complete = true;
	for (int d0 = 0; d0 < kDim0; d0++) {
		for (int d1 = 0; d1 < kDim1; d1++) {
			for (int d2 = 0; d2 < kDim2; d2++) {
				const uint32 frameCount = f.readUint32LE();
				if (f.eos() || f.err() || 1024 < frameCount) {
					warning("ZoombiniAnimation: stopped before cell %d in '%s'", cellIndex, path.toString().c_str());
					complete = false;
					break;
				}

				Cell &cell = loadedCells[cellIndex];
				cell.frames.reserve(frameCount);

				for (uint32 fr = 0; fr < frameCount; fr++) {
					const uint32 outerSize = f.readUint32LE();
					if (f.err() || f.eos()) {
						warning("ZoombiniAnimation: missing frame size at cell %d frame %u", cellIndex, fr);
						complete = false;
						break;
					}

					RleBlock *frame = new RleBlock();
					if (!frame->loadAnmFrame(&f, outerSize)) {
						warning("ZoombiniAnimation: failed cell %d frame %u", cellIndex, fr);
						delete frame;
						complete = false;
						break;
					}

					cell.frames.push_back(frame);
					recoveredFrameCount += 1;
				}

				cellIndex += 1;
				if (!complete)
					break;
			}
			if (!complete)
				break;
		}
		if (!complete)
			break;
	}
	if (f.pos() != f.size()) {
		warning("ZoombiniAnimation: ignoring %u unparsed bytes in '%s'", static_cast<uint32>(f.size() - f.pos()), path.toString().c_str());
		complete = false;
	}
	if (cellIndex == 0 && recoveredFrameCount == 0)
		return false;

	for (int index = 0; index < kCellCount; index++)
		_cells[index].frames.swap(loadedCells[index].frames);

	debug(2, "ZoombiniAnimation: loaded %d of %d cells and %u frames from '%s'%s", cellIndex, kCellCount, recoveredFrameCount,
		  path.toString().c_str(), complete ? "" : " with salvage warnings");
	return true;
}

const RleBlock *ZoombiniAnimation::getFrame(int cellIndex, int frameIndex) const {
	if (cellIndex < 0 || kCellCount <= cellIndex)
		return nullptr;
	const Cell &cell = _cells[cellIndex];
	if (frameIndex < 0 || static_cast<int>(cell.frames.size()) <= frameIndex)
		return nullptr;
	return cell.frames[frameIndex];
}

int ZoombiniAnimation::getFrameCount(int cellIndex) const {
	if (cellIndex < 0 || kCellCount <= cellIndex)
		return 0;
	return _cells[cellIndex].frames.size();
}

Common::Point ZoombiniAnimation::getSpriteSize(int cell, int frame) const {
	if (cell < 0 || kDim0 <= cell || frame < 0)
		return Common::Point();
	const int entry = cell * kDim1 * kDim2;
	const int selectedFrame = getFrameCount(entry) == 1 ? 0 : frame;
	const RleBlock *sprite = getFrame(entry, selectedFrame);
	if (!sprite)
		return Common::Point();
	return Common::Point(sprite->getWidth(), sprite->getHeight());
}

void ZoombiniAnimation::drawZoombini(Graphics::ManagedSurface *screen, const ZmbTrait &traits, const Common::Point32 &pos,
									 int cell, int frame, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip) const {
	if (cell < 0 || kDim0 <= cell || frame < 0)
		return;
	const int baseIndex = cell * kDim1 * kDim2;
	for (int layer = 0; layer < kDim1; layer++) {
		int variant = 0;
		if (0 < layer)
			variant = traits.getValue(static_cast<ZmbTrait::TraitIndex>(layer - 1));
		if (variant < 0 || kDim2 <= variant)
			continue;
		const int entry = baseIndex + layer * kDim2 + variant;
		const int selectedFrame = getFrameCount(entry) == 1 ? 0 : frame;
		const RleBlock *sprite = getFrame(entry, selectedFrame);
		if (!sprite)
			continue;
		if (clip)
			sprite->drawToScreenClipped(screen, pos, *clip, alphaLUT);
		else
			sprite->drawToScreen(screen, pos, alphaLUT);
	}
}

// ============================================================================
// UIButton - clickable button with normal, hovered, and disabled states.
// ============================================================================

UIButton::UIButton()
	: _rect(0, 0, 0, 0),
	  _enabled(true),
	  _drawWhenNotHovered(true),
	  _useRleMaskBlend(false),
	  _wasHovering(false),
	  _isHovering(false),
	  _normalBB(nullptr),
	  _hoverBB(nullptr),
	  _disabledBB(nullptr),
	  _normalRle(nullptr),
	  _hoverRle(nullptr),
	  _disabledRle(nullptr),
	  _overlay(nullptr),
	  _overlayOffset(),
	  _overlayClipRight(nullptr) {
}

UIButton::~UIButton() {
	clearImages();
}

void UIButton::clearImages() {
	delete _normalBB;
	delete _hoverBB;
	delete _disabledBB;
	delete _normalRle;
	delete _hoverRle;
	delete _disabledRle;
	_normalBB = nullptr;
	_hoverBB = nullptr;
	_disabledBB = nullptr;
	_normalRle = nullptr;
	_hoverRle = nullptr;
	_disabledRle = nullptr;
}

bool UIButton::loadImages(const Common::Path &normalPath,
						  const Common::Path &hoverPath,
						  const Common::Path &disabledPath) {
	clearImages();
	_useRleMaskBlend = false;
	if (!loadImage(normalPath, _normalBB, _normalRle)) {
		warning("UIButton: Failed to load normal image: %s", normalPath.toString().c_str());
		return false;
	}

	if (_rect.width() == 0) {
		if (_normalBB) {
			_rect.setWidth(_normalBB->getWidth());
			_rect.setHeight(_normalBB->getHeight());
		} else if (_normalRle) {
			_rect.setWidth(_normalRle->getWidth());
			_rect.setHeight(_normalRle->getHeight());
		}
	}

	if (!loadImage(hoverPath, _hoverBB, _hoverRle))
		debug(1, "UIButton: No hover image for: %s", hoverPath.toString().c_str());
	if (!loadImage(disabledPath, _disabledBB, _disabledRle))
		debug(1, "UIButton: No disabled image for: %s", disabledPath.toString().c_str());

	return _normalBB != nullptr || _normalRle != nullptr;
}

bool UIButton::loadImagesWithMask(const Common::Path &normalPath, const Common::Path &normalMask,
								  const Common::Path &hoverPath, const Common::Path &hoverMask,
								  const Common::Path &disabledPath, const Common::Path &disabledMask) {
	clearImages();
	_useRleMaskBlend = true;
	if (!loadMaskedImage(normalPath, normalMask, _normalBB, _normalRle)) {
		warning("UIButton: Failed to load normal image with mask");
		return false;
	}

	if (_rect.width() == 0) {
		if (_normalRle) {
			_rect.setWidth(_normalRle->getWidth());
			_rect.setHeight(_normalRle->getHeight());
		} else if (_normalBB) {
			_rect.setWidth(_normalBB->getWidth());
			_rect.setHeight(_normalBB->getHeight());
		}
	}

	if (!loadMaskedImage(hoverPath, hoverMask, _hoverBB, _hoverRle))
		debug(1, "UIButton: No masked hover image for: %s", hoverPath.toString().c_str());
	if (!loadMaskedImage(disabledPath, disabledMask, _disabledBB, _disabledRle))
		debug(1, "UIButton: No masked disabled image for: %s", disabledPath.toString().c_str());
	return true;
}

bool UIButton::loadImage(const Common::Path &path, BitBlock *&bitmap, RleBlock *&rle) {
	if (path.empty())
		return true;

	bitmap = new BitBlock();
	if (bitmap->load(path))
		return true;
	delete bitmap;
	bitmap = nullptr;

	rle = new RleBlock();
	if (rle->load(path))
		return true;
	delete rle;
	rle = nullptr;
	return false;
}

bool UIButton::loadMaskedImage(const Common::Path &colorPath, const Common::Path &maskPath, BitBlock *&bitmap, RleBlock *&rle) {
	if (colorPath.empty() && maskPath.empty())
		return true;
	if (colorPath.empty() || maskPath.empty())
		return false;

	rle = new RleBlock();
	if (rle->load(colorPath))
		return true;
	delete rle;
	rle = nullptr;

	bitmap = new BitBlock();
	if (bitmap->loadFromColorAlphaBMP(colorPath, maskPath))
		return true;
	delete bitmap;
	bitmap = nullptr;
	return false;
}

void UIButton::setRect(const Common::Point32 &pos, int width, int height) {
	_rect = Common::Rect(pos.x, pos.y, pos.x + width, pos.y + height);
}

void UIButton::setRect(const Common::Rect &rect) {
	_rect = rect;
}

bool UIButton::containsPoint(const Common::Point32 &pos) const {
	return _rect.left < pos.x && pos.x < _rect.right && _rect.top < pos.y && pos.y < _rect.bottom;
}

void UIButton::setOverlay(const RleBlock *overlay, const Common::Point32 &offset, const int *clipRight) {
	_overlay = overlay;
	_overlayOffset = offset;
	_overlayClipRight = clipRight;
}

int UIButton::drawAndHitTest(Graphics::ManagedSurface *dst, const Common::Point32 &mousePos,
							 const AlphaBlendLUT &alphaLUT) {
	_wasHovering = _isHovering;
	_isHovering = false;

	if (_enabled && containsPoint(mousePos))
		_isHovering = true;

	// Select image to draw
	BitBlock *imgToDraw = nullptr;
	RleBlock *rleToDraw = nullptr;

	if (_enabled && _isHovering) {
		if (_hoverBB) {
			imgToDraw = _hoverBB;
		} else if (_hoverRle) {
			rleToDraw = _hoverRle;
		}
	} else if (_enabled && _drawWhenNotHovered) {
		if (_normalBB) {
			imgToDraw = _normalBB;
		} else if (_normalRle) {
			rleToDraw = _normalRle;
		}
	} else if (!_enabled) {
		if (_disabledBB) {
			imgToDraw = _disabledBB;
		} else if (_disabledRle) {
			rleToDraw = _disabledRle;
		}
	}

	// Draw
	if (imgToDraw) {
		if (imgToDraw->hasAlpha()) {
			if (_useRleMaskBlend)
				imgToDraw->drawRleMaskBlend(dst, Common::Point32(_rect.left, _rect.top), alphaLUT);
			else
				imgToDraw->drawAlphaBlend(dst, Common::Point32(_rect.left, _rect.top));
		} else {
			imgToDraw->drawToSurface(dst, Common::Point32(_rect.left, _rect.top));
		}
	} else if (rleToDraw) {
		rleToDraw->drawToScreen(dst, Common::Point32(_rect.left, _rect.top), alphaLUT);
	}
	if (_overlay && _overlayClipRight) {
		const Common::Rect32 clip(0, 0, *_overlayClipRight, MIN(static_cast<int>(dst->h), 600));
		_overlay->drawToScreenClipped(dst, Common::Point32(_rect.left + _overlayOffset.x, _rect.top + _overlayOffset.y), clip, alphaLUT);
	}

	if (_isHovering)
		return _wasHovering ? 1 : 2;
	return 0;
}

// ============================================================================
// BitmapFont - bitmap-based font for UI text rendering.
// ============================================================================

BitmapFont::BitmapFont() : _loaded(false) {
	for (int i = 0; i < kNumGlyphs; i++) {
		_glyphs[i] = nullptr;
	}
}

BitmapFont::~BitmapFont() {
	for (int i = 0; i < kNumGlyphs; i++) {
		delete _glyphs[i];
	}
}

bool BitmapFont::load(const Common::Path &basePath, byte r, byte g, byte b) {
	Common::Path colorPath(basePath);
	if (!basePath.toString().hasSuffixIgnoreCase(".bmt"))
		colorPath = colorPath.append(".bmt");
	Common::Path stemPath(colorPath);
	stemPath.removeExtension();
	const Common::Path alphaPath(stemPath.toString() + "-A.bmt");

	BitBlock alphaBB;
	if (!alphaBB.loadFromColorAlphaBMP(colorPath, alphaPath)) {
		warning("BitmapFont: Failed to load BMP pair from %s", basePath.toString().c_str());
		return false;
	}

	const byte *srcAlpha = alphaBB.getAlpha();
	if (!srcAlpha) {
		warning("BitmapFont: No alpha channel in font BMP");
		return false;
	}

	const int width = alphaBB.getWidth();
	const int height = alphaBB.getHeight();

	BitBlock *loadedGlyphs[kNumGlyphs] = {};
	int glyphIndex = 0;
	int col = 1;

	while (col < width && glyphIndex < kNumGlyphs) {
		while (col < width) {
			bool blank = true;
			for (int row = 0; row < height; row++) {
				if (srcAlpha[row * width + col] != 0) {
					blank = false;
					break;
				}
			}
			if (!blank)
				break;
			col += 1;
		}

		if (col >= width)
			break;

		const int startCol = col;
		while (col < width) {
			bool blank = true;
			for (int row = 0; row < height; row++) {
				if (srcAlpha[row * width + col] != 0) {
					blank = false;
					break;
				}
			}
			if (blank)
				break;
			col += 1;
		}

		const int glyphWidth = col - startCol + 2;

		BitBlock *glyph = new BitBlock();
		glyph->createEmpty(glyphWidth, height, true);

		byte *dstAlpha = const_cast<byte *>(glyph->getAlpha());
		const int copyWidth = MIN(glyphWidth, width - startCol);
		for (int row = 0; row < height; row++) {
			memcpy(dstAlpha + row * glyphWidth, srcAlpha + row * width + startCol, copyWidth);
		}

		byte *dstPixels = const_cast<byte *>(glyph->getPixels());
		const int totalPixels = glyphWidth * height;
		for (int i = 0; i < totalPixels; i++) {
			dstPixels[i * 4 + 0] = r;
			dstPixels[i * 4 + 1] = g;
			dstPixels[i * 4 + 2] = b;
			dstPixels[i * 4 + 3] = 255;
		}

		loadedGlyphs[glyphIndex] = glyph;
		glyphIndex += 1;
	}

	if (glyphIndex != kNumGlyphs) {
		for (int i = 0; i < kNumGlyphs; i++)
			delete loadedGlyphs[i];
		warning("BitmapFont: expected %d glyphs but found %d in '%s'", kNumGlyphs, glyphIndex, colorPath.toString().c_str());
		return false;
	}

	for (int i = 0; i < kNumGlyphs; i++) {
		delete _glyphs[i];
		_glyphs[i] = loadedGlyphs[i];
	}
	_loaded = true;
	debug(1, "BitmapFont: Loaded %d glyphs from %s (color %d,%d,%d)",
		  glyphIndex, basePath.toString().c_str(), r, g, b);

	return true;
}

int BitmapFont::charToGlyphIndex(char c) {
	if (c >= 'A' && c <= 'Z') {
		return c - 'A'; // 0-25
	}
	if (c >= 'a' && c <= 'z') {
		return c - 'a' + 26; // 26-51
	}
	if (c >= '0' && c <= '9') {
		return c - '0' + 52; // 52-61
	}

	switch (c) {
	case '.':
		return 62;
	case ',':
		return 63;
	case ';':
		return 64;
	case ':':
		return 65;
	case '/':
		return 66;
	case '(':
		return 67;
	case ')':
		return 68;
	case '-':
		return 69;
	case '+':
		return 70;
	case '=':
		return 71;
	case '@':
		return 72;
	case '&':
		return 73;
	case '#':
		return 74;
	case '\'':
		return 75;
	case '?':
		return 76;
	case '!':
		return 77;
	case '*':
		return 78;
	case '_':
		return 80;
	default:
		return -1;
	}
}

int BitmapFont::drawString(Graphics::ManagedSurface *dst, const Common::Point32 &pos,
						   const Common::String &text, const AlphaBlendLUT &alphaLUT) const {
	if (!_loaded) {
		return 0;
	}

	int curX = pos.x;

	for (uint i = 0; i < text.size(); i++) {
		char c = text[i];

		if (c == ' ') {
			curX += kSpaceWidth;
			continue;
		}

		int glyphIdx = charToGlyphIndex(c);
		if (glyphIdx < 0 || glyphIdx >= kNumGlyphs || !_glyphs[glyphIdx]) {
			curX += kSpaceWidth; // Unknown character, treat as space
			continue;
		}

		_glyphs[glyphIdx]->drawRleMaskBlend(dst, Common::Point32(curX, pos.y), alphaLUT);
		curX += _glyphs[glyphIdx]->getWidth() + 2;
	}

	return curX - pos.x;
}

int BitmapFont::getStringWidth(const Common::String &text) const {
	if (!_loaded) {
		return 0;
	}

	int width = 0;

	for (uint i = 0; i < text.size(); i++) {
		char c = text[i];

		if (c == ' ') {
			width += kSpaceWidth;
			continue;
		}

		int glyphIdx = charToGlyphIndex(c);
		if (glyphIdx < 0 || glyphIdx >= kNumGlyphs || !_glyphs[glyphIdx]) {
			width += kSpaceWidth;
			continue;
		}

		width += _glyphs[glyphIdx]->getWidth() + 2;
	}

	return width;
}

// ============================================================================
// VolumePanel
// ============================================================================

VolumePanel::VolumePanel()
	: _musicVolume(100),
	  _sfxVolume(100),
	  _speechVolume(100),
	  _initialMusicVolume(100),
	  _initialSfxVolume(100),
	  _initialSpeechVolume(100),
	  _musicSliderX(kSliderMaxX),
	  _sfxSliderX(kSliderMaxX),
	  _speechSliderX(kSliderMaxX),
	  _activeSlider(-1),
	  _heldMousePos(),
	  _hasHeldMouse(false),
	  _lastAdjustedSlider(-1),
	  _soundManager(nullptr),
	  _speechPreviewSoundId(-1),
	  _sfxPreviewSoundId(-1),
	  _gaugeImage(nullptr) {
}

VolumePanel::~VolumePanel() {
	for (int i = 0; i < 3; i++)
		_sliderLabels[i].setOverlay(nullptr, Common::Point32(), nullptr);
	if (_soundManager) {
		if (0 <= _speechPreviewSoundId)
			_soundManager->unload(_speechPreviewSoundId);
		if (0 <= _sfxPreviewSoundId)
			_soundManager->unload(_sfxPreviewSoundId);
	}
	delete _gaugeImage;
}

bool VolumePanel::init(SoundManager *soundManager) {
	_soundManager = soundManager;
	delete _gaugeImage;
	_gaugeImage = new RleBlock();
	bool loaded = true;
	if (!_gaugeImage->loadFromFile(Common::Path("bmp/menu/OPTION - Jauge.rb"))) {
		warning("VolumePanel: Failed to load gauge image");
		delete _gaugeImage;
		_gaugeImage = nullptr;
		loaded = false;
	}

	_okButton.setRect(Common::Point32(294, 487), 76, 74);
	if (!_okButton.loadImages(Common::Path("bmp/menu/MENU - Valid - OK"), Common::Path("bmp/menu/MENU - Valid - OK highlight")))
		loaded = false;

	_noButton.setRect(Common::Point32(468, 487), 76, 74);
	if (!_noButton.loadImages(Common::Path("bmp/menu/MENU - Valid - NO"), Common::Path("bmp/menu/MENU - Valid - NO highlight")))
		loaded = false;

	_sliderLabels[0].setRect(Common::Point32(kLabelX, kMusicLabelY), kLabelW, kLabelH);
	if (!_sliderLabels[0].loadImages(Common::Path("bmp/menu/OPTION - Musique NORMAL"), Common::Path("bmp/menu/OPTION - Musique HIGHLIGHT")))
		loaded = false;

	_sliderLabels[1].setRect(Common::Point32(kLabelX, kSfxLabelY), kLabelW, kLabelH);
	if (!_sliderLabels[1].loadImages(Common::Path("bmp/menu/OPTION - Bruitages NORMAL"), Common::Path("bmp/menu/OPTION - Bruitages HILITE")))
		loaded = false;

	_sliderLabels[2].setRect(Common::Point32(kLabelX, kSpeechLabelY), kLabelW, kLabelH);
	if (!_sliderLabels[2].loadImages(Common::Path("bmp/menu/OPTION - Dialogues NORMAL"), Common::Path("bmp/menu/OPTION - Dialogues HILITE")))
		loaded = false;

	_musicSliderX = volumeToPixel(_musicVolume);
	_sfxSliderX = volumeToPixel(_sfxVolume);
	_speechSliderX = volumeToPixel(_speechVolume);
	_sliderLabels[0].setOverlay(_gaugeImage, Common::Point32(kSliderMinX - kLabelX, kMusicGaugeY - kMusicLabelY), &_musicSliderX);
	_sliderLabels[1].setOverlay(_gaugeImage, Common::Point32(kSliderMinX - kLabelX, kSfxGaugeY - kSfxLabelY), &_sfxSliderX);
	_sliderLabels[2].setOverlay(_gaugeImage, Common::Point32(kSliderMinX - kLabelX, kSpeechGaugeY - kSpeechLabelY), &_speechSliderX);
	if (_soundManager) {
		_speechPreviewSoundId = _soundManager->load(false, Common::Path("sounds/voice.wav"), false);
		_sfxPreviewSoundId = _soundManager->load(false, Common::Path("sounds/fx/03-BS01.wav"), false);
	}

	return loaded;
}

int VolumePanel::pixelToVolume(int x) {
	// Convert the 288-pixel gauge range to a percentage.
	int vol = (100 * (x - kSliderMinX)) / kSliderRange;
	if (vol < 0)
		vol = 0;
	if (vol > 100)
		vol = 100;
	return vol;
}

int VolumePanel::volumeToPixel(int volume) {
	return (kSliderRange * volume) / 100 + kSliderMinX;
}

void VolumePanel::setMusicVolume(int vol) {
	_musicVolume = CLIP(vol, 0, 100);
	_musicSliderX = volumeToPixel(_musicVolume);
}

void VolumePanel::setSfxVolume(int vol) {
	_sfxVolume = CLIP(vol, 0, 100);
	_sfxSliderX = volumeToPixel(_sfxVolume);
}

void VolumePanel::setSpeechVolume(int vol) {
	_speechVolume = CLIP(vol, 0, 100);
	_speechSliderX = volumeToPixel(_speechVolume);
}

void VolumePanel::setInitialVolumes(int music, int sfx, int speech) {
	setMusicVolume(music);
	setSfxVolume(sfx);
	setSpeechVolume(speech);
	_initialMusicVolume = _musicVolume;
	_initialSfxVolume = _sfxVolume;
	_initialSpeechVolume = _speechVolume;
}


VolumePanelResult VolumePanel::handleMouseInput(const Common::Point32 &mousePos, bool mouseDown, bool mouseReleased) {
	Common::Point32 effectivePos = mousePos;
	if (mouseDown) {
		_heldMousePos = mousePos;
		_hasHeldMouse = true;
		int hoveredSlider = -1;
		for (int i = 0; i < 3; i++) {
			if (_sliderLabels[i].containsPoint(mousePos)) {
				hoveredSlider = i;
				break;
			}
		}
		if (0 <= hoveredSlider) {
			_activeSlider = hoveredSlider;
			switch (hoveredSlider) {
			case 0:
				_musicSliderX = mousePos.x;
				break;
			case 1:
				_sfxSliderX = mousePos.x;
				break;
			case 2:
				_speechSliderX = mousePos.x;
				break;
			}
		}
	} else {
		int releasedSlider = _activeSlider;
		_activeSlider = -1;
		if (mouseReleased) {
			if (_hasHeldMouse)
				effectivePos = _heldMousePos;
			_hasHeldMouse = false;
			if (releasedSlider < 0) {
				for (int i = 0; i < 3; i++) {
					if (_sliderLabels[i].containsPoint(effectivePos)) {
						releasedSlider = i;
						break;
					}
				}
			}
			if (0 <= releasedSlider) {
				const int newVolume = pixelToVolume(effectivePos.x);
				switch (releasedSlider) {
				case 0:
					setMusicVolume(newVolume);
					break;
				case 1:
					setSfxVolume(newVolume);
					break;
				case 2:
					setSpeechVolume(newVolume);
					break;
				}
				_lastAdjustedSlider = releasedSlider;
				return kVolumePanelChanged;
			}
		}
	}

	if (mouseReleased && _okButton.containsPoint(effectivePos))
		return kVolumePanelApply;
	if (mouseReleased && _noButton.containsPoint(effectivePos))
		return kVolumePanelCancel;
	return kVolumePanelOpen;
}

void VolumePanel::playPreviewSound() {
	const int adjustedSlider = _lastAdjustedSlider;
	_lastAdjustedSlider = -1;
	if (!_soundManager)
		return;

	if (adjustedSlider == 1 && 0 <= _sfxPreviewSoundId) {
		_soundManager->playWithVolume(_sfxPreviewSoundId, _sfxVolume);
	} else if (adjustedSlider == 2 && 0 <= _speechPreviewSoundId) {
		_soundManager->stop(_speechPreviewSoundId);
		_soundManager->playWithVolume(_speechPreviewSoundId, _speechVolume);
	}
}

void VolumePanel::draw(Graphics::ManagedSurface *dst, const Common::Point32 &mousePos, const AlphaBlendLUT &alphaLUT) {
	for (int i = 0; i < 3; i++)
		_sliderLabels[i].drawAndHitTest(dst, mousePos, alphaLUT);
	_okButton.drawAndHitTest(dst, mousePos, alphaLUT);
	_noButton.drawAndHitTest(dst, mousePos, alphaLUT);
}

} // End of namespace Zoombini2
