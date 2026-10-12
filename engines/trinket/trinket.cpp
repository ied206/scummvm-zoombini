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

#include "trinket/trinket.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/fs.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/wincursor.h"
#include "gui/debugger.h"
#include "trinket/dialogs.h"

namespace Trinket {

class TrinketDebugger : public GUI::Debugger {
public:
	explicit TrinketDebugger(TrinketEngine &vm) : _vm(vm) {
		registerCmd("page", WRAP_METHOD(TrinketDebugger, page));
		registerCmd("puzzle", WRAP_METHOD(TrinketDebugger, page));
	}

private:
	TrinketEngine &_vm;
	bool page(int argc, const char **argv) {
		if (argc < 2) {
			debugPrintf("Usage: page <command> [arguments]\n");
			return true;
		}
		Common::String command = argv[1];
		for (int i = 2; i < argc; i++)
			command += Common::String(" ") + argv[i];
		Common::String result = _vm.debugPageCommand(command);
		debugPrintf("%s", result.c_str());
		return true;
	}
};

TrinketEngine::TrinketEngine(OSystem *system, const Subengine &subengine) : Engine(system), _resources(subengine.createResources()), _music(*_mixer, *_resources),
																			_saveFiles(subengine.createSaveFiles(*system->getSavefileManager(), _targetName)),
																			_launcher(*system->getSavefileManager(), _targetName, *_mixer),
																			_subengine(subengine) {
	setDebugger(new TrinketDebugger(*this));
	_mainMenuDialog = new TrinketMainMenuDialog(*this);
	_dataPath = ConfMan.getPath("path");
}

TrinketEngine::~TrinketEngine() {
	delete _game;
}

bool TrinketEngine::getGameSoundSettings(Shell::Game::SoundSettings &settings) const {
	if (!_playing || !_game)
		return false;
	settings = _game->soundSettings();
	return true;
}

void TrinketEngine::setGameSoundSettings(const Shell::Game::SoundSettings &settings) {
	if (!_playing || !_game)
		return;
	_game->setSoundSettings(settings);
	_game->applySoundSettings();
	savePlayer();
}

void TrinketEngine::applyGameSettings() {
	_random.setAlgorithm(Shell::Random::getPrngAlgorithm());
}

void TrinketEngine::syncSoundSettings() {
	if (_playing && _game)
		_game->applySoundSettings();
	else
		Engine::syncSoundSettings();
}

bool TrinketEngine::savePlayer() {
	if (_bootSession || (_playing && _player < 0))
		return true;
	if (!_game || _player < 0)
		return false;
	return writePlayer(*_game);
}

const Shell::SaveFiles::Player *TrinketEngine::activePlayer() const {
	if (_bootSession)
		return &_bootPlayer;
	if (_player < 0 || _saveFiles->players().size() <= static_cast<uint>(_player))
		return nullptr;
	return &_saveFiles->players()[_player];
}

bool TrinketEngine::writePlayer(Shell::Game &game, bool forceRegistry) {
	if (_bootSession || !activePlayer())
		return false;
	debug(1, "Trinket: saving player");
	Shell::SaveFiles::Player &player = _saveFiles->players()[_player];
	Common::ScopedPtr<Common::OutSaveFile> stream(_system->getSavefileManager()->openForSaving(_saveFiles->stateName(_player, player.current)));
	if (!stream || !game.save(*stream)) {
		warning("Trinket: could not write game state for player '%s', slot %u", player.name.c_str(), player.current);
		return false;
	}
	stream->finalize();
	if (stream->err()) {
		warning("Trinket: could not finish saving player '%s', slot %u", player.name.c_str(), player.current);
		return false;
	}
	stream.reset();
	if (!_saveFiles->storeLocation(_player, game.currentLocation(), forceRegistry)) {
		warning("Trinket: could not store the registry for player '%s', slot %u", player.name.c_str(), player.current);
		return false;
	}
	debug(1, "Trinket: player saved");
	return true;
}

Common::Array<uint> TrinketEngine::visiblePlayers() const {
	Common::Array<uint> visible;
	for (uint i = 0; i < _saveFiles->players().size(); i++) {
		if (_saveFiles->isVisible(_saveFiles->players()[i].name))
			visible.push_back(i);
	}
	return visible;
}

Common::Array<Common::String> TrinketEngine::playerNames() const {
	Common::Array<Common::String> names;
	for (uint index : visiblePlayers())
		names.push_back(_saveFiles->players()[index].name);
	return names;
}

bool TrinketEngine::removeSavedPlayer(const Common::String &name) {
	if (_bootSession)
		return false;
	for (uint index = 0; index < _saveFiles->players().size(); index++) {
		if (_saveFiles->players()[index].name != name)
			continue;
		uint32 activeID = activePlayer() ? activePlayer()->id : 0;
		bool result = _saveFiles->remove(index);
		_player = -1;
		for (uint i = 0; i < _saveFiles->players().size(); i++)
			if (_saveFiles->players()[i].id == activeID)
				_player = i;
		return result;
	}
	return false;
}

bool TrinketEngine::startWithoutPlayer() {
	if (_playing)
		return false;
	int previousPlayer = _player;
	_player = -1;
	Common::ScopedPtr<Shell::Game> candidate(_subengine.create(*this));
	if (!candidate || !candidate->prepareState(nullptr, Common::String()) || !candidate->startPrepared(true, true)) {
		candidate.reset();
		_player = previousPlayer;
		_music.reset();
		syncSoundSettings();
		return false;
	}
	delete _game;
	_game = candidate.release();
	_player = -1;
	_playing = true;
	_launcher.suspend();
	return true;
}

void TrinketEngine::returnToLauncher() {
	if (!savePlayer())
		return;
	if (_bootSession) {
		delete _game;
		_game = nullptr;
		_playing = false;
		_bootSession = false;
		_music.reset();
		syncSoundSettings();
		if (!loadLauncher())
			quitGame();
		return;
	}
	if (0 <= _player) {
		Common::String name = _saveFiles->primaryName(_saveFiles->players()[_player].name);
		for (uint i = 0; i < _saveFiles->players().size(); i++) {
			if (_saveFiles->players()[i].name == name) {
				_player = i;
				break;
			}
		}
	}
	delete _game;
	_game = nullptr;
	_playing = false;
	_music.reset();
	syncSoundSettings();
	_launcher.resume();
}

bool TrinketEngine::hasGamePlayer(const Common::String &name) const {
	if (_bootSession)
		return _bootPlayer.name == name;
	for (const Shell::SaveFiles::Player &player : _saveFiles->players())
		if (player.name == name)
			return true;
	return false;
}

bool TrinketEngine::activateScriptPlayer(Shell::Game &game, const Common::String &name) {
	const Shell::SaveFiles::Player *active = activePlayer();
	if (!active)
		return false;
	if (name == active->name)
		return true;
	if (_bootSession || !writePlayer(game))
		return false;
	Common::MemoryWriteStreamDynamic previousState(DisposeAfterUse::YES);
	if (!game.save(previousState))
		return false;
	const int previousPlayer = _player;
	const Common::String previousName = active->name;
	const Common::Array<Shell::SaveFiles::Player> previousPlayers = _saveFiles->players();
	Shell::SaveFiles::Checkpoint checkpoint;
	bool checkpointed = false;
	auto rollback = [&]() {
		if (checkpointed)
			_saveFiles->restore(checkpoint);
		_player = previousPlayer;
		_saveFiles->players() = previousPlayers;
		Common::MemoryReadStream stream(previousState.getData(), previousState.size());
		if (!game.prepareState(&stream, previousName))
			error("Could not restore the in-memory game state");
		game.applySoundSettings();
		return false;
	};
	int selected = -1;
	for (uint i = 0; i < _saveFiles->players().size(); i++)
		if (_saveFiles->players()[i].name == name)
			selected = i;
	bool fresh = selected < 0;
	if (fresh)
		selected = _saveFiles->add(name, _subengine.initialLocation);
	if (selected < 0)
		return false;
	const Shell::SaveFiles::Player &player = _saveFiles->players()[selected];
	Common::ScopedPtr<Common::InSaveFile> stream;
	if (!fresh) {
		stream.reset(_system->getSavefileManager()->openForLoading(_saveFiles->stateName(selected, player.current)));
		if (!stream)
			return rollback();
	}
	if (!game.prepareState(stream.get(), name))
		return rollback();
	_player = selected;
	if (fresh) {
		if (!_saveFiles->checkpoint(selected, player.current, checkpoint))
			return rollback();
		checkpointed = true;
		if (!writePlayer(game, true))
			return rollback();
	}
	return true;
}

bool TrinketEngine::activateScriptGame(Shell::Game &game, uint slot, bool replace, bool append) {
	if (_bootSession || !activePlayer())
		return false;
	Shell::SaveFiles::Player &player = _saveFiles->players()[_player];
	if (append)
		slot = player.count + 1;
	if (!slot || player.locations.size() < slot || (!append && player.count < slot))
		return false;
	if (!append && !replace && slot == player.current)
		return true;
	if (!writePlayer(game))
		return false;
	Common::MemoryWriteStreamDynamic previousState(DisposeAfterUse::YES);
	if (!game.save(previousState))
		return false;
	const Shell::SaveFiles::Player previousPlayer = player;
	Shell::SaveFiles::Checkpoint checkpoint;
	if (append && !_saveFiles->checkpoint(_player, slot, checkpoint))
		return false;
	auto rollback = [&]() {
		if (append)
			_saveFiles->restore(checkpoint);
		player = previousPlayer;
		Common::MemoryReadStream previous(previousState.getData(), previousState.size());
		if (!game.prepareState(&previous, player.name))
			error("Could not restore the in-memory game state");
		game.applySoundSettings();
		return false;
	};
	Common::ScopedPtr<Common::InSaveFile> stream;
	if (!append && !replace) {
		stream.reset(_system->getSavefileManager()->openForLoading(_saveFiles->stateName(_player, slot)));
		if (!stream)
			return false;
	}
	if (!game.prepareState(stream.get(), player.name))
		return rollback();
	player.current = slot;
	if (append)
		player.count = slot;
	if (append && !writePlayer(game, true))
		return rollback();
	return true;
}

bool TrinketEngine::activate(bool fresh, bool fromLauncher) {
	debug(1, "Trinket: activating player %d, fresh %d", _player, fresh);
	if (_bootSession || !activePlayer())
		return false;
	Shell::SaveFiles::Player &player = _saveFiles->players()[_player];
	Common::ScopedPtr<Common::InSaveFile> stream;
	if (!fresh) {
		stream.reset(_system->getSavefileManager()->openForLoading(_saveFiles->stateName(_player, player.current)));
		if (!stream) {
			warning("Trinket: could not open save file %s", _saveFiles->stateName(_player, player.current).c_str());
			return false;
		}
	}
	Shell::Game *candidate = _subengine.create(*this);
	if (!candidate || !candidate->prepareState(stream.get(), player.name)) {
		delete candidate;
		warning("Could not load game state or required resources: player '%s', slot %u", player.name.c_str(), player.current);
		return false;
	}
	stream.reset();
	Shell::SaveFiles::Checkpoint checkpoint;
	if (fresh && !_saveFiles->checkpoint(_player, player.current, checkpoint)) {
		delete candidate;
		return false;
	}
	// New saves are persisted before script initialization and movie/scene entry.
	// Opening an existing save must not rewrite its state merely by loading it.
	if (fresh && !writePlayer(*candidate, true)) {
		delete candidate;
		_saveFiles->restore(checkpoint);
		return false;
	}
	// Old scene destruction can consume random draws before the new scene starts.
	delete _game;
	_game = nullptr;
	_playing = false;
	if (!candidate->startPrepared(fresh, fromLauncher)) {
		delete candidate;
		_music.reset();
		syncSoundSettings();
		_launcher.resume();
		if (fresh)
			_saveFiles->restore(checkpoint);
		warning("Could not load game state or required resources: game startup failed");
		return false;
	}
	_game = candidate;
	_playing = true;
	logVersions(_subengine.gameVersions);
	_launcher.suspend();
	debug(1, "Trinket: game resources ready");
	return true;
}

bool TrinketEngine::switchSlot(const Shell::Game::SaveRequest &request) {
	if (_bootSession || !_playing || !_game || !activePlayer() || request.action == Shell::Game::SaveRequest::kNone00)
		return false;
	Shell::SaveFiles::Player &player = _saveFiles->players()[_player];
	bool append = request.action == Shell::Game::SaveRequest::kAppend01;
	bool replace = request.action == Shell::Game::SaveRequest::kReplace03;
	uint slot = append ? player.count + 1 : request.slot;
	if (!slot || player.locations.size() < slot || (!append && player.count < slot))
		return false;
	if (!append && !replace && slot == player.current)
		return true;
	// Persist the departing game before changing the save's active slot.
	if (!savePlayer())
		return false;
	Shell::SaveFiles::Player previous = player;
	Common::ScopedPtr<Common::InSaveFile> stream;
	if (!append)
		stream.reset(_system->getSavefileManager()->openForLoading(_saveFiles->stateName(_player, slot)));
	bool available = stream.get() != nullptr;
	if (!append && !replace && !available) {
		warning("Trinket: could not open save file %s", _saveFiles->stateName(_player, slot).c_str());
		return false;
	}
	bool fresh = append || replace;
	Common::ScopedPtr<Shell::Game> candidate(_subengine.create(*this));
	if (!candidate || !candidate->prepareState(fresh ? nullptr : stream.get(), player.name)) {
		warning("Could not load game state or required resources: player '%s', slot %u", player.name.c_str(), slot);
		return false;
	}
	stream.reset();
	Shell::SaveFiles::Checkpoint checkpoint;
	if ((append || !available) && !_saveFiles->checkpoint(_player, slot, checkpoint))
		return false;
	player.current = slot;
	if (append)
		player.count = slot;
	// Replacing a readable slot resets its live world, but retains its file until a later save.
	if ((append || !available) && !writePlayer(*candidate, append)) {
		player = previous;
		_saveFiles->restore(checkpoint);
		return false;
	}
	delete _game;
	_game = nullptr;
	_playing = false;
	if (!candidate->startPrepared(fresh, false)) {
		candidate.reset();
		_music.reset();
		syncSoundSettings();
		_launcher.resume();
		player = previous;
		if (append || !available)
			_saveFiles->restore(checkpoint);
		warning("Could not load game state or required resources: slot %u startup failed", slot);
		return false;
	}
	_game = candidate.release();
	_playing = true;
	return true;
}

void TrinketEngine::launcherEvent(const Common::Event &event) {
	Launcher::Application::Request request = _launcher.event(event);
	if (request == Launcher::Application::kNone00)
		return;
	if (request == Launcher::Application::kQuit05) {
		quitGame();
		return;
	}
	if (request == Launcher::Application::kStartGame06) {
		if (!startWithoutPlayer())
			warning("Trinket: could not start the game's sign-in screen");
		return;
	}
	Common::String name = _launcher.selectedName();
	_player = -1;
	for (uint i = 0; i < _saveFiles->players().size(); i++) {
		if (_saveFiles->players()[i].name == name)
			_player = i;
	}
	if (request == Launcher::Application::kDelete02) {
		if (!((_player < 0 || _saveFiles->remove(_player)) && _launcher.removeName()))
			warning("Trinket: could not remove all files for player '%s'", name.c_str());
		_player = -1;
		return;
	}
	if (request == Launcher::Application::kPlay01) {
		if (!_launcher.commitName()) {
			warning("Trinket: could not save the launcher user registry");
			return;
		}
		bool fresh = _player < 0;
		if (fresh) {
			_player = _saveFiles->add(name, _subengine.initialLocation);
			if (_player < 0)
				warning("Trinket: could not create the player record '%s'", name.c_str());
		}
		if (0 <= _player && !activate(fresh) && fresh) {
			_saveFiles->players().remove_at(_player);
			_player = -1;
		}
	} else if (request == Launcher::Application::kSlot04 && 0 <= _player) {
		if (_launcher.requestedSlot() <= _saveFiles->players()[_player].count)
			_saveFiles->players()[_player].current = _launcher.requestedSlot();
	} else if (request == Launcher::Application::kNewGame03 && 0 <= _player) {
		Shell::SaveFiles::Player &player = _saveFiles->players()[_player];
		if (player.count < _saveFiles->maxSlots()) {
			Shell::SaveFiles::Player previous = player;
			player.count += 1;
			player.current = player.count;
			if (!activate(true))
				player = previous;
		}
	}
}

void TrinketEngine::logVersions(const RuntimeVersions &versions) {
	const ComponentVersion *components[] = {
		&versions.om,
		&versions.core,
		&versions.shell,
		&versions.puzzle,
		&versions.application,
	};
	for (uint i = 0; i < ARRAYSIZE(components); i++) {
		const ComponentVersion &component = *components[i];
		if (component.module)
			debug(0, "Trinket original reference [%s] %s: version %s; built %s", versions.label ? versions.label : "unknown", component.module,
				  component.version ? component.version : "unknown", component.builtAt.toString().c_str());
	}
}

bool TrinketEngine::loadLauncher() {
	debug(0, "Trinket: loading player registry");
	if (!_saveFiles->load()) {
		warning("Trinket: could not load the player registry");
		return false;
	}
	if (!_launcher.load(_dataPath, _subengine.launcherProduct)) {
		warning("Trinket: could not load the TLC launcher resources or user registry");
		return false;
	}
	Common::Array<uint> visible = visiblePlayers();
	_player = visible.empty() ? -1 : static_cast<int>(visible[0]);
	debug(0, "Trinket launcher ready");
	logVersions(_subengine.launcherVersions);
	return true;
}

void TrinketEngine::updateScreenSize() {
	int width = _playing ? _subengine.width : 800;
	int height = _playing ? _subengine.height : 600;
	if (_gfx && _gfx->surface().w == width && _gfx->surface().h == height)
		return;
	Graphics::PixelFormat format = Graphics::PixelFormat::createFormatRGBA32();
	initGraphics(width, height, &format);
	_gfx.reset(new OM::Canvas(width, height));
}

Common::Error TrinketEngine::run() {
	syncSoundSettings();
	debug(0, "Trinket: loading game resources");
	if (!_subengine.loadResources(*_resources, _dataPath)) {
		warning("Trinket: could not load game resources");
		return Common::kNoError;
	}
	debug(0, "Trinket: initializing canvas");
	updateScreenSize();
	Common::ScopedPtr<Graphics::Cursor> launcherCursor(Graphics::makeDefaultWinCursor());
	CursorMan.replaceCursor(launcherCursor.get());
	CursorMan.showMouse(true);
	int bootParam = ConfMan.getInt("boot_param");
	if (bootParam) {
		Common::ScopedPtr<Shell::Game> candidate(_subengine.create(*this));
		if (candidate && candidate->prepareBoot(bootParam)) {
			_bootSession = true;
			_bootPlayer.name = _saveFiles->bootName();
			_bootPlayer.id = 0;
			_bootPlayer.count = _bootPlayer.current = 1;
			_bootPlayer.locations.resize(_saveFiles->maxSlots(), candidate->currentLocation());
			if (candidate->startPrepared(false, false)) {
				_game = candidate.release();
				_playing = true;
				logVersions(_subengine.gameVersions);
				debug(1, "Trinket: direct practice ready; persistent saves are not loaded or written");
			} else {
				candidate.reset();
				_bootSession = false;
				_music.reset();
				syncSoundSettings();
				warning("Could not load game state or required resources: direct practice startup failed");
			}
		} else {
			warning("Trinket: ignoring unsupported boot_param %d", bootParam);
		}
	}
	if (!_playing && !loadLauncher())
		return Common::kNoError;
	while (!shouldQuit()) {
		updateScreenSize();
		Common::Event event;
		while (_eventMan->pollEvent(event)) {
			if (_playing) {
				if (_game->canReturnToLauncher() && event.type == Common::EVENT_KEYDOWN && (event.kbd.keycode == Common::KEYCODE_ESCAPE || event.kbd.ascii == 27)) {
					returnToLauncher();
				} else {
					_game->event(event);
					if (_game->takeReturnToLauncherRequest())
						returnToLauncher();
					else {
						Shell::Game::SaveRequest request = _game->takeSaveRequest();
						if (request.action != Shell::Game::SaveRequest::kNone00)
							switchSlot(request);
					}
				}
			} else {
				launcherEvent(event);
			}
		}
		if (_playing && !_game->requestedPlayer().empty()) {
			Common::String requested = _game->requestedPlayer();
			_game->clearPlayerRequest();
			if (_bootSession) {
				returnToLauncher();
				continue;
			}
			int previousPlayer = _player;
			if (savePlayer()) {
				int target = -1;
				for (uint i = 0; i < _saveFiles->players().size(); i++) {
					if (_saveFiles->players()[i].name == requested)
						target = i;
				}
				bool fresh = target < 0;
				if (fresh)
					target = _saveFiles->add(requested, _subengine.initialLocation);
				if (0 <= target) {
					_player = target;
					if (!activate(fresh, false)) {
						if (fresh)
							_saveFiles->players().remove_at(target);
						_player = previousPlayer;
					}
				} else {
					warning("Trinket: could not create the player record '%s'", requested.c_str());
				}
			}
		}
		updateScreenSize();
		OM::Canvas &canvas = *_gfx;
		if (_playing) {
			_game->draw(canvas, _system->getMillis());
		} else {
			_launcher.draw(canvas);
		}
		_system->copyRectToScreen(canvas.surface().getPixels(), canvas.surface().pitch, 0, 0, canvas.surface().w, canvas.surface().h);
		_system->updateScreen();
		_system->delayMillis(20);
	}
	if (_playing)
		savePlayer();
	return Common::kNoError;
}

} // namespace Trinket
