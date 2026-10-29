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

#ifndef TRINKET_METAENGINE_H
#define TRINKET_METAENGINE_H

#include "backends/keymapper/keymap.h"
#include "trinket/detection.h"

class TrinketMetaEngine : public AdvancedMetaEngine<Trinket::TrinketGameDescription> {
public:
	/** Persisted RR1Cloud9 startup selection. */
	enum class StartupMode : uint32 {
		kDirect00 = 0,
		kLauncher01 = 1,
	};
	/** Configuration key for the RR1Cloud9 startup selection. */
	static constexpr const char *kOptionStartupMode = "startup_mode";
	/** Configuration key for red-green color assistance. */
	static constexpr const char *kOptionColorBlindMode = "color_blind_mode";
	/** Configuration key for the selected pseudo-random generator. */
	static constexpr const char *kOptionPrngAlgorithm = "prng_algorithm";
	/** Configuration key for the initial music volume. */
	static constexpr const char *kOptionMusicVolume = "music_volume";
	/** Configuration key for the initial sound-effects volume. */
	static constexpr const char *kOptionSfxVolume = "sfx_volume";
	/** Configuration key for the initial speech volume. */
	static constexpr const char *kOptionSpeechVolume = "speech_volume";
	/** Configuration key for the initial mute state. */
	static constexpr const char *kOptionMute = "mute";
	/** Configuration key for the optional startup boot parameter. */
	static constexpr const char *kConfigBootParam = "boot_param";
	/** Configuration key for the optional Zoombini 3 random seed override. */
	static constexpr const char *kConfigRandomSeed = "random_seed";

	/** Return the Trinket engine identifier. */
	const char *getName() const override { return "trinket"; }
	/** Create a Trinket engine for a detected game and return any initialization error. */
	Common::Error createInstance(OSystem *syst, Engine **engine, const Trinket::TrinketGameDescription *desc) const override;
	/** Return false for optional metaengine features not implemented by Trinket. */
	bool hasFeature(MetaEngineFeature feature) const override { return false; }
	/** Create input mappings for the selected game target. */
	Common::KeymapArray initKeymaps(const char *target) const override;
	/** Register the engine and subengine defaults for a game target. */
	void registerDefaultSettings(const Common::String &target) const override;
	/** Create the target-specific engine options panel. */
	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;

private:
	/** Script identifier for zoombini3. */
	static constexpr const char *kZoombini3Name = "zoombini3";
	/** Script identifier for rr1cloud9. */
	static constexpr const char *kRr1cloud9Name = "rr1cloud9";
	/** Input action identifier for removing a saved player name. */
	static constexpr const char *kREMOVENAMEName = "REMOVE_NAME";
};

#endif
