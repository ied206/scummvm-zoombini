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

#include <math.h>

#include "common/debug.h"
#include "common/endian.h"
#include "common/ptr.h"
#include "common/textconsole.h"

#include "engines/util.h"

#include "graphics/blit.h"
#include "graphics/pixelformat.h"
#include "image/bmp.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/page_base.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/text_manager.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr Size32 Gfx::kScreenSize;
constexpr Graphics::PixelFormat Gfx::kScreenFormat;
constexpr int Gfx::kNoMaxWidth;
constexpr const char *Gfx::kBasicZoombiniAnimationPath;
constexpr const char *Gfx::kCelebrationZoombiniAnimationPath;
constexpr const char *Gfx::kFallingZoombiniAnimationPath;
constexpr const char *Gfx::kSuctionZoombiniAnimationPath;
constexpr const char *Gfx::kFloatingZoombiniAnimationPath;
constexpr const char *Gfx::kJumpingZoombiniAnimationPath;
constexpr const char *Gfx::kPickupZoombiniAnimationPath;
constexpr const char *Gfx::kFidgetZoombiniAnimationPath;
constexpr const char *Gfx::kMapTransTinyZoombiniAnimationPath;
constexpr Graphics::PixelFormat RleBlock::kSourceFormat;
constexpr Size32 VolumePanel::kLabelSize;
constexpr const char *VolumePanel::kGaugeImagePath;
constexpr const char *VolumePanel::kOkNormalPath;
constexpr const char *VolumePanel::kOkNormalMaskPath;
constexpr const char *VolumePanel::kOkHighlightPath;
constexpr const char *VolumePanel::kOkHighlightMaskPath;
constexpr const char *VolumePanel::kNoNormalPath;
constexpr const char *VolumePanel::kNoNormalMaskPath;
constexpr const char *VolumePanel::kNoHighlightPath;
constexpr const char *VolumePanel::kNoHighlightMaskPath;
constexpr const char *VolumePanel::kMusicNormalPath;
constexpr const char *VolumePanel::kMusicHighlightPath;
constexpr const char *VolumePanel::kMusicMaskPath;
constexpr const char *VolumePanel::kSfxNormalPath;
constexpr const char *VolumePanel::kSfxHighlightPath;
constexpr const char *VolumePanel::kSfxMaskPath;
constexpr const char *VolumePanel::kSpeechNormalPath;
constexpr const char *VolumePanel::kSpeechHighlightPath;
constexpr const char *VolumePanel::kSpeechMaskPath;
constexpr const char *VolumePanel::kSpeechPreviewSoundPath;
constexpr const char *VolumePanel::kSfxPreviewSoundPath;

constexpr const char *Gfx::kNameBoxSpritePath;
constexpr RGBColor Gfx::PredefinedColor::kBlack;
constexpr RGBColor Gfx::PredefinedColor::kDark00;
constexpr RGBColor Gfx::PredefinedColor::kBlue01;
constexpr RGBColor Gfx::PredefinedColor::kGreen02;
constexpr RGBColor Gfx::PredefinedColor::kWhite03;
constexpr RGBColor Gfx::PredefinedColor::kRed04;
constexpr const char *Gfx::kDropTargetGlowPath;

void ManagedSurface32::fillRect(const Common::Rect32 &rect, uint32 color) {
	if (!rect.isValidRect())
		return;

	Common::Rect32 clip32 = rect;
	clip32.clip(w, h);
	if (clip32.isEmpty())
		return;
	const Common::Rect clip16 = Common::Rect(clip32.left, clip32.top, clip32.right, clip32.bottom);
	Graphics::ManagedSurface::fillRect(clip16, color);
}

void ManagedSurface32::frameRect(const Common::Rect32 &rect, uint32 color) {
	if (!rect.isValidRect())
		return;

	Common::Rect32 clip32 = rect;
	clip32.clip(w, h);
	if (clip32.isEmpty())
		return;
	const Common::Rect clip16 = Common::Rect(clip32.left, clip32.top, clip32.right, clip32.bottom);
	Graphics::ManagedSurface::frameRect(clip16, color);
}

void ManagedSurface32::copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect32 &subRect) {
	if (!subRect.isValidRect())
		return;

	const Common::Rect32 srcBounds(srcSurface.w, srcSurface.h);
	const Common::Rect32 src = subRect.findIntersectingRect(srcBounds);
	if (src.isEmpty())
		return;

	const int32 destLeft = destX + src.left - subRect.left;
	const int32 destTop = destY + src.top - subRect.top;
	const int32 destRight = destLeft + src.width();
	const int32 destBottom = destTop + src.height();
	if (destRight <= 0 || destBottom <= 0 || w <= destLeft || h <= destTop)
		return;

	const int32 clippedDestLeft = destLeft < 0 ? 0 : destLeft;
	const int32 clippedDestTop = destTop < 0 ? 0 : destTop;
	const int32 clippedDestRight = w < destRight ? w : destRight;
	const int32 clippedDestBottom = h < destBottom ? h : destBottom;
	const int32 clippedSrcLeft = src.left + clippedDestLeft - destLeft;
	const int32 clippedSrcTop = src.top + clippedDestTop - destTop;
	const int32 clippedSrcRight = clippedSrcLeft + clippedDestRight - clippedDestLeft;
	const int32 clippedSrcBottom = clippedSrcTop + clippedDestBottom - clippedDestTop;

	const Common::Rect clippedSrc(static_cast<int16>(clippedSrcLeft), static_cast<int16>(clippedSrcTop), static_cast<int16>(clippedSrcRight), static_cast<int16>(clippedSrcBottom));
	Graphics::ManagedSurface::copyRectToSurface(srcSurface, static_cast<int>(clippedDestLeft), static_cast<int>(clippedDestTop), clippedSrc);
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

RGBColor AlphaBlendLUT::blend(const RGBColor &srcColor, const RGBColor &destColor, byte sourceFactor, byte destFactor) const {
	return RGBColor(
		static_cast<byte>(MIN<int>(scale(sourceFactor, srcColor.r) + scale(destFactor, destColor.r), 255)),
		static_cast<byte>(MIN<int>(scale(sourceFactor, srcColor.g) + scale(destFactor, destColor.g), 255)),
		static_cast<byte>(MIN<int>(scale(sourceFactor, srcColor.b) + scale(destFactor, destColor.b), 255)));
}

BitBlock::BitBlock(Zoombini2Engine *vm)
	: _vm(vm) {
}

BitBlock::~BitBlock() {
	delete[] _pixels;
	delete[] _alphaMap;
}

/** Load a color BMP and separate alpha BMP into one drawable block. */
bool BitBlock::loadFromColorAlphaBMP(const Common::Path &colorPath, const Common::Path &alphaPath) {
	Common::ScopedPtr<Common::SeekableReadStream> colorStream(_vm->openResourceFile(colorPath.toString('/')));
	if (!colorStream) {
		if (!_vm->isDemo())
			warning("BitBlock: cannot open color BMP '%s'", colorPath.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &colorFile = *colorStream;

	BitBlock loaded(_vm);
	if (!loaded.loadColorBMP(&colorFile))
		return false;

	Common::ScopedPtr<Common::SeekableReadStream> alphaStream(_vm->openResourceFile(alphaPath.toString('/')));
	if (!alphaStream) {
		if (!_vm->isDemo())
			warning("BitBlock: cannot open alpha BMP '%s'", alphaPath.toString().c_str());
		return false;
	}
	Common::SeekableReadStream &alphaFile = *alphaStream;
	if (!loaded.loadAlphaBMP(&alphaFile)) {
		warning("BitBlock: failed to load alpha BMP '%s'", alphaPath.toString().c_str());
		return false;
	}

	loaded._resourcePath = colorPath;
	loaded._alphaResourcePath = alphaPath;
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

	loaded._resourcePath = colorPath;
	swapData(loaded);
	return true;
}

/** Load one cached BB record described by @ref BitBlock. */
bool BitBlock::loadFromBB(const Common::Path &bbPath) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->openResourceFile(bbPath.toString('/')));
	if (!stream) {
		debug(3, "BitBlock: cannot open BB '%s'", bbPath.toString().c_str());
		return false;
	}
	return loadBBStream(*stream, bbPath);
}

bool BitBlock::loadBBStream(Common::SeekableReadStream &f, const Common::Path &bbPath) {
	/* int32 alphaMapPlaceholder = */ f.readSint32LE();
	/* int32 pixelsPlaceholder = */ f.readSint32LE();
	const uint32 bufferSize = f.readUint32LE();
	const int32 width = f.readSint32LE();
	const int32 height = f.readSint32LE();
	Size32 size(width, height);
	const uint32 dataSize = f.readUint32LE();

	if (f.err() || f.eos() || size.width <= 0 || size.height <= 0) {
		warning("BitBlock: invalid BB header in '%s'", bbPath.toString().c_str());
		return false;
	}

	const uint64 pixelCount64 = static_cast<uint64>(size.width) * static_cast<uint64>(size.height);
	const uint64 expectedSize64 = pixelCount64 * 3;
	// Drawing clips through signed 16-bit rectangles, and the decoded buffer uses four bytes per pixel.
	if (INT16_MAX < size.width || INT16_MAX < size.height || static_cast<uint64>(INT32_MAX) / 4 < pixelCount64 || UINT32_MAX < expectedSize64) {
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
	if (!recoveredPixelCount) {
		warning("BitBlock: no complete BB pixels in '%s'", bbPath.toString().c_str());
		return false;
	}

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

	// Missing rows do not justify allocating their declared pixel storage.
	size.height = static_cast<int32>((static_cast<uint64>(recoveredPixelCount) + size.width - 1) / size.width);
	const uint32 pixelCount = static_cast<uint32>(size.width) * static_cast<uint32>(size.height);
	const uint32 decodedSize = pixelCount * 4;
	byte *pixels = new byte[decodedSize];
	for (uint32 i = 0; i < pixelCount; i++) {
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
	_resourcePath = bbPath;
	_alphaResourcePath.clear();

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

	Common::Path srcPath(basePath);
	Common::Path cachePath(basePath);
	if (pathString.hasSuffixIgnoreCase(".bmp") || pathString.hasSuffixIgnoreCase(".bmt")) {
		cachePath.removeExtension();
		cachePath = cachePath.append(".bb");
	} else {
		cachePath = cachePath.append(".bb");
		srcPath = srcPath.append(".bmp");
	}

	if (loadFromBB(cachePath))
		return true;
	return loadFromColorBMP(srcPath);
}

void BitBlock::recordPageCacheUse() const {
	_vm->recordPageResourceCacheUse(_resourcePath.toString('/'), Zoombini2Engine::PageResourceType::kBitmap);
	if (!_alphaResourcePath.empty())
		_vm->recordPageResourceCacheUse(_alphaResourcePath.toString('/'), Zoombini2Engine::PageResourceType::kBitmap);
}

void BitBlock::createEmpty(const Size32 &size, bool withAlpha) {
	delete[] _pixels;
	delete[] _alphaMap;
	_size = size;
	_pixels = new byte[size.width * size.height * 4]();
	_alphaMap = withAlpha ? new byte[size.width * size.height]() : nullptr;
	_resourcePath.clear();
	_alphaResourcePath.clear();
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
	SWAP(_resourcePath, other._resourcePath);
	SWAP(_alphaResourcePath, other._alphaResourcePath);
}

byte BitBlock::blendChannel(byte src, byte dest, byte mask) {
	// The bitmap supplies the source contribution without further scaling.
	// Only the destination is scaled by the inverse mask, using integer division by 255.
	const int result = src + (dest * (255 - mask)) / 255;
	return static_cast<byte>(MIN(result, 255));
}

void BitBlock::drawToSurface(ManagedSurface32 *destSurface, const Common::Point32 &pos) const {
	drawToSurfaceInternal(destSurface, pos, Common::Rect32(_size.width, _size.height), false);
}

void BitBlock::drawToSurfaceColorKey(ManagedSurface32 *destSurface, const Common::Point32 &pos) const {
	drawToSurfaceInternal(destSurface, pos, Common::Rect32(_size.width, _size.height), true);
}

void BitBlock::drawSubRect(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect &srcRect) const {
	drawToSurfaceInternal(destSurface, pos, Common::Rect32(srcRect.left, srcRect.top, srcRect.right, srcRect.bottom), false);
}

void BitBlock::drawToSurfaceInternal(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect32 &srcRect, bool useColorKey) const {
	if (!_pixels || srcRect.isEmpty())
		return;

	const Common::Rect32 src = srcRect.findIntersectingRect(Common::Rect32(_size.width, _size.height));
	if (src.isEmpty())
		return;

	const int32 left = pos.x + src.left - srcRect.left;
	const int32 top = pos.y + src.top - srcRect.top;
	const int32 right = left + src.width();
	const int32 bottom = top + src.height();
	if (right <= 0 || bottom <= 0 || destSurface->w <= left || destSurface->h <= top)
		return;

	const Common::Rect dest(MAX<int32>(0, left), MAX<int32>(0, top), MIN<int32>(destSurface->w, right), MIN<int32>(destSurface->h, bottom));
	const int srcX = src.left + static_cast<int>(dest.left - left);
	const int srcY = src.top + static_cast<int>(dest.top - top);
	const uint srcPitch = static_cast<uint>(_size.width) * 4;
	const byte *srcPixels = _pixels + static_cast<size_t>(srcY) * srcPitch + static_cast<size_t>(srcX) * 4;
	byte *destPixels = static_cast<byte *>(destSurface->getBasePtr(dest.left, dest.top));
	bool converted = false;
	if (useColorKey) {
		static constexpr Graphics::PixelFormat kSourceFormat = Graphics::PixelFormat::createFormatRGBA32();
		const uint32 colorKey = RGBColor(_pixels[0], _pixels[1], _pixels[2]).toPixel(kSourceFormat);
		converted = Graphics::crossKeyBlit(destPixels, srcPixels, destSurface->pitch, srcPitch, dest.width(), dest.height(), destSurface->format,
										   kSourceFormat, colorKey);
	} else {
		// Ignore the stored fourth byte, including zero-filled bitmaps, and produce opaque output.
		static constexpr Graphics::PixelFormat kSourceFormat = Graphics::PixelFormat::createFormatRGBA32(false);
		converted = Graphics::crossBlit(destPixels, srcPixels, destSurface->pitch, srcPitch, dest.width(), dest.height(), destSurface->format,
										kSourceFormat);
	}
	if (!converted) {
		warning("BitBlock: unsupported pixel conversion");
		return;
	}
	destSurface->addDirtyRect(dest);
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
	assert(dst->format.bytesPerPixel == 4);
	static constexpr Graphics::PixelFormat kSourceFormat = Graphics::PixelFormat::createFormatRGBA32();

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

			const RGBColor srcColor = RGBColor::fromPixel(READ_UINT32(src), kSourceFormat);
			const RGBColor destColor = RGBColor::fromPixel(READ_UINT32(dstPixel), dst->format);
			const RGBColor blendedColor(blendChannel(srcColor.r, destColor.r, alpha), blendChannel(srcColor.g, destColor.g, alpha),
										blendChannel(srcColor.b, destColor.b, alpha));
			WRITE_UINT32(dstPixel, blendedColor.toPixel(dst->format));
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
	assert(dst->format.bytesPerPixel == 4);
	static constexpr Graphics::PixelFormat kSourceFormat = Graphics::PixelFormat::createFormatRGBA32();

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
			const RGBColor srcColor = RGBColor::fromPixel(READ_UINT32(src), kSourceFormat);
			if (mask == 255) {
				WRITE_UINT32(destPtr, srcColor.toPixel(dst->format));
			} else {
				// Round the source and destination contributions separately through the LUT.
				const byte invAlpha = 255 - mask;
				const RGBColor destColor = RGBColor::fromPixel(READ_UINT32(destPtr), dst->format);
				const RGBColor blendedColor = alphaLUT.blend(srcColor, destColor, mask, invAlpha);
				WRITE_UINT32(destPtr, blendedColor.toPixel(dst->format));
			}
		}
	}
}

RleBlock::RleBlock(Zoombini2Engine *vm)
	: _vm(vm) {
}

RleBlock::~RleBlock() {
	free(_pixels);
}

void RleBlock::swapData(RleBlock &other) {
	SWAP(_size, other._size);
	_spans.swap(other._spans);
	SWAP(_pixels, other._pixels);
	SWAP(_pixelCount, other._pixelCount);
	SWAP(_loaded, other._loaded);
	SWAP(_resourcePath, other._resourcePath);
}

/**
 * Load one serialized RLE record from a standalone `.rb` resource.
 * File: [24-byte header] [4-byte dataSize] [dataSize bytes RLE data].
 */
bool RleBlock::loadFromStream(Common::SeekableReadStream *stream) {
	/* uint32 cachedEffectiveHeight = */ stream->readUint32LE();
	/* uint32 serializedDataPointer = */ stream->readUint32LE();
	const uint32 headerSize = stream->readUint32LE();
	const int32 width = stream->readSint32LE();
	const int32 height = stream->readSint32LE();
	const Size32 size(width, height);
	/* uint32 uninitializedSerializedField = */ stream->readUint32LE();
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

	Common::Array<Span> spans;
	uint32 *pixels = nullptr;
	uint32 pixelCount = 0;
	bool salvagedSpans = false;
	if (!decodeSpans(encodedData, readableSize, spans, pixels, pixelCount, salvagedSpans)) {
		warning("RleBlock: malformed RLE span data");
		free(encodedData);
		return false;
	}
	free(encodedData);
	if (salvagedSpans)
		warning("RleBlock: discarded a malformed RLE tail after complete spans");

	free(_pixels);
	_size = size;
	_spans.swap(spans);
	_pixels = pixels;
	_pixelCount = pixelCount;
	_loaded = true;
	_resourcePath.clear();

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
	loaded._resourcePath = resolvedPath;
	swapData(loaded);
	return true;
}

bool RleBlock::load(const Common::Path &basePath) {
	return loadFromFile(basePath);
}

void RleBlock::recordPageCacheUse() const {
	_vm->recordPageResourceCacheUse(_resourcePath.toString('/'), Zoombini2Engine::PageResourceType::kSprite);
}

bool RleBlock::loadAnimationFrame(Common::SeekableReadStream *stream, uint32 outerSize) {
	/* uint32 cachedEffectiveHeight = */ stream->readUint32LE();
	/* uint32 serializedDataPointer = */ stream->readUint32LE();
	const uint32 headerSize = stream->readUint32LE();
	const int32 width = stream->readSint32LE();
	const int32 height = stream->readSint32LE();
	const Size32 size(width, height);
	/* uint32 uninitializedSerializedField = */ stream->readUint32LE();

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

	Common::Array<Span> spans;
	uint32 *pixels = nullptr;
	uint32 pixelCount = 0;
	bool salvagedSpans = false;
	if (!decodeSpans(encodedData, readableSize, spans, pixels, pixelCount, salvagedSpans)) {
		free(encodedData);
		return false;
	}
	free(encodedData);
	if (salvagedSpans)
		warning("RleBlock: discarded a malformed ANM RLE tail after complete spans");

	free(_pixels);
	_size = size;
	_spans.swap(spans);
	_pixels = pixels;
	_pixelCount = pixelCount;
	_loaded = true;
	_resourcePath.clear();
	return true;
}

bool RleBlock::decodeSpans(const byte *srcData, uint32 srcSize, Common::Array<Span> &spans, uint32 *&pixels, uint32 &pixelCount, bool &salvaged) {
	spans.clear();
	pixels = nullptr;
	pixelCount = 0;
	salvaged = false;
	if (!srcData || srcSize < 2)
		return false;

	const byte *src = srcData + 2;
	const byte *srcEnd = srcData + srcSize;
	uint32 spanCount = 0;
	uint64 requiredPixelCount = 0;
	while (src < srcEnd) {
		if (static_cast<size_t>(srcEnd - src) < 7) {
			salvaged = true;
			break;
		}
		const int16 encodedPixelCount = READ_LE_INT16(src + 4);
		const byte rawMode = src[6];
		if (encodedPixelCount < 0 || static_cast<byte>(SpanMode::kInverseAlphaBgr01) < rawMode) {
			salvaged = true;
			break;
		}
		const SpanMode mode = static_cast<SpanMode>(rawMode);
		src += 7;
		const uint32 encodedPixelSize = mode == SpanMode::kOpaqueBgr00 ? 3 : 4;
		const uint32 availablePixelCount = MIN<uint32>(encodedPixelCount, static_cast<size_t>(srcEnd - src) / encodedPixelSize);
		src += availablePixelCount * encodedPixelSize;
		requiredPixelCount += availablePixelCount;
		if (0xFFFFFFFFU / sizeof(uint32) < requiredPixelCount || 0xFFFFFFFFU / sizeof(Span) <= spanCount)
			return false;
		spanCount += 1;
		if (availablePixelCount != static_cast<uint32>(encodedPixelCount)) {
			salvaged = true;
			break;
		}
	}

	// The first serialized payload starts at offset 0x09 after the two-byte prefix and seven-byte span header.
	// Later seven-byte headers likewise do not guarantee four-byte payload alignment.
	// Keep headers separate and place every decoded pixel in one uint32-aligned allocation for the shared 32-bit blitters.
	uint32 *decodedPixels = nullptr;
	if (requiredPixelCount != 0)
		decodedPixels = static_cast<uint32 *>(malloc(static_cast<size_t>(requiredPixelCount) * sizeof(uint32)));
	if (requiredPixelCount != 0 && !decodedPixels)
		return false;

	Common::Array<Span> decodedSpans;
	decodedSpans.reserve(spanCount);
	src = srcData + 2;
	uint32 decodedPixelCount = 0;
	for (uint32 spanIndex = 0; spanIndex < spanCount; spanIndex++) {
		Span span;
		span.xOffset = READ_LE_INT16(src);
		span.yOffset = READ_LE_INT16(src + 2);
		const int16 encodedPixelCount = READ_LE_INT16(src + 4);
		span.mode = static_cast<SpanMode>(src[6]);
		span.pixelOffset = decodedPixelCount;
		const uint32 encodedPixelSize = span.mode == SpanMode::kOpaqueBgr00 ? 3 : 4;
		span.pixelCount = MIN<uint32>(encodedPixelCount, static_cast<size_t>(srcEnd - src - 7) / encodedPixelSize);
		src += 7;
		byte *dst = nullptr;
		if (span.pixelCount != 0)
			dst = reinterpret_cast<byte *>(decodedPixels + decodedPixelCount);

		if (span.mode == SpanMode::kInverseAlphaBgr01) {
			const uint32 copySize = span.pixelCount * 4;
			if (copySize != 0)
				memcpy(dst, src, copySize);
			src += copySize;
		} else {
			for (uint32 i = 0; i < span.pixelCount; i++) {
				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				dst[3] = 0;
				src += 3;
				dst += 4;
			}
		}
		decodedPixelCount += span.pixelCount;
		decodedSpans.push_back(span);
	}

	spans.swap(decodedSpans);
	pixels = decodedPixels;
	pixelCount = decodedPixelCount;
	return true;
}

byte RleBlock::blendChannel(byte src, byte dest, byte invAlpha, const AlphaBlendLUT &alphaLUT) {
	// @ref Graphics::alphaBlit normally handles per-pixel opacity, but expects straight RGB and forward alpha.
	// Mode 1 RLE stores premultiplied BGR and inverse alpha, so that blitter would scale the source again.
	// Keep `min(255, source + floor(destination * inverseAlpha / 256))` via @ref AlphaBlendLUT.
	// Unpremultiplying first would lose integer precision and would not preserve the saturated sum.
	const int destPart = alphaLUT.scale(invAlpha, dest);
	return static_cast<byte>(MIN(static_cast<int>(src) + destPart, 255));
}

bool RleBlock::recolorPixel(const byte *source, bool premultiplied, const RGBColor &targetColor, byte srcBrightness, bool preserveGradient, RGBColor &adjustedColor) {
	const int alpha = premultiplied ? 255 - source[3] : 255;
	if (alpha < 8)
		return false;
	const RGBColor pixelColor = RGBColor::fromPixel(READ_UINT32(source), kSourceFormat);
	const RGBColor sourceColor(static_cast<byte>(MIN(255, pixelColor.r * 255 / alpha)), static_cast<byte>(MIN(255, pixelColor.g * 255 / alpha)),
							   static_cast<byte>(MIN(255, pixelColor.b * 255 / alpha)));
	const int brightest = sourceColor.getMaxChannel();
	const int darkest = sourceColor.getMinChannel();
	if (brightest - darkest < 24)
		return false;
	if (preserveGradient) {
		adjustedColor = Gfx::recolorNoseGradientRGB(sourceColor, targetColor);
		return true;
	}
	adjustedColor.r = static_cast<byte>(MIN(255, targetColor.r * brightest / srcBrightness));
	adjustedColor.g = static_cast<byte>(MIN(255, targetColor.g * brightest / srcBrightness));
	adjustedColor.b = static_cast<byte>(MIN(255, targetColor.b * brightest / srcBrightness));
	return true;
}

/**
 * Draw the RLE spans with opaque copying or lookup-table alpha blending.
 */
void RleBlock::drawToScreen(ManagedSurface32 *destSurface, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const {
	drawToScreenInternal(destSurface, pos, Common::Rect32(destSurface->w, destSurface->h), alphaLUT);
}

void RleBlock::drawToScreenColorKey(ManagedSurface32 *destSurface, const Common::Point32 &pos, const RGBColor &colorKey, const AlphaBlendLUT &alphaLUT) const {
	drawToScreenInternal(destSurface, pos, Common::Rect32(destSurface->w, destSurface->h), alphaLUT, &colorKey);
}

void RleBlock::drawToScreenClipped(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect32 &clip, const AlphaBlendLUT &alphaLUT) const {
	drawToScreenInternal(destSurface, pos, clip, alphaLUT);
}

void RleBlock::drawToScreenRecolored(ManagedSurface32 *destSurface, const Common::Point32 &pos, const RGBColor &targetColor, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip, byte sourceBrightness) const {
	if (!destSurface)
		return;
	const RecolorSettings recolor = {targetColor, sourceBrightness ? sourceBrightness : static_cast<byte>(1), false};
	const Common::Rect32 drawClip = clip ? *clip : Common::Rect32(destSurface->w, destSurface->h);
	drawToScreenInternal(destSurface, pos, drawClip, alphaLUT, nullptr, &recolor);
}

void RleBlock::drawToScreenRecoloredWithGradient(ManagedSurface32 *destSurface, const Common::Point32 &pos, const RGBColor &targetColor, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip) const {
	if (!destSurface)
		return;
	const RecolorSettings recolor = {targetColor, 255, true};
	const Common::Rect32 drawClip = clip ? *clip : Common::Rect32(destSurface->w, destSurface->h);
	drawToScreenInternal(destSurface, pos, drawClip, alphaLUT, nullptr, &recolor);
}

void RleBlock::drawToScreenMirrored(ManagedSurface32 *destSurface, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const {
	if (!_loaded || !destSurface)
		return;
	assert(destSurface->format.bytesPerPixel == 4);
	for (uint spanIndex = 0; spanIndex < _spans.size(); spanIndex++) {
		const Span &span = _spans[spanIndex];
		assert(span.pixelOffset <= _pixelCount);
		assert(span.pixelCount <= _pixelCount - span.pixelOffset);

		const int32 screenX = pos.x + _size.width - span.xOffset - static_cast<int32>(span.pixelCount);
		const int32 screenY = pos.y + span.yOffset;
		if (screenY < 0 || destSurface->h <= screenY)
			continue;
		const int32 left = MAX<int32>(screenX, 0);
		const int32 right = MIN<int32>(screenX + static_cast<int32>(span.pixelCount), destSurface->w);
		if (right <= left)
			continue;

		const int32 sourceCol = screenX + static_cast<int32>(span.pixelCount) - 1 - left;
		const byte *sourcePixels = reinterpret_cast<const byte *>(_pixels + span.pixelOffset);
		byte *dstPixel = static_cast<byte *>(destSurface->getBasePtr(left, screenY));
		for (int32 x = left; x < right; x++) {
			const byte *srcPixel = sourcePixels + (sourceCol - (x - left)) * 4;
			if (span.mode == SpanMode::kOpaqueBgr00) {
				if (!Graphics::crossBlit(dstPixel, srcPixel, destSurface->pitch, 4, 1, 1, destSurface->format, kSourceFormat)) {
					warning("RleBlock: unsupported mirrored opaque pixel conversion");
					return;
				}
			} else {
				assert(span.mode == SpanMode::kInverseAlphaBgr01);
				const byte invAlpha = srcPixel[3];
				const RGBColor sourceColor = RGBColor::fromPixel(READ_UINT32(srcPixel), kSourceFormat);
				const RGBColor destColor = RGBColor::fromPixel(READ_UINT32(dstPixel), destSurface->format);
				const RGBColor blendedColor(blendChannel(sourceColor.r, destColor.r, invAlpha, alphaLUT),
											blendChannel(sourceColor.g, destColor.g, invAlpha, alphaLUT),
											blendChannel(sourceColor.b, destColor.b, invAlpha, alphaLUT));
				WRITE_UINT32(dstPixel, blendedColor.toPixel(destSurface->format));
			}
			dstPixel += 4;
		}
		destSurface->addDirtyRect(Common::Rect(left, screenY, right, screenY + 1));
	}
}

void RleBlock::drawToScreenSolidColor(ManagedSurface32 *destSurface, const Common::Point32 &pos, const RGBColor &color, const AlphaBlendLUT &alphaLUT) const {
	if (!_loaded || !destSurface)
		return;
	assert(destSurface->format.bytesPerPixel == 4);
	const uint32 solidPixel = color.toPixel(destSurface->format);
	for (uint spanIndex = 0; spanIndex < _spans.size(); spanIndex++) {
		const Span &span = _spans[spanIndex];
		const int32 screenX = pos.x + span.xOffset;
		const int32 screenY = pos.y + span.yOffset;
		if (screenY < 0 || destSurface->h <= screenY)
			continue;
		const int32 left = MAX<int32>(screenX, 0);
		const int32 right = MIN<int32>(screenX + static_cast<int32>(span.pixelCount), destSurface->w);
		if (right <= left)
			continue;
		const int startCol = static_cast<int>(left - screenX);
		const byte *srcPixel = reinterpret_cast<const byte *>(_pixels + span.pixelOffset + startCol);
		byte *dstPixel = static_cast<byte *>(destSurface->getBasePtr(left, screenY));
		for (int x = left; x < right; x++) {
			if (span.mode == SpanMode::kOpaqueBgr00) {
				WRITE_UINT32(dstPixel, solidPixel);
			} else {
				const byte inverseAlpha = srcPixel[3];
				const byte forwardAlpha = 255 - inverseAlpha;
				const RGBColor destColor = RGBColor::fromPixel(READ_UINT32(dstPixel), destSurface->format);
				const RGBColor blendedColor = alphaLUT.blend(color, destColor, forwardAlpha, inverseAlpha);
				WRITE_UINT32(dstPixel, blendedColor.toPixel(destSurface->format));
			}
			srcPixel += 4;
			dstPixel += 4;
		}
		destSurface->addDirtyRect(Common::Rect(left, screenY, right, screenY + 1));
	}
}

void RleBlock::drawToScreenInternal(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect32 &clip,
									const AlphaBlendLUT &alphaLUT, const RGBColor *colorKey, const RecolorSettings *recolor) const {
	if (!_loaded || !clip.isValidRect())
		return;

	const Common::Rect32 destClip = clip.findIntersectingRect(Common::Rect32(destSurface->w, destSurface->h));
	if (destClip.isEmpty())
		return;
	assert(destSurface->format.bytesPerPixel == 4);
	const uint32 colorKeyPixel = colorKey ? colorKey->toPixel(kSourceFormat) : 0;

	for (uint spanIndex = 0; spanIndex < _spans.size(); spanIndex++) {
		const Span &span = _spans[spanIndex];
		assert(span.pixelOffset <= _pixelCount);
		assert(span.pixelCount <= _pixelCount - span.pixelOffset);

		const int32 screenX = pos.x + span.xOffset;
		const int32 screenY = pos.y + span.yOffset;
		if (screenY < destClip.top || destClip.bottom <= screenY)
			continue;

		const int32 left = MAX<int32>(screenX, destClip.left);
		const int32 right = MIN<int32>(screenX + static_cast<int32>(span.pixelCount), destClip.right);
		if (left < right) {
			const int x = static_cast<int>(left);
			const int y = static_cast<int>(screenY);
			const int count = static_cast<int>(right - left);
			const int startCol = static_cast<int>(left - screenX);
			const byte *srcPixel = reinterpret_cast<const byte *>(_pixels + span.pixelOffset + startCol);
			byte *dstPixel = static_cast<byte *>(destSurface->getBasePtr(x, y));

			if (span.mode == SpanMode::kOpaqueBgr00) {
				if (recolor) {
					for (int i = 0; i < count; i++) {
						RGBColor adjustedColor;
						if (!recolorPixel(srcPixel, false, recolor->targetColor, recolor->sourceBrightness, recolor->preserveGradient, adjustedColor))
							adjustedColor = RGBColor::fromPixel(READ_UINT32(srcPixel), kSourceFormat);
						WRITE_UINT32(dstPixel, adjustedColor.toPixel(destSurface->format));
						srcPixel += 4;
						dstPixel += 4;
					}
				} else {
					bool converted = false;
					if (colorKey)
						converted = Graphics::crossKeyBlit(dstPixel, srcPixel, destSurface->pitch, count * 4, count, 1, destSurface->format, kSourceFormat, colorKeyPixel);
					else
						converted = Graphics::crossBlit(dstPixel, srcPixel, destSurface->pitch, count * 4, count, 1, destSurface->format, kSourceFormat);
					if (!converted) {
						warning("RleBlock: unsupported opaque pixel conversion");
						continue;
					}
				}
				destSurface->addDirtyRect(Common::Rect(x, y, x + count, y + 1));
			} else {
				assert(span.mode == SpanMode::kInverseAlphaBgr01);
				// Mode 1 stores premultiplied BGR followed by inverse alpha.
				for (int i = 0; i < count; i++) {
					const byte invAlpha = srcPixel[3];
					RGBColor sourceColor = RGBColor::fromPixel(READ_UINT32(srcPixel), kSourceFormat);
					if (recolor) {
						RGBColor adjustedColor;
						if (recolorPixel(srcPixel, true, recolor->targetColor, recolor->sourceBrightness, recolor->preserveGradient, adjustedColor)) {
							const byte alpha = 255 - invAlpha;
							sourceColor = alphaLUT.scale(alpha, adjustedColor);
						}
					}
					const RGBColor destColor = RGBColor::fromPixel(READ_UINT32(dstPixel), destSurface->format);
					const RGBColor blendedColor(
						blendChannel(sourceColor.r, destColor.r, invAlpha, alphaLUT),
						blendChannel(sourceColor.g, destColor.g, invAlpha, alphaLUT),
						blendChannel(sourceColor.b, destColor.b, invAlpha, alphaLUT));
					WRITE_UINT32(dstPixel, blendedColor.toPixel(destSurface->format));
					srcPixel += 4;
					dstPixel += 4;
				}
				destSurface->addDirtyRect(Common::Rect(x, y, x + count, y + 1));
			}
		}
	}
}

Gfx::Gfx(Zoombini2Engine *vm)
	: _vm(vm) {
	initGraphics(Gfx::kScreenSize.width, Gfx::kScreenSize.height, &kScreenFormat);
	_screen = new ManagedSurface32(Gfx::kScreenSize, kScreenFormat);
	_pageLayerStack = new PageLayerStack(vm);
	_dropTargetGlowSprite = loadSharedRleBlock(kDropTargetGlowPath);
	if (!_dropTargetGlowSprite)
		warning("Gfx: Failed to load shared drop-target glow");
}

Gfx::~Gfx() {
	delete _pageLayerStack;
	clearZoombiniAnimationCache();
	clearPageBitmapCache();
	clearRleBlockCache(_sharedRleBlocks);
	clearBitBlockCache(_sharedBitBlocks);
	delete _screen;
}

const ZoombiniAnimation *Gfx::loadZoombiniAnimation(const Common::Path &path, uint32 frameDelay) {
	const Common::Path key = Zoombini2Engine::normalizeResourcePath(path.toString('/'));
	ZoombiniAnimationCache::const_iterator cached = _zoombiniAnimationCache.find(key);
	if (cached != _zoombiniAnimationCache.end()) {
		cached->_value->setFrameDelay(frameDelay);
		_vm->recordPageResourceCacheUse(key.toString('/'), Zoombini2Engine::PageResourceType::kZoombiniAnimation);
		return cached->_value;
	}

	ZoombiniAnimation *animation = new ZoombiniAnimation(_vm);
	if (!animation->loadFromFile(key)) {
		delete animation;
		return nullptr;
	}
	animation->setFrameDelay(frameDelay);
	_zoombiniAnimationCache[key] = animation;
	_vm->recordPageResourceCacheUse(key.toString('/'), Zoombini2Engine::PageResourceType::kZoombiniAnimation);
	return animation;
}

void Gfx::loadCommonZoombiniAnimations() {
	if (!_commonZoombiniAnimations.empty())
		return;

	/** Resource and shared playback timing for one common Zoombini grid. */
	struct CommonAnimationDefinition {
		/** Common-grid identity used by pages and runners. */
		ZmbAnimationKind animKind;
		/** Logical ANM path resolved during engine startup. */
		const char *path;
		/** Shared frame duration in milliseconds. */
		uint32 frameDelay;
	};
	static constexpr CommonAnimationDefinition kDefinitions[] = {
		{ZmbAnimationKind::kBasic, kBasicZoombiniAnimationPath, 50},
		{ZmbAnimationKind::kCelebration, kCelebrationZoombiniAnimationPath, 50},
		{ZmbAnimationKind::kFalling, kFallingZoombiniAnimationPath, 70},
		{ZmbAnimationKind::kSuction, kSuctionZoombiniAnimationPath, 50},
		{ZmbAnimationKind::kFloating, kFloatingZoombiniAnimationPath, 190},
		{ZmbAnimationKind::kJumping, kJumpingZoombiniAnimationPath, 70},
		{ZmbAnimationKind::kPickup, kPickupZoombiniAnimationPath, 100},
		{ZmbAnimationKind::kFidget, kFidgetZoombiniAnimationPath, 50},
		{ZmbAnimationKind::kMapTransTiny, kMapTransTinyZoombiniAnimationPath, 50},
	};
	for (int i = 0; i < ARRAYSIZE(kDefinitions); i++) {
		const CommonAnimationDefinition &def = kDefinitions[i];
		const ZoombiniAnimation *animation = loadZoombiniAnimation(Common::Path(def.path), def.frameDelay);
		_commonZoombiniAnimations[def.animKind] = animation;
		if (!animation)
			warning("Gfx: Failed to load common Zoombini animation '%s'", def.path);
	}
}

const ZoombiniAnimation *Gfx::getCommonZoombiniAnimation(ZmbAnimationKind animKind) const {
	CommonZoombiniAnimationMap::const_iterator entry = _commonZoombiniAnimations.find(animKind);
	if (entry == _commonZoombiniAnimations.end() || !entry->_value)
		return nullptr;
	entry->_value->recordPageCacheUse();
	return entry->_value;
}

void Gfx::clearZoombiniAnimationCache() {
	for (ZoombiniAnimationCache::iterator entry = _zoombiniAnimationCache.begin(); entry != _zoombiniAnimationCache.end(); entry++)
		delete entry->_value;
	_zoombiniAnimationCache.clear();
	_commonZoombiniAnimations.clear();
}

void Gfx::clearPageLayers() {
	if (_pageLayerStack)
		_pageLayerStack->clear();
}

RleBlock *Gfx::loadRleBlock(RleBlockCache &cache, const Common::String &key) {
	RleBlockCache::const_iterator cached = cache.find(key);
	if (cached != cache.end()) {
		if (cached->_value)
			cached->_value->recordPageCacheUse();
		return cached->_value;
	}

	RleBlock *sprite = new RleBlock(_vm);
	const bool loaded = sprite->loadFromFile(Common::Path(key));
	if (!loaded) {
		delete sprite;
		sprite = nullptr;
	}
	cache[key] = sprite;
	if (sprite)
		sprite->recordPageCacheUse();
	return sprite;
}

BitBlock *Gfx::loadBitBlock(BitBlockCache &cache, const Common::String &key) {
	BitBlockCache::const_iterator cached = cache.find(key);
	if (cached != cache.end()) {
		if (cached->_value)
			cached->_value->recordPageCacheUse();
		return cached->_value;
	}

	BitBlock *bitmap = new BitBlock(_vm);
	const bool loaded = bitmap->load(Common::Path(key));
	if (!loaded) {
		delete bitmap;
		bitmap = nullptr;
	}
	cache[key] = bitmap;
	if (bitmap)
		bitmap->recordPageCacheUse();
	return bitmap;
}

BitBlock *Gfx::loadMaskedBitBlock(const Common::String &colorKey, const Common::String &alphaKey) {
	for (uint i = 0; i < _pageMaskedBitBlocks.size(); i++) {
		const MaskedBitBlockEntry &entry = _pageMaskedBitBlocks[i];
		if (entry.colorPath.equalsIgnoreCase(colorKey) && entry.alphaPath.equalsIgnoreCase(alphaKey)) {
			entry.bitmap->recordPageCacheUse();
			return entry.bitmap;
		}
	}

	const Common::Path colorFilePath(colorKey);
	const Common::Path alphaFilePath(alphaKey);
	BitBlock *bitmap = new BitBlock(_vm);
	if (!bitmap->loadFromColorAlphaBMP(colorFilePath, alphaFilePath)) {
		delete bitmap;
		return nullptr;
	}
	const MaskedBitBlockEntry entry = {colorKey, alphaKey, bitmap};
	_pageMaskedBitBlocks.push_back(entry);
	bitmap->recordPageCacheUse();
	return bitmap;
}

void Gfx::clearRleBlockCache(RleBlockCache &cache) {
	for (RleBlockCache::iterator entry = cache.begin(); entry != cache.end(); entry++)
		delete entry->_value;
	cache.clear();
}

void Gfx::clearBitBlockCache(BitBlockCache &cache) {
	for (BitBlockCache::iterator entry = cache.begin(); entry != cache.end(); entry++)
		delete entry->_value;
	cache.clear();
}

void Gfx::clearPageBitmapCache() {
	clearBackground();
	clearRleBlockCache(_pageRleBlocks);
	clearBitBlockCache(_pageBitBlocks);
	for (uint i = 0; i < _pageMaskedBitBlocks.size(); i++)
		delete _pageMaskedBitBlocks[i].bitmap;
	_pageMaskedBitBlocks.clear();
}

ManagedSurface32 *Gfx::createSurface(const Size32 &size) const {
	return new ManagedSurface32(size, _screen->format);
}

void Gfx::captureScreen(ManagedSurface32 *destSurface) const {
	assert(destSurface != nullptr);
	destSurface->copyFrom(*_screen);
}

void Gfx::copyToScreen(const ManagedSurface32 &srcSurface) const {
	_screen->copyFrom(srcSurface);
}

void Gfx::captureScreenRegion(ManagedSurface32 *destSurface, const Common::Rect &srcRect) const {
	assert(destSurface != nullptr);
	destSurface->copyRectToSurface(*_screen, 0, 0, srcRect);
}

void Gfx::copyRegionToScreen(const ManagedSurface32 &srcSurface, const Common::Point &destSurface) const {
	_screen->copyRectToSurface(srcSurface, destSurface.x, destSurface.y, Common::Rect(srcSurface.w, srcSurface.h));
}

void Gfx::drawBitBlock(ManagedSurface32 *destSurface, const BitBlock *bitmap, const Common::Point32 &pos) const {
	if (destSurface && bitmap)
		bitmap->drawToSurface(destSurface, pos);
}

void Gfx::drawBitBlockColorKey(ManagedSurface32 *destSurface, const BitBlock *bitmap, const Common::Point32 &pos) const {
	if (destSurface && bitmap)
		bitmap->drawToSurfaceColorKey(destSurface, pos);
}

Size32 Gfx::getPageBitBlockSize(const Common::String &key) {
	BitBlock *bitmap = loadPageBitBlock(key);
	return bitmap ? bitmap->getSize() : Size32();
}

void Gfx::drawBitBlockSubRect(ManagedSurface32 *destSurface, const BitBlock *bitmap, const Common::Point32 &pos, const Common::Rect &srcRect) const {
	if (destSurface && bitmap)
		bitmap->drawSubRect(destSurface, pos, srcRect);
}

bool Gfx::loadBackground(const Common::String &key) {
	BitBlock bitmap(_vm);
	if (!bitmap.load(Common::Path(key)))
		return false;

	ManagedSurface32 *bgSurface = createSurface(bitmap.getSize());
	bitmap.drawToSurface(bgSurface, Common::Point32(0, 0));
	bitmap.recordPageCacheUse();
	clearBackground();
	_backSurface = bgSurface;
	return true;
}

void Gfx::clearBackground() {
	delete _backSurface;
	_backSurface = nullptr;
}

void Gfx::drawBackground(ManagedSurface32 *destSurface, const Common::Point32 &pos) const {
	if (destSurface && _backSurface)
		destSurface->blitFrom(*_backSurface, pos);
}

void Gfx::drawBackgroundSubRect(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect &srcRect) const {
	if (destSurface && _backSurface)
		destSurface->copyRectToSurface(_backSurface->rawSurface(), pos.x, pos.y, Common::Rect32(srcRect.left, srcRect.top, srcRect.right, srcRect.bottom));
}

void Gfx::drawRleBlock(ManagedSurface32 *destSurface, const RleBlock *sprite, const Common::Point32 &pos) const {
	if (destSurface && sprite)
		sprite->drawToScreen(destSurface, pos, _vm->getAlphaLUT());
}

void Gfx::drawRleBlockColorKey(ManagedSurface32 *destSurface, const RleBlock *sprite, const Common::Point32 &pos,
							   const RGBColor &colorKey) const {
	if (destSurface && sprite)
		sprite->drawToScreenColorKey(destSurface, pos, colorKey, _vm->getAlphaLUT());
}

Size32 Gfx::getPageRleBlockSize(const Common::String &key) {
	RleBlock *sprite = loadPageRleBlock(key);
	return sprite ? sprite->getSize() : Size32();
}

void Gfx::drawRleBlockClipped(ManagedSurface32 *destSurface, const RleBlock *sprite, const Common::Point32 &pos, const Common::Rect32 &clip) const {
	if (destSurface && sprite)
		sprite->drawToScreenClipped(destSurface, pos, clip, _vm->getAlphaLUT());
}

void Gfx::drawAnimationFrame(ManagedSurface32 *destSurface, const Animation *animation, int frameIndex, const Common::Point32 &pos) const {
	if (!destSurface || !animation)
		return;

	drawRleBlock(destSurface, animation->getFrame(frameIndex), pos);
}

void Gfx::drawPageNoseTraitSprite(ManagedSurface32 *destSurface, const Common::String &key, const Common::Point32 &pos, byte value) {
	RGBColor displayColor;
	if (noseColorRGB(_vm->getColorAssistMode(), value, displayColor)) {
		const RleBlock *sprite = loadPageRleBlock(key);
		if (sprite)
			sprite->drawToScreenRecoloredWithGradient(destSurface, pos, displayColor, _vm->getAlphaLUT());
	} else {
		drawPageRleBlock(destSurface, key, pos);
	}
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
	const ColorAssistMode colorAssistMode = animation->getColorAssistMode();
	for (int layer = 0; layer < ZoombiniAnimation::kDim1; layer++) {
		int variant = 0;
		if (0 < layer)
			variant = traits.getValue(static_cast<ZmbTrait::TraitKind>(layer - 1));
		if (variant < 0 || ZoombiniAnimation::kDim2 <= variant)
			continue;
		const int entry = baseIndex + layer * ZoombiniAnimation::kDim2 + variant;
		const int selectedFrame = animation->getFrameCount(entry) == 1 ? 0 : frame;
		const RleBlock *sprite = animation->getFrame(entry, selectedFrame);
		if (!sprite)
			continue;
		RGBColor displayColor;
		if (layer == 2 && noseColorRGB(colorAssistMode, traits._nose, displayColor))
			sprite->drawToScreenRecoloredWithGradient(screen, pos, displayColor, alphaLUT, clip);
		else if (clip)
			sprite->drawToScreenClipped(screen, pos, *clip, alphaLUT);
		else
			sprite->drawToScreen(screen, pos, alphaLUT);
	}
}

void Gfx::drawZoombiniPreview(ManagedSurface32 *screen, const ZoombiniAnimation *animation,
							  const byte (&selectedValues)[ZmbTrait::kTraitKindCount], const Common::Point32 &pos) const {
	drawZoombiniPreview(screen, animation, selectedValues, pos, _vm->getAlphaLUT());
}

void Gfx::drawZoombiniPreview(ManagedSurface32 *screen, const ZoombiniAnimation *animation,
							  const byte (&selectedValues)[ZmbTrait::kTraitKindCount], const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) {
	if (!screen || !animation)
		return;
	static constexpr int kBaseCell = 990;
	static constexpr int kFeatureCellBases[ZmbTrait::kTraitKindCount] = {
		996,
		1002,
		1008,
		1014,
	};
	static constexpr int kFeatureDrawOrder[ZmbTrait::kTraitKindCount] = {
		0,
		2,
		3,
		1,
	};
	const RleBlock *frame = animation->getFrame(kBaseCell, 0);
	if (frame)
		frame->drawToScreen(screen, pos, alphaLUT);
	for (int i = 0; i < ZmbTrait::kTraitKindCount; i++) {
		const int feature = kFeatureDrawOrder[i];
		const byte value = selectedValues[feature];
		if (1 <= value && value <= ZmbTrait::kTraitValueCount) {
			frame = animation->getFrame(kFeatureCellBases[feature] + value, 0);
			if (frame) {
				RGBColor displayColor;
				if (feature == static_cast<int>(ZmbTrait::TraitKind::kNose01) &&
					noseColorRGB(animation->getColorAssistMode(), value, displayColor))
					frame->drawToScreenRecoloredWithGradient(screen, pos, displayColor, alphaLUT);
				else
					frame->drawToScreen(screen, pos, alphaLUT);
			}
		}
	}
}

bool Gfx::noseColorRGB(ColorAssistMode mode, byte value, RGBColor &color) {
	if (value < 1 || ZmbTrait::kTraitValueCount < value)
		return false;
	switch (mode) {
	case ColorAssistMode::kEnhancedDistinction:
		if (value != 5)
			return false;
		color = RGBColor(167, 108, 212);
		return true;
	case ColorAssistMode::kRedGreenBlindAssist: {
		static constexpr RGBColor kNoseColors[ZmbTrait::kTraitValueCount] = {
			RGBColor(230, 159, 0),
			RGBColor(255, 255, 255),
			RGBColor(0, 182, 217),
			RGBColor(101, 121, 255),
			RGBColor(102, 0, 68),
		};
		color = kNoseColors[value - 1];
		return true;
	}
	case ColorAssistMode::kOriginal:
	default:
		return false;
	}
}

RGBColor Gfx::recolorNoseGradientRGB(const RGBColor &srcColor, const RGBColor &targetColor) {
	const int sourceMaximum = srcColor.getMaxChannel();
	const int sourceMinimum = srcColor.getMinChannel();
	const int targetMaximum = targetColor.getMaxChannel();
	const int targetMinimum = targetColor.getMinChannel();
	if (sourceMaximum == 0 || targetMaximum == 0)
		return RGBColor();
	const int value = sourceMaximum * targetMaximum / 255;
	const int targetRange = targetMaximum - targetMinimum;
	if (targetRange == 0)
		return RGBColor(static_cast<byte>(value), static_cast<byte>(value), static_cast<byte>(value));
	const int sourceSaturation = (sourceMaximum - sourceMinimum) * 255 / sourceMaximum;
	const int targetSaturation = targetRange * 255 / targetMaximum;
	// Keep source value changes and half of its saturation variation in the new hue.
	const int chroma = value * (sourceSaturation + targetSaturation) / 510;
	const int base = value - chroma;
	return RGBColor(static_cast<byte>(base + chroma * (targetColor.r - targetMinimum) / targetRange),
					static_cast<byte>(base + chroma * (targetColor.g - targetMinimum) / targetRange),
					static_cast<byte>(base + chroma * (targetColor.b - targetMinimum) / targetRange));
}

void Gfx::drawZoombiniRunner(ManagedSurface32 *screen, const ZoombiniRunner *runner, const Common::Rect32 *clip, int scrollX, int backgroundWidth) const {
	drawZoombiniRunner(screen, runner, _vm->getAlphaLUT(), clip, scrollX, backgroundWidth, _dropTargetGlowSprite);
}

void Gfx::drawZoombiniRunnerWithTraits(ManagedSurface32 *screen, const ZoombiniRunner *runner, const ZmbTrait &presentationTraits,
									   const Common::Rect32 *clip, int scrollX, int backgroundWidth) const {
	drawZoombiniRunner(screen, runner, _vm->getAlphaLUT(), clip, scrollX, backgroundWidth, _dropTargetGlowSprite, &presentationTraits);
}

void Gfx::drawZoombiniRunner(ManagedSurface32 *screen, const ZoombiniRunner *runner, const AlphaBlendLUT &alphaLUT,
							 const Common::Rect32 *clip, int scrollX, int backgroundWidth, const RleBlock *dropTargetIndicator,
							 const ZmbTrait *presentationTraits) {
	if (!screen || !runner || !runner->getActiveAnimation() || runner->isHidden())
		return;
	const Common::Rect32 spriteRect = runner->getSpriteRect(scrollX, backgroundWidth);
	if (spriteRect.right <= 0 || spriteRect.bottom <= 0 || screen->w <= spriteRect.left || screen->h <= spriteRect.top)
		return;
	const Common::Point32 drawPos = runner->getDrawPosition(scrollX, backgroundWidth);
	if (runner->isDragging() && runner->getHoveredDropTargetIndex() != -1 && dropTargetIndicator)
		dropTargetIndicator->drawToScreen(screen, drawPos, alphaLUT);
	const int frame = runner->isAnimationActive() ? runner->getAnimationFrame() : 0;
	const ZmbTrait &traitsToDraw = presentationTraits ? *presentationTraits : runner->getTraits();
	drawZoombini(screen, runner->getActiveAnimation(), traitsToDraw, drawPos, runner->getAnimationCell(), frame, alphaLUT, clip);
}

int Gfx::drawString(ManagedSurface32 *destSurface, const Common::String &text, const Common::Point32 &pos, int maxWidth, const TextConf &conf) const {
	if (!destSurface)
		return 0;
	if (conf.fontKind == FontKind::kBmtFont) {
		BmtFont *bmtFont = _vm->_text->bmtFont();
		if (!bmtFont)
			return 0;
		return bmtFont->drawString(destSurface, pos, text, maxWidth, conf.fillColor);
	}
	const Common::U32String decoded = TextManager::toU32String(text, _vm->getLanguage());
	TextConf alignedConf = conf;
	alignedConf.hAlign = Graphics::kTextAlignLeft;
	drawString(destSurface, decoded, pos, maxWidth, alignedConf);
	const Graphics::Font *font = _vm->_text->resolveFont(conf.fontKind);
	return font ? font->getStringWidth(decoded) : 0;
}

int Gfx::getTextWidth(const Common::String &text, const TextConf &conf) const {
	if (conf.fontKind == FontKind::kBmtFont) {
		BmtFont *bmtFont = _vm->_text->bmtFont();
		if (!bmtFont)
			return 0;
		return bmtFont->getStringLayoutWidth(text);
	}
	return getTextWidth(TextManager::toU32String(text, _vm->getLanguage()), conf);
}

void Gfx::drawDragNameTooltip(ManagedSurface32 *destSurface, const Common::String &name) {
	if (!destSurface)
		return;
	if (!_nameBoxSprite) {
		_nameBoxSprite = loadSharedRleBlock(kNameBoxSpritePath);
		if (!_nameBoxSprite)
			return;
	}
	BmtFont *bmtFont = _vm->_text->bmtFont();
	if (!bmtFont)
		return;
	static constexpr int kPlateX = 310;
	static constexpr int kPlateY = 565;
	static constexpr int kTextCenterX = 400;
	static constexpr int kTextY = 570;
	drawRleBlock(destSurface, _nameBoxSprite, Common::Point32(kPlateX, kPlateY));
	const int width = bmtFont->getStringLayoutWidth(name);
	const uint32 tint = Gfx::PredefinedColor::kDark00.toPixel(destSurface->format);
	bmtFont->drawString(destSurface, Common::Point32(kTextCenterX - width / 2, kTextY), name, Gfx::kNoMaxWidth, tint);
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

void Gfx::fillSlantedRect(ManagedSurface32 *destSurface, const Common::Rect32 &rect, int slant, uint32 color, bool antialias) const {
	if (!destSurface || rect.width() <= 0 || rect.height() <= 0)
		return;

	// Determine Y range to draw.
	const int firstY = MAX(0, rect.top);
	const int endY = MIN<int>(destSurface->h, rect.bottom);
	if (endY <= firstY)
		return;

	// [*] Non-AA path: Simplely draw a line by scanline.
	if (!antialias) {
		for (int y = firstY; y < endY; y++) {
			const int offset = static_cast<int>((static_cast<int64>(y) - rect.top) * slant / rect.height());
			destSurface->hLine(rect.left + offset, y, rect.right + offset - 1, color);
		}
		return;
	}

	// [*] AA path: Find the leftmost/rightmost X coordinates with subpixel precision, by scanline.
	// Calculate the size of a temporary surface for per-pixel geometric coverage.
	// The shift varies linearly, so checking the first and last rows is enough.
	// Measure shifts from rect.top before clipping, to preserve the original slant regardless of clipping.
	const float firstRowShiftX = (firstY - rect.top) * slant / static_cast<float>(rect.height());
	const float lastRowShiftX = (endY - 1 - rect.top) * slant / static_cast<float>(rect.height());
	const float leftmostX = rect.left + MIN(firstRowShiftX, lastRowShiftX);
	const float rightmostX = rect.right + MAX(firstRowShiftX, lastRowShiftX);
	const int firstX = static_cast<int>(floorf(CLIP(leftmostX, 0.0f, static_cast<float>(destSurface->w))));
	const int endX = static_cast<int>(ceilf(CLIP(rightmostX, 0.0f, static_cast<float>(destSurface->w))));
	const int width = endX - firstX;
	const int height = endY - firstY;
	if (width <= 0)
		return;
	// A surface holds a pixel coverage in a range of 0 ~ 255.
	// For a subpixel granularity, partially covered pixels are represented as partial transparency.
	Graphics::ManagedSurface coverage(width, height, Graphics::PixelFormat::createFormatCLUT8());

	// Draw each scanline with subpixel X granularity.
	for (int y = firstY; y < endY; y++) {
		const float shiftX = (y - rect.top) * slant / static_cast<float>(rect.height());
		const float rawLeftX = rect.left + shiftX;
		const float rawRightX = rect.right + shiftX;
		hLineAA(coverage.surfacePtr(), rawLeftX - firstX, y - firstY, rawRightX - firstX);
	}
	blitColorWithCoverage(destSurface, coverage.surfacePtr(), Common::Point32(firstX, firstY), color);
}

void Gfx::frameRect(ManagedSurface32 *destSurface, const Common::Rect &rect, uint32 color) const {
	if (destSurface)
		destSurface->frameRect(rect, color);
}

void Gfx::drawLine(ManagedSurface32 *destSurface, const Common::Point32 &start, const Common::Point32 &end, uint32 color, int thickness, bool antialias) const {
	if (!destSurface || thickness <= 0)
		return;

	// [*] AA path
	if (antialias) {
		drawLineAA(destSurface, start, end, color, thickness);
		return;
	}

	// [*] Non-AA path
	if (thickness <= 1)
		destSurface->drawLine(start.x, start.y, end.x, end.y, color);
	else
		destSurface->drawThickLine(start.x, start.y, end.x, end.y, thickness, thickness, color);
}

void Gfx::drawLines(ManagedSurface32 *destSurface, const Common::Array<LineSegment> &lines, uint32 color, int thickness, bool antialias) const {
	if (!destSurface || thickness <= 0 || lines.empty())
		return;

	// [*] AA path
	if (antialias) {
		drawLinesAA(destSurface, lines, color, thickness);
		return;
	}

	// [*] Non-AA path
	for (const LineSegment &line : lines)
		drawLine(destSurface, line.start, line.end, color, thickness);
}

void Gfx::hLineAA(Graphics::Surface *coverage, float left, int y, float right) {
	assert(coverage && coverage->format.isCLUT8());
	if (y < 0 || coverage->h <= y || right <= left)
		return;

	// Calculate the size of a target area for per-pixel geometric coverage.
	const int firstX = static_cast<int>(floorf(CLIP(left, 0.0f, static_cast<float>(coverage->w))));
	const int endX = static_cast<int>(ceilf(CLIP(right, 0.0f, static_cast<float>(coverage->w))));
	if (endX <= firstX)
		return;

	// Fill only interior pixels to 255; merge the two boundary pixels with their existing coverage below.
	if (firstX + 2 < endX)
		coverage->hLine(firstX + 1, y, endX - 2, 255);

	// Adding 0.5f rounds nonnegative coverage to the nearest byte value before the cast truncates it.
	const float firstCoverage = MIN(right, static_cast<float>(firstX + 1)) - MAX(left, static_cast<float>(firstX));
	const uint32 firstValue = static_cast<byte>(CLIP(firstCoverage, 0.0f, 1.0f) * 255.0f + 0.5f);
	coverage->setPixel(firstX, y, MAX(coverage->getPixel(firstX, y), firstValue));
	if (firstX + 1 < endX) {
		const float lastCoverage = MIN(right, static_cast<float>(endX)) - MAX(left, static_cast<float>(endX - 1));
		// Adding 0.5f rounds nonnegative coverage to the nearest byte value before the cast truncates it.
		const uint32 lastValue = static_cast<byte>(CLIP(lastCoverage, 0.0f, 1.0f) * 255.0f + 0.5f);
		coverage->setPixel(endX - 1, y, MAX(coverage->getPixel(endX - 1, y), lastValue));
	}
}

void Gfx::vLineAA(Graphics::Surface *coverage, int x, float top, float bottom) {
	assert(coverage && coverage->format.isCLUT8());
	if (x < 0 || coverage->w <= x || bottom <= top)
		return;

	// Calculate the size of a target area for per-pixel geometric coverage.
	const int firstY = static_cast<int>(floorf(CLIP(top, 0.0f, static_cast<float>(coverage->h))));
	const int endY = static_cast<int>(ceilf(CLIP(bottom, 0.0f, static_cast<float>(coverage->h))));
	if (endY <= firstY)
		return;

	// Fill only interior pixels to 255; merge the two boundary pixels with their existing coverage below.
	if (firstY + 2 < endY)
		coverage->vLine(x, firstY + 1, endY - 2, 255);

	// Adding 0.5f rounds nonnegative coverage to the nearest byte value before the cast truncates it.
	const float firstCoverage = MIN(bottom, static_cast<float>(firstY + 1)) - MAX(top, static_cast<float>(firstY));
	const uint32 firstValue = static_cast<byte>(CLIP(firstCoverage, 0.0f, 1.0f) * 255.0f + 0.5f);
	coverage->setPixel(x, firstY, MAX(coverage->getPixel(x, firstY), firstValue));
	if (firstY + 1 < endY) {
		const float lastCoverage = MIN(bottom, static_cast<float>(endY)) - MAX(top, static_cast<float>(endY - 1));
		// Adding 0.5f rounds nonnegative coverage to the nearest byte value before the cast truncates it.
		const uint32 lastValue = static_cast<byte>(CLIP(lastCoverage, 0.0f, 1.0f) * 255.0f + 0.5f);
		coverage->setPixel(x, endY - 1, MAX(coverage->getPixel(x, endY - 1), lastValue));
	}
}

void Gfx::blitColorWithCoverage(ManagedSurface32 *destSurface, const Graphics::Surface *coverage, const Common::Point32 &pos, uint32 color) {
	assert(coverage && coverage->format.isCLUT8());

	byte srcAlpha = 0;
	byte srcRed = 0;
	byte srcGreen = 0;
	byte srcBlue = 0;
	destSurface->format.colorToARGB(color, srcAlpha, srcRed, srcGreen, srcBlue);

	// Keep coverage in an alpha-capable BGRA32 format, regardless of the dest format.
	ManagedSurface32 coloredCoverage(Size32(coverage->w, coverage->h), kScreenFormat);

	// crossBlitMap() uses this table to map each coverage byte to a colored pixel with effective alpha.
	// This conversion table is separate from the coverage surface's palette; no palette is assigned to that surface.
	uint32 colorMap[256] = {};
	for (uint i = 0; i < ARRAYSIZE(colorMap); i++) {
		// Effective srcAlpha = colorAlpha * coverage / 255; RGB stays unchanged.
		// i is coverage (0..255); adding 127 rounds alpha * i / 255 to the nearest integer.
		const byte pixelAlpha = static_cast<byte>((srcAlpha * i + 127) / 255);
		colorMap[i] = coloredCoverage.format.ARGBToColor(pixelAlpha, srcRed, srcGreen, srcBlue);
	}

	// Map each coverage byte to its color and effective alpha using the shared converter.
	Graphics::crossBlitMap(static_cast<byte *>(coloredCoverage.getPixels()), static_cast<const byte *>(coverage->getPixels()),
						   coloredCoverage.pitch, coverage->pitch, coverage->w, coverage->h, coloredCoverage.format.bytesPerPixel, colorMap);

	// Combine the final image to destSurface.
	destSurface->blitFrom(coloredCoverage, pos);
}

void Gfx::drawLineAA(ManagedSurface32 *destSurface, const Common::Point32 &start, const Common::Point32 &end, uint32 color, int thickness) {
	const Common::Array<LineSegment> lines = {LineSegment{start, end}};
	drawLinesAA(destSurface, lines, color, thickness);
}

void Gfx::drawLinesAA(ManagedSurface32 *destSurface, const Common::Array<LineSegment> &lines, uint32 color, int thickness) {
	int32 left = destSurface->w;
	int32 top = destSurface->h;
	int32 right = 0;
	int32 bottom = 0;
	for (const LineSegment &line : lines) {
		left = MIN(left, MIN(line.start.x, line.end.x));
		top = MIN(top, MIN(line.start.y, line.end.y));
		right = MAX(right, MAX(line.start.x, line.end.x) + thickness);
		bottom = MAX(bottom, MAX(line.start.y, line.end.y) + thickness);
	}
	const int32 firstX = MAX(0, left);
	const int32 firstY = MAX(0, top);
	const int32 endX = MIN<int32>(destSurface->w, right);
	const int32 endY = MIN<int32>(destSurface->h, bottom);
	const int32 width = endX - firstX;
	const int32 height = endY - firstY;
	if (width <= 0 || height <= 0)
		return;

	// Every segment contributes to this shared mask; overlaps are blended only once.
	Graphics::ManagedSurface coverage(width, height, Graphics::PixelFormat::createFormatCLUT8());
	const Common::Point32 origin(firstX, firstY);
	for (const LineSegment &line : lines) {
		// Convert one integer segment to coverage-local float coordinates for the shared area calculation.
		const SubpixelLineSegment localSegment = {
			static_cast<float>(line.start.x - firstX),
			static_cast<float>(line.start.y - firstY),
			static_cast<float>(line.end.x - firstX),
			static_cast<float>(line.end.y - firstY),
		};
		rasterizeLineAA(coverage.surfacePtr(), localSegment, thickness);
	}
	blitColorWithCoverage(destSurface, coverage.surfacePtr(), origin, color);
}

float Gfx::integrateClampedCoverage(float start, float end) {
	if (end < start)
		SWAP(start, end);
	if (end <= 0.0f)
		return 0.0f;
	if (1.0f <= start)
		return 1.0f;
	// Without clipping, the ramp's mean is the average of its endpoint heights.
	if (0.0f <= start && end <= 1.0f)
		return (start + end) * 0.5f;
	const float span = end - start;
	if (start < 0.0f) {
		// Crossing zero leaves a triangle; crossing both limits also adds a height-one plateau.
		if (end <= 1.0f)
			return (end / span) * end * 0.5f;
		return (end - 0.5f) / span;
	}
	// Crossing only the upper limit removes a triangle from full coverage.
	const float tail = 1.0f - start;
	return 1.0f - (tail / span) * tail * 0.5f;
}

void Gfx::rasterizeLineAA(Graphics::Surface *coverage, SubpixelLineSegment segment, float thickness) {
	assert(coverage && coverage->format.isCLUT8());
	// Sweep a square pen whose top-left corner follows the straight segment, extending toward +x and +y.
	// steep is true when the absolute Y change exceeds X: swap axes to scan along Y.
	// Otherwise (including equal changes), scan along X; restore the original axes when writing pixels.
	const bool steep = ABS(segment.endX - segment.startX) < ABS(segment.endY - segment.startY);
	if (steep) {
		SWAP(segment.startX, segment.startY);
		SWAP(segment.endX, segment.endY);
	}
	if (segment.endX < segment.startX) {
		SWAP(segment.startX, segment.endX);
		SWAP(segment.startY, segment.endY);
	}
	// major is the scan axis (Y when steep, X otherwise); slope is minor-axis change per major-axis pixel.
	const int majorSize = steep ? coverage->h : coverage->w;
	const int minorSize = steep ? coverage->w : coverage->h;
	const float length = segment.endX - segment.startX;
	const float slope = 0.0f < length ? (segment.endY - segment.startY) / length : 0.0f;
	// Include pen thickness, clip to the coverage surface, then round outwards; majorEnd is exclusive.
	const int majorBegin = static_cast<int>(floorf(CLIP(segment.startX, 0.0f, static_cast<float>(majorSize))));
	const int majorEnd = static_cast<int>(ceilf(CLIP(segment.endX + thickness, 0.0f, static_cast<float>(majorSize))));
	for (int major = majorBegin; major < majorEnd; major++) {
		// Only integrate the part of this pixel column touched by the swept pen.
		const float first = MAX(segment.startX, static_cast<float>(major));
		const float last = MIN(segment.endX + thickness, static_cast<float>(major + 1));
		const float columnWidth = last - first;
		const float firstCorner = CLIP(segment.endX, first, last);
		const float secondCorner = CLIP(segment.startX + thickness, first, last);
		// At the segment's endX and startX + thickness, a boundary switches between an endpoint cap and the sloping segment.
		// Split here so both span boundaries remain linear within each slice.
		float cuts[] = {
			first,
			MIN(firstCorner, secondCorner),
			MAX(firstCorner, secondCorner),
			last,
		};
		// Clipped corners often coincide with first or last; discard duplicate cuts before evaluating boundaries.
		uint cutCount = 1;
		for (uint i = 1; i < ARRAYSIZE(cuts); i++) {
			if (cuts[cutCount - 1] < cuts[i]) {
				cuts[cutCount] = cuts[i];
				cutCount += 1;
			}
		}
		float low[ARRAYSIZE(cuts)] = {};
		float high[ARRAYSIZE(cuts)] = {};
		for (uint i = 0; i < cutCount; i++) {
			// Find the line positions reached by the two major-axis pen edges at this cut.
			// Their order depends on the slope; adding thickness gives the larger minor-coordinate boundary.
			const float firstMinor = segment.startY + MAX(0.0f, cuts[i] - segment.startX - thickness) * slope;
			const float lastMinor = segment.startY + MIN(length, cuts[i] - segment.startX) * slope;
			low[i] = MIN(firstMinor, lastMinor);
			high[i] = MAX(firstMinor, lastMinor) + thickness;
		}
		// Each boundary moves in only one direction, so its endpoint extremes bound all touched rows.
		// Inner bounds identify rows covered over the entire [first,last] interval.
		const float outerLow = MIN(low[0], low[cutCount - 1]);
		const float outerHigh = MAX(high[0], high[cutCount - 1]);
		const float innerLow = MAX(low[0], low[cutCount - 1]);
		const float innerHigh = MIN(high[0], high[cutCount - 1]);
		const int minorBegin = static_cast<int>(floorf(CLIP(outerLow, 0.0f, static_cast<float>(minorSize))));
		const int minorEnd = static_cast<int>(ceilf(CLIP(outerHigh, 0.0f, static_cast<float>(minorSize))));
		const int solidBegin = static_cast<int>(ceilf(CLIP(innerLow, 0.0f, static_cast<float>(minorSize))));
		const int solidEnd = static_cast<int>(floorf(CLIP(innerHigh, 0.0f, static_cast<float>(minorSize))));
		for (int minor = minorBegin; minor < minorEnd; minor++) {
			// Full coverage requires the entire pixel in both axes; writing 255 also preserves MAX merging.
			if (columnWidth == 1.0f && minor == solidBegin && minor < solidEnd) {
				if (steep)
					coverage->hLine(solidBegin, major, solidEnd - 1, 255);
				else
					coverage->vLine(major, solidBegin, solidEnd - 1, 255);
				// Skip the whole span that was just filled; only its surrounding edge pixels need individual work.
				minor = solidEnd - 1;
				continue;
			}
			const int x = steep ? minor : major;
			const int y = steep ? major : minor;
			byte &pixel = *static_cast<byte *>(coverage->getBasePtr(x, y));
			if (pixel == 255)
				continue;
			// A row inside the inner bounds needs only the major-axis overlap as its area.
			float area = columnWidth;
			if (minor < innerLow || innerHigh < minor + 1) {
				area = 0.0f;
				for (uint i = 0; i + 1 < cutCount; i++) {
					// Clamped upper minus lower ramp means give the average height inside this pixel.
					// Multiply by the slice width and sum to integrate its covered area.
					const float upper = integrateClampedCoverage(high[i] - minor, high[i + 1] - minor);
					const float lower = integrateClampedCoverage(low[i] - minor, low[i + 1] - minor);
					area += (cuts[i + 1] - cuts[i]) * (upper - lower);
				}
			}
			// Keep the strongest segment coverage; summing would make overlaps and joins more opaque.
			// Adding 0.5f rounds nonnegative coverage to the nearest byte value before the cast truncates it.
			pixel = MAX(pixel, static_cast<byte>(CLIP(area, 0.0f, 1.0f) * 255.0f + 0.5f));
		}
	}
}

void Gfx::drawCurveLineSegmentsAA(ManagedSurface32 *surface, const Common::Array<SubpixelLineSegment> &lineSegments, uint32 color, int thickness) {
	if (lineSegments.empty())
		return;
	// The pen extends each endpoint toward +x and +y by thickness pixels.
	float left = surface->w;
	float top = surface->h;
	float right = 0.0f;
	float bottom = 0.0f;
	for (const SubpixelLineSegment &segment : lineSegments) {
		left = MIN(left, MIN(segment.startX, segment.endX));
		top = MIN(top, MIN(segment.startY, segment.endY));
		right = MAX(right, MAX(segment.startX, segment.endX) + thickness);
		bottom = MAX(bottom, MAX(segment.startY, segment.endY) + thickness);
	}
	// Round outwards and clip to the destination before allocating a one-byte coverage mask.
	const int firstX = static_cast<int>(floorf(CLIP(left, 0.0f, static_cast<float>(surface->w))));
	const int firstY = static_cast<int>(floorf(CLIP(top, 0.0f, static_cast<float>(surface->h))));
	const int endX = static_cast<int>(ceilf(CLIP(right, 0.0f, static_cast<float>(surface->w))));
	const int endY = static_cast<int>(ceilf(CLIP(bottom, 0.0f, static_cast<float>(surface->h))));
	const int width = endX - firstX;
	const int height = endY - firstY;
	if (width <= 0 || height <= 0)
		return;
	Graphics::ManagedSurface coverage(width, height, Graphics::PixelFormat::createFormatCLUT8());
	// The mask's (0,0) corresponds to destination pixel (firstX,firstY).
	// Subtract that origin here and add it back when compositing the mask onto the destination.
	for (const SubpixelLineSegment &segment : lineSegments) {
		const SubpixelLineSegment localSegment = {
			segment.startX - firstX,
			segment.startY - firstY,
			segment.endX - firstX,
			segment.endY - firstY,
		};
		rasterizeLineAA(coverage.surfacePtr(), localSegment, thickness);
	}
	// Composite the merged coverage once so shared pixels and joins retain the stroke's opacity.
	blitColorWithCoverage(surface, coverage.surfacePtr(), Common::Point32(firstX, firstY), color);
}

void Gfx::drawCubicBezierCurve(ManagedSurface32 *destSurface, CubicBezierCurve &curve, uint32 color, int thickness, bool antialias) const {
	if (!destSurface)
		return;
	const Common::Rect32 sampleBounds = Common::Rect32(0, 0, destSurface->w, destSurface->h);
	drawCubicBezierCurve(destSurface, curve, color, thickness, sampleBounds, antialias);
}

void Gfx::drawCubicBezierCurve(ManagedSurface32 *destSurface, CubicBezierCurve &curve, uint32 color, int thickness, const Common::Rect32 &sampleBounds, bool antialias) const {
	if (!destSurface || thickness <= 0)
		return;
	// Sample on the evaluator's five-millisecond tick, with a cap on work for one draw.
	static constexpr uint32 kEvaluationInterval = 5;
	static constexpr uint32 kMaxEvaluationCount = 1024;
	curve.computeCoeffs();
	curve.setStartTime(0);
	// Capture straight segments from evaluated positions; the curve keeps its control points and playback state.
	Common::Array<SubpixelLineSegment> lineSegments;
	// Integer positions decide which endpoint pairs pass the bounds check; floats retain AA geometry.
	Common::Point32 prevIntPos = curve.getStartPosition();
	float prevX = prevIntPos.x;
	float prevY = prevIntPos.y;
	bool completed = false;
	for (uint32 sample = 0; sample < kMaxEvaluationCount; sample++) {
		Common::Point32 current;
		if (!curve.evaluate(sample * kEvaluationInterval, current)) {
			completed = true;
			break;
		}
		float currentX = 0.0f;
		float currentY = 0.0f;
		if (antialias)
			curve.getSubpixelPosition(currentX, currentY);
		// Accept the same endpoint pairs as integer drawing; clipping happens later in the rasterizer.
		if (sampleBounds.contains(prevIntPos) && sampleBounds.contains(current)) {
			if (antialias) {
				lineSegments.push_back({prevX, prevY, currentX, currentY});
			} else {
				drawLine(destSurface, prevIntPos, current, color, thickness);
			}
		}
		prevIntPos = current;
		prevX = currentX;
		prevY = currentY;
	}
	// Sampling need not land on the authored endpoint, so finish with it after playback completes.
	// Do not append it when sampling stopped at the iteration limit.
	if (completed) {
		const Common::Point32 end = curve.getEndPosition();
		if (sampleBounds.contains(prevIntPos) && sampleBounds.contains(end)) {
			if (antialias) {
				lineSegments.push_back(SubpixelLineSegment{prevX, prevY, static_cast<float>(end.x), static_cast<float>(end.y)});
			} else {
				drawLine(destSurface, prevIntPos, end, color, thickness);
			}
		}
	}
	if (antialias)
		drawCurveLineSegmentsAA(destSurface, lineSegments, color, thickness);
}

void Gfx::drawString(ManagedSurface32 *destSurface, const Common::U32String &text, const Common::Point32 &pos, int maxWidth, const TextConf &conf) const {
	if (!destSurface)
		return;
	if (conf.fontKind == FontKind::kBmtFont) {
		drawString(destSurface, TextManager::fromU32String(text, _vm->getLanguage()), pos, maxWidth, conf);
		return;
	}
	const int effectiveMaxWidth = maxWidth == kNoMaxWidth ? destSurface->w : maxWidth;

	// Resolve the shared UI font inside the graphics layer so callers never hold font objects.
	const Graphics::Font *font = _vm->_text->resolveFont(conf.fontKind);
	if (!font)
		return;

	// Draw the four one-pixel outline passes before the main text, if requested to.
	if (conf.outlineEffect) {
		static constexpr Common::Point32 kOutlineOffsets[4] = {
			Common::Point32(-1, 0),
			Common::Point32(1, 0),
			Common::Point32(0, -1),
			Common::Point32(0, 1),
		};
		for (int i = 0; i < ARRAYSIZE(kOutlineOffsets); i++) {
			const Common::Point32 offset = kOutlineOffsets[i];
			font->drawString(destSurface, text, pos.x + offset.x, pos.y + offset.y, effectiveMaxWidth, conf.outlineColor, conf.hAlign);
		}
	}

	// Draw the main text.
	font->drawString(destSurface, text, pos.x, pos.y, effectiveMaxWidth, conf.fillColor, conf.hAlign);
}

int Gfx::getTextWidth(const Common::U32String &text, const TextConf &conf) const {
	if (conf.fontKind == FontKind::kBmtFont)
		return getTextWidth(TextManager::fromU32String(text, _vm->getLanguage()), conf);
	const Graphics::Font *font = _vm->_text->resolveFont(conf.fontKind);
	return font ? font->getStringWidth(text) : 0;
}

int Gfx::getFontHeight(const TextConf &conf) const {
	if (conf.fontKind == FontKind::kBmtFont && !_vm->_text->loadTextFont())
		return 0;
	const Graphics::Font *font = _vm->_text->resolveFont(conf.fontKind);
	return font ? font->getFontHeight() : 0;
}

void Gfx::wordWrapText(const Common::U32String &text, int maxWidth, Common::Array<Common::U32String> &lines, const TextConf &conf) const {
	if (conf.fontKind == FontKind::kBmtFont) {
		lines.clear();
		if (!_vm->_text->loadTextFont())
			return;
		BmtFont *bmtFont = _vm->_text->bmtFont();
		if (!bmtFont)
			return;
		Common::Array<Common::String> encodedLines;
		bmtFont->wordWrapText(TextManager::fromU32String(text, _vm->getLanguage()), maxWidth, encodedLines);
		for (uint i = 0; i < encodedLines.size(); i++)
			lines.push_back(TextManager::toU32String(encodedLines[i], _vm->getLanguage()));
		return;
	}
	const Graphics::Font *font = _vm->_text->resolveFont(conf.fontKind);
	if (!font) {
		lines.clear();
		return;
	}
	font->wordWrapText(text, maxWidth, lines);
}

// ============================================================================
// Animation
// ============================================================================

Animation::Animation(Zoombini2Engine *vm)
	: _vm(vm) {
}

Animation::~Animation() {
	for (uint i = 0; i < _frames.size(); i++)
		delete _frames[i];
}

/** Load one AN frame sequence described by @ref Animation. */
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

AreaMask::AreaMask(Zoombini2Engine *vm)
	: _vm(vm) {
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
	if (stream.size() < pixelOffset + static_cast<int64>(stride) * height) {
		warning("AreaMask: truncated 1-bit bitmap '%s'", path.toString().c_str());
		stream.seek(0);
		return false;
	}
	Common::Array<byte> pixels;
	pixels.resize(static_cast<uint32>(pixelCount));
	for (int32 row = 0; row < height; row++) {
		const int32 srcRow = bottomUp ? height - 1 - row : row;
		stream.seek(static_cast<int64>(pixelOffset) + static_cast<int64>(srcRow) * stride);
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
		if (!_vm->isDemo())
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

ZoombiniAnimation::ZoombiniAnimation(Zoombini2Engine *vm)
	: _vm(vm) {
}

ZoombiniAnimation::~ZoombiniAnimation() {
}

bool ZoombiniAnimation::loadFromFile(const Common::Path &path) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->openResourceFile(path.toString('/')));
	if (!stream) {
		if (!_vm->isDemo())
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

	for (int i = 0; i < kCellCount; i++)
		_cells[i].frames.swap(loadedCells[i].frames);
	_resourcePath = path;

	debug(2, "ZoombiniAnimation: loaded %d of %d cells and %u frames from '%s'%s", cellIndex, kCellCount, recoveredFrameCount,
		  path.toString().c_str(), complete ? "" : " with salvage warnings");
	return true;
}

void ZoombiniAnimation::recordPageCacheUse() const {
	_vm->recordPageResourceCacheUse(_resourcePath.toString('/'), Zoombini2Engine::PageResourceType::kZoombiniAnimation);
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

ColorAssistMode ZoombiniAnimation::getColorAssistMode() const {
	return _vm->getColorAssistMode();
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
	// Images are shared through the graphics page cache and released by @ref Gfx::clearPageBitmapCache.
	// Drop only this button's borrowed references so the cache and other buttons keep valid images.
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

	const Common::String key = path.toString('/');
	bitmap = _vm->_gfx->loadPageBitBlock(key);
	if (bitmap)
		return true;

	rle = _vm->_gfx->loadPageRleBlock(key);
	if (rle)
		return true;
	return false;
}

bool UIButton::loadMaskedImage(const Common::Path &colorPath, const Common::Path &maskPath, BitBlock *&bitmap, RleBlock *&rle) {
	if (colorPath.empty() && maskPath.empty())
		return true;
	if (colorPath.empty() || maskPath.empty())
		return false;

	const Common::String colorKey = colorPath.toString('/');
	rle = _vm->_gfx->loadPageRleBlock(colorKey);
	if (rle)
		return true;

	bitmap = _vm->_gfx->loadPageMaskedBitBlock(colorKey, maskPath.toString('/'));
	if (bitmap)
		return true;
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
		const Common::Rect32 clip(0, 0, *_overlayClipRight, MIN(static_cast<int>(dst->h), Gfx::kScreenSize.height));
		_overlay->drawToScreenClipped(dst, Common::Point32(_rect.left + _overlayOffset.x, _rect.top + _overlayOffset.y), clip, alphaLUT);
	}

	if (_isHovering)
		return _wasHovering ? 1 : 2;
	return 0;
}

// ============================================================================
// BitmapFont - bitmap-based font for UI text rendering.
// ============================================================================

BmtFont::BmtFont(Zoombini2Engine *vm)
	: _vm(vm) {
}

bool BmtFont::load(const Common::Path &basePath) {
	for (int i = 0; i < kNumGlyphs; i++) {
		delete[] _glyphs[i].mask;
		_glyphs[i].mask = nullptr;
		_glyphs[i].width = 0;
		_glyphs[i].height = 0;
	}
	_loaded = false;
	_maxCharWidth = kSpaceWidth;

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

		if (size.width <= col)
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
		if (_maxCharWidth < glyphWidth + 2)
			_maxCharWidth = glyphWidth + 2;
		glyph.mask = new byte[glyphWidth * size.height]();
		const int copyWidth = MIN(glyphWidth, size.width - startCol);
		for (int row = 0; row < size.height; row++) {
			memcpy(glyph.mask + row * glyphWidth, srcAlpha + row * size.width + startCol, copyWidth);
		}

		glyphIndex += 1;
	}

	int expectedGlyphs = 81;
	switch (_vm->getLanguage()) {
	case Common::HE_ISR:
		expectedGlyphs = 80;
		break;
	case Common::SV_SWE:
		expectedGlyphs = 84;
		break;
	default:
		break;
	}
	if (glyphIndex != expectedGlyphs) {
		warning("BitmapFont: expected %d glyphs but found %d in '%s'", expectedGlyphs, glyphIndex, colorPath.toString().c_str());
		return false;
	}

	_loaded = true;
	debug(1, "BitmapFont: Loaded %d glyphs from %s", glyphIndex, basePath.toString().c_str());

	return true;
}

int BmtFont::charToGlyphIndex(uint32 character) const {
	if (255 < character)
		return -1;
	if ('A' <= character && character <= 'Z')
		return character - 'A';
	if ('a' <= character && character <= 'z')
		return character - 'a' + 26;
	if ('0' <= character && character <= '9')
		return character - '0' + 52;

	switch (_vm->getLanguage()) {
	case Common::HE_ISR:
		if (0xE0 <= character && character <= 0xFA) {
			static constexpr byte kHebrewGlyphs[] = {
				45,
				28,
				29,
				44,
				47,
				46,
				51,
				35,
				50,
				33,
				37,
				31,
				36,
				40,
				39,
				34,
				27,
				49,
				32,
				64,
				41,
				62,
				38,
				30,
				43,
				26,
				63,
			};
			return kHebrewGlyphs[character - 0xE0];
		}
		break;
	case Common::SV_SWE: {
		static constexpr byte kSwedishCharacters[] = {
			0xC6,
			0xD8,
			0xC5,
			0xC4,
			0xD6,
			0xE6,
			0xF8,
			0xE5,
			0xE4,
			0xF6,
		};
		for (int i = 0; i < ARRAYSIZE(kSwedishCharacters); i++) {
			if (character == kSwedishCharacters[i])
				return 62 + i;
		}
		return -1;
	}
	default:
		break;
	}

	if (character == '_')
		return 80;
	return asciiPunctuationToGlyphIndex(character);
}

int BmtFont::asciiPunctuationToGlyphIndex(uint32 character) {
	static constexpr char kPunctuationCharacters[] = ".,;:/()-+=@&#'?!*";
	if (character == 0 || 255 < character)
		return -1;
	const char *entry = strchr(kPunctuationCharacters, character);
	if (!entry)
		return -1;
	return 62 + static_cast<int>(entry - kPunctuationCharacters);
}

int BmtFont::getFontHeight() const {
	return _loaded ? _glyphs[0].height : 0;
}

int BmtFont::getMaxCharWidth() const {
	return _loaded ? _maxCharWidth : 0;
}

int BmtFont::getCharWidth(uint32 character) const {
	if (!_loaded)
		return 0;
	if (character == ' ')
		return kSpaceWidth;
	const int glyphIndex = charToGlyphIndex(character);
	if (glyphIndex < 0 || kNumGlyphs <= glyphIndex || !_glyphs[glyphIndex].mask)
		return 0;
	return _glyphs[glyphIndex].width + 2;
}

Common::Rect BmtFont::getBoundingBox(uint32 character) const {
	if (!_loaded)
		return Common::Rect();
	const int glyphIndex = charToGlyphIndex(character);
	if (glyphIndex < 0 || kNumGlyphs <= glyphIndex || !_glyphs[glyphIndex].mask)
		return Common::Rect();
	return Common::Rect(_glyphs[glyphIndex].width, _glyphs[glyphIndex].height);
}

void BmtFont::drawChar(Graphics::Surface *dst, uint32 character, int x, int y, uint32 color) const {
	if (!dst || !_loaded)
		return;
	const int glyphIndex = charToGlyphIndex(character);
	if (glyphIndex < 0 || kNumGlyphs <= glyphIndex || !_glyphs[glyphIndex].mask)
		return;
	const RGBColor tint = RGBColor::fromPixel(color, dst->format);
	drawGlyph(dst, _glyphs[glyphIndex], Common::Point32(x, y), tint, _vm->getAlphaLUT());
}

void BmtFont::drawGlyph(Graphics::Surface *dst, const Glyph &glyph, const Common::Point32 &pos, const RGBColor &color, const AlphaBlendLUT &alphaLUT) const {
	if (!glyph.mask)
		return;
	assert(dst->format.bytesPerPixel == 4);
	const uint32 solidPixel = color.toPixel(dst->format);

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
				WRITE_UINT32(destPtr, solidPixel);
			} else {
				// The tint is uniform across the glyph, so scale it by the
				// coverage and scale the destination by the coverage's inverse.
				const byte invAlpha = 255 - mask;
				const RGBColor destColor = RGBColor::fromPixel(READ_UINT32(destPtr), dst->format);
				const RGBColor blendedColor = alphaLUT.blend(color, destColor, mask, invAlpha);
				WRITE_UINT32(destPtr, blendedColor.toPixel(dst->format));
			}
		}
	}
}

int BmtFont::drawString(ManagedSurface32 *dst, const Common::Point32 &pos, const Common::String &text, int maxWidth, uint32 color) const {
	if (!dst || !_loaded)
		return 0;
	const bool hasWidthLimit = maxWidth != Gfx::kNoMaxWidth;
	const int effectiveMaxWidth = hasWidthLimit ? maxWidth : dst->w;
	if (!_vm->isHebrew()) {
		Graphics::Font::drawString(dst, text, pos.x, pos.y, effectiveMaxWidth, color, Graphics::kTextAlignLeft, 0, false, !hasWidthLimit);
		return Graphics::Font::getStringWidth(text);
	}

	const int rightX = pos.x + effectiveMaxWidth + 1;
	int curX = pos.x;
	// Hebrew visits game bytes backwards; only digits move the pen left.
	for (uint i = 0; i < text.size(); i++) {
		const byte c = static_cast<byte>(text[text.size() - i - 1]);
		if (hasWidthLimit && rightX < curX + getBoundingBox(c).right)
			break;
		drawChar(dst, c, curX, pos.y, color);
		const int advance = getCharWidth(c);
		if ('0' <= c && c <= '9')
			curX -= advance;
		else
			curX += advance;
	}

	return curX - pos.x;
}

int BmtFont::getStringLayoutWidth(const Common::String &text) const {
	if (!_loaded)
		return 0;
	const bool hebrew = _vm->isHebrew();
	const bool swedish = _vm->isSwedish();
	if (!hebrew && !swedish)
		return Graphics::Font::getStringWidth(text);

	int width = 0;
	// Game measurement deliberately differs from drawing in these two releases.
	for (uint i = 0; i < text.size(); i++) {
		const byte c = static_cast<byte>(text[i]);
		if (swedish && 0x80 <= c)
			continue;
		int advance = getCharWidth(c);
		if (swedish) {
			// Punctuation contributes its ASCII-slot advance even though the Swedish drawing map rejects it.
			const int glyphIndex = asciiPunctuationToGlyphIndex(c);
			if (0 <= glyphIndex && glyphIndex < kNumGlyphs && _glyphs[glyphIndex].mask)
				advance = _glyphs[glyphIndex].width + 2;
		}
		if (hebrew && (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')))
			width -= advance;
		else if (!hebrew || (c < 0x80 && c != '_'))
			width += advance;
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
	for (int i = 0; i < ARRAYSIZE(_sliderLabels); i++)
		_sliderLabels[i].setOverlay(nullptr, Common::Point32(), nullptr);
	if (_soundManager) {
		if (0 <= _speechPreviewSoundId)
			_soundManager->unload(_speechPreviewSoundId);
		if (0 <= _sfxPreviewSoundId)
			_soundManager->unload(_sfxPreviewSoundId);
	}
}

VolumePanel::SliderKind VolumePanel::getSliderKind(int labelIndex) {
	return static_cast<SliderKind>(kSlider_Music02 + labelIndex);
}

UIButton &VolumePanel::getSliderLabel(SliderKind sliderKind) {
	return _sliderLabels[sliderKind - kSlider_Music02];
}

bool VolumePanel::init(SoundManager *soundManager) {
	_soundManager = soundManager;
	_gaugeImage = _vm->_gfx->loadPageRleBlock(kGaugeImagePath);
	bool loaded = true;
	if (!_gaugeImage) {
		warning("VolumePanel: Failed to load gauge image");
		loaded = false;
	}

	_okButton.setRect(Common::Point32(294, 487), Size32(76, 74));
	if (!_okButton.loadImagesWithMask(Common::Path(kOkNormalPath), Common::Path(kOkNormalMaskPath), Common::Path(kOkHighlightPath), Common::Path(kOkHighlightMaskPath)))
		loaded = false;

	_noButton.setRect(Common::Point32(468, 487), Size32(76, 74));
	if (!_noButton.loadImagesWithMask(Common::Path(kNoNormalPath), Common::Path(kNoNormalMaskPath), Common::Path(kNoHighlightPath), Common::Path(kNoHighlightMaskPath)))
		loaded = false;

	UIButton &musicLabel = getSliderLabel(kSlider_Music02);
	musicLabel.setRect(Common::Point32(kLabelX, kMusicLabelY), kLabelSize);
	const Common::Path musicMask(kMusicMaskPath);
	if (!musicLabel.loadImagesWithMask(Common::Path(kMusicNormalPath), musicMask, Common::Path(kMusicHighlightPath), musicMask))
		loaded = false;

	UIButton &sfxLabel = getSliderLabel(kSlider_Sfx03);
	sfxLabel.setRect(Common::Point32(kLabelX, kSfxLabelY), kLabelSize);
	const Common::Path sfxMask(kSfxMaskPath);
	if (!sfxLabel.loadImagesWithMask(Common::Path(kSfxNormalPath), sfxMask, Common::Path(kSfxHighlightPath), sfxMask))
		loaded = false;

	UIButton &speechLabel = getSliderLabel(kSlider_Speech04);
	speechLabel.setRect(Common::Point32(kLabelX, kSpeechLabelY), kLabelSize);
	const Common::Path speechMask(kSpeechMaskPath);
	if (!speechLabel.loadImagesWithMask(Common::Path(kSpeechNormalPath), speechMask, Common::Path(kSpeechHighlightPath), speechMask))
		loaded = false;

	_musicSliderX = volumeToPixel(_settings.getMusic());
	_sfxSliderX = volumeToPixel(_settings.getSfx());
	_speechSliderX = volumeToPixel(_settings.getSpeech());
	musicLabel.setOverlay(_gaugeImage, Common::Point32(kSliderMinX - kLabelX, kMusicGaugeY - kMusicLabelY), &_musicSliderX);
	sfxLabel.setOverlay(_gaugeImage, Common::Point32(kSliderMinX - kLabelX, kSfxGaugeY - kSfxLabelY), &_sfxSliderX);
	speechLabel.setOverlay(_gaugeImage, Common::Point32(kSliderMinX - kLabelX, kSpeechGaugeY - kSpeechLabelY), &_speechSliderX);
	if (_soundManager) {
		_speechPreviewSoundId = _soundManager->load(false, Common::Path(kSpeechPreviewSoundPath), false);
		_sfxPreviewSoundId = _soundManager->load(false, Common::Path(kSfxPreviewSoundPath), false);
	}

	return loaded;
}

int VolumePanel::pixelToVolume(int x) {
	// Convert the 288-pixel gauge range to a percentage.
	int vol = (100 * (x - kSliderMinX)) / kSliderRange;
	if (vol < 0)
		vol = 0;
	if (100 < vol)
		vol = 100;
	return vol;
}

int VolumePanel::volumeToPixel(int volume) {
	return (kSliderRange * volume) / 100 + kSliderMinX;
}

void VolumePanel::setMusicVolume(int vol) {
	_settings.setMusic(vol);
	_musicSliderX = volumeToPixel(_settings.getMusic());
}

void VolumePanel::setSfxVolume(int vol) {
	_settings.setSfx(vol);
	_sfxSliderX = volumeToPixel(_settings.getSfx());
}

void VolumePanel::setSpeechVolume(int vol) {
	_settings.setSpeech(vol);
	_speechSliderX = volumeToPixel(_settings.getSpeech());
}

void VolumePanel::setInitialVolumes(int music, int sfx, int speech) {
	_settings.setInitialVolumes(music, sfx, speech);
	_musicSliderX = volumeToPixel(_settings.getMusic());
	_sfxSliderX = volumeToPixel(_settings.getSfx());
	_speechSliderX = volumeToPixel(_settings.getSpeech());
}

VolumePanelResult VolumePanel::handleMouseInput(const Common::Point32 &mousePos, bool mouseDown, bool mouseReleased) {
	Common::Point32 effectivePos = mousePos;
	if (mouseDown) {
		_heldMousePos = mousePos;
		_hasHeldMouse = true;
		SliderKind hoveredSlider = kSlider_None00;
		for (int i = 0; i < ARRAYSIZE(_sliderLabels); i++) {
			if (_sliderLabels[i].containsPoint(mousePos)) {
				hoveredSlider = getSliderKind(i);
				break;
			}
		}
		if (hoveredSlider != kSlider_None00) {
			_activeSlider = hoveredSlider;
			switch (hoveredSlider) {
			case kSlider_Music02:
				_musicSliderX = mousePos.x;
				break;
			case kSlider_Sfx03:
				_sfxSliderX = mousePos.x;
				break;
			case kSlider_Speech04:
				_speechSliderX = mousePos.x;
				break;
			default:
				break;
			}
		}
	} else {
		SliderKind releasedSlider = _activeSlider;
		_activeSlider = kSlider_None00;
		if (mouseReleased) {
			if (_hasHeldMouse)
				effectivePos = _heldMousePos;
			_hasHeldMouse = false;
			if (releasedSlider == kSlider_None00) {
				for (int i = 0; i < ARRAYSIZE(_sliderLabels); i++) {
					if (_sliderLabels[i].containsPoint(effectivePos)) {
						releasedSlider = getSliderKind(i);
						break;
					}
				}
			}
			if (releasedSlider != kSlider_None00) {
				const int newVolume = pixelToVolume(effectivePos.x);
				switch (releasedSlider) {
				case kSlider_Music02:
					setMusicVolume(newVolume);
					break;
				case kSlider_Sfx03:
					setSfxVolume(newVolume);
					break;
				case kSlider_Speech04:
					setSpeechVolume(newVolume);
					break;
				default:
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
	const SliderKind adjustedSlider = _lastAdjustedSlider;
	_lastAdjustedSlider = kSlider_None00;
	if (!_soundManager)
		return;

	switch (adjustedSlider) {
	case kSlider_Sfx03:
		if (0 <= _sfxPreviewSoundId)
			_soundManager->playWithVolume(_sfxPreviewSoundId, _settings.getSfx());
		break;
	case kSlider_Speech04:
		if (0 <= _speechPreviewSoundId) {
			_soundManager->stop(_speechPreviewSoundId);
			_soundManager->playWithVolume(_speechPreviewSoundId, _settings.getSpeech());
		}
		break;
	default:
		break;
	}
}

void VolumePanel::draw(ManagedSurface32 *dst, const Common::Point32 &mousePos, const AlphaBlendLUT &alphaLUT) {
	for (int i = 0; i < ARRAYSIZE(_sliderLabels); i++)
		_sliderLabels[i].drawAndHitTest(dst, mousePos, alphaLUT);
	_okButton.drawAndHitTest(dst, mousePos, alphaLUT);
	_noButton.drawAndHitTest(dst, mousePos, alphaLUT);
}

} // End of namespace Zoombini2
