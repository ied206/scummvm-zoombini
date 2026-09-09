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
} // namespace GUI

namespace Zoombini2 {

/** Modal ScummVM dialog for managing one target's player-profile files. */
class Zoombini2SaveManagementDialog : public GUI::Dialog {
public:
	/** Construct a profile manager for the target configuration @p domain. */
	explicit Zoombini2SaveManagementDialog(const Common::String &domain);

	/** Populate the profile list and enter the modal dialog. */
	void open() override;
	/** Handle list editing, deletion, and ordinary dialog commands. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	/** Command used to begin or finish editing the selected profile name. */
	static const uint32 kEditProfileCommand;
	/** Command used to delete the selected profile. */
	static const uint32 kDeleteProfileCommand;

	/** Reload the sorted profile list and optionally reselect @p selectedProfile. */
	void refreshProfiles(const Common::String &selectedProfile = Common::String());
	/** Enable or disable actions for the current selection and edit state. */
	void updateButtons();
	/** Validate and commit the profile name currently edited by the list widget. */
	void finishRename();
	/** Confirm and delete the selected profile. */
	void deleteSelectedProfile();

	/** Target configuration domain used to namespace profile files. */
	Common::String _domain;
	/** Profile names mirrored by @ref Zoombini2SaveManagementDialog::_profileList. */
	Common::StringArray _profileNames;
	/** Pre-edit profile name retained while a row is being edited. */
	Common::String _profileNameBeforeEdit;
	/** Dialog-owned profile list widget. */
	GUI::ListWidget *_profileList;
	/** Dialog-owned rename button. */
	GUI::ButtonWidget *_editButton;
	/** Dialog-owned delete button. */
	GUI::ButtonWidget *_deleteButton;
	/** Whether the selected profile row is currently editable. */
	bool _editingProfile;
};

/** Engine-options entry point for the target-scoped profile manager. */
class Zoombini2OptionsWidget : public GUI::OptionsContainerWidget {
public:
	/** Construct the options container under @p boss for @p domain. */
	Zoombini2OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain);

	/** Retain the empty options hook required by the container interface. */
	void load() override;
	/** Report that this launcher widget has no persistent option values. */
	bool save() override;
	/** Open the profile manager or forward an ordinary widget command. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	/** Command used to open @ref Zoombini2SaveManagementDialog. */
	static const uint32 kManageProfilesCommand;

	/** Define this widget's overlay-compatible GUI layout. */
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_DIALOGS_H
