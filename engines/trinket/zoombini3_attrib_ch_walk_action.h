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
	/** Parse a scripted character-walk action and its waypoint list. */
	static Shell::Action *create(Shell::ActionSystem &system, const Core::ScriptArguments &sargs, bool interruptible);
	/** Detach callbacks from any character still executing the walk. */
	~AttribChWalkAction() override;
	/** Resolve the target character and begin the configured path. */
	void start() override;
	/** Stop the active walk and detach completion callbacks. */
	void halt() override;
	/** Pause the target character's walk. */
	void pause() override;
	/** Resume the target character's walk. */
	void resume() override;
	/** Copy the target and waypoint settings into an inactive action. */
	Core::ScriptableObject *clone() const override;

private:
	/** Script identifier for rAttribute Character. */
	static constexpr const char *kRAttributeCharacterName = "RAttributeCharacter";

	struct Waypoint {
		/** Absolute destination or relative displacement in game pixels. */
		Common::Point position;
		/** Whether the waypoint is absolute rather than relative to the character. */
		bool absolute = true;
	};
	/** Configure a target character and walk-completion policies. */
	AttribChWalkAction(Shell::ActionSystem &system, const Core::ScriptValue &target, bool clearPath, bool keepPosition, bool interruptible) : Action(system, interruptible), _target(target), _clearPath(clearPath), _keepPosition(keepPosition) {}
	/** Script value identifying the character that will walk. */
	Core::ScriptValue _target;
	/** Absolute and relative moves retained for action startup. */
	Common::Array<Waypoint> _waypoints;
	/** Character currently watched for walk completion. */
	AttributeCharacter *_character = nullptr;
	/** Whether finishing the action clears the character's queued path. */
	bool _clearPath;
	/** Whether finishing the action retains the final position. */
	bool _keepPosition;
	/** Whether the current action has already received its completion callback. */
	bool _completed = false;
	/** Resolve the script target as an attribute-composed character. */
	AttributeCharacter *target() const;
	/** Remove walk and destruction callbacks from the target character. */
	void detach();
	/** Install the configured waypoints and start the character walk. */
	void startMovement(AttributeCharacter &character);
	/** Finish the action after the character completes its path. */
	void walkFinished(AttributeCharacter &) override;
	/** Clear the target pointer when the character is destroyed. */
	void objectDestroyed(Core::ScriptableObject &) override { _character = nullptr; }
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
