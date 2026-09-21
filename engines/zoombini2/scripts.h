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

#include "zoombini2/graphics.h"
#include "zoombini2/state.h"

namespace Zoombini2 {

class AlphaBlendLUT;
class Animation;
class AreaMask;
class Random;
class RleBlock;
class Zoombini2Engine;
class ZoombiniAnimation;

/** Playback modes used by @ref AnimationRunner. */
enum class AnimationRunnerMode {
	/** Remain inactive until started, then stop and activate the linked runner after one cycle. */
	kPlayOnce00 = 0,
	/** Begin immediately and restart from the first timed entry after every cycle. */
	kLoop01 = 1,
	/** Remain inactive until started, then stop after one cycle. */
	kPlayOnceAndHide02 = 2,
	/** Remain inactive until started, then restart from the first timed entry after every cycle. */
	kLoopAfterStart03 = 3,
	/** Skip drawing, timing, and callbacks. */
	kDisabled04 = 4,
	/** Wait for a randomized gate, play one cycle, and return to the waiting state. */
	kRandomIdle05 = 5
};

/**
 * Mutable script playback state for one page-selected @ref Animation.
 *
 * AN resources contain only decoded image frames. The page script supplies the
 * frame order, cumulative timing, playback mode, optional link, and callbacks.
 * Drawing and cycle completion remain ordered so callbacks run only after the
 * current frame has been materialized.
 */
class AnimationRunner : public Common::NonCopyable {
public:
	/** Fixed internal screen width used when a page has no scrolling background. */
	static constexpr int kDefaultBackgroundWidth = 800;
	/** Page callback with a borrowed context and the runner that dispatched it. */
	typedef void (*Callback)(void *context, AnimationRunner *runner);

	/** Bind an empty runner to the game clock and initialize its mode-specific state. */
	AnimationRunner(Zoombini2Engine *vm, const Common::Point32 &position, AnimationRunnerMode mode);
	/** Release the optional background snapshot. */
	~AnimationRunner();

	/** Select the borrowed immutable frame sequence. */
	void setAnimation(const Animation *animation) { _animation = animation; }
	/** Return the borrowed immutable frame sequence. */
	const Animation *getAnimation() const { return _animation; }
	/** Append one page-authored frame and duration to the cumulative timing table. */
	bool addTimedFrame(int frameIndex, uint32 durationMs);
	/** Remove every timed entry and the optional end-of-cycle redirect. */
	void clearTimedFrames();
	/** Configure a background snapshot restored when completion leaves this runner inactive. */
	void initBackBuffer(const Size32 &size);

	/** Start at the first timed entry without changing the playback mode or position. */
	void start(uint32 tickCount);
	/** Start at the first timed entry and replace the drawing position. */
	void startAt(const Common::Point32 &position, uint32 tickCount);
	/** Restart the current mode at the first timed entry. */
	void reset(uint32 tickCount);
	/** Stop playback and select the play-once mode. */
	void stop();
	/** Replace the playback mode without implicitly starting or stopping the runner. */
	void setMode(AnimationRunnerMode mode) { _mode = mode; }
	/** Return the current playback mode. */
	AnimationRunnerMode getMode() const { return _mode; }
	/** Return whether a timed entry is active. */
	bool isActive() const { return _currentTimedEntryIndex != -1; }

	/** Replace the drawing position. */
	void setPosition(const Common::Point32 &position) { _position = position; }
	/** Return the drawing position before page-layer scrolling. */
	Common::Point32 getPosition() const { return _position; }
	/** Replace the page-coordinate rectangle used by general-object pointer input. */
	void setHitRect(const Common::Rect32 &rect) { _hitRect = rect; }
	/** Return the page-coordinate rectangle used by general-object pointer input. */
	const Common::Rect32 &getHitRect() const { return _hitRect; }
	/** Return whether @p point lies strictly inside the page-coordinate hit rectangle. */
	bool containsHitPoint(const Common::Point32 &point) const;
	/** Select whether hover hit testing considers this runner. */
	void setHitTestEnabled(bool enabled) { _hitTestEnabled = enabled; }
	/** Return whether hover hit testing considers this runner. */
	bool isHitTestEnabled() const { return _hitTestEnabled; }
	/** Select whether the page cursor treats this runner as interactive. */
	void setInputEnabled(bool enabled) { _inputEnabled = enabled; }
	/** Return whether the page cursor treats this runner as interactive. */
	bool isInputEnabled() const { return _inputEnabled; }
	/** Record this runner's stable index within its PageLayer. */
	void setLayerIndex(uint layerIndex) { _layerIndex = layerIndex; }
	/** Return this runner's stable index within its PageLayer. */
	uint getLayerIndex() const { return _layerIndex; }
	/** Activate @p runner when this runner finishes a play-once cycle. */
	void setLinkedRunner(AnimationRunner *runner) { _linkedRunner = runner; }
	/** Redirect a completed cycle to @p timedEntryIndex without applying its mode or callback. */
	bool setNextTimedEntry(int timedEntryIndex);
	/** Remove the completed-cycle redirect. */
	void clearNextTimedEntry() { _nextTimedEntryIndex = -1; }

	/** Install the callback invoked only by @ref invokeInteractionCallback. */
	void setInteractionCallback(Callback callback, void *context = nullptr);
	/** Install the callback invoked at an unredirected cycle boundary. */
	void setCompletionCallback(Callback callback, void *context = nullptr);
	/** Invoke the interaction callback without advancing animation state. */
	void invokeInteractionCallback();

	/** Enter a due random-idle cycle before the runner is drawn. */
	void prepareForDraw(uint32 tickCount);
	/** Draw the current timed entry without advancing or dispatching callbacks. */
	void draw(ManagedSurface32 *screen, const AlphaBlendLUT &alphaLUT, int scrollX = 0, int backgroundWidth = kDefaultBackgroundWidth);
	/** Select the next timed entry or complete the cycle after the current entry has been drawn. */
	void advanceAfterDraw(uint32 tickCount);
	/** Restore the latest configured background snapshot when the runner is inactive. */
	void restoreBackgroundIfInactive(ManagedSurface32 *screen) const;
	/** Capture the configured background rectangle at the current page-layer position. */
	void captureBackground(ManagedSurface32 *screen, int scrollX = 0, int backgroundWidth = kDefaultBackgroundWidth);
	/** Perform the original prepare, draw, and advance sequence in one call. */
	void drawAndUpdate(ManagedSurface32 *screen, const AlphaBlendLUT &alphaLUT, uint32 tickCount, int scrollX = 0,
					   int backgroundWidth = kDefaultBackgroundWidth);

	/** Return the active timing-table index, or -1 while inactive. */
	int getCurrentTimedEntryIndex() const { return _currentTimedEntryIndex; }
	/** Return the image-frame index selected by the active timed entry, or -1. */
	int getCurrentFrameIndex() const;

private:
	/** One page-authored image-frame selection and its cumulative end time. */
	struct TimedFrame {
		int16 frameIndex = -1;
		uint32 cumulativeEndMs = 0;
	};

	/** Maximum number of entries addressable by the original byte-sized count. */
	static constexpr uint kMaxTimedFrameCount = 255;

	Zoombini2Engine *_vm;
	const Animation *_animation = nullptr;
	Common::Point32 _position;
	AnimationRunnerMode _mode;
	Common::Array<TimedFrame> _timedFrames;
	uint32 _totalDurationMs = 0;
	uint32 _cycleStartTime = 0;
	uint32 _randomIdleGateTime = 0;
	int _currentTimedEntryIndex = -1;
	int _nextTimedEntryIndex = -1;
	AnimationRunner *_linkedRunner = nullptr;
	Common::Rect32 _hitRect = Common::Rect32();
	bool _hitTestEnabled = false;
	bool _inputEnabled = true;
	uint _layerIndex = 0;
	Callback _interactionCallback = nullptr;
	void *_interactionCallbackContext = nullptr;
	Callback _completionCallback = nullptr;
	void *_completionCallbackContext = nullptr;
	Size32 _backBufferSize = Size32();
	ManagedSurface32 *_backBuffer = nullptr;
	Common::Rect32 _backBufferScreenRect;
	Common::Point32 _backBufferDrawPosition;
	bool _backBufferValid = false;

	uint32 getRandomDelay(uint32 maximumInclusive) const;
	int getScrolledX(int scrollX, int backgroundWidth) const;
	void saveBackground(ManagedSurface32 *screen, const Common::Point32 &drawPosition);
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

	/** Signed 32-bit raw representation retained by this value. */
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
	/** Replace the raw Q10 step value retained by this segment. */
	void setStepValue(int stepValue);
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
 * first continuation segment after the last segment completes, skipping the
 * FIRST segment that moves an object onto the repeating route.
 */
struct PathObject {
	/** Ordered segments released with this path. */
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
	/** Release every retained segment. */
	~PathObject();

	/** Parse and return a newly allocated path, or nullptr on failure. */
	static PathObject *loadFromPAT(Zoombini2Engine *vm, const Common::Path &path);
	/** Append one segment and retain its final endpoint using the selected numeric representation. */
	void appendSegment(const Common::Point32 &start, const Common::Point32 &ctrl0, const Common::Point32 &ctrl1, const Common::Point32 &end,
					   int stepValue, int waitInitial);
	/** Reset segment state and begin evaluation at @p tickCount. */
	void start(uint32 tickCount);
	/** Replace every segment's raw Q10 step value. */
	void setStepValueForAllSegments(int stepValue);
	/** Advance to @p tickCount and write the current screen pos. */
	bool advance(uint32 tickCount, Common::Point32 &outPos);

private:
	/** Borrowed vm used to select the path evaluator and open PAT resources. */
	Zoombini2Engine *_vm;
	/** Apply the live engine option to every already loaded segment. */
	void synchronizeNumericMode();
};

/** One page-managed target accepted by the common Zoombini input lifecycle. */
struct ZoombiniDropTarget {
	/** Callback invoked after the target releases or receives a Zoombini. */
	typedef void (*Callback)(void *context, int targetIndex, int zoombiniIndex);

	/** Strict target rectangle tested against the held Zoombini's foot point. */
	Common::Rect32 rect = Common::Rect32();
	/** Whether this target currently retains a Zoombini index. */
	bool occupied = false;
	/** Optional page callback for target membership changes. */
	Callback callback = nullptr;
	/** Borrowed page context passed to @ref callback. */
	void *callbackContext = nullptr;
	/** Index assigned on the latest occupied transition; ignored while free. */
	int zoombiniIndex = -1;
};

/** Observable result of one common Zoombini pointer-input pass. */
enum class ZoombiniInputResult {
	/** No held or eligible Zoombini consumed the event. */
	kIgnored00 = 0,
	/** A Zoombini entered the click-to-carry state. */
	kPickedUp01 = 1,
	/** A held Zoombini was assigned to a page-managed target. */
	kDroppedOnTarget02 = 2,
	/** A held Zoombini was settled on an accepted area-mask byte. */
	kDroppedOnArea03 = 3,
	/** A held Zoombini moved or rejected a drop and remains carried. */
	kStillHeld04 = 4
};

/**
 * Runtime runner for one active Zoombini.
 *
 * The four visible traits use the inclusive range 1 through 5.
 * @ref ZoombiniRunner::_traitHash caches their packed identifier as
 * `feet + 8 * (nose + 8 * (eyes + 8 * hair))`.
 * Save files persist the complete five-byte @ref ZoombiniRunner::_traits record
 * and the NUL-terminated @ref ZoombiniRunner::_name buffer. The remaining fields
 * implement the common movement, dragging, animation, and page-placement
 * lifecycle shared by active Zoombinis.
 */
class ZoombiniRunner {
public:
	/** Completion branches selected by the active sprite-grid role. */
	enum class AnimationCompletionPolicy {
		/** Apply the ordinary callback, stop, or callback-free repeat branch at frame 11. */
		kOrdinary00 = 0,
		/** Restore idle state without dispatching a retained callback or applying a position correction. */
		kBypassCallbackAndCorrection01 = 1
	};

	/** Callback invoked after an ordinary runtime animation completes. */
	typedef void (*AnimationCompleteCallback)(void *context, ZoombiniRunner *zoombini);
	/** Frame value at which Zoombini animation selects its grid-role completion branch. */
	static constexpr int kAnimationCompletionFrame = 11;
	/** Background-width sentinel that suppresses scrolling for the Zoombini renderer. */
	static constexpr int kUnscrolledBackgroundWidth = -144;

	/** Initialize every field to the inactive runtime baseline. */
	ZoombiniRunner();
	/** Release the retained movement path. */
	~ZoombiniRunner();

	/** Assign the four visible traits, refresh their hash, and leave the stored unused slot unchanged. */
	void setTraits(const ZmbTrait &traits);
	/** Select the default borrowed sprite grid and its resting cell. */
	void setDefaultAnimation(const ZoombiniAnimation *animation, int cellIndex = 33);
	/** Replace only the active borrowed sprite grid while retaining the grid restored at reset. */
	void setActiveAnimation(const ZoombiniAnimation *animation);
	/** Update the screen position and, when enabled, periodically refresh the directional animation cell. */
	void setPosition(const Common::Point32 &pos);
	/** Replace the retained path and begin evaluating it at @p tickCount. */
	void startMovement(PathObject *path, uint32 tickCount);
	/** Advance the retained path and apply @p spriteOffset without releasing a completed path. */
	bool advanceMovement(uint32 tickCount, const Common::Point32 &spriteOffset = Common::Point32());
	/** Release the retained path without changing the current position. */
	void clearMovement();
	/** Start a page-selected animation using the delay retained beside its borrowed grid. */
	void startAnimation(const ZoombiniAnimation *animation, int cellIndex, uint32 tickCount,
						AnimationCompletionPolicy completionPolicy = AnimationCompletionPolicy::kOrdinary00);
	/** Replace the separately retained one-shot animation completion callback. */
	void setAnimationCompleteCallback(AnimationCompleteCallback callback, void *context = nullptr);
	/** Stop and restore a callback-free ordinary animation at frame 11 instead of repeating it. */
	void stopAnimationOnCompletion() { _stopAnimationOnCompletion = true; }
	/** Start a direction-tracked animation on the current grid. */
	void startDirectionTrackedAnimation(uint32 tickCount);
	/** Give an eligible resting Zoombini the original one-in-250 chance to start the borrowed idle grid. */
	bool tryStartIdleAnimation(const ZoombiniAnimation *animation, Random &randomSrc, uint32 tickCount);
	/** Restore the saved grid and cell 33 after an animation. */
	void resetAnimation();
	/** Schedule a due frame advance while preserving the frame drawn by the current render pass. */
	void updateAnimation(uint32 tickCount);
	/** Apply a scheduled frame transition immediately after the current frame has been drawn. */
	void advanceAnimationAfterDraw();
	/** Return whether @p point lies inside the original fixed Zoombini pickup rectangle. */
	bool hitTest(const Common::Point32 &point) const;
	/** Begin the common pointer-drag lifecycle if input and the pickup rectangle permit it. */
	bool beginDrag(const Common::Point32 &pointerPos, const ZoombiniAnimation *pickupAnimation, uint32 tickCount);
	/** Move an actively dragged Zoombini with the pointer. */
	void updateDrag(const Common::Point32 &pointerPos);
	/** Finish dragging and optionally retain the original grab point at the release position. */
	void endDrag(bool applyGrabOffset);
	/**
	 * Apply the original release-driven click-to-carry lifecycle to one page roster.
	 *
	 * Pointer movement updates an existing carry even when @p clickReleased is false.
	 * A release picks an eligible Zoombini up or attempts to drop the one already
	 * held onto the first overlapping free release target, then onto the area mask.
	 */
	static ZoombiniInputResult handlePointerInput(const Common::Array<ZoombiniRunner *> &zoombinis, const Common::Point32 &pointerPos,
												  bool clickReleased, const ZoombiniAnimation *pickupAnimation, uint32 tickCount,
												  Common::Array<ZoombiniDropTarget> *dropTargets = nullptr, const AreaMask *areaMask = nullptr,
												  int scrollX = 0, int backgroundWidth = AnimationRunner::kDefaultBackgroundWidth);
	/** Stably sort selected roster indices by their logical screen Y position. */
	static void sortDrawOrderByY(const Common::Array<ZoombiniRunner *> &zoombinis, Common::Array<uint> &order);
	/** Return the current drawing anchor after applying page scrolling or the active grab offset. */
	Common::Point32 getDrawPosition(int scrollX = 0, int backgroundWidth = AnimationRunner::kDefaultBackgroundWidth) const;
	/** Forward legacy drawing calls to @ref Gfx::drawZoombiniRunner without advancing animation. */
	void draw(ManagedSurface32 *screen, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip = nullptr,
			  int scrollX = 0, int backgroundWidth = AnimationRunner::kDefaultBackgroundWidth, const RleBlock *dropTargetIndicator = nullptr) const;
	/** Return the sprite rectangle derived from the selected grid's base layer. */
	Common::Rect32 getSpriteRect(int scrollX = 0, int backgroundWidth = AnimationRunner::kDefaultBackgroundWidth) const;

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
	/** 16-bit width and height of the selected base sprite used by redraw bounds. */
	Size16 _spriteSize = Size16();
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
	/** Whether drawing selects the animated frame instead of frame zero. */
	bool _animationActive = false;
	/** Cell selected from the active Zoombini animation grid. */
	int32 _animationCell = 0;
	/** Current frame in the selected animation cell. */
	int32 _animationFrame = 0;
	/** Whether drawing is suppressed while page logic retains this Zoombini. */
	bool _hidden = false;
	/** Pointer-to-sprite offset preserved during the active drag. */
	Common::Point32 _dragOffset = Common::Point32();
	/** Optional one-shot callback retained independently from animation starts and resets. */
	AnimationCompleteCallback _animationCompleteCallback = nullptr;
	/** Borrowed context passed to @ref _animationCompleteCallback. */
	void *_animationCompleteCallbackContext = nullptr;
	/** Whether movement periodically selects a new directional cell. */
	bool _tracksMovementDirection = false;
	/** Vertical correction applied once when a callback-free ordinary animation first repeats. */
	int32 _completionVerticalOffset = 0;
	/** Whether callback-free ordinary completion stops without applying its correction. */
	bool _stopAnimationOnCompletion = false;
	/** Whether the next frame transition waits for the current frame to be drawn. */
	bool _animationAdvancePending = false;
	/** Whether the active grid bypasses the ordinary callback and position-correction branch. */
	bool _bypassAnimationCompletionWork = false;

private:
	/** Return whether @p point lies strictly inside @p rect. */
	static bool isStrictlyInside(const Common::Point32 &point, const Common::Rect32 &rect);
	/** Return whether common input may pick this Zoombini up at @p pointerPos. */
	bool canBeginDrag(const Common::Point32 &pointerPos) const;
	/** Enter the held state after page target-removal callbacks have run. */
	void startDrag(const Common::Point32 &pointerPos, const ZoombiniAnimation *pickupAnimation, uint32 tickCount);
	/** Return the last free target containing the unscrolled held foot point, or -1. */
	static int findHoveredDropTarget(const ZoombiniRunner &zoombini, const Common::Point32 &pointerPos,
									 const Common::Array<ZoombiniDropTarget> &targets);
	/** Return the first free target containing the release foot point after page scrolling is applied. */
	static int findReleaseDropTarget(const ZoombiniRunner &zoombini, const Common::Point32 &pointerPos,
									 const Common::Array<ZoombiniDropTarget> &targets, int scrollX, int backgroundWidth);
	/** Refresh @ref ZoombiniRunner::_spriteSize from the current grid and cell. */
	void updateSpriteSize();
	/** Apply one scheduled transition and the frame-11 grid-role branch. */
	void commitAnimationAdvance();
	/** Disallow copying the retained movement path. */
	ZoombiniRunner(const ZoombiniRunner &) = delete;
	/** Disallow assigning the retained movement path. */
	ZoombiniRunner &operator=(const ZoombiniRunner &) = delete;
};

} // End of namespace Zoombini2

#endif
