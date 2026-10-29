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

#include "common/platform.h"
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
	/** Construct the game-specific runtime for the shared engine. */
	Shell::Game *(*create)(TrinketEngine &vm);
	/** Construct the game's resource provider. */
	OM::ResourceProvider *(*createResources)();
	/** Open the selected release's resource set and resolve its data directory. */
	bool (*loadResources)(OM::ResourceProvider &resources, Common::Path &dataPath);
	/** Game title shown by shared engine interfaces. */
	const char *title;
	/** Construct the game's registry and slot-file manager. */
	Shell::SaveFiles *(*createSaveFiles)(Common::SaveFileManager &manager, const Common::String &target);

	/** Reference components of the selected native game release. */
	const RuntimeVersions &gameVersions;

	/** Reference components of the selected native launcher release. */
	const RuntimeVersions &launcherVersions;
	/** Product identifier used to select the launcher layout. */
	const char *launcherProduct;
	/** Location assigned to a newly created game slot. */
	int initialLocation;
	/** Logical game viewport width in pixels. */
	uint16 width;
	/** Logical game viewport height in pixels. */
	uint16 height;
};

class TrinketEngine : public Engine {
public:
	/** Initialize shared services for the selected subengine and detected game platform. */
	TrinketEngine(OSystem *system, const Subengine &subengine, Common::Platform platform);
	/** Stop gameplay and release shared runtime services. */
	~TrinketEngine() override;
	/** Initialize resources and run the launcher and gameplay event loop. */
	Common::Error run() override;
	/** Expose the game options widget in the in-game ScummVM settings dialog. */
	bool hasFeature(EngineFeature feature) const override { return feature == kSupportsChangingOptionsDuringRuntime; }

	/** Apply the configured PRNG algorithm to the shared gameplay stream. */
	void applyGameSettings() override;
	/** Install the detected platform's default arrow beneath temporary game cursors. */
	void setDefaultCursor();
	/** Apply current configuration and active-save audio settings. */
	void syncSoundSettings() override;
	/** Read the active save's audio settings, returning false outside gameplay. */
	bool getGameSoundSettings(Shell::Game::SoundSettings &settings) const;
	/** Update the active save's audio settings and apply them to the mixer. */
	void setGameSoundSettings(const Shell::Game::SoundSettings &settings);
	/** Return the resource provider for the detected game release. */
	const OM::ResourceProvider &resources() const { return *_resources; }
	/** Return the game font cache; launcher fonts use a separate cache. */
	OM::FontCache &fonts() { return _fonts; }
	/** Return the visible registered player names. */
	Common::Array<Common::String> playerNames() const;

	/** Remove a confirmed player selection from the game sign-in screen and adjust the active index. */
	bool removeSavedPlayer(const Common::String &name);
	/** Return the mixer used for game and launcher audio. */
	Audio::Mixer &mixer() const { return *_mixer; }
	/** Owned gameplay random generator shared by scripts and native objects for this engine instance. */
	Shell::Random *_rnd = nullptr;
	/** Return the background-music controller shared across scenes. */
	Shell::BackgroundMusic &music() { return _music; }
	/** Return the canvas used to draw the current application state. */
	OM::Canvas &gfx() { return *_gfx; }
	/** Return the subengine's logical game-pixel viewport. */
	Common::Rect gameViewport() const { return Common::Rect(_subengine.width, _subengine.height); }
	/** Forward a debugger command to the active game's puzzle handler. */
	Common::String debugPageCommand(const Common::String &command) {
		return _playing && _game ? _game->debugPageCommand(command) : "No game is active.\n";
	}
	/** Launcher interface, absent while a subengine displays its sign-in screen. */
	const Shell::SaveFiles::Player *activePlayer() const;
	/** Check whether the game registry contains the supplied player name. */
	bool hasGamePlayer(const Common::String &name) const;
	/** Switch to the player requested by the active game's script. */
	bool activateScriptPlayer(Shell::Game &game, const Common::String &name);
	/** Switch or create the game slot requested by the active script. */
	bool activateScriptGame(Shell::Game &game, uint slot, bool replace, bool append = false);

private:
	/** ScummVM configuration key for path. */
	static constexpr const char *kConfigPath = "path";
	/** Script identifier for unknown. */
	static constexpr const char *kUnknownName = "unknown";
	/** Script identifier for rr1cloud9. */
	static constexpr const char *kRr1cloud9Name = "rr1cloud9";

	/** Resource provider for the detected release. */
	Common::ScopedPtr<OM::ResourceProvider> _resources;
	/** Game-font cache destroyed before its resource provider. */
	OM::FontCache _fonts;
	/** Resolved canonical directory containing the game's resources. */
	Common::Path _dataPath;
	/** Background-music state retained across game scenes. */
	Shell::BackgroundMusic _music;
	/** Canvas used by launcher and gameplay rendering. */
	Common::ScopedPtr<OM::Canvas> _gfx;
	/** Game-specific player registry and save-slot storage. */
	Common::ScopedPtr<Shell::SaveFiles> _saveFiles;
	/** Launcher screens, user selection and launcher audio state. */
	Launcher::Application _launcher;
	/** Game runtime currently prepared for play, or null before creation. */
	Shell::Game *_game = nullptr;
	/** Game-specific factories, viewport and native version metadata. */
	const Subengine &_subengine;
	/** Detected game platform used to select the default system arrow. */
	const Common::Platform _platform;
	/** Selected game-registry player index, or minus one without a named player. */
	int _player = -1;
	/** Whether input and drawing currently target gameplay. */
	bool _playing = false;
	/** Whether startup and game exit pass through the TLC launcher. */
	bool _useLauncher = true;
	/** Whether the current session uses temporary direct-start player state. */
	bool _bootSession = false;
	/** Temporary player record used for direct puzzle startup. */
	Shell::SaveFiles::Player _bootPlayer;
	/** Select the logical viewport required by the current launcher or game state. */
	void updateScreenSize();
	/** Prepare a direct game session without selecting a registered player. */
	bool startWithoutPlayer(bool fromLauncher = true);
	/** Dispatch launcher input and handle player selection or game-start requests. */
	void launcherEvent(const Common::Event &event);
	/** Start or resume the selected player's prepared game state. */
	bool activate(bool fresh, bool fromLauncher = true);
	/** Apply a pending script request to select, replace or append a game slot. */
	bool switchSlot(const Shell::Game::SaveRequest &request);
	/** Write the active player's game state and current location. */
	bool savePlayer();
	/** Load launcher resources and synchronize its player list. */
	bool loadLauncher();
	/** Write a game's checkpoint and update its player registry when required. */
	bool writePlayer(Shell::Game &game, bool forceRegistry = false);
	/** Return registry indices of players shown by the launcher. */
	Common::Array<uint> visiblePlayers() const;
	/** Save the current session and return to the configured launcher or exit path. */
	void exitGame();
	/** Write the selected native component version metadata to the debug log. */
	static void logVersions(const RuntimeVersions &versions);
};

} // End of namespace Trinket

#endif
