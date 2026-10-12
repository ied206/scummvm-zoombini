/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_SHELL_SOUND_ACTION_H
#define TRINKET_SHELL_SOUND_ACTION_H

#include "trinket/ompp/sound.h"
#include "trinket/shell/action.h"

namespace Trinket {
namespace Shell {

/** One queued sound, with its own mixer handle and completion lifetime. */
class SoundAction : public Action {
public:
	SoundAction(ActionSystem &system, Audio::Mixer &mixer, const OM::ResourceProvider &resources,
				const Common::String &path, Audio::Mixer::SoundType type, bool interruptible, uint repeat = 1);
	void start() override;
	void halt() override;
	void pause() override;
	void resume() override;
	Core::ScriptableObject *clone() const override;

private:
	Audio::Mixer &_mixer;
	const OM::ResourceProvider &_resources;
	Common::String _path;
	Audio::Mixer::SoundType _type;
	uint _repeat;
	OM::Sound _sound;
	void updateAction(uint32 time) override;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
