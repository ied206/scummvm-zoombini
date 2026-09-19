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

#include "graphics/blit.h"
#include "image/bmp.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/page_base.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr Size32 ManagedSurface32::kScreenSize;
constexpr Size32 VolumePanel::kLabelSize;

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

void ManagedSurface32::frameRect(const Common::Rect32 &rect, uint32 color) {
	if (!rect.isValidRect())
		return;

	Common::Rect32 clip32 = rect;
	clip32.clip(w, h);
	if (clip32.isEmpty())
		return;
	Common::Rect clip16 = Common::Rect(clip32.left, clip32.top, clip32.right, clip32.bottom);
	Graphics::ManagedSurface::frameRect(clip16, color);
}

void ManagedSurface32::blitFrom(const ManagedSurface32 &src, const Common::Point32 &destPos) {
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
	const Size32 size(width, height);
	const uint32 dataSize = f.readUint32LE();

	if (f.err() || f.eos() || size.width <= 0 || size.height <= 0) {
		warning("BitBlock: invalid BB header in '%s'", bbPath.toString().c_str());
		return false;
	}

	const uint64 pixelCount64 = static_cast<uint64>(size.width) * static_cast<uint64>(size.height);
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
	_size = size;
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

void BitBlock::createEmpty(const Size32 &size, bool withAlpha) {
	delete[] _pixels;
	delete[] _alphaMap;
	_size = size;
	_pixels = new byte[size.width * size.height * 4]();
	_alphaMap = withAlpha ? new byte[size.width * size.height]() : nullptr;
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

	const Size32 size(surface->w, surface->h);
	const uint64 pixelCount64 = static_cast<uint64>(size.width) * static_cast<uint64>(size.height);
	if (0x3FFFFFFFU < pixelCount64) {
		warning("BitBlock: BMP dimensions are too large");
		return false;
	}

	Common::ScopedPtr<Graphics::Surface, Graphics::SurfaceDeleter> rgbaSurface(surface->convertTo(Graphics::PixelFormat::createFormatRGBA32()));
	byte *pixels = new byte[static_cast<uint32>(pixelCount64) * 4];
	Graphics::copyBlit(pixels, static_cast<const byte *>(rgbaSurface->getPixels()), size.width * 4, rgbaSurface->pitch, size.width, size.height, 4);

	delete[] _pixels;
	delete[] _alphaMap;
	_size = size;
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

	const Size32 alphaSize(surface->w, surface->h);
	if (alphaSize.width != _size.width || alphaSize.height != _size.height) {
		warning("BitBlock: alpha BMP size %dx%d doesn't match color %dx%d",
				alphaSize.width, alphaSize.height, _size.width, _size.height);
		return false;
	}

	byte *alphaMap = new byte[alphaSize.width * alphaSize.height];
	Graphics::copyBlit(alphaMap, static_cast<const byte *>(surface->getPixels()), alphaSize.width, surface->pitch, alphaSize.width, alphaSize.height, 1);

	delete[] _alphaMap;
	_alphaMap = alphaMap;

	return true;
}

void BitBlock::swapData(BitBlock &other) {
	SWAP(_size, other._size);
	SWAP(_pixels, other._pixels);
	SWAP(_alphaMap, other._alphaMap);
}

byte BitBlock::blendChannel(byte src, byte dest, byte mask) {
	// The bitmap supplies the source contribution without further scaling.
	// Only the destination is scaled by the inverse mask, using integer division by 255.
	const int result = src + (dest * (255 - mask)) / 255;
	return static_cast<byte>(MIN(result, 255));
}

void BitBlock::drawToSurface(ManagedSurface32 *destSurface, const Common::Point32 &pos) const {
	drawOpaque(destSurface, pos, Common::Rect32(_size.width, _size.height));
}

void BitBlock::drawSubRect(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect &srcRect) const {
	drawOpaque(destSurface, pos, Common::Rect32(srcRect.left, srcRect.top, srcRect.right, srcRect.bottom));
}

void BitBlock::drawOpaque(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect32 &srcRect) const {
	if (!_pixels || srcRect.isEmpty())
		return;

	const Common::Rect32 source = srcRect.findIntersectingRect(Common::Rect32(_size.width, _size.height));
	if (source.isEmpty())
		return;

	// Widen before translating so off-screen 32-bit positions cannot overflow.
	const int64 left = static_cast<int64>(pos.x) + source.left - srcRect.left;
	const int64 top = static_cast<int64>(pos.y) + source.top - srcRect.top;
	const int64 right = left + source.width();
	const int64 bottom = top + source.height();
	if (right <= 0 || bottom <= 0 || destSurface->w <= left || destSurface->h <= top)
		return;

	const Common::Rect destination(MAX<int64>(0, left), MAX<int64>(0, top),
		MIN<int64>(destSurface->w, right), MIN<int64>(destSurface->h, bottom));
	const int sourceX = source.left + static_cast<int>(destination.left - left);
	const int sourceY = source.top + static_cast<int>(destination.top - top);
	const uint sourcePitch = static_cast<uint>(_size.width) * 4;
	const byte *sourcePixels = _pixels + static_cast<size_t>(sourceY) * sourcePitch + static_cast<size_t>(sourceX) * 4;
	byte *destinationPixels = static_cast<byte *>(destSurface->getBasePtr(destination.left, destination.top));
	// Ignore the stored fourth byte, including zero-filled bitmaps, and produce opaque output.
	static constexpr Graphics::PixelFormat kSourceFormat = Graphics::PixelFormat::createFormatRGBA32(false);
	if (!Graphics::crossBlit(destinationPixels, sourcePixels, destSurface->pitch, sourcePitch,
			destination.width(), destination.height(), destSurface->format, kSourceFormat)) {
		warning("BitBlock: unsupported opaque pixel conversion");
		return;
	}
	destSurface->addDirtyRect(destination);
}

/**
 * Draw with the separate-mask bitmap blend rule.
 *
 * @ref Graphics::alphaMaskBlit is the usual shared primitive for a separate alpha mask.
 * It scales the source by opacity, whereas this path adds the supplied source unchanged:
 * `min(255, source + floor(destination * (255 - mask) / 255))`.
 * Keep the custom blend to preserve that source contribution, saturation, and /255 rounding.
 * Zero masks leave the destination untouched; other masks produce opaque output.
 */
void BitBlock::drawAlphaBlend(ManagedSurface32 *dst, const Common::Point32 &pos) const {
	if (!_pixels || !_alphaMap)
		return;

	for (int row = 0; row < _size.height; row++) {
		int dy = pos.y + row;
		if (dy < 0 || dst->h <= dy)
			continue;

		for (int col = 0; col < _size.width; col++) {
			int dx = pos.x + col;
			if (dx < 0 || dst->w <= dx)
				continue;

			byte alpha = _alphaMap[row * _size.width + col];
			if (alpha == 0)
				continue;

			const byte *src = _pixels + (row * _size.width + col) * 4;
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
 * @ref Graphics::alphaMaskBlit would normally handle this unpremultiplied bitmap and mask.
 * Its partial-opacity path modulates the mask and rounds the combined weighted sum once.
 * This path instead keeps the mask unchanged and rounds each /256 product separately:
 * `floor(source * mask / 256) + floor(destination * (255 - mask) / 256)`.
 * The rounding order can change a channel by one, so retain @ref AlphaBlendLUT for exact pixels.
 * Zero masks skip the pixel, full masks copy RGB exactly, and every written pixel has alpha 255.
 */
void BitBlock::drawRleMaskBlend(ManagedSurface32 *dst, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const {
	if (!_pixels || !_alphaMap)
		return;

	for (int row = 0; row < _size.height; row++) {
		const int destY = pos.y + row;
		if (destY < 0 || dst->h <= destY)
			continue;
		for (int column = 0; column < _size.width; column++) {
			const int destX = pos.x + column;
			if (destX < 0 || dst->w <= destX)
				continue;

			const byte mask = _alphaMap[row * _size.width + column];
			if (mask == 0)
				continue;
			const byte *src = _pixels + (row * _size.width + column) * 4;
			byte *destPtr = static_cast<byte *>(dst->getBasePtr(destX, destY));
			if (mask == 255) {
				destPtr[0] = src[2];
				destPtr[1] = src[1];
				destPtr[2] = src[0];
			} else {
				// Round the source and destination contributions separately through the LUT.
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
	SWAP(_size, other._size);
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
	const Size32 size(width, height);
	const int32 field20 = stream->readSint32LE();
	const uint32 externalSize = stream->readUint32LE();

	if (stream->err() || headerSize != externalSize || size.width <= 0 || size.height <= 0 || headerSize < 2) {
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
	_size = size;
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
		warning("RleBlock: ignoring %" PRIu64 " trailing bytes in '%s'", f.size() - f.pos(), resolvedPath.toString().c_str());
	swapData(loaded);
	return true;
}

bool RleBlock::load(const Common::Path &basePath) {
	return loadFromFile(basePath);
}

bool RleBlock::loadAnimationFrame(Common::SeekableReadStream *stream, uint32 outerSize) {
	/* uint32 effectiveHeight = */ stream->readUint32LE();
	/* uint32 dataPlaceholder = */ stream->readUint32LE();
	const uint32 headerSize = stream->readUint32LE();
	const int32 width = stream->readSint32LE();
	const int32 height = stream->readSint32LE();
	const Size32 size(width, height);
	const int32 field20 = stream->readSint32LE();

	if (stream->err() || headerSize != outerSize || size.width <= 0 || size.height <= 0 || headerSize < 2) {
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
	_size = size;
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
	// @ref Graphics::alphaBlit normally handles per-pixel opacity, but expects straight RGB and forward alpha.
	// Mode-1 RLE stores premultiplied BGR and inverse alpha, so that blitter would scale the source again.
	// Keep `min(255, source + floor(destination * inverseAlpha / 256))` via @ref AlphaBlendLUT.
	// Unpremultiplying first would lose integer precision and would not preserve the saturated sum.
	const int destPart = alphaLUT.scale(invAlpha, dest);
	return static_cast<byte>(MIN(static_cast<int>(src) + destPart, 255));
}

/**
 * Draw the RLE spans with opaque copying or lookup-table alpha blending.
 *
 * Span data begins after the two-byte resource prefix.
 */
void RleBlock::drawToScreen(ManagedSurface32 *destSurface, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const {
	drawToScreenClipped(destSurface, pos, Common::Rect32(destSurface->w, destSurface->h), alphaLUT);
}

void RleBlock::drawToScreenClipped(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect32 &clip, const AlphaBlendLUT &alphaLUT) const {
	if (!_rleData || _dataSize < 2 || !clip.isValidRect())
		return;

	const Common::Rect32 destinationClip = clip.findIntersectingRect(Common::Rect32(destSurface->w, destSurface->h));
	if (destinationClip.isEmpty())
		return;
	assert(destSurface->format.bytesPerPixel == 4);

	const byte *ptr = _rleData + 2;
	const byte *end = _rleData + _dataSize;

	while (ptr < end) {
		if (end - ptr < 7)
			break;

		const int16 xOff = READ_LE_INT16(ptr);
		const int16 yOff = READ_LE_INT16(ptr + 2);
		const int16 pixelCount = READ_LE_INT16(ptr + 4);
		const byte mode = ptr[6];
		ptr += 7;
		if (pixelCount < 0 || 1 < mode || static_cast<uint32>(end - ptr) < static_cast<uint32>(pixelCount) * 4)
			return;

		// Widen before translating so off-screen 32-bit positions cannot overflow.
		const int64 screenX = static_cast<int64>(pos.x) + xOff;
		const int64 screenY = static_cast<int64>(pos.y) + yOff;
		if (screenY < destinationClip.top || destinationClip.bottom <= screenY) {
			ptr += pixelCount * 4;
			continue;
		}

		const int64 left = MAX<int64>(screenX, destinationClip.left);
		const int64 right = MIN<int64>(screenX + pixelCount, destinationClip.right);
		if (left < right) {
			const int x = static_cast<int>(left);
			const int y = static_cast<int>(screenY);
			const int count = static_cast<int>(right - left);
			const int startCol = static_cast<int>(left - screenX);
			const byte *srcPixel = ptr + startCol * 4;
			byte *dstPixel = static_cast<byte *>(destSurface->getBasePtr(x, y));

			if (mode == 0) {
				// Copy potentially unaligned payload bytes before applying typed alpha writes in place.
				// Preserve raw BGRA byte order independently of the destination format.
				static constexpr Graphics::PixelFormat kByteFormat = Graphics::PixelFormat::createFormatBGRA32();
				destSurface->copyRectToSurface(srcPixel, count * 4, x, y, count, 1);
				Graphics::setAlpha(dstPixel, dstPixel, destSurface->pitch, destSurface->pitch, count, 1, kByteFormat, false, 255);
			} else {
				// Mode 1 stores premultiplied BGR followed by inverse alpha.
				for (int i = 0; i < count; i++) {
					const byte invAlpha = srcPixel[3];
					dstPixel[0] = blendChannel(srcPixel[0], dstPixel[0], invAlpha, alphaLUT);
					dstPixel[1] = blendChannel(srcPixel[1], dstPixel[1], invAlpha, alphaLUT);
					dstPixel[2] = blendChannel(srcPixel[2], dstPixel[2], invAlpha, alphaLUT);
					dstPixel[3] = 255;
					srcPixel += 4;
					dstPixel += 4;
				}
				destSurface->addDirtyRect(Common::Rect(x, y, x + count, y + 1));
			}
		}
		ptr += pixelCount * 4;
	}
}

constexpr const char *Gfx::kTextFontPath;

Gfx::Gfx(Zoombini2Engine *vm) : _vm(vm), _pageLayerStack(new PageLayerStack(vm)) {
}

Gfx::~Gfx() {
	delete _background;
	delete _pageLayerStack;
	delete _nameBoxSprite;
	delete _textFont;
}

void Gfx::clearPageLayers() {
	if (_pageLayerStack)
		_pageLayerStack->clear();
}

ManagedSurface32 *Gfx::createSurface(const Size32 &size) const {
	return new ManagedSurface32(size, _vm->getScreen()->format);
}

void Gfx::captureScreen(ManagedSurface32 *destSurface) const {
	assert(destSurface != nullptr);
	destSurface->copyFrom(*_vm->getScreen());
}

void Gfx::copyToScreen(const ManagedSurface32 &srcSurface) const {
	_vm->getScreen()->copyFrom(srcSurface);
}

void Gfx::captureScreenRegion(ManagedSurface32 *destSurface, const Common::Rect &srcRect) const {
	assert(destSurface != nullptr);
	destSurface->copyRectToSurface(*_vm->getScreen(), 0, 0, srcRect);
}

void Gfx::copyRegionToScreen(const ManagedSurface32 &srcSurface, const Common::Point &destSurface) const {
	_vm->getScreen()->copyRectToSurface(srcSurface, destSurface.x, destSurface.y, Common::Rect(srcSurface.w, srcSurface.h));
}

void Gfx::drawBitBlock(ManagedSurface32 *destSurface, const BitBlock *bitmap, const Common::Point32 &pos) const {
	if (destSurface && bitmap)
		bitmap->drawToSurface(destSurface, pos);
}

void Gfx::drawBitBlockSubRect(ManagedSurface32 *destSurface, const BitBlock *bitmap, const Common::Point32 &pos, const Common::Rect &srcRect) const {
	if (destSurface && bitmap)
		bitmap->drawSubRect(destSurface, pos, srcRect);
}

bool Gfx::loadBackground(const Common::Path &path) {
	BitBlock *background = new BitBlock(_vm);
	if (!background->load(path)) {
		delete background;
		return false;
	}

	delete _background;
	_background = background;
	return true;
}

void Gfx::clearBackground() {
	delete _background;
	_background = nullptr;
}

void Gfx::drawBackground(ManagedSurface32 *destSurface, const Common::Point32 &pos) const {
	if (destSurface && _background)
		_background->drawToSurface(destSurface, pos);
}

void Gfx::drawBackgroundSubRect(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect &srcRect) const {
	if (destSurface && _background)
		_background->drawSubRect(destSurface, pos, srcRect);
}

void Gfx::drawRleBlock(ManagedSurface32 *destSurface, const RleBlock *sprite, const Common::Point32 &pos) const {
	if (destSurface && sprite)
		sprite->drawToScreen(destSurface, pos, _vm->getAlphaLUT());
}

void Gfx::drawAnimationFrame(ManagedSurface32 *destSurface, const Animation *animation, int frameIndex, const Common::Point32 &pos) const {
	if (!destSurface || !animation)
		return;

	drawRleBlock(destSurface, animation->getFrame(frameIndex), pos);
}

void Gfx::drawAndUpdateAnimationRunner(ManagedSurface32 *destSurface, AnimationRunner *runner, uint32 tickCount, int scrollX, int backgroundWidth) const {
	if (destSurface && runner)
		runner->drawAndUpdate(destSurface, _vm->getAlphaLUT(), tickCount, scrollX, backgroundWidth);
}

void Gfx::drawZoombini(ManagedSurface32 *screen, const ZoombiniAnimation *animation, const ZmbTrait &traits,
					   const Common::Point32 &pos, int cell, int frame, const Common::Rect32 *clip) const {
	drawZoombini(screen, animation, traits, pos, cell, frame, _vm->getAlphaLUT(), clip);
}

void Gfx::drawZoombini(ManagedSurface32 *screen, const ZoombiniAnimation *animation, const ZmbTrait &traits,
					   const Common::Point32 &pos, int cell, int frame, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip) {
	if (!screen || !animation || cell < 0 || ZoombiniAnimation::kDim0 <= cell || frame < 0)
		return;
	const int baseIndex = cell * ZoombiniAnimation::kDim1 * ZoombiniAnimation::kDim2;
	for (int layer = 0; layer < ZoombiniAnimation::kDim1; layer++) {
		int variant = 0;
		if (0 < layer)
			variant = traits.getValue(static_cast<ZmbTrait::TraitIndex>(layer - 1));
		if (variant < 0 || ZoombiniAnimation::kDim2 <= variant)
			continue;
		const int entry = baseIndex + layer * ZoombiniAnimation::kDim2 + variant;
		const int selectedFrame = animation->getFrameCount(entry) == 1 ? 0 : frame;
		const RleBlock *sprite = animation->getFrame(entry, selectedFrame);
		if (!sprite)
			continue;
		if (clip)
			sprite->drawToScreenClipped(screen, pos, *clip, alphaLUT);
		else
			sprite->drawToScreen(screen, pos, alphaLUT);
	}
}

void Gfx::drawZoombiniPreview(ManagedSurface32 *screen, const ZoombiniAnimation *animation,
							const int (&selectedValues)[ZmbTrait::kTraitCount], const Common::Point32 &pos) const {
	drawZoombiniPreview(screen, animation, selectedValues, pos, _vm->getAlphaLUT());
}

void Gfx::drawZoombiniPreview(ManagedSurface32 *screen, const ZoombiniAnimation *animation,
							const int (&selectedValues)[ZmbTrait::kTraitCount], const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) {
	if (!screen || !animation)
		return;
	static constexpr int kBaseCell = 990;
	static constexpr int kFeatureCellBases[ZmbTrait::kTraitCount] = {996, 1002, 1008, 1014};
	static constexpr int kFeatureDrawOrder[ZmbTrait::kTraitCount] = {0, 2, 3, 1};
	const RleBlock *frame = animation->getFrame(kBaseCell, 0);
	if (frame)
		frame->drawToScreen(screen, pos, alphaLUT);
	for (int i = 0; i < ZmbTrait::kTraitCount; i++) {
		const int feature = kFeatureDrawOrder[i];
		const int value = selectedValues[feature];
		if (1 <= value && value <= ZmbTrait::kTraitValueCount) {
			frame = animation->getFrame(kFeatureCellBases[feature] + value, 0);
			if (frame)
				frame->drawToScreen(screen, pos, alphaLUT);
		}
	}
}

void Gfx::drawZoombiniRunner(ManagedSurface32 *screen, const ZoombiniRunner *runner, const Common::Rect32 *clip,
							 int scrollX, int backgroundWidth, const RleBlock *dropTargetIndicator) const {
	drawZoombiniRunner(screen, runner, _vm->getAlphaLUT(), clip, scrollX, backgroundWidth, dropTargetIndicator);
}

void Gfx::drawZoombiniRunner(ManagedSurface32 *screen, const ZoombiniRunner *runner, const AlphaBlendLUT &alphaLUT,
							 const Common::Rect32 *clip, int scrollX, int backgroundWidth, const RleBlock *dropTargetIndicator) {
	if (!screen || !runner || !runner->_activeAnimation || runner->_hidden)
		return;
	const Common::Rect32 spriteRect = runner->getSpriteRect(scrollX, backgroundWidth);
	if (spriteRect.right <= 0 || spriteRect.bottom <= 0 || screen->w <= spriteRect.left || screen->h <= spriteRect.top)
		return;
	const Common::Point32 drawPos = runner->getDrawPosition(scrollX, backgroundWidth);
	if (runner->_dragging && runner->_hoveredDropTargetIndex != -1 && dropTargetIndicator)
		dropTargetIndicator->drawToScreen(screen, drawPos, alphaLUT);
	const int frame = runner->_animationActive ? runner->_animationFrame : 0;
	drawZoombini(screen, runner->_activeAnimation, runner->_traits, drawPos, runner->_animationCell, frame, alphaLUT, clip);
}

void Gfx::textColorRGB(TextColor color, byte &red, byte &green, byte &blue) {
	static constexpr byte kColors[4][3] = {
		{16, 16, 16},
		{0, 0, 255},
		{0, 255, 0},
		{255, 255, 255},
	};
	const int index = static_cast<int>(color);
	red = kColors[index][0];
	green = kColors[index][1];
	blue = kColors[index][2];
}

bool Gfx::loadTextFont(TextColor color) {
	const int index = static_cast<int>(color);
	if (index < 0 || 4 <= index)
		return false;
	if (hasTextFont(color))
		return true;
	if (!_textFont)
		_textFont = new BitmapFont(_vm);
	return _textFont->load(Common::Path(kTextFontPath));
}

bool Gfx::hasTextFont(TextColor color) const {
	const int index = static_cast<int>(color);
	return 0 <= index && index < 4 && _textFont && _textFont->isLoaded();
}

int Gfx::drawText(ManagedSurface32 *destSurface, TextColor color, const Common::Point32 &pos, const Common::String &text) const {
	if (!destSurface || !hasTextFont(color))
		return 0;
	byte red, green, blue;
	textColorRGB(color, red, green, blue);
	return _textFont->drawString(destSurface, pos, text, red, green, blue, _vm->getAlphaLUT());
}

int Gfx::getTextWidth(const Common::String &text, TextColor color) const {
	if (!hasTextFont(color))
		return 0;
	return _textFont->getStringWidth(text);
}

void Gfx::drawDragNameTooltip(ManagedSurface32 *destSurface, const Common::String &name) {
	if (!destSurface)
		return;
	if (!_nameBoxSprite) {
		_nameBoxSprite = _vm->loadRleBlock("bmp/menu/name_box.rb");
		if (!_nameBoxSprite)
			return;
	}
	if (!loadTextFont(TextColor::kDark00))
		return;
	static constexpr int kPlateX = 310;
	static constexpr int kPlateY = 565;
	static constexpr int kTextCenterX = 400;
	static constexpr int kTextY = 570;
	drawRleBlock(destSurface, _nameBoxSprite, Common::Point32(kPlateX, kPlateY));
	const int width = _textFont->getStringWidth(name);
	byte red, green, blue;
	textColorRGB(TextColor::kDark00, red, green, blue);
	_textFont->drawString(destSurface, Common::Point32(kTextCenterX - width / 2, kTextY), name, red, green, blue, _vm->getAlphaLUT());
}

void Gfx::maskRejectedArea(ManagedSurface32 *destSurface, const AreaMask *areaMask) {
	if (!destSurface)
		return;
	if (!areaMask) {
		fillRect(destSurface, Common::Rect32(destSurface->w, destSurface->h), 0);
		return;
	}
	const int width = MIN<int>(destSurface->w, 800);
	const int height = MIN<int>(destSurface->h, 600);
	for (int y = 0; y < height; y++) {
		int runStart = -1;
		for (int groupX = 0; groupX < width; groupX += 8) {
			if (!areaMask->hasMarkedByteAt(Common::Point32(groupX, y))) {
				if (runStart < 0)
					runStart = groupX;
			} else if (0 <= runStart) {
				fillRect(destSurface, Common::Rect32(runStart, y, groupX, y + 1), 0);
				runStart = -1;
			}
		}
		if (0 <= runStart)
			fillRect(destSurface, Common::Rect32(runStart, y, width, y + 1), 0);
	}
}

void Gfx::fillRect(ManagedSurface32 *destSurface, const Common::Rect32 &rect, uint32 color) const {
	if (destSurface)
		destSurface->fillRect(rect, color);
}

void Gfx::fillRect(ManagedSurface32 *destSurface, const Common::Rect &rect, uint32 color) const {
	if (destSurface)
		destSurface->fillRect(rect, color);
}

void Gfx::frameRect(ManagedSurface32 *destSurface, const Common::Rect32 &rect, uint32 color) const {
	if (destSurface)
		destSurface->frameRect(rect, color);
}

void Gfx::frameRect(ManagedSurface32 *destSurface, const Common::Rect &rect, uint32 color) const {
	if (destSurface)
		destSurface->frameRect(rect, color);
}

void Gfx::drawLine(ManagedSurface32 *destSurface, const Common::Point32 &start, const Common::Point32 &end, uint32 color) const {
	if (destSurface)
		destSurface->drawLine(start.x, start.y, end.x, end.y, color);
}

ManagedSurface32 *Gfx::createMapTransitionBackground(PageId sourcePage, int mapRegion, RouteBranch routeBranch) {
	ManagedSurface32 *background = createSurface(ManagedSurface32::kScreenSize);

	const Common::String backgroundPath = Common::String::format("#bmp/maptrans/bigmap_background_%d", mapRegion);
	BitBlock bitmap(_vm);
	if (bitmap.load(Common::Path(backgroundPath))) {
		drawBitBlock(background, &bitmap, Common::Point32(0, 0));
	} else {
		warning("MapTransition: Failed to load background %s", backgroundPath.c_str());
		fillRect(background, Common::Rect32(0, 0, ManagedSurface32::kScreenSize.width, ManagedSurface32::kScreenSize.height), 0);
	}

	drawMapOverlays(background, sourcePage, mapRegion, routeBranch);
	return background;
}

/** Load, draw, and release one cached map-overlay RLE sprite. */
void Gfx::drawOverlaySprite(ManagedSurface32 *dst, const Common::String &name, const Common::Point32 &pos) {
	const Common::Path overlayPath(Common::String::format("bmp/maptrans/%s.bmp", name.c_str()));

	RleBlock overlay(_vm);
	if (overlay.load(overlayPath)) {
		drawRleBlock(dst, &overlay, pos);
	} else {
		debug(2, "MapTransition: overlay '%s' not found", name.c_str());
	}
}

/**
 * Draw map overlay segments and icons based on visited-page state.
 *
 * Key pattern for each overlay:
 *   Draw if dstPageId was already visited during the current game,
 *   or if the current transition starts at srcPageId and that page is visited
 *   but dstPageId has not been reached at this level yet.
 *
 * Route direction at the page-4 fork is tracked through @ref GameState::hasPageVisit.
 * Page 6 visit kind 1 selects the upper path and page 5 selects the lower path.
 */
void Gfx::drawMapOverlays(ManagedSurface32 *dst, PageId sourcePage, int mapRegion, RouteBranch routeBranch) {
	GameState *gs = _vm->getGameState();

	// Helper lambda: standard overlay visibility check.
	// "Show this path piece if destSurface was previously visited,
	// OR if we're currently transitioning and haven't arrived yet."
	auto visible = [&](int srcPageId, int dstPageId) -> bool {
		return gs->isPageVisited(dstPageId) || (sourcePage == srcPageId && gs->isPageVisited(srcPageId) && !gs->hasPageVisit(dstPageId, 1));
	};

	switch (mapRegion) {
	case 1: {
		// Map region one covers ShelterZombiniville through Rescue Site I.
		const bool seg01vis = visible(0, 1);
		const bool seg02vis = visible(1, 2);
		const bool seg03vis = visible(2, 3);
		const bool seg04vis = visible(3, 4);

		if (seg01vis)
			drawOverlaySprite(dst, "bigmap_segment_01", Common::Point32(264, 206));
		if (seg02vis)
			drawOverlaySprite(dst, "bigmap_segment_02", Common::Point32(369, 94));
		if (seg03vis)
			drawOverlaySprite(dst, "bigmap_segment_03", Common::Point32(520, 74));
		if (seg04vis)
			drawOverlaySprite(dst, "bigmap_segment_03", Common::Point32(632, 156));

		// Icons
		if (seg01vis) {
			drawOverlaySprite(dst, "bigmap_icon_01", Common::Point32(259, 302));
			drawOverlaySprite(dst, "bigmap_icon_02", Common::Point32(281, 131));
		}
		if (seg02vis)
			drawOverlaySprite(dst, "bigmap_icon_03", Common::Point32(421, 26));
		if (seg03vis)
			drawOverlaySprite(dst, "bigmap_icon_04", Common::Point32(564, 99));
		if (seg04vis)
			drawOverlaySprite(dst, "bigmap_icon_05", Common::Point32(653, 191));
		break;
	}

	case 2: {
		// Map region two covers both routes between the rescue sites.
		const bool northRoute = gs->hasPageVisit(6, 1) || routeBranch == RouteBranch::kLeft01;
		const bool southRoute = gs->hasPageVisit(5, 1) || routeBranch == RouteBranch::kRight02;

		// Unconditional: start of route from Rescue1
		drawOverlaySprite(dst, "bigmap_segment_03", Common::Point32(-18, 198));
		drawOverlaySprite(dst, "bigmap_segment_04", Common::Point32(99, 280));

		// Top path: fork, Magic Wall, Chez Norf, then Rescue Site II.
		if (northRoute && visible(4, 6))
			drawOverlaySprite(dst, "bigmap_segment_05a", Common::Point32(201, 260));

		// Magic Wall to Chez Norf segment.
		const bool czNorfSeg = gs->isPageVisited(8) || (sourcePage == 6 && gs->isPageVisited(6) && !gs->hasPageVisit(8, 1));
		if (czNorfSeg)
			drawOverlaySprite(dst, "bigmap_segment_06a", Common::Point32(310, 230));

		// Chez Norf to Rescue Site II segment.
		if (gs->hasPageVisit(8, 1)) {
			if (gs->isPageVisited(9) || (sourcePage == 8 && gs->isPageVisited(8) && !gs->hasPageVisit(9, 1)))
				drawOverlaySprite(dst, "bigmap_segment_07a", Common::Point32(480, 233));
		}

		// Bottom path: fork, Mystic Marsh, Wall of Fleens, then Rescue Site II.
		if (southRoute && visible(4, 5))
			drawOverlaySprite(dst, "bigmap_segment_05b", Common::Point32(164, 376));

		// Mystic Marsh to Wall of Fleens segment.
		const bool wofSeg = gs->isPageVisited(7) || (sourcePage == 5 && gs->isPageVisited(5) && !gs->hasPageVisit(7, 1));
		if (wofSeg)
			drawOverlaySprite(dst, "bigmap_segment_06b", Common::Point32(339, 476));

		// Wall of Fleens to Rescue Site II segment.
		if (gs->hasPageVisit(7, 1)) {
			if (gs->isPageVisited(9) || (sourcePage == 7 && gs->isPageVisited(7) && !gs->hasPageVisit(9, 1)))
				drawOverlaySprite(dst, "bigmap_segment_07b", Common::Point32(512, 360));
		}

		// These route icons are always visible in region two.
		drawOverlaySprite(dst, "bigmap_icon_04", Common::Point32(27, 223));
		drawOverlaySprite(dst, "bigmap_icon_05", Common::Point32(116, 315));

		// Top route icons
		if (northRoute && visible(4, 6))
			drawOverlaySprite(dst, "bigmap_icon_06a", Common::Point32(259, 210));
		if (czNorfSeg)
			drawOverlaySprite(dst, "bigmap_icon_07a", Common::Point32(440, 170));

		// Rescue2 icon (reachable from either path)
		if (gs->isPageVisited(9) ||
			(sourcePage == 8 && gs->isPageVisited(8) && !gs->hasPageVisit(9, 1)) ||
			(sourcePage == 7 && gs->isPageVisited(7) && !gs->hasPageVisit(9, 1)))
			drawOverlaySprite(dst, "bigmap_icon_08", Common::Point32(476, 271));

		// Bottom route icons
		if (southRoute && visible(4, 5))
			drawOverlaySprite(dst, "bigmap_icon_06b", Common::Point32(290, 418));
		if (visible(5, 7))
			drawOverlaySprite(dst, "bigmap_icon_07b", Common::Point32(443, 430));
		break;
	}

	case 3: {
		// Map region three covers Rescue Site II through the finale.
		const bool northRouteVisited = gs->hasPageVisit(6, 1);
		const bool czNorfFlag = gs->hasPageVisit(8, 1);
		const bool wofFlag = gs->hasPageVisit(7, 1);

		// Previous route segments (show which path was taken)
		if (northRouteVisited) {
			drawOverlaySprite(dst, "bigmap_segment_05a", Common::Point32(-27, 418));
			drawOverlaySprite(dst, "bigmap_segment_06a", Common::Point32(87, 386));
		}
		if (czNorfFlag)
			drawOverlaySprite(dst, "bigmap_segment_07a", Common::Point32(251, 383));
		if (wofFlag)
			drawOverlaySprite(dst, "bigmap_segment_07b", Common::Point32(293, 515));

		// Rescue Site II to Snowboard Gulch is always visible.
		drawOverlaySprite(dst, "bigmap_segment_08", Common::Point32(313, 407));

		// Snowboard Gulch to Boolie Boggle.
		if (visible(10, 11))
			drawOverlaySprite(dst, "bigmap_segment_09", Common::Point32(434, 328));
		// Boolie Boggle to the finale.
		if (visible(11, 12))
			drawOverlaySprite(dst, "bigmap_segment_10", Common::Point32(527, 111));

		// Prior-route icons remain conditional on saved progress.
		if (northRouteVisited)
			drawOverlaySprite(dst, "bigmap_icon_06a", Common::Point32(33, 366));
		if (czNorfFlag)
			drawOverlaySprite(dst, "bigmap_icon_07a", Common::Point32(214, 326));

		// Unconditional icons
		drawOverlaySprite(dst, "bigmap_icon_08", Common::Point32(252, 426));
		drawOverlaySprite(dst, "bigmap_icon_07b", Common::Point32(66, 574));
		drawOverlaySprite(dst, "bigmap_icon_09", Common::Point32(367, 367));

		// Conditional icons
		if (visible(10, 11))
			drawOverlaySprite(dst, "bigmap_icon_10", Common::Point32(469, 273));
		if (visible(11, 12))
			drawOverlaySprite(dst, "bigmap_icon_11", Common::Point32(608, -12));
		break;
	}

	default:
		// Use the first map region as a safe fallback.
		drawOverlaySprite(dst, "bigmap_segment_01", Common::Point32(264, 206));
		drawOverlaySprite(dst, "bigmap_icon_01", Common::Point32(259, 302));
		drawOverlaySprite(dst, "bigmap_icon_02", Common::Point32(281, 131));
		break;
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
// AreaMask
// ============================================================================

AreaMask::AreaMask(Zoombini2Engine *vm) : _vm(vm) {
}

bool AreaMask::loadOneBitBitmap(Common::SeekableReadStream &stream, const Common::Path &path) {
	stream.seek(0);
	if (stream.size() < 62)
		return false;
	byte header[62];
	if (stream.read(header, sizeof(header)) != sizeof(header))
		return false;
	if (header[0] != 'B' || header[1] != 'M')
		return false;
	const uint32 pixelOffset = static_cast<uint32>(header[10]) | (static_cast<uint32>(header[11]) << 8) |
							   (static_cast<uint32>(header[12]) << 16) | (static_cast<uint32>(header[13]) << 24);
	const int32 width = static_cast<int32>(static_cast<uint32>(header[18]) | (static_cast<uint32>(header[19]) << 8) |
										   (static_cast<uint32>(header[20]) << 16) | (static_cast<uint32>(header[21]) << 24));
	int32 height = static_cast<int32>(static_cast<uint32>(header[22]) | (static_cast<uint32>(header[23]) << 8) |
									  (static_cast<uint32>(header[24]) << 16) | (static_cast<uint32>(header[25]) << 24));
	const uint32 bitsPerPixel = static_cast<uint32>(header[28]) | (static_cast<uint32>(header[29]) << 8);
	const uint32 compression = static_cast<uint32>(header[30]) | (static_cast<uint32>(header[31]) << 8) |
							   (static_cast<uint32>(header[32]) << 16) | (static_cast<uint32>(header[33]) << 24);
	if (bitsPerPixel != 1 || compression != 0 || width <= 0 || height == 0) {
		stream.seek(0);
		return false;
	}
	bool bottomUp = true;
	if (height < 0) {
		bottomUp = false;
		height = -height;
	}
	const uint64 pixelCount = static_cast<uint64>(width) * static_cast<uint64>(height);
	if (0xFFFFFFFFU < pixelCount)
		return false;
	const uint32 stride = (static_cast<uint32>(width) + 31) / 32 * 4;
	if (static_cast<uint64>(pixelOffset) + static_cast<uint64>(stride) * static_cast<uint64>(height) > static_cast<uint64>(stream.size())) {
		warning("AreaMask: truncated 1-bit bitmap '%s'", path.toString().c_str());
		stream.seek(0);
		return false;
	}
	Common::Array<byte> pixels;
	pixels.resize(static_cast<uint32>(pixelCount));
	for (int32 row = 0; row < height; row++) {
		const int32 sourceRow = bottomUp ? height - 1 - row : row;
		stream.seek(static_cast<int64>(pixelOffset) + static_cast<int64>(sourceRow) * stride);
		byte packed = 0;
		for (int32 col = 0; col < width; col++) {
			if (col % 8 == 0)
				packed = stream.readByte();
			pixels[static_cast<uint32>(row) * static_cast<uint32>(width) + static_cast<uint32>(col)] =
				static_cast<byte>((packed >> (7 - (col % 8))) & 1);
		}
	}
	_size = Size32(width, height);
	_pixels.swap(pixels);
	uint32 markedCount = 0;
	for (uint32 i = 0; i < static_cast<uint32>(pixelCount); i++) {
		if (_pixels[i])
			markedCount += 1;
	}
	debug(1, "AreaMask: decoded 1-bit mask '%s' as %dx%d with %u marked pixels", path.toString().c_str(), width, height, markedCount);
	return true;
}

bool AreaMask::loadFromFile(const Common::Path &path) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->openResourceFile(path.toString('/')));
	if (!stream) {
		warning("AreaMask: cannot open '%s'", path.toString().c_str());
		return false;
	}

	if (loadOneBitBitmap(*stream, path))
		return true;
	stream->seek(0);

	Image::BitmapDecoder decoder;
	if (!decoder.loadStream(*stream)) {
		warning("AreaMask: failed to decode '%s'", path.toString().c_str());
		return false;
	}
	const Graphics::Surface *surface = decoder.getSurface();
	if (!surface || !surface->format.isCLUT8() || surface->w <= 0 || surface->h <= 0) {
		warning("AreaMask: '%s' is not a valid indexed bitmap", path.toString().c_str());
		return false;
	}

	const Size32 size(surface->w, surface->h);
	const uint64 pixelCount = static_cast<uint64>(size.width) * static_cast<uint64>(size.height);
	if (0xFFFFFFFFU < pixelCount) {
		warning("AreaMask: dimensions are too large in '%s'", path.toString().c_str());
		return false;
	}

	Common::Array<byte> pixels;
	pixels.resize(static_cast<uint32>(pixelCount));
	Graphics::copyBlit(&pixels[0], static_cast<const byte *>(surface->getPixels()), size.width, surface->pitch, size.width, size.height, 1);

	_size = size;
	_pixels.swap(pixels);
	return true;
}

uint32 AreaMask::countMarkedPixels() const {
	uint32 markedCount = 0;
	for (uint i = 0; i < _pixels.size(); i++) {
		if (_pixels[i] != 0)
			markedCount += 1;
	}
	return markedCount;
}

bool AreaMask::hasMarkedByteAt(const Common::Point32 &point) const {
	if (point.x <= 0 || point.y <= 0 || _size.width <= point.x || _size.height <= point.y || _pixels.empty())
		return false;

	const int byteStartX = point.x & ~7;
	const int byteEndX = MIN(byteStartX + 8, _size.width);
	for (int x = byteStartX; x < byteEndX; x++) {
		if (_pixels[point.y * _size.width + x] != 0)
			return true;
	}
	return false;
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
					if (!frame->loadAnimationFrame(&f, outerSize)) {
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

Size32 ZoombiniAnimation::getSpriteSize(int cell, int frame) const {
	if (cell < 0 || kDim0 <= cell || frame < 0)
		return Size32();
	const int entry = cell * kDim1 * kDim2;
	const int selectedFrame = getFrameCount(entry) == 1 ? 0 : frame;
	const RleBlock *sprite = getFrame(entry, selectedFrame);
	if (!sprite)
		return Size32();
	return sprite->getSize();
}

void ZoombiniAnimation::drawZoombini(ManagedSurface32 *screen, const ZmbTrait &traits, const Common::Point32 &pos,
									 int cell, int frame, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip) const {
	Gfx::drawZoombini(screen, this, traits, pos, cell, frame, alphaLUT, clip);
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
			const Size32 size = _normalBB->getSize();
			_rect.setWidth(size.width);
			_rect.setHeight(size.height);
		} else if (_normalRle) {
			const Size32 size = _normalRle->getSize();
			_rect.setWidth(size.width);
			_rect.setHeight(size.height);
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
			const Size32 size = _normalRle->getSize();
			_rect.setWidth(size.width);
			_rect.setHeight(size.height);
		} else if (_normalBB) {
			const Size32 size = _normalBB->getSize();
			_rect.setWidth(size.width);
			_rect.setHeight(size.height);
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

void UIButton::setRect(const Common::Point32 &pos, const Size32 &size) {
	_rect = Common::Rect(pos.x, pos.y, pos.x + size.width, pos.y + size.height);
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

int UIButton::drawAndHitTest(ManagedSurface32 *dst, const Common::Point32 &mousePos,
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
		const Common::Rect32 clip(0, 0, *_overlayClipRight, MIN(static_cast<int>(dst->h), ManagedSurface32::kScreenSize.height));
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

bool BitmapFont::load(const Common::Path &basePath) {
	for (int i = 0; i < kNumGlyphs; i++) {
		delete[] _glyphs[i].mask;
		_glyphs[i].mask = nullptr;
		_glyphs[i].width = 0;
		_glyphs[i].height = 0;
	}
	_loaded = false;

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

	const Size32 size = alphaBB.getSize();

	int glyphIndex = 0;
	int col = 1;

	while (col < size.width && glyphIndex < kNumGlyphs) {
		while (col < size.width) {
			bool blank = true;
			for (int row = 0; row < size.height; row++) {
				if (srcAlpha[row * size.width + col] != 0) {
					blank = false;
					break;
				}
			}
			if (!blank)
				break;
			col += 1;
		}

		if (col >= size.width)
			break;

		const int startCol = col;
		while (col < size.width) {
			bool blank = true;
			for (int row = 0; row < size.height; row++) {
				if (srcAlpha[row * size.width + col] != 0) {
					blank = false;
					break;
				}
			}
			if (blank)
				break;
			col += 1;
		}

		const int glyphWidth = col - startCol + 2;

		Glyph &glyph = _glyphs[glyphIndex];
		glyph.width = glyphWidth;
		glyph.height = size.height;
		glyph.mask = new byte[glyphWidth * size.height]();
		const int copyWidth = MIN(glyphWidth, size.width - startCol);
		for (int row = 0; row < size.height; row++) {
			memcpy(glyph.mask + row * glyphWidth, srcAlpha + row * size.width + startCol, copyWidth);
		}

		glyphIndex += 1;
	}

	if (glyphIndex != kNumGlyphs) {
		warning("BitmapFont: expected %d glyphs but found %d in '%s'", kNumGlyphs, glyphIndex, colorPath.toString().c_str());
		return false;
	}

	_loaded = true;
	debug(1, "BitmapFont: Loaded %d glyphs from %s", glyphIndex, basePath.toString().c_str());

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

void BitmapFont::drawGlyph(ManagedSurface32 *dst, const Glyph &glyph, const Common::Point32 &pos,
						   byte red, byte green, byte blue, const AlphaBlendLUT &alphaLUT) const {
	if (!glyph.mask)
		return;

	for (int row = 0; row < glyph.height; row++) {
		const int destY = pos.y + row;
		if (destY < 0 || dst->h <= destY)
			continue;
		for (int column = 0; column < glyph.width; column++) {
			const int destX = pos.x + column;
			if (destX < 0 || dst->w <= destX)
				continue;

			const byte mask = glyph.mask[row * glyph.width + column];
			if (mask == 0)
				continue;
			byte *destPtr = static_cast<byte *>(dst->getBasePtr(destX, destY));
			if (mask == 255) {
				destPtr[0] = blue;
				destPtr[1] = green;
				destPtr[2] = red;
			} else {
				// The tint is uniform across the glyph, so scale it by the
				// coverage and scale the destination by the coverage's inverse.
				const byte invAlpha = 255 - mask;
				const int srcBlue = alphaLUT.scale(mask, blue);
				const int srcGreen = alphaLUT.scale(mask, green);
				const int srcRed = alphaLUT.scale(mask, red);
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

int BitmapFont::drawString(ManagedSurface32 *dst, const Common::Point32 &pos,
						   const Common::String &text, byte red, byte green, byte blue,
						   const AlphaBlendLUT &alphaLUT) const {
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
		if (glyphIdx < 0 || glyphIdx >= kNumGlyphs || !_glyphs[glyphIdx].mask) {
			curX += kSpaceWidth; // Unknown character, treat as space
			continue;
		}

		drawGlyph(dst, _glyphs[glyphIdx], Common::Point32(curX, pos.y), red, green, blue, alphaLUT);
		curX += _glyphs[glyphIdx].width + 2;
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
		if (glyphIdx < 0 || glyphIdx >= kNumGlyphs || !_glyphs[glyphIdx].mask) {
			width += kSpaceWidth;
			continue;
		}

		width += _glyphs[glyphIdx].width + 2;
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

	_okButton.setRect(Common::Point32(294, 487), Size32(76, 74));
	if (!_okButton.loadImages(Common::Path("bmp/menu/MENU - Valid - OK"), Common::Path("bmp/menu/MENU - Valid - OK highlight")))
		loaded = false;

	_noButton.setRect(Common::Point32(468, 487), Size32(76, 74));
	if (!_noButton.loadImages(Common::Path("bmp/menu/MENU - Valid - NO"), Common::Path("bmp/menu/MENU - Valid - NO highlight")))
		loaded = false;

	_sliderLabels[0].setRect(Common::Point32(kLabelX, kMusicLabelY), kLabelSize);
	if (!_sliderLabels[0].loadImages(Common::Path("bmp/menu/OPTION - Musique NORMAL"), Common::Path("bmp/menu/OPTION - Musique HIGHLIGHT")))
		loaded = false;

	_sliderLabels[1].setRect(Common::Point32(kLabelX, kSfxLabelY), kLabelSize);
	if (!_sliderLabels[1].loadImages(Common::Path("bmp/menu/OPTION - Bruitages NORMAL"), Common::Path("bmp/menu/OPTION - Bruitages HILITE")))
		loaded = false;

	_sliderLabels[2].setRect(Common::Point32(kLabelX, kSpeechLabelY), kLabelSize);
	if (!_sliderLabels[2].loadImages(Common::Path("bmp/menu/OPTION - Dialogues NORMAL"), Common::Path("bmp/menu/OPTION - Dialogues HILITE")))
		loaded = false;

	_musicSliderX = volumeToPixel(_settings._music);
	_sfxSliderX = volumeToPixel(_settings._sfx);
	_speechSliderX = volumeToPixel(_settings._speech);
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
	_settings.setMusic(vol);
	_musicSliderX = volumeToPixel(_settings._music);
}

void VolumePanel::setSfxVolume(int vol) {
	_settings.setSfx(vol);
	_sfxSliderX = volumeToPixel(_settings._sfx);
}

void VolumePanel::setSpeechVolume(int vol) {
	_settings.setSpeech(vol);
	_speechSliderX = volumeToPixel(_settings._speech);
}

void VolumePanel::setInitialVolumes(int music, int sfx, int speech) {
	_settings.setInitialVolumes(music, sfx, speech);
	_musicSliderX = volumeToPixel(_settings._music);
	_sfxSliderX = volumeToPixel(_settings._sfx);
	_speechSliderX = volumeToPixel(_settings._speech);
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
		_soundManager->playWithVolume(_sfxPreviewSoundId, _settings._sfx);
	} else if (adjustedSlider == 2 && 0 <= _speechPreviewSoundId) {
		_soundManager->stop(_speechPreviewSoundId);
		_soundManager->playWithVolume(_speechPreviewSoundId, _settings._speech);
	}
}

void VolumePanel::draw(ManagedSurface32 *dst, const Common::Point32 &mousePos, const AlphaBlendLUT &alphaLUT) {
	for (int i = 0; i < 3; i++)
		_sliderLabels[i].drawAndHitTest(dst, mousePos, alphaLUT);
	_okButton.drawAndHitTest(dst, mousePos, alphaLUT);
	_noButton.drawAndHitTest(dst, mousePos, alphaLUT);
}

} // End of namespace Zoombini2
