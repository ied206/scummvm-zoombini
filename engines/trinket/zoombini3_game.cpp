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

#include "trinket/zoombini3_game.h"

#include "common/config-manager.h"
#include "common/debug.h"
#include "trinket/trinket.h"
#include "trinket/zoombini3_script_session.h"

namespace Trinket {
namespace Zoombini3 {

Common::String Game::debugPageCommand(const Common::String &command) {
	return _session ? _session->debugCommand(command) : "No puzzle is active.\n";
}

Game::Game(TrinketEngine &vm) : _vm(vm) {
	WorldPort::resetWorksheet();
}

Game::~Game() {
	_session.reset();
}

bool Game::prepareState(Common::SeekableReadStream *save, const Common::String &playerName) {
	if (save && !_world.read(*save))
		return false;
	_playerName = playerName;
	_practice = playerName.hasSuffix("_Practice");
	if (save)
		return true;
	_world.resetNative();
	_world.initializeNewGame(_practice);
	// ScummVM deliberately inherits launcher preferences instead of the original defaults.
	SoundSettings settings = {
		ConfMan.getInt("music_volume"),
		ConfMan.getInt("sfx_volume"),
		ConfMan.getInt("speech_volume"),
		ConfMan.getBool("mute"),
	};
	setSoundSettings(settings);
	if (_session)
		_session->inheritInitialSoundSettings();
	return true;
}

bool Game::startPrepared(bool fresh, bool fromLauncher) {
	applySoundSettings();
	_session.reset(new ScriptSession(*this));
	return _session->start(fresh, fromLauncher);
}

bool Game::parsePracticeBootParam(int bootParam, Location &location, uint &level) {
	if (bootParam <= 0)
		return false;
	int page = bootParam / kPracticeBootParamPageFactor;
	int difficulty = bootParam % kPracticeBootParamPageFactor;
	if (page < kCatapult01 || kBarn07 < page || difficulty < 1 || 3 < difficulty)
		return false;
	location = static_cast<Location>(page);
	level = difficulty;
	return true;
}

bool Game::prepareBoot(int bootParam) {
	Location location;
	uint level;
	if (!parsePracticeBootParam(bootParam, location, level))
		return false;
	if (location < kCatapult01 || kBarn07 < location) {
		warning("Zoombini3: boot target %d is not implemented yet", static_cast<int>(location));
		return false;
	}
	if (!prepareState(nullptr, "Boot_Practice"))
		return false;
	_world.setLevel(location - kCatapult01, level);
	_world.setLocation(location);
	debug(1, "Zoombini3: practice boot parameter=%d page=%d level=%u", bootParam, static_cast<int>(location), level);
	return true;
}

Shell::Game::SoundSettings Game::soundSettings() const {
	const int scale = Audio::Mixer::kMaxMixerVolume;
	SoundSettings settings = {CLIP(_world.musicVolume(), 0, 100) * scale / 100,
							  CLIP(_world.effectsVolume(), 0, 100) * scale / 100, CLIP(_world.dialogueVolume(), 0, 100) * scale / 100,
							  _world.isSoundMuted()};
	return settings;
}

void Game::setSoundSettings(const SoundSettings &settings) {
	const int scale = Audio::Mixer::kMaxMixerVolume;
	_world.setMusicVolume((CLIP(settings.music, 0, scale) * 100 + scale / 2) / scale);
	_world.setEffectsVolume((CLIP(settings.effects, 0, scale) * 100 + scale / 2) / scale);
	_world.setDialogueVolume((CLIP(settings.speech, 0, scale) * 100 + scale / 2) / scale);
	// Keep ScummVM's mute switch in the save without losing the three slider values.
	_world.setSoundMuted(settings.mute);
	if (_session)
		_session->syncSoundSettings();
}

void Game::applySoundSettings() {
	SoundSettings settings = soundSettings();
	Audio::Mixer &mixer = _vm.mixer();
	const Audio::Mixer::SoundType types[] = {
		Audio::Mixer::kPlainSoundType,
		Audio::Mixer::kMusicSoundType,
		Audio::Mixer::kSFXSoundType,
		Audio::Mixer::kSpeechSoundType,
	};
	for (uint i = 0; i < ARRAYSIZE(types); i++)
		mixer.muteSoundType(types[i], settings.mute);
	// Music retains Shell's own volume and enable state. Other channels use category volume.
	mixer.setVolumeForSoundType(Audio::Mixer::kPlainSoundType, settings.effects);
	mixer.setVolumeForSoundType(Audio::Mixer::kMusicSoundType, Audio::Mixer::kMaxMixerVolume);
	mixer.setVolumeForSoundType(Audio::Mixer::kSFXSoundType, settings.effects);
	mixer.setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, settings.speech);
	_vm.music().setEnabled(!_world.isBackgroundMusicOff());
	_vm.music().setVolume(CLIP(_world.musicVolume(), 0, 100));
}

bool Game::canReturnToLauncher() const {
	return !_session || !_session->hasModal();
}

bool Game::takeReturnToLauncherRequest() {
	bool requested = _quitRequested;
	_quitRequested = false;
	return requested;
}

void Game::event(const Common::Event &event) {
	if (_session)
		_session->event(event);
}

void Game::draw(OM::Canvas &canvas, uint32 time) {
	if (_session)
		_session->draw(canvas, time);
}

} // namespace Zoombini3
} // namespace Trinket
