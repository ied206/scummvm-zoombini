/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_SHELL_ACTION_H
#define TRINKET_SHELL_ACTION_H

#include "trinket/core/script_runtime.h"

namespace Trinket {
namespace Shell {

class Action;
class ActionContainer;
class Random;

/** Action factories and deferred child disposal belong to one script session. */
class ActionSystem {
public:
	class Factory {
	public:
		/** Allow destruction through the action-factory interface. */
		virtual ~Factory() {}
		/** Construct the named game action from its script arguments. */
		virtual Action *createAction(const Common::String &name, const Core::ScriptArguments &sargs, bool interruptible) = 0;
	};
	class Scope {
	public:
		/** Defer action deletion until the outermost action callback returns. */
		explicit Scope(ActionSystem &system) : _system(system) { _system._depth += 1; }
		/** Release the callback scope and delete retired actions when it is safe. */
		~Scope();

	private:
		/** Action dispatcher whose callback nesting is guarded by this scope. */
		ActionSystem &_system;
	};
	class ContainerFactory : public Core::ObjectRegistry::Factory {
	public:
		/** Configure creation of sequential or parallel action containers. */
		ContainerFactory(ActionSystem &system, bool parallel) : _system(system), _parallel(parallel) {}
		/** Create an action container from its script arguments. */
		Core::ScriptableObject *create(const Core::ScriptArguments &sargs) override;

	private:
		/** Dispatcher used to create and retire the container's actions. */
		ActionSystem &_system;
		/** Whether the factory creates parallel composites instead of sequential queues. */
		bool _parallel;
	};
	/** Connect script object lookup to the game's action factory. */
	ActionSystem(Core::ScriptRuntime &runtime, Factory &factory) : _runtime(runtime), _factory(factory) {}
	/** Delete remaining actions after detaching their callbacks. */
	~ActionSystem();
	/** Construct a built-in or game-specific action from script arguments. */
	Action *create(const Core::ScriptArguments &sargs, bool interruptible);
	/** Queue an action for deletion after active callbacks have finished. */
	void retire(Action *action);
	/** Advance registered actions using the supplied millisecond timestamp. */
	void update(uint32 time);
	/** Set the timestamp used when actions start outside the update pass. */
	void setTime(uint32 time) { _time = time; }
	/** Return the current action timestamp in milliseconds. */
	uint32 currentTime() const { return _time; }
	/** Resolve a script object reference or name, optionally checking its class. */
	Core::ScriptableObject *resolve(const Core::ScriptValue &value) const;
	/** Resolve a script object reference or name, optionally checking its class. */
	Core::ScriptableObject *resolve(const Core::ScriptValue &value, const Common::String &className) const;
	/** Resolve a script value to a registered action, or return null. */
	Action *resolveAction(const Core::ScriptValue &value) const;

private:
	/** Script identifier for delay Action. */
	static constexpr const char *kDelayActionName = "DelayAction";
	/** Script identifier for property Action. */
	static constexpr const char *kPropertyActionName = "PropertyAction";

	/** Script runtime supplying object references and named values. */
	Core::ScriptRuntime &_runtime;
	/** Game-specific factory used after checking built-in action classes. */
	Factory &_factory;
	/** Registered actions advanced by each update pass. */
	Common::Array<Action *> _actions;
	/** Actions waiting for callback nesting to unwind before deletion. */
	Common::Array<Action *> _retired;
	/** Number of active scopes preventing immediate action deletion. */
	uint _depth = 0;
	/** Current action timestamp in milliseconds. */
	uint32 _time = 0;
	/** Register an action for updates. */
	void attach(Action &action);
	/** Remove an action from the update list. */
	void detach(Action &action);
	/** Delete retired actions when no callback scope is active. */
	void dispose();
	friend class Action;
};

/** Native action state; halt clears activity without sending finished. */
class Action : public Core::ScriptableObject {
public:
	class Listener {
	public:
		/** Allow destruction through the action-listener interface. */
		virtual ~Listener() {}
		/** Notify the container or caller that an action completed. */
		virtual void actionFinished(Action &action) = 0;
	};
	/** Register an inactive action with its interruption policy. */
	Action(ActionSystem &system, bool interruptible);
	/** Detach the action from its dispatcher. */
	~Action() override;
	/** Defer deletion until the current action callback has returned. */
	void dispose() override { _system.retire(this); }
	/** Read the action's active state or interruption policy. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Dispatch script requests to start, halt, pause or resume the action. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Mark the action active so later updates can advance it. */
	virtual void start();
	/** Stop the action without emitting its finished event. */
	virtual void halt();
	/** Suspend advancement while retaining the active operation. */
	virtual void pause();
	/** Continue advancement after a pause. */
	virtual void resume();
	/** Advance an active action once for the supplied millisecond timestamp. */
	void update(uint32 time);
	/** Return whether the action has started and has not stopped or completed. */
	bool isActive() const { return _active; }
	/** Return whether a containing action may halt this operation. */
	bool isInterruptible() const { return _interruptible; }
	/** Set the listener notified when the action finishes. */
	void setListener(Listener *listener) { _listener = listener; }
	/** Detach the supplied listener if it is still registered. */
	void removeListener(Listener &listener) {
		if (_listener == &listener)
			_listener = nullptr;
	}

protected:
	/** Dispatcher that updates the action and defers its deletion during callbacks. */
	ActionSystem &_system;
	/** Whether the action is waiting for updates or completion. */
	bool _active = false;
	/** Whether updates are temporarily suspended. */
	bool _paused = false;
	/** Stop activity and notify the listener and script finished event. */
	void finished();
	/** Return whether deletion has been requested for this action. */
	bool isRetired() const { return _retired; }
	/** Advance the action-specific operation at the supplied millisecond timestamp. */
	virtual void updateAction(uint32 time) {}

private:
	/** Script event name for finished. */
	static constexpr const char *kEventFinished = "finished";
	/** Script property name for is Interruptible. */
	static constexpr const char *kPropertyIsInterruptible = "isInterruptible";
	/** Script property name for is Active. */
	static constexpr const char *kPropertyIsActive = "isActive";
	/** Script method name for start. */
	static constexpr const char *kMethodStart = "start";
	/** Script method name for halt. */
	static constexpr const char *kMethodHalt = "halt";
	/** Script method name for pause. */
	static constexpr const char *kMethodPause = "pause";
	/** Script method name for resume. */
	static constexpr const char *kMethodResume = "resume";

	/** Listener notified when the action completes, or null if none is set. */
	Listener *_listener = nullptr;
	/** Whether the operation may be halted by a containing action. */
	bool _interruptible;
	/** Whether the action has already advanced at its recorded update timestamp. */
	bool _updated = false;
	/** Whether deletion is pending after active callbacks return. */
	bool _retired = false;
	/** Millisecond timestamp of the most recent action update. */
	uint32 _lastUpdate = 0;
	friend class ActionSystem;
};

class DelayAction : public Action {
public:
	/** Configure a fixed millisecond delay and its interruption policy. */
	DelayAction(ActionSystem &system, uint32 duration, bool interruptible) : Action(system, interruptible), _duration(duration) {}
	/** Begin a fresh delay using the configured duration. */
	void start() override;
	/** Preserve the remaining delay while updates are paused. */
	void pause() override;
	/** Restart timing from the preserved remaining delay. */
	void resume() override;
	/** Create an inactive copy with the same configured delay. */
	Core::ScriptableObject *clone() const override { return new DelayAction(_system, _duration, isInterruptible()); }

private:
	/** Configured delay in milliseconds. */
	uint32 _duration;
	/** Milliseconds still to wait after a pause or restart. */
	uint32 _remaining = 0;
	/** Millisecond timestamp when the current delay interval began. */
	uint32 _started = 0;
	/** Complete the action when the remaining delay has elapsed. */
	void updateAction(uint32 time) override;
};

/** A retained property assignment, applied on start and on halt. */
class PropertyAction : public Action {
public:
	/** Retain a target, property name and argument list for a deferred assignment. */
	PropertyAction(ActionSystem &system, const Core::ScriptValue &target, const Common::String &property,
				   const Core::ScriptArguments &values, bool interruptible) : Action(system, interruptible), _target(target), _property(property), _values(values) {}
	/** Apply the retained property assignment and finish the action. */
	void start() override;
	/** Apply the retained assignment before halting the action. */
	void halt() override;
	/** Copy the retained property assignment into a new inactive action. */
	Core::ScriptableObject *clone() const override { return new PropertyAction(_system, _target, _property, _values, isInterruptible()); }

private:
	/** Script value resolving the object whose property will be assigned. */
	Core::ScriptValue _target;
	/** Name of the target object's script property. */
	Common::String _property;
	/** Property arguments followed by the value to assign. */
	Core::ScriptArguments _values;
	/** Resolve the target and apply the retained property assignment. */
	void execute();
};

/** Shared CRT draw at each start, with the native exclusive upper bound. */
class RandomDelayAction : public Action {
public:
	/** Configure a randomly selected millisecond delay between the supplied limits. */
	RandomDelayAction(ActionSystem &system, Random &random, uint32 minimum, uint32 maximum, bool interruptible) : Action(system, interruptible), _random(random), _minimum(minimum), _maximum(maximum) {}
	/** Draw a new delay with an exclusive upper bound and begin waiting. */
	void start() override;
	/** Preserve the unelapsed part of the selected delay. */
	void pause() override;
	/** Continue waiting for the preserved part of the delay. */
	void resume() override;
	/** Create an inactive action with the same random source and delay limits. */
	Core::ScriptableObject *clone() const override { return new RandomDelayAction(_system, _random, _minimum, _maximum, isInterruptible()); }

private:
	/** Shared gameplay random stream used once at each start. */
	Random &_random;
	/** Inclusive minimum delay in milliseconds. */
	uint32 _minimum;
	/** Exclusive maximum delay in milliseconds. */
	uint32 _maximum;
	/** Milliseconds left in the selected delay. */
	uint32 _remaining = 0;
	/** Millisecond timestamp when the current wait interval began. */
	uint32 _started = 0;
	/** Complete the action when the selected delay has elapsed. */
	void updateAction(uint32 time) override;
};

/** Run child actions sequentially for RQueue or concurrently for RCompositeAction. */
class ActionContainer : public Action, private Action::Listener {
public:
	/** Configure a sequential queue or parallel composite and its repeat count. */
	ActionContainer(ActionSystem &system, bool parallel, bool interruptible = true, int repeat = 1);
	/** Detach child listeners and dispose of the contained actions. */
	~ActionContainer() override;
	/** Append an action to the container, rejecting a null or invalid child. */
	bool add(Action *action);
	/** Stop and dispose of all contained actions. */
	void clear();
	/** Start the child actions according to the queue or parallel policy. */
	void start() override;
	/** Halt interruptible children while preserving noninterruptible work. */
	void halt() override;
	/** Pause active children and container advancement. */
	void pause() override;
	/** Resume active children and container advancement. */
	void resume() override;
	/** Read the child count, repeat count or inherited action properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Change the repeat count through the script property interface. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Handle scripted child insertion and clearing, or an inherited action method. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Create another container with copies of its child actions. */
	Core::ScriptableObject *clone() const override;

private:
	/** Script property name for action Count. */
	static constexpr const char *kPropertyActionCount = "actionCount";
	/** Script property name for repeat Count. */
	static constexpr const char *kPropertyRepeatCount = "repeatCount";
	/** Script method name for add. */
	static constexpr const char *kMethodAdd = "add";
	/** Script method name for clear. */
	static constexpr const char *kMethodClear = "clear";

	/** Contained actions in their execution order. */
	Common::Array<Action *> _children;
	/** Whether children run together instead of one at a time. */
	bool _parallel;
	/** Configured number of queue passes; minus one repeats indefinitely. */
	int _repeat;
	/** Index of the child currently running in a sequential queue. */
	uint _current = 0;
	/** Number of parallel children that have completed or halted. */
	uint _finished = 0;
	/** Number of complete sequential passes already performed. */
	uint _repeats = 0;
	/** Advance the queue or finish the composite after a child completes. */
	void actionFinished(Action &action) override;
	/** Advance active children using the container's current timestamp. */
	void updateAction(uint32 time) override;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
