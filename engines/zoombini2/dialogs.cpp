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

#include "common/savefile.h"
#include "common/system.h"

#include "gui/ThemeEval.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/widgets/list.h"

#include "zoombini2/dialogs.h"
#include "zoombini2/saveload.h"

namespace Zoombini2 {

const uint32 Zoombini2SaveManagementDialog::kEditProfileCommand = 'z2ed';
const uint32 Zoombini2SaveManagementDialog::kDeleteProfileCommand = 'z2dl';
const uint32 Zoombini2OptionsWidget::kManageProfilesCommand = 'z2mg';

Zoombini2SaveManagementDialog::Zoombini2SaveManagementDialog(const Common::String &domain)
	: GUI::Dialog(55, 50, 530, 360, true), _domain(domain), _profileList(nullptr), _editButton(nullptr), _deleteButton(nullptr), _editingProfile(false) {
	new GUI::StaticTextWidget(this, 16, 12, 498, 24, true, Common::U32String("Saved games"), Graphics::kTextAlignCenter);

	_profileList = new GUI::ListWidget(this, 16, 44, 498, 250, true);
	_profileList->setEditable(true);
	_profileList->setNumberingMode(GUI::kListNumberingOff);

	_editButton = new GUI::ButtonWidget(this, 16, 312, 100, 28, true, Common::U32String("Edit"), Common::U32String(), kEditProfileCommand);
	_deleteButton = new GUI::ButtonWidget(this, 124, 312, 100, 28, true, Common::U32String("Delete"), Common::U32String(), kDeleteProfileCommand);
	new GUI::ButtonWidget(this, 414, 312, 100, 28, true, Common::U32String("Close"), Common::U32String(), GUI::kCloseCmd, Common::ASCII_ESCAPE);
}

void Zoombini2SaveManagementDialog::open() {
	GUI::Dialog::open();
	refreshProfiles();
	g_gui.scheduleTopDialogRedraw();
}

void Zoombini2SaveManagementDialog::refreshProfiles(const Common::String &selectedProfile) {
	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain);
	_profileNames = savegameManager.listProfiles();

	Common::U32StringArray displayNames;
	int selectedIndex = -1;
	for (uint i = 0; i < _profileNames.size(); i++) {
		displayNames.push_back(Common::U32String(_profileNames[i].c_str()));
		if (!selectedProfile.empty() && _profileNames[i].equalsIgnoreCase(selectedProfile))
			selectedIndex = static_cast<int>(i);
	}
	_profileList->setList(displayNames);
	if (selectedIndex < 0 && !_profileNames.empty())
		selectedIndex = 0;
	_profileList->setSelected(selectedIndex);
	_editingProfile = false;
	_profileNameBeforeEdit.clear();
	updateButtons();
}

void Zoombini2SaveManagementDialog::updateButtons() {
	const int selectedIndex = _profileList->getSelected();
	const bool hasSelection = 0 <= selectedIndex && selectedIndex < static_cast<int>(_profileNames.size());
	_editButton->setEnabled(hasSelection);
	_deleteButton->setEnabled(hasSelection);
}

void Zoombini2SaveManagementDialog::finishRename() {
	const int selectedIndex = _profileList->getSelected();
	if (!_editingProfile || selectedIndex < 0 || static_cast<int>(_profileNames.size()) <= selectedIndex)
		return;

	const Common::String newProfileName = _profileList->getSelectedString().encode();
	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain);
	if (!savegameManager.renameProfile(_profileNameBeforeEdit, newProfileName)) {
		GUI::MessageDialog errorDialog(Common::U32String("Invalid file name for saving"));
		errorDialog.runModal();
		refreshProfiles(_profileNameBeforeEdit);
		return;
	}
	refreshProfiles(newProfileName);
}

void Zoombini2SaveManagementDialog::deleteSelectedProfile() {
	const int selectedIndex = _profileList->getSelected();
	if (selectedIndex < 0 || static_cast<int>(_profileNames.size()) <= selectedIndex)
		return;

	GUI::MessageDialog confirmation(Common::U32String("Do you really want to delete this saved game?"), Common::U32String("Delete"),
			Common::U32String("Cancel"));
	if (confirmation.runModal() != GUI::kMessageOK)
		return;

	const Common::String profileName = _profileNames[selectedIndex];
	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain);
	if (!savegameManager.deleteProfile(profileName)) {
		GUI::MessageDialog errorDialog(Common::U32String("Error deleting saved game"));
		errorDialog.runModal();
		return;
	}
	refreshProfiles();
}

void Zoombini2SaveManagementDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case GUI::kListSelectionChangedCmd:
		updateButtons();
		break;
	case kEditProfileCommand: {
		const int selectedIndex = _profileList->getSelected();
		if (0 <= selectedIndex && selectedIndex < static_cast<int>(_profileNames.size())) {
			_profileNameBeforeEdit = _profileNames[selectedIndex];
			_editingProfile = true;
			_profileList->startEditMode();
		}
		break;
	}
	case GUI::kListItemActivatedCmd:
		finishRename();
		break;
	case kDeleteProfileCommand:
	case GUI::kListItemRemovalRequestCmd:
		deleteSelectedProfile();
		break;
	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
		break;
	}
}

Zoombini2OptionsWidget::Zoombini2OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain)
	: GUI::OptionsContainerWidget(boss, name, "Zoombini2EngineOptionsDialog", domain) {
	GUI::ButtonWidget *manageProfilesButton = new GUI::ButtonWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.ManageProfiles", Common::U32String("Saved games"),
			Common::U32String(), kManageProfilesCommand);
	manageProfilesButton->setTarget(this);
}

void Zoombini2OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical)
		.addPadding(0, 0, 0, 0)
		.addWidget("ManageProfiles", "Button")
		.closeLayout()
		.closeDialog();
}

void Zoombini2OptionsWidget::load() {
}

bool Zoombini2OptionsWidget::save() {
	return false;
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
