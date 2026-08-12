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

#include "common/scummsys.h"
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/random.h"
#include "common/rect.h"
#include "common/array.h"

#include "engines/engine.h"
#include "engines/advancedDetector.h"

#include "graphics/managed_surface.h"
#include "graphics/surface.h"

namespace Zoombini2 {

class Zoombini2Engine;
class SoundManager;
class GameState;
class Page;
class Zoombini;
class RleBlock;
class BitBlock;
class Sidebar;

/**
 * Screen dimensions — hardcoded 800x600 as in original.
 */
const int kScreenWidth = 800;
const int kScreenHeight = 600;

/**
 * Page IDs — maps to the original page dispatcher (WorldDispatcher_461020).
 * Original class prefix: CL_ (e.g. CL_maptrans, CL_waterslide).
 */
enum PageId {
	kPageMenuLoad      = -4,
	kPageMenuNew       = -3,
	kPageMenuOptions   = -2,
	kPageNone          = -1,
	kPageZombiniville  = 0,
	kPageCrazyTurtle   = 1,
	kPageWaterslide    = 2,
	kPageAquacube      = 3,
	kPageRescue1       = 4,
	kPageMysticMarsh   = 5,
	kPageMagicWall     = 6,
	kPageWallOfFleens  = 7,
	kPageChezNorf      = 8,
	kPageRescue2       = 9,
	kPageSnowboard     = 10,
	kPageBoolies       = 11,
	kPageBooliewood    = 12,
	kPageCredits       = 16,
	kPageTLCLogo       = 17,
	kPageTitleAnim     = 18,
	kPageStoryBmp      = 20,
	kPageStoryAnim     = 21,
	kPageMapTrans      = 22,
	kPageFinal         = 23,
	kPageTitleScreen   = 24,
	kPageLogopoly      = 25,
	kPageWorldMap      = 30,
	kPageMenuAlt       = 40,
	kPageArisu         = 1972
};

/**
 * Zoombini feature types. Each zoombini has 4 features, values 1-5.
 */
enum ZoombiniFeature {
	kFeatureHair  = 0,
	kFeatureEyes  = 1,
	kFeatureNose  = 2,
	kFeatureFeet  = 3,
	kNumFeatures  = 4,
	kNumFeatureValues = 5
};

/**
 * Maximum zoombini pack size.
 * Zombiniville start: 16, after rescue: 8.
 */
const int kMaxPackSize = 16;
const int kPostRescuePackSize = 8;

/**
 * Total unique zoombini feature combinations: 5^4 = 625.
 */
const int kMaxCombinations = 625;

class Zoombini2Engine : public Engine {
public:
	Zoombini2Engine(OSystem *syst, const ADGameDescription *desc);
	~Zoombini2Engine() override;

	Common::Error run() override;

	bool hasFeature(EngineFeature f) const override;

	const ADGameDescription *_gameDescription;

	Common::RandomSource *getRandom() { return _rnd; }

	Common::Point getMousePos() const { return _mousePos; }
	bool isMouseDown() const { return _mouseDown; }
	bool isMouseClicked() const { return _mouseClicked; }
	uint32 getLastKeyPressed() const { return _lastKeyPressed; }

	Common::Language getLanguage() const { return _gameDescription->language; }

	Graphics::ManagedSurface *getScreen() { return _screen; }
	Graphics::ManagedSurface *getCurrentScreen() { return _screen; }
	const byte (*getAlphaLUT() const)[256] { return _alphaBlendLUT; }
	SoundManager *getSoundManager() { return _soundManager; }
	GameState *getGameState() { return _gameState; }
	int ensureMapMusic();
	void setMapMusicVolume(int volume);

	/**
	 * Load a BitBlock resource from archive
	 */
	BitBlock *loadBitBlock(const Common::String &path);

	/**
	 * Load an RleBlock resource from archive
	 */
	RleBlock *loadRleBlock(const Common::String &path);

	/**
	 * Check if a resource exists in the archive
	 */
	bool hasResource(const Common::String &path);

	/**
	 * Add to the pause time accumulator (for accurate timing)
	 */
	void addPauseTime(uint32 ms) { _pauseTimeAccum += ms; }

	/**
	 * Return the save directory FSNode (<gameDataDir>/save/).
	 * Creates the directory if it does not exist.
	 */
	Common::FSNode getSaveDir() const;

	/**
	 * Write the current game state to <saveDir>/<name>.mk.
	 * @return true on success.
	 */
	bool writeGameSave(const Common::String &name);

	/**
	 * Read game state from <saveDir>/<name>.mk.
	 * @return true on success.
	 */
	bool readGameSave(const Common::String &name);

	/**
	 * Delete <saveDir>/<name>.mk.
	 */
	void deleteGameSave(const Common::String &name);

	// Global zoombini list (original: g_zoombiniListBegin/End at 0x4E0904/08)
	Common::Array<Zoombini *> _globalZoombinis;

	// Page management
	void requestPageChange(int pageId) { _nextPageId = pageId; }
	void setQuitAfterCredits(bool quit) { _quitAfterCredits = quit; }
	bool getQuitAfterCredits() const { return _quitAfterCredits; }
	int getCurrentPageId() const { return _currentPageId; }
	Page *getCurrentPage() { return _currentPage; }

	// Timing (original: g_gameTickCount at 0x4E0910)
	uint32 getGameTickCount() const;

	// Global state flags
	bool _returningFromPuzzle;   // 0x572A62
	byte _gameFlagB;             // 0x572A61
	int _routeDirection;         // 0x572A5C: 1=left, 2=right
	int _maptransSourceWorld;    // 0x572A3C
	bool _isPaused;              // 0x571C50
	uint32 _pauseTimeAccum;      // 0x571D58
	uint32 _pauseTimeStart;      // 0x571D5C
	bool _zoombiniWalkingFlag;   // 0x571D9C
	bool _skipMode;              // 0x561C08
	int _lastRouteDirection;     // 0x561C0C

	// Selected features for Zombiniville (original: g_selectedFeatures at 0x4B9FAE)
	int16 _selectedFeatures[kNumFeatures]; // 0xFFFF = unset, 1-5 = selected

	// Global world transition timer
	int _worldTransitionTimer;   // 0x572A40
	int _transitionState;        // 0x571DFC

private:
	Common::RandomSource *_rnd;

	// Graphics
	Graphics::ManagedSurface *_screen;

	// Cursor system — original: Cursor__LoadSprites_45B630, Cursor__DrawAndSave_45B830
	// Uses RleBlock sprites from Bmp/cursor/cursor01-04.rb
	RleBlock *_cursorSprite;     // Current cursor RleBlock
	int _cursorHotspotX;         // Hotspot X offset
	int _cursorHotspotY;         // Hotspot Y offset
	bool _cursorVisible;         // Cursor visibility flag

	// Audio
	SoundManager *_soundManager;
	int _mapMusicId;

	// Game state (CGame)
	GameState *_gameState;

	// Sidebar UI system
	Sidebar *_sidebar;

	// Input
	Common::Point _mousePos;
	bool _mouseDown;
	bool _mouseClicked;
	uint32 _lastKeyPressed;

	// Page system
	int _currentPageId;
	int _nextPageId;
	Page *_currentPage;
	bool _quitAfterCredits;   ///< When true, CreditsPage exits the game on finish

	// Timing
	uint32 _startTime;

	// Alpha blend lookup table (256x256) — original at 0x561C30
	byte _alphaBlendLUT[256][256];

	void initAlphaLUT();
	void stopMapMusic();
	void initCursor();
	void registerCursorWithCursorMan();
	void drawCursor();
	void processEvents();
	void mainGameLoop();
	void switchPage(int pageId);
	void destroyCurrentPage();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_ZOOMBINI2_H
