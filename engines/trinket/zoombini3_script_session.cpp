/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/zoombini3_script_session.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "gui/message.h"
#include "trinket/shell/animation_action.h"
#include "trinket/shell/character_action.h"
#include "trinket/shell/hotspot.h"
#include "trinket/shell/sound_action.h"
#include "trinket/trinket.h"
#include "trinket/zoombini3_attrib_ch_walk_action.h"
#include "trinket/zoombini3_game.h"
#include "trinket/zoombini3_options.h"
#include "trinket/zoombini3_pages/interactive_ziv.h"
#include "trinket/zoombini3_pages/puzzle_barn.h"
#include "trinket/zoombini3_pages/puzzle_catapult.h"
#include "trinket/zoombini3_pages/puzzle_corral.h"
#include "trinket/zoombini3_pages/puzzle_garden.h"
#include "trinket/zoombini3_pages/puzzle_glyph.h"
#include "trinket/zoombini3_pages/puzzle_greenhouse.h"
#include "trinket/zoombini3_pages/puzzle_planetarium.h"
#include "trinket/zoombini3_quit.h"

namespace Trinket {
namespace Zoombini3 {

class ScriptSession::OptionsObject : public Core::ScriptableObject {
public:
	explicit OptionsObject(ScriptSession &session) : _session(session),
													 _options(session._paths, session._game._vm.mixer(), session._game._world) {
		addEvent("newGame");
		addEvent("gameSelected");
		addEvent("cheatChange");
		addEvent("closed");
	}
	bool load() {
		const Shell::SaveFiles::Player *player = _session._game._vm.activePlayer();
		return player && _options.load(_session._game._practice, _session._game._vm.music().volume(), *player);
	}
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override {
		if (property == "selectedGame")
			result = Core::ScriptValue(static_cast<int32>(_selected));
		else if (property == "replaceSelectedGame")
			result = Core::ScriptValue(static_cast<int32>(_replace));
		else
			return ScriptableObject::get(property, arguments, result);
		return true;
	}
	void event(const Common::Event &event, uint32 time) {
		Options::Result result = _options.event(event, time);
		if (result.volumeChanges) {
			_session._game.applySoundSettings();
			if (result.volumeChanges & Options::kDialogue02)
				_session._window.broadcast("speechVolumeChanged");
			if (result.volumeChanges & Options::kEffects04)
				_session._window.broadcast("sfxVolumeChanged");
		}
		if (result.music == Options::kStartCredits01) {
			_savedMusic = _session._game._vm.music().resource();
			_session._game._vm.music().setResource(_session._paths.resourcePath("403", ".wav"));
		} else if (result.music == Options::kRestoreMusic02) {
			_session._game._vm.music().setResource(_savedMusic);
		}
		_selected = result.selectedSlot;
		_replace = result.replaceSelectedGame;
		if (result.action == Options::kNewGame01)
			emitEvent("newGame");
		else if (result.action == Options::kLoadGame02)
			emitEvent("gameSelected");
		else if (result.action == Options::kClose05)
			emitEvent("closed");
		else if (result.action == Options::kQuit04)
			_session.openQuit();
	}
	void draw(OM::Canvas &canvas, uint32 time) { _options.draw(canvas, time); }
	bool overControl(Common::Point point) const { return _options.overControl(point); }
	void syncVolumes() { _options.syncVolumes(_session._game._world.musicVolume()); }

private:
	ScriptSession &_session;
	Options _options;
	Common::String _savedMusic;
	uint _selected = 0;
	bool _replace = false;
};

ScriptSession::ScriptSession(Game &game) : _game(game), _paths(game._vm.resources()), _scene(_paths), _scroller(_scene, game._vm.gameViewport(), 50),
										   _scheduler(*this), _actions(_runtime, *this), _window(*this, game._vm.music()),
										   _queueFactory(_actions, false), _compositeFactory(_actions, true), _sceneFactory(_window), _resourceFactory(*this),
										   _worldFactory(game._world, *this), _movieFactory(*this), _radioFactory(_runtime),
										   _chuteSound(game._vm.mixer()), _gearSound(game._vm.mixer()) {
	static const char *const classes[] = {
		"RAnimation",
		"RFlyButton",
		"RQuitButton",
		"RText",
		"RXMLFile",
		"ROptions",
		"RAttributeCharacter",
		"RCharacter",
		"RCatapultGame",
		"RGlyphGame",
		"RPlanetariumGame",
		"RIncubator",
		"Greenhouse",
		"RGardenGame",
		"RCorralGame",
		"RBarnGame",
		"RZIVGame",
		"RHotSpot",
	};
	for (const char *name : classes)
		_factories.push_back(new NativeFactory(*this, name));
	for (auto &sound : _glyphSounds)
		sound.reset(new OM::Sound(game._vm.mixer()));
	_transient["inGame"] = Core::ScriptValue(game._practice ? 0 : 1);
	_transient["previousLocation"] = Core::ScriptValue(Common::String());
	_transient["isAutoLevelingGlobal"] = Core::ScriptValue(0);
	_transient["debugMode"] = Core::ScriptValue(0);
}

ScriptSession::~ScriptSession() {
	_runtime.clear();
	_scheduler.detach(_runtime);
	_scroller.stop();
	_scene.clear();
	for (NativeFactory *factory : _factories)
		delete factory;
	for (OM::Sound *sound : _sounds)
		delete sound;
	if (0 <= _cursor)
		CursorMan.popCursor();
	CursorMan.showMouse(true);
	for (Graphics::Surface &cursor : _cursors)
		cursor.free();
	_frozenScreen.free();
}

void ScriptSession::registerClasses() {
	_runtime.setFunctions(this);
	_runtime.objects().registerClass("RScenePort", _sceneFactory);
	_runtime.objects().registerClass("RResourcePort", _resourceFactory);
	_runtime.objects().registerClass("RWorldPort", _worldFactory);
	_runtime.objects().registerClass("RBinkMovie", _movieFactory);
	_runtime.objects().registerClass("RRadioGroup", _radioFactory);
	_runtime.objects().registerClass("RQueue", _queueFactory);
	_runtime.objects().registerClass("RCompositeAction", _compositeFactory);
	for (NativeFactory *factory : _factories)
		_runtime.objects().registerClass(factory->name(), *factory);
}

bool ScriptSession::start(bool fresh, bool fromLauncher) {
	_fresh = fresh;
	if (fresh)
		inheritInitialSoundSettings();
	_launcherPlayer = _game._playerName;
	_lastHostTime = g_system->getMillis();
	_time = _lastHostTime;
	if (!fromLauncher) {
		_autoLevelingGlobal = true;
		_transient["isAutoLevelingGlobal"] = Core::ScriptValue(1);
		_window.addKeyEvent("Ctrl-O", "optionsKeyPressed");
		_window.addKeyEvent("Cmd-O", "optionsKeyPressed");
	}
	Common::ScopedPtr<Common::SeekableReadStream> font(_paths.open("/z3common/351.nfnt"));
	_font.reset(font ? OM::NFNTFont::load(*font) : nullptr);
	if (!_font) {
		warning("Trinket: could not load the startup font z3common/351.nfnt");
		return false;
	}
	if (!installCursors())
		return false;
	Common::String script = fromLauncher ? "main.mps" : WorldPort::nameForLocation(_game._world.currentLocation()) + ".mps";
	if (!loadScript(_runtime, script))
		return false;
	_scheduler.attach(_runtime, script);
	if (!_runtime.start()) {
		fail("startup");
		return false;
	}
	if (!_scheduler.tick(_time)) {
		fail("startup");
		return false;
	}
	return true;
}

bool ScriptSession::loadScript(Core::ScriptRuntime &thread, const Common::String &name) {
	thread.clear();
	_scroller.stop();
	_scene.clear();
	_pressed = _hovered = _tracked = 0;
	_chuteSound.stop();
	_gearSound.stop();
	for (auto &sound : _glyphSounds)
		sound->stop();
	Common::File stream;
	if (!stream.open(Common::Path(name))) {
		warning("Trinket: could not open script %s", name.c_str());
		return false;
	}
	if (!thread.load(stream)) {
		warning("Trinket: could not load script %s: %s", name.c_str(), thread.lastError().c_str());
		return false;
	}
	_scriptName = name;
	registerClasses();
	thread.setTime(_time);
	resetPlayerProperties();
	return true;
}

Core::ScriptableObject *ScriptSession::NativeFactory::create(const Core::ScriptArguments &arguments) {
	return _session.createNative(_name, arguments);
}

Core::ScriptableObject *ScriptSession::createNative(const Common::String &name, const Core::ScriptArguments &arguments) {
	if (name == "RAnimation" || name == "RFlyButton" || name == "RQuitButton") {
		bool button = name != "RAnimation";
		if (arguments.size() != 1 && arguments.size() != 3)
			return nullptr;
		uint resource = !button && arguments.size() == 3 ? 2 : 0;
		Common::String path = _paths.resourcePath(arguments[resource].text(), ".ao");
		Shell::Sprite *sprite = _scene.add(path, 0, 0, button);
		if (!sprite || !sprite->startAnimation()) {
			warning("Trinket: could not create %s animation '%s' (resolved path '%s')", name.c_str(), arguments[resource].text().c_str(), path.c_str());
			return nullptr;
		}
		sprite->setFrame(1);
		if (!button)
			sprite->setRepeat(-1);
		sprite->flyButton = button;
		sprite->spotCursor = "301";
		sprite->grabCursor = "304";
		if (arguments.size() == 3) {
			uint first = button ? 1 : 0;
			if (arguments[first].integer() != 11111)
				sprite->x = arguments[first].integer();
			if (arguments[first + 1].integer() != 11111)
				sprite->y = arguments[first + 1].integer();
		}
		return sprite;
	}
	if (name == "RText") {
		Common::ScopedPtr<Shell::TextObject> text(new Shell::TextObject(_paths, _font));
		return text->create(arguments) ? text.release() : nullptr;
	}
	if (name == "RXMLFile" && arguments.size() == 1) {
		Common::ScopedPtr<Shell::XMLFile> file(new Shell::XMLFile());
		if (arguments[0].text() == "@launcher") {
			Common::String player = OM::XMLParser::escape(_launcherPlayer);
			if (!file->loadText("<?xml version=\"1.0\"?><launcher><command>play</command><user><name>\"" + player + "\"</name></user></launcher>"))
				return nullptr;
		} else if (!file->load(arguments[0].text())) {
			return nullptr;
		}
		return file.release();
	}
	if (name == "RHotSpot") {
		Shell::HotSpot::Factory factory;
		return factory.create(arguments);
	}
	if (name == "ROptions") {
		Common::ScopedPtr<OptionsObject> options(new OptionsObject(*this));
		return options->load() ? options.release() : nullptr;
	}
	if (name == "RAttributeCharacter") {
		Common::ScopedPtr<AttributeCharacter> character(new AttributeCharacter(_game._vm, _scene));
		if (!character->create(arguments, _time))
			return nullptr;
		character->setAnimationResources(this);
		return character.release();
	}
	if (name == "RCharacter" && (arguments.size() == 1 || arguments.size() == 2 || arguments.size() == 4)) {
		Common::Point position(11111, 11111);
		int depth = arguments.size() == 2 ? arguments[1].integer() : 0;
		if (arguments.size() == 4) {
			position = Common::Point(arguments[1].integer(), arguments[2].integer());
			depth = arguments[3].integer();
		}
		Common::ScopedPtr<Shell::Character> character(new Shell::Character(_scene, _game._vm.mixer(), _paths, _game._vm.rnd(), *this));
		return character->create(arguments[0].integer(), depth, position) ? character.release() : nullptr;
	}
	if (name == "RCatapultGame" && (arguments.empty() || (arguments.size() == 1 && arguments[0].text() == "!")))
		return new CatapultMachine(_scene);
	if (name == "RGlyphGame" && arguments.size() == 3)
		return new GlyphMachine(_scene, _game._vm.rnd(), Common::Point(arguments[0].integer(), arguments[1].integer()), arguments[2].integer());
	if (name == "RPlanetariumGame" && arguments.size() == 1)
		return new PlanetariumMachine(_scene, _game._vm.rnd(), arguments[0].integer());
	if (name == "RIncubator" && arguments.size() == 2) {
		Common::ScopedPtr<Incubator> incubator(new Incubator(_game._vm, _scene, arguments[0].integer(), arguments[1].integer()));
		return incubator->load() ? incubator.release() : nullptr;
	}
	if (name == "Greenhouse" && arguments.size() == 1)
		return new GreenhouseMachine(_game._vm, _scene, arguments[0].integer());
	if (name == "RGardenGame" && (arguments.empty() || (arguments.size() == 1 && arguments[0].text() == "!")))
		return new GardenMachine(_game._vm, _scene);
	if (name == "RCorralGame" && (arguments.empty() || (arguments.size() == 1 && arguments[0].text() == "!")))
		return new CorralMachine(_game._vm, _scene);
	if (name == "RBarnGame" && arguments.size() <= 1)
		return new BarnMachine(_game._vm, _scene, arguments.empty() || arguments[0].text() == "!" ? 0 : arguments[0].integer());
	if (name == "RZIVGame" && arguments.size() <= 1)
		return new ZIVMachine(_scene, _scroller, _time);
	return nullptr;
}

void ScriptSession::resetPlayerProperties() {
	for (auto it = _definitions.begin(); it != _definitions.end(); ++it) {
		if (!it->_value.saveable)
			continue;
		Common::String value;
		if (!_game._world.stringProperty(it->_key, value))
			_game._world.setStringProperty(it->_key, it->_value.string ? "" : "0");
	}
}

bool ScriptSession::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property == "playerName" && arguments.empty()) {
		result = Core::ScriptValue(_game._playerName);
		return true;
	}
	if (property == "isNewPlayer" && arguments.size() <= 1) {
		Common::String player = arguments.empty() ? _game._playerName : arguments[0].text();
		result = Core::ScriptValue(static_cast<int32>((_fresh && player == _launcherPlayer) || !_game._vm.hasGamePlayer(player)));
		return true;
	}
	if (property == "autoEnabled") {
		Location location = static_cast<Location>(_game._world.currentLocation());
		if (!arguments.empty() && !arguments[0].text().empty() && !WorldPort::locationForName(arguments[0].text(), location))
			return false;
		if (_autoLevelingGlobal)
			location = kGlyph02;
		if (location < kCatapult01 || kBarn07 < location)
			return false;
		result = Core::ScriptValue(_game._world.autoEnabled(location - kCatapult01));
		return true;
	}
	if (arguments.empty() && _transient.contains(property)) {
		result = _transient[property];
		return true;
	}
	return false;
}

bool ScriptSession::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property == "autoEnabled") {
		Location location = static_cast<Location>(_game._world.currentLocation());
		if (!arguments.empty() && !arguments[0].text().empty() && !WorldPort::locationForName(arguments[0].text(), location))
			return false;
		if (_autoLevelingGlobal) {
			for (uint i = 0; i < 7; i++)
				_game._world.setAutoEnabled(i, value.integer());
			return true;
		}
		return kCatapult01 <= location && location <= kBarn07 && _game._world.setAutoEnabled(location - kCatapult01, value.integer());
	}
	if (arguments.empty() && _transient.contains(property)) {
		_transient[property] = value;
		if (property == "isAutoLevelingGlobal")
			_autoLevelingGlobal = value.integer() != 0;
		return true;
	}
	if (arguments.empty() && (property == "backgroundMusicVolume" || property == "dialogVolume" || property == "sfxVolume" ||
							  property == "isBackgroundMusicOff")) {
		uint initial = 0;
		if (property == "backgroundMusicVolume")
			initial = kMusic01;
		else if (property == "dialogVolume")
			initial = kDialogue02;
		else if (property == "sfxVolume")
			initial = kEffects04;
		// The initial MPS defaults retain ScummVM's launcher preferences for each new slot.
		if (_initialSoundProperties & initial)
			_initialSoundProperties &= ~initial;
		else
			_game._world.setStringProperty(property, value.text());
		_game.applySoundSettings();
		return true;
	}
	return false;
}

bool ScriptSession::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if ((method == "addIntProperty" || method == "addStringProperty") && !arguments.empty() && arguments.size() <= 2) {
		PropertyDefinition definition;
		definition.string = method == "addStringProperty";
		definition.saveable = arguments.size() < 2 || arguments[1].text().empty() || arguments[1].integer() != 0;
		_definitions[arguments[0].text()] = definition;
		if (!definition.saveable && !_transient.contains(arguments[0].text()))
			_transient[arguments[0].text()] = definition.string ? Core::ScriptValue(Common::String()) : Core::ScriptValue(0);
		resetPlayerProperties();
		return true;
	}
	if (method == "activatePlayer" && arguments.size() == 1) {
		if (!_game._vm.activateScriptPlayer(_game, arguments[0].text()))
			return false;
		if (arguments[0].text() == _launcherPlayer)
			_fresh = false;
		resetPlayerProperties();
		_game.applySoundSettings();
		return true;
	}
	if ((method == "activateGame" && !arguments.empty() && arguments.size() <= 2) || (method == "addGame" && arguments.empty())) {
		bool append = method == "addGame";
		if (!_game._vm.activateScriptGame(_game, append ? 0 : arguments[0].integer(),
										  !append && 1 < arguments.size() && arguments[1].integer() != 0, append))
			return false;
		resetPlayerProperties();
		_game.applySoundSettings();
		return true;
	}
	if (method == "newGame" && arguments.empty()) {
		_game._world.resetGame();
		return true;
	}
	return false;
}

bool ScriptSession::resourceCommand(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (!arguments.empty())
		return false;
	if (method == "uncacheResources") {
		_scene.uncacheUnusedResources();
		return true;
	}
	// Native modules are linked into the subengine; their objects retain heap ownership.
	return method == "deactivateAllDLLs" || method == "uncacheAllDLLs";
}

uint ScriptSession::playSound(const Common::String &resource, Audio::Mixer::SoundType type, uint repeat) {
	Common::ScopedPtr<OM::Sound> sound(new OM::Sound(_game._vm.mixer()));
	if (!sound->play(_paths, _paths.resourcePath(resource, ".wav"), type, repeat))
		return 0;
	_sounds.push_back(sound.release());
	return _sounds.size();
}

bool ScriptSession::invoke(const Common::String &name, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (name == "MessageBox" && arguments.size() == 1) {
		GUI::MessageDialog dialog(arguments[0].text().decode());
		dialog.runModal();
		_lastHostTime = g_system->getMillis();
		return true;
	}
	if (name == "RandomNumber" && arguments.size() == 2) {
		result = Core::ScriptValue(_game._vm.rnd().number(arguments[0].integer(), arguments[1].integer()));
		return true;
	}
	if (name == "GetLauncherXMLFilename" && arguments.empty()) {
		result = Core::ScriptValue(Common::String("@launcher"));
		return true;
	}
	if (name == "readINI" && arguments.size() == 1) {
		Common::String key = arguments[0].text();
		result = Core::ScriptValue(ConfMan.hasKey(key) ? ConfMan.getInt(key) : 0);
		return true;
	}
	if ((name == "addPath" || name == "use") && arguments.size() == 1) {
		if (name == "use")
			_paths.use(arguments[0].text());
		else
			_paths.addPath(arguments[0].text());
		return true;
	}
	if (name == "removeAllPaths" && arguments.empty()) {
		_paths.removeAllPaths();
		return true;
	}
	if ((name == "bulkFile" || name == "indexFile") && arguments.size() == 1) {
		Common::File file;
		return file.open(Common::Path(arguments[0].text()));
	}
	if (name == "InstallCursors" && arguments.empty())
		return installCursors();
	if ((name == "StartTransition" || name == "EndTransition") && arguments.empty()) {
		_transition = name == "StartTransition";
		if (!_transition)
			_frozen = false;
		return true;
	}
	if ((name == "SetClosedCaptioningActive" || name == "SetDoubleClicksEnabled") && arguments.size() == 1) {
		if (name == "SetClosedCaptioningActive")
			_closedCaptioning = arguments[0].integer() != 0;
		else
			_doubleClicks = arguments[0].integer() != 0;
		return true;
	}
	if (name == "PlaySound" && arguments.size() == 1) {
		result = Core::ScriptValue(static_cast<int32>(playSound(arguments[0].text())));
		return true;
	}
	if ((name == "PauseSound" || name == "ResumeSound" || name == "KillSound") && arguments.size() == 1) {
		int tag = arguments[0].integer();
		if (0 < tag && static_cast<uint>(tag) <= _sounds.size() && _sounds[tag - 1]) {
			if (name == "KillSound")
				_sounds[tag - 1]->stop();
			else
				_sounds[tag - 1]->pause(name == "PauseSound");
		}
		return true;
	}
	if (name == "IntersectTest" && arguments.size() == 2) {
		Core::ScriptValue a = arguments[0].kind() == Core::ScriptValue::kObject ? arguments[0] : _runtime.variable(arguments[0].text());
		Core::ScriptValue b = arguments[1].kind() == Core::ScriptValue::kObject ? arguments[1] : _runtime.variable(arguments[1].text());
		Core::Trinket *left = visualObject(a.reference()), *right = visualObject(b.reference());
		if (!left || !right)
			return false;
		result = Core::ScriptValue(static_cast<int32>(Common::Rect(left->x, left->y, left->x + left->width(), left->y + left->height()).intersects(Common::Rect(right->x, right->y, right->x + right->width(), right->y + right->height()))));
		return true;
	}
	return false;
}

Shell::Action *ScriptSession::createAction(const Common::String &name, const Core::ScriptArguments &arguments, bool interruptible) {
	if (name == "AttribChWalkAction")
		return AttribChWalkAction::create(_actions, arguments, interruptible);
	if (name == "RandomDelayAction" && (arguments.size() == 2 || arguments.size() == 3)) {
		uint32 first = arguments[0].integer(), last = arguments[1].integer();
		if (first == last)
			return nullptr;
		if (2 < arguments.size())
			interruptible = arguments[2].integer() != 0;
		return new Shell::RandomDelayAction(_actions, _game._vm.rnd(), first, last, interruptible);
	}
	if (name == "PlayAnimAction" && !arguments.empty() && arguments.size() <= 2) {
		if (1 < arguments.size())
			interruptible = arguments[1].integer() != 0;
		return new Shell::PlayAnimationAction(_actions, _scene, arguments[0], interruptible);
	}
	if (name == "AnimAction" && 3 <= arguments.size() && arguments.size() <= 6) {
		Common::Point position(11111, 11111);
		uint first = arguments.size() <= 4 ? 1 : 3;
		if (3 == first)
			position = Common::Point(arguments[1].integer(), arguments[2].integer());
		if (first + 2 < arguments.size())
			interruptible = arguments[first + 2].integer() != 0;
		return new Shell::AnimationAction(_actions, _scene, _paths.resourcePath(arguments[0].text(), ".ao"),
										  position, arguments[first].integer(), arguments[first + 1].integer(), interruptible);
	}
	if ((name == "CharacterSpeechAction" || name == "CharacterAnimAction") && 2 <= arguments.size() && arguments.size() <= 4 &&
		_actions.resolve(arguments[0], "RCharacter")) {
		if (3 < arguments.size())
			interruptible = arguments[3].integer() != 0;
		return new Shell::CharacterAction(_actions, arguments[0], arguments[1].integer(), name == "CharacterSpeechAction",
										  name == "CharacterAnimAction" && 2 < arguments.size() ? arguments[2].integer() : 1, true, interruptible);
	}
	if ((name == "SoundAction" || name == "LoopedSoundAction" || name == "SpeechAction") && !arguments.empty() && arguments.size() <= 4) {
		if (name == "SpeechAction" && 2 < arguments.size())
			interruptible = arguments[2].integer() != 0;
		Audio::Mixer::SoundType type = name == "SpeechAction" ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType;
		uint repeat = name == "LoopedSoundAction" ? 0 : 1;
		if (name == "SpeechAction" && 3 < arguments.size())
			repeat = arguments[3].integer();
		return new Shell::SoundAction(_actions, _game._vm.mixer(), _paths, _paths.resourcePath(arguments[0].text(), ".wav"),
									  type, interruptible, repeat);
	}
	return nullptr;
}

Common::String ScriptSession::characterAnimationPath(uint resource) const {
	return _paths.resourcePath(Common::String::format("%u", resource), ".ao");
}

Common::String ScriptSession::characterResource(uint resource, Shell::Character::Resources::Kind kind, uint part) const {
	Common::String suffix;
	if (kind == Shell::Character::Resources::kAnimation)
		suffix = ".ao";
	else if (kind == Shell::Character::Resources::kSound)
		suffix = ".wav";
	else if (kind == Shell::Character::Resources::kLips)
		suffix = part == 1 ? ".lips" : Common::String::format(".lip%u", part);
	else
		suffix = ".tc";
	return _paths.resourcePath(Common::String::format("%u", resource), suffix);
}

bool ScriptSession::installBackground(const Core::ScriptValue &image, const Core::ScriptValue &palette) {
	if (!_scene.setBackground(_paths.resourcePath(image.text(), ".jpg")))
		return false;
	if (800 < _scene.backgroundWidth())
		_scroller.start(_time);
	return true;
}

Common::String ScriptSession::musicResource(const Core::ScriptValue &id) const {
	return id.integer() == 0 && id.isNumeric() ? Common::String() : _paths.resourcePath(id.text(), ".wav");
}

void ScriptSession::freezeWindow(bool screenOnly) {
	_frozenScreen.copyFrom(_game._vm.gfx().surface());
	_frozen = true;
}

void ScriptSession::hideCursor(bool hidden) {
	_cursorHidden = hidden;
	CursorMan.showMouse(!hidden);
}

void ScriptSession::movieCreated(Shell::Movie &movie) {
	_movies.push_back(&movie);
}

void ScriptSession::movieDestroyed(Shell::Movie &movie) {
	for (uint i = 0; i < _movies.size(); i++) {
		if (_movies[i] == &movie) {
			_movies.remove_at(i);
			return;
		}
	}
}

void ScriptSession::moviePlaying(Shell::Movie &movie, bool playing) {
	if (playing)
		_movieTasks += 1;
	else if (_movieTasks)
		_movieTasks -= 1;
	_lastHostTime = g_system->getMillis();
	hideCursor(_movieTasks != 0);
}

Core::Trinket *ScriptSession::visualObject(uint32 reference) {
	static const char *const classes[] = {
		"RAnimation",
		"RFlyButton",
		"RQuitButton",
		"RText",
		"RHotSpot",
		"RAttributeCharacter",
		"RCharacter",
		"RCatapultGame",
		"RGlyphGame",
		"RIncubator",
		"RGardenGame",
		"RCorralGame",
		"RZIVGame",
	};
	for (const char *name : classes) {
		Core::ScriptableObject *object = _runtime.objects().resolve(reference, name);
		if (object)
			return static_cast<Core::Trinket *>(object);
	}
	return nullptr;
}

uint32 ScriptSession::hitObject(Common::Point point) {
	Shell::Sprite *sprite = _scene.hitSprite(point);
	Core::ScriptableObject *source = sprite;
	if (sprite && sprite->parent())
		source = sprite->parent();
	int depth = sprite ? sprite->z : INT_MIN;
	uint32 hit = 0;
	for (uint32 reference : _runtime.objects().references()) {
		Core::Trinket *object = visualObject(reference);
		if (!object || !object->enabled || !object->visible)
			continue;
		if (object == source) {
			hit = reference;
			continue;
		}
		if (!_runtime.objects().resolve(reference, "RHotSpot") && !_runtime.objects().resolve(reference, "RText"))
			continue;
		if (depth <= object->z && Common::Rect(object->x, object->y, object->x + object->width(), object->y + object->height()).contains(point)) {
			hit = reference;
			depth = object->z;
		}
	}
	return hit;
}

void ScriptSession::nativeEvent(const Common::Event &event) {
	for (uint32 reference : _runtime.objects().references()) {
		Core::ScriptableObject *object = _runtime.objects().resolve(reference);
		if (!object)
			continue;
		if (_runtime.objects().resolve(reference, "RCatapultGame")) {
			CatapultMachine *machine = static_cast<CatapultMachine *>(object);
			int action = _scene.event(event);
			if (event.type == Common::EVENT_LBUTTONDOWN && Common::Rect(44, 0, 78, 58).contains(event.mouse))
				action = CatapultMachine::kInsertMud01;
			if (action)
				machine->action(action);
		} else if (_runtime.objects().resolve(reference, "RGlyphGame")) {
			static_cast<GlyphMachine *>(object)->event(event, _time);
		} else if (_runtime.objects().resolve(reference, "RPlanetariumGame")) {
			static_cast<PlanetariumMachine *>(object)->event(event, _time);
		} else if (_runtime.objects().resolve(reference, "Greenhouse")) {
			static_cast<GreenhouseMachine *>(object)->event(event);
		} else if (_runtime.objects().resolve(reference, "RGardenGame")) {
			static_cast<GardenMachine *>(object)->event(event);
		} else if (_runtime.objects().resolve(reference, "RCorralGame")) {
			static_cast<CorralMachine *>(object)->event(event, _time);
		} else if (_runtime.objects().resolve(reference, "RBarnGame")) {
			static_cast<BarnMachine *>(object)->event(event, _time);
		}
	}
}

void ScriptSession::event(const Common::Event &event) {
	if (_failed)
		return;
	_runtime.setTime(_time);
	_actions.setTime(_time);
	_scheduler.resetIdle();
	if (_quitDialog) {
		QuitDialog::Result result = _quitDialog->event(event);
		if (result != QuitDialog::kPending) {
			_quitDialog.reset();
			_window.broadcast("resumed");
			if (result == QuitDialog::kQuit00) {
				_window.broadcast("exittingGame");
				quit();
			}
		}
		return;
	}
	Common::Array<uint32> options = _runtime.objects().references("ROptions");
	if (!options.empty()) {
		OptionsObject *object = static_cast<OptionsObject *>(_runtime.objects().resolve(options.back(), "ROptions"));
		if (object)
			object->event(event, _time);
		return;
	}
	if (event.type == Common::EVENT_MOUSEMOVE || event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP)
		_mouse = event.mouse;
	if (_window.event(event))
		return;
	if (_movieTasks) {
		if (event.type == Common::EVENT_LBUTTONDOWN && !_movies.empty())
			_movies.back()->emitEvent("mouseDown");
		return;
	}
	_scroller.event(event);
	if (event.type != Common::EVENT_LBUTTONDOWN && event.type != Common::EVENT_LBUTTONUP && event.type != Common::EVENT_MOUSEMOVE)
		return;
	Core::ScriptableObject *tracked = _runtime.objects().resolve(_tracked);
	AttributeCharacter *trackedCharacter = static_cast<AttributeCharacter *>(_runtime.objects().resolve(_tracked, "RAttributeCharacter"));
	if (trackedCharacter && !trackedCharacter->isDragging()) {
		_tracked = _pressed = 0;
		tracked = nullptr;
	}
	if (tracked) {
		AttributeCharacter *character = static_cast<AttributeCharacter *>(_runtime.objects().resolve(_tracked, "RAttributeCharacter"));
		Shell::Sprite *animation = _scene.resolveSprite(tracked);
		if (event.type == Common::EVENT_MOUSEMOVE) {
			if (character)
				character->dragTo(event.mouse);
			else if (animation && animation->movable) {
				Common::Point position = event.mouse - _dragOffset;
				if (!animation->corral.isEmpty()) {
					position.x = CLIP<int>(position.x, animation->corral.left, animation->corral.right - animation->width());
					position.y = CLIP<int>(position.y, animation->corral.top, animation->corral.bottom - animation->height());
				}
				animation->x = position.x;
				animation->y = position.y;
			}
			return;
		}
		if ((character && event.type == Common::EVENT_LBUTTONUP) || (animation &&
																	 (event.type == Common::EVENT_LBUTTONDOWN || (event.type == Common::EVENT_LBUTTONUP && !Common::Rect(800, 600).contains(event.mouse))))) {
			_tracked = 0;
			_pressed = 0;
			if (character)
				character->endDrag();
			else {
				tracked->emitEvent("mouseUp");
				tracked->emitEvent("endTracking");
			}
		}
		return;
	}
	uint32 hit = hitObject(event.mouse);
	if (_hovered != hit && !_pressed) {
		Core::ScriptableObject *old = _runtime.objects().resolve(_hovered);
		if (Shell::Sprite *sprite = _scene.resolveSprite(old))
			sprite->hovered = false;
		if (old)
			old->emitEvent("flyOff");
		_hovered = hit;
		Core::ScriptableObject *current = _runtime.objects().resolve(hit);
		if (Shell::Sprite *sprite = _scene.resolveSprite(current))
			sprite->flyOn();
		if (current)
			current->emitEvent("flyOn");
	}
	Core::ScriptableObject *object = _runtime.objects().resolve(hit);
	if (_runtime.objects().resolve(hit, "RCatapultGame") || _runtime.objects().resolve(hit, "RGlyphGame") ||
		_runtime.objects().resolve(hit, "RPlanetariumGame") || _runtime.objects().resolve(hit, "RGardenGame") ||
		_runtime.objects().resolve(hit, "RCorralGame") || _runtime.objects().resolve(hit, "RBarnGame"))
		object = nullptr;
	Shell::Sprite *sprite = _scene.resolveSprite(object);
	if (event.type == Common::EVENT_LBUTTONDOWN && object) {
		_pressed = hit;
		object->setCallTime(_time);
		AttributeCharacter *character = static_cast<AttributeCharacter *>(_runtime.objects().resolve(hit, "RAttributeCharacter"));
		if (character && character->beginDrag(event.mouse, _time))
			_tracked = hit;
		else if (sprite && sprite->button)
			sprite->buttonPressed(_time);
		else if (sprite && sprite->tracking) {
			_tracked = hit;
			_dragOffset = event.mouse - Common::Point(sprite->x, sprite->y);
			sprite->emitEvent("startTracking");
		}
		object->emitEvent("mouseDown");
		return;
	}
	Core::ScriptableObject *pressed = _runtime.objects().resolve(_pressed);
	Shell::Sprite *button = _scene.resolveSprite(pressed);
	if (event.type == Common::EVENT_MOUSEMOVE && button && button->button) {
		bool inside = hit == _pressed;
		if (inside && !button->pressed)
			button->buttonPressed(_time);
		button->pressed = inside;
		return;
	}
	if (event.type == Common::EVENT_LBUTTONUP && pressed) {
		if (button && button->button) {
			button->pressed = false;
			button->hovered = hit == _pressed;
			if (hit == _pressed) {
				if (_runtime.objects().resolve(_pressed, "RQuitButton"))
					openQuit();
				else
					button->emitEvent("hit");
			}
		} else {
			pressed->emitEvent("mouseUp");
		}
		_pressed = 0;
		return;
	}
	if (event.type != Common::EVENT_LBUTTONDOWN || !object)
		nativeEvent(event);
	if (event.type == Common::EVENT_LBUTTONDOWN && !object && !_scene.hitSprite(event.mouse))
		_window.backgroundClicked();
}

void ScriptSession::openQuit() {
	if (_quitDialog)
		return;
	Common::ScopedPtr<QuitDialog> dialog(new QuitDialog(_paths, _game._vm.mixer()));
	if (!dialog->load()) {
		fail("quit dialog resources");
		return;
	}
	_window.broadcast("paused");
	_quitDialog.reset(dialog.release());
}

bool ScriptSession::hasModal() const {
	return _movieTasks || _quitDialog || !_runtime.objects().references("ROptions").empty() ||
		   _runtime.variable("gHelpHS").kind() == Core::ScriptValue::kObject || _runtime.variable("lockScreen").kind() == Core::ScriptValue::kObject;
}

void ScriptSession::syncSoundSettings() {
	for (uint32 reference : _runtime.objects().references("ROptions"))
		static_cast<OptionsObject *>(_runtime.objects().resolve(reference, "ROptions"))->syncVolumes();
}

bool ScriptSession::installCursors() {
	if (_cursors[0].getPixels())
		return true;
	for (uint i = 0; i < ARRAYSIZE(_cursors); i++) {
		Common::ScopedPtr<Common::SeekableReadStream> stream(_paths.open(Common::String::format("/z3common/%u.ao", 301 + i)));
		Core::Animation animation;
		if (!stream || !animation.load(*stream) || animation.frameCount() != 1 || !animation.decode(0, _cursors[i])) {
			warning("Trinket: could not load startup cursor z3common/%u.ao", 301 + i);
			return false;
		}
	}
	return true;
}

void ScriptSession::updateCursor() {
	CursorMan.showMouse(!_cursorHidden);
	int cursor = 0;
	Core::Trinket *object = visualObject(_tracked ? _tracked : hitObject(_mouse));
	if (object) {
		int id = atoi((_tracked ? object->grabCursor : object->spotCursor).c_str());
		if (301 <= id && id <= 306)
			cursor = id - 301;
	}
	for (uint32 reference : _runtime.objects().references("RGlyphGame")) {
		GlyphMachine *machine = static_cast<GlyphMachine *>(_runtime.objects().resolve(reference, "RGlyphGame"));
		if (machine->dragging())
			cursor = 3;
		else if (0 <= machine->hitTile(_mouse))
			cursor = 4;
	}
	for (uint32 reference : _runtime.objects().references("RPlanetariumGame")) {
		PlanetariumMachine *machine = static_cast<PlanetariumMachine *>(_runtime.objects().resolve(reference, "RPlanetariumGame"));
		if (machine->dragging())
			cursor = 3;
		else if (0 <= machine->hitPiece(_mouse))
			cursor = 4;
	}
	for (uint32 reference : _runtime.objects().references("Greenhouse")) {
		GreenhouseMachine *machine = static_cast<GreenhouseMachine *>(_runtime.objects().resolve(reference, "Greenhouse"));
		if (machine->isDragging())
			cursor = 3;
		else if (machine->overDraggable(_mouse))
			cursor = 4;
	}
	for (uint32 reference : _runtime.objects().references("RCorralGame")) {
		CorralMachine *machine = static_cast<CorralMachine *>(_runtime.objects().resolve(reference, "RCorralGame"));
		if (machine->isDragging())
			cursor = 3;
		else if (machine->overDraggable(_mouse))
			cursor = 4;
	}
	for (uint32 reference : _runtime.objects().references("RBarnGame")) {
		BarnMachine *machine = static_cast<BarnMachine *>(_runtime.objects().resolve(reference, "RBarnGame"));
		if (machine->isDragging())
			cursor = 3;
		else if (machine->overDraggable(_mouse))
			cursor = 4;
	}
	if (_quitDialog)
		cursor = _quitDialog->overButton(_mouse) ? 2 : 0;
	for (uint32 reference : _runtime.objects().references("ROptions")) {
		OptionsObject *options = static_cast<OptionsObject *>(_runtime.objects().resolve(reference, "ROptions"));
		if (options)
			cursor = options->overControl(_mouse) ? 2 : 0;
	}
	if (_cursor == cursor)
		return;
	static constexpr int hotspotX[] = {
		1,
		1,
		3,
		7,
		11,
		15,
	},
						 hotspotY[] = {
							 1,
							 1,
							 1,
							 5,
							 6,
							 15,
						 };
	if (_cursor < 0)
		CursorMan.pushCursor(_cursors[cursor], hotspotX[cursor], hotspotY[cursor], 0);
	else
		CursorMan.replaceCursor(_cursors[cursor], hotspotX[cursor], hotspotY[cursor], 0);
	_cursor = cursor;
}

void ScriptSession::updateNative() {
	Common::Array<uint16> sounds;
	for (uint32 reference : _runtime.objects().references("RAttributeCharacter")) {
		AttributeCharacter *character = static_cast<AttributeCharacter *>(_runtime.objects().resolve(reference, "RAttributeCharacter"));
		if (!character)
			continue;
		character->update(_time, sounds);
		character = static_cast<AttributeCharacter *>(_runtime.objects().resolve(reference, "RAttributeCharacter"));
		if (character && character->hasResourceError())
			fail("attribute character resources");
	}
	for (uint32 reference : _runtime.objects().references("RCharacter")) {
		Shell::Character *character = static_cast<Shell::Character *>(_runtime.objects().resolve(reference, "RCharacter"));
		if (character)
			character->update(_time, sounds);
	}
	bool sceneUpdated = false;
	for (uint32 reference : _runtime.objects().references()) {
		Core::ScriptableObject *object = _runtime.objects().resolve(reference);
		if (!object)
			continue;
		uint16 sound;
		if (_runtime.objects().resolve(reference, "RCatapultGame")) {
			CatapultMachine *machine = static_cast<CatapultMachine *>(object);
			machine->update(_time);
			sceneUpdated = true;
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Catapult resources");
			bool chute = machine->chuteMoving(), gears = machine->machineRunning();
			bool paused = _runtime.variable("gPauseDepth").integer() != 0;
			_chuteSound.pause(paused);
			_gearSound.pause(paused);
			if (paused)
				continue;
			if (chute && !_chuteSound.isPlaying())
				_chuteSound.play(_paths, _paths.resourcePath(Common::String::format("%u", CatapultMachine::kResSound10400_ChuteLoop), ".wav"),
								 Audio::Mixer::kSFXSoundType, 0);
			if (!chute)
				_chuteSound.stop();
			if (gears && !_gearSound.isPlaying())
				_gearSound.play(_paths, _paths.resourcePath(Common::String::format("%u", CatapultMachine::kResSound10409_GearLoop), ".wav"),
								Audio::Mixer::kSFXSoundType, 0);
			if (!gears)
				_gearSound.stop();
		} else if (_runtime.objects().resolve(reference, "RGlyphGame")) {
			GlyphMachine *machine = static_cast<GlyphMachine *>(object);
			machine->update(_time);
			sceneUpdated = true;
			int channel;
			for (auto &channelSound : _glyphSounds)
				channelSound->pause(_runtime.variable("gPauseDepth").integer() != 0);
			while (machine->pollSound(sound, &channel)) {
				Common::String resource = _paths.resourcePath(Common::String::format("%u", sound), ".wav");
				if (0 <= channel && static_cast<uint>(channel) < ARRAYSIZE(_glyphSounds))
					_glyphSounds[channel]->play(_paths, resource, Audio::Mixer::kSFXSoundType);
				else
					playSound(Common::String::format("%u", sound));
			}
			if (machine->hasResourceError())
				fail("Glyph resources");
		} else if (_runtime.objects().resolve(reference, "RPlanetariumGame")) {
			PlanetariumMachine *machine = static_cast<PlanetariumMachine *>(object);
			machine->update(_time);
			sceneUpdated = true;
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Planetarium resources");
		} else if (_runtime.objects().resolve(reference, "RIncubator")) {
			static_cast<Incubator *>(object)->update(_time);
		} else if (_runtime.objects().resolve(reference, "Greenhouse")) {
			GreenhouseMachine *machine = static_cast<GreenhouseMachine *>(object);
			machine->update(_time);
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Greenhouse resources");
		} else if (_runtime.objects().resolve(reference, "RGardenGame")) {
			GardenMachine *machine = static_cast<GardenMachine *>(object);
			machine->update(_time);
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Garden resources");
		} else if (_runtime.objects().resolve(reference, "RCorralGame")) {
			CorralMachine *machine = static_cast<CorralMachine *>(object);
			machine->update(_time);
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Corral resources");
		} else if (_runtime.objects().resolve(reference, "RBarnGame")) {
			BarnMachine *machine = static_cast<BarnMachine *>(object);
			machine->update(_time);
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Barn resources");
		} else if (_runtime.objects().resolve(reference, "RZIVGame")) {
			static_cast<ZIVMachine *>(object)->update(_time);
		}
	}
	if (!sceneUpdated)
		_scene.update(_time, sounds);
	for (uint16 sound : sounds)
		playSound(Common::String::format("%u", sound));
	for (uint32 reference : _runtime.objects().references()) {
		Shell::Sprite *sprite = _scene.resolveSprite(_runtime.objects().resolve(reference));
		Common::String sound;
		if (sprite && sprite->pollClickSound(sound))
			playSound(sound);
	}
}

void ScriptSession::draw(OM::Canvas &canvas, uint32 time) {
	if (_failed)
		return;
	bool exclusive = _movieTasks != 0;
	if (!exclusive)
		_time += time - _lastHostTime;
	_lastHostTime = time;
	_runtime.setTime(_time);
	_actions.setTime(_time);
	if (!exclusive) {
		_actions.update(_time);
		_scroller.freeze(_runtime.variable("gPauseDepth").integer() != 0, _time);
		_scroller.update(_time);
		updateNative();
	}
	for (uint i = 0; i < _movies.size(); i++)
		_movies[i]->update();
	if (!_scheduler.tick(_time)) {
		fail("scheduler");
		return;
	}
	for (uint i = 0; i < _sounds.size(); i++) {
		if (_sounds[i] && !_sounds[i]->isPlaying()) {
			delete _sounds[i];
			_sounds[i] = nullptr;
		}
	}
	updateCursor();
	if (_frozen && _frozenScreen.getPixels()) {
		canvas.blit(_frozenScreen, 0, 0);
	} else {
		_scene.drawBackground(canvas);
		struct Visual {
			uint32 reference;
			int depth;
		};
		Common::Array<Visual> visuals;
		for (uint32 reference : _runtime.objects().references()) {
			Core::Trinket *object = visualObject(reference);
			if (!object || (!_runtime.objects().resolve(reference, "RText") && !_runtime.objects().resolve(reference, "RHotSpot")))
				continue;
			Visual visual = {reference, object->z};
			uint index = 0;
			while (index < visuals.size() && visuals[index].depth <= visual.depth)
				index += 1;
			visuals.insert_at(index, visual);
		}
		int first = INT_MIN;
		for (const Visual &visual : visuals) {
			_scene.drawSprites(canvas, first, visual.depth);
			if (Shell::TextObject *text = static_cast<Shell::TextObject *>(_runtime.objects().resolve(visual.reference, "RText")))
				text->draw(canvas);
			else if (Shell::HotSpot *spot = static_cast<Shell::HotSpot *>(_runtime.objects().resolve(visual.reference, "RHotSpot")))
				spot->draw(canvas);
			first = visual.depth == INT_MAX ? INT_MAX : visual.depth + 1;
		}
		_scene.drawSprites(canvas, first, INT_MAX);
	}
	for (Shell::Movie *movie : _movies)
		movie->draw(canvas);
	for (uint32 reference : _runtime.objects().references("ROptions")) {
		OptionsObject *options = static_cast<OptionsObject *>(_runtime.objects().resolve(reference, "ROptions"));
		if (options)
			options->draw(canvas, _time);
	}
	if (_quitDialog)
		_quitDialog->draw(canvas, _time);
}

Common::String ScriptSession::debugCommand(const Common::String &command) {
	if (hasModal())
		return "Close the dialog or wait for the transition first.\n";
	int location = _game._world.currentLocation();
	uint level = kCatapult01 <= location && location <= kBarn07 ? _game._world.level(location - kCatapult01) : 0;
	uint moved = _runtime.variable("gTokensMoved").integer(), party = _runtime.variable("tokens").integer();
	if (!party)
		party = _runtime.variable("curTokenCount").integer();
	if (command == "finish") {
		if (location < kCatapult01 || kBarn07 < location)
			return "No puzzle is active.\n";
		if (!_game._practice) {
			int token;
			while (0 <= (token = _game._world.nextToken(-1, location))) {
				if (location == kCorral06) {
					Common::Array<uint32> games = _runtime.objects().references("RCorralGame");
					if (!games.empty()) {
						const CorralRules &rules = static_cast<CorralMachine *>(_runtime.objects().resolve(games[0], "RCorralGame"))->rules();
						if (moved < rules.party()) {
							_game._world.setTokenAttribute(token, 0, 0);
							_game._world.setTokenAttribute(token, 1, rules.zerble(moved).feet);
							_game._world.setTokenAttribute(token, 2, rules.zerble(moved).tail);
						}
					}
				}
				if (!_game._world.moveToken(token))
					break;
				moved += 1;
			}
		}
		_runtime.setVariable("gTokensMoved", Core::ScriptValue(static_cast<int32>(MAX(moved, party))));
		_runtime.setVariable("gNextLoc", _runtime.variable(_game._practice ? "kMapLocation" : "kNextLocation"));
		if (!_runtime.runProcedure("gSceneTransitionMovie"))
			return "Debug departure failed: " + _runtime.lastError() + "\n";
		return "Debug completion: party accepted; departure requested without awarding puzzle score.\n";
	}
	for (uint32 reference : _runtime.objects().references()) {
		Core::ScriptableObject *object = _runtime.objects().resolve(reference);
		if (!object)
			continue;
		if (_runtime.objects().resolve(reference, "RCatapultGame")) {
			CatapultMachine *machine = static_cast<CatapultMachine *>(object);
			if (command == "answer")
				return machine->debugAnswer();
			if (command == "chance" || command == "chances")
				return machine->debugChance();
		} else if (_runtime.objects().resolve(reference, "RGlyphGame")) {
			GlyphMachine *machine = static_cast<GlyphMachine *>(object);
			if (command == "answer")
				return machine->debugAnswer(level);
			if (command == "chance" || command == "chances") {
				uint tries = _runtime.variable("triesMade").integer(), maximum = _runtime.variable("maxTries").integer();
				return Common::String::format("The Wall: %u mistakes remaining (%u used of %u); %u of %u party members passed.\n",
											  maximum > tries ? maximum - tries : 0, tries, maximum, moved, party);
			}
		} else if (_runtime.objects().resolve(reference, "RPlanetariumGame")) {
			PlanetariumMachine *machine = static_cast<PlanetariumMachine *>(object);
			if (command == "answer")
				return machine->debugAnswer(level);
			if (command == "chance" || command == "chances") {
				uint remaining[2][2] = {};
				for (uint i = 0; i < machine->rules().pieceCount(); i++) {
					const PlanetariumRules::Piece &piece = machine->rules().piece(i);
					if (piece.enabled && !piece.consumed)
						remaining[piece.exhibit][piece.type] += 1;
				}
				return Common::String::format("Planetarium: left %u coins/%u bills, right %u coins/%u bills; %u of %u party members passed.\n",
											  remaining[0][0], remaining[0][1], remaining[1][0], remaining[1][1], moved, party);
			}
		} else if (_runtime.objects().resolve(reference, "Greenhouse")) {
			GreenhouseMachine *machine = static_cast<GreenhouseMachine *>(object);
			if (command == "answer")
				return machine->debugAnswer();
			if (command == "chance" || command == "chances")
				return Common::String::format("Greenhouse: %u of 12 moths passed; %u wand swaps.\n", moved, machine->rules().remainingSwaps());
		} else if (_runtime.objects().resolve(reference, "RGardenGame")) {
			GardenMachine *machine = static_cast<GardenMachine *>(object);
			if (command == "answer")
				return machine->debugAnswer();
			if (command == "chance" || command == "chances")
				return Common::String::format("Garden: %u of %u plants accepted; %u rejections remaining%s.\n",
											  moved, party, machine->rules().chances(), machine->returnPending() ? " (one return pending)" : "");
		} else if (_runtime.objects().resolve(reference, "RCorralGame")) {
			CorralMachine *machine = static_cast<CorralMachine *>(object);
			if (command == "answer")
				return machine->debugAnswer();
			if (command == "chance" || command == "chances")
				return Common::String::format("Corral: %u of %u zerbles fed; %u fruit remaining; %d projector charges%s.\n",
											  moved, machine->rules().party(), machine->rules().fruitRemaining(), machine->rules().turns(),
											  machine->feedingPending() ? " (feeding pending)" : "");
		} else if (_runtime.objects().resolve(reference, "RBarnGame")) {
			BarnMachine *machine = static_cast<BarnMachine *>(object);
			if (command == "answer")
				return machine->debugAnswer();
			if (command == "chance" || command == "chances")
				return Common::String::format("Barn: %u of %u parents paired; %u incorrect pairings remaining; %u water drains%s.\n",
											  moved, machine->rules().party(), machine->rules().chances(), machine->drained(),
											  machine->drained() < machine->rules().wrong() ? " (one drain pending removal)" : "");
		}
	}
	if (command == "chance" || command == "chances")
		return Common::String::format("%s: %u of %u party members passed.\n", _scriptName.c_str(), moved, party);
	return Common::String::format("MPS %s: %s\n", _scriptName.c_str(), _runtime.lastError().empty() ? "running" : _runtime.lastError().c_str());
}

void ScriptSession::fail(const Common::String &context) {
	_failed = true;
	warning("Trinket MPS %s: %s: %s", _scriptName.c_str(), context.c_str(), _runtime.lastError().c_str());
}

void ScriptSession::quit() {
	_game._quitRequested = true;
}

} // namespace Zoombini3
} // namespace Trinket
