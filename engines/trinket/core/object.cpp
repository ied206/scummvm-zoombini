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

#include "trinket/core/object.h"

#include "common/ptr.h"

namespace Trinket {
namespace Core {

ScriptValue ScriptValue::object(uint32 reference) {
	ScriptValue value;
	value._kind = kObject;
	value._reference = reference;
	return value;
}

ScriptValue ScriptValue::fromText(const Common::String &text) {
	if (text.empty())
		return ScriptValue(text);
	char *end = nullptr;
	double number = strtod(text.c_str(), &end);
	if (end && end != text.c_str() && !*end) {
		if (-2147483648.0 <= number && number <= 2147483647.0 && static_cast<int32>(number) == number)
			return ScriptValue(static_cast<int32>(number));
		return ScriptValue(number);
	}
	return ScriptValue(text);
}

int32 ScriptValue::integer() const {
	double value = number();
	if (!(-2147483648.0 <= value && value <= 2147483647.0))
		return 0;
	return static_cast<int32>(value);
}

double ScriptValue::number() const {
	if (isNumeric())
		return _number;
	if (_kind == kString)
		return atof(_string.c_str());
	return 0;
}

Common::String ScriptValue::text() const {
	if (_kind == kInteger)
		return Common::String::format("%d", integer());
	if (_kind == kReal)
		return Common::String::format("%.15g", _number);
	if (_kind == kString)
		return _string;
	if (_kind == kObject && _reference)
		return Common::String::format("@%u", _reference);
	return "0";
}

ScriptableObject::~ScriptableObject() {
	while (!_observers.empty()) {
		Observer *observer = _observers.front();
		_observers.remove_at(0);
		observer->objectDestroyed(*this);
	}
	for (auto it = _events.begin(); it != _events.end(); ++it)
		delete it->_value;
	delete _completion;
}

bool ScriptableObject::get(const Common::String &, const ScriptArguments &, ScriptValue &) {
	return false;
}

bool ScriptableObject::set(const Common::String &, const ScriptArguments &, const ScriptValue &) {
	return false;
}

bool ScriptableObject::play(const Common::String &, const ScriptArguments &, ScriptValue &) {
	return false;
}

void ScriptableObject::bindCompletion(const ObjectLink *link) {
	ObjectLink *copy = link ? link->clone() : nullptr;
	delete _completion;
	_completion = copy;
}

void ScriptableObject::complete() {
	// Detach before dispatch: the callback may start another operation on this object.
	Common::ScopedPtr<ObjectLink> link(_completion);
	_completion = nullptr;
	if (link)
		link->execute(*this);
}

void ScriptableObject::addEvent(const Common::String &event) {
	if (!_events.contains(event))
		_events[event] = nullptr;
}

void ScriptableObject::copyEventsTo(ScriptableObject &copy) const {
	for (auto it = _events.begin(); it != _events.end(); ++it) {
		copy.addEvent(it->_key);
		copy.bindEvent(it->_key, it->_value);
	}
}

bool ScriptableObject::bindEvent(const Common::String &event, const ObjectLink *link) {
	auto found = _events.find(event);
	if (found == _events.end())
		return false;
	ObjectLink *copy = link ? link->clone() : nullptr;
	delete found->_value;
	found->_value = copy;
	return true;
}

void ScriptableObject::emitEvent(const Common::String &event) {
	auto found = _events.find(event);
	if (found == _events.end() || !found->_value)
		return;
	// Keep the dispatched link alive if its handler replaces it or destroys the source.
	Common::ScopedPtr<ObjectLink> link(found->_value->clone());
	link->execute(*this);
}

void ScriptableObject::addObserver(Observer &observer) {
	for (uint i = 0; i < _observers.size(); i++) {
		if (_observers[i] == &observer)
			return;
	}
	_observers.push_back(&observer);
}

void ScriptableObject::removeObserver(Observer &observer) {
	for (uint i = 0; i < _observers.size(); i++) {
		if (_observers[i] == &observer) {
			_observers.remove_at(i);
			return;
		}
	}
}

ObjectRegistry::~ObjectRegistry() {
	clear();
}

bool ObjectRegistry::registerClass(const Common::String &name, Factory &factory) {
	if (name.empty() || _factories.contains(name))
		return false;
	_factories[name] = &factory;
	return true;
}

uint32 ObjectRegistry::bind(ScriptableObject &object) {
	for (uint i = 0; i < _objects.size(); i++) {
		if (_objects[i].object == &object)
			return i + 1;
	}
	Entry entry;
	entry.object = &object;
	_objects.push_back(entry);
	object.addObserver(*this);
	return _objects.size();
}

uint32 ObjectRegistry::create(const Common::String &name, const ScriptArguments &arguments) {
	auto found = _factories.find(name);
	if (found == _factories.end())
		return 0;
	ScriptableObject *object = found->_value->create(arguments);
	if (!object)
		return 0;
	uint32 reference = bind(*object);
	_objects[reference - 1].owned = true;
	_objects[reference - 1].className = name;
	return reference;
}

uint32 ObjectRegistry::clone(uint32 reference) {
	ScriptableObject *source = resolve(reference);
	ScriptableObject *copy = source ? source->clone() : nullptr;
	if (!copy)
		return 0;
	uint32 result = bind(*copy);
	_objects[result - 1].owned = true;
	_objects[result - 1].className = _objects[reference - 1].className;
	return result;
}

ScriptableObject *ObjectRegistry::resolve(uint32 reference) const {
	if (!reference || _objects.size() < reference)
		return nullptr;
	return _objects[reference - 1].object;
}

ScriptableObject *ObjectRegistry::resolve(uint32 reference, const Common::String &className) const {
	ScriptableObject *object = resolve(reference);
	return object && _objects[reference - 1].className == className ? object : nullptr;
}

Common::Array<uint32> ObjectRegistry::references(const Common::String &className) const {
	Common::Array<uint32> result;
	for (uint i = 0; i < _objects.size(); i++) {
		if (_objects[i].object && (className.empty() || _objects[i].className == className))
			result.push_back(i + 1);
	}
	return result;
}

void ObjectRegistry::drop(uint32 reference) {
	if (!reference || _objects.size() < reference)
		return;
	Entry entry = _objects[reference - 1];
	_objects[reference - 1].object = nullptr;
	if (!entry.object)
		return;
	entry.object->removeObserver(*this);
	if (entry.owned)
		entry.object->dispose();
}

void ObjectRegistry::clear() {
	for (uint i = 0; i < _objects.size(); i++)
		drop(i + 1);
	// Reference numbers are not reused after a clear.
	_factories.clear();
}

void ObjectRegistry::objectDestroyed(ScriptableObject &object) {
	for (uint i = 0; i < _objects.size(); i++) {
		if (_objects[i].object == &object)
			_objects[i].object = nullptr;
	}
}

} // namespace Core
} // namespace Trinket
