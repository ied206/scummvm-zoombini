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
#include "common/gui_options.h"
#include "common/translation.h"
#include "engines/achievements.h"
#include "engines/metaengine.h"
#include "gui/gui-manager.h"
#include "gui/widgets/scrollcontainer.h"
#include "gui/widgets/tab.h"
#include "trinket/trinket.h"

namespace Trinket {

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
