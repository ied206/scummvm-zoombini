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

#include "trinket/zoombini3_attrib_ch_walk_action.h"

namespace Trinket {
namespace Zoombini3 {

constexpr const char *AttribChWalkAction::kRAttributeCharacterName;

Shell::Action *AttribChWalkAction::create(Shell::ActionSystem &system, const Core::ScriptArguments &sargs, bool interruptible) {
	if (sargs.empty() || (4 < sargs.size() && (sargs.size() - 4) % 3) ||
		!system.resolve(sargs[0], kRAttributeCharacterName))
		return nullptr;
	bool clearPath = sargs.size() < 2 || sargs[1].toInt32() != 0;
	bool keepPosition = 2 < sargs.size() && sargs[2].toInt32() != 0;
	if (3 < sargs.size())
		interruptible = sargs[3].toInt32() != 0;
	AttribChWalkAction *action = new AttribChWalkAction(system, sargs[0], clearPath, keepPosition, interruptible);
	for (uint i = 4; i < sargs.size(); i += 3) {
		Waypoint waypoint;
		waypoint.position = Common::Point(sargs[i].toInt32(), sargs[i + 1].toInt32());
		waypoint.absolute = sargs[i + 2].toInt32() != 0;
		action->_waypoints.push_back(waypoint);
	}
	return action;
}

AttributeCharacter *AttribChWalkAction::target() const {
	return static_cast<AttributeCharacter *>(_system.resolve(_target, kRAttributeCharacterName));
}

AttribChWalkAction::~AttribChWalkAction() {
	detach();
}

void AttribChWalkAction::detach() {
	if (_character) {
		_character->removeWalkListener(*this);
		_character->removeObserver(*this);
		_character = nullptr;
	}
}

void AttribChWalkAction::startMovement(AttributeCharacter &character) {
	for (uint i = 0; i < _waypoints.size(); i++)
		character.addWalkMove(_waypoints[i].position, _waypoints[i].absolute);
	if (!character.isWalking()) {
		character.setWalkListener(this);
		if (character.walk(true, _system.currentTime(), _clearPath, _keepPosition))
			return;
		character.removeWalkListener(*this);
	}
	walkFinished(character);
}

void AttribChWalkAction::start() {
	Shell::ActionSystem::Scope scope(_system);
	detach();
	Action::start();
	_completed = false;
	_character = target();
	if (!_character) {
		finished();
		return;
	}
	_character->addObserver(*this);
	startMovement(*_character);
}

void AttribChWalkAction::halt() {
	Shell::ActionSystem::Scope scope(_system);
	Action::halt();
	AttributeCharacter *character = target();
	if (character) {
		if (!_completed && !character->isWalking()) {
			detach();
			_character = character;
			_character->addObserver(*this);
			startMovement(*character);
		}
		detach();
		character->stopWalk(!_keepPosition);
	}
	_completed = true;
}

void AttribChWalkAction::pause() {
	Action::pause();
	AttributeCharacter *character = target();
	if (character)
		character->pause(true, _system.currentTime());
}

void AttribChWalkAction::resume() {
	Action::resume();
	AttributeCharacter *character = target();
	if (character)
		character->pause(false, _system.currentTime());
}

void AttribChWalkAction::walkFinished(AttributeCharacter &) {
	Shell::ActionSystem::Scope scope(_system);
	detach();
	_completed = true;
	finished();
}

Core::ScriptableObject *AttribChWalkAction::clone() const {
	AttribChWalkAction *copy = new AttribChWalkAction(_system, _target, _clearPath, _keepPosition, isInterruptible());
	copy->_waypoints = _waypoints;
	return copy;
}

} // namespace Zoombini3
} // namespace Trinket
