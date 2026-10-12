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

#include "gui/widget.h"

namespace GUI {
class PopUpWidget;
}

namespace Trinket {
namespace Zoombini3 {

class OptionsWidget : public GUI::OptionsContainerWidget {
public:
	OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain);
	void load() override;
	bool save() override;
	void setEnabled(bool enabled) override;
	void handleCommand(GUI::CommandSender *sender, uint32 command, uint32 data) override;

protected:
	void defineLayout(GUI::ThemeEval &layouts, const Common::String &name, const Common::String &overlay) const override;

private:
	/** Label used to align the PRNG selector using its displayed text. */
	GUI::StaticTextWidget *_prngAlgorithmLabel = nullptr;

	/** Select the original or ScummVM standard PRNG algorithm. */
	GUI::PopUpWidget *_prngAlgorithmPopUp = nullptr;
	GUI::ButtonWidget *_manage = nullptr;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
