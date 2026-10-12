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

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "trinket/detection.h"
#include "trinket/trinket.h"
#ifdef ENABLE_ZOOMBINI3
#include "trinket/zoombini3.h"
#include "trinket/zoombini3_dialogs.h"
#endif
#ifdef ENABLE_RR1CLOUD9
#include "trinket/rr1cloud9/rr1cloud9.h"
#endif

class TrinketMetaEngine : public AdvancedMetaEngine<Trinket::TrinketGameDescription> {
public:
	const char *getName() const override {
		return "trinket";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const Trinket::TrinketGameDescription *desc) const override {
#ifdef ENABLE_ZOOMBINI3
		if (!strcmp(desc->desc.gameId, "zoombini3") && desc->desc.platform == Common::kPlatformWindows) {
			*engine = new Trinket::TrinketEngine(syst, Trinket::Zoombini3::getSubengine());
			return Common::kNoError;
		}
#endif
#ifdef ENABLE_RR1CLOUD9
		if (!strcmp(desc->desc.gameId, "rr1cloud9") && desc->desc.platform == Common::kPlatformWindows) {
			*engine = new Trinket::TrinketEngine(syst, Trinket::RR1Cloud9::getSubengine());
			return Common::kNoError;
		}
#endif
		return Common::kUnsupportedGameidError;
	}

	bool hasFeature(MetaEngineFeature feature) const override { return false; }
	Common::KeymapArray initKeymaps(const char *target) const override {
		Common::KeymapArray keymaps = AdvancedMetaEngine::initKeymaps(target);
		Common::Action *remove = new Common::Action("REMOVE_NAME", _("Remove player name"));
		remove->setKeyEvent(Common::KeyState(Common::KEYCODE_r, 'r', Common::KBD_CTRL));
		remove->addDefaultInputMapping("C+r");
		keymaps[0]->addAction(remove);
		return keymaps;
	}
	void registerDefaultSettings(const Common::String &target) const override {
		(void)target;
		ConfMan.registerDefault(Trinket::Shell::Random::kPrngAlgorithmOption, static_cast<int>(Trinket::Shell::Random::PrngAlgorithm::kOriginalPrng));
	}
#ifdef ENABLE_ZOOMBINI3
	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override {
		if (ConfMan.get("gameid", target) == "zoombini3")
			return new Trinket::Zoombini3::OptionsWidget(boss, name, target);
		return nullptr;
	}
#endif
};

#if PLUGIN_ENABLED_DYNAMIC(TRINKET)
REGISTER_PLUGIN_DYNAMIC(TRINKET, PLUGIN_TYPE_ENGINE, TrinketMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TRINKET, PLUGIN_TYPE_ENGINE, TrinketMetaEngine);
#endif
