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

#ifndef TRINKET_CORE_OBJECT_H
#define TRINKET_CORE_OBJECT_H

#include "common/array.h"
#include "common/hash-str.h"
#include "common/hashmap.h"

namespace Trinket {
namespace Core {

class ScriptableObject;

/** Values passed between scripts and native object properties. */
class ScriptValue {
public:
	enum Kind {
		kVoid,
		kInteger,
		kReal,
		kString,
		kObject,
	};
	ScriptValue() = default;
	explicit ScriptValue(int32 value) : _kind(kInteger), _number(value) {}
	explicit ScriptValue(double value) : _kind(kReal), _number(value) {}
	explicit ScriptValue(const Common::String &value) : _kind(kString), _string(value) {}
	static ScriptValue object(uint32 reference);
	static ScriptValue fromText(const Common::String &text);
	Kind kind() const { return _kind; }
	int32 integer() const;
	double number() const;
	Common::String text() const;
	uint32 reference() const { return _reference; }
	bool isNumeric() const { return _kind == kInteger || _kind == kReal; }

private:
	Kind _kind = kVoid;
	double _number = 0;
	Common::String _string;
	uint32 _reference = 0;
};

typedef Common::Array<ScriptValue> ScriptArguments;

/** Owned, cloneable callback link for an object's script event. */
class ObjectLink {
public:
	virtual ~ObjectLink() {}
	virtual ObjectLink *clone() const = 0;
	virtual void execute(ScriptableObject &source) = 0;
};

class ScriptableObject {
public:
	class Observer {
	public:
		virtual ~Observer() {}
		virtual void objectDestroyed(ScriptableObject &object) = 0;
	};

	class Lifetime : private Observer {
	public:
		explicit Lifetime(ScriptableObject &object) : _object(&object) { object.addObserver(*this); }
		~Lifetime() {
			if (_object)
				_object->removeObserver(*this);
		}
		bool isAlive() const { return _object != nullptr; }

	private:
		ScriptableObject *_object;
		void objectDestroyed(ScriptableObject &) override { _object = nullptr; }
		Lifetime(const Lifetime &) = delete;
		Lifetime &operator=(const Lifetime &) = delete;
	};

	virtual ~ScriptableObject();
	virtual bool get(const Common::String &property, const ScriptArguments &arguments, ScriptValue &result);
	virtual bool set(const Common::String &property, const ScriptArguments &arguments, const ScriptValue &value);
	virtual bool play(const Common::String &method, const ScriptArguments &arguments, ScriptValue &result);
	virtual ScriptableObject *clone() const { return nullptr; }
	virtual void dispose() { delete this; }
	/** Completion is separate from named events and has one owner at a time. */
	void bindCompletion(const ObjectLink *link);
	void complete();
	bool bindEvent(const Common::String &event, const ObjectLink *link);
	void emitEvent(const Common::String &event);
	void addObserver(Observer &observer);
	void removeObserver(Observer &observer);
	void setCallTime(uint32 time) { _callTime = time; }

protected:
	ScriptableObject() = default;
	void addEvent(const Common::String &event);
	void copyEventsTo(ScriptableObject &copy) const;
	uint32 callTime() const { return _callTime; }

private:
	Common::HashMap<Common::String, ObjectLink *> _events;
	Common::Array<Observer *> _observers;
	ObjectLink *_completion = nullptr;
	uint32 _callTime = 0;
	ScriptableObject(const ScriptableObject &) = delete;
	ScriptableObject &operator=(const ScriptableObject &) = delete;
};

/** Session-local class registration and native-object references. */
class ObjectRegistry : private ScriptableObject::Observer {
public:
	class Factory {
	public:
		virtual ~Factory() {}
		virtual ScriptableObject *create(const ScriptArguments &arguments) = 0;
	};

	~ObjectRegistry();
	bool registerClass(const Common::String &name, Factory &factory);
	bool hasClass(const Common::String &name) const { return _factories.contains(name); }
	uint32 create(const Common::String &name, const ScriptArguments &arguments);
	uint32 clone(uint32 reference);
	uint32 bind(ScriptableObject &object);
	ScriptableObject *resolve(uint32 reference) const;
	ScriptableObject *resolve(uint32 reference, const Common::String &className) const;
	Common::Array<uint32> references(const Common::String &className = Common::String()) const;
	void drop(uint32 reference);
	void clear();

private:
	struct Entry {
		ScriptableObject *object = nullptr;
		bool owned = false;
		Common::String className;
	};
	Common::HashMap<Common::String, Factory *> _factories;
	Common::Array<Entry> _objects;
	void objectDestroyed(ScriptableObject &object) override;
	ObjectRegistry(const ObjectRegistry &) = delete;
	ObjectRegistry &operator=(const ObjectRegistry &) = delete;

public:
	ObjectRegistry() = default;
};

} // End of namespace Core
} // End of namespace Trinket

#endif
