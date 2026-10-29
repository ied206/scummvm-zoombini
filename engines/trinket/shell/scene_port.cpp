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

constexpr const char *ScenePort::kBackgroundClickedName;
constexpr const char *ScenePort::kEffectFinishedName;
constexpr const char *ScenePort::kExittingGameName;
constexpr const char *ScenePort::kPausedName;
constexpr const char *ScenePort::kResumedName;
constexpr const char *ScenePort::kReturnPressedName;
constexpr const char *ScenePort::kSpacebarPressedName;
constexpr const char *ScenePort::kMouseMovedName;
constexpr const char *ScenePort::kSfxVolumeChangedName;
constexpr const char *ScenePort::kSpeechVolumeChangedName;
constexpr const char *ScenePort::kPropertyBackgroundMusicID;
constexpr const char *ScenePort::kPropertyBackgroundMusicVolume;
constexpr const char *ScenePort::kPropertyIsBackgroundMusicEnabled;
constexpr const char *ScenePort::kPropertyDebugMode;
constexpr const char *ScenePort::kPropertyMouseMoveEvents;
constexpr const char *ScenePort::kPropertyMouseX;
constexpr const char *ScenePort::kPropertyMouseY;
constexpr const char *ScenePort::kPropertyIsCursorHidden;
constexpr const char *ScenePort::kMethodDisplayScreen;
constexpr const char *ScenePort::kMethodScene;
constexpr const char *ScenePort::kMethodAddKeyEvent;
constexpr const char *ScenePort::kMethodFreeze;
constexpr const char *ScenePort::kMethodFreezeScreen;
constexpr const char *ScenePort::kMethodHideCursor;
constexpr const char *ScenePort::kMethodShowCursor;
constexpr const char *ScenePort::kMethodStopBackgroundMusic;

constexpr const char *ScenePortEnvironment::kEventF7;
constexpr const char *ScenePortEnvironment::kEventMouseMoved;
constexpr const char *ScenePortEnvironment::kEventSpacebarPressed;
constexpr const char *ScenePortEnvironment::kEventReturnPressed;
constexpr const char *ScenePortEnvironment::kCtrlName;
constexpr const char *ScenePortEnvironment::kCmdName;

ScenePortEnvironment::ScenePortEnvironment(Window &window, BackgroundMusic &music) : _window(window), _music(music) {
	addKeyEvent(kEventF7, kEventF7);
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
			for (uint j = 0; j < _nextBroadcast.size(); j++) {
				if (i < _nextBroadcast[j])
					_nextBroadcast[j] -= 1;
			}
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
			broadcast(kEventMouseMoved);
		return false;
	}
	if (event.type != Common::EVENT_KEYDOWN)
		return false;
	if (event.kbd.keycode == Common::KEYCODE_SPACE) {
		broadcast(kEventSpacebarPressed);
		return true;
	}
	if (event.kbd.keycode == Common::KEYCODE_RETURN || event.kbd.keycode == Common::KEYCODE_KP_ENTER) {
		broadcast(kEventReturnPressed);
		return true;
	}
	Common::String key;
	if (event.kbd.keycode == Common::KEYCODE_F7) {
		broadcast(kEventF7);
		return true;
	}
	if (event.kbd.flags & Common::KBD_CTRL)
		key = kCtrlName;
	else if (event.kbd.flags & Common::KBD_META)
		key = kCmdName;
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
	static constexpr const char *events[] = {
		kBackgroundClickedName,
		kEffectFinishedName,
		kExittingGameName,
		kPausedName,
		kResumedName,
		kReturnPressedName,
		kSpacebarPressedName,
		kMouseMovedName,
		kSfxVolumeChangedName,
		kSpeechVolumeChangedName,
	};
	for (uint i = 0; i < ARRAYSIZE(events); i++)
		addEvent(events[i]);
	_environment.attach(*this);
}

ScenePort::~ScenePort() {
	_environment.detach(*this);
}

bool ScenePort::get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (!sargs.empty())
		return false;
	if (property.equals(kPropertyBackgroundMusicID))
		result = _environment._musicID;
	else if (property.equals(kPropertyBackgroundMusicVolume))
		result = Core::ScriptValue(static_cast<int32>(_environment._music.volume()));
	else if (property.equals(kPropertyIsBackgroundMusicEnabled))
		result = Core::ScriptValue(static_cast<int32>(_environment._music.isEnabled()));
	else if (property.equals(kPropertyDebugMode))
		result = Core::ScriptValue(0);
	else if (property.equals(kPropertyMouseMoveEvents))
		result = Core::ScriptValue(static_cast<int32>(_environment._mouseMoveEvents));
	else if (property.equals(kPropertyMouseX))
		result = Core::ScriptValue(_environment._mouse.x);
	else if (property.equals(kPropertyMouseY))
		result = Core::ScriptValue(_environment._mouse.y);
	else if (property.equals(kPropertyIsCursorHidden))
		result = Core::ScriptValue(static_cast<int32>(_environment._window.isCursorHidden()));
	else
		return ScriptableObject::get(property, sargs, result);
	return true;
}

bool ScenePort::set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) {
	if (!sargs.empty())
		return false;
	if (property.equals(kPropertyBackgroundMusicID)) {
		_environment._musicID = value;
		_environment._music.setResource(_environment._window.musicResource(value));
	} else if (property.equals(kPropertyBackgroundMusicVolume)) {
		_environment._music.setVolume(CLIP<int32>(value.toInt32(), 0, 100));
	} else if (property.equals(kPropertyIsBackgroundMusicEnabled)) {
		_environment._music.setEnabled(value.toInt32() != 0);
	} else if (property.equals(kPropertyMouseMoveEvents)) {
		_environment._mouseMoveEvents = value.toInt32() != 0;
	} else {
		return ScriptableObject::set(property, sargs, value);
	}
	return true;
}

bool ScenePort::play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (method.equals(kMethodDisplayScreen) && sargs.empty()) {
		_environment._window.displayWindow();
		return true;
	}
	if (method.equals(kMethodScene) && !sargs.empty() && sargs.size() <= 2) {
		return _environment._window.installBackground(sargs[0], sargs.size() == 2 ? sargs[1] : sargs[0]);
	}
	if (method.equals(kMethodAddKeyEvent) && sargs.size() == 2) {
		_environment.addKeyEvent(sargs[0].toText(), sargs[1].toText());
		return true;
	}
	if ((method.equals(kMethodFreeze) || method.equals(kMethodFreezeScreen)) && sargs.empty()) {
		_environment._window.freezeWindow(method.equals(kMethodFreezeScreen));
		return true;
	}
	if ((method.equals(kMethodHideCursor) || method.equals(kMethodShowCursor)) && sargs.empty()) {
		_environment._window.hideCursor(method.equals(kMethodHideCursor));
		return true;
	}
	if (method.equals(kMethodStopBackgroundMusic) && sargs.size() <= 1) {
		_environment._music.stop();
		return true;
	}
	return ScriptableObject::play(method, sargs, result);
}

} // namespace Shell
} // namespace Trinket
