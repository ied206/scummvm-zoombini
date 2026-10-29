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
		/** Allow destruction through the scene-window interface. */
		virtual ~Window() {}
		/** Select a background image and palette through script resource values. */
		virtual bool installBackground(const Core::ScriptValue &image, const Core::ScriptValue &palette) = 0;
		/** Freeze screen updates, optionally leaving the rest of the scene active. */
		virtual void freezeWindow(bool screenOnly) = 0;
		/** Present the scene after applying pending window changes. */
		virtual void displayWindow() = 0;
		/** Show or hide the game cursor. */
		virtual void hideCursor(bool hidden) = 0;
		/** Return whether the game cursor is hidden. */
		virtual bool isCursorHidden() const = 0;
		/** Resolve a script music identifier to its resource path. */
		virtual Common::String musicResource(const Core::ScriptValue &id) const = 0;
	};
	/** Connect scene-port script objects to shared window and music services. */
	ScenePortEnvironment(Window &window, BackgroundMusic &music);
	/** Detach all scene ports before the environment disappears. */
	~ScenePortEnvironment();
	/** Deliver a named event to the registered scene ports. */
	void broadcast(const Common::String &event);
	/** Translate pointer and registered key input into scene-port events. */
	bool event(const Common::Event &event);
	/** Map an authored key name to the script event it triggers. */
	void addKeyEvent(const Common::String &key, const Common::String &event);
	/** Broadcast a pointer click that did not hit a scene object. */
	void backgroundClicked() { broadcast("backgroundClicked"); }

private:
	/** Script event name for f7. */
	static constexpr const char *kEventF7 = "F7";
	/** Script event name for mouse Moved. */
	static constexpr const char *kEventMouseMoved = "mouseMoved";
	/** Script event name for spacebar Pressed. */
	static constexpr const char *kEventSpacebarPressed = "spacebarPressed";
	/** Script event name for return Pressed. */
	static constexpr const char *kEventReturnPressed = "returnPressed";
	/** Script identifier for ctrl-. */
	static constexpr const char *kCtrlName = "Ctrl-";
	/** Script identifier for cmd-. */
	static constexpr const char *kCmdName = "Cmd-";

	/** Window interface used for background, display and cursor commands. */
	Window &_window;
	/** Shared background-music controller used by scene ports. */
	BackgroundMusic &_music;
	/** Scene ports currently registered for event broadcasts. */
	Common::Array<ScenePort *> _ports;
	/** Traversal positions retained while nested broadcasts modify port registration. */
	Common::Array<uint> _nextBroadcast;
	/** Authored key names mapped to scene-port event names. */
	Common::HashMap<Common::String, Common::String> _keys;
	/** Most recent pointer position in game pixels. */
	Common::Point _mouse;
	/** Script identifier of the selected background-music resource. */
	Core::ScriptValue _musicID;
	/** Whether pointer motion should be broadcast to scene ports. */
	bool _mouseMoveEvents = false;
	/** Register a scene port for window and input event broadcasts. */
	void attach(ScenePort &port);
	/** Remove a scene port and adjust active broadcast traversal positions. */
	void detach(ScenePort &port);
	friend class ScenePort;
};

/** Expose scene and window operations and dispatch scripted scene events. */
class ScenePort : public Core::ScriptableObject {
public:
	/** Register a script-facing scene port with its event environment. */
	explicit ScenePort(ScenePortEnvironment &environment);
	/** Unregister the port from window and input broadcasts. */
	~ScenePort() override;
	/** Read scene-port window, cursor, pointer or music properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Change scene-port window, cursor or music properties. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch display, background, key-event and music script commands. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Create another scene port registered with the same environment. */
	Core::ScriptableObject *clone() const override { return new ScenePort(_environment); }
	class Factory : public Core::ObjectRegistry::Factory {
	public:
		/** Configure scene-port creation for the supplied event environment. */
		explicit Factory(ScenePortEnvironment &environment) : _environment(environment) {}
		/** Create a scene port registered with this factory's environment. */
		Core::ScriptableObject *create(const Core::ScriptArguments &sargs) override { return new ScenePort(_environment); }

	private:
		/** Shared window, music and input services used by new scene ports. */
		ScenePortEnvironment &_environment;
	};

private:
	/** Script identifier for background Clicked. */
	static constexpr const char *kBackgroundClickedName = "backgroundClicked";
	/** Script identifier for effect Finished. */
	static constexpr const char *kEffectFinishedName = "effectFinished";
	/** Script identifier for exitting Game. */
	static constexpr const char *kExittingGameName = "exittingGame";
	/** Script identifier for paused. */
	static constexpr const char *kPausedName = "paused";
	/** Script identifier for resumed. */
	static constexpr const char *kResumedName = "resumed";
	/** Script identifier for return Pressed. */
	static constexpr const char *kReturnPressedName = "returnPressed";
	/** Script identifier for spacebar Pressed. */
	static constexpr const char *kSpacebarPressedName = "spacebarPressed";
	/** Script identifier for mouse Moved. */
	static constexpr const char *kMouseMovedName = "mouseMoved";
	/** Script identifier for sfx Volume Changed. */
	static constexpr const char *kSfxVolumeChangedName = "sfxVolumeChanged";
	/** Script identifier for speech Volume Changed. */
	static constexpr const char *kSpeechVolumeChangedName = "speechVolumeChanged";
	/** Script property name for background Music ID. */
	static constexpr const char *kPropertyBackgroundMusicID = "backgroundMusicID";
	/** Script property name for background Music Volume. */
	static constexpr const char *kPropertyBackgroundMusicVolume = "backgroundMusicVolume";
	/** Script property name for is Background Music Enabled. */
	static constexpr const char *kPropertyIsBackgroundMusicEnabled = "isBackgroundMusicEnabled";
	/** Script property name for debug Mode. */
	static constexpr const char *kPropertyDebugMode = "debugMode";
	/** Script property name for mouse Move Events. */
	static constexpr const char *kPropertyMouseMoveEvents = "mouseMoveEvents";
	/** Script property name for mouse X. */
	static constexpr const char *kPropertyMouseX = "mouseX";
	/** Script property name for mouse Y. */
	static constexpr const char *kPropertyMouseY = "mouseY";
	/** Script property name for is Cursor Hidden. */
	static constexpr const char *kPropertyIsCursorHidden = "isCursorHidden";
	/** Script method name for display Screen. */
	static constexpr const char *kMethodDisplayScreen = "displayScreen";
	/** Script method name for scene. */
	static constexpr const char *kMethodScene = "scene";
	/** Script method name for add Key Event. */
	static constexpr const char *kMethodAddKeyEvent = "addKeyEvent";
	/** Script method name for freeze. */
	static constexpr const char *kMethodFreeze = "freeze";
	/** Script method name for freeze Screen. */
	static constexpr const char *kMethodFreezeScreen = "freezeScreen";
	/** Script method name for hide Cursor. */
	static constexpr const char *kMethodHideCursor = "hideCursor";
	/** Script method name for show Cursor. */
	static constexpr const char *kMethodShowCursor = "showCursor";
	/** Script method name for stop Background Music. */
	static constexpr const char *kMethodStopBackgroundMusic = "stopBackgroundMusic";

	/** Shared window, music and input services used by this port. */
	ScenePortEnvironment &_environment;
	/** Declare an additional event that the environment can broadcast. */
	void registerEvent(const Common::String &event) { addEvent(event); }
	friend class ScenePortEnvironment;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
