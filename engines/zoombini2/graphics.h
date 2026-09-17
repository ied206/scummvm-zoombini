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
#include "common/noncopyable.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"

#include "graphics/managed_surface.h"

#include "zoombini2/state.h"

namespace Zoombini2 {

enum PageId : int;
enum class RouteBranch : int;

/** Signed width and height without positional semantics. */
template<typename T, typename ConcreteSize>
struct SizeBase {
	T width;
	T height;

	constexpr SizeBase() : width(0), height(0) {}
	constexpr SizeBase(T widthValue, T heightValue) : width(widthValue), height(heightValue) {}

	/** Create a size by adding the @p delta dimensions to this size. */
	ConcreteSize operator+(const ConcreteSize &delta) const {
		return ConcreteSize(static_cast<T>(width + delta.width), static_cast<T>(height + delta.height));
	}
	/** Create a size by subtracting the @p delta dimensions from this size. */
	ConcreteSize operator-(const ConcreteSize &delta) const {
		return ConcreteSize(static_cast<T>(width - delta.width), static_cast<T>(height - delta.height));
	}
	/** Create a size by dividing both dimensions by the (int) @p divisor. */
	ConcreteSize operator/(int divisor) const {
		return ConcreteSize(static_cast<T>(width / divisor), static_cast<T>(height / divisor));
	}
	/** Create a size by multiplying both dimensions by the (int) @p multiplier. */
	ConcreteSize operator*(int multiplier) const {
		return ConcreteSize(static_cast<T>(width * multiplier), static_cast<T>(height * multiplier));
	}
	/** Create a size by dividing both dimensions by the (double) @p divisor. */
	ConcreteSize operator/(double divisor) const {
		return ConcreteSize(static_cast<T>(width / divisor), static_cast<T>(height / divisor));
	}
	/** Create a size by multiplying both dimensions by the (double) @p multiplier. */
	ConcreteSize operator*(double multiplier) const {
		return ConcreteSize(static_cast<T>(width * multiplier), static_cast<T>(height * multiplier));
	}

	/** Change this size by adding the @p delta dimensions. */
	void operator+=(const ConcreteSize &delta) {
		width += delta.width;
		height += delta.height;
	}

	/** Change this size by subtracting the @p delta dimensions. */
	void operator-=(const ConcreteSize &delta) {
		width -= delta.width;
		height -= delta.height;
	}
};

/**
 * Old GCC does not support constructor inheritance.
 */
#define BEGIN_SIZE_TYPE(T, Size) \
	struct Size : public SizeBase<T, Size> {
#define END_SIZE_TYPE(T, Size)                                                                          \
	constexpr Size() : SizeBase() {}                                                                    \
	constexpr Size(T widthValue, T heightValue) : SizeBase(widthValue, heightValue) {}                  \
	}                                                                                                   \
	;                                                                                                   \
	static inline Size operator*(int multiplier, const Size &size) {                                    \
		return Size(static_cast<T>(size.width * multiplier), static_cast<T>(size.height * multiplier)); \
	}                                                                                                   \
	static inline Size operator*(double multiplier, const Size &size) {                                 \
		return Size(static_cast<T>(size.width * multiplier), static_cast<T>(size.height * multiplier)); \
	}

BEGIN_SIZE_TYPE(int16, Size16)
END_SIZE_TYPE(int16, Size16)
BEGIN_SIZE_TYPE(int32, Size32)
constexpr Size32(const Size16 &size) : SizeBase(static_cast<int32>(size.width), static_cast<int32>(size.height)) {}
END_SIZE_TYPE(int32, Size32)

class Zoombini2Engine;
class SoundManager;
class Animation;
class AnimationRunner;
class AreaMask;
class BitBlock;
class BitmapFont;
class ManagedSurface32;
class PageLayerStack;
class RleBlock;
class ZoombiniRunner;
class ZoombiniAnimation;

/** Provides page-facing operations for composed game graphics. */
class Gfx {
public:
	/** Construct the graphics interface for one game instance. */
	explicit Gfx(Zoombini2Engine *vm);
	/** Release the graphics interface for one game instance. */
	~Gfx();

	/** Create a managed surface in the current game screen format. */
	ManagedSurface32 *createSurface(const Size32 &size) const;
	/** Copy the current game screen into @p destination. */
	void captureScreen(ManagedSurface32 *destination) const;
	/** Copy @p source onto the current game screen. */
	void copyToScreen(const ManagedSurface32 &source) const;
	/** Capture a rectangle from the current game screen at the destination origin. */
	void captureScreenRegion(ManagedSurface32 *destination, const Common::Rect &sourceRect) const;
	/** Restore @p source onto the current game screen at @p destination. */
	void copyRegionToScreen(const ManagedSurface32 &source, const Common::Point &destination) const;

	/** Draw an uncompressed bitmap through the shared Z2 rendering boundary. */
	void drawBitBlock(ManagedSurface32 *destination, const BitBlock *bitmap, const Common::Point32 &position) const;
	/** Draw one bitmap sub-rectangle through the shared Z2 rendering boundary. */
	void drawBitBlockSubRect(ManagedSurface32 *destination, const BitBlock *bitmap, const Common::Point32 &position, const Common::Rect &sourceRect) const;
	/** Load the active page background, replacing any previous one. */
	bool loadBackground(const Common::Path &path);
	/** Release the active page background. */
	void clearBackground();
	/** Return whether a page background is loaded. */
	bool hasBackground() const { return _background != nullptr; }
	/** Draw the loaded page background at @p position. */
	void drawBackground(ManagedSurface32 *destination, const Common::Point32 &position) const;
	/** Draw a sub-rectangle of the loaded page background at @p position. */
	void drawBackgroundSubRect(ManagedSurface32 *destination, const Common::Point32 &position, const Common::Rect &sourceRect) const;
	/** Return the page layer collection released with the graphics interface. */
	PageLayerStack *getPageLayerStack() const { return _pageLayerStack; }
	/** Remove every page layer and reset transient stack state. */
	void clearPageLayers();
	/** Draw an RLE sprite through the shared Z2 rendering boundary. */
	void drawRleBlock(ManagedSurface32 *destination, const RleBlock *sprite, const Common::Point32 &position) const;
	/** Draw one frame from an animation through the shared Z2 rendering boundary. */
	void drawAnimationFrame(ManagedSurface32 *destination, const Animation *animation, int frameIndex, const Common::Point32 &position) const;
	/** Draw and advance one general-object animation runner. */
	void drawAndUpdateAnimationRunner(ManagedSurface32 *destination, AnimationRunner *runner, uint32 tickCount, int scrollX, int backgroundWidth) const;
	/** Draw a Zoombini body and trait stack through the shared Z2 rendering boundary. */
	void drawZoombini(ManagedSurface32 *destination, const ZoombiniAnimation *animation, const ZmbTrait &traits, const Common::Point32 &position, int cell, int frame, const Common::Rect32 *clip = nullptr) const;
	/** Draw one active Zoombini runner through the shared Z2 rendering boundary. */
	void drawZoombiniRunner(ManagedSurface32 *destination, const ZoombiniRunner *runner) const;
	/** Draw bitmap-font text and return its horizontal pixel advance. */
	int drawString(ManagedSurface32 *destination, const BitmapFont *font, const Common::Point32 &position, const Common::String &text) const;
	/** Draw the held Zoombini name plate centered at the bottom of @p destination. */
	void drawDragNameTooltip(ManagedSurface32 *destination, const Common::String &name);
	/** Black out @p destination where @p areaMask rejects drops, or everywhere when @p areaMask is nullptr. */
	void maskRejectedArea(ManagedSurface32 *destination, const AreaMask *areaMask);
	/** Fill a clipped rectangle through the shared Z2 rendering boundary. */
	void fillRect(ManagedSurface32 *destination, const Common::Rect32 &rect, uint32 color) const;
	/** Fill a 16-bit API-boundary rectangle through the shared Z2 rendering boundary. */
	void fillRect(ManagedSurface32 *destination, const Common::Rect &rect, uint32 color) const;
	/** Draw a clipped rectangular outline through the shared Z2 rendering boundary. */
	void frameRect(ManagedSurface32 *destination, const Common::Rect32 &rect, uint32 color) const;
	/** Draw a 16-bit API-boundary rectangular outline through the shared Z2 rendering boundary. */
	void frameRect(ManagedSurface32 *destination, const Common::Rect &rect, uint32 color) const;
	/** Draw a line through the shared Z2 rendering boundary. */
	void drawLine(ManagedSurface32 *destination, const Common::Point32 &start, const Common::Point32 &end, uint32 color) const;

	/** Create the route-map background with all state-dependent overlays applied. */
	ManagedSurface32 *createMapTransitionBackground(PageId sourcePage, int mapRegion, RouteBranch routeBranch);

private:
	/** Load, draw, and release one cached map-overlay RLE sprite. */
	void drawOverlaySprite(ManagedSurface32 *dst, const Common::String &name, const Common::Point32 &pos);
	/** Compose the route-map overlays appropriate to the current progress. */
	void drawMapOverlays(ManagedSurface32 *dst, PageId sourcePage, int mapRegion, RouteBranch routeBranch);

	/** Name-plate sprite drawn under the held Zoombini name. */
	RleBlock *_nameBoxSprite = nullptr;
	/** Shared tooltip font drawn over the name plate. */
	BitmapFont *_tooltipFont = nullptr;
	/** Active page background released with the graphics interface. */
	BitBlock *_background = nullptr;
	/** Page layer collection released with the graphics interface. */
	PageLayerStack *_pageLayerStack = nullptr;

	/** Borrowed game instance used for resource resolution and shared blend state. */
	Zoombini2Engine *_vm;
};

/**
 * Screen surface with 32-bit coordinates.
 *
 * @remarks The game renders into a fixed 800x600 surface, while original
 * resource coordinates use 32-bit values. This adapter accepts Point32 and
 * Rect32 without exposing the ScummVM base surface to Z2 callers.
 */
class ManagedSurface32 : public Graphics::ManagedSurface {
public:
	/** Dimensions of the fixed internal game screen. */
	static constexpr Size32 kScreenSize = Size32(800, 600);

	/** Create a screen surface with the supplied dimensions and pixel format. */
	ManagedSurface32(const Size32 &size, const Graphics::PixelFormat &pixelFormat)
		: Graphics::ManagedSurface(size.width, size.height, pixelFormat) {}

	using Graphics::ManagedSurface::blitFrom;
	using Graphics::ManagedSurface::fillRect;
	using Graphics::ManagedSurface::frameRect;

	/** Fill a 32-bit rectangle after clipping it to this surface. */
	void fillRect(const Common::Rect32 &rect, uint32 color);
	/** Draw a 32-bit rectangular outline after clipping it to this surface. */
	void frameRect(const Common::Rect32 &rect, uint32 color);
	/** Copy a managed surface at a 32-bit destination pos after clipping. */
	void blitFrom(const ManagedSurface32 &src, const Common::Point32 &destPos);
};

/**
 * Immutable 8-bit channel-scaling lookup table used by alpha compositing.
 *
 * Each game instance keeps one table and shares it across its drawing operations.
 *
 * The table stores every possible product of two byte-sized values:
 * `scale(factor, value) = (factor * value) >> 8`.
 * The shift is intentional. It is integer division by 256 with truncation, so
 * it preserves the renderer's byte-based blend rule and is not interchangeable
 * with division by 255.
 *
 * `factor` is the coefficient applied to one channel. It is an opacity mask
 * when producing a premultiplied source contribution, or an inverse-alpha
 * value when retaining the destination contribution. `value` is one 8-bit
 * color channel.
 *
 * A mode-1 RLE pixel stores premultiplied BGR in its first three bytes and
 * inverse alpha in its fourth byte. RLE drawing therefore computes
 * `source + scale(inverseAlpha, destination)` for each channel. The
 * @ref BitBlock::drawRleMaskBlend method has an uncompressed bitmap and a
 * separate mask, so it computes
 * `scale(mask, source) + scale(255 - mask, destination)` instead.
 *
 * The table is populated once because these products are needed for every
 * blended color channel. A lookup avoids repeating multiplication and
 * division in the pixel loops while retaining the exact integer result.
 * It is not used by @ref BitBlock::drawAlphaBlend(), which implements the
 * separate-mask `/255` blend rule.
 */
class AlphaBlendLUT : Common::NonCopyable {
public:
	/** Number of values in each byte-sized lookup dimension, from 0 through 255. */
	static constexpr int kValueCount = 256;

	/** Populate every factor-and-value combination used by @ref scale. */
	AlphaBlendLUT();

	/**
	 * Return `floor(factor * value / 256)` for two byte-sized values.
	 *
	 * @param factor Opacity or inverse-alpha coefficient.
	 * @param value Source or destination color-channel value.
	 */
	byte scale(byte factor, byte value) const { return _values[factor][value]; }

private:
	/** Cached results indexed as `[factor][channelValue]`. */
	byte _values[kValueCount][kValueCount];
};

/**
 * Uncompressed RGBA bitmap with an optional alpha mask.
 *
 * A BitBlock may be loaded from a color BMP, a color-and-alpha BMP pair, or
 * the game's cached BB format. Drawing clips source pixels to the destination
 * surface and preserves the destination outside the covered region.
 */
class BitBlock {
public:
	/** Construct an empty bitmap bound to @p vm. */
	explicit BitBlock(Zoombini2Engine *vm);
	/** Release the member pixel and alpha buffers. */
	~BitBlock();

	/** Load a mandatory color BMP and mandatory alpha-mask BMP. */
	bool loadFromColorAlphaBMP(const Common::Path &colorPath, const Common::Path &alphaPath);
	/** Load a 24-bit color BMP without a separate alpha mask. */
	bool loadFromColorBMP(const Common::Path &colorPath);
	/** Load the cached BB representation at @p bbPath. */
	bool loadFromBB(const Common::Path &bbPath);

	/**
	 * Load a cached or source bitmap for @p basePath.
	 *
	 * Exact `.bb` inputs are opened directly. Bitmap inputs first try a sibling
	 * `.bb`; extensionless inputs try `.bb` and then `.bmp`.
	 */
	bool load(const Common::Path &basePath);

	/** Allocate a zeroed bitmap and optionally a fully transparent alpha mask. */
	void createEmpty(const Size32 &size, bool withAlpha);

	/** Draw the full bitmap without alpha blending at @p pos. */
	void drawToSurface(ManagedSurface32 *dst, const Common::Point32 &pos) const;
	/** Draw @p srcRect from this bitmap without alpha blending at @p pos. */
	void drawSubRect(ManagedSurface32 *dst, const Common::Point32 &pos, const Common::Rect &srcRect) const;
	/**
	 * Draw the full bitmap with the separate-mask `/255` blend rule at @p pos.
	 *
	 * The source channel is added as supplied, while the destination channel is
	 * scaled by `(255 - mask) / 255`; this path intentionally does not use the
	 * RLE `/256` lookup table.
	 */
	void drawAlphaBlend(ManagedSurface32 *dst, const Common::Point32 &pos) const;
	/**
	 * Draw a bitmap-mask pair at @p pos using the RLE premultiplied blend rule.
	 *
	 * @p alphaLUT scales the raw source channel by the mask and the destination
	 * channel by the mask's inverse, using the renderer's `/256` rule.
	 */
	void drawRleMaskBlend(ManagedSurface32 *dst, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const;

	/** Return the bitmap dimensions in pixels. */
	const Size32 &getSize() const { return _size; }
	/** Return the bitmap width in pixels. */
	int32 getWidth() const { return _size.width; }
	/** Return the bitmap height in pixels. */
	int32 getHeight() const { return _size.height; }
	/** Return whether this bitmap has a separate alpha mask. */
	bool hasAlpha() const { return _alphaMap != nullptr; }
	/** Return the borrowed RGBA pixel buffer. */
	const byte *getPixels() const { return _pixels; }
	/** Return the borrowed alpha buffer, or nullptr when no mask is loaded. */
	const byte *getAlpha() const { return _alphaMap; }

private:
	/** Borrowed vm used to resolve bitmap resources. */
	Zoombini2Engine *_vm;
	/** Bitmap dimensions in pixels. */
	Size32 _size = Size32();
	/** RGBA pixel storage held by this bitmap, with four bytes per pixel. */
	byte *_pixels = nullptr;
	/** Optional one-byte-per-pixel alpha storage held by this bitmap, or nullptr. */
	byte *_alphaMap = nullptr;

	/** Decode a 24-bit BMP from @p stream through the shared image decoder. */
	bool loadColorBMP(Common::SeekableReadStream *stream);
	/** Decode an indexed alpha-mask BMP from @p stream through the shared image decoder. */
	bool loadAlphaBMP(Common::SeekableReadStream *stream);
	/** Exchange bitmap buffer state with @p other. */
	void swapData(BitBlock &other);
	/**
	 * Blend one bitmap channel with the destination through a separate mask.
	 *
	 * The source channel is added as supplied, and only the destination is
	 * scaled by the mask's inverse using integer `/255` division.
	 */
	static byte blendChannel(byte src, byte dest, byte mask);
};

/**
 * Represents one RLE-compressed sprite frame in the engine's drawing format.
 *
 * The frame header stores its dimensions and encoded byte count. Encoded spans
 * contain screen-relative coordinates, a pixel count, an opaque-or-alpha mode,
 * and expanded four-byte pixel records. Mode-1 records store premultiplied BGR
 * plus inverse alpha, which is composited through an @ref AlphaBlendLUT.
 * Drawing clips malformed or off-screen spans instead of writing outside the
 * destination surface.
 */
class RleBlock {
public:
	/** Construct an empty RLE frame bound to @p vm. */
	explicit RleBlock(Zoombini2Engine *vm);
	/** Release the member RLE data. */
	~RleBlock();

	/** Load the recoverable prefix of an RB or AN frame record from @p stream. */
	bool loadFromStream(Common::SeekableReadStream *stream);
	/** Open and load an RB record, ignoring warned trailing bytes. */
	bool loadFromFile(const Common::Path &path);
	/** Resolve @p basePath to an RB cache path and load it. */
	bool load(const Common::Path &basePath);
	/** Load the recoverable prefix of an ANM frame with a trusted size boundary. */
	bool loadAnmFrame(Common::SeekableReadStream *stream, uint32 outerSize);

	/**
	 * Draw this frame at @p pos using opaque copies or lookup-table blending.
	 *
	 * Mode-1 spans use their premultiplied BGR channels and inverse-alpha byte
	 * with @p alphaLUT.
	 */
	void drawToScreen(ManagedSurface32 *dst, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const;
	/**
	 * Draw this frame inside @p clip using opaque copies or lookup-table blending.
	 *
	 * Mode-1 spans use their premultiplied BGR channels and inverse-alpha byte
	 * with @p alphaLUT.
	 */
	void drawToScreenClipped(ManagedSurface32 *dst, const Common::Point32 &pos, const Common::Rect32 &clip, const AlphaBlendLUT &alphaLUT) const;

	/** Return the frame dimensions in pixels. */
	const Size32 &getSize() const { return _size; }
	/** Return the frame width in pixels. */
	int32 getWidth() const { return _size.width; }
	/** Return the frame height in pixels. */
	int32 getHeight() const { return _size.height; }
	/** Return whether encoded frame data has been loaded. */
	bool isValid() const { return _rleData != nullptr; }

private:
	/** Borrowed vm used to resolve RLE resources. */
	Zoombini2Engine *_vm;
	/** Frame dimensions in pixels. */
	Size32 _size = Size32();
	/** Number of bytes in @ref RleBlock::_rleData after load-time expansion. */
	uint32 _dataSize = 0;
	/** Expanded RLE span storage held by this frame, with four bytes per encoded pixel. */
	byte *_rleData = nullptr;
	/** Trailing header value retained with the decoded frame. */
	int32 _field20 = 0;

	/** Expand complete encoded spans and report whether a malformed tail was discarded. */
	static bool expand3to4bpp(const byte *srcData, uint32 srcSize, byte *&expandedData, uint32 &expandedSize, bool &salvaged);
	/**
	 * Add one premultiplied source channel to the destination scaled by @p inverseAlpha.
	 *
	 * Mode-1 RLE pixels store the premultiplied source channel directly, so only
	 * the destination term needs an @p alphaLUT lookup.
	 * The sum is saturated at 255.
	 */
	static byte blendChannel(byte src, byte dest, byte inverseAlpha, const AlphaBlendLUT &alphaLUT);
	/** Exchange frame buffer state with @p other. */
	void swapData(RleBlock &other);
};

/** Represents the ordered RLE frames decoded from one AN animation file. */
class Animation {
public:
	/** Construct an animation without frames, bound to @p vm. */
	explicit Animation(Zoombini2Engine *vm);
	/** Release all retained frames. */
	~Animation();

	/** Load every recoverable frame from @p path. */
	bool loadFromFile(const Common::Path &path);

	/** Return the number of loaded frames. */
	int getFrameCount() const { return _frames.size(); }
	/** Return a borrowed frame, or nullptr when @p index is outside the sequence. */
	const RleBlock *getFrame(int index) const;

private:
	/** Borrowed vm passed to each decoded frame. */
	Zoombini2Engine *_vm;
	/** Ordered frame sequence released with this animation. */
	Common::Array<RleBlock *> _frames;
};

/**
 * Decoded 1-bit page-area bitmap used by common Zoombini drop handling.
 *
 * The original tests the packed source byte containing a point rather than an
 * individual bit. The decoder keeps the pixel row and reproduces that byte-wide
 * acceptance rule explicitly.
 */
class AreaMask : public Common::NonCopyable {
public:
	/** Construct an empty mask bound to @p vm. */
	explicit AreaMask(Zoombini2Engine *vm);
	/** Load a 1-bit BMP mask through the engine resource resolver. */
	bool loadFromFile(const Common::Path &path);
	/** Return the decoded mask dimensions, or an empty size when unloaded. */
	Size32 getSize() const { return _size; }
	/** Return the number of marked pixels in the decoded mask. */
	uint32 countMarkedPixels() const;
	/** Return whether the source byte containing @p point has any marked bit. */
	bool hasMarkedByteAt(const Common::Point32 &point) const;

private:
	Zoombini2Engine *_vm;
	Size32 _size = Size32();
	Common::Array<byte> _pixels;

	/** Decode an uncompressed 1-bit BMP into palette indices, or return false. */
	bool loadOneBitBitmap(Common::SeekableReadStream &stream, const Common::Path &path);
};

/**
 * Represents the fixed three-dimensional sprite-frame grid loaded from an ANM file.
 *
 * The first index selects one of 100 movement or animation cells. The second
 * selects the body or one of four feature layers, and the third selects the
 * base image or one of five feature values.
 */
class ZoombiniAnimation {
public:
	/** Number of movement and animation cells. */
	static constexpr int kDim0 = 100;
	/** Number of sprite layers per cell. */
	static constexpr int kDim1 = ZmbTrait::kTraitCount + 1;
	/** Number of base-or-feature variants per layer. */
	static constexpr int kDim2 = ZmbTrait::kTraitValueCount + 1;
	/** Total number of independently framed grid entries. */
	static constexpr int kCellCount = kDim0 * kDim1 * kDim2;

	/** Construct an empty sprite grid bound to @p vm. */
	explicit ZoombiniAnimation(Zoombini2Engine *vm);
	/** Release every frame retained by the sprite grid. */
	~ZoombiniAnimation();

	/** Load every recoverable grid entry from @p path. */
	bool loadFromFile(const Common::Path &path);
	/** Set the page-configured delay shared by every Zoombini using this grid. */
	void setFrameDelay(uint32 frameDelay) { _frameDelay = frameDelay; }
	/** Return the page-configured delay shared by every Zoombini using this grid. */
	uint32 getFrameDelay() const { return _frameDelay; }
	/** Return a borrowed frame, or nullptr when either index is invalid. */
	const RleBlock *getFrame(int cellIndex, int frameIndex) const;
	/** Return the number of frames in @p cellIndex, or zero for an invalid cell. */
	int getFrameCount(int cellIndex) const;
	/** Return the base layer's dimensions for one Zoombini cell and frame. */
	Size32 getSpriteSize(int cell, int frame) const;
	/** Draw body and trait layers, retaining frame zero for single-frame entries and honoring an optional clip rectangle. */
	void drawZoombini(ManagedSurface32 *screen, const ZmbTrait &traits, const Common::Point32 &pos,
					  int cell, int frame, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip = nullptr) const;

private:
	/** Borrowed vm used to open the ANM resource and construct its frames. */
	Zoombini2Engine *_vm;
	/** Page-configured delay retained beside the shared sprite grid. */
	uint32 _frameDelay = 0;
	/** Represents the frames assigned to one sprite-grid entry. */
	struct Cell {
		/** Ordered frames stored in this grid entry. */
		Common::Array<RleBlock *> frames;
		/** Release every retained frame. */
		~Cell();
	};

	/** Fixed sprite grid indexed by cell, layer, and feature value. */
	Cell _cells[kCellCount];
};

/**
 * Handles the normal, highlighted, and disabled images for one button.
 *
 * A button may use an uncompressed @ref BitBlock or an @ref RleBlock for each
 * state. Its draw call also performs the frame's hit test and reports whether
 * the pointer has just entered or remains inside the enabled button.
 */
class UIButton {
public:
	/** Construct an enabled button bound to @p vm. */
	explicit UIButton(Zoombini2Engine *vm);
	/** Release every image retained by the button. */
	~UIButton();

	/**
	 * Load normal and optional alternate images without separate alpha masks.
	 *
	 * Each non-empty path is tried as a BitBlock and then as an RLE block.
	 * The normal state is required. Missing highlighted or disabled states are
	 * non-fatal and leave those visual states empty.
	 */
	bool loadImages(const Common::Path &normalPath, const Common::Path &hoverPath = Common::Path(),
					const Common::Path &disabledPath = Common::Path());

	/** Load normal and optional masked states from RLE caches or BMP pairs. */
	bool loadImagesWithMask(const Common::Path &normalPath, const Common::Path &normalMask,
							const Common::Path &hoverPath = Common::Path(), const Common::Path &hoverMask = Common::Path(),
							const Common::Path &disabledPath = Common::Path(), const Common::Path &disabledMask = Common::Path());

	/** Set the button rectangle from a screen position and size. */
	void setRect(const Common::Point32 &pos, const Size32 &size);
	/** Replace the button rectangle with @p rect. */
	void setRect(const Common::Rect &rect);
	/** Enable or disable pointer interaction. */
	void setEnabled(bool enabled) { _enabled = enabled; }
	/** Return whether pointer interaction is enabled. */
	bool isEnabled() const { return _enabled; }
	/** Select whether the normal image is drawn while the pointer is outside. */
	void setDrawWhenNotHovered(bool draw) { _drawWhenNotHovered = draw; }
	/** Attach a borrowed clipped overlay whose right edge is read when drawing. */
	void setOverlay(const RleBlock *overlay, const Common::Point32 &offset, const int *clipRight);

	/**
	 * Draw the state selected by @p mousePos.
	 *
	 * @return Zero when not hovered, two when newly hovered, or one when the
	 * pointer remains over the button from the preceding draw.
	 */
	int drawAndHitTest(ManagedSurface32 *dst, const Common::Point32 &mousePos, const AlphaBlendLUT &alphaLUT);

	/** Return whether @p pos is inside the button rectangle. */
	bool containsPoint(const Common::Point32 &pos) const;
	/** Return whether @p point is inside the button rectangle. */
	bool containsPoint(const Common::Point &point) const { return containsPoint(Common::Point32(point)); }

	/** Return the button rectangle. */
	const Common::Rect &getRect() const { return _rect; }
	/** Return whether the pointer was over the button during the preceding draw. */
	bool wasHovering() const { return _wasHovering; }
	/** Return whether the pointer was over the button during the current draw. */
	bool isHovering() const { return _isHovering; }

private:
	/** Borrowed vm used to construct and load the button's images. */
	Zoombini2Engine *_vm;
	/** Button pos and hit-test bounds. */
	Common::Rect _rect = Common::Rect();
	/** Whether the button responds to pointer input. */
	bool _enabled = true;
	/** Whether the normal state is drawn while the pointer is outside. */
	bool _drawWhenNotHovered = true;
	/** Whether masked bitmap fallbacks use the RLE encoder's blend rule. */
	bool _useRleMaskBlend = false;
	/** Hover state retained from the preceding draw. */
	bool _wasHovering = false;
	/** Hover state computed during the current draw. */
	bool _isHovering = false;

	/** Uncompressed normal-state image held by this button. */
	BitBlock *_normalBB = nullptr;
	/** Uncompressed highlighted-state image held by this button. */
	BitBlock *_hoverBB = nullptr;
	/** Uncompressed disabled-state image held by this button. */
	BitBlock *_disabledBB = nullptr;
	/** RLE normal-state image held by this button. */
	RleBlock *_normalRle = nullptr;
	/** RLE highlighted-state image held by this button. */
	RleBlock *_hoverRle = nullptr;
	/** RLE disabled-state image held by this button. */
	RleBlock *_disabledRle = nullptr;
	/** Borrowed overlay drawn after the selected button state. */
	const RleBlock *_overlay = nullptr;
	/** Overlay offset relative to the button. */
	Common::Point32 _overlayOffset = Common::Point32();
	/** Borrowed absolute clipping coordinate for the overlay's right edge. */
	const int *_overlayClipRight = nullptr;

	/** Release all retained state images. */
	void clearImages();
	/** Load one image, preferring the cached bitmap representation. */
	bool loadImage(const Common::Path &path, BitBlock *&bitmap, RleBlock *&rle);
	/** Load one masked state from its RLE cache or source bitmap pair. */
	bool loadMaskedImage(const Common::Path &colorPath, const Common::Path &maskPath, BitBlock *&bitmap, RleBlock *&rle);
};

/** Button indices used by the seven controls on the sign-in screen. */
enum MenuButtonId {
	kMenuButtonNext = 0,     ///< Scroll toward the preceding visible profile rows.
	kMenuButtonPrev = 1,     ///< Scroll toward the following visible profile rows.
	kMenuButtonStart = 2,    ///< Start the selected saved adventure.
	kMenuButtonOptions = 3,  ///< Open the volume panel.
	kMenuButtonNew = 4,      ///< Begin entry of a new profile name.
	kMenuButtonTraining = 5, ///< Open the practice map.
	kMenuButtonQuit = 6,     ///< Request exit from the sign-in screen.
	kMenuButtonCount = 7     ///< Number of sign-in buttons.
};

/**
 * Renders text with the 81 glyphs extracted from one bitmap-font strip.
 *
 * The supported glyph sequence contains uppercase letters, lowercase letters,
 * digits, and eighteen punctuation characters. Spaces and unsupported bytes
 * advance by @ref BitmapFont::kSpaceWidth without drawing.
 */
class BitmapFont {
public:
	/** Number of glyphs in the fixed font-strip mapping. */
	static constexpr int kNumGlyphs = 81;
	/** Horizontal advance used for spaces and unsupported characters. */
	static constexpr int kSpaceWidth = 10;

	/** Construct an unloaded font bound to @p vm. */
	explicit BitmapFont(Zoombini2Engine *vm);
	/** Release every extracted glyph bitmap. */
	~BitmapFont();

	/** Load a BMT color-and-alpha pair and color its extracted glyphs. */
	bool load(const Common::Path &basePath, byte red, byte green, byte blue);
	/** Draw @p text at @p pos and return its horizontal pixel advance. */
	int drawString(ManagedSurface32 *dst, const Common::Point32 &pos, const Common::String &text,
				   const AlphaBlendLUT &alphaLUT) const;
	/** Return the horizontal pixel advance for @p text without drawing it. */
	int getStringWidth(const Common::String &text) const;
	/** Return the glyph index for @p character, or -1 when it is unsupported. */
	static int charToGlyphIndex(char character);
	/** Return whether glyph extraction completed. */
	bool isLoaded() const { return _loaded; }

private:
	/** Borrowed vm used to load the font strip and construct glyphs. */
	Zoombini2Engine *_vm;
	/** Whether the font strip has been processed. */
	bool _loaded = false;
	/** Glyph bitmaps held by this font in character-mapping order. */
	BitBlock *_glyphs[kNumGlyphs] = {};
};

/** Result of one @ref VolumePanel input-and-draw pass. */
enum VolumePanelResult {
	kVolumePanelOpen,    ///< The panel remains open without a new volume change.
	kVolumePanelChanged, ///< At least one preview volume changed during this pass.
	kVolumePanelApply,   ///< The player accepted the preview values.
	kVolumePanelCancel   ///< The player cancelled the preview values.
};

/**
 * Controls the music, sound-effect, and speech sliders shared by map and menu pages.
 *
 * Slider changes are previewed while the panel remains open. Callers decide
 * whether an apply result commits the values or a cancel result restores the
 * initial values captured by @ref VolumePanel::setInitialVolumes.
 */
class VolumePanel {
public:
	/** Leftmost selectable gauge coordinate. */
	static constexpr int kSliderMinX = 350;
	/** Rightmost selectable gauge coordinate. */
	static constexpr int kSliderMaxX = 638;
	/** Number of pixels in the selectable gauge interval. */
	static constexpr int kSliderRange = 288;

	/** Shared X coordinate of each slider label button. */
	static constexpr int kLabelX = 157;
	/** Shared slider label dimensions. */
	static constexpr Size32 kLabelSize = Size32(520, 64);
	/** Music slider label Y coordinate. */
	static constexpr int kMusicLabelY = 224;
	/** Sound-effect slider label Y coordinate. */
	static constexpr int kSfxLabelY = 286;
	/** Speech slider label Y coordinate. */
	static constexpr int kSpeechLabelY = 351;

	/** Music gauge Y coordinate. */
	static constexpr int kMusicGaugeY = 240;
	/** Sound-effect gauge Y coordinate. */
	static constexpr int kSfxGaugeY = 305;
	/** Speech gauge Y coordinate. */
	static constexpr int kSpeechGaugeY = 374;

	/** Construct a panel bound to @p vm with all volumes set to 100 percent. */
	explicit VolumePanel(Zoombini2Engine *vm);
	/** Release the preview sounds and retained gauge image after the buttons stop borrowing it. */
	~VolumePanel();

	/** Load gauge, button, and optional audio-preview resources. */
	bool init(SoundManager *soundManager = nullptr);

	/** Track a slider drag or process one mouse release without painting unrelated page state. */
	VolumePanelResult handleMouseInput(const Common::Point32 &mousePos, bool mouseDown, bool mouseReleased);
	/** Play the category-specific sample for the most recently released slider. */
	void playPreviewSound();
	/** Paint the panel without applying input or changing audio. */
	void draw(ManagedSurface32 *dst, const Common::Point32 &mousePos, const AlphaBlendLUT &alphaLUT);

	/** Return the current music volume percentage. */
	int getMusicVolume() const { return _musicVolume; }
	/** Return the current sound-effect volume percentage. */
	int getSfxVolume() const { return _sfxVolume; }
	/** Return the current speech volume percentage. */
	int getSpeechVolume() const { return _speechVolume; }
	/** Return the initial music volume percentage. */
	int getInitialMusicVolume() const { return _initialMusicVolume; }
	/** Return the initial sound-effect volume percentage. */
	int getInitialSfxVolume() const { return _initialSfxVolume; }
	/** Return the initial speech volume percentage. */
	int getInitialSpeechVolume() const { return _initialSpeechVolume; }

	/** Clamp and assign the current music volume. */
	void setMusicVolume(int volume);
	/** Clamp and assign the current sound-effect volume. */
	void setSfxVolume(int volume);
	/** Clamp and assign the current speech volume. */
	void setSpeechVolume(int volume);
	/** Assign current values and capture them as the panel's cancellation baseline. */
	void setInitialVolumes(int music, int sfx, int speech);

	/** Clamp @p x to the gauge interval and convert it to a percentage. */
	static int pixelToVolume(int x);
	/** Convert @p volume from a percentage to a gauge X coordinate. */
	static int volumeToPixel(int volume);

private:
	/** Borrowed vm used by the panel's gauge and button resources. */
	Zoombini2Engine *_vm;
	/** Current music volume percentage. */
	int _musicVolume = 100;
	/** Current sound-effect volume percentage. */
	int _sfxVolume = 100;
	/** Current speech volume percentage. */
	int _speechVolume = 100;
	/** Music volume restored when the caller cancels. */
	int _initialMusicVolume = 100;
	/** Sound-effect volume restored when the caller cancels. */
	int _initialSfxVolume = 100;
	/** Speech volume restored when the caller cancels. */
	int _initialSpeechVolume = 100;

	/** Current music gauge endpoint. */
	int _musicSliderX = kSliderMaxX;
	/** Current sound-effect gauge endpoint. */
	int _sfxSliderX = kSliderMaxX;
	/** Current speech gauge endpoint. */
	int _speechSliderX = kSliderMaxX;
	/** Dragged slider index, or -1 when no slider is captured. */
	int _activeSlider = -1;
	/** Most recent mouse position observed while the primary button was held. */
	Common::Point32 _heldMousePos = Common::Point32();
	/** Whether held-mouse coordinates are available for the next release. */
	bool _hasHeldMouse = false;
	/** Most recently released slider awaiting category-specific audio preview. */
	int _lastAdjustedSlider = -1;
	/** Borrowed sound manager that retains the panel's logical preview records. */
	SoundManager *_soundManager = nullptr;
	/** Speech preview sound identifier retained by the panel. */
	int _speechPreviewSoundId = -1;
	/** Sound-effect preview identifier retained by the panel. */
	int _sfxPreviewSoundId = -1;

	/** Gauge sprite held by this panel and shared by the three slider rows. */
	RleBlock *_gaugeImage = nullptr;
	/** Music, sound-effect, and speech label buttons. */
	UIButton _sliderLabels[3];
	/** Apply button. */
	UIButton _okButton;
	/** Cancel button. */
	UIButton _noButton;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_GRAPHICS_H
