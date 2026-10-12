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

Trinket::Trinket() {
	addEvent("mouseDown");
	addEvent("mouseUp");
	addEvent("dblClick");
	addEvent("flyOn");
	addEvent("flyOff");
}

Trinket::~Trinket() {
	if (_childObserver)
		_childObserver->childDestroyed(*this);
}

void Trinket::attach(Trinket *parent, ChildObserver *observer) {
	_parent = parent;
	_childObserver = observer;
}

bool Trinket::get(const Common::String &property, const ScriptArguments &arguments, ScriptValue &result) {
	if (property == "x")
		result = ScriptValue(x);
	else if (property == "y")
		result = ScriptValue(y);
	else if (property == "z")
		result = ScriptValue(z);
	else if (property == "visible")
		result = ScriptValue(static_cast<int32>(visible));
	else if (property == "touchy")
		result = ScriptValue(static_cast<int32>(enabled));
	else if (property == "movable")
		result = ScriptValue(static_cast<int32>(movable));
	else if (property == "tag")
		result = ScriptValue(tag);
	else if (property == "width")
		result = ScriptValue(width());
	else if (property == "height")
		result = ScriptValue(height());
	else
		return ScriptableObject::get(property, arguments, result);
	return true;
}

bool Trinket::set(const Common::String &property, const ScriptArguments &arguments, const ScriptValue &value) {
	if (property == "x")
		x = value.integer();
	else if (property == "y")
		y = value.integer();
	else if (property == "z") {
		z = value.integer();
		if (_childObserver)
			_childObserver->childDepthChanged(*this);
	} else if (property == "visible")
		visible = value.integer() != 0;
	else if (property == "touchy")
		enabled = value.integer() != 0;
	else if (property == "movable")
		movable = value.integer() != 0;
	else if (property == "tag")
		tag = value.integer();
	else
		return ScriptableObject::set(property, arguments, value);
	return true;
}

bool Trinket::play(const Common::String &method, const ScriptArguments &arguments, ScriptValue &result) {
	if (method == "setAOCursor" && !arguments.empty() && arguments.size() <= 2) {
		spotCursor = arguments[0].text();
		if (1 < arguments.size())
			grabCursor = arguments[1].text();
		return true;
	}
	if ((method != "place" && method != "offset") || arguments.size() != 2)
		return ScriptableObject::play(method, arguments, result);
	if (method == "place") {
		return set("x", ScriptArguments(), arguments[0]) && set("y", ScriptArguments(), arguments[1]);
	} else {
		return set("x", ScriptArguments(), ScriptValue(static_cast<int32>(x + arguments[0].integer()))) &&
			   set("y", ScriptArguments(), ScriptValue(static_cast<int32>(y + arguments[1].integer())));
	}
}

} // namespace Core
} // namespace Trinket
