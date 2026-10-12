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

#include "engines/dialogs.h"

namespace Trinket {

class TrinketEngine;

/** Standard runtime options with volume controls owned by the active save. */
class GameOptionsDialog : public GUI::OptionsDialog {
public:
	explicit GameOptionsDialog(TrinketEngine &vm);
	void build() override;
	void apply() override;
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	enum Command {
		kMusicVolume01 = 1,
		kEffectsVolume02 = 2,
		kSpeechVolume03 = 3,
		kMute04 = 4,
	};
	struct VolumeControl {
		GUI::StaticTextWidget *description;
		GUI::SliderWidget *slider;
		GUI::StaticTextWidget *label;
	};
	TrinketEngine &_vm;
	GUI::OptionsContainerWidget *_engineOptions;
	VolumeControl _volumes[3];
	GUI::CheckboxWidget *_mute;
	void addSaveVolumeControls(GUI::GuiObject *boss);
	void updateVolumeControls();
};

class TrinketMainMenuDialog : public MainMenuDialog {
public:
	explicit TrinketMainMenuDialog(TrinketEngine &vm);
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) override;

private:
	TrinketEngine &_vm;
};

} // End of namespace Trinket

#endif
