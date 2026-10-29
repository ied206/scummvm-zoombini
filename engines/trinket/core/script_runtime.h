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
		/** Allow destruction through the host-function interface. */
		virtual ~Functions() {}
		/** Dispatch a named host function and write its result, returning false if the call fails. */
		virtual bool invoke(const Common::String &name, const ScriptArguments &sargs, ScriptValue &result) = 0;

		/** Commands supplied by the hosting application retain their program counter until accepted. */
		virtual bool command(uint opcode, const ScriptArguments &sargs) { return false; }
	};

	/** Create an inactive thread with no compiled image or host bindings. */
	ScriptRuntime() = default;
	/** Detach the thread and release its objects and callbacks. */
	~ScriptRuntime();
	/** Load a compiled MPS image and initialize its variables and constants. */
	bool load(Common::SeekableReadStream &stream);
	/** Detach bindings and reset the image, object registry and execution state. */
	void clear();
	/** Return the host function dispatcher, or detach it with null. */
	void setFunctions(Functions *functions) { _functions = functions; }
	/** Set the current millisecond timestamp without executing instructions. */
	void setTime(uint32 time) { _time = time; }
	/** Return the thread's session-local native-object registry. */
	ObjectRegistry &objects() { return _objects; }
	/** Return the thread's session-local native-object registry. */
	const ObjectRegistry &objects() const { return _objects; }
	/** Assign a named variable, reclassifying nonobject text with the original VM's numeric rules. */
	void setVariable(const Common::String &name, const ScriptValue &value);
	/** Return the named runtime value, or a void value when it has not been assigned. */
	ScriptValue variable(const Common::String &name) const;
	/** Bind a native object to a script variable name. */
	void bindObject(const Common::String &name, ScriptableObject &object);
	/** Bind a native object's named event to a script procedure. */
	bool bindEvent(ScriptableObject &object, const Common::String &event, const Common::String &procedure);
	/** Enter a named procedure with the supplied arguments, returning false if dispatch fails. */
	bool runProcedure(const Common::String &name, const ScriptArguments &sargs = ScriptArguments());
	/** Schedule the loaded script from its entry point. */
	bool start();
	/** Run ready work at the supplied millisecond time, optionally aging script timers. */
	bool advance(uint32 time, bool ageTimers = false);
	/** Prevent ordinary instruction execution until the thread resumes. */
	void suspend() { _suspended = true; }
	/** Allow ordinary instruction execution after suspension. */
	void resume() { _suspended = false; }
	/** Reset accumulated idle-timer ticks without changing the bound procedure. */
	void resetIdle() { _idle.elapsed = 0; }
	/** Return whether this thread is scheduled at a valid instruction with no replacement request. */
	bool isActive() const { return _scheduled && 0 <= _pc && _request == kNoRequest; }
	/** Return whether a native asynchronous operation is holding the thread. */
	bool isBlocked() const { return _blocked != 0; }
	/** Return whether ordinary instruction execution is suspended. */
	bool isSuspended() const { return _suspended; }
	/** Return the most recent execution or image-loading error. */
	const Common::String &lastError() const { return _error; }

private:
	/** Prefix used to recognize authored constant names. */
	static constexpr const char *kConstantPrefix = "k";
	/** Script identifier for sYSTEMWIN. */
	static constexpr const char *kSYSTEMWINName = "SYSTEMWIN";
	/** Script identifier for rECEIVER. */
	static constexpr const char *kRECEIVERName = "RECEIVER";
	/** Script identifier for rESULT. */
	static constexpr const char *kRESULTName = "RESULT";
	/** Script identifier for schedule. */
	static constexpr const char *kScheduleName = "schedule";
	/** Script identifier for idle. */
	static constexpr const char *kIdleName = "idle";
	/** Script identifier for random Number. */
	static constexpr const char *kRandomNumberName = "RandomNumber";
	/** Script identifier for stack. */
	static constexpr const char *kStackName = "stack";
	/** Script identifier for wait. */
	static constexpr const char *kWaitName = "wait";
	/** Script identifier for trinkets. */
	static constexpr const char *kTrinketsName = "trinkets";
	/** Script identifier for var. */
	static constexpr const char *kVarName = "var";
	/** Script identifier for script. */
	static constexpr const char *kScriptName = "script";
	/** Script identifier for default. */
	static constexpr const char *kDefaultName = "default";
	/** Script property that returns the bound variable name. */
	static constexpr const char *kNameProperty = "name";

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
		/** Instruction index to resume after the procedure returns. */
		int returnPC = -1;
		/** Script arguments retained for this procedure call. */
		ScriptArguments sargs;
	};
	struct Loop {
		/** Current signed 16-bit loop value. */
		int16 current = 0;
		/** Inclusive final value of the loop. */
		int16 limit = 0;
		/** Signed increment applied at each loop iteration. */
		int16 step = 1;
		/** Comparison direction chosen from the initial value and limit. */
		int direction = 0;
		/** Instruction index used to leave the loop. */
		int endPC = 0;
	};
	struct Binding {
		/** Object reference associated with the event binding. */
		uint32 reference = 0;
		/** Name of the event bound to a script procedure. */
		Common::String event;
	};
	struct PendingEvent {
		/** Session-local source object reference, or zero for a timer event. */
		uint32 reference = 0;
		/** Script procedure to enter when this queued event is dispatched. */
		Common::String procedure;
		/** Source object's script variable name supplied during event dispatch. */
		Common::String name;
		/** Arguments delivered when the queued event procedure runs. */
		ScriptArguments sargs;
	};
	struct Timer {
		/** Procedure invoked when the timer becomes due. */
		Common::String procedure;
		/** Number of scheduler aging ticks accumulated since the last reset. */
		int32 elapsed = 0;
		/** Aging-tick threshold that must be exceeded before the procedure runs. */
		int32 limit = 0;
	};
	struct Interrupt {
		/** Instruction index saved while an event procedure interrupts execution. */
		int pc = -1;
		/** Saved conditional state of the interrupted procedure. */
		bool condition = true;
		/** Saved suspension state restored after the event procedure returns. */
		bool suspended = false;
		/** Call frames preserved while the event procedure runs. */
		Common::Array<Frame> frames;
	};
	class EventLink : public ObjectLink {
	public:
		/** Bind a runtime to a named script event procedure. */
		EventLink(ScriptRuntime &runtime, const Common::String &procedure) : _runtime(runtime), _procedure(procedure) {}
		/** Allocate a callback copy retaining the same runtime and procedure. */
		ObjectLink *clone() const override { return new EventLink(_runtime, _procedure); }
		/** Queue the bound procedure for the object that emitted the event. */
		void execute(ScriptableObject &source) override;

	private:
		/** Runtime that receives the queued event. */
		ScriptRuntime &_runtime;
		/** Procedure name copied into each queued event. */
		Common::String _procedure;
	};
	class CompletionLink : public ObjectLink {
	public:
		/** Bind an asynchronous completion callback to a runtime. */
		explicit CompletionLink(ScriptRuntime &runtime) : _runtime(runtime) {}
		/** Allocate another callback for the same runtime. */
		ObjectLink *clone() const override { return new CompletionLink(_runtime); }
		/** Release the runtime's wait on the completed object. */
		void execute(ScriptableObject &source) override;

	private:
		/** Runtime waiting for the native operation to finish. */
		ScriptRuntime &_runtime;
	};

	/** Compiled instructions, operands and authored variables loaded for this thread. */
	Script _image;
	/** Registry that tracks native objects referenced by this thread. */
	ObjectRegistry _objects;
	/** Host dispatcher for named functions and application commands. */
	Functions *_functions = nullptr;
	/** Current script values indexed by resolved variable name. */
	Common::HashMap<Common::String, ScriptValue> _values;
	/** Script variable names indexed by bound native-object reference. */
	Common::HashMap<uint32, Common::String> _objectNames;
	/** Procedure entry instruction indices indexed by name. */
	Common::HashMap<Common::String, int> _procedures;
	/** Event bindings that must be detached when this thread clears. */
	Common::Array<Binding> _bindings;
	/** Pending native and timer events in delivery order. */
	Common::Array<PendingEvent> _events;
	/** Procedure return positions and retained call arguments. */
	Common::Array<Frame> _frames;
	/** Loop counters for the active execution stack. */
	Common::Array<Loop> _loops;
	/** Most recent failure description, empty while execution is valid. */
	Common::String _error;
	/** Next instruction index, or minus one when execution has stopped. */
	int _pc = -1;
	/** Current host timestamp in milliseconds. */
	uint32 _time = 0;
	/** Conditional flag consumed by branch instructions. */
	bool _condition = true;
	/** Whether an execution pass is already on the stack. */
	bool _executing = false;
	/** Whether the thread remains eligible for scheduler advancement. */
	bool _scheduled = false;
	/** Whether ordinary instruction execution is suspended. */
	bool _suspended = false;
	/** Whether queued events may interrupt ordinary execution. */
	bool _eventsEnabled = true;
	/** Whether this execution pass has voluntarily yielded. */
	bool _yielded = false;
	/** Whether an event procedure is using the saved interrupt context. */
	bool _inInterrupt = false;
	/** Reference of the native object whose completion the thread awaits. */
	uint32 _blocked = 0;
	/** Millisecond deadline that postpones thread or scheduler execution. */
	uint32 _sleepUntil = 0;
	/** Idle procedure and its accumulated aging ticks. */
	Timer _idle;
	/** Scheduled procedure and its accumulated aging ticks. */
	Timer _schedule;
	/** Execution context preserved during an event procedure. */
	Interrupt _interrupt;
	/** Deferred script replacement, exit or application-quit request. */
	Request _request = kNoRequest;
	/** Variable index naming the requested script, or minus one when unused. */
	int _requestVar = -1;
	/** Scheduler coordinating this thread, or null when detached. */
	ScriptScheduler *_scheduler = nullptr;
	/** Queue the source object's event procedure for later dispatch. */
	void objectEvent(ScriptableObject &source, const Common::String &procedure);
	/** Remove bindings and references to an object that is being destroyed. */
	void objectDestroyed(ScriptableObject &object) override;
	/** Clear the native-operation wait and its completion binding. */
	void unblock();
	/** Save the current execution context and enter a queued event procedure. */
	bool enterInterrupt(const PendingEvent &event);
	/** Restore the execution context saved before the event procedure. */
	void leaveInterrupt();
	/** Deliver ready queued events until execution must yield or fails. */
	bool dispatchEvents();
	/** Increment timer ticks and dispatch any timer procedure that becomes due. */
	bool age();
	/** Drop the variable's native-object reference and remove its current value. */
	void dropVariable(const Common::String &name);
	/** Set up a named procedure call with its arguments. */
	bool execute(const Common::String &name, const ScriptArguments &sargs);
	/** Execute one instruction and update the program counter. */
	bool step();
	/** Collect the variable indices in an instruction's terminated operand list. */
	bool operands(int start, Common::Array<int16> &indices);
	/** Resolve a variable to a value while bounding recursive evaluation depth. */
	bool evaluate(int index, ScriptValue &value, uint depth = 0);
	/** Read a numeric operand without evaluating nonnumeric literals or consuming placeholder random draws. */
	bool evaluateNumber(int index, ScriptValue &value, uint depth = 0);
	/** Read an evaluated operand using its original classification's rounding or decimal-prefix rule. */
	int32 integer(int index, const ScriptValue &value) const;
	/** Resolve an authored variable path to its runtime name. */
	bool variableName(int index, Common::String &name, uint depth = 0);
	/** Evaluate an encoded Polish expression while consuming operators and operands. */
	bool expression(const Script::Variable &variable, uint &position, int &operand, ScriptValue &value, uint depth);
	/** Apply one script operator to its operands and write the result. */
	bool operate(char operation, const ScriptValue &left, const ScriptValue &right, ScriptValue &result);
	/** Evaluate a half-open range of operand indices into call arguments. */
	bool arguments(const Common::Array<int16> &indices, uint first, uint end, ScriptArguments &values);
	/** Record an execution error and return false. */
	bool fail(const Common::String &message);
	/** Set the next instruction index after checking the destination. */
	void jump(int destination);
	/** Return a named procedure's entry index, or minus one when it is unavailable. */
	int procedure(const Common::String &name) const;
	/** Materialize declared constants without executing scene setup. */
	void initConstants();
	/** Prevent accidental copies of this stateful object. */
	ScriptRuntime(const ScriptRuntime &) = delete;
	/** Prevent accidental copies of this stateful object. */
	ScriptRuntime &operator=(const ScriptRuntime &) = delete;
	friend class ScriptScheduler;
};

/** Session-local tick clock and deferred script replacement coordinator. */
class ScriptScheduler {
public:
	class Loader {
	public:
		/** Allow destruction through the script-loader interface. */
		virtual ~Loader() {}
		/** Replace a thread's image with the named script, returning false on failure. */
		virtual bool loadScript(ScriptRuntime &thread, const Common::String &name) = 0;
		/** Request application shutdown from a script command. */
		virtual void quit() = 0;
	};
	/** Create a scheduler using a script loader. */
	explicit ScriptScheduler(Loader &loader) : _loader(loader) {}
	/** Detach all threads from this scheduler. */
	~ScriptScheduler();
	/** Attach or rename a thread in the scheduling order. */
	void attach(ScriptRuntime &thread, const Common::String &name);
	/** Remove a thread without destroying it. */
	void detach(ScriptRuntime &thread);
	/** Advance attached threads at a millisecond timestamp and handle deferred requests. */
	bool tick(uint32 time);
	/** Reset idle timers on all attached threads. */
	void resetIdle();
	/** Suspend ordinary execution on all attached threads. */
	void suspend();
	/** Resume ordinary execution on all attached threads. */
	void resume();
	/** Return whether a scheduler tick is currently on the stack. */
	bool isTicking() const { return _ticking; }

private:
	struct Thread {
		/** Thread attached to this scheduler. */
		ScriptRuntime *runtime = nullptr;
		/** Script name used for deferred calls and replacement. */
		Common::String name;
	};
	/** Application interface for script loading and shutdown. */
	Loader &_loader;
	/** Attached thread pointers and script names in scheduling order. */
	Common::Array<Thread> _threads;
	/** Pending script names retained while calls are resolved. */
	Common::Array<Common::String> _calledScripts;
	/** Millisecond timestamp of the most recent timer-aging pass. */
	uint32 _lastAge = 0;
	/** Millisecond deadline that postpones all attached threads. */
	uint32 _sleepUntil = 0;
	/** Whether a tick is running, preventing recursive scheduler entry. */
	bool _ticking = false;
	/** Prevent accidental copies of this stateful object. */
	ScriptScheduler(const ScriptScheduler &) = delete;
	/** Prevent accidental copies of this stateful object. */
	ScriptScheduler &operator=(const ScriptScheduler &) = delete;
	friend class ScriptRuntime;
};

} // End of namespace Core
} // End of namespace Trinket

#endif
