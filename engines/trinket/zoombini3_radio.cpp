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

#include "trinket/zoombini3_radio.h"

namespace Trinket {
namespace Zoombini3 {

RadioGroup::RadioGroup(Core::ScriptRuntime *runtime) : _runtime(runtime) {
	addEvent("changed");
}

RadioGroup::~RadioGroup() {
	for (uint i = 0; i < _members.size(); i++) {
		if (_members[i]) {
			_members[i]->bindEvent("mouseDown", nullptr);
			_members[i]->bindEvent("flyOn", nullptr);
			_members[i]->bindEvent("flyOff", nullptr);
			_members[i]->removeObserver(*this);
		}
	}
}

void RadioGroup::add(Shell::Sprite &member) {
	for (uint i = 0; i < _members.size(); i++) {
		if (_members[i] == &member)
			return;
	}
	_members.push_back(&member);
	member.addObserver(*this);
	MemberLink link(*this);
	member.bindEvent("mouseDown", &link);
	MemberLink on(*this, 1), off(*this, -1);
	member.bindEvent("flyOn", &on);
	member.bindEvent("flyOff", &off);
}

void RadioGroup::MemberLink::execute(Core::ScriptableObject &source) {
	for (uint i = 0; i < _group._members.size(); i++) {
		if (_group._members[i] == &source) {
			if (!_hover)
				_group.clicked(i);
			else if (_group._selected != static_cast<int>(i))
				_group._members[i]->setFrame(0 < _hover ? 4 : 1);
			return;
		}
	}
}

void RadioGroup::clicked(uint index) {
	if (_selected != static_cast<int>(index) && select(index))
		emitEvent("changed");
}

bool RadioGroup::select(int index) {
	if (index < 0 || _members.size() <= static_cast<uint>(index) || !_members[index])
		return false;
	if (_selected == index)
		return true;
	if (0 <= _selected && _members[_selected])
		_members[_selected]->setFrame(1);
	_previous = _selected;
	_selected = index;
	_members[index]->setFrame(2);
	return true;
}

void RadioGroup::clearSelection() {
	_selected = _previous = -1;
	for (uint i = 0; i < _members.size(); i++) {
		if (_members[i])
			_members[i]->setFrame(1);
	}
}

bool RadioGroup::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property == "selection")
		result = Core::ScriptValue(_selected);
	else if (property == "last")
		result = Core::ScriptValue(_previous);
	else if (property == "groupName")
		result = Core::ScriptValue(_name);
	else
		return ScriptableObject::get(property, arguments, result);
	return true;
}

bool RadioGroup::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property == "selection")
		return select(value.integer());
	if (property == "groupName") {
		_name = value.text();
		return true;
	}
	return ScriptableObject::set(property, arguments, value);
}

bool RadioGroup::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (method == "add" && arguments.size() == 1 && _runtime) {
		Core::ScriptValue member = arguments[0];
		if (member.kind() != Core::ScriptValue::kObject)
			member = _runtime->variable(member.text());
		Shell::Sprite *sprite = static_cast<Shell::Sprite *>(_runtime->objects().resolve(member.reference(), "RAnimation"));
		if (!sprite)
			return false;
		add(*sprite);
		return true;
	}
	if (method == "clear") {
		clearSelection();
		return true;
	}
	return ScriptableObject::play(method, arguments, result);
}

void RadioGroup::objectDestroyed(Core::ScriptableObject &object) {
	for (uint i = 0; i < _members.size(); i++) {
		if (_members[i] == &object) {
			_members[i] = nullptr;
			if (_selected == static_cast<int>(i))
				_selected = -1;
			if (_previous == static_cast<int>(i))
				_previous = -1;
		}
	}
}

} // namespace Zoombini3
} // namespace Trinket
