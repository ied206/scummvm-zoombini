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

#include <string.h>

#include "common/debug.h"
#include "common/fs.h"

#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/pages/save_file_list.h"
#include "zoombini2/pages/menuscreen.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

const char *const MenuScreenPage::kValidNameCharacters =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";

MenuScreenPage::MenuScreenPage(Zoombini2Engine *engine)
	: Page(engine), _state(kSaveMenuMain), _background(nullptr),
	  _selectorNormal(nullptr), _selectorHilite(nullptr), _selectionBar(nullptr),
	  _fileList(nullptr), _blipSoundId(-1), _typeSoundId(-1),
	  _deleteSoundId(-1), _mapMusicId(-1), _volumePanel(nullptr),
	  _confirmType(kSaveMenuConfirmNone), _confirmPanelNothing(nullptr),
	  _confirmPanelOk(nullptr), _confirmPanelCancel(nullptr),
	  _confirmTextDelete(nullptr), _confirmTextQuit(nullptr),
	  _confirmButtonHover(0), _confirmX(0), _confirmY(0) {
	_pageId = kPageMenuOptions;
	for (int i = 0; i < kMenuButtonCount; ++i)
		_buttons[i] = nullptr;
}

MenuScreenPage::~MenuScreenPage() {
	delete _fileList;
	delete _background;
	delete _selectorNormal;
	delete _selectorHilite;
	delete _selectionBar;
	for (int i = 0; i < kMenuButtonCount; ++i)
		delete _buttons[i];

	delete _volumePanel;
	delete _confirmPanelNothing;
	delete _confirmPanelOk;
	delete _confirmPanelCancel;
	delete _confirmTextDelete;
	delete _confirmTextQuit;

	SoundManager *sound = _engine->getSoundManager();
	if (sound) {
		if (0 <= _blipSoundId)
			sound->unload(_blipSoundId);
		if (0 <= _typeSoundId)
			sound->unload(_typeSoundId);
		if (0 <= _deleteSoundId)
			sound->unload(_deleteSoundId);
	}
}

void MenuScreenPage::init() {
	loadResources();
	loadButtons();

	_fileList = new SaveFileList(kFileListX, kFileListY, _selectionBar);
	if (!_fileList->init())
		warning("MenuScreenPage: Failed to load save-list fonts");
	scanSaveFiles();

	_mapMusicId = _engine->ensureMapMusic();
	_state = kSaveMenuMain;
}

void MenuScreenPage::loadResources() {
	_background = new BitBlock();
	if (!_background->load(Common::Path("bmp/menu/background")))
		warning("MenuScreenPage: Failed to load menu background");

	_selectorNormal = new BitBlock();
	if (!_selectorNormal->load(Common::Path("bmp/menu/PARTIEs - selector NORMAL")))
		warning("MenuScreenPage: Failed to load normal selector");

	_selectorHilite = new BitBlock();
	if (!_selectorHilite->load(Common::Path("bmp/menu/PARTIEs - selector HILITE")))
		warning("MenuScreenPage: Failed to load highlighted selector");

	_selectionBar = new RleBlock();
	if (!_selectionBar->loadFromFile(Common::Path("bmp/menu/barre-cache.rb")))
		warning("MenuScreenPage: Failed to load save selection bar");

	SoundManager *sound = _engine->getSoundManager();
	if (sound) {
		_blipSoundId = sound->load(false, Common::Path("sounds/blip.wav"), false);
		_typeSoundId = sound->load(false, Common::Path("sounds/fx/i-bs5.wav"), false);
		_deleteSoundId = sound->load(false, Common::Path("sounds/fx/del.wav"), false);
	}
}

void MenuScreenPage::loadButtons() {
	struct ButtonDefinition {
		const char *normalPath;
		const char *hoverPath;
		const char *disabledPath;
		int x;
		int y;
		int width;
		int height;
	};

	const ButtonDefinition definitions[kMenuButtonCount] = {
		{"bmp/menu/PARTIEs - ArrowUP NORMAL", "bmp/menu/PARTIEs - ArrowUP HIGHLIGHT", nullptr, 613, 350, 46, 50},
		{"bmp/menu/PARTIEs - ArrowDOWN NORMAL", "bmp/menu/PARTIEs - ArrowDOWN HILITE", nullptr, 613, 416, 46, 50},
		{"bmp/menu/Start Normal", "bmp/menu/Start Highlight", "bmp/menu/Start Gray", 27, 561, 145, 39},
		{"bmp/menu/PANEL - Options  NORMAL", "bmp/menu/PANEL - Options  HIGHLIGHT", nullptr, 175, 561, 145, 39},
		{"bmp/menu/New Normal", "bmp/menu/New Highlight", "bmp/menu/New Gray", 321, 561, 145, 39},
		{"bmp/menu/PANEL - Entrainement NORMAL", "bmp/menu/PANEL - Entraine HILITE", nullptr, 468, 561, 145, 39},
		{"bmp/menu/PANEL - Quitter NORMAL", "bmp/menu/PANEL - Quitter HIGHLIGHT", nullptr, 613, 561, 145, 39}
	};

	for (int i = 0; i < kMenuButtonCount; ++i) {
		const ButtonDefinition &definition = definitions[i];
		_buttons[i] = new UIButton();
		_buttons[i]->setRect(definition.x, definition.y, definition.width, definition.height);
		_buttons[i]->loadImages(Common::Path(definition.normalPath),
		                        Common::Path(definition.hoverPath),
		                        definition.disabledPath ? Common::Path(definition.disabledPath) : Common::Path());
	}
}

void MenuScreenPage::scanSaveFiles() {
	_fileList->clear();

	Common::FSNode saveDirectory = _engine->getSaveDir();
	if (!saveDirectory.exists() || !saveDirectory.isDirectory())
		return;

	Common::FSList files;
	saveDirectory.getChildren(files, Common::FSNode::kListFilesOnly);
	for (uint i = 0; i < files.size() && _fileList->getItemCount() < SaveFileList::kMaximumItems; ++i) {
		const Common::String fileName = files[i].getName();
		if (fileName.size() < 4 || !fileName.substr(fileName.size() - 3).equalsIgnoreCase(".mk"))
			continue;

		const Common::String stem = fileName.substr(0, fileName.size() - 3);
		if (SaveFileList::kMaximumNameLength < static_cast<int>(stem.size())) {
			warning("MenuScreenPage: Ignoring overlong save name '%s'", stem.c_str());
			continue;
		}
		_fileList->addItemSorted(stem);
	}
}

void MenuScreenPage::update() {
	const uint32 keyCode = _engine->getLastKeyPressed();
	if (_state == kSaveMenuMain && keyCode != 0) {
		handleKeyInput(keyCode);
	} else if (_state == kSaveMenuOptions && keyCode == Common::KEYCODE_ESCAPE) {
		closeOptionsDialog(false);
	} else if (_state == kSaveMenuConfirm) {
		if (keyCode == Common::KEYCODE_ESCAPE)
			closeConfirmDialog();
		_confirmButtonHover = hitTestConfirmDialog(_engine->getMousePos().x, _engine->getMousePos().y);
	}
}

void MenuScreenPage::draw(Graphics::ManagedSurface *screen) {
	if (_state == kSaveMenuMain) {
		drawMain(screen);
	} else if (_state == kSaveMenuOptions) {
		if (_background)
			_background->drawToSurface(screen, 0, 0);
		if (_volumePanel) {
			const VolumePanelResult result = _volumePanel->drawAndHandleInput(screen,
			        _engine->getMousePos().x, _engine->getMousePos().y,
			        _engine->isMouseDown(), _engine->isMouseClicked(),
			        _engine->getAlphaLUT());
			if (result == kVolumePanelChanged)
				applyOptionVolumes(true);
			else if (result == kVolumePanelApply)
				closeOptionsDialog(true);
			else if (result == kVolumePanelCancel)
				closeOptionsDialog(false);
		}
	} else {
		drawMain(screen);
		drawConfirmDialog(screen);
	}
}

void MenuScreenPage::drawMain(Graphics::ManagedSurface *screen) {
	if (_background)
		_background->drawToSurface(screen, 0, 0);

	const Common::Point mouse = _engine->getMousePos();
	BitBlock *selector = isInSelectorArea(mouse) ? _selectorHilite : _selectorNormal;
	if (selector)
		selector->drawToSurface(screen, kFileListX, kFileListY);
	if (_fileList)
		_fileList->draw(screen, _engine->getAlphaLUT());
	drawButtons(screen, mouse.x, mouse.y);
}

void MenuScreenPage::drawButtons(Graphics::ManagedSurface *screen, int mouseX, int mouseY) {
	_buttons[kMenuButtonNext]->setEnabled(_fileList->canMoveUp());
	_buttons[kMenuButtonPrev]->setEnabled(_fileList->canMoveDown());
	_buttons[kMenuButtonStart]->setEnabled(_fileList->hasValidSelection());
	_buttons[kMenuButtonNew]->setEnabled(_fileList->canBeginNewEntry());

	for (int i = 0; i < kMenuButtonCount; ++i) {
		if (_buttons[i] && _buttons[i]->drawAndHitTest(screen, mouseX, mouseY,
		        _engine->getAlphaLUT()) == 1)
			playSound(_blipSoundId);
	}
}

void MenuScreenPage::handleClick(const Common::Point &pos) {
	if (_state == kSaveMenuOptions)
		return;
	if (_state == kSaveMenuConfirm) {
		const int buttonId = hitTestConfirmDialog(pos.x, pos.y);
		if (0 < buttonId)
			handleConfirmClick(buttonId);
		return;
	}

	const int buttonId = hitTestButton(pos);
	if (0 <= buttonId) {
		handleButtonClick(buttonId);
		return;
	}
	if (_fileList->handleClick(pos))
		playSound(_blipSoundId);
}

int MenuScreenPage::hitTestButton(const Common::Point &pos) const {
	for (int i = 0; i < kMenuButtonCount; ++i) {
		if (_buttons[i] && _buttons[i]->isEnabled() && _buttons[i]->containsPoint(pos))
			return i;
	}
	return -1;
}

bool MenuScreenPage::isInSelectorArea(const Common::Point &pos) const {
	return kFileListX < pos.x && pos.x < kFileListX + kSelectorWidth &&
	       kFileListY < pos.y && pos.y < kFileListY + kSelectorHeight;
}

void MenuScreenPage::handleButtonClick(int buttonId) {
	switch (buttonId) {
	case kMenuButtonNext:
		_fileList->moveSelectionUp();
		break;
	case kMenuButtonPrev:
		_fileList->moveSelectionDown();
		break;
	case kMenuButtonStart:
		startSelectedSave();
		break;
	case kMenuButtonOptions:
		openOptionsDialog();
		break;
	case kMenuButtonNew:
		_fileList->beginOrConfirmNewEntry();
		break;
	case kMenuButtonTraining:
		break;
	case kMenuButtonQuit:
		openConfirmDialog(kSaveMenuConfirmQuit);
		break;
	default:
		return;
	}
	playSound(_blipSoundId);
}

void MenuScreenPage::handleKeyInput(uint32 keyCode) {
	if (keyCode < 128 && strchr(kValidNameCharacters, static_cast<char>(keyCode))) {
		const SaveFileList::TextInputResult result = _fileList->handleCharacter(static_cast<char>(keyCode));
		if (result == SaveFileList::kTextAccepted)
			playSound(_typeSoundId);
		else if (result == SaveFileList::kTextListFull)
			warning("MenuScreenPage: Save list is full");
		return;
	}

	switch (keyCode) {
	case Common::KEYCODE_UP:
		_fileList->moveSelectionUp();
		break;
	case Common::KEYCODE_DOWN:
		_fileList->moveSelectionDown();
		break;
	case Common::KEYCODE_PAGEUP:
		_fileList->scrollPageUp();
		break;
	case Common::KEYCODE_PAGEDOWN:
		_fileList->scrollPageDown();
		break;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		startSelectedSave();
		break;
	case Common::KEYCODE_BACKSPACE:
		if (_fileList->handleBackspace())
			playSound(_deleteSoundId);
		break;
	case Common::KEYCODE_DELETE:
		requestDeleteConfirmation();
		break;
	default:
		break;
	}
}

void MenuScreenPage::startSelectedSave() {
	if (!_fileList->hasValidSelection())
		return;

	const Common::String saveName = _fileList->getSelectedName();
	if (saveName.empty())
		return;

	GameState *gameState = _engine->getGameState();
	bool success = false;
	if (_fileList->isEditing()) {
		gameState->init();
		gameState->_playerName = saveName;
		success = _engine->writeGameSave(saveName);
	} else {
		success = _engine->readGameSave(saveName);
		if (!success)
			_fileList->deleteSelected();
	}

	if (!success)
		return;
	if (gameState->isWorldVisitedAtDiff(0, 1))
		_engine->requestPageChange(kPageMenuLoad);
	else
		_engine->requestPageChange(kPageTitleAnim);
}

void MenuScreenPage::requestDeleteConfirmation() {
	if (_fileList->hasValidSelection() && !_fileList->isEditing())
		openConfirmDialog(kSaveMenuConfirmDelete);
}

void MenuScreenPage::openOptionsDialog() {
	if (!_volumePanel) {
		_volumePanel = new VolumePanel();
		_volumePanel->init();
		SoundManager *sound = _engine->getSoundManager();
		if (sound)
			_volumePanel->setInitialVolumes(sound->_volumeMusic, sound->_volumeSFX,
			                                sound->_volumeSpeech);
	}
	_state = kSaveMenuOptions;
}

void MenuScreenPage::closeOptionsDialog(bool applyChanges) {
	applyOptionVolumes(applyChanges);
	delete _volumePanel;
	_volumePanel = nullptr;
	_state = kSaveMenuMain;
}

void MenuScreenPage::applyOptionVolumes(bool usePanelValues) {
	if (!_volumePanel)
		return;
	SoundManager *sound = _engine->getSoundManager();
	if (!sound)
		return;
	sound->_volumeMusic = usePanelValues ? _volumePanel->getMusicVolume() :
	                                      _volumePanel->getInitialMusicVolume();
	sound->_volumeSFX = usePanelValues ? _volumePanel->getSfxVolume() :
	                                    _volumePanel->getInitialSfxVolume();
	sound->_volumeSpeech = usePanelValues ? _volumePanel->getSpeechVolume() :
	                                       _volumePanel->getInitialSpeechVolume();
	if (0 <= _mapMusicId)
		sound->setVolume(_mapMusicId, sound->_volumeMusic);
}

void MenuScreenPage::openConfirmDialog(SaveMenuConfirmType type) {
	_confirmType = type;
	_confirmButtonHover = 0;

	if (!_confirmPanelNothing) {
		_confirmPanelNothing = new RleBlock();
		_confirmPanelNothing->loadFromFile(Common::Path("bmp/menu/QUIT_panel_nothing.rb"));
	}
	if (!_confirmPanelOk) {
		_confirmPanelOk = new RleBlock();
		_confirmPanelOk->loadFromFile(Common::Path("bmp/menu/QUIT_panel_ok.rb"));
	}
	if (!_confirmPanelCancel) {
		_confirmPanelCancel = new RleBlock();
		_confirmPanelCancel->loadFromFile(Common::Path("bmp/menu/QUIT_panel_cancel.rb"));
	}

	if (type == kSaveMenuConfirmDelete && !_confirmTextDelete) {
		_confirmTextDelete = new BitBlock();
		_confirmTextDelete->load(Common::Path("bmp/menu/Quit_panel_text_suppr"));
	} else if (type == kSaveMenuConfirmQuit && !_confirmTextQuit) {
		_confirmTextQuit = new BitBlock();
		_confirmTextQuit->load(Common::Path("bmp/menu/Quit_panel_text_quit"));
	}

	if (_confirmPanelOk && _confirmPanelOk->isValid()) {
		_confirmX = kScreenWidth / 2 - _confirmPanelOk->getWidth() / 2;
		_confirmY = kScreenHeight / 2 - _confirmPanelOk->getHeight() / 2;
	}
	_state = kSaveMenuConfirm;
}

void MenuScreenPage::closeConfirmDialog() {
	_confirmType = kSaveMenuConfirmNone;
	_confirmButtonHover = 0;
	_state = kSaveMenuMain;
}

void MenuScreenPage::drawConfirmDialog(Graphics::ManagedSurface *screen) {
	RleBlock *panel = _confirmPanelNothing;
	if (_confirmButtonHover == 1)
		panel = _confirmPanelOk;
	else if (_confirmButtonHover == 2)
		panel = _confirmPanelCancel;
	if (panel && panel->isValid())
		panel->drawToScreen(screen, _confirmX, _confirmY, _engine->getAlphaLUT());

	BitBlock *textImage = _confirmType == kSaveMenuConfirmDelete ? _confirmTextDelete : _confirmTextQuit;
	if (textImage)
		textImage->drawToSurface(screen, _confirmX, _confirmY);
}

int MenuScreenPage::hitTestConfirmDialog(int x, int y) const {
	if (_confirmX + 207 < x && x < _confirmX + 272 &&
	    _confirmY + 77 < y && y < _confirmY + 145)
		return 1;
	if (_confirmX + 287 < x && x < _confirmX + 352 &&
	    _confirmY + 77 < y && y < _confirmY + 145)
		return 2;
	return 0;
}

void MenuScreenPage::handleConfirmClick(int buttonId) {
	if (buttonId == 1) {
		if (_confirmType == kSaveMenuConfirmDelete)
			deleteSelectedSave();
		else if (_confirmType == kSaveMenuConfirmQuit) {
			closeConfirmDialog();
			_engine->setQuitAfterCredits(true);
			_engine->requestPageChange(kPageCredits);
			return;
		}
	}
	closeConfirmDialog();
}

void MenuScreenPage::deleteSelectedSave() {
	const Common::String saveName = _fileList->getSelectedName();
	if (saveName.empty())
		return;

	_engine->deleteGameSave(saveName);
	_fileList->deleteSelected();
	playSound(_deleteSoundId);
}

void MenuScreenPage::playSound(int soundId) {
	SoundManager *sound = _engine->getSoundManager();
	if (sound && 0 <= soundId)
		sound->playWithVolume(soundId, sound->_volumeSFX);
}

} // End of namespace Zoombini2
