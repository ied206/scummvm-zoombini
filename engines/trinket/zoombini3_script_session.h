/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_ZOOMBINI3_SCRIPT_SESSION_H
#define TRINKET_ZOOMBINI3_SCRIPT_SESSION_H

#include "trinket/core/script_runtime.h"
#include "trinket/shell/action.h"
#include "trinket/shell/character.h"
#include "trinket/shell/movie.h"
#include "trinket/shell/resource_paths.h"
#include "trinket/shell/scene_port.h"
#include "trinket/shell/scroller.h"
#include "trinket/shell/text_object.h"
#include "trinket/shell/xml_file.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_radio.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

class Game;
class Options;
class QuitDialog;

/** Application-owned MPS scheduler and borrowed native window/game services. */
class ScriptSession : private Core::ScriptRuntime::Functions, private Core::ScriptScheduler::Loader, private Shell::ActionSystem::Factory, private Shell::ScenePortEnvironment::Window, private Shell::Movie::Host, private Shell::ResourcePort::Host, private WorldPort::Host, private AttributeCharacter::AnimationResources, private Shell::Character::Resources {
public:
	explicit ScriptSession(Game &game);
	~ScriptSession();
	bool start(bool fresh, bool fromLauncher);
	void event(const Common::Event &event);
	void draw(OM::Canvas &canvas, uint32 time);
	bool hasModal() const;
	void syncSoundSettings();
	void inheritInitialSoundSettings() { _initialSoundProperties = kMusic01 | kDialogue02 | kEffects04; }
	Common::String debugCommand(const Common::String &command);

private:
	enum InitialSoundProperty {
		kMusic01 = 1,
		kDialogue02 = 2,
		kEffects04 = 4,
	};
	class NativeFactory : public Core::ObjectRegistry::Factory {
	public:
		NativeFactory(ScriptSession &session, const Common::String &name) : _session(session), _name(name) {}
		Core::ScriptableObject *create(const Core::ScriptArguments &arguments) override;
		const Common::String &name() const { return _name; }

	private:
		ScriptSession &_session;
		Common::String _name;
	};
	class OptionsObject;
	struct PropertyDefinition {
		bool string = false, saveable = true;
	};
	Game &_game;
	Shell::ResourcePaths _paths;
	Shell::Scene _scene;
	Shell::Scroller _scroller;
	Core::ScriptRuntime _runtime;
	Core::ScriptScheduler _scheduler;
	Shell::ActionSystem _actions;
	Shell::ScenePortEnvironment _window;
	Shell::ActionSystem::ContainerFactory _queueFactory, _compositeFactory;
	Shell::ScenePort::Factory _sceneFactory;
	Shell::ResourcePort::Factory _resourceFactory;
	WorldPort::Factory _worldFactory;
	Shell::Movie::Factory _movieFactory;
	RadioGroup::Factory _radioFactory;
	Common::Array<NativeFactory *> _factories;
	Common::HashMap<Common::String, PropertyDefinition> _definitions;
	Common::HashMap<Common::String, Core::ScriptValue> _transient;
	Common::Array<Shell::Movie *> _movies;
	Common::Array<OM::Sound *> _sounds;
	Common::ScopedPtr<OM::Sound> _glyphSounds[10];
	Common::ScopedPtr<QuitDialog> _quitDialog;
	Common::SharedPtr<OM::NFNTFont> _font;
	OM::Sound _chuteSound, _gearSound;
	Graphics::Surface _frozenScreen, _cursors[6];
	Common::String _scriptName, _launcherPlayer;
	Common::Point _mouse, _dragOffset;
	uint32 _pressed = 0, _hovered = 0, _tracked = 0, _time = 0, _lastHostTime = 0;
	uint32 _movieTasks = 0;
	int _cursor = -1;
	uint _initialSoundProperties = 0;
	bool _fresh = false, _cursorHidden = false, _frozen = false, _transition = false;
	bool _closedCaptioning = false, _doubleClicks = true, _failed = false;
	bool _autoLevelingGlobal = false;
	void registerClasses();
	Core::ScriptableObject *createNative(const Common::String &name, const Core::ScriptArguments &arguments);
	bool loadScript(Core::ScriptRuntime &thread, const Common::String &name) override;
	void quit() override;
	bool invoke(const Common::String &name, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool resourceCommand(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	Shell::Action *createAction(const Common::String &name, const Core::ScriptArguments &arguments, bool interruptible) override;
	bool installBackground(const Core::ScriptValue &image, const Core::ScriptValue &palette) override;
	void freezeWindow(bool screenOnly) override;
	void displayWindow() override { _frozen = false; }
	void hideCursor(bool hidden) override;
	bool isCursorHidden() const override { return _cursorHidden; }
	Common::String musicResource(const Core::ScriptValue &id) const override;
	void movieCreated(Shell::Movie &movie) override;
	void movieDestroyed(Shell::Movie &movie) override;
	void moviePlaying(Shell::Movie &movie, bool playing) override;
	Common::String characterAnimationPath(uint resource) const override;
	Common::String characterResource(uint resource, Shell::Character::Resources::Kind kind, uint part = 1) const override;
	uint playSound(const Common::String &resource, Audio::Mixer::SoundType type = Audio::Mixer::kSFXSoundType, uint repeat = 1);
	void updateNative();
	void nativeEvent(const Common::Event &event);
	uint32 hitObject(Common::Point point);
	Core::Trinket *visualObject(uint32 reference);
	void updateCursor();
	bool installCursors();
	void openQuit();
	void resetPlayerProperties();
	void fail(const Common::String &context);
	ScriptSession(const ScriptSession &) = delete;
	ScriptSession &operator=(const ScriptSession &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
