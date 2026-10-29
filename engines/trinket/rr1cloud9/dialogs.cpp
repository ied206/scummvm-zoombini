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
 */

#include "trinket/rr1cloud9/dialogs.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/engine.h"
#include "gui/ThemeEval.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/widgets/popup.h"
#include "trinket/metaengine.h"

namespace Trinket {
namespace RR1Cloud9 {

constexpr const char *SaveManagementDialog::kExportFileFormat;
SaveManagementDialog::SaveManagementDialog(const Common::String &target)
	: Trinket::SaveManagementDialog(false), _saves(*g_system->getSavefileManager(), target) {}

void SaveManagementDialog::refreshSaveList() {
	clearSaveRows();
	if (!_saves.load()) {
		GUI::MessageDialog(_("Could not read the player registries.")).runModal();
		finishSaveRows(false);
		return;
	}
	for (uint i = 0; i < _saves.players().size(); i++) {
		const Shell::SaveFiles::Player &player = _saves.players()[i];
		const Common::U32String labels[] = {
			Common::U32String::format("%u", player.id), Common::U32String(player.name, Common::kUtf8),
			Common::U32String(U"1 / 1"), Common::U32String::format(_("Scene %d"), player.locations[0])
		};
		addSaveRow(i, 1, labels);
	}
	finishSaveRows();
}

void SaveManagementDialog::handleSaveCommand(uint32 command, uint player, uint slot) {
	bool success = false;
	Common::U32String failure;
	switch (command) {
	case kRename: {
		failure = _("The operation failed. Choose a unique player name of 1 to 16 ASCII characters.");
		Common::String name = _saves.players()[player].name;
		if (!editPlayerName(name, _("Rename"), 16))
			return;
		success = _saves.rename(player, name);
		break;
	}
	case kDelete: {
		failure = _("Unable to delete the selected saved game.");
		GUI::MessageDialog confirm(_("Delete this player and saved game?"), _("Delete"), _("Cancel"));
		if (confirm.runModal() != GUI::kMessageOK)
			return;
		success = _saves.remove(player);
		break;
	}
	case kExport: {
		failure = _("Unable to export the selected saved game.");
		const Common::String filename = Common::String::format(kExportFileFormat, _saves.players()[player].id);
		Common::ScopedPtr<Common::SeekableWriteStream> stream(openExportFile(filename));
		if (!stream)
			return;
		success = _saves.exportSlot(player, *stream);
		stream->finalize();
		success = success && !stream->err();
		break;
	}
	default:
		return;
	}
	if (!success) {
		if (_saves.rollbackFailed())
			GUI::MessageDialog(_("The operation failed and some files could not be restored. Check your save directory before continuing.")).runModal();
		else
			GUI::MessageDialog(failure).runModal();
	}
	refreshSaveList();
}

void SaveManagementDialog::importSave() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(openImportFile(_("Select a ScummVM RR1Cloud9 saved game")));
	if (!stream)
		return;
	Common::String name;
	if (!editPlayerName(name, _("Save name"), 16))
		return;
	if (!_saves.importSave(name, *stream)) {
		if (_saves.rollbackFailed())
			GUI::MessageDialog(_("The operation failed and some files could not be restored. Check your save directory before continuing.")).runModal();
		else
			GUI::MessageDialog(_("Unable to import the saved game. Check the file and choose a unique player name.")).runModal();
	}
	refreshSaveList();
}

OptionsWidget::OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain)
	: GUI::OptionsContainerWidget(boss, name, "TrinketRR1Cloud9Options", domain) {
	_saveFilesHdr = new GUI::StaticTextWidget(widgetsBoss(), "TrinketRR1Cloud9Options.SaveFilesHeader", _("Manage savefiles"),
		Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	_saveFilesHdr->setAlign(Graphics::kTextAlignStart);
	_manage = new GUI::ButtonWidget(widgetsBoss(), "TrinketRR1Cloud9Options.ManageSaves", _("Manage"), Common::U32String(), kManageSaves);
	_manage->setTarget(this);
	_colorBlindMode = new GUI::CheckboxWidget(widgetsBoss(), "TrinketRR1Cloud9Options.ColorBlindMode",
		_("Color Blind Mode (Experimental)"), _("Adjust puzzle colors for red-green color blindness."));
	const Common::U32String startupModeTip = _("Start the game directly or through the Learning Launcher. "
												 "Changes take effect the next time you start the game.");
	_startupModeLabel = new GUI::StaticTextWidget(widgetsBoss(), "TrinketRR1Cloud9Options.StartupModeLabel", _("Startup mode:"), startupModeTip);
	_startupModeLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_startupModePopUp = new GUI::PopUpWidget(widgetsBoss(), "TrinketRR1Cloud9Options.StartupMode", startupModeTip);
	_startupModePopUp->appendEntry(_("Start directly"), static_cast<uint32>(TrinketMetaEngine::StartupMode::kDirect00));
	_startupModePopUp->appendEntry(_("Start through the Learning Launcher"), static_cast<uint32>(TrinketMetaEngine::StartupMode::kLauncher01));
}

void OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	static constexpr int kOptionLabelPadding = 4;
	const int lineHeight = layouts.getVar("Globals.Line.Height");
	const int labelW = g_gui.getStringWidth(_startupModeLabel->getLabel()) + kOptionLabelPadding;
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical)
		.addWidget("SaveFilesHeader", "", -1, lineHeight)
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
		.addWidget("ManageSaves", "Button")
		.closeLayout()
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("StartupModeLabel", "", labelW, lineHeight)
		.addWidget("StartupMode", "PopUp")
		.closeLayout()
		.addWidget("ColorBlindMode", "Checkbox")
		.closeLayout()
		.closeDialog();
}

void OptionsWidget::load() {
	_colorBlindMode->setState(ConfMan.getBool(TrinketMetaEngine::kOptionColorBlindMode, _domain));
	_startupModePopUp->setSelectedTag(ConfMan.getInt(TrinketMetaEngine::kOptionStartupMode, _domain));
}

bool OptionsWidget::save() {
	ConfMan.setBool(TrinketMetaEngine::kOptionColorBlindMode, _colorBlindMode->getState(), _domain);
	const TrinketMetaEngine::StartupMode startupMode = static_cast<TrinketMetaEngine::StartupMode>(_startupModePopUp->getSelectedTag());
	ConfMan.setInt(TrinketMetaEngine::kOptionStartupMode, static_cast<int>(startupMode), _domain);
	return true;
}

void OptionsWidget::setEnabled(bool enabled) {
	_saveFilesHdr->setEnabled(enabled);
	_manage->setEnabled(enabled);
	_colorBlindMode->setEnabled(enabled);
	_startupModeLabel->setEnabled(enabled);
	_startupModePopUp->setEnabled(enabled);
}

void OptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) {
	if (command != kManageSaves) {
		GUI::OptionsContainerWidget::handleCommand(sender, command, data);
		return;
	}
	SaveManagementDialog(_domain).runModal();
}

} // End of namespace RR1Cloud9
} // End of namespace Trinket
