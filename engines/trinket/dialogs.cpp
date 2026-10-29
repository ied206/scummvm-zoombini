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

#include "trinket/dialogs.h"

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/gui_options.h"
#include "common/system.h"
#include "common/translation.h"
#include "engines/achievements.h"
#include "engines/engine.h"
#include "engines/metaengine.h"
#include "gui/ThemeEval.h"
#include "gui/browser.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/scrollcontainer.h"
#include "gui/widgets/tab.h"
#include "trinket/trinket.h"

namespace Trinket {

namespace {

/** Separate name-entry modal used by save rename and clone actions. */
class SaveNameDialog : public GUI::Dialog {
public:
	/** Show the current name and the game's byte limit before accepting an edit. */
	SaveNameDialog(const Common::String &name, const Common::U32String &title, uint maxLength) : GUI::Dialog(0, 0, 1, 1) {
		_title = new GUI::StaticTextWidget(this, 0, 0, 1, 1, title, Graphics::kTextAlignStart);
		// I18N: %u is the maximum number of single-byte player name characters.
		_desc = new GUI::StaticTextWidget(this, 0, 0, 1, 1,
										  Common::U32String::format(_("Use 1 to %u ASCII characters."), maxLength), Graphics::kTextAlignStart);
		_edit = new GUI::EditTextWidget(this, 0, 0, 1, 1, Common::U32String(name, Common::kUtf8));
		_ok = new GUI::ButtonWidget(this, 0, 0, 1, 1, _("OK"), Common::U32String(), GUI::kOKCmd);
		_cancel = new GUI::ButtonWidget(this, 0, 0, 1, 1, _("Cancel"), Common::U32String(), GUI::kCloseCmd);
	}
	/** Fit name entry to the safe overlay area and current font metrics. */
	void reflowLayout() override {
		const Common::Rect safeArea = g_system->getSafeOverlayArea();
		const int margin = MAX(4, static_cast<int>(8 * g_gui.getScaleFactor()));
		const int gap = MAX(2, static_cast<int>(4 * g_gui.getScaleFactor()));
		const int lineHeight = g_gui.getFontHeight() + gap;
		const int buttonHeight = MAX(g_gui.xmlEval()->getVar("Globals.Button.Height", 0), lineHeight + gap);
		_w = MIN<int>(safeArea.width() - 2 * gap, static_cast<int>(360 * g_gui.getScaleFactor()));
		_h = MIN<int>(safeArea.height(), 2 * margin + 2 * lineHeight + 2 * buttonHeight + 3 * gap);
		_x = safeArea.left + (safeArea.width() - _w) / 2;
		_y = safeArea.top + (safeArea.height() - _h) / 2;
		_title->setPos(margin, margin);
		_title->setSize(_w - 2 * margin, lineHeight);
		_desc->setPos(margin, margin + lineHeight + gap);
		_desc->setSize(_w - 2 * margin, lineHeight);
		_desc->setTooltip(_desc->getLabel());
		_edit->setPos(margin, margin + 2 * (lineHeight + gap));
		_edit->setSize(_w - 2 * margin, buttonHeight);
		const int buttonWidth = (_w - 2 * margin - gap) / 2;
		_ok->setPos(margin, _h - margin - buttonHeight);
		_ok->setSize(buttonWidth, buttonHeight);
		_cancel->setPos(margin + buttonWidth + gap, _h - margin - buttonHeight);
		_cancel->setSize(buttonWidth, buttonHeight);
		GUI::Dialog::reflowLayout();
	}
	/** Return the accepted or cancelled command to the caller, matching the save-name modal. */
	void handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) override {
		if (command == GUI::kOKCmd || command == GUI::kCloseCmd) {
			setResult(command);
			close();
		} else {
			GUI::Dialog::handleCommand(sender, command, data);
		}
	}
	/** Return the edited player name as UTF-8 for game-specific validation. */
	Common::String name() const { return _edit->getEditString().encode(Common::kUtf8); }

private:
	/** Rename or clone heading. */
	GUI::StaticTextWidget *_title;
	/** Name length and character guidance. */
	GUI::StaticTextWidget *_desc;
	/** Editable player name. */
	GUI::EditTextWidget *_edit;
	/** Accept the edited name. */
	GUI::ButtonWidget *_ok;
	/** Dismiss the edit without changing the save. */
	GUI::ButtonWidget *_cancel;
};

} // End of anonymous namespace

SaveManagementDialog::SaveManagementDialog(bool multipleSlots) : GUI::Dialog(0, 0, 1, 1), _selGroup(this, kSelectionChanged) {
	_description = new GUI::StaticTextWidget(this, 0, 0, 1, 1, _("Manage savefiles"), Graphics::kTextAlignStart);
	_header = new GUI::ContainerWidget(this, 0, 0, 1, 1);
	_header->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);
	const Common::U32String headings[] = {_("ID"), _("Name"), _("Game"), _("Location")};
	for (uint i = 0; i < ARRAYSIZE(headings); i++)
		_columns[i] = new GUI::StaticTextWidget(_header, 0, 0, 1, 1, headings[i], Graphics::kTextAlignCenter);
	_list = new GUI::ScrollContainerWidget(this, 0, 0, 1, 1);
	_table = new GUI::ContainerWidget(_list, 0, 0, 1, 0);
	const Common::U32String labels[] = {_("Rename"), _("Clone"), _("Import"), _("Export"), _("Delete"), _("Select")};
	const Common::U32String tooltips[] = {
		_("Rename the selected player."), _("Clone the selected player and all saved games."),
		_("Import a saved game as a new player."), _("Export only the selected game slot."),
		_("Delete the selected player and all saved games."), _("Set the selected game as this player's current slot.")};
	const uint32 commands[] = {kRename, kClone, kImport, kExport, kDelete, kSelect};
	for (uint i = 0; i < ARRAYSIZE(commands); i++) {
		if (!multipleSlots && (commands[i] == kClone || commands[i] == kSelect))
			continue;
		GUI::ButtonWidget *button = new GUI::ButtonWidget(this, 0, 0, 1, 1, labels[i], tooltips[i], commands[i]);
		button->setTarget(this);
		_actions.push_back(button);
	}
	_close = new GUI::ButtonWidget(this, 0, 0, 1, 1, _("Close"), Common::U32String(), GUI::kCloseCmd);
}

void SaveManagementDialog::open() {
	GUI::Dialog::open();
	if (g_engine) {
		GUI::MessageDialog(_("Exit the game before managing saved games.")).runModal();
		close();
		return;
	}
	refreshSaveList();
}

void SaveManagementDialog::clearSaveRows() {
	_rowCount = 0;
	for (Row &row : _rows) {
		row.selection->setVisible(false);
		for (GUI::StaticTextWidget *label : row.labels)
			label->setVisible(false);
	}
}

void SaveManagementDialog::addSaveRow(uint player, uint slot, const Common::U32String (&labels)[4]) {
	if (_rows.size() == _rowCount) {
		Row row;
		row.selection = new GUI::RadiobuttonWidget(_table, 0, 0, 1, 1, &_selGroup, _rowCount, Common::U32String());
		for (uint i = 0; i < ARRAYSIZE(row.labels); i++)
			row.labels[i] = new GUI::StaticTextWidget(_table, 0, 0, 1, 1, Common::U32String(), Graphics::kTextAlignLeft,
													  Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_rows.push_back(row);
	}
	Row &row = _rows[_rowCount];
	row.player = player;
	row.slot = slot;
	row.selection->setVisible(true);
	for (uint i = 0; i < ARRAYSIZE(row.labels); i++) {
		row.labels[i]->setLabel(labels[i]);
		row.labels[i]->setTooltip(labels[i]);
		row.labels[i]->setVisible(true);
	}
	_rowCount += 1;
}

void SaveManagementDialog::finishSaveRows(bool loaded) {
	_savesAvailable = loaded;
	if (_rowCount == 0)
		_selectedRow = -1;
	else
		_selectedRow = CLIP<int>(_selectedRow, 0, _rowCount - 1);
	_selGroup.setValue(_selectedRow);
	reflowLayout();
	updateActionButtonState();
	g_gui.scheduleTopDialogRedraw();
}

void SaveManagementDialog::reflowLayout() {
	const Common::Rect safeArea = g_system->getSafeOverlayArea();
	const float scale = g_gui.getScaleFactor();
	const int outerMargin = MAX(1, static_cast<int>(4 * scale));
	const int margin = MAX(2, static_cast<int>(6 * scale));
	const int gap = MAX(1, static_cast<int>(4 * scale));
	const int padding = MAX(2, static_cast<int>(6 * scale));
	const Graphics::Font &font = g_gui.getFont(GUI::ThemeEngine::kFontStyleNormal);
	const int fontHeight = MAX(g_gui.getFontHeight(), font.getFontHeight());
	const int lineHeight = fontHeight + gap;
	const int buttonHeight = MAX(g_gui.xmlEval()->getVar("Globals.Button.Height", 0), fontHeight + padding);
	const int scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", static_cast<int>(9 * scale));
	_w = MIN<int>(safeArea.width() - 2 * outerMargin, static_cast<int>(524 * scale) + scrollbarWidth);
	_h = MIN<int>(safeArea.height(), static_cast<int>(344 * scale));
	_x = safeArea.left + (safeArea.width() - _w) / 2;
	_y = safeArea.top + (safeArea.height() - _h) / 2;
	const int contentWidth = _w - 2 * margin;
	const int tableWidth = contentWidth - scrollbarWidth;
	_description->setPos(margin, margin);
	_description->setSize(contentWidth, lineHeight);
	_description->setTooltip(_description->getLabel());
	int actionX = margin;
	int actionY = margin + lineHeight + gap;
	for (GUI::ButtonWidget *button : _actions) {
		const int width = MIN(MAX(g_gui.getStringWidth(button->getLabel()) + 2 * padding, static_cast<int>(22 * scale)),
							  (contentWidth - 3 * gap) / 4);
		if (margin < actionX && margin + contentWidth < actionX + width) {
			actionX = margin;
			actionY += buttonHeight + gap;
		}
		button->setPos(actionX, actionY);
		button->setSize(width, buttonHeight);
		actionX += width + gap;
	}
	const int selectionWidth = font.getFontHeight() + padding;
	int widths[] = {
		MAX(font.getStringWidth(Common::U32String(U"9999999")), g_gui.getStringWidth(_columns[0]->getLabel())) + padding,
		static_cast<int>(88 * scale),
		MAX(font.getStringWidth(Common::U32String(U"6 / 6 *")), g_gui.getStringWidth(_columns[2]->getLabel())) + padding,
		static_cast<int>(160 * scale)};
	const bool stacked = tableWidth < selectionWidth + widths[0] + widths[1] + widths[2] + widths[3];
	int rowHeight = lineHeight;
	if (stacked) {
		rowHeight += lineHeight;
		widths[3] = tableWidth - selectionWidth;
	}
	int fixedWidth = selectionWidth + widths[0] + widths[2];
	if (!stacked)
		fixedWidth += widths[3];
	widths[1] = MAX(1, tableWidth - fixedWidth);
	int x = selectionWidth;
	for (uint i = 0; i < ARRAYSIZE(widths); i++) {
		int y = 0;
		if (stacked && i == 3) {
			x = selectionWidth;
			y = lineHeight;
		}
		_columns[i]->setPos(x, y);
		_columns[i]->setSize(widths[i], lineHeight);
		_columns[i]->setTooltip(_columns[i]->getLabel());
		for (uint row = 0; row < _rowCount; row++) {
			_rows[row].labels[i]->setPos(x, row * rowHeight + y);
			_rows[row].labels[i]->setSize(widths[i], lineHeight);
		}
		x += widths[i];
	}
	for (uint row = 0; row < _rowCount; row++) {
		_rows[row].selection->setPos(0, row * rowHeight + (lineHeight - font.getFontHeight()) / 2);
		_rows[row].selection->setSize(selectionWidth, font.getFontHeight());
	}
	const int headerY = actionY + buttonHeight + gap;
	_header->setPos(margin, headerY);
	_header->setSize(tableWidth, rowHeight);
	_table->setSize(tableWidth, _rowCount * rowHeight);
	const int footerY = _h - margin - buttonHeight;
	const int closeWidth = MIN(contentWidth, MAX(g_gui.getStringWidth(_close->getLabel()) + 2 * padding,
												 g_gui.xmlEval()->getVar("Globals.Button.Width", 0)));
	_close->setPos(margin + (contentWidth - closeWidth) / 2, footerY);
	_close->setSize(closeWidth, buttonHeight);
	_close->setTooltip(_close->getLabel());
	const int listY = headerY + rowHeight;
	_list->setPos(margin, listY);
	_list->setSize(tableWidth, MAX(1, footerY - gap - listY));
	GUI::Dialog::reflowLayout();
}

void SaveManagementDialog::updateActionButtonState() {
	const bool enabled = _savesAvailable && !g_engine;
	const bool hasSelection = enabled && 0 <= _selectedRow && static_cast<uint>(_selectedRow) < _rowCount;
	for (GUI::ButtonWidget *button : _actions)
		button->setEnabled(hasSelection || (enabled && button->getCmd() == kImport));
}

void SaveManagementDialog::handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) {
	if (command == kSelectionChanged) {
		_selectedRow = -1;
		if (data < _rowCount)
			_selectedRow = data;
		updateActionButtonState();
		return;
	}
	for (GUI::ButtonWidget *button : _actions) {
		if (command != button->getCmd())
			continue;
		if (_savesAvailable && !g_engine && command == kImport) {
			importSave();
			return;
		}
		if (_savesAvailable && !g_engine && 0 <= _selectedRow && static_cast<uint>(_selectedRow) < _rowCount) {
			const Row &row = _rows[_selectedRow];
			handleSaveCommand(command, row.player, row.slot);
		}
		return;
	}
	GUI::Dialog::handleCommand(sender, command, data);
}

bool SaveManagementDialog::editPlayerName(Common::String &name, const Common::U32String &title, uint maxLength) {
	SaveNameDialog dialog(name, title, maxLength);
	if (dialog.runModal() != GUI::kOKCmd)
		return false;
	name = dialog.name();
	return true;
}

Common::SeekableReadStream *SaveManagementDialog::openImportFile(const Common::U32String &title) {
	GUI::BrowserDialog browser(title, false);
	if (browser.runModal() <= 0)
		return nullptr;
	Common::SeekableReadStream *stream = browser.getResult().createReadStream();
	if (!stream)
		GUI::MessageDialog(_("Unable to read the selected saved game.")).runModal();
	return stream;
}

Common::SeekableWriteStream *SaveManagementDialog::openExportFile(const Common::String &filename) {
	GUI::BrowserDialog browser(_("Select the directory for the exported saved game"), true);
	if (browser.runModal() <= 0)
		return nullptr;
	Common::FSNode destination = browser.getResult().getChild(filename);
	if (destination.exists()) {
		GUI::MessageDialog confirm(_("The exported file already exists. Replace it?"), _("Replace"), _("Cancel"));
		if (confirm.runModal() != GUI::kMessageOK)
			return nullptr;
	}
	Common::SeekableWriteStream *stream = destination.createWriteStream();
	if (!stream)
		GUI::MessageDialog(_("Unable to write the exported saved game.")).runModal();
	return stream;
}

GameOptionsDialog::GameOptionsDialog(TrinketEngine &vm) : GUI::OptionsDialog("", "GlobalConfig"), _vm(vm), _engineOptions(nullptr) {
	const Common::String &domain = ConfMan.getActiveDomainName();
	const MetaEngine *metaEngine = vm.getMetaEngine();
	GUI::TabWidget *tab = new GUI::TabWidget(this, "GlobalConfig.TabWidget");
	int gameTab = tab->addTab(_("Game"), "GlobalConfig_Engine");
	if (vm.hasFeature(Engine::kSupportsChangingOptionsDuringRuntime)) {
		GUI::ScrollContainerWidget *container = new GUI::ScrollContainerWidget(tab, "GlobalConfig_Engine.Container", "GlobalConfig_Engine_Container");
		container->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);
		container->setTarget(this);
		_engineOptions = metaEngine->buildEngineOptionsWidget(container, "GlobalConfig_Engine_Container.Container", domain);
	}
	if (_engineOptions)
		_engineOptions->setParentDialog(this);
	else
		tab->removeTab(gameTab);

	tab->addTab(_("Audio"), "GlobalConfig_Audio");
	// Leave the base volume widgets absent so its apply never writes launcher audio keys.
	addSaveVolumeControls(tab);
	if (vm.hasFeature(Engine::kSupportsSubtitleOptions)) {
		addSubtitleControls(tab, "GlobalConfig_Audio.", 255);
		setSubtitleSettingsState(true);
	}
	Common::KeymapArray keymaps = metaEngine->initKeymaps(domain.c_str());
	if (!keymaps.empty()) {
		tab->addTab(_("Keymaps"), "GlobalConfig_KeyMapper");
		GUI::ScrollContainerWidget *container = new GUI::ScrollContainerWidget(tab, "GlobalConfig_KeyMapper.Container", "GlobalConfig_KeyMapper_Container");
		container->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);
		container->setTarget(this);
		addKeyMapperControls(container, "GlobalConfig_KeyMapper_Container.", keymaps, domain);
	}
	int backendTab = tab->addTab(_("Backend"), "GlobalConfig_Backend");
	GUI::ScrollContainerWidget *container = new GUI::ScrollContainerWidget(tab, "GlobalConfig_Backend.Container", "GlobalConfig_Backend_Container");
	container->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);
	container->setTarget(this);
	_backendOptions = g_system->buildBackendOptionsWidget(container, "GlobalConfig_Backend_Container.Container", domain);
	if (_backendOptions)
		_backendOptions->setParentDialog(this);
	else
		tab->removeTab(backendTab);
	AchMan.setActiveDomain(metaEngine->getAchievementsInfo(domain));
	if (AchMan.getAchievementCount()) {
		tab->addTab(_("Achievements"), "GlobalConfig_Achievements");
		addAchievementsControls(tab, "GlobalConfig_Achievements.");
	}
	if (AchMan.getStatCount()) {
		tab->addTab(_("Statistics"), "GlobalConfig_Achievements");
		addStatisticsControls(tab, "GlobalConfig_Achievements.");
	}
	tab->setActiveTab(0);
	new GUI::ButtonWidget(this, "GlobalConfig.Ok", _("~O~K"), Common::U32String(), GUI::kOKCmd);
	new GUI::ButtonWidget(this, "GlobalConfig.Cancel", _("~C~ancel"), Common::U32String(), GUI::kCloseCmd);
}

void GameOptionsDialog::addSaveVolumeControls(GUI::GuiObject *boss) {
	const Common::String prefix = "GlobalConfig_Audio.";
	const char *names[] = {
		"Music",
		"Sfx",
		"Speech",
	};
	const Common::U32String descriptions[] = {
		g_gui.useLowResGUI() ? _c("Music volume:", "lowres") : _("Music volume:"),
		g_gui.useLowResGUI() ? _c("SFX volume:", "lowres") : _("SFX volume:"),
		g_gui.useLowResGUI() ? _c("Speech volume:", "lowres") : _("Speech volume:"),
	};
	for (uint i = 0; i < ARRAYSIZE(_volumes); i++) {
		Common::String name = prefix + "vc" + names[i];
		_volumes[i].description = new GUI::StaticTextWidget(boss, name + "Text", descriptions[i]);
		_volumes[i].slider = new GUI::SliderWidget(boss, name + "Slider", Common::U32String(), kMusicVolume01 + i);
		_volumes[i].slider->setMinValue(0);
		_volumes[i].slider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
		_volumes[i].label = new GUI::StaticTextWidget(boss, name + "Label", Common::U32String("100%"), Common::U32String(),
													  GUI::ThemeEngine::kFontStyleBold, Common::UNK_LANG, false);
		_volumes[i].label->setFlags(GUI::WIDGET_CLEARBG);
	}
	_mute = new GUI::CheckboxWidget(boss, prefix + "vcMuteCheckbox", _("Mute all"), Common::U32String(), kMute04);
}

void GameOptionsDialog::build() {
	GUI::OptionsDialog::build();
	if (_engineOptions)
		_engineOptions->load();
	Shell::Game::SoundSettings settings;
	if (_vm.getGameSoundSettings(settings)) {
		_volumes[0].slider->setValue(settings.music);
		_volumes[1].slider->setValue(settings.effects);
		_volumes[2].slider->setValue(settings.speech);
		_mute->setState(settings.mute);
	}
	updateVolumeControls();
}

void GameOptionsDialog::updateVolumeControls() {
	const char *flags[] = {
		GUIO_NOMUSIC,
		GUIO_NOSFX,
		GUIO_NOSPEECHVOLUME,
	};
	for (uint i = 0; i < ARRAYSIZE(_volumes); i++) {
		bool enabled = !_mute->getState() && !_guioptions.contains(flags[i]);
		if (i == 2 && _guioptions.contains(GUIO_NOSPEECH))
			enabled = false;
		_volumes[i].description->setEnabled(enabled);
		_volumes[i].slider->setEnabled(enabled);
		_volumes[i].label->setEnabled(enabled);
		_volumes[i].label->setValue(_volumes[i].slider->getValue());
	}
}

void GameOptionsDialog::apply() {
	if (_engineOptions)
		_engineOptions->save();
	Shell::Game::SoundSettings settings = {
		_volumes[0].slider->getValue(),
		_volumes[1].slider->getValue(),
		_volumes[2].slider->getValue(),
		_mute->getState(),
	};
	_vm.setGameSoundSettings(settings);
	GUI::OptionsDialog::apply();
}

void GameOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (kMusicVolume01 <= cmd && cmd <= kMute04)
		updateVolumeControls();
	else
		GUI::OptionsDialog::handleCommand(sender, cmd, data);
}

TrinketMainMenuDialog::TrinketMainMenuDialog(TrinketEngine &vm) : MainMenuDialog(&vm), _vm(vm) {}

void TrinketMainMenuDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	Shell::Game::SoundSettings settings;
	if (cmd == kOptionsCmd && _vm.getGameSoundSettings(settings)) {
		GameOptionsDialog dialog(_vm);
		dialog.runModal();
		g_system->applyBackendSettings();
		_vm.applyGameSettings();
		_vm.syncSoundSettings();
	} else {
		MainMenuDialog::handleCommand(sender, cmd, data);
	}
}

} // namespace Trinket
