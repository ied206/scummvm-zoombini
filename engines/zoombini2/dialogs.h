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

#ifndef ZOOMBINI2_DIALOGS_H
#define ZOOMBINI2_DIALOGS_H

#include "common/fs.h"
#include "common/language.h"
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
class PopUpWidget;
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

/** Modal dialog for entering a Zoombini2 savefile name. */
class Zoombini2SavefileNameDialog : public GUI::Dialog {
public:
	/** Construct the editor titled @p title with @p initialName. */
	Zoombini2SavefileNameDialog(const Common::U32String &title, const Common::U32String &initialName, Common::Language language);

	/** Return the entered savefile name. */
	Common::U32String getSavefileName() const;
	/** Keep the editor centered in the current overlay. */
	void reflowLayout() override;
	/** Accept or cancel the edited savefile name. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	/** Savefile-name editor managed by this dialog. */
	GUI::EditTextWidget *_edit = nullptr;
};

/** Modal ScummVM dialog for managing one target's independent .mk savefiles. */
class Zoombini2SaveManagementDialog : public GUI::Dialog {
public:
	/** Construct the save-management dialog for target configuration @p domain. */
	explicit Zoombini2SaveManagementDialog(const Common::String &domain);

	/** Populate the savefile table and enter the modal dialog. */
	void open() override;
	/** Keep the save-management table centered in the current overlay. */
	void reflowLayout() override;
	/** Handle table selection, savefile actions, and ordinary dialog commands. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	static constexpr int kMaximumSavefileRows = 99;
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

	/** Command used to rename the selected savefile. */
	static constexpr uint32 kEditSavefileCommand = 'z2ed';
	/** Command used to clone the selected savefile. */
	static constexpr uint32 kDuplicateSavefileCommand = 'z2cp';
	/** Command used to delete the selected savefile. */
	static constexpr uint32 kDeleteSavefileCommand = 'z2dl';
	/** Command used to import one independent Z2 save file. */
	static constexpr uint32 kImportSavefileCommand = 'z2im';
	/** Command used to export the selected independent Z2 save file. */
	static constexpr uint32 kExportSavefileCommand = 'z2ex';
	/** Command emitted when a savefile-table radio button changes selection. */
	static constexpr uint32 kSavefileSelectionChangedCommand = 'z2sl';

	/** Reload the sorted savefile table and optionally reselect @p selectedSavefile. */
	void refreshSavefiles(const Common::String &selectedSavefile = Common::String());
	/** Resize the scrollable row container to its populated row count. */
	void updateSavefileTableLayout();
	/** Enable or disable actions for the current selection. */
	void updateButtons();
	/** Return whether this target's running engine will automatically write @p savefileName. */
	bool isActiveSavefile(const Common::String &savefileName) const;
	/** Prompt for and commit a new name for the selected savefile. */
	void renameSelectedSavefile();
	/** Prompt for and create a distinct copy of the selected savefile. */
	void duplicateSelectedSavefile();
	/** Choose, validate, and import one external Z2 .mk file. */
	void importSavefile();
	/** Copy the selected savefile to an external Z2 .mk file. */
	void exportSelectedSavefile();
	/** Confirm and delete the selected savefile. */
	void deleteSelectedSavefile();
	/** Return an existing case-insensitive child, or the normal child when absent. */
	static Common::FSNode findChild(const Common::FSNode &directory, const Common::String &name);
	/** Scale a fixed dialog measurement for the active GUI scale. */
	static int scaleDialogValue(int value);

	/** Target configuration domain whose savefiles this dialog manages. */
	Common::String _domain;
	/** Language selected for savefile-name encoding and input validation. */
	Common::Language _language = Common::UNK_LANG;
	/** Savefile names mirrored by the visible savefile rows. */
	Common::StringArray _savefileNames;
	/** Exclusive selection group for the savefile-table rows. */
	GUI::RadiobuttonGroup _savefileSelectionGroup;
	/** Selected row in @ref Zoombini2SaveManagementDialog::_savefileNames, or -1. */
	int _selectedSavefileIndex = -1;
	/** Scroll container holding savefile rows. */
	GUI::ScrollContainerWidget *_savefileList = nullptr;
	/** Container holding the fixed-column row widgets. */
	GUI::ContainerWidget *_savefileTable = nullptr;
	/** Number of rows currently populated in the table. */
	int _savefileRowCount = 0;
	/** Rename button managed by this dialog. */
	GUI::ButtonWidget *_editButton = nullptr;
	/** Clone button managed by this dialog. */
	GUI::ButtonWidget *_duplicateButton = nullptr;
	/** Import button managed by this dialog. */
	GUI::ButtonWidget *_importButton = nullptr;
	/** Export button managed by this dialog. */
	GUI::ButtonWidget *_exportButton = nullptr;
	/** Delete button managed by this dialog. */
	GUI::ButtonWidget *_deleteButton = nullptr;
	/** Uncaptioned selection controls for visible savefile rows. */
	GUI::RadiobuttonWidget *_savefileSelectionButtons[kMaximumSavefileRows] = {};
	/** Savefile-name cells. */
	GUI::StaticTextWidget *_savefileNameLabels[kMaximumSavefileRows] = {};
	/** Zombiniville-stage population cells. */
	GUI::StaticTextWidget *_zombinivilleLabels[kMaximumSavefileRows] = {};
	/** Rescue Site I population cells. */
	GUI::StaticTextWidget *_rescue1Labels[kMaximumSavefileRows] = {};
	/** Rescue Site II population cells. */
	GUI::StaticTextWidget *_rescue2Labels[kMaximumSavefileRows] = {};
	/** Booliewood population cells. */
	GUI::StaticTextWidget *_booliewoodLabels[kMaximumSavefileRows] = {};
	/** Serialized active-party population cells. */
	GUI::StaticTextWidget *_activePartyLabels[kMaximumSavefileRows] = {};
	/** Whether each visible savefile row contains a valid parsed .mk stream. */
	bool _savefileStateValid[kMaximumSavefileRows] = {};
};

/** Engine-options entry point for target-scoped savefiles and compatibility switches. */
class Zoombini2OptionsWidget : public GUI::OptionsContainerWidget {
public:
	/** Construct the options container under @p boss for @p domain. */
	Zoombini2OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain);

	/** Load every compatibility switch from the target configuration. */
	void load() override;
	/** Store every compatibility switch in the target configuration. */
	bool save() override;
	/** Open the savefile manager or forward an ordinary widget command. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	/** Thin themed separator used to divide option groups. */
	class SeparatorWidget;
	/** Numeric frame-rate entry. */
	class FrameRateNumberBox;
	/** Command used to open @ref Zoombini2SaveManagementDialog. */
	static constexpr uint32 kManageSavefilesCommand = 'z2mg';
	static constexpr uint32 kUnlockFrameRateCommand = 'z2ul';
	/** Enable per-savefile write-lock controls for repeatable tests. */
	GUI::CheckboxWidget *_savefileReadOnlyToggleCheckbox = nullptr;
	/** Toggle selecting stereo rather than mono game-audio streams. */
	GUI::CheckboxWidget *_stereoOutputCheckbox = nullptr;
	/** Toggle selecting floating-point rather than original Q10 Bezier calculations. */
	GUI::CheckboxWidget *_floatingPointPathsCheckbox = nullptr;
	/** Toggle hiding the stray streaks in the Fleen departure animation. */
	GUI::CheckboxWidget *_fixFleenDepartureStreakCheckbox = nullptr;
	/** Toggle enabling the enhanced keyboard shortcut set. */
	GUI::CheckboxWidget *_enhancedKbdShortcutsCheckbox = nullptr;
	/** Toggle keying the solid color from affected help-sheet bitmaps. */
	GUI::CheckboxWidget *_transparentHelpPagesCheckbox = nullptr;
	/** Select the color-only presentation used for noses and colored puzzle pieces. */
	GUI::PopUpWidget *_colorAssistPopUp = nullptr;
	/** Toggle for the F2, F3, P, and Chez Norf C developer keys. */
	GUI::CheckboxWidget *_debugHotkeysCheckbox = nullptr;
	/** Toggle selecting the alternate level-one Waterslide pairing. */
	GUI::CheckboxWidget *_greedyWaterslideCheckbox = nullptr;
	/** Toggle protecting Aqua Cube's first direct lever move from a Fleen. */
	GUI::CheckboxWidget *_aquacubeSafeFirstMoveCheckbox = nullptr;
	/** Toggle access to recoverable level 4 puzzles from the practice map. */
	GUI::CheckboxWidget *_allowCutLevel4PracticePuzzlesCheckbox = nullptr;
	/** Toggle selecting the original Windows random-number generator. */
	GUI::CheckboxWidget *_originalPrngCheckbox = nullptr;
	/** Logic pacing selector showing the 60Hz LCD and 75Hz CRT choices. */
	GUI::PopUpWidget *_pacingPopUp = nullptr;
	/** Frame-rate limit in frames per second. */
	FrameRateNumberBox *_frameRateNumberBox = nullptr;
	/** Disable the engine-side frame-rate limit. */
	GUI::CheckboxWidget *_unlockFrameRateCheckbox = nullptr;

	/** Define this widget's overlay-compatible GUI layout. */
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const override;
	void updateFrameRateControls();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_DIALOGS_H
