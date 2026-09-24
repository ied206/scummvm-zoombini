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

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/gui_options.h"
#include "common/language.h"
#include "common/savefile.h"
#include "common/system.h"
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

constexpr uint32 Zoombini2SaveManagementDialog::kEditProfileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kDuplicateProfileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kDeleteProfileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kImportProfileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kExportProfileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kProfileSelectionChangedCommand;
constexpr uint32 Zoombini2OptionsWidget::kManageProfilesCommand;
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
			value = MIN<int>(::Zoombini2MetaEngine::kMaxFrameRate, value * 10 + static_cast<int>(text[i] - '0'));
		}
		return CLIP<int>(value, ::Zoombini2MetaEngine::kMinFrameRate, ::Zoombini2MetaEngine::kMaxFrameRate);
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

Zoombini2ProfileNameDialog::Zoombini2ProfileNameDialog(const Common::U32String &title, const Common::U32String &initialName)
	: GUI::Dialog(0, 0, 360, 144) {
	new GUI::StaticTextWidget(this, 12, 10, 336, 24, title, Graphics::kTextAlignStart);
	new GUI::StaticTextWidget(this, 12, 38, 336, 20, false, Common::U32String("Use 1 to 16 letters, digits, or spaces"), Graphics::kTextAlignStart,
							  Common::U32String(), GUI::ThemeEngine::kFontStyleNormal, Common::UNK_LANG, false);
	_edit = new GUI::EditTextWidget(this, 12, 62, 336, 28, false, initialName);
	new GUI::ButtonWidget(this, 12, 104, 150, 28, false, Common::U32String("OK"), Common::U32String(), GUI::kOKCmd);
	new GUI::ButtonWidget(this, 198, 104, 150, 28, false, Common::U32String("Cancel"), Common::U32String(), GUI::kCloseCmd);
}

Common::U32String Zoombini2ProfileNameDialog::getProfileName() const {
	return _edit->getEditString();
}

void Zoombini2ProfileNameDialog::reflowLayout() {
	const Size32 screenSize(g_system->getOverlayWidth(), g_system->getOverlayHeight());
	_x = MAX<int>(0, (screenSize.width - _w) / 2);
	_y = MAX<int>(0, (screenSize.height - _h) / 2);
	GUI::Dialog::reflowLayout();
}

void Zoombini2ProfileNameDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
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
	: GUI::Dialog(0, 0, 1, 1), _domain(domain), _profileSelectionGroup(this, kProfileSelectionChangedCommand) {

	new GUI::StaticTextWidget(this, kDialogMargin, 8, kTableWidth, 24, true, Common::U32String("Manage saved games"), Graphics::kTextAlignStart);

	static constexpr int kActionButtonGap = 6;
	static constexpr int kActionButtonCount = 5;
	static constexpr int kActionButtonWidth = (kTableWidth - (kActionButtonCount - 1) * kActionButtonGap) / kActionButtonCount;
	int actionX = kDialogMargin;
	_editButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, Common::U32String("Rename"),
										Common::U32String(), kEditProfileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_duplicateButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, Common::U32String("Clone"),
											 Common::U32String(), kDuplicateProfileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_importButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, Common::U32String("Import"),
										  Common::U32String(), kImportProfileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_exportButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, Common::U32String("Export"),
										  Common::U32String(), kExportProfileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_deleteButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kTableWidth - actionX + kDialogMargin, kButtonHeight, true,
										  Common::U32String("Delete"), Common::U32String(), kDeleteProfileCommand);

	GUI::ContainerWidget *profileHeader = new GUI::ContainerWidget(this, kDialogMargin, kHeaderTop, kTableWidth, kTableRowHeight, true);
	profileHeader->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);
	new GUI::StaticTextWidget(profileHeader, kNameX, 0, kNameWidth, kTableRowHeight, true, Common::U32String("Name"), Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(profileHeader, kZombinivilleX, 0, kZombinivilleWidth, kTableRowHeight, true, Common::U32String("Zombiniville"),
							  Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(profileHeader, kRescue1X, 0, kRescueWidth, kTableRowHeight, true, Common::U32String("Rescue I"), Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(profileHeader, kRescue2X, 0, kRescueWidth, kTableRowHeight, true, Common::U32String("Rescue II"), Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(profileHeader, kBooliewoodX, 0, kBooliewoodWidth, kTableRowHeight, true, Common::U32String("Booliewood"),
							  Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(profileHeader, kActivePartyX, 0, kActivePartyWidth, kTableRowHeight, true, Common::U32String("Active"),
							  Graphics::kTextAlignCenter);

	_profileList = new GUI::ScrollContainerWidget(this, scaleDialogValue(kDialogMargin), scaleDialogValue(kListTop), scaleDialogValue(kTableWidth),
												  scaleDialogValue(kListHeight));
	_profileTable = new GUI::ContainerWidget(_profileList, 0, 0, kTableWidth, 0, true);
	const int fontHeight = MAX<int>(1, static_cast<int>(g_gui.getFontHeight() / g_gui.getScaleFactor()));
	const int selectionHeight = MIN<int>(kTableRowHeight, fontHeight);
	const int selectionOffset = (kTableRowHeight - selectionHeight) / 2;
	for (int i = 0; i < kMaximumProfileRows; i++) {
		const int y = i * kTableRowHeight;
		_profileSelectionButtons[i] = new GUI::RadiobuttonWidget(_profileTable, kSelectionX, y + selectionOffset, kSelectionWidth, selectionHeight, true,
																 &_profileSelectionGroup, i, Common::U32String(), Common::U32String());
		_profileNameLabels[i] = new GUI::StaticTextWidget(_profileTable, kNameX, y, kNameWidth, kTableRowHeight, true, Common::U32String(),
														  Graphics::kTextAlignLeft, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_zombinivilleLabels[i] = new GUI::StaticTextWidget(_profileTable, kZombinivilleX, y, kZombinivilleWidth, kTableRowHeight, true,
														   Common::U32String(), Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_rescue1Labels[i] = new GUI::StaticTextWidget(_profileTable, kRescue1X, y, kRescueWidth, kTableRowHeight, true, Common::U32String(),
													  Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_rescue2Labels[i] = new GUI::StaticTextWidget(_profileTable, kRescue2X, y, kRescueWidth, kTableRowHeight, true, Common::U32String(),
													  Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_booliewoodLabels[i] = new GUI::StaticTextWidget(_profileTable, kBooliewoodX, y, kBooliewoodWidth, kTableRowHeight, true, Common::U32String(),
														 Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_activePartyLabels[i] = new GUI::StaticTextWidget(_profileTable, kActivePartyX, y, kActivePartyWidth, kTableRowHeight, true, Common::U32String(),
														  Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	}

	new GUI::ButtonWidget(this, kDialogMargin + (kTableWidth - 120) / 2, kBottomButtonsTop, 120, kButtonHeight, true, Common::U32String("Close"),
						  Common::U32String(), GUI::kCloseCmd, Common::ASCII_ESCAPE);
}

void Zoombini2SaveManagementDialog::open() {
	GUI::Dialog::open();
	refreshProfiles();
	_profileList->reflowLayout();
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

void Zoombini2SaveManagementDialog::refreshProfiles(const Common::String &selectedProfile) {
	Common::String profileToSelect = selectedProfile;
	if (profileToSelect.empty() && 0 <= _selectedProfileIndex && _selectedProfileIndex < static_cast<int>(_profileNames.size()))
		profileToSelect = _profileNames[_selectedProfileIndex];

	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain);
	const Common::Array<Zoombini2ProfileSummary> summaries = savegameManager.listProfileSummaries();
	_profileNames.clear();
	_profileRowCount = MIN<int>(summaries.size(), kMaximumProfileRows);
	_selectedProfileIndex = -1;
	for (int i = 0; i < _profileRowCount; i++) {
		_profileNames.push_back(summaries[i]._profileName);
		if (!profileToSelect.empty() && summaries[i]._profileName.equalsIgnoreCase(profileToSelect))
			_selectedProfileIndex = i;
	}
	if (_selectedProfileIndex < 0 && 0 < _profileRowCount)
		_selectedProfileIndex = 0;

	for (int i = 0; i < kMaximumProfileRows; i++) {
		const bool visible = i < _profileRowCount;
		_profileSelectionButtons[i]->setVisible(visible);
		_profileNameLabels[i]->setVisible(visible);
		_zombinivilleLabels[i]->setVisible(visible);
		_rescue1Labels[i]->setVisible(visible);
		_rescue2Labels[i]->setVisible(visible);
		_booliewoodLabels[i]->setVisible(visible);
		_activePartyLabels[i]->setVisible(visible);
		_profileStateValid[i] = visible && summaries[i]._stateValid;
		if (!visible)
			continue;

		const Zoombini2ProfileSummary &summary = summaries[i];
		_profileNameLabels[i]->setLabel(Common::U32String(summary._profileName.c_str()));
		if (summary._stateValid) {
			_zombinivilleLabels[i]->setLabel(Common::U32String::format("%d", summary._population._zombinivilleCount));
			_rescue1Labels[i]->setLabel(Common::U32String::format("%d", summary._population._rescue1Count));
			_rescue2Labels[i]->setLabel(Common::U32String::format("%d", summary._population._rescue2Count));
			_booliewoodLabels[i]->setLabel(Common::U32String::format("%d", summary._population._booliewoodCount));
			_activePartyLabels[i]->setLabel(Common::U32String::format("%d", summary._population._activePartyCount));
		} else {
			_zombinivilleLabels[i]->setLabel(Common::U32String("?"));
			_rescue1Labels[i]->setLabel(Common::U32String("?"));
			_rescue2Labels[i]->setLabel(Common::U32String("?"));
			_booliewoodLabels[i]->setLabel(Common::U32String("?"));
			_activePartyLabels[i]->setLabel(Common::U32String("?"));
		}

		GUI::ThemeEngine::FontColor fontColor = GUI::ThemeEngine::kFontColorOverride;
		if (summary._stateValid)
			fontColor = GUI::ThemeEngine::kFontColorNormal;
		GUI::StaticTextWidget *rowLabels[] = {
			_profileNameLabels[i],
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

	_profileSelectionGroup.setValue(_selectedProfileIndex);
	updateProfileTableLayout();
	updateButtons();
}

void Zoombini2SaveManagementDialog::updateProfileTableLayout() {
	_profileTable->setSize(scaleDialogValue(kTableWidth), scaleDialogValue(_profileRowCount * kTableRowHeight));
}

void Zoombini2SaveManagementDialog::updateButtons() {
	const bool hasSelection = 0 <= _selectedProfileIndex && _selectedProfileIndex < _profileRowCount;
	const bool stateValid = hasSelection && _profileStateValid[_selectedProfileIndex];
	_editButton->setEnabled(stateValid);
	_duplicateButton->setEnabled(stateValid && _profileRowCount < kMaximumProfileRows);
	_importButton->setEnabled(true);
	_exportButton->setEnabled(stateValid);
	_deleteButton->setEnabled(hasSelection);
}

void Zoombini2SaveManagementDialog::renameSelectedProfile() {
	if (_selectedProfileIndex < 0 || static_cast<int>(_profileNames.size()) <= _selectedProfileIndex || !_profileStateValid[_selectedProfileIndex])
		return;

	const Common::String oldProfileName = _profileNames[_selectedProfileIndex];
	Zoombini2ProfileNameDialog nameDialog(Common::U32String("Rename saved game"), Common::U32String(oldProfileName.c_str()));
	if (nameDialog.runModal() != GUI::kOKCmd)
		return;

	const Common::String newProfileName = nameDialog.getProfileName().encode();
	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain);
	if (!savegameManager.renameProfile(oldProfileName, newProfileName)) {
		GUI::MessageDialog errorDialog(Common::U32String("Invalid file name for saving"));
		errorDialog.runModal();
		return;
	}
	refreshProfiles(newProfileName);
	_profileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void Zoombini2SaveManagementDialog::duplicateSelectedProfile() {
	if (_selectedProfileIndex < 0 || static_cast<int>(_profileNames.size()) <= _selectedProfileIndex || !_profileStateValid[_selectedProfileIndex] ||
		kMaximumProfileRows <= _profileRowCount)
		return;

	const Common::String srcProfileName = _profileNames[_selectedProfileIndex];
	Zoombini2ProfileNameDialog nameDialog(Common::U32String("Clone saved game"), Common::U32String());
	if (nameDialog.runModal() != GUI::kOKCmd)
		return;

	const Common::String newProfileName = nameDialog.getProfileName().encode();
	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain);
	if (!savegameManager.duplicateProfile(srcProfileName, newProfileName)) {
		GUI::MessageDialog errorDialog(Common::U32String("Enter a unique name containing 1 to 16 letters, digits, or spaces"));
		errorDialog.runModal();
		return;
	}
	refreshProfiles(newProfileName);
	_profileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void Zoombini2SaveManagementDialog::importProfile() {
	GUI::BrowserDialog browser(Common::U32String("Select one Zoombini2 .mk saved game"), false);
	if (browser.runModal() <= 0)
		return;

	const Common::FSNode src = browser.getResult();
	const Common::String srcName = src.getName();
	const size_t extensionPosition = srcName.findLastOf('.');
	if (!src.exists() || src.isDirectory() || extensionPosition == Common::String::npos ||
		!srcName.substr(extensionPosition).equalsIgnoreCase(".mk")) {
		GUI::MessageDialog errorDialog(Common::U32String("Select one Zoombini2 .mk saved game"));
		errorDialog.runModal();
		return;
	}

	const Common::String profileName = srcName.substr(0, extensionPosition);
	if (!Zoombini2SavegameManager::isValidProfileName(profileName)) {
		GUI::MessageDialog errorDialog(Common::U32String("Imported save names must use 1 to 16 letters, digits, or spaces"));
		errorDialog.runModal();
		return;
	}

	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain);
	if (savegameManager.profileExists(profileName)) {
		GUI::MessageDialog confirmation(Common::U32String("A saved game with this name already exists. Replace it?"),
										Common::U32String("Replace"), Common::U32String("Cancel"));
		if (confirmation.runModal() != GUI::kMessageOK)
			return;
	}

	Common::SeekableReadStream *stream = src.createReadStream();
	const bool imported = savegameManager.importProfile(profileName, stream, true);
	delete stream;
	if (!imported) {
		GUI::MessageDialog errorDialog(Common::U32String("The selected file is not a valid Zoombini2 saved game"));
		errorDialog.runModal();
		return;
	}

	refreshProfiles(profileName);
	_profileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
	GUI::MessageDialog successDialog(Common::U32String("The saved game was imported"));
	successDialog.runModal();
}

void Zoombini2SaveManagementDialog::exportSelectedProfile() {
	if (_selectedProfileIndex < 0 || static_cast<int>(_profileNames.size()) <= _selectedProfileIndex || !_profileStateValid[_selectedProfileIndex])
		return;

	GUI::BrowserDialog browser(Common::U32String("Select the directory for the exported Zoombini2 .mk save"), true);
	if (browser.runModal() <= 0)
		return;

	const Common::FSNode directory = browser.getResult();
	if (!directory.isDirectory()) {
		GUI::MessageDialog errorDialog(Common::U32String("The selected destination is not a directory"));
		errorDialog.runModal();
		return;
	}

	const Common::String profileName = _profileNames[_selectedProfileIndex];
	const Common::FSNode dest = findChild(directory, profileName + ".mk");
	if (dest.exists()) {
		GUI::MessageDialog confirmation(Common::U32String("The .mk file already exists. Replace it?"),
										Common::U32String("Replace"), Common::U32String("Cancel"));
		if (confirmation.runModal() != GUI::kMessageOK)
			return;
	}

	Common::SeekableWriteStream *stream = dest.createWriteStream(false);
	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain);
	bool exported = stream && savegameManager.exportProfile(profileName, stream);
	if (stream) {
		stream->finalize();
		exported = exported && !stream->err();
	}
	delete stream;
	if (!exported) {
		GUI::MessageDialog errorDialog(Common::U32String("Unable to export the selected saved game"));
		errorDialog.runModal();
		return;
	}

	GUI::MessageDialog successDialog(Common::U32String("The saved game was exported as a Zoombini2 .mk file"));
	successDialog.runModal();
}

void Zoombini2SaveManagementDialog::deleteSelectedProfile() {
	if (_selectedProfileIndex < 0 || static_cast<int>(_profileNames.size()) <= _selectedProfileIndex)
		return;

	GUI::MessageDialog confirmation(Common::U32String("Do you really want to delete this saved game?"), Common::U32String("Delete"),
									Common::U32String("Cancel"));
	if (confirmation.runModal() != GUI::kMessageOK)
		return;

	const Common::String profileName = _profileNames[_selectedProfileIndex];
	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain);
	if (!savegameManager.deleteProfile(profileName)) {
		GUI::MessageDialog errorDialog(Common::U32String("Error deleting saved game"));
		errorDialog.runModal();
		return;
	}
	refreshProfiles();
	_profileList->reflowLayout();
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
	case kProfileSelectionChangedCommand:
		if (data < static_cast<uint32>(_profileRowCount))
			_selectedProfileIndex = static_cast<int>(data);
		else
			_selectedProfileIndex = -1;
		updateButtons();
		break;
	case kEditProfileCommand:
		renameSelectedProfile();
		break;
	case kDuplicateProfileCommand:
		duplicateSelectedProfile();
		break;
	case kImportProfileCommand:
		importProfile();
		break;
	case kExportProfileCommand:
		exportSelectedProfile();
		break;
	case kDeleteProfileCommand:
		deleteSelectedProfile();
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
															  Common::U32String("Save management"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	header->setAlign(Graphics::TextAlign::kTextAlignStart);
	GUI::ButtonWidget *manageProfilesButton = new GUI::ButtonWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.ManageProfiles",
																	Common::U32String("Saved games"), Common::U32String(), kManageProfilesCommand);
	manageProfilesButton->setTarget(this);
	_savefilesReadOnlyCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.SavefilesReadOnly",
														 Common::U32String("Lock automatic savefile writes"),
														 Common::U32String("Prevents automatic progress saves during tests. New profiles and deletion remain available; import and rename are blocked."));

	new SeparatorWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayEnhancementsSeparator");
	header = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayEnhancements",
									   Common::U32String("Gameplay Enhancements"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	header->setAlign(Graphics::TextAlign::kTextAlignStart);
	_stereoOutputCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.StereoOutput", Common::U32String("Enable stereo game audio"),
													Common::U32String("Keeps both channels of stereo WAV resources instead of downmixing them to mono."));
	_floatingPointPathsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.FloatingPointPaths",
														  Common::U32String("Use floating-point path calculations"),
														  Common::U32String("Uses 32-bit floating point instead of the original signed Q10 fixed-point arithmetic for Bezier movement paths."));
	_enhancedKbdShortcutsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.EnhancedKbdShortcuts",
															Common::U32String("Enable enhanced keyboard shortcuts"),
															Common::U32String("Enables some ScummVM-only keyboard shortcuts for quality of life improvements."));
	if (Common::checkGameGUIOption(GAMEOPTION_HELP_PAGE_COLOR_KEYING, ConfMan.get("guioptions", domain))) {
		_transparentHelpPagesCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.TransparentHelpPages",
																Common::U32String("Remove solid color behind help text"),
																Common::U32String("Uses the help sheet's corner color as a transparency key for affected releases."));
	}

	new SeparatorWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayAdjustmentSeparator");
	header = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayAdjustment",
									   Common::U32String("Gameplay Adjustment"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	header->setAlign(Graphics::TextAlign::kTextAlignStart);
	_debugHotkeysCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.DebugHotkeys", Common::U32String("Enable developer hotkeys"),
													Common::U32String("Enables F2/F3 party exchange, P puzzle completion, and the Chez Norf C overlay."));
	_greedyWaterslideCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GreedyWaterslide",
														Common::U32String("Use alternate Pipes of Paloo pairing"),
														Common::U32String("Selects the alternate pairing branch for level one."));
	_aquacubeSafeFirstMoveCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.AquacubeSafeFirstMove",
															 Common::U32String("Protect Aqua Cube's first lever move"),
															 Common::U32String("On level three, swaps the hidden axes of two levers if the first direct lever press would enter a Fleen cell."));
	_allowCutLevel4PracticePuzzlesCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.AllowCutLevel4PracticePuzzles",
																	 Common::U32String("Allow cut level 4 puzzles in practice mode"),
																	 Common::U32String("Adds a level 4 practice-map tab. Puzzles without a recovered level 4 remain at level three."));
	_originalPrngCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.OriginalPRNG",
													Common::U32String("Use original random number generator (requires restart)"),
													Common::U32String("Uses the original Windows engine's Visual C++ 6.0 CRT generator instead of ScummVM's default."));
	GUI::StaticTextWidget *frameRateLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.FrameRateLabel",
																	  Common::U32String("Frame rate (FPS):"));
	frameRateLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_frameRateNumberBox = new FrameRateNumberBox(widgetsBoss(), "Zoombini2EngineOptionsDialog.FrameRate",
												 ::Zoombini2MetaEngine::kDefaultFrameRate,
												 Common::U32String("Enter a whole number from 30 to 240 FPS."));
	GUI::StaticTextWidget *pacingLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.PacingLabel",
																   Common::U32String("Logic pacing:"));
	pacingLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_pacingPopUp = new GUI::PopUpWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.Pacing",
										Common::U32String("Select 60Hz LCD or 75Hz CRT logic pacing. Affects Booliewood panorama scrolling speed and idle animation trigger rates. Rendering frame rate is unaffected."));
	_pacingPopUp->appendEntry(Common::U32String("60Hz LCD"), 60);
	_pacingPopUp->appendEntry(Common::U32String("75Hz CRT"), 75);
	_unlockFrameRateCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.UnlockFrameRate",
													   Common::U32String("Unlock frame rate"),
													   Common::U32String("Removes the engine frame-rate limit. Display VSync may still limit presentation."),
													   kUnlockFrameRateCommand);
	_unlockFrameRateCheckbox->setTarget(this);
}

void Zoombini2OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	const int lineHeight = layouts.getVar("Globals.Line.Height");
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical)
		.addPadding(0, 0, 0, 0)
		.addSpace(10)
		.addWidget("SaveFilesSeparator", "", -1, 2)
		.addWidget("SaveFilesHeader", "", -1, lineHeight)
		.addWidget("ManageProfiles", "Button")
		.addWidget("SavefilesReadOnly", "Checkbox")
		.addSpace(10)
		.addWidget("GameplayEnhancementsSeparator", "", -1, 2)
		.addWidget("GameplayEnhancements", "", -1, lineHeight)
		.addWidget("StereoOutput", "Checkbox")
		.addWidget("FloatingPointPaths", "Checkbox")
		.addWidget("EnhancedKbdShortcuts", "Checkbox")
		.addWidget("TransparentHelpPages", "Checkbox")
		.addSpace(10)
		.addWidget("GameplayAdjustmentSeparator", "", -1, 2)
		.addWidget("GameplayAdjustment", "", -1, lineHeight)
		.addWidget("DebugHotkeys", "Checkbox")
		.addWidget("GreedyWaterslide", "Checkbox")
		.addWidget("AquacubeSafeFirstMove", "Checkbox")
		.addWidget("AllowCutLevel4PracticePuzzles", "Checkbox")
		.addWidget("OriginalPRNG", "Checkbox")
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addWidget("PacingLabel", "OptionsLabel")
		.addWidget("Pacing", "", 120, lineHeight)
		.closeLayout()
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addWidget("FrameRateLabel", "OptionsLabel")
		.addWidget("FrameRate", "", 64, lineHeight)
		.closeLayout()
		.addWidget("UnlockFrameRate", "Checkbox")
		.closeLayout()
		.closeDialog();
}

void Zoombini2OptionsWidget::load() {
	_savefilesReadOnlyCheckbox->setState(ConfMan.getBool(::Zoombini2MetaEngine::kConfigSavefilesReadOnly, _domain));
	_stereoOutputCheckbox->setState(ConfMan.getBool(::Zoombini2MetaEngine::kConfigStereoOutput, _domain));
	_floatingPointPathsCheckbox->setState(ConfMan.getBool(::Zoombini2MetaEngine::kConfigUseFloatingPointPaths, _domain));
	_enhancedKbdShortcutsCheckbox->setState(ConfMan.getBool(::Zoombini2MetaEngine::kConfigEnhancedKbdShortcuts, _domain));
	if (_transparentHelpPagesCheckbox)
		_transparentHelpPagesCheckbox->setState(ConfMan.getBool(::Zoombini2MetaEngine::kConfigTransparentHelpPages, _domain));
	_debugHotkeysCheckbox->setState(ConfMan.getBool(::Zoombini2MetaEngine::kConfigDebugHotkeys, _domain));
	_greedyWaterslideCheckbox->setState(ConfMan.getBool(::Zoombini2MetaEngine::kConfigGreedyWaterslidePairing, _domain));
	_aquacubeSafeFirstMoveCheckbox->setState(ConfMan.getBool(::Zoombini2MetaEngine::kConfigAquacubeSafeFirstMove, _domain));
	_allowCutLevel4PracticePuzzlesCheckbox->setState(ConfMan.getBool(::Zoombini2MetaEngine::kConfigAllowCutLevel4PracticePuzzles, _domain));
	_originalPrngCheckbox->setState(ConfMan.getBool(::Zoombini2MetaEngine::kConfigOriginalPRNG, _domain));
	_pacingPopUp->setSelectedTag(ConfMan.getInt(::Zoombini2MetaEngine::kConfigLogicPacingHz, _domain) == 75 ? 75 : 60);
	const int frameRate = CLIP<int>(ConfMan.getInt(::Zoombini2MetaEngine::kConfigFrameRate, _domain),
									::Zoombini2MetaEngine::kMinFrameRate, ::Zoombini2MetaEngine::kMaxFrameRate);
	_frameRateNumberBox->setValue(frameRate);
	_unlockFrameRateCheckbox->setState(ConfMan.getBool(::Zoombini2MetaEngine::kConfigUnlockFrameRate, _domain));
	updateFrameRateControls();
}

bool Zoombini2OptionsWidget::save() {
	const int frameRate = _frameRateNumberBox->getValue();
	_frameRateNumberBox->setValue(frameRate);

	const bool originalPrngChanged = ConfMan.getBool(::Zoombini2MetaEngine::kConfigOriginalPRNG, _domain) != _originalPrngCheckbox->getState();

	ConfMan.setBool(::Zoombini2MetaEngine::kConfigSavefilesReadOnly, _savefilesReadOnlyCheckbox->getState(), _domain);
	ConfMan.setBool(::Zoombini2MetaEngine::kConfigStereoOutput, _stereoOutputCheckbox->getState(), _domain);
	ConfMan.setBool(::Zoombini2MetaEngine::kConfigUseFloatingPointPaths, _floatingPointPathsCheckbox->getState(), _domain);
	ConfMan.setBool(::Zoombini2MetaEngine::kConfigEnhancedKbdShortcuts, _enhancedKbdShortcutsCheckbox->getState(), _domain);
	if (_transparentHelpPagesCheckbox)
		ConfMan.setBool(::Zoombini2MetaEngine::kConfigTransparentHelpPages, _transparentHelpPagesCheckbox->getState(), _domain);
	ConfMan.setBool(::Zoombini2MetaEngine::kConfigDebugHotkeys, _debugHotkeysCheckbox->getState(), _domain);
	ConfMan.setBool(::Zoombini2MetaEngine::kConfigGreedyWaterslidePairing, _greedyWaterslideCheckbox->getState(), _domain);
	ConfMan.setBool(::Zoombini2MetaEngine::kConfigAquacubeSafeFirstMove, _aquacubeSafeFirstMoveCheckbox->getState(), _domain);
	ConfMan.setBool(::Zoombini2MetaEngine::kConfigAllowCutLevel4PracticePuzzles, _allowCutLevel4PracticePuzzlesCheckbox->getState(), _domain);
	ConfMan.setBool(::Zoombini2MetaEngine::kConfigOriginalPRNG, _originalPrngCheckbox->getState(), _domain);
	ConfMan.setInt(::Zoombini2MetaEngine::kConfigLogicPacingHz, _pacingPopUp->getSelectedTag() == 75 ? 75 : 60, _domain);
	ConfMan.setInt(::Zoombini2MetaEngine::kConfigFrameRate, frameRate, _domain);
	ConfMan.setBool(::Zoombini2MetaEngine::kConfigUnlockFrameRate, _unlockFrameRateCheckbox->getState(), _domain);
	if (originalPrngChanged && g_engine) {
		GUI::MessageDialog dialog(Common::U32String("The random number generator change will take effect after restarting the game."));
		dialog.runModal();
	}
	return true;
}

void Zoombini2OptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kUnlockFrameRateCommand) {
		updateFrameRateControls();
		return;
	}
	if (cmd == kManageProfilesCommand) {
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
