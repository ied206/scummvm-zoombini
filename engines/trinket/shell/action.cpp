/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/shell/action.h"

#include "trinket/shell/random.h"

namespace Trinket {
namespace Shell {

/** References an existing script action without cloning its authored children. */
class ReferencedAction : public Action, private Action::Listener, private Core::ScriptableObject::Observer {
public:
	ReferencedAction(ActionSystem &system, const Core::ScriptValue &target, bool interruptible) : Action(system, interruptible), _target(target) {}
	~ReferencedAction() override {
		if (_action) {
			_action->removeListener(*this);
			_action->removeObserver(*this);
		}
	}
	void start() override {
		if (_action) {
			_action->removeListener(*this);
			_action->removeObserver(*this);
		}
		_action = _system.resolveAction(_target);
		if (_action) {
			_action->addObserver(*this);
			_action->setListener(this);
			_action->start();
		} else {
			finished();
		}
	}
	void halt() override {
		Action *action = _system.resolveAction(_target);
		if (action)
			action->halt();
	}
	void pause() override {
		Action *action = _system.resolveAction(_target);
		if (action)
			action->pause();
	}
	void resume() override {
		Action *action = _system.resolveAction(_target);
		if (action)
			action->resume();
	}
	Core::ScriptableObject *clone() const override { return new ReferencedAction(_system, _target, isInterruptible()); }

private:
	Core::ScriptValue _target;
	Action *_action = nullptr;
	void actionFinished(Action &) override { finished(); }
	void objectDestroyed(Core::ScriptableObject &) override { _action = nullptr; }
};

ActionSystem::Scope::~Scope() {
	_system._depth -= 1;
	if (!_system._depth)
		_system.dispose();
}

ActionSystem::~ActionSystem() {
	dispose();
}

Core::ScriptableObject *ActionSystem::ContainerFactory::create(const Core::ScriptArguments &arguments) {
	if (2 < arguments.size())
		return nullptr;
	int repeat = arguments.empty() ? 1 : arguments[0].integer();
	bool interruptible = _parallel ? repeat != 0 : arguments.size() < 2 || arguments[1].integer() != 0;
	return new ActionContainer(_system, _parallel, interruptible, repeat);
}

Core::ScriptableObject *ActionSystem::resolve(const Core::ScriptValue &value) const {
	Core::ScriptValue reference = value;
	if (reference.kind() != Core::ScriptValue::kObject)
		reference = _runtime.variable(value.text());
	return reference.kind() == Core::ScriptValue::kObject ? _runtime.objects().resolve(reference.reference()) : nullptr;
}

Core::ScriptableObject *ActionSystem::resolve(const Core::ScriptValue &value, const Common::String &className) const {
	Core::ScriptValue reference = value;
	if (reference.kind() != Core::ScriptValue::kObject)
		reference = _runtime.variable(value.text());
	return reference.kind() == Core::ScriptValue::kObject ? _runtime.objects().resolve(reference.reference(), className) : nullptr;
}

Action *ActionSystem::resolveAction(const Core::ScriptValue &value) const {
	Core::ScriptableObject *object = resolve(value);
	for (uint i = 0; i < _actions.size(); i++) {
		if (_actions[i] && _actions[i] == object)
			return _actions[i];
	}
	return nullptr;
}

Action *ActionSystem::create(const Core::ScriptArguments &arguments, bool interruptible) {
	if (arguments.empty())
		return nullptr;
	Core::ScriptableObject *object = resolve(arguments[0]);
	if (object) {
		Action *action = resolveAction(arguments[0]);
		if (!action)
			return nullptr;
		return new ReferencedAction(*this, arguments[0], action->isInterruptible());
	}
	Core::ScriptArguments parameters;
	for (uint i = 1; i < arguments.size(); i++)
		parameters.push_back(arguments[i]);
	if (arguments[0].text() == "DelayAction") {
		if (parameters.empty() || 2 < parameters.size())
			return nullptr;
		if (1 < parameters.size())
			interruptible = parameters[1].integer() != 0;
		return new DelayAction(*this, static_cast<uint32>(parameters[0].integer()), interruptible);
	}
	if (arguments[0].text() == "PropertyAction") {
		if (parameters.size() < 3 || !resolve(parameters[0]))
			return nullptr;
		if (3 < parameters.size())
			interruptible = parameters[3].integer() != 0;
		Core::ScriptArguments values;
		values.push_back(parameters[2]);
		for (uint i = 4; i < parameters.size(); i++)
			values.push_back(parameters[i]);
		return new PropertyAction(*this, parameters[0], parameters[1].text(), values, interruptible);
	}
	return _factory.createAction(arguments[0].text(), parameters, interruptible);
}

void PropertyAction::execute() {
	Core::ScriptableObject *target = _system.resolve(_target);
	if (!target || _values.empty())
		return;
	Core::ScriptArguments arguments;
	for (uint i = 0; i + 1 < _values.size(); i++)
		arguments.push_back(_values[i]);
	target->setCallTime(_system.currentTime());
	target->set(_property, arguments, _values.back());
}

void PropertyAction::start() {
	ActionSystem::Scope scope(_system);
	Action::start();
	execute();
	if (!isRetired())
		finished();
}

void PropertyAction::halt() {
	ActionSystem::Scope scope(_system);
	Action::halt();
	execute();
}

void ActionSystem::attach(Action &action) {
	_actions.push_back(&action);
}

void ActionSystem::detach(Action &action) {
	for (uint i = 0; i < _actions.size(); i++) {
		if (_actions[i] == &action) {
			_actions[i] = nullptr;
			break;
		}
	}
}

void ActionSystem::retire(Action *action) {
	if (!action || action->_retired)
		return;
	action->_retired = true;
	action->setListener(nullptr);
	action->Action::halt();
	detach(*action);
	_retired.push_back(action);
	if (!_depth)
		dispose();
}

void ActionSystem::dispose() {
	while (!_retired.empty()) {
		Action *action = _retired.back();
		_retired.pop_back();
		delete action;
	}
}

void ActionSystem::update(uint32 time) {
	Scope scope(*this);
	_time = time;
	uint count = _actions.size();
	for (uint i = 0; i < count; i++) {
		if (_actions[i])
			_actions[i]->update(time);
	}
	for (uint i = _actions.size(); i; i--) {
		if (!_actions[i - 1])
			_actions.remove_at(i - 1);
	}
}

Action::Action(ActionSystem &system, bool interruptible) : _system(system), _interruptible(interruptible) {
	addEvent("finished");
	_system.attach(*this);
}

Action::~Action() {
	_system.detach(*this);
}

bool Action::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property == "isInterruptible")
		result = Core::ScriptValue(static_cast<int32>(_interruptible));
	else if (property == "isActive")
		result = Core::ScriptValue(static_cast<int32>(_active));
	else
		return ScriptableObject::get(property, arguments, result);
	return true;
}

bool Action::play(const Common::String &method, const Core::ScriptArguments &, Core::ScriptValue &) {
	ActionSystem::Scope scope(_system);
	if (method == "start")
		start();
	else if (method == "halt")
		halt();
	else if (method == "pause")
		pause();
	else if (method == "resume")
		resume();
	else
		return false;
	return true;
}

void Action::start() {
	_active = true;
	_paused = _updated = false;
}

void Action::halt() {
	_active = false;
}

void Action::pause() { _paused = true; }

void Action::resume() { _paused = false; }

void Action::update(uint32 time) {
	if (!_active || _paused || (_updated && _lastUpdate == time))
		return;
	_updated = true;
	_lastUpdate = time;
	updateAction(time);
}

void Action::finished() {
	ActionSystem::Scope scope(_system);
	_active = false;
	if (_listener)
		_listener->actionFinished(*this);
	if (_retired)
		return;
	complete();
	if (!_retired)
		emitEvent("finished");
}

void DelayAction::start() {
	Action::start();
	_remaining = _duration;
	_started = _system.currentTime();
}

void DelayAction::pause() {
	if (!_paused) {
		uint32 elapsed = _system.currentTime() - _started;
		_remaining = elapsed < _remaining ? _remaining - elapsed : 0;
		Action::pause();
	}
}

void DelayAction::resume() {
	_started = _system.currentTime();
	Action::resume();
}

void DelayAction::updateAction(uint32 time) {
	if (_remaining <= time - _started)
		finished();
}

void RandomDelayAction::start() {
	ActionSystem::Scope scope(_system);
	Action::start();
	if (_minimum == _maximum) {
		finished();
		return;
	}
	_remaining = _minimum + _random.draw() % (_maximum - _minimum);
	_started = _system.currentTime();
}

void RandomDelayAction::pause() {
	if (!_paused && isActive()) {
		_remaining += _started - _system.currentTime();
		Action::pause();
	}
}

void RandomDelayAction::resume() {
	if (_paused && isActive()) {
		_started = _system.currentTime();
		Action::resume();
	}
}

void RandomDelayAction::updateAction(uint32 time) {
	if (_remaining <= time - _started)
		finished();
}

ActionContainer::ActionContainer(ActionSystem &system, bool parallel, bool interruptible, int repeat) : Action(system, interruptible), _parallel(parallel), _repeat(repeat ? repeat : 1) {
}

ActionContainer::~ActionContainer() {
	ActionSystem::Scope scope(_system);
	for (uint i = 0; i < _children.size(); i++)
		_system.retire(_children[i]);
}

bool ActionContainer::add(Action *action) {
	if (!action)
		return false;
	action->setListener(this);
	_children.push_back(action);
	if (_parallel && isActive() && !action->isActive())
		_finished += 1;
	return true;
}

void ActionContainer::clear() {
	ActionSystem::Scope scope(_system);
	halt();
	if (!_parallel && _current < _children.size())
		return;
	for (uint i = 0; i < _children.size(); i++)
		_system.retire(_children[i]);
	_children.clear();
	_current = _finished = 0;
}

void ActionContainer::start() {
	ActionSystem::Scope scope(_system);
	if (isActive())
		return;
	Action::start();
	_current = _finished = _repeats = 0;
	if (_children.empty()) {
		finished();
	} else if (_parallel) {
		uint count = _children.size();
		for (uint i = 0; i < count && i < _children.size() && !isRetired(); i++)
			_children[i]->start();
	} else {
		_children[0]->start();
	}
}

void ActionContainer::halt() {
	ActionSystem::Scope scope(_system);
	if (_parallel) {
		_finished = 0;
		for (uint i = 0; i < _children.size(); i++) {
			if (_children[i]->isInterruptible()) {
				_children[i]->halt();
				_finished += 1;
			}
		}
		if (_finished == _children.size())
			Action::halt();
	} else {
		bool halted = false;
		while (_current < _children.size() && _children[_current]->isInterruptible()) {
			_children[_current]->halt();
			_current += 1;
			halted = true;
		}
		if (_children.size() <= _current)
			Action::halt();
		else if (halted)
			_children[_current]->start();
	}
}

void ActionContainer::pause() {
	Action::pause();
	if (_parallel) {
		for (uint i = 0; i < _children.size(); i++)
			_children[i]->pause();
	} else if (_current < _children.size()) {
		_children[_current]->pause();
	}
}

void ActionContainer::resume() {
	Action::resume();
	if (_parallel) {
		for (uint i = 0; i < _children.size(); i++)
			_children[i]->resume();
	} else if (_current < _children.size()) {
		_children[_current]->resume();
	}
}

void ActionContainer::actionFinished(Action &) {
	if (!isActive())
		return;
	if (_parallel) {
		_finished += 1;
		if (_finished == _children.size())
			finished();
	} else {
		_current += 1;
		if (_children.size() <= _current) {
			_repeats += 1;
			if (_repeat != -1 && static_cast<uint>(_repeat) <= _repeats) {
				finished();
				return;
			}
			_current = 0;
		}
		_children[_current]->start();
	}
}

void ActionContainer::updateAction(uint32 time) {
	if (_parallel) {
		uint count = _children.size();
		for (uint i = 0; i < count && i < _children.size(); i++)
			_children[i]->update(time);
	} else if (_current < _children.size()) {
		_children[_current]->update(time);
	}
}

bool ActionContainer::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property == "actionCount")
		result = Core::ScriptValue(static_cast<int32>(_children.size()));
	else if (property == "repeatCount" && !_parallel)
		result = Core::ScriptValue(_repeat);
	else
		return Action::get(property, arguments, result);
	return true;
}

bool ActionContainer::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property != "repeatCount" || _parallel)
		return ScriptableObject::set(property, arguments, value);
	_repeat = value.integer() ? value.integer() : 1;
	return true;
}

bool ActionContainer::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	ActionSystem::Scope scope(_system);
	if (method == "add")
		return add(_system.create(arguments, isInterruptible()));
	if (method == "clear" && !_parallel) {
		clear();
		return true;
	}
	return Action::play(method, arguments, result);
}

Core::ScriptableObject *ActionContainer::clone() const {
	ActionContainer *copy = new ActionContainer(_system, _parallel, isInterruptible(), _repeat);
	for (uint i = 0; i < _children.size(); i++) {
		Action *child = static_cast<Action *>(_children[i]->clone());
		if (!copy->add(child)) {
			delete copy;
			return nullptr;
		}
	}
	return copy;
}

} // namespace Shell
} // namespace Trinket
