/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_SHELL_CHARACTER_ACTION_H
#define TRINKET_SHELL_CHARACTER_ACTION_H

#include "trinket/shell/action.h"
#include "trinket/shell/character.h"

namespace Trinket {
namespace Shell {

/** Queued character speech or animation with a borrowed target. */
class CharacterAction : public Action, private Character::CompletionListener, private Core::ScriptableObject::Observer {
public:
	CharacterAction(ActionSystem &system, const Core::ScriptValue &target, uint resource, bool speech,
					int repeat, bool keepVisible, bool interruptible) : Action(system, interruptible), _target(target), _resource(resource), _speech(speech), _repeat(repeat), _keepVisible(keepVisible) {}
	~CharacterAction() override;
	void start() override;
	void halt() override;
	void pause() override;
	void resume() override;
	Core::ScriptableObject *clone() const override {
		return new CharacterAction(_system, _target, _resource, _speech, _repeat, _keepVisible, isInterruptible());
	}

private:
	Core::ScriptValue _target;
	uint _resource;
	bool _speech;
	int _repeat;
	bool _keepVisible;
	Character *_character = nullptr;
	Character *target() const { return static_cast<Character *>(_system.resolve(_target, "RCharacter")); }
	void detach();
	void characterFinished(Character &) override;
	void objectDestroyed(Core::ScriptableObject &) override { _character = nullptr; }
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
