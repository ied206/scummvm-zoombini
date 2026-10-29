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

/**
 * MPS scheduler connecting script execution to native window and game services.
 * MPS creates native game objects through registered factories.
 * @ref Trinket::Shell::ResourcePaths resolves asset names against the engine's indexed provider.
 * @ref Trinket::Shell::Scene loads visual resources and displays the sprites used by those objects.
 */
class ScriptSession : private Core::ScriptRuntime::Functions, private Core::ScriptScheduler::Loader, private Shell::ActionSystem::Factory, private Shell::ScenePortEnvironment::Window, private Shell::Movie::Host, private Shell::ResourcePort::Host, private WorldPort::Host, private AttributeCharacter::AnimationResources, private Shell::Character::Resources {
public:
	/** Connect the world's scripts to native scene, action, window and audio services. */
	explicit ScriptSession(Game &game);
	/** Stop native playback and release script objects and session graphics. */
	~ScriptSession();
	/** Enter the fresh-game or resumed script path with its launcher context. */
	bool start(bool fresh, bool fromLauncher);
	/** Dispatch input to modal controls, native puzzle objects and scripted scene events. */
	void event(const Common::Event &event);
	/** Advance scripts and native objects and draw the session at a host timestamp. */
	void draw(OM::Canvas &canvas, uint32 time);
	/** Return whether an in-game modal currently intercepts gameplay input. */
	bool hasModal() const;
	/** Apply the current world's sound settings to session audio. */
	void syncSoundSettings();
	/** Preserve inherited music, dialogue and effects preferences during initial script setup. */
	void inheritInitialSoundSettings() { _initSoundProps = kMusic01 | kDialogue02 | kEffects04; }
	/** Run a scene or puzzle debugger command and return its response. */
	Common::String debugCommand(const Common::String &command);

private:
	/** Script event name for new Game. */
	static constexpr const char *kEventNewGame = "newGame";
	/** Script event name for game Selected. */
	static constexpr const char *kEventGameSelected = "gameSelected";
	/** Script event name for cheat Change. */
	static constexpr const char *kEventCheatChange = "cheatChange";
	/** Script event name for closed. */
	static constexpr const char *kEventClosed = "closed";
	/** Script property name for selected Game. */
	static constexpr const char *kPropertySelectedGame = "selectedGame";
	/** Script property name for replace Selected Game. */
	static constexpr const char *kPropertyReplaceSelectedGame = "replaceSelectedGame";
	/** Script event name for speech Volume Changed. */
	static constexpr const char *kEventSpeechVolumeChanged = "speechVolumeChanged";
	/** Script event name for sfx Volume Changed. */
	static constexpr const char *kEventSfxVolumeChanged = "sfxVolumeChanged";
	/** Script identifier for rAnimation. */
	static constexpr const char *kRAnimationName = "RAnimation";
	/** Script identifier for rFly Button. */
	static constexpr const char *kRFlyButtonName = "RFlyButton";
	/** Script identifier for rQuit Button. */
	static constexpr const char *kRQuitButtonName = "RQuitButton";
	/** Script identifier for rText. */
	static constexpr const char *kRTextName = "RText";
	/** Script identifier for rXMLFile. */
	static constexpr const char *kRXMLFileName = "RXMLFile";
	/** Script identifier for rOptions. */
	static constexpr const char *kROptionsName = "ROptions";
	/** Script identifier for rAttribute Character. */
	static constexpr const char *kRAttributeCharacterName = "RAttributeCharacter";
	/** Script identifier for rCharacter. */
	static constexpr const char *kRCharacterName = "RCharacter";
	/** Script identifier for rCatapult Game. */
	static constexpr const char *kRCatapultGameName = "RCatapultGame";
	/** Script identifier for rGlyph Game. */
	static constexpr const char *kRGlyphGameName = "RGlyphGame";
	/** Script identifier for rPlanetarium Game. */
	static constexpr const char *kRPlanetariumGameName = "RPlanetariumGame";
	/** Script identifier for rIncubator. */
	static constexpr const char *kRIncubatorName = "RIncubator";
	/** Script identifier for greenhouse. */
	static constexpr const char *kGreenhouseName = "Greenhouse";
	/** Script identifier for rGarden Game. */
	static constexpr const char *kRGardenGameName = "RGardenGame";
	/** Script identifier for rCorral Game. */
	static constexpr const char *kRCorralGameName = "RCorralGame";
	/** Script identifier for rBarn Game. */
	static constexpr const char *kRBarnGameName = "RBarnGame";
	/** Script identifier for rZIVGame. */
	static constexpr const char *kRZIVGameName = "RZIVGame";
	/** Script identifier for rHot Spot. */
	static constexpr const char *kRHotSpotName = "RHotSpot";
	/** Script identifier for in Game. */
	static constexpr const char *kInGameName = "inGame";
	/** Script identifier for previous Location. */
	static constexpr const char *kPreviousLocationName = "previousLocation";
	/** Script property name for is Auto Leveling Global. */
	static constexpr const char *kPropertyIsAutoLevelingGlobal = "isAutoLevelingGlobal";
	/** Script identifier for debug Mode. */
	static constexpr const char *kDebugModeName = "debugMode";
	/** Script class name for rScene Port. */
	static constexpr const char *kClassRScenePort = "RScenePort";
	/** Script class name for rResource Port. */
	static constexpr const char *kClassRResourcePort = "RResourcePort";
	/** Script class name for rWorld Port. */
	static constexpr const char *kClassRWorldPort = "RWorldPort";
	/** Script class name for rBink Movie. */
	static constexpr const char *kClassRBinkMovie = "RBinkMovie";
	/** Script class name for rRadio Group. */
	static constexpr const char *kClassRRadioGroup = "RRadioGroup";
	/** Script class name for rQueue. */
	static constexpr const char *kClassRQueue = "RQueue";
	/** Script class name for rComposite Action. */
	static constexpr const char *kClassRCompositeAction = "RCompositeAction";
	/** Script identifier for ctrl-O. */
	static constexpr const char *kCtrlOName = "Ctrl-O";
	/** Script identifier for options Key Pressed. */
	static constexpr const char *kOptionsKeyPressedName = "optionsKeyPressed";
	/** Script identifier for cmd-O. */
	static constexpr const char *kCmdOName = "Cmd-O";
	/** Script identifier for 301. */
	static constexpr const char *k301Name = "301";
	/** Script identifier for 304. */
	static constexpr const char *k304Name = "304";
	/** Script identifier for @launcher. */
	static constexpr const char *kLauncherName = "@launcher";
	/** Script property name for player Name. */
	static constexpr const char *kPropertyPlayerName = "playerName";
	/** Script property name for is New Player. */
	static constexpr const char *kPropertyIsNewPlayer = "isNewPlayer";
	/** Script property name for auto Enabled. */
	static constexpr const char *kPropertyAutoEnabled = "autoEnabled";
	/** Script property name for background Music Volume. */
	static constexpr const char *kPropertyBackgroundMusicVolume = "backgroundMusicVolume";
	/** Script property name for dialog Volume. */
	static constexpr const char *kPropertyDialogVolume = "dialogVolume";
	/** Script property name for sfx Volume. */
	static constexpr const char *kPropertySfxVolume = "sfxVolume";
	/** Script property name for is Background Music Off. */
	static constexpr const char *kPropertyIsBackgroundMusicOff = "isBackgroundMusicOff";
	/** Script method name for add Int Property. */
	static constexpr const char *kMethodAddIntProperty = "addIntProperty";
	/** Script method name for add String Property. */
	static constexpr const char *kMethodAddStringProperty = "addStringProperty";
	/** Script method name for activate Player. */
	static constexpr const char *kMethodActivatePlayer = "activatePlayer";
	/** Script method name for activate Game. */
	static constexpr const char *kMethodActivateGame = "activateGame";
	/** Script method name for add Game. */
	static constexpr const char *kMethodAddGame = "addGame";
	/** Script method name for uncache Resources. */
	static constexpr const char *kMethodUncacheResources = "uncacheResources";
	/** Script method name for deactivate All DLLs. */
	static constexpr const char *kMethodDeactivateAllDLLs = "deactivateAllDLLs";
	/** Script method name for uncache All DLLs. */
	static constexpr const char *kMethodUncacheAllDLLs = "uncacheAllDLLs";
	/** Script identifier for message Box. */
	static constexpr const char *kMessageBoxName = "MessageBox";
	/** Script identifier for random Number. */
	static constexpr const char *kRandomNumberName = "RandomNumber";
	/** Script identifier for get Launcher XMLFilename. */
	static constexpr const char *kGetLauncherXMLFilenameName = "GetLauncherXMLFilename";
	/** Script identifier for read INI. */
	static constexpr const char *kReadININame = "readINI";
	/** Script identifier for add Path. */
	static constexpr const char *kAddPathName = "addPath";
	/** Script identifier for use. */
	static constexpr const char *kUseName = "use";
	/** Script identifier for remove All Paths. */
	static constexpr const char *kRemoveAllPathsName = "removeAllPaths";
	/** Script identifier for bulk File. */
	static constexpr const char *kBulkFileName = "bulkFile";
	/** Script identifier for index File. */
	static constexpr const char *kIndexFileName = "indexFile";
	/** Script identifier for install Cursors. */
	static constexpr const char *kInstallCursorsName = "InstallCursors";
	/** Script identifier for start Transition. */
	static constexpr const char *kStartTransitionName = "StartTransition";
	/** Script identifier for end Transition. */
	static constexpr const char *kEndTransitionName = "EndTransition";
	/** Script identifier for set Closed Captioning Active. */
	static constexpr const char *kSetClosedCaptioningActiveName = "SetClosedCaptioningActive";
	/** Script identifier for set Double Clicks Enabled. */
	static constexpr const char *kSetDoubleClicksEnabledName = "SetDoubleClicksEnabled";
	/** Script identifier for play Sound. */
	static constexpr const char *kPlaySoundName = "PlaySound";
	/** Script identifier for pause Sound. */
	static constexpr const char *kPauseSoundName = "PauseSound";
	/** Script identifier for resume Sound. */
	static constexpr const char *kResumeSoundName = "ResumeSound";
	/** Script identifier for kill Sound. */
	static constexpr const char *kKillSoundName = "KillSound";
	/** Script identifier for intersect Test. */
	static constexpr const char *kIntersectTestName = "IntersectTest";
	/** Script identifier for attrib Ch Walk Action. */
	static constexpr const char *kAttribChWalkActionName = "AttribChWalkAction";
	/** Script identifier for random Delay Action. */
	static constexpr const char *kRandomDelayActionName = "RandomDelayAction";
	/** Script identifier for play Anim Action. */
	static constexpr const char *kPlayAnimActionName = "PlayAnimAction";
	/** Script identifier for anim Action. */
	static constexpr const char *kAnimActionName = "AnimAction";
	/** Script identifier for character Speech Action. */
	static constexpr const char *kCharacterSpeechActionName = "CharacterSpeechAction";
	/** Script identifier for character Anim Action. */
	static constexpr const char *kCharacterAnimActionName = "CharacterAnimAction";
	/** Script identifier for sound Action. */
	static constexpr const char *kSoundActionName = "SoundAction";
	/** Script identifier for looped Sound Action. */
	static constexpr const char *kLoopedSoundActionName = "LoopedSoundAction";
	/** Script identifier for speech Action. */
	static constexpr const char *kSpeechActionName = "SpeechAction";
	/** Script event name for resumed. */
	static constexpr const char *kEventResumed = "resumed";
	/** Script event name for exitting Game. */
	static constexpr const char *kEventExittingGame = "exittingGame";
	/** Script event name for mouse Down. */
	static constexpr const char *kEventMouseDown = "mouseDown";
	/** Script event name for mouse Up. */
	static constexpr const char *kEventMouseUp = "mouseUp";
	/** Script event name for end Tracking. */
	static constexpr const char *kEventEndTracking = "endTracking";
	/** Script event name for fly Off. */
	static constexpr const char *kEventFlyOff = "flyOff";
	/** Script event name for fly On. */
	static constexpr const char *kEventFlyOn = "flyOn";
	/** Script event name for start Tracking. */
	static constexpr const char *kEventStartTracking = "startTracking";
	/** Script event name for hit. */
	static constexpr const char *kEventHit = "hit";
	/** Script event name for paused. */
	static constexpr const char *kEventPaused = "paused";
	/** Script identifier for g Help HS. */
	static constexpr const char *kGHelpHSName = "gHelpHS";
	/** Script identifier for lock Screen. */
	static constexpr const char *kLockScreenName = "lockScreen";
	/** Script identifier for g Pause Depth. */
	static constexpr const char *kGPauseDepthName = "gPauseDepth";
	/** Script identifier for cache. */
	static constexpr const char *kCacheName = "cache";
	/** Script identifier for g Tokens Moved. */
	static constexpr const char *kGTokensMovedName = "gTokensMoved";
	/** Script identifier for tokens. */
	static constexpr const char *kTokensName = "tokens";
	/** Script identifier for cur Token Count. */
	static constexpr const char *kCurTokenCountName = "curTokenCount";
	/** Script identifier for finish. */
	static constexpr const char *kFinishName = "finish";
	/** Script identifier for g Next Loc. */
	static constexpr const char *kGNextLocName = "gNextLoc";
	/** Authored data key for Map Location. */
	static constexpr const char *kMapLocationKey = "kMapLocation";
	/** Authored data key for Next Location. */
	static constexpr const char *kNextLocationKey = "kNextLocation";
	/** Script identifier for g Scene Transition Movie. */
	static constexpr const char *kGSceneTransitionMovieName = "gSceneTransitionMovie";
	/** Script identifier for answer. */
	static constexpr const char *kAnswerName = "answer";
	/** Script identifier for chance. */
	static constexpr const char *kChanceName = "chance";
	/** Script identifier for chances. */
	static constexpr const char *kChancesName = "chances";
	/** Script identifier for tries Made. */
	static constexpr const char *kTriesMadeName = "triesMade";
	/** Script identifier for max Tries. */
	static constexpr const char *kMaxTriesName = "maxTries";
	/** Script identifier for running. */
	static constexpr const char *kRunningName = "running";

	/** Font used by the startup script presentation. */
	static constexpr const char *kStartupFont = "/z3common/351.nfnt";
	/** Startup cursor path indexed by resource ID. */
	static constexpr const char *kCursorFormat = "/z3common/%u.ao";
	/** Entry script when returning from the launcher. */
	static constexpr const char *kMainScript = "main.mps";
	/** Extension of compiled page scripts. */
	static constexpr const char *kScriptExtension = ".mps";
	/** Extension of animation sequences. */
	static constexpr const char *kAnimationExtension = ".ao";
	/** Extension of sound resources. */
	static constexpr const char *kSoundExtension = ".wav";
	/** Extension used to resolve scripted backgrounds. */
	static constexpr const char *kBackgroundExtension = ".jpg";
	/** Extension of the first character lip animation. */
	static constexpr const char *kLipsExtension = ".lips";
	/** Extension indexed by additional character lip part. */
	static constexpr const char *kLipsExtensionFormat = ".lip%u";
	/** Extension of character body animation. */
	static constexpr const char *kBodyExtension = ".tc";
	/** Resource ID of the options credits music. */
	static constexpr const char *kCreditsMusicID = "403";

	enum InitialSoundProperty {
		kMusic01 = 1,
		kDialogue02 = 2,
		kEffects04 = 4,
	};
	class NativeFactory : public Core::ObjectRegistry::Factory {
	public:
		/** Register a native script class name with the session's constructor dispatcher. */
		NativeFactory(ScriptSession &session, const Common::String &name) : _session(session), _name(name) {}
		/** Construct the registered native class from script arguments. */
		Core::ScriptableObject *create(const Core::ScriptArguments &sargs) override;
		/** Return the registered script class name. */
		const Common::String &name() const { return _name; }

	private:
		/** Session that dispatches construction of the native object. */
		ScriptSession &_session;
		/** Script class name associated with this factory. */
		Common::String _name;
	};
	class OptionsObject;
	struct PropertyDefinition {
		/** Whether the script property stores text rather than a numeric value. */
		bool string = false;
		/** Whether property updates are written to persistent world state. */
		bool saveable = true;
	};
	/** Game providing the current world, player and engine services. */
	Game &_game;
	/** Resource path rules for the active script and scene. */
	Shell::ResourcePaths _paths;
	/** Sprites, background and input state of the active scene. */
	Shell::Scene _scene;
	/** Horizontal panorama scrolling for supported scenes. */
	Shell::Scroller _scroller;
	/** Current compiled script thread and native object registry. */
	Core::ScriptRuntime _runtime;
	/** Shared script timing and deferred call coordinator. */
	Core::ScriptScheduler _scheduler;
	/** Native asynchronous actions created by scripts. */
	Shell::ActionSystem _actions;
	/** Scene-port window, pointer, key and music services. */
	Shell::ScenePortEnvironment _window;
	/** Factory for sequential action containers. */
	Shell::ActionSystem::ContainerFactory _queueFactory;
	/** Factory for parallel action containers. */
	Shell::ActionSystem::ContainerFactory _compositeFactory;
	/** Factory for scripted scene-port objects. */
	Shell::ScenePort::Factory _sceneFactory;
	/** Factory for scripted resource-port objects. */
	Shell::ResourcePort::Factory _resFactory;
	/** Factory for global and location-specific world ports. */
	WorldPort::Factory _worldFactory;
	/** Factory for script-controlled movie playback. */
	Shell::Movie::Factory _movieFactory;
	/** Factory for exclusive-choice sprite groups. */
	RadioGroup::Factory _radioFactory;
	/** Registered constructors for game-specific native script classes. */
	Common::Array<NativeFactory *> _factories;
	/** Script property definitions indexed by property name. */
	Common::HashMap<Common::String, PropertyDefinition> _definitions;
	/** Runtime property values excluded from persistent world saves. */
	Common::HashMap<Common::String, Core::ScriptValue> _transient;
	/** Movies registered with the current script session. */
	Common::Array<Shell::Movie *> _movies;
	/** Sound handles allocated for script-requested playback. */
	Common::Array<OM::Sound *> _sounds;
	/** Separate sound handles for Glyph feedback channels. */
	Common::ScopedPtr<OM::Sound> _glyphSounds[10];
	/** Quit-confirmation interface while its modal is active. */
	Common::ScopedPtr<QuitDialog> _quitDialog;
	/** FontMan-owned default font used by script-created text objects. */
	const Graphics::Font *_font = nullptr;
	/** Playback state for the catapult chute mechanism. */
	OM::Sound _chuteSound;
	/** Playback state for the catapult drive mechanism. */
	OM::Sound _gearSound;
	/** Captured scene pixels retained while the script freezes display. */
	Graphics::Surface _frozenScreen;
	/** Decoded game cursor surfaces indexed by cursor role. */
	Graphics::Surface _cursors[6];
	/** Name of the script currently loaded by the session. */
	Common::String _scriptName;
	/** Launcher player name supplied to the startup script. */
	Common::String _launcherPlayer;
	/** Most recent pointer position in game pixels. */
	Common::Point _mouse;
	/** Pointer displacement retained for the current object drag. */
	Common::Point _dragOffset;
	/** Script reference selected when the current pointer press began. */
	uint32 _pressed = 0;
	/** Script reference currently receiving pointer-hover feedback. */
	uint32 _hovered = 0;
	/** Script reference tracking the current pointer drag. */
	uint32 _tracked = 0;
	/** Current script-session timestamp in milliseconds. */
	uint32 _time = 0;
	/** Previous host timestamp used to advance the session clock. */
	uint32 _lastHostTime = 0;
	/** Number of active movie playback tasks reported to the session. */
	uint32 _movieTasks = 0;
	/** Index of the installed game cursor, or minus one before selection. */
	int _cursor = -1;
	/** Mask of inherited audio properties protected during initial script assignments. */
	uint _initSoundProps = 0;
	/** Whether this session is entering the new-game script path. */
	bool _fresh = false;
	/** Whether script window state currently hides the cursor. */
	bool _cursorHidden = false;
	/** Whether the session draws its captured frozen screen. */
	bool _frozen = false;
	/** Whether a script-driven scene or application transition is in progress. */
	bool _transition = false;
	/** Stored script preference for closed captions. */
	bool _closedCaptioning = false;
	/** Whether double-click input is enabled for script objects. */
	bool _doubleClicks = true;
	/** Whether an unrecoverable session or resource error occurred. */
	bool _failed = false;
	/** Global automatic-leveling value exposed by the script interface. */
	bool _autoLevelingGlobal = false;
	/** Register shared and game-specific native constructors with the runtime. */
	void registerClasses();
	/** Construct a named native script object from its arguments. */
	Core::ScriptableObject *createNative(const Common::String &name, const Core::ScriptArguments &sargs);
	/** Load and initialize a named script in the supplied thread. */
	bool loadScript(Core::ScriptRuntime &thread, const Common::String &name) override;
	/** Record the game's request to leave the active session. */
	void quit() override;
	/** Dispatch a named host function called by the script runtime. */
	bool invoke(const Common::String &name, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Handle script resource lookup and configuration commands. */
	bool resourceCommand(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Read game-specific properties not handled directly by the world port. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign game-specific runtime and persistent properties. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch game-specific world-port methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Construct a game-specific asynchronous action from script arguments. */
	Shell::Action *createAction(const Common::String &name, const Core::ScriptArguments &sargs, bool interruptible) override;
	/** Resolve script image and palette values and replace the scene background. */
	bool installBackground(const Core::ScriptValue &image, const Core::ScriptValue &palette) override;
	/** Capture the current display and apply the requested script freeze mode. */
	void freezeWindow(bool screenOnly) override;
	/** Resume live scene drawing instead of the frozen capture. */
	void displayWindow() override { _frozen = false; }
	/** Apply the script-requested cursor visibility state. */
	void hideCursor(bool hidden) override;
	/** Return the current script cursor-visibility state. */
	bool isCursorHidden() const override { return _cursorHidden; }
	/** Resolve a script music value to its resource path. */
	Common::String musicResource(const Core::ScriptValue &id) const override;
	/** Register a newly created movie for session playback updates. */
	void movieCreated(Shell::Movie &movie) override;
	/** Remove a destroyed movie from session tracking. */
	void movieDestroyed(Shell::Movie &movie) override;
	/** Update the count of active movie playback tasks. */
	void moviePlaying(Shell::Movie &movie, bool playing) override;
	/** Resolve an attribute-character animation ID through the current resource paths. */
	Common::String characterAnimationPath(uint resource) const override;
	/** Resolve a character speech, animation or choreography resource and optional part. */
	Common::String characterResource(uint resource, Shell::Character::Resources::Kind kind, uint part = 1) const override;
	/** Start a script-requested sound and return its session sound handle. */
	uint playSound(const Common::String &resource, Audio::Mixer::SoundType type = Audio::Mixer::kSFXSoundType, uint repeat = 1);
	/** Advance puzzle machines, characters, movies and other native scene objects. */
	void updateNative();
	/** Deliver input to active native puzzle machines. */
	void nativeEvent(const Common::Event &event);
	/** Return the script reference of the interactive object at a game-pixel point. */
	uint32 hitObject(Common::Point point);
	/** Resolve a script reference to a visual Trinket object, or return null. */
	Core::Trinket *visualObject(uint32 reference);
	/** Select the default, hover or drag cursor for current interaction. */
	void updateCursor();
	/** Decode game cursor resources for the session. */
	bool installCursors();
	/** Create and open the quit-confirmation modal. */
	void openQuit();
	/** Reset transient player-specific values before a player transition. */
	void resetPlayerProperties();
	/** Record the failure context and stop further session advancement. */
	void fail(const Common::String &context);
	/** Prevent accidental copies of this stateful object. */
	ScriptSession(const ScriptSession &) = delete;
	/** Prevent accidental copies of this stateful object. */
	ScriptSession &operator=(const ScriptSession &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
