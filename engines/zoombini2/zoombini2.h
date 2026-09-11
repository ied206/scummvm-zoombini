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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ZOOMBINI2_ZOOMBINI2_H
#define ZOOMBINI2_ZOOMBINI2_H

#include "common/array.h"
#include "common/error.h"
#include "common/events.h"
#include "common/hashmap.h"
#include "common/rect.h"
#include "common/scummsys.h"

#include "engines/engine.h"

#include "graphics/managed_surface.h"
#include "graphics/surface.h"

#include "zoombini2/detection.h"
#include "zoombini2/graphics.h"
#include "zoombini2/random.h"
#include "zoombini2/state.h"

namespace Common {
class SeekableReadStream;
}

namespace Zoombini2 {

class BitBlock;
class DialogMsgBox;
class PageBase;
class RleBlock;
class SoundManager;
class Sidebar;
class ZoombiniAnimation;
class ZoombiniState;

/** Width of the fixed internal game screen. */
const int kScreenWidth = 800;

/** Height of the fixed internal game screen. */
const int kScreenHeight = 600;

/** Target configuration key enabling the developer hotkeys. */
extern const char *const kConfigDebugHotkeys;
/** Target configuration key selecting stereo game-audio streams. */
extern const char *const kConfigStereoOutput;
/** Target configuration key selecting the alternate level-one Waterslide pairing. */
extern const char *const kConfigGreedyWaterslidePairing;
/** Target configuration key selecting the once-per-frame gameplay clock snapshot. */
extern const char *const kConfigCachedFrameTime;
/** Target configuration key selecting floating-point Bezier path calculations. */
extern const char *const kConfigUseFloatingPointPaths;
/** Target configuration key selecting the original Windows random-number generator. */
extern const char *const kConfigOriginalPRNG;

/** Numeric page identifiers accepted by the engine dispatcher. */
enum PageId {
	kPageMenuLoad = -4,         ///< Saved-adventure sign-in flow.
	kPageMenuPractice = -3,     ///< Practice-map sign-in flow.
	kPageMenuOptions = -2,      ///< Sign-in screen.
	kPageNone = -1,             ///< No dispatched page.
	kPageZombiniville = 0,      ///< Zoombiniville party-assembly shelter.
	kPageCrazyTurtle = 1,       ///< Turtle Hurdle puzzle.
	kPageWaterslide = 2,        ///< Pipes of Paloo puzzle.
	kPageAquacube = 3,          ///< Aqua Cube puzzle.
	kPageRescue1 = 4,           ///< First rescue-site shelter.
	kPageMysticMarsh = 5,       ///< Bubble Bumpers puzzle.
	kPageMagicWall = 6,         ///< Beetle Bug Alley puzzle.
	kPageWallOfFleens = 7,      ///< Magic Mirrors puzzle.
	kPageChezNorf = 8,          ///< Chez Norf puzzle.
	kPageRescue2 = 9,           ///< Second rescue-site shelter.
	kPageSnowboard = 10,        ///< Snowboard Gulch puzzle.
	kPageBoolies = 11,          ///< Boolie Boggle puzzle.
	kPageBooliewood = 12,       ///< Booliewood arrival shelter.
	kPageCredits = 16,          ///< Credits transition.
	kPageLogoTLC = 17,          ///< Splash video of The Learning Company
	kPageCutsceneFirst = 18,    ///< First story video.
	kPageCutsceneSecond = 20,   ///< Second story video.
	kPageCutsceneThird = 21,    ///< Third story video.
	kPageMapTrans = 22,         ///< Route-map travel transition
	kPageFinal = 23,            ///< Booliewood final-celebration shelter
	kPageTitleScreen = 24,      ///< Title screen
	kPageLogoPolygon = 25,      ///< Splash video of Polygon Studio
	kPageMapScreen = 30,        ///< ScummVM map-screen dispatcher alias
	kPageMenuAlt = 40,          ///< Alternate sign-in route
	kPageLogoArisuMedia = 1972, ///< (v1.1KR only) Splash video of ArisuMedia
};

/** Rescue Site I branch selected for the next map transition. */
enum class RouteBranch {
	kNone00 = 0, ///< No branch selected.
	kLeft01 = 1, ///< Left branch.
	kRight02 = 2 ///< Right branch.
};

/** Maximum number of Zoombinis in the starting party. */
const int kMaxPackSize = 16;

/** Maximum party size after a rescue-site transition. */
const int kPostRescuePackSize = 8;

/** Number of distinct four-feature combinations. */
const int kMaxCombinations = 625;

/**
 * Owns global resources, input, page dispatch, and the active game session.
 *
 * The engine presents a fixed 800x600 surface. It owns the shared game state,
 * global party, sidebar controls, sound manager, cursor resources, and exactly one
 * dispatched @ref Page at a time.
 */
class Zoombini2Engine : public Engine {
public:
	/** Construct an engine for one detected release. */
	Zoombini2Engine(OSystem *syst, const Zoombini2GameDescription *desc);
	/** Release the active page and all resources retained for this game instance. */
	~Zoombini2Engine() override;

	/** Initialize resources and run the main page loop. */
	Common::Error run() override;
	/** Initialize the original CD Data and installed-resource roots. */
	void initializePath(const Common::FSNode &gamePath) override;
	/** Return whether the engine exposes @p feature. */
	bool hasFeature(EngineFeature feature) const override;
	/** Synchronize the standard ScummVM audio settings with the game-facing percentages. */
	void syncSoundSettings() override;
	/** Refresh target-scoped engine settings changed through the global menu. */
	void applyGameSettings() override;

	/** Detected release descriptor retained for the engine lifetime. */
	const Zoombini2GameDescription *_gameDescription;

	/** Return the gameplay random generator for this game instance. */
	Zoombini2Random *getRandom() { return _rnd; }
	/** Apply the v1.0 release-family puzzle-generation reseed from the current gameplay tick. */
	void reseedRandomForV10();
	/** Return the most recently processed game-space mouse position. */
	Common::Point32 getMousePos() const { return _mousePos; }
	/** Return whether the primary mouse button is currently held. */
	bool isMouseDown() const { return _mouseDown; }

	/** Return the detected release language. */
	Common::Language getLanguage() const { return _gameDescription->desc.language; }
	/** Return the detected release feature flags. */
	uint32 getFeatures() const { return _gameDescription->features; }

	/** Return the drawing surface for this game instance. */
	ManagedSurface32 *getScreen() { return _screen; }
	/** Return the drawing surface used by the active page. */
	ManagedSurface32 *getCurrentScreen() { return _screen; }
	/** Return the single immutable alpha-blending lookup table shared by this game instance. */
	const AlphaBlendLUT &getAlphaLUT() const { return _alphaBlendLUT; }
	/** Return the sound manager for this game instance. */
	SoundManager *getSoundManager() { return _soundManager; }
	/** Return the active profile state for this game instance. */
	GameState *getGameState() { return _gameState; }
	/** Return the engine-owned shared message-box dialog. */
	DialogMsgBox *getMsgBoxDialog() { return _msgBoxDialog; }
	/** Start or retain the shared map-music stream and return its sound identifier. */
	int ensureMapMusic();
	/** Return the game-facing music volume percentage. */
	int getMusicVolume() const;
	/** Return the game-facing sound-effect volume percentage. */
	int getSFXVolume() const;
	/** Return the game-facing speech volume percentage. */
	int getSpeechVolume() const;
	/** Preview the three game-facing volume percentages without changing the target configuration. */
	void previewSoundVolumes(int music, int sfx, int speech);
	/** Store the three game-facing volume percentages in the active target and apply them immediately. */
	void saveSoundVolumes(int music, int sfx, int speech);
	/** Return whether the alternate level-one Waterslide pairing is enabled. */
	bool useGreedyWaterslidePairing() const { return _useGreedyWaterslidePairing; }
	/** Return whether Bezier paths use floating-point rather than original Q10 calculations. */
	bool useFloatingPointPaths() const { return _useFloatingPointPaths; }
	/** Return whether the Chez Norf diagnostic overlay key is currently held. */
	bool showChezNorfDebugOverlay() const { return _debugHotkeysEnabled && _debugOverlayKeyDown; }

	/** Load a BitBlock through the engine resource resolver. */
	BitBlock *loadBitBlock(const Common::String &path);
	/** Load an RLE block through the engine resource resolver. */
	RleBlock *loadRleBlock(const Common::String &path);
	/** Load or return an immutable Zoombini animation set cached by this game instance. */
	const ZoombiniAnimation *loadZoombiniAnimation(const Common::Path &path);
	/** Open an original logical resource name through the engine's CD/installed-root resolver. */
	Common::SeekableReadStream *openResourceFile(const Common::String &path) const;
	/** Return whether @p path resolves through the engine's CD/installed-root resolver. */
	bool hasResource(const Common::String &path) const;

	/** Add @p ms to the time excluded from gameplay tick calculations. */
	void addPauseTime(uint32 ms) { _pauseTimeAccum += ms; }

	/** Write the current profile under @p name. */
	bool writeGameSave(const Common::String &name);
	/** Replace the current profile with the profile stored under @p name. */
	bool readGameSave(const Common::String &name);
	/** Delete the active target's profile named @p name. */
	bool deleteGameSave(const Common::String &name);
	/** Return the active target's valid profile names in display order. */
	Common::StringArray listGameSaves() const;
	/** Delete and clear every entry in @ref Zoombini2Engine::_globalZoombinis. */
	void clearGlobalZoombinis();

	/** Party entries retained for active gameplay outside @ref GameState. */
	Common::Array<ZoombiniState *> _globalZoombinis;

	/** Request that the main loop replace the active page with @p pageId. */
	void requestPageChange(int pageId) { _nextPageId = pageId; }
	/** Return whether the pending transition leads back to a map or sign-in flow. */
	bool isReturningToMap() const { return _nextPageId == kPageMenuLoad || _nextPageId == kPageMenuPractice || _nextPageId == kPageMapScreen; }
	/** Return whether the pending page uses the route-map travel transition. */
	bool isStartingMapTransition() const { return _nextPageId == kPageMapTrans; }
	/** Return the active page identifier. */
	int getCurrentPageId() const { return _currentPageId; }
	/** Return the borrowed active page. */
	PageBase *getCurrentPage() { return _currentPage; }

	/** Return elapsed gameplay milliseconds with accumulated pause time removed. */
	uint32 getGameTickCount() const;

	/** Whether the next puzzle entry restores the profile's party. */
	bool _returningFromPuzzle = false;
	/** Whether the active map-screen flow represents a saved adventure. */
	bool _isSavedGame = false;
	/** Shared byte-sized gameplay flag consumed by page flow. */
	byte _gameFlagB = 0;
	/** Selected Rescue Site I branch for the next map transition. */
	RouteBranch _routeDirection = RouteBranch::kNone00;
	/** Gameplay page shown as the source of the next map transition. */
	PageId _mapTransitionSourcePageId = kPageZombiniville;
	/** Whether the engine's gameplay clock is currently paused. */
	bool _isPaused = false;
	/** Total paused time excluded from @ref Zoombini2Engine::getGameTickCount. */
	uint32 _pauseTimeAccum = 0;
	/** System tick captured when the current pause began. */
	uint32 _pauseTimeStart = 0;
	/** Whether at least one route-transition Zoombini is still walking. */
	bool _zoombiniWalkingFlag = false;
	/** Whether the current transition may skip its remaining presentation. */
	bool _skipMode = false;
	/** Most recently completed rescue-route branch. */
	RouteBranch _lastRouteDirection = RouteBranch::kNone00;

	/** Selected ShelterZombiniville feature values, or -1 for an unselected slot. */
	int16 _selectedFeatures[ZmbTrait::kTraitCount] = {-1, -1, -1, -1};

	/** Deadline or countdown used by the active page transition. */
	int _pageTransitionTimer = 0;
	/** Shared stage value used by page-transition flow. */
	int _transitionState = 0;

private:
	class ResourceFileResolver;

	typedef Common::HashMap<Common::Path, ZoombiniAnimation *, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> ZoombiniAnimationCache;

	/** Maximum presentation-loop rate used to prevent the ScummVM backend from busy-spinning. */
	static constexpr uint32 kTargetFrameRate = 60;
	/** Duration of one presentation-loop pass at @ref kTargetFrameRate. */
	static constexpr double kTargetFrameTimeMs = 1000.0 / kTargetFrameRate;

	/** Return the unique child directory whose name matches @p name without case. */
	static Common::FSNode findChildDirectoryIgnoreCase(const Common::FSNode &directory, const char *name);

	/** Gameplay random generator for this game instance. */
	Zoombini2Random *_rnd;
	/** Original logical resource-name resolver for the CD and installed roots. */
	ResourceFileResolver *_resourceFileResolver = nullptr;
	/** Fixed-size drawing surface for this game instance. */
	ManagedSurface32 *_screen = nullptr;
	/** Immutable animation sets shared by page lifetimes. */
	ZoombiniAnimationCache _zoombiniAnimationCache;

	/** Cursor sprite registered for this game instance. */
	RleBlock *_cursorSprite = nullptr;
	/** Signed 16-bit hotspot offset used by the active cursor image. */
	Common::Point _cursorHotspot = Common::Point();
	/** Whether CursorMan should present the game cursor. */
	bool _cursorVisible = true;

	/** Sound manager for this game instance. */
	SoundManager *_soundManager = nullptr;
	/** Shared map-music sound identifier, or -1 when stopped. */
	int _mapMusicId = -1;

	/** Active profile state for this game instance. */
	GameState *_gameState = nullptr;
	/** Shared Help, Map, and Go controls for this game instance. */
	Sidebar *_sidebar = nullptr;
	/** Shared two-button message-box dialog for pages and controls. */
	DialogMsgBox *_msgBoxDialog = nullptr;

	/** Most recently processed game-space mouse position. */
	Common::Point32 _mousePos = Common::Point32();
	/** Whether the primary mouse button is currently held. */
	bool _mouseDown = false;
	/** Ordered input events awaiting the current page's dispatch boundary. */
	Common::Array<Common::Event> _pendingPageEvents;

	/** Active page identifier. */
	int _currentPageId = kPageNone;
	/** Requested replacement page identifier. */
	int _nextPageId = kPageLogoTLC;
	/** Active page, or nullptr between page lifetimes. */
	PageBase *_currentPage = nullptr;

	/** System tick used as the gameplay-clock origin. */
	uint32 _startTime = 0;
	/** Gameplay tick snapshot refreshed once per main-loop pass. */
	uint32 _cachedGameTickCount = 0;
	/** Whether the developer hotkeys are active. */
	bool _debugHotkeysEnabled = false;
	/** Whether newly started stereo game-audio streams retain both channels. */
	bool _stereoOutputEnabled = false;
	/** Whether Waterslide level one uses the alternate greedy pairing. */
	bool _useGreedyWaterslidePairing = false;
	/** Whether gameplay time reads use the current frame snapshot. */
	bool _useCachedFrameTime = false;
	/** Whether Bezier paths use the optional floating-point evaluator. */
	bool _useFloatingPointPaths = false;
	/** Held state of the global puzzle-completion key. */
	bool _debugCompletionKeyDown = false;
	/** Held state of the Chez Norf diagnostic-overlay key. */
	bool _debugOverlayKeyDown = false;
	/** Single immutable per-channel alpha-blending lookup table shared by this game instance. */
	AlphaBlendLUT _alphaBlendLUT;
	/** Convert a ScummVM mixer value to the game-facing zero-to-one-hundred scale. */
	static int mixerVolumeToPercent(int volume);
	/** Convert a game-facing zero-to-one-hundred percentage to a ScummVM mixer value. */
	static int percentToMixerVolume(int volume);
	/** Compute gameplay time directly from the backend clock. */
	uint32 calculateGameTickCount() const;
	/** Load the target-scoped compatibility and gameplay-improvement switches from ScummVM configuration. */
	void refreshEngineSettings();
	/** Export the active party's trait tuples to working-directory zoombini.set. */
	void exportZoombiniSet() const;
	/** Import bounded trait tuples from working-directory zoombini.set. */
	void importZoombiniSet();
	/** Apply the held global puzzle-completion shortcut to the active roster and page. */
	void applyDebugPuzzleCompletion();
	/** Stop the shared map-music stream and clear its identifier. */
	void stopMapMusic();
	/** Credit the party that completed Boolie Boggle to the active profile. */
	void recordBooliesCompletion();
	/** Release every Zoombini sprite grid cached by this game instance. */
	void clearZoombiniAnimationCache();
	/** Load and register the game cursor. */
	void initCursor();
	/** Register the loaded cursor sprite with CursorMan. */
	void registerCursorWithCursorMan();
	/** Consume pending backend events and update frame-local input state. */
	void processEvents();
	/** Apply a queued page replacement before the active frame is dispatched. */
	void applyPendingPageChange();
	/**
	 * Dispatch queued input while preserving shared and page-owned modal boundaries.
	 * @return True if a shared modal was active when dispatch began.
	 */
	bool dispatchPageEvents();
	/** Advance and draw the active page together with the shared controls. */
	void drawFrame();
	/** Copy the composed game surface to the backend and present it. */
	void presentFrame();
	/** Process and present one complete engine frame. */
	void runFrame();
	/** Select the startup page and run frames until the engine quits. */
	void mainGameLoop();
	/** Destroy the active page and construct @p pageId. */
	void switchPage(int pageId);
	/** Release the active page and clear its pointer. */
	void destroyCurrentPage();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_ZOOMBINI2_H
