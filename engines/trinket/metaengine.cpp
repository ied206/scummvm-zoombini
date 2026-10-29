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

#include "trinket/metaengine.h"
#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "base/plugins.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "trinket/shell/random.h"
#include "trinket/trinket.h"
#ifdef ENABLE_ZOOMBINI3
#include "trinket/zoombini3.h"
#include "trinket/zoombini3_dialogs.h"
#endif
#ifdef ENABLE_RR1CLOUD9
#include "trinket/rr1cloud9/dialogs.h"
#include "trinket/rr1cloud9/rr1cloud9.h"
#endif

constexpr const char *TrinketMetaEngine::kOptionStartupMode;
constexpr const char *TrinketMetaEngine::kOptionColorBlindMode;
constexpr const char *TrinketMetaEngine::kOptionPrngAlgorithm;
constexpr const char *TrinketMetaEngine::kOptionMusicVolume;
constexpr const char *TrinketMetaEngine::kOptionSfxVolume;
constexpr const char *TrinketMetaEngine::kOptionSpeechVolume;
constexpr const char *TrinketMetaEngine::kOptionMute;
constexpr const char *TrinketMetaEngine::kConfigBootParam;
constexpr const char *TrinketMetaEngine::kConfigRandomSeed;
constexpr const char *TrinketMetaEngine::kZoombini3Name;
constexpr const char *TrinketMetaEngine::kRr1cloud9Name;
constexpr const char *TrinketMetaEngine::kREMOVENAMEName;

Common::Error TrinketMetaEngine::createInstance(OSystem *syst, Engine **engine, const Trinket::TrinketGameDescription *desc) const {
	if (desc->desc.platform != Common::kPlatformWindows && desc->desc.platform != Common::kPlatformMacintosh)
		return Common::kUnsupportedGameidError;
#ifdef ENABLE_ZOOMBINI3
	if (!strcmp(desc->desc.gameId, kZoombini3Name)) {
		*engine = new Trinket::TrinketEngine(syst, Trinket::Zoombini3::getSubengine(), desc->desc.platform);
		return Common::kNoError;
	}
#endif
#ifdef ENABLE_RR1CLOUD9
	if (!strcmp(desc->desc.gameId, kRr1cloud9Name)) {
		*engine = new Trinket::TrinketEngine(syst, Trinket::RR1Cloud9::getSubengine(), desc->desc.platform);
		return Common::kNoError;
	}
#endif
	return Common::kUnsupportedGameidError;
}

Common::KeymapArray TrinketMetaEngine::initKeymaps(const char *target) const {
	Common::KeymapArray keymaps = AdvancedMetaEngine::initKeymaps(target);
	Common::Action *remove = new Common::Action(kREMOVENAMEName, _("Remove player name"));
	remove->setKeyEvent(Common::KeyState(Common::KEYCODE_r, 'r', Common::KBD_CTRL));
	remove->addDefaultInputMapping("C+r");
	keymaps[0]->addAction(remove);
	return keymaps;
}

void TrinketMetaEngine::registerDefaultSettings(const Common::String &target) const {
	(void)target;
	ConfMan.registerDefault(kOptionPrngAlgorithm, static_cast<int>(Trinket::Shell::Random::PrngAlgorithm::kOriginalPrng));
	ConfMan.registerDefault(kOptionStartupMode, static_cast<int>(StartupMode::kDirect00));
	ConfMan.registerDefault(kOptionColorBlindMode, false);
}

GUI::OptionsContainerWidget *TrinketMetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	const Common::String gameId = getGameId(target.c_str());

#ifdef ENABLE_ZOOMBINI3
	if (gameId.equals(kZoombini3Name))
		return new Trinket::Zoombini3::OptionsWidget(boss, name, target);
#endif
#ifdef ENABLE_RR1CLOUD9
	if (gameId.equals(kRr1cloud9Name))
		return new Trinket::RR1Cloud9::OptionsWidget(boss, name, target);
#endif
	return nullptr;
}

#if PLUGIN_ENABLED_DYNAMIC(TRINKET)
REGISTER_PLUGIN_DYNAMIC(TRINKET, PLUGIN_TYPE_ENGINE, TrinketMetaEngine);
#else
REGISTER_PLUGIN_STATIC(TRINKET, PLUGIN_TYPE_ENGINE, TrinketMetaEngine);
#endif
