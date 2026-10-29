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

/** Queue speech or animation on a selected character. */
class CharacterAction : public Action, private Character::CompletionListener, private Core::ScriptableObject::Observer {
public:
	/** Configure a character speech or animation operation and its repeat policy. */
	CharacterAction(ActionSystem &system, const Core::ScriptValue &target, uint resource, bool speech,
					int repeat, bool keepVisible, bool interruptible) : Action(system, interruptible), _target(target), _resource(resource), _speech(speech), _repeat(repeat), _keepVisible(keepVisible) {}
	/** Detach from any character still running the operation. */
	~CharacterAction() override;
	/** Resolve the character and begin the configured speech or animation. */
	void start() override;
	/** Stop the character operation and detach its callbacks. */
	void halt() override;
	/** Pause the active character operation. */
	void pause() override;
	/** Resume the active character operation. */
	void resume() override;
	/** Copy the character operation settings into an inactive action. */
	Core::ScriptableObject *clone() const override {
		return new CharacterAction(_system, _target, _resource, _speech, _repeat, _keepVisible, isInterruptible());
	}

private:
	/** Script value identifying the character to control. */
	Core::ScriptValue _target;
	/** Character resource ID for the speech or animation. */
	uint _resource;
	/** Whether the operation plays speech rather than a gesture animation. */
	bool _speech;
	/** Number of animation repeats requested by the action. */
	int _repeat;
	/** Whether the character remains visible when the operation ends. */
	bool _keepVisible;
	/** Character currently watched for operation completion. */
	Character *_character = nullptr;
	/** Resolve the script target as an RCharacter instance, or return null. */
	Character *target() const { return static_cast<Character *>(_system.resolve(_target, "RCharacter")); }
	/** Remove completion and destruction callbacks from the character. */
	void detach();
	/** Detach from the character and complete the action. */
	void characterFinished(Character &) override;
	/** Clear the character pointer when the target is destroyed. */
	void objectDestroyed(Core::ScriptableObject &) override { _character = nullptr; }
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
