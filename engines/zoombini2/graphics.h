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

class SoundManager;

/**
 * Screen surface with 32-bit coordinates.
 *
 * @remark Zoombini2 runs on 800x600, so 16-bit Point/Rect is enough.
 * But the original coordinates are encoded in 32-bit.
 * Make ManageSurface to take Point32/Rect32 as a parameter to simply the code.
 */
class ManagedSurface32 : public Graphics::ManagedSurface {
public:
	/** Create a screen surface with the supplied dimensions and pixel format. */
	ManagedSurface32(int width, int height, const Graphics::PixelFormat &pixelFormat)
		: Graphics::ManagedSurface(width, height, pixelFormat) {}

	using Graphics::ManagedSurface::blitFrom;
	using Graphics::ManagedSurface::fillRect;

	/** Fill a 32-bit rectangle after clipping it to this surface. */
	void fillRect(const Common::Rect32 &rect, uint32 color);
	/** Copy a managed surface at a 32-bit destination pos after clipping. */
	void blitFrom(const Graphics::ManagedSurface &src, const Common::Point32 &destPos);
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
	static const int kValueCount = 256;

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
	/** Construct an empty bitmap. */
	BitBlock();
	/** Release the owned pixel and alpha buffers. */
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
	void createEmpty(int width, int height, bool withAlpha);

	/** Draw the full bitmap without alpha blending at @p pos. */
	void drawToSurface(Graphics::ManagedSurface *dst, const Common::Point32 &pos) const;
	/** Draw @p srcRect from this bitmap without alpha blending at @p pos. */
	void drawSubRect(Graphics::ManagedSurface *dst, const Common::Point32 &pos, const Common::Rect &srcRect) const;
	/**
	 * Draw the full bitmap with the separate-mask `/255` blend rule at @p pos.
	 *
	 * The source channel is added as supplied, while the destination channel is
	 * scaled by `(255 - mask) / 255`; this path intentionally does not use the
	 * RLE `/256` lookup table.
	 */
	void drawAlphaBlend(Graphics::ManagedSurface *dst, const Common::Point32 &pos) const;
	/**
	 * Draw a bitmap-mask pair at @p pos using the RLE premultiplied blend rule.
	 *
	 * @p alphaLUT scales the raw source channel by the mask and the destination
	 * channel by the mask's inverse, using the renderer's `/256` rule.
	 */
	void drawRleMaskBlend(Graphics::ManagedSurface *dst, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const;

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
	/** RGBA pixel storage held by this bitmap, with four bytes per pixel. */
	byte *_pixels;
	/** Optional one-byte-per-pixel alpha storage held by this bitmap, or nullptr. */
	byte *_alphaMap;

	/** Decode a 24-bit bottom-up BMP from @p stream. */
	bool loadColorBMP(Common::SeekableReadStream *stream);
	/** Decode an 8-bit alpha-mask BMP from @p stream. */
	bool loadAlphaBMP(Common::SeekableReadStream *stream);
	/** Exchange owned bitmap state with @p other. */
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
	/** Construct an empty RLE frame. */
	RleBlock();
	/** Release the owned RLE data. */
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
	void drawToScreen(Graphics::ManagedSurface *dst, const Common::Point32 &pos, const AlphaBlendLUT &alphaLUT) const;
	/**
	 * Draw this frame inside @p clip using opaque copies or lookup-table blending.
	 *
	 * Mode-1 spans use their premultiplied BGR channels and inverse-alpha byte
	 * with @p alphaLUT.
	 */
	void drawToScreenClipped(Graphics::ManagedSurface *dst, const Common::Point32 &pos, const Common::Rect32 &clip, const AlphaBlendLUT &alphaLUT) const;

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
	/** Number of bytes in @ref RleBlock::_rleData after load-time expansion. */
	uint32 _dataSize;
	/** Expanded RLE span storage held by this frame, with four bytes per encoded pixel. */
	byte *_rleData;
	/** Trailing header value retained with the decoded frame. */
	int32 _field20;

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
	/** Exchange owned frame state with @p other. */
	void swapData(RleBlock &other);
};

/** Represents the ordered RLE frames decoded from one AN animation file. */
class Animation {
public:
	/** Construct an animation without frames. */
	Animation();
	/** Release all owned frames. */
	~Animation();

	/** Load every recoverable frame from @p path. */
	bool loadFromFile(const Common::Path &path);

	/** Return the number of loaded frames. */
	int getFrameCount() const { return _frames.size(); }
	/** Return a borrowed frame, or nullptr when @p index is outside the sequence. */
	const RleBlock *getFrame(int index) const;

private:
	/** Ordered frame sequence owned by this animation. */
	Common::Array<RleBlock *> _frames;
};

/** Native 32-bit floating-point scalar used by the optional path evaluator. */
typedef float Float32;
static_assert(sizeof(Float32) == 4, "Float32 must use four-byte native float storage.");

/**
 * Q10 fixed point type
 * 
 * Stores one real number as signed 32-bit value, encoded as Q10 fixed-point.
 *
 * @par Q10 notation
 * `Q10` means that exactly 10 low bits are fractional bits. It does not name
 * the storage width; the `32` in `Fixed32` means that the complete raw word is
 * 32 bits. In Qm.n notation that excludes the sign bit from m, this layout is
 * Q21.10.
 *
 * @par Representation and range
 * The raw two's-complement word has this layout:
 * @code
 *  31             30                         10 9                  0
 * +----------------+----------------------------+--------------------+
 * | sign bit       | 21 remaining integer bits  | 10 fractional bits |
 * +----------------+----------------------------+--------------------+
 * @endcode
 *
 * The complete word represents `rawValue / 1024`, giving a resolution of
 * `1/1024`, or `0.0009765625`. Raw `0x00000400` is `1.0`, `0x00000001` is
 * `1/1024`, and `0xFFFFFC00` is `-1.0`. For negative values, the fractional
 * field is part of the complete two's-complement value rather than an
 * independently signed component.
 *
 * Every `int32` bit pattern represents a finite value. The inclusive range is
 * `-2097152.0` through `2097151.9990234375`; there are no NaN, infinity, or
 * denormal representations. Integer conversion is non-wrapping for inputs from
 * `-2097152` through `2097151`.
 *
 * @par Arithmetic and overflow
 * Arithmetic is not saturating. Addition and subtraction wrap modulo 2^32 when
 * the mathematical result leaves the representable range. Multiplication
 * deliberately retains the low 32 bits of the raw product before shifting
 * right by 10; it produces the expected non-wrapped Q10 result only when the
 * signed raw product fits in an `int32` before that shift. Division requires a
 * non-zero divisor, forms a 64-bit scaled numerator, truncates the raw quotient
 * toward zero, and wraps the quotient if it leaves the 32-bit raw range.
 * Compound operators use the same rules. Comparisons order the signed raw
 * values, and conversion to an integer uses an arithmetic right shift, which
 * rounds a negative fractional value toward negative infinity.
 *
 * Construct raw values with @ref Fixed32::fromRaw and integer values with
 * @ref Fixed32::fromInt.
 */
class Fixed32 {
public:
	/** Construct zero. */
	constexpr Fixed32() : _rawValue(0) {}

	/** Construct a fixed-point value from its signed raw representation. */
	static constexpr Fixed32 fromRaw(int32 rawValue) { return Fixed32(rawValue); }
	/** Convert an integer to Q10, wrapping inputs outside `-2097152` through `2097151`. */
	static Fixed32 fromInt(int32 value);
	/** Quantize a finite in-range floating-point value to Q10 by truncating its scaled raw value toward zero. */
	static Fixed32 fromFloat(Float32 value);
	/** Convert to an integer with an arithmetic right shift toward negative infinity. */
	int32 toInt() const;
	/** Convert the exact Q10 value to native 32-bit floating point. */
	Float32 toFloat() const;

	/** Multiply by @p right, retain the low 32 product bits, and shift by 10. */
	Fixed32 operator*(const Fixed32 &right) const;
	/** Divide by non-zero @p right with a 64-bit numerator and truncate toward zero. */
	Fixed32 operator/(const Fixed32 &right) const;
	/** Subtract @p right with 32-bit wraparound. */
	Fixed32 operator-(const Fixed32 &right) const;
	/** Add @p right with 32-bit wraparound. */
	Fixed32 operator+(const Fixed32 &right) const;
	/** Add @p right with 32-bit wraparound. */
	void operator+=(const Fixed32 &right);
	/** Subtract @p right with 32-bit wraparound. */
	void operator-=(const Fixed32 &right);
	/** Multiply by @p right and assign the wrapped result. */
	void operator*=(const Fixed32 &right);
	/** Divide by @p right and assign the wrapped result. */
	void operator/=(const Fixed32 &right);
	/** Compare the signed raw values. */
	bool operator>(const Fixed32 &right) const;
	/** Compare the raw values for equality. */
	bool operator==(const Fixed32 &right) const;
	/** Compare the raw values for inequality. */
	bool operator!=(const Fixed32 &right) const;
	/** Format the exact fixed-point value without redundant trailing zeroes. */
	Common::String toString() const;

private:
	constexpr explicit Fixed32(int32 rawValue) : _rawValue(rawValue) {}

	static Fixed32 fromRawBits(uint32 value);
	static int32 arithmeticShiftRight(uint32 value);
	static constexpr int kFractionalBits = 10;
	friend struct PointFixed32;

	/** Signed 32-bit raw representation owned by this value. */
	int32 _rawValue;
};

/**
 * A point struct comprised of two Fixed32 coordinate.
 *
 * Component arithmetic follows the wrapping rules of @ref Fixed32.
 * @ref PointFixed32::sqrDist widens both component squares to 64 bits before
 * reducing their sum to Q10, but its final result still wraps to a `Fixed32`.
 */
struct PointFixed32 : public Common::PointBase<Fixed32, PointFixed32> {
private:
	typedef Common::PointBase<Fixed32, PointFixed32> Base;

public:
	/** Construct the fixed-point coordinate (0, 0). */
	constexpr PointFixed32() : Base(Fixed32(), Fixed32()) {}
	/** Construct a coordinate from two fixed-point values. */
	constexpr PointFixed32(const Fixed32 &xValue, const Fixed32 &yValue) : Base(xValue, yValue) {}

	/** Compare both components for equality. */
	bool operator==(const PointFixed32 &right) const;
	/** Compare either component for inequality. */
	bool operator!=(const PointFixed32 &right) const;
	/** Add @p right component by component. */
	PointFixed32 operator+(const PointFixed32 &right) const;
	/** Subtract @p right component by component. */
	PointFixed32 operator-(const PointFixed32 &right) const;
	/** Multiply both components by @p right. */
	PointFixed32 operator*(const Fixed32 &right) const;
	/** Multiply both components by the integer @p right. */
	PointFixed32 operator*(int32 right) const;
	/** Divide both components by @p right. */
	PointFixed32 operator/(const Fixed32 &right) const;
	/** Divide both components by the integer @p right. */
	PointFixed32 operator/(int32 right) const;
	/** Add @p right component by component. */
	void operator+=(const PointFixed32 &right);
	/** Subtract @p right component by component. */
	void operator-=(const PointFixed32 &right);
	/** Multiply both components by @p right. */
	void operator*=(const Fixed32 &right);
	/** Divide both components by @p right. */
	void operator/=(const Fixed32 &right);

	/** Return the squared distance using 64-bit intermediates and a wrapped Q10 result. */
	Fixed32 sqrDist(const PointFixed32 &right) const;
	/** Format both fixed-point components separated by a comma. */
	Common::String toString() const;

private:
	/** Return the unsigned difference between two raw component values. */
	static uint64 rawMagnitudeDifference(const Fixed32 &left, const Fixed32 &right);
};

/** Multiply both components of @p right by @p left. */
PointFixed32 operator*(const Fixed32 &left, const PointFixed32 &right);
/** Multiply both components of @p right by the integer @p left. */
PointFixed32 operator*(int32 left, const PointFixed32 &right);

/**
 * Runtime-selectable evaluator for one cubic Bezier segment.
 *
 * The original-compatible evaluator stores all scalar state in @ref Fixed32.
 * The optional gameplay-improvement evaluator instantiates the same template
 * with @ref Float32. @ref CurveSegment::evaluate advances either evaluator from
 * the stored start tick, step, and optional initial wait.
 */
class CurveSegment : public Common::NonCopyable {
public:
	/** Construct an empty segment using the selected numeric representation. */
	explicit CurveSegment(bool useFloatingPoint = false);
	/** Release the selected numeric evaluator. */
	~CurveSegment();
	/** Initialize control points, timing values, and polynomial coefficients. */
	void init(const Common::Point32 &start, const Common::Point32 &ctrl0, const Common::Point32 &ctrl1, const Common::Point32 &end, int stepVal, int waitVal);
	/** Derive polynomial coefficients from the four control points. */
	void computeCoeffs();
	/** Evaluate the pos at @p tickCount and report whether the segment remains active. */
	bool evaluate(uint32 tickCount, Common::Point32 &outPos);
	/** Return the start coordinate in screen pixels. */
	Common::Point32 getStartPosition() const;
	/** Return the end coordinate in screen pixels. */
	Common::Point32 getEndPosition() const;
	/** Return the most recently evaluated coordinate in screen pixels. */
	Common::Point32 getPosition() const;
	/** Set the gameplay tick from which parameter advancement is measured. */
	void setStartTime(uint32 tickCount);
	/** Select the fixed- or floating-point template instance while preserving segment progress. */
	void setFloatingPointMode(bool useFloatingPoint);

private:
	class Evaluator;
	struct FixedNumeric;
	struct FloatNumeric;
	template<typename Numeric>
	class TypedEvaluator;

	Evaluator *_evaluator;
	Common::Point32 _start;
	Common::Point32 _control0;
	Common::Point32 _control1;
	Common::Point32 _end;
	int _stepValue;
	int _waitInitial;
	bool _initialized;
	bool _coefficientsReady;
	bool _useFloatingPoint;

	/** Allocate the template instance selected by @p useFloatingPoint. */
	static Evaluator *createEvaluator(bool useFloatingPoint);
	/** Rebuild the evaluator from the integer path descriptor while retaining runtime progress. */
	void rebuildEvaluator();
};

/**
 * Advances through the chained segments loaded from one PAT path.
 *
 * Non-looping paths stop at their final endpoint. Looping paths return to the
 * first segment after the last segment completes.
 */
struct PathObject {
	/** Ordered segments owned by this path. */
	Common::Array<CurveSegment *> segments;
	/** Index of the segment currently being evaluated. */
	int currentSegment;
	/** Whether the path restarts after the last segment. */
	bool looping;
	/** Whether a non-looping path has reached its final endpoint. */
	bool finished;
	/** Final endpoint in screen pixels. */
	Common::Point32 endPos;
	/** Gameplay tick at which the path started. */
	uint32 startTime;

	/** Construct an empty, inactive path. */
	PathObject();
	/** Release every owned segment. */
	~PathObject();

	/** Parse and return a newly allocated path, or nullptr on failure. */
	static PathObject *loadFromPAT(const Common::Path &path);
	/** Append one owned segment using the engine's currently selected numeric representation. */
	void appendSegment(const Common::Point32 &start, const Common::Point32 &ctrl0, const Common::Point32 &ctrl1, const Common::Point32 &end,
					   int stepValue, int waitInitial);
	/** Reset segment state and begin evaluation at @p tickCount. */
	void start(uint32 tickCount);
	/** Advance to @p tickCount and write the current screen pos. */
	bool advance(uint32 tickCount, Common::Point32 &outPos);

private:
	/** Apply the live engine option to every already loaded segment. */
	void synchronizeNumericMode();
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
	static const int kDim0 = 100;
	/** Number of sprite layers per cell. */
	static const int kDim1 = ZmbTrait::kTraitCount + 1;
	/** Number of base-or-feature variants per layer. */
	static const int kDim2 = ZmbTrait::kTraitValueCount + 1;
	/** Total number of independently framed grid entries. */
	static const int kCellCount = kDim0 * kDim1 * kDim2;

	/** Construct an empty sprite grid. */
	ZoombiniAnimation();
	/** Release every frame owned by the sprite grid. */
	~ZoombiniAnimation();

	/** Load every recoverable grid entry from @p path. */
	bool loadFromFile(const Common::Path &path);
	/** Return a borrowed frame, or nullptr when either index is invalid. */
	const RleBlock *getFrame(int cellIndex, int frameIndex) const;
	/** Return the number of frames in @p cellIndex, or zero for an invalid cell. */
	int getFrameCount(int cellIndex) const;
	/** Return the base layer's dimensions for one Zoombini cell and frame. */
	Common::Point getSpriteSize(int cell, int frame) const;
	/** Draw body and trait layers, retaining frame zero for single-frame entries and honoring an optional clip rectangle. */
	void drawZoombini(Graphics::ManagedSurface *screen, const ZmbTrait &traits, const Common::Point32 &pos,
					  int cell, int frame, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip = nullptr) const;

private:
	/** Represents the frames assigned to one sprite-grid entry. */
	struct Cell {
		/** Ordered frames stored in this grid entry. */
		Common::Array<RleBlock *> frames;
		/** Release every owned frame. */
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
	/** Construct an enabled button with an empty rectangle and no images. */
	UIButton();
	/** Release every image owned by the button. */
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
	void setRect(const Common::Point32 &pos, int width, int height);
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
	int drawAndHitTest(Graphics::ManagedSurface *dst, const Common::Point32 &mousePos, const AlphaBlendLUT &alphaLUT);

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
	/** Button pos and hit-test bounds. */
	Common::Rect _rect;
	/** Whether the button responds to pointer input. */
	bool _enabled;
	/** Whether the normal state is drawn while the pointer is outside. */
	bool _drawWhenNotHovered;
	/** Whether masked bitmap fallbacks use the RLE encoder's blend rule. */
	bool _useRleMaskBlend;
	/** Hover state retained from the preceding draw. */
	bool _wasHovering;
	/** Hover state computed during the current draw. */
	bool _isHovering;

	/** Uncompressed normal-state image held by this button. */
	BitBlock *_normalBB;
	/** Uncompressed highlighted-state image held by this button. */
	BitBlock *_hoverBB;
	/** Uncompressed disabled-state image held by this button. */
	BitBlock *_disabledBB;
	/** RLE normal-state image held by this button. */
	RleBlock *_normalRle;
	/** RLE highlighted-state image held by this button. */
	RleBlock *_hoverRle;
	/** RLE disabled-state image held by this button. */
	RleBlock *_disabledRle;
	/** Borrowed overlay drawn after the selected button state. */
	const RleBlock *_overlay;
	/** Overlay offset relative to the button. */
	Common::Point32 _overlayOffset;
	/** Borrowed absolute clipping coordinate for the overlay's right edge. */
	const int *_overlayClipRight;

	/** Release all owned state images. */
	void clearImages();
	/** Load one image, preferring the cached bitmap representation. */
	static bool loadImage(const Common::Path &path, BitBlock *&bitmap, RleBlock *&rle);
	/** Load one masked state from its RLE cache or source bitmap pair. */
	static bool loadMaskedImage(const Common::Path &colorPath, const Common::Path &maskPath, BitBlock *&bitmap, RleBlock *&rle);
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
	static const int kNumGlyphs = 81;
	/** Horizontal advance used for spaces and unsupported characters. */
	static const int kSpaceWidth = 10;

	/** Construct an unloaded font. */
	BitmapFont();
	/** Release every extracted glyph bitmap. */
	~BitmapFont();

	/** Load a BMT color-and-alpha pair and color its extracted glyphs. */
	bool load(const Common::Path &basePath, byte red, byte green, byte blue);
	/** Draw @p text at @p pos and return its horizontal pixel advance. */
	int drawString(Graphics::ManagedSurface *dst, const Common::Point32 &pos, const Common::String &text,
				   const AlphaBlendLUT &alphaLUT) const;
	/** Return the horizontal pixel advance for @p text without drawing it. */
	int getStringWidth(const Common::String &text) const;
	/** Return the glyph index for @p character, or -1 when it is unsupported. */
	static int charToGlyphIndex(char character);
	/** Return whether glyph extraction completed. */
	bool isLoaded() const { return _loaded; }

private:
	/** Whether the font strip has been processed. */
	bool _loaded;
	/** Glyph bitmaps held by this font in character-mapping order. */
	BitBlock *_glyphs[kNumGlyphs];
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
	static const int kSliderMinX = 350;
	/** Rightmost selectable gauge coordinate. */
	static const int kSliderMaxX = 638;
	/** Number of pixels in the selectable gauge interval. */
	static const int kSliderRange = 288;

	/** Shared X coordinate of each slider label button. */
	static const int kLabelX = 157;
	/** Shared slider label width. */
	static const int kLabelW = 520;
	/** Shared slider label height. */
	static const int kLabelH = 64;
	/** Music slider label Y coordinate. */
	static const int kMusicLabelY = 224;
	/** Sound-effect slider label Y coordinate. */
	static const int kSfxLabelY = 286;
	/** Speech slider label Y coordinate. */
	static const int kSpeechLabelY = 351;

	/** Music gauge Y coordinate. */
	static const int kMusicGaugeY = 240;
	/** Sound-effect gauge Y coordinate. */
	static const int kSfxGaugeY = 305;
	/** Speech gauge Y coordinate. */
	static const int kSpeechGaugeY = 374;

	/** Construct a panel with all current and initial volumes set to 100 percent. */
	VolumePanel();
	/** Release the preview sounds and owned gauge image after the buttons stop borrowing it. */
	~VolumePanel();

	/** Load gauge, button, and optional audio-preview resources. */
	bool init(SoundManager *soundManager = nullptr);

	/** Track a slider drag or process one mouse release without painting unrelated page state. */
	VolumePanelResult handleMouseInput(const Common::Point32 &mousePos, bool mouseDown, bool mouseReleased);
	/** Play the category-specific sample for the most recently released slider. */
	void playPreviewSound();
	/** Paint the panel without applying input or changing audio. */
	void draw(Graphics::ManagedSurface *dst, const Common::Point32 &mousePos, const AlphaBlendLUT &alphaLUT);

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
	/** Current music volume percentage. */
	int _musicVolume;
	/** Current sound-effect volume percentage. */
	int _sfxVolume;
	/** Current speech volume percentage. */
	int _speechVolume;
	/** Music volume restored when the caller cancels. */
	int _initialMusicVolume;
	/** Sound-effect volume restored when the caller cancels. */
	int _initialSfxVolume;
	/** Speech volume restored when the caller cancels. */
	int _initialSpeechVolume;

	/** Current music gauge endpoint. */
	int _musicSliderX;
	/** Current sound-effect gauge endpoint. */
	int _sfxSliderX;
	/** Current speech gauge endpoint. */
	int _speechSliderX;
	/** Dragged slider index, or -1 when no slider is captured. */
	int _activeSlider;
	/** Most recent mouse position observed while the primary button was held. */
	Common::Point32 _heldMousePos;
	/** Whether held-mouse coordinates are available for the next release. */
	bool _hasHeldMouse;
	/** Most recently released slider awaiting category-specific audio preview. */
	int _lastAdjustedSlider;
	/** Borrowed sound manager that owns the panel's logical preview records. */
	SoundManager *_soundManager;
	/** Panel-owned speech preview sound identifier. */
	int _speechPreviewSoundId;
	/** Panel-owned sound-effect preview identifier. */
	int _sfxPreviewSoundId;

	/** Gauge sprite held by this panel and shared by the three slider rows. */
	RleBlock *_gaugeImage;
	/** Music, sound-effect, and speech label buttons. */
	UIButton _sliderLabels[3];
	/** Apply button. */
	UIButton _okButton;
	/** Cancel button. */
	UIButton _noButton;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_GRAPHICS_H
