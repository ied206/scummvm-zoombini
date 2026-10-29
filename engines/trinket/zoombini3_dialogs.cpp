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

#include "trinket/zoombini3_dialogs.h"

#include "common/config-manager.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/engine.h"
#include "gui/ThemeEval.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/widgets/popup.h"
#include "trinket/metaengine.h"
#include "trinket/shell/random.h"
#include "trinket/zoombini3_saves.h"
#include "trinket/zoombini3_world.h"

namespace Trinket {
namespace Zoombini3 {

constexpr const char *SaveManagementDialog::kExportFileFormat;

SaveManagementDialog::SaveManagementDialog(const Common::String &target)
	: Trinket::SaveManagementDialog(true), _saves(*g_system->getSavefileManager(), target) {}

Common::U32String SaveManagementDialog::locationName(int location) {
	switch (location) {
	case kCatapult01:
		return _("The Catapult");
	case kGlyph02:
		return _("The Wall");
	case kPlanetarium03:
		return _("The Planetarium");
	case kGreenhouse04:
		return _("The Greenhouse");
	case kGarden05:
		return _("The Garden");
	case kCorral06:
		return _("The Corral");
	case kBarn07:
		return _("The Barn");
	case kRecruitment11:
		return _("Zoombiniville");
	case kMap12:
		return _("Map");
	case kIsland13:
		return _("Zoombini Isle");
	default:
		return _("Unknown");
	}
}

void SaveManagementDialog::refreshSaveList() {
	clearSaveRows();
	if (!_saves.load()) {
		GUI::MessageDialog(_("Could not read the player registries.")).runModal();
		finishSaveRows(false);
		return;
	}
	for (uint i = 0; i < _saves.players().size(); i++) {
		const Shell::SaveFiles::Player &player = _saves.players()[i];
		if (!_saves.isVisible(player.name))
			continue;
		for (uint slot = 1; slot <= player.count; slot++) {
			Common::U32String game = Common::U32String::format("%u / %u", slot, player.count);
			if (slot == player.current)
				game += Common::U32String(U" *");
			const Common::U32String labels[] = {
				Common::U32String::format("%u", player.id), Common::U32String(player.name, Common::kUtf8),
				game, locationName(player.locations[slot - 1])
			};
			addSaveRow(i, slot, labels);
		}
	}
	finishSaveRows();
}

void SaveManagementDialog::handleSaveCommand(uint32 command, uint player, uint slot) {
	bool success = false;
	Common::U32String failure;
	switch (command) {
	case kRename:
	case kClone: {
		failure = _("The operation failed. Choose a normal player for save operations and a unique name of 1 to 21 ASCII characters.");
		Common::String name = _saves.players()[player].name;
		Common::U32String title = _("Rename");
		if (command == kClone)
			title = _("Clone");
		if (!editPlayerName(name, title, 21))
			return;
		success = _saves.rename(player, name, command == kClone);
		break;
	}
	case kDelete: {
		failure = _("Unable to delete the selected saved game.");
		GUI::MessageDialog confirm(_("Delete this player, all six game slots and the practice save?"), _("Delete"), _("Cancel"));
		if (confirm.runModal() != GUI::kMessageOK)
			return;
		success = _saves.remove(player);
		break;
	}
	case kSelect:
		failure = _("Unable to select the saved game.");
		success = _saves.select(player, slot);
		break;
	case kExport: {
		failure = _("Unable to export the selected saved game.");
		const Common::String filename = Common::String::format(kExportFileFormat, _saves.players()[player].id, slot);
		Common::ScopedPtr<Common::SeekableWriteStream> stream(openExportFile(filename));
		if (!stream)
			return;
		success = _saves.exportSlot(player, slot, *stream);
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
	Common::ScopedPtr<Common::SeekableReadStream> stream(openImportFile(_("Select a Zoombinis: Island Odyssey User...GZ.dat saved game")));
	if (!stream)
		return;
	Common::String name;
	if (!editPlayerName(name, _("Save name"), 21))
		return;
	if (!_saves.importSave(name, *stream)) {
		if (_saves.rollbackFailed())
			GUI::MessageDialog(_("The operation failed and some files could not be restored. Check your save directory before continuing.")).runModal();
		else
			GUI::MessageDialog(_("Unable to import the saved game. Check the file and choose a unique player name.")).runModal();
	}
	refreshSaveList();
}

OptionsWidget::OptionsWidget(GUI::GuiObject *boss, const Common::String &name,
	const Common::String &domain) : GUI::OptionsContainerWidget(boss, name, "TrinketZoombini3Options", domain) {
	_saveFilesHdr = new GUI::StaticTextWidget(widgetsBoss(), "TrinketZoombini3Options.SaveFilesHeader", _("Manage savefiles"),
		Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	_saveFilesHdr->setAlign(Graphics::kTextAlignStart);
	_colorBlindMode = new GUI::CheckboxWidget(widgetsBoss(), "TrinketZoombini3Options.ColorBlindMode",
		_("Color Blind Mode (Experimental)"), _("Adjust puzzle colors for red-green color blindness."));
	const Common::U32String prngTooltip = _("Selects the original game's PRNG or ScummVM's standard PRNG. "
											"Changing this option reseeds the PRNG immediately.");
	_prngAlgoLabel = new GUI::StaticTextWidget(widgetsBoss(), "TrinketZoombini3Options.PrngAlgorithmLabel", _("PRNG algorithm:"), prngTooltip);
	_prngAlgoLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_prngAlgoPopUp = new GUI::PopUpWidget(widgetsBoss(), "TrinketZoombini3Options.PrngAlgorithm", prngTooltip);
	_prngAlgoPopUp->appendEntry(_("Original PRNG"), static_cast<uint32>(Shell::Random::PrngAlgorithm::kOriginalPrng));
	_prngAlgoPopUp->appendEntry(_("ScummVM Standard PRNG"), static_cast<uint32>(Shell::Random::PrngAlgorithm::kStandardPrng));
	_manage = new GUI::ButtonWidget(widgetsBoss(), "TrinketZoombini3Options.Saves", _("Manage"), Common::U32String(), kManageSaves);
	_manage->setTarget(this);
}

void OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &name, const Common::String &overlay) const {
	const int lineHeight = layouts.getVar("Globals.Line.Height");
	static constexpr int kOptionLabelPadding = 4;
	const int prngLabelWidth = g_gui.getStringWidth(_prngAlgoLabel->getLabel()) + kOptionLabelPadding;
	layouts.addDialog(name, overlay)
		.addLayout(GUI::ThemeLayout::kLayoutVertical)
		.addWidget("SaveFilesHeader", "", -1, lineHeight)
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal)
		.addWidget("Saves", "Button")
		.closeLayout()
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("PrngAlgorithmLabel", "", prngLabelWidth, lineHeight)
		.addWidget("PrngAlgorithm", "PopUp")
		.closeLayout()
		.addWidget("ColorBlindMode", "Checkbox")
		.closeLayout()
		.closeDialog();
}

void OptionsWidget::load() {
	_colorBlindMode->setState(ConfMan.getBool(TrinketMetaEngine::kOptionColorBlindMode, _domain));
	_prngAlgoPopUp->setSelectedTag(static_cast<uint32>(Shell::Random::getPrngAlgorithm(_domain)));
}

bool OptionsWidget::save() {
	const Shell::Random::PrngAlgorithm prngAlgorithm = static_cast<Shell::Random::PrngAlgorithm>(_prngAlgoPopUp->getSelectedTag());
	const bool changed = Shell::Random::getPrngAlgorithm(_domain) != prngAlgorithm ||
		ConfMan.getBool(TrinketMetaEngine::kOptionColorBlindMode, _domain) != _colorBlindMode->getState();
	ConfMan.setBool(TrinketMetaEngine::kOptionColorBlindMode, _colorBlindMode->getState(), _domain);
	ConfMan.setInt(TrinketMetaEngine::kOptionPrngAlgorithm, static_cast<int>(prngAlgorithm), _domain);
	return changed;
}

void OptionsWidget::setEnabled(bool enabled) {
	_saveFilesHdr->setEnabled(enabled);
	_colorBlindMode->setEnabled(enabled);
	_prngAlgoLabel->setEnabled(enabled);
	_prngAlgoPopUp->setEnabled(enabled);
	_manage->setEnabled(enabled);
}

void OptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) {
	if (command != kManageSaves) {
		GUI::OptionsContainerWidget::handleCommand(sender, command, data);
		return;
	}
	if (g_engine) {
		GUI::MessageDialog(_("Exit the game before managing saved games.")).runModal();
		return;
	}
	SaveManagementDialog(_domain).runModal();
}

} // namespace Zoombini3
} // namespace Trinket
