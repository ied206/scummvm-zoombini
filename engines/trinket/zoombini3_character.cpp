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

#include "trinket/zoombini3_character.h"

#include "common/system.h"
#include "common/util.h"
#include "trinket/math.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace Zoombini3 {

constexpr const char *AttributeCharacter::kEventFinished;
constexpr const char *AttributeCharacter::kEventDropped;
constexpr const char *AttributeCharacter::kPropertyRealX;
constexpr const char *AttributeCharacter::kPropertyRealY;
constexpr const char *AttributeCharacter::kPropertyDroppedX;
constexpr const char *AttributeCharacter::kPropertyDroppedY;
constexpr const char *AttributeCharacter::kPropertyIsCorralled;
constexpr const char *AttributeCharacter::kPropertyDoWalkSFX;
constexpr const char *AttributeCharacter::kPropertyDoZUpdates;
constexpr const char *AttributeCharacter::kPropertyZBaseY;
constexpr const char *AttributeCharacter::kPropertyLoadFidgetsInBkgd;
constexpr const char *AttributeCharacter::kMethodPlayAnim;
constexpr const char *AttributeCharacter::kMethodSetHomePoint;
constexpr const char *AttributeCharacter::kMethodForceDrop;
constexpr const char *AttributeCharacter::kMethodAddWalkMove;
constexpr const char *AttributeCharacter::kMethodWalk;
constexpr const char *AttributeCharacter::kMethodStopAction;
constexpr const char *AttributeCharacter::kMethodStop;
constexpr const char *AttributeCharacter::kMethodPause;
constexpr const char *AttributeCharacter::kMethodResume;
constexpr const char *AttributeCharacter::kPropertyX;
constexpr const char *AttributeCharacter::kPropertyY;
constexpr const char *AttributeCharacter::kPropertyVisible;
constexpr const char *AttributeCharacter::kPropertyZ;

constexpr const char *AttributeCharacter::kAsset;
constexpr const char *AttributeCharacter::kSound;

AttributeCharacter::AttributeCharacter(TrinketEngine &vm, Shell::Scene &scene) : _vm(vm), _scene(scene), _walkSound(vm.mixer()) {
	addEvent(kEventFinished);
	addEvent(kEventDropped);
}

bool AttributeCharacter::get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (property.equals(kPropertyRealX))
		result = Core::ScriptValue(static_cast<int32>(_position.x));
	else if (property.equals(kPropertyRealY))
		result = Core::ScriptValue(static_cast<int32>(_position.y));
	else if (property.equals(kPropertyDroppedX))
		result = Core::ScriptValue(static_cast<int32>(_dropped.x));
	else if (property.equals(kPropertyDroppedY))
		result = Core::ScriptValue(static_cast<int32>(_dropped.y));
	else if (property.equals(kPropertyIsCorralled))
		result = Core::ScriptValue(static_cast<int32>(_isCorralled));
	else if (property.equals(kPropertyDoWalkSFX))
		result = Core::ScriptValue(static_cast<int32>(_doWalkSFX));
	else if (property.equals(kPropertyDoZUpdates))
		result = Core::ScriptValue(static_cast<int32>(_depthFromY));
	else if (property.equals(kPropertyZBaseY))
		result = Core::ScriptValue(static_cast<int32>(_depthOriginY));
	else if (property.equals(kPropertyLoadFidgetsInBkgd))
		result = Core::ScriptValue(static_cast<int32>(_loadFidgetsBg));
	else
		return Shell::MultiTrinket::get(property, sargs, result);
	return true;
}

bool AttributeCharacter::play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (method.equals(kMethodPlayAnim) && !sargs.empty() && sargs.size() <= 4)
		return playFeedback(sargs[0].toInt32(), 1 < sargs.size() ? sargs[1].toInt32() : 1,
							sargs.size() < 3 || sargs[2].toInt32() != 0, 3 < sargs.size() ? sargs[3].toInt32() : -1, callTime());
	if (method.equals(kMethodSetHomePoint) && sargs.size() == 2) {
		setPosition(Common::Point(sargs[0].toInt32(), sargs[1].toInt32()));
		return true;
	}
	if (method.equals(kMethodForceDrop) && sargs.empty()) {
		endDrag(false);
		return true;
	}
	if (method.equals(kMethodAddWalkMove) && (sargs.size() == 2 || sargs.size() == 3)) {
		addWalkMove(Common::Point(sargs[0].toInt32(), sargs[1].toInt32()), sargs.size() < 3 || sargs[2].toInt32() != 0);
		return true;
	}
	if (method.equals(kMethodWalk) && sargs.size() <= 2)
		return walk(true, callTime(), sargs.empty() || sargs[0].toInt32() != 0, 1 < sargs.size() && sargs[1].toInt32() != 0);
	if ((method.equals(kMethodStopAction) || method.equals(kMethodStop)) && sargs.empty()) {
		stopWalk(!_keepWalkPosition);
		stopFeedback();
		return true;
	}
	if ((method.equals(kMethodPause) || method.equals(kMethodResume)) && sargs.empty()) {
		pause(method.equals(kMethodPause), callTime());
		return true;
	}
	return Shell::MultiTrinket::play(method, sargs, result);
}

bool AttributeCharacter::set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) {
	if (property.equals(kPropertyX))
		setPosition(Common::Point(value.toInt32(), _position.y));
	else if (property.equals(kPropertyY))
		setPosition(Common::Point(_position.x, value.toInt32()));
	else if (property.equals(kPropertyVisible))
		setVisible(value.toInt32() != 0, callTime());
	else if (property.equals(kPropertyZ))
		setDepth(value.toInt32());
	else if (property.equals(kPropertyDoWalkSFX))
		_doWalkSFX = value.toInt32() != 0;
	else if (property.equals(kPropertyDoZUpdates))
		setDepthFromY(value.toInt32() != 0, _depthOriginY);
	else if (property.equals(kPropertyZBaseY))
		setDepthFromY(_depthFromY, value.toInt32());
	else if (property.equals(kPropertyLoadFidgetsInBkgd))
		_loadFidgetsBg = value.toInt32() != 0;
	else if (property.equals(kPropertyIsCorralled)) {
		_scriptCorral = true;
		_isCorralled = value.toInt32() != 0;
	} else
		return Shell::MultiTrinket::set(property, sargs, value);
	return true;
}

AttributeCharacter::~AttributeCharacter() {
	_walkSound.stop();
	clearTimers(g_system->getMillis());
	clearParts(_moving);
	clearParts(_dragParts);
	clearParts(_feedback);
	clearParts(_base);
}

uint32 AttributeCharacter::ticks(uint32 time) {
	// Equivalent to truncating the original double product for a 32-bit millisecond count.
	return static_cast<uint64>(time) * 60 / 1000;
}

void AttributeCharacter::clearParts(Common::Array<Shell::Sprite *> &parts) {
	for (uint i = 0; i < parts.size(); i++)
		_scene.remove(parts[i]);
	parts.clear();
}

bool AttributeCharacter::playFeedback(uint resource, int repeat, bool keepVisible, int selector, uint32 time) {
	if (_walking || _dragging || !_feedback.empty())
		return true;
	if (selector != -2 && selector != -1)
		return false;
	finishVariant(time);
	_feedbackKeepVisible = keepVisible;
	if (selector == -2) {
		Common::String path = _animRes ? _animRes->characterAnimationPath(resource) : Common::String::format(kAsset, resource);
		Shell::Sprite *sprite = _scene.add(path, _z);
		if (!sprite)
			return false;
		sprite->managed = true;
		sprite->enabled = false;
		sprite->x += _position.x - _origin.x;
		sprite->y += _position.y - _origin.y;
		if (!sprite->startAnimation()) {
			_scene.remove(sprite);
			return false;
		}
		addTrinket(*sprite);
		sprite->playAnimation();
		_feedback.push_back(sprite);
	} else {
		for (uint i = 0; i < _count; i++) {
			Shell::Sprite *part = loadPart(resource + i * 10 + _traits[i], i);
			_feedback.push_back(part);
			if (part) {
				part->setRepeat(repeat);
				part->setFrame(1);
				part->playAnimation();
				part->pauseAnimation(i != 0);
			}
		}
	}
	for (uint i = 0; i < _base.size(); i++) {
		if (_base[i])
			_base[i]->visible = false;
	}
	return _valid;
}

void AttributeCharacter::stopFeedback() {
	if (_feedback.empty())
		return;
	clearParts(_feedback);
	for (uint i = 0; i < _base.size(); i++) {
		if (_base[i])
			_base[i]->visible = _visible;
	}
}

void AttributeCharacter::clearTimers(uint32 time) {
	// Removing active feedback still reschedules it before deleting its timer.
	finishVariant(time);
	for (uint i = 0; i < _timers.size(); i++) {
		for (uint j = 0; j < _timers[i].variants.size(); j++)
			clearParts(_timers[i].variants[j].parts);
	}
	_timers.clear();
	_activeTimer = _activeVariant = -1;
}

Shell::Sprite *AttributeCharacter::loadPart(uint resource, uint attribute, bool movement) {
	static const int layers[] = {
		2,
		1,
		5,
		4,
		3,
		6,
	};
	static const int zerbleLayers[] = {
		1,
		3,
		2,
	};
	int layer = layers[attribute];
	if (_type == 1)
		layer = movement ? attribute + 1 : zerbleLayers[attribute];
	Shell::Sprite *part = _scene.add(Common::String::format(kAsset, resource), _z + layer);
	if (!part) {
		_valid = false;
		warning("Could not load character resource %u", resource);
		return nullptr;
	}
	part->managed = true;
	addTrinket(*part);
	part->visible = _visible;
	part->x += _position.x - _origin.x;
	part->y += _position.y - _origin.y;
	part->mirrorX = _position.x;
	part->mirrored = _mirror;
	part->startAnimation(-1);
	part->setFrame(1);
	return part;
}

bool AttributeCharacter::create(const Core::ScriptArguments &sargs, uint32 time) {
	if (sargs.empty() || sargs[0].toInt32() < 0 || 2 < sargs[0].toInt32())
		return false;
	uint type = sargs[0].toInt32();
	uint count;
	if (type == 1)
		count = 3;
	else if (type == 2)
		count = 6;
	else
		count = 5;
	if (sargs.size() != count + 3 && sargs.size() != count + 5)
		return false;
	int32 traits[6] = {};
	for (uint i = 0; i < count; i++)
		traits[i] = sargs[i + 1].toInt32();
	Common::Point position(11111, 11111);
	int depth = sargs[count + 2].toInt32();
	if (sargs.size() == count + 5) {
		position = Common::Point(sargs[count + 2].toInt32(), sargs[count + 3].toInt32());
		depth = sargs[count + 4].toInt32();
	}
	return create(type, traits, sargs[count + 1].toInt32(), position, depth, time);
}

bool AttributeCharacter::create(uint type, const int32 *traits, uint pose, Common::Point position, int depth, uint32 time) {
	if (2 < type || 5 < pose)
		return false;
	_type = type;
	if (type == 1)
		_count = 3;
	else if (type == 2)
		_count = 6;
	else
		_count = 5;
	for (uint i = 0; i < _count; i++) {
		int count;
		if (i == 0 || i == 5)
			count = 1;
		else if (type == 1)
			count = 3;
		else
			count = 5;
		if (traits[i] < 0 || count <= traits[i])
			return false;
		_traits[i] = traits[i];
	}
	_origin = type == 1 ? Common::Point(395, 302) : Common::Point(399, 319);
	if (position.x == 11111 || position.y == 11111)
		position = _origin;
	_position = position;
	x = position.x;
	y = position.y;
	_home = position;
	_z = depth;
	z = depth;
	return setPose(pose, time);
}

void AttributeCharacter::schedule(Timer &timer, uint32 time) {
	uint seconds = timer.minimum + _vm.rnd().draw() % (timer.maximum - timer.minimum);
	timer.deadline = ticks(time) + 60 * seconds;
}

void AttributeCharacter::addTimer(uint minimum, uint maximum, uint firstBase, int maskAttribute, uint secondBase, uint32 time) {
	Timer timer;
	timer.minimum = minimum;
	timer.maximum = maximum;
	schedule(timer, time);
	for (uint v = 0; v < (secondBase ? 2u : 1u); v++) {
		Variant variant;
		for (uint a = 0; a < _count; a++) {
			Shell::Sprite *part = nullptr;
			if (maskAttribute < 0 || static_cast<uint>(maskAttribute) == a) {
				part = loadPart((v ? secondBase : firstBase) + a * 10 + _traits[a], a);
				if (part)
					part->visible = false;
			}
			variant.parts.push_back(part);
		}
		timer.variants.push_back(variant);
		timer.available.push_back(v);
	}
	_timers.push_back(timer);
}

bool AttributeCharacter::setPose(uint pose, uint32 time) {
	if (5 < pose)
		return false;
	_mirror = 3 <= pose;
	int basePose = pose % 3;
	if (_pose != basePose) {
		_pose = basePose;
		clearParts(_base);
		uint base = _type == 1 ? 2500 + basePose * 30 : 600 + basePose * 60;
		for (uint i = 0; i < _count; i++)
			_base.push_back(loadPart(base + i * 10 + _traits[i], i));
		clearTimers(time);
		if (_type == 1) {
			addTimer(5, 8, 3190 + basePose * 30, 0, 0, time);
			if (basePose == 0)
				addTimer(10, 80, 3310, -1, 3280, time);
		} else if (basePose < 2) {
			addTimer(5, 8, 1900 + basePose * 50, 3, 0, time);
		}
	}
	for (uint i = 0; i < _base.size(); i++) {
		if (_base[i])
			_base[i]->mirrored = _mirror;
	}
	if (0 <= _activeTimer) {
		Variant &variant = _timers[_activeTimer].variants[_activeVariant];
		for (uint i = 0; i < variant.parts.size(); i++) {
			if (variant.parts[i])
				variant.parts[i]->mirrored = _mirror;
		}
	}
	return _valid;
}

void AttributeCharacter::startVariant(uint timerIndex, uint variantIndex) {
	_activeTimer = timerIndex;
	_activeVariant = variantIndex;
	Variant &variant = _timers[timerIndex].variants[variantIndex];
	bool first = true;
	for (uint i = 0; i < variant.parts.size(); i++) {
		Shell::Sprite *part = variant.parts[i];
		if (!part)
			continue;
		if (_base[i])
			_base[i]->visible = false;
		part->visible = _visible;
		part->mirrored = _mirror;
		part->startAnimation(0);
		part->setFrame(1);
		part->pauseAnimation(!first);
		first = false;
	}
}

void AttributeCharacter::finishVariant(uint32 time) {
	if (_activeTimer < 0)
		return;
	Timer &timer = _timers[_activeTimer];
	Variant &variant = timer.variants[_activeVariant];
	for (uint i = 0; i < variant.parts.size(); i++) {
		if (variant.parts[i]) {
			variant.parts[i]->visible = false;
			variant.parts[i]->pauseAnimation(true);
			if (_base[i])
				_base[i]->visible = _visible;
		}
	}
	schedule(timer, time);
	_activeTimer = _activeVariant = -1;
}

Common::Rect AttributeCharacter::bounds() const {
	Common::Array<Shell::Sprite *> parts = _walking ? _moving : _base;
	if (!_feedback.empty())
		parts = _feedback;
	if (_dragging)
		parts = _dragParts;
	if (0 <= _activeTimer) {
		const Variant &variant = _timers[_activeTimer].variants[_activeVariant];
		for (uint i = 0; i < variant.parts.size(); i++)
			parts.push_back(variant.parts[i]);
	}
	Common::Rect result;
	bool first = true;
	for (uint i = 0; i < parts.size(); i++) {
		Shell::Sprite *part = parts[i];
		if (!part || !part->visible)
			continue;
		int width = part->width();
		int left = part->mirrored ? 2 * part->mirrorX - part->x - width : part->x;
		Common::Rect bounds(left, part->y, left + width, part->y + part->height());
		if (first)
			result = bounds;
		else
			result.extend(bounds);
		first = false;
	}
	return result;
}

void AttributeCharacter::addWalkMove(Common::Point destination, bool absolute) {
	_path.push_back(destination);
	_absolutePath.push_back(absolute);
}

void AttributeCharacter::setVisible(bool shown, uint32 time) {
	visible = shown;
	if (_visible == shown)
		return;
	if (!shown && _dragging)
		endDrag(false);
	_visible = shown;
	finishVariant(time);
	Common::Array<Shell::Sprite *> &parts = _walking ? _moving : _base;
	for (uint i = 0; i < parts.size(); i++) {
		if (parts[i])
			parts[i]->visible = shown && _feedback.empty();
	}
	for (uint i = 0; i < _feedback.size(); i++) {
		if (_feedback[i])
			_feedback[i]->visible = shown;
	}
}

void AttributeCharacter::setDepth(int depth) {
	if (_z == depth)
		return;
	Common::Array<Shell::Sprite *> parts = _base;
	parts.push_back(_moving);
	parts.push_back(_dragParts);
	parts.push_back(_feedback);
	for (uint i = 0; i < _timers.size(); i++) {
		for (uint j = 0; j < _timers[i].variants.size(); j++)
			parts.push_back(_timers[i].variants[j].parts);
	}
	for (uint i = 0; i < parts.size(); i++) {
		if (parts[i])
			_scene.setDepth(parts[i], parts[i]->z + depth - _z);
	}
	_z = depth;
	Core::Trinket::set(kPropertyZ, Core::ScriptArguments(), Core::ScriptValue(static_cast<int32>(depth)));
}

void AttributeCharacter::setDepthFromY(bool usePosition, int originY) {
	if (usePosition && !_depthFromY)
		_depthBase = _z;
	_depthFromY = usePosition;
	_depthOriginY = originY;
	if (usePosition && !_dragging)
		setDepth(_depthBase - _depthOriginY + _position.y);
}

void AttributeCharacter::stopWalk(bool finishPath) {
	_walkListener = nullptr;
	if (!_walking)
		return;
	if (finishPath && !_path.empty()) {
		Common::Point destination = _start + _delta;
		for (uint i = _nextPoint; i < _path.size(); i++)
			destination = _absolutePath[i] ? _path[i] : destination + _path[i];
		moveTo(destination);
	}
	clearParts(_moving);
	_walkSound.stop();
	if (_clearWalkPath) {
		_path.clear();
		_absolutePath.clear();
	}
	_walking = false;
	for (uint i = 0; i < _base.size(); i++) {
		if (_base[i])
			_base[i]->visible = _visible;
	}
}

bool AttributeCharacter::beginDrag(Common::Point mouse, uint32 time, const Common::Rect &corral) {
	if (_paused || !_visible || _dragging)
		return false;
	stopWalk(true);
	stopFeedback();
	finishVariant(time);
	_dragHome = _home;
	_dragOffset = mouse - _position;
	_dragDepth = _z;
	_dragCorral = _scriptCorral && !_isCorralled ? Common::Rect() : corral;
	_dragOutsideX = _dragOutsideY = false;
	setDepth(10000);
	for (uint i = 0; i < _count; i++) {
		uint base;
		if (i == 1) {
			if (_type == 1)
				base = 3060;
			else
				base = 2030;
		} else {
			if (_type == 1)
				base = 2530;
			else
				base = 660;
		}
		Shell::Sprite *part = loadPart(base + i * 10 + _traits[i], i);
		_dragParts.push_back(part);
		if (part)
			part->pauseAnimation(i != 1);
		if (_base[i])
			_base[i]->visible = false;
	}
	_dragging = true;
	return _valid;
}

void AttributeCharacter::dragTo(Common::Point mouse) {
	if (!_dragging || _paused)
		return;
	Common::Point wanted = mouse - _dragOffset;
	if (_dragCorral.isEmpty()) {
		moveTo(wanted);
		return;
	}
	Common::Rect box = bounds();
	int minimumX = _dragCorral.left - box.left + _position.x;
	int maximumX = _dragCorral.right - box.right + _position.x;
	int minimumY = _dragCorral.top - box.top + _position.y;
	int maximumY = _dragCorral.bottom - box.bottom + _position.y;
	bool outsideX = wanted.x < minimumX || maximumX < wanted.x;
	bool outsideY = wanted.y < minimumY || maximumY < wanted.y;
	// Once clipped, retain that edge until the unconstrained anchor re-enters.
	if (_dragOutsideX && outsideX)
		wanted.x = _position.x;
	else
		wanted.x = CLIP<int>(wanted.x, minimumX, maximumX);
	if (_dragOutsideY && outsideY)
		wanted.y = _position.y;
	else
		wanted.y = CLIP<int>(wanted.y, minimumY, maximumY);
	_dragOutsideX = outsideX;
	_dragOutsideY = outsideY;
	moveTo(wanted);
}

Common::Point AttributeCharacter::endDrag(bool notify) {
	Common::Point dropped = _position;
	if (!_dragging)
		return dropped;
	moveTo(_dragHome);
	clearParts(_dragParts);
	setDepth(_dragDepth);
	_dragging = false;
	for (uint i = 0; i < _base.size(); i++) {
		if (_base[i])
			_base[i]->visible = _visible;
	}
	if (notify) {
		_dropped = dropped;
		emitEvent(kEventDropped);
	}
	return dropped;
}

void AttributeCharacter::beginSegment() {
	_start = _position;
	_delta = _absolutePath[_nextPoint] ? _path[_nextPoint] - _position : _path[_nextPoint];
	_nextPoint += 1;
	_distance = Math::distance(_delta);
	_travelled = 0;
	_lastMove = 0;
	double angle = atan2(-static_cast<double>(_delta.y), static_cast<double>(_delta.x)) * 180.0f * (1.0 / M_PI);
	if (angle < 0)
		angle += 360;
	static const int zmbDirections[][2] = {
		{1000, 315},
		{1300, 0},
		{1600, 45}};
	static const int zerbleDirections[][2] = {
		{2700, 342},
		{3400, 306},
		{2820, 0},
		{2940, 36},
		{3520, 72}};
	const int (*directions)[2] = _type == 1 ? zerbleDirections : zmbDirections;
	uint count = _type == 1 ? 5 : 3;
	float best = 360.0f;
	uint direction = 0;
	bool mirror = false;
	for (uint i = 0; i < count; i++) {
		for (uint flipped = 0; flipped < 2; flipped++) {
			int facing = directions[i][1];
			if (flipped)
				facing = facing < 90 ? 180 - facing : 540 - facing;
			double difference = fabs(facing - angle);
			const double wrapped = 360.0f - difference;
			if (wrapped < difference)
				difference = static_cast<float>(wrapped);
			if (difference < best) {
				best = difference;
				direction = i;
				mirror = flipped != 0;
			}
		}
	}
	uint frame = 1;
	uint frameCount = 0;
	if (1 < _moving.size() && _moving[1]) {
		frameCount = _moving[1]->getNumberOfFrames();
		frame = _moving[1]->getFrame() + 1;
		if (frameCount < frame)
			frame = 1;
	}
	if (_direction < 0 || static_cast<uint>(_direction) != direction) {
		clearParts(_moving);
		for (uint i = 0; i < _count; i++) {
			uint resource = directions[direction][0] + i * 10 + (_type == 1 ? 30 : 60) * _traits[1];
			if (i != 1)
				resource += _traits[i];
			_moving.push_back(loadPart(resource, i, true));
			if (_base[i])
				_base[i]->visible = false;
		}
	}
	if (_direction < 0) {
		frame = 1;
	} else if (_walkMirror != mirror && _traits[1] < 2) {
		frame += frameCount / 2;
		if (frameCount < frame)
			frame -= frameCount;
	}
	bool first = true;
	for (uint i = 0; i < _moving.size(); i++) {
		if (_moving[i]) {
			_moving[i]->startAnimation(-1);
			_moving[i]->setFrame(frame);
			_moving[i]->pauseAnimation(!first);
			_moving[i]->mirrored = mirror;
			first = false;
		}
	}
	_direction = direction;
	_walkMirror = mirror;
}

bool AttributeCharacter::walk(bool sound, uint32 time, bool clearPath, bool keepPosition) {
	if (_walking || _path.empty())
		return false;
	if (_dragging)
		endDrag(false);
	stopFeedback();
	finishVariant(time);
	_clearWalkPath = clearPath;
	_keepWalkPosition = keepPosition;
	_direction = -1;
	_nextPoint = 0;
	beginSegment();
	_walking = true;
	if (sound && _doWalkSFX)
		_walkSound.play(_vm.resources(), Common::String::format(kSound, (_type == 1 ? 2723 : 1310) + _traits[1]),
						Audio::Mixer::kSFXSoundType, 1);
	return _valid;
}

void AttributeCharacter::moveTo(Common::Point position) {
	Common::Point offset = position - _position;
	Common::Array<Shell::Sprite *> parts = _base;
	parts.push_back(_moving);
	parts.push_back(_dragParts);
	parts.push_back(_feedback);
	for (uint i = 0; i < _timers.size(); i++) {
		for (uint j = 0; j < _timers[i].variants.size(); j++)
			parts.push_back(_timers[i].variants[j].parts);
	}
	for (uint i = 0; i < parts.size(); i++) {
		if (parts[i]) {
			parts[i]->x += offset.x;
			parts[i]->y += offset.y;
			parts[i]->mirrorX = position.x;
		}
	}
	_position = position;
	x = position.x;
	y = position.y;
	if (_depthFromY && !_dragging)
		setDepth(_depthBase - _depthOriginY + _position.y);
}

void AttributeCharacter::updateParts(Common::Array<Shell::Sprite *> &parts, uint32 time, Common::Array<uint16> &sounds) {
	Shell::Sprite *leader = nullptr;
	for (uint i = 0; i < parts.size(); i++) {
		if (!parts[i])
			continue;
		if (!leader) {
			leader = parts[i];
			leader->update(time, sounds);
		} else {
			parts[i]->setFrame(leader->displayedFrame());
		}
	}
}

bool AttributeCharacter::update(uint32 time, Common::Array<uint16> &sounds) {
	bool finished = false;
	if (_walking) {
		if (!_paused) {
			uint32 previous = _lastMove;
			_lastMove = time;
			if (previous) {
				static constexpr float speed = 80.0f * 0.001f;
				const float step = static_cast<float>(static_cast<double>(time - previous) * speed);
				// Store the next frame's distance after rounding, but interpolate this frame before rounding.
				const double travelled = static_cast<double>(_travelled) + step;
				_travelled = static_cast<float>(travelled);
				Common::Point delta = _delta;
				if (travelled < _distance) {
					double fraction = travelled / _distance;
					int movedX = static_cast<int>(abs(_delta.x) * fraction);
					float movedY = abs(_delta.y) * fraction;
					delta.x = _delta.x < 0 ? -movedX : movedX;
					delta.y = _delta.y < 0 ? -static_cast<int>(movedY) : static_cast<int>(movedY);
				}
				moveTo(_start + delta);
				if (delta == _delta) {
					if (_nextPoint < _path.size()) {
						beginSegment();
					} else {
						clearParts(_moving);
						_walkSound.stop();
						for (uint i = 0; i < _base.size(); i++) {
							if (_base[i])
								_base[i]->visible = _visible;
						}
						if (_clearWalkPath) {
							_path.clear();
							_absolutePath.clear();
						}
						_walking = false;
						finished = true;
					}
				}
			}
		}
	} else {
		for (uint i = 0; i < _timers.size(); i++) {
			Timer &timer = _timers[i];
			if (ticks(time) < timer.deadline)
				continue;
			if (_paused || _dragging || !_feedback.empty() || 0 <= _activeTimer) {
				schedule(timer, time);
			} else if (!timer.available.empty()) {
				uint choice = _vm.rnd().draw() % timer.available.size();
				startVariant(i, timer.available[choice]);
				timer.available.remove_at(choice);
				if (timer.available.empty()) {
					// Restore authored order immediately, before the selected animation finishes.
					for (uint j = 0; j < timer.variants.size(); j++)
						timer.available.push_back(j);
				}
			}
			break;
		}
	}
	if (!_paused) {
		if (!_feedback.empty()) {
			updateParts(_feedback, time, sounds);
			Shell::Sprite *leader = nullptr;
			for (uint i = 0; i < _feedback.size() && !leader; i++)
				leader = _feedback[i];
			if (leader && !leader->isRunning()) {
				stopFeedback();
				if (!_feedbackKeepVisible)
					setVisible(false, time);
				finished = true;
			}
		}
		if (_dragging && 1 < _dragParts.size() && _dragParts[1]) {
			_dragParts[1]->update(time, sounds);
			for (uint i = 0; i < _dragParts.size(); i++) {
				if (i != 1 && _dragParts[i])
					_dragParts[i]->setFrame(_dragParts[1]->displayedFrame());
			}
		}
		if (_walking)
			updateParts(_moving, time, sounds);
		if (0 <= _activeTimer) {
			Variant &variant = _timers[_activeTimer].variants[_activeVariant];
			updateParts(variant.parts, time, sounds);
			for (uint i = 0; i < variant.parts.size(); i++) {
				if (variant.parts[i]) {
					if (!variant.parts[i]->isRunning())
						finishVariant(time);
					break;
				}
			}
		}
	}
	if (finished) {
		Core::ScriptableObject::Lifetime lifetime(*this);
		WalkListener *listener = _walkListener;
		_walkListener = nullptr;
		if (listener)
			listener->walkFinished(*this);
		if (lifetime.isAlive())
			complete();
		if (lifetime.isAlive())
			emitEvent(kEventFinished);
	}
	return finished;
}

void AttributeCharacter::pause(bool paused, uint32 time) {
	if (_dragging && 1 < _dragParts.size() && _dragParts[1])
		_dragParts[1]->pauseAnimation(paused);
	if (_paused != paused) {
		Common::Array<Shell::Sprite *> *parts = nullptr;
		if (_walking) {
			parts = &_moving;
			_walkSound.pause(paused);
		} else if (!_feedback.empty()) {
			parts = &_feedback;
		} else if (0 <= _activeTimer) {
			parts = &_timers[_activeTimer].variants[_activeVariant].parts;
		}
		// Only the first present part advances; the other parts follow its frame.
		if (parts) {
			for (uint i = 0; i < parts->size(); i++) {
				if ((*parts)[i]) {
					(*parts)[i]->pauseAnimation(paused);
					break;
				}
			}
		}
	}
	if (!paused)
		_lastMove = time;
	_paused = paused;
}

} // namespace Zoombini3
} // namespace Trinket
