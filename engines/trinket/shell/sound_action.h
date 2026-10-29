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

/** Play one queued sound and signal action completion when playback ends. */
class SoundAction : public Action {
public:
	/** Configure one queued sound with its resource path, mixer category and repeat count. */
	SoundAction(ActionSystem &system, Audio::Mixer &mixer, const OM::ResourceProvider &resources,
				const Common::String &path, Audio::Mixer::SoundType type, bool interruptible, uint repeat = 1);
	/** Load and play the configured sound. */
	void start() override;
	/** Stop playback and halt the action. */
	void halt() override;
	/** Pause playback on this action's sound handle. */
	void pause() override;
	/** Resume playback on this action's sound handle. */
	void resume() override;
	/** Create an inactive action with the same sound settings. */
	Core::ScriptableObject *clone() const override;

private:
	/** Mixer used to play the queued sound. */
	Audio::Mixer &_mixer;
	/** Provider used to open the sound resource. */
	const OM::ResourceProvider &_resources;
	/** Resource path of the sound played when the action starts. */
	Common::String _path;
	/** Mixer category controlling how the sound is mixed. */
	Audio::Mixer::SoundType _type;
	/** Number of sound repetitions requested by the action. */
	uint _repeat;
	/** Playback handle and decoded stream control for the active sound. */
	OM::Sound _sound;
	/** Finish the action when its sound is no longer playing. */
	void updateAction(uint32 time) override;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
