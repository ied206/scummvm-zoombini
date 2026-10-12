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

#ifndef TRINKET_CORE_SCRIPT_RUNTIME_H
#define TRINKET_CORE_SCRIPT_RUNTIME_H

#include "trinket/core/object.h"
#include "trinket/core/script.h"

namespace Trinket {
namespace Core {

class ScriptScheduler;

/** Cooperative compiled MPS thread and its native object bindings. */
class ScriptRuntime : private ScriptableObject::Observer {
public:
	class Functions {
	public:
		virtual ~Functions() {}
		virtual bool invoke(const Common::String &name, const ScriptArguments &arguments, ScriptValue &result) = 0;

		/** Commands supplied by the hosting application retain their program counter until accepted. */
		virtual bool command(uint opcode, const ScriptArguments &arguments) { return false; }
	};

	ScriptRuntime() = default;
	~ScriptRuntime();
	bool load(Common::SeekableReadStream &stream);
	void clear();
	void setFunctions(Functions *functions) { _functions = functions; }
	void setTime(uint32 time) { _time = time; }
	ObjectRegistry &objects() { return _objects; }
	const ObjectRegistry &objects() const { return _objects; }
	void setVariable(const Common::String &name, const ScriptValue &value);
	ScriptValue variable(const Common::String &name) const;
	void bindObject(const Common::String &name, ScriptableObject &object);
	bool bindEvent(ScriptableObject &object, const Common::String &event, const Common::String &procedure);
	bool runProcedure(const Common::String &name, const ScriptArguments &arguments = ScriptArguments());
	bool start();
	bool advance(uint32 time, bool ageTimers = false);
	void suspend() { _suspended = true; }
	void resume() { _suspended = false; }
	void resetIdle() { _idle.elapsed = 0; }
	bool isActive() const { return _scheduled && 0 <= _pc && _request == kNoRequest; }
	bool isBlocked() const { return _blocked != 0; }
	bool isSuspended() const { return _suspended; }
	const Common::String &lastError() const { return _error; }

private:
	enum Opcode {
		kEnd00 = 0,
		kCall06 = 6,
		kProcedure07 = 7,
		kForInit08 = 8,
		kForStep09 = 9,
		kCheck10 = 10,
		kJump11 = 11,
		kConditional12 = 12,
		kDebug13 = 13,
		kSay14 = 14,
		kReturn15 = 15,
		kAssign18 = 18,
		kDrop19 = 19,
		kInvoke21 = 21,
		kWait22 = 22,
		kIdle23 = 23,
		kSchedule24 = 24,
		kExit25 = 25,
		kSuspend26 = 26,
		kResume27 = 27,
		kPause28 = 28,
		kDebug29 = 29,
		kSet35 = 35,
		kGet36 = 36,
		kYield37 = 37,
		kPlay40 = 40,
		kRelay45 = 45,
		kQuit46 = 46,
	};
	enum Request {
		kNoRequest,
		kCallScript,
		kRelayScript,
		kExitScript,
		kQuitApplication,
	};
	struct Frame {
		int returnPC = -1;
		ScriptArguments arguments;
	};
	struct Loop {
		int16 current = 0, limit = 0, step = 1;
		int direction = 0;
		int endPC = 0;
	};
	struct Binding {
		uint32 reference = 0;
		Common::String event;
	};
	struct PendingEvent {
		uint32 reference = 0;
		Common::String procedure;
		Common::String name;
		ScriptArguments arguments;
	};
	struct Timer {
		Common::String procedure;
		int32 elapsed = 0, limit = 0;
	};
	struct Interrupt {
		int pc = -1;
		bool condition = true, suspended = false;
		Common::Array<Frame> frames;
	};
	class EventLink : public ObjectLink {
	public:
		EventLink(ScriptRuntime &runtime, const Common::String &procedure) : _runtime(runtime), _procedure(procedure) {}
		ObjectLink *clone() const override { return new EventLink(_runtime, _procedure); }
		void execute(ScriptableObject &source) override;

	private:
		ScriptRuntime &_runtime;
		Common::String _procedure;
	};
	class CompletionLink : public ObjectLink {
	public:
		explicit CompletionLink(ScriptRuntime &runtime) : _runtime(runtime) {}
		ObjectLink *clone() const override { return new CompletionLink(_runtime); }
		void execute(ScriptableObject &source) override;

	private:
		ScriptRuntime &_runtime;
	};

	Script _image;
	ObjectRegistry _objects;
	Functions *_functions = nullptr;
	Common::HashMap<Common::String, ScriptValue> _values;
	Common::HashMap<uint32, Common::String> _objectNames;
	Common::HashMap<Common::String, int> _procedures;
	Common::Array<Binding> _bindings;
	Common::Array<PendingEvent> _events;
	Common::Array<Frame> _frames;
	Common::Array<Loop> _loops;
	Common::String _error;
	int _pc = -1;
	uint32 _time = 0;
	bool _condition = true, _executing = false;
	bool _scheduled = false, _suspended = false, _eventsEnabled = true, _yielded = false, _inInterrupt = false;
	uint32 _blocked = 0, _sleepUntil = 0;
	Timer _idle, _schedule;
	Interrupt _interrupt;
	Request _request = kNoRequest;
	int _requestVariable = -1;
	ScriptScheduler *_scheduler = nullptr;
	void objectEvent(ScriptableObject &source, const Common::String &procedure);
	void objectDestroyed(ScriptableObject &object) override;
	void unblock();
	bool enterInterrupt(const PendingEvent &event);
	void leaveInterrupt();
	bool dispatchEvents();
	bool age();
	void dropVariable(const Common::String &name);
	bool execute(const Common::String &name, const ScriptArguments &arguments);
	bool step();
	bool operands(int start, Common::Array<int16> &indices);
	bool evaluate(int index, ScriptValue &value, uint depth = 0);
	bool variableName(int index, Common::String &name, uint depth = 0);
	bool expression(const Script::Variable &variable, uint &position, int &operand, ScriptValue &value, uint depth);
	bool operate(char operation, const ScriptValue &left, const ScriptValue &right, ScriptValue &result);
	bool arguments(const Common::Array<int16> &indices, uint first, uint end, ScriptArguments &values);
	bool fail(const Common::String &message);
	void jump(int destination);
	int procedure(const Common::String &name) const;
	void initializeConstants();
	ScriptRuntime(const ScriptRuntime &) = delete;
	ScriptRuntime &operator=(const ScriptRuntime &) = delete;
	friend class ScriptScheduler;
};

/** Session-local tick clock and deferred script replacement coordinator. */
class ScriptScheduler {
public:
	class Loader {
	public:
		virtual ~Loader() {}
		virtual bool loadScript(ScriptRuntime &thread, const Common::String &name) = 0;
		virtual void quit() = 0;
	};
	explicit ScriptScheduler(Loader &loader) : _loader(loader) {}
	~ScriptScheduler();
	void attach(ScriptRuntime &thread, const Common::String &name);
	void detach(ScriptRuntime &thread);
	bool tick(uint32 time);
	void resetIdle();
	void suspend();
	void resume();
	bool isTicking() const { return _ticking; }

private:
	struct Thread {
		ScriptRuntime *runtime = nullptr;
		Common::String name;
	};
	Loader &_loader;
	Common::Array<Thread> _threads;
	Common::Array<Common::String> _calledScripts;
	uint32 _lastAge = 0, _sleepUntil = 0;
	bool _ticking = false;
	ScriptScheduler(const ScriptScheduler &) = delete;
	ScriptScheduler &operator=(const ScriptScheduler &) = delete;
	friend class ScriptRuntime;
};

} // End of namespace Core
} // End of namespace Trinket

#endif
