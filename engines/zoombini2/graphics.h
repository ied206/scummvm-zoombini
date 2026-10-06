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

#include "graphics/font.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"

#include "zoombini2/state.h"

namespace Zoombini2 {

/** One RGB color with eight bits per channel and no alpha component. */
struct RGBColor {
	byte r;
	byte g;
	byte b;

	constexpr RGBColor(byte red = 0, byte green = 0, byte blue = 0) : r(red), g(green), b(blue) {}
	/** Decode a packed pixel with @p pixelFormat. */
	static RGBColor fromPixel(uint32 pixel, const Graphics::PixelFormat &pixelFormat) {
		RGBColor color;
		pixelFormat.colorToRGB(pixel, color.r, color.g, color.b);
		return color;
	}
	/** Encode this color with @p pixelFormat. */
	uint32 toPixel(const Graphics::PixelFormat &pixelFormat) const { return pixelFormat.RGBToColor(r, g, b); }
	/** Return the greatest RGB channel value. */
	byte getMaxChannel() const { return MAX(r, MAX(g, b)); }
	/** Return the least RGB channel value. */
	byte getMinChannel() const { return MIN(r, MIN(g, b)); }
};

class AlphaBlendLUT;

/** Optional color presentation for small displays and red-green color vision deficiency. */
enum class ColorAssistMode : byte {
	kOriginal00 = 0,
	kSmallScreen01 = 1,
	kRedGreen02 = 2
};

/**
 * Signed width and height without positional semantics.
 * Modeled after @ref Common::PointBase struct.
 */
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
#define BEGIN_Z2_SIZE_TYPE(T, Size) \
	struct Size : public SizeBase<T, Size> {
#define END_Z2_SIZE_TYPE(T, Size)                                                                       \
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

BEGIN_Z2_SIZE_TYPE(int16, Size16)
END_Z2_SIZE_TYPE(int16, Size16)
BEGIN_Z2_SIZE_TYPE(int32, Size32)
constexpr Size32(const Size16 &size) : SizeBase(static_cast<int32>(size.width), static_cast<int32>(size.height)) {}
END_Z2_SIZE_TYPE(int32, Size32)

class Zoombini2Engine;
class SoundManager;
class Animation;
class AnimationRunner;
class AreaMask;
class BitBlock;
class BmtFont;
class ManagedSurface32;
class PageLayerStack;
class RleBlock;
class ZoombiniRunner;
class ZoombiniAnimation;

/** Initializes the game screen and provides page-facing graphics operations. */
class Gfx : public Common::NonCopyable {
public:
	/** Construct the graphics interface for one game instance. */
	explicit Gfx(Zoombini2Engine *vm);
	/** Release the graphics interface for one game instance. */
	~Gfx();

	/** Return the fixed drawing surface for this game instance. */
	ManagedSurface32 *getScreen() const { return _screen; }
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
	/** Draw an RLE sprite while skipping opaque pixels matching @p colorKey. */
	void drawRleBlockColorKey(ManagedSurface32 *destSurface, const RleBlock *sprite, const Common::Point32 &pos, const RGBColor &colorKey) const;
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
	void drawSharedRleBlockColorKey(ManagedSurface32 *destSurface, const Common::String &key, const Common::Point32 &pos, const RGBColor &colorKey) {
		if (destSurface)
			drawRleBlockColorKey(destSurface, loadSharedRleBlock(key), pos, colorKey);
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
	/** Draw a nose-trait icon with the same display palette as Zoombini nose layers. */
	void drawPageNoseTraitSprite(ManagedSurface32 *destSurface, const Common::String &key, const Common::Point32 &pos, byte value);
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
	void drawZoombiniPreview(ManagedSurface32 *destSurface, const ZoombiniAnimation *animation, const byte (&selectedValues)[ZmbTrait::kTraitKindCount], const Common::Point32 &pos) const;
	/** Compose the same picker preview with an explicit blend table. */
	static void drawZoombiniPreview(ManagedSurface32 *destSurface, const ZoombiniAnimation *animation, const byte (&selectedValues)[ZmbTrait::kTraitKindCount], const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT);
	/** Select the display color for a nose trait without changing its logical value. */
	static bool noseColorRGB(ColorAssistMode mode, byte value, RGBColor &color);
	/** Shift a nose pixel toward the display hue while retaining its value and saturation variation. */
	static RGBColor recolorNoseGradientRGB(const RGBColor &srcColor, const RGBColor &targetColor);
	/** Draw one visible runner and its available drop-target glow at the scrolled or dragged anchor without advancing animation. */
	void drawZoombiniRunner(ManagedSurface32 *destSurface, const ZoombiniRunner *runner, const Common::Rect32 *clip = nullptr, int scrollX = 0, int backgroundWidth = 800) const;
	/** Draw one runner with temporary presentation traits without changing its gameplay traits. */
	void drawZoombiniRunnerWithTraits(ManagedSurface32 *destSurface, const ZoombiniRunner *runner, const ZmbTrait &presentationTraits,
									  const Common::Rect32 *clip = nullptr, int scrollX = 0, int backgroundWidth = 800) const;
	/** Apply runner visibility, bounds and frame selection with an explicit blend table and optional presentation traits. */
	static void drawZoombiniRunner(ManagedSurface32 *destSurface, const ZoombiniRunner *runner, const AlphaBlendLUT &alphaLUT,
								   const Common::Rect32 *clip = nullptr, int scrollX = 0, int backgroundWidth = 800,
								   const RleBlock *dropTargetIndicator = nullptr, const ZmbTrait *presentationTraits = nullptr);
	/** Color variants of the shared bitmap text strip. */
	enum class TextColor {
		kDark00 = 0,
		kBlue01 = 1,
		kGreen02 = 2,
		kWhite03 = 3,
		kRed04 = 4
	};

	/** Load the shared text strip on demand; failed loads can be retried by the caller. */
	bool loadTextFont(TextColor color);
	/** Return whether the shared text strip is ready. */
	bool hasTextFont(TextColor color) const;
	/** Draw bitmap text tinted with @p color and return its horizontal advance. */
	int drawText(ManagedSurface32 *destSurface, TextColor color, const Common::Point32 &pos, const Common::String &text) const;
	/** Measure text by BmtFont with the active release's game rules. */
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
	ManagedSurface32 *createMapTransitionBackground(PageId srcPageId, int mapRegion);

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
	/** Yellow mask drawn beneath a held Zoombini over an available drop target. */
	static constexpr const char *kDropTargetGlowPath = "bmp/cursor/glow";
	/** Path format for one map transition overlay. */
	static constexpr const char *kMapTransitionOverlayPathFormat = "bmp/maptrans/%s.bmp";
	/** Path format for the map transition background. */
	static constexpr const char *kMapTransitionBackgroundPathFormat = "#bmp/maptrans/bigmap_background_%d";
	/** Single coverage-mask glyph set tinted per draw, retained until graphics shutdown. */
	BmtFont *_textFont = nullptr;

	/** Draw one map-overlay RLE sprite retained for the current page. */
	void drawOverlaySprite(ManagedSurface32 *destSurface, const Common::String &name, const Common::Point32 &pos);
	/** Compose the route-map overlays appropriate to the current progress. */
	void drawMapOverlays(ManagedSurface32 *destSurface, PageId srcPageId, int mapRegion);
	/** Return the tint for one text color. */
	static RGBColor textColor(TextColor color);
	/** Name-plate sprite drawn under the held Zoombini name. */
	RleBlock *_nameBoxSprite = nullptr;
	/** Shared drop-target glow retained for the game instance. */
	RleBlock *_dropTargetGlowSprite = nullptr;
	/** Active page background borrowed from the page bitmap cache. */
	BitBlock *_background = nullptr;
	/** Decoded bitmaps borrowed by the current page and its transient controls. */
	RleBlockCache _pageRleBlocks;
	BitBlockCache _pageBitBlocks;
	Common::Array<MaskedBitBlockEntry> _pageMaskedBitBlocks;
	/** Decoded bitmaps borrowed by UI and cursors that survive page changes. */
	RleBlockCache _sharedRleBlocks;
	BitBlockCache _sharedBitBlocks;
	/** Fixed drawing surface presented by the engine. */
	ManagedSurface32 *_screen = nullptr;
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
	/** Scale source and destination channels separately, then add with saturation at 255. */
	RGBColor blend(const RGBColor &srcColor, const RGBColor &destColor, byte sourceFactor, byte destFactor) const;

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
class BitBlock : public Common::NonCopyable {
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
	/** Decode the bounded BB pixel prefix and retain only rows containing recovered pixels. */
	bool loadBBStream(Common::SeekableReadStream &stream, const Common::Path &path);
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
class RleBlock : public Common::NonCopyable {
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
	/** Draw this frame while skipping opaque pixels matching @p colorKey. */
	void drawToScreenColorKey(ManagedSurface32 *destSurface, const Common::Point32 &pos, const RGBColor &colorKey, const AlphaBlendLUT &alphaLUT) const;
	/**
	 * Draw this frame inside @p clip using opaque copies or lookup-table blending.
	 *
	 * Mode 1 spans use their premultiplied BGR channels and inverse-alpha byte
	 * with @p alphaLUT.
	 */
	void drawToScreenClipped(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect32 &clip, const AlphaBlendLUT &alphaLUT) const;
	/** Draw this frame with its chromatic pixels shifted toward one RGB color, preserving shading and alpha. */
	void drawToScreenRecolored(ManagedSurface32 *destSurface, const Common::Point32 &pos, const RGBColor &targetColor,
							   const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip = nullptr, byte sourceBrightness = 255) const;
	/** Draw this frame in a new hue while retaining each chromatic pixel's value and saturation variation. */
	void drawToScreenRecoloredWithGradient(ManagedSurface32 *destSurface, const Common::Point32 &pos, const RGBColor &targetColor, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip = nullptr) const;
	/** Draw this frame mirrored horizontally within its own width. */
	void drawToScreenMirrored(ManagedSurface32 *destSurface, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const;
	/** Draw the RLE coverage using one color while retaining inverse-alpha edges. */
	void drawToScreenSolidColor(ManagedSurface32 *destSurface, const Common::Point32 &pos, const RGBColor &color, const AlphaBlendLUT &alphaLUT) const;

	/** Return the frame dimensions in pixels. */
	const Size32 &getSize() const { return _size; }
	/** Return the frame width in pixels. */
	int32 getWidth() const { return _size.width; }
	/** Return the frame height in pixels. */
	int32 getHeight() const { return _size.height; }
	/** Return whether frame data has been loaded. */
	bool isValid() const { return _loaded; }

private:
	/** Source pixel layout of decoded RLE frame data. */
	static constexpr Graphics::PixelFormat kSourceFormat = Graphics::PixelFormat::createFormatBGRA32(false);
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
	/** Target color and source adjustment used while drawing a recolored frame. */
	struct RecolorSettings {
		RGBColor targetColor;
		byte sourceBrightness;
		bool preserveGradient;
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
	/** Recolor a BGR pixel while retaining neutral highlights and optionally preserving source saturation variation. */
	static bool recolorPixel(const byte *source, bool premultiplied, const RGBColor &targetColor, byte sourceBrightness, bool preserveGradient, RGBColor &adjustedColor);
	/** Draw this frame with clipping and an optional opaque-span color key. */
	void drawToScreenInternal(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::Rect32 &clip, const AlphaBlendLUT &alphaLUT,
							  const RGBColor *colorKey = nullptr, const RecolorSettings *recolor = nullptr) const;
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
class Animation : public Common::NonCopyable {
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
class ZoombiniAnimation : public Common::NonCopyable {
public:
	/** Number of movement and animation cells. */
	static constexpr int kDim0 = 100;
	/** Number of sprite layers per cell. */
	static constexpr int kDim1 = ZmbTrait::kTraitKindCount + 1;
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
	/** Return the current presentation setting for this animation's game instance. */
	ColorAssistMode getColorAssistMode() const;
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
	kMenuButtonNext = 0,     ///< Scroll toward the preceding visible savefile rows.
	kMenuButtonPrev = 1,     ///< Scroll toward the following visible savefile rows.
	kMenuButtonStart = 2,    ///< Start the selected saved adventure.
	kMenuButtonOptions = 3,  ///< Open the volume panel.
	kMenuButtonNew = 4,      ///< Begin entry of a new savefile name.
	kMenuButtonPractice = 5, ///< Open the practice map.
	kMenuButtonQuit = 6,     ///< Request exit from the sign-in screen.
	kMenuButtonCount = 7     ///< Number of sign-in buttons.
};

/**
 * Renders text with the glyphs from one font strip bitmap.
 *
 * A font uses two files whose `.bmt` extension contains standard BMP data:
 * @code
 * <base>.bmt    24-bit BGR color strip
 * <base>-A.bmt  indexed coverage strip with matching dimensions
 * @endcode
 *
 * Pixel column zero (x = 0) is reserved and skipped.
 * Blank pixel columns (x != 0), whose coverage values are all zero, separate glyphs.
 * A nonblank pixel column contains at least one nonzero coverage value.
 * Each contiguous run of nonblank pixel columns forms the next glyph in the fixed character sequence.
 * The retained glyph width includes two trailing pixel columns beyond that run.
 *
 * Example coverage strip: '.' means zero coverage and '#' means nonzero coverage.
 * @code
 * x:  0 1 2 3 4 5 6 7 8 9
 *     . . # . . . # # . .
 *     . # . # . . # . # .
 *     . # # # . . # # . .
 *     . # . # . . # . # .
 *       [ A ]     [ B ]
 * @endcode
 * Glyph A occupies pixel columns 1-3, and blank columns 4-5 separate it from glyph B in columns 6-8.
 *
 * The usual strip has 81 glyphs.
 * The v1.1SE strip has 84.
 * The v1.0HE strip has 80 and puts Hebrew
 * shapes in both Latin letter ranges, while Hebrew bytes select matching
 * slots directly. Spaces advance by @ref BmtFont::kSpaceWidth.
 *
 * The strip is loaded once and only its per-pixel coverage masks are retained.
 * The glyph color is a uniform tint applied at draw time, so this single glyph
 * set serves every text color.
 * Game strings retain the Hebrew and Swedish draw/measure rules through
 * @ref BmtFont::drawString and @ref BmtFont::getStringWidth.
 */
class BmtFont : public Graphics::Font {
public:
	/** Maximum number of glyphs in the font-strip mapping. */
	static constexpr int kNumGlyphs = 84;
	/** Horizontal advance used for spaces. */
	static constexpr int kSpaceWidth = 10;

	/** Construct an unloaded font bound to @p vm. */
	explicit BmtFont(Zoombini2Engine *vm);

	/** Load the BMT color-and-alpha pair and extract the coverage masks. */
	bool load(const Common::Path &basePath);
	int getFontHeight() const override;
	int getMaxCharWidth() const override;
	int getCharWidth(uint32 character) const override;
	using Graphics::Font::getBoundingBox;
	Common::Rect getBoundingBox(uint32 character) const override;
	using Graphics::Font::drawChar;
	void drawChar(Graphics::Surface *destSurface, uint32 character, int x, int y, uint32 color) const override;
	using Graphics::Font::drawString;
	/** Draw game bytes, including Hebrew traversal, and return the horizontal advance. */
	int drawString(ManagedSurface32 *destSurface, const Common::Point32 &pos, const Common::String &text, const RGBColor &color) const;
	using Graphics::Font::getStringWidth;
	/** Measure game bytes with the Hebrew and Swedish release-specific rules. */
	int getStringWidth(const Common::String &text) const;
	/** Return the glyph index for @p character, or -1 when it is unsupported. */
	int charToGlyphIndex(uint32 ch) const;
	/** Return whether glyph extraction completed. */
	bool isLoaded() const { return _loaded; }

private:
	/** Borrowed vm used to load the font strip and construct glyphs. */
	Zoombini2Engine *_vm;
	/** Whether the font strip has been processed. */
	bool _loaded = false;
	/** Largest glyph advance, including the fixed space advance. */
	int _maxCharWidth = 0;
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
	void drawGlyph(Graphics::Surface *destSurface, const Glyph &glyph, const Common::Point32 &pos, const RGBColor &color, const AlphaBlendLUT &alphaLUT) const;
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
class VolumePanel : public Common::NonCopyable {
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
	int getMusicVolume() const { return _settings.getMusic(); }
	/** Return the current sound-effect volume percentage. */
	int getSfxVolume() const { return _settings.getSfx(); }
	/** Return the current speech volume percentage. */
	int getSpeechVolume() const { return _settings.getSpeech(); }
	/** Return the initial music volume percentage. */
	int getInitialMusicVolume() const { return _settings.getInitialMusic(); }
	/** Return the initial sound-effect volume percentage. */
	int getInitialSfxVolume() const { return _settings.getInitialSfx(); }
	/** Return the initial speech volume percentage. */
	int getInitialSpeechVolume() const { return _settings.getInitialSpeech(); }

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
