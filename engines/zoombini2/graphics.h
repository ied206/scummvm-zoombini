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
#include "common/hash-str.h"
#include "common/noncopyable.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"

#include "graphics/managed_surface.h"

#include "zoombini2/state.h"

namespace Zoombini2 {

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
class AlphaBlendLUT;
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
	/** Load an RLE sprite once for the current page and return a borrowed pointer. */
	RleBlock *loadPageRleBlock(const Common::String &key) { return loadRleBlock(_pageRleBlocks, key); }
	/** Load an uncompressed bitmap once for the current page and return a borrowed pointer. */
	BitBlock *loadPageBitBlock(const Common::String &key) { return loadBitBlock(_pageBitBlocks, key); }
	/** Load a color and alpha BMP pair once for the current page. */
	BitBlock *loadPageMaskedBitBlock(const Common::String &colorKey, const Common::String &alphaKey) {
		return loadMaskedBitBlock(colorKey, alphaKey);
	}
	/** Load an RLE sprite retained for the lifetime of the graphics interface. */
	RleBlock *loadSharedRleBlock(const Common::String &key) { return loadRleBlock(_sharedRleBlocks, key); }
	/** Load a bitmap retained for the lifetime of the graphics interface. */
	BitBlock *loadSharedBitBlock(const Common::String &key) { return loadBitBlock(_sharedBitBlocks, key); }
	/** Release cached page bitmaps after all page and dialog users have closed. */
	void clearPageBitmapCache();
	/** Copy the current game screen into @p destSurface. */
	void captureScreen(ManagedSurface32 *destSurface) const;
	/** Copy @p source onto the current game screen. */
	void copyToScreen(const ManagedSurface32 &srcSurface) const;
	/** Capture a rectangle from the current game screen at the destSurface origin. */
	void captureScreenRegion(ManagedSurface32 *destSurface, const Common::Rect &srcRect) const;
	/** Restore @p source onto the current game screen at @p destSurface. */
	void copyRegionToScreen(const ManagedSurface32 &srcSurface, const Common::Point &destSurface) const;

	/** Draw an uncompressed bitmap through the shared Z2 rendering boundary. */
	void drawBitBlock(ManagedSurface32 *destSurface, const BitBlock *bitmap, const Common::Point32 &pos) const;
	/** Draw an uncompressed bitmap while treating its top-left pixel color as transparent. */
	void drawBitBlockColorKey(ManagedSurface32 *destSurface, const BitBlock *bitmap, const Common::Point32 &pos) const;
	/** Load and draw a bitmap from the current page cache. */
	void drawPageBitBlock(ManagedSurface32 *destSurface, const Common::String &key, const Common::Point32 &pos) {
		if (destSurface)
			drawBitBlock(destSurface, loadPageBitBlock(key), pos);
	}
	/** Load and draw a page bitmap using its top-left pixel as a transparency key. */
	void drawPageBitBlockColorKey(ManagedSurface32 *destSurface, const Common::String &key, const Common::Point32 &pos) {
		if (destSurface)
			drawBitBlockColorKey(destSurface, loadPageBitBlock(key), pos);
	}
	/** Load and draw a bitmap retained across page changes. */
	void drawSharedBitBlock(ManagedSurface32 *destSurface, const Common::String &key, const Common::Point32 &pos) {
		if (destSurface)
			drawBitBlock(destSurface, loadSharedBitBlock(key), pos);
	}
	/** Return a page bitmap's dimensions, or an empty size when it cannot be loaded. */
	Size32 getPageBitBlockSize(const Common::String &key);
	/** Draw one bitmap sub-rectangle through the shared Z2 rendering boundary. */
	void drawBitBlockSubRect(ManagedSurface32 *destSurface, const BitBlock *bitmap, const Common::Point32 &pos, const Common::Rect &srcRect) const;
	/** Load and draw one sub-rectangle of a page bitmap. */
	void drawPageBitBlockSubRect(ManagedSurface32 *destSurface, const Common::String &key, const Common::Point32 &pos, const Common::Rect &srcRect) {
		if (destSurface)
			drawBitBlockSubRect(destSurface, loadPageBitBlock(key), pos, srcRect);
	}
	/** Load the active page background, replacing any previous one. */
	bool loadBackground(const Common::String &key);
	/** Forget the active page background; its bitmap remains in the page cache. */
	void clearBackground();
	/** Return whether a page background is loaded. */
	bool hasBackground() const { return _background != nullptr; }
	/** Draw the loaded page background at @p pos. */
	void drawBackground(ManagedSurface32 *destSurface, const Common::Point32 &pos) const;
	/** Draw a sub-rectangle of the loaded page background at @p pos. */
	void drawBackgroundSubRect(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect &srcRect) const;
	/** Return the page layer collection released with the graphics interface. */
	PageLayerStack *getPageLayerStack() const { return _pageLayerStack; }
	/** Remove every page layer and reset transient stack state. */
	void clearPageLayers();
	/** Draw an RLE sprite through the shared Z2 rendering boundary. */
	void drawRleBlock(ManagedSurface32 *destSurface, const RleBlock *sprite, const Common::Point32 &pos) const;
	/** Draw an RLE sprite while skipping opaque pixels matching @p red, @p green, and @p blue. */
	void drawRleBlockColorKey(ManagedSurface32 *destSurface, const RleBlock *sprite, const Common::Point32 &pos, byte red, byte green, byte blue) const;
	/** Load and draw an RLE sprite from the current page cache. */
	void drawPageRleBlock(ManagedSurface32 *destSurface, const Common::String &key, const Common::Point32 &pos) {
		if (destSurface)
			drawRleBlock(destSurface, loadPageRleBlock(key), pos);
	}
	/** Load and draw an RLE sprite retained across page changes. */
	void drawSharedRleBlock(ManagedSurface32 *destSurface, const Common::String &key, const Common::Point32 &pos) {
		if (destSurface)
			drawRleBlock(destSurface, loadSharedRleBlock(key), pos);
	}
	/** Load and draw a shared RLE sprite while skipping one opaque RGB color. */
	void drawSharedRleBlockColorKey(ManagedSurface32 *destSurface, const Common::String &key, const Common::Point32 &pos, byte red, byte green,
									byte blue) {
		if (destSurface)
			drawRleBlockColorKey(destSurface, loadSharedRleBlock(key), pos, red, green, blue);
	}
	/** Return a page RLE sprite's dimensions, or an empty size when it cannot be loaded. */
	Size32 getPageRleBlockSize(const Common::String &key);
	/** Draw an RLE sprite inside an absolute screen clip. */
	void drawRleBlockClipped(ManagedSurface32 *destSurface, const RleBlock *sprite, const Common::Point32 &pos, const Common::Rect32 &clip) const;
	/** Load and draw a page RLE sprite inside an absolute screen clip. */
	void drawPageRleBlockClipped(ManagedSurface32 *destSurface, const Common::String &key, const Common::Point32 &pos, const Common::Rect32 &clip) {
		if (destSurface)
			drawRleBlockClipped(destSurface, loadPageRleBlock(key), pos, clip);
	}
	/** Draw one frame from an animation through the shared Z2 rendering boundary. */
	void drawAnimationFrame(ManagedSurface32 *destSurface, const Animation *animation, int frameIndex, const Common::Point32 &pos) const;
	/** Draw and advance one general-object animation runner. */
	void drawAndUpdateAnimationRunner(ManagedSurface32 *destSurface, AnimationRunner *runner, uint32 tickCount, int scrollX, int bgWidth) const;
	/** Compose body, Feet, Nose, Hair and Eyes, retaining frame zero for single-frame entries. */
	void drawZoombini(ManagedSurface32 *destSurface, const ZoombiniAnimation *animation, const ZmbTrait &traits,
					  const Common::Point32 &pos, int cell, int frame, const Common::Rect32 *clip = nullptr) const;
	/** Compose the same layers with an explicit blend table for compatibility callers. */
	static void drawZoombini(ManagedSurface32 *destSurface, const ZoombiniAnimation *animation, const ZmbTrait &traits,
							 const Common::Point32 &pos, int cell, int frame, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip = nullptr);
	/**
	 * Compose the large picker preview as body, Feet, Hair, Eyes and Nose at frame zero.
	 * Unlike @ref Gfx::drawZoombini, only selected values 1 through 5 add feature layers.
	 */
	void drawZoombiniPreview(ManagedSurface32 *destSurface, const ZoombiniAnimation *animation,
							 const int (&selectedValues)[ZmbTrait::kTraitCount], const Common::Point32 &pos) const;
	/** Compose the same picker preview with an explicit blend table. */
	static void drawZoombiniPreview(ManagedSurface32 *destSurface, const ZoombiniAnimation *animation,
									const int (&selectedValues)[ZmbTrait::kTraitCount], const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT);
	/** Draw one visible runner at its scrolled or dragged anchor without advancing animation. */
	void drawZoombiniRunner(ManagedSurface32 *destSurface, const ZoombiniRunner *runner, const Common::Rect32 *clip = nullptr,
							int scrollX = 0, int backgroundWidth = 800, const RleBlock *dropTargetIndicator = nullptr) const;
	/** Apply runner visibility, bounds and frame selection with an explicit blend table. */
	static void drawZoombiniRunner(ManagedSurface32 *destSurface, const ZoombiniRunner *runner, const AlphaBlendLUT &alphaLUT,
								   const Common::Rect32 *clip = nullptr, int scrollX = 0, int backgroundWidth = 800,
								   const RleBlock *dropTargetIndicator = nullptr);
	/** Color variants of the shared bitmap text strip. */
	enum class TextColor {
		kDark00 = 0,
		kBlue01 = 1,
		kGreen02 = 2,
		kWhite03 = 3
	};

	/** Load the shared text strip on demand; failed loads can be retried by the caller. */
	bool loadTextFont(TextColor color);
	/** Return whether the shared text strip is ready. */
	bool hasTextFont(TextColor color) const;
	/** Draw bitmap text tinted with @p color and return its horizontal advance. */
	int drawText(ManagedSurface32 *destSurface, TextColor color, const Common::Point32 &pos, const Common::String &text) const;
	/** Measure text with the same glyph advances used by @ref Gfx::drawText. */
	int getTextWidth(const Common::String &text, TextColor color) const;
	/** Draw the held Zoombini name plate centered at the bottom of @p destSurface. */
	void drawDragNameTooltip(ManagedSurface32 *destSurface, const Common::String &name);
	/** Black out @p destSurface where @p areaMask rejects drops, or everywhere when @p areaMask is nullptr. */
	void maskRejectedArea(ManagedSurface32 *destSurface, const AreaMask *areaMask);
	/** Fill a clipped rectangle through the shared Z2 rendering boundary. */
	void fillRect(ManagedSurface32 *destSurface, const Common::Rect32 &rect, uint32 color) const;
	/** Fill a 16-bit API-boundary rectangle through the shared Z2 rendering boundary. */
	void fillRect(ManagedSurface32 *destSurface, const Common::Rect &rect, uint32 color) const;
	/** Draw a clipped rectangular outline through the shared Z2 rendering boundary. */
	void frameRect(ManagedSurface32 *destSurface, const Common::Rect32 &rect, uint32 color) const;
	/** Draw a 16-bit API-boundary rectangular outline through the shared Z2 rendering boundary. */
	void frameRect(ManagedSurface32 *destSurface, const Common::Rect &rect, uint32 color) const;
	/** Draw a line through the shared Z2 rendering boundary. */
	void drawLine(ManagedSurface32 *destSurface, const Common::Point32 &start, const Common::Point32 &end, uint32 color) const;

	/** Create the route-map background with all state-dependent overlays applied. */
	ManagedSurface32 *createMapTransitionBackground(PageId srcPageId, int mapRegion, RouteBranch routeBranch);

private:
	struct MaskedBitBlockEntry {
		Common::String colorPath;
		Common::String alphaPath;
		BitBlock *bitmap;
	};
	typedef Common::HashMap<Common::String, RleBlock *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> RleBlockCache;
	typedef Common::HashMap<Common::String, BitBlock *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> BitBlockCache;

	/** Reuse a decoded resource or a cached failed load. */
	RleBlock *loadRleBlock(RleBlockCache &cache, const Common::String &key);
	BitBlock *loadBitBlock(BitBlockCache &cache, const Common::String &key);
	BitBlock *loadMaskedBitBlock(const Common::String &colorKey, const Common::String &alphaKey);
	/** Release every value in one path-indexed cache. */
	static void clearRleBlockCache(RleBlockCache &cache);
	static void clearBitBlockCache(BitBlockCache &cache);

	/** Bitmap strip shared by all UI text colors. */
	static constexpr const char *kTextFontPath = "bmp/typo";
	/** Name plate sprite shown while dragging a Zoombini. */
	static constexpr const char *kNameBoxSpritePath = "bmp/menu/name_box.rb";
	/** Path format for one map transition overlay. */
	static constexpr const char *kMapTransitionOverlayPathFormat = "bmp/maptrans/%s.bmp";
	/** Path format for the map transition background. */
	static constexpr const char *kMapTransitionBackgroundPathFormat = "#bmp/maptrans/bigmap_background_%d";
	/** Single coverage-mask glyph set tinted per draw, retained until graphics shutdown. */
	BitmapFont *_textFont = nullptr;

	/** Draw one map-overlay RLE sprite retained for the current page. */
	void drawOverlaySprite(ManagedSurface32 *destSurface, const Common::String &name, const Common::Point32 &pos);
	/** Compose the route-map overlays appropriate to the current progress. */
	void drawMapOverlays(ManagedSurface32 *destSurface, PageId srcPageId, int mapRegion, RouteBranch routeBranch);
	/** Return the tint channels for one text color. */
	static void textColorRGB(TextColor color, byte &red, byte &green, byte &blue);
	/** Name-plate sprite drawn under the held Zoombini name. */
	RleBlock *_nameBoxSprite = nullptr;
	/** Active page background borrowed from the page bitmap cache. */
	BitBlock *_background = nullptr;
	/** Decoded bitmaps borrowed by the current page and its transient controls. */
	RleBlockCache _pageRleBlocks;
	BitBlockCache _pageBitBlocks;
	Common::Array<MaskedBitBlockEntry> _pageMaskedBitBlocks;
	/** Decoded bitmaps borrowed by UI and cursors that survive page changes. */
	RleBlockCache _sharedRleBlocks;
	BitBlockCache _sharedBitBlocks;
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
	using Graphics::ManagedSurface::copyRectToSurface;
	using Graphics::ManagedSurface::fillRect;
	using Graphics::ManagedSurface::frameRect;

	/** Fill a 32-bit rectangle after clipping it to this surface. */
	void fillRect(const Common::Rect32 &rect, uint32 color);
	/** Draw a 32-bit rectangular outline after clipping it to this surface. */
	void frameRect(const Common::Rect32 &rect, uint32 color);
	/** Copy a source surface rectangle using 32-bit coordinates after clipping. */
	void copyRectToSurface(const Graphics::Surface &srcSurface, int destX, int destY, const Common::Rect32 &subRect);
	/** Copy a managed surface at a 32-bit destSurface pos after clipping. */
	void blitFrom(const ManagedSurface32 &srcSurface, const Common::Point32 &destPos);
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
 * value when retaining the destSurface contribution. `value` is one 8-bit
 * color channel.
 *
 * A mode 1 RLE pixel stores premultiplied BGR in its first three bytes and
 * inverse alpha in its fourth byte. RLE drawing therefore computes
 * `source + scale(inverseAlpha, destSurface)` for each channel. The
 * @ref BitBlock::drawRleMaskBlend method has an uncompressed bitmap and a
 * separate mask, so it computes
 * `scale(mask, source) + scale(255 - mask, destSurface)` instead.
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
	 * @param value Source or destSurface color-channel value.
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
 * the game's cached BB format. Drawing clips source pixels to the destSurface
 * surface and preserves the destSurface outside the covered region.
 *
 * Cached `.bb` files use the following little-endian layout:
 * @code
 * uint32 serializedAlphaMapPointer
 * uint32 serializedPixelPointer
 * uint32 bufferSize
 * int32  width
 * int32  height
 * uint32 externalPixelSize
 * byte   bgrPixels[bufferSize]
 * @endcode
 *
 * The first two fields are serialized runtime pointer slots and are ignored.
 * Both size fields normally equal `width * height * 3`.
 * Pixels are stored as row-major BGR24 from the top row to the bottom row and
 * are expanded to opaque RGBA32 when loaded.
 * A truncated cache retains every complete BGR pixel and fills the unavailable
 * tail with opaque black.
 *
 * Color `.bmp` and `.bmt` inputs are standard 24-bit BGR bitmaps.
 * A separate alpha bitmap must decode to an indexed surface with matching
 * dimensions, and each stored palette index is used directly as pixel coverage.
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
	void drawToSurface(ManagedSurface32 *destSurface, const Common::Point32 &pos) const;
	/** Draw the full bitmap while treating its top-left pixel color as transparent. */
	void drawToSurfaceColorKey(ManagedSurface32 *destSurface, const Common::Point32 &pos) const;
	/** Draw @p srcRect from this bitmap without alpha blending at @p pos. */
	void drawSubRect(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect &srcRect) const;
	/**
	 * Draw the full bitmap with the separate-mask `/255` blend rule at @p pos.
	 *
	 * The source channel is added as supplied, while the destSurface channel is
	 * scaled by `(255 - mask) / 255`; this path intentionally does not use the
	 * RLE `/256` lookup table.
	 */
	void drawAlphaBlend(ManagedSurface32 *destSurface, const Common::Point32 &pos) const;
	/**
	 * Draw a bitmap-mask pair at @p pos using the RLE premultiplied blend rule.
	 *
	 * @p alphaLUT scales the raw source channel by the mask and the destSurface
	 * channel by the mask's inverse, using the renderer's `/256` rule.
	 */
	void drawRleMaskBlend(ManagedSurface32 *destSurface, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const;

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

	/** Clip and convert a bitmap region, optionally treating the full bitmap's top-left color as transparent. */
	void drawToSurfaceInternal(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect32 &srcRect, bool useColorKey) const;
	/** Decode a 24-bit BMP from @p stream through the shared image decoder. */
	bool loadColorBMP(Common::SeekableReadStream *stream);
	/** Decode an indexed alpha-mask BMP from @p stream through the shared image decoder. */
	bool loadAlphaBMP(Common::SeekableReadStream *stream);
	/** Exchange bitmap buffer state with @p other. */
	void swapData(BitBlock &other);
	/**
	 * Blend one bitmap channel with the destSurface through a separate mask.
	 *
	 * The source channel is added as supplied, and only the destSurface is
	 * scaled by the mask's inverse using integer `/255` division.
	 */
	static byte blendChannel(byte srcSurface, byte dest, byte mask);
};

/**
 * Represents one RLE-compressed sprite frame in the engine's drawing format.
 *
 * Every frame begins with this little-endian 24-byte header:
 * @code
 * uint32 cachedEffectiveHeight
 * uint32 serializedDataPointer
 * uint32 payloadSize
 * int32  width
 * int32  height
 * uint32 uninitializedSerializedField
 * @endcode
 *
 * Only the payload size, width, and height are resource metadata.
 * The other fields are consumed but ignored.
 * A standalone `.rb` record stores a duplicate 32-bit payload size immediately
 * after this header, while animation containers supply an outer size according
 * to their own layout and then use the same header and RLE payload.
 *
 * The shared little-endian RLE payload has this structure:
 * @code
 * uint16 effectiveHeight
 * repeat until the declared payload ends:
 *     int16  xOffset
 *     int16  yOffset
 *     int16  pixelCount
 *     uint8  mode
 *     pixel[pixelCount]
 * @endcode
 *
 * Mode 0 stores three bytes per pixel in BGR order. These pixels are completely
 * opaque and replace the destination unless an explicit color key skips them.
 * Mode 1 stores four bytes per pixel: premultiplied BGR followed by inverse
 * alpha. Each destination channel is replaced with
 * `min(255, source + floor(destination * inverseAlpha / 256))` through an
 * @ref AlphaBlendLUT. Areas omitted by the span list leave the destination
 * unchanged.
 *
 * Loading separates the serialized span metadata from aligned four-byte pixel
 * storage. Mode 0 gains an unused fourth byte, while mode 1 retains its inverse
 * alpha byte without conversion.
 * Drawing clips malformed or off-screen spans instead of writing outside the
 * destSurface surface.
 */
class RleBlock {
public:
	/** Construct an empty RLE frame bound to @p vm. */
	explicit RleBlock(Zoombini2Engine *vm);
	/** Release the decoded pixel storage. */
	~RleBlock();

	/** Load the recoverable prefix of an RB or AN frame record from @p stream. */
	bool loadFromStream(Common::SeekableReadStream *stream);
	/** Open and load an RB record, ignoring warned trailing bytes. */
	bool loadFromFile(const Common::Path &path);
	/** Resolve @p basePath to an RB cache path and load it. */
	bool load(const Common::Path &basePath);
	/** Load the recoverable prefix of an ANM frame with a trusted size boundary. */
	bool loadAnimationFrame(Common::SeekableReadStream *stream, uint32 outerSize);

	/**
	 * Draw this frame at @p pos using opaque copies or lookup-table blending.
	 *
	 * Mode 1 spans use their premultiplied BGR channels and inverse-alpha byte
	 * with @p alphaLUT.
	 */
	void drawToScreen(ManagedSurface32 *destSurface, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const;
	/** Draw this frame while skipping opaque pixels matching @p red, @p green, and @p blue. */
	void drawToScreenColorKey(ManagedSurface32 *destSurface, const Common::Point32 &pos, byte red, byte green, byte blue, const AlphaBlendLUT &alphaLUT) const;
	/**
	 * Draw this frame inside @p clip using opaque copies or lookup-table blending.
	 *
	 * Mode 1 spans use their premultiplied BGR channels and inverse-alpha byte
	 * with @p alphaLUT.
	 */
	void drawToScreenClipped(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect32 &clip, const AlphaBlendLUT &alphaLUT) const;

	/** Return the frame dimensions in pixels. */
	const Size32 &getSize() const { return _size; }
	/** Return the frame width in pixels. */
	int32 getWidth() const { return _size.width; }
	/** Return the frame height in pixels. */
	int32 getHeight() const { return _size.height; }
	/** Return whether frame data has been loaded. */
	bool isValid() const { return _loaded; }

private:
	/** Serialized pixel layout and composition rule selected by one span's mode byte. */
	enum class SpanMode : byte {
		/** Mode 0: three-byte opaque BGR pixels which replace the destination. */
		kOpaqueBgr00 = 0,
		/** Mode 1: premultiplied BGR plus inverse alpha for the original composition rule. */
		kInverseAlphaBgr01 = 1
	};

	/** One decoded span whose pixels occupy a contiguous range in @ref RleBlock::_pixels. */
	struct Span {
		/** Signed horizontal offset from the frame's draw position. */
		int32 xOffset = 0;
		/** Signed vertical offset from the frame's draw position. */
		int32 yOffset = 0;
		/** First four-byte pixel record in @ref RleBlock::_pixels. */
		uint32 pixelOffset = 0;
		/** Number of consecutive pixels in this span. */
		uint32 pixelCount = 0;
		/** Serialized pixel layout and composition rule for this span. */
		SpanMode mode = SpanMode::kOpaqueBgr00;
	};

	/** Borrowed vm used to resolve RLE resources. */
	Zoombini2Engine *_vm;
	/** Frame dimensions in pixels. */
	Size32 _size = Size32();
	/** Decoded span metadata held by this frame. */
	Common::Array<Span> _spans;
	/** Aligned decoded pixel storage held by this frame. */
	uint32 *_pixels = nullptr;
	/** Number of four-byte records in @ref RleBlock::_pixels. */
	uint32 _pixelCount = 0;
	/** Whether a structurally bounded frame prefix has been loaded. */
	bool _loaded = false;

	/** Decode the structurally bounded span prefix into metadata and aligned pixels, reporting whether a malformed tail was discarded. */
	static bool decodeSpans(const byte *srcData, uint32 srcSize, Common::Array<Span> &spans, uint32 *&pixels, uint32 &pixelCount, bool &salvaged);
	/**
	 * Add one premultiplied source channel to the destSurface scaled by @p inverseAlpha.
	 *
	 * Mode 1 RLE pixels store the premultiplied source channel directly, so only
	 * the destSurface term needs an @p alphaLUT lookup.
	 * The sum is saturated at 255.
	 */
	static byte blendChannel(byte srcSurface, byte dest, byte inverseAlpha, const AlphaBlendLUT &alphaLUT);
	/** Draw this frame with clipping and an optional opaque-span color key. */
	void drawToScreenInternal(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect32 &clip, const AlphaBlendLUT &alphaLUT,
							  bool useColorKey, byte red, byte green, byte blue) const;
	/** Exchange frame buffer state with @p other. */
	void swapData(RleBlock &other);
};

/**
 * Represents the ordered RLE frames decoded from one AN animation file.
 *
 * An `.an` file is one little-endian frame sequence:
 * @code
 * uint32 frameCount
 * repeat frameCount times:
 *     byte   rleHeader[24]
 *     uint32 externalDataSize
 *     byte   rlePayload[externalDataSize]
 * @endcode
 *
 * Each embedded frame has the same header, duplicate size, and span payload as
 * a standalone @ref RleBlock `.rb` record.
 * The file contains only frame order and image data; playback timing, events,
 * sounds, and looping policy are supplied by the caller.
 */
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
 * The direct decoder accepts a standard uncompressed 1-bit BMP:
 * @code
 * 14-byte BITMAPFILEHEADER
 * BITMAPINFOHEADER and two-color palette
 * padding up to BITMAPFILEHEADER::bfOffBits
 * repeat abs(height) rows:
 *     byte packedPixels[((width + 31) / 32) * 4]
 * @endcode
 *
 * Pixels are packed most-significant bit first, rows are padded to a four-byte
 * boundary, and the sign of the BMP height selects bottom-up or top-down rows.
 * Other indexed BMP variants use the shared bitmap decoder and retain their
 * palette indices as one byte per pixel.
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
 *
 * An `.anm` file serializes every grid entry in that exact nested order:
 * @code
 * repeat 100 movementOrAnimation cells:
 *     repeat 5 bodyOrFeature layers:
 *         repeat 6 baseOrFeatureValue variants:
 *             uint32 frameCount
 *             repeat frameCount times:
 *                 uint32 outerDataSize
 *                 byte   rleHeader[24]
 *                 byte   rlePayload[outerDataSize]
 * @endcode
 *
 * The outer size precedes the embedded @ref RleBlock header and must match the
 * payload size stored in that header.
 * This differs from `.an`, where the duplicate size follows the RLE header.
 * The file does not store playback timing; @ref setFrameDelay supplies one
 * page-configured delay for the loaded grid.
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
	/** Forward legacy drawing calls to @ref Gfx::drawZoombini with the supplied blend table and clip. */
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

	/** Set the button rectangle from a screen pos and size. */
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
	int drawAndHitTest(ManagedSurface32 *destSurface, const Common::Point32 &mousePos, const AlphaBlendLUT &alphaLUT);

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

	/** Uncompressed normal-state image borrowed from the graphics page cache. */
	BitBlock *_normalBB = nullptr;
	/** Uncompressed highlighted-state image borrowed from the graphics page cache. */
	BitBlock *_hoverBB = nullptr;
	/** Uncompressed disabled-state image borrowed from the graphics page cache. */
	BitBlock *_disabledBB = nullptr;
	/** RLE normal-state image borrowed from the graphics page cache. */
	RleBlock *_normalRle = nullptr;
	/** RLE highlighted-state image borrowed from the graphics page cache. */
	RleBlock *_hoverRle = nullptr;
	/** RLE disabled-state image borrowed from the graphics page cache. */
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
 * A font uses two files whose `.bmt` extension still contains standard BMP data:
 * @code
 * <base>.bmt    24-bit BGR color strip
 * <base>-A.bmt  indexed coverage strip with matching dimensions
 * @endcode
 *
 * Column zero is reserved and skipped.
 * All-zero columns in the coverage strip separate glyphs, while each contiguous
 * run of nonzero columns forms the next glyph in the fixed character sequence.
 * The retained glyph width includes two trailing pixels beyond that nonzero run.
 *
 * The supported glyph sequence contains uppercase letters, lowercase letters,
 * digits, and eighteen punctuation characters. Spaces and unsupported bytes
 * advance by @ref BitmapFont::kSpaceWidth without drawing.
 *
 * The strip is loaded once and only its per-pixel coverage masks are retained.
 * The glyph color is a uniform tint applied at draw time, so this single glyph
 * set serves every text color.
 */
class BitmapFont {
public:
	/** Number of glyphs in the fixed font-strip mapping. */
	static constexpr int kNumGlyphs = 81;
	/** Horizontal advance used for spaces and unsupported characters. */
	static constexpr int kSpaceWidth = 10;

	/** Construct an unloaded font bound to @p vm. */
	explicit BitmapFont(Zoombini2Engine *vm);

	/** Load the BMT color-and-alpha pair and extract the coverage masks. */
	bool load(const Common::Path &basePath);
	/** Draw @p text tinted with @p color at @p pos and return its horizontal pixel advance. */
	int drawString(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::String &text, byte red, byte green, byte blue, const AlphaBlendLUT &alphaLUT) const;
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
	/** One extracted glyph: its coverage mask and dimensions. */
	struct Glyph {
		/** Per-pixel coverage, 0 (transparent) through 255 (opaque). */
		byte *mask = nullptr;
		/** Glyph width in pixels. */
		int width = 0;
		/** Glyph height in pixels. */
		int height = 0;
		/** Release the coverage mask. */
		~Glyph() { delete[] mask; }
	};
	/** Glyph coverage masks held by this font in character-mapping order. */
	Glyph _glyphs[kNumGlyphs];

	/**
	 * Draw one glyph's coverage mask tinted with @p color at @p pos.
	 *
	 * Full-coverage pixels copy the tint; partial-coverage pixels add the
	 * tint scaled by the coverage and the destination scaled by its inverse,
	 * matching the RLE premultiplied blend rule.
	 */
	void drawGlyph(ManagedSurface32 *destSurface, const Glyph &glyph, const Common::Point32 &pos, byte red, byte green, byte blue, const AlphaBlendLUT &alphaLUT) const;
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
	void draw(ManagedSurface32 *destSurface, const Common::Point32 &mousePos, const AlphaBlendLUT &alphaLUT);

	/** Return the current music volume percentage. */
	int getMusicVolume() const { return _settings._music; }
	/** Return the current sound-effect volume percentage. */
	int getSfxVolume() const { return _settings._sfx; }
	/** Return the current speech volume percentage. */
	int getSpeechVolume() const { return _settings._speech; }
	/** Return the initial music volume percentage. */
	int getInitialMusicVolume() const { return _settings._initialMusic; }
	/** Return the initial sound-effect volume percentage. */
	int getInitialSfxVolume() const { return _settings._initialSfx; }
	/** Return the initial speech volume percentage. */
	int getInitialSpeechVolume() const { return _settings._initialSpeech; }

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
	/** Resource paths for panel artwork and audio previews. */
	static constexpr const char *kGaugeImagePath = "bmp/menu/OPTION - Jauge.rb";
	static constexpr const char *kOkNormalPath = "bmp/menu/MENU - Valid - OK";
	static constexpr const char *kOkNormalMaskPath = "bmp/menu/MENUValidOK-a";
	static constexpr const char *kOkHighlightPath = "bmp/menu/MENU - Valid - OK highlight";
	static constexpr const char *kOkHighlightMaskPath = "bmp/menu/OKHighlight-a";
	static constexpr const char *kNoNormalPath = "bmp/menu/MENU - Valid - NO";
	static constexpr const char *kNoNormalMaskPath = "bmp/menu/MENUValidNO-a";
	static constexpr const char *kNoHighlightPath = "bmp/menu/MENU - Valid - NO highlight";
	static constexpr const char *kNoHighlightMaskPath = "bmp/menu/NOHighlight-a";
	static constexpr const char *kMusicNormalPath = "bmp/menu/OPTION - Musique NORMAL";
	static constexpr const char *kMusicHighlightPath = "bmp/menu/OPTION - Musique HIGHLIGHT";
	static constexpr const char *kMusicMaskPath = "bmp/menu/OPTIONMusique-a";
	static constexpr const char *kSfxNormalPath = "bmp/menu/OPTION - Bruitages NORMAL";
	static constexpr const char *kSfxHighlightPath = "bmp/menu/OPTION - Bruitages HILITE";
	static constexpr const char *kSfxMaskPath = "bmp/menu/OPTION_Bruitages-a";
	static constexpr const char *kSpeechNormalPath = "bmp/menu/OPTION - Dialogues NORMAL";
	static constexpr const char *kSpeechHighlightPath = "bmp/menu/OPTION - Dialogues HILITE";
	static constexpr const char *kSpeechMaskPath = "bmp/menu/OPTIONDialogues-a";
	static constexpr const char *kSpeechPreviewSoundPath = "sounds/voice.wav";
	static constexpr const char *kSfxPreviewSoundPath = "sounds/fx/03-BS01.wav";

	/** Borrowed vm used by the panel's gauge and button resources. */
	Zoombini2Engine *_vm;
	/** Current and cancellation-baseline audio levels edited by this session. */
	VolumeSettings _settings;

	/** Current music gauge endpoint. */
	int _musicSliderX = kSliderMaxX;
	/** Current sound-effect gauge endpoint. */
	int _sfxSliderX = kSliderMaxX;
	/** Current speech gauge endpoint. */
	int _speechSliderX = kSliderMaxX;
	/** Dragged slider index, or -1 when no slider is captured. */
	int _activeSlider = -1;
	/** Most recent mouse pos observed while the primary button was held. */
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
