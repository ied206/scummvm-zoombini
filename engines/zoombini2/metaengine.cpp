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

#include "common/config-manager.h"

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
	case kBaseV10:
		return (_features & GF_Z2_V10) != 0 && _language != Common::HE_ISR;
	case kHebrewV10:
		return (_features & GF_Z2_V10) != 0 && _language == Common::HE_ISR;
	case kExtendedV11:
		return (_features & GF_Z2_V11) != 0;
	default:
		return false;
	}
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
