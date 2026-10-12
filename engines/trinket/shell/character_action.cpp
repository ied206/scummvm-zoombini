/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/shell/character_action.h"

namespace Trinket {
namespace Shell {

CharacterAction::~CharacterAction() {
	detach();
}

void CharacterAction::detach() {
	if (_character) {
		_character->removeCompletionListener(*this);
		_character->removeObserver(*this);
		_character = nullptr;
	}
}

void CharacterAction::start() {
	ActionSystem::Scope scope(_system);
	detach();
	Action::start();
	_character = target();
	if (!_character) {
		finished();
		return;
	}
	_character->addObserver(*this);
	_character->setCallTime(_system.currentTime());
	if (_speech)
		_character->playSpeech(_resource, this);
	else
		_character->playAnim(_resource, _repeat, _keepVisible, this);
}

void CharacterAction::halt() {
	ActionSystem::Scope scope(_system);
	Action::halt();
	Character *character = target();
	detach();
	if (character)
		character->stop();
}

void CharacterAction::pause() {
	Action::pause();
	Character *character = target();
	if (character)
		character->pause(true, _system.currentTime());
}

void CharacterAction::resume() {
	Action::resume();
	Character *character = target();
	if (character)
		character->pause(false, _system.currentTime());
}

void CharacterAction::characterFinished(Character &) {
	ActionSystem::Scope scope(_system);
	detach();
	finished();
}

} // namespace Shell
} // namespace Trinket
