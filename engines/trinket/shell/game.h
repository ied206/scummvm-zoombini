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

#ifndef TRINKET_SHELL_GAME_H
#define TRINKET_SHELL_GAME_H

#include "common/events.h"
#include "common/stream.h"
#include "trinket/ompp/canvas.h"

namespace Trinket {
namespace Shell {

/** Game-specific state and scenes hosted by the shared Trinket application. */
class Game {
public:
	/** Save-specific controls, using Audio::Mixer::kMaxMixerVolume as full scale. */
	struct SoundSettings {
		int music, effects, speech;
		bool mute;
	};
	struct SaveRequest {
		enum Action {
			kNone00 = 0,
			kAppend01 = 1,
			kLoad02 = 2,
			kReplace03 = 3,
		};
		Action action = kNone00;
		uint slot = 0;
		SaveRequest() = default;
		SaveRequest(Action requestedAction, uint requestedSlot = 0) : action(requestedAction), slot(requestedSlot) {}
	};
	virtual ~Game() {}
	/** Decode or reset state without entering a scene or consuming its random draws. */
	virtual bool prepareState(Common::SeekableReadStream *save, const Common::String &playerName) = 0;

	/** Prepare a disposable direct-entry session; false leaves normal startup in control. */
	virtual bool prepareBoot(int bootParam) { return false; }
	virtual bool startPrepared(bool fresh, bool fromLauncher) = 0;
	virtual SoundSettings soundSettings() const = 0;
	virtual void setSoundSettings(const SoundSettings &settings) = 0;
	virtual void applySoundSettings() = 0;
	virtual Common::String requestedPlayer() const { return Common::String(); }
	virtual void clearPlayerRequest() {}
	virtual SaveRequest takeSaveRequest() { return SaveRequest(); }
	virtual bool save(Common::WriteStream &stream) const = 0;
	virtual int currentLocation() const = 0;
	virtual bool canReturnToLauncher() const { return true; }
	virtual bool takeReturnToLauncherRequest() { return false; }
	virtual void event(const Common::Event &event) = 0;
	virtual void draw(OM::Canvas &canvas, uint32 time) = 0;
	virtual Common::String debugPageCommand(const Common::String &command) { return "No puzzle is active.\n"; }
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
