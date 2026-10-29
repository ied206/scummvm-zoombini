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

#ifndef TRINKET_ZOOMBINI3_GAME_H
#define TRINKET_ZOOMBINI3_GAME_H

#include "common/ptr.h"
#include "trinket/shell/game.h"
#include "trinket/zoombini3_world.h"

namespace Trinket {

class TrinketEngine;
namespace Zoombini3 {

class ScriptSession;

/** Run the Zoombini 3 application scripts and maintain saved world state. */
class Game : public Shell::Game {
public:
	/** Connect a Zoombinis world and script session to the engine. */
	explicit Game(TrinketEngine &vm);
	/** Stop the active script session before releasing game state. */
	~Game() override;
	/** Construct the Zoombinis runtime through the shared subengine interface. */
	static Shell::Game *create(TrinketEngine &vm) { return new Game(vm); }
	/** Load a player's save or initialize a fresh world before entering gameplay. */
	bool prepareState(Common::SeekableReadStream *save, const Common::String &playerName) override;
	/** Prepare a direct practice scene from the configured boot parameter. */
	bool prepareBoot(int bootParam) override;
	/** Decode a supported practice location and difficulty without changing outputs on failure. */
	static bool parsePracticeBootParam(int bootParam, Location &location, uint &level);
	/** Enter the prepared world through its new-game, resume or launcher path. */
	bool startPrepared(bool fresh, bool fromLauncher) override;
	/** Return music, effects, speech and mute preferences stored in the world. */
	SoundSettings soundSettings() const override;
	/** Store the supplied audio preferences in the world. */
	void setSoundSettings(const SoundSettings &settings) override;
	/** Apply the world's saved volumes and mute preference to the mixer. */
	void applySoundSettings() override;
	/** Serialize the current world to the supplied save stream. */
	bool save(Common::WriteStream &stream) const override { return _world.write(stream); }
	/** Return the active world's location code. */
	int currentLocation() const override { return _world.currentLocation(); }
	/** Check whether the active script state allows Escape to leave gameplay. */
	bool canQuitWithEscape() const override;
	/** Return and clear the game's pending quit request. */
	bool takeQuitRequest() override;
	/** Forward input to the active script session. */
	void event(const Common::Event &event) override;
	/** Advance and draw the active script session at a millisecond timestamp. */
	void draw(OM::Canvas &canvas, uint32 time) override;
	/** Run an active-page debugger command and return its response. */
	Common::String debugPageCommand(const Common::String &command) override;

private:
	/** Suffix identifying  Practice. */
	static constexpr const char *kPracticeSuffix = "_Practice";
	/** Script identifier for boot Practice. */
	static constexpr const char *kBootPracticeName = "Boot_Practice";

	friend class ScriptSession;
	/** Multiplier separating a practice location from its difficulty digit. */
	static constexpr int kPracticeBootParamPageFactor = 100;
	/** Engine supplying shared resources, drawing and audio services. */
	TrinketEngine &_vm;
	/** Persistent character, location, activity and preference state. */
	World _world;
	/** Current script session and its scene objects. */
	Common::ScopedPtr<ScriptSession> _session;
	/** Name associated with the prepared save state. */
	Common::String _playerName;
	/** Whether the prepared world uses practice progression. */
	bool _practice = false;
	/** Whether the script session requested a return from gameplay. */
	bool _quitRequested = false;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
