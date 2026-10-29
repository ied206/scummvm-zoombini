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

/** Run RR1 startup movies, scene navigation and portable story checkpoints. */
class Game : public Shell::Game {
public:
	/** Connect Reader Rabbit story progress, page presentation and audio to the engine. */
	explicit Game(TrinketEngine &vm);
	/** Stop movies and release the active page and modal interfaces. */
	~Game() override;
	/** Construct the Reader Rabbit runtime through the shared subengine interface. */
	static Shell::Game *create(TrinketEngine &vm) { return new Game(vm); }
	/** Load player progress or prepare a fresh story before entering gameplay. */
	bool prepareState(Common::SeekableReadStream *save, const Common::String &playerName) override;
	/** Decode a portable checkpoint for game entry or offline import, without entering a scene. */
	static bool readSave(Common::SeekableReadStream &save, int &location, SoundSettings &sound, bool &arrivalPending, World &world);
	/** Prepare direct page startup from a supported boot parameter. */
	bool prepareBoot(int bootParam) override;
	/** Enter the prepared story, opening movie or sign-in path. */
	bool startPrepared(bool fresh, bool fromLauncher) override;
	/** Return the current game's stored audio preferences. */
	SoundSettings soundSettings() const override { return _sound; }
	/** Replace the game's stored audio preferences. */
	void setSoundSettings(const SoundSettings &settings) override { _sound = settings; }
	/** Apply the stored game audio levels and mute state. */
	void applySoundSettings() override;
	/** Return a pending sign-in request to switch players. */
	Common::String requestedPlayer() const override { return _requestedPlayer; }
	/** Clear the pending player-switch request. */
	void clearPlayerRequest() override { _requestedPlayer.clear(); }
	/** Serialize current story progress and audio preferences. */
	bool save(Common::WriteStream &stream) const override;
	/** Return the story location, preserving it while practice is active. */
	int currentLocation() const override { return _practiceStory ? _practiceStory->location : _location; }
	/** Report that page-specific handling controls Escape rather than the shared engine. */
	bool canQuitWithEscape() const override { return false; }
	/** Return and clear a pending request to leave gameplay. */
	bool takeQuitRequest() override;
	/** Dispatch input to movies, modals, sign-in or the active page. */
	void event(const Common::Event &event) override;
	/** Advance and draw the current game presentation at a millisecond timestamp. */
	void draw(OM::Canvas &canvas, uint32 time) override;
	/** Run a debugger command for the active Reader Rabbit page. */
	Common::String debugPageCommand(const Common::String &command) override;

private:
	/** Prefix of the debugger page-navigation command. */
	static constexpr const char *kGoCommandPrefix = "go ";
	/** Script identifier for navigation. */
	static constexpr const char *kNavigationName = "Navigation";
	/** World-state key for r1 Ending Pending. */
	static constexpr const char *kR1EndingPendingKey = "R1EndingPending";
	/** World-state key for is Background Music Off. */
	static constexpr const char *kIsBackgroundMusicOffKey = "isBackgroundMusicOff";
	/** Resource path for tlclogo.bik. */
	static constexpr const char *kTlclogoBikPath = "tlclogo.bik";
	/** Resource path for r1mvop1.bik. */
	static constexpr const char *kR1mvop1BikPath = "r1mvop1.bik";
	/** Resource path for rr1title.bik. */
	static constexpr const char *kRr1titleBikPath = "rr1title.bik";
	/** Resource path for r1mvop2.bik. */
	static constexpr const char *kR1mvop2BikPath = "r1mvop2.bik";
	/** Resource path for r1mvshipdown.bik. */
	static constexpr const char *kR1mvshipdownBikPath = "r1mvshipdown.bik";
	/** Resource path for rr1mvcq.bik. */
	static constexpr const char *kRr1mvcqBikPath = "rr1mvcq.bik";
	/** Resource path for r1mvshipup.bik. */
	static constexpr const char *kR1mvshipupBikPath = "r1mvshipup.bik";
	/** Resource path for r1mvcl1.bik. */
	static constexpr const char *kR1mvcl1BikPath = "r1mvcl1.bik";
	/** Resource path for r1mvdemo.bik. */
	static constexpr const char *kR1mvdemoBikPath = "r1mvdemo.bik";
	/** Resource path for r1mvclosing.bik. */
	static constexpr const char *kR1mvclosingBikPath = "r1mvclosing.bik";
	/** World-state key for movie:0. */
	static constexpr const char *kMovie0Key = "movie:0";
	/** World-state key for movie:1. */
	static constexpr const char *kMovie1Key = "movie:1";
	/** World-state key for movie:2. */
	static constexpr const char *kMovie2Key = "movie:2";
	/** World-state key for movie:3. */
	static constexpr const char *kMovie3Key = "movie:3";
	/** World-state key for movie:4. */
	static constexpr const char *kMovie4Key = "movie:4";
	/** World-state key for movie:5. */
	static constexpr const char *kMovie5Key = "movie:5";
	/** World-state key for r1 CN3 Movie Pending. */
	static constexpr const char *kR1CN3MoviePendingKey = "R1CN3MoviePending";
	/** World-state key for r1 LN2 Movie Pending. */
	static constexpr const char *kR1LN2MoviePendingKey = "R1LN2MoviePending";
	/** Authored data key for Background ID. */
	static constexpr const char *kBackgroundIDKey = "kBackgroundID";
	/** World-state key for previous Location. */
	static constexpr const char *kPreviousLocationKey = "previousLocation";
	/** World-state key for r1 CN3 From Street. */
	static constexpr const char *kR1CN3FromStreetKey = "R1CN3FromStreet";
	/** Authored data key for Background Music ID. */
	static constexpr const char *kBackgroundMusicIDKey = "kBackgroundMusicID";
	/** Authored data key for Background Music IDs.1. */
	static constexpr const char *kBackgroundMusicIDs1Key = "kBackgroundMusicIDs.1";
	/** World-state key for movie:closing. */
	static constexpr const char *kMovieClosingKey = "movie:closing";
	/** World-state key for r1 Ending Seen. */
	static constexpr const char *kR1EndingSeenKey = "R1EndingSeen";
	/** Format for script names or values: .%u. */
	static constexpr const char *kUnsignedIndexFormat = ".%u";
	/** Script identifier for map Dest Script. */
	static constexpr const char *kMapDestScriptName = "mapDestScript";
	/** World-state key for pm Icon Enabled. */
	static constexpr const char *kPmIconEnabledKey = "pmIconEnabled";
	/** Script identifier for map Icon. */
	static constexpr const char *kMapIconName = "mapIcon";
	/** World-state key for been To Map. */
	static constexpr const char *kBeenToMapKey = "beenToMap";
	/** World-state key for map PMSpeech. */
	static constexpr const char *kMapPMSpeechKey = "mapPMSpeech";
	/** World-state key for map PM1st Time Speech. */
	static constexpr const char *kMapPM1stTimeSpeechKey = "mapPM1stTimeSpeech";
	/** Script identifier for map. */
	static constexpr const char *kMapName = "map";
	/** Script identifier for r1end. */
	static constexpr const char *kR1endName = "r1end";
	/** World-state key for r1 CN4 Exit Pending. */
	static constexpr const char *kR1CN4ExitPendingKey = "R1CN4ExitPending";
	/** World-state key for r1 LG2 Exit Pending. */
	static constexpr const char *kR1LG2ExitPendingKey = "R1LG2ExitPending";
	/** World-state key for r1 CN2 Exit Pending. */
	static constexpr const char *kR1CN2ExitPendingKey = "R1CN2ExitPending";
	/** Script identifier for cache. */
	static constexpr const char *kCacheName = "cache";

	/** Background path indexed by page name and resource ID. */
	static constexpr const char *kPageBackgroundFormat = "%s/%d.bk16";
	/** Music path indexed by page name and resource ID. */
	static constexpr const char *kPageMusicFormat = "%stunz/%d.wav";
	/** Map icon path indexed by resource ID. */
	static constexpr const char *kMapAnimationFormat = "r1map/%d.ao";
	/** Map speech path indexed by resource ID. */
	static constexpr const char *kMapSpeechFormat = "r1map/%d.wav";
	/** Close button displayed over the map. */
	static constexpr const char *kMapCloseButton = "r1map/1853.ao";
	/** Previous-page button displayed over the map. */
	static constexpr const char *kMapPreviousButton = "r1map/4001.ao";
	/** Next-page button displayed over the map. */
	static constexpr const char *kMapNextButton = "r1map/4002.ao";
	/** Background animation for the navigation map. */
	static constexpr const char *kMapBackground = "r1map/4300.ao";

	/** Engine supplying shared graphics, audio, resources and player storage. */
	TrinketEngine &_vm;
	/** Scene displaying the active game's page background and controls. */
	Shell::Scene _scene;
	/** Sprites displayed by the story map overlay. */
	Shell::Scene _mapScene;
	/** Playback state for map speech. */
	OM::Sound _mapVoice;
	/** Authored constants and exits for the active page. */
	Page _page;
	/** Current story and activity progress with saved artwork. */
	World _world;
	/** Native activity and character presentation for the active page. */
	PageScene _presentation;
	/** Player name associated with the current story. */
	Common::String _name;
	/** Name awaiting a player switch after sign-in confirmation. */
	Common::String _requestedPlayer;
	/** Stored music, effects, speech and mute preferences. */
	SoundSettings _sound = {256, 256, 256, false};
	/** Current story location code. */
	int _location = 1;
	/** Whether the game-specific sign-in screen is active. */
	bool _signin = false;
	/** Whether the story map overlay is open. */
	bool _map = false;
	/** Whether the game is handling a return from a secondary screen. */
	bool _return = false;
	/** Whether saved story progress was loaded for the current player. */
	bool _storyLoaded = false;
	struct Story {
		/** Story progress preserved while a practice session is active. */
		World world;
		/** Story audio preferences preserved during practice. */
		SoundSettings sound;
		/** Story player name preserved during practice. */
		Common::String name;
		/** Story location restored after leaving practice. */
		int location;
		/** Whether an arrival presentation was pending before practice. */
		bool arrival;
		/** Whether story progress had been loaded before practice. */
		bool loaded;
	};
	/** Story snapshot restored after leaving temporary practice play. */
	Common::ScopedPtr<Story> _practiceStory;
	/** Current game-specific player-selection interface. */
	Common::ScopedPtr<Signin> _signinUI;
	/** Current credits or ending-credit presentation. */
	Common::ScopedPtr<Credits> _credits;
	/** Current game options interface. */
	Common::ScopedPtr<Options> _options;
	/** Background-music resource retained while options are open. */
	Common::String _optionsMusic;
	/** Music volume restored after closing options. */
	uint _optionsMusicVol = 100;
	/** Whether the current page must reload after an options change. */
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
	/** Location entered after the current movie finishes. */
	int _movieDest = 1;
	/** Most recent game timestamp in milliseconds. */
	uint32 _time = 0;
	/** Decoder for opening and story movies. */
	Video::BinkDecoder _movie;
	/** Most recent decoded movie frame used for drawing. */
	Graphics::Surface _movieFrame;
	/** Opening or story-movie stage currently being presented. */
	MovieStage _movieStage = kNoMovie;
	/** Whether the destination still needs its arrival presentation. */
	bool _arrivalPending = false;
	/** Whether the current activity is running as temporary practice. */
	bool _practice = false;
	/** Load and begin the selected opening or story-movie stage. */
	bool startMovie(MovieStage stage);
	/** Convert the saved native music level to a playback percentage. */
	uint musicVolume() const;
	/** Close the current movie and continue to sign-in or its story destination. */
	void finishMovie(bool skipToSignin = false);
	/** Load authored page data and initialize its native presentation. */
	bool enterPage(int location);
	/** Load and open the game-specific player-selection screen. */
	bool showSignin();
	/** Begin the story-completion movie sequence. */
	bool startEnding();
	/** Load ordinary or ending credits. */
	bool showCredits(bool ending);
	/** Open the story map and its audio presentation. */
	void showMap();
	/** Recolor only marble badges and music-grid rows within an authored map icon. */
	static void setMapIconColors(Shell::Sprite &icon, int resource);
	/** Close the map and restore the current page presentation. */
	void closeMap();
	/** Load the game-specific options interface. */
	bool showOptions();
	/** Handle options requests for level, movie, page or story changes. */
	void optionsEvent(const Common::Event &event);
	/** Restore the music resource and volume retained before options opened. */
	void restoreOptionsMusic();
	/** Dispatch sign-in input and handle its resulting request. */
	void signinEvent(const Common::Event &event);
	/** Apply a sign-in request to player selection or story startup. */
	void signinRequest(int request);
	/** Preserve story state and enter temporary practice play. */
	void startPractice();
	/** Map an authored page name to its game location index. */
	static int pageIndex(const Common::String &name);
	/** Authored page names indexed by game location. */
	static const char *const kPages[];
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
