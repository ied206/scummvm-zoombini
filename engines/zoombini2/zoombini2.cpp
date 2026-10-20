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
#include "zoombini2/text_manager.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *Zoombini2Engine::kCursorPaths[Zoombini2Engine::kCursorCount];
constexpr const char *Zoombini2Engine::kQuitConfirmationPath;
constexpr const char *Zoombini2Engine::kDebugZoombiniSetFileNameFormat;

/** Resolve original logical resource names against their distinct physical roots. */
class Zoombini2Engine::ResourceFileResolver : public Common::NonCopyable {
public:
	/** Create disc and installed directory views retained until resolver destruction. */
	ResourceFileResolver(const Common::FSNode &cdDataDirectory, const Common::FSNode &installedDirectory)
		: _cdDataDirectory(cdDataDirectory.isDirectory() ? new Common::FSDirectory(cdDataDirectory, 8) : nullptr),
		  _installedDirectory(installedDirectory.isDirectory() ? new Common::FSDirectory(installedDirectory, 8) : nullptr) {
	}

	/** Delete the retained disc and installed directory views. */
	~ResourceFileResolver() {
		delete _cdDataDirectory;
		delete _installedDirectory;
	}

	/** Return whether the logical resource exists in its selected physical root. */
	bool hasFile(const Common::String &path) const {
		Common::Path relativePath;
		const Common::FSDirectory *directory = resolvePath(path, relativePath);
		return directory && directory->hasFile(relativePath);
	}

	/** Return a newly allocated stream for the logical resource, or nullptr when unavailable. */
	Common::SeekableReadStream *openFile(const Common::String &path) const {
		Common::Path relativePath;
		const Common::FSDirectory *directory = resolvePath(path, relativePath);
		return directory ? directory->createReadStreamForMember(relativePath) : nullptr;
	}

private:
	/** Select the disc or installed directory and write its normalized relative path. */
	const Common::FSDirectory *resolvePath(const Common::String &path, Common::Path &relativePath) const {
		const Common::String logicalPath = normalizeResourcePath(path).toString('/');
		const bool useInstalledDirectory = !logicalPath.empty() && logicalPath[0] == '#';
		const uint offset = useInstalledDirectory ? 1 : 0;
		if (logicalPath.size() <= offset)
			return nullptr;

		relativePath = Common::Path(logicalPath.substr(offset), '/');
		return useInstalledDirectory ? _installedDirectory : _cdDataDirectory;
	}

	/** Disc Data directory view released with this resolver, or nullptr when absent. */
	Common::FSDirectory *_cdDataDirectory;
	/** Installed HD directory view released with this resolver, or nullptr when absent. */
	Common::FSDirectory *_installedDirectory;
};

Common::Path Zoombini2Engine::normalizeResourcePath(const Common::String &path) {
	Common::String logicalPath(path);
	logicalPath.replace('\\', '/');
	const bool installed = !logicalPath.empty() && logicalPath[0] == '#';
	uint offset = 0;
	if (installed)
		offset = 1;
	else if (logicalPath.hasPrefixIgnoreCase("Data/"))
		offset = 5;
	if (offset < logicalPath.size() && logicalPath[offset] == '.')
		offset += 1;
	if (offset < logicalPath.size() && logicalPath[offset] == '/')
		offset += 1;
	logicalPath = logicalPath.substr(offset);
	if (installed && !logicalPath.empty())
		logicalPath = "#" + logicalPath;
	return Common::Path(logicalPath, '/');
}

Zoombini2Engine::PageResourceType Zoombini2Engine::inferPageResourceType(const Common::Path &path) {
	const Common::String name = path.toString('/');
	if (name.hasSuffixIgnoreCase(".bb") || name.hasSuffixIgnoreCase(".bmp"))
		return PageResourceType::kBitmap;
	if (name.hasSuffixIgnoreCase(".rb"))
		return PageResourceType::kSprite;
	if (name.hasSuffixIgnoreCase(".an"))
		return PageResourceType::kAnimation;
	if (name.hasSuffixIgnoreCase(".anm"))
		return PageResourceType::kZoombiniAnimation;
	if (name.hasSuffixIgnoreCase(".pat"))
		return PageResourceType::kPath;
	if (name.hasSuffixIgnoreCase(".wav"))
		return PageResourceType::kSound;
	if (name.hasSuffixIgnoreCase(".bik"))
		return PageResourceType::kVideo;
	if (name.hasSuffixIgnoreCase(".ztl"))
		return PageResourceType::kLayout;
	return PageResourceType::kUnknown;
}

void Zoombini2Engine::recordPageResource(const Common::String &path, ResourceAccess access, PageResourceType type) const {
	const Common::Path key = normalizeResourcePath(path);
	if (key.empty())
		return;
	PageResourceIndex::const_iterator found = _pageResourceIndex.find(key);
	uint index;
	if (found == _pageResourceIndex.end()) {
		index = _pageResources.size();
		PageResource resource;
		resource.path = key;
		resource.type = inferPageResourceType(key);
		_pageResources.push_back(resource);
		_pageResourceIndex[key] = index;
	} else {
		index = found->_value;
	}
	if (type != PageResourceType::kUnknown)
		_pageResources[index].type = type;
	PageResource &resource = _pageResources[index];
	if (access == ResourceAccess::kOpen) {
		resource.opened = true;
		resource.openCount++;
	} else if (access == ResourceAccess::kDecode) {
		resource.decodeCount++;
	} else {
		resource.cacheUsed = true;
		resource.cacheRequests++;
		if (access == ResourceAccess::kCacheHit)
			resource.cacheHits++;
	}
}

void Zoombini2Engine::recordPageResourceCacheUse(const Common::String &path, PageResourceType type, bool cacheHit) const {
	const ResourceAccess access = cacheHit ? ResourceAccess::kCacheHit : ResourceAccess::kCacheMiss;
	recordPageResource(path, access, type);
}

void Zoombini2Engine::recordPageResourceDecode(const Common::String &path, PageResourceType type) const {
	recordPageResource(path, ResourceAccess::kDecode, type);
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

	clearPagePATCache();
	_pageResources.clear();
	_pageResourceIndex.clear();
	delete _resourceFileResolver;
	_resourceFileResolver = new ResourceFileResolver(cdDataRoot, installedRoot);
}

Zoombini2Engine::Zoombini2Engine(OSystem *syst, const Zoombini2GameDescription *desc)
	: Engine(syst), _gameDescription(desc), _relInfo(*desc) {

	_rnd = new Random("zoombini2");
	_mainMenuDialog = new Zoombini2MenuDialog(this);

	refreshEngineSettings();
}

Zoombini2Engine::~Zoombini2Engine() {
	_nextPageId = PageId::kPageNone_M1;
	destroyActivePage();
	clearCursorImages(0);
	delete _puzzleRetryState;
	if (_state)
		writeGameSave(_state->getPlayerName());
	delete _state;

	delete _sidebar;
	delete _soundManager;
	delete _gfx;
	delete _text;
	delete _rnd;
	delete _resourceFileResolver;
}

PathObject *Zoombini2Engine::loadPagePATPath(const Common::Path &path) {
	const Common::Path key = normalizeResourcePath(path.toString('/'));
	const Common::String logicalPath = key.toString('/');
	// Reuse the page's cached template, returning a separate copy for playback.
	PATPathCache::const_iterator cached = _pagePATPaths.find(key);

	if (cached != _pagePATPaths.end()) {
		if (cached->_value)
			recordPageResourceCacheUse(logicalPath, PageResourceType::kPath);
		return cached->_value ? cached->_value->createUnstartedCopy() : nullptr;
	}

	// On a cache miss, read the PAT file and cache its template or the failed load.
	const PathObject *pathTemplate = PathObject::loadFromPAT(this, key);

	_pagePATPaths[key] = pathTemplate;
	if (pathTemplate) {
		recordPageResourceDecode(logicalPath, PageResourceType::kPath);
		recordPageResourceCacheUse(logicalPath, PageResourceType::kPath, false);
	}
	return pathTemplate ? pathTemplate->createUnstartedCopy() : nullptr;
}

void Zoombini2Engine::clearPagePATCache() {
	for (PATPathCache::iterator entry = _pagePATPaths.begin(); entry != _pagePATPaths.end(); entry++)
		delete entry->_value;
	_pagePATPaths.clear();
}

bool Zoombini2Engine::writeGameSave(const Common::String &savefileName) {
	if (inPracticeMode() || !_state || _state->getPlayerName().empty() || savefileName.empty() || _activeSavefileName.empty())
		return true;
	const Common::String &activeSavefileName = _activeSavefileName;
	if (isGameSaveWriteLocked(activeSavefileName) || _activeSavefileReadOnly)
		return true;
	return writeGameSavefile(activeSavefileName);
}

void Zoombini2Engine::resetGameState() {
	_activeSavefileName.clear();
	_activeSavefileReadOnly = false;
	setGameMode(GameMode::kPracticeMode);
	if (_state)
		_state->init();
}

bool Zoombini2Engine::isSavefileReadOnlyToggleEnabled() const {
	return ConfMan.getBool(Zoombini2MetaEngine::kConfigEnableSavefileReadOnlyToggle, ConfMan.getActiveDomainName());
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
	if (_practicePageId == PageId::kPageNone_M1 || _practicePuzzleLevel == 0)
		return false;

	pageId = _practicePageId;
	level = _practicePuzzleLevel;
	partySize = _practicePartySize;
	_practicePageId = PageId::kPageNone_M1;
	_practicePuzzleLevel = 0;
	_practicePartySize = 0;
	return true;
}

void Zoombini2Engine::queueDebugPracticeLaunch(PageId pageId, int level, uint partySize) {
	_practicePageId = pageId;
	_practicePuzzleLevel = level;
	_practicePartySize = partySize;
	_debugPracticeResetState = true;
	setGameMode(GameMode::kPracticeMode);
	requestPageChange(PageId::kPageMapPractice_M3);
}

Zoombini2Engine::DebugRouteRequestResult Zoombini2Engine::requestDebugMapTransition(PageId targetPageId, PageId sourcePageId, RouteBranch routeBranch, uint level) {
	if (!_state || !_activePage)
		return DebugRouteRequestResult::kNoActivePage;
	if (4 < level)
		return DebugRouteRequestResult::kInvalidLevel;

	const uint targetPartySize = InteractiveMap::getPracticePartySize(targetPageId);
	if (targetPartySize == 0 && (level != 0 || inPracticeMode()))
		return DebugRouteRequestResult::kPracticeRequiresPuzzle;

	const int rescuedBoolies = level == 0 ? _state->_rescuedBoolieCount : 0;
	if (TransitionMapTrans::getDestPage(sourcePageId, routeBranch, rescuedBoolies) != targetPageId)
		return DebugRouteRequestResult::kUnreachableDestination;
	if (level == 0 && inJourneyMode() && (!_activePage->debugCanDepart() || (_state->_activeZoombinis.empty() && _state->_savedRoster.empty())))
		return DebugRouteRequestResult::kNoTravelingParty;

	if (level != 0) {
		setGameMode(GameMode::kPracticeMode);
		_debugXferPracticeLevel = static_cast<int>(level);
		_debugXferPracticeTarget = targetPageId;
		_debugXferResetState = true;
	} else if (inPracticeMode()) {
		switch (_activePageId) {
		case PageId::kPageMapPractice_M3:
			if (_nextPageId == PageId::kPageNone_M1)
				_debugXferPracticeLevel = getPracticeLevel();
			else
				_debugXferPracticeLevel = _state->getLevel();
			break;
		case PageId::kPageMapTrans_22:
			if (_debugXferPracticeLevel == 0)
				_debugXferPracticeLevel = _state->getLevel();
			break;
		default:
			_debugXferPracticeLevel = _state->getLevel();
			break;
		}
		_debugXferPracticeTarget = targetPageId;
	}

	_mapTransitSrcPageId = sourcePageId;
	_routeDirection = routeBranch;
	_debugXferDest = targetPartySize != 0 ? targetPageId : sourcePageId;
	_debugMapTransition = true;
	requestPageChange(PageId::kPageMapTrans_22);
	return DebugRouteRequestResult::kQueued;
}

void Zoombini2Engine::setPracticeLevel(int level) {
	if ((1 <= level && level <= 3) || (level == 4 && _allowCutL4PracticePuzzles))
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
	if (getActivePageId() == PageId::kPageCredits_16)
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
			requestPageChange(PageId::kPageCredits_16);
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
	// A new debug view closes its open predecessor before opening behind it.
	// Unrelated modals still block the request while they own input and rendering.
	DialogBase *activeDialog = getActiveDialog();
	DialogDebug *activeDebugDialog = dynamic_cast<DialogDebug *>(activeDialog);
	if (activeDialog && !activeDebugDialog)
		return false;
	if (activeDebugDialog)
		activeDebugDialog->close();
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
	if (_rnd) {
		const int prngAlgorithmVal = ConfMan.getInt(Zoombini2MetaEngine::kConfigPrngAlgorithm);
		const PrngAlgorithmKind prngAlgorithm = static_cast<PrngAlgorithmKind>(prngAlgorithmVal);
		_rnd->setAlgorithm(prngAlgorithm);
	}
	if (_soundManager)
		_soundManager->setAudioOutputMode(_audioOutputMode);
}

Common::Error Zoombini2Engine::run() {
	// Init subsystems
	_text = new TextManager(this);
	if (!_text->loadTextFont())
		warning("Zoombini2Engine: Failed to load bitmap text strip");
	_gfx = new Gfx(this);
	_gfx->loadCommonZoombiniAnimations();
	initCursor();
	_soundManager = new SoundManager(this, _mixer);
	_soundManager->setAudioOutputMode(_audioOutputMode);
	syncSoundSettings();
	_state = new GameState(this);
	_sidebar = new Sidebar(this); // Shared Help, Map, Go buttons
	setDebugger(new Zoombini2Console(this));

	// Seed both snapshots from the same clock so the first frame delta stays small.
	_cachedGameTickCount = getTotalPlayTime();
	_prevFrameTickCount = _cachedGameTickCount;
	_frameTimeOriginMs = g_system->getMillis();
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
	switch (type) {
	case CursorType::kDefault:
	case CursorType::kInteractive:
		_baseCursorType = type;
		break;
	case CursorType::kRestoreBase:
		_pageCursorType = type;
		break;
	default:
		if (static_cast<uint>(type) < kCursorCount) {
			_pageCursorType = type;
		} else {
			warning("Zoombini2Engine: Invalid cursor type %u", static_cast<uint>(type));
			return;
		}
		break;
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

void Zoombini2Engine::setDialogPaused(bool paused) {
	if (_dialogPauseToken.isActive() == paused)
		return;
	if (paused)
		_dialogPauseToken = pauseEngine();
	else
		_dialogPauseToken.clear();
}

void Zoombini2Engine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);
	if (!_soundManager)
		return;
	if (pause)
		_soundManager->pauseAll();
	else
		_soundManager->resumeAll();
}

void Zoombini2Engine::restartGoBlink() {
	if (_sidebar)
		_sidebar->restartGoBlink();
}

void Zoombini2Engine::reseedRandomForV10() {
	if ((_gameDescription->features & GF_Z2_V10) != 0)
		_rnd->setSeed(getTotalPlayTime());
}

int Zoombini2Engine::getLogicPacingHz() const {
	switch (_logicPacingMode) {
	case LogicPacingMode::k75Hz:
		return 75;
	case LogicPacingMode::k60Hz:
	default:
		return 60;
	}
}

int Zoombini2Engine::mixerVolumeToPercent(int volume) {
	return CLIP((CLIP<int>(volume, 0, Audio::Mixer::kMaxMixerVolume) * kMaxVolumePercent + 128) / 256, 0, kMaxVolumePercent);
}

int Zoombini2Engine::percentToMixerVolume(int volume) {
	return MIN<int>(Audio::Mixer::kMaxMixerVolume, (CLIP(volume, 0, kMaxVolumePercent) * 256) / kMaxVolumePercent);
}

void Zoombini2Engine::refreshEngineSettings() {
	const int frameRate = CLIP<int>(ConfMan.getInt(Zoombini2MetaEngine::kConfigFrameRate), Zoombini2MetaEngine::kMinFrameRate, Zoombini2MetaEngine::kMaxFrameRate);
	const bool unlockFrameRate = ConfMan.getBool(Zoombini2MetaEngine::kConfigUnlockFrameRate);
	if (_frameRate != frameRate || _unlockFrameRate != unlockFrameRate) {
		_frameRate = frameRate;
		_unlockFrameRate = unlockFrameRate;
		_frameTimeOriginMs = g_system->getMillis();
		_lastFrameElapsedMs = 0;
		_hasFrameIndex = false;
	}
	_debugHotkeysEnabled = ConfMan.getBool(Zoombini2MetaEngine::kConfigDebugHotkeys);
	_chezNorfSubtitlesEnabled = ConfMan.getBool(Zoombini2MetaEngine::kConfigChezNorfClueSubtitles);
	const int audioOutputValue = ConfMan.getInt(Zoombini2MetaEngine::kConfigAudioOutputMode);
	_audioOutputMode = static_cast<AudioOutputMode>(audioOutputValue);
	const int colorAssistValue = ConfMan.getInt(Zoombini2MetaEngine::kConfigColorAssistMode);
	const ColorAssistMode colorAssistMode = static_cast<ColorAssistMode>(colorAssistValue);
	switch (colorAssistMode) {
	case ColorAssistMode::kEnhancedDistinction:
	case ColorAssistMode::kRedGreenBlindAssist:
		_colorAssistMode = colorAssistMode;
		break;
	case ColorAssistMode::kOriginal:
	default:
		_colorAssistMode = ColorAssistMode::kOriginal;
		break;
	}
	_useGreedyWaterslidePairing = ConfMan.getBool(Zoombini2MetaEngine::kConfigGreedyWaterslidePairing);
	const int bezierCurveDataTypeValue = ConfMan.getInt(Zoombini2MetaEngine::kConfigBezierCurveDataType);
	_bezierCurveDataType = static_cast<BezierCurveDataType>(bezierCurveDataTypeValue);
	_wallOfFleensFixDepartStreak = ConfMan.getBool(Zoombini2MetaEngine::kConfigWallOfFleensFixDepartStreak);
	_mysticMarshFillCellGaps = ConfMan.getBool(Zoombini2MetaEngine::kConfigMysticMarshFillCellGaps);
	_mysticMarshExpandAreaMask = ConfMan.getBool(Zoombini2MetaEngine::kConfigMysticMarshExpandAreaMask);
	_enhancedKbdShortcuts = ConfMan.getBool(Zoombini2MetaEngine::kConfigEnhancedKbdShortcuts);
	_allowCutL4PracticePuzzles = ConfMan.getBool(Zoombini2MetaEngine::kConfigRestoreL4PracticePuzzles);
	const int logicPacingValue = ConfMan.getInt(Zoombini2MetaEngine::kConfigLogicPacingMode);
	_logicPacingMode = static_cast<LogicPacingMode>(logicPacingValue);
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
	if (_activePageId == PageId::kPageZombiniville_00 || _activePageId == PageId::kPageRescue1_04 || _activePageId == PageId::kPageRescue2_09 ||
		_activePageId == PageId::kPageBooliewood_12 || _activePageId == PageId::kPageFinal_23)
		return false;

	for (uint i = 0; i < _state->_activeZoombinis.size(); i++) {
		if (_state->_activeZoombinis[i])
			_state->_activeZoombinis[i]->setCanAdvanceFromPage(true);
	}
	_zoombiniWalkingFlag = true;
	if (_activePage)
		_activePage->applyDebugPuzzleCompletion();
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
				if (_activePageId == PageId::kPageMenuOptions_M2)
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
				switch (event.kbd.keycode) {
				case Common::KEYCODE_p:
					_debugCompletionKeyDown = true;
					break;
				case Common::KEYCODE_c:
					_debugOverlayKeyDown = true;
					break;
				default:
					break;
				}
			}
			_pendingPageEvents.push_back(event);
			break;
		case Common::EVENT_KEYUP:
			switch (event.kbd.keycode) {
			case Common::KEYCODE_p:
				_debugCompletionKeyDown = false;
				break;
			case Common::KEYCODE_c:
				_debugOverlayKeyDown = false;
				break;
			default:
				break;
			}
			_pendingPageEvents.push_back(event);
			break;
		default:
			break;
		}
	}
}

void Zoombini2Engine::applyPendingPageChange() {
	if (_nextPageId == PageId::kPageNone_M1)
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
		if (_nextPageId != PageId::kPageNone_M1)
			break;
		DialogBase *dialog = getActiveDialog();
		const bool dialogEventWasActive = dialog != nullptr;
		const bool pageDialogWasActive = _activePage->hasActiveDialog();
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
			_activePage->handleEvent(event);
		if (event.type == Common::EVENT_LBUTTONUP)
			_modalOwnedPress = false;

		DialogBase *activeDialog = getActiveDialog();
		modalInputBlocked = modalInputBlocked || activeDialog != nullptr;
		if ((dialogEventWasActive && dialog != activeDialog) || (pageDialogWasActive && !_activePage->hasActiveDialog()))
			break;
	}
	_mousePos = polledMousePos;
	cleanupClosedDialogs();

	return dialogWasActive;
}

void Zoombini2Engine::drawFrame() {
	ManagedSurface32 *screen = _gfx->getScreen();
	if (!_activePage) {
		screen->fillRect(Common::Rect32(Gfx::kScreenSize.width, Gfx::kScreenSize.height), 0);
		return;
	}

	if (_debugHotkeysEnabled && _debugCompletionKeyDown)
		runBuiltinDebugAction(BuiltinDebugAction::kCompletePuzzle);
	const bool dialogWasActive = dispatchPageEvents();
	DialogBase *dialog = getActiveDialog();
	const bool pageRendered = !dialogWasActive && !dialog;
	const bool advanceState = _nextPageId == PageId::kPageNone_M1;
	if (pageRendered)
		_activePage->onFrame(screen, advanceState);

	// The shared sidebar polls the final frame mouse state before drawing its controls.
	DialogBase *dialogBeforeSidebar = dialog;
	if (_sidebar && (!dialog || dialog->drawsSidebarOnTop()))
		_sidebar->drawAndHandleInput(screen, _nextPageId == PageId::kPageNone_M1);
	dialog = getActiveDialog();
	if (dialog && (dialog == dialogBeforeSidebar || dialog->drawsSidebarOnTop())) {
		dialog->render(screen);
		if (dialog->drawsSidebarOnTop() && _sidebar)
			_sidebar->drawOverDialog(screen);
	}
	updateHoldOverlay(pageRendered && advanceState);
}

const ZoombiniRunner *Zoombini2Engine::getHeldGlobalZoombini() const {
	for (uint i = 0; i < _state->_activeZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _state->_activeZoombinis[i];
		if (zoombini && zoombini->isHeld())
			return zoombini;
	}
	return nullptr;
}

void Zoombini2Engine::updateHoldOverlay(bool advanceState) {
	const ZoombiniRunner *held = getHeldGlobalZoombini();
	const bool holding = held != nullptr;
	if (_cursorVisible == holding) {
		_cursorVisible = !holding;
		CursorMan.showMouse(_cursorVisible);
	}
	if (!holding || !_gfx)
		return;
	if (getActiveDialog())
		return;
	ManagedSurface32 *screen = _gfx->getScreen();
	_activePage->renderHoldOverlay(screen, advanceState);
	_gfx->drawZoombiniNameBox(screen, held->getName());
}

void Zoombini2Engine::presentFrame() {
	const uint32 now = g_system->getMillis();
	if (!_unlockFrameRate && now == _lastPresentTimeMs)
		return;
	_lastPresentTimeMs = now;
	ManagedSurface32 *screen = _gfx->getScreen();
	g_system->copyRectToScreen(screen->getPixels(), screen->pitch, 0, 0, Gfx::kScreenSize.width, Gfx::kScreenSize.height);
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
	_cachedGameTickCount = getTotalPlayTime();
	if (_soundManager)
		_soundManager->updateSpeechQueue();
	applyPendingPageChange();
	drawFrame();
	presentFrame();
}

void Zoombini2Engine::mainGameLoop() {
	_nextPageId = _relInfo.getStartupPageId();
	if (!isDemo() && configurePracticeBootParamLaunch())
		_nextPageId = PageId::kPageMapPractice_M3;

	while (!shouldQuit())
		runFrame();
}

bool Zoombini2Engine::configurePracticeBootParamLaunch() {
	const int bootParam = ConfMan.getInt("boot_param");
	if (bootParam == 0)
		return false;

	const PageId pageId = static_cast<PageId>(bootParam / kPracticeBootParamPageFactor);
	const int level = bootParam % kPracticeBootParamPageFactor;
	if (InteractiveMap::getPracticePartySize(pageId) == 0 || level < 1 || 4 < level) {
		warning("Zoombini2: unsupported practice boot parameter %d", bootParam);
		return false;
	}

	_practicePageId = pageId;
	_practicePuzzleLevel = level;
	_practiceLevel = level;
	debug(1, "Zoombini2: practice boot parameter=%d page=%d level=%d", bootParam, static_cast<int>(pageId), level);
	return true;
}

void Zoombini2Engine::destroyActivePage() {
	for (uint i = 0; i < _dialogStack.size(); i++) {
		_dialogStack[i]->close();
		delete _dialogStack[i];
	}
	_dialogStack.clear();
	if (_activePage) {
		delete _activePage;
		_activePage = nullptr;
	}
	// Reset the shared page-layer collection so the replacement page starts empty.
	if (_gfx) {
		setCursor(CursorType::kDefault);
		setCursor(CursorType::kRestoreBase);
		clearCursorImages(static_cast<uint>(CursorType::kMainDishSandwich));
		_gfx->clearPageLayers();
		_gfx->clearPageBitmapCache();
		// Clear the screen so a replacement page using double buffering cannot present stale content.
		_gfx->getScreen()->fillRect(Common::Rect32(Gfx::kScreenSize.width, Gfx::kScreenSize.height), 0);
	}
	clearPagePATCache();
	_pageResources.clear();
	_pageResourceIndex.clear();
}

bool Zoombini2Engine::debugRetryPuzzle() {
	if (!_activePage || _activePage->getCategory() != PageCategory::kPuzzle || !_puzzleRetryState || _nextPageId != PageId::kPageNone_M1)
		return false;
	_debugPuzzleRetryPending = true;
	requestPageChange(_activePageId);
	return true;
}

/**
 * Destroy the active page and instantiate the requested page.
 */
void Zoombini2Engine::switchPage(PageId pageId) {
	debug(1, "Zoombini2: Switching from page %d to page %d", static_cast<int>(_activePageId), static_cast<int>(pageId));
	const bool retryPuzzle = _debugPuzzleRetryPending && pageId == _activePageId;
	_debugPuzzleRetryPending = false;
	destroyActivePage();
	if (retryPuzzle) {
		delete _state;
		_state = _puzzleRetryState;
		_puzzleRetryState = nullptr;
		_rnd->setSeed(_puzzleRetrySeed);
	}
	delete _puzzleRetryState;
	_puzzleRetryState = nullptr;
	_zoombiniWalkingFlag = false;
	if (pageId != PageId::kPageMapTrans_22)
		_debugMapTransition = false;
	if (pageId == PageId::kPageMapPractice_M3 && _debugPracticeResetState) {
		resetGameState();
		_debugPracticeResetState = false;
	}
	if (pageId == PageId::kPageMapTrans_22 && _debugXferDest != PageId::kPageNone_M1) {
		if (_debugXferResetState) {
			resetGameState();
			_state->_level = _debugXferPracticeLevel;
			_debugXferResetState = false;
		}
		const uint routePartySize = InteractiveMap::getPracticePartySize(_debugXferDest);
		if (inJourneyMode())
			_state->prepareDebugRouteParty(_debugXferDest, routePartySize);
		else {
			while (routePartySize < _state->_activeZoombinis.size()) {
				delete _state->_activeZoombinis.back();
				_state->_activeZoombinis.pop_back();
			}
		}
		if (inPracticeMode() && _state->_activeZoombinis.empty())
			InteractiveMap::createPracticeParty(this, _debugXferDest);
		_debugXferDest = PageId::kPageNone_M1;
		_state->_currentPage = _mapTransitSrcPageId;
	}
	if (pageId == _debugXferPracticeTarget && _debugXferPracticeLevel != 0) {
		_state->_level = _debugXferPracticeLevel;
		_debugXferPracticeLevel = 0;
		_debugXferPracticeTarget = PageId::kPageNone_M1;
	}
	_nextPageId = PageId::kPageNone_M1;
	_activePageId = pageId;

	switch (pageId) {
	case PageId::kPageMapJourney_M4:
		_activePage = new InteractiveMap(this, GameMode::kJourneyMode);
		break;
	case PageId::kPageMapPractice_M3:
		_activePage = new InteractiveMap(this, GameMode::kPracticeMode);
		break;
	case PageId::kPageMenuOptions_M2:
		_activePage = new InteractiveMenu(this);
		break;
	case PageId::kPageZombiniville_00:
		_activePage = new ShelterZombiniville(this);
		break;
	case PageId::kPageCrazyTurtle_01:
		_activePage = new PuzzleCrazyTurtle(this);
		break;
	case PageId::kPageWaterslide_02:
		_activePage = new PuzzleWaterslide(this);
		break;
	case PageId::kPageAquacube_03:
		_activePage = new PuzzleAquacube(this);
		break;
	case PageId::kPageRescue1_04:
		_activePage = new ShelterRescueSite1(this);
		break;
	case PageId::kPageMysticMarsh_05:
		_activePage = new PuzzleMysticMarsh(this);
		break;
	case PageId::kPageMagicWall_06:
		_activePage = new PuzzleMagicWall(this);
		break;
	case PageId::kPageWallOfFleens_07:
		_activePage = new PuzzleWallOfFleens(this);
		break;
	case PageId::kPageChezNorf_08:
		_activePage = new PuzzleChezNorf(this);
		break;
	case PageId::kPageRescue2_09:
		_activePage = new ShelterRescueSite2(this);
		break;
	case PageId::kPageSnowboard_10:
		_activePage = new PuzzleSnowboard(this);
		break;
	case PageId::kPageBoolies_11:
		_activePage = new PuzzleBoolies(this);
		break;
	case PageId::kPageBooliewood_12:
		_activePage = new ShelterBooliewood(this);
		break;
	case PageId::kPageCredits_16:
		_activePage = new TransitionCredits(this);
		break;
	case PageId::kPageLogoTLC_17:
		_activePage = new TransitionVideo(this, PageId::kPageLogoTLC_17);
		break;
	case PageId::kPageCutsceneFirst_18:
		setGameMode(GameMode::kJourneyMode);
		_activePage = new TransitionVideo(this, PageId::kPageCutsceneFirst_18);
		break;
	case PageId::kPageCutsceneSecond_20:
		_activePage = new TransitionVideo(this, PageId::kPageCutsceneSecond_20);
		break;
	case PageId::kPageCutsceneThird_21:
		_activePage = new TransitionVideo(this, PageId::kPageCutsceneThird_21);
		break;
	case PageId::kPageMapTrans_22:
		_activePage = new TransitionMapTrans(this);
		break;
	case PageId::kPageFinal_23:
		_activePage = new ShelterFinal(this);
		break;
	case PageId::kPageTitleScreen_24:
		_activePage = new TransitionTitle(this);
		break;
	case PageId::kPageLogoPolygon_25:
		_activePage = new TransitionVideo(this, PageId::kPageLogoPolygon_25);
		break;
	case PageId::kPageLogoArisuMedia_1972:
		_activePage = new TransitionVideo(this, PageId::kPageLogoArisuMedia_1972);
		break;
	default:
		warning("Zoombini2: Unknown page %d", static_cast<int>(pageId));
		break;
	}

	if (_activePage) {
		if (_state && _activePage->getCategory() == PageCategory::kPuzzle) {
			_puzzleRetryState = _state->cloneForPageEntry();
			_puzzleRetrySeed = _rnd->getSeed();
		}
		// Non-puzzle pages use level one for help sheets and journey-map access.
		if (_state && _activePage->getCategory() != PageCategory::kPuzzle) {
			_state->_level = 1;
			if (inJourneyMode())
				_state->setPageLevel(pageId, 1);
		}
		_activePage->init();
		if (_state && PageId::kPageZombiniville_00 <= pageId && pageId <= PageId::kPageBooliewood_12)
			_state->_currentPage = pageId;
	}
}

Common::SeekableReadStream *Zoombini2Engine::openResourceFile(const Common::String &path) const {
	Common::SeekableReadStream *stream = _resourceFileResolver ? _resourceFileResolver->openFile(path) : nullptr;
	if (stream)
		recordPageResource(path, ResourceAccess::kOpen);
	return stream;
}

bool Zoombini2Engine::hasResource(const Common::String &path) const {
	return _resourceFileResolver && _resourceFileResolver->hasFile(path);
}

} // End of namespace Zoombini2
