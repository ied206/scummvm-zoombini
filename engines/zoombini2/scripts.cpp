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

#include "common/ptr.h"
#include "common/textconsole.h"
#include "common/tokenizer.h"
#include "common/util.h"

#include "zoombini2/graphics.h"
#include "zoombini2/random.h"
#include "zoombini2/scripts.h"
#include "zoombini2/zoombini2.h"

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

namespace Zoombini2 {

const StringParser::ParseConf StringParser::kDecimalProfile = {false};
const StringParser::ParseConf StringParser::kDecimalOrHexProfile = {true};

StringParser::Result StringParser::parseSignedInt(const char *text, int32 &result, const ParseConf &conf) {
	if (!text || text[0] == '\0')
		return Result::kEmptyInput;

	const char *digits = text;
	while (Common::isSpace(digits[0]))
		digits += 1;

	int base = 10;
	if (conf.allowHexadecimal && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X'))
		base = 16;

	char *endPtr = nullptr;
	errno = 0;
	const long parsed = strtol(text, &endPtr, base);

	if (errno == ERANGE)
		return Result::kOutOfRange;
	if (errno != 0)
		return Result::kConversionFailure;
	if (endPtr == text)
		return Result::kInvalidInput;
	if (*endPtr != '\0')
		return Result::kTrailingCharacters;
	if (parsed < INT32_MIN || INT32_MAX < parsed)
		return Result::kOutOfRange;

	result = static_cast<int32>(parsed);
	return Result::kSuccess;
}

StringParser::Result StringParser::parseUnsignedInt(const char *text, uint32 &result, const ParseConf &conf) {
	if (!text || text[0] == '\0')
		return Result::kEmptyInput;

	const char *digits = text;
	while (Common::isSpace(digits[0]))
		digits += 1;
	if (digits[0] == '-')
		return Result::kInvalidInput;

	int base = 10;
	if (conf.allowHexadecimal && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X'))
		base = 16;

	char *endPtr = nullptr;
	errno = 0;
	const unsigned long parsed = strtoul(text, &endPtr, base);

	if (errno == ERANGE)
		return Result::kOutOfRange;
	if (errno != 0)
		return Result::kConversionFailure;
	if (endPtr == text)
		return Result::kInvalidInput;
	if (*endPtr != '\0')
		return Result::kTrailingCharacters;
	if (UINT32_MAX < parsed)
		return Result::kOutOfRange;

	result = static_cast<uint32>(parsed);
	return Result::kSuccess;
}

// ============================================================================
// AnimationRunner
// ============================================================================

AnimationRunner::AnimationRunner(Zoombini2Engine *vm, const Common::Point32 &position, AnimationRunnerMode mode)
	: _vm(vm), _position(position), _mode(mode) {
	const uint32 tickCount = _vm ? _vm->getFrameTickCount() : 0;
	switch (_mode) {
	case AnimationRunnerMode::kLoop01:
		_currentTimedEntryIndex = 0;
		_cycleStartTime = tickCount;
		break;
	case AnimationRunnerMode::kRandomIdle05:
		_randomIdleGateTime = tickCount + getRandomDelay(6999);
		break;
	default:
		break;
	}
}

AnimationRunner::~AnimationRunner() {
	delete _backBuffer;
}

bool AnimationRunner::addTimedFrame(int frameIndex, uint32 durationMs) {
	if (kMaxTimedFrameCount <= _timedFrames.size()) {
		warning("AnimationRunner: cannot append more than %u timed frames", kMaxTimedFrameCount);
		return false;
	}
	if (frameIndex < -1 || 32767 < frameIndex) {
		warning("AnimationRunner: frame index %d is outside the signed 16-bit timing format", frameIndex);
		return false;
	}

	_totalDurationMs += durationMs;
	TimedFrame timedFrame;
	timedFrame.frameIndex = static_cast<int16>(frameIndex);
	timedFrame.cumulativeEndMs = _totalDurationMs;
	_timedFrames.push_back(timedFrame);
	return true;
}

void AnimationRunner::clearTimedFrames() {
	_timedFrames.clear();
	_totalDurationMs = 0;
	_nextTimedEntryIndex = -1;
}

void AnimationRunner::initBackBuffer(const Size32 &size) {
	delete _backBuffer;
	_backBuffer = nullptr;
	_backBufferSize.width = MAX(0, size.width);
	_backBufferSize.height = MAX(0, size.height);
	_backBufferValid = false;
}

void AnimationRunner::start(uint32 tickCount) {
	_currentTimedEntryIndex = 0;
	_cycleStartTime = tickCount;
}

void AnimationRunner::startAt(const Common::Point32 &position, uint32 tickCount) {
	_position = position;
	start(tickCount);
}

void AnimationRunner::reset(uint32 tickCount) {
	start(tickCount);
}

void AnimationRunner::stop() {
	_currentTimedEntryIndex = -1;
	_mode = AnimationRunnerMode::kPlayOnce00;
}

bool AnimationRunner::setNextTimedEntry(int timedEntryIndex) {
	if (timedEntryIndex < 0 || static_cast<int>(_timedFrames.size()) <= timedEntryIndex) {
		warning("AnimationRunner: timed-entry redirect %d is outside the table", timedEntryIndex);
		return false;
	}
	_nextTimedEntryIndex = timedEntryIndex;
	return true;
}

void AnimationRunner::setInteractionCallback(Callback callback, void *context) {
	_interactionCallback = callback;
	_interactionCallbackContext = context;
}

void AnimationRunner::setCompletionCallback(Callback callback, void *context) {
	_completionCallback = callback;
	_completionCallbackContext = context;
}

void AnimationRunner::invokeInteractionCallback() {
	if (_interactionCallback)
		_interactionCallback(_interactionCallbackContext, this);
}

void AnimationRunner::prepareForDraw(uint32 tickCount) {
	if (_mode != AnimationRunnerMode::kRandomIdle05 || isActive() || !(_randomIdleGateTime < tickCount))
		return;

	_currentTimedEntryIndex = 0;
	_cycleStartTime = tickCount;
	_randomIdleGateTime = tickCount + getRandomDelay(5999) + 2000;
}

void AnimationRunner::draw(ManagedSurface32 *screen, const AlphaBlendLUT &alphaLUT, int scrollX, int backgroundWidth) {
	if (!screen || _mode == AnimationRunnerMode::kDisabled04)
		return;
	const int frameIndex = getCurrentFrameIndex();
	if (!_animation || frameIndex == -1)
		return;
	const RleBlock *frame = _animation->getFrame(frameIndex);
	if (!frame) {
		_backBufferValid = false;
		return;
	}
	const Common::Point32 drawPosition(getScrolledX(scrollX, backgroundWidth), _position.y);
	saveBackground(screen, drawPosition);
	frame->drawToScreen(screen, drawPosition, alphaLUT);
}

void AnimationRunner::advanceAfterDraw(uint32 tickCount) {
	if (_mode == AnimationRunnerMode::kDisabled04 || !isActive())
		return;

	bool selectedTimedEntry = false;
	const uint32 elapsed = tickCount - _cycleStartTime;
	for (int i = static_cast<int>(_timedFrames.size()) - 1; 0 <= i; i -= 1) {
		if (elapsed < _timedFrames[i].cumulativeEndMs) {
			_currentTimedEntryIndex = i;
			selectedTimedEntry = true;
		}
	}
	if (selectedTimedEntry)
		return;

	if (_nextTimedEntryIndex != -1) {
		_currentTimedEntryIndex = _nextTimedEntryIndex;
		return;
	}

	switch (_mode) {
	case AnimationRunnerMode::kPlayOnce00:
		_currentTimedEntryIndex = -1;
		if (_linkedRunner)
			_linkedRunner->_currentTimedEntryIndex = 0;
		break;
	case AnimationRunnerMode::kLoop01:
	case AnimationRunnerMode::kLoopAfterStart03:
		_currentTimedEntryIndex = 0;
		_cycleStartTime = tickCount;
		break;
	case AnimationRunnerMode::kPlayOnceAndHide02:
	case AnimationRunnerMode::kRandomIdle05:
		_currentTimedEntryIndex = -1;
		break;
	case AnimationRunnerMode::kDisabled04:
		return;
	}

	if (_completionCallback)
		_completionCallback(_completionCallbackContext, this);
}

void AnimationRunner::restoreBackgroundIfInactive(ManagedSurface32 *screen) const {
	if (!screen || isActive() || !_backBuffer || !_backBufferValid)
		return;
	const int srcX = _backBufferScreenRect.left - _backBufferDrawPosition.x;
	const int srcY = _backBufferScreenRect.top - _backBufferDrawPosition.y;
	const Common::Rect srcRect(srcX, srcY, srcX + _backBufferScreenRect.width(), srcY + _backBufferScreenRect.height());
	screen->copyRectToSurface(*_backBuffer, _backBufferScreenRect.left, _backBufferScreenRect.top, srcRect);
}

void AnimationRunner::captureBackground(ManagedSurface32 *screen, int scrollX, int backgroundWidth) {
	if (!screen)
		return;
	saveBackground(screen, Common::Point32(getScrolledX(scrollX, backgroundWidth), _position.y));
}

void AnimationRunner::drawAndUpdate(ManagedSurface32 *screen, const AlphaBlendLUT &alphaLUT, uint32 tickCount, int scrollX, int backgroundWidth) {
	prepareForDraw(tickCount);
	draw(screen, alphaLUT, scrollX, backgroundWidth);
	advanceAfterDraw(tickCount);
	restoreBackgroundIfInactive(screen);
}

int AnimationRunner::getCurrentFrameIndex() const {
	if (_currentTimedEntryIndex < 0 || static_cast<int>(_timedFrames.size()) <= _currentTimedEntryIndex)
		return -1;
	return _timedFrames[_currentTimedEntryIndex].frameIndex;
}

bool AnimationRunner::containsHitPoint(const Common::Point32 &point) const {
	return _hitRect.left < point.x && point.x < _hitRect.right && _hitRect.top < point.y && point.y < _hitRect.bottom;
}

uint32 AnimationRunner::getRandomDelay(uint32 maximumInclusive) const {
	if (!_vm || !_vm->_rnd)
		return 0;
	return static_cast<uint32>(_vm->_rnd->getRandomNumber(static_cast<int32>(maximumInclusive)));
}

int AnimationRunner::getScrolledX(int scrollX, int backgroundWidth) const {
	if (backgroundWidth == Gfx::kScreenSize.width)
		return _position.x;
	int screenX = _position.x - scrollX;
	if (backgroundWidth - Gfx::kScreenSize.width < scrollX)
		screenX += backgroundWidth;
	return screenX;
}

void AnimationRunner::saveBackground(ManagedSurface32 *screen, const Common::Point32 &drawPosition) {
	_backBufferValid = false;
	if (_backBufferSize.width <= 0 || _backBufferSize.height <= 0)
		return;
	if (!_backBuffer)
		_backBuffer = _vm->_gfx->createSurface(_backBufferSize);

	_backBufferDrawPosition = drawPosition;
	_backBufferScreenRect = Common::Rect32(drawPosition.x, drawPosition.y, drawPosition.x + _backBufferSize.width, drawPosition.y + _backBufferSize.height);
	_backBufferScreenRect.clip(screen->w, screen->h);
	if (_backBufferScreenRect.isEmpty())
		return;
	const int destX = _backBufferScreenRect.left - drawPosition.x;
	const int destY = _backBufferScreenRect.top - drawPosition.y;
	_backBuffer->copyRectToSurface(*screen, destX, destY, _backBufferScreenRect);
	_backBufferValid = true;
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

bool Fixed32::operator<(const Fixed32 &right) const {
	return _rawValue < right._rawValue;
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
	const int64 difference = static_cast<int64>(left.getRawValue()) - static_cast<int64>(right.getRawValue());
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
// CubicBezierCurve - runtime-selected cubic Bezier numeric representation.
// ============================================================================

class CubicBezierCurve::Evaluator {
public:
	/** Release an evaluator through its numeric-independent interface. */
	virtual ~Evaluator() {}
	/** Return an independently allocated copy of the evaluator and its playback state. */
	virtual Evaluator *clone() const = 0;
	/** Assign pixel control points, a raw Q10 step and an endpoint evaluation wait count. */
	virtual void init(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1, const Common::Point32 &end,
					  int stepValue, int waitInitial) = 0;
	/** Derive the cubic coefficients and reset parameter and endpoint wait progress. */
	virtual void computeCoeffs() = 0;
	/** Write the point for the gameplay timestamp in milliseconds; return false after traversal and endpoint waits. */
	virtual bool evaluate(uint32 tickCount, Common::Point32 &outPosition) = 0;
	/** Return the latest evaluated point as integer pixel coordinates. */
	virtual Common::Point32 getPosition() const = 0;
	/** Set the gameplay timestamp in milliseconds from which curve progress is measured. */
	virtual void setStartTime(uint32 tickCount) = 0;
	/** Return the gameplay timestamp in milliseconds from which curve progress is measured. */
	virtual uint32 getStartTime() const = 0;
	/** Set the number of endpoint evaluations remaining after traversal. */
	virtual void setWaitRemaining(int waitRemaining) = 0;
	/** Return the number of endpoint evaluations remaining after traversal. */
	virtual int getWaitRemaining() const = 0;
	/** Set the dimensionless curve parameter from a native float. */
	virtual void setParameter(Float32 parameter) = 0;
	/** Return the dimensionless curve parameter as a native float. */
	virtual Float32 getParameter() const = 0;
};

struct CubicBezierCurve::FixedNumeric {
	typedef Fixed32 Scalar;
	typedef PointFixed32 Point;

	/** Convert an integer coordinate to a Q10 scalar. */
	static Scalar fromInt(int32 value) { return Fixed32::fromInt(value); }
	/** Interpret an integer as a raw Q10 scalar. */
	static Scalar fromRaw(int32 value) { return Fixed32::fromRaw(value); }
	/** Convert a native float to a Q10 scalar. */
	static Scalar fromFloat(Float32 value) { return Fixed32::fromFloat(value); }
	/** Return the integer component of a Q10 scalar. */
	static int32 toInt(const Scalar &value) { return value.toInt(); }
	/** Convert a Q10 scalar to a native float. */
	static Float32 toFloat(const Scalar &value) { return value.toFloat(); }
	/** Return whether the left scalar is strictly greater than the right scalar. */
	static bool greater(const Scalar &left, const Scalar &right) { return right < left; }
};

struct CubicBezierCurve::FloatNumeric {
	typedef Float32 Scalar;

	struct Point {
		/** Horizontal curve coordinate in floating-point pixel units. */
		Scalar x = 0.0f;
		/** Vertical curve coordinate in floating-point pixel units. */
		Scalar y = 0.0f;

		/** Construct a floating-point point at the origin. */
		Point() = default;
		/** Construct a point from floating-point pixel coordinates. */
		Point(Scalar xValue, Scalar yValue)
			: x(xValue), y(yValue) {}

		/** Return the component-wise sum of two points. */
		Point operator+(const Point &right) const { return Point(x + right.x, y + right.y); }
		/** Return the component-wise difference of two points. */
		Point operator-(const Point &right) const { return Point(x - right.x, y - right.y); }
		/** Scale both point coordinates by the supplied scalar. */
		Point operator*(Scalar right) const { return Point(x * right, y * right); }
	};

	/** Convert an integer coordinate to a floating-point scalar. */
	static Scalar fromInt(int32 value) { return static_cast<Scalar>(value); }
	/** Convert raw Q10 units to a floating-point scalar. */
	static Scalar fromRaw(int32 value) { return static_cast<Scalar>(value) / 1024.0f; }
	/** Retain a native float without conversion. */
	static Scalar fromFloat(Float32 value) { return value; }
	/** Round a floating-point coordinate down to an integer pixel coordinate. */
	static int32 toInt(Scalar value) {
		int32 result = static_cast<int32>(value);
		if (value < static_cast<Scalar>(result))
			result -= 1;
		return result;
	}
	/** Return the native floating-point scalar. */
	static Float32 toFloat(Scalar value) { return value; }
	/** Return whether the left scalar is strictly greater than the right scalar. */
	static bool greater(Scalar left, Scalar right) { return right < left; }
};

template<typename Numeric>
class CubicBezierCurve::TypedEvaluator : public CubicBezierCurve::Evaluator {
public:
	typedef typename Numeric::Scalar Scalar;
	typedef typename Numeric::Point Point;

	/** Construct an evaluator with zeroed control points, coefficients and playback state. */
	TypedEvaluator() = default;

	/** Return an independently allocated copy of the evaluator and its playback state. */
	Evaluator *clone() const override {
		return new TypedEvaluator<Numeric>(*this);
	}

	/** Assign pixel control points, a raw Q10 step and an endpoint evaluation wait count. */
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

	/** Derive the cubic coefficients and reset parameter and endpoint wait progress. */
	void computeCoeffs() override {
		_parameter = Scalar();
		const Scalar three = Numeric::fromInt(3);
		_linear = (_control0 - _start) * three;
		_quadratic = (_control1 - _control0) * three - _linear;
		_cubic = _end - _start - _linear - _quadratic;
		_waitRemaining = _waitInitial;
	}

	/** Write the point for the gameplay timestamp in milliseconds; return false after traversal and endpoint waits. */
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
			outPosition = getPosition();
			return true;
		}

		return false;
	}

	/** Return the latest evaluated point as integer pixel coordinates. */
	Common::Point32 getPosition() const override {
		return Common::Point32(Numeric::toInt(_position.x), Numeric::toInt(_position.y));
	}

	/** Set the gameplay timestamp in milliseconds from which curve progress is measured. */
	void setStartTime(uint32 tickCount) override { _startTime = tickCount; }
	/** Return the gameplay timestamp in milliseconds from which curve progress is measured. */
	uint32 getStartTime() const override { return _startTime; }
	/** Set the number of endpoint evaluations remaining after traversal. */
	void setWaitRemaining(int waitRemaining) override { _waitRemaining = waitRemaining; }
	/** Return the number of endpoint evaluations remaining after traversal. */
	int getWaitRemaining() const override { return _waitRemaining; }
	/** Set the dimensionless curve parameter from a native float. */
	void setParameter(Float32 parameter) override {
		_parameter = Numeric::fromFloat(parameter);
		calculatePosition();
	}
	/** Return the dimensionless curve parameter as a native float. */
	Float32 getParameter() const override { return Numeric::toFloat(_parameter); }

private:
	/** Start point in the selected numeric representation. */
	Point _start = Point();
	/** First control point in the selected numeric representation. */
	Point _control0 = Point();
	/** Second control point in the selected numeric representation. */
	Point _control1 = Point();
	/** End point in the selected numeric representation. */
	Point _end = Point();
	/** Coefficient of the cubic parameter term for both coordinates. */
	Point _cubic = Point();
	/** Coefficient of the quadratic parameter term for both coordinates. */
	Point _quadratic = Point();
	/** Coefficient of the linear parameter term for both coordinates. */
	Point _linear = Point();
	/** Current dimensionless curve parameter in the selected numeric representation. */
	Scalar _parameter = Scalar();
	/** Most recently evaluated point in the selected numeric representation. */
	Point _position = Point();
	/** Parameter increment per five-millisecond elapsed-time step. */
	Scalar _step = Scalar();
	/** Configured number of extra endpoint evaluations after traversal. */
	int _waitInitial = 0;
	/** Number of extra endpoint evaluations still to perform. */
	int _waitRemaining = 0;
	/** Gameplay timestamp in milliseconds from which curve progress is measured. */
	uint32 _startTime = 0;

	/** Evaluate the retained cubic polynomial at the current parameter. */
	void calculatePosition() {
		const Scalar squaredParameter = _parameter * _parameter;
		const Scalar cubedParameter = squaredParameter * _parameter;
		_position = _start + _linear * _parameter + _quadratic * squaredParameter + _cubic * cubedParameter;
	}
};

CubicBezierCurve::CubicBezierCurve(BezierCurveDataType dataType)
	: _evaluator(createEvaluator(dataType)), _dataType(dataType) {
}

CubicBezierCurve::CubicBezierCurve(const CubicBezierCurve &other)
	: _evaluator(other._evaluator->clone()), _start(other._start), _control0(other._control0), _control1(other._control1), _end(other._end),
	  _stepValue(other._stepValue), _waitInitial(other._waitInitial), _initialized(other._initialized),
	  _coefficientsReady(other._coefficientsReady), _dataType(other._dataType) {
}

CubicBezierCurve &CubicBezierCurve::operator=(const CubicBezierCurve &other) {
	if (this == &other)
		return *this;

	Evaluator *evaluator = other._evaluator->clone();
	delete _evaluator;
	_evaluator = evaluator;
	_start = other._start;
	_control0 = other._control0;
	_control1 = other._control1;
	_end = other._end;
	_stepValue = other._stepValue;
	_waitInitial = other._waitInitial;
	_initialized = other._initialized;
	_coefficientsReady = other._coefficientsReady;
	_dataType = other._dataType;
	return *this;
}

CubicBezierCurve::~CubicBezierCurve() {
	delete _evaluator;
}

void CubicBezierCurve::init(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1, const Common::Point32 &end, int stepValue, int waitInitial) {
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

void CubicBezierCurve::computeCoeffs() {
	assert(_initialized);
	_evaluator->computeCoeffs();
	_coefficientsReady = true;
}

bool CubicBezierCurve::evaluate(uint32 tickCount, Common::Point32 &outPosition) {
	assert(_coefficientsReady);
	return _evaluator->evaluate(tickCount, outPosition);
}

Common::Point32 CubicBezierCurve::getStartPosition() const {
	return _start;
}

Common::Point32 CubicBezierCurve::getEndPosition() const {
	return _end;
}

Common::Point32 CubicBezierCurve::getPosition() const {
	return _initialized ? _evaluator->getPosition() : Common::Point32();
}

void CubicBezierCurve::setStartTime(uint32 tickCount) {
	_evaluator->setStartTime(tickCount);
}

void CubicBezierCurve::setStepValue(int stepValue) {
	if (_stepValue == stepValue)
		return;
	_stepValue = stepValue;
	rebuildEvaluator();
}

void CubicBezierCurve::setDataType(BezierCurveDataType dataType) {
	if (_dataType == dataType)
		return;
	_dataType = dataType;
	rebuildEvaluator();
}

CubicBezierCurve::Evaluator *CubicBezierCurve::createEvaluator(BezierCurveDataType dataType) {
	switch (dataType) {
	case BezierCurveDataType::kF32FloatingPoint:
		return new TypedEvaluator<FloatNumeric>();
	case BezierCurveDataType::kQ10FixedPoint:
	default:
		return new TypedEvaluator<FixedNumeric>();
	}
}

void CubicBezierCurve::rebuildEvaluator() {
	const uint32 previousStartTime = _evaluator->getStartTime();
	const int previousWaitRemaining = _evaluator->getWaitRemaining();
	const Float32 previousParameter = _evaluator->getParameter();

	delete _evaluator;
	_evaluator = createEvaluator(_dataType);
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
// PathObject - Bezier path composed of chained CubicBezierCurves.
// ============================================================================

PathObject::PathObject(Zoombini2Engine *vm)
	: _vm(vm) {
}

PathObject::~PathObject() {
	for (uint i = 0; i < segments.size(); i++)
		delete segments[i];
}

PathObject *PathObject::createUnstartedCopy() const {
	PathObject *path = new PathObject(_vm);
	const BezierCurveDataType dataType = _vm->getBezierCurveDataType();

	for (uint i = 0; i < segments.size(); i++) {
		CubicBezierCurve *segment = new CubicBezierCurve(*segments[i]);

		segment->setDataType(dataType);
		path->segments.push_back(segment);
	}
	path->looping = looping;
	path->endPos = endPos;
	return path;
}

void PathObject::appendSegment(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1, const Common::Point32 &end, int stepValue, int waitInitial) {
	CubicBezierCurve *segment = new CubicBezierCurve(_vm->getBezierCurveDataType());
	segment->init(start, control0, control1, end, stepValue, waitInitial);
	segments.push_back(segment);
	endPos = end;
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
PathObject *PathObject::loadFromPAT(Zoombini2Engine *vm, const Common::Path &path) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(vm->openResourceFile(path.toString('/')));
	if (!stream) {
		warning("PathObject: cannot open PAT '%s'", path.toString().c_str());
		return nullptr;
	}
	return loadPATStream(vm, *stream);
}

bool PathObject::parsePATIntegerToken(const Common::String &token, int &value) {
	int32 parsed = 0;
	if (StringParser::parseSignedInt(token.c_str(), parsed, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
		parsed < INT_MIN || INT_MAX < parsed)
		return false;
	value = static_cast<int>(parsed);
	return true;
}

bool PathObject::matchesPATSeparator(const Common::String &separators, char expected) {
	if (expected == '\0') {
		for (uint i = 0; i < separators.size(); i++) {
			if (!Common::isSpace(separators[i]))
				return false;
		}
		return true;
	}

	if (expected == ',') {
		if (separators.empty() || separators[0] != ',')
			return false;
		for (uint i = 1; i < separators.size(); i++) {
			if (!Common::isSpace(separators[i]))
				return false;
		}
		return true;
	}

	if (expected == ' ') {
		if (separators.empty())
			return false;
		for (uint i = 0; i < separators.size(); i++) {
			if (!Common::isSpace(separators[i]))
				return false;
		}
		return true;
	}

	return separators == Common::String(expected);
}

bool PathObject::parsePATSegmentRecord(const Common::String &line, bool first, Common::Array<int> &coordinates, int &stepValue, int &waitValue) {
	coordinates.clear();
	if (first && !line.hasPrefix("FIRST=coord:"))
		return false;
	const uint coordCount = first ? 8 : 6;

	Common::StringTokenizer tokenizer(line, "=,: \t\r\n");
	if (tokenizer.empty())
		return false;

	const Common::String key = tokenizer.nextToken();
	if (first) {
		if (key != "FIRST")
			return false;
	} else {
		int extensionIndex = 0;
		if (!key.hasPrefix("NEXT_") || !parsePATIntegerToken(key.substr(5), extensionIndex))
			return false;
		(void)extensionIndex;
	}
	if (!matchesPATSeparator(tokenizer.delimitersAtTokenEnd(), '='))
		return false;

	if (tokenizer.empty() || tokenizer.nextToken() != "coord" || !matchesPATSeparator(tokenizer.delimitersAtTokenEnd(), ':'))
		return false;

	for (uint i = 0; i < coordCount; i++) {
		int coordinate = 0;
		if (tokenizer.empty() || !parsePATIntegerToken(tokenizer.nextToken(), coordinate))
			return false;
		coordinates.push_back(coordinate);
		const char separator = i + 1 < coordCount ? ',' : ' ';
		if (!matchesPATSeparator(tokenizer.delimitersAtTokenEnd(), separator))
			return false;
	}

	if (tokenizer.empty() || tokenizer.nextToken() != "step" || !matchesPATSeparator(tokenizer.delimitersAtTokenEnd(), ':') ||
		tokenizer.empty() || !parsePATIntegerToken(tokenizer.nextToken(), stepValue) ||
		!matchesPATSeparator(tokenizer.delimitersAtTokenEnd(), ' '))
		return false;

	if (tokenizer.empty() || tokenizer.nextToken() != "wait" || !matchesPATSeparator(tokenizer.delimitersAtTokenEnd(), ':') ||
		tokenizer.empty() || !parsePATIntegerToken(tokenizer.nextToken(), waitValue))
		return false;

	return matchesPATSeparator(tokenizer.delimitersAtTokenEnd(), '\0') && tokenizer.empty();
}

PathObject *PathObject::loadPATStream(Zoombini2Engine *vm, Common::SeekableReadStream &f) {
	PathObject *obj = new PathObject(vm);

	// Read FIRST line
	Common::String line = f.readLine();
	Common::Array<int> coords;
	int stepVal = 0;
	int waitVal = 0;
	if (parsePATSegmentRecord(line, true, coords, stepVal, waitVal)) {
		obj->appendSegment(Common::Point32(coords[0], coords[1]), Common::Point32(coords[2], coords[3]),
						   Common::Point32(coords[4], coords[5]), Common::Point32(coords[6], coords[7]), stepVal, waitVal);
	}

	if (obj->segments.empty()) {
		warning("PathObject: PAT has no valid FIRST segment");
		delete obj;
		return nullptr;
	}

	// Read NEXT lines (start with 'N')
	while (!f.eos()) {
		line = f.readLine();
		if (line.empty() || line[0] != 'N')
			break;

		// Parse the continuation coordinates, speed, and endpoint wait.
		if (parsePATSegmentRecord(line, false, coords, stepVal, waitVal)) {
			CubicBezierCurve *prev = obj->segments.back();
			// P0 comes from previous segment's P1
			obj->appendSegment(prev->getEndPosition(), Common::Point32(coords[0], coords[1]),
							   Common::Point32(coords[2], coords[3]), Common::Point32(coords[4], coords[5]), stepVal, waitVal);
		}
	}

	if (obj->segments.empty()) {
		delete obj;
		return nullptr;
	}

	// Set endpoint from last segment
	CubicBezierCurve *last = obj->segments.back();
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
	_finished = false;
	startTime = tickCount;

	for (uint i = 0; i < segments.size(); i++)
		segments[i]->setStartTime(tickCount);

	if (!segments.empty())
		segments[0]->computeCoeffs();
}

void PathObject::setStepValueForAllSegments(int stepValue) {
	for (uint i = 0; i < segments.size(); i++)
		segments[i]->setStepValue(stepValue);
}

/**
 * Advance the path evaluation. Returns true if still walking,
 * false if the entire path is complete.
 * @p outPos receives the current screen pos.
 */
bool PathObject::advance(uint32 tickCount, Common::Point32 &outPos) {
	synchronizeNumericMode();
	if (_finished) {
		outPos = endPos;
		return false;
	}

	if (static_cast<int>(segments.size()) <= currentSegment) {
		_finished = true;
		outPos = endPos;
		return false;
	}

	CubicBezierCurve *seg = segments[currentSegment];
	if (seg->evaluate(tickCount, outPos))
		return true;

	// Advance after the current segment completes.
	currentSegment += 1;
	if (static_cast<int>(segments.size()) <= currentSegment) {
		if (looping && 1 < segments.size()) {
			currentSegment = 1;
			CubicBezierCurve *next = segments[currentSegment];
			next->computeCoeffs();
			next->setStartTime(tickCount);
			next->evaluate(tickCount, outPos);
			return true;
		} else {
			_finished = true;
			outPos = endPos;
			return false;
		}
	}

	// Compute coefficients for the next segment and evaluate it
	CubicBezierCurve *next = segments[currentSegment];
	next->computeCoeffs();
	next->setStartTime(tickCount);
	next->evaluate(tickCount, outPos);
	return true;
}

void PathObject::synchronizeNumericMode() {
	const BezierCurveDataType dataType = _vm->getBezierCurveDataType();
	for (uint i = 0; i < segments.size(); i++)
		segments[i]->setDataType(dataType);
}

ZoombiniRunner::ZoombiniRunner(Zoombini2Engine *vm)
	: _vm(vm) {
}

const ZoombiniAnimation *ZoombiniRunner::getCommonAnimation(ZmbAnimationKind animKind) const {
	assert(_vm && _vm->_gfx);
	return _vm->_gfx->getCommonZoombiniAnimation(animKind);
}

ZoombiniRunner::~ZoombiniRunner() {
	clearMovement();
}

ZoombiniRunner *ZoombiniRunner::cloneForRoster() const {
	ZoombiniRunner *copy = new ZoombiniRunner(_vm);
	copy->setStoredTraits(getTraits());
	copy->setNameBytes(getNameBytes());
	return copy;
}

void ZoombiniRunner::setStoredTraits(const ZmbTrait &traits) {
	_traits = traits;
	_traitHash = _traits.calculateHash();
}

Common::String ZoombiniRunner::getName() const {
	uint length = 0;
	while (length < sizeof(_name) && _name[length])
		length += 1;
	return Common::String(_name, length);
}

void ZoombiniRunner::setTraits(const ZmbTrait &other) {
	_traits._feet = other._feet;
	_traits._nose = other._nose;
	_traits._hair = other._hair;
	_traits._eyes = other._eyes;
	_traitHash = _traits.calculateHash();
}

void ZoombiniRunner::setDefaultAnimation(const ZoombiniAnimation *animation, int cellIndex) {
	_activeAnimation = animation;
	_savedAnimation = animation;
	_animationCell = cellIndex;
	_animationFrame = 0;
	_animationActive = false;
	_idleAnimationEnabled = true;
	_tracksMovementDirection = false;
	_animationAdvancePending = false;
	_bypassAnimationCompletionWork = false;
	updateSpriteSize();
}

void ZoombiniRunner::setActiveAnimation(const ZoombiniAnimation *animation) {
	_activeAnimation = animation;
	updateSpriteSize();
}

void ZoombiniRunner::setPosition(const Common::Point32 &pos) {
	if (_screenPos == pos)
		return;

	_previousScreenPos = _screenPos;
	_screenPos = pos;
	if (!_tracksMovementDirection)
		return;
	if (_directionUpdateCooldown != 0) {
		_directionUpdateCooldown -= 1;
		return;
	}

	_directionUpdateCooldown = 10;
	const int deltaX = pos.x - _previousScreenPos.x;
	const int deltaY = pos.y - _previousScreenPos.y;
	const double distance = sqrt(static_cast<double>(deltaX) * deltaX + static_cast<double>(deltaY) * deltaY);
	if (distance == 0.0)
		return;
	int direction = static_cast<int>(acos(CLIP(static_cast<double>(deltaX) / distance, -1.0, 1.0)) * 180.0 / M_PI) / 22;
	if (_previousScreenPos.y < pos.y)
		direction = -direction;
	static constexpr int kDirectionCells[17] = {
		44,
		41,
		11,
		12,
		22,
		23,
		33,
		36,
		66,
		69,
		99,
		98,
		88,
		87,
		77,
		74,
		44,
	};
	if (-8 <= direction && direction <= 8)
		_animationCell = kDirectionCells[direction + 8];
	updateSpriteSize();
}

void ZoombiniRunner::startMovement(PathObject *path, uint32 tickCount) {
	clearMovement();
	_movementPath = path;
	startMovement(tickCount);
}

bool ZoombiniRunner::loadMovementPathFromPAT(Zoombini2Engine *vm, const Common::Path &path) {
	PathObject *movementPath = vm->loadPagePATPath(path);
	if (!movementPath)
		return false;
	clearMovement();
	_movementPath = movementPath;
	return true;
}

void ZoombiniRunner::startMovement(uint32 tickCount) {
	if (_movementPath)
		_movementPath->start(tickCount);
}

bool ZoombiniRunner::advanceMovement(uint32 tickCount, const Common::Point32 &spriteOffset) {
	if (!_movementPath)
		return false;

	Common::Point32 pathPos;
	if (_movementPath->currentSegment < static_cast<int>(_movementPath->segments.size()))
		pathPos = _movementPath->segments[_movementPath->currentSegment]->getPosition();
	else
		pathPos = _movementPath->endPos;
	const bool active = _movementPath->advance(tickCount, pathPos);
	setPosition(Common::Point32(pathPos.x - spriteOffset.x, pathPos.y - spriteOffset.y));
	return active;
}

void ZoombiniRunner::clearMovement() {
	delete _movementPath;
	_movementPath = nullptr;
}

void ZoombiniRunner::startAnimation(const ZoombiniAnimation *animation, int cellIndex, uint32 tickCount,
									AnimationCompletionPolicy completionPolicy) {
	_savedAnimation = _activeAnimation;
	if (animation)
		_activeAnimation = animation;
	_idleAnimationEnabled = false;
	_animationActive = true;
	_animationCell = cellIndex;
	_animationFrame = 1;
	_nextAnimationFrameTime = tickCount + (_activeAnimation ? _activeAnimation->getFrameDelay() : 0);
	_stopAnimationOnCompletion = false;
	_completionVerticalOffset = 0;
	_animationAdvancePending = false;
	_bypassAnimationCompletionWork = completionPolicy == AnimationCompletionPolicy::kBypassCallbackAndCorrection01;
	updateSpriteSize();
}

void ZoombiniRunner::setAnimationCompleteCallback(AnimationCompleteCallback callback, void *context) {
	_animationCompleteCallback = callback;
	_animationCompleteCallbackContext = context;
}

void ZoombiniRunner::startDirectionTrackedAnimation(uint32 tickCount) {
	_directionUpdateCooldown = 0;
	_tracksMovementDirection = true;
	startAnimation(nullptr, 33, tickCount);
}

bool ZoombiniRunner::tryStartCelebrationAnimation(const ZoombiniAnimation *animation, Random &randomSrc, uint32 tickCount, uint32 elapsedMs, int pacingHz) {
	if (!animation || !canAdvanceFromPage() || _animationActive)
		return false;
	// Bank quota so the original one-in-20 roll runs pacingHz times per second.
	// The remainder spreads multi-roll frames evenly when pacing exceeds the render rate.
	// The celebrate grid is an ordinary page-selected grid, so it repeats like the original.
	_celebrateRollQuota += static_cast<int64>(elapsedMs) * pacingHz;
	while (1000 <= _celebrateRollQuota) {
		_celebrateRollQuota -= 1000;
		if (randomSrc.getRandomNumber(19) == 1) {
			_celebrateRollQuota = 0;
			startAnimation(animation, 33, tickCount);
			return true;
		}
	}
	return false;
}

bool ZoombiniRunner::tryStartIdleAnimation(const ZoombiniAnimation *animation, Random &randomSrc, uint32 tickCount, uint32 elapsedMs, int pacingHz) {
	if (!animation || _hidden || !_idleAnimationEnabled || _animationActive || _movementPath || _dragging)
		return false;
	// Bank quota so the original one-in-250 roll runs pacingHz times per second.
	// The remainder spreads double-roll frames evenly when pacing exceeds the render rate.
	_idleRollQuota += static_cast<int64>(elapsedMs) * pacingHz;
	while (1000 <= _idleRollQuota) {
		_idleRollQuota -= 1000;
		if (randomSrc.getRandomNumber(249) == 1) {
			_idleRollQuota = 0;
			startAnimation(animation, 33, tickCount, AnimationCompletionPolicy::kBypassCallbackAndCorrection01);
			return true;
		}
	}
	return false;
}

void ZoombiniRunner::resetAnimation() {
	_animationActive = false;
	_animationCell = 33;
	_idleAnimationEnabled = true;
	if (_savedAnimation)
		_activeAnimation = _savedAnimation;
	_savedAnimation = _activeAnimation;
	_nextAnimationFrameTime = 0;
	_tracksMovementDirection = false;
	_stopAnimationOnCompletion = false;
	_completionVerticalOffset = 0;
	_animationAdvancePending = false;
	_bypassAnimationCompletionWork = false;
	updateSpriteSize();
}

void ZoombiniRunner::updateAnimation(uint32 tickCount) {
	if (_hidden || _animationAdvancePending)
		return;
	if ((!_animationActive && !_movementPath) || !_activeAnimation)
		return;

	if (_nextAnimationFrameTime < tickCount) {
		_nextAnimationFrameTime = tickCount + _activeAnimation->getFrameDelay();
		_animationAdvancePending = true;
	}
	updateSpriteSize();
}

void ZoombiniRunner::advanceAnimationAfterDraw() {
	if (_animationAdvancePending)
		commitAnimationAdvance();
}

void ZoombiniRunner::commitAnimationAdvance() {
	_animationAdvancePending = false;
	if (!_animationActive && !_movementPath)
		return;

	_animationFrame += 1;
	if (_animationFrame < kAnimationCompletionFrame) {
		updateSpriteSize();
		return;
	}

	if (_bypassAnimationCompletionWork) {
		resetAnimation();
		return;
	}

	if (_animationCompleteCallback) {
		AnimationCompleteCallback callback = _animationCompleteCallback;
		void *callbackContext = _animationCompleteCallbackContext;
		_animationCompleteCallback = nullptr;
		_animationCompleteCallbackContext = nullptr;
		resetAnimation();
		callback(callbackContext, this);
		return;
	}

	if (_stopAnimationOnCompletion) {
		resetAnimation();
		return;
	}

	_screenPos.y -= _completionVerticalOffset;
	_previousScreenPos.y = _screenPos.y;
	_animationFrame = 1;
	_completionVerticalOffset = 0;
	updateSpriteSize();
}

bool ZoombiniRunner::hitTest(const Common::Point32 &point) const {
	return _screenPos.x + 13 < point.x && point.x < _screenPos.x + 44 &&
		   _screenPos.y + 11 < point.y && point.y < _screenPos.y + 42;
}

bool ZoombiniRunner::beginDrag(const Common::Point32 &pointerPos, const ZoombiniAnimation *pickupAnimation, uint32 tickCount) {
	if (!canBeginDrag(pointerPos))
		return false;
	startDrag(pointerPos, pickupAnimation, tickCount);
	return true;
}

bool ZoombiniRunner::canBeginDrag(const Common::Point32 &pointerPos) const {
	const Common::Point32 adjustedHitPoint(pointerPos.x + 2, pointerPos.y - 3);
	return _inputEnabled && !_dragging && hitTest(adjustedHitPoint);
}

void ZoombiniRunner::startDrag(const Common::Point32 &pointerPos, const ZoombiniAnimation *pickupAnimation, uint32 tickCount) {
	_dragging = true;
	_dragOrigin = _screenPos;
	_dragOffset = Common::Point32(pointerPos.x - _screenPos.x - 3, pointerPos.y - _screenPos.y - 10);
	setPosition(Common::Point32(pointerPos.x - 3, pointerPos.y - 10));
	resetAnimation();
	startAnimation(pickupAnimation, 33, tickCount);
}

void ZoombiniRunner::updateDrag(const Common::Point32 &pointerPos) {
	if (!_dragging)
		return;
	setPosition(Common::Point32(pointerPos.x - 3, pointerPos.y - 10));
}

void ZoombiniRunner::endDrag(bool applyGrabOffset) {
	if (!_dragging)
		return;
	_dragging = false;
	if (applyGrabOffset) {
		_screenPos.x -= _dragOffset.x;
		_screenPos.y -= _dragOffset.y;
	}
	_overDropTarget = false;
	_hoveredDropTargetIndex = -1;
	resetAnimation();
}

bool ZoombiniRunner::isStrictlyInside(const Common::Point32 &point, const Common::Rect32 &rect) {
	return rect.left < point.x && point.x < rect.right && rect.top < point.y && point.y < rect.bottom;
}

int ZoombiniRunner::findHoveredDropTarget(const ZoombiniRunner &zoombini, const Common::Point32 &pointerPos, const Common::Array<ZmbDropTarget> &targets) {
	const Common::Point32 footPoint(pointerPos.x - zoombini._dragOffset.x + 17, pointerPos.y - zoombini._dragOffset.y + 46);
	int targetIndex = -1;
	for (uint i = 0; i < targets.size(); i++) {
		if (!targets[i].occupied && isStrictlyInside(footPoint, targets[i].rect))
			targetIndex = static_cast<int>(i);
	}
	return targetIndex;
}

int ZoombiniRunner::findReleaseDropTarget(const ZoombiniRunner &zoombini, const Common::Point32 &pointerPos, const Common::Array<ZmbDropTarget> &targets, int scrollX, int backgroundWidth) {
	Common::Point32 footPoint(pointerPos.x - zoombini._dragOffset.x + 17, pointerPos.y - zoombini._dragOffset.y + 46);
	if (backgroundWidth != AnimationRunner::kDefaultBackgroundWidth) {
		footPoint.x += scrollX;
		if (backgroundWidth <= footPoint.x)
			footPoint.x -= backgroundWidth;
	}

	for (uint i = 0; i < targets.size(); i++) {
		if (!targets[i].occupied && isStrictlyInside(footPoint, targets[i].rect))
			return static_cast<int>(i);
	}
	return -1;
}

ZmbDropResult ZoombiniRunner::handleHoldAndDrop(Zoombini2Engine *vm, const Common::Array<ZoombiniRunner *> &zoombinis, const Common::Point32 &cursorPos,
												ZmbHoldDropInput input, ZmbAnimationKind animKind, Common::Array<ZmbDropTarget> *dropTargets,
												const AreaMask *areaMask, int scrollX, int backgroundWidth) {
	const ZoombiniAnimation *animation = nullptr;
	for (uint i = 0; i < zoombinis.size(); i++) {
		if (zoombinis[i]) {
			animation = zoombinis[i]->getCommonAnimation(animKind);
			break;
		}
	}
	return handleHoldAndDrop(vm, zoombinis, cursorPos, input, animation, dropTargets, areaMask, scrollX, backgroundWidth);
}

ZmbDropResult ZoombiniRunner::handleHoldAndDrop(Zoombini2Engine *vm, const Common::Array<ZoombiniRunner *> &zoombinis, const Common::Point32 &cursorPos,
												ZmbHoldDropInput input, const ZoombiniAnimation *pickupAnimation, Common::Array<ZmbDropTarget> *dropTargets,
												const AreaMask *areaMask, int scrollX, int backgroundWidth) {
	const uint32 tickCount = vm->getTotalPlayTime();
	ZoombiniRunner *dragged = nullptr;
	int draggedIndex = -1;
	for (uint i = 0; i < zoombinis.size(); i++) {
		if (zoombinis[i] && zoombinis[i]->_dragging) {
			if (!dragged) {
				dragged = zoombinis[i];
				draggedIndex = static_cast<int>(i);
			}
		} else if (zoombinis[i]) {
			zoombinis[i]->_overDropTarget = false;
			zoombinis[i]->_hoveredDropTargetIndex = -1;
		}
	}

	if (dragged) {
		dragged->updateDrag(cursorPos);
		dragged->_overDropTarget = false;
		dragged->_hoveredDropTargetIndex = -1;
		if (dropTargets) {
			dragged->_hoveredDropTargetIndex = findHoveredDropTarget(*dragged, cursorPos, *dropTargets);
			dragged->_overDropTarget = dragged->_hoveredDropTargetIndex != -1;
		}

		switch (input) {
		case ZmbHoldDropInput::kMoveOnly:
			return ZmbDropResult::kStillHeld04;
		case ZmbHoldDropInput::kLeftButtonRelease:
			break;
		}
		int targetIndex = -1;
		if (dropTargets)
			targetIndex = findReleaseDropTarget(*dragged, cursorPos, *dropTargets, scrollX, backgroundWidth);
		if (dropTargets && targetIndex != -1) {
			ZmbDropTarget &target = (*dropTargets)[targetIndex];
			dragged->endDrag(false);
			target.occupied = true;
			target.zoombiniIndex = draggedIndex;
			if (target.callback)
				target.callback(target.callbackContext, targetIndex, draggedIndex);
			return ZmbDropResult::kDroppedOnTarget02;
		}
		if (areaMask && areaMask->hasMarkedByteAt(Common::Point32(cursorPos.x, cursorPos.y))) {
			dragged->endDrag(true);
			return ZmbDropResult::kDroppedOnArea03;
		}
		return ZmbDropResult::kStillHeld04;
	}

	switch (input) {
	case ZmbHoldDropInput::kMoveOnly:
		return ZmbDropResult::kIgnored00;
	case ZmbHoldDropInput::kLeftButtonRelease:
		break;
	}
	for (uint i = 0; i < zoombinis.size(); i++) {
		ZoombiniRunner *zoombini = zoombinis[i];
		if (!zoombini || !zoombini->canBeginDrag(cursorPos))
			continue;
		if (dropTargets) {
			for (uint targetIndex = 0; targetIndex < dropTargets->size(); targetIndex++) {
				ZmbDropTarget &target = (*dropTargets)[targetIndex];
				if (!target.occupied || target.zoombiniIndex != static_cast<int>(i))
					continue;
				target.occupied = false;
				if (target.callback)
					target.callback(target.callbackContext, static_cast<int>(targetIndex), static_cast<int>(i));
			}
		}
		zoombini->startDrag(cursorPos, pickupAnimation, tickCount);
		return ZmbDropResult::kPickedUp01;
	}
	return ZmbDropResult::kIgnored00;
}

void ZoombiniRunner::sortDrawOrderByY(const Common::Array<ZoombiniRunner *> &zoombinis, Common::Array<uint> &order) {
	bool swapped;
	do {
		swapped = false;
		for (uint i = 1; i < order.size(); i++) {
			if (zoombinis[order[i]]->_screenPos.y < zoombinis[order[i - 1]]->_screenPos.y) {
				SWAP(order[i - 1], order[i]);
				swapped = true;
			}
		}
	} while (swapped);
}

Common::Point32 ZoombiniRunner::getDrawPosition(int scrollX, int backgroundWidth) const {
	if (_dragging)
		return Common::Point32(_screenPos.x - _dragOffset.x, _screenPos.y - _dragOffset.y);
	if (backgroundWidth == kUnscrolledBackgroundWidth || backgroundWidth == AnimationRunner::kDefaultBackgroundWidth)
		return _screenPos;

	Common::Point32 drawPos(_screenPos.x - scrollX, _screenPos.y);
	if (backgroundWidth - AnimationRunner::kDefaultBackgroundWidth < scrollX)
		drawPos.x += backgroundWidth;
	return drawPos;
}

void ZoombiniRunner::draw(ManagedSurface32 *screen, const AlphaBlendLUT &alphaLUT, const Common::Rect32 *clip,
						  int scrollX, int backgroundWidth, const RleBlock *dropTargetIndicator) const {
	Gfx::drawZoombiniRunner(screen, this, alphaLUT, clip, scrollX, backgroundWidth, dropTargetIndicator);
}

Common::Rect32 ZoombiniRunner::getSpriteRect(int scrollX, int backgroundWidth) const {
	const Common::Point32 pos = getDrawPosition(scrollX, backgroundWidth);
	return Common::Rect32(pos.x, pos.y, pos.x + _spriteSize.width, pos.y + _spriteSize.height);
}

void ZoombiniRunner::updateSpriteSize() {
	Size32 spriteSize;
	if (_activeAnimation) {
		int animationFrame = 0;
		if (_animationActive)
			animationFrame = _animationFrame;
		spriteSize = _activeAnimation->getSpriteSize(_animationCell, animationFrame);
	}
	_spriteSize = Size16(static_cast<int16>(spriteSize.width), static_cast<int16>(spriteSize.height));
}

} // End of namespace Zoombini2
