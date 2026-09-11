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

#include "common/fs.h"
#include "common/str-array.h"
#include "engines/dialogs.h"
#include "gui/dialog.h"
#include "gui/widget.h"

namespace GUI {
class ButtonWidget;
class CheckboxWidget;
class CommandSender;
class ContainerWidget;
class EditTextWidget;
class RadiobuttonGroup;
class RadiobuttonWidget;
class ScrollContainerWidget;
class StaticTextWidget;
class ThemeEval;
} // namespace GUI

namespace Zoombini2 {

class Zoombini2Engine;

/** In-game ScummVM menu that applies Zoombini2 settings when its options dialog closes. */
class Zoombini2MenuDialog : public MainMenuDialog {
public:
	/** Bind the menu to the running @p vm instance. */
	explicit Zoombini2MenuDialog(Zoombini2Engine *vm);

	/** Apply game and sound settings immediately after the nested options dialog closes. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	/** Borrowed vm used to apply game-specific settings. */
	Zoombini2Engine *_vm;
};

/** Modal dialog for renaming a Zoombini2 profile. */
class Zoombini2ProfileNameDialog : public GUI::Dialog {
public:
	/** Construct the editor with @p initialName. */
	explicit Zoombini2ProfileNameDialog(const Common::U32String &initialName);

	/** Return the entered profile name. */
	Common::U32String getProfileName() const;
	/** Keep the editor centered in the current overlay. */
	void reflowLayout() override;
	/** Accept or cancel the edited profile name. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	/** Profile-name editor managed by this dialog. */
	GUI::EditTextWidget *_edit = nullptr;
};

/** Modal ScummVM dialog for managing one target's player-profile files. */
class Zoombini2SaveManagementDialog : public GUI::Dialog {
public:
	/** Construct a profile manager for the target configuration @p domain. */
	explicit Zoombini2SaveManagementDialog(const Common::String &domain);

	/** Populate the profile table and enter the modal dialog. */
	void open() override;
	/** Keep the save-management table centered in the current overlay. */
	void reflowLayout() override;
	/** Handle table selection, profile actions, and ordinary dialog commands. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	static constexpr int kMaximumProfileRows = 99;
	static constexpr int kSelectionX = 0;
	static constexpr int kSelectionWidth = 24;
	static constexpr int kNameX = kSelectionX + kSelectionWidth;
	static constexpr int kNameWidth = 106;
	static constexpr int kZombinivilleX = kNameX + kNameWidth;
	static constexpr int kZombinivilleWidth = 92;
	static constexpr int kRescue1X = kZombinivilleX + kZombinivilleWidth;
	static constexpr int kRescueWidth = 72;
	static constexpr int kRescue2X = kRescue1X + kRescueWidth;
	static constexpr int kBooliewoodX = kRescue2X + kRescueWidth;
	static constexpr int kBooliewoodWidth = 92;
	static constexpr int kActivePartyX = kBooliewoodX + kBooliewoodWidth;
	static constexpr int kActivePartyWidth = 64;
	static constexpr int kTableWidth = kActivePartyX + kActivePartyWidth;
	static constexpr int kTableRowHeight = 24;
	static constexpr int kDialogMargin = 12;
	static constexpr int kActionButtonsTop = 34;
	static constexpr int kHeaderTop = 68;
	static constexpr int kListTop = 92;
	static constexpr int kListHeight = 196;
	static constexpr int kBottomButtonsTop = 300;
	static constexpr int kButtonHeight = 28;
	static constexpr int kDialogHeight = 344;

	/** Command used to rename the selected profile. */
	static const uint32 kEditProfileCommand;
	/** Command used to delete the selected profile. */
	static const uint32 kDeleteProfileCommand;
	/** Command used to import one independent Z2 save file. */
	static const uint32 kImportProfileCommand;
	/** Command used to export the selected independent Z2 save file. */
	static const uint32 kExportProfileCommand;
	/** Command emitted when a profile-table radio button changes selection. */
	static const uint32 kProfileSelectionChangedCommand;

	/** Reload the sorted profile table and optionally reselect @p selectedProfile. */
	void refreshProfiles(const Common::String &selectedProfile = Common::String());
	/** Resize the scrollable row container to its populated row count. */
	void updateProfileTableLayout();
	/** Enable or disable actions for the current selection. */
	void updateButtons();
	/** Prompt for and commit a new name for the selected profile. */
	void renameSelectedProfile();
	/** Choose, validate, and import one external Z2 .mk file. */
	void importProfile();
	/** Copy the selected profile to an external Z2 .mk file. */
	void exportSelectedProfile();
	/** Confirm and delete the selected profile. */
	void deleteSelectedProfile();
	/** Return an existing case-insensitive child, or the normal child when absent. */
	static Common::FSNode findChild(const Common::FSNode &directory, const Common::String &name);
	/** Scale a fixed dialog measurement for the active GUI scale. */
	static int scaleDialogValue(int value);

	/** Target configuration domain used to namespace profile files. */
	Common::String _domain;
	/** Profile names mirrored by the visible profile rows. */
	Common::StringArray _profileNames;
	/** Exclusive selection group for the profile-table rows. */
	GUI::RadiobuttonGroup _profileSelectionGroup;
	/** Selected row in @ref Zoombini2SaveManagementDialog::_profileNames, or -1. */
	int _selectedProfileIndex = -1;
	/** Scroll container holding profile rows. */
	GUI::ScrollContainerWidget *_profileList = nullptr;
	/** Container holding the fixed-column row widgets. */
	GUI::ContainerWidget *_profileTable = nullptr;
	/** Number of rows currently populated in the table. */
	int _profileRowCount = 0;
	/** Rename button managed by this dialog. */
	GUI::ButtonWidget *_editButton = nullptr;
	/** Import button managed by this dialog. */
	GUI::ButtonWidget *_importButton = nullptr;
	/** Export button managed by this dialog. */
	GUI::ButtonWidget *_exportButton = nullptr;
	/** Delete button managed by this dialog. */
	GUI::ButtonWidget *_deleteButton = nullptr;
	/** Uncaptioned selection controls for visible profile rows. */
	GUI::RadiobuttonWidget *_profileSelectionButtons[kMaximumProfileRows] = {};
	/** Profile-name cells. */
	GUI::StaticTextWidget *_profileNameLabels[kMaximumProfileRows] = {};
	/** Zombiniville-stage population cells. */
	GUI::StaticTextWidget *_zombinivilleLabels[kMaximumProfileRows] = {};
	/** Rescue Site I population cells. */
	GUI::StaticTextWidget *_rescue1Labels[kMaximumProfileRows] = {};
	/** Rescue Site II population cells. */
	GUI::StaticTextWidget *_rescue2Labels[kMaximumProfileRows] = {};
	/** Booliewood population cells. */
	GUI::StaticTextWidget *_booliewoodLabels[kMaximumProfileRows] = {};
	/** Serialized active-party population cells. */
	GUI::StaticTextWidget *_activePartyLabels[kMaximumProfileRows] = {};
	/** Whether each visible profile row contains a valid parsed .mk stream. */
	bool _profileStateValid[kMaximumProfileRows] = {};
};

/** Engine-options entry point for target-scoped profiles and compatibility switches. */
class Zoombini2OptionsWidget : public GUI::OptionsContainerWidget {
public:
	/** Construct the options container under @p boss for @p domain. */
	Zoombini2OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain);

	/** Load every compatibility switch from the target configuration. */
	void load() override;
	/** Store every compatibility switch in the target configuration. */
	bool save() override;
	/** Open the profile manager or forward an ordinary widget command. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	/** Thin themed separator used to divide option groups. */
	class SeparatorWidget;
	/** Command used to open @ref Zoombini2SaveManagementDialog. */
	static const uint32 kManageProfilesCommand;
	/** Toggle for the F2, F3, P, and Chez Norf C developer keys. */
	GUI::CheckboxWidget *_debugHotkeysCheckbox = nullptr;
	/** Toggle selecting stereo rather than mono game-audio streams. */
	GUI::CheckboxWidget *_stereoOutputCheckbox = nullptr;
	/** Toggle selecting the alternate level-one Waterslide pairing. */
	GUI::CheckboxWidget *_greedyWaterslideCheckbox = nullptr;
	/** Toggle selecting one gameplay-clock snapshot per rendered frame. */
	GUI::CheckboxWidget *_cachedFrameTimeCheckbox = nullptr;
	/** Toggle selecting the original Windows random-number generator. */
	GUI::CheckboxWidget *_originalPrngCheckbox = nullptr;
	/** Toggle selecting floating-point rather than original Q10 Bezier calculations. */
	GUI::CheckboxWidget *_floatingPointPathsCheckbox = nullptr;

	/** Define this widget's overlay-compatible GUI layout. */
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_DIALOGS_H
