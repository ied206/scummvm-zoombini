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
#include "common/str.h"

#include "engines/engine.h"

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
class DialogDebug;
class DialogMsgBox;
enum class DialogMsgBoxButton;
class PageBase;
class RleBlock;
class SoundManager;
class Sidebar;
class ZoombiniAnimation;
class ZoombiniRunner;

/** Rescue Site I branch selected for the next map transition. */
enum class RouteBranch : int {
	kNone00 = 0, ///< No branch selected.
	kLeft01 = 1, ///< Left branch.
	kRight02 = 2 ///< Right branch.
};

/** Maximum number of Zoombinis in the starting party. */
const uint kMaxPackSize = 16;

/** Maximum party size after a rescue-site transition. */
const uint kPostRescuePackSize = 8;

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

	/** Apply the v1.0 release-family puzzle-generation reseed from the current gameplay tick. */
	void reseedRandomForV10();
	/** Return the most recently processed game-space mouse position. */
	Common::Point32 getMousePos() const { return _mousePos; }
	/** Present the interactive hover cursor instead of the default cursor. */
	void setHoverCursorActive(bool active);
	/** Select a page's carried-item cursor, or restore normal hover selection with nullptr. */
	void setPageCursorSprite(const RleBlock *sprite);

	/** Return the detected release language. */
	Common::Language getLanguage() const { return _gameDescription->desc.language; }
	/** Return whether the detected game is the Korean release. */
	bool isKorean() const { return getLanguage() == Common::KO_KOR; }
	/** Return the detected release feature flags. */
	uint32 getFeatures() const { return _gameDescription->features; }
	/** Return whether the detected release is the playable demo. */
	bool isDemo() const { return (_gameDescription->desc.flags & ADGF_DEMO) != 0; }

	/** Return the drawing surface for this game instance. */
	ManagedSurface32 *getScreen() { return _screen; }
	/** Return the drawing surface used by the active page. */
	ManagedSurface32 *getCurrentScreen() { return _screen; }
	/** Return the single immutable alpha-blending lookup table shared by this game instance. */
	const AlphaBlendLUT &getAlphaLUT() const { return _alphaBlendLUT; }
	/** Return the sound manager for this game instance. */
	SoundManager *getSoundManager() { return _soundManager; }
	/** Return the engine-owned shared message-box dialog. */
	DialogMsgBox *getMsgBoxDialog() { return _msgBoxDialog; }
	/** Return the engine-owned debug area-mask dialog. */
	DialogDebug *getDebugDialog() { return _debugDialog; }
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
	/** Return whether Aqua Cube protects its first direct lever move from a Fleen. */
	bool useAquacubeSafeFirstMove() const { return _useAquacubeSafeFirstMove; }
	/** Return whether Bezier paths use floating-point rather than original Q10 calculations. */
	bool useFloatingPointPaths() const { return _useFloatingPointPaths; }
	/** Return whether the enhanced keyboard shortcut set is enabled. */
	bool useEnhancedKbdShortcuts() const { return _enhancedKbdShortcuts; }
	/** Return the logic pacing rate in Hz selected for frame-derived speeds.
	 * It gates only Booliewood panorama scrolling and the banked idle-roll quota;
	 * millisecond-deadline animations are unaffected. */
	int getLogicPacingHz() const { return _logicPacingHz; }
	/** Return whether the Chez Norf diagnostic overlay key is currently held. */
	bool showChezNorfDebugOverlay() const { return _debugHotkeysEnabled && _debugOverlayKeyDown; }

	/** Load or return a shared Zoombini sprite grid and select its page-configured frame delay. */
	const ZoombiniAnimation *loadZoombiniAnimation(const Common::Path &path, uint32 frameDelay);
	/** Open an original logical resource name through the engine's CD/installed-root resolver. */
	Common::SeekableReadStream *openResourceFile(const Common::String &path) const;
	/** Return whether @p path resolves through the engine's CD/installed-root resolver. */
	bool hasResource(const Common::String &path) const;
	/** Return and clear the validated direct-practice request, if the startup command supplied one. */
	bool takePracticePuzzleLaunch(PageId &pageId, int &level);
	/** Return the practice-map level retained for this engine session. */
	int getPracticeLevel() const { return _practiceLevel; }
	/** Retain a validated practice-map level for later map visits. */
	void setPracticeLevel(int level);

	/** Pause or resume the gameplay clock for a game dialog. */
	void setDialogPaused(bool paused);

	/** Write the current profile to its active storage slot, or succeed without writing for a physically read-only loaded profile. */
	bool writeGameSave(const Common::String &name);
	/** Create a new player-selected profile without overwriting another one. */
	bool createGameSave(const Common::String &name);
	/** Replace a player-selected profile after the caller obtains explicit confirmation. */
	bool overwriteGameSave(const Common::String &name);
	/** Replace the current profile with the profile stored under @p name. */
	bool readGameSave(const Common::String &name);
	/** Delete the active target's profile named @p name. */
	bool deleteGameSave(const Common::String &name);
	/** Return the active target's valid profile names in display order. */
	Common::StringArray listGameSaves() const;
	/** Return whether the active target's listed save for @p name is read-only. */
	bool isGameSaveReadOnly(const Common::String &name) const;
	/** Current profile state, including its active party and rescue storage. */
	GameState *_state = nullptr;

	/** Request that the main loop replace the active page with @p pageId. */
	void requestPageChange(PageId pageId) { _nextPageId = pageId; }
	/** Restart the sidebar Go-button attention blink with original timing. */
	void restartGoBlink();
	/** Return whether the pending transition leads back to a map or sign-in flow. */
	bool isReturningToMap() const { return _nextPageId == kPageMenuLoad || _nextPageId == kPageMenuPractice || _nextPageId == kPageMapScreen; }
	/** Return whether the pending page uses the route-map travel transition. */
	bool isStartingMapTransition() const { return _nextPageId == kPageMapTrans; }
	/** Return the active page identifier. */
	PageId getCurrentPageId() const { return _currentPageId; }
	/** Return the borrowed active page. */
	PageBase *getCurrentPage() { return _currentPage; }

	/** Return gameplay milliseconds from the current clock read. */
	uint32 getGameTickCount() const;
	/** Return the gameplay millisecond snapshot captured before the current page pass. */
	uint32 getFrameTickCount() const { return _cachedGameTickCount; }
	/** Return gameplay milliseconds elapsed between the last two page passes. */
	uint32 getFrameDeltaMs() const { return _cachedGameTickCount - _prevFrameTickCount; }

	/** Gameplay random generator for this game instance. */
	Random *_rnd;
	/** Shared graphics interface used by pages and engine rendering. */
	Gfx *_gfx = nullptr;

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
	/** Total paused time excluded from @ref Zoombini2Engine::getGameTickCount. */
	uint32 _pauseTimeAccum = 0;
	/** System tick captured when the current pause began. */
	uint32 _pauseTimeStart = 0;
	/** Independent dialog and ScummVM modal reasons for freezing gameplay time. */
	bool _dialogPaused = false;
	bool _backendPaused = false;
	/** Whether at least one route-transition Zoombini is still walking. */
	bool _zoombiniWalkingFlag = false;
	/** Whether the current transition may skip its remaining presentation. */
	bool _skipMode = false;
	/** Most recently completed rescue-route branch. */
	RouteBranch _lastRouteDirection = RouteBranch::kNone00;

	/** Selected ShelterZombiniville feature values, or -1 for an unselected slot. */
	int16 _selectedFeatures[ZmbTrait::kTraitCount] = {
		-1,
		-1,
		-1,
		-1,
	};

	/** Deadline or countdown used by the active page transition. */
	int _pageTransitionTimer = 0;
	/** Shared stage value used by page-transition flow. */
	int _transitionState = 0;

private:
	class ResourceFileResolver;
	/** Decimal factor separating a direct-practice page ID from its difficulty. */
	static constexpr int kPracticeBootParamPageFactor = 100;
	static constexpr const char *kCursorSpritePath = "bmp/cursor/cursor01.rb";
	static constexpr const char *kInteractiveCursorSpritePath = "bmp/cursor/cursor02.rb";
	static constexpr const char *kQuitConfirmationPath = "bmp/menu/Quit_panel_text_quit";

	/** Physical storage profile selected by a load, create, or confirmed overwrite. */
	Common::String _activeSaveProfileName;
	/** Whether the active loaded storage profile must retain its original bytes. */
	bool _activeSaveProfileReadOnly = false;

	/** Serialize the current profile after the caller applies its save policy. */
	bool saveGameProfile(const Common::String &name);

	typedef Common::HashMap<Common::Path, ZoombiniAnimation *, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> ZoombiniAnimationCache;

	/** Return the unique child directory whose name matches @p name without case. */
	static Common::FSNode findChildDirectoryIgnoreCase(const Common::FSNode &directory, const char *name);
	/** Decode and validate a direct-practice boot parameter before the initial page is selected. */
	bool configurePracticeBootParamLaunch();

	/** Original logical resource-name resolver for the CD and installed roots. */
	ResourceFileResolver *_resourceFileResolver = nullptr;
	/** Fixed-size drawing surface for this game instance. */
	ManagedSurface32 *_screen = nullptr;
	/** Immutable animation sets shared by page lifetimes. */
	ZoombiniAnimationCache _zoombiniAnimationCache;

	/** Cursor sprite borrowed from the graphics shared cache. */
	RleBlock *_cursorSprite = nullptr;
	/** Interactive hover cursor sprite borrowed from the graphics shared cache. */
	RleBlock *_interactiveCursorSprite = nullptr;
	/** Whether the interactive hover cursor is currently presented. */
	bool _hoverCursorActive = false;
	/** Borrowed carried-item sprite, cleared by the page before its resources are released. */
	const RleBlock *_pageCursorSprite = nullptr;
	/** Signed 16-bit hotspot offset used by the active cursor image. */
	Common::Point _cursorHotspot = Common::Point();
	/** Whether CursorMan should present the game cursor. */
	bool _cursorVisible = true;

	/** Sound manager for this game instance. */
	SoundManager *_soundManager = nullptr;

	/** Shared Help, Map, and Go controls for this game instance. */
	Sidebar *_sidebar = nullptr;
	/** Shared two-button message-box dialog for pages and controls. */
	DialogMsgBox *_msgBoxDialog = nullptr;
	/** Debug area-mask dialog opened from the console. */
	DialogDebug *_debugDialog = nullptr;

	/** Most recently processed game-space mouse position. */
	Common::Point32 _mousePos = Common::Point32();
	/** Ordered input events awaiting the current page's dispatch boundary. */
	Common::Array<Common::Event> _pendingPageEvents;
	/** Whether the tracked press began inside a modal dialog. Its release belongs to that gesture even if the press dismissed the modal. */
	bool _modalOwnedPress = false;

	/** Active page identifier. */
	PageId _currentPageId = kPageNone;
	/** Requested replacement page identifier. */
	PageId _nextPageId = kPageLogoTLC;
	/** Active page, or nullptr between page lifetimes. */
	PageBase *_currentPage = nullptr;
	/** Validated direct-practice destination pending the practice-map setup. */
	PageId _practicePageId = kPageNone;
	/** Validated direct-practice level pending the practice-map setup. */
	int _practicePuzzleLevel = 0;
	/** Practice-map level retained while puzzle pages replace the map. */
	int _practiceLevel = 1;

	/** System tick used as the gameplay-clock origin. */
	uint32 _startTime = 0;
	/** Gameplay tick snapshot refreshed once per main-loop pass. */
	uint32 _cachedGameTickCount = 0;
	/** Gameplay tick snapshot from the previous main-loop pass. */
	uint32 _prevFrameTickCount = 0;
	/** Last backend millisecond accepted for presentation. */
	uint32 _lastPresentTimeMs = 0;
	/** Host-time origin used to number frame slots at the selected rate. */
	uint32 _frameTimeOriginMs = 0;
	/** Last host-time offset observed by the frame scheduler. */
	uint32 _lastFrameElapsedMs = 0;
	/** Most recent frame slot processed by the engine. */
	uint64 _lastFrameIndex = 0;
	bool _hasFrameIndex = false;
	int _frameRate = 60;
	bool _unlockFrameRate = false;
	/** Whether the developer hotkeys are active. */
	bool _debugHotkeysEnabled = false;
	/** Whether newly started stereo game-audio streams retain both channels. */
	bool _stereoOutputEnabled = false;
	/** Whether Waterslide level one uses the alternate greedy pairing. */
	bool _useGreedyWaterslidePairing = false;
	/** Whether Aqua Cube level three protects its first direct lever move from a Fleen. */
	bool _useAquacubeSafeFirstMove = false;
	/** Whether Bezier paths use the optional floating-point evaluator. */
	bool _useFloatingPointPaths = false;
	/** Whether the enhanced keyboard shortcut set is enabled. */
	bool _enhancedKbdShortcuts = false;
	/** Logic pacing rate in Hz selected for frame-derived speeds. */
	int _logicPacingHz = 75;
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
	/** Update one pause reason and account only the combined paused interval. */
	void setPauseState(bool &reason, bool paused);
	/** Track the ScummVM modal pause alongside game dialogs. */
	void pauseEngineIntern(bool pause) override;
	/** Load the target-scoped compatibility and gameplay-improvement switches from ScummVM configuration. */
	void refreshEngineSettings();
	/** Export the active party's trait tuples to working-directory zoombini.set. */
	void exportZoombiniSet() const;
	/** Import bounded trait tuples from working-directory zoombini.set. */
	void importZoombiniSet();
	/** Apply the held global puzzle-completion shortcut to the active roster and page. */
	void applyDebugPuzzleCompletion();
	/** Release every Zoombini sprite grid cached by this game instance. */
	void clearZoombiniAnimationCache();
	/** Load and register the game cursor. */
	void initCursor();
	/** Register the loaded cursor sprite with CursorMan. */
	void registerCursorWithCursorMan();
	/** Register @p sprite with CursorMan using the shared conversion. */
	void registerCursorSpriteWithCursorMan(const RleBlock *sprite);
	/** Return the first held global Zoombini, or nullptr when none is held. */
	const ZoombiniRunner *getDraggedGlobalZoombini() const;
	/** Hide the cursor while held, then draw the page overlay and held name plate. */
	void updateDragOverlay(bool advanceState);
	/** Open the shared quit confirmation unless the original close gates suppress it. */
	void handleQuitRequest();
	/** Open the shared quit confirmation through the message-box dialog. */
	void requestQuitConfirmation();
	/** Apply the shared quit-confirmation result. */
	void handleQuitConfirmation(DialogMsgBoxButton button);
	/** Drain backend events and update frame-local input state. */
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
	/** Wait for the next frame slot without replaying any missed slots. */
	void waitForFrameSlot();
	/** Process and present one complete engine frame. */
	void runFrame();
	/** Select the startup page and run frames until the engine quits. */
	void mainGameLoop();
	/** Destroy the active page and construct @p pageId. */
	void switchPage(PageId pageId);
	/** Release the active page and clear its pointer. */
	void destroyCurrentPage();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_ZOOMBINI2_H
