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

/**
 * Values passed between scripts and native object properties.
 * VM arithmetic reads numeric payloads; native integer parameters parse retained text.
 * @verbatim
 * Authored 1e3 -----> numeric 1000 ------> VM arithmetic
 *              `---> text "1e3" --------> native integer parameter 1
 * Computed 1e3 + 0 -> numeric 1000 -> text "1000" -> native integer parameter 1000
 * @endverbatim
 * Numeric operations produce fresh values whose text is formatted from the result.
 */
class ScriptValue {
public:
	enum Kind {
		kVoid,
		kInteger,
		kReal,
		kString,
		kObject,
	};
	/** Construct a void script value. */
	ScriptValue() = default;
	/** Store a signed 32-bit integer as a numeric script value. */
	explicit ScriptValue(int32 value) : _kind(kInteger), _integer(value) {}
	/** Store a double-precision value, reducing exact integers within the original numeric limits. */
	explicit ScriptValue(double value);
	/** Copy text into a string script value without parsing it. */
	explicit ScriptValue(const Common::String &value) : _kind(kString), _string(value) {}
	/** Create a script object value holding the supplied session-local reference. */
	static ScriptValue object(uint32 reference);
	/** Preserve text and classify a digit, sign or dot prefix using the original numeric reduction rules. */
	static ScriptValue fromText(const Common::String &text);
	/** Restore a compiled integer literal and its authored text without reclassifying it. */
	static ScriptValue fromLiteral(int32 value, const Common::String &text);
	/** Restore a compiled real literal and its authored text without reducing it to an integer. */
	static ScriptValue fromLiteral(double value, const Common::String &text);
	/** Return the stored value kind. */
	Kind kind() const { return _kind; }
	/** Read the decimal integer prefix of the value's text, as native object parameters do. */
	int32 toInt32() const;
	/** Read a VM numeric value as the nearest signed integer, choosing the lower integer on a tie. */
	int32 toInt32Rounded() const;
	/** Truncate a numeric value using the original signed conversion's low 32 bits. */
	int32 toInt32Trunc() const;
	/** Return the numeric payload, or zero for strings and other nonnumeric kinds. */
	double toFloat64() const;
	/** Return retained script text, or format a number or nonzero object reference. */
	Common::String toText() const;
	/**
	 * Convert through @ref Trinket::Core::ScriptValue::toInt32 without validating enumerator membership.
	 * @tparam T Dest enum type, including enum class types.
	 */
	template<class T>
	T toEnum() const { return static_cast<T>(toInt32()); }
	/** Return the stored session-local object reference, or zero for other value kinds. */
	uint32 reference() const { return _reference; }
	/** Return whether this value stores an integer or real rather than text or an object. */
	bool isNumeric() const { return _kind == kInteger || _kind == kReal; }

private:
	/** Smallest value eligible for real-to-integer reduction. */
	static constexpr int32 kIntegerLimitMin = -2000000000;
	/** Largest value eligible for real-to-integer reduction. */
	static constexpr int32 kIntegerLimitMax = 2000000000;
	/** Reduce an exact real to an integer when its original text length and value permit it. */
	void reduce(uint length);
	/**
	 * Truncate @p value toward zero through a signed 64-bit temporary.
	 * @ref Trinket::Core::ScriptValue::toInt32Trunc uses this for VM integer division and remainder operands.
	 * @ref Trinket::Core::ScriptValue::toInt32Rounded uses this for floor/ceil candidates,
	 * including FOR bounds/steps and substring positions/lengths.
	 * @return The signed low 32 bits, or zero for nonfinite inputs or inputs outside the signed 64-bit conversion range.
	 */
	static int32 truncate(double value);
	/** Value kind selecting the payload; retained text can coexist with a numeric payload. */
	Kind _kind = kVoid;
	/** Signed 32-bit payload used by integer values. */
	int32 _integer = 0;
	/** Double-precision payload used by real values. */
	double _real = 0.0;
	/** Retained text for strings and numeric values, also used by native parameter conversions. */
	Common::String _string;
	/** Whether numeric input supplied text, including an explicitly empty literal. */
	bool _hasText = false;
	/** Session-local object identity used by object values; zero denotes no object. */
	uint32 _reference = 0;
};

typedef Common::Array<ScriptValue> ScriptArguments;

/** Callback invoked by a script event or operation completion. */
class ObjectLink {
public:
	/** Allow destruction through the callback interface. */
	virtual ~ObjectLink() {}
	/** Allocate a callback copy for the caller to use and delete. */
	virtual ObjectLink *clone() const = 0;
	/** Run the callback for the object that emitted it. */
	virtual void execute(ScriptableObject &source) = 0;
};

class ScriptableObject {
public:
	class Observer {
	public:
		/** Allow destruction through the observer interface. */
		virtual ~Observer() {}
		/** Notify the observer before the watched object becomes unavailable. */
		virtual void objectDestroyed(ScriptableObject &object) = 0;
	};

	class Lifetime : private Observer {
	public:
		/** Track whether an object survives callbacks that may destroy it. */
		explicit Lifetime(ScriptableObject &object) : _object(&object) { object.addObserver(*this); }
		/** Detach from the watched object if it is still alive. */
		~Lifetime() {
			if (_object)
				_object->removeObserver(*this);
		}
		/** Return whether the watched object has survived since this guard was created. */
		bool isAlive() const { return _object != nullptr; }

	private:
		/** Watched object, cleared by its destruction notification. */
		ScriptableObject *_object;
		/** Clear the watched pointer when the object is destroyed. */
		void objectDestroyed(ScriptableObject &) override { _object = nullptr; }
		/** Prevent accidental copies of this stateful object. */
		Lifetime(const Lifetime &) = delete;
		/** Prevent accidental copies of this stateful object. */
		Lifetime &operator=(const Lifetime &) = delete;
	};

	/** Notify observers and release the event and completion callbacks. */
	virtual ~ScriptableObject();
	/** Read a supported script property into the result, returning false if it is unavailable. */
	virtual bool get(const Common::String &property, const ScriptArguments &sargs, ScriptValue &result);
	/** Assign a supported script property, returning false if the assignment is rejected. */
	virtual bool set(const Common::String &property, const ScriptArguments &sargs, const ScriptValue &value);
	/** Invoke a supported script method and write its result, returning false if it is unavailable. */
	virtual bool play(const Common::String &method, const ScriptArguments &sargs, ScriptValue &result);
	/** Allocate a script clone, or return null when this object cannot be cloned. */
	virtual ScriptableObject *clone() const { return nullptr; }
	/** Delete this scriptable object. */
	virtual void dispose() { delete this; }
	/** Replace the completion callback with a copy of the supplied link, or clear it with null. */
	void bindCompletion(const ObjectLink *link);
	/** Detach and invoke the completion callback so it may safely bind another operation. */
	void complete();
	/** Copy a callback for an existing named event, returning false if the event is unknown. */
	bool bindEvent(const Common::String &event, const ObjectLink *link);
	/** Invoke a temporary callback copy so handlers may replace the binding or destroy the source. */
	void emitEvent(const Common::String &event);
	/** Register a destruction observer once. */
	void addObserver(Observer &observer);
	/** Remove a destruction observer if it is registered. */
	void removeObserver(Observer &observer);
	/** Set the millisecond timestamp used by the current script call. */
	void setCallTime(uint32 time) { _callTime = time; }

protected:
	/** Create an object with no callbacks or observers registered. */
	ScriptableObject() = default;
	/** Declare a named event without binding a callback. */
	void addEvent(const Common::String &event);
	/** Copy named event bindings into another object. */
	void copyEventsTo(ScriptableObject &copy) const;
	/** Return the millisecond timestamp supplied for the current script call. */
	uint32 callTime() const { return _callTime; }

private:
	/** Callback copies indexed by declared script event name. */
	Common::HashMap<Common::String, ObjectLink *> _events;
	/** Observers notified when this object is destroyed. */
	Common::Array<Observer *> _observers;
	/** Callback for the current asynchronous operation, or null if none is bound. */
	ObjectLink *_completion = nullptr;
	/** Millisecond timestamp attached to the current script call. */
	uint32 _callTime = 0;
	/** Prevent accidental copies of this stateful object. */
	ScriptableObject(const ScriptableObject &) = delete;
	/** Prevent accidental copies of this stateful object. */
	ScriptableObject &operator=(const ScriptableObject &) = delete;
};

/** Session-local class registration and native-object references. */
class ObjectRegistry : private ScriptableObject::Observer {
public:
	class Factory {
	public:
		/** Allow destruction through the native-object factory interface. */
		virtual ~Factory() {}
		/** Construct a native object from script arguments for registration. */
		virtual ScriptableObject *create(const ScriptArguments &sargs) = 0;
	};

	/** Dispose of created objects and detach from objects. */
	~ObjectRegistry();
	/** Associate a script class name with a factory, rejecting duplicate names. */
	bool registerClass(const Common::String &name, Factory &factory);
	/** Return whether a factory is registered for the script class name. */
	bool hasClass(const Common::String &name) const { return _factories.contains(name); }
	/** Create and register a native object, returning its reference or zero on failure. */
	uint32 create(const Common::String &name, const ScriptArguments &sargs);
	/** Create and register a clone of a live reference, or return zero on failure. */
	uint32 clone(uint32 reference);
	/** Assign a script reference to an existing object without scheduling its deletion. */
	uint32 bind(ScriptableObject &object);
	/** Resolve a live reference, optionally requiring its registered class name; return null on failure. */
	ScriptableObject *resolve(uint32 reference) const;
	/** Resolve a live reference, optionally requiring its registered class name; return null on failure. */
	ScriptableObject *resolve(uint32 reference, const Common::String &className) const;
	/** List live references, optionally filtering by registered script class name. */
	Common::Array<uint32> references(const Common::String &className = Common::String()) const;
	/** Release a reference, deleting objects created through the registry. */
	void drop(uint32 reference);
	/** Release all references and factories without reusing old reference numbers. */
	void clear();

private:
	struct Entry {
		/** Native object associated with this script reference. */
		ScriptableObject *object = nullptr;
		/** Whether dropping this reference must also dispose of the object. */
		bool owned = false;
		/** Registered script class name, empty for objects bound directly by the host. */
		Common::String className;
	};
	/** Native-object factories indexed by script class name. */
	Common::HashMap<Common::String, Factory *> _factories;
	/** Reference-indexed entries whose positions remain stable after objects are dropped. */
	Common::Array<Entry> _objects;
	/** Invalidate the registry entry when its object is destroyed. */
	void objectDestroyed(ScriptableObject &object) override;
	/** Prevent accidental copies of this stateful object. */
	ObjectRegistry(const ObjectRegistry &) = delete;
	/** Prevent accidental copies of this stateful object. */
	ObjectRegistry &operator=(const ObjectRegistry &) = delete;

public:
	/** Create an empty registry with no script references assigned. */
	ObjectRegistry() = default;
};

} // End of namespace Core
} // End of namespace Trinket

#endif
