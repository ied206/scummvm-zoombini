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
	/** Persisted selection for mono downmixing or stereo game-audio playback. */
	enum class AudioOutputMode : uint32 {
		/** Downmix stereo game-audio resources to mono. */
		kMonoDownmix = 0,
		/** Preserve both channels of stereo game-audio resources. */
		kStereoPlayback = 1
	};
	/** Target configuration key selecting game-audio channel handling. */
	static constexpr const char *kConfigAudioOutputMode = "audio_output_mode";
	/** Target configuration key selecting floating-point Bezier path calculations. */
	static constexpr const char *kConfigUseFloatingPointPaths = "use_floating_point_paths";
	/** Target configuration key hiding the stray Fleen departure streaks. */
	static constexpr const char *kConfigFixFleenDepartureStreak = "fix_fleen_depart_streak";
	/** Target configuration key enabling the enhanced keyboard shortcut set. */
	static constexpr const char *kConfigEnhancedKbdShortcuts = "enhanced_kbd_shortcuts";
	/** Target configuration key making solid help-sheet backgrounds transparent. */
	static constexpr const char *kConfigTransparentHelpPages = "transparent_help_pages";
	/** Target configuration key enabling the developer hotkeys. */
	static constexpr const char *kConfigDebugHotkeys = "debug_hotkeys";
	/** Target configuration key selecting the alternate level-one Waterslide pairing. */
	static constexpr const char *kConfigGreedyWaterslidePairing = "greedy_waterslide_pairing";
	/** Target configuration key protecting Aqua Cube's first direct lever move from a Fleen. */
	static constexpr const char *kConfigAquacubeSafeFirstMove = "aquacube_safe_first_move";
	/** Target configuration key exposing recoverable level 4 puzzles on the practice map. */
	static constexpr const char *kConfigAllowCutLevel4PracticePuzzles = "allow_cut_level4_practice_puzzles";
	/** Persisted and runtime pseudo-random generator selection. */
	enum class PrngAlgorithm : uint32 {
		/** Use the MSVC-compatible linear congruential stream. */
		kOriginalPrng = 0,
		/** Use @ref Common::RandomSource. */
		kStandardPrng = 1,
	};
	/** Target configuration key selecting the pseudo-random generator. */
	static constexpr const char *kConfigPrngAlgorithm = "prng_algorithm";
	/** Persisted and runtime selection for color-only presentation assistance. */
	enum class ColorAssistMode : uint32 {
		/** Preserve the original artwork colors. */
		kOriginal = 0,
		/** Make selected similar colors easier to distinguish. */
		kEnhancedDistinction = 1,
		/** Use colors chosen for red-green color vision deficiency. */
		kRedGreenBlindAssist = 2
	};
	/** Target configuration key selecting the optional color-only presentation. */
	static constexpr const char *kConfigColorAssistMode = "color_assist_mode";
	/** Reference rate for frame-derived motion and random-event quotas, independent of presentation FPS and timed animations. */
	enum class LogicPacingMode : uint32 {
		/** Normalize frame-derived motion and random-event quotas to 60 updates per second (LCD preset). */
		k60Hz = 60,
		/** Normalize frame-derived motion and random-event quotas to 75 updates per second (CRT preset). */
		k75Hz = 75,
	};
	/** Target configuration key selecting the logic pacing rate mode. */
	static constexpr const char *kConfigLogicPacingMode = "logic_pacing_mode";
	/** Target configuration key for the presentation rate; it does not select the logic pacing reference. */
	static constexpr const char *kConfigFrameRate = "frame_rate";
	/** Target configuration key bypassing the presentation limiter while retaining elapsed-time logic updates. */
	static constexpr const char *kConfigUnlockFrameRate = "unlock_frame_rate";
	static constexpr int kMinFrameRate = 30;
	static constexpr int kDefaultFrameRate = 60;
	static constexpr int kMaxFrameRate = 240;

	const char *getName() const override { return "zoombini2"; }

	Common::Error createInstance(OSystem *syst, Engine **engine, const Zoombini2::Zoombini2GameDescription *desc) const override;

	bool hasFeature(MetaEngineFeature f) const override;
	void registerDefaultSettings(const Common::String &target) const override;
	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
};

#endif // ZOOMBINI2_METAENGINE_H
