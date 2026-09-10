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
#include "common/language.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/util.h"

#include "gui/ThemeEval.h"
#include "gui/browser.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/scrollcontainer.h"

#include "zoombini2/dialogs.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

const uint32 Zoombini2SaveManagementDialog::kEditProfileCommand = 'z2ed';
const uint32 Zoombini2SaveManagementDialog::kDeleteProfileCommand = 'z2dl';
const uint32 Zoombini2SaveManagementDialog::kImportProfileCommand = 'z2im';
const uint32 Zoombini2SaveManagementDialog::kExportProfileCommand = 'z2ex';
const uint32 Zoombini2SaveManagementDialog::kProfileSelectionChangedCommand = 'z2sl';
const uint32 Zoombini2OptionsWidget::kManageProfilesCommand = 'z2mg';

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

Zoombini2MenuDialog::Zoombini2MenuDialog(Zoombini2Engine *vm) : MainMenuDialog(vm) {
}

void Zoombini2MenuDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kOptionsCmd) {
		GUI::ConfigDialog configDialog;
		configDialog.runModal();

		Zoombini2Engine *vm = static_cast<Zoombini2Engine *>(_engine);
		vm->applyGameSettings();
		vm->syncSoundSettings();
		return;
	}

	MainMenuDialog::handleCommand(sender, cmd, data);
}

Zoombini2ProfileNameDialog::Zoombini2ProfileNameDialog(const Common::U32String &initialName)
	: GUI::Dialog(0, 0, 360, 144), _edit(nullptr) {
	new GUI::StaticTextWidget(this, 12, 10, 336, 24, Common::U32String("Rename saved game"), Graphics::kTextAlignStart);
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
	const int screenWidth = g_system->getOverlayWidth();
	const int screenHeight = g_system->getOverlayHeight();
	_x = MAX<int>(0, (screenWidth - _w) / 2);
	_y = MAX<int>(0, (screenHeight - _h) / 2);
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
	: GUI::Dialog(0, 0, 1, 1), _domain(domain), _profileSelectionGroup(this, kProfileSelectionChangedCommand), _selectedProfileIndex(-1),
	  _profileList(nullptr), _profileTable(nullptr), _profileRowCount(0), _editButton(nullptr), _importButton(nullptr), _exportButton(nullptr),
	  _deleteButton(nullptr) {
	for (int i = 0; i < kMaximumProfileRows; i++) {
		_profileSelectionButtons[i] = nullptr;
		_profileNameLabels[i] = nullptr;
		_zombinivilleLabels[i] = nullptr;
		_rescue1Labels[i] = nullptr;
		_rescue2Labels[i] = nullptr;
		_booliewoodLabels[i] = nullptr;
		_activePartyLabels[i] = nullptr;
		_profileStateValid[i] = false;
	}

	new GUI::StaticTextWidget(this, kDialogMargin, 8, kTableWidth, 24, true, Common::U32String("Manage saved games"), Graphics::kTextAlignStart);

	static constexpr int kActionButtonGap = 6;
	static constexpr int kActionButtonWidth = (kTableWidth - 3 * kActionButtonGap) / 4;
	int actionX = kDialogMargin;
	_editButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, Common::U32String("Rename"),
										Common::U32String(), kEditProfileCommand);
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
	const int width = scaleDialogValue(kTableWidth + 2 * kDialogMargin) + scrollbarWidth;
	const int height = scaleDialogValue(kDialogHeight);
	const int screenWidth = g_system->getOverlayWidth();
	const int screenHeight = g_system->getOverlayHeight();
	_x = MAX<int>(0, (screenWidth - width) / 2);
	_y = MAX<int>(0, (screenHeight - height) / 2);
	_w = width;
	_h = height;
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

		const GUI::ThemeEngine::FontColor fontColor = summary._stateValid ? GUI::ThemeEngine::kFontColorNormal : GUI::ThemeEngine::kFontColorOverride;
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
	_importButton->setEnabled(true);
	_exportButton->setEnabled(stateValid);
	_deleteButton->setEnabled(hasSelection);
}

void Zoombini2SaveManagementDialog::renameSelectedProfile() {
	if (_selectedProfileIndex < 0 || static_cast<int>(_profileNames.size()) <= _selectedProfileIndex || !_profileStateValid[_selectedProfileIndex])
		return;

	const Common::String oldProfileName = _profileNames[_selectedProfileIndex];
	Zoombini2ProfileNameDialog nameDialog(Common::U32String(oldProfileName.c_str()));
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
	: GUI::OptionsContainerWidget(boss, name, "Zoombini2EngineOptionsDialog", domain), _debugHotkeysCheckbox(nullptr), _stereoOutputCheckbox(nullptr),
	  _greedyWaterslideCheckbox(nullptr), _cachedFrameTimeCheckbox(nullptr), _floatingPointPathsCheckbox(nullptr) {
	_debugHotkeysCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.DebugHotkeys", Common::U32String("Enable developer hotkeys"),
													Common::U32String("Enables F2/F3 party exchange, P puzzle completion, and the Chez Norf C overlay."));
	_stereoOutputCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.StereoOutput", Common::U32String("Enable stereo game audio"),
													Common::U32String("Keeps both channels of stereo WAV resources instead of downmixing them to mono."));
	_greedyWaterslideCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GreedyWaterslide",
												Common::U32String("Use alternate Pipes of Paloo pairing"),
												Common::U32String("Selects the alternate pairing branch for level one."));
	_cachedFrameTimeCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.CachedFrameTime",
													   Common::U32String("Use frame-cached game timing"),
													   Common::U32String("Makes gameplay time reads share one clock snapshot per rendered frame."));

	new SeparatorWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayImprovementsSeparator");
	GUI::StaticTextWidget *gameplayImprovementsHeader = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayImprovements",
																	 Common::U32String("Gameplay improvements"), Common::U32String(),
																	 GUI::ThemeEngine::kFontStyleBold);
	gameplayImprovementsHeader->setAlign(Graphics::TextAlign::kTextAlignStart);
	_floatingPointPathsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.FloatingPointPaths",
															 Common::U32String("Use floating-point path calculations"),
															 Common::U32String("Uses 32-bit floating point instead of the original signed Q10 fixed-point arithmetic for Bezier movement paths."));

	GUI::ButtonWidget *manageProfilesButton = new GUI::ButtonWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.ManageProfiles", Common::U32String("Saved games"),
																	Common::U32String(), kManageProfilesCommand);
	manageProfilesButton->setTarget(this);
}

void Zoombini2OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical)
		.addPadding(0, 0, 0, 0)
		.addWidget("DebugHotkeys", "Checkbox")
		.addWidget("StereoOutput", "Checkbox")
		.addWidget("GreedyWaterslide", "Checkbox")
		.addWidget("CachedFrameTime", "Checkbox")
		.addWidget("ManageProfiles", "Button")
		.addSpace(10)
		.addWidget("GameplayImprovementsSeparator", "", -1, 2)
		.addWidget("GameplayImprovements", "OptionsLabel")
		.addWidget("FloatingPointPaths", "Checkbox")
		.closeLayout()
		.closeDialog();
}

void Zoombini2OptionsWidget::load() {
	_debugHotkeysCheckbox->setState(ConfMan.getBool(kConfigDebugHotkeys, _domain));
	_stereoOutputCheckbox->setState(ConfMan.getBool(kConfigStereoOutput, _domain));
	_greedyWaterslideCheckbox->setState(ConfMan.getBool(kConfigGreedyWaterslidePairing, _domain));
	_cachedFrameTimeCheckbox->setState(ConfMan.getBool(kConfigCachedFrameTime, _domain));
	_floatingPointPathsCheckbox->setState(ConfMan.getBool(kConfigUseFloatingPointPaths, _domain));
}

bool Zoombini2OptionsWidget::save() {
	ConfMan.setBool(kConfigDebugHotkeys, _debugHotkeysCheckbox->getState(), _domain);
	ConfMan.setBool(kConfigStereoOutput, _stereoOutputCheckbox->getState(), _domain);
	ConfMan.setBool(kConfigGreedyWaterslidePairing, _greedyWaterslideCheckbox->getState(), _domain);
	ConfMan.setBool(kConfigCachedFrameTime, _cachedFrameTimeCheckbox->getState(), _domain);
	ConfMan.setBool(kConfigUseFloatingPointPaths, _floatingPointPathsCheckbox->getState(), _domain);
	return true;
}

void Zoombini2OptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kManageProfilesCommand) {
		Zoombini2SaveManagementDialog dialog(_domain);
		dialog.runModal();
		return;
	}
	GUI::OptionsContainerWidget::handleCommand(sender, cmd, data);
}

} // End of namespace Zoombini2
