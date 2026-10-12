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

#ifndef TRINKET_ZOOMBINI3_ATTRIB_CH_WALK_ACTION_H
#define TRINKET_ZOOMBINI3_ATTRIB_CH_WALK_ACTION_H

#include "trinket/shell/action.h"
#include "trinket/zoombini3_character.h"

namespace Trinket {
namespace Zoombini3 {

/**
 * Authored character walking, including pending movement on queue halt.
 */
class AttribChWalkAction : public Shell::Action, private AttributeCharacter::WalkListener, private Core::ScriptableObject::Observer {
public:
	static Shell::Action *create(Shell::ActionSystem &system, const Core::ScriptArguments &arguments, bool interruptible);
	~AttribChWalkAction() override;
	void start() override;
	void halt() override;
	void pause() override;
	void resume() override;
	Core::ScriptableObject *clone() const override;

private:
	struct Waypoint {
		Common::Point position;
		bool absolute = true;
	};
	AttribChWalkAction(Shell::ActionSystem &system, const Core::ScriptValue &target, bool clearPath, bool keepPosition, bool interruptible) : Action(system, interruptible), _target(target), _clearPath(clearPath), _keepPosition(keepPosition) {}
	Core::ScriptValue _target;
	Common::Array<Waypoint> _waypoints;
	AttributeCharacter *_character = nullptr;
	bool _clearPath, _keepPosition, _completed = false;
	AttributeCharacter *target() const;
	void detach();
	void startMovement(AttributeCharacter &character);
	void walkFinished(AttributeCharacter &) override;
	void objectDestroyed(Core::ScriptableObject &) override { _character = nullptr; }
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
