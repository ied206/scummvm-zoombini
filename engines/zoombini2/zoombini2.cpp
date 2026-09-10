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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/tokenizer.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"
#include "graphics/pixelformat.h"

#include "zoombini2/dialogs.h"
#include "zoombini2/graphics.h"
#include "zoombini2/pages/interactive_base.h"
#include "zoombini2/pages/interactive_map.h"
#include "zoombini2/pages/interactive_menu.h"
#include "zoombini2/pages/page_base.h"
#include "zoombini2/pages/puzzle_aquacube.h"
#include "zoombini2/pages/puzzle_base.h"
#include "zoombini2/pages/puzzle_boolies.h"
#include "zoombini2/pages/puzzle_cheznorf.h"
#include "zoombini2/pages/puzzle_crazyturtle.h"
#include "zoombini2/pages/puzzle_magicwall.h"
#include "zoombini2/pages/puzzle_mysticmarsh.h"
#include "zoombini2/pages/puzzle_snowboard.h"
#include "zoombini2/pages/puzzle_walloffleens.h"
#include "zoombini2/pages/puzzle_waterslide.h"
#include "zoombini2/pages/shelter_booliewood.h"
#include "zoombini2/pages/shelter_final.h"
#include "zoombini2/pages/shelter_rescue1.h"
#include "zoombini2/pages/shelter_rescue2.h"
#include "zoombini2/pages/shelter_zombiniville.h"
#include "zoombini2/pages/transition_credits.h"
#include "zoombini2/pages/transition_maptrans.h"
#include "zoombini2/pages/transition_title.h"
#include "zoombini2/pages/transition_video.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

const char *const kConfigDebugHotkeys = "debug_hotkeys";
const char *const kConfigStereoOutput = "stereo_output";
const char *const kConfigGreedyWaterslidePairing = "greedy_waterslide_pairing";
const char *const kConfigCachedFrameTime = "cached_frame_time";
const char *const kConfigUseFloatingPointPaths = "use_floating_point_paths";

/** Resolve original logical resource names against their distinct physical roots. */
class Zoombini2Engine::ResourceFileResolver {
public:
	ResourceFileResolver(const Common::FSNode &cdDataDirectory, const Common::FSNode &installedDirectory)
		: _cdDataDirectory(cdDataDirectory.isDirectory() ? new Common::FSDirectory(cdDataDirectory, 8) : nullptr),
		  _installedDirectory(installedDirectory.isDirectory() ? new Common::FSDirectory(installedDirectory, 8) : nullptr) {
	}

	~ResourceFileResolver() {
		delete _cdDataDirectory;
		delete _installedDirectory;
	}

	bool hasFile(const Common::String &path) const {
		Common::Path relativePath;
		const Common::FSDirectory *directory = resolvePath(path, relativePath);
		return directory && directory->hasFile(relativePath);
	}

	Common::SeekableReadStream *openFile(const Common::String &path) const {
		Common::Path relativePath;
		const Common::FSDirectory *directory = resolvePath(path, relativePath);
		return directory ? directory->createReadStreamForMember(relativePath) : nullptr;
	}

private:
	const Common::FSDirectory *resolvePath(const Common::String &path, Common::Path &relativePath) const {
		Common::String logicalPath(path);
		logicalPath.replace('\\', '/');
		uint offset = 0;
		const bool useInstalledDirectory = !logicalPath.empty() && logicalPath[0] == '#';
		if (useInstalledDirectory)
			offset += 1;
		else if (logicalPath.hasPrefixIgnoreCase("Data/"))
			offset += 5;
		if (offset < logicalPath.size() && logicalPath[offset] == '.')
			offset += 1;
		if (offset < logicalPath.size() && logicalPath[offset] == '/')
			offset += 1;
		if (logicalPath.size() <= offset)
			return nullptr;

		relativePath = Common::Path(logicalPath.substr(offset), '/');
		return useInstalledDirectory ? _installedDirectory : _cdDataDirectory;
	}

	Common::FSDirectory *_cdDataDirectory;
	Common::FSDirectory *_installedDirectory;
};

Common::Path Zoombini2Engine::selectMoviePath(const char *fullSizePath, const char *halfSizePath) const {
	if (hasResource(fullSizePath))
		return Common::Path(fullSizePath);

	return Common::Path(halfSizePath);
}

Common::FSNode Zoombini2Engine::findChildDirectoryIgnoreCase(const Common::FSNode &directory, const char *name) {
	Common::FSList children;
	if (!directory.getChildren(children, Common::FSNode::kListDirectoriesOnly))
		return Common::FSNode();

	Common::FSNode match;
	for (const Common::FSNode &child : children) {
		if (!child.getRealName().equalsIgnoreCase(name))
			continue;

		if (match.exists()) {
			warning("Zoombini2Engine: multiple child directories match '%s' without case", name);
			return Common::FSNode();
		}
		match = child;
	}

	return match;
}

void Zoombini2Engine::initializePath(const Common::FSNode &gamePath) {
	const Common::FSNode dataDir = findChildDirectoryIgnoreCase(gamePath, "Data");
	const Common::FSNode dataBmpDir = findChildDirectoryIgnoreCase(dataDir, "Bmp");
	const Common::FSNode installDir = findChildDirectoryIgnoreCase(gamePath, "INSTALL");
	const Common::FSNode installHdDir = findChildDirectoryIgnoreCase(installDir, "HD");
	const Common::FSNode rootBmpDir = findChildDirectoryIgnoreCase(gamePath, "Bmp");

	Common::FSNode cdDataRoot = dataDir;
	Common::FSNode installedRoot = installHdDir;
	if (!dataBmpDir.isDirectory() && rootBmpDir.isDirectory()) {
		// Some supported installed releases already present both logical trees in one flattened directory.
		cdDataRoot = gamePath;
		installedRoot = gamePath;
	}

	if (!cdDataRoot.isDirectory())
		warning("Zoombini2Engine: CD Data resource root is unavailable");
	if (!installedRoot.isDirectory())
		warning("Zoombini2Engine: installed resource root is unavailable");

	delete _resourceFileResolver;
	_resourceFileResolver = new ResourceFileResolver(cdDataRoot, installedRoot);
}

Zoombini2Engine::Zoombini2Engine(OSystem *syst, const Zoombini2GameDescription *desc)
	: Engine(syst), _gameDescription(desc) {

	_rnd = new Common::RandomSource("zoombini2");
	_resourceFileResolver = nullptr;

	_screen = nullptr;
	_soundManager = nullptr;
	_mapMusicId = -1;
	_gameState = nullptr;
	_currentPage = nullptr;
	_threeButtons = nullptr;
	_mainMenuDialog = new Zoombini2MenuDialog(this);

	// Cursor system
	_cursorSprite = nullptr;
	_cursorHotspot = Common::Point();
	_cursorVisible = true;

	_mouseDown = false;
	_currentPageId = kPageNone;
	_nextPageId = kPageLogoTLC;

	_startTime = 0;
	_cachedGameTickCount = 0;
	_debugHotkeysEnabled = false;
	_stereoOutputEnabled = false;
	_useGreedyWaterslidePairing = false;
	_useCachedFrameTime = false;
	_useFloatingPointPaths = false;
	_debugCompletionKeyDown = false;
	_debugOverlayKeyDown = false;

	// Global state flags
	_returningFromPuzzle = false;
	_isSavedGame = false;
	_gameFlagB = 0;
	_routeDirection = RouteBranch::kNone00;
	_mapTransitionSourcePageId = kPageZombiniville;
	_isPaused = false;
	_pauseTimeAccum = 0;
	_pauseTimeStart = 0;
	_zoombiniWalkingFlag = false;
	_skipMode = false;
	_lastRouteDirection = RouteBranch::kNone00;
	_pageTransitionTimer = 0;
	_transitionState = 0;

	for (int i = 0; i < ZmbTrait::kTraitCount; i++)
		_selectedFeatures[i] = -1;

	refreshEngineSettings();
}

Zoombini2Engine::~Zoombini2Engine() {
	_nextPageId = kPageNone;
	destroyCurrentPage();
	if (_gameState)
		writeGameSave(_gameState->_playerName);
	clearGlobalZoombinis();
	clearZoombiniAnimationCache();

	delete _cursorSprite;
	delete _gameState;
	delete _threeButtons;
	delete _soundManager;
	delete _screen;
	delete _rnd;
	delete _resourceFileResolver;
}

void Zoombini2Engine::clearGlobalZoombinis() {
	for (uint i = 0; i < _globalZoombinis.size(); i++)
		delete _globalZoombinis[i];
	_globalZoombinis.clear();
}

void Zoombini2Engine::recordBooliesCompletion() {
	if (!_gameState || _globalZoombinis.empty() || !_globalZoombinis[0])
		return;

	const int rescuedBooliesPerZoombini = _globalZoombinis[0]->_rescuedBooliesPerZoombini;
	_gameState->_rescuedBoolieCount += static_cast<int32>(_globalZoombinis.size()) * rescuedBooliesPerZoombini;
	for (uint i = 0; i < _globalZoombinis.size(); i++) {
		if (_globalZoombinis[i])
			_gameState->recordCompletedZoombini(*_globalZoombinis[i]);
	}
}

const ZoombiniAnimation *Zoombini2Engine::loadZoombiniAnimation(const Common::Path &path) {
	ZoombiniAnimationCache::const_iterator cached = _zoombiniAnimationCache.find(path);
	if (cached != _zoombiniAnimationCache.end())
		return cached->_value;

	ZoombiniAnimation *animation = new ZoombiniAnimation();
	if (!animation->loadFromFile(path)) {
		delete animation;
		return nullptr;
	}
	_zoombiniAnimationCache[path] = animation;
	return animation;
}

void Zoombini2Engine::clearZoombiniAnimationCache() {
	for (ZoombiniAnimationCache::iterator entry = _zoombiniAnimationCache.begin(); entry != _zoombiniAnimationCache.end(); entry++)
		delete entry->_value;
	_zoombiniAnimationCache.clear();
}

bool Zoombini2Engine::writeGameSave(const Common::String &name) {
	if (!_gameState)
		return false;
	Zoombini2SavegameManager savegameManager(_saveFileMan, ConfMan.getActiveDomainName());
	return savegameManager.saveProfile(name, *_gameState, &_globalZoombinis);
}

bool Zoombini2Engine::readGameSave(const Common::String &name) {
	if (!_gameState)
		return false;
	Zoombini2SavegameManager savegameManager(_saveFileMan, ConfMan.getActiveDomainName());
	return savegameManager.loadProfile(name, *_gameState);
}

bool Zoombini2Engine::deleteGameSave(const Common::String &name) {
	Zoombini2SavegameManager savegameManager(_saveFileMan, ConfMan.getActiveDomainName());
	return savegameManager.deleteProfile(name);
}

Common::StringArray Zoombini2Engine::listGameSaves() const {
	Zoombini2SavegameManager savegameManager(_saveFileMan, ConfMan.getActiveDomainName());
	return savegameManager.listProfiles();
}

int Zoombini2Engine::ensureMapMusic() {
	if (!_soundManager)
		return -1;

	if (_mapMusicId < 0)
		_mapMusicId = _soundManager->load(true, Common::Path("#sounds/music/ZMR-MapScreen.wav"), true);
	if (0 <= _mapMusicId) {
		if (!_soundManager->isPlaying(_mapMusicId))
			_soundManager->playLoop(_mapMusicId);
		_soundManager->setVolume(_mapMusicId, _soundManager->_volumeMusic);
	}
	return _mapMusicId;
}

int Zoombini2Engine::getMusicVolume() const {
	return _soundManager ? _soundManager->_volumeMusic : mixerVolumeToPercent(ConfMan.getInt("music_volume"));
}

int Zoombini2Engine::getSFXVolume() const {
	return _soundManager ? _soundManager->_volumeSFX : mixerVolumeToPercent(ConfMan.getInt("sfx_volume"));
}

int Zoombini2Engine::getSpeechVolume() const {
	return _soundManager ? _soundManager->_volumeSpeech : mixerVolumeToPercent(ConfMan.getInt("speech_volume"));
}

void Zoombini2Engine::previewSoundVolumes(int music, int sfx, int speech) {
	if (_soundManager)
		_soundManager->setVolumeSettings(music, sfx, speech);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, percentToMixerVolume(music));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, percentToMixerVolume(sfx));
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, percentToMixerVolume(speech));
}

void Zoombini2Engine::saveSoundVolumes(int music, int sfx, int speech) {
	previewSoundVolumes(music, sfx, speech);
	ConfMan.setInt("music_volume", percentToMixerVolume(music));
	ConfMan.setInt("sfx_volume", percentToMixerVolume(sfx));
	ConfMan.setInt("speech_volume", percentToMixerVolume(speech));
	ConfMan.flushToDisk();
}

void Zoombini2Engine::stopMapMusic() {
	if (!_soundManager || _mapMusicId < 0)
		return;
	_soundManager->stop(_mapMusicId);
	_soundManager->unload(_mapMusicId);
	_mapMusicId = -1;
}

bool Zoombini2Engine::hasFeature(EngineFeature f) const {
	return f == kSupportsReturnToLauncher || f == kSupportsChangingOptionsDuringRuntime;
}

void Zoombini2Engine::syncSoundSettings() {
	Engine::syncSoundSettings();
	if (_soundManager) {
		_soundManager->setVolumeSettings(mixerVolumeToPercent(ConfMan.getInt("music_volume")), mixerVolumeToPercent(ConfMan.getInt("sfx_volume")),
										 mixerVolumeToPercent(ConfMan.getInt("speech_volume")));
	}
}

void Zoombini2Engine::applyGameSettings() {
	refreshEngineSettings();
	if (_soundManager)
		_soundManager->setStereoOutputEnabled(_stereoOutputEnabled);
}

Common::Error Zoombini2Engine::run() {
	// Initialize 800x600 32-bit graphics
	// Use RGBA8888 format (same as internal surfaces)
	Graphics::PixelFormat format32(4, 8, 8, 8, 8, 16, 8, 0, 24);
	::initGraphics(kScreenWidth, kScreenHeight, &format32);

	_screen = new ManagedSurface32(kScreenWidth, kScreenHeight, format32);

	// Initialize cursor system
	initCursor();

	// Initialize sound manager
	_soundManager = new SoundManager(this, _mixer);
	_soundManager->setStereoOutputEnabled(_stereoOutputEnabled);
	syncSoundSettings();

	// Initialize game state
	_gameState = new GameState();

	// Initialize the shared Help, Map, and Go controls.
	_threeButtons = new ThreeButtons(this);

	_startTime = g_system->getMillis();
	_cachedGameTickCount = 0;

	// Run the main game loop
	mainGameLoop();

	return Common::kNoError;
}

/**
 * Load the four game cursor sprites and register the default cursor.
 *
 * CursorMan keeps the cursor visible over both the game viewport and any surrounding border.
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

	// The default cursor hotspot is its top-left corner.
	_cursorHotspot = Common::Point();
	_cursorVisible = true;

	// Register cursor with CursorMan so it's visible in the black border area.
	// Render the RLE cursor data into an RGBA surface.
	registerCursorWithCursorMan();
}

/** Leave cursor drawing to CursorMan, which owns the active hardware cursor. */
void Zoombini2Engine::drawCursor() {
	// CursorMan handles cursor rendering, so no software drawing is needed.
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
	byte *buf = new byte[bufSize](); // zero-initialized = transparent black

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
	_cursorSprite->drawToScreen(&blackSurf, Common::Point32(0, 0), _alphaBlendLUT);

	// Render onto white background
	Graphics::ManagedSurface whiteSurf(w, h, Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));
	whiteSurf.fillRect(Common::Rect(w, h), whiteSurf.format.ARGBToColor(255, 255, 255, 255));
	_cursorSprite->drawToScreen(&whiteSurf, Common::Point32(0, 0), _alphaBlendLUT);

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
			buf[i * 4 + 3] = (byte)alpha;                    // A
		}
	}

	// Register with CursorMan
	Graphics::PixelFormat cursorFormat(4, 8, 8, 8, 8, 16, 8, 0, 24);
	CursorMan.replaceCursor(buf, w, h, _cursorHotspot.x, _cursorHotspot.y,
							0, &cursorFormat);
	CursorMan.showMouse(true);

	delete[] buf;
}

uint32 Zoombini2Engine::getGameTickCount() const {
	return _useCachedFrameTime ? _cachedGameTickCount : calculateGameTickCount();
}

uint32 Zoombini2Engine::calculateGameTickCount() const {
	const uint32 now = g_system->getMillis();
	uint32 elapsed = now - _startTime;
	if (_isPaused)
		elapsed -= _pauseTimeAccum + (now - _pauseTimeStart);
	else
		elapsed -= _pauseTimeAccum;
	return elapsed;
}

int Zoombini2Engine::mixerVolumeToPercent(int volume) {
	return CLIP((CLIP<int>(volume, 0, Audio::Mixer::kMaxMixerVolume) * kMaxVolumePercent + 128) / 256, 0, kMaxVolumePercent);
}

int Zoombini2Engine::percentToMixerVolume(int volume) {
	return MIN<int>(Audio::Mixer::kMaxMixerVolume, (CLIP(volume, 0, kMaxVolumePercent) * 256) / kMaxVolumePercent);
}

void Zoombini2Engine::refreshEngineSettings() {
	_debugHotkeysEnabled = ConfMan.getBool(kConfigDebugHotkeys);
	_stereoOutputEnabled = ConfMan.getBool(kConfigStereoOutput);
	_useGreedyWaterslidePairing = ConfMan.getBool(kConfigGreedyWaterslidePairing);
	_useCachedFrameTime = ConfMan.getBool(kConfigCachedFrameTime);
	_useFloatingPointPaths = ConfMan.getBool(kConfigUseFloatingPointPaths);
	if (!_debugHotkeysEnabled) {
		_debugCompletionKeyDown = false;
		_debugOverlayKeyDown = false;
	}
}

void Zoombini2Engine::exportZoombiniSet() const {
	if (_globalZoombinis.empty())
		return;

	Common::FSNode outputNode(Common::Path("zoombini.set"));
	Common::SeekableWriteStream *output = outputNode.createWriteStream(false);
	if (!output)
		return;

	output->writeString(Common::String::format("%u\n", _globalZoombinis.size()));
	for (uint i = 0; i < _globalZoombinis.size(); i++) {
		const ZoombiniState *zoombini = _globalZoombinis[i];
		if (!zoombini)
			continue;
		output->writeString(Common::String::format("%u %u %u %u\n", zoombini->_traits._feet, zoombini->_traits._nose, zoombini->_traits._hair,
												   zoombini->_traits._eyes));
	}
	output->finalize();
	delete output;
}

void Zoombini2Engine::importZoombiniSet() {
	Common::FSNode inputNode(Common::Path("zoombini.set"));
	Common::SeekableReadStream *input = inputNode.createReadStream();
	if (!input)
		return;

	const uint fileCount = static_cast<uint>(input->readLine().asUint64());
	const uint importCount = MIN<uint>(fileCount, _globalZoombinis.size());
	for (uint i = 0; i < importCount && !input->eos(); i++) {
		Common::StringTokenizer tokens(input->readLine());
		byte values[ZmbTrait::kTraitCount];
		bool completeTuple = true;
		for (int traitIndex = 0; traitIndex < ZmbTrait::kTraitCount; traitIndex++) {
			if (tokens.empty()) {
				completeTuple = false;
				break;
			}
			values[traitIndex] = static_cast<byte>(tokens.nextToken().asUint64());
		}
		if (!completeTuple)
			break;
		if (_globalZoombinis[i])
			_globalZoombinis[i]->setTraits(ZmbTrait(values[0], values[1], values[2], values[3]));
	}
	delete input;
}

void Zoombini2Engine::applyDebugPuzzleCompletion() {
	if (!_debugHotkeysEnabled || !_debugCompletionKeyDown)
		return;
	if (_currentPageId == kPageZombiniville || _currentPageId == kPageRescue1 || _currentPageId == kPageRescue2 || _currentPageId == kPageBooliewood ||
		_currentPageId == kPageFinal)
		return;

	for (uint i = 0; i < _globalZoombinis.size(); i++) {
		if (_globalZoombinis[i])
			_globalZoombinis[i]->_puzzleStatus = 1;
	}
	_zoombiniWalkingFlag = true;
	if (_currentPage)
		_currentPage->applyDebugPuzzleCompletion();
}

void Zoombini2Engine::processEvents() {
	Common::Event event;
	_pendingPageEvents.clear();

	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;
		case Common::EVENT_MAINMENU:
			openMainMenuDialog();
			break;
		case Common::EVENT_LBUTTONDOWN:
			_pendingPageEvents.push_back(event);
			_mouseDown = true;
			_mousePos = event.mouse;
			break;
		case Common::EVENT_LBUTTONUP:
			_pendingPageEvents.push_back(event);
			_mouseDown = false;
			_mousePos = event.mouse;
			break;
		case Common::EVENT_MOUSEMOVE:
			_pendingPageEvents.push_back(event);
			_mousePos = event.mouse;
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.keycode == Common::KEYCODE_F5) {
				openMainMenuDialog();
				break;
			}
			if (_debugHotkeysEnabled) {
				if (event.kbd.keycode == Common::KEYCODE_F2)
					exportZoombiniSet();
				else if (event.kbd.keycode == Common::KEYCODE_F3)
					importZoombiniSet();
				else if (event.kbd.keycode == Common::KEYCODE_p)
					_debugCompletionKeyDown = true;
				else if (event.kbd.keycode == Common::KEYCODE_c)
					_debugOverlayKeyDown = true;
			}
			_pendingPageEvents.push_back(event);
			break;
		case Common::EVENT_KEYUP:
			if (event.kbd.keycode == Common::KEYCODE_p)
				_debugCompletionKeyDown = false;
			else if (event.kbd.keycode == Common::KEYCODE_c)
				_debugOverlayKeyDown = false;
			_pendingPageEvents.push_back(event);
			break;
		default:
			break;
		}
	}
}

/** Run page updates, drawing, cursor changes, and presentation at approximately 30 frames per second. */
void Zoombini2Engine::mainGameLoop() {
	// The Korean release starts with the Arisu logo; other releases start with the TLC logo.
	if (getLanguage() == Common::KO_KOR) {
		_nextPageId = kPageLogoArisuMedia;
	} else {
		_nextPageId = kPageLogoTLC;
	}

	while (!shouldQuit()) {
		_cachedGameTickCount = calculateGameTickCount();
		processEvents();
		if (shouldQuit())
			break;

		// Handle page transitions
		if (_nextPageId != kPageNone) {
			const int requestedPage = _nextPageId;
			_nextPageId = kPageNone;
			switchPage(requestedPage);
			_pendingPageEvents.clear();
		}

		// Update current page
		if (_currentPage) {
			applyDebugPuzzleCompletion();
			const bool dialogWasActive = _threeButtons && _threeButtons->hasActiveDialog();
			bool modalInputBlocked = dialogWasActive;
			const Common::Point32 polledMousePos = _mousePos;
			const bool polledMouseDown = _mouseDown;
			for (const Common::Event &event : _pendingPageEvents) {
				if (_nextPageId != kPageNone)
					break;
				modalInputBlocked = modalInputBlocked || (_threeButtons && _threeButtons->hasActiveDialog());
				const bool pageDialogWasActive = _currentPage->hasActiveDialog();
				if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP || event.type == Common::EVENT_MOUSEMOVE)
					_mousePos = event.mouse;
				if (event.type == Common::EVENT_LBUTTONDOWN)
					_mouseDown = true;
				else if (event.type == Common::EVENT_LBUTTONUP)
					_mouseDown = false;
				EventHandleResult result = EventHandleResult::kPassthrough;
				if (_threeButtons)
					result = _threeButtons->handleEvent(event);
				if (result == EventHandleResult::kPassthrough && !modalInputBlocked)
					_currentPage->handleEvent(event);
				if ((pageDialogWasActive && !_currentPage->hasActiveDialog()) ||
					(modalInputBlocked && (!_threeButtons || !_threeButtons->hasActiveDialog())))
					break;
			}
			_mousePos = polledMousePos;
			_mouseDown = polledMouseDown;

			const bool dialogActive = dialogWasActive || (_threeButtons && _threeButtons->hasActiveDialog());
			if (!dialogActive)
				_currentPage->onFrame(_screen, _nextPageId == kPageNone);

			// The shared sidebar polls the frame mouse state before it draws its controls.
			if (_threeButtons) {
				_threeButtons->drawAndHandleInput(_screen, _nextPageId == kPageNone);
			}
		} else {
			_screen->fillRect(Common::Rect32(kScreenWidth, kScreenHeight), 0);
		}

		// Draw cursor on top of everything
		drawCursor();

		// Present to screen
		g_system->copyRectToScreen(_screen->getPixels(), _screen->pitch,
								   0, 0, kScreenWidth, kScreenHeight);
		g_system->updateScreen();

		// Limit presentation to approximately 30 frames per second.
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
	if (_screen)
		_screen->fillRect(Common::Rect32(kScreenWidth, kScreenHeight), 0);
}

/**
 * Destroy the current page and instantiate the requested page.
 */
void Zoombini2Engine::switchPage(int pageId) {
	debug(1, "Zoombini2: Switching from page %d to page %d", _currentPageId, pageId);
	if (_currentPageId == kPageBoolies && pageId == kPageMapTrans)
		recordBooliesCompletion();
	const bool usesMapMusic = pageId == kPageMenuLoad || pageId == kPageMenuPractice ||
							  pageId == kPageMenuOptions || pageId == kPageMapScreen ||
							  pageId == kPageMenuAlt;
	if (!usesMapMusic)
		stopMapMusic();

	destroyCurrentPage();
	_currentPageId = pageId;

	switch (pageId) {
	case kPageLogoTLC:
		_currentPage = new TransitionVideo(this,
										   selectMoviePath("movies/tlclogo.bik", "movies/tlclogo50%.bik"),
										   kPageLogoPolygon);
		break;
	case kPageLogoPolygon:
		_currentPage = new TransitionVideo(this, Common::Path("movies/logopoly.bik"), kPageTitleScreen);
		break;
	case kPageCutsceneFirst:
		_isSavedGame = true;
		_currentPage = new TransitionVideo(this,
										   selectMoviePath("movies/zoom_movie1_100%.bik", "movies/zoom_movie1_50%.bik"),
										   kPageZombiniville);
		break;
	case kPageCutsceneSecond:
		if (_gameState)
			_gameState->markRescue1MoviePlayed();
		_currentPage = new TransitionVideo(this,
										   selectMoviePath("movies/zoom_movie2_100%.bik", "movies/zoom_movie2_50%.bik"),
										   kPageRescue1);
		break;
	case kPageCutsceneThird:
		if (_gameState)
			_gameState->markRescue2MoviePlayed();
		_currentPage = new TransitionVideo(this,
										   selectMoviePath("movies/zoom_movie3_100%.bik", "movies/zoom_movie3_50%.bik"),
										   kPageRescue2);
		break;
	case kPageTitleScreen:
		_currentPage = new TransitionTitle(this);
		break;
	case kPageMenuPractice:
		_currentPage = new InteractiveMap(this, kMapScreenPractice);
		break;
	case kPageMenuLoad:
		_currentPage = new InteractiveMap(this, kMapScreenSavedGame);
		break;
	case kPageMapScreen:
		_currentPage = new InteractiveMap(this, kMapScreenSavedGame);
		break;
	case kPageMenuOptions:
	case kPageMenuAlt:
		// Open the save-file menu from the map's Parties button.
		_currentPage = new InteractiveMenu(this);
		break;
	case kPageZombiniville:
		_currentPage = new ShelterZombiniville(this);
		break;
	case kPageMapTrans:
		_currentPage = new TransitionMapTrans(this);
		break;
	case kPageMysticMarsh:
		_currentPage = new PuzzleMysticMarsh(this);
		break;
	case kPageChezNorf:
		_currentPage = new PuzzleChezNorf(this);
		break;
	case kPageWallOfFleens:
		_currentPage = new PuzzleWallOfFleens(this);
		break;
	case kPageBooliewood:
		_currentPage = new ShelterBooliewood(this);
		break;
	case kPageCrazyTurtle:
		_currentPage = new PuzzleCrazyTurtle(this);
		break;
	case kPageBoolies:
		_currentPage = new PuzzleBoolies(this);
		break;
	case kPageMagicWall:
		_currentPage = new PuzzleMagicWall(this);
		break;
	case kPageAquaCube:
		_currentPage = new PuzzleAquacube(this);
		break;
	case kPageSnowboard:
		_currentPage = new PuzzleSnowboard(this);
		break;
	case kPageWaterSlide:
		_currentPage = new PuzzleWaterslide(this);
		break;
	case kPageRescue1:
		_currentPage = new ShelterRescueSite1(this);
		break;
	case kPageRescue2:
		_currentPage = new ShelterRescueSite2(this);
		break;
	case kPageFinal:
		_currentPage = new ShelterFinal(this);
		break;
	case kPageCredits:
		_currentPage = new TransitionCredits(this);
		break;
	case kPageLogoArisuMedia:
		_currentPage = new TransitionVideo(this, Common::Path("movies/arisu.bik"), kPageLogoTLC);
		break;
	default:
		warning("Zoombini2: Unknown page %d", pageId);
		break;
	}

	if (_currentPage) {
		_currentPage->init();
		if (_gameState && kPageZombiniville <= pageId && pageId <= kPageBooliewood)
			_gameState->_currentGameplayPageId = pageId;
	}
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

Common::SeekableReadStream *Zoombini2Engine::openResourceFile(const Common::String &path) const {
	return _resourceFileResolver ? _resourceFileResolver->openFile(path) : nullptr;
}

bool Zoombini2Engine::hasResource(const Common::String &path) const {
	return _resourceFileResolver && _resourceFileResolver->hasFile(path);
}

} // End of namespace Zoombini2
