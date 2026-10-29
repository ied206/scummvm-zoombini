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

#ifndef TRINKET_RR1CLOUD9_DIALOGS_H
#define TRINKET_RR1CLOUD9_DIALOGS_H

#include "common/noncopyable.h"
#include "trinket/dialogs.h"
#include "trinket/rr1cloud9/saves.h"

namespace GUI {
class PopUpWidget;
}

namespace Trinket {
namespace RR1Cloud9 {

/** Offline RR1Cloud9 save table using its portable single-slot codec. */
class SaveManagementDialog : public Trinket::SaveManagementDialog {
public:
	/** Create the save manager for one ScummVM configuration target. */
	explicit SaveManagementDialog(const Common::String &target);

private:
	/** Portable export filename indexed by player ID. */
	static constexpr const char *kExportFileFormat = "User%03u_1GZ.dat";
	/** Reload RR player records and populate their single checkpoint rows. */
	void refreshSaveList() override;
	/** Execute a confirmed operation through the RR save manager. */
	void handleSaveCommand(uint32 command, uint player, uint slot) override;
	/** Import a portable checkpoint as a new named player. */
	void importSave() override;
	/** Target-scoped RR checkpoints and launcher identities. */
	Saves _saves;
};

/** ScummVM engine options for Reader Rabbit: Capers on Cloud Nine. */
class OptionsWidget : public GUI::OptionsContainerWidget, private Common::NonCopyable {
public:
	/** Create the game-specific settings controls. */
	OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain);
	/** Load the target's saved settings into the controls. */
	void load() override;
	/** Save the controls to the target's settings. */
	bool save() override;
	/** Enable or disable all settings controls. */
	void setEnabled(bool enabled) override;
	/** Open the separate offline save-management modal. */
	void handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) override;

private:
	/** Commands used only by the ScummVM options widget. */
	enum Command {
		kManageSaves = 'rMgm'
	};
	/** Arrange the settings controls in the supplied overlay. */
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	/** Toggle the puzzle-specific red-green color replacements. */
	GUI::CheckboxWidget *_colorBlindMode;
	/** Label describing the persisted startup selector. */
	GUI::StaticTextWidget *_startupModeLabel;
	/** Select direct sign-in or Learning Launcher startup. */
	GUI::PopUpWidget *_startupModePopUp;
	/** Heading above the save-management controls. */
	GUI::StaticTextWidget *_saveFilesHdr;
	/** Open the configuration target's save-management modal. */
	GUI::ButtonWidget *_manage;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
