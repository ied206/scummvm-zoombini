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

#include "base/plugins.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "common/config-manager.h"
#include "common/translation.h"

#include "zoombini2/dialogs.h"
#include "zoombini2/graphics.h"
#include "zoombini2/metaengine.h"
#include "zoombini2/random.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

ZmbReleaseInfo::ZmbReleaseInfo(const Zoombini2GameDescription &gameDesc)
	: _features(gameDesc.features), _language(gameDesc.desc.language), _demo((gameDesc.desc.flags & ADGF_DEMO) != 0) {
}

bool ZmbReleaseInfo::isFamily(Family family) const {
	switch (family) {
	case Family::kV10:
		return (_features & GF_Z2_V10) != 0;
	case Family::kV11:
		return (_features & GF_Z2_V11) != 0;
	}
	return false;
}

bool ZmbReleaseInfo::isFamily(Family family, Common::Language language) const {
	return isFamily(family) && _language == language;
}

ZmbReleaseInfo::BitmapTextLayout ZmbReleaseInfo::getBitmapTextLayout() const {
	switch (_language) {
	case Common::HE_ISR:
		return BitmapTextLayout::kHebrew;
	case Common::SV_SWE:
		return BitmapTextLayout::kSwedish;
	default:
		return BitmapTextLayout::kLatin;
	}
}

ZmbReleaseInfo::NameTooltipLayout ZmbReleaseInfo::getNameTooltipLayout() const {
	NameTooltipLayout layout;
	layout.platePosition = Common::Point32(310, 565);
	layout.textAnchor = Common::Point32(400, 570);
	if (getBitmapTextLayout() == BitmapTextLayout::kHebrew)
		layout.textAnchor.x -= 50;
	return layout;
}

PageId ZmbReleaseInfo::getStartupPageId() const {
	if (_demo)
		return PageId::kPageTitleScreen_24;

	// The Korean release starts with the ArisuMedia logo before the TLC and Polygon logos.
	// Other releases start with the TLC logo.
	switch (_language) {
	case Common::KO_KOR:
		return PageId::kPageLogoArisuMedia_1972;
	default:
		return PageId::kPageLogoTLC_17;
	}
}

} // End of namespace Zoombini2

Common::Error Zoombini2MetaEngine::createInstance(OSystem *syst, Engine **engine, const Zoombini2::Zoombini2GameDescription *desc) const {
	*engine = new Zoombini2::Zoombini2Engine(syst, desc);
	return Common::kNoError;
}

bool Zoombini2MetaEngine::hasFeature(MetaEngineFeature f) const {
	return false;
}

Common::Array<Common::Keymap *> Zoombini2MetaEngine::initKeymaps(const char *target) const {
	using namespace Common;
	(void)target;

	Keymap *defaultKeymap = new Keymap(Keymap::kKeymapTypeGame, "engine-default", _("Default game keymappings"));
	defaultKeymap->setPartialMatchAllowed(false);

	// The global keymap supplies left-stick cursor movement, RB slowdown, Start and Back.
	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left click"));
	act->setLeftClickEvent();
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	act->addDefaultInputMapping("JOY_RIGHT_TRIGGER");
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionMiddleClick, _("Middle click"));
	act->setMiddleClickEvent();
	act->addDefaultInputMapping("MOUSE_MIDDLE");
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right click"));
	act->setRightClickEvent();
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_LEFT_TRIGGER");
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionPause, _("Pause"));
	act->setKeyEvent(KeyState(KEYCODE_SPACE, ' '));
	act->addDefaultInputMapping("SPACE");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionOpenMainMenu, _("Game menu"));
	act->setKeyEvent(KeyState(KEYCODE_F5, ASCII_F5));
	act->addDefaultInputMapping("F5");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionSkip, _("Skip"));
	act->setKeyEvent(KeyState(KEYCODE_ESCAPE, ASCII_ESCAPE));
	act->addDefaultInputMapping("ESCAPE");
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping("JOY_Y");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	act = new Action("RETURN", _("Confirm"));
	act->setKeyEvent(KeyState(KEYCODE_RETURN, ASCII_RETURN));
	act->addDefaultInputMapping("RETURN");
	act->addDefaultInputMapping("JOY_X");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionMoveUp, _("Up"));
	act->setKeyEvent(KEYCODE_UP);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Down"));
	act->setKeyEvent(KEYCODE_DOWN);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Left"));
	act->setKeyEvent(KEYCODE_LEFT);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Right"));
	act->setKeyEvent(KEYCODE_RIGHT);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	act->allowKbdRepeats();
	defaultKeymap->addAction(act);

	return Keymap::arrayOf(defaultKeymap);
}

void Zoombini2MetaEngine::registerDefaultSettings(const Common::String &target) const {
	(void)target;
	ConfMan.registerDefault(kConfigEnableSavefileReadOnlyToggle, false);
	ConfMan.registerDefault(kConfigAudioOutputMode, static_cast<int>(Zoombini2::AudioOutputMode::kMonoDownmix));
	ConfMan.registerDefault(kConfigBezierCurveDataType, static_cast<int>(Zoombini2::BezierCurveDataType::kQ10FixedPoint));
	ConfMan.registerDefault(kConfigWallOfFleensFixDepartStreak, true);
	ConfMan.registerDefault(kConfigMysticMarshFillCellGaps, true);
	ConfMan.registerDefault(kConfigMysticMarshExpandAreaMask, true);
	ConfMan.registerDefault(kConfigEnhancedKbdShortcuts, true);
	ConfMan.registerDefault(kConfigHelpDialogTransparentBody, true);
	ConfMan.registerDefault(kConfigChezNorfClueSubtitles, false);
	ConfMan.registerDefault(kConfigDebugHotkeys, false);
	ConfMan.registerDefault(kConfigGreedyWaterslidePairing, false);
	ConfMan.registerDefault(kConfigRestoreL4PracticePuzzles, false);
	ConfMan.registerDefault(kConfigPrngAlgorithm, static_cast<int>(Zoombini2::PrngAlgorithmKind::kOriginalPrng));
	ConfMan.registerDefault(kConfigColorAssistMode, static_cast<int>(Zoombini2::ColorAssistMode::kOriginal));
	ConfMan.registerDefault(kConfigLogicPacingMode, static_cast<int>(Zoombini2::LogicPacingMode::k75Hz));
	ConfMan.registerDefault(kConfigFrameRate, kDefaultFrameRate);
	ConfMan.registerDefault(kConfigUnlockFrameRate, false);
}

GUI::OptionsContainerWidget *Zoombini2MetaEngine::buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	return new Zoombini2::Zoombini2OptionsWidget(boss, name, target);
}

#if PLUGIN_ENABLED_DYNAMIC(ZOOMBINI2)
REGISTER_PLUGIN_DYNAMIC(ZOOMBINI2, PLUGIN_TYPE_ENGINE, Zoombini2MetaEngine);
#else
REGISTER_PLUGIN_STATIC(ZOOMBINI2, PLUGIN_TYPE_ENGINE, Zoombini2MetaEngine);
#endif
