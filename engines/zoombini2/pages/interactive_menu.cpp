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

#include "common/algorithm.h"
#include "common/callback.h"
#include "common/debug.h"
#include "zoombini2/graphics.h"
#include "zoombini2/pages/dialog_msgbox.h"
#include "zoombini2/pages/interactive_menu.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *InteractiveMenu::kBackgroundPath;
constexpr const char *InteractiveMenu::kSelectorNormalPath;
constexpr const char *InteractiveMenu::kSelectorHighlightPath;
constexpr const char *InteractiveMenu::kSelectionBarPath;
constexpr const char *InteractiveMenu::kBlipSoundPath;
constexpr const char *InteractiveMenu::kTypeSoundPath;
constexpr const char *InteractiveMenu::kDeleteSoundPath;
constexpr const char *InteractiveMenu::kArrowUpNormalPath;
constexpr const char *InteractiveMenu::kArrowUpHighlightPath;
constexpr const char *InteractiveMenu::kArrowDownNormalPath;
constexpr const char *InteractiveMenu::kArrowDownHighlightPath;
constexpr const char *InteractiveMenu::kStartNormalPath;
constexpr const char *InteractiveMenu::kStartHighlightPath;
constexpr const char *InteractiveMenu::kStartDisabledPath;
constexpr const char *InteractiveMenu::kOptionsNormalPath;
constexpr const char *InteractiveMenu::kOptionsHighlightPath;
constexpr const char *InteractiveMenu::kNewNormalPath;
constexpr const char *InteractiveMenu::kNewHighlightPath;
constexpr const char *InteractiveMenu::kNewDisabledPath;
constexpr const char *InteractiveMenu::kPracticeNormalPath;
constexpr const char *InteractiveMenu::kPracticeHighlightPath;
constexpr const char *InteractiveMenu::kQuitNormalPath;
constexpr const char *InteractiveMenu::kQuitHighlightPath;
constexpr const char *InteractiveMenu::kDeleteConfirmationPath;
constexpr const char *InteractiveMenu::kQuitConfirmationPath;

constexpr Common::Point32 InteractiveMenu::kFileListPos;
constexpr Size32 InteractiveMenu::kSelectorSize;

constexpr const char *InteractiveMenu::kValidNameCharacters;

InteractiveMenu::InteractiveMenu(Zoombini2Engine *vm)
	: InteractiveBase(vm) {
	_pageId = kPageMenuOptions;
}

InteractiveMenu::~InteractiveMenu() {
	delete _fileList;
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
	if (!_vm->_gfx->loadBackground(Common::Path(kBackgroundPath)))
		warning("MenuScreenPage: Failed to load menu background");

	_selectorNormal = new BitBlock(_vm);
	if (!_selectorNormal->load(Common::Path(kSelectorNormalPath)))
		warning("MenuScreenPage: Failed to load normal selector");

	_selectorHilite = new BitBlock(_vm);
	if (!_selectorHilite->load(Common::Path(kSelectorHighlightPath)))
		warning("MenuScreenPage: Failed to load highlighted selector");

	_selectionBar = new RleBlock(_vm);
	if (!_selectionBar->loadFromFile(Common::Path(kSelectionBarPath)))
		warning("MenuScreenPage: Failed to load save selection bar");

	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		_blipSoundId = sound->load(false, Common::Path(kBlipSoundPath), false);
		_typeSoundId = sound->load(false, Common::Path(kTypeSoundPath), false);
		_deleteSoundId = sound->load(false, Common::Path(kDeleteSoundPath), false);
	}
}

void InteractiveMenu::loadButtons() {
	struct ButtonDefinition {
		const char *normalPath;
		const char *hoverPath;
		const char *disabledPath;
		Common::Point32 pos;
		Size32 size;
	};

	const ButtonDefinition definitions[kMenuButtonCount] = {
		{kArrowUpNormalPath, kArrowUpHighlightPath, nullptr, Common::Point32(613, 350), Size32(46, 50)},
		{kArrowDownNormalPath, kArrowDownHighlightPath, nullptr, Common::Point32(613, 416), Size32(46, 50)},
		{kStartNormalPath, kStartHighlightPath, kStartDisabledPath, Common::Point32(27, 561), Size32(145, 39)},
		{kOptionsNormalPath, kOptionsHighlightPath, nullptr, Common::Point32(175, 561), Size32(145, 39)},
		{kNewNormalPath, kNewHighlightPath, kNewDisabledPath, Common::Point32(321, 561), Size32(145, 39)},
		{kPracticeNormalPath, kPracticeHighlightPath, nullptr, Common::Point32(468, 561), Size32(145, 39)},
		{kQuitNormalPath, kQuitHighlightPath, nullptr, Common::Point32(613, 561), Size32(145, 39)},
	};

	for (int i = 0; i < kMenuButtonCount; ++i) {
		const ButtonDefinition &definition = definitions[i];
		_buttons[i] = new UIButton(_vm);
		_buttons[i]->setRect(definition.pos, definition.size);
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
	if (_state == MenuScreenState::kMain00) {
		updateButtonAvailability();
	} else if (_volumePanel) {
		_volumePanel->handleMouseInput(_vm->getMousePos(), _volumePanelMouseDown, false);
	}
}

void InteractiveMenu::onRenderContent(ManagedSurface32 *screen) {
	if (_state == MenuScreenState::kOptions01) {
		_vm->_gfx->drawBackground(screen, Common::Point32(0, 0));
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
	(void)pos;
	return _state == MenuScreenState::kOptions01 ? EventHandleResult::kConsumed : EventHandleResult::kPassthrough;
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
	_vm->_gfx->drawBackground(screen, Common::Point32(0, 0));

	const Common::Point32 mousePos = _vm->getMousePos();
	const Common::Point mouseEventPos(mousePos.x, mousePos.y);
	BitBlock *selector = isInSelectorArea(mouseEventPos) ? _selectorHilite : _selectorNormal;
	if (selector)
		selector->drawToSurface(screen, kFileListPos);
	if (_fileList)
		_fileList->draw(screen, _vm->getAlphaLUT());
	drawButtonsAndUpdateHover(screen, mousePos);
}

void InteractiveMenu::updateButtonAvailability() {
	_buttons[kMenuButtonNext]->setEnabled(_fileList->canMoveUp());
	_buttons[kMenuButtonPrev]->setEnabled(_fileList->canMoveDown());
	_buttons[kMenuButtonStart]->setEnabled(_fileList->hasValidSelection());
	_buttons[kMenuButtonNew]->setEnabled(_fileList->canBeginNewEntry());
}

void InteractiveMenu::drawButtonsAndUpdateHover(ManagedSurface32 *screen, const Common::Point32 &mousePos) {
	for (int i = 0; i < kMenuButtonCount; i++) {
		if (_buttons[i] && _buttons[i]->drawAndHitTest(screen, mousePos, _vm->getAlphaLUT()) == 2)
			playSound(_blipSoundId);
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
	return kFileListPos.x < pos.x && pos.x < kFileListPos.x + kSelectorSize.width &&
		   kFileListPos.y < pos.y && pos.y < kFileListPos.y + kSelectorSize.height;
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
	_vm->getMsgBoxDialog()->request(Common::Path(kDeleteConfirmationPath), new Common::Callback<InteractiveMenu, DialogMsgBoxButton>(this, &InteractiveMenu::handleDeleteConfirmation));
}

void InteractiveMenu::requestQuitConfirmation() {
	_vm->getMsgBoxDialog()->request(Common::Path(kQuitConfirmationPath), new Common::Callback<InteractiveMenu, DialogMsgBoxButton>(this, &InteractiveMenu::handleQuitConfirmation));
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

InteractiveMenu::SaveFileList::SaveFileList(Zoombini2Engine *vm, const Common::Point32 &pos, RleBlock *selectionBar)
	: _vm(vm), _pos(pos), _selectionBar(selectionBar) {
}

bool InteractiveMenu::SaveFileList::init() {
	return _vm->_gfx->loadTextFont(Gfx::TextColor::kDark00) &&
		   _vm->_gfx->loadTextFont(Gfx::TextColor::kBlue01) &&
		   _vm->_gfx->loadTextFont(Gfx::TextColor::kGreen02);
}

void InteractiveMenu::SaveFileList::clear() {
	_items.clear();
	_editBuffer.clear();
	_editState = kEditIdle00;
	_selectedIndex = 0;
	_scrollOffset = 0;
	_validSelection = false;
}

bool InteractiveMenu::SaveFileList::addItemSorted(const Common::String &name) {
	if (kMaximumItems <= static_cast<int>(_items.size()) || name.empty() ||
		kMaximumNameLength < static_cast<int>(name.size()))
		return false;

	insertItem(findInsertionPoint(name), name);
	_selectedIndex = 0;
	_scrollOffset = 0;
	_validSelection = true;
	return true;
}

void InteractiveMenu::SaveFileList::draw(ManagedSurface32 *screen, const AlphaBlendLUT &alphaLUT) const {
	for (int row = 0; row < kVisibleRows; ++row) {
		const int itemIndex = _scrollOffset + row;
		if (static_cast<int>(_items.size()) <= itemIndex)
			break;

		const bool selected = itemIndex == _selectedIndex;
		if (selected && _selectionBar && _selectionBar->isValid()) {
			_selectionBar->drawToScreen(screen, Common::Point32(_pos.x + kSelectionOffsetX, _pos.y + kSelectionOffsetY + row * kRowStride), alphaLUT);
		}

		Gfx::TextColor color = Gfx::TextColor::kDark00;
		if (selected && _editState == kEditPrefixMatch01)
			color = Gfx::TextColor::kBlue01;
		else if (selected && kEditProvisional02 <= _editState)
			color = Gfx::TextColor::kGreen02;

		const Common::Point32 textPos(_pos.x + kSelectionOffsetX + kTextOffsetX, _pos.y + kSelectionOffsetY + kTextOffsetY + row * kRowStride);
		_vm->_gfx->drawText(screen, color, textPos, _items[itemIndex]);
	}
}

bool InteractiveMenu::SaveFileList::handleClick(const Common::Point &pos) {
	if (kEditPrefixMatch01 < _editState || !_selectionBar || !_selectionBar->isValid())
		return false;

	const int left = _pos.x + kSelectionOffsetX;
	const int top = _pos.y + kSelectionOffsetY;
	const Size32 selectionSize(_selectionBar->getSize().width, _selectionBar->getSize().height * kVisibleRows);
	if (pos.x < left || left + selectionSize.width <= pos.x || pos.y < top || top + selectionSize.height <= pos.y)
		return false;

	const int row = (pos.y - top) / kRowStride;
	const int itemIndex = _scrollOffset + row;
	if (row < 0 || kVisibleRows <= row || static_cast<int>(_items.size()) <= itemIndex)
		return false;

	_selectedIndex = itemIndex;
	_editBuffer.clear();
	_editState = kEditIdle00;
	_validSelection = true;
	return true;
}

InteractiveMenu::SaveFileList::TextInputResult InteractiveMenu::SaveFileList::handleCharacter(char c) {
	if (!canAppendCharacter(c))
		return kTextRejected00;

	c = normalizeCharacter(c);
	_editBuffer += c;

	if (_editState <= kEditPrefixMatch01) {
		const int matchingIndex = findPrefix(_editBuffer);
		if (0 <= matchingIndex) {
			_selectedIndex = matchingIndex;
			_editState = kEditPrefixMatch01;
		} else {
			if (kMaximumItems <= static_cast<int>(_items.size())) {
				_editBuffer.deleteLastChar();
				return kTextListFull02;
			}

			_selectedIndex = findInsertionPoint(_editBuffer);
			insertItem(_selectedIndex, _editBuffer);
			_editState = kEditProvisional02;
		}
		_validSelection = true;
	} else if (_editState == kEditProvisional02) {
		_items[_selectedIndex] = _editBuffer;
		_validSelection = true;
	} else {
		_items[_selectedIndex] = _editBuffer;
		_validSelection = !isDuplicate(_editBuffer, _selectedIndex);
	}

	revealSelection();
	return kTextAccepted01;
}

bool InteractiveMenu::SaveFileList::handleBackspace() {
	if (_editBuffer.empty() && _editState != kEditExplicit03)
		return false;
	if (_editBuffer.empty()) {
		removeItem(_selectedIndex);
		_editState = kEditIdle00;
		clampSelection();
		_validSelection = !_items.empty();
		revealSelection();
		return true;
	}

	_editBuffer.deleteLastChar();

	if (_editState == kEditPrefixMatch01) {
		if (_editBuffer.empty()) {
			_editState = kEditIdle00;
			_validSelection = !_items.empty();
		} else {
			const int matchingIndex = findPrefix(_editBuffer);
			if (0 <= matchingIndex)
				_selectedIndex = matchingIndex;
			_validSelection = 0 <= matchingIndex;
		}
	} else if (_editState == kEditProvisional02) {
		if (_editBuffer.empty()) {
			removeItem(_selectedIndex);
			_editState = kEditIdle00;
			_selectedIndex = 0;
			_scrollOffset = 0;
			_validSelection = !_items.empty();
		} else {
			const int provisionalIndex = _selectedIndex;
			const int matchingIndex = findPrefix(_editBuffer, provisionalIndex);
			if (0 <= matchingIndex) {
				removeItem(provisionalIndex);
				_selectedIndex = matchingIndex;
				if (provisionalIndex < matchingIndex)
					_selectedIndex -= 1;
				_editState = kEditPrefixMatch01;
			} else {
				_items[_selectedIndex] = _editBuffer;
			}
			_validSelection = true;
		}
	} else if (_editState == kEditExplicit03) {
		if (_editBuffer.empty()) {
			removeItem(_selectedIndex);
			_editState = kEditIdle00;
			clampSelection();
			_validSelection = !_items.empty();
		} else {
			_items[_selectedIndex] = _editBuffer;
			_validSelection = !isDuplicate(_editBuffer, _selectedIndex);
		}
	}

	revealSelection();
	return true;
}

void InteractiveMenu::SaveFileList::beginOrConfirmNewEntry() {
	if (!canBeginNewEntry())
		return;

	if (_editState == kEditIdle00) {
		insertItem(0, Common::String());
		_selectedIndex = 0;
		_scrollOffset = 0;
		_editBuffer.clear();
		_editState = kEditExplicit03;
		_validSelection = false;
	} else if (_editState == kEditPrefixMatch01) {
		insertItem(_selectedIndex, _editBuffer);
		_editState = kEditExplicit03;
		_validSelection = !isDuplicate(_editBuffer, _selectedIndex);
	} else if (_editState == kEditProvisional02) {
		_editState = kEditExplicit03;
		_validSelection = !isDuplicate(_editBuffer, _selectedIndex);
	}

	revealSelection();
}

void InteractiveMenu::SaveFileList::moveSelectionUp() {
	if (!canMoveUp())
		return;
	_selectedIndex -= 1;
	_editBuffer.clear();
	revealSelection();
}

void InteractiveMenu::SaveFileList::moveSelectionDown() {
	if (!canMoveDown())
		return;
	_selectedIndex += 1;
	_editBuffer.clear();
	revealSelection();
}

void InteractiveMenu::SaveFileList::scrollPageUp() {
	if (!canPageUp())
		return;
	_scrollOffset = MAX(0, _scrollOffset - kVisibleRows);
	if (_scrollOffset + kVisibleRows <= _selectedIndex)
		_selectedIndex = _scrollOffset + kVisibleRows - 1;
}

void InteractiveMenu::SaveFileList::scrollPageDown() {
	if (!canPageDown())
		return;
	_scrollOffset = MIN(static_cast<int>(_items.size()) - kVisibleRows,
						_scrollOffset + kVisibleRows);
	if (_selectedIndex < _scrollOffset)
		_selectedIndex = _scrollOffset;
}

bool InteractiveMenu::SaveFileList::canMoveUp() const {
	return _editState == kEditIdle00 && 0 < _selectedIndex;
}

bool InteractiveMenu::SaveFileList::canMoveDown() const {
	return _editState == kEditIdle00 && _selectedIndex + 1 < static_cast<int>(_items.size());
}

bool InteractiveMenu::SaveFileList::canPageUp() const {
	return _editState == kEditIdle00 && 0 < _scrollOffset;
}

bool InteractiveMenu::SaveFileList::canPageDown() const {
	return _editState == kEditIdle00 && _scrollOffset + kVisibleRows < static_cast<int>(_items.size());
}

bool InteractiveMenu::SaveFileList::canBeginNewEntry() const {
	return _items.size() < kMaximumItems && !isEditing();
}

Common::String InteractiveMenu::SaveFileList::getSelectedName() const {
	if (!_validSelection || _selectedIndex < 0 || static_cast<int>(_items.size()) <= _selectedIndex)
		return Common::String();
	return _items[_selectedIndex];
}

void InteractiveMenu::SaveFileList::deleteSelected() {
	if (_selectedIndex < 0 || static_cast<int>(_items.size()) <= _selectedIndex)
		return;

	removeItem(_selectedIndex);
	_editBuffer.clear();
	_editState = kEditIdle00;
	clampSelection();
	_validSelection = !_items.empty();
}

int InteractiveMenu::SaveFileList::findInsertionPoint(const Common::String &name) const {
	int index = 0;
	while (index < static_cast<int>(_items.size()) && _items[index].compareToIgnoreCase(name) < 0)
		index += 1;
	return index;
}

int InteractiveMenu::SaveFileList::findPrefix(const Common::String &prefix, int ignoredIndex) const {
	for (int i = 0; i < static_cast<int>(_items.size()); ++i) {
		if (i != ignoredIndex && _items[i].hasPrefixIgnoreCase(prefix))
			return i;
	}
	return -1;
}

bool InteractiveMenu::SaveFileList::isDuplicate(const Common::String &name, int ignoredIndex) const {
	for (int i = 0; i < static_cast<int>(_items.size()); ++i) {
		if (i != ignoredIndex && _items[i].equalsIgnoreCase(name))
			return true;
	}
	return false;
}

bool InteractiveMenu::SaveFileList::canAppendCharacter(char c) const {
	if (kMaximumNameLength <= static_cast<int>(_editBuffer.size()))
		return false;
	if (c == ' ' && (_editBuffer.empty() || _editBuffer.lastChar() == ' '))
		return false;

	Common::String prospective = _editBuffer;
	prospective += normalizeCharacter(c);
	if (!_vm->_gfx->hasTextFont(Gfx::TextColor::kDark00) || !_selectionBar || !_selectionBar->isValid())
		return true;
	return _vm->_gfx->getTextWidth(prospective, Gfx::TextColor::kDark00) <= _selectionBar->getWidth() - 5;
}

char InteractiveMenu::SaveFileList::normalizeCharacter(char c) const {
	const bool capitalize = _editBuffer.empty() || _editBuffer.lastChar() == ' ';
	if ('a' <= c && c <= 'z')
		return capitalize ? c - 'a' + 'A' : c;
	if ('A' <= c && c <= 'Z')
		return capitalize ? c : c - 'A' + 'a';
	return c;
}

void InteractiveMenu::SaveFileList::insertItem(int index, const Common::String &name) {
	_items.insert_at(index, name);
}

void InteractiveMenu::SaveFileList::removeItem(int index) {
	if (0 <= index && index < static_cast<int>(_items.size()))
		_items.remove_at(index);
}

void InteractiveMenu::SaveFileList::revealSelection() {
	if (_items.empty()) {
		_selectedIndex = 0;
		_scrollOffset = 0;
		return;
	}

	clampSelection();
	if (_selectedIndex < _scrollOffset || _scrollOffset + kVisibleRows <= _selectedIndex) {
		_scrollOffset = _selectedIndex - kVisibleRows / 2;
		_scrollOffset = MAX(0, _scrollOffset);
		_scrollOffset = MIN(_scrollOffset, MAX(0, static_cast<int>(_items.size()) - kVisibleRows));
	}
}

void InteractiveMenu::SaveFileList::clampSelection() {
	if (_items.empty()) {
		_selectedIndex = 0;
		_scrollOffset = 0;
		return;
	}

	_selectedIndex = CLIP(_selectedIndex, 0, static_cast<int>(_items.size()) - 1);
	_scrollOffset = CLIP(_scrollOffset, 0, MAX(0, static_cast<int>(_items.size()) - kVisibleRows));
}

} // End of namespace Zoombini2
