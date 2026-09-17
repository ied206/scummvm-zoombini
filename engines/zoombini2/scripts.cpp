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

#include <math.h>

#include "common/ptr.h"
#include "common/textconsole.h"

#include "zoombini2/graphics.h"
#include "zoombini2/random.h"
#include "zoombini2/scripts.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// AnimationRunner
// ============================================================================

AnimationRunner::AnimationRunner(Zoombini2Engine *vm, const Common::Point32 &position, AnimationRunnerMode mode)
	: _vm(vm), _position(position), _mode(mode) {
	const uint32 tickCount = _vm ? _vm->getGameTickCount() : 0;
	if (_mode == AnimationRunnerMode::kLoop01) {
		_currentTimedEntryIndex = 0;
		_cycleStartTime = tickCount;
	} else if (_mode == AnimationRunnerMode::kRandomIdle05) {
		_randomIdleGateTime = tickCount + getRandomDelay(6999);
	}
}

AnimationRunner::~AnimationRunner() {
	delete _backBuffer;
}

bool AnimationRunner::addTimedFrame(int frameIndex, uint32 durationMs) {
	if (kMaxTimedFrameCount <= static_cast<int>(_timedFrames.size())) {
		warning("AnimationRunner: cannot append more than %d timed frames", kMaxTimedFrameCount);
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
	for (int index = static_cast<int>(_timedFrames.size()) - 1; 0 <= index; index -= 1) {
		if (elapsed < _timedFrames[index].cumulativeEndMs) {
			_currentTimedEntryIndex = index;
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
	const int sourceX = _backBufferScreenRect.left - _backBufferDrawPosition.x;
	const int sourceY = _backBufferScreenRect.top - _backBufferDrawPosition.y;
	const Common::Rect sourceRect(sourceX, sourceY, sourceX + _backBufferScreenRect.width(), sourceY + _backBufferScreenRect.height());
	screen->copyRectToSurface(*_backBuffer, _backBufferScreenRect.left, _backBufferScreenRect.top, sourceRect);
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
	if (backgroundWidth == ManagedSurface32::kScreenSize.width)
		return _position.x;
	int screenX = _position.x - scrollX;
	if (backgroundWidth - ManagedSurface32::kScreenSize.width < scrollX)
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
	_backBufferScreenRect = Common::Rect(drawPosition.x, drawPosition.y, drawPosition.x + _backBufferSize.width,
									 drawPosition.y + _backBufferSize.height);
	_backBufferScreenRect.clip(screen->w, screen->h);
	if (_backBufferScreenRect.isEmpty())
		return;
	const int destinationX = _backBufferScreenRect.left - drawPosition.x;
	const int destinationY = _backBufferScreenRect.top - drawPosition.y;
	_backBuffer->copyRectToSurface(*screen, destinationX, destinationY, _backBufferScreenRect);
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

bool Fixed32::operator>(const Fixed32 &right) const {
	return right._rawValue < _rawValue;
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
	const int64 difference = static_cast<int64>(left._rawValue) - static_cast<int64>(right._rawValue);
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
// CurveSegment - runtime-selected cubic Bezier numeric representation.
// ============================================================================

class CurveSegment::Evaluator {
public:
	virtual ~Evaluator() {}
	virtual void init(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1, const Common::Point32 &end,
					  int stepValue, int waitInitial) = 0;
	virtual void computeCoeffs() = 0;
	virtual bool evaluate(uint32 tickCount, Common::Point32 &outPosition) = 0;
	virtual Common::Point32 getPosition() const = 0;
	virtual void setStartTime(uint32 tickCount) = 0;
	virtual uint32 getStartTime() const = 0;
	virtual void setWaitRemaining(int waitRemaining) = 0;
	virtual int getWaitRemaining() const = 0;
	virtual void setParameter(Float32 parameter) = 0;
	virtual Float32 getParameter() const = 0;
};

struct CurveSegment::FixedNumeric {
	typedef Fixed32 Scalar;
	typedef PointFixed32 Point;

	static Scalar fromInt(int32 value) { return Fixed32::fromInt(value); }
	static Scalar fromRaw(int32 value) { return Fixed32::fromRaw(value); }
	static Scalar fromFloat(Float32 value) { return Fixed32::fromFloat(value); }
	static int32 toInt(const Scalar &value) { return value.toInt(); }
	static Float32 toFloat(const Scalar &value) { return value.toFloat(); }
	static bool greater(const Scalar &left, const Scalar &right) { return left > right; }
};

struct CurveSegment::FloatNumeric {
	typedef Float32 Scalar;

	struct Point {
		Scalar x = 0.0f;
		Scalar y = 0.0f;

		Point() = default;
		Point(Scalar xValue, Scalar yValue) : x(xValue), y(yValue) {}

		Point operator+(const Point &right) const { return Point(x + right.x, y + right.y); }
		Point operator-(const Point &right) const { return Point(x - right.x, y - right.y); }
		Point operator*(Scalar right) const { return Point(x * right, y * right); }
	};

	static Scalar fromInt(int32 value) { return static_cast<Scalar>(value); }
	static Scalar fromRaw(int32 value) { return static_cast<Scalar>(value) / 1024.0f; }
	static Scalar fromFloat(Float32 value) { return value; }
	static int32 toInt(Scalar value) {
		int32 result = static_cast<int32>(value);
		if (value < static_cast<Scalar>(result))
			result -= 1;
		return result;
	}
	static Float32 toFloat(Scalar value) { return value; }
	static bool greater(Scalar left, Scalar right) { return right < left; }
};

template<typename Numeric>
class CurveSegment::TypedEvaluator : public CurveSegment::Evaluator {
public:
	typedef typename Numeric::Scalar Scalar;
	typedef typename Numeric::Point Point;

	TypedEvaluator() = default;

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

	void computeCoeffs() override {
		_parameter = Scalar();
		const Scalar three = Numeric::fromInt(3);
		_linear = (_control0 - _start) * three;
		_quadratic = (_control1 - _control0) * three - _linear;
		_cubic = _end - _start - _linear - _quadratic;
		_waitRemaining = _waitInitial;
	}

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

	Common::Point32 getPosition() const override {
		return Common::Point32(Numeric::toInt(_position.x), Numeric::toInt(_position.y));
	}

	void setStartTime(uint32 tickCount) override { _startTime = tickCount; }
	uint32 getStartTime() const override { return _startTime; }
	void setWaitRemaining(int waitRemaining) override { _waitRemaining = waitRemaining; }
	int getWaitRemaining() const override { return _waitRemaining; }
	void setParameter(Float32 parameter) override {
		_parameter = Numeric::fromFloat(parameter);
		calculatePosition();
	}
	Float32 getParameter() const override { return Numeric::toFloat(_parameter); }

private:
	Point _start = Point();
	Point _control0 = Point();
	Point _control1 = Point();
	Point _end = Point();
	Point _cubic = Point();
	Point _quadratic = Point();
	Point _linear = Point();
	Scalar _parameter = Scalar();
	Point _position = Point();
	Scalar _step = Scalar();
	int _waitInitial = 0;
	int _waitRemaining = 0;
	uint32 _startTime = 0;

	void calculatePosition() {
		const Scalar squaredParameter = _parameter * _parameter;
		const Scalar cubedParameter = squaredParameter * _parameter;
		_position = _start + _linear * _parameter + _quadratic * squaredParameter + _cubic * cubedParameter;
	}
};

CurveSegment::CurveSegment(bool useFloatingPoint)
	: _evaluator(createEvaluator(useFloatingPoint)), _useFloatingPoint(useFloatingPoint) {
}

CurveSegment::~CurveSegment() {
	delete _evaluator;
}

void CurveSegment::init(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1, const Common::Point32 &end,
						int stepValue, int waitInitial) {
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

void CurveSegment::computeCoeffs() {
	assert(_initialized);
	_evaluator->computeCoeffs();
	_coefficientsReady = true;
}

bool CurveSegment::evaluate(uint32 tickCount, Common::Point32 &outPosition) {
	assert(_coefficientsReady);
	return _evaluator->evaluate(tickCount, outPosition);
}

Common::Point32 CurveSegment::getStartPosition() const {
	return _start;
}

Common::Point32 CurveSegment::getEndPosition() const {
	return _end;
}

Common::Point32 CurveSegment::getPosition() const {
	return _initialized ? _evaluator->getPosition() : Common::Point32();
}

void CurveSegment::setStartTime(uint32 tickCount) {
	_evaluator->setStartTime(tickCount);
}

void CurveSegment::setStepValue(int stepValue) {
	if (_stepValue == stepValue)
		return;
	_stepValue = stepValue;
	rebuildEvaluator();
}

void CurveSegment::setFloatingPointMode(bool useFloatingPoint) {
	if (_useFloatingPoint == useFloatingPoint)
		return;
	_useFloatingPoint = useFloatingPoint;
	rebuildEvaluator();
}

CurveSegment::Evaluator *CurveSegment::createEvaluator(bool useFloatingPoint) {
	if (useFloatingPoint)
		return new TypedEvaluator<FloatNumeric>();
	return new TypedEvaluator<FixedNumeric>();
}

void CurveSegment::rebuildEvaluator() {
	const uint32 previousStartTime = _evaluator->getStartTime();
	const int previousWaitRemaining = _evaluator->getWaitRemaining();
	const Float32 previousParameter = _evaluator->getParameter();

	delete _evaluator;
	_evaluator = createEvaluator(_useFloatingPoint);
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
// PathObject - Bezier path composed of chained CurveSegments.
// ============================================================================

PathObject::PathObject(Zoombini2Engine *vm)
	: _vm(vm) {
}

PathObject::~PathObject() {
	for (uint i = 0; i < segments.size(); i++)
		delete segments[i];
}

void PathObject::appendSegment(const Common::Point32 &start, const Common::Point32 &control0, const Common::Point32 &control1, const Common::Point32 &end,
							   int stepValue, int waitInitial) {
	CurveSegment *segment = new CurveSegment(_vm->useFloatingPointPaths());
	segment->init(start, control0, control1, end, stepValue, waitInitial);
	segments.push_back(segment);
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
	Common::SeekableReadStream &f = *stream;

	PathObject *obj = new PathObject(vm);

	// Read FIRST line
	Common::String line = f.readLine();
	if (line.hasPrefix("FIRST=coord:")) {
		int x0, y0, cx0, cy0, cx1, cy1, x1, y1, stepVal, waitVal;
		if (sscanf(line.c_str(),
				   "FIRST=coord:%d,%d,%d,%d,%d,%d,%d,%d step:%d wait:%d",
				   &x0, &y0, &cx0, &cy0, &cx1, &cy1, &x1, &y1,
				   &stepVal, &waitVal) >= 10) {
			obj->appendSegment(Common::Point32(x0, y0), Common::Point32(cx0, cy0), Common::Point32(cx1, cy1), Common::Point32(x1, y1), stepVal, waitVal);
		}
	}

	// Read NEXT lines (start with 'N')
	while (!f.eos()) {
		line = f.readLine();
		if (line.empty() || line[0] != 'N')
			break;

		// Parse the continuation coordinates, speed, and endpoint wait.
		int ext, ncx0, ncy0, ncx1, ncy1, nx1, ny1, stepVal, waitVal;
		// Skip the leading 'N' and parse the remaining `EXT` record.
		if (sscanf(line.c_str() + 1,
				   "EXT_%d=coord:%d,%d,%d,%d,%d,%d step:%d wait:%d",
				   &ext, &ncx0, &ncy0, &ncx1, &ncy1, &nx1, &ny1, &stepVal, &waitVal) >= 9) {
			CurveSegment *prev = obj->segments.back();
			// P0 comes from previous segment's P1
			obj->appendSegment(prev->getEndPosition(), Common::Point32(ncx0, ncy0), Common::Point32(ncx1, ncy1), Common::Point32(nx1, ny1), stepVal, waitVal);
		}
	}

	if (obj->segments.empty()) {
		delete obj;
		return nullptr;
	}

	// Set endpoint from last segment
	CurveSegment *last = obj->segments.back();
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
	finished = false;
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
	if (finished) {
		outPos = endPos;
		return false;
	}

	if (currentSegment >= static_cast<int>(segments.size())) {
		finished = true;
		outPos = endPos;
		return false;
	}

	CurveSegment *seg = segments[currentSegment];
	if (seg->evaluate(tickCount, outPos))
		return true;

	// Advance after the current segment completes.
	currentSegment += 1;
	if (static_cast<int>(segments.size()) <= currentSegment) {
		if (looping && 1 < segments.size()) {
			currentSegment = 1;
			CurveSegment *next = segments[currentSegment];
			next->computeCoeffs();
			next->setStartTime(tickCount);
			next->evaluate(tickCount, outPos);
			return true;
		} else {
			finished = true;
			outPos = endPos;
			return false;
		}
	}

	// Compute coefficients for the next segment and evaluate it
	CurveSegment *next = segments[currentSegment];
	next->computeCoeffs();
	next->setStartTime(tickCount);
	next->evaluate(tickCount, outPos);
	return true;
}

void PathObject::synchronizeNumericMode() {
	const bool useFloatingPoint = _vm->useFloatingPointPaths();
	for (uint i = 0; i < segments.size(); i++)
		segments[i]->setFloatingPointMode(useFloatingPoint);
}

ZoombiniRunner::ZoombiniRunner() {
}

ZoombiniRunner::~ZoombiniRunner() {
	clearMovement();
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
	static constexpr int kDirectionCells[17] = {44, 41, 11, 12, 22, 23, 33, 36, 66, 69, 99, 98, 88, 87, 77, 74, 44};
	if (-8 <= direction && direction <= 8)
		_animationCell = kDirectionCells[direction + 8];
	updateSpriteSize();
}

void ZoombiniRunner::startMovement(PathObject *path, uint32 tickCount) {
	clearMovement();
	_movementPath = path;
	if (_movementPath)
		_movementPath->start(tickCount);
}

bool ZoombiniRunner::advanceMovement(uint32 tickCount, const Common::Point32 &spriteOffset) {
	if (!_movementPath)
		return false;

	Common::Point32 pathPos = _movementPath->currentSegment < static_cast<int>(_movementPath->segments.size())
								  ? _movementPath->segments[_movementPath->currentSegment]->getPosition()
								  : _movementPath->endPos;
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

bool ZoombiniRunner::tryStartIdleAnimation(const ZoombiniAnimation *animation, Random &randomSrc, uint32 tickCount) {
	if (!animation || _hidden || !_idleAnimationEnabled || _animationActive || _movementPath || _dragging)
		return false;
	if (randomSrc.getRandomNumber(249) != 1)
		return false;
	startAnimation(animation, 33, tickCount, AnimationCompletionPolicy::kBypassCallbackAndCorrection01);
	return true;
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

int ZoombiniRunner::findHoveredDropTarget(const ZoombiniRunner &zoombini, const Common::Point32 &pointerPos,
										  const Common::Array<ZoombiniDropTarget> &targets) {
	const Common::Point32 footPoint(pointerPos.x - zoombini._dragOffset.x + 17, pointerPos.y - zoombini._dragOffset.y + 46);
	int targetIndex = -1;
	for (uint i = 0; i < targets.size(); i++) {
		if (!targets[i].occupied && isStrictlyInside(footPoint, targets[i].rect))
			targetIndex = static_cast<int>(i);
	}
	return targetIndex;
}

int ZoombiniRunner::findReleaseDropTarget(const ZoombiniRunner &zoombini, const Common::Point32 &pointerPos,
										  const Common::Array<ZoombiniDropTarget> &targets, int scrollX, int backgroundWidth) {
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

ZoombiniInputResult ZoombiniRunner::handlePointerInput(const Common::Array<ZoombiniRunner *> &zoombinis, const Common::Point32 &pointerPos,
													   bool clickReleased, const ZoombiniAnimation *pickupAnimation, uint32 tickCount,
													   Common::Array<ZoombiniDropTarget> *dropTargets, const AreaMask *areaMask,
													   int scrollX, int backgroundWidth) {
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
		dragged->updateDrag(pointerPos);
		dragged->_overDropTarget = false;
		dragged->_hoveredDropTargetIndex = -1;
		if (dropTargets) {
			dragged->_hoveredDropTargetIndex = findHoveredDropTarget(*dragged, pointerPos, *dropTargets);
			dragged->_overDropTarget = dragged->_hoveredDropTargetIndex != -1;
		}

		if (!clickReleased)
			return ZoombiniInputResult::kStillHeld04;
		const int targetIndex = dropTargets ? findReleaseDropTarget(*dragged, pointerPos, *dropTargets, scrollX, backgroundWidth) : -1;
		if (dropTargets && targetIndex != -1) {
			ZoombiniDropTarget &target = (*dropTargets)[targetIndex];
			dragged->endDrag(false);
			target.occupied = true;
			target.zoombiniIndex = draggedIndex;
			if (target.callback)
				target.callback(target.callbackContext, targetIndex, draggedIndex);
			return ZoombiniInputResult::kDroppedOnTarget02;
		}
		if (areaMask && areaMask->hasMarkedByteAt(Common::Point32(pointerPos.x, pointerPos.y))) {
			dragged->endDrag(true);
			return ZoombiniInputResult::kDroppedOnArea03;
		}
		return ZoombiniInputResult::kStillHeld04;
	}

	if (!clickReleased)
		return ZoombiniInputResult::kIgnored00;
	for (uint i = 0; i < zoombinis.size(); i++) {
		ZoombiniRunner *zoombini = zoombinis[i];
		if (!zoombini || !zoombini->canBeginDrag(pointerPos))
			continue;
		if (dropTargets) {
			for (uint targetIndex = 0; targetIndex < dropTargets->size(); targetIndex++) {
				ZoombiniDropTarget &target = (*dropTargets)[targetIndex];
				if (!target.occupied || target.zoombiniIndex != static_cast<int>(i))
					continue;
				target.occupied = false;
				if (target.callback)
					target.callback(target.callbackContext, static_cast<int>(targetIndex), static_cast<int>(i));
			}
		}
		zoombini->startDrag(pointerPos, pickupAnimation, tickCount);
		return ZoombiniInputResult::kPickedUp01;
	}
	return ZoombiniInputResult::kIgnored00;
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
	if (!screen || !_activeAnimation || _hidden)
		return;
	const Common::Rect32 spriteRect = getSpriteRect(scrollX, backgroundWidth);
	if (spriteRect.right <= 0 || spriteRect.bottom <= 0 || screen->w <= spriteRect.left || screen->h <= spriteRect.top)
		return;
	const Common::Point32 drawPos = getDrawPosition(scrollX, backgroundWidth);
	if (_dragging && _hoveredDropTargetIndex != -1 && dropTargetIndicator)
		dropTargetIndicator->drawToScreen(screen, drawPos, alphaLUT);
	const int frame = _animationActive ? _animationFrame : 0;
	_activeAnimation->drawZoombini(screen, _traits, drawPos, _animationCell, frame, alphaLUT, clip);
}

Common::Rect32 ZoombiniRunner::getSpriteRect(int scrollX, int backgroundWidth) const {
	const Common::Point32 pos = getDrawPosition(scrollX, backgroundWidth);
	return Common::Rect32(pos.x, pos.y, pos.x + _spriteSize.width, pos.y + _spriteSize.height);
}

void ZoombiniRunner::updateSpriteSize() {
	const Size32 spriteSize = _activeAnimation ? _activeAnimation->getSpriteSize(_animationCell, _animationActive ? _animationFrame : 0) : Size32();
	_spriteSize = Size16(static_cast<int16>(spriteSize.width), static_cast<int16>(spriteSize.height));
}

} // End of namespace Zoombini2
