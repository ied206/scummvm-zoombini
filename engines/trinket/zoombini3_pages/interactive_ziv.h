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

#ifndef TRINKET_ZOOMBINI3_PAGES_INTERACTIVE_ZIV_H
#define TRINKET_ZOOMBINI3_PAGES_INTERACTIVE_ZIV_H

#include "trinket/ompp/sound.h"
#include "trinket/shell/multitrinket.h"
#include "trinket/shell/scroller.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {

class TrinketEngine;
namespace Zoombini3 {

/** Native island decorations, inhabitants and visibility-margin updates. */
class ZIVMachine : public Shell::MultiTrinket {
public:
	/** Connect the interactive village display to its scene and horizontal scroller. */
	ZIVMachine(Shell::Scene &scene, Shell::Scroller &scroller, uint32 time);
	/** Remove village animations and stop tracking them for scrolling. */
	~ZIVMachine() override;
	/** Dispatch village population setup and playback-control script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Advance village animations and periodically cull off-screen activity. */
	void update(uint32 time);
	/** Return whether a required village resource failed to load. */
	bool hasResourceError() const { return !_valid; }

private:
	/** Script method name for pause Round. */
	static constexpr const char *kMethodPauseRound = "pauseRound";
	/** Script method name for resume Round. */
	static constexpr const char *kMethodResumeRound = "resumeRound";
	/** Script method name for setup Game. */
	static constexpr const char *kMethodSetupGame = "setupGame";

	/** Animation path pattern for village close-up resources. */
	static constexpr const char *kAnimation = "/z3v3cd/%u.ao";
	/** Scene displaying village characters and ambient animations. */
	Shell::Scene &_scene;
	/** Horizontal scroller used by the village panorama. */
	Shell::Scroller &_scroller;
	/** Ambient animation sprites displayed in the village. */
	Common::Array<Shell::Sprite *> _ambient;
	/** Character and population animation sprites in the current village setup. */
	Common::Array<Shell::Sprite *> _animations;
	/** Animations to resume after the village is unpaused. */
	Common::Array<Shell::Sprite *> _pausedAnims;
	/** Source-time millisecond deadline for off-screen culling, retained across pause. */
	uint32 _cullTime;
	/** Whether village animations are paused. */
	bool _paused = false;
	/** Whether required village resources and setup are valid. */
	bool _valid = true;
	/** Build the village display for the supplied character count. */
	bool setup(uint tokens, uint32 time);
	/** Create a village animation with optional off-screen culling and repeat count. */
	bool addAnimation(uint id, bool cull, int repeat = -1);
	/** Remove the current population animation sprites. */
	void clearAnimations();
	/** Pause or resume active village animations. */
	void pause(bool paused);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
