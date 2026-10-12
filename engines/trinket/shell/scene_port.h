/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_SHELL_SCENE_PORT_H
#define TRINKET_SHELL_SCENE_PORT_H

#include "common/events.h"
#include "common/rect.h"
#include "trinket/core/object.h"
#include "trinket/shell/music.h"

namespace Trinket {
namespace Shell {

class ScenePort;

/** Shared native window services and ScenePort event registrations. */
class ScenePortEnvironment {
public:
	class Window {
	public:
		virtual ~Window() {}
		virtual bool installBackground(const Core::ScriptValue &image, const Core::ScriptValue &palette) = 0;
		virtual void freezeWindow(bool screenOnly) = 0;
		virtual void displayWindow() = 0;
		virtual void hideCursor(bool hidden) = 0;
		virtual bool isCursorHidden() const = 0;
		virtual Common::String musicResource(const Core::ScriptValue &id) const = 0;
	};
	ScenePortEnvironment(Window &window, BackgroundMusic &music);
	~ScenePortEnvironment();
	void broadcast(const Common::String &event);
	bool event(const Common::Event &event);
	void addKeyEvent(const Common::String &key, const Common::String &event);
	void backgroundClicked() { broadcast("backgroundClicked"); }

private:
	Window &_window;
	BackgroundMusic &_music;
	Common::Array<ScenePort *> _ports;
	Common::Array<uint> _nextBroadcast;
	Common::HashMap<Common::String, Common::String> _keys;
	Common::Point _mouse;
	Core::ScriptValue _musicID;
	bool _mouseMoveEvents = false;
	void attach(ScenePort &port);
	void detach(ScenePort &port);
	friend class ScenePort;
};

/** RScenePort owns event links and borrows the application's shared window. */
class ScenePort : public Core::ScriptableObject {
public:
	explicit ScenePort(ScenePortEnvironment &environment);
	~ScenePort() override;
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	Core::ScriptableObject *clone() const override { return new ScenePort(_environment); }
	class Factory : public Core::ObjectRegistry::Factory {
	public:
		explicit Factory(ScenePortEnvironment &environment) : _environment(environment) {}
		Core::ScriptableObject *create(const Core::ScriptArguments &arguments) override { return new ScenePort(_environment); }

	private:
		ScenePortEnvironment &_environment;
	};

private:
	ScenePortEnvironment &_environment;
	void registerEvent(const Common::String &event) { addEvent(event); }
	friend class ScenePortEnvironment;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
