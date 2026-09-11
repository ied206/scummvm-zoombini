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

#include "image/bmp.h"

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
	// Fill the complete table once.
	// Blended pixels reuse these products for every color channel,
	// avoiding multiplication and division in the inner drawing loops.
	//
	// The shift is deliberate: it produces floor(factor * value / 256),
	// which is the renderer's exact byte-based rule rather than a /255 blend.
	for (int factor = 0; factor < kValueCount; factor++) {
		for (int value = 0; value < kValueCount; value++)
			_values[factor][value] = static_cast<byte>((factor * value) >> 8);
	}
}

BitBlock::BitBlock(Zoombini2Engine *vm) : _vm(vm) {
}

BitBlock::~BitBlock() {
	delete[] _pixels;
	delete[] _alphaMap;
}

/** Load a color BMP and separate alpha BMP into one drawable block. */
bool BitBlock::loadFromColorAlphaBMP(const Common::Path &colorPath, const Common::Path &alphaPath) {
	Common::ScopedPtr<Common::SeekableReadStream> colorStream(_vm->openResourceFile(colorPath.toString('/')));
	if (!colorStream) {
		warning("BitBlock: cannot open color BMP '%s'", colorPath.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &colorFile = *colorStream;

	BitBlock loaded(_vm);
	if (!loaded.loadColorBMP(&colorFile))
		return false;

	Common::ScopedPtr<Common::SeekableReadStream> alphaStream(_vm->openResourceFile(alphaPath.toString('/')));
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
	Common::ScopedPtr<Common::SeekableReadStream> colorStream(_vm->openResourceFile(colorPath.toString('/')));
	if (!colorStream) {
		debug(3, "BitBlock: cannot open BMP '%s'", colorPath.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &colorFile = *colorStream;
	BitBlock loaded(_vm);
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
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->openResourceFile(bbPath.toString('/')));
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

bool BitBlock::loadColorBMP(Common::SeekableReadStream *stream) {
	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(*stream)) {
		warning("BitBlock: failed to decode color BMP");
		return false;
	}

	const Graphics::Surface *surface = decoder.getSurface();
	if (!surface || surface->format != Graphics::PixelFormat::createFormatBGR24()) {
		warning("BitBlock: color BMP is not 24-bit BGR");
		return false;
	}

	const int32 width = surface->w;
	const int32 height = surface->h;
	const uint64 pixelCount64 = static_cast<uint64>(width) * static_cast<uint64>(height);
	if (0x3FFFFFFFU < pixelCount64) {
		warning("BitBlock: BMP dimensions are too large");
		return false;
	}

	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> rgbaSurface(surface->convertTo(Graphics::PixelFormat::createFormatRGBA32()));
	byte *pixels = new byte[static_cast<uint32>(pixelCount64) * 4];
	for (int row = 0; row < height; row++)
		memcpy(pixels + row * width * 4, rgbaSurface->getBasePtr(0, row), width * 4);

	delete[] _pixels;
	delete[] _alphaMap;
	_width = width;
	_height = height;
	_pixels = pixels;
	_alphaMap = nullptr;

	return true;
}

bool BitBlock::loadAlphaBMP(Common::SeekableReadStream *stream) {
	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(*stream))
		return false;

	const Graphics::Surface *surface = decoder.getSurface();
	if (!surface || !surface->format.isCLUT8()) {
		warning("BitBlock: alpha BMP is not indexed");
		return false;
	}

	const int alphaWidth = surface->w;
	const int alphaHeight = surface->h;
	if (alphaWidth != _width || alphaHeight != _height) {
		warning("BitBlock: alpha BMP size %dx%d doesn't match color %dx%d",
				alphaWidth, alphaHeight, _width, _height);
		return false;
	}

	byte *alphaMap = new byte[alphaWidth * alphaHeight];
	for (int row = 0; row < alphaHeight; row++)
		memcpy(alphaMap + row * alphaWidth, surface->getBasePtr(0, row), alphaWidth);

	delete[] _alphaMap;
	_alphaMap = alphaMap;

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

RleBlock::RleBlock(Zoombini2Engine *vm) : _vm(vm) {
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

	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->openResourceFile(resolvedPath.toString('/')));
	if (!stream) {
		debug(3, "RleBlock: cannot open '%s'", resolvedPath.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &f = *stream;
	RleBlock loaded(_vm);
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

Animation::Animation(Zoombini2Engine *vm) : _vm(vm) {
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

	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->openResourceFile(resolvedPath.toString('/')));
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
		RleBlock *frame = new RleBlock(_vm);
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
// ZoombiniAnimation
// ============================================================================

ZoombiniAnimation::Cell::~Cell() {
	for (uint i = 0; i < frames.size(); i++)
		delete frames[i];
}

ZoombiniAnimation::ZoombiniAnimation(Zoombini2Engine *vm) : _vm(vm) {
}

ZoombiniAnimation::~ZoombiniAnimation() {
}

bool ZoombiniAnimation::loadFromFile(const Common::Path &path) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->openResourceFile(path.toString('/')));
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

					RleBlock *frame = new RleBlock(_vm);
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

UIButton::UIButton(Zoombini2Engine *vm)
	: _vm(vm) {
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

	bitmap = new BitBlock(_vm);
	if (bitmap->load(path))
		return true;
	delete bitmap;
	bitmap = nullptr;

	rle = new RleBlock(_vm);
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

	rle = new RleBlock(_vm);
	if (rle->load(colorPath))
		return true;
	delete rle;
	rle = nullptr;

	bitmap = new BitBlock(_vm);
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

BitmapFont::BitmapFont(Zoombini2Engine *vm) : _vm(vm) {
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

	BitBlock alphaBB(_vm);
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

		BitBlock *glyph = new BitBlock(_vm);
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

VolumePanel::VolumePanel(Zoombini2Engine *vm)
	: _vm(vm),
	  _sliderLabels{UIButton(vm), UIButton(vm), UIButton(vm)},
	  _okButton(vm),
	  _noButton(vm) {
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
	_gaugeImage = new RleBlock(_vm);
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
