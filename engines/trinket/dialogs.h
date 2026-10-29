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

#ifndef TRINKET_DIALOGS_H
#define TRINKET_DIALOGS_H

#include "common/noncopyable.h"
#include "common/stream.h"
#include "engines/dialogs.h"

namespace Trinket {

class TrinketEngine;

/** Modal save table shared by the game-specific offline save managers. */
class SaveManagementDialog : public GUI::Dialog, private Common::NonCopyable {
public:
	/** Create the table and actions; Z3 also exposes cloning and slot selection. */
	explicit SaveManagementDialog(bool multipleSlots);
	/** Load the target's saves when the modal opens. */
	void open() override;
	/** Fit the table, wrapping actions and centered Close button to the safe area. */
	void reflowLayout() override;
	/** Change row selection or dispatch an action to the game-specific manager. */
	void handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) override;

protected:
	/** Commands used only by the ScummVM save-management controls. */
	enum Command {
		kSelectionChanged = 'tSel',
		kRename = 'tRen',
		kClone = 'tCln',
		kImport = 'tImp',
		kExport = 'tExp',
		kDelete = 'tDel',
		kSelect = 'tCur'
	};
	/** Reload the game's registry and populate the table through addSaveRow(). */
	virtual void refreshSaveList() = 0;
	/** Perform an action on a registry player index and its one-based slot. */
	virtual void handleSaveCommand(uint32 command, uint player, uint slot) = 0;
	/** Import a new named save independently of the current table selection. */
	virtual void importSave() = 0;
	/** Hide old rows before repopulating, retaining widgets and selection. */
	void clearSaveRows();
	/** Append a registry reference and its displayed ID, name, slot and location. */
	void addSaveRow(uint player, uint slot, const Common::U32String (&labels)[4]);
	/** Restore selection and layout; a failed registry load disables all actions. */
	void finishSaveRows(bool loaded = true);
	/** Prompt for a new player name; cancellation leaves name unchanged. */
	static bool editPlayerName(Common::String &name, const Common::U32String &title, uint maxLength);
	/** Browse for an import stream and return it to the caller. */
	static Common::SeekableReadStream *openImportFile(const Common::U32String &title);
	/** Browse for an export directory and confirm an existing destination. */
	static Common::SeekableWriteStream *openExportFile(const Common::String &filename);

private:
	/** One table row; registry references remain indices into the game's save manager. */
	struct Row {
		/** Zero-based game registry player index. */
		uint player = 0;
		/** One-based game slot number. */
		uint slot = 0;
		/** Uncaptioned exclusive row selector, destroyed with the table. */
		GUI::RadiobuttonWidget *selection = nullptr;
		/** Table cells in player ID, name, game slot and location order. */
		GUI::StaticTextWidget *labels[4] = {};
	};
	/** Update actions for the current selection and offline-only operation. */
	void updateActionButtonState();
	/** Exclusive row selection group, with zero-based row values. */
	GUI::RadiobuttonGroup _selGroup;
	/** Row widgets retained across registry reloads. */
	Common::Array<Row> _rows;
	/** Number of visible rows in the current registry. */
	uint _rowCount = 0;
	/** Whether both registries were read successfully for the displayed rows. */
	bool _savesAvailable = false;
	/** Selected visible row, or -1 when the table is empty. */
	int _selectedRow = -1;
	/** Description above the action buttons. */
	GUI::StaticTextWidget *_description;
	/** Fixed table header above the scrolling rows. */
	GUI::ContainerWidget *_header;
	/** Headings in player ID, name, game slot and location order. */
	GUI::StaticTextWidget *_columns[4] = {};
	/** Scroll viewport for the save table. */
	GUI::ScrollContainerWidget *_list;
	/** Container whose height follows the visible save rows. */
	GUI::ContainerWidget *_table;
	/** Selection-dependent action buttons in display order. */
	Common::Array<GUI::ButtonWidget *> _actions;
	/** Centered button that closes the modal. */
	GUI::ButtonWidget *_close;
};

/** Runtime options that read and update the active save's volume settings. */
class GameOptionsDialog : public GUI::OptionsDialog {
public:
	/** Create runtime settings for the engine and active save. */
	explicit GameOptionsDialog(TrinketEngine &vm);
	/** Build the standard runtime settings and per-save audio controls. */
	void build() override;
	/** Apply runtime settings and write the active save's audio preferences. */
	void apply() override;
	/** Handle per-save audio controls and forward other settings commands. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	enum Command {
		kMusicVolume01 = 1,
		kEffectsVolume02 = 2,
		kSpeechVolume03 = 3,
		kMute04 = 4,
	};
	struct VolumeControl {
		/** Caption widget for one per-save mixer volume. */
		GUI::StaticTextWidget *description;
		/** Slider widget editing the volume in the range zero to one hundred. */
		GUI::SliderWidget *slider;
		/** Text widget showing the slider's current numeric value. */
		GUI::StaticTextWidget *label;
	};
	/** Engine whose active save supplies the runtime audio settings. */
	TrinketEngine &_vm;
	/** Panel for subengine-specific runtime settings. */
	GUI::OptionsContainerWidget *_engineOptions;
	/** Music, effects and speech controls in mixer-category order. */
	VolumeControl _volumes[3];
	/** Control for the active save's mute setting. */
	GUI::CheckboxWidget *_mute;
	/** Add music, effects, speech and mute controls to the supplied GUI parent. */
	void addSaveVolumeControls(GUI::GuiObject *boss);
	/** Refresh numeric labels and enabled states from the current audio settings. */
	void updateVolumeControls();
};

class TrinketMainMenuDialog : public MainMenuDialog {
public:
	/** Create the in-game menu for the engine. */
	explicit TrinketMainMenuDialog(TrinketEngine &vm);
	/** Open the Trinket runtime options dialog or dispatch a standard menu command. */
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	/** Engine used to construct its runtime options dialog. */
	TrinketEngine &_vm;
};

} // End of namespace Trinket

#endif
