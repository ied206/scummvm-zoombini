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

#include "trinket/core/trinket.h"

namespace Trinket {
namespace Core {

constexpr const char *Trinket::kEventMouseDown;
constexpr const char *Trinket::kEventMouseUp;
constexpr const char *Trinket::kEventDblClick;
constexpr const char *Trinket::kEventFlyOn;
constexpr const char *Trinket::kEventFlyOff;
constexpr const char *Trinket::kPropertyX;
constexpr const char *Trinket::kPropertyY;
constexpr const char *Trinket::kPropertyZ;
constexpr const char *Trinket::kPropertyVisible;
constexpr const char *Trinket::kPropertyTouchy;
constexpr const char *Trinket::kPropertyMovable;
constexpr const char *Trinket::kPropertyTag;
constexpr const char *Trinket::kPropertyWidth;
constexpr const char *Trinket::kPropertyHeight;
constexpr const char *Trinket::kMethodSetAOCursor;
constexpr const char *Trinket::kMethodPlace;
constexpr const char *Trinket::kOffsetName;

Trinket::Trinket() {
	addEvent(kEventMouseDown);
	addEvent(kEventMouseUp);
	addEvent(kEventDblClick);
	addEvent(kEventFlyOn);
	addEvent(kEventFlyOff);
}

Trinket::~Trinket() {
	if (_childObserver)
		_childObserver->childDestroyed(*this);
}

void Trinket::attach(Trinket *parent, ChildObserver *observer) {
	_parent = parent;
	_childObserver = observer;
}

bool Trinket::get(const Common::String &property, const ScriptArguments &sargs, ScriptValue &result) {
	if (property.equals(kPropertyX))
		result = ScriptValue(x);
	else if (property.equals(kPropertyY))
		result = ScriptValue(y);
	else if (property.equals(kPropertyZ))
		result = ScriptValue(z);
	else if (property.equals(kPropertyVisible))
		result = ScriptValue(static_cast<int32>(visible));
	else if (property.equals(kPropertyTouchy))
		result = ScriptValue(static_cast<int32>(enabled));
	else if (property.equals(kPropertyMovable))
		result = ScriptValue(static_cast<int32>(movable));
	else if (property.equals(kPropertyTag))
		result = ScriptValue(tag);
	else if (property.equals(kPropertyWidth))
		result = ScriptValue(width());
	else if (property.equals(kPropertyHeight))
		result = ScriptValue(height());
	else
		return ScriptableObject::get(property, sargs, result);
	return true;
}

bool Trinket::set(const Common::String &property, const ScriptArguments &sargs, const ScriptValue &value) {
	if (property.equals(kPropertyX))
		x = value.toInt32();
	else if (property.equals(kPropertyY))
		y = value.toInt32();
	else if (property.equals(kPropertyZ)) {
		z = value.toInt32();
		if (_childObserver)
			_childObserver->childDepthChanged(*this);
	} else if (property.equals(kPropertyVisible))
		visible = value.toInt32() != 0;
	else if (property.equals(kPropertyTouchy))
		enabled = value.toInt32() != 0;
	else if (property.equals(kPropertyMovable))
		movable = value.toInt32() != 0;
	else if (property.equals(kPropertyTag))
		tag = value.toInt32();
	else
		return ScriptableObject::set(property, sargs, value);
	return true;
}

bool Trinket::play(const Common::String &method, const ScriptArguments &sargs, ScriptValue &result) {
	if (method.equals(kMethodSetAOCursor) && !sargs.empty() && sargs.size() <= 2) {
		spotCursor = sargs[0].toText();
		if (1 < sargs.size())
			grabCursor = sargs[1].toText();
		return true;
	}
	if ((!method.equals(kMethodPlace) && !method.equals(kOffsetName)) || sargs.size() != 2)
		return ScriptableObject::play(method, sargs, result);
	if (method.equals(kMethodPlace)) {
		return set(kPropertyX, ScriptArguments(), sargs[0]) && set(kPropertyY, ScriptArguments(), sargs[1]);
	} else {
		return set(kPropertyX, ScriptArguments(), ScriptValue(x + sargs[0].toInt32())) &&
			   set(kPropertyY, ScriptArguments(), ScriptValue(y + sargs[1].toInt32()));
	}
}

} // namespace Core
} // namespace Trinket
