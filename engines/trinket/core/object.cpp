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
#include "common/util.h"
#include "trinket/string_parser.h"

namespace Trinket {
namespace Core {

ScriptValue::ScriptValue(double value) : _kind(kReal), _real(value) {
	reduce(0);
}

ScriptValue ScriptValue::object(uint32 reference) {
	ScriptValue value;
	value._kind = kObject;
	value._reference = reference;
	return value;
}

ScriptValue ScriptValue::fromText(const Common::String &text) {
	ScriptValue value(text);
	if (text.empty() || (!Common::isDigit(text[0]) && text[0] != '.' && text[0] != '-' && text[0] != '+'))
		return value;
	value._hasText = true;
	if (text.size() <= 10) {
		int32 integer = 0;
		if (StringParser::parseSignedInt(text.c_str(), integer, StringParser::kDecimalProfile) == StringParser::Result::kSuccess &&
			-kIntegerLimit <= integer && integer <= kIntegerLimit) {
			value._kind = kInteger;
			value._integer = integer;
			return value;
		}
	}
	value._kind = kReal;
	value._real = StringParser::parseDoublePrefix(text.c_str());
	value.reduce(text.size());
	return value;
}

ScriptValue ScriptValue::fromLiteral(int32 number, const Common::String &text) {
	ScriptValue value(number);
	value._string = text;
	value._hasText = true;
	return value;
}

ScriptValue ScriptValue::fromLiteral(double number, const Common::String &text) {
	ScriptValue value;
	value._kind = kReal;
	value._real = number;
	value._string = text;
	value._hasText = true;
	return value;
}

void ScriptValue::reduce(uint length) {
	if (_kind != kReal || 10 < length || !(-kIntegerLimit <= _real && _real <= kIntegerLimit))
		return;
	int32 integer = static_cast<int32>(_real);
	if (integer == _real) {
		_kind = kInteger;
		_integer = integer;
	}
}

int32 ScriptValue::truncate(double value) {
	if (!(static_cast<double>(INT64_MIN) <= value && value < -static_cast<double>(INT64_MIN)))
		return 0;
	uint32 bits = static_cast<uint32>(static_cast<int64>(value));
	if (INT32_MAX < bits)
		return INT32_MIN + static_cast<int32>(bits - static_cast<uint32>(INT32_MAX) - 1);
	return static_cast<int32>(bits);
}

int32 ScriptValue::toInt32() const {
	if (_kind == kInteger && !_hasText)
		return _integer;
	return StringParser::parseInt32Prefix(toText().c_str());
}

int32 ScriptValue::toInt32Rounded() const {
	if (_kind == kInteger)
		return _integer;
	if (_kind != kReal)
		return 0;
	int32 lower = truncate(floor(_real));
	int32 upper = truncate(ceil(_real));
	if (static_cast<double>(upper) - _real < _real - static_cast<double>(lower))
		return upper;
	return lower;
}

int32 ScriptValue::toInt32Trunc() const {
	if (_kind == kInteger)
		return _integer;
	if (_kind == kReal)
		return truncate(_real);
	return 0;
}

double ScriptValue::toFloat64() const {
	if (_kind == kInteger)
		return _integer;
	if (_kind == kReal)
		return _real;
	return 0;
}

Common::String ScriptValue::toText() const {
	if (_kind == kString || (isNumeric() && _hasText))
		return _string;
	if (_kind == kInteger)
		return Common::String::format("%d", _integer);
	if (_kind == kReal)
		return Common::String::format("%.15g", _real);
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

uint32 ObjectRegistry::create(const Common::String &name, const ScriptArguments &sargs) {
	auto found = _factories.find(name);
	if (found == _factories.end())
		return 0;
	ScriptableObject *object = found->_value->create(sargs);
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
	return object && _objects[reference - 1].className.equals(className) ? object : nullptr;
}

Common::Array<uint32> ObjectRegistry::references(const Common::String &className) const {
	Common::Array<uint32> result;
	for (uint i = 0; i < _objects.size(); i++) {
		if (_objects[i].object && (className.empty() || _objects[i].className.equals(className)))
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
