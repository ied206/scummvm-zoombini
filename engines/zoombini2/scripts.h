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

#ifndef ZOOMBINI2_SCRIPTS_H
#define ZOOMBINI2_SCRIPTS_H

#include "common/array.h"
#include "common/noncopyable.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"

#include "zoombini2/state.h"

namespace Graphics {
class ManagedSurface;
}

namespace Zoombini2 {

class AlphaBlendLUT;
class Zoombini2Random;
class Zoombini2Engine;
class ZoombiniAnimation;

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
	constexpr Fixed32() {}

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
	int32 _rawValue = 0;
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
	Common::Point32 _start = Common::Point32();
	Common::Point32 _control0 = Common::Point32();
	Common::Point32 _control1 = Common::Point32();
	Common::Point32 _end = Common::Point32();
	int _stepValue = 0;
	int _waitInitial = 0;
	bool _initialized = false;
	bool _coefficientsReady = false;
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
	int currentSegment = 0;
	/** Whether the path restarts after the last segment. */
	bool looping = false;
	/** Whether a non-looping path has reached its final endpoint. */
	bool finished = false;
	/** Final endpoint in screen pixels. */
	Common::Point32 endPos = Common::Point32();
	/** Gameplay tick at which the path started. */
	uint32 startTime = 0;

	/** Construct an empty, inactive path bound to @p vm. */
	explicit PathObject(Zoombini2Engine *vm);
	/** Release every owned segment. */
	~PathObject();

	/** Parse and return a newly allocated path, or nullptr on failure. */
	static PathObject *loadFromPAT(Zoombini2Engine *vm, const Common::Path &path);
	/** Append one owned segment using the engine's currently selected numeric representation. */
	void appendSegment(const Common::Point32 &start, const Common::Point32 &ctrl0, const Common::Point32 &ctrl1, const Common::Point32 &end,
					   int stepValue, int waitInitial);
	/** Reset segment state and begin evaluation at @p tickCount. */
	void start(uint32 tickCount);
	/** Advance to @p tickCount and write the current screen pos. */
	bool advance(uint32 tickCount, Common::Point32 &outPos);

private:
	/** Borrowed vm used to select the path evaluator and open PAT resources. */
	Zoombini2Engine *_vm;
	/** Apply the live engine option to every already loaded segment. */
	void synchronizeNumericMode();
};

/**
 * Runtime and serialized state for one Zoombini.
 *
 * The four visible traits use the inclusive range 1 through 5.
 * @ref ZoombiniState::_traitHash caches their packed identifier as
 * `feet + 8 * (nose + 8 * (eyes + 8 * hair))`.
 * Save files persist the complete five-byte @ref ZoombiniState::_traits record
 * and the NUL-terminated @ref ZoombiniState::_name buffer. The remaining fields
 * implement the common movement, dragging, animation, and page-placement
 * lifecycle shared by active Zoombinis.
 */
class ZoombiniState {
public:
	/** Callback invoked after a non-looping runtime animation completes. */
	typedef void (*AnimationCompleteCallback)(ZoombiniState *zoombini);

	/** Initialize every field to the inactive runtime baseline. */
	ZoombiniState();
	/** Release the owned movement path. */
	~ZoombiniState();

	/** Select four deterministic trait values from a local seeded generator. */
	void randomize(uint32 seed);
	/** Assign the four visible traits, refresh their hash, and leave the stored unused slot unchanged. */
	void setTraits(const ZmbTrait &traits);
	/** Select the default borrowed sprite grid and its resting cell. */
	void setDefaultAnimation(const ZoombiniAnimation *animation, int cellIndex = 33);
	/** Update the screen position and, when enabled, periodically refresh the directional animation cell. */
	void setPosition(const Common::Point32 &pos);
	/** Replace the owned path and begin evaluating it at @p tickCount. */
	void startMovement(PathObject *path, uint32 tickCount);
	/** Advance the owned path, applying @p spriteOffset to its evaluated coordinates. */
	bool advanceMovement(uint32 tickCount, const Common::Point32 &spriteOffset = Common::Point32(), bool hideAtEnd = false);
	/** Release the owned path without changing the current position. */
	void clearMovement();
	/** Start a page-selected animation on a borrowed grid. */
	void startAnimation(const ZoombiniAnimation *animation, int cellIndex, uint32 tickCount, uint32 frameDelay, bool loop = false,
						AnimationCompleteCallback callback = nullptr);
	/** Start a direction-tracked animation on the current grid. */
	void startDirectionTrackedAnimation(uint32 tickCount, uint32 frameDelay);
	/** Give an eligible resting Zoombini the original one-in-250 chance to start the borrowed idle grid. */
	bool tryStartIdleAnimation(const ZoombiniAnimation *animation, Zoombini2Random &randomSrc, uint32 tickCount, uint32 frameDelay);
	/** Restore the saved grid and original cell 33 idle state after an animation. */
	void resetAnimation();
	/** Advance the active animation by one frame after its current deadline. */
	void updateAnimation(uint32 tickCount);
	/** Return whether @p point lies inside the original fixed Zoombini pickup rectangle. */
	bool hitTest(const Common::Point32 &point) const;
	/** Begin the common pointer-drag lifecycle if input and the pickup rectangle permit it. */
	bool beginDrag(const Common::Point32 &pointerPos, const ZoombiniAnimation *pickupAnimation, uint32 tickCount, uint32 frameDelay);
	/** Move an actively dragged Zoombini with the pointer. */
	void updateDrag(const Common::Point32 &pointerPos);
	/** Finish dragging at @p dropPos, or settle at the current drawn position when it is null. */
	void endDrag(const Common::Point32 *dropPos = nullptr);
	/** Return the current drawing anchor after applying the active drag offset. */
	Common::Point32 getDrawPosition() const;
	/** Draw the active body and trait layers unless this Zoombini is hidden. */
	void draw(Graphics::ManagedSurface *screen, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip = nullptr) const;
	/** Return the sprite rectangle derived from the selected grid's base layer. */
	Common::Rect32 getSpriteRect() const;

	/** Stored one-based trait record, including its serialized unused slot zero. */
	ZmbTrait _traits = ZmbTrait();
	/** Runtime-only packed cache derived from the four visible traits. */
	uint16 _traitHash = 0xFFFF;
	/** NUL-terminated character name retained by boards and profile saves. */
	char _name[kZoombiniNameSize] = {};
	/** Countdown between direction-cell recalculations while movement tracking is enabled. */
	int32 _directionUpdateCooldown = 0;
	/** Current signed 32-bit screen position. */
	Common::Point32 _screenPos = Common::Point32();
	/** Screen position immediately before the most recent movement update. */
	Common::Point32 _previousScreenPos = Common::Point32();
	/** Width and height of the selected base sprite used by redraw bounds. */
	Common::Point _spriteSize = Common::Point();
	/** Current movement path held by this Zoombini until movement ends. */
	PathObject *_movementPath = nullptr;
	/** Whether the current page allows this Zoombini to receive input. */
	bool _inputEnabled = false;
	/** Whether the common input lifecycle currently holds this Zoombini. */
	bool _dragging = false;
	/** Screen position saved when the current drag began. */
	Common::Point32 _dragOrigin = Common::Point32();
	/** Borrowed sprite grid currently used to draw this Zoombini. */
	const ZoombiniAnimation *_activeAnimation = nullptr;
	/** Borrowed sprite grid restored when the current animation ends. */
	const ZoombiniAnimation *_savedAnimation = nullptr;
	/** Slot, route, table, or maze placement index assigned by the active page. */
	int32 _placementIndex = -1;
	/** Whether the common renderer may start an ambient idle animation. */
	bool _idleAnimationEnabled = true;
	/** Gameplay tick at which the next animation frame becomes due. */
	uint32 _nextAnimationFrameTime = 0;
	/** Progress value assigned by the active page; zero and one meanings depend on the active puzzle. */
	byte _puzzleStatus = 0;
	/** Number of rescued Boolies credited when this Zoombini completes Boolie Boggle. */
	int32 _rescuedBooliesPerZoombini = 0;
	/** Whether this Zoombini has completed the active puzzle's exit sequence. */
	bool _exitComplete = false;
	/** Whether the dragged Zoombini currently overlaps an available drop target. */
	bool _overDropTarget = false;
	/** Index of the available drop target under the dragged Zoombini, or `-1`. */
	int32 _hoveredDropTargetIndex = -1;
	/** Whether frame advancement is active. */
	bool _animationActive = false;
	/** Cell selected from the active Zoombini animation grid. */
	int32 _animationCell = 0;
	/** Current frame in the selected animation cell. */
	int32 _animationFrame = 0;
	/** Whether drawing is suppressed while page logic retains this Zoombini. */
	bool _hidden = false;
	/** Pointer-to-sprite offset preserved during the active drag. */
	Common::Point32 _dragOffset = Common::Point32();
	/** Optional completion callback for a non-looping animation. */
	AnimationCompleteCallback _animationCompleteCallback = nullptr;
	/** Whether movement periodically selects a new directional cell. */
	bool _tracksMovementDirection = false;
	/** Vertical correction applied when a non-looping animation completes. */
	int32 _completionVerticalOffset = 0;
	/** Whether animation completion leaves the current position unchanged. */
	bool _preservePositionOnAnimationEnd = false;
	/** Whether frame advancement wraps back to frame one. */
	bool _animationLoops = false;
	/** Milliseconds scheduled between frames of the active animation. */
	uint32 _animationFrameDelay = 0;

private:
	/** Refresh @ref ZoombiniState::_spriteSize from the current grid and cell. */
	void updateSpriteSize();
	/** Disallow copying the owned movement path. */
	ZoombiniState(const ZoombiniState &) = delete;
	/** Disallow assigning the owned movement path. */
	ZoombiniState &operator=(const ZoombiniState &) = delete;
};

} // End of namespace Zoombini2

#endif
