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
		virtual ~Factory() {}
		virtual Action *createAction(const Common::String &name, const Core::ScriptArguments &arguments, bool interruptible) = 0;
	};
	class Scope {
	public:
		explicit Scope(ActionSystem &system) : _system(system) { _system._depth += 1; }
		~Scope();

	private:
		ActionSystem &_system;
	};
	class ContainerFactory : public Core::ObjectRegistry::Factory {
	public:
		ContainerFactory(ActionSystem &system, bool parallel) : _system(system), _parallel(parallel) {}
		Core::ScriptableObject *create(const Core::ScriptArguments &arguments) override;

	private:
		ActionSystem &_system;
		bool _parallel;
	};
	ActionSystem(Core::ScriptRuntime &runtime, Factory &factory) : _runtime(runtime), _factory(factory) {}
	~ActionSystem();
	Action *create(const Core::ScriptArguments &arguments, bool interruptible);
	void retire(Action *action);
	void update(uint32 time);
	void setTime(uint32 time) { _time = time; }
	uint32 currentTime() const { return _time; }
	Core::ScriptableObject *resolve(const Core::ScriptValue &value) const;
	Core::ScriptableObject *resolve(const Core::ScriptValue &value, const Common::String &className) const;
	Action *resolveAction(const Core::ScriptValue &value) const;

private:
	Core::ScriptRuntime &_runtime;
	Factory &_factory;
	Common::Array<Action *> _actions, _retired;
	uint _depth = 0;
	uint32 _time = 0;
	void attach(Action &action);
	void detach(Action &action);
	void dispose();
	friend class Action;
};

/** Native action state; halt clears activity without sending finished. */
class Action : public Core::ScriptableObject {
public:
	class Listener {
	public:
		virtual ~Listener() {}
		virtual void actionFinished(Action &action) = 0;
	};
	Action(ActionSystem &system, bool interruptible);
	~Action() override;
	void dispose() override { _system.retire(this); }
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	virtual void start();
	virtual void halt();
	virtual void pause();
	virtual void resume();
	void update(uint32 time);
	bool isActive() const { return _active; }
	bool isInterruptible() const { return _interruptible; }
	void setListener(Listener *listener) { _listener = listener; }
	void removeListener(Listener &listener) {
		if (_listener == &listener)
			_listener = nullptr;
	}

protected:
	ActionSystem &_system;
	bool _active = false, _paused = false;
	void finished();
	bool isRetired() const { return _retired; }
	virtual void updateAction(uint32 time) {}

private:
	Listener *_listener = nullptr;
	bool _interruptible, _updated = false, _retired = false;
	uint32 _lastUpdate = 0;
	friend class ActionSystem;
};

class DelayAction : public Action {
public:
	DelayAction(ActionSystem &system, uint32 duration, bool interruptible) : Action(system, interruptible), _duration(duration) {}
	void start() override;
	void pause() override;
	void resume() override;
	Core::ScriptableObject *clone() const override { return new DelayAction(_system, _duration, isInterruptible()); }

private:
	uint32 _duration, _remaining = 0, _started = 0;
	void updateAction(uint32 time) override;
};

/** A retained property assignment, applied on start and on halt. */
class PropertyAction : public Action {
public:
	PropertyAction(ActionSystem &system, const Core::ScriptValue &target, const Common::String &property,
				   const Core::ScriptArguments &values, bool interruptible) : Action(system, interruptible), _target(target), _property(property), _values(values) {}
	void start() override;
	void halt() override;
	Core::ScriptableObject *clone() const override { return new PropertyAction(_system, _target, _property, _values, isInterruptible()); }

private:
	Core::ScriptValue _target;
	Common::String _property;
	Core::ScriptArguments _values;
	void execute();
};

/** Shared CRT draw at each start, with the native exclusive upper bound. */
class RandomDelayAction : public Action {
public:
	RandomDelayAction(ActionSystem &system, Random &random, uint32 minimum, uint32 maximum, bool interruptible) : Action(system, interruptible), _random(random), _minimum(minimum), _maximum(maximum) {}
	void start() override;
	void pause() override;
	void resume() override;
	Core::ScriptableObject *clone() const override { return new RandomDelayAction(_system, _random, _minimum, _maximum, isInterruptible()); }

private:
	Random &_random;
	uint32 _minimum, _maximum, _remaining = 0, _started = 0;
	void updateAction(uint32 time) override;
};

/** Sequential RQueue or parallel RCompositeAction, owning its child actions. */
class ActionContainer : public Action, private Action::Listener {
public:
	ActionContainer(ActionSystem &system, bool parallel, bool interruptible = true, int repeat = 1);
	~ActionContainer() override;
	bool add(Action *action);
	void clear();
	void start() override;
	void halt() override;
	void pause() override;
	void resume() override;
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	Core::ScriptableObject *clone() const override;

private:
	Common::Array<Action *> _children;
	bool _parallel;
	int _repeat;
	uint _current = 0, _finished = 0, _repeats = 0;
	void actionFinished(Action &action) override;
	void updateAction(uint32 time) override;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
