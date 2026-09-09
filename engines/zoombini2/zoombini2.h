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
#include "common/random.h"
#include "common/rect.h"
#include "common/scummsys.h"

#include "engines/engine.h"

#include "graphics/managed_surface.h"
#include "graphics/surface.h"

#include "zoombini2/detection.h"

namespace Zoombini2 {

class BitBlock;
class GameState;
class Page;
class RleBlock;
class Sidebar;
class SoundManager;
class ZoombiniState;

/** Width of the fixed internal game screen. */
const int kScreenWidth = 800;

/** Height of the fixed internal game screen. */
const int kScreenHeight = 600;

/** Numeric page identifiers accepted by the engine dispatcher. */
enum PageId {
	kPageMenuLoad = -4,    ///< Saved-adventure sign-in flow.
	kPageMenuNew = -3,     ///< Practice-map sign-in flow.
	kPageMenuOptions = -2, ///< Sign-in screen.
	kPageNone = -1,        ///< No dispatched page.
	kPageZombiniville = 0, ///< Zoombiniville party-assembly shelter.
	kPageCrazyTurtle = 1,  ///< Turtle Hurdle puzzle.
	kPageWaterslide = 2,   ///< Pipes of Paloo puzzle.
	kPageAquacube = 3,     ///< Aqua Cube puzzle.
	kPageRescue1 = 4,      ///< First rescue-site shelter.
	kPageMysticMarsh = 5,  ///< Bubble Bumpers puzzle.
	kPageMagicWall = 6,    ///< Beetle Bug Alley puzzle.
	kPageWallOfFleens = 7, ///< Magic Mirrors puzzle.
	kPageChezNorf = 8,     ///< Chez Norf puzzle.
	kPageRescue2 = 9,      ///< Second rescue-site shelter.
	kPageSnowboard = 10,   ///< Snowboard Gulch puzzle.
	kPageBoolies = 11,     ///< Boolie Boggle puzzle.
	kPageBooliewood = 12,  ///< Booliewood arrival shelter.
	kPageCredits = 16,     ///< Credits transition.
	kPageTLCLogo = 17,     ///< Publisher-logo video.
	kPageTitleAnim = 18,   ///< First story video.
	kPageStoryBmp = 20,    ///< Second story video.
	kPageStoryAnim = 21,   ///< Third story video.
	kPageMapTrans = 22,    ///< Route-map travel transition.
	kPageFinal = 23,       ///< Booliewood final-celebration shelter.
	kPageTitleScreen = 24, ///< Title screen.
	kPageLogopoly = 25,    ///< Logopoly video.
	kPageWorldMap = 30,    ///< ScummVM world-map dispatcher alias.
	kPageMenuAlt = 40,     ///< Alternate sign-in route.
	kPageArisu = 1972      ///< Arisu splash video.
};

/** Visible feature positions in a @ref ZoombiniState. */
enum ZoombiniFeature {
	kFeatureHair = 0,     ///< Hair feature slot.
	kFeatureEyes = 1,     ///< Eyes feature slot.
	kFeatureNose = 2,     ///< Nose feature slot.
	kFeatureFeet = 3,     ///< Feet feature slot.
	kNumFeatures = 4,     ///< Number of visible feature slots.
	kNumFeatureValues = 5 ///< Number of values available in each feature slot.
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
 * global party, sidebar, sound manager, cursor resources, and exactly one
 * dispatched @ref Page at a time.
 */
class Zoombini2Engine : public Engine {
public:
	/** Construct an engine for one detected release. */
	Zoombini2Engine(OSystem *syst, const Zoombini2GameDescription *desc);
	/** Release the active page and all engine-owned session resources. */
	~Zoombini2Engine() override;

	/** Initialize resources and run the main page loop. */
	Common::Error run() override;
	/** Return whether the engine exposes @p feature. */
	bool hasFeature(EngineFeature feature) const override;

	/** Detected release descriptor retained for the engine lifetime. */
	const Zoombini2GameDescription *_gameDescription;

	/** Return the engine-owned gameplay random generator. */
	Common::RandomSource *getRandom() { return _rnd; }
	/** Return the most recently processed game-space mouse position. */
	Common::Point getMousePos() const { return _mousePos; }
	/** Return whether the primary mouse button is currently held. */
	bool isMouseDown() const { return _mouseDown; }
	/** Return whether the primary mouse button was pressed during this frame. */
	bool isMouseClicked() const { return _mouseClicked; }
	/** Return the most recently processed key code. */
	uint32 getLastKeyPressed() const { return _lastKeyPressed; }

	/** Return the detected release language. */
	Common::Language getLanguage() const { return _gameDescription->desc.language; }
	/** Return the detected release feature flags. */
	uint32 getFeatures() const { return _gameDescription->features; }

	/** Return the engine-owned drawing surface. */
	Graphics::ManagedSurface *getScreen() { return _screen; }
	/** Return the engine-owned drawing surface used by the active page. */
	Graphics::ManagedSurface *getCurrentScreen() { return _screen; }
	/** Return the immutable per-channel alpha-blending lookup table. */
	const byte (*getAlphaLUT() const)[256] { return _alphaBlendLUT; }
	/** Return the engine-owned sound manager. */
	SoundManager *getSoundManager() { return _soundManager; }
	/** Return the engine-owned active profile state. */
	GameState *getGameState() { return _gameState; }
	/** Start or retain the shared map-music stream and return its sound identifier. */
	int ensureMapMusic();
	/** Set the volume of the shared map-music stream. */
	void setMapMusicVolume(int volume);

	/** Load a BitBlock through the configured loose-file search paths. */
	BitBlock *loadBitBlock(const Common::String &path);
	/** Load an RLE block through the configured loose-file search paths. */
	RleBlock *loadRleBlock(const Common::String &path);
	/** Return whether @p path resolves through the configured search paths. */
	bool hasResource(const Common::String &path);

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

	/** Engine-owned party entries not currently owned by @ref GameState. */
	Common::Array<ZoombiniState *> _globalZoombinis;

	/** Request that the main loop replace the active page with @p pageId. */
	void requestPageChange(int pageId) { _nextPageId = pageId; }
	/** Return whether the pending transition leads back to a map or sign-in flow. */
	bool isReturningToMap() const { return _nextPageId == kPageMenuLoad || _nextPageId == kPageMenuNew || _nextPageId == kPageWorldMap; }
	/** Return whether the pending transition uses the route-map travel page. */
	bool isAdvancingWorld() const { return _nextPageId == kPageMapTrans; }
	/** Return the active page identifier. */
	int getCurrentPageId() const { return _currentPageId; }
	/** Return the borrowed active page. */
	Page *getCurrentPage() { return _currentPage; }

	/** Return elapsed gameplay milliseconds with accumulated pause time removed. */
	uint32 getGameTickCount() const;

	/** Whether the next puzzle entry restores the profile-owned party. */
	bool _returningFromPuzzle;
	/** Whether the active world-map flow represents a saved adventure. */
	bool _isSavedGame;
	/** Shared byte-sized gameplay flag consumed by page flow. */
	byte _gameFlagB;
	/** Selected rescue route, where 1 is left and 2 is right. */
	int _routeDirection;
	/** Gameplay world shown as the source of the next map transition. */
	int _maptransSourceWorld;
	/** Whether the engine's gameplay clock is currently paused. */
	bool _isPaused;
	/** Total paused time excluded from @ref Zoombini2Engine::getGameTickCount. */
	uint32 _pauseTimeAccum;
	/** System tick captured when the current pause began. */
	uint32 _pauseTimeStart;
	/** Whether at least one route-transition Zoombini is still walking. */
	bool _zoombiniWalkingFlag;
	/** Whether the current transition may skip its remaining presentation. */
	bool _skipMode;
	/** Most recently completed rescue-route direction. */
	int _lastRouteDirection;

	/** Selected Zombiniville feature values, or -1 for an unselected slot. */
	int16 _selectedFeatures[kNumFeatures];

	/** Deadline or countdown used by the active world transition. */
	int _worldTransitionTimer;
	/** Shared stage value used by world-transition flow. */
	int _transitionState;

private:
	/** Select the full-size movie when present, otherwise return the half-size path. */
	static Common::Path selectMoviePath(const char *fullSizePath, const char *halfSizePath);

	/** Engine-owned gameplay random generator. */
	Common::RandomSource *_rnd;
	/** Engine-owned fixed-size drawing surface. */
	Graphics::ManagedSurface *_screen;

	/** Engine-owned cursor sprite. */
	RleBlock *_cursorSprite;
	/** Signed 16-bit hotspot offset used by the active cursor image. */
	Common::Point _cursorHotspot;
	/** Whether CursorMan should present the game cursor. */
	bool _cursorVisible;

	/** Engine-owned sound manager. */
	SoundManager *_soundManager;
	/** Shared map-music sound identifier, or -1 when stopped. */
	int _mapMusicId;

	/** Engine-owned active profile state. */
	GameState *_gameState;
	/** Engine-owned global sidebar. */
	Sidebar *_sidebar;

	/** Most recently processed game-space mouse position. */
	Common::Point _mousePos;
	/** Whether the primary mouse button is currently held. */
	bool _mouseDown;
	/** Whether the primary mouse button was pressed during this frame. */
	bool _mouseClicked;
	/** Most recently processed key code. */
	uint32 _lastKeyPressed;

	/** Active page identifier. */
	int _currentPageId;
	/** Requested replacement page identifier. */
	int _nextPageId;
	/** Engine-owned active page, or nullptr between page lifetimes. */
	Page *_currentPage;

	/** System tick used as the gameplay-clock origin. */
	uint32 _startTime;
	/** Per-channel alpha-blending lookup table. */
	byte _alphaBlendLUT[256][256];

	/** Populate @ref Zoombini2Engine::_alphaBlendLUT. */
	void initAlphaLUT();
	/** Stop the shared map-music stream and clear its identifier. */
	void stopMapMusic();
	/** Load and register the game cursor. */
	void initCursor();
	/** Register the loaded cursor sprite with CursorMan. */
	void registerCursorWithCursorMan();
	/** Retain the cursor draw hook used by the main loop. */
	void drawCursor();
	/** Consume pending backend events and update frame-local input state. */
	void processEvents();
	/** Update, draw, and transition pages until the engine quits. */
	void mainGameLoop();
	/** Destroy the active page and construct @p pageId. */
	void switchPage(int pageId);
	/** Release the active page and clear its pointer. */
	void destroyCurrentPage();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_ZOOMBINI2_H
