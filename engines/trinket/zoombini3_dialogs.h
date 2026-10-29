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

#ifndef TRINKET_ZOOMBINI3_DIALOGS_H
#define TRINKET_ZOOMBINI3_DIALOGS_H

#include "trinket/dialogs.h"
#include "trinket/zoombini3_saves.h"

namespace GUI {
class PopUpWidget;
}

namespace Trinket {
namespace Zoombini3 {

/** Offline Z3 save table using the native slot and practice-companion policies. */
class SaveManagementDialog : public Trinket::SaveManagementDialog {
public:
	/** Create the save manager for one ScummVM configuration target. */
	explicit SaveManagementDialog(const Common::String &target);

private:
	/** Native export filename indexed by player ID and one-based slot. */
	static constexpr const char *kExportFileFormat = "User%03u_%uGZ.dat";
	/** Reload native players and visible slots from the existing save manager. */
	void refreshSaveList() override;
	/** Execute a confirmed operation through the Z3 save manager. */
	void handleSaveCommand(uint32 command, uint player, uint slot) override;
	/** Import a native save as a new named player, as in the Z1 management modal. */
	void importSave() override;
	/** Translate the native saved location for the table. */
	static Common::U32String locationName(int location);
	/** Target-scoped transactions, launcher identities and practice companions. */
	Saves _saves;
};

class OptionsWidget : public GUI::OptionsContainerWidget {
public:
	/** Create the game-specific settings controls. */
	OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain);
	/** Load the target's saved settings into the controls. */
	void load() override;
	/** Save the controls and report whether the settings changed. */
	bool save() override;
	/** Enable or disable all settings controls. */
	void setEnabled(bool enabled) override;
	/** Open the separate save-management modal. */
	void handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) override;

protected:
	/** Arrange the settings controls in the supplied overlay. */
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &name, const Common::String &overlay) const override;

private:
	/** Commands used by this ScummVM options widget. */
	enum Command {
		kManageSaves = 'zMgm'
	};
	/** Toggle the puzzle-specific red-green color replacements. */
	GUI::CheckboxWidget *_colorBlindMode = nullptr;
	/** Label used to align the PRNG selector using its displayed text. */
	GUI::StaticTextWidget *_prngAlgoLabel = nullptr;

	/** Select the original or ScummVM standard PRNG algorithm. */
	GUI::PopUpWidget *_prngAlgoPopUp = nullptr;
	/** Button opening the target's offline save-management modal. */
	GUI::ButtonWidget *_manage = nullptr;
	/** Heading above the save-management controls. */
	GUI::StaticTextWidget *_saveFilesHdr = nullptr;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
