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

#include <stdio.h>

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/system.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"

#include "zoombini2/zoombini2.h"
#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/pages/page.h"
#include "zoombini2/pages/video.h"
#include "zoombini2/pages/title.h"
#include "zoombini2/pages/zombiniville.h"
#include "zoombini2/pages/maptrans.h"
#include "zoombini2/pages/puzzle.h"
#include "zoombini2/pages/aquacube.h"
#include "zoombini2/pages/booliewood.h"
#include "zoombini2/pages/boolies.h"
#include "zoombini2/pages/cheznorf.h"
#include "zoombini2/pages/crazyturtle.h"
#include "zoombini2/pages/magicwall.h"
#include "zoombini2/pages/mysticmarsh.h"
#include "zoombini2/pages/snowboard.h"
#include "zoombini2/pages/walloffleens.h"
#include "zoombini2/pages/waterslide.h"
#include "zoombini2/pages/menuscreen.h"
#include "zoombini2/pages/rescue.h"
#include "zoombini2/pages/credits.h"
#include "zoombini2/pages/final.h"
#include "zoombini2/pages/worldmap.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/sidebar.h"

namespace Zoombini2 {

static Common::Path selectMoviePath(const char *fullSizePath, const char *halfSizePath) {
	const Common::Path fullSize(fullSizePath);
	if (SearchMan.hasFile(fullSize))
		return fullSize;

	return Common::Path(halfSizePath);
}

Zoombini2Engine::Zoombini2Engine(OSystem *syst, const ADGameDescription *desc)
	: Engine(syst), _gameDescription(desc) {

	_rnd = new Common::RandomSource("zoombini2");

	_screen = nullptr;
	_soundManager = nullptr;
	_mapMusicId = -1;
	_gameState = nullptr;
	_currentPage = nullptr;
	_sidebar = nullptr;

	// Cursor system
	_cursorSprite = nullptr;
	_cursorHotspotX = 0;
	_cursorHotspotY = 0;
	_cursorVisible = true;

	_mouseDown = false;
	_mouseClicked = false;
	_lastKeyPressed = 0;
	_currentPageId = kPageNone;
	_nextPageId = kPageTLCLogo;
	_quitAfterCredits = false;

	_startTime = 0;

	// Global state flags
	_returningFromPuzzle = false;
	_gameFlagB = 0;
	_routeDirection = 0;
	_maptransSourceWorld = 0;
	_isPaused = false;
	_pauseTimeAccum = 0;
	_pauseTimeStart = 0;
	_zoombiniWalkingFlag = false;
	_skipMode = false;
	_lastRouteDirection = 0;
	_worldTransitionTimer = 0;
	_transitionState = 0;

	for (int i = 0; i < kNumFeatures; i++)
		_selectedFeatures[i] = -1; // 0xFFFF = unset

	memset(_alphaBlendLUT, 0, sizeof(_alphaBlendLUT));
}

Zoombini2Engine::~Zoombini2Engine() {
	destroyCurrentPage();

	for (uint i = 0; i < _globalZoombinis.size(); i++)
		delete _globalZoombinis[i];
	_globalZoombinis.clear();

	delete _cursorSprite;
	delete _gameState;
	delete _soundManager;
	delete _sidebar;
	delete _screen;
	delete _rnd;
}

Common::FSNode Zoombini2Engine::getSaveDir() const {
	// Return <gameDataDir>/save/, creating it if needed.
	Common::FSNode gameDir(ConfMan.getPath("path"));
	Common::FSNode saveDir = gameDir.getChild("save");
	if (!saveDir.exists()) {
		saveDir.createDirectory();
	}
	return saveDir;
}

bool Zoombini2Engine::writeGameSave(const Common::String &name) {
	// Write game state to <saveDir>/<name>.mk.
	Common::FSNode saveDir = getSaveDir();
	Common::FSNode saveFile = saveDir.getChild(name + ".mk");
	Common::WriteStream *stream = saveFile.createWriteStream();
	if (!stream) {
		warning("Zoombini2Engine::writeGameSave: Could not open '%s.mk' for writing", name.c_str());
		return false;
	}

	bool ok = _gameState && _gameState->save(stream);
	delete stream;

	if (ok) {
		debug(1, "Saved game to %s.mk", name.c_str());
	} else {
		warning("Zoombini2Engine::writeGameSave: Failed to write '%s.mk'", name.c_str());
	}
	return ok;
}

bool Zoombini2Engine::readGameSave(const Common::String &name) {
	// Read game state from <saveDir>/<name>.mk.
	Common::FSNode saveDir = getSaveDir();
	Common::FSNode saveFile = saveDir.getChild(name + ".mk");
	if (!saveFile.exists()) {
		warning("Zoombini2Engine::readGameSave: '%s.mk' not found", name.c_str());
		return false;
	}

	Common::SeekableReadStream *stream = saveFile.createReadStream();
	if (!stream) {
		warning("Zoombini2Engine::readGameSave: Could not open '%s.mk' for reading", name.c_str());
		return false;
	}

	bool ok = _gameState && _gameState->load(stream);
	delete stream;

	if (ok) {
		debug(1, "Loaded game from %s.mk", name.c_str());
	} else {
		warning("Zoombini2Engine::readGameSave: Failed to load '%s.mk'", name.c_str());
	}
	return ok;
}

void Zoombini2Engine::deleteGameSave(const Common::String &name) {
	const Common::FSNode saveFile = getSaveDir().getChild(name + ".mk");
	const Common::String nativePath = saveFile.getPath().toString(Common::Path::kNativeSeparator);
	if (remove(nativePath.c_str()) != 0)
		warning("Zoombini2Engine::deleteGameSave: Failed to delete '%s.mk'", name.c_str());
}

int Zoombini2Engine::ensureMapMusic() {
	if (!_soundManager)
		return -1;

	if (_mapMusicId < 0)
		_mapMusicId = _soundManager->load(true, Common::Path("sounds/music/ZMR-MapScreen.wav"), true);
	if (0 <= _mapMusicId) {
		if (!_soundManager->isPlaying(_mapMusicId))
			_soundManager->playLoop(_mapMusicId);
		_soundManager->setVolume(_mapMusicId, _soundManager->_volumeMusic);
	}
	return _mapMusicId;
}

void Zoombini2Engine::setMapMusicVolume(int volume) {
	if (_soundManager && 0 <= _mapMusicId)
		_soundManager->setVolume(_mapMusicId, volume);
}

void Zoombini2Engine::stopMapMusic() {
	if (!_soundManager || _mapMusicId < 0)
		return;
	_soundManager->stop(_mapMusicId);
	_soundManager->unload(_mapMusicId);
	_mapMusicId = -1;
}

bool Zoombini2Engine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher);
}

Common::Error Zoombini2Engine::run() {
	// Add subdirectories for ISO layout support
	// BOTH folders are required: Data/ has main resources, INSTALL/HD/ has music and cached backgrounds
	// INSTALL/HD gets higher priority (1) so cached .bb files override raw .bmp from Data/
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	// "INSTALL/HD" is stored as "HD" in SearchSet (the leaf directory name)
	if (!SearchMan.hasArchive("HD"))
		SearchMan.addSubDirectoryMatching(gameDataDir, "INSTALL/HD", 1, 4);  // Priority 1 - music & cached files
	if (!SearchMan.hasArchive("Data"))
		SearchMan.addSubDirectoryMatching(gameDataDir, "Data", 0, 4);        // Priority 0 - main data

	// Initialize 800x600 32-bit graphics
	// Use RGBA8888 format (same as internal surfaces)
	Graphics::PixelFormat format32(4, 8, 8, 8, 8, 16, 8, 0, 24);
	::initGraphics(kScreenWidth, kScreenHeight, &format32);

	_screen = new Graphics::ManagedSurface(kScreenWidth, kScreenHeight, format32);

	// Initialize alpha blend LUT
	initAlphaLUT();

	// Initialize cursor system
	initCursor();

	// Initialize sound manager
	_soundManager = new SoundManager(_mixer);

	// Initialize game state
	_gameState = new GameState();
	_gameState->init();

	// Initialize sidebar UI system
	_sidebar = new Sidebar(this);

	_startTime = g_system->getMillis();

	// Run the main game loop
	mainGameLoop();

	return Common::kNoError;
}

/**
 * Initialize the alpha blending lookup table.
 * Original: WinMain at 0x463ED0.
 * Formula: LUT[alpha][value] = (alpha * value) >> 8
 */
void Zoombini2Engine::initAlphaLUT() {
	for (int alpha = 0; alpha < 256; alpha++) {
		for (int value = 0; value < 256; value++) {
			_alphaBlendLUT[alpha][value] = (alpha * value) >> 8;
		}
	}
}

/**
 * Initialize the cursor system.
 * Original: MainGameLoop_4651E0 loads cursor sprites from Bmp/cursor/cursor01-04.rb.
 * The cursor is a software sprite drawn over the game screen.
 *
 * We also register the cursor with CursorMan so it appears in the black
 * border area around the game screen when the window is larger than 800x600.
 */
void Zoombini2Engine::initCursor() {
	// Load cursor sprite from cursor01.rb (default cursor)
	_cursorSprite = new RleBlock();
	if (!_cursorSprite->loadFromFile(Common::Path("bmp/cursor/cursor01.rb"))) {
		warning("Zoombini2Engine: Failed to load cursor sprite");
		delete _cursorSprite;
		_cursorSprite = nullptr;
		return;
	}

	// Hotspot for cursor01 is at (0, 0) — top-left corner
	_cursorHotspotX = 0;
	_cursorHotspotY = 0;
	_cursorVisible = true;

	// Register cursor with CursorMan so it's visible in the black border area.
	// Render the RLE cursor data into an RGBA surface.
	registerCursorWithCursorMan();
}

/**
 * Draw the cursor sprite at the current mouse position.
 * Original: Cursor__DrawAndSave_45B830.
 *
 * Software cursor rendering is now disabled because the cursor is registered
 * with CursorMan in registerCursorWithCursorMan(), which handles rendering
 * at the backend level. This ensures the cursor is visible even in the
 * black border area outside the 800x600 game screen.
 */
void Zoombini2Engine::drawCursor() {
	// CursorMan handles cursor rendering — no software drawing needed.
}

/**
 * Convert the RLE cursor sprite to a pixel buffer and register with CursorMan.
 * This allows the cursor to be visible even in the black border area around
 * the game screen when the window is larger than 800x600.
 */
void Zoombini2Engine::registerCursorWithCursorMan() {
	if (!_cursorSprite || !_cursorSprite->isValid())
		return;

	int w = _cursorSprite->getWidth();
	int h = _cursorSprite->getHeight();
	if (w <= 0 || h <= 0)
		return;

	// Create BGRA buffer initialized to fully transparent
	// Using the same pixel format as the engine: BGRA8888
	// (bytesPerPixel=4, rBits=8, gBits=8, bBits=8, aBits=8,
	//  rShift=16, gShift=8, bShift=0, aShift=24)
	int bufSize = w * h * 4;
	byte *buf = new byte[bufSize]();  // zero-initialized = transparent black

	// Render RLE cursor sprite into the buffer.
	// RLE format after expand3to4bpp:
	//   2 bytes: effectiveHeight
	//   Spans: xOff(i16) + yOff(i16) + pixelCount(i16) + mode(u8) + pixelData(count*4)
	//   Mode 0: opaque pixels [B, G, R, pad]
	//   Mode 1: premultiplied alpha [premultB, premultG, premultR, invAlpha]

	// Access internal RLE data via drawToScreen onto a temporary surface,
	// then extract the alpha channel by rendering to both black and white backgrounds.

	// Render onto black background
	Graphics::ManagedSurface blackSurf(w, h, Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));
	blackSurf.fillRect(Common::Rect(w, h), blackSurf.format.ARGBToColor(255, 0, 0, 0));
	_cursorSprite->drawToScreen(&blackSurf, 0, 0, _alphaBlendLUT);

	// Render onto white background
	Graphics::ManagedSurface whiteSurf(w, h, Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));
	whiteSurf.fillRect(Common::Rect(w, h), whiteSurf.format.ARGBToColor(255, 255, 255, 255));
	_cursorSprite->drawToScreen(&whiteSurf, 0, 0, _alphaBlendLUT);

	// Derive alpha from the two renders:
	// For premultiplied alpha compositing: result = src_premult + invAlpha * dst / 255
	// On black (dst=0): result_black = src_premult
	// On white (dst=255): result_white = src_premult + invAlpha
	// So: invAlpha = result_white - result_black
	//     alpha = 255 - invAlpha
	//     color = src_premult * 255 / alpha (un-premultiply)
	const byte *blackPixels = (const byte *)blackSurf.getPixels();
	const byte *whitePixels = (const byte *)whiteSurf.getPixels();

	for (int i = 0; i < w * h; i++) {
		int bBlack = blackPixels[i * 4 + 0];
		int gBlack = blackPixels[i * 4 + 1];
		int rBlack = blackPixels[i * 4 + 2];

		int bWhite = whitePixels[i * 4 + 0];
		int gWhite = whitePixels[i * 4 + 1];
		int rWhite = whitePixels[i * 4 + 2];

		// invAlpha is the average of the per-channel differences
		int invAlpha = MAX(MAX(bWhite - bBlack, gWhite - gBlack), rWhite - rBlack);
		int alpha = 255 - invAlpha;

		if (alpha <= 0) {
			// Fully transparent
			buf[i * 4 + 0] = 0;
			buf[i * 4 + 1] = 0;
			buf[i * 4 + 2] = 0;
			buf[i * 4 + 3] = 0;
		} else {
			// Un-premultiply the colors
			buf[i * 4 + 0] = MIN(bBlack * 255 / alpha, 255); // B
			buf[i * 4 + 1] = MIN(gBlack * 255 / alpha, 255); // G
			buf[i * 4 + 2] = MIN(rBlack * 255 / alpha, 255); // R
			buf[i * 4 + 3] = (byte)alpha;                     // A
		}
	}

	// Register with CursorMan
	Graphics::PixelFormat cursorFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
	CursorMan.replaceCursor(buf, w, h, _cursorHotspotX, _cursorHotspotY,
	                        0, &cursorFormat);
	CursorMan.showMouse(true);

	delete[] buf;
}

uint32 Zoombini2Engine::getGameTickCount() const {
	uint32 elapsed = g_system->getMillis() - _startTime;
	if (_isPaused)
		elapsed -= _pauseTimeAccum + (g_system->getMillis() - _pauseTimeStart);
	else
		elapsed -= _pauseTimeAccum;
	return elapsed;
}

void Zoombini2Engine::processEvents() {
	Common::Event event;

	_mouseClicked = false;
	_lastKeyPressed = 0;

	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;

		case Common::EVENT_LBUTTONDOWN:
			_mouseDown = true;
			_mouseClicked = true;
			_mousePos = event.mouse;
			break;

		case Common::EVENT_LBUTTONUP:
			_mouseDown = false;
			_mousePos = event.mouse;
			break;

		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			break;

		case Common::EVENT_KEYDOWN:
			_lastKeyPressed = event.kbd.keycode;
			break;

		default:
			break;
		}
	}
}

/**
 * Main game loop — corresponds to MainGameLoop_4651E0.
 * Runs page update/draw cycle at approximately 30fps (original tick rate).
 */
void Zoombini2Engine::mainGameLoop() {
	// Korean release starts with Arisu logo video, others start with TLC Logo
	// Original: PageDispatcher_461020, case 1972 (kPageArisu) plays "arisu.bik"
	if (getLanguage() == Common::KO_KOR) {
		_nextPageId = kPageArisu;
	} else {
		_nextPageId = kPageTLCLogo;
	}

	while (!shouldQuit()) {
		processEvents();

		// Handle page transitions
		if (_nextPageId != kPageNone) {
			switchPage(_nextPageId);
			_nextPageId = kPageNone;
		}

		// Update current page
		if (_currentPage) {
			// Handle mouse movement for sidebar hover
			if (_sidebar) {
				_sidebar->handleMouseMove(_mousePos);
			}

			// Handle mouse clicks
			if (_mouseClicked) {
				// Check sidebar first (if help screen is active, it captures all clicks)
				bool handledBySidebar = false;
				if (_sidebar) {
					handledBySidebar = _sidebar->handleClick(_mousePos);
				}

				// If sidebar didn't handle it, pass to page
				if (!handledBySidebar) {
					_currentPage->handleClick(_mousePos);
				}
			}

			_currentPage->update();

			// Draw current page
			// Only clear screen if the page requests it — allows pages to
			// preserve frame buffer (double buffering like the original).
			if (_currentPage->needsScreenClear())
				_screen->fillRect(Common::Rect(kScreenWidth, kScreenHeight), 0);
			_currentPage->draw(_screen);

			// Draw sidebar on top of page (if visible)
			if (_sidebar) {
				_sidebar->draw(_screen);
			}
		} else {
			_screen->fillRect(Common::Rect(kScreenWidth, kScreenHeight), 0);
		}

		// Draw cursor on top of everything
		drawCursor();

		// Present to screen
		g_system->copyRectToScreen(_screen->getPixels(), _screen->pitch,
			0, 0, kScreenWidth, kScreenHeight);
		g_system->updateScreen();

		// ~30 fps (original game runs at approximately 30fps)
		g_system->delayMillis(33);
	}
}

void Zoombini2Engine::destroyCurrentPage() {
	if (_currentPage) {
		delete _currentPage;
		_currentPage = nullptr;
	}
	// Clear screen on page destroy to prevent stale content showing
	// when transitioning to a new page that uses double buffering.
	_screen->fillRect(Common::Rect(kScreenWidth, kScreenHeight), 0);
}

/**
 * Destroy the current page and instantiate the requested page.
 */
void Zoombini2Engine::switchPage(int pageId) {
	debug(1, "Zoombini2: Switching from page %d to page %d", _currentPageId, pageId);
	const bool usesMapMusic = pageId == kPageMenuLoad || pageId == kPageMenuNew ||
	                          pageId == kPageMenuOptions || pageId == kPageWorldMap ||
	                          pageId == kPageMenuAlt;
	if (!usesMapMusic)
		stopMapMusic();

	destroyCurrentPage();
	_currentPageId = pageId;

	switch (pageId) {
	case kPageTLCLogo:
		_currentPage = new VideoPage(this,
		    selectMoviePath("movies/tlclogo.bik", "movies/tlclogo50%.bik"),
		    kPageLogopoly);
		break;

	case kPageLogopoly:
		_currentPage = new VideoPage(this, Common::Path("movies/logopoly.bik"), kPageTitleScreen);
		break;

	case kPageTitleAnim:
		_currentPage = new VideoPage(this,
		    selectMoviePath("movies/zoom_movie1_100%.bik", "movies/zoom_movie1_50%.bik"),
		    kPageZombiniville);
		break;

	case kPageStoryBmp:
		if (_gameState)
			_gameState->markRescue1MoviePlayed();
		_currentPage = new VideoPage(this,
		    selectMoviePath("movies/zoom_movie2_100%.bik", "movies/zoom_movie2_50%.bik"),
		    kPageRescue1);
		break;

	case kPageStoryAnim:
		if (_gameState)
			_gameState->markRescue2MoviePlayed();
		_currentPage = new VideoPage(this,
		    selectMoviePath("movies/zoom_movie3_100%.bik", "movies/zoom_movie3_50%.bik"),
		    kPageRescue2);
		break;

	case kPageTitleScreen:
		_currentPage = new TitleScreen(this);
		break;

	case kPageMenuNew:
		_currentPage = new WorldMapPage(this, kWorldMapPractice);
		break;

	case kPageMenuLoad:
		_currentPage = new WorldMapPage(this, kWorldMapSavedGame);
		break;

	case kPageWorldMap:
		_currentPage = new WorldMapPage(this, kWorldMapSavedGame);
		break;

	case kPageMenuOptions:
	case kPageMenuAlt:
		// Open the save-file menu from the map's Parties button.
		_currentPage = new MenuScreenPage(this);
		break;

	case kPageZombiniville:
		_currentPage = new Zombiniville(this);
		break;

	case kPageMapTrans:
		_currentPage = new MapTransition(this);
		break;

	case kPageMysticMarsh:
		_currentPage = new MysticMarshPuzzle(this);
		break;

	case kPageChezNorf:
		_currentPage = new ChezNorfPuzzle(this);
		break;

	case kPageWallOfFleens:
		_currentPage = new WallOfFleensPuzzle(this);
		break;

	case kPageBooliewood:
		_currentPage = new BooliewoodPage(this);
		break;

	case kPageCrazyTurtle:
		_currentPage = new CrazyTurtlePuzzle(this);
		break;

	case kPageBoolies:
		_currentPage = new BooliesPuzzle(this);
		break;

	case kPageMagicWall:
		_currentPage = new MagicWallPuzzle(this);
		break;

	case kPageAquacube:
		_currentPage = new AquacubePuzzle(this);
		break;

	case kPageSnowboard:
		_currentPage = new SnowboardPuzzle(this);
		break;

	case kPageWaterslide:
		_currentPage = new WaterslidePuzzle(this);
		break;

	case kPageRescue1:
		// Original: Rescue1__Init_428A00, object size 0x88
		_currentPage = new RescuePage(this, 1);
		break;

	case kPageRescue2:
		// Original: Rescue2__Init_42ADC0, object size 0x54
		_currentPage = new RescuePage(this, 2);
		break;

	case kPageFinal:
		// Original: Final__Init_418740, object size 0x54
		_currentPage = new FinalPage(this);
		break;

	case kPageCredits: {
		// Original: Credits__Init_417E60 (0x417E60), object size 0x20.
		// Scrolling credits bitmap at 30px/sec; BGM: ZMR-Transition.wav.
		bool quitAfter = _quitAfterCredits;
		_quitAfterCredits = false;
		_currentPage = new CreditsPage(this, quitAfter);
		break;
	}

	case kPageArisu:
		_currentPage = new VideoPage(this, Common::Path("movies/arisu.bik"), kPageTLCLogo);
		break;

	default:
		warning("Zoombini2: Unknown page %d", pageId);
		break;
	}

	if (_currentPage)
		_currentPage->init();
}

BitBlock *Zoombini2Engine::loadBitBlock(const Common::String &path) {
	BitBlock *bb = new BitBlock();
	if (bb->load(Common::Path(path))) {
		return bb;
	}
	delete bb;
	return nullptr;
}

RleBlock *Zoombini2Engine::loadRleBlock(const Common::String &path) {
	RleBlock *rle = new RleBlock();
	if (rle->loadFromFile(Common::Path(path))) {
		return rle;
	}
	delete rle;
	return nullptr;
}

bool Zoombini2Engine::hasResource(const Common::String &path) {
	Common::File file;
	return file.exists(Common::Path(path));
}

} // End of namespace Zoombini2
