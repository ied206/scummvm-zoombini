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
		/** Stored music volume in the subengine's native volume scale. */
		int music;
		/** Stored sound-effects volume in the subengine's native volume scale. */
		int effects;
		/** Stored speech volume in the subengine's native volume scale. */
		int speech;
		/** Whether all game audio is muted. */
		bool mute;
	};
	struct SaveRequest {
		enum Action {
			kNone00 = 0,
			kAppend01 = 1,
			kLoad02 = 2,
			kReplace03 = 3,
		};
		/** Pending save operation, or none when no operation was requested. */
		Action action = kNone00;
		/** One-based slot affected by the pending operation. */
		uint slot = 0;
		/** Describe a save operation and its slot, defaulting to no request. */
		SaveRequest() = default;
		/** Describe a save operation and its slot, defaulting to no request. */
		SaveRequest(Action requestedAction, uint requestedSlot = 0) : action(requestedAction), slot(requestedSlot) {}
	};
	/** Allow destruction through the shared game interface. */
	virtual ~Game() {}
	/** Decode or reset state without entering a scene or consuming its random draws. */
	virtual bool prepareState(Common::SeekableReadStream *save, const Common::String &playerName) = 0;

	/** Prepare a disposable direct-entry session; false leaves normal startup in control. */
	virtual bool prepareBoot(int bootParam) { return false; }
	/** Enter the already prepared game state, selecting new-game or resumed setup. */
	virtual bool startPrepared(bool fresh, bool fromLauncher) = 0;
	/** Return the active save's audio preferences. */
	virtual SoundSettings soundSettings() const = 0;
	/** Replace the active save's audio preferences. */
	virtual void setSoundSettings(const SoundSettings &settings) = 0;
	/** Apply the active save's volumes and mute state to the mixer. */
	virtual void applySoundSettings() = 0;
	/** Return the player name requested by the game, or an empty string if none is pending. */
	virtual Common::String requestedPlayer() const { return Common::String(); }
	/** Clear the pending request to change players. */
	virtual void clearPlayerRequest() {}
	/** Return and clear a pending save-management operation. */
	virtual SaveRequest takeSaveRequest() { return SaveRequest(); }
	/** Write the active game's checkpoint to the supplied stream. */
	virtual bool save(Common::WriteStream &stream) const = 0;
	/** Return the active game location used by the save registry. */
	virtual int currentLocation() const = 0;
	/** Return whether Escape may leave the current game state. */
	virtual bool canQuitWithEscape() const { return true; }
	/** Return and clear a pending request to leave gameplay. */
	virtual bool takeQuitRequest() { return false; }
	/** Dispatch an input event to the active game state. */
	virtual void event(const Common::Event &event) = 0;
	/** Advance and draw the active game at the supplied millisecond timestamp. */
	virtual void draw(OM::Canvas &canvas, uint32 time) = 0;
	/** Run a debugger command for the active puzzle and return its response text. */
	virtual Common::String debugPageCommand(const Common::String &command) { return "No puzzle is active.\n"; }
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
