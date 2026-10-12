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

/** Owns saved World state and the application MPS session. */
class Game : public Shell::Game {
public:
	explicit Game(TrinketEngine &vm);
	~Game() override;
	static Shell::Game *create(TrinketEngine &vm) { return new Game(vm); }
	bool prepareState(Common::SeekableReadStream *save, const Common::String &playerName) override;
	bool prepareBoot(int bootParam) override;
	static bool parsePracticeBootParam(int bootParam, Location &location, uint &level);
	bool startPrepared(bool fresh, bool fromLauncher) override;
	SoundSettings soundSettings() const override;
	void setSoundSettings(const SoundSettings &settings) override;
	void applySoundSettings() override;
	bool save(Common::WriteStream &stream) const override { return _world.write(stream); }
	int currentLocation() const override { return _world.currentLocation(); }
	bool canReturnToLauncher() const override;
	bool takeReturnToLauncherRequest() override;
	void event(const Common::Event &event) override;
	void draw(OM::Canvas &canvas, uint32 time) override;
	Common::String debugPageCommand(const Common::String &command) override;

private:
	friend class ScriptSession;
	static constexpr int kPracticeBootParamPageFactor = 100;
	TrinketEngine &_vm;
	World _world;
	Common::ScopedPtr<ScriptSession> _session;
	Common::String _playerName;
	bool _practice = false, _quitRequested = false;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
