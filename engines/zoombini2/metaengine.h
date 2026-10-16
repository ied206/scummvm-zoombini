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

#ifndef ZOOMBINI2_METAENGINE_H
#define ZOOMBINI2_METAENGINE_H

#include "common/error.h"
#include "common/str.h"

#include "engines/advancedDetector.h"

#include "zoombini2/detection.h"

namespace Common {
class String;
}

namespace GUI {
class GuiObject;
class OptionsContainerWidget;
} // namespace GUI

class Engine;
class OSystem;

/** Meta-engine settings and factory for the Zoombini2 engine. */
class Zoombini2MetaEngine : public AdvancedMetaEngine<Zoombini2::Zoombini2GameDescription> {
public:
	/** Target configuration key enabling per-savefile write-lock controls. */
	static constexpr const char *kConfigEnableSavefileReadOnlyToggle = "enable_savefile_readonly_toggle";
	/** Target configuration key selecting game-audio channel handling. */
	static constexpr const char *kConfigAudioOutputMode = "audio_output_mode";
	/** Target configuration key selecting datatype for Bezier curve calculations. */
	static constexpr const char *kConfigBezierCurveDataType = "bezier_curve_datatype";
	/** Target configuration key enabling the enhanced keyboard shortcut set. */
	static constexpr const char *kConfigEnhancedKbdShortcuts = "enhanced_kbd_shortcuts";
	/** Target configuration key hiding the stray Fleen departure streaks. */
	static constexpr const char *kConfigFixFleenDepartStreak = "fix_fleen_depart_streak";
	/** Target configuration key making solid help-sheet backgrounds transparent. */
	static constexpr const char *kConfigHelpDialogTransparentBody = "help_dialog_transparent_body";
	/** Target configuration key enabling optional English Norf clue subtitles. */
	static constexpr const char *kConfigChezNorfClueSubtitles = "cheznorf_clue_subtitles";
	/** Target configuration key enabling the developer hotkeys. */
	static constexpr const char *kConfigDebugHotkeys = "debug_hotkeys";
	/** Target configuration key selecting the alternate level-one Waterslide pairing. */
	static constexpr const char *kConfigGreedyWaterslidePairing = "greedy_waterslide_pairing";
	/** Target configuration key exposing recoverable level 4 puzzles on the practice map. */
	static constexpr const char *kConfigRestoreL4PracticePuzzles = "restore_l4_practice_puzzles";
	/** Target configuration key selecting the pseudo-random generator. */
	static constexpr const char *kConfigPrngAlgorithm = "prng_algorithm";
	/** Target configuration key selecting the optional color-only presentation. */
	static constexpr const char *kConfigColorAssistMode = "color_assist_mode";
	/** Target configuration key selecting the logic pacing rate mode. */
	static constexpr const char *kConfigLogicPacingMode = "logic_pacing_mode";
	/** Target configuration key for the presentation rate; it does not select the logic pacing reference. */
	static constexpr const char *kConfigFrameRate = "frame_rate";
	/** Target configuration key bypassing the presentation limiter while retaining elapsed-time logic updates. */
	static constexpr const char *kConfigUnlockFrameRate = "unlock_frame_rate";
	/** Lowest configurable presentation rate in frames per second. */
	static constexpr int kMinFrameRate = 30;
	/** Default presentation rate in frames per second. */
	static constexpr int kDefaultFrameRate = 60;
	/** Highest configurable presentation rate in frames per second. */
	static constexpr int kMaxFrameRate = 240;

	/** Return the engine identifier used by the launcher. */
	const char *getName() const override { return "zoombini2"; }

	/** Construct the runtime engine for a detected game description. */
	Common::Error createInstance(OSystem *syst, Engine **engine, const Zoombini2::Zoombini2GameDescription *desc) const override;

	/** Report the launcher and save-management features supported by this engine. */
	bool hasFeature(MetaEngineFeature f) const override;
	/** Register engine-specific option defaults for the supplied target. */
	void registerDefaultSettings(const Common::String &target) const override;
	/** Create the engine-specific launcher options widget. */
	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
};

#endif // ZOOMBINI2_METAENGINE_H
