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

#ifndef TRINKET_RR1CLOUD9_GAME_H
#define TRINKET_RR1CLOUD9_GAME_H

#include "trinket/ompp/font.h"
#include "trinket/ompp/sound.h"
#include "trinket/rr1cloud9/page.h"
#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/shell/game.h"
#include "trinket/shell/scene.h"
#include "video/bink_decoder.h"

namespace Trinket {

class TrinketEngine;
namespace RR1Cloud9 {
class Credits;
class Options;
class Signin;

/** Owns RR1 startup movies, scene navigation and portable story checkpoints. */
class Game : public Shell::Game {
public:
	explicit Game(TrinketEngine &vm);
	~Game() override;
	static Shell::Game *create(TrinketEngine &vm) { return new Game(vm); }
	bool prepareState(Common::SeekableReadStream *save, const Common::String &playerName) override;
	bool prepareBoot(int bootParam) override;
	bool startPrepared(bool fresh, bool fromLauncher) override;
	SoundSettings soundSettings() const override { return _sound; }
	void setSoundSettings(const SoundSettings &settings) override { _sound = settings; }
	void applySoundSettings() override;
	Common::String requestedPlayer() const override { return _requestedPlayer; }
	void clearPlayerRequest() override { _requestedPlayer.clear(); }
	bool save(Common::WriteStream &stream) const override;
	int currentLocation() const override { return _practiceStory ? _practiceStory->location : _location; }
	bool canReturnToLauncher() const override { return false; }
	bool takeReturnToLauncherRequest() override;
	void event(const Common::Event &event) override;
	void draw(OM::Canvas &canvas, uint32 time) override;
	Common::String debugPageCommand(const Common::String &command) override;

private:
	TrinketEngine &_vm;
	Shell::Scene _scene;
	Shell::Scene _mapScene;
	OM::Sound _mapVoice;
	Page _page;
	World _world;
	PageScene _presentation;
	Common::String _name, _requestedPlayer;
	SoundSettings _sound = {256, 256, 256, false};
	int _location = 1;
	bool _signin = false, _map = false, _return = false, _storyLoaded = false;
	struct Story {
		World world;
		SoundSettings sound;
		Common::String name;
		int location;
		bool arrival, loaded;
	};
	Common::ScopedPtr<Story> _practiceStory;
	Common::ScopedPtr<Signin> _signinUI;
	Common::ScopedPtr<Credits> _credits;
	Common::ScopedPtr<Options> _options;
	Common::String _optionsMusic;
	uint _optionsMusicVolume = 100;
	bool _reloadAfterOptions = false;
	enum MovieStage {
		kNoMovie,
		kLogo,
		kOpening,
		kTitle,
		kArrival,
		kDescent,
		kQueen,
		kAscent,
		kInventor,
		kGalleryDemo,
		kClosing,
		kReplay,
	};
	int _movieDestination = 1;
	uint32 _time = 0;
	Video::BinkDecoder _movie;
	Graphics::Surface _movieFrame;
	MovieStage _movieStage = kNoMovie;
	bool _arrivalPending = false, _practice = false;
	bool startMovie(MovieStage stage);
	uint musicVolume() const;
	void finishMovie(bool skipToSignin = false);
	bool enterPage(int location);
	bool showSignin();
	bool startEnding();
	bool showCredits(bool ending);
	void showMap();
	void closeMap();
	bool showOptions();
	void optionsEvent(const Common::Event &event);
	void restoreOptionsMusic();
	void signinEvent(const Common::Event &event);
	void signinRequest(int request);
	void startPractice();
	static int pageIndex(const Common::String &name);
	static const char *const kPages[];
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
