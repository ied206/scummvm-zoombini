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

#include "trinket/rr1cloud9/game.h"

#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/translation.h"
#include "trinket/rr1cloud9/credits.h"
#include "trinket/rr1cloud9/options.h"
#include "trinket/rr1cloud9/print.h"
#include "trinket/rr1cloud9/signin.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

const char *const Game::kPages[] = {
	"r1cn1",
	"r1cn2",
	"r1cn3",
	"r1cn4",
	"r1ca1",
	"r1ca2",
	"r1cg1",
	"r1cg2",
	"r1ln2",
	"r1lg2",
	"r1la1",
	"r1la2",
	"r1la3",
	"r1in1",
	"r1in4",
	"r1ig1",
	"r1ia1",
	"r1ia2",
	"r1ia3",
	"r1rss",
};

Game::Game(TrinketEngine &vm) : _vm(vm), _scene(vm.resources()), _mapScene(vm.resources()), _mapVoice(vm.mixer()), _presentation(vm, _scene, _world) {}

Game::~Game() {
	_movie.close();
	_movieFrame.free();
}

int Game::pageIndex(const Common::String &name) {
	for (uint i = 0; i < ARRAYSIZE(kPages); i++) {
		if (name == kPages[i])
			return i + 1;
	}
	return 0;
}

bool Game::prepareState(Common::SeekableReadStream *save, const Common::String &playerName) {
	closeMap();
	_signinUI.reset();
	_practiceStory.reset();
	_presentation.clear();
	_credits.reset();
	_options.reset();
	_reloadAfterOptions = false;
	_name = playerName;
	_signin = playerName.empty();
	_storyLoaded = !playerName.empty();
	_location = 1;
	_arrivalPending = !playerName.empty() && !save;
	_practice = false;
	_world.reset();
	_sound = {ConfMan.getInt("music_volume"), ConfMan.getInt("sfx_volume"), ConfMan.getInt("speech_volume"), ConfMan.getBool("mute")};
	if (!save)
		return true;
	// This portable story checkpoint is independent of native World serialization.
	if (save->readUint32BE() != MKTAG('R', '1', 'C', '9'))
		return false;
	uint version = save->readUint16LE();
	if (version < 1 || 4 < version)
		return false;
	_location = save->readUint16LE();
	_sound.music = save->readUint16LE();
	_sound.effects = save->readUint16LE();
	_sound.speech = save->readUint16LE();
	_sound.mute = save->readByte() != 0;
	if (2 <= version) {
		uint pending = save->readByte();
		if (1 < pending)
			return false;
		_arrivalPending = pending != 0;
	}
	if (3 <= version && !_world.load(*save))
		return false;
	if (4 <= version && !_world.artwork.load(*save))
		return false;
	return !save->err() && !save->eos() && 1 <= _location && _location <= static_cast<int>(ARRAYSIZE(kPages)) &&
		   _sound.music <= Audio::Mixer::kMaxMixerVolume && _sound.effects <= Audio::Mixer::kMaxMixerVolume &&
		   _sound.speech <= Audio::Mixer::kMaxMixerVolume;
}

bool Game::prepareBoot(int bootParam) {
	if (bootParam < 1 || static_cast<int>(ARRAYSIZE(kPages)) < bootParam)
		return false;
	prepareState(nullptr, "Navigation");
	_location = bootParam;
	_arrivalPending = false;
	return true;
}

bool Game::startPrepared(bool fresh, bool fromLauncher) {
	applySoundSettings();
	if (_signin)
		return startMovie(kLogo);
	if (_arrivalPending)
		return startMovie(kArrival);
	if (_world.get("R1EndingPending"))
		return startEnding();
	return enterPage(_location);
}

uint Game::musicVolume() const {
	// The authored music gain and Sound API use percent; save settings use mixer units.
	return _sound.music * (_location == 15 ? 75 : 90) / Audio::Mixer::kMaxMixerVolume;
}

void Game::applySoundSettings() {
	_vm.music().setEnabled(!_sound.mute && !_world.get("isBackgroundMusicOff"));
	_vm.music().setVolume(musicVolume());
	_movie.setVolume(_sound.mute ? 0 : MIN(_sound.speech, 255));
	_vm.mixer().setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, _sound.mute ? 0 : _sound.speech);
	_vm.mixer().setVolumeForSoundType(Audio::Mixer::kSFXSoundType, _sound.mute ? 0 : _sound.effects);
}

bool Game::save(Common::WriteStream &stream) const {
	if (_practiceStory ? !_practiceStory->loaded : (!_storyLoaded || _practice))
		return false;
	const World &world = _practiceStory ? _practiceStory->world : _world;
	const SoundSettings &sound = _practiceStory ? _practiceStory->sound : _sound;
	stream.writeUint32BE(MKTAG('R', '1', 'C', '9'));
	if (!_practiceStory)
		_presentation.checkpoint();
	stream.writeUint16LE(4);
	stream.writeUint16LE(currentLocation());
	stream.writeUint16LE(sound.music);
	stream.writeUint16LE(sound.effects);
	stream.writeUint16LE(sound.speech);
	stream.writeByte(sound.mute);
	stream.writeByte(_practiceStory ? _practiceStory->arrival : _arrivalPending);
	return world.save(stream) && world.artwork.save(stream);
}

bool Game::startMovie(MovieStage stage) {
	if (stage == kReplay && !_options)
		return false;
	static const char *const filenames[] = {
		"",
		"tlclogo.bik",
		"r1mvop1.bik",
		"rr1title.bik",
		"r1mvop2.bik",
		"r1mvshipdown.bik",
		"rr1mvcq.bik",
		"r1mvshipup.bik",
		"r1mvcl1.bik",
		"r1mvdemo.bik",
		"r1mvclosing.bik",
	};
	_movie.close();
	_movieFrame.free();
	_movieStage = kNoMovie;
	const char *filename = stage == kReplay && _options ? Options::movieFile(_options->movie()) : filenames[stage];
	Common::SeekableReadStream *stream = _vm.resources().open(filename);
	if (!stream || !_movie.loadStream(stream)) {
		warning("RR1Cloud9: could not load movie %s", filename);
		return false;
	}
	_vm.music().stop();
	_movie.setVolume(_sound.mute ? 0 : MIN(_sound.speech, 255));
	_movieStage = stage;
	if (stage == kArrival) {
		_world.set("movie:0", 1);
		_world.set("movie:1", 1);
	} else if (stage == kQueen)
		_world.set("movie:2", 1);
	else if (stage == kGalleryDemo)
		_world.set("movie:3", 1);
	else if (stage == kInventor)
		_world.set("movie:4", 1);
	else if (stage == kClosing)
		_world.set("movie:5", 1);
	_movie.start();
	debug(1, "RR1Cloud9: movie %s started", filename);
	return true;
}

void Game::finishMovie(bool skipToSignin) {
	MovieStage previous = _movieStage;
	_movie.close();
	_movieFrame.free();
	_movieStage = kNoMovie;
	bool ready;
	if (previous == kReplay) {
		restoreOptionsMusic();
		ready = true;
	} else if (previous == kClosing) {
		ready = showCredits(true);
	} else if (previous == kQueen) {
		_presentation.pause(false, _time);
		_presentation.queenMovieFinished();
		ready = true;
	} else if (previous == kGalleryDemo) {
		ready = enterPage(6);
		if (ready)
			_world.set("R1CN3MoviePending", 0);
	} else if (previous == kInventor) {
		ready = enterPage(13);
		if (ready)
			_world.set("R1LN2MoviePending", 0);
	} else if (previous == kDescent || previous == kAscent) {
		ready = enterPage(_movieDestination);
	} else if (previous == kArrival) {
		ready = enterPage(1);
		if (ready)
			_arrivalPending = false;
	} else if (skipToSignin || previous == kTitle) {
		ready = showSignin();
	} else {
		ready = startMovie(previous == kLogo ? kOpening : kTitle);
	}
	if (!ready)
		_return = true;
}

bool Game::enterPage(int location) {
	if (location < 1 || static_cast<int>(ARRAYSIZE(kPages)) < location)
		return false;
	Common::String name = kPages[location - 1];
	_credits.reset();
	Page candidate;
	if (!candidate.load(_vm.resources(), name))
		return false;
	// Load the new background before releasing the current scene.
	Common::String background = Common::String::format("%s/%d.bk16", name.c_str(), candidate.number("kBackgroundID"));
	Shell::Scene probe(_vm.resources());
	if (!probe.setBackground(background))
		return false;
	_movie.close();
	_movieFrame.free();
	_movieStage = kNoMovie;
	if (_presentation.active())
		_world.set("previousLocation", _location);
	_presentation.clear();
	_scene.clear();
	if (!_scene.setBackground(background))
		return false;
	if (location == 3 && _location != 3)
		_world.set("R1CN3FromStreet", _location != 6);
	_page = candidate;
	_signinUI.reset();
	if (!_presentation.enter(_page, name, _practice))
		return false;
	_world.enter(name);
	_location = location;
	closeMap();
	_signin = false;
	int music = _page.number("kBackgroundMusicID");
	if (name == "r1in4")
		music = _page.number("kBackgroundID");
	if (!music)
		music = _page.number("kBackgroundMusicIDs.1");
	_vm.music().reset();
	_vm.music().setEnabled(!_sound.mute && !_world.get("isBackgroundMusicOff"));
	if (music)
		_vm.music().start(Common::String::format("%stunz/%d.wav", name.c_str(), music), musicVolume());
	debug(1, "RR1Cloud9: page %s ready, %u exits", name.c_str(), _page.exits().size());
	return true;
}

bool Game::showSignin() {
	closeMap();
	_presentation.checkpoint();
	_presentation.clear();
	_credits.reset();
	_options.reset();
	if (_practiceStory) {
		_world = _practiceStory->world;
		_sound = _practiceStory->sound;
		_name = _practiceStory->name;
		_location = _practiceStory->location;
		_arrivalPending = _practiceStory->arrival;
		_storyLoaded = _practiceStory->loaded;
		_practiceStory.reset();
	}
	_practice = false;
	_signin = true;
	applySoundSettings();
	_vm.music().stop();
	_signinUI.reset(new Signin(_vm, _scene));
	bool ready = _signinUI->enter();
	if (ready)
		debug(1, "RR1Cloud9: sign-in ready");
	return ready;
}

bool Game::startEnding() {
	_world.set("R1EndingPending", 1);
	_world.set("movie:closing", 1);
	_location = 15;
	_presentation.clear();
	_scene.clear();
	closeMap();
	_signin = false;
	return startMovie(kClosing);
}

bool Game::showCredits(bool ending) {
	_credits.reset(new Credits(_vm));
	if (!_credits->enter(ending)) {
		_credits.reset();
		return false;
	}
	_vm.music().setVolume(_sound.music * 100 / Audio::Mixer::kMaxMixerVolume);
	if (ending) {
		_world.set("R1EndingPending", 0);
		_world.set("R1EndingSeen", 1);
	}
	return true;
}

void Game::startPractice() {
	_presentation.checkpoint();
	_practiceStory.reset(new Story{_world, _sound, _name, _location, _arrivalPending, _storyLoaded});
	_world.reset();
	_name = "Practice Mode";
	_practice = true;
	_arrivalPending = false;
	showMap();
}

void Game::signinRequest(int request) {
	if (request == Signin::kStart) {
		_requestedPlayer = _signinUI->selectedName();
	} else if (request == Signin::kPractice) {
		_signinUI->stopSound();
		startPractice();
	} else if (request == Signin::kQuit) {
		_return = true;
	} else if (request == Signin::kDelete) {
		Common::String name = _signinUI->deletedName();
		if (!_vm.removeSavedPlayer(name))
			g_system->displayMessageOnOSD(_("Could not remove all saved game files."));
		if (_name == name && !_vm.hasGamePlayer(name)) {
			_storyLoaded = false;
			_world.reset();
			_name.clear();
		}
		_signinUI->refresh();
	}
}

void Game::signinEvent(const Common::Event &event) {
	signinRequest(_signinUI->event(event, _time));
}

void Game::showMap() {
	if (_map || _presentation.blocksMap())
		return;
	Page mapData;
	if (!mapData.load(_vm.resources(), "r1main"))
		return;
	_mapScene.clear();
	if (!_mapScene.add("r1map/4300.ao", 0)) {
		return;
	}
	for (uint i = 1; i <= 19; i++) {
		Common::String suffix = Common::String::format(".%u", i);
		Common::String destination = mapData.text("mapDestScript" + suffix);
		bool enabled = _practice ? mapData.number("pmIconEnabled" + suffix) == 1 : _world.visits(destination) > 0;
		if (!_practice && (_location == 13 || i == 13))
			enabled = _location == 13 && i == 13;
		if (enabled)
			_mapScene.add(Common::String::format("r1map/%d.ao", mapData.number("mapIcon" + suffix)), 1, pageIndex(destination), true);
	}
	_mapScene.add("r1map/1853.ao", 2, 21, true);
	_mapScene.add("r1map/4001.ao", 2, 22, true);
	_mapScene.add("r1map/4002.ao", 2, 23, true);
	_map = true;
	_presentation.pause(true, _time);
	if (_practice) {
		int speech = mapData.number(_world.get("beenToMap") ? "mapPMSpeech" : "mapPM1stTimeSpeech");
		_world.set("beenToMap", 1);
		_mapVoice.play(_vm.resources(), Common::String::format("r1map/%d.wav", speech), Audio::Mixer::kSpeechSoundType);
	}
	debug(1, "RR1Cloud9: map ready");
}

void Game::closeMap() {
	_mapVoice.stop();
	_map = false;
}

bool Game::showOptions() {
	_mapVoice.stop();
	_options.reset(new Options(_vm, _world, _name, _signin ? 0 : _location, _practice || _signin));
	if (!_options->enter()) {
		_options.reset();
		return false;
	}
	_optionsMusic = _vm.music().resource();
	_optionsMusicVolume = _vm.music().volume();
	return true;
}

void Game::restoreOptionsMusic() {
	_vm.music().start(_optionsMusic, _optionsMusicVolume);
	_vm.music().setEnabled(!_sound.mute && !_world.get("isBackgroundMusicOff"));
}

void Game::optionsEvent(const Common::Event &event) {
	Options::Request request = _options->event(event);
	_vm.music().setEnabled(!_sound.mute && !_world.get("isBackgroundMusicOff"));
	if (request == Options::kNone)
		return;
	_options->stopSound();
	if (request == Options::kCredits) {
		if (!showCredits(false))
			_return = true;
		return;
	}
	if (request == Options::kMovie) {
		if (!startMovie(kReplay))
			_return = true;
		return;
	}
	if (request == Options::kPrint) {
		OM::Canvas report(640, 480);
		_options->draw(report, _time);
		Print::save(report.surface());
		return;
	}
	int destination = _options->destination();
	_reloadAfterOptions |= _options->changedCurrentLevel();
	_options.reset();
	if (request == Options::kQuit)
		_return = true;
	else if (request == Options::kSignin) {
		// The host saves the story before switching to another player.
		if (!showSignin())
			_return = true;
	} else if (request == Options::kLocation) {
		if (destination == _location && !_signin && !_reloadAfterOptions) {
			closeMap();
			_presentation.pause(false, _time);
		} else if (!enterPage(destination))
			_return = true;
		_reloadAfterOptions = false;
	}
}

bool Game::takeReturnToLauncherRequest() {
	bool requested = _return;
	_return = false;
	return requested;
}

void Game::event(const Common::Event &event) {
	if (event.type == Common::EVENT_MOUSEMOVE || event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP)
		debug(5, "RR1Cloud9: pointer %d at %d,%d", event.type, event.mouse.x, event.mouse.y);
	if (_movieStage != kNoMovie) {
		if (_movieStage == kClosing || _movieStage == kReplay) {
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_KEYDOWN)
				finishMovie();
			return;
		}
		if (event.type == Common::EVENT_LBUTTONDOWN ||
			(event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_RETURN))
			finishMovie();
		else if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_SPACE)
			finishMovie(true);
		else if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)
			_return = true;
		return;
	}
	if (_credits) {
		int action = _credits->event(event);
		if (action == 1) {
			if (!showSignin())
				_return = true;
		} else if (action == 2) {
			_return = true;
		} else if (action == 3 && _options) {
			_credits.reset();
			restoreOptionsMusic();
		}
		return;
	}
	if (_options) {
		optionsEvent(event);
		return;
	}
	if (_signin && !_map) {
		signinEvent(event);
		return;
	}
	if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE) {
		if (_map && _signin) {
			closeMap();
			_practice = false;
			showSignin();
		} else if (_map) {
			closeMap();
			if (_reloadAfterOptions) {
				_reloadAfterOptions = false;
				if (!enterPage(_location))
					_return = true;
			} else
				_presentation.pause(false, _time);
		} else
			_return = true;
		return;
	}
	if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_m) {
		if (_map) {
			closeMap();
			if (_signin) {
				if (!showSignin())
					_return = true;
			} else if (_reloadAfterOptions) {
				_reloadAfterOptions = false;
				if (!enterPage(_location))
					_return = true;
			} else
				_presentation.pause(false, _time);
		} else
			showMap();
		return;
	}
	if (_map) {
		int action = _mapScene.event(event);
		if (action == 22) {
			showOptions();
			return;
		}
		if (action == 23) {
			_mapVoice.stop();
			_return = true;
			return;
		}
		if (action == 21) {
			closeMap();
			if (_signin) {
				_practice = false;
				showSignin();
			} else {
				if (_reloadAfterOptions)
					enterPage(_location);
				else
					_presentation.pause(false, _time);
				_reloadAfterOptions = false;
			}
		} else if (action == _location && !_signin) {
			closeMap();
			if (_reloadAfterOptions)
				enterPage(_location);
			else
				_presentation.pause(false, _time);
			_reloadAfterOptions = false;
		} else if (action) {
			enterPage(action);
			_reloadAfterOptions = false;
		}
		return;
	}
	if (_presentation.event(event) || event.type != Common::EVENT_LBUTTONDOWN)
		return;
	Common::String action = _presentation.click(event.mouse);
	if (!action.empty()) {
		if (action == "map")
			showMap();
		return;
	}
	if (_practice)
		return;
	for (const auto &exit : _page.exits()) {
		if (!exit.rect.contains(event.mouse))
			continue;
		int destination = pageIndex(exit.destination);
		if (destination) {
			if ((_location == 1 && exit.destination == "r1in4") || (_location == 15 && exit.destination == "r1cn1")) {
				_movieDestination = destination;
				_presentation.pause(true, _time);
				if (!startMovie(_location == 1 ? kDescent : kAscent))
					_return = true;
			} else {
				enterPage(destination);
			}
		} else
			g_system->displayMessageOnOSD(_("This story transition is not implemented yet."));
		return;
	}
}

void Game::draw(OM::Canvas &canvas, uint32 time) {
	_time = time;
	if (_movieStage != kNoMovie) {
		if (_movie.endOfVideo()) {
			finishMovie();
		} else if (_movie.needsUpdate()) {
			const Graphics::Surface *frame = _movie.decodeNextFrame();
			if (frame) {
				Common::ScopedPtr<Graphics::Surface> converted(frame->convertTo(canvas.surface().format));
				_movieFrame.free();
				_movieFrame.copyFrom(*converted);
				converted->free();
			}
		}
		if (_movieStage != kNoMovie) {
			canvas.surface().fillRect(Common::Rect(640, 480), 0);
			if (_movieFrame.getPixels())
				canvas.blit(_movieFrame, 0, 0);
			return;
		}
	}
	Common::Array<uint16> sounds;
	if (_credits) {
		_credits->draw(canvas, time);
		return;
	}
	if (_options) {
		_options->draw(canvas, time);
		return;
	}
	if (_map) {
		_mapScene.update(time, sounds);
		_mapScene.draw(canvas);
		return;
	}
	if (_signin && _signinUI) {
		signinRequest(_signinUI->update());
		_signinUI->draw(canvas, time);
		return;
	}
	_presentation.update(time);
	if (!_presentation.destination().empty()) {
		if (_presentation.destination() == "r1end") {
			if (!startEnding())
				_return = true;
			return;
		}
		if (!enterPage(pageIndex(_presentation.destination())))
			_return = true;
		else {
			_world.set("R1CN4ExitPending", 0);
			_world.set("R1LG2ExitPending", 0);
		}
		return;
	}
	if (_presentation.wantsQueenExit()) {
		if (!enterPage(pageIndex("r1cn3")))
			_return = true;
		else
			_world.set("R1CN2ExitPending", 0);
		return;
	}
	if (_presentation.wantsGalleryMovie()) {
		_presentation.pause(true, time);
		if (!startMovie(kGalleryDemo))
			_return = true;
		return;
	}
	if (int destination = _presentation.galleryDestination()) {
		if (!enterPage(destination))
			_return = true;
		return;
	}
	if (_presentation.wantsInventorMovie()) {
		_presentation.pause(true, time);
		if (!startMovie(kInventor))
			_return = true;
		return;
	}
	if (_presentation.wantsQueenMovie()) {
		_presentation.pause(true, time);
		if (!startMovie(kQueen))
			_return = true;
		return;
	}
	_scene.update(time, sounds);
	_presentation.playSounds(sounds);
	_presentation.draw(canvas);
}

Common::String Game::debugPageCommand(const Common::String &command) {
	if (command.hasPrefix("go "))
		return enterPage(pageIndex(command.substr(3))) ? "Page entered.\n" : "Unknown or unavailable page.\n";
	if (command == "map") {
		if (!_signin)
			showMap();
		return "Map requested.\n";
	}
	Common::String result = "RR1 Cloud Nine scene debugger. Use: page go <name>, page map\n";
	for (const char *name : kPages)
		result += Common::String(name) + " ";
	return result + "\n";
}

} // namespace RR1Cloud9
} // namespace Trinket
