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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ZOOMBINI2_ZOOMBINI2_H
#define ZOOMBINI2_ZOOMBINI2_H

#include "common/array.h"
#include "common/callback.h"
#include "common/error.h"
#include "common/events.h"
#include "common/hashmap.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/ustr.h"

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
class DialogBase;
struct DialogDebugCommand;
enum class DialogMsgBoxButton;
class PageBase;
class RleBlock;
class SoundManager;
class Sidebar;
class ZoombiniAnimation;
class ZoombiniRunner;

/**
 * Owns global resources, input, page dispatch, and the active game session.
 *
 * The engine presents a fixed 800x600 surface. It owns the shared game state,
 * global party, sidebar controls, sound manager, cursor resources, and exactly one
 * dispatched @ref Page at a time.
 */
class Zoombini2Engine : public Engine {
public:
	/** Rescue Site I branch selected for the next map transition. */
	enum class RouteBranch : int {
		kNone00 = 0, ///< No branch selected.
		kLeft01 = 1, ///< Left branch.
		kRight02 = 2 ///< Right branch.
	};

	/** Cursor artwork or a request to remove the current page cursor. */
	enum class CursorType : byte {
		kDefault,     ///< Pointing hand
		kInteractive, ///< Spread hand
		kScrollLeft,  ///< Finger pointing left
		kScrollRight, ///< Finger pointing right
		/** Chez Norf main dishes. */
		kMainDishSandwich,
		kMainDishFish,
		kMainDishSalad,
		/** Chez Norf drinks. */
		kDrinkTea,
		kDrinkOrangeJuice,
		kDrinkMilk,
		/** Chez Norf desserts. */
		kDessertFruitPie,
		kDessertWatermelon,
		kDessertIceCream,
		kMealTray,   ///< Held Chez Norf meal tray.
		kRestoreBase ///< Clear to the current default or interactive cursor.
	};

	/** Original developer actions that can also be invoked from the ScummVM debugger. */
	enum class BuiltinDebugAction {
		kExportZoombiniSet,
		kImportZoombiniSet,
		kCompletePuzzle
	};

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
	/** Select a cursor by its logical kind, or remove a page cursor with @ref CursorType::kRestoreBase. */
	void setCursor(CursorType type);

	/** Return the detected release language. */
	Common::Language getLanguage() const { return _gameDescription->desc.language; }
	/** Return whether the detected game is the Korean release. */
	bool isKorean() const { return getLanguage() == Common::KO_KOR; }
	/** Return whether the detected language is Hebrew. */
	bool isHebrew() const { return getLanguage() == Common::HE_ISR; }
	/** Return whether the detected language is Swedish. */
	bool isSwedish() const { return getLanguage() == Common::SV_SWE; }
	/** Return the detected release feature flags. */
	uint32 getFeatures() const { return _gameDescription->features; }
	/** Return whether the detected release is the playable demo. */
	bool isDemo() const { return (_gameDescription->desc.flags & ADGF_DEMO) != 0; }

	/** Return the single immutable alpha-blending lookup table shared by this game instance. */
	const AlphaBlendLUT &getAlphaLUT() const { return _alphaBlendLUT; }
	/** Return the sound manager for this game instance. */
	SoundManager *getSoundManager() { return _soundManager; }
	/** Queue a game-resource confirmation and retain its completion callback. */
	bool requestMsgBox(const Common::Path &textPath, Common::BaseCallback<DialogMsgBoxButton> *callback,
					   const Common::Point32 &pos = Common::Point32(-1, -1));
	/** Queue a UI-text confirmation and retain its completion callback. */
	bool requestUiTextMsgBox(const Common::U32String &text, Common::BaseCallback<DialogMsgBoxButton> *callback,
							 const Common::Point32 &pos = Common::Point32(-1, -1));
	/** Open the help overlay for the current page and difficulty. */
	bool openHelpDialog(PageId pageId, int level);
	/** Open a console debug view. */
	bool openDebugDialog(const DialogDebugCommand &cmd);
	/** Return the currently active engine dialog, if any. */
	DialogBase *getActiveDialog() const;
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
	/** Return whether the Wall of Fleens departure artwork correction is enabled. */
	bool fixFleenDepartStreak() const { return _fixFleenDepartStreak; }
	/** Return whether the enhanced keyboard shortcut set is enabled. */
	bool useEnhancedKbdShortcuts() const { return _enhancedKbdShortcuts; }
	/** Return whether the practice map exposes recoverable level 4 puzzles. */
	bool allowCutLevel4PracticePuzzles() const { return _allowCutLevel4PracticePuzzles; }
	/** Return the logic pacing rate in Hz selected for frame-derived speeds.
	 * It gates only Booliewood panorama scrolling and the banked idle-roll quota;
	 * millisecond-deadline animations are unaffected. */
	int getLogicPacingHz() const { return _logicPacingHz; }
	/** Return whether the Chez Norf diagnostic overlay key is currently held. */
	bool showChezNorfDebugOverlay() const { return _debugHotkeysEnabled && _debugOverlayKeyDown; }
	/** Report the active target's developer-hotkey gate. */
	bool areBuiltinDebugHotkeysEnabled() const { return _debugHotkeysEnabled; }
	/** Return the target-prefixed roster-trait filename used in the configured save location. */
	Common::String getDebugZoombiniSetFileName() const;
	/** Invoke one original developer action without requiring an in-game key event. */
	bool runBuiltinDebugAction(BuiltinDebugAction action);

	/** Load or return a shared Zoombini sprite grid and select its page-configured frame delay. */
	const ZoombiniAnimation *loadZoombiniAnimation(const Common::Path &path, uint32 frameDelay);
	/** Open an original logical resource name through the engine's CD/installed-root resolver. */
	Common::SeekableReadStream *openResourceFile(const Common::String &path) const;
	/** Return whether @p path resolves through the engine's CD/installed-root resolver. */
	bool hasResource(const Common::String &path) const;
	/** Return and clear the validated direct-practice request, if the startup command supplied one. */
	bool takePracticePuzzleLaunch(PageId &pageId, int &level, uint &partySize);
	/** Queue a console-only practice launch without exposing an unsupported map tier. */
	void queueDebugPracticeLaunch(PageId pageId, int level, uint partySize);
	/** Return whether this page supports the debug-only level 4 practice tier. */
	static bool supportsInternalPracticeLevel4(PageId pageId);
	/** Return the practice-map level retained for this engine session. */
	int getPracticeLevel() const { return _practiceLevel; }
	/** Retain a validated practice-map level for later map visits. */
	void setPracticeLevel(int level);

	/** Pause or resume the gameplay clock for a game dialog. */
	void setDialogPaused(bool paused);

	/** Write the current game state to its active savefile, or succeed without writing while that file is locked. */
	bool writeGameSave(const Common::String &savefileName);
	/** Return whether the per-savefile write-lock controls are enabled for this target. */
	bool isSavefileReadOnlyToggleEnabled() const;
	/** Return whether automatic writes are locked for @p savefileName during this game session. */
	bool isGameSaveWriteLocked(const Common::String &savefileName) const;
	/** Toggle automatic writes for one writable savefile and report whether its state changed. */
	bool toggleGameSaveWriteLock(const Common::String &savefileName);
	/** Create a savefile for a new saved game without overwriting another one. */
	bool createGameSave(const Common::String &savefileName);
	/** Replace a savefile after the caller obtains explicit confirmation. */
	bool overwriteGameSave(const Common::String &savefileName);
	/** Load the saved game stored under @p savefileName. */
	bool readGameSave(const Common::String &savefileName);
	/** Delete the active target's savefile named @p savefileName. */
	bool deleteGameSave(const Common::String &savefileName);
	/** Return the active target's valid savefile names in display order. */
	Common::StringArray listGameSaves() const;
	/** Return whether the active target's listed savefile for @p savefileName is read-only. */
	bool isGameSaveReadOnly(const Common::String &savefileName) const;
	/** Current game state, including its active party and rescue storage. */
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
	/** Return the active color-only presentation setting. */
	ColorAssistMode getColorAssistMode() const { return _colorAssistMode; }
	/** Return the gameplay millisecond snapshot captured before the current page pass. */
	uint32 getFrameTickCount() const { return _cachedGameTickCount; }
	/** Return gameplay milliseconds elapsed between the last two page passes. */
	uint32 getFrameDeltaMs() const { return _cachedGameTickCount - _prevFrameTickCount; }

	/** Gameplay random generator for this game instance. */
	Random *_rnd;
	/** Shared graphics interface used by pages and engine rendering. */
	Gfx *_gfx = nullptr;

	/** Whether the next puzzle entry restores the saved party retained by the current game state. */
	bool _returningFromPuzzle = false;
	/** Whether the active map-screen flow represents a saved adventure. */
	bool _isSavedGame = false;
	/** Shared byte-sized gameplay flag consumed by page flow. */
	byte _gameFlagB = 0;
	/** Selected Rescue Site I branch for the next map transition. */
	RouteBranch _routeDirection = RouteBranch::kNone00;
	/** Gameplay page shown as the source of the next map transition. */
	PageId _mapTransitionSourcePageId = kPageZombiniville;
	/** Destination whose debug map transition needs a fallback traveling party. */
	PageId _debugXferDestination = kPageNone;
	/** Explicit console transition level applied after the outgoing page is destroyed. */
	int _debugXferPracticeLevel = 0;
	/** Destination that receives the retained practice level after the transition. */
	PageId _debugXferPracticeTarget = kPageNone;
	/** Whether the debug transition starts a fresh practice session. */
	bool _debugXferResetState = false;
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
	int16 _selectedFeatures[ZmbTrait::kTraitKindCount] = {
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
	static constexpr uint kCursorCount = static_cast<uint>(CursorType::kRestoreBase);
	static constexpr const char *kCursorPaths[kCursorCount] = {
		"bmp/cursor/cursor01.rb",
		"bmp/cursor/cursor02.rb",
		"bmp/cursor/cursor03.rb",
		"bmp/cursor/cursor04.rb",
		"bmp/chez_norf/miam_sandwitch",
		"bmp/chez_norf/miam_poisson",
		"bmp/chez_norf/miam_salade",
		"bmp/chez_norf/glouglou_cafe",
		"bmp/chez_norf/glouglou_orange",
		"bmp/chez_norf/glouglou_lait",
		"bmp/chez_norf/slurp_tarte",
		"bmp/chez_norf/slurp_pasteque",
		"bmp/chez_norf/slurp_glace",
		"bmp/chez_norf/plato_mini"};
	static constexpr const char *kQuitConfirmationPath = "bmp/menu/Quit_panel_text_quit";
	static constexpr const char *kDebugZoombiniSetFileNameFormat = "%s-zoombini.set";
	/** Maximum editable roster-document size in bytes. */
	static constexpr uint32 kDebugZoombiniSetMaxSize = 4096;
	/** Maximum roster size accepted by the developer interchange format. */
	static constexpr int kDebugZoombiniSetMaxMembers = 16;

	/** Savefile selected by a load, create, or confirmed overwrite. */
	Common::String _activeSavefileName;
	/** Whether the active loaded savefile must retain its original bytes. */
	bool _activeSavefileReadOnly = false;
	/** Session-local automatic-write overrides keyed by savefile name. */
	Common::HashMap<Common::String, bool> _saveWriteLockOverrides;

	/** Write the current game state to @p savefileName after applying its save policy. */
	bool writeGameSavefile(const Common::String &savefileName);

	typedef Common::HashMap<Common::Path, ZoombiniAnimation *, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> ZoombiniAnimationCache;

	/** Cursor pixels derived once from a cached RLE sprite. */
	struct CursorImage {
		Size32 size;
		byte *pixels;

		explicit CursorImage(const Size32 &cursorSize);
		~CursorImage();
	};

	/** Return the unique child directory whose name matches @p name without case. */
	static Common::FSNode findChildDirectoryIgnoreCase(const Common::FSNode &directory, const char *name);
	/** Decode and validate a direct-practice boot parameter before the initial page is selected. */
	bool configurePracticeBootParamLaunch();
	/** Release closed engine dialogs after their event handlers and callbacks return. */
	void cleanupClosedDialogs();

	/** Original logical resource-name resolver for the CD and installed roots. */
	ResourceFileResolver *_resourceFileResolver = nullptr;
	/** Immutable animation sets shared by page lifetimes. */
	ZoombiniAnimationCache _zoombiniAnimationCache;

	/** Default or interactive cursor requested by the active page. */
	CursorType _baseCursorType = CursorType::kDefault;
	/** Page cursor displayed above the base cursor until restored. */
	CursorType _pageCursorType = CursorType::kRestoreBase;
	/** Cursor last registered with CursorMan. */
	CursorType _activeCursorType = CursorType::kDefault;
	/** Whether a cursor has been registered for this game instance. */
	bool _cursorRegistered = false;
	/** BGRA cursor pixels indexed by @ref CursorType, with page entries cleared on transition. */
	CursorImage *_cursorImages[kCursorCount] = {};
	/** Missing or invalid cursor art already reported for this cache lifetime. */
	bool _cursorUnavailable[kCursorCount] = {};
	/** Signed 16-bit hotspot offset used by the active cursor image. */
	Common::Point _cursorHotspot = Common::Point();
	/** Whether CursorMan should present the game cursor. */
	bool _cursorVisible = true;

	/** Sound manager for this game instance. */
	SoundManager *_soundManager = nullptr;

	/** Shared Help, Map, and Go controls for this game instance. */
	Sidebar *_sidebar = nullptr;
	/** Engine dialogs opened on demand; closed dialogs are retired after event dispatch. */
	Common::Array<DialogBase *> _dialogStack;

	/** Most recently processed game-space mouse position. */
	Common::Point32 _mousePos = Common::Point32();
	/** Ordered input events awaiting the current page's dispatch boundary. */
	Common::Array<Common::Event> _pendingPageEvents;
	/**
	 * Whether the tracked press began inside a shared modal dialog.
	 * Its release belongs to that gesture even if the press dismissed the modal.
	 */
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
	/** Optional console-requested party size; zero selects the route default. */
	uint _practicePartySize = 0;
	/** Reset the current adventure only after its active page releases borrowed state. */
	bool _debugPracticeResetState = false;
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
	/** Color assist mode, mainly for colorblinds. */
	ColorAssistMode _colorAssistMode = ColorAssistMode::kOriginal00;
	/** Whether Waterslide level one uses the alternate greedy pairing. */
	bool _useGreedyWaterslidePairing = false;
	/** Whether Aqua Cube level three protects its first direct lever move from a Fleen. */
	bool _useAquacubeSafeFirstMove = false;
	/** Whether Bezier paths use the optional floating-point evaluator. */
	bool _useFloatingPointPaths = false;
	/** Whether the departing Fleen omits the malformed white-nose frames. */
	bool _fixFleenDepartStreak = true;
	/** Whether the enhanced keyboard shortcut set is enabled. */
	bool _enhancedKbdShortcuts = false;
	/** Whether the practice map exposes recoverable level 4 puzzles. */
	bool _allowCutLevel4PracticePuzzles = false;
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
	/** Export the active party's trait tuples as uncompressed text through @ref Common::SaveFileManager. */
	void exportZoombiniSet() const;
	/** Validate the complete save-location trait file before applying any member changes. */
	void importZoombiniSet();
	/** Read one complete whitespace-delimited decimal token within a bounded text snapshot. */
	static bool readDebugSetInteger(const char *&cursor, const char *end, int32 &value);
	/** Apply the global puzzle-completion shortcut to the active roster and page. */
	bool applyDebugPuzzleCompletion();
	/** Release every Zoombini sprite grid cached by this game instance. */
	void clearZoombiniAnimationCache();
	/** Load and register the game cursor. */
	void initCursor();
	/** Convert @p sprite to BGRA cursor pixels. */
	CursorImage *createCursorImage(const RleBlock *sprite) const;
	/** Release derived cursor pixels beginning at @p firstIndex. */
	void clearCursorImages(uint firstIndex);
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
