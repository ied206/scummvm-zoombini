/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/shell/sound_action.h"

#include "common/debug.h"

namespace Trinket {
namespace Shell {

SoundAction::SoundAction(ActionSystem &system, Audio::Mixer &mixer, const OM::ResourceProvider &resources,
						 const Common::String &path, Audio::Mixer::SoundType type, bool interruptible, uint repeat) : Action(system, interruptible), _mixer(mixer), _resources(resources), _path(path), _type(type), _repeat(repeat), _sound(mixer) {
}

void SoundAction::start() {
	Action::start();
	if (!_sound.play(_resources, _path, _type, _repeat)) {
		warning("Could not start queued sound %s", _path.c_str());
		finished();
	}
}

void SoundAction::halt() {
	_sound.stop();
	Action::halt();
}

void SoundAction::pause() {
	_sound.pause(true);
	Action::pause();
}

void SoundAction::resume() {
	_sound.pause(false);
	Action::resume();
}

void SoundAction::updateAction(uint32) {
	if (!_sound.isPlaying())
		finished();
}

Core::ScriptableObject *SoundAction::clone() const {
	return new SoundAction(_system, _mixer, _resources, _path, _type, isInterruptible(), _repeat);
}

} // namespace Shell
} // namespace Trinket
