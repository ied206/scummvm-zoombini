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

#include "common/callback.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/fs.h"
#include "common/savefile.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "graphics/cursorman.h"
#include "graphics/pixelformat.h"

#include "zoombini2/console.h"
#include "zoombini2/dialogs.h"
#include "zoombini2/graphics.h"
#include "zoombini2/metaengine.h"
#include "zoombini2/pages/dialog_debug.h"
#include "zoombini2/pages/dialog_help.h"
#include "zoombini2/pages/dialog_msgbox.h"
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
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *Zoombini2Engine::kCursorPaths[Zoombini2Engine::kCursorCount];
constexpr const char *Zoombini2Engine::kQuitConfirmationPath;
constexpr const char *Zoombini2Engine::kDebugZoombiniSetFileNameFormat;

/** Resolve original logical resource names against their distinct physical roots. */
class Zoombini2Engine::ResourceFileResolver : public Common::NonCopyable {
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

	_rnd = new Random("zoombini2");
	_mainMenuDialog = new Zoombini2MenuDialog(this);

	refreshEngineSettings();
}

Zoombini2Engine::~Zoombini2Engine() {
	_nextPageId = kPageNone;
	destroyCurrentPage();
	clearCursorImages(0);
	if (_state)
		writeGameSave(_state->getPlayerName());
	delete _state;
	clearZoombiniAnimationCache();

	delete _sidebar;
	delete _soundManager;
	delete _gfx;
	delete _rnd;
	delete _resourceFileResolver;
}

const ZoombiniAnimation *Zoombini2Engine::loadZoombiniAnimation(const Common::Path &path, uint32 frameDelay) {
	ZoombiniAnimationCache::const_iterator cached = _zoombiniAnimationCache.find(path);
	if (cached != _zoombiniAnimationCache.end()) {
		cached->_value->setFrameDelay(frameDelay);
		return cached->_value;
	}

	ZoombiniAnimation *animation = new ZoombiniAnimation(this);
	if (!animation->loadFromFile(path)) {
		delete animation;
		return nullptr;
	}
	animation->setFrameDelay(frameDelay);
	_zoombiniAnimationCache[path] = animation;
	return animation;
}

void Zoombini2Engine::clearZoombiniAnimationCache() {
	for (ZoombiniAnimationCache::iterator entry = _zoombiniAnimationCache.begin(); entry != _zoombiniAnimationCache.end(); entry++)
		delete entry->_value;
	_zoombiniAnimationCache.clear();
}

bool Zoombini2Engine::writeGameSave(const Common::String &savefileName) {
	const Common::String &activeSavefileName = _activeSavefileName.empty() ? savefileName : _activeSavefileName;
	if (isGameSaveWriteLocked(activeSavefileName) || _activeSavefileReadOnly)
		return true;
	return writeGameSavefile(activeSavefileName);
}

bool Zoombini2Engine::isSavefileReadOnlyToggleEnabled() const {
	return ConfMan.getBool(::Zoombini2MetaEngine::kConfigEnableSavefileReadOnlyToggle, ConfMan.getActiveDomainName());
}

bool Zoombini2Engine::isGameSaveWriteLocked(const Common::String &savefileName) const {
	Common::HashMap<Common::String, bool>::const_iterator entry = _saveWriteLockOverrides.find(savefileName);
	return entry != _saveWriteLockOverrides.end() && entry->_value;
}

bool Zoombini2Engine::toggleGameSaveWriteLock(const Common::String &savefileName) {
	if (!isSavefileReadOnlyToggleEnabled() || savefileName.empty())
		return false;
	if (isGameSaveReadOnly(savefileName)) {
		g_system->displayMessageOnOSD(Common::U32String("This savefile is read-only on disk"));
		return false;
	}
	const bool locked = !isGameSaveWriteLocked(savefileName);
	_saveWriteLockOverrides[savefileName] = locked;
	debug(1, "Zoombini2: automatic writes %s for savefile '%s'", locked ? "locked" : "unlocked", savefileName.c_str());
	g_system->displayMessageOnOSD(Common::U32String(locked ? "Savefile locked" : "Savefile unlocked"));
	return true;
}

bool Zoombini2Engine::createGameSave(const Common::String &savefileName) {
	Zoombini2SavegameManager savegameManager(_saveFileMan, ConfMan.getActiveDomainName(), getLanguage());
	if (savegameManager.savefileExists(savefileName))
		return false;
	const bool saved = writeGameSavefile(savefileName);
	if (saved) {
		_saveWriteLockOverrides.erase(savefileName);
		_activeSavefileName = savefileName;
		_activeSavefileReadOnly = false;
	}
	return saved;
}

bool Zoombini2Engine::overwriteGameSave(const Common::String &savefileName) {
	const bool saved = writeGameSavefile(savefileName);
	if (saved) {
		_activeSavefileName = savefileName;
		_activeSavefileReadOnly = false;
	}
	return saved;
}

bool Zoombini2Engine::writeGameSavefile(const Common::String &savefileName) {
	if (!_state)
		return false;
	Zoombini2SavegameManager savegameManager(_saveFileMan, ConfMan.getActiveDomainName(), getLanguage());
	return savegameManager.writeSavefile(savefileName, *_state);
}

bool Zoombini2Engine::readGameSave(const Common::String &savefileName) {
	if (!_state)
		return false;
	Zoombini2SavegameManager savegameManager(_saveFileMan, ConfMan.getActiveDomainName(), getLanguage());
	const bool loaded = savegameManager.loadSavefile(savefileName, *_state);
	if (loaded) {
		_activeSavefileName = savefileName;
		_activeSavefileReadOnly = isGameSaveReadOnly(savefileName);
	}
	return loaded;
}

bool Zoombini2Engine::deleteGameSave(const Common::String &savefileName) {
	Zoombini2SavegameManager savegameManager(_saveFileMan, ConfMan.getActiveDomainName(), getLanguage());
	const bool deleted = savegameManager.deleteSavefile(savefileName);
	if (deleted && _activeSavefileName == savefileName) {
		_activeSavefileName.clear();
		_activeSavefileReadOnly = false;
	}
	if (deleted)
		_saveWriteLockOverrides.erase(savefileName);
	return deleted;
}

Common::StringArray Zoombini2Engine::listGameSaves() const {
	Zoombini2SavegameManager savegameManager(_saveFileMan, ConfMan.getActiveDomainName(), getLanguage());
	return savegameManager.listSavefiles();
}

bool Zoombini2Engine::isGameSaveReadOnly(const Common::String &savefileName) const {
	Zoombini2SavegameManager savegameManager(_saveFileMan, ConfMan.getActiveDomainName(), getLanguage());
	return savegameManager.isSavefileReadOnly(savefileName);
}

bool Zoombini2Engine::takePracticePuzzleLaunch(PageId &pageId, int &level, uint &partySize) {
	if (_practicePageId == kPageNone || _practicePuzzleLevel == 0)
		return false;

	pageId = _practicePageId;
	level = _practicePuzzleLevel;
	partySize = _practicePartySize;
	_practicePageId = kPageNone;
	_practicePuzzleLevel = 0;
	_practicePartySize = 0;
	return true;
}

void Zoombini2Engine::queueDebugPracticeLaunch(PageId pageId, int level, uint partySize) {
	_practicePageId = pageId;
	_practicePuzzleLevel = level;
	_practicePartySize = partySize;
	_debugPracticeResetState = true;
	_isSavedGame = false;
	requestPageChange(kPageMenuPractice);
}

bool Zoombini2Engine::supportsInternalPracticeLevel4(PageId pageId) {
	switch (pageId) {
	case kPageCrazyTurtle:
	case kPageWaterslide:
	case kPageAquacube:
	case kPageMysticMarsh:
	case kPageMagicWall:
	case kPageWallOfFleens:
	case kPageBoolies:
		return true;
	case kPageChezNorf:
	case kPageSnowboard:
	default:
		return false;
	}
}

void Zoombini2Engine::setPracticeLevel(int level) {
	if ((1 <= level && level <= 3) || (level == 4 && _allowCutLevel4PracticePuzzles))
		_practiceLevel = level;
}

int Zoombini2Engine::getMusicVolume() const {
	return _soundManager ? _soundManager->getMusicVolume() : mixerVolumeToPercent(ConfMan.getInt("music_volume"));
}

int Zoombini2Engine::getSFXVolume() const {
	return _soundManager ? _soundManager->getSfxVolume() : mixerVolumeToPercent(ConfMan.getInt("sfx_volume"));
}

int Zoombini2Engine::getSpeechVolume() const {
	return _soundManager ? _soundManager->getSpeechVolume() : mixerVolumeToPercent(ConfMan.getInt("speech_volume"));
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

/**
 * Exit the demo immediately on an Alt+F4 or window-close request.
 * Retail releases suppress confirmation while another modal or credits is active.
 * Otherwise they drain pending input and open the shared quit confirmation.
 * The backend delivers Alt+F4 through EVENT_QUIT.
 */
void Zoombini2Engine::handleQuitRequest() {
	if (isDemo()) {
		quitGame();
		return;
	}
	if (getCurrentPageId() == kPageCredits)
		return;

	g_system->getEventManager()->resetQuit();
	_pendingPageEvents.clear();

	if (getActiveDialog())
		return;

	requestQuitConfirmation();
}

void Zoombini2Engine::requestQuitConfirmation() {
	const Common::Point32 position = isDemo() ? Common::Point32(212, 270) : Common::Point32(-1, -1);
	requestMsgBox(Common::Path(kQuitConfirmationPath),
				  new Common::Callback<Zoombini2Engine, DialogMsgBoxButton>(this, &Zoombini2Engine::handleQuitConfirmation), position);
}

void Zoombini2Engine::handleQuitConfirmation(DialogMsgBoxButton button) {
	if (button == DialogMsgBoxButton::kOkay01) {
		if (isDemo())
			quitGame();
		else
			requestPageChange(kPageCredits);
	}
}

DialogBase *Zoombini2Engine::getActiveDialog() const {
	for (uint i = _dialogStack.size(); 0 < i; i--) {
		if (_dialogStack[i - 1]->isActive())
			return _dialogStack[i - 1];
	}
	return nullptr;
}

void Zoombini2Engine::cleanupClosedDialogs() {
	for (uint i = 0; i < _dialogStack.size();) {
		if (_dialogStack[i]->isActive()) {
			i += 1;
			continue;
		}
		delete _dialogStack[i];
		_dialogStack.remove_at(i);
	}
}

bool Zoombini2Engine::requestMsgBox(const Common::Path &textPath, Common::BaseCallback<DialogMsgBoxButton> *callback, const Common::Point32 &pos) {
	if (getActiveDialog()) {
		delete callback;
		return false;
	}
	DialogMsgBox *dialog = new DialogMsgBox(this);
	if (!dialog->request(textPath, callback, pos)) {
		delete dialog;
		return false;
	}
	_dialogStack.push_back(dialog);
	return true;
}

bool Zoombini2Engine::requestUiTextMsgBox(const Common::U32String &message, Common::BaseCallback<DialogMsgBoxButton> *callback,
										  const Common::Point32 &pos) {
	if (getActiveDialog()) {
		delete callback;
		return false;
	}
	DialogMsgBox *dialog = new DialogMsgBox(this);
	if (!dialog->requestUiText(message, callback, pos)) {
		delete dialog;
		return false;
	}
	_dialogStack.push_back(dialog);
	return true;
}

bool Zoombini2Engine::openHelpDialog(PageId pageId, int level) {
	if (getActiveDialog())
		return false;
	DialogHelp *dialog = new DialogHelp(this);
	if (!dialog->open(pageId, level)) {
		delete dialog;
		return false;
	}
	_dialogStack.push_back(dialog);
	return true;
}

bool Zoombini2Engine::openDebugDialog(const DialogDebugCommand &cmd) {
	if (getActiveDialog())
		return false;
	DialogDebug *dialog = new DialogDebug(this);
	if (!dialog->open(cmd)) {
		delete dialog;
		return false;
	}
	_dialogStack.push_back(dialog);
	return true;
}

bool Zoombini2Engine::hasFeature(EngineFeature f) const {
	return f == kSupportsReturnToLauncher || f == kSupportsChangingOptionsDuringRuntime || f == kSupportsQuitDialogOverride;
}

void Zoombini2Engine::syncSoundSettings() {
	Engine::syncSoundSettings();
	if (_soundManager) {
		_soundManager->setVolumeSettings(mixerVolumeToPercent(ConfMan.getInt("music_volume")),
										 mixerVolumeToPercent(ConfMan.getInt("sfx_volume")),
										 mixerVolumeToPercent(ConfMan.getInt("speech_volume")));
	}
}

void Zoombini2Engine::applyGameSettings() {
	refreshEngineSettings();
	if (_soundManager)
		_soundManager->setStereoOutputEnabled(_stereoOutputEnabled);
}

Common::Error Zoombini2Engine::run() {
	// Init subsystems
	_gfx = new Gfx(this);
	initCursor();
	_soundManager = new SoundManager(this, _mixer);
	_soundManager->setStereoOutputEnabled(_stereoOutputEnabled);
	syncSoundSettings();
	_state = new GameState();
	_sidebar = new Sidebar(this); // Shared Help, Map, Go buttons
	setDebugger(new Zoombini2Console(this));

	_startTime = g_system->getMillis();
	_cachedGameTickCount = 0;
	_prevFrameTickCount = 0;
	_frameTimeOriginMs = _startTime;
	_lastFrameElapsedMs = 0;
	_hasFrameIndex = false;

	// Run the main game loop
	mainGameLoop();

	return Common::kNoError;
}

Zoombini2Engine::CursorImage::CursorImage(const Size32 &cursorSize)
	: size(cursorSize), pixels(new byte[static_cast<size_t>(cursorSize.width) * cursorSize.height * 4]()) {
}

Zoombini2Engine::CursorImage::~CursorImage() {
	delete[] pixels;
}

/**
 * Load and register the default cursor. The hover cursor is loaded on first use.
 *
 * CursorMan keeps the cursor visible over both the game viewport and any surrounding border.
 */
void Zoombini2Engine::initCursor() {
	// The cursor click point is three pixels right and ten pixels below its draw origin.
	_cursorHotspot = Common::Point(3, 10);
	_cursorVisible = true;

	setCursor(CursorType::kDefault);
}

void Zoombini2Engine::setCursor(CursorType type) {
	if (type == CursorType::kDefault || type == CursorType::kInteractive) {
		_baseCursorType = type;
	} else if (type == CursorType::kRestoreBase) {
		_pageCursorType = type;
	} else if (static_cast<uint>(type) < kCursorCount) {
		_pageCursorType = type;
	} else {
		warning("Zoombini2Engine: Invalid cursor type %u", static_cast<uint>(type));
		return;
	}

	const CursorType activeType = _pageCursorType == CursorType::kRestoreBase ? _baseCursorType : _pageCursorType;
	if (_cursorRegistered && activeType == _activeCursorType)
		return;

	const uint index = static_cast<uint>(activeType);
	if (!_cursorImages[index] && !_cursorUnavailable[index]) {
		RleBlock *sprite = nullptr;
		if (index < static_cast<uint>(CursorType::kMainDishSandwich))
			sprite = _gfx->loadSharedRleBlock(kCursorPaths[index]);
		else
			sprite = _gfx->loadPageRleBlock(kCursorPaths[index]);
		if (!sprite || !sprite->isValid() || sprite->getSize().width <= 0 || sprite->getSize().height <= 0) {
			warning("Zoombini2Engine: Failed to load cursor %s", kCursorPaths[index]);
			_cursorUnavailable[index] = true;
		} else {
			_cursorImages[index] = createCursorImage(sprite);
		}
	}
	if (!_cursorImages[index]) {
		if (_pageCursorType != CursorType::kRestoreBase) {
			_pageCursorType = CursorType::kRestoreBase;
			setCursor(CursorType::kRestoreBase);
		} else if (_baseCursorType == CursorType::kInteractive) {
			_baseCursorType = CursorType::kDefault;
			setCursor(CursorType::kDefault);
		}
		return;
	}

	const CursorImage *image = _cursorImages[index];
	static constexpr Graphics::PixelFormat cursorFormat = Graphics::PixelFormat::createFormatBGRA32();
	CursorMan.replaceCursor(image->pixels, image->size.width, image->size.height, _cursorHotspot.x, _cursorHotspot.y, 0, &cursorFormat);
	CursorMan.showMouse(true);
	_activeCursorType = activeType;
	_cursorRegistered = true;
}

Zoombini2Engine::CursorImage *Zoombini2Engine::createCursorImage(const RleBlock *sprite) const {
	const Size32 size = sprite->getSize();
	CursorImage *image = new CursorImage(size);
	byte *buf = image->pixels;

	// Render the RLE cursor through its public drawing path,
	// then extract the alpha channel by comparing the result over black and white backgrounds.

	// Render onto black background
	ManagedSurface32 blackSurf(size, Graphics::PixelFormat::createFormatBGRA32());
	blackSurf.fillRect(Common::Rect(size.width, size.height), blackSurf.format.ARGBToColor(255, 0, 0, 0));
	sprite->drawToScreen(&blackSurf, Common::Point32(0, 0), _alphaBlendLUT);

	// Render onto white background
	ManagedSurface32 whiteSurf(size, Graphics::PixelFormat::createFormatBGRA32());
	whiteSurf.fillRect(Common::Rect(size.width, size.height), whiteSurf.format.ARGBToColor(255, 255, 255, 255));
	sprite->drawToScreen(&whiteSurf, Common::Point32(0, 0), _alphaBlendLUT);

	// Derive alpha from the two renders:
	// For premultiplied alpha compositing: result = src_premult + invAlpha * dst / 255
	// On black (dst=0): result_black = src_premult
	// On white (dst=255): result_white = src_premult + invAlpha
	// So: invAlpha = result_white - result_black
	//     alpha = 255 - invAlpha
	//     color = src_premult * 255 / alpha (un-premultiply)
	const byte *blackPixels = static_cast<const byte *>(blackSurf.getPixels());
	const byte *whitePixels = static_cast<const byte *>(whiteSurf.getPixels());

	for (int i = 0; i < size.width * size.height; i++) {
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
			buf[i * 4 + 3] = static_cast<byte>(alpha);       // A
		}
	}

	return image;
}

void Zoombini2Engine::clearCursorImages(uint firstIndex) {
	for (uint i = firstIndex; i < kCursorCount; i++) {
		delete _cursorImages[i];
		_cursorImages[i] = nullptr;
		_cursorUnavailable[i] = false;
	}
}

uint32 Zoombini2Engine::getGameTickCount() const {
	return calculateGameTickCount();
}

void Zoombini2Engine::setPauseState(bool &reason, bool paused) {
	if (reason == paused)
		return;
	const bool wasPaused = _dialogPaused || _backendPaused;
	reason = paused;
	const bool isPaused = _dialogPaused || _backendPaused;
	const uint32 now = g_system->getMillis();
	if (!wasPaused && isPaused) {
		_pauseTimeStart = now;
		if (_soundManager)
			_soundManager->pauseAll();
		_mixer->pauseAll(true);
	} else if (wasPaused && !isPaused) {
		_pauseTimeAccum += now - _pauseTimeStart;
		if (_soundManager)
			_soundManager->resumeAll();
		_mixer->pauseAll(false);
	}
}

void Zoombini2Engine::setDialogPaused(bool paused) {
	setPauseState(_dialogPaused, paused);
}

void Zoombini2Engine::pauseEngineIntern(bool pause) {
	setPauseState(_backendPaused, pause);
}

void Zoombini2Engine::restartGoBlink() {
	if (_sidebar)
		_sidebar->restartGoBlink();
}

void Zoombini2Engine::reseedRandomForV10() {
	if ((_gameDescription->features & GF_Z2_V10) != 0)
		_rnd->setSeed(getGameTickCount());
}

uint32 Zoombini2Engine::calculateGameTickCount() const {
	const uint32 now = g_system->getMillis();
	uint32 elapsed = now - _startTime;
	if (_dialogPaused || _backendPaused)
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
	const int frameRate = CLIP<int>(ConfMan.getInt(::Zoombini2MetaEngine::kConfigFrameRate),
									::Zoombini2MetaEngine::kMinFrameRate, ::Zoombini2MetaEngine::kMaxFrameRate);
	const bool unlockFrameRate = ConfMan.getBool(::Zoombini2MetaEngine::kConfigUnlockFrameRate);
	if (_frameRate != frameRate || _unlockFrameRate != unlockFrameRate) {
		_frameRate = frameRate;
		_unlockFrameRate = unlockFrameRate;
		_frameTimeOriginMs = g_system->getMillis();
		_lastFrameElapsedMs = 0;
		_hasFrameIndex = false;
	}
	_debugHotkeysEnabled = ConfMan.getBool(::Zoombini2MetaEngine::kConfigDebugHotkeys);
	_stereoOutputEnabled = ConfMan.getBool(::Zoombini2MetaEngine::kConfigStereoOutput);
	const int colorAssistValue = ConfMan.getInt(::Zoombini2MetaEngine::kConfigColorAssistMode);
	_colorAssistMode = 0 <= colorAssistValue && colorAssistValue <= 2 ? static_cast<ColorAssistMode>(colorAssistValue) : ColorAssistMode::kOriginal00;
	_useGreedyWaterslidePairing = ConfMan.getBool(::Zoombini2MetaEngine::kConfigGreedyWaterslidePairing);
	_useAquacubeSafeFirstMove = ConfMan.getBool(::Zoombini2MetaEngine::kConfigAquacubeSafeFirstMove);
	_useFloatingPointPaths = ConfMan.getBool(::Zoombini2MetaEngine::kConfigUseFloatingPointPaths);
	_fixFleenDepartStreak = ConfMan.getBool(::Zoombini2MetaEngine::kConfigFixFleenDepartureStreak);
	_enhancedKbdShortcuts = ConfMan.getBool(::Zoombini2MetaEngine::kConfigEnhancedKbdShortcuts);
	_allowCutLevel4PracticePuzzles = ConfMan.getBool(::Zoombini2MetaEngine::kConfigAllowCutLevel4PracticePuzzles);
	_logicPacingHz = ConfMan.getInt(::Zoombini2MetaEngine::kConfigLogicPacingHz) == 75 ? 75 : 60;
	if (!_debugHotkeysEnabled) {
		_debugCompletionKeyDown = false;
		_debugOverlayKeyDown = false;
	}
}

Common::String Zoombini2Engine::getDebugZoombiniSetFileName() const {
	const char *target = _targetName.empty() ? "zoombini2" : _targetName.c_str();
	return Common::String::format(kDebugZoombiniSetFileNameFormat, target);
}

void Zoombini2Engine::exportZoombiniSet() const {
	if (_state->_activeZoombinis.empty())
		return;

	const Common::String fileName = getDebugZoombiniSetFileName();
	Common::OutSaveFile *output = g_system->getSavefileManager()->openForSaving(fileName, false);
	if (!output) {
		warning("Cannot open Zoombini trait file for saving: %s", fileName.c_str());
		return;
	}

	output->writeString(Common::String::format("%u\n", _state->_activeZoombinis.size()));
	for (uint i = 0; i < _state->_activeZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _state->_activeZoombinis[i];
		if (!zoombini)
			continue;
		output->writeString(Common::String::format("%u %u %u %u\n", zoombini->getTraits()._feet, zoombini->getTraits()._nose, zoombini->getTraits()._hair,
												   zoombini->getTraits()._eyes));
	}
	output->finalize();
	if (output->err())
		warning("Cannot save Zoombini trait file: %s", fileName.c_str());
	delete output;
}

bool Zoombini2Engine::readDebugSetInteger(const char *&cursor, const char *end, int32 &value) {
	while (cursor < end && Common::isSpace(static_cast<byte>(*cursor)))
		cursor += 1;
	if (cursor == end)
		return false;

	const bool negative = *cursor == '-';
	if (*cursor == '+' || *cursor == '-')
		cursor += 1;

	const int64 limit = negative ? 2147483648LL : 2147483647LL;
	int64 magnitude = 0;
	bool hasDigit = false;
	while (cursor < end && Common::isDigit(static_cast<byte>(*cursor))) {
		hasDigit = true;
		const int digit = *cursor - '0';
		if ((limit - digit) / 10 < magnitude)
			return false;
		magnitude = magnitude * 10 + digit;
		cursor += 1;
	}
	if (!hasDigit || (cursor < end && !Common::isSpace(static_cast<byte>(*cursor))))
		return false;
	value = static_cast<int32>(negative ? -magnitude : magnitude);
	return true;
}

void Zoombini2Engine::importZoombiniSet() {
	const Common::String fileName = getDebugZoombiniSetFileName();
	// The interchange document is plain text, so imports must not invoke automatic decompression.
	Common::InSaveFile *input = g_system->getSavefileManager()->openRawFile(fileName);
	if (!input)
		return;

	const int64 fileSize = input->size();
	if (fileSize <= 0 || kDebugZoombiniSetMaxSize < fileSize) {
		delete input;
		warning("Rejected Zoombini trait file '%s': expected 1-%u bytes", fileName.c_str(), kDebugZoombiniSetMaxSize);
		return;
	}
	char data[kDebugZoombiniSetMaxSize];
	const uint32 dataSize = static_cast<uint32>(fileSize);
	const uint32 bytesRead = input->read(data, dataSize);
	const bool readOk = bytesRead == dataSize && !input->err() && input->size() == fileSize;
	delete input;
	if (!readOk) {
		warning("Rejected Zoombini trait file '%s': incomplete or failed read", fileName.c_str());
		return;
	}

	const char *cursor = data;
	const char *end = data + dataSize;
	int32 fileCount = 0;
	if (!readDebugSetInteger(cursor, end, fileCount) || fileCount < 1 || kDebugZoombiniSetMaxMembers < fileCount) {
		warning("Rejected Zoombini trait file '%s': expected 1-%d members", fileName.c_str(), kDebugZoombiniSetMaxMembers);
		return;
	}
	ZmbTrait traits[kDebugZoombiniSetMaxMembers];
	for (int i = 0; i < fileCount; i++) {
		int32 values[ZmbTrait::kTraitKindCount];
		for (int traitIndex = 0; traitIndex < ZmbTrait::kTraitKindCount; traitIndex++) {
			if (!readDebugSetInteger(cursor, end, values[traitIndex]) || values[traitIndex] < 1 || ZmbTrait::kTraitValueCount < values[traitIndex]) {
				warning("Rejected Zoombini trait file '%s': invalid trait %d of member %d", fileName.c_str(), traitIndex + 1, i + 1);
				return;
			}
		}
		traits[i] = ZmbTrait(static_cast<byte>(values[0]), static_cast<byte>(values[1]), static_cast<byte>(values[2]), static_cast<byte>(values[3]));
	}
	while (cursor < end && Common::isSpace(static_cast<byte>(*cursor)))
		cursor += 1;
	if (cursor != end) {
		warning("Rejected Zoombini trait file '%s': unexpected trailing data", fileName.c_str());
		return;
	}

	const uint importCount = MIN<uint>(static_cast<uint>(fileCount), _state->_activeZoombinis.size());
	for (uint i = 0; i < importCount; i++) {
		if (!_state->_activeZoombinis[i]) {
			warning("Rejected Zoombini trait file '%s': active member %u is unavailable", fileName.c_str(), i + 1);
			return;
		}
	}
	for (uint i = 0; i < importCount; i++)
		_state->_activeZoombinis[i]->setTraits(traits[i]);
}

bool Zoombini2Engine::runBuiltinDebugAction(BuiltinDebugAction action) {
	switch (action) {
	case BuiltinDebugAction::kExportZoombiniSet:
		exportZoombiniSet();
		return true;
	case BuiltinDebugAction::kImportZoombiniSet:
		importZoombiniSet();
		return true;
	case BuiltinDebugAction::kCompletePuzzle:
		return applyDebugPuzzleCompletion();
	}
	return false;
}

bool Zoombini2Engine::applyDebugPuzzleCompletion() {
	if (_currentPageId == kPageZombiniville || _currentPageId == kPageRescue1 || _currentPageId == kPageRescue2 || _currentPageId == kPageBooliewood ||
		_currentPageId == kPageFinal)
		return false;

	for (uint i = 0; i < _state->_activeZoombinis.size(); i++) {
		if (_state->_activeZoombinis[i])
			_state->_activeZoombinis[i]->setCanAdvanceFromPage(true);
	}
	_zoombiniWalkingFlag = true;
	if (_currentPage)
		_currentPage->applyDebugPuzzleCompletion();
	return true;
}

void Zoombini2Engine::processEvents() {
	Common::Event event;
	_pendingPageEvents.clear();
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
			handleQuitRequest();
			break;
		case Common::EVENT_RETURN_TO_LAUNCHER:
			return;
		case Common::EVENT_FOCUS_LOST:
			_debugCompletionKeyDown = false;
			_debugOverlayKeyDown = false;
			break;
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			_pendingPageEvents.push_back(event);
			_mousePos = event.mouse;
			break;
		case Common::EVENT_LBUTTONUP:
			_pendingPageEvents.push_back(event);
			_mousePos = event.mouse;
			break;
		case Common::EVENT_MOUSEMOVE:
			if (!_pendingPageEvents.empty() && _pendingPageEvents.back().type == Common::EVENT_MOUSEMOVE)
				_pendingPageEvents.back() = event;
			else
				_pendingPageEvents.push_back(event);
			_mousePos = event.mouse;
			break;
		case Common::EVENT_KEYDOWN:
			// The global Ctrl+F5 mapping opens the menu before its event reaches this engine.
			if (event.kbd.keycode == Common::KEYCODE_F5) {
				openMainMenuDialog();
				break;
			}
			if (event.kbd.keycode == Common::KEYCODE_k && event.kbd.hasFlags(Common::KBD_CTRL) && isSavefileReadOnlyToggleEnabled()) {
				if (_currentPageId == kPageMenuOptions || _currentPageId == kPageMenuAlt)
					_pendingPageEvents.push_back(event);
				else if (!event.kbdRepeat && !_activeSavefileName.empty())
					toggleGameSaveWriteLock(_activeSavefileName);
				break;
			}
			if (event.kbd.keycode == Common::KEYCODE_F2 || event.kbd.keycode == Common::KEYCODE_F3) {
				if (_debugHotkeysEnabled) {
					if (event.kbd.keycode == Common::KEYCODE_F2)
						runBuiltinDebugAction(BuiltinDebugAction::kExportZoombiniSet);
					else
						runBuiltinDebugAction(BuiltinDebugAction::kImportZoombiniSet);
				}
				break;
			}
			if (_debugHotkeysEnabled) {
				if (event.kbd.keycode == Common::KEYCODE_p)
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

void Zoombini2Engine::applyPendingPageChange() {
	if (_nextPageId == kPageNone)
		return;

	const PageId requestedPage = _nextPageId;
	switchPage(requestedPage);

	// Input collected for the previous page must not reach the replacement page.
	_pendingPageEvents.clear();
}

bool Zoombini2Engine::dispatchPageEvents() {
	const bool dialogWasActive = getActiveDialog() != nullptr;
	bool modalInputBlocked = dialogWasActive;

	// Event dispatch temporarily replays each event's mouse position. The sidebar then polls the final position for this frame.
	const Common::Point32 polledMousePos = _mousePos;
	for (const Common::Event &event : _pendingPageEvents) {
		if (_nextPageId != kPageNone)
			break;
		DialogBase *dialog = getActiveDialog();
		const bool dialogEventWasActive = dialog != nullptr;
		const bool pageDialogWasActive = _currentPage->hasActiveDialog();
		modalInputBlocked = modalInputBlocked || dialogEventWasActive;
		if (event.type == Common::EVENT_LBUTTONDOWN)
			_modalOwnedPress = dialogEventWasActive;

		if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP || event.type == Common::EVENT_MOUSEMOVE)
			_mousePos = event.mouse;

		EventHandleResult result = EventHandleResult::kPassthrough;
		if (dialogEventWasActive)
			result = dialog->handleEvent(event);
		else if (_sidebar)
			result = _sidebar->handleEvent(event);
		// A press that began inside a shared modal owns its release. The press may
		// have dismissed the modal, but the release must still not reach the page.
		const bool releaseAfterModalPress = event.type == Common::EVENT_LBUTTONUP && _modalOwnedPress;
		if (result == EventHandleResult::kPassthrough && !modalInputBlocked && !releaseAfterModalPress)
			_currentPage->handleEvent(event);
		if (event.type == Common::EVENT_LBUTTONUP)
			_modalOwnedPress = false;

		DialogBase *activeDialog = getActiveDialog();
		modalInputBlocked = modalInputBlocked || activeDialog != nullptr;
		if ((dialogEventWasActive && dialog != activeDialog) || (pageDialogWasActive && !_currentPage->hasActiveDialog()))
			break;
	}
	_mousePos = polledMousePos;
	cleanupClosedDialogs();

	return dialogWasActive;
}

void Zoombini2Engine::drawFrame() {
	ManagedSurface32 *screen = _gfx->getScreen();
	if (!_currentPage) {
		screen->fillRect(Common::Rect32(ManagedSurface32::kScreenSize.width, ManagedSurface32::kScreenSize.height), 0);
		return;
	}

	if (_debugHotkeysEnabled && _debugCompletionKeyDown)
		runBuiltinDebugAction(BuiltinDebugAction::kCompletePuzzle);
	const bool dialogWasActive = dispatchPageEvents();
	DialogBase *dialog = getActiveDialog();
	const bool pageRendered = !dialogWasActive && !dialog;
	const bool advanceState = _nextPageId == kPageNone;
	if (pageRendered)
		_currentPage->onFrame(screen, advanceState);

	// The shared sidebar polls the final frame mouse state before drawing its controls.
	DialogBase *dialogBeforeSidebar = dialog;
	if (_sidebar && (!dialog || dialog->drawsSidebarOnTop()))
		_sidebar->drawAndHandleInput(screen, _nextPageId == kPageNone);
	dialog = getActiveDialog();
	if (dialog && (dialog == dialogBeforeSidebar || dialog->drawsSidebarOnTop())) {
		dialog->render(screen);
		if (dialog->drawsSidebarOnTop() && _sidebar)
			_sidebar->drawOverDialog(screen);
	}
	updateDragOverlay(pageRendered && advanceState);
}

const ZoombiniRunner *Zoombini2Engine::getDraggedGlobalZoombini() const {
	for (uint i = 0; i < _state->_activeZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _state->_activeZoombinis[i];
		if (zoombini && zoombini->isDragging())
			return zoombini;
	}
	return nullptr;
}

void Zoombini2Engine::updateDragOverlay(bool advanceState) {
	const ZoombiniRunner *dragged = getDraggedGlobalZoombini();
	const bool dragging = dragged != nullptr;
	if (_cursorVisible == dragging) {
		_cursorVisible = !dragging;
		CursorMan.showMouse(_cursorVisible);
	}
	if (!dragging || !_gfx)
		return;
	if (getActiveDialog())
		return;
	ManagedSurface32 *screen = _gfx->getScreen();
	_currentPage->renderDragOverlay(screen, advanceState);
	_gfx->drawDragNameTooltip(screen, Common::String(dragged->getName()));
}

void Zoombini2Engine::presentFrame() {
	const uint32 now = g_system->getMillis();
	if (!_unlockFrameRate && now == _lastPresentTimeMs)
		return;
	_lastPresentTimeMs = now;
	ManagedSurface32 *screen = _gfx->getScreen();
	g_system->copyRectToScreen(screen->getPixels(), screen->pitch, 0, 0, ManagedSurface32::kScreenSize.width, ManagedSurface32::kScreenSize.height);
	g_system->updateScreen();
}

void Zoombini2Engine::waitForFrameSlot() {
	if (_unlockFrameRate)
		return;

	uint32 elapsed = g_system->getMillis() - _frameTimeOriginMs;
	if (elapsed < _lastFrameElapsedMs) {
		// Reset the frame number after the 32-bit millisecond clock completes a full cycle.
		_frameTimeOriginMs = g_system->getMillis();
		elapsed = 0;
		_hasFrameIndex = false;
	}

	uint64 frameIndex = (static_cast<uint64>(elapsed) * static_cast<uint32>(_frameRate)) / 1000;
	while (_hasFrameIndex && frameIndex <= _lastFrameIndex) {
		const uint64 nextFrameTime = ((_lastFrameIndex + 1) * 1000 + static_cast<uint32>(_frameRate) - 1) / static_cast<uint32>(_frameRate);
		const uint32 waitMs = elapsed < nextFrameTime ? static_cast<uint32>(nextFrameTime - elapsed) : 1;
		g_system->delayMillis(waitMs);
		elapsed = g_system->getMillis() - _frameTimeOriginMs;
		frameIndex = (static_cast<uint64>(elapsed) * static_cast<uint32>(_frameRate)) / 1000;
	}

	_lastFrameElapsedMs = elapsed;
	_lastFrameIndex = frameIndex;
	_hasFrameIndex = true;
}

void Zoombini2Engine::runFrame() {
	waitForFrameSlot();
	processEvents();
	if (shouldQuit())
		return;
	// Keep one gameplay-time snapshot for every active pass.
	_prevFrameTickCount = _cachedGameTickCount;
	_cachedGameTickCount = calculateGameTickCount();
	if (_soundManager)
		_soundManager->updateSpeechQueue();
	applyPendingPageChange();
	drawFrame();
	presentFrame();
}

void Zoombini2Engine::mainGameLoop() {
	if (isDemo()) {
		_nextPageId = kPageTitleScreen;
	} else if (configurePracticeBootParamLaunch()) {
		_nextPageId = kPageMenuPractice;
	} else {
		// The Korean release starts with the ArisuMedia logo before the TLC and Polygon logos.
		// Other releases start with the TLC logo.
		_nextPageId = getLanguage() == Common::KO_KOR ? kPageLogoArisuMedia : kPageLogoTLC;
	}

	while (!shouldQuit())
		runFrame();
}

bool Zoombini2Engine::configurePracticeBootParamLaunch() {
	const int bootParam = ConfMan.getInt("boot_param");
	if (bootParam == 0)
		return false;

	const PageId pageId = static_cast<PageId>(bootParam / kPracticeBootParamPageFactor);
	const int level = bootParam % kPracticeBootParamPageFactor;
	if (InteractiveMap::getPracticePartySize(pageId) == 0 || level < 1 || 4 < level || (level == 4 && !supportsInternalPracticeLevel4(pageId))) {
		warning("Zoombini2: unsupported practice boot parameter %d", bootParam);
		return false;
	}

	_practicePageId = pageId;
	_practicePuzzleLevel = level;
	_practiceLevel = level;
	debug(1, "Zoombini2: practice boot parameter=%d page=%d level=%d", bootParam, static_cast<int>(pageId), level);
	return true;
}

void Zoombini2Engine::destroyCurrentPage() {
	for (uint i = 0; i < _dialogStack.size(); i++) {
		_dialogStack[i]->close();
		delete _dialogStack[i];
	}
	_dialogStack.clear();
	if (_currentPage) {
		delete _currentPage;
		_currentPage = nullptr;
	}
	// Reset the shared page-layer collection so the replacement page starts empty.
	if (_gfx) {
		setCursor(CursorType::kDefault);
		setCursor(CursorType::kRestoreBase);
		clearCursorImages(static_cast<uint>(CursorType::kMainDishSandwich));
		_gfx->clearPageLayers();
		_gfx->clearPageBitmapCache();
		// Clear the screen so a replacement page using double buffering cannot present stale content.
		_gfx->getScreen()->fillRect(Common::Rect32(ManagedSurface32::kScreenSize.width, ManagedSurface32::kScreenSize.height), 0);
	}
}

/**
 * Destroy the current page and instantiate the requested page.
 */
void Zoombini2Engine::switchPage(PageId pageId) {
	debug(1, "Zoombini2: Switching from page %d to page %d", static_cast<int>(_currentPageId), static_cast<int>(pageId));
	if (_currentPageId == kPageBoolies && pageId == kPageMapTrans)
		_state->recordBooliesCompletion();
	destroyCurrentPage();
	_zoombiniWalkingFlag = false;
	if (pageId == kPageMenuPractice && _debugPracticeResetState) {
		_state->init();
		_activeSavefileName.clear();
		_activeSavefileReadOnly = false;
		_debugPracticeResetState = false;
	}
	if (pageId == kPageMapTrans && _debugXferDestination != kPageNone) {
		if (_debugXferResetState) {
			_state->init();
			_activeSavefileName.clear();
			_activeSavefileReadOnly = false;
			_state->_level = _debugXferPracticeLevel;
			_debugXferResetState = false;
		}
		const uint routePartySize = InteractiveMap::getPracticePartySize(_debugXferDestination);
		while (routePartySize < _state->_activeZoombinis.size()) {
			delete _state->_activeZoombinis.back();
			_state->_activeZoombinis.pop_back();
		}
		if (_state && _state->_activeZoombinis.empty())
			InteractiveMap::createPracticeParty(this, _debugXferDestination);
		_debugXferDestination = kPageNone;
	}
	if (pageId == _debugXferPracticeTarget && _debugXferPracticeLevel != 0) {
		_state->_level = _debugXferPracticeLevel;
		_debugXferPracticeLevel = 0;
		_debugXferPracticeTarget = kPageNone;
	}
	_nextPageId = kPageNone;
	_currentPageId = pageId;

	switch (pageId) {
	case kPageLogoTLC:
		_currentPage = new TransitionVideo(this, kPageLogoTLC);
		break;
	case kPageLogoPolygon:
		_currentPage = new TransitionVideo(this, kPageLogoPolygon);
		break;
	case kPageCutsceneFirst:
		_isSavedGame = true;
		_currentPage = new TransitionVideo(this, kPageCutsceneFirst);
		break;
	case kPageCutsceneSecond:
		if (_state)
			_state->markRescue1MoviePlayed();
		_currentPage = new TransitionVideo(this, kPageCutsceneSecond);
		break;
	case kPageCutsceneThird:
		if (_state)
			_state->markRescue2MoviePlayed();
		_currentPage = new TransitionVideo(this, kPageCutsceneThird);
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
	case kPageAquacube:
		_currentPage = new PuzzleAquacube(this);
		break;
	case kPageSnowboard:
		_currentPage = new PuzzleSnowboard(this);
		break;
	case kPageWaterslide:
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
		_currentPage = new TransitionVideo(this, kPageLogoArisuMedia);
		break;
	default:
		warning("Zoombini2: Unknown page %d", static_cast<int>(pageId));
		break;
	}

	if (_currentPage) {
		// The original publishes one effective difficulty per page switch. Puzzle
		// destinations carry their stored world difficulty, while shelter, movie,
		// and transition destinations pass 0, which is coerced to level 1. Mirror
		// that coercion so non-puzzle pages (e.g. Booliewood) always resolve the
		// easy help sheet instead of a missing medium/hard one.
		if (_state && _currentPage->getCategory() != PageCategory::kPuzzle)
			_state->_level = 1;
		_currentPage->init();
		if (_state && kPageZombiniville <= pageId && pageId <= kPageBooliewood)
			_state->_currentGameplayPageId = pageId;
	}
}

Common::SeekableReadStream *Zoombini2Engine::openResourceFile(const Common::String &path) const {
	return _resourceFileResolver ? _resourceFileResolver->openFile(path) : nullptr;
}

bool Zoombini2Engine::hasResource(const Common::String &path) const {
	return _resourceFileResolver && _resourceFileResolver->hasFile(path);
}

} // End of namespace Zoombini2
