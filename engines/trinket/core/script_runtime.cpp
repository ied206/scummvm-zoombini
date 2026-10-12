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

#include "trinket/core/script_runtime.h"

#include "common/debug.h"

namespace Trinket {
namespace Core {

ScriptRuntime::~ScriptRuntime() {
	clear();
	if (_scheduler)
		_scheduler->detach(*this);
}

void ScriptRuntime::clear() {
	unblock();
	for (uint i = 0; i < _bindings.size(); i++) {
		ScriptableObject *object = _objects.resolve(_bindings[i].reference);
		if (object)
			object->bindEvent(_bindings[i].event, nullptr);
	}
	_bindings.clear();
	_events.clear();
	_objects.clear();
	_values.clear();
	_objectNames.clear();
	_procedures.clear();
	_frames.clear();
	_loops.clear();
	_functions = nullptr;
	_pc = -1;
	_executing = false;
	_scheduled = _suspended = _yielded = _inInterrupt = false;
	_eventsEnabled = _condition = true;
	_sleepUntil = 0;
	_idle = Timer();
	_schedule = Timer();
	_interrupt = Interrupt();
	_request = kNoRequest;
	_requestVariable = -1;
	_error.clear();
}

bool ScriptRuntime::load(Common::SeekableReadStream &stream) {
	clear();
	if (!_image.load(stream))
		return fail("Invalid compiled MPS image");
	const Common::Array<Script::Variable> &variables = _image.variables();
	for (uint i = 0; i < variables.size(); i++) {
		const Script::Variable &value = variables[i];
		if (value.classification != 0 || value.name.empty())
			continue;
		if (value.type == 3)
			_values[value.name] = ScriptValue(value.integer);
		else if (value.type == 4)
			_values[value.name] = ScriptValue(atof(value.real.c_str()));
		else if (value.type == 2)
			_values[value.name] = ScriptValue(value.extra);
		if (value.extra.hasPrefix("+") && value.type == 3 &&
			0 <= value.integer && static_cast<uint>(value.integer) < _image.instructions().size()) {
			_procedures[value.name] = value.integer;
			_values[value.name] = ScriptValue(value.extra);
		}
	}
	initializeConstants();
	return _error.empty();
}

void ScriptRuntime::initializeConstants() {
	// Materialize literal constants and earlier constant aliases without executing scene setup.
	const Common::Array<Script::Instruction> &instructions = _image.instructions();
	const Common::Array<Script::Variable> &variables = _image.variables();
	for (uint i = 0; i < instructions.size(); i++) {
		if (instructions[i].opcode != kAssign18)
			continue;
		Common::Array<int16> indices;
		if (!operands(instructions[i].operand, indices))
			return;
		if (indices.size() != 2)
			continue;
		const Script::Variable &target = variables[indices[0]];
		const Script::Variable &source = variables[indices[1]];
		bool fixedTarget = target.classification == 0;
		if (target.classification == 2) {
			fixedTarget = 0 <= target.count && static_cast<uint>(target.count) <= target.array.size();
			for (int segment = 0; fixedTarget && segment < target.count; segment++) {
				int index = target.array[segment];
				fixedTarget = 0 <= index && static_cast<uint>(index) < variables.size() && variables[index].classification == 1;
			}
		}
		bool constantSource = source.classification == 1;
		if (source.classification == 0 && source.name.hasPrefix("k") && _values.contains(source.name))
			constantSource = true;
		if (fixedTarget && target.name.hasPrefix("k") && constantSource) {
			ScriptValue value;
			Common::String name;
			if (!evaluate(indices[1], value) || !variableName(indices[0], name))
				return;
			_values[name] = value;
		}
	}
}

ScriptValue ScriptRuntime::variable(const Common::String &name) const {
	auto found = _values.find(name);
	return found == _values.end() ? ScriptValue() : found->_value;
}

void ScriptRuntime::setVariable(const Common::String &name, const ScriptValue &value) {
	_values[name] = value;
	if (value.kind() == ScriptValue::kObject && name != "SYSTEMWIN" && name != "RECEIVER" && name != "RESULT" &&
		!_objectNames.contains(value.reference()))
		_objectNames[value.reference()] = name;
}

void ScriptRuntime::bindObject(const Common::String &name, ScriptableObject &object) {
	setVariable(name, ScriptValue::object(_objects.bind(object)));
}

bool ScriptRuntime::bindEvent(ScriptableObject &object, const Common::String &event, const Common::String &name) {
	if (procedure(name) < 0)
		return fail("Unknown event procedure: " + name);
	EventLink link(*this, name);
	if (!object.bindEvent(event, &link))
		return fail("Unknown object event: " + event);
	uint32 reference = _objects.bind(object);
	for (uint i = 0; i < _bindings.size(); i++) {
		if (_bindings[i].reference == reference && _bindings[i].event == event)
			return true;
	}
	Binding binding;
	binding.reference = reference;
	binding.event = event;
	_bindings.push_back(binding);
	return true;
}

void ScriptRuntime::EventLink::execute(ScriptableObject &source) {
	_runtime.objectEvent(source, _procedure);
}

void ScriptRuntime::objectEvent(ScriptableObject &source, const Common::String &name) {
	if (!_eventsEnabled || _blocked)
		return;
	PendingEvent event;
	event.reference = _objects.bind(source);
	event.procedure = name;
	_events.push_back(event);
	if (!_executing && !_scheduled)
		runProcedure(Common::String());
}

void ScriptRuntime::CompletionLink::execute(ScriptableObject &) {
	_runtime.unblock();
}

void ScriptRuntime::unblock() {
	ScriptableObject *object = _objects.resolve(_blocked);
	_blocked = 0;
	if (object) {
		object->bindCompletion(nullptr);
		object->removeObserver(*this);
	}
}

void ScriptRuntime::objectDestroyed(ScriptableObject &) {
	_blocked = 0;
}

bool ScriptRuntime::start() {
	if (_image.instructions().empty())
		return fail("Cannot start an empty image");
	unblock();
	_frames.clear();
	_loops.clear();
	_events.clear();
	_interrupt = Interrupt();
	_pc = 0;
	_scheduled = true;
	_suspended = _inInterrupt = _yielded = false;
	_eventsEnabled = _condition = true;
	_request = kNoRequest;
	_error.clear();
	return true;
}

void ScriptRuntime::leaveInterrupt() {
	_pc = _interrupt.pc;
	_condition = _interrupt.condition;
	_suspended = _interrupt.suspended;
	_frames = _interrupt.frames;
	_interrupt = Interrupt();
	_inInterrupt = false;
}

bool ScriptRuntime::enterInterrupt(const PendingEvent &event) {
	int destination = procedure(event.procedure);
	if (destination < 0)
		return fail("Unknown interrupt procedure: " + event.procedure);
	if (_inInterrupt)
		leaveInterrupt();
	_interrupt.pc = _pc;
	_interrupt.condition = _condition;
	_interrupt.suspended = _suspended;
	_interrupt.frames = _frames;
	_inInterrupt = true;
	_suspended = false;
	_pc = destination + 1;
	if (event.reference)
		setVariable("RECEIVER", ScriptValue::object(event.reference));
	else if (!event.name.empty())
		setVariable("RECEIVER", ScriptValue(event.name));
	if (!event.arguments.empty()) {
		Common::Array<int16> indices;
		if (!operands(_image.instructions()[destination].operand, indices))
			return false;
		for (uint i = 1; i < indices.size() && i <= event.arguments.size(); i++) {
			Common::String name;
			if (!variableName(indices[i], name))
				return false;
			setVariable(name, event.arguments[i - 1]);
		}
	}
	return true;
}

bool ScriptRuntime::dispatchEvents() {
	uint count = 0;
	while (!_events.empty() && !_blocked && _request == kNoRequest) {
		if (1024 <= count)
			return fail("Event dispatch budget exceeded");
		count += 1;
		PendingEvent event = _events.front();
		_events.remove_at(0);
		if (!_eventsEnabled || (event.reference && !_objects.resolve(event.reference)))
			continue;
		if (!enterInterrupt(event))
			return false;
		_yielded = false;
		uint instructions = 0;
		while (_inInterrupt && !_suspended && !_blocked && !_yielded && _request == kNoRequest && _error.empty()) {
			if (16384 <= instructions)
				return fail("Interrupt instruction budget exceeded");
			instructions += 1;
			if (!step())
				return false;
		}
	}
	return _error.empty();
}

bool ScriptRuntime::age() {
	_idle.elapsed += 1;
	_schedule.elapsed += 1;
	if (!_schedule.procedure.empty() && _schedule.limit < _schedule.elapsed) {
		PendingEvent event;
		event.procedure = _schedule.procedure;
		event.name = "schedule";
		_schedule.procedure.clear();
		_idle.elapsed = _schedule.elapsed = 0;
		if (_eventsEnabled && !_blocked) {
			if (!enterInterrupt(event) || !step())
				return false;
			_yielded = true;
			return true;
		}
	}
	if (!_idle.procedure.empty() && _idle.limit < _idle.elapsed) {
		PendingEvent event;
		event.procedure = _idle.procedure;
		event.name = "idle";
		_idle.elapsed = 0;
		if (_eventsEnabled && !_blocked) {
			if (!enterInterrupt(event) || !step())
				return false;
			_yielded = true;
		}
	}
	return true;
}

bool ScriptRuntime::advance(uint32 time, bool ageTimers) {
	if (_executing)
		return fail("Reentrant thread advance");
	_time = time;
	if (!_scheduled || !_error.empty() || _request != kNoRequest)
		return _error.empty();
	_executing = true;
	_yielded = false;
	uint32 &sleepUntil = _scheduler ? _scheduler->_sleepUntil : _sleepUntil;
	if (!_blocked && sleepUntil && static_cast<int32>(time - sleepUntil) > 0) {
		sleepUntil = 0;
		_suspended = false;
	}
	bool success = !ageTimers || age();
	if (success && !_yielded)
		success = dispatchEvents();
	uint count = 0;
	while (success && 0 <= _pc && !_suspended && !_blocked && !_yielded && _request == kNoRequest) {
		if (16384 <= count) {
			success = fail("Thread instruction budget exceeded");
			break;
		}
		count += 1;
		success = step();
		if (success)
			success = dispatchEvents();
	}
	_executing = false;
	return success;
}

int ScriptRuntime::procedure(const Common::String &name) const {
	Common::String target = name;
	for (uint depth = 0; depth < 128; depth++) {
		auto found = _procedures.find(target);
		if (found != _procedures.end())
			return found->_value;
		if (target.hasPrefix("+")) {
			int address = atoi(target.c_str() + 1);
			const Common::Array<Script::Instruction> &instructions = _image.instructions();
			if (0 <= address && static_cast<uint>(address) < instructions.size() && instructions[address].opcode == kProcedure07)
				return address;
			return -1;
		}
		auto value = _values.find(target);
		if (value == _values.end() || value->_value.text() == target)
			return -1;
		target = value->_value.text();
	}
	return -1;
}

bool ScriptRuntime::fail(const Common::String &message) {
	if (_error.empty())
		_error = Common::String::format("MPS instruction %d: %s", _pc - 1, message.c_str());
	_pc = -1;
	return false;
}

bool ScriptRuntime::runProcedure(const Common::String &name, const ScriptArguments &parameters) {
	if (_scheduled) {
		PendingEvent event;
		event.procedure = name;
		event.arguments = parameters;
		if (!name.empty())
			_events.push_back(event);
		return _executing || advance(_time);
	}
	if (_executing)
		return fail("Reentrant procedure invocation");
	_executing = true;
	_error.clear();
	bool success = name.empty() || execute(name, parameters);
	uint eventCount = 0;
	while (success && !_events.empty()) {
		if (1024 <= eventCount) {
			success = fail("Event dispatch budget exceeded");
			break;
		}
		eventCount += 1;
		PendingEvent event = _events.front();
		_events.remove_at(0);
		if (!_objects.resolve(event.reference))
			continue;
		ScriptValue previous = variable("RECEIVER");
		setVariable("RECEIVER", ScriptValue::object(event.reference));
		success = execute(event.procedure, ScriptArguments());
		setVariable("RECEIVER", previous);
	}
	if (!success)
		_events.clear();
	_executing = false;
	return success;
}

bool ScriptRuntime::execute(const Common::String &name, const ScriptArguments &parameters) {
	_pc = procedure(name);
	if (_pc < 0)
		return fail("Unknown procedure: " + name);
	_frames.clear();
	_loops.clear();
	Frame frame;
	frame.arguments = parameters;
	_frames.push_back(frame);
	uint count = 0;
	_yielded = false;
	while (0 <= _pc && _error.empty() && !_suspended && !_blocked && !_yielded && _request == kNoRequest) {
		if (16384 <= count)
			return fail("Procedure instruction budget exceeded");
		count += 1;
		if (!step())
			return false;
	}
	return _error.empty();
}

bool ScriptRuntime::operands(int start, Common::Array<int16> &indices) {
	const Common::Array<int16> &table = _image.operands();
	if (start < 0 || table.size() <= static_cast<uint>(start))
		return fail("Invalid operand table index");
	for (uint i = start; i < table.size(); i++) {
		int16 index = table[i];
		if (index == -1)
			return true;
		if (index < 0 || _image.variables().size() <= static_cast<uint>(index))
			return fail("Invalid variable index");
		indices.push_back(index);
	}
	return fail("Unterminated operand list");
}

bool ScriptRuntime::variableName(int index, Common::String &name, uint depth) {
	const Common::Array<Script::Variable> &variables = _image.variables();
	if (128 <= depth || index < 0 || variables.size() <= static_cast<uint>(index))
		return fail("Invalid or recursive variable reference");
	const Script::Variable &variable = variables[index];
	if (variable.indirect == -1 && variable.reference)
		return variableName(variable.reference, name, depth + 1);
	name = variable.name;
	if (variable.classification != 2)
		return true;
	uint dot = name.find('.');
	if (dot == Common::String::npos || variable.count < 0 || variable.array.size() < static_cast<uint>(variable.count))
		return fail("Invalid dotted variable");
	name = name.substr(0, dot);
	for (int i = 0; i < variable.count; i++) {
		ScriptValue segment;
		if (!evaluate(variable.array[i], segment, depth + 1))
			return false;
		Common::String text = segment.text();
		if (!text.empty())
			name += "." + text;
	}
	return true;
}

bool ScriptRuntime::evaluate(int index, ScriptValue &value, uint depth) {
	const Common::Array<Script::Variable> &variables = _image.variables();
	if (128 <= depth || index < 0 || variables.size() <= static_cast<uint>(index))
		return fail("Invalid or recursive value");
	const Script::Variable &variable = variables[index];
	if (variable.indirect == -1 && variable.reference)
		return evaluate(variable.reference, value, depth + 1);
	if (variable.classification == 0 || variable.classification == 2) {
		Common::String name;
		if (!variableName(index, name, depth + 1))
			return false;
		auto found = _values.find(name);
		value = found == _values.end() ? ScriptValue(name) : found->_value;
		return true;
	}
	if (variable.classification == 1) {
		if (variable.name.hasPrefix("?")) {
			ScriptArguments arguments;
			arguments.push_back(ScriptValue(0));
			arguments.push_back(ScriptValue(9));
			if (!_functions || !_functions->invoke("RandomNumber", arguments, value))
				return fail("Random literal requires an original RNG binding");
		} else if (variable.type == 3)
			value = ScriptValue(variable.integer);
		else if (variable.type == 4)
			value = ScriptValue(atof(variable.real.c_str()));
		else if (variable.name.hasPrefix("\"") || variable.name.hasPrefix("'")) {
			if (variable.name.size() < 2)
				return fail("Malformed string literal");
			value = ScriptValue(variable.name.substr(1, variable.name.size() - 2));
		} else {
			value = ScriptValue::fromText(variable.name);
		}
		return true;
	}
	if (variable.classification == 3) {
		if (variable.count < 1 || 2 < variable.count || variable.array.size() < static_cast<uint>(variable.count + 1))
			return fail("Invalid string slice operands");
		ScriptValue source, start, length;
		if (!evaluate(variable.array[0], source, depth + 1))
			return false;
		Common::String text = source.text();
		if (variable.count == 2) {
			if (!evaluate(variable.array[2], length, depth + 1))
				return false;
			uint count = MIN<uint>(MAX<int32>(0, length.integer()), text.size());
			if (variable.array[1] == -1) {
				value = ScriptValue(text.substr(text.size() - count));
				return true;
			}
		}
		if (!evaluate(variable.array[1], start, depth + 1))
			return false;
		int32 position = start.integer();
		if (static_cast<int64>(text.size()) < position) {
			value = ScriptValue(Common::String());
			return true;
		}
		uint offset = position < 2 ? 0 : static_cast<uint>(position - 1);
		uint count = text.size() - offset;
		if (variable.count == 2)
			count = MIN<uint>(count, MAX<int32>(0, length.integer()));
		value = ScriptValue(text.substr(offset, count));
		return true;
	}
	if (variable.classification == 4) {
		uint position = 0;
		int operand = variable.count;
		if (operand < 0 || variable.array.size() < static_cast<uint>(operand))
			return fail("Invalid expression operands");
		if (!expression(variable, position, operand, value, depth + 1))
			return false;
		if (position != variable.extra.size() || operand)
			return fail("Unconsumed expression data");
		return true;
	}
	return fail("Unsupported variable classification");
}

bool ScriptRuntime::expression(const Script::Variable &variable, uint &position, int &operand, ScriptValue &value, uint depth) {
	if (128 <= depth || variable.extra.size() <= position)
		return fail("Malformed Polish expression");
	char token = variable.extra[position++];
	if (token == 'z') {
		value = ScriptValue(0);
		return true;
	}
	if (token == 't') {
		if (operand <= 0)
			return fail("Expression operand underflow");
		operand -= 1;
		return evaluate(variable.array[operand], value, depth + 1);
	}
	if (token != 's' || variable.extra.size() <= position)
		return fail("Unknown expression token");
	char operation = variable.extra[position++];
	ScriptValue right, left;
	if (!expression(variable, position, operand, right, depth + 1))
		return false;
	if (operation == '`') {
		value = ScriptValue(right.text());
		return true;
	}
	if (operation == '~') {
		value = ScriptValue(static_cast<int32>(right.text().size()));
		return true;
	}
	if (operation == '$')
		return fail("Dynamic expression compilation is not implemented");
	if (!expression(variable, position, operand, left, depth + 1))
		return false;
	return operate(operation, left, right, value);
}

bool ScriptRuntime::operate(char operation, const ScriptValue &left, const ScriptValue &right, ScriptValue &result) {
	if (left.kind() == ScriptValue::kObject && right.kind() == ScriptValue::kObject) {
		result = ScriptValue(0);
		return true;
	}
	bool numeric = left.isNumeric() && right.isNumeric();
	double a = left.number(), b = right.number();
	if (operation == '+' && !numeric) {
		Common::String text = left.text() + right.text();
		if (1024 <= text.size())
			return fail("Expression string exceeds the native limit");
		result = ScriptValue(text);
		return true;
	}
	if (operation == '=' || operation == '#' || operation == '<' || operation == '>') {
		int comparison = numeric ? (a < b ? -1 : (b < a ? 1 : 0)) : left.text().compareTo(right.text());
		bool matches = false;
		if (operation == '=')
			matches = comparison == 0;
		else if (operation == '#')
			matches = comparison != 0;
		else if (operation == '<')
			matches = comparison < 0;
		else
			matches = 0 < comparison;
		result = ScriptValue(static_cast<int32>(matches));
		return true;
	}
	if (operation == '@') {
		uint position = right.text().find(left.text());
		result = ScriptValue(position == Common::String::npos ? 0 : static_cast<int32>(position + 1));
		return true;
	}
	if (!numeric)
		return fail("Numeric operation on a nonnumeric value");
	if (operation == '+')
		result = ScriptValue(a + b);
	else if (operation == '-')
		result = ScriptValue(a - b);
	else if (operation == '*')
		result = ScriptValue(a * b);
	else if (operation == '^') {
		if (a < 0 && floor(b) != b)
			return fail("Negative base with a fractional exponent");
		result = ScriptValue(pow(a, b));
	} else if (operation == '/') {
		if (b * b < 0.00000001)
			return fail("Division by a near-zero value");
		result = ScriptValue(a / b);
	} else if (operation == '%' || operation == '\\') {
		int32 divisor = right.integer();
		int32 dividend = left.integer();
		if (!divisor || (dividend == INT_MIN && divisor == -1))
			return fail("Invalid integer division");
		result = ScriptValue(operation == '%' ? dividend % divisor : dividend / divisor);
	} else {
		return fail("Unsupported expression operator");
	}
	return true;
}

bool ScriptRuntime::arguments(const Common::Array<int16> &indices, uint first, uint end, ScriptArguments &values) {
	for (uint i = first; i < end; i++) {
		ScriptValue value;
		if (!evaluate(indices[i], value))
			return false;
		values.push_back(value);
	}
	return true;
}

void ScriptRuntime::jump(int destination) {
	_pc = destination;
	if (!_loops.empty() && _loops.back().endPC == destination)
		_loops.pop_back();
}

void ScriptRuntime::dropVariable(const Common::String &name) {
	ScriptValue value = variable(name);
	if (value.kind() == ScriptValue::kObject) {
		uint32 reference = value.reference();
		if (_blocked == reference)
			unblock();
		for (uint i = _events.size(); i; i--) {
			if (_events[i - 1].reference == reference)
				_events.remove_at(i - 1);
		}
		for (uint i = _bindings.size(); i; i--) {
			if (_bindings[i - 1].reference == reference) {
				ScriptableObject *object = _objects.resolve(reference);
				if (object)
					object->bindEvent(_bindings[i - 1].event, nullptr);
				_bindings.remove_at(i - 1);
			}
		}
		_objects.drop(reference);
		_objectNames.erase(reference);
		for (auto it = _values.begin(); it != _values.end(); ++it) {
			if (it->_value.kind() == ScriptValue::kObject && it->_value.reference() == reference)
				it->_value = ScriptValue();
		}
	}
	_values[name] = ScriptValue();
}

bool ScriptRuntime::step() {
	const Common::Array<Script::Instruction> &instructions = _image.instructions();
	if (_pc == static_cast<int>(instructions.size())) {
		// The native compiled-image loader appends a zero command after the file records.
		if (_functions)
			_functions->command(kEnd00, ScriptArguments());
		_yielded = true;
		return true;
	}
	if (_pc < 0 || instructions.size() < static_cast<uint>(_pc))
		return fail("Instruction address outside the image");
	Script::Instruction instruction = instructions[_pc++];
	if (instruction.opcode == kEnd00) {
		if (_functions)
			_functions->command(kEnd00, ScriptArguments());
		_pc -= 1;
		_yielded = true;
		return true;
	}
	if (instruction.opcode == kWait22) {
		_suspended = true;
		return true;
	}
	if (instruction.opcode == kSuspend26 || instruction.opcode == kResume27) {
		_eventsEnabled = instruction.opcode == kResume27;
		return true;
	}
	if (instruction.opcode == kExit25) {
		_request = kExitScript;
		return true;
	}
	if (instruction.opcode == kCheck10 || instruction.opcode == kJump11) {
		if (instruction.opcode == kJump11 || !_condition)
			jump(instruction.operand);
		return true;
	}
	if (instruction.opcode == kDebug13 || instruction.opcode == kDebug29)
		return true;
	if (instruction.opcode == kReturn15) {
		if (_inInterrupt && _frames.size() == _interrupt.frames.size()) {
			leaveInterrupt();
			return true;
		}
		if (_frames.empty())
			return fail("Procedure stack underflow");
		Frame frame = _frames.back();
		_frames.pop_back();
		_pc = frame.returnPC;
		return true;
	}
	Common::Array<int16> indices;
	if (!operands(instruction.operand, indices))
		return false;
	if (instruction.opcode == kProcedure07) {
		if (indices.empty())
			return fail("Malformed procedure declaration");
		if (_frames.empty()) {
			while (static_cast<uint>(_pc) < instructions.size() && instructions[_pc].opcode != kReturn15)
				_pc += 1;
			if (instructions.size() <= static_cast<uint>(_pc))
				return fail("Unterminated procedure declaration");
			_pc += 1;
			return true;
		}
		const ScriptArguments &parameters = _frames.back().arguments;
		for (uint i = 1; i < indices.size() && i <= parameters.size(); i++) {
			Common::String name;
			if (!variableName(indices[i], name))
				return false;
			setVariable(name, parameters[i - 1]);
		}
		return true;
	}
	if (indices.empty()) {
		if (instruction.opcode == kDrop19)
			return true;
		if (instruction.opcode == kPlay40)
			return true;
		if (instruction.opcode == kConditional12) {
			_condition = true;
			return true;
		}
		return fail("Missing instruction operands");
	}
	if (instruction.opcode == kIdle23 || instruction.opcode == kSchedule24) {
		ScriptValue duration;
		if (!evaluate(indices[0], duration))
			return false;
		Timer &timer = instruction.opcode == kIdle23 ? _idle : _schedule;
		timer = Timer();
		if (duration.number() != 0 && 1 < indices.size()) {
			if (!variableName(indices[1], timer.procedure) || procedure(timer.procedure) < 0)
				return fail("Unknown timer procedure");
			double ticks = duration.number() * 9.0;
			if (!(-2147483648.0 <= ticks && ticks <= 2147483647.0))
				return fail("Timer duration outside the native range");
			timer.limit = static_cast<int32>(ticks);
		}
		return true;
	}
	if (instruction.opcode == kPause28) {
		ScriptValue duration;
		if (!evaluate(indices[0], duration))
			return false;
		double delay = duration.number() * 900.0;
		if (!(0 <= delay && delay <= 2147483647.0))
			return fail("Sleep duration outside the supported range");
		uint32 &sleepUntil = _scheduler ? _scheduler->_sleepUntil : _sleepUntil;
		sleepUntil = _time + static_cast<uint32>(delay);
		_suspended = true;
		return true;
	}
	if (instruction.opcode == kYield37) {
		ScriptValue value;
		if (!evaluate(indices[0], value))
			return false;
		_yielded = value.text().size() == 2;
		return true;
	}
	if (instruction.opcode == kRelay45 || instruction.opcode == kQuit46) {
		_request = instruction.opcode == kRelay45 ? kRelayScript : kQuitApplication;
		_requestVariable = indices[0];
		return true;
	}
	if (instruction.opcode == kDrop19) {
		for (uint i = 0; i < indices.size(); i++) {
			Common::String name;
			if (!variableName(indices[i], name))
				return false;
			if (name == "schedule" || name == "idle") {
				(name == "schedule" ? _schedule : _idle).procedure.clear();
				_inInterrupt = false;
			} else if (name == "stack") {
				_frames.clear();
				_interrupt.frames.clear();
			} else if (name == "wait") {
				_suspended = _interrupt.suspended = false;
			} else if (name == "[" && i + 1 < indices.size()) {
				i += 1;
				ScriptValue value;
				Common::String prefix;
				if (!evaluate(indices[i], value) || !variableName(indices[i], prefix))
					return false;
				if (value.kind() != ScriptValue::kVoid)
					prefix = value.text();
				prefix += ".";
				Common::Array<Common::String> names;
				for (auto it = _values.begin(); it != _values.end(); ++it) {
					if (it->_key.hasPrefix(prefix) && it->_key.find('.', prefix.size()) == Common::String::npos)
						names.push_back(it->_key);
				}
				for (uint j = 0; j < names.size(); j++)
					dropVariable(names[j]);
			} else if (name == "trinkets") {
				Common::Array<Common::String> names;
				for (auto it = _values.begin(); it != _values.end(); ++it) {
					if (it->_value.kind() == ScriptValue::kObject)
						names.push_back(it->_key);
				}
				for (uint j = 0; j < names.size(); j++)
					dropVariable(names[j]);
				_events.clear();
			} else if ((name == "var" || name == "script") && i + 1 < indices.size()) {
				i += 1;
				ScriptValue value;
				if (!evaluate(indices[i], value))
					return false;
				if (name == "var")
					dropVariable(value.text());
				else if (_scheduler && value.text() == "stack")
					_scheduler->_calledScripts.clear();
			} else {
				dropVariable(name);
			}
		}
		return true;
	}
	if (instruction.opcode == kSay14) {
		ScriptArguments values;
		if (!arguments(indices, 0, indices.size(), values))
			return false;
		Common::String text;
		for (uint i = 0; i < values.size(); i++)
			text += values[i].text();
		debug("MPS: %s", text.c_str());
		return true;
	}
	if (instruction.opcode == kForInit08) {
		if (indices.size() == 1)
			return true;
		if (indices.size() < 2 || instructions.size() <= static_cast<uint>(_pc + 1))
			return fail("Malformed for loop");
		ScriptArguments values;
		if (!arguments(indices, 1, indices.size(), values))
			return false;
		Loop loop;
		loop.current = static_cast<int16>(values[0].integer());
		loop.limit = values.size() < 2 ? 0 : static_cast<int16>(values[1].integer());
		loop.step = values.size() < 2 ? -1 : (values.size() < 3 ? 1 : static_cast<int16>(values[2].integer()));
		loop.direction = loop.current < loop.limit ? 1 : -1;
		loop.endPC = instructions[_pc + 1].operand;
		loop.current = static_cast<int16>(loop.current - loop.step);
		_loops.push_back(loop);
		return true;
	}
	if (instruction.opcode == kForStep09) {
		if (indices.size() == 1) {
			ScriptValue value;
			if (!evaluate(indices[0], value))
				return false;
			Common::String text = value.text();
			_condition = text.empty() || text[0] != '0';
			return true;
		}
		if (_loops.empty())
			return fail("For loop stack underflow");
		Loop &loop = _loops.back();
		loop.current = static_cast<int16>(loop.current + loop.step);
		Common::String name;
		if (!variableName(indices[0], name))
			return false;
		setVariable(name, ScriptValue(static_cast<int32>(loop.current)));
		_condition = loop.direction && (loop.current == loop.limit ||
										(0 < loop.direction ? loop.current < loop.limit : loop.limit < loop.current));
		if (loop.current == loop.limit)
			loop.direction = 0;
		return true;
	}
	if (instruction.opcode == kConditional12) {
		ScriptValue value;
		if (!evaluate(indices[0], value))
			return false;
		Common::String text = value.text();
		_condition = text.empty() || text[0] != '0';
		return true;
	}
	if (instruction.opcode == kAssign18) {
		if (indices.size() < 2)
			return fail("Missing assignment value");
		Common::String name;
		ScriptValue value;
		if (!variableName(indices[0], name) || !evaluate(indices[1], value))
			return false;
		if (_objects.hasClass(value.text())) {
			ScriptArguments parameters;
			if (!arguments(indices, 2, indices.size(), parameters))
				return false;
			uint32 reference = _objects.create(value.text(), parameters);
			if (!reference)
				return fail("Native object construction failed: " + value.text());
			value = ScriptValue::object(reference);
		} else if (indices.size() != 2) {
			return fail("Unregistered native class: " + value.text());
		} else if (value.kind() == ScriptValue::kObject && indices[0] != indices[1] &&
				   !_image.variables()[indices[0]].indirect) {
			uint32 reference = _objects.clone(value.reference());
			if (!reference)
				return fail("Native object does not support cloning");
			value = ScriptValue::object(reference);
		}
		ScriptValue previous = variable(name);
		if (previous.kind() == ScriptValue::kObject &&
			(value.kind() != ScriptValue::kObject || previous.reference() != value.reference()))
			dropVariable(name);
		setVariable(name, value);
		return true;
	}
	if (instruction.opcode == kCall06) {
		Common::String name;
		if (!variableName(indices[0], name))
			return false;
		int destination = procedure(name);
		for (uint depth = 0; destination < 0 && depth < 128; depth++) {
			Common::String alias = variable(name).text();
			if (alias.hasPrefix("+")) {
				int address = atoi(alias.c_str() + 1);
				if (0 <= address && static_cast<uint>(address) < instructions.size() && instructions[address].opcode == kProcedure07)
					destination = address;
				break;
			}
			if (alias == name || !_values.contains(alias))
				break;
			name = alias;
			destination = procedure(name);
		}
		if (destination < 0) {
			_request = kCallScript;
			_requestVariable = indices[0];
			return true;
		}
		if (128 <= _frames.size())
			return fail("Call stack overflow: " + name);
		Frame frame;
		frame.returnPC = _pc;
		if (!arguments(indices, 1, indices.size(), frame.arguments))
			return false;
		_frames.push_back(frame);
		_pc = destination;
		return true;
	}
	if (instruction.opcode == kInvoke21) {
		ScriptValue name, result;
		ScriptArguments parameters;
		if (!evaluate(indices[0], name) || !arguments(indices, 1, indices.size(), parameters))
			return false;
		if (!_functions || !_functions->invoke(name.text(), parameters, result))
			return fail("Unbound native function: " + name.text());
		setVariable("RESULT", result);
		return true;
	}
	if (instruction.opcode == kSet35 || instruction.opcode == kGet36 || instruction.opcode == kPlay40) {
		if (indices.size() < 2 && instruction.opcode != kPlay40)
			return fail("Missing object property or method");
		ScriptValue target, property, result;
		if (!evaluate(indices[0], target) || (1 < indices.size() && !evaluate(indices[1], property)))
			return false;
		ScriptableObject *object = target.kind() == ScriptValue::kObject ? _objects.resolve(target.reference()) : nullptr;
		if (!object)
			return fail("Missing native object for " + property.text());
		object->setCallTime(_time);
		ScriptArguments parameters;
		uint end = instruction.opcode == kPlay40 ? indices.size() : indices.size() - 1;
		bool blocking = false;
		if (instruction.opcode == kPlay40) {
			Common::String method = property.text();
			if (indices.size() == 1 || method.empty() || method.hasPrefix("}")) {
				blocking = method.hasPrefix("}");
				property = ScriptValue(Common::String("default"));
			}
			for (uint i = 2; i < end; i++) {
				ScriptValue value;
				if (!evaluate(indices[i], value))
					return false;
				if (value.text().hasPrefix("}")) {
					blocking = true;
					break;
				}
				parameters.push_back(value);
			}
		} else if (!arguments(indices, 2, end, parameters)) {
			return false;
		}
		if (instruction.opcode == kGet36) {
			if (indices.size() < 3)
				return fail("Missing get destination");
			if (property.text() == "name") {
				Common::String name;
				if (!variableName(indices[0], name))
					return false;
				if (_objectNames.contains(target.reference()))
					name = _objectNames[target.reference()];
				result = ScriptValue(name);
			} else if (!object->get(property.text(), parameters, result)) {
				return fail("Unknown readable property: " + property.text());
			}
			Common::String name;
			if (!variableName(indices.back(), name))
				return false;
			setVariable(name, result);
		} else if (instruction.opcode == kSet35) {
			if (indices.size() == 2) {
				if (!object->bindEvent(property.text(), nullptr))
					return fail("Unknown event: " + property.text());
			} else {
				Common::String name;
				if (!variableName(indices.back(), name))
					return false;
				if (0 <= procedure(name))
					return bindEvent(*object, property.text(), name);
				ScriptValue value;
				if (!evaluate(indices.back(), value) || !object->set(property.text(), parameters, value))
					return fail("Unknown writable property: " + property.text());
			}
		} else {
			if (blocking) {
				unblock();
				_blocked = target.reference();
				object->addObserver(*this);
				CompletionLink link(*this);
				object->bindCompletion(&link);
			}
			if (!object->play(property.text(), parameters, result)) {
				if (blocking)
					unblock();
				else
					return fail("Unknown object method: " + property.text());
			}
		}
		return true;
	}
	ScriptArguments parameters;
	if (!arguments(indices, 0, indices.size(), parameters))
		return false;
	if (!_functions || !_functions->command(instruction.opcode, parameters)) {
		_pc -= 1;
		_yielded = true;
	}
	return true;
}

ScriptScheduler::~ScriptScheduler() {
	for (uint i = 0; i < _threads.size(); i++)
		_threads[i].runtime->_scheduler = nullptr;
}

void ScriptScheduler::attach(ScriptRuntime &runtime, const Common::String &name) {
	if (runtime._scheduler)
		runtime._scheduler->detach(runtime);
	Thread thread;
	thread.runtime = &runtime;
	thread.name = name;
	_threads.push_back(thread);
	runtime._scheduler = this;
}

void ScriptScheduler::detach(ScriptRuntime &runtime) {
	for (uint i = 0; i < _threads.size(); i++) {
		if (_threads[i].runtime == &runtime) {
			_threads.remove_at(i);
			break;
		}
	}
	runtime._scheduler = nullptr;
}

bool ScriptScheduler::tick(uint32 time) {
	if (_ticking)
		return false;
	_ticking = true;
	bool ageTimers = !_lastAge || 100 < time - _lastAge;
	if (ageTimers)
		_lastAge = time;
	bool success = true;
	for (uint i = 0; i < _threads.size(); i++) {
		ScriptRuntime &runtime = *_threads[i].runtime;
		if (!runtime.advance(time, ageTimers)) {
			success = false;
			break;
		}
		if (runtime._request == ScriptRuntime::kNoRequest)
			continue;
		ScriptRuntime::Request request = runtime._request;
		Common::String name;
		if (request == ScriptRuntime::kExitScript) {
			if (!_calledScripts.empty()) {
				name = _calledScripts.back();
				_calledScripts.pop_back();
			}
		} else {
			ScriptValue target;
			if (!runtime.evaluate(runtime._requestVariable, target)) {
				success = false;
				break;
			}
			name = target.text();
			if (request == ScriptRuntime::kCallScript)
				_calledScripts.push_back(_threads[i].name);
		}
		if (request == ScriptRuntime::kQuitApplication)
			_loader.quit();
		if (name.empty()) {
			runtime._scheduled = false;
			runtime._pc = -1;
			runtime._request = ScriptRuntime::kNoRequest;
		} else {
			success = _loader.loadScript(runtime, name);
			if (success) {
				_threads[i].name = name;
				success = runtime.start();
			}
		}
		// Native script replacement ends this tick, including advances of later threads.
		break;
	}
	_ticking = false;
	return success;
}

void ScriptScheduler::resetIdle() {
	for (uint i = 0; i < _threads.size(); i++)
		_threads[i].runtime->resetIdle();
}

void ScriptScheduler::suspend() {
	for (uint i = 0; i < _threads.size(); i++)
		_threads[i].runtime->suspend();
}

void ScriptScheduler::resume() {
	for (uint i = 0; i < _threads.size(); i++)
		_threads[i].runtime->resume();
}

} // namespace Core
} // namespace Trinket
