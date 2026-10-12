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
#include "common/fs.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/engine.h"
#include "gui/ThemeEval.h"
#include "gui/browser.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/list.h"
#include "gui/widgets/popup.h"
#include "trinket/shell/random.h"
#include "trinket/zoombini3_saves.h"
#include "trinket/zoombini3_world.h"

namespace Trinket {
namespace Zoombini3 {

enum Command {
	kManage01 = 1,
	kRename02 = 2,
	kClone03 = 3,
	kImport04 = 4,
	kExport05 = 5,
	kDelete06 = 6,
	kSelect07 = 7,
};

class SaveDialog : public GUI::Dialog {
public:
	explicit SaveDialog(const Common::String &target) : GUI::Dialog(0, 0, 600, 400, true),
														_saves(*g_system->getSavefileManager(), target) {
		new GUI::StaticTextWidget(this, 12, 10, 576, 20, true, _("Manage saved games"), Graphics::kTextAlignStart);
		_list = new GUI::ListWidget(this, 12, 38, 576, 246, true);
		_list->setEditable(false);
		_list->setNumberingMode(GUI::kListNumberingOff);
		new GUI::StaticTextWidget(this, 12, 292, 130, 20, true, _("New player name:"), Graphics::kTextAlignStart);
		_name = new GUI::EditTextWidget(this, 146, 290, 442, 24, true, Common::U32String());
		new GUI::ButtonWidget(this, 12, 322, 90, 26, true, _("Rename"), Common::U32String(), kRename02);
		new GUI::ButtonWidget(this, 109, 322, 90, 26, true, _("Clone"), Common::U32String(), kClone03);
		new GUI::ButtonWidget(this, 206, 322, 90, 26, true, _("Import"), Common::U32String(), kImport04);
		new GUI::ButtonWidget(this, 303, 322, 90, 26, true, _("Export"), Common::U32String(), kExport05);
		new GUI::ButtonWidget(this, 400, 322, 90, 26, true, _("Delete"), Common::U32String(), kDelete06);
		new GUI::ButtonWidget(this, 497, 322, 91, 26, true, _("Select"), Common::U32String(), kSelect07);
		new GUI::ButtonWidget(this, 242, 362, 116, 26, true, _("Close"), Common::U32String(), GUI::kCloseCmd);
	}
	void reflowLayout() override {
		_w = static_cast<int>(600 * g_gui.getScaleFactor());
		_h = static_cast<int>(400 * g_gui.getScaleFactor());
		_x = MAX<int>(0, (g_system->getOverlayWidth() - _w) / 2);
		_y = MAX<int>(0, (g_system->getOverlayHeight() - _h) / 2);
		GUI::Dialog::reflowLayout();
	}
	void open() override {
		GUI::Dialog::open();
		refresh();
	}

private:
	struct Row {
		uint player, slot;
	};
	Saves _saves;
	Common::Array<Row> _rows;
	GUI::ListWidget *_list;
	GUI::EditTextWidget *_name;
	static Common::U32String location(int value) {
		switch (value) {
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
	void refresh() {
		_rows.clear();
		Common::U32StringArray labels;
		if (!_saves.load()) {
			GUI::MessageDialog(_("Could not read the player registries.")).runModal();
			_list->setList(labels);
			return;
		}
		for (uint i = 0; i < _saves.players().size(); i++) {
			const Shell::SaveFiles::Player &player = _saves.players()[i];
			for (uint slot = 1; slot <= player.count; slot++) {
				Row row;
				row.player = i;
				row.slot = slot;
				_rows.push_back(row);
				labels.push_back(Common::U32String::format(_("%s   Game %u / %u   %s%s"), player.name.c_str(),
														   slot, player.count, location(player.locations[slot - 1]).encode().c_str(), slot == player.current ? " *" : ""));
			}
		}
		_list->setList(labels);
		g_gui.scheduleTopDialogRedraw();
	}
	void handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) override {
		if (command < kRename02 || kSelect07 < command) {
			GUI::Dialog::handleCommand(sender, command, data);
			return;
		}
		int selected = _list->getSelected();
		if (selected < 0 || _rows.size() <= static_cast<uint>(selected))
			return;
		const Row row = _rows[selected];
		bool success = false;
		if (command == kRename02 || command == kClone03) {
			success = _saves.rename(row.player, _name->getEditString().encode(Common::kUtf8), command == kClone03);
		} else if (command == kDelete06) {
			GUI::MessageDialog confirm(_("Delete this player, all six game slots and the practice save?"), _("Delete"), _("Cancel"));
			if (confirm.runModal() != GUI::kMessageOK)
				return;
			success = _saves.remove(row.player);
		} else if (command == kSelect07) {
			success = _saves.select(row.player, row.slot);
		} else if (command == kImport04) {
			GUI::BrowserDialog browser(_("Select a Zoombinis: Island Odyssey User...GZ.dat saved game"), false);
			if (browser.runModal() <= 0)
				return;
			GUI::MessageDialog confirm(_("Replace the selected game slot?"), _("Replace"), _("Cancel"));
			if (confirm.runModal() != GUI::kMessageOK)
				return;
			Common::ScopedPtr<Common::SeekableReadStream> stream(browser.getResult().createReadStream());
			success = stream && _saves.importSlot(row.player, row.slot, *stream);
		} else if (command == kExport05) {
			GUI::BrowserDialog browser(_("Select the directory for the exported saved game"), true);
			if (browser.runModal() <= 0)
				return;
			Common::String file = Common::String::format("User%03u_%uGZ.dat", _saves.players()[row.player].id, row.slot);
			Common::FSNode destination = browser.getResult().getChild(file);
			if (destination.exists()) {
				GUI::MessageDialog confirm(_("The exported file already exists. Replace it?"), _("Replace"), _("Cancel"));
				if (confirm.runModal() != GUI::kMessageOK)
					return;
			}
			Common::ScopedPtr<Common::SeekableWriteStream> stream(destination.createWriteStream());
			success = stream && _saves.exportSlot(row.player, row.slot, *stream);
			if (stream) {
				stream->finalize();
				success = success && !stream->err();
			}
		}
		if (!success) {
			if (_saves.rollbackFailed())
				GUI::MessageDialog(_("The operation failed and some files could not be restored. Check your save directory before continuing.")).runModal();
			else
				GUI::MessageDialog(_("The operation failed. Choose a normal player for save operations and a unique name of 1 to 21 ASCII characters.")).runModal();
		}
		refresh();
	}
};

OptionsWidget::OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain) : GUI::OptionsContainerWidget(boss, name, "TrinketZoombini3Options", domain) {
	const Common::U32String prngTooltip = _("Selects the original game's PRNG or ScummVM's standard PRNG. "
											"Changing this option reseeds the PRNG immediately.");
	_prngAlgorithmLabel = new GUI::StaticTextWidget(widgetsBoss(), "TrinketZoombini3Options.PrngAlgorithmLabel", _("PRNG algorithm:"), prngTooltip);
	_prngAlgorithmLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_prngAlgorithmPopUp = new GUI::PopUpWidget(widgetsBoss(), "TrinketZoombini3Options.PrngAlgorithm", prngTooltip);
	_prngAlgorithmPopUp->appendEntry(_("Original PRNG"), static_cast<uint32>(Shell::Random::PrngAlgorithm::kOriginalPrng));
	_prngAlgorithmPopUp->appendEntry(_("ScummVM Standard PRNG"), static_cast<uint32>(Shell::Random::PrngAlgorithm::kStandardPrng));
	_manage = new GUI::ButtonWidget(widgetsBoss(), "TrinketZoombini3Options.Saves", _("Saved games"), Common::U32String(), kManage01);
	_manage->setTarget(this);
}

void OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &name, const Common::String &overlay) const {
	const int lineHeight = layouts.getVar("Globals.Line.Height");
	static constexpr int kOptionLabelPadding = 4;
	const int prngLabelWidth = g_gui.getStringWidth(_prngAlgorithmLabel->getLabel()) + kOptionLabelPadding;
	layouts.addDialog(name, overlay).addLayout(GUI::ThemeLayout::kLayoutVertical).addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12).addPadding(0, 0, 0, 0).addWidget("PrngAlgorithmLabel", "", prngLabelWidth, lineHeight).addWidget("PrngAlgorithm", "PopUp").closeLayout().addWidget("Saves", "Button").closeLayout().closeDialog();
}

void OptionsWidget::load() {
	_prngAlgorithmPopUp->setSelectedTag(static_cast<uint32>(Shell::Random::getPrngAlgorithm(_domain)));
}

bool OptionsWidget::save() {
	const Shell::Random::PrngAlgorithm prngAlgorithm = static_cast<Shell::Random::PrngAlgorithm>(_prngAlgorithmPopUp->getSelectedTag());
	const bool changed = Shell::Random::getPrngAlgorithm(_domain) != prngAlgorithm;
	ConfMan.setInt(Shell::Random::kPrngAlgorithmOption, static_cast<int>(prngAlgorithm), _domain);
	return changed;
}

void OptionsWidget::setEnabled(bool enabled) {
	_prngAlgorithmLabel->setEnabled(enabled);
	_prngAlgorithmPopUp->setEnabled(enabled);
	_manage->setEnabled(enabled);
}

void OptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) {
	if (command != kManage01) {
		GUI::OptionsContainerWidget::handleCommand(sender, command, data);
		return;
	}
	if (g_engine) {
		GUI::MessageDialog(_("Exit the game before managing saved games.")).runModal();
		return;
	}
	SaveDialog(_domain).runModal();
}

} // namespace Zoombini3
} // namespace Trinket
