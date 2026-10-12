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
	ZIVMachine(Shell::Scene &scene, Shell::Scroller &scroller, uint32 time);
	~ZIVMachine() override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	void update(uint32 time);
	bool hasResourceError() const { return !_valid; }

private:
	static constexpr const char *kAnimation = "/z3v3cd/%u.ao";
	Shell::Scene &_scene;
	Shell::Scroller &_scroller;
	Common::Array<Shell::Sprite *> _ambient, _animations, _pausedAnimations;
	uint32 _cullTime;
	bool _paused = false, _valid = true;
	bool setup(uint tokens, uint32 time);
	bool addAnimation(uint id, bool cull, int repeat = -1);
	void clearAnimations();
	void pause(bool paused);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
