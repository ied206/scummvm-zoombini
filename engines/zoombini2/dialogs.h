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

#ifndef ZOOMBINI2_DIALOGS_H
#define ZOOMBINI2_DIALOGS_H

#include "common/str-array.h"
#include "gui/dialog.h"
#include "gui/widget.h"

namespace GUI {
class ButtonWidget;
class CommandSender;
class ListWidget;
class ThemeEval;
}

namespace Zoombini2 {

/** Modal ScummVM dialog for managing one target's player-profile files. */
class Zoombini2SaveManagementDialog : public GUI::Dialog {
public:
	explicit Zoombini2SaveManagementDialog(const Common::String &domain);

	void open() override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	static const uint32 kEditProfileCommand;
	static const uint32 kDeleteProfileCommand;

	void refreshProfiles(const Common::String &selectedProfile = Common::String());
	void updateButtons();
	void finishRename();
	void deleteSelectedProfile();

	Common::String _domain;
	Common::StringArray _profileNames;
	Common::String _profileNameBeforeEdit;
	GUI::ListWidget *_profileList;
	GUI::ButtonWidget *_editButton;
	GUI::ButtonWidget *_deleteButton;
	bool _editingProfile;
};

/** Engine-options entry point for the target-scoped profile manager. */
class Zoombini2OptionsWidget : public GUI::OptionsContainerWidget {
public:
	Zoombini2OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain);

	void load() override;
	bool save() override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	static const uint32 kManageProfilesCommand;

	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_DIALOGS_H
