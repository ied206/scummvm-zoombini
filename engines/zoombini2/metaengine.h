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
 *
 */

#ifndef ZOOMBINI2_METAENGINE_H
#define ZOOMBINI2_METAENGINE_H

#include "common/error.h"

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
	/** Target configuration key that blocks automatic progress writes during tests. */
	static constexpr const char *kConfigSavefilesReadOnly = "savefiles_read_only";
	/** Target configuration key selecting stereo game-audio streams. */
	static constexpr const char *kConfigStereoOutput = "stereo_output";
	/** Target configuration key selecting floating-point Bezier path calculations. */
	static constexpr const char *kConfigUseFloatingPointPaths = "use_floating_point_paths";
	/** Target configuration key enabling the enhanced keyboard shortcut set. */
	static constexpr const char *kConfigEnhancedKbdShortcuts = "enhanced_kbd_shortcuts";
	/** Target configuration key enabling the developer hotkeys. */
	static constexpr const char *kConfigDebugHotkeys = "debug_hotkeys";
	/** Target configuration key selecting the alternate level-one Waterslide pairing. */
	static constexpr const char *kConfigGreedyWaterslidePairing = "greedy_waterslide_pairing";
	/** Target configuration key selecting the original Windows random-number generator. */
	static constexpr const char *kConfigOriginalPRNG = "original_prng";
	/** Target frame rate for the engine's presentation loop. */
	static constexpr const char *kConfigFrameRate = "frame_rate";
	/** Disable the engine's frame-rate limiter. */
	static constexpr const char *kConfigUnlockFrameRate = "unlock_frame_rate";
	static constexpr int kMinFrameRate = 30;
	static constexpr int kDefaultFrameRate = 60;
	static constexpr int kMaxFrameRate = 240;

	const char *getName() const override {
		return "zoombini2";
	}

	Common::Error createInstance(OSystem *syst, Engine **engine, const Zoombini2::Zoombini2GameDescription *desc) const override;

	bool hasFeature(MetaEngineFeature f) const override;
	void registerDefaultSettings(const Common::String &target) const override;
	GUI::OptionsContainerWidget *buildEngineOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const override;
};

#endif // ZOOMBINI2_METAENGINE_H
