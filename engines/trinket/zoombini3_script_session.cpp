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
#include "trinket/string_parser.h"
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

constexpr const char *ScriptSession::kEventNewGame;
constexpr const char *ScriptSession::kEventGameSelected;
constexpr const char *ScriptSession::kEventCheatChange;
constexpr const char *ScriptSession::kEventClosed;
constexpr const char *ScriptSession::kPropertySelectedGame;
constexpr const char *ScriptSession::kPropertyReplaceSelectedGame;
constexpr const char *ScriptSession::kEventSpeechVolumeChanged;
constexpr const char *ScriptSession::kEventSfxVolumeChanged;
constexpr const char *ScriptSession::kRAnimationName;
constexpr const char *ScriptSession::kRFlyButtonName;
constexpr const char *ScriptSession::kRQuitButtonName;
constexpr const char *ScriptSession::kRTextName;
constexpr const char *ScriptSession::kRXMLFileName;
constexpr const char *ScriptSession::kROptionsName;
constexpr const char *ScriptSession::kRAttributeCharacterName;
constexpr const char *ScriptSession::kRCharacterName;
constexpr const char *ScriptSession::kRCatapultGameName;
constexpr const char *ScriptSession::kRGlyphGameName;
constexpr const char *ScriptSession::kRPlanetariumGameName;
constexpr const char *ScriptSession::kRIncubatorName;
constexpr const char *ScriptSession::kGreenhouseName;
constexpr const char *ScriptSession::kRGardenGameName;
constexpr const char *ScriptSession::kRCorralGameName;
constexpr const char *ScriptSession::kRBarnGameName;
constexpr const char *ScriptSession::kRZIVGameName;
constexpr const char *ScriptSession::kRHotSpotName;
constexpr const char *ScriptSession::kInGameName;
constexpr const char *ScriptSession::kPreviousLocationName;
constexpr const char *ScriptSession::kPropertyIsAutoLevelingGlobal;
constexpr const char *ScriptSession::kDebugModeName;
constexpr const char *ScriptSession::kClassRScenePort;
constexpr const char *ScriptSession::kClassRResourcePort;
constexpr const char *ScriptSession::kClassRWorldPort;
constexpr const char *ScriptSession::kClassRBinkMovie;
constexpr const char *ScriptSession::kClassRRadioGroup;
constexpr const char *ScriptSession::kClassRQueue;
constexpr const char *ScriptSession::kClassRCompositeAction;
constexpr const char *ScriptSession::kCtrlOName;
constexpr const char *ScriptSession::kOptionsKeyPressedName;
constexpr const char *ScriptSession::kCmdOName;
constexpr const char *ScriptSession::k301Name;
constexpr const char *ScriptSession::k304Name;
constexpr const char *ScriptSession::kLauncherName;
constexpr const char *ScriptSession::kPropertyPlayerName;
constexpr const char *ScriptSession::kPropertyIsNewPlayer;
constexpr const char *ScriptSession::kPropertyAutoEnabled;
constexpr const char *ScriptSession::kPropertyBackgroundMusicVolume;
constexpr const char *ScriptSession::kPropertyDialogVolume;
constexpr const char *ScriptSession::kPropertySfxVolume;
constexpr const char *ScriptSession::kPropertyIsBackgroundMusicOff;
constexpr const char *ScriptSession::kMethodAddIntProperty;
constexpr const char *ScriptSession::kMethodAddStringProperty;
constexpr const char *ScriptSession::kMethodActivatePlayer;
constexpr const char *ScriptSession::kMethodActivateGame;
constexpr const char *ScriptSession::kMethodAddGame;
constexpr const char *ScriptSession::kMethodUncacheResources;
constexpr const char *ScriptSession::kMethodDeactivateAllDLLs;
constexpr const char *ScriptSession::kMethodUncacheAllDLLs;
constexpr const char *ScriptSession::kMessageBoxName;
constexpr const char *ScriptSession::kRandomNumberName;
constexpr const char *ScriptSession::kGetLauncherXMLFilenameName;
constexpr const char *ScriptSession::kReadININame;
constexpr const char *ScriptSession::kAddPathName;
constexpr const char *ScriptSession::kUseName;
constexpr const char *ScriptSession::kRemoveAllPathsName;
constexpr const char *ScriptSession::kBulkFileName;
constexpr const char *ScriptSession::kIndexFileName;
constexpr const char *ScriptSession::kInstallCursorsName;
constexpr const char *ScriptSession::kStartTransitionName;
constexpr const char *ScriptSession::kEndTransitionName;
constexpr const char *ScriptSession::kSetClosedCaptioningActiveName;
constexpr const char *ScriptSession::kSetDoubleClicksEnabledName;
constexpr const char *ScriptSession::kPlaySoundName;
constexpr const char *ScriptSession::kPauseSoundName;
constexpr const char *ScriptSession::kResumeSoundName;
constexpr const char *ScriptSession::kKillSoundName;
constexpr const char *ScriptSession::kIntersectTestName;
constexpr const char *ScriptSession::kAttribChWalkActionName;
constexpr const char *ScriptSession::kRandomDelayActionName;
constexpr const char *ScriptSession::kPlayAnimActionName;
constexpr const char *ScriptSession::kAnimActionName;
constexpr const char *ScriptSession::kCharacterSpeechActionName;
constexpr const char *ScriptSession::kCharacterAnimActionName;
constexpr const char *ScriptSession::kSoundActionName;
constexpr const char *ScriptSession::kLoopedSoundActionName;
constexpr const char *ScriptSession::kSpeechActionName;
constexpr const char *ScriptSession::kEventResumed;
constexpr const char *ScriptSession::kEventExittingGame;
constexpr const char *ScriptSession::kEventMouseDown;
constexpr const char *ScriptSession::kEventMouseUp;
constexpr const char *ScriptSession::kEventEndTracking;
constexpr const char *ScriptSession::kEventFlyOff;
constexpr const char *ScriptSession::kEventFlyOn;
constexpr const char *ScriptSession::kEventStartTracking;
constexpr const char *ScriptSession::kEventHit;
constexpr const char *ScriptSession::kEventPaused;
constexpr const char *ScriptSession::kGHelpHSName;
constexpr const char *ScriptSession::kLockScreenName;
constexpr const char *ScriptSession::kGPauseDepthName;
constexpr const char *ScriptSession::kCacheName;
constexpr const char *ScriptSession::kGTokensMovedName;
constexpr const char *ScriptSession::kTokensName;
constexpr const char *ScriptSession::kCurTokenCountName;
constexpr const char *ScriptSession::kFinishName;
constexpr const char *ScriptSession::kGNextLocName;
constexpr const char *ScriptSession::kMapLocationKey;
constexpr const char *ScriptSession::kNextLocationKey;
constexpr const char *ScriptSession::kGSceneTransitionMovieName;
constexpr const char *ScriptSession::kAnswerName;
constexpr const char *ScriptSession::kChanceName;
constexpr const char *ScriptSession::kChancesName;
constexpr const char *ScriptSession::kTriesMadeName;
constexpr const char *ScriptSession::kMaxTriesName;
constexpr const char *ScriptSession::kRunningName;

constexpr const char *ScriptSession::kStartupFont;
constexpr const char *ScriptSession::kCursorFormat;
constexpr const char *ScriptSession::kMainScript;
constexpr const char *ScriptSession::kScriptExtension;
constexpr const char *ScriptSession::kAnimationExtension;
constexpr const char *ScriptSession::kSoundExtension;
constexpr const char *ScriptSession::kBackgroundExtension;
constexpr const char *ScriptSession::kLipsExtension;
constexpr const char *ScriptSession::kLipsExtensionFormat;
constexpr const char *ScriptSession::kBodyExtension;
constexpr const char *ScriptSession::kCreditsMusicID;

class ScriptSession::OptionsObject : public Core::ScriptableObject {
public:
	explicit OptionsObject(ScriptSession &session) : _session(session),
													 _options(session._paths, session._game._vm.fonts(), session._game._vm.mixer(), session._game._world) {
		addEvent(kEventNewGame);
		addEvent(kEventGameSelected);
		addEvent(kEventCheatChange);
		addEvent(kEventClosed);
	}
	bool load() {
		const Shell::SaveFiles::Player *player = _session._game._vm.activePlayer();
		return player && _options.load(_session._game._practice, _session._game._vm.music().volume(), *player);
	}
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override {
		if (property.equals(kPropertySelectedGame))
			result = Core::ScriptValue(static_cast<int32>(_selected));
		else if (property.equals(kPropertyReplaceSelectedGame))
			result = Core::ScriptValue(static_cast<int32>(_replace));
		else
			return ScriptableObject::get(property, sargs, result);
		return true;
	}
	void event(const Common::Event &event, uint32 time) {
		Options::Result result = _options.event(event, time);
		if (result.volumeChanges) {
			_session._game.applySoundSettings();
			if (result.volumeChanges & Options::kDialogue02)
				_session._window.broadcast(kEventSpeechVolumeChanged);
			if (result.volumeChanges & Options::kEffects04)
				_session._window.broadcast(kEventSfxVolumeChanged);
		}
		if (result.music == Options::kStartCredits01) {
			_savedMusic = _session._game._vm.music().resource();
			_session._game._vm.music().setResource(_session._paths.resourcePath(kCreditsMusicID, kSoundExtension));
		} else if (result.music == Options::kRestoreMusic02) {
			_session._game._vm.music().setResource(_savedMusic);
		}
		_selected = result.selectedSlot;
		_replace = result.replaceSelGame;
		if (result.action == Options::kNewGame01)
			emitEvent(kEventNewGame);
		else if (result.action == Options::kLoadGame02)
			emitEvent(kEventGameSelected);
		else if (result.action == Options::kClose05)
			emitEvent(kEventClosed);
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
										   _queueFactory(_actions, false), _compositeFactory(_actions, true), _sceneFactory(_window), _resFactory(*this),
										   _worldFactory(game._world, *this), _movieFactory(*this), _radioFactory(_runtime),
										   _chuteSound(game._vm.mixer()), _gearSound(game._vm.mixer()) {
	static constexpr const char *classes[] = {
		kRAnimationName,
		kRFlyButtonName,
		kRQuitButtonName,
		kRTextName,
		kRXMLFileName,
		kROptionsName,
		kRAttributeCharacterName,
		kRCharacterName,
		kRCatapultGameName,
		kRGlyphGameName,
		kRPlanetariumGameName,
		kRIncubatorName,
		kGreenhouseName,
		kRGardenGameName,
		kRCorralGameName,
		kRBarnGameName,
		kRZIVGameName,
		kRHotSpotName,
	};
	for (const char *name : classes)
		_factories.push_back(new NativeFactory(*this, name));
	for (auto &sound : _glyphSounds)
		sound.reset(new OM::Sound(game._vm.mixer()));
	_transient[kInGameName] = Core::ScriptValue(game._practice ? 0 : 1);
	_transient[kPreviousLocationName] = Core::ScriptValue(Common::String());
	_transient[kPropertyIsAutoLevelingGlobal] = Core::ScriptValue(0);
	_transient[kDebugModeName] = Core::ScriptValue(0);
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
	_runtime.objects().registerClass(kClassRScenePort, _sceneFactory);
	_runtime.objects().registerClass(kClassRResourcePort, _resFactory);
	_runtime.objects().registerClass(kClassRWorldPort, _worldFactory);
	_runtime.objects().registerClass(kClassRBinkMovie, _movieFactory);
	_runtime.objects().registerClass(kClassRRadioGroup, _radioFactory);
	_runtime.objects().registerClass(kClassRQueue, _queueFactory);
	_runtime.objects().registerClass(kClassRCompositeAction, _compositeFactory);
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
		_transient[kPropertyIsAutoLevelingGlobal] = Core::ScriptValue(1);
		_window.addKeyEvent(kCtrlOName, kOptionsKeyPressedName);
		_window.addKeyEvent(kCmdOName, kOptionsKeyPressedName);
	}
	_font = _game._vm.fonts().load(kStartupFont);
	if (!_font) {
		warning("Trinket: could not load the startup font z3common/351.nfnt");
		return false;
	}
	if (!installCursors())
		return false;
	Common::String script = fromLauncher ? kMainScript : WorldPort::nameForLocation(_game._world.currentLocation()) + kScriptExtension;
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
	_game._vm.fonts().removeUnused();
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

Core::ScriptableObject *ScriptSession::NativeFactory::create(const Core::ScriptArguments &sargs) {
	return _session.createNative(_name, sargs);
}

Core::ScriptableObject *ScriptSession::createNative(const Common::String &name, const Core::ScriptArguments &sargs) {
	if (name.equals(kRAnimationName) || name.equals(kRFlyButtonName) || name.equals(kRQuitButtonName)) {
		bool button = !name.equals(kRAnimationName);
		if (sargs.size() != 1 && sargs.size() != 3)
			return nullptr;
		uint resource = !button && sargs.size() == 3 ? 2 : 0;
		Common::String path = _paths.resourcePath(sargs[resource].toText(), kAnimationExtension);
		Shell::Sprite *sprite = _scene.add(path, 0, 0, button);
		if (!sprite || !sprite->startAnimation()) {
			warning("Trinket: could not create %s animation '%s' (resolved path '%s')", name.c_str(), sargs[resource].toText().c_str(), path.c_str());
			return nullptr;
		}
		sprite->setFrame(1);
		if (!button)
			sprite->setRepeat(-1);
		sprite->flyButton = button;
		sprite->spotCursor = k301Name;
		sprite->grabCursor = k304Name;
		if (sargs.size() == 3) {
			uint first = button ? 1 : 0;
			if (sargs[first].toInt32() != 11111)
				sprite->x = sargs[first].toInt32();
			if (sargs[first + 1].toInt32() != 11111)
				sprite->y = sargs[first + 1].toInt32();
		}
		return sprite;
	}
	if (name.equals(kRTextName)) {
		Common::ScopedPtr<Shell::TextObject> text(new Shell::TextObject(_paths, _game._vm.fonts(), _font));
		return text->create(sargs) ? text.release() : nullptr;
	}
	if (name.equals(kRXMLFileName) && sargs.size() == 1) {
		Common::ScopedPtr<Shell::XMLFile> file(new Shell::XMLFile());
		if (sargs[0].toText().equals(kLauncherName)) {
			Common::String player = OM::XMLParser::escape(_launcherPlayer);
			if (!file->loadText("<?xml version=\"1.0\"?><launcher><command>play</command><user><name>\"" + player + "\"</name></user></launcher>"))
				return nullptr;
		} else if (!file->load(sargs[0].toText())) {
			return nullptr;
		}
		return file.release();
	}
	if (name.equals(kRHotSpotName)) {
		Shell::HotSpot::Factory factory;
		return factory.create(sargs);
	}
	if (name.equals(kROptionsName)) {
		Common::ScopedPtr<OptionsObject> options(new OptionsObject(*this));
		return options->load() ? options.release() : nullptr;
	}
	if (name.equals(kRAttributeCharacterName)) {
		Common::ScopedPtr<AttributeCharacter> character(new AttributeCharacter(_game._vm, _scene));
		if (!character->create(sargs, _time))
			return nullptr;
		character->setAnimationResources(this);
		return character.release();
	}
	if (name.equals(kRCharacterName) && (sargs.size() == 1 || sargs.size() == 2 || sargs.size() == 4)) {
		Common::Point position(11111, 11111);
		int depth = sargs.size() == 2 ? sargs[1].toInt32() : 0;
		if (sargs.size() == 4) {
			position = Common::Point(sargs[1].toInt32(), sargs[2].toInt32());
			depth = sargs[3].toInt32();
		}
		Common::ScopedPtr<Shell::Character> character(new Shell::Character(_scene, _game._vm.mixer(), _paths, *_game._vm._rnd, *this));
		return character->create(sargs[0].toInt32(), depth, position) ? character.release() : nullptr;
	}
	if (name.equals(kRCatapultGameName) && (sargs.empty() || (sargs.size() == 1 && sargs[0].toText().equals("!"))))
		return new CatapultMachine(_scene);
	if (name.equals(kRGlyphGameName) && sargs.size() == 3)
		return new GlyphMachine(_scene, *_game._vm._rnd, Common::Point(sargs[0].toInt32(), sargs[1].toInt32()), sargs[2].toInt32());
	if (name.equals(kRPlanetariumGameName) && sargs.size() == 1)
		return new PlanetariumMachine(_scene, *_game._vm._rnd, sargs[0].toInt32());
	if (name.equals(kRIncubatorName) && sargs.size() == 2) {
		Common::ScopedPtr<Incubator> incubator(new Incubator(_game._vm, _scene, sargs[0].toInt32(), sargs[1].toInt32()));
		return incubator->load() ? incubator.release() : nullptr;
	}
	if (name.equals(kGreenhouseName) && sargs.size() == 1)
		return new GreenhouseMachine(_game._vm, _scene, sargs[0].toInt32());
	if (name.equals(kRGardenGameName) && (sargs.empty() || (sargs.size() == 1 && sargs[0].toText().equals("!"))))
		return new GardenMachine(_game._vm, _scene);
	if (name.equals(kRCorralGameName) && (sargs.empty() || (sargs.size() == 1 && sargs[0].toText().equals("!"))))
		return new CorralMachine(_game._vm, _scene);
	if (name.equals(kRBarnGameName) && sargs.size() <= 1)
		return new BarnMachine(_game._vm, _scene, sargs.empty() || sargs[0].toText().equals("!") ? 0 : sargs[0].toInt32());
	if (name.equals(kRZIVGameName) && sargs.size() <= 1)
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

bool ScriptSession::get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (property.equals(kPropertyPlayerName) && sargs.empty()) {
		result = Core::ScriptValue(_game._playerName);
		return true;
	}
	if (property.equals(kPropertyIsNewPlayer) && sargs.size() <= 1) {
		Common::String player = sargs.empty() ? _game._playerName : sargs[0].toText();
		result = Core::ScriptValue(static_cast<int32>((_fresh && player.equals(_launcherPlayer)) || !_game._vm.hasGamePlayer(player)));
		return true;
	}
	if (property.equals(kPropertyAutoEnabled)) {
		Location location = static_cast<Location>(_game._world.currentLocation());
		if (!sargs.empty() && !sargs[0].toText().empty() && !WorldPort::locationForName(sargs[0].toText(), location))
			return false;
		if (_autoLevelingGlobal)
			location = kGlyph02;
		if (location < kCatapult01 || kBarn07 < location)
			return false;
		result = Core::ScriptValue(_game._world.autoEnabled(location - kCatapult01));
		return true;
	}
	if (sargs.empty() && _transient.contains(property)) {
		result = _transient[property];
		return true;
	}
	return false;
}

bool ScriptSession::set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) {
	if (property.equals(kPropertyAutoEnabled)) {
		Location location = static_cast<Location>(_game._world.currentLocation());
		if (!sargs.empty() && !sargs[0].toText().empty() && !WorldPort::locationForName(sargs[0].toText(), location))
			return false;
		if (_autoLevelingGlobal) {
			for (uint i = 0; i < 7; i++)
				_game._world.setAutoEnabled(i, value.toInt32());
			return true;
		}
		return kCatapult01 <= location && location <= kBarn07 && _game._world.setAutoEnabled(location - kCatapult01, value.toInt32());
	}
	if (sargs.empty() && _transient.contains(property)) {
		_transient[property] = value;
		if (property.equals(kPropertyIsAutoLevelingGlobal))
			_autoLevelingGlobal = value.toInt32() != 0;
		return true;
	}
	if (sargs.empty() && (property.equals(kPropertyBackgroundMusicVolume) || property.equals(kPropertyDialogVolume) || property.equals(kPropertySfxVolume) ||
							  property.equals(kPropertyIsBackgroundMusicOff))) {
		uint initial = 0;
		if (property.equals(kPropertyBackgroundMusicVolume))
			initial = kMusic01;
		else if (property.equals(kPropertyDialogVolume))
			initial = kDialogue02;
		else if (property.equals(kPropertySfxVolume))
			initial = kEffects04;
		// The initial MPS defaults retain ScummVM's launcher preferences for each new slot.
		if (_initSoundProps & initial)
			_initSoundProps &= ~initial;
		else
			_game._world.setStringProperty(property, value.toText());
		_game.applySoundSettings();
		return true;
	}
	return false;
}

bool ScriptSession::play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if ((method.equals(kMethodAddIntProperty) || method.equals(kMethodAddStringProperty)) && !sargs.empty() && sargs.size() <= 2) {
		PropertyDefinition definition;
		definition.string = method.equals(kMethodAddStringProperty);
		definition.saveable = sargs.size() < 2 || sargs[1].toText().empty() || sargs[1].toInt32() != 0;
		_definitions[sargs[0].toText()] = definition;
		if (!definition.saveable && !_transient.contains(sargs[0].toText()))
			_transient[sargs[0].toText()] = definition.string ? Core::ScriptValue(Common::String()) : Core::ScriptValue(0);
		resetPlayerProperties();
		return true;
	}
	if (method.equals(kMethodActivatePlayer) && sargs.size() == 1) {
		if (!_game._vm.activateScriptPlayer(_game, sargs[0].toText()))
			return false;
		if (sargs[0].toText().equals(_launcherPlayer))
			_fresh = false;
		resetPlayerProperties();
		_game.applySoundSettings();
		return true;
	}
	if ((method.equals(kMethodActivateGame) && !sargs.empty() && sargs.size() <= 2) || (method.equals(kMethodAddGame) && sargs.empty())) {
		bool append = method.equals(kMethodAddGame);
		if (!_game._vm.activateScriptGame(_game, append ? 0 : sargs[0].toInt32(),
										  !append && 1 < sargs.size() && sargs[1].toInt32() != 0, append))
			return false;
		resetPlayerProperties();
		_game.applySoundSettings();
		return true;
	}
	if (method.equals(kEventNewGame) && sargs.empty()) {
		_game._world.resetGame();
		return true;
	}
	return false;
}

bool ScriptSession::resourceCommand(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (!sargs.empty())
		return false;
	if (method.equals(kMethodUncacheResources)) {
		_scene.uncacheUnusedResources();
		_game._vm.fonts().removeUnused();
		return true;
	}
	// Native modules are linked into the subengine; their objects retain heap ownership.
	return method.equals(kMethodDeactivateAllDLLs) || method.equals(kMethodUncacheAllDLLs);
}

uint ScriptSession::playSound(const Common::String &resource, Audio::Mixer::SoundType type, uint repeat) {
	Common::ScopedPtr<OM::Sound> sound(new OM::Sound(_game._vm.mixer()));
	if (!sound->play(_paths, _paths.resourcePath(resource, kSoundExtension), type, repeat))
		return 0;
	_sounds.push_back(sound.release());
	return _sounds.size();
}

bool ScriptSession::invoke(const Common::String &name, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (name.equals(kMessageBoxName) && sargs.size() == 1) {
		GUI::MessageDialog dialog(sargs[0].toText().decode());
		dialog.runModal();
		_lastHostTime = g_system->getMillis();
		return true;
	}
	if (name.equals(kRandomNumberName) && sargs.size() == 2) {
		result = Core::ScriptValue(_game._vm._rnd->getRandomNumberRng(sargs[0].toInt32(), sargs[1].toInt32()));
		return true;
	}
	if (name.equals(kGetLauncherXMLFilenameName) && sargs.empty()) {
		result = Core::ScriptValue(Common::String(kLauncherName));
		return true;
	}
	if (name.equals(kReadININame) && sargs.size() == 1) {
		Common::String key = sargs[0].toText();
		result = Core::ScriptValue(ConfMan.hasKey(key) ? ConfMan.getInt(key) : 0);
		return true;
	}
	if ((name.equals(kAddPathName) || name.equals(kUseName)) && sargs.size() == 1) {
		if (name.equals(kUseName))
			_paths.use(sargs[0].toText());
		else
			_paths.addPath(sargs[0].toText());
		return true;
	}
	if (name.equals(kRemoveAllPathsName) && sargs.empty()) {
		_paths.removeAllPaths();
		return true;
	}
	if ((name.equals(kBulkFileName) || name.equals(kIndexFileName)) && sargs.size() == 1) {
		Common::File file;
		return file.open(Common::Path(sargs[0].toText()));
	}
	if (name.equals(kInstallCursorsName) && sargs.empty())
		return installCursors();
	if ((name.equals(kStartTransitionName) || name.equals(kEndTransitionName)) && sargs.empty()) {
		_transition = name.equals(kStartTransitionName);
		if (!_transition)
			_frozen = false;
		return true;
	}
	if ((name.equals(kSetClosedCaptioningActiveName) || name.equals(kSetDoubleClicksEnabledName)) && sargs.size() == 1) {
		if (name.equals(kSetClosedCaptioningActiveName))
			_closedCaptioning = sargs[0].toInt32() != 0;
		else
			_doubleClicks = sargs[0].toInt32() != 0;
		return true;
	}
	if (name.equals(kPlaySoundName) && sargs.size() == 1) {
		result = Core::ScriptValue(static_cast<int32>(playSound(sargs[0].toText())));
		return true;
	}
	if ((name.equals(kPauseSoundName) || name.equals(kResumeSoundName) || name.equals(kKillSoundName)) && sargs.size() == 1) {
		int tag = sargs[0].toInt32();
		if (0 < tag && static_cast<uint>(tag) <= _sounds.size() && _sounds[tag - 1]) {
			if (name.equals(kKillSoundName))
				_sounds[tag - 1]->stop();
			else
				_sounds[tag - 1]->pause(name.equals(kPauseSoundName));
		}
		return true;
	}
	if (name.equals(kIntersectTestName) && sargs.size() == 2) {
		Core::ScriptValue a = sargs[0].kind() == Core::ScriptValue::kObject ? sargs[0] : _runtime.variable(sargs[0].toText());
		Core::ScriptValue b = sargs[1].kind() == Core::ScriptValue::kObject ? sargs[1] : _runtime.variable(sargs[1].toText());
		Core::Trinket *left = visualObject(a.reference());
		Core::Trinket *right = visualObject(b.reference());
		if (!left || !right)
			return false;
		result = Core::ScriptValue(static_cast<int32>(Common::Rect(left->x, left->y, left->x + left->width(), left->y + left->height()).intersects(Common::Rect(right->x, right->y, right->x + right->width(), right->y + right->height()))));
		return true;
	}
	return false;
}

Shell::Action *ScriptSession::createAction(const Common::String &name, const Core::ScriptArguments &sargs, bool interruptible) {
	if (name.equals(kAttribChWalkActionName))
		return AttribChWalkAction::create(_actions, sargs, interruptible);
	if (name.equals(kRandomDelayActionName) && (sargs.size() == 2 || sargs.size() == 3)) {
		uint32 first = sargs[0].toInt32();
		uint32 last = sargs[1].toInt32();
		if (first == last)
			return nullptr;
		if (2 < sargs.size())
			interruptible = sargs[2].toInt32() != 0;
		return new Shell::RandomDelayAction(_actions, *_game._vm._rnd, first, last, interruptible);
	}
	if (name.equals(kPlayAnimActionName) && !sargs.empty() && sargs.size() <= 2) {
		if (1 < sargs.size())
			interruptible = sargs[1].toInt32() != 0;
		return new Shell::PlayAnimationAction(_actions, _scene, sargs[0], interruptible);
	}
	if (name.equals(kAnimActionName) && 3 <= sargs.size() && sargs.size() <= 6) {
		Common::Point position(11111, 11111);
		uint first = sargs.size() <= 4 ? 1 : 3;
		if (3 == first)
			position = Common::Point(sargs[1].toInt32(), sargs[2].toInt32());
		if (first + 2 < sargs.size())
			interruptible = sargs[first + 2].toInt32() != 0;
		return new Shell::AnimationAction(_actions, _scene, _paths.resourcePath(sargs[0].toText(), kAnimationExtension),
										  position, sargs[first].toInt32(), sargs[first + 1].toInt32(), interruptible);
	}
	if ((name.equals(kCharacterSpeechActionName) || name.equals(kCharacterAnimActionName)) && 2 <= sargs.size() && sargs.size() <= 4 &&
		_actions.resolve(sargs[0], kRCharacterName)) {
		if (3 < sargs.size())
			interruptible = sargs[3].toInt32() != 0;
		return new Shell::CharacterAction(_actions, sargs[0], sargs[1].toInt32(), name.equals(kCharacterSpeechActionName),
										  name.equals(kCharacterAnimActionName) && 2 < sargs.size() ? sargs[2].toInt32() : 1, true, interruptible);
	}
	if ((name.equals(kSoundActionName) || name.equals(kLoopedSoundActionName) || name.equals(kSpeechActionName)) && !sargs.empty() && sargs.size() <= 4) {
		if (name.equals(kSpeechActionName) && 2 < sargs.size())
			interruptible = sargs[2].toInt32() != 0;
		Audio::Mixer::SoundType type = name.equals(kSpeechActionName) ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType;
		uint repeat = name.equals(kLoopedSoundActionName) ? 0 : 1;
		if (name.equals(kSpeechActionName) && 3 < sargs.size())
			repeat = sargs[3].toInt32();
		return new Shell::SoundAction(_actions, _game._vm.mixer(), _paths, _paths.resourcePath(sargs[0].toText(), kSoundExtension),
									  type, interruptible, repeat);
	}
	return nullptr;
}

Common::String ScriptSession::characterAnimationPath(uint resource) const {
	return _paths.resourcePath(Common::String::format("%u", resource), kAnimationExtension);
}

Common::String ScriptSession::characterResource(uint resource, Shell::Character::Resources::Kind kind, uint part) const {
	Common::String suffix;
	if (kind == Shell::Character::Resources::kAnimation)
		suffix = kAnimationExtension;
	else if (kind == Shell::Character::Resources::kSound)
		suffix = kSoundExtension;
	else if (kind == Shell::Character::Resources::kLips)
		suffix = part == 1 ? kLipsExtension : Common::String::format(kLipsExtensionFormat, part);
	else
		suffix = kBodyExtension;
	return _paths.resourcePath(Common::String::format("%u", resource), suffix);
}

bool ScriptSession::installBackground(const Core::ScriptValue &image, const Core::ScriptValue &palette) {
	if (!_scene.setBackground(_paths.resourcePath(image.toText(), kBackgroundExtension)))
		return false;
	if (800 < _scene.backgroundWidth())
		_scroller.start(_time);
	return true;
}

Common::String ScriptSession::musicResource(const Core::ScriptValue &id) const {
	return id.toInt32() == 0 && id.isNumeric() ? Common::String() : _paths.resourcePath(id.toText(), kSoundExtension);
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
	static constexpr const char *classes[] = {
		kRAnimationName,
		kRFlyButtonName,
		kRQuitButtonName,
		kRTextName,
		kRHotSpotName,
		kRAttributeCharacterName,
		kRCharacterName,
		kRCatapultGameName,
		kRGlyphGameName,
		kRIncubatorName,
		kRGardenGameName,
		kRCorralGameName,
		kRZIVGameName,
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
		if (!_runtime.objects().resolve(reference, kRHotSpotName) && !_runtime.objects().resolve(reference, kRTextName))
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
		if (_runtime.objects().resolve(reference, kRCatapultGameName)) {
			CatapultMachine *machine = static_cast<CatapultMachine *>(object);
			int action = _scene.event(event);
			if (event.type == Common::EVENT_LBUTTONDOWN && Common::Rect(44, 0, 78, 58).contains(event.mouse))
				action = CatapultMachine::kInsertMud01;
			if (action)
				machine->action(action);
		} else if (_runtime.objects().resolve(reference, kRGlyphGameName)) {
			static_cast<GlyphMachine *>(object)->event(event, _time);
		} else if (_runtime.objects().resolve(reference, kRPlanetariumGameName)) {
			static_cast<PlanetariumMachine *>(object)->event(event, _time);
		} else if (_runtime.objects().resolve(reference, kGreenhouseName)) {
			static_cast<GreenhouseMachine *>(object)->event(event);
		} else if (_runtime.objects().resolve(reference, kRGardenGameName)) {
			static_cast<GardenMachine *>(object)->event(event);
		} else if (_runtime.objects().resolve(reference, kRCorralGameName)) {
			static_cast<CorralMachine *>(object)->event(event, _time);
		} else if (_runtime.objects().resolve(reference, kRBarnGameName)) {
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
			_window.broadcast(kEventResumed);
			if (result == QuitDialog::kQuit00) {
				_window.broadcast(kEventExittingGame);
				quit();
			}
		}
		return;
	}
	Common::Array<uint32> options = _runtime.objects().references(kROptionsName);
	if (!options.empty()) {
		OptionsObject *object = static_cast<OptionsObject *>(_runtime.objects().resolve(options.back(), kROptionsName));
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
			_movies.back()->emitEvent(kEventMouseDown);
		return;
	}
	_scroller.event(event);
	if (event.type != Common::EVENT_LBUTTONDOWN && event.type != Common::EVENT_LBUTTONUP && event.type != Common::EVENT_MOUSEMOVE)
		return;
	Core::ScriptableObject *tracked = _runtime.objects().resolve(_tracked);
	AttributeCharacter *trackedChar = static_cast<AttributeCharacter *>(_runtime.objects().resolve(_tracked, kRAttributeCharacterName));
	if (trackedChar && !trackedChar->isDragging()) {
		_tracked = _pressed = 0;
		tracked = nullptr;
	}
	if (tracked) {
		AttributeCharacter *character = static_cast<AttributeCharacter *>(_runtime.objects().resolve(_tracked, kRAttributeCharacterName));
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
				tracked->emitEvent(kEventMouseUp);
				tracked->emitEvent(kEventEndTracking);
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
			old->emitEvent(kEventFlyOff);
		_hovered = hit;
		Core::ScriptableObject *current = _runtime.objects().resolve(hit);
		if (Shell::Sprite *sprite = _scene.resolveSprite(current))
			sprite->flyOn();
		if (current)
			current->emitEvent(kEventFlyOn);
	}
	Core::ScriptableObject *object = _runtime.objects().resolve(hit);
	if (_runtime.objects().resolve(hit, kRCatapultGameName) || _runtime.objects().resolve(hit, kRGlyphGameName) ||
		_runtime.objects().resolve(hit, kRPlanetariumGameName) || _runtime.objects().resolve(hit, kRGardenGameName) ||
		_runtime.objects().resolve(hit, kRCorralGameName) || _runtime.objects().resolve(hit, kRBarnGameName))
		object = nullptr;
	Shell::Sprite *sprite = _scene.resolveSprite(object);
	if (event.type == Common::EVENT_LBUTTONDOWN && object) {
		_pressed = hit;
		object->setCallTime(_time);
		AttributeCharacter *character = static_cast<AttributeCharacter *>(_runtime.objects().resolve(hit, kRAttributeCharacterName));
		if (character && character->beginDrag(event.mouse, _time))
			_tracked = hit;
		else if (sprite && sprite->button)
			sprite->buttonPressed(_time);
		else if (sprite && sprite->tracking) {
			_tracked = hit;
			_dragOffset = event.mouse - Common::Point(sprite->x, sprite->y);
			sprite->emitEvent(kEventStartTracking);
		}
		object->emitEvent(kEventMouseDown);
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
				if (_runtime.objects().resolve(_pressed, kRQuitButtonName))
					openQuit();
				else
					button->emitEvent(kEventHit);
			}
		} else {
			pressed->emitEvent(kEventMouseUp);
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
	_window.broadcast(kEventPaused);
	_quitDialog.reset(dialog.release());
}

bool ScriptSession::hasModal() const {
	return _movieTasks || _quitDialog || !_runtime.objects().references(kROptionsName).empty() ||
		   _runtime.variable(kGHelpHSName).kind() == Core::ScriptValue::kObject || _runtime.variable(kLockScreenName).kind() == Core::ScriptValue::kObject;
}

void ScriptSession::syncSoundSettings() {
	for (uint32 reference : _runtime.objects().references(kROptionsName))
		static_cast<OptionsObject *>(_runtime.objects().resolve(reference, kROptionsName))->syncVolumes();
}

bool ScriptSession::installCursors() {
	if (_cursors[0].getPixels())
		return true;
	for (uint i = 0; i < ARRAYSIZE(_cursors); i++) {
		const Core::Animation *animation = _scene.loadAnimation(Common::String::format(kCursorFormat, 301 + i));
		if (!animation || animation->frameCount() != 1 || !animation->picture(0)) {
			warning("Trinket: could not load startup cursor z3common/%u.ao", 301 + i);
			return false;
		}
		_cursors[i].copyFrom(*animation->picture(0));
	}
	return true;
}

void ScriptSession::updateCursor() {
	CursorMan.showMouse(!_cursorHidden);
	int cursor = 0;
	Core::Trinket *object = visualObject(_tracked ? _tracked : hitObject(_mouse));
	if (object) {
		Common::String cursorText;
		if (_tracked)
			cursorText = object->grabCursor;
		else
			cursorText = object->spotCursor;
		int32 id = 0;
		if (StringParser::parseSignedInt(cursorText.c_str(), id, StringParser::kDecimalProfile) == StringParser::Result::kSuccess &&
			301 <= id && id <= 306)
			cursor = id - 301;
	}
	for (uint32 reference : _runtime.objects().references(kRGlyphGameName)) {
		GlyphMachine *machine = static_cast<GlyphMachine *>(_runtime.objects().resolve(reference, kRGlyphGameName));
		if (machine->dragging())
			cursor = 3;
		else if (0 <= machine->hitTile(_mouse))
			cursor = 4;
	}
	for (uint32 reference : _runtime.objects().references(kRPlanetariumGameName)) {
		PlanetariumMachine *machine = static_cast<PlanetariumMachine *>(_runtime.objects().resolve(reference, kRPlanetariumGameName));
		if (machine->dragging())
			cursor = 3;
		else if (0 <= machine->hitPiece(_mouse))
			cursor = 4;
	}
	for (uint32 reference : _runtime.objects().references(kGreenhouseName)) {
		GreenhouseMachine *machine = static_cast<GreenhouseMachine *>(_runtime.objects().resolve(reference, kGreenhouseName));
		if (machine->isDragging())
			cursor = 3;
		else if (machine->overDraggable(_mouse))
			cursor = 4;
	}
	for (uint32 reference : _runtime.objects().references(kRCorralGameName)) {
		CorralMachine *machine = static_cast<CorralMachine *>(_runtime.objects().resolve(reference, kRCorralGameName));
		if (machine->isDragging())
			cursor = 3;
		else if (machine->overDraggable(_mouse))
			cursor = 4;
	}
	for (uint32 reference : _runtime.objects().references(kRBarnGameName)) {
		BarnMachine *machine = static_cast<BarnMachine *>(_runtime.objects().resolve(reference, kRBarnGameName));
		if (machine->isDragging())
			cursor = 3;
		else if (machine->overDraggable(_mouse))
			cursor = 4;
	}
	if (_quitDialog)
		cursor = _quitDialog->overButton(_mouse) ? 2 : 0;
	for (uint32 reference : _runtime.objects().references(kROptionsName)) {
		OptionsObject *options = static_cast<OptionsObject *>(_runtime.objects().resolve(reference, kROptionsName));
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
	};
	static constexpr int hotspotY[] = {
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
	for (uint32 reference : _runtime.objects().references(kRAttributeCharacterName)) {
		AttributeCharacter *character = static_cast<AttributeCharacter *>(_runtime.objects().resolve(reference, kRAttributeCharacterName));
		if (!character)
			continue;
		character->update(_time, sounds);
		character = static_cast<AttributeCharacter *>(_runtime.objects().resolve(reference, kRAttributeCharacterName));
		if (character && character->hasResourceError())
			fail("attribute character resources");
	}
	for (uint32 reference : _runtime.objects().references(kRCharacterName)) {
		Shell::Character *character = static_cast<Shell::Character *>(_runtime.objects().resolve(reference, kRCharacterName));
		if (character)
			character->update(_time, sounds);
	}
	bool sceneUpdated = false;
	for (uint32 reference : _runtime.objects().references()) {
		Core::ScriptableObject *object = _runtime.objects().resolve(reference);
		if (!object)
			continue;
		uint16 sound;
		if (_runtime.objects().resolve(reference, kRCatapultGameName)) {
			CatapultMachine *machine = static_cast<CatapultMachine *>(object);
			machine->update(_time);
			sceneUpdated = true;
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Catapult resources");
			bool chute = machine->chuteMoving();
			bool gears = machine->machineRunning();
			bool paused = _runtime.variable(kGPauseDepthName).toInt32() != 0;
			_chuteSound.pause(paused);
			_gearSound.pause(paused);
			if (paused)
				continue;
			if (chute && !_chuteSound.isPlaying())
				_chuteSound.play(_paths, _paths.resourcePath(Common::String::format("%u", CatapultMachine::kResSound10400_ChuteLoop), kSoundExtension),
								 Audio::Mixer::kSFXSoundType, 0);
			if (!chute)
				_chuteSound.stop();
			if (gears && !_gearSound.isPlaying())
				_gearSound.play(_paths, _paths.resourcePath(Common::String::format("%u", CatapultMachine::kResSound10409_GearLoop), kSoundExtension),
								Audio::Mixer::kSFXSoundType, 0);
			if (!gears)
				_gearSound.stop();
		} else if (_runtime.objects().resolve(reference, kRGlyphGameName)) {
			GlyphMachine *machine = static_cast<GlyphMachine *>(object);
			machine->update(_time);
			sceneUpdated = true;
			int channel;
			for (auto &channelSound : _glyphSounds)
				channelSound->pause(_runtime.variable(kGPauseDepthName).toInt32() != 0);
			while (machine->pollSound(sound, &channel)) {
				Common::String resource = _paths.resourcePath(Common::String::format("%u", sound), kSoundExtension);
				if (0 <= channel && static_cast<uint>(channel) < ARRAYSIZE(_glyphSounds))
					_glyphSounds[channel]->play(_paths, resource, Audio::Mixer::kSFXSoundType);
				else
					playSound(Common::String::format("%u", sound));
			}
			if (machine->hasResourceError())
				fail("Glyph resources");
		} else if (_runtime.objects().resolve(reference, kRPlanetariumGameName)) {
			PlanetariumMachine *machine = static_cast<PlanetariumMachine *>(object);
			machine->update(_time);
			sceneUpdated = true;
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Planetarium resources");
		} else if (_runtime.objects().resolve(reference, kRIncubatorName)) {
			static_cast<Incubator *>(object)->update(_time);
		} else if (_runtime.objects().resolve(reference, kGreenhouseName)) {
			GreenhouseMachine *machine = static_cast<GreenhouseMachine *>(object);
			machine->update(_time);
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Greenhouse resources");
		} else if (_runtime.objects().resolve(reference, kRGardenGameName)) {
			GardenMachine *machine = static_cast<GardenMachine *>(object);
			machine->update(_time);
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Garden resources");
		} else if (_runtime.objects().resolve(reference, kRCorralGameName)) {
			CorralMachine *machine = static_cast<CorralMachine *>(object);
			machine->update(_time);
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Corral resources");
		} else if (_runtime.objects().resolve(reference, kRBarnGameName)) {
			BarnMachine *machine = static_cast<BarnMachine *>(object);
			machine->update(_time);
			while (machine->pollSound(sound))
				playSound(Common::String::format("%u", sound));
			if (machine->hasResourceError())
				fail("Barn resources");
		} else if (_runtime.objects().resolve(reference, kRZIVGameName)) {
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
		_scroller.freeze(_runtime.variable(kGPauseDepthName).toInt32() != 0, _time);
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
			if (!object || (!_runtime.objects().resolve(reference, kRTextName) && !_runtime.objects().resolve(reference, kRHotSpotName)))
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
			if (Shell::TextObject *text = static_cast<Shell::TextObject *>(_runtime.objects().resolve(visual.reference, kRTextName)))
				text->draw(canvas);
			else if (Shell::HotSpot *spot = static_cast<Shell::HotSpot *>(_runtime.objects().resolve(visual.reference, kRHotSpotName)))
				spot->draw(canvas);
			first = visual.depth == INT_MAX ? INT_MAX : visual.depth + 1;
		}
		_scene.drawSprites(canvas, first, INT_MAX);
	}
	for (Shell::Movie *movie : _movies)
		movie->draw(canvas);
	for (uint32 reference : _runtime.objects().references(kROptionsName)) {
		OptionsObject *options = static_cast<OptionsObject *>(_runtime.objects().resolve(reference, kROptionsName));
		if (options)
			options->draw(canvas, _time);
	}
	if (_quitDialog)
		_quitDialog->draw(canvas, _time);
}

Common::String ScriptSession::debugCommand(const Common::String &command) {
	if (command.equals(kCacheName)) {
		OM::FontCache &fonts = _game._vm.fonts();
		return _scene.cacheDescription() + Common::String::format(" fonts=%u fontDecodes=%" PRIu64 " fontHits=%" PRIu64 "\n",
			fonts.size(), fonts.decodes(), fonts.hits());
	}
	if (hasModal())
		return "Close the dialog or wait for the transition first.\n";
	int location = _game._world.currentLocation();
	uint level = kCatapult01 <= location && location <= kBarn07 ? _game._world.level(location - kCatapult01) : 0;
	uint moved = _runtime.variable(kGTokensMovedName).toInt32();
	uint party = _runtime.variable(kTokensName).toInt32();
	if (!party)
		party = _runtime.variable(kCurTokenCountName).toInt32();
	if (command.equals(kFinishName)) {
		if (location < kCatapult01 || kBarn07 < location)
			return "No puzzle is active.\n";
		if (!_game._practice) {
			int token;
			while (0 <= (token = _game._world.nextToken(-1, location))) {
				if (location == kCorral06) {
					Common::Array<uint32> games = _runtime.objects().references(kRCorralGameName);
					if (!games.empty()) {
						const CorralRules &rules = static_cast<CorralMachine *>(_runtime.objects().resolve(games[0], kRCorralGameName))->rules();
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
		_runtime.setVariable(kGTokensMovedName, Core::ScriptValue(static_cast<int32>(MAX(moved, party))));
		_runtime.setVariable(kGNextLocName, _runtime.variable(_game._practice ? kMapLocationKey : kNextLocationKey));
		if (!_runtime.runProcedure(kGSceneTransitionMovieName))
			return "Debug departure failed: " + _runtime.lastError() + "\n";
		return "Debug completion: party accepted; departure requested without awarding puzzle score.\n";
	}
	for (uint32 reference : _runtime.objects().references()) {
		Core::ScriptableObject *object = _runtime.objects().resolve(reference);
		if (!object)
			continue;
		if (_runtime.objects().resolve(reference, kRCatapultGameName)) {
			CatapultMachine *machine = static_cast<CatapultMachine *>(object);
			if (command.equals(kAnswerName))
				return machine->debugAnswer();
			if (command.equals(kChanceName) || command.equals(kChancesName))
				return machine->debugChance();
		} else if (_runtime.objects().resolve(reference, kRGlyphGameName)) {
			GlyphMachine *machine = static_cast<GlyphMachine *>(object);
			if (command.equals(kAnswerName))
				return machine->debugAnswer(level);
			if (command.equals(kChanceName) || command.equals(kChancesName)) {
				uint tries = _runtime.variable(kTriesMadeName).toInt32();
				uint maximum = _runtime.variable(kMaxTriesName).toInt32();
				return Common::String::format("The Wall: %u mistakes remaining (%u used of %u); %u of %u party members passed.\n",
											  maximum > tries ? maximum - tries : 0, tries, maximum, moved, party);
			}
		} else if (_runtime.objects().resolve(reference, kRPlanetariumGameName)) {
			PlanetariumMachine *machine = static_cast<PlanetariumMachine *>(object);
			if (command.equals(kAnswerName))
				return machine->debugAnswer(level);
			if (command.equals(kChanceName) || command.equals(kChancesName)) {
				uint remaining[2][2] = {};
				for (uint i = 0; i < machine->rules().pieceCount(); i++) {
					const PlanetariumRules::Piece &piece = machine->rules().piece(i);
					if (piece.enabled && !piece.consumed)
						remaining[piece.exhibit][piece.type] += 1;
				}
				return Common::String::format("Planetarium: left %u coins/%u bills, right %u coins/%u bills; %u of %u party members passed.\n",
											  remaining[0][0], remaining[0][1], remaining[1][0], remaining[1][1], moved, party);
			}
		} else if (_runtime.objects().resolve(reference, kGreenhouseName)) {
			GreenhouseMachine *machine = static_cast<GreenhouseMachine *>(object);
			if (command.equals(kAnswerName))
				return machine->debugAnswer();
			if (command.equals(kChanceName) || command.equals(kChancesName))
				return Common::String::format("Greenhouse: %u of 12 moths passed; %u wand swaps.\n", moved, machine->rules().remainingSwaps());
		} else if (_runtime.objects().resolve(reference, kRGardenGameName)) {
			GardenMachine *machine = static_cast<GardenMachine *>(object);
			if (command.equals(kAnswerName))
				return machine->debugAnswer();
			if (command.equals(kChanceName) || command.equals(kChancesName))
				return Common::String::format("Garden: %u of %u plants accepted; %u rejections remaining%s.\n",
											  moved, party, machine->rules().chances(), machine->returnPending() ? " (one return pending)" : "");
		} else if (_runtime.objects().resolve(reference, kRCorralGameName)) {
			CorralMachine *machine = static_cast<CorralMachine *>(object);
			if (command.equals(kAnswerName))
				return machine->debugAnswer();
			if (command.equals(kChanceName) || command.equals(kChancesName))
				return Common::String::format("Corral: %u of %u zerbles fed; %u fruit remaining; %d projector charges%s.\n",
											  moved, machine->rules().party(), machine->rules().fruitRemaining(), machine->rules().turns(),
											  machine->feedingPending() ? " (feeding pending)" : "");
		} else if (_runtime.objects().resolve(reference, kRBarnGameName)) {
			BarnMachine *machine = static_cast<BarnMachine *>(object);
			if (command.equals(kAnswerName))
				return machine->debugAnswer();
			if (command.equals(kChanceName) || command.equals(kChancesName))
				return Common::String::format("Barn: %u of %u parents paired; %u incorrect pairings remaining; %u water drains%s.\n",
											  moved, machine->rules().party(), machine->rules().chances(), machine->drained(),
											  machine->drained() < machine->rules().wrong() ? " (one drain pending removal)" : "");
		}
	}
	if (command.equals(kChanceName) || command.equals(kChancesName))
		return Common::String::format("%s: %u of %u party members passed.\n", _scriptName.c_str(), moved, party);
	return Common::String::format("MPS %s: %s\n", _scriptName.c_str(), _runtime.lastError().empty() ? kRunningName : _runtime.lastError().c_str());
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
