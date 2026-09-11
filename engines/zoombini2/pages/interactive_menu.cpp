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

#include "common/callback.h"
#include "common/debug.h"
#include "zoombini2/graphics.h"
#include "zoombini2/pages/dialog_msgbox.h"
#include "zoombini2/pages/interactive_menu.h"
#include "zoombini2/pages/save_file_list.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

const Common::Point32 InteractiveMenu::kFileListPos(157, 286);

const char *const InteractiveMenu::kValidNameCharacters =
	"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";

InteractiveMenu::InteractiveMenu(Zoombini2Engine *vm)
	: InteractiveBase(vm) {
	_pageId = kPageMenuOptions;
}

InteractiveMenu::~InteractiveMenu() {
	delete _fileList;
	delete _background;
	delete _selectorNormal;
	delete _selectorHilite;
	delete _selectionBar;
	for (int i = 0; i < kMenuButtonCount; ++i)
		delete _buttons[i];

	delete _volumePanel;

	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		if (0 <= _blipSoundId)
			sound->unload(_blipSoundId);
		if (0 <= _typeSoundId)
			sound->unload(_typeSoundId);
		if (0 <= _deleteSoundId)
			sound->unload(_deleteSoundId);
	}
}

void InteractiveMenu::init() {
	_vm->getGameState()->init();
	_vm->clearGlobalZoombinis();
	loadResources();
	loadButtons();

	_fileList = new SaveFileList(_vm, kFileListPos, _selectionBar);
	if (!_fileList->init())
		warning("MenuScreenPage: Failed to load save-list fonts");
	scanSaveFiles();

	_mapMusicId = _vm->ensureMapMusic();
	_state = MenuScreenState::kMain00;
}

void InteractiveMenu::loadResources() {
	_background = new BitBlock(_vm);
	if (!_background->load(Common::Path("#bmp/menu/background")))
		warning("MenuScreenPage: Failed to load menu background");

	_selectorNormal = new BitBlock(_vm);
	if (!_selectorNormal->load(Common::Path("bmp/menu/PARTIEs - selector NORMAL")))
		warning("MenuScreenPage: Failed to load normal selector");

	_selectorHilite = new BitBlock(_vm);
	if (!_selectorHilite->load(Common::Path("bmp/menu/PARTIEs - selector HILITE")))
		warning("MenuScreenPage: Failed to load highlighted selector");

	_selectionBar = new RleBlock(_vm);
	if (!_selectionBar->loadFromFile(Common::Path("bmp/menu/barre-cache.rb")))
		warning("MenuScreenPage: Failed to load save selection bar");

	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		_blipSoundId = sound->load(false, Common::Path("sounds/blip.wav"), false);
		_typeSoundId = sound->load(false, Common::Path("sounds/fx/i-bs5.wav"), false);
		_deleteSoundId = sound->load(false, Common::Path("sounds/fx/del.wav"), false);
	}
}

void InteractiveMenu::loadButtons() {
	struct ButtonDefinition {
		const char *normalPath;
		const char *hoverPath;
		const char *disabledPath;
		Common::Point32 pos;
		int width;
		int height;
	};

	const ButtonDefinition definitions[kMenuButtonCount] = {
		{"bmp/menu/PARTIEs - ArrowUP NORMAL", "bmp/menu/PARTIEs - ArrowUP HIGHLIGHT", nullptr, Common::Point32(613, 350), 46, 50},
		{"bmp/menu/PARTIEs - ArrowDOWN NORMAL", "bmp/menu/PARTIEs - ArrowDOWN HILITE", nullptr, Common::Point32(613, 416), 46, 50},
		{"bmp/menu/Start Normal", "bmp/menu/Start Highlight", "bmp/menu/Start Gray", Common::Point32(27, 561), 145, 39},
		{"bmp/menu/PANEL - Options  NORMAL", "bmp/menu/PANEL - Options  HIGHLIGHT", nullptr, Common::Point32(175, 561), 145, 39},
		{"bmp/menu/New Normal", "bmp/menu/New Highlight", "bmp/menu/New Gray", Common::Point32(321, 561), 145, 39},
		{"bmp/menu/PANEL - Entrainement NORMAL", "bmp/menu/PANEL - Entraine HILITE", nullptr, Common::Point32(468, 561), 145, 39},
		{"bmp/menu/PANEL - Quitter NORMAL", "bmp/menu/PANEL - Quitter HIGHLIGHT", nullptr, Common::Point32(613, 561), 145, 39}};

	for (int i = 0; i < kMenuButtonCount; ++i) {
		const ButtonDefinition &definition = definitions[i];
		_buttons[i] = new UIButton(_vm);
		_buttons[i]->setRect(definition.pos, definition.width, definition.height);
		_buttons[i]->loadImages(Common::Path(definition.normalPath),
								Common::Path(definition.hoverPath),
								definition.disabledPath ? Common::Path(definition.disabledPath) : Common::Path());
	}
}

void InteractiveMenu::scanSaveFiles() {
	_fileList->clear();
	const Common::StringArray profiles = _vm->listGameSaves();
	for (uint i = 0; i < profiles.size() && _fileList->getItemCount() < SaveFileList::kMaximumItems; i++)
		_fileList->addItemSorted(profiles[i]);
}

void InteractiveMenu::onUpdate() {
	updateButtonAvailability();
	const Common::Point32 mousePos = _vm->getMousePos();
	onMouseMove(Common::Point(mousePos.x, mousePos.y));
}

void InteractiveMenu::onRenderScene(ManagedSurface32 *screen) {
	if (_state == MenuScreenState::kOptions01) {
		if (_background)
			_background->drawToSurface(screen, Common::Point32(0, 0));
	} else {
		drawMain(screen);
	}
}

void InteractiveMenu::onRenderForeground(ManagedSurface32 *screen) {
	if (_state == MenuScreenState::kOptions01 && _volumePanel) {
		const Common::Point32 mousePos(_vm->getMousePos());
		_volumePanel->draw(screen, mousePos, _vm->getAlphaLUT());
	}
}

EventHandleResult InteractiveMenu::handleVolumePanelInput(const Common::Point &pos, bool mouseReleased) {
	if (!_volumePanel)
		return EventHandleResult::kPassthrough;
	const VolumePanelResult result = _volumePanel->handleMouseInput(Common::Point32(pos), _volumePanelMouseDown, mouseReleased);
	if (result == kVolumePanelChanged) {
		applyOptionVolumes(true, false);
		_volumePanel->playPreviewSound();
	} else if (result == kVolumePanelApply) {
		closeOptionsDialog(true);
	} else if (result == kVolumePanelCancel) {
		closeOptionsDialog(false);
	}
	return EventHandleResult::kConsumed;
}

EventHandleResult InteractiveMenu::onLButtonUp(const Common::Point &pos) {
	_volumePanelMouseDown = false;
	return handleVolumePanelInput(pos, true);
}

EventHandleResult InteractiveMenu::onMouseMove(const Common::Point &pos) {
	if (_state == MenuScreenState::kOptions01) {
		if (_volumePanel)
			_volumePanel->handleMouseInput(Common::Point32(pos), _volumePanelMouseDown, false);
		return EventHandleResult::kConsumed;
	}
	const int hovered = hitTestButton(pos);
	if (hovered != _hoveredButton && 0 <= hovered)
		playSound(_blipSoundId);
	_hoveredButton = hovered;
	return EventHandleResult::kPassthrough;
}

EventHandleResult InteractiveMenu::onKeyDown(const Common::KeyState &key, bool repeat) {
	(void)repeat;
	if (_state == MenuScreenState::kMain00) {
		const uint32 keyCode = key.ascii ? static_cast<uint32>(key.ascii) : static_cast<uint32>(key.keycode);
		handleKeyInput(keyCode);
		updateButtonAvailability();
	}
	return EventHandleResult::kConsumed;
}
void InteractiveMenu::drawMain(ManagedSurface32 *screen) {
	if (_background)
		_background->drawToSurface(screen, Common::Point32(0, 0));

	const Common::Point32 mousePos = _vm->getMousePos();
	const Common::Point mouseEventPos(mousePos.x, mousePos.y);
	BitBlock *selector = isInSelectorArea(mouseEventPos) ? _selectorHilite : _selectorNormal;
	if (selector)
		selector->drawToSurface(screen, kFileListPos);
	if (_fileList)
		_fileList->draw(screen, _vm->getAlphaLUT());
	drawButtons(screen, mousePos);
}

void InteractiveMenu::updateButtonAvailability() {
	_buttons[kMenuButtonNext]->setEnabled(_fileList->canMoveUp());
	_buttons[kMenuButtonPrev]->setEnabled(_fileList->canMoveDown());
	_buttons[kMenuButtonStart]->setEnabled(_fileList->hasValidSelection());
	_buttons[kMenuButtonNew]->setEnabled(_fileList->canBeginNewEntry());
}

void InteractiveMenu::drawButtons(ManagedSurface32 *screen, const Common::Point32 &mousePos) {
	for (int i = 0; i < kMenuButtonCount; i++) {
		if (_buttons[i])
			_buttons[i]->drawAndHitTest(screen, mousePos, _vm->getAlphaLUT());
	}
}

EventHandleResult InteractiveMenu::onLButtonDown(const Common::Point &pos) {
	updateButtonAvailability();
	if (_state == MenuScreenState::kOptions01) {
		_volumePanelMouseDown = true;
		if (_volumePanel)
			_volumePanel->handleMouseInput(Common::Point32(pos), true, false);
		return EventHandleResult::kConsumed;
	}

	const int buttonId = hitTestButton(pos);
	if (0 <= buttonId) {
		handleButtonClick(buttonId);
		return EventHandleResult::kConsumed;
	}
	if (_fileList->handleClick(pos)) {
		playSound(_blipSoundId);
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

int InteractiveMenu::hitTestButton(const Common::Point &pos) const {
	for (int i = 0; i < kMenuButtonCount; ++i) {
		if (_buttons[i] && _buttons[i]->isEnabled() && _buttons[i]->containsPoint(pos))
			return i;
	}
	return -1;
}

bool InteractiveMenu::isInSelectorArea(const Common::Point &pos) const {
	return kFileListPos.x < pos.x && pos.x < kFileListPos.x + kSelectorWidth &&
		   kFileListPos.y < pos.y && pos.y < kFileListPos.y + kSelectorHeight;
}

void InteractiveMenu::handleButtonClick(int buttonId) {
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
		_vm->requestPageChange(kPageMenuPractice);
		break;
	case kMenuButtonQuit:
		requestQuitConfirmation();
		break;
	default:
		return;
	}
	playSound(_blipSoundId);
}

void InteractiveMenu::handleKeyInput(uint32 keyCode) {
	if (keyCode < 128 && strchr(kValidNameCharacters, static_cast<char>(keyCode))) {
		const SaveFileList::TextInputResult result = _fileList->handleCharacter(static_cast<char>(keyCode));
		if (result == SaveFileList::kTextAccepted01)
			playSound(_typeSoundId);
		else if (result == SaveFileList::kTextListFull02)
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

void InteractiveMenu::startSelectedSave() {
	if (!_fileList->hasValidSelection())
		return;

	const Common::String saveName = _fileList->getSelectedName();
	if (saveName.empty())
		return;

	GameState *gameState = _vm->getGameState();
	bool success = false;
	if (_fileList->isEditing()) {
		gameState->init();
		gameState->_playerName = saveName;
		success = _vm->writeGameSave(saveName);
	} else {
		success = _vm->readGameSave(saveName);
		if (!success)
			_fileList->deleteSelected();
	}

	if (!success)
		return;
	if (gameState->hasPageVisit(0, 1))
		_vm->requestPageChange(kPageMenuLoad);
	else
		_vm->requestPageChange(kPageCutsceneFirst);
}

void InteractiveMenu::requestDeleteConfirmation() {
	if (!_fileList->hasValidSelection() || _fileList->isEditing())
		return;

	_pendingDeleteProfileName = _fileList->getSelectedName();
	if (_pendingDeleteProfileName.empty())
		return;
	_vm->getMsgBoxDialog()->request(Common::Path("bmp/menu/Quit_panel_text_suppr"),
		new Common::Callback<InteractiveMenu, DialogMsgBoxButton>(this, &InteractiveMenu::handleDeleteConfirmation));
}

void InteractiveMenu::requestQuitConfirmation() {
	_vm->getMsgBoxDialog()->request(Common::Path("bmp/menu/Quit_panel_text_quit"),
		new Common::Callback<InteractiveMenu, DialogMsgBoxButton>(this, &InteractiveMenu::handleQuitConfirmation));
}

void InteractiveMenu::handleDeleteConfirmation(DialogMsgBoxButton button) {
	if (button == DialogMsgBoxButton::kOkay01)
		deleteSelectedSave();
	_pendingDeleteProfileName.clear();
}

void InteractiveMenu::handleQuitConfirmation(DialogMsgBoxButton button) {
	if (button == DialogMsgBoxButton::kOkay01)
		_vm->requestPageChange(kPageCredits);
}

void InteractiveMenu::openOptionsDialog() {
	if (!_volumePanel) {
		_volumePanel = new VolumePanel(_vm);
		_volumePanel->init(_vm->getSoundManager());
		_volumePanel->setInitialVolumes(_vm->getMusicVolume(), _vm->getSFXVolume(), _vm->getSpeechVolume());
	}
	_state = MenuScreenState::kOptions01;
}

void InteractiveMenu::closeOptionsDialog(bool applyChanges) {
	applyOptionVolumes(applyChanges, applyChanges);
	delete _volumePanel;
	_volumePanel = nullptr;
	_state = MenuScreenState::kMain00;
}

void InteractiveMenu::applyOptionVolumes(bool usePanelValues, bool persistChanges) {
	if (!_volumePanel)
		return;
	const int music = usePanelValues ? _volumePanel->getMusicVolume() : _volumePanel->getInitialMusicVolume();
	const int sfx = usePanelValues ? _volumePanel->getSfxVolume() : _volumePanel->getInitialSfxVolume();
	const int speech = usePanelValues ? _volumePanel->getSpeechVolume() : _volumePanel->getInitialSpeechVolume();
	if (persistChanges)
		_vm->saveSoundVolumes(music, sfx, speech);
	else
		_vm->previewSoundVolumes(music, sfx, speech);
}

void InteractiveMenu::deleteSelectedSave() {
	if (_pendingDeleteProfileName.empty())
		return;

	if (_vm->deleteGameSave(_pendingDeleteProfileName)) {
		_fileList->deleteSelected();
		playSound(_deleteSoundId);
	} else {
		warning("MenuScreenPage: Failed to delete profile '%s'", _pendingDeleteProfileName.c_str());
	}
}

void InteractiveMenu::playSound(int soundId) {
	SoundManager *sound = _vm->getSoundManager();
	if (sound && 0 <= soundId)
		sound->playWithVolume(soundId, sound->_volumeSFX);
}

} // End of namespace Zoombini2
