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

#ifndef TRINKET_LAUNCHER_APPLICATION_H
#define TRINKET_LAUNCHER_APPLICATION_H

#include "common/formats/winexe_ne.h"
#include "common/savefile.h"
#include "graphics/wincursor.h"
#include "trinket/launcher/registry.h"
#include "trinket/ompp/font.h"
#include "trinket/ompp/sound.h"
#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "video/bink_decoder.h"

namespace Trinket {
namespace Launcher {

/** Shared TLC sign-in and in-process game handoff. Game saves remain subengine-owned. */
class Application : public OM::ResourceProvider {
public:
	enum Request {
		kNone00 = 0,
		kPlay01 = 1,
		kDelete02 = 2,
		kNewGame03 = 3,
		kSlot04 = 4,
		kQuit05 = 5,
		kStartGame06 = 6,
	};
	Application(Common::SaveFileManager &manager, const Common::String &target, Audio::Mixer &mixer);
	~Application() {
		_movieFrame.free();
		delete _hoverCursor;
		delete _arrowCursor;
	}
	bool load(const Common::Path &gamePath, const char *product);
	Common::SeekableReadStream *open(const Common::String &name) const override;
	Request event(const Common::Event &event);
	void draw(OM::Canvas &canvas);
	Common::String selectedName() const;
	bool commitName();
	bool removeName();
	void resume();
	void suspend();
	uint requestedSlot() const { return _slot; }

private:
	enum ContentAction {
		kGridPrevious1900 = 1900,
		kGridNext1901 = 1901,
		kWorldMovie1902 = 1902,
		kContentLink2000 = 2000,
	};
	struct TextBlock {
		Common::SharedPtr<OM::NFNTFont> font;
		Common::Array<Common::String> lines;
		Common::Rect rect;
		byte red, green, blue;
		bool centered;
	};
	typedef Registry::User User;
	Common::SaveFileManager &_manager;
	Common::String _saveName;
	Common::Array<User> _users;
	uint _nextID = 1, _slot = 1;
	int _selected = -1, _first = 0;
	bool _startup = true, _newEntry = false, _confirmDelete = false;
	bool _content = false, _musicEnabled = true, _soundEnabled = true;
	bool _updatesAutomatic = true;
	bool _gameSignin = false;
	bool _productPlay = false;
	Graphics::WinCursorGroup *_hoverCursor = nullptr;
	Graphics::Cursor *_arrowCursor = nullptr;
	bool _pointerOverButton = false;
	uint _defaultMusic = 0, _musicVolume = 100;
	Common::String _musicPath;
	Common::String _typed;
	Common::Path _root;
	mutable Common::NEResources _common, _signin, _product;
	mutable Common::NEResources _brands, _parents, _audio, _news;
	mutable Common::NEResources _worldLibraries[6];
	OM::XMLParser _layout, _startupLayout;
	OM::XMLParser _homeLayout, _parentsLayout;
	OM::XMLParser _demoLayouts[6], _newsLayout;
	const OM::XMLParser *_contentLayout = nullptr, *_previousLayout = nullptr;
	Common::String _contentBranch, _previousBranch;
	uint _gridPage = 0;
	Video::BinkDecoder _contentMovie;
	Graphics::Surface _movieFrame;
	Common::Point _moviePosition;
	int _world = -1;
	Common::Array<TextBlock> _textBlocks;
	Common::Array<Common::String> _links;
	Common::Array<Common::String> _linkParameters;
	Shell::Scene _frameScene;
	Shell::Sprite *_verticalTab = nullptr, *_horizontalTab = nullptr, *_playCD = nullptr;
	Shell::Scene _scene;
	Shell::Scene _dialogScene;
	const Graphics::Font *_listFont = nullptr;
	Common::SharedPtr<OM::NFNTFont> _dialogFont;
	OM::Sound _voice;
	OM::Sound _music, _click;
	Shell::Sprite *_ok = nullptr, *_newButton = nullptr;
	Common::Rect _listRect;
	static constexpr const char *kRoot = "LAUNCHER";
	static constexpr const char *kCursorExecutable = "win/TLCLauncher.exe";
	static constexpr const char *kCommon = "rsc/tlccommon.rsc";
	static constexpr const char *kSignin = "rsc/tlcsignin.rsc";
	static constexpr const char *kSigninXML = "rsc/TLCSignin.xml";
	static constexpr const char *kHomeXML = "rsc/TLCHome.xml";
	static constexpr const char *kParentsXML = "rsc/TLCParents.xml";
	static constexpr const char *kBrands = "rsc/tlcbrands.rsc";
	static constexpr const char *kParents = "rsc/tlcparents.rsc";
	static constexpr const char *kAudio = "rsc/tlcaudio.rsc";
	static constexpr const char *kNews = "USERS/rsc/tlcwhatsnew.rsc";
	static constexpr const char *kMovieDirectory = "TLCLauncher";
	static constexpr const char *kWorldMovies[6] = {
		"RRandI.bik",
		"mvjopening.bik",
		"Clue640.bik",
		"zoom_movie1.bik",
		"OT32211.bik",
		"CSD_final.bik",
	};
	static constexpr const char *kWorldLibraries[6] = {
		"rsc/tlcrrworld.rsc",
		"rsc/tlcsfworld.rsc",
		"rsc/tlccfworld.rsc",
		"rsc/tlczoworld.rsc",
		"rsc/tlcotworld.rsc",
		"rsc/tlccsworld.rsc",
	};
	static constexpr const char *kDemoXML[6] = {
		"TLCRRDemos.xml",
		"TLCSFDemos.xml",
		"TLCCFDemos.xml",
		"TLCZODemos.xml",
		"TLCOTDemos.xml",
		"TLCCSDemos.xml",
	};
	static constexpr const char *kStartupXML = "USERS/installed/TLCLstartup.xml";
	static constexpr const char *kMusicResource = "%s/%u.wav";
	static constexpr const char *kClickSound = "common/1001.wav";
	static constexpr const char *kProductRSC = "PRODUCT/rsc/%s.rsc";
	static constexpr const char *kProductXML = "PRODUCT/installed/%s.xml";
	static constexpr const char *kSigninBranch = "launcher/signin/keyboard/";
	static constexpr const char *kBackground = "common/3000.jpg";
	static constexpr const char *kAnimation = "%s/%d.ao";
	static constexpr const char *kEnterSpeech = "signin/4081.wav";
	static constexpr const char *kRemoveSpeech = "signin/4071.wav";
	static constexpr const char *kDialogFont = "common/15018.nfnt";
	bool loadUsers();
	bool storeUsers() const;
	bool showStartup();
	bool showProductScreen(const Common::String &branch);
	bool showSignin();
	bool buildFrame();
	bool showContent(const OM::XMLParser &xml, const Common::String &branch);
	void followLink(const Common::String &link, const Common::String &parameters = Common::String());
	bool addContentScript(const OM::XMLParser &xml, const Common::String &script);
	bool addContentGrid(const OM::XMLParser &xml);
	bool showWorld(uint world);
	void playContentMovie(const Common::String &name, int x, int y);
	void stopContentMovie();
	static void notifyNotImplemented();
	void drawContentText(OM::Canvas &canvas);
	bool addContentText(const OM::XMLParser &xml, uint node);
	void startMusic();
	void selectMusic(uint id, const char *library);
	static Shell::Sprite *addFrameObject(Shell::Scene &scene, int id, int x, int y, int action = 0);
	static void configureButton(Shell::Sprite &sprite);
	void updateCursor(bool overButton);
	void showDeleteDialog();
	uint32 listColor(const OM::Canvas &canvas, const char *name) const;
	Shell::Sprite *add(const OM::XMLParser &xml, const Common::String &branch, const char *library, int action);
	void addCharacter(char character);
	void matchName();
	void select(int index);
	bool valid() const;
	static bool userLess(const User &left, const User &right) { return left.name < right.name; }
};

} // End of namespace Launcher
} // End of namespace Trinket

#endif
