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

#ifndef TRINKET_TRINKET_H
#define TRINKET_TRINKET_H

#include "engines/engine.h"
#include "trinket/launcher/application.h"
#include "trinket/ompp/resource.h"
#include "trinket/shell/game.h"
#include "trinket/shell/music.h"
#include "trinket/shell/random.h"
#include "trinket/shell/savefiles.h"
#include "trinket/version.h"

namespace Trinket {

/*
 * Shared framework layers and their original module names:
 * OM: ompp32x.dll; the TLC launcher uses ompp32x.tlc.
 * Trinket Core: tcx.dll in Z3, trinketcore.dll in Reader Rabbit,
 * and TLCLLTrinketcore.dll in the TLC launcher.
 * Trinket Shell: trinket.dll in Z3, shelldll.dll in Reader Rabbit,
 * and TLCLLShelldll.dll in the TLC launcher.
 * PuzzleEngine.dll supplies the shared answer/container framework.
 * Shell supports both the launcher and gameplay; it is not the launcher itself.
 */

/** Static registration supplied by a game module, with no shared-to-game dependency. */
class TrinketEngine;

struct Subengine {
	Shell::Game *(*create)(TrinketEngine &vm);
	OM::ResourceProvider *(*createResources)();
	bool (*loadResources)(OM::ResourceProvider &resources, Common::Path &dataPath);
	const char *title;
	Shell::SaveFiles *(*createSaveFiles)(Common::SaveFileManager &manager, const Common::String &target);

	/** Reference components of the selected native game release. */
	const RuntimeVersions &gameVersions;

	/** Reference components of the selected native launcher release. */
	const RuntimeVersions &launcherVersions;
	const char *launcherProduct;
	int initialLocation;
	uint16 width, height;
};

class TrinketEngine : public Engine {
public:
	TrinketEngine(OSystem *system, const Subengine &subengine);
	~TrinketEngine() override;
	Common::Error run() override;

	/** Apply the configured PRNG algorithm to the shared gameplay stream. */
	void applyGameSettings() override;
	void syncSoundSettings() override;
	bool getGameSoundSettings(Shell::Game::SoundSettings &settings) const;
	void setGameSoundSettings(const Shell::Game::SoundSettings &settings);
	const OM::ResourceProvider &resources() const { return *_resources; }
	Common::Array<Common::String> playerNames() const;

	/** Remove a confirmed game-owned sign-in selection and keep the active index valid. */
	bool removeSavedPlayer(const Common::String &name);
	Audio::Mixer &mixer() const { return *_mixer; }
	Shell::Random &rnd() { return _random; }
	Shell::BackgroundMusic &music() { return _music; }
	OM::Canvas &gfx() { return *_gfx; }
	Common::Rect gameViewport() const { return Common::Rect(_subengine.width, _subengine.height); }
	Common::String debugPageCommand(const Common::String &command) {
		return _playing && _game ? _game->debugPageCommand(command) : "No game is active.\n";
	}
	/** May be absent while a game owns its sign-in screen, including during startup. */
	const Shell::SaveFiles::Player *activePlayer() const;
	bool hasGamePlayer(const Common::String &name) const;
	bool activateScriptPlayer(Shell::Game &game, const Common::String &name);
	bool activateScriptGame(Shell::Game &game, uint slot, bool replace, bool append = false);

private:
	Common::ScopedPtr<OM::ResourceProvider> _resources;
	Common::Path _dataPath;
	Shell::Random _random;
	Shell::BackgroundMusic _music;
	Common::ScopedPtr<OM::Canvas> _gfx;
	Common::ScopedPtr<Shell::SaveFiles> _saveFiles;
	Launcher::Application _launcher;
	Shell::Game *_game = nullptr;
	const Subengine &_subengine;
	int _player = -1;
	bool _playing = false;
	bool _bootSession = false;
	Shell::SaveFiles::Player _bootPlayer;
	void updateScreenSize();
	bool startWithoutPlayer();
	void launcherEvent(const Common::Event &event);
	bool activate(bool fresh, bool fromLauncher = true);
	bool switchSlot(const Shell::Game::SaveRequest &request);
	bool savePlayer();
	bool loadLauncher();
	bool writePlayer(Shell::Game &game, bool forceRegistry = false);
	Common::Array<uint> visiblePlayers() const;
	void returnToLauncher();
	static void logVersions(const RuntimeVersions &versions);
};

} // End of namespace Trinket

#endif
