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
#include "common/fs.h"
#include "common/gui_options.h"
#include "common/language.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/util.h"

#include "gui/ThemeEval.h"
#include "gui/browser.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/scrollcontainer.h"

#include "zoombini2/dialogs.h"
#include "zoombini2/graphics.h"
#include "zoombini2/metaengine.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr uint32 Zoombini2SaveManagementDialog::kEditSavefileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kDuplicateSavefileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kDeleteSavefileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kImportSavefileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kExportSavefileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kSavefileSelectionChangedCommand;
constexpr uint32 Zoombini2OptionsWidget::kManageSavefilesCommand;
constexpr uint32 Zoombini2OptionsWidget::kUnlockFrameRateCommand;

class Zoombini2OptionsWidget::SeparatorWidget : public GUI::Widget {
public:
	SeparatorWidget(GUI::GuiObject *boss, const Common::String &name) : GUI::Widget(boss, name) {
		setFlags(GUI::WIDGET_ENABLED | GUI::WIDGET_CLEARBG);
	}

protected:
	void drawWidget() override {
		g_gui.theme()->drawLineSeparator(Common::Rect(_x, _y, _x + _w, _y + _h));
	}
};

class Zoombini2OptionsWidget::FrameRateNumberBox : public GUI::EditTextWidget {
public:
	FrameRateNumberBox(GUI::GuiObject *boss, const Common::String &name, int value, const Common::U32String &tooltip)
		: GUI::EditTextWidget(boss, name, Common::U32String::format("%d", value), tooltip), _value(value) {}

	void setValue(int value) {
		_value = value;
		setEditString(Common::U32String::format("%d", value));
	}

	int getValue() const {
		const Common::U32String &text = getEditString();
		if (text.empty())
			return _value;
		int value = 0;
		for (uint i = 0; i < text.size(); i++) {
			if (text[i] < '0' || '9' < text[i])
				return _value;
			value = MIN<int>(Zoombini2MetaEngine::kMaxFrameRate, value * 10 + static_cast<int>(text[i] - '0'));
		}
		return CLIP<int>(value, Zoombini2MetaEngine::kMinFrameRate, Zoombini2MetaEngine::kMaxFrameRate);
	}

protected:
	bool isCharAllowed(Common::u32char_type_t character) const override {
		return '0' <= character && character <= '9';
	}

	void lostFocusWidget() override {
		setValue(getValue());
		GUI::EditTextWidget::lostFocusWidget();
	}

private:
	int _value;
};

Zoombini2MenuDialog::Zoombini2MenuDialog(Zoombini2Engine *vm) : MainMenuDialog(vm), _vm(vm) {
}

void Zoombini2MenuDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kOptionsCmd) {
		GUI::ConfigDialog configDialog;
		configDialog.runModal();

		_vm->applyGameSettings();
		_vm->syncSoundSettings();
		return;
	}

	MainMenuDialog::handleCommand(sender, cmd, data);
}

Zoombini2SavefileNameDialog::Zoombini2SavefileNameDialog(const Common::U32String &title, const Common::U32String &initialName, Common::Language language)
	: GUI::Dialog(0, 0, 360, 144) {
	new GUI::StaticTextWidget(this, 12, 10, 336, 24, title, Graphics::kTextAlignStart);
	Common::U32String nameHint;
	if (language == Common::HE_ISR)
		nameHint = _("1-16 letters or spaces; , . ; also allowed");
	else
		nameHint = _("Use 1 to 16 letters, digits, or spaces");
	new GUI::StaticTextWidget(this, 12, 38, 336, 20, false, nameHint, Graphics::kTextAlignStart,
							  Common::U32String(), GUI::ThemeEngine::kFontStyleNormal, Common::UNK_LANG, false);
	_edit = new GUI::EditTextWidget(this, 12, 62, 336, 28, false, initialName);
	new GUI::ButtonWidget(this, 12, 104, 150, 28, false, _("OK"), Common::U32String(), GUI::kOKCmd);
	new GUI::ButtonWidget(this, 198, 104, 150, 28, false, _("Cancel"), Common::U32String(), GUI::kCloseCmd);
}

Common::U32String Zoombini2SavefileNameDialog::getSavefileName() const {
	return _edit->getEditString();
}

void Zoombini2SavefileNameDialog::reflowLayout() {
	const Size32 screenSize(g_system->getOverlayWidth(), g_system->getOverlayHeight());
	_x = MAX<int>(0, (screenSize.width - _w) / 2);
	_y = MAX<int>(0, (screenSize.height - _h) / 2);
	GUI::Dialog::reflowLayout();
}

void Zoombini2SavefileNameDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == GUI::kOKCmd) {
		setResult(GUI::kOKCmd);
		close();
	} else if (cmd == GUI::kCloseCmd) {
		setResult(GUI::kCloseCmd);
		close();
	} else {
		GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

Zoombini2SaveManagementDialog::Zoombini2SaveManagementDialog(const Common::String &domain)
	: GUI::Dialog(0, 0, 1, 1), _domain(domain), _savefileSelectionGroup(this, kSavefileSelectionChangedCommand) {
	_language = Common::parseLanguage(ConfMan.get("language", _domain));
	if (g_engine && ConfMan.getActiveDomainName() == _domain && Common::String(g_engine->getMetaEngine()->getName()) == "zoombini2") {
		const Zoombini2Engine *vm = static_cast<const Zoombini2Engine *>(g_engine);
		_language = vm->getLanguage();
	}

	new GUI::StaticTextWidget(this, kDialogMargin, 8, kTableWidth, 24, true, _("Manage saved games"), Graphics::kTextAlignStart);

	static constexpr int kActionButtonGap = 6;
	static constexpr int kActionButtonCount = 5;
	static constexpr int kActionButtonWidth = (kTableWidth - (kActionButtonCount - 1) * kActionButtonGap) / kActionButtonCount;
	int actionX = kDialogMargin;
	_editButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, _("Rename"),
										_("Return to the launcher before renaming the active saved game."), kEditSavefileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_duplicateButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, _("Clone"),
											 Common::U32String(), kDuplicateSavefileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_importButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, _("Import"),
										  Common::U32String(), kImportSavefileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_exportButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, _("Export"),
										  Common::U32String(), kExportSavefileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_deleteButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kTableWidth - actionX + kDialogMargin, kButtonHeight, true,
										  _("Delete"), _("Return to the launcher before deleting the active saved game."),
										  kDeleteSavefileCommand);

	GUI::ContainerWidget *savefileHeader = new GUI::ContainerWidget(this, kDialogMargin, kHeaderTop, kTableWidth, kTableRowHeight, true);
	savefileHeader->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);
	new GUI::StaticTextWidget(savefileHeader, kNameX, 0, kNameWidth, kTableRowHeight, true, _("Name"), Graphics::kTextAlignCenter);
	// I18N: Name of the Zoombini settlement.
	new GUI::StaticTextWidget(savefileHeader, kZombinivilleX, 0, kZombinivilleWidth, kTableRowHeight, true, _("Zombiniville"),
							  Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(savefileHeader, kRescue1X, 0, kRescueWidth, kTableRowHeight, true, _("Rescue I"), Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(savefileHeader, kRescue2X, 0, kRescueWidth, kTableRowHeight, true, _("Rescue II"), Graphics::kTextAlignCenter);
	// I18N: Name of the Boolie shelter.
	new GUI::StaticTextWidget(savefileHeader, kBooliewoodX, 0, kBooliewoodWidth, kTableRowHeight, true, _("Booliewood"),
							  Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(savefileHeader, kActivePartyX, 0, kActivePartyWidth, kTableRowHeight, true, _("Active"),
							  Graphics::kTextAlignCenter);

	_savefileList = new GUI::ScrollContainerWidget(this, scaleDialogValue(kDialogMargin), scaleDialogValue(kListTop), scaleDialogValue(kTableWidth),
												   scaleDialogValue(kListHeight));
	_savefileTable = new GUI::ContainerWidget(_savefileList, 0, 0, kTableWidth, 0, true);
	const int fontHeight = MAX<int>(1, static_cast<int>(g_gui.getFontHeight() / g_gui.getScaleFactor()));
	const int selectionHeight = MIN<int>(kTableRowHeight, fontHeight);
	const int selectionOffset = (kTableRowHeight - selectionHeight) / 2;
	for (int i = 0; i < kMaximumSavefileRows; i++) {
		const int y = i * kTableRowHeight;
		_savefileSelectionButtons[i] = new GUI::RadiobuttonWidget(_savefileTable, kSelectionX, y + selectionOffset, kSelectionWidth, selectionHeight, true,
																  &_savefileSelectionGroup, i, Common::U32String(), Common::U32String());
		_savefileNameLabels[i] = new GUI::StaticTextWidget(_savefileTable, kNameX, y, kNameWidth, kTableRowHeight, true, Common::U32String(),
														   Graphics::kTextAlignLeft, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_zombinivilleLabels[i] = new GUI::StaticTextWidget(_savefileTable, kZombinivilleX, y, kZombinivilleWidth, kTableRowHeight, true,
														   Common::U32String(), Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_rescue1Labels[i] = new GUI::StaticTextWidget(_savefileTable, kRescue1X, y, kRescueWidth, kTableRowHeight, true, Common::U32String(),
													  Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_rescue2Labels[i] = new GUI::StaticTextWidget(_savefileTable, kRescue2X, y, kRescueWidth, kTableRowHeight, true, Common::U32String(),
													  Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_booliewoodLabels[i] = new GUI::StaticTextWidget(_savefileTable, kBooliewoodX, y, kBooliewoodWidth, kTableRowHeight, true, Common::U32String(),
														 Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_activePartyLabels[i] = new GUI::StaticTextWidget(_savefileTable, kActivePartyX, y, kActivePartyWidth, kTableRowHeight, true, Common::U32String(),
														  Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	}

	new GUI::ButtonWidget(this, kDialogMargin + (kTableWidth - 120) / 2, kBottomButtonsTop, 120, kButtonHeight, true, _("Close"),
						  Common::U32String(), GUI::kCloseCmd, Common::ASCII_ESCAPE);
}

void Zoombini2SaveManagementDialog::open() {
	GUI::Dialog::open();
	refreshSavefiles();
	_savefileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void Zoombini2SaveManagementDialog::reflowLayout() {
	const int scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", 16);
	const Size32 dialogSize(scaleDialogValue(kTableWidth + 2 * kDialogMargin) + scrollbarWidth, scaleDialogValue(kDialogHeight));
	const Size32 screenSize(g_system->getOverlayWidth(), g_system->getOverlayHeight());
	_x = MAX<int>(0, (screenSize.width - dialogSize.width) / 2);
	_y = MAX<int>(0, (screenSize.height - dialogSize.height) / 2);
	_w = dialogSize.width;
	_h = dialogSize.height;
	GUI::Dialog::reflowLayout();
}

void Zoombini2SaveManagementDialog::refreshSavefiles(const Common::String &selectedSavefile) {
	Common::String savefileNameToSelect = selectedSavefile;
	if (savefileNameToSelect.empty() && 0 <= _selectedSavefileIndex && _selectedSavefileIndex < static_cast<int>(_savefileNames.size()))
		savefileNameToSelect = _savefileNames[_selectedSavefileIndex];

	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	const Common::Array<Zoombini2SavefileSummary> summaries = savegameManager.listSavefileSummaries();
	_savefileNames.clear();
	_savefileRowCount = MIN<int>(summaries.size(), kMaximumSavefileRows);
	_selectedSavefileIndex = -1;
	for (int i = 0; i < _savefileRowCount; i++) {
		_savefileNames.push_back(summaries[i]._savefileName);
		if (!savefileNameToSelect.empty() && summaries[i]._savefileName.equalsIgnoreCase(savefileNameToSelect))
			_selectedSavefileIndex = i;
	}
	if (_selectedSavefileIndex < 0 && 0 < _savefileRowCount)
		_selectedSavefileIndex = 0;

	for (int i = 0; i < kMaximumSavefileRows; i++) {
		const bool visible = i < _savefileRowCount;
		_savefileSelectionButtons[i]->setVisible(visible);
		_savefileNameLabels[i]->setVisible(visible);
		_zombinivilleLabels[i]->setVisible(visible);
		_rescue1Labels[i]->setVisible(visible);
		_rescue2Labels[i]->setVisible(visible);
		_booliewoodLabels[i]->setVisible(visible);
		_activePartyLabels[i]->setVisible(visible);
		_savefileStateValid[i] = visible && summaries[i]._stateValid;
		if (!visible)
			continue;

		const Zoombini2SavefileSummary &summary = summaries[i];
		_savefileNameLabels[i]->setLabel(savegameManager.decodeSavefileName(summary._savefileName));
		if (summary._stateValid) {
			_zombinivilleLabels[i]->setLabel(Common::U32String::format("%d", summary._population._zombinivilleCount));
			_rescue1Labels[i]->setLabel(Common::U32String::format("%d", summary._population._rescue1Count));
			_rescue2Labels[i]->setLabel(Common::U32String::format("%d", summary._population._rescue2Count));
			_booliewoodLabels[i]->setLabel(Common::U32String::format("%d", summary._population._booliewoodCount));
			_activePartyLabels[i]->setLabel(Common::U32String::format("%d", summary._population._activePartyCount));
		} else {
			// I18N: Placeholder for a population count in an invalid saved game.
			_zombinivilleLabels[i]->setLabel(_("?"));
			_rescue1Labels[i]->setLabel(_("?"));
			_rescue2Labels[i]->setLabel(_("?"));
			_booliewoodLabels[i]->setLabel(_("?"));
			_activePartyLabels[i]->setLabel(_("?"));
		}

		GUI::ThemeEngine::FontColor fontColor = GUI::ThemeEngine::kFontColorOverride;
		if (summary._stateValid)
			fontColor = GUI::ThemeEngine::kFontColorNormal;
		GUI::StaticTextWidget *rowLabels[] = {
			_savefileNameLabels[i],
			_zombinivilleLabels[i],
			_rescue1Labels[i],
			_rescue2Labels[i],
			_booliewoodLabels[i],
			_activePartyLabels[i],
		};
		for (uint labelIndex = 0; labelIndex < ARRAYSIZE(rowLabels); labelIndex++) {
			rowLabels[labelIndex]->setFontColor(fontColor);
			rowLabels[labelIndex]->markAsDirty();
		}
	}

	_savefileSelectionGroup.setValue(_selectedSavefileIndex);
	updateSavefileTableLayout();
	updateButtons();
}

void Zoombini2SaveManagementDialog::updateSavefileTableLayout() {
	_savefileTable->setSize(scaleDialogValue(kTableWidth), scaleDialogValue(_savefileRowCount * kTableRowHeight));
}

void Zoombini2SaveManagementDialog::updateButtons() {
	const bool hasSelection = 0 <= _selectedSavefileIndex && _selectedSavefileIndex < _savefileRowCount;
	const bool stateValid = hasSelection && _savefileStateValid[_selectedSavefileIndex];
	const bool activeSavefile = hasSelection && isActiveSavefile(_savefileNames[_selectedSavefileIndex]);
	_editButton->setEnabled(stateValid && !activeSavefile);
	_duplicateButton->setEnabled(stateValid && _savefileRowCount < kMaximumSavefileRows);
	_importButton->setEnabled(true);
	_exportButton->setEnabled(stateValid);
	_deleteButton->setEnabled(hasSelection && !activeSavefile);
}

bool Zoombini2SaveManagementDialog::isActiveSavefile(const Common::String &savefileName) const {
	if (!g_engine || ConfMan.getActiveDomainName() != _domain || Common::String(g_engine->getMetaEngine()->getName()) != "zoombini2")
		return false;
	const Zoombini2Engine *vm = static_cast<const Zoombini2Engine *>(g_engine);
	return vm->isActiveGameSave(savefileName);
}

void Zoombini2SaveManagementDialog::renameSelectedSavefile() {
	if (_selectedSavefileIndex < 0 || static_cast<int>(_savefileNames.size()) <= _selectedSavefileIndex || !_savefileStateValid[_selectedSavefileIndex])
		return;

	const Common::String oldSavefileName = _savefileNames[_selectedSavefileIndex];
	if (isActiveSavefile(oldSavefileName))
		return;
	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	Zoombini2SavefileNameDialog nameDialog(_("Rename saved game"), savegameManager.decodeSavefileName(oldSavefileName), _language);
	if (nameDialog.runModal() != GUI::kOKCmd)
		return;

	Common::String newSavefileName;
	if (!savegameManager.encodeSavefileName(nameDialog.getSavefileName(), newSavefileName) ||
		!savegameManager.renameSavefile(oldSavefileName, newSavefileName)) {
		GUI::MessageDialog errorDialog(_("Invalid file name for saving"));
		errorDialog.runModal();
		return;
	}
	refreshSavefiles(newSavefileName);
	_savefileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void Zoombini2SaveManagementDialog::duplicateSelectedSavefile() {
	if (_selectedSavefileIndex < 0 || static_cast<int>(_savefileNames.size()) <= _selectedSavefileIndex || !_savefileStateValid[_selectedSavefileIndex] ||
		kMaximumSavefileRows <= _savefileRowCount)
		return;

	const Common::String srcSavefileName = _savefileNames[_selectedSavefileIndex];
	Zoombini2SavefileNameDialog nameDialog(_("Clone saved game"), Common::U32String(), _language);
	if (nameDialog.runModal() != GUI::kOKCmd)
		return;

	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	Common::String newSavefileName;
	if (!savegameManager.encodeSavefileName(nameDialog.getSavefileName(), newSavefileName) ||
		!savegameManager.duplicateSavefile(srcSavefileName, newSavefileName)) {
		GUI::MessageDialog errorDialog(_("Enter a unique name using the allowed characters for this language"));
		errorDialog.runModal();
		return;
	}
	refreshSavefiles(newSavefileName);
	_savefileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void Zoombini2SaveManagementDialog::importSavefile() {
	GUI::BrowserDialog browser(_("Select one Zoombini2 .mk saved game"), false);
	if (browser.runModal() <= 0)
		return;

	const Common::FSNode src = browser.getResult();
	const Common::String srcName = src.getName();
	const size_t extensionPosition = srcName.findLastOf('.');
	if (!src.exists() || src.isDirectory() || extensionPosition == Common::String::npos ||
		!srcName.substr(extensionPosition).equalsIgnoreCase(".mk")) {
		GUI::MessageDialog errorDialog(_("Select one Zoombini2 .mk saved game"));
		errorDialog.runModal();
		return;
	}

	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	Common::String savefileName;
	Common::U32String displayName = srcName.substr(0, extensionPosition).decode(Common::kUtf8);
	bool validName = savegameManager.encodeSavefileName(displayName, savefileName) &&
					 Zoombini2SavegameManager::isValidSavefileName(savefileName, _language);
	while (!validName) {
		Zoombini2SavefileNameDialog nameDialog(_("Choose compatible save name"), displayName, _language);
		if (nameDialog.runModal() != GUI::kOKCmd)
			return;
		displayName = nameDialog.getSavefileName();
		validName = savegameManager.encodeSavefileName(displayName, savefileName) &&
					Zoombini2SavegameManager::isValidSavefileName(savefileName, _language);
		if (!validName) {
			GUI::MessageDialog errorDialog(_("The name contains characters unavailable in this language"));
			errorDialog.runModal();
		}
	}

	if (isActiveSavefile(savefileName)) {
		GUI::MessageDialog errorDialog(_("Return to the launcher before replacing the active saved game."));
		errorDialog.runModal();
		return;
	}
	if (savegameManager.savefileExists(savefileName)) {
		GUI::MessageDialog confirmation(_("A saved game with this name already exists. Replace it?"),
										_("Replace"), _("Cancel"));
		if (confirmation.runModal() != GUI::kMessageOK)
			return;
	}

	Common::SeekableReadStream *stream = src.createReadStream();
	const bool imported = savegameManager.importSavefile(savefileName, stream, true);
	delete stream;
	if (!imported) {
		GUI::MessageDialog errorDialog(_("The selected file is not a valid Zoombini2 saved game"));
		errorDialog.runModal();
		return;
	}

	refreshSavefiles(savefileName);
	_savefileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
	GUI::MessageDialog successDialog(_("The saved game was imported"));
	successDialog.runModal();
}

void Zoombini2SaveManagementDialog::exportSelectedSavefile() {
	if (_selectedSavefileIndex < 0 || static_cast<int>(_savefileNames.size()) <= _selectedSavefileIndex || !_savefileStateValid[_selectedSavefileIndex])
		return;

	GUI::BrowserDialog browser(_("Select the directory for the exported Zoombini2 .mk save"), true);
	if (browser.runModal() <= 0)
		return;

	const Common::FSNode directory = browser.getResult();
	if (!directory.isDirectory()) {
		GUI::MessageDialog errorDialog(_("The selected destination is not a directory"));
		errorDialog.runModal();
		return;
	}

	const Common::String savefileName = _savefileNames[_selectedSavefileIndex];
	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	const Common::String exportName = savegameManager.decodeSavefileName(savefileName).encode(Common::kUtf8) + ".mk";
	const Common::FSNode dest = findChild(directory, exportName);
	if (dest.exists()) {
		GUI::MessageDialog confirmation(_("The .mk file already exists. Replace it?"),
										_("Replace"), _("Cancel"));
		if (confirmation.runModal() != GUI::kMessageOK)
			return;
	}

	Common::SeekableWriteStream *stream = dest.createWriteStream(false);
	bool exported = stream && savegameManager.exportSavefile(savefileName, stream);
	if (stream) {
		stream->finalize();
		exported = exported && !stream->err();
	}
	delete stream;
	if (!exported) {
		GUI::MessageDialog errorDialog(_("Unable to export the selected saved game"));
		errorDialog.runModal();
		return;
	}

	GUI::MessageDialog successDialog(_("The saved game was exported as a Zoombini2 .mk file"));
	successDialog.runModal();
}

void Zoombini2SaveManagementDialog::deleteSelectedSavefile() {
	if (_selectedSavefileIndex < 0 || static_cast<int>(_savefileNames.size()) <= _selectedSavefileIndex)
		return;
	const Common::String savefileName = _savefileNames[_selectedSavefileIndex];
	if (isActiveSavefile(savefileName))
		return;

	GUI::MessageDialog confirmation(_("Do you really want to delete this saved game?"), _("Delete"),
									_("Cancel"));
	if (confirmation.runModal() != GUI::kMessageOK)
		return;

	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	if (!savegameManager.deleteSavefile(savefileName)) {
		GUI::MessageDialog errorDialog(_("Error deleting saved game"));
		errorDialog.runModal();
		return;
	}
	refreshSavefiles();
	_savefileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

Common::FSNode Zoombini2SaveManagementDialog::findChild(const Common::FSNode &directory, const Common::String &name) {
	Common::FSNode direct = directory.getChild(name);
	if (direct.exists())
		return direct;

	Common::FSList children;
	if (directory.getChildren(children, Common::FSNode::kListFilesOnly)) {
		for (Common::FSList::const_iterator child = children.begin(); child != children.end(); child++) {
			if (child->getName().equalsIgnoreCase(name))
				return *child;
		}
	}
	return direct;
}

int Zoombini2SaveManagementDialog::scaleDialogValue(int value) {
	return 0 < value ? static_cast<int>(value * g_gui.getScaleFactor()) : value;
}

void Zoombini2SaveManagementDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSavefileSelectionChangedCommand:
		if (data < static_cast<uint32>(_savefileRowCount))
			_selectedSavefileIndex = static_cast<int>(data);
		else
			_selectedSavefileIndex = -1;
		updateButtons();
		break;
	case kEditSavefileCommand:
		renameSelectedSavefile();
		break;
	case kDuplicateSavefileCommand:
		duplicateSelectedSavefile();
		break;
	case kImportSavefileCommand:
		importSavefile();
		break;
	case kExportSavefileCommand:
		exportSelectedSavefile();
		break;
	case kDeleteSavefileCommand:
		deleteSelectedSavefile();
		break;
	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
		break;
	}
}

Zoombini2OptionsWidget::Zoombini2OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain)
	: GUI::OptionsContainerWidget(boss, name, "Zoombini2EngineOptionsDialog", domain) {
	new SeparatorWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.SaveFilesSeparator");
	GUI::StaticTextWidget *header = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.SaveFilesHeader",
															  _("Save management"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	header->setAlign(Graphics::TextAlign::kTextAlignStart);
	GUI::ButtonWidget *manageSavefilesButton = new GUI::ButtonWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.ManageSavefiles",
																	 _("Saved games"), Common::U32String(), kManageSavefilesCommand);
	manageSavefilesButton->setTarget(this);
	_savefileReadOnlyToggleCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.SavefileReadOnlyToggle",
															  _("Enable savefile readonly toggle (Ctrl-K)"),
															  _("Ctrl+K or right-click toggles automatic save writes for one savefile during this game session. "
																"A file without write permission remains read-only."));

	new SeparatorWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayEnhancementsSeparator");
	header = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayEnhancements",
									   _("Gameplay Enhancements"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	header->setAlign(Graphics::TextAlign::kTextAlignStart);
	_stereoOutputCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.StereoOutput", _("Enable stereo game audio"),
													_("Keeps both channels of stereo WAV resources instead of downmixing them to mono."));
	_floatingPointPathsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.FloatingPointPaths",
														  _("Use floating-point path calculations"),
														  _("Uses 32-bit floating point instead of the original signed Q10 fixed-point arithmetic for Bezier movement paths."));
	_fixFleenDepartureStreakCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.FixFleenDepartureStreak",
															   _("Remove stray streaks from fleeing Fleens"),
															   _("Hides stray brown streaks in some Fleen departure frames in Magic Mirrors."));
	_enhancedKbdShortcutsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.EnhancedKbdShortcuts",
															_("Enable enhanced keyboard shortcuts"),
															_("Enables some ScummVM-only keyboard shortcuts for quality of life improvements."));
	if (Common::checkGameGUIOption(GAMEOPTION_HELP_PAGE_COLOR_KEYING, ConfMan.get("guioptions", domain))) {
		_transparentHelpPagesCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.TransparentHelpPages",
																_("Remove solid color behind help text"),
																_("Uses the help sheet's corner color as a transparency key for affected releases."));
	}
	_colorAssistLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.ColorAssistLabel",
												  _("Color assistance:"));
	_colorAssistLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_colorAssistPopUp = new GUI::PopUpWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.ColorAssist",
											 _("Adjusts Zoombini noses and puzzle colors."));
	_colorAssistPopUp->appendEntry(_("Original colors"), 0);
	_colorAssistPopUp->appendEntry(_("Small screen"), 1);
	_colorAssistPopUp->appendEntry(_("Red-green color assist"), 2);

	new SeparatorWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayAdjustmentSeparator");
	header = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayAdjustment",
									   _("Gameplay Adjustment"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	header->setAlign(Graphics::TextAlign::kTextAlignStart);
	_debugHotkeysCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.DebugHotkeys", _("Enable developer hotkeys"),
													_("Enables F2/F3 party exchange, P puzzle completion, and the Chez Norf C overlay."));
	_greedyWaterslideCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GreedyWaterslide",
														_("Use alternate Pipes of Paloo pairing"),
														_("Selects the alternate pairing branch for level one."));
	_aquacubeSafeFirstMoveCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.AquacubeSafeFirstMove",
															 _("Protect Aqua Cube's first lever move"),
															 _("On level three, swaps the hidden axes of two levers if the first direct lever press would enter a Fleen cell."));
	_allowCutLevel4PracticePuzzlesCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.AllowCutLevel4PracticePuzzles",
																	 _("Allow cut level 4 puzzles in practice mode"),
																	 _("Adds a level 4 practice-map tab. Puzzles without a recovered level 4 remain at level three."));
	const Common::U32String prngAlgorithmTooltip = _("Selects the pseudo random number generator algorithm.");
	_prngAlgorithmLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.PrngAlgorithmLabel", _("PRNG Algorithm:"), prngAlgorithmTooltip);
	_prngAlgorithmLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_prngAlgorithmPopUp = new GUI::PopUpWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.PrngAlgorithm", prngAlgorithmTooltip);
	_prngAlgorithmPopUp->appendEntry(_("Original PRNG"), static_cast<uint32>(Zoombini2MetaEngine::PrngAlgorithm::kOriginalPrng));
	_prngAlgorithmPopUp->appendEntry(_("ScummVM Standard PRNG"), static_cast<uint32>(Zoombini2MetaEngine::PrngAlgorithm::kStandardPrng));
	_frameRateLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.FrameRateLabel", _("Frame rate (FPS):"));
	_frameRateLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_frameRateNumberBox = new FrameRateNumberBox(widgetsBoss(), "Zoombini2EngineOptionsDialog.FrameRate",
												 Zoombini2MetaEngine::kDefaultFrameRate,
												 _("Enter a whole number from 30 to 240 FPS."));
	_pacingLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.PacingLabel", _("Logic pacing:"));
	_pacingLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_pacingPopUp = new GUI::PopUpWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.Pacing",
										_("Select 60Hz LCD or 75Hz CRT logic pacing. "
										  "Affects Booliewood panorama scrolling speed and idle animation trigger rates. "
										  "Rendering frame rate is unaffected."));
	_pacingPopUp->appendEntry(_("60Hz (LCD preset)"), static_cast<uint32>(Zoombini2MetaEngine::LogicPacingMode::k60Hz));
	_pacingPopUp->appendEntry(_("75Hz (CRT preset)"), static_cast<uint32>(Zoombini2MetaEngine::LogicPacingMode::k75Hz));
	_unlockFrameRateCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.UnlockFrameRate",
													   _("Unlock frame rate"),
													   _("Removes the engine frame-rate limit. Display VSync may still limit presentation."),
													   kUnlockFrameRateCommand);
	_unlockFrameRateCheckbox->setTarget(this);
}

void Zoombini2OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	const int lineHeight = layouts.getVar("Globals.Line.Height");
	static constexpr int kOptionLabelPadding = 4;
	const GUI::StaticTextWidget *const optionLabels[] = {
		_colorAssistLabel,
		_prngAlgorithmLabel,
		_pacingLabel,
		_frameRateLabel,
	};
	int optionLabelWidth = 0;
	for (uint labelIdx = 0; labelIdx < ARRAYSIZE(optionLabels); labelIdx++)
		optionLabelWidth = MAX(optionLabelWidth, g_gui.getStringWidth(optionLabels[labelIdx]->getLabel()));
	optionLabelWidth += kOptionLabelPadding;
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical)
		.addPadding(0, 0, 0, 0)
		.addSpace(10)
		.addWidget("SaveFilesSeparator", "", -1, 2)
		.addWidget("SaveFilesHeader", "", -1, lineHeight)
		.addWidget("ManageSavefiles", "Button")
		.addWidget("SavefileReadOnlyToggle", "Checkbox")
		.addSpace(10)
		.addWidget("GameplayEnhancementsSeparator", "", -1, 2)
		.addWidget("GameplayEnhancements", "", -1, lineHeight)
		.addWidget("StereoOutput", "Checkbox")
		.addWidget("FloatingPointPaths", "Checkbox")
		.addWidget("FixFleenDepartureStreak", "Checkbox")
		.addWidget("EnhancedKbdShortcuts", "Checkbox")
		.addWidget("TransparentHelpPages", "Checkbox")
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("ColorAssistLabel", "", optionLabelWidth, lineHeight)
		.addWidget("ColorAssist", "", 210, lineHeight)
		.closeLayout()
		.addSpace(10)
		.addWidget("GameplayAdjustmentSeparator", "", -1, 2)
		.addWidget("GameplayAdjustment", "", -1, lineHeight)
		.addWidget("DebugHotkeys", "Checkbox")
		.addWidget("GreedyWaterslide", "Checkbox")
		.addWidget("AquacubeSafeFirstMove", "Checkbox")
		.addWidget("AllowCutLevel4PracticePuzzles", "Checkbox")
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("PrngAlgorithmLabel", "", optionLabelWidth, lineHeight)
		.addWidget("PrngAlgorithm", "PopUp")
		.closeLayout()
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("PacingLabel", "", optionLabelWidth, lineHeight)
		.addWidget("Pacing", "", 120, lineHeight)
		.closeLayout()
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("FrameRateLabel", "", optionLabelWidth, lineHeight)
		.addWidget("FrameRate", "", 64, lineHeight)
		.closeLayout()
		.addWidget("UnlockFrameRate", "Checkbox")
		.closeLayout()
		.closeDialog();
}

void Zoombini2OptionsWidget::load() {
	_savefileReadOnlyToggleCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigEnableSavefileReadOnlyToggle, _domain));
	_stereoOutputCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigStereoOutput, _domain));
	_floatingPointPathsCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigUseFloatingPointPaths, _domain));
	_fixFleenDepartureStreakCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigFixFleenDepartureStreak, _domain));
	_enhancedKbdShortcutsCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigEnhancedKbdShortcuts, _domain));
	if (_transparentHelpPagesCheckbox)
		_transparentHelpPagesCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigTransparentHelpPages, _domain));
	const int colorAssistValue = ConfMan.getInt(Zoombini2MetaEngine::kConfigColorAssistMode, _domain);
	_colorAssistPopUp->setSelectedTag(0 <= colorAssistValue && colorAssistValue <= 2 ? colorAssistValue : 0);
	_debugHotkeysCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigDebugHotkeys, _domain));
	_greedyWaterslideCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigGreedyWaterslidePairing, _domain));
	_aquacubeSafeFirstMoveCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigAquacubeSafeFirstMove, _domain));
	_allowCutLevel4PracticePuzzlesCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigAllowCutLevel4PracticePuzzles, _domain));
	_prngAlgorithmPopUp->setSelectedTag(ConfMan.getInt(Zoombini2MetaEngine::kConfigPrngAlgorithm, _domain));
	_pacingPopUp->setSelectedTag(ConfMan.getInt(Zoombini2MetaEngine::kConfigLogicPacingMode, _domain));
	const int frameRate = CLIP<int>(ConfMan.getInt(Zoombini2MetaEngine::kConfigFrameRate, _domain), Zoombini2MetaEngine::kMinFrameRate, Zoombini2MetaEngine::kMaxFrameRate);
	_frameRateNumberBox->setValue(frameRate);
	_unlockFrameRateCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigUnlockFrameRate, _domain));
	updateFrameRateControls();
}

bool Zoombini2OptionsWidget::save() {
	const int frameRate = _frameRateNumberBox->getValue();
	_frameRateNumberBox->setValue(frameRate);

	const Zoombini2MetaEngine::PrngAlgorithm prngAlgorithm = static_cast<Zoombini2MetaEngine::PrngAlgorithm>(_prngAlgorithmPopUp->getSelectedTag());
	const Zoombini2MetaEngine::LogicPacingMode logicPacingMode = static_cast<Zoombini2MetaEngine::LogicPacingMode>(_pacingPopUp->getSelectedTag());

	ConfMan.setBool(Zoombini2MetaEngine::kConfigEnableSavefileReadOnlyToggle, _savefileReadOnlyToggleCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigStereoOutput, _stereoOutputCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigUseFloatingPointPaths, _floatingPointPathsCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigFixFleenDepartureStreak, _fixFleenDepartureStreakCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigEnhancedKbdShortcuts, _enhancedKbdShortcutsCheckbox->getState(), _domain);
	if (_transparentHelpPagesCheckbox)
		ConfMan.setBool(Zoombini2MetaEngine::kConfigTransparentHelpPages, _transparentHelpPagesCheckbox->getState(), _domain);
	ConfMan.setInt(Zoombini2MetaEngine::kConfigColorAssistMode, _colorAssistPopUp->getSelectedTag(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigDebugHotkeys, _debugHotkeysCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigGreedyWaterslidePairing, _greedyWaterslideCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigAquacubeSafeFirstMove, _aquacubeSafeFirstMoveCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigAllowCutLevel4PracticePuzzles, _allowCutLevel4PracticePuzzlesCheckbox->getState(), _domain);
	ConfMan.setInt(Zoombini2MetaEngine::kConfigPrngAlgorithm, static_cast<int>(prngAlgorithm), _domain);
	ConfMan.setInt(Zoombini2MetaEngine::kConfigLogicPacingMode, static_cast<int>(logicPacingMode), _domain);
	ConfMan.setInt(Zoombini2MetaEngine::kConfigFrameRate, frameRate, _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigUnlockFrameRate, _unlockFrameRateCheckbox->getState(), _domain);
	return true;
}

void Zoombini2OptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kUnlockFrameRateCommand) {
		updateFrameRateControls();
		return;
	}
	if (cmd == kManageSavefilesCommand) {
		Zoombini2SaveManagementDialog dialog(_domain);
		dialog.runModal();
		return;
	}
	GUI::OptionsContainerWidget::handleCommand(sender, cmd, data);
}

void Zoombini2OptionsWidget::updateFrameRateControls() {
	const bool unlocked = _unlockFrameRateCheckbox->getState();
	_frameRateNumberBox->setEnabled(!unlocked);
}

} // End of namespace Zoombini2
