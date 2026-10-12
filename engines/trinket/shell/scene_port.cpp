/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/shell/scene_port.h"

namespace Trinket {
namespace Shell {

ScenePortEnvironment::ScenePortEnvironment(Window &window, BackgroundMusic &music) : _window(window), _music(music) {
	addKeyEvent("F7", "F7");
}

ScenePortEnvironment::~ScenePortEnvironment() {
	assert(_ports.empty());
}

void ScenePortEnvironment::attach(ScenePort &port) {
	_ports.push_back(&port);
	for (auto it = _keys.begin(); it != _keys.end(); ++it)
		port.registerEvent(it->_value);
}

void ScenePortEnvironment::detach(ScenePort &port) {
	for (uint i = 0; i < _ports.size(); i++) {
		if (_ports[i] == &port) {
			_ports.remove_at(i);
			for (uint j = 0; j < _nextBroadcast.size(); j++)
				if (i < _nextBroadcast[j])
					_nextBroadcast[j] -= 1;
			return;
		}
	}
}

void ScenePortEnvironment::addKeyEvent(const Common::String &key, const Common::String &event) {
	_keys[key] = event;
	for (uint i = 0; i < _ports.size(); i++)
		_ports[i]->registerEvent(event);
}

void ScenePortEnvironment::broadcast(const Common::String &event) {
	_nextBroadcast.push_back(0);
	while (_nextBroadcast.back() < _ports.size()) {
		ScenePort *port = _ports[_nextBroadcast.back()];
		_nextBroadcast.back() += 1;
		port->emitEvent(event);
	}
	_nextBroadcast.pop_back();
}

bool ScenePortEnvironment::event(const Common::Event &event) {
	if (event.type == Common::EVENT_MOUSEMOVE) {
		_mouse = event.mouse;
		if (_mouseMoveEvents)
			broadcast("mouseMoved");
		return false;
	}
	if (event.type != Common::EVENT_KEYDOWN)
		return false;
	if (event.kbd.keycode == Common::KEYCODE_SPACE) {
		broadcast("spacebarPressed");
		return true;
	}
	if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
		broadcast("returnPressed");
		return true;
	}
	Common::String key;
	if (event.kbd.keycode == Common::KEYCODE_F7) {
		broadcast("F7");
		return true;
	}
	if (event.kbd.flags & Common::KBD_CTRL)
		key = "Ctrl-";
	else if (event.kbd.flags & Common::KBD_META)
		key = "Cmd-";
	int character = event.kbd.ascii;
	if (!key.empty() && 'a' <= character && character <= 'z')
		character -= 'a' - 'A';
	if (!character && Common::KEYCODE_a <= event.kbd.keycode && event.kbd.keycode <= Common::KEYCODE_z)
		character = 'A' + event.kbd.keycode - Common::KEYCODE_a;
	if (!character)
		return false;
	key += static_cast<char>(character);
	auto found = _keys.find(key);
	if (found == _keys.end())
		return false;
	broadcast(found->_value);
	return true;
}

ScenePort::ScenePort(ScenePortEnvironment &environment) : _environment(environment) {
	static const char *const events[] = {
		"backgroundClicked",
		"effectFinished",
		"exittingGame",
		"paused",
		"resumed",
		"returnPressed",
		"spacebarPressed",
		"mouseMoved",
		"sfxVolumeChanged",
		"speechVolumeChanged",
	};
	for (uint i = 0; i < ARRAYSIZE(events); i++)
		addEvent(events[i]);
	_environment.attach(*this);
}

ScenePort::~ScenePort() {
	_environment.detach(*this);
}

bool ScenePort::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (!arguments.empty())
		return false;
	if (property == "backgroundMusicID")
		result = _environment._musicID;
	else if (property == "backgroundMusicVolume")
		result = Core::ScriptValue(static_cast<int32>(_environment._music.volume()));
	else if (property == "isBackgroundMusicEnabled")
		result = Core::ScriptValue(static_cast<int32>(_environment._music.isEnabled()));
	else if (property == "debugMode")
		result = Core::ScriptValue(0);
	else if (property == "mouseMoveEvents")
		result = Core::ScriptValue(static_cast<int32>(_environment._mouseMoveEvents));
	else if (property == "mouseX")
		result = Core::ScriptValue(_environment._mouse.x);
	else if (property == "mouseY")
		result = Core::ScriptValue(_environment._mouse.y);
	else if (property == "isCursorHidden")
		result = Core::ScriptValue(static_cast<int32>(_environment._window.isCursorHidden()));
	else
		return ScriptableObject::get(property, arguments, result);
	return true;
}

bool ScenePort::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (!arguments.empty())
		return false;
	if (property == "backgroundMusicID") {
		_environment._musicID = value;
		_environment._music.setResource(_environment._window.musicResource(value));
	} else if (property == "backgroundMusicVolume") {
		_environment._music.setVolume(CLIP<int32>(value.integer(), 0, 100));
	} else if (property == "isBackgroundMusicEnabled") {
		_environment._music.setEnabled(value.integer() != 0);
	} else if (property == "mouseMoveEvents") {
		_environment._mouseMoveEvents = value.integer() != 0;
	} else {
		return ScriptableObject::set(property, arguments, value);
	}
	return true;
}

bool ScenePort::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (method == "displayScreen" && arguments.empty()) {
		_environment._window.displayWindow();
		return true;
	}
	if (method == "scene" && !arguments.empty() && arguments.size() <= 2) {
		return _environment._window.installBackground(arguments[0], arguments.size() == 2 ? arguments[1] : arguments[0]);
	}
	if (method == "addKeyEvent" && arguments.size() == 2) {
		_environment.addKeyEvent(arguments[0].text(), arguments[1].text());
		return true;
	}
	if ((method == "freeze" || method == "freezeScreen") && arguments.empty()) {
		_environment._window.freezeWindow(method == "freezeScreen");
		return true;
	}
	if ((method == "hideCursor" || method == "showCursor") && arguments.empty()) {
		_environment._window.hideCursor(method == "hideCursor");
		return true;
	}
	if (method == "stopBackgroundMusic" && arguments.size() <= 1) {
		_environment._music.stop();
		return true;
	}
	return ScriptableObject::play(method, arguments, result);
}

} // namespace Shell
} // namespace Trinket
