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

#include "trinket/shell/multitrinket.h"

namespace Trinket {
namespace Shell {

MultiTrinket::~MultiTrinket() {
	clearTrinkets();
}

bool MultiTrinket::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property != "userData")
		return Core::Trinket::get(property, arguments, result);
	result = Core::ScriptValue::fromText(_userData);
	return true;
}

bool MultiTrinket::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property != "userData")
		return Core::Trinket::set(property, arguments, value);
	_userData = value.text();
	return true;
}

bool MultiTrinket::addTrinket(Core::Trinket &child) {
	if (&child == this || child.parent())
		return false;
	for (const Core::Trinket *ancestor = this; ancestor; ancestor = ancestor->parent()) {
		if (ancestor == &child)
			return false;
	}
	child.attach(this, this);
	insert(child);
	return true;
}

void MultiTrinket::insert(Core::Trinket &child) {
	uint position = 0;
	while (position < _children.size() && _children[position]->z <= child.z)
		position += 1;
	_children.insert_at(position, &child);
}

void MultiTrinket::removeTrinket(Core::Trinket &child) {
	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i] == &child) {
			_children.remove_at(i);
			child.attach(nullptr, nullptr);
			return;
		}
	}
}

void MultiTrinket::clearTrinkets() {
	for (uint i = 0; i < _children.size(); i++)
		_children[i]->attach(nullptr, nullptr);
	_children.clear();
}

void MultiTrinket::childDestroyed(Core::Trinket &child) {
	removeTrinket(child);
}

void MultiTrinket::childDepthChanged(Core::Trinket &child) {
	removeTrinket(child);
	child.attach(this, this);
	insert(child);
}

} // namespace Shell
} // namespace Trinket
