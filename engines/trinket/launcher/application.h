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

class TrinketEngine;

namespace Launcher {

/** Display the shared TLC sign-in screen and hand off to a game's startup and save handling. */
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
	/** Initialize launcher services using the engine's platform-specific default cursor. */
	Application(TrinketEngine &vm, Common::SaveFileManager &manager, const Common::String &target, Audio::Mixer &mixer);
	/** Free decoded launcher movie pixels and remove the custom hover cursor. */
	~Application() {
		_movieFrame.free();
		delete _hoverCursor;
	}
	/** Load the selected product's launcher resources and reset its platform-specific default arrow. */
	bool load(const Common::Path &gamePath, const char *product);
	/** Open a normalized launcher resource path, returning null if it is unavailable. */
	Common::SeekableReadStream *open(const Common::String &name) const override;
	/** Dispatch launcher input and return any requested application transition. */
	Request event(const Common::Event &event);
	/** Draw the current launcher screen, content and modal dialog. */
	void draw(OM::Canvas &canvas);
	/** Return the selected or newly typed player name. */
	Common::String selectedName() const;
	/** Validate and save the selected player name. */
	bool commitName();
	/** Remove the selected launcher user after confirmation. */
	bool removeName();
	/** Resume launcher playback after returning from gameplay. */
	void resume();
	/** Suspend launcher activity while gameplay is active. */
	void suspend();
	/** Return the one-based game slot selected for the launch request. */
	uint requestedSlot() const { return _slot; }

private:
	/** Script property name for startup/music. */
	static constexpr const char *kPropertyStartupMusic = "startup/music";
	/** Script property name for startup/signin/execute. */
	static constexpr const char *kPropertyStartupSigninExecute = "startup/signin/execute";
	/** Script property name for no. */
	static constexpr const char *kPropertyNo = "no";
	/** Script identifier for lXML. */
	static constexpr const char *kLXMLName = "LXML";
	/** Script identifier for aSEQ. */
	static constexpr const char *kASEQName = "ASEQ";
	/** Script identifier for rRGB. */
	static constexpr const char *kRRGBName = "RRGB";
	/** Script identifier for jPEG. */
	static constexpr const char *kJPEGName = "JPEG";
	/** Script identifier for wAVE. */
	static constexpr const char *kWAVEName = "WAVE";
	/** Script identifier for nFNT. */
	static constexpr const char *kNFNTName = "NFNT";
	/** Script identifier for common. */
	static constexpr const char *kCommonName = "common";
	/** Script identifier for launcher/c Bar/cmd Buttons/brand Order/id. */
	static constexpr const char *kLauncherCBarCmdButtonsBrandOrderIdName = "launcher/cBar/cmdButtons/brandOrder/id";
	/** Maximum number of brand order buttons in the parents layout. */
	static constexpr int kMaxBrandOrderButtonCount = 6;
	/** Script identifier for launcher/c Bar/cmd Buttons/. */
	static constexpr const char *kLauncherCBarCmdButtonsName = "launcher/cBar/cmdButtons/";
	/** Script identifier for /cb. */
	static constexpr const char *kCbName = "/cb";
	/** Script identifier for launcher/c Bar/cmd Button Area/x. */
	static constexpr const char *kLauncherCBarCmdButtonAreaXName = "launcher/cBar/cmdButtonArea/x";
	/** Script identifier for launcher/c Bar/cmd Button Area/y. */
	static constexpr const char *kLauncherCBarCmdButtonAreaYName = "launcher/cBar/cmdButtonArea/y";
	/** Script identifier for /id. */
	static constexpr const char *kIdName = "/id";
	/** Script identifier for launcher/startup/screen. */
	static constexpr const char *kLauncherStartupScreenName = "launcher/startup/screen";
	/** Script identifier for launcher/music. */
	static constexpr const char *kLauncherMusicName = "launcher/music";
	/** XML element or attribute name for condition. */
	static constexpr const char *kXMLCondition = "condition";
	/** Script identifier for file Mac. */
	static constexpr const char *kFileMacName = "fileMac";
	/** Script identifier for no File Mac. */
	static constexpr const char *kNoFileMacName = "noFileMac";
	/** Script identifier for install. */
	static constexpr const char *kInstallName = "install";
	/** Script identifier for file. */
	static constexpr const char *kFileName = "file";
	/** Script identifier for file Win. */
	static constexpr const char *kFileWinName = "fileWin";
	/** Script identifier for no File. */
	static constexpr const char *kNoFileName = "noFile";
	/** Script identifier for no File Win. */
	static constexpr const char *kNoFileWinName = "noFileWin";
	/** XML element or attribute name for filename. */
	static constexpr const char *kXMLFilename = "filename";
	/** XML element or attribute name for type. */
	static constexpr const char *kXMLType = "type";
	/** Script class name for music. */
	static constexpr const char *kClassMusic = "music";
	/** XML element or attribute name for id. */
	static constexpr const char *kXMLId = "id";
	/** Script class name for text. */
	static constexpr const char *kClassText = "text";
	/** Script class name for toon. */
	static constexpr const char *kClassToon = "toon";
	/** Script class name for fob. */
	static constexpr const char *kClassFob = "fob";
	/** XML element or attribute name for target. */
	static constexpr const char *kXMLTarget = "target";
	/** XML element or attribute name for parameters. */
	static constexpr const char *kXMLParameters = "parameters";
	/** XML element or attribute name for x. */
	static constexpr const char *kXMLX = "x";
	/** XML element or attribute name for y. */
	static constexpr const char *kXMLY = "y";
	/** XML element or attribute name for class. */
	static constexpr const char *kXMLClass = "class";
	/** XML element or attribute name for checkbox. */
	static constexpr const char *kXMLCheckbox = "checkbox";
	/** XML element or attribute name for checked If/value. */
	static constexpr const char *kXMLCheckedIfValue = "checkedIf/value";
	/** XML element or attribute name for auto. */
	static constexpr const char *kXMLAuto = "auto";
	/** XML element or attribute name for start Frame. */
	static constexpr const char *kXMLStartFrame = "startFrame";
	/** XML element or attribute name for init Script. */
	static constexpr const char *kXMLInitScript = "initScript";
	/** XML element or attribute name for xml:. */
	static constexpr const char *kXMLXml = "xml:";
	/** XML element or attribute name for xml File. */
	static constexpr const char *kXMLXmlFile = "xmlFile";
	/** XML element or attribute name for dest. */
	static constexpr const char *kXMLDest = "dest";
	/** Script identifier for launcher/c Bar/left/id. */
	static constexpr const char *kLauncherCBarLeftIdName = "launcher/cBar/left/id";
	/** Script identifier for launcher/c Bar/left/x. */
	static constexpr const char *kLauncherCBarLeftXName = "launcher/cBar/left/x";
	/** Script identifier for launcher/c Bar/left/y. */
	static constexpr const char *kLauncherCBarLeftYName = "launcher/cBar/left/y";
	/** Script identifier for launcher/c Bar/right/id. */
	static constexpr const char *kLauncherCBarRightIdName = "launcher/cBar/right/id";
	/** Script identifier for launcher/c Bar/right/x. */
	static constexpr const char *kLauncherCBarRightXName = "launcher/cBar/right/x";
	/** Script identifier for launcher/c Bar/right/y. */
	static constexpr const char *kLauncherCBarRightYName = "launcher/cBar/right/y";
	/** Script identifier for brand. */
	static constexpr const char *kBrandName = "Brand";
	/** Script identifier for demos. */
	static constexpr const char *kDemosName = "Demos";
	/** Script identifier for init My CDRROMs. */
	static constexpr const char *kInitMyCDRROMsName = "initMyCDRROMs";
	/** Script identifier for product:startup. */
	static constexpr const char *kProductStartupName = "product:startup";
	/** Script identifier for product. */
	static constexpr const char *kProductName = "product";
	/** Script identifier for init Updates. */
	static constexpr const char *kInitUpdatesName = "initUpdates";
	/** Script identifier for init RRWorld. */
	static constexpr const char *kInitRRWorldName = "initRRWorld";
	/** Script identifier for init SFWorld. */
	static constexpr const char *kInitSFWorldName = "initSFWorld";
	/** Script identifier for init CFWorld. */
	static constexpr const char *kInitCFWorldName = "initCFWorld";
	/** Script identifier for init ZOWorld. */
	static constexpr const char *kInitZOWorldName = "initZOWorld";
	/** Script identifier for init OTWorld. */
	static constexpr const char *kInitOTWorldName = "initOTWorld";
	/** Script identifier for init CSWorld. */
	static constexpr const char *kInitCSWorldName = "initCSWorld";
	/** Script identifier for rscript:home Butt Handler. */
	static constexpr const char *kRscriptHomeButtHandlerName = "rscript:homeButtHandler";
	/** Script identifier for rscript:init Parents Area. */
	static constexpr const char *kRscriptInitParentsAreaName = "rscript:initParentsArea";
	/** Script identifier for launcher/content/mainmenu. */
	static constexpr const char *kLauncherContentMainmenuName = "launcher/content/mainmenu";
	/** Script identifier for rscript:init Product Grid. */
	static constexpr const char *kRscriptInitProductGridName = "rscript:initProductGrid";
	/** Script identifier for launcher/content/product Grid3to7. */
	static constexpr const char *kLauncherContentProductGrid3to7Name = "launcher/content/productGrid3to7";
	/** Script identifier for rscript:set Updates Auto. */
	static constexpr const char *kRscriptSetUpdatesAutoName = "rscript:setUpdatesAuto";
	/** Script identifier for rscript:set Updates Manual. */
	static constexpr const char *kRscriptSetUpdatesManualName = "rscript:setUpdatesManual";
	/** Script identifier for rscript:go Back. */
	static constexpr const char *kRscriptGoBackName = "rscript:goBack";
	/** Script identifier for startup:. */
	static constexpr const char *kStartupName = "startup:";
	/** Script identifier for rscript:stop Movie. */
	static constexpr const char *kRscriptStopMovieName = "rscript:stopMovie";
	/** Script identifier for bink:. */
	static constexpr const char *kBinkName = "bink:";
	/** Script identifier for launcher/startup/. */
	static constexpr const char *kLauncherStartupName = "launcher/startup/";
	/** Script identifier for launcher/content/. */
	static constexpr const char *kLauncherContentName = "launcher/content/";
	/** Script identifier for /x. */
	static constexpr const char *kXName = "/x";
	/** Script identifier for /y. */
	static constexpr const char *kYName = "/y";
	/** Script identifier for startup/screen. */
	static constexpr const char *kStartupScreenName = "startup/screen";
	/** Script identifier for all. */
	static constexpr const char *kAllName = "all";
	/** Script class name for scene. */
	static constexpr const char *kClassScene = "scene";
	/** XML element or attribute name for play. */
	static constexpr const char *kXMLPlay = "play";
	/** XML element or attribute name for link. */
	static constexpr const char *kXMLLink = "link";
	/** XML element or attribute name for external:. */
	static constexpr const char *kXMLExternal = "external:";
	/** Script class name for product/%d.jpg. */
	static constexpr const char *kClassProductSignedIndexJpg = "product/%d.jpg";
	/** Script identifier for background. */
	static constexpr const char *kBackgroundName = "background";
	/** Script identifier for signin. */
	static constexpr const char *kSigninName = "signin";
	/** Script identifier for name Box. */
	static constexpr const char *kNameBoxName = "nameBox";
	/** Script identifier for ok. */
	static constexpr const char *kOkName = "ok";
	/** Script identifier for new Name. */
	static constexpr const char *kNewNameName = "newName";
	/** Script identifier for cancel. */
	static constexpr const char *kCancelName = "cancel";
	/** Script identifier for up Arrow. */
	static constexpr const char *kUpArrowName = "upArrow";
	/** Script identifier for dn Arrow. */
	static constexpr const char *kDnArrowName = "dnArrow";
	/** Script identifier for backspace. */
	static constexpr const char *kBackspaceName = "backspace";
	/** Script identifier for space. */
	static constexpr const char *kSpaceName = "space";
	/** Script identifier for name List/x. */
	static constexpr const char *kNameListXName = "nameList/x";
	/** Script identifier for name List/y. */
	static constexpr const char *kNameListYName = "nameList/y";
	/** Script identifier for name List/w. */
	static constexpr const char *kNameListWName = "nameList/w";
	/** Script identifier for name List/h. */
	static constexpr const char *kNameListHName = "nameList/h";
	/** Script identifier for launcher/startup/help. */
	static constexpr const char *kLauncherStartupHelpName = "launcher/startup/help";
	/** Script identifier for launcher/content/whats New. */
	static constexpr const char *kLauncherContentWhatsNewName = "launcher/content/whatsNew";
	/** Script identifier for rr Brand. */
	static constexpr const char *kRrBrandName = "rrBrand";
	/** Script identifier for sf Brand. */
	static constexpr const char *kSfBrandName = "sfBrand";
	/** Script identifier for cf Brand. */
	static constexpr const char *kCfBrandName = "cfBrand";
	/** Script identifier for zo Brand. */
	static constexpr const char *kZoBrandName = "zoBrand";
	/** Script identifier for ot Brand. */
	static constexpr const char *kOtBrandName = "otBrand";
	/** Script identifier for cs Brand. */
	static constexpr const char *kCsBrandName = "csBrand";
	/** Script identifier for name List/. */
	static constexpr const char *kNameListName = "nameList/";
	/** Script identifier for /r. */
	static constexpr const char *kRName = "/r";
	/** Script identifier for /g. */
	static constexpr const char *kGName = "/g";
	/** Script identifier for /b. */
	static constexpr const char *kBName = "/b";
	/** Script identifier for bc. */
	static constexpr const char *kBcName = "bc";
	/** Script identifier for stc. */
	static constexpr const char *kStcName = "stc";
	/** Script identifier for ntc. */
	static constexpr const char *kNtcName = "ntc";
	/** Script identifier for hbc. */
	static constexpr const char *kHbcName = "hbc";
	/** Script identifier for mtc. */
	static constexpr const char *kMtcName = "mtc";
	/** Script property name for dialogs/delete/msg. */
	static constexpr const char *kPropertyDialogsDeleteMsg = "dialogs/delete/msg";
	/** XML element or attribute name for w. */
	static constexpr const char *kXMLW = "w";
	/** XML element or attribute name for h. */
	static constexpr const char *kXMLH = "h";
	/** XML element or attribute name for font. */
	static constexpr const char *kXMLFont = "font";
	/** XML element or attribute name for content. */
	static constexpr const char *kXMLContent = "content";
	/** XML element or attribute name for color/r. */
	static constexpr const char *kXMLColorR = "color/r";
	/** XML element or attribute name for color/g. */
	static constexpr const char *kXMLColorG = "color/g";
	/** XML element or attribute name for color/b. */
	static constexpr const char *kXMLColorB = "color/b";
	/** XML element or attribute name for center. */
	static constexpr const char *kXMLCenter = "center";
	/** XML element or attribute name for true. */
	static constexpr const char *kXMLTrue = "true";

	/** Target-specific launcher user registry suffix. */
	static constexpr const char *kUsersFileSuffix = ".trinket-launcher-users.xml";
	/** Directory containing launcher resource libraries and layouts. */
	static constexpr const char *kResourceDirectory = "rsc";
	/** Extension of launcher animation resources. */
	static constexpr const char *kAnimationExtension = ".ao";
	/** Extension of launcher palette resources. */
	static constexpr const char *kPaletteExtension = ".rgb";
	/** Extension of launcher background images. */
	static constexpr const char *kImageExtension = ".jpg";
	/** Extension of launcher audio resources. */
	static constexpr const char *kSoundExtension = ".wav";
	/** Extension of launcher bitmap fonts. */
	static constexpr const char *kFontExtension = ".nfnt";
	/** Virtual prefix of the sign-in resource library. */
	static constexpr const char *kSigninLibraryPrefix = "signin/";
	/** Virtual prefix of product resource libraries. */
	static constexpr const char *kProductLibraryPrefix = "product/";
	/** Virtual prefix of the shared launcher resource library. */
	static constexpr const char *kCommonLibraryPrefix = "common/";
	/** Filename used by links to the parents layout. */
	static constexpr const char *kParentsXMLName = "TLCParents.xml";
	/** Filename used by links to the home layout. */
	static constexpr const char *kHomeXMLName = "TLCHome.xml";

	enum ContentAction {
		kGridPrevious1900 = 1900,
		kGridNext1901 = 1901,
		kWorldMovie1902 = 1902,
		kContentLink2000 = 2000,
	};
	struct TextBlock {
		/** FontMan-owned font used to draw this content block. */
		const Graphics::Font *font = nullptr;
		/** Wrapped text lines in display order. */
		Common::Array<Common::String> lines;
		/** Game-pixel rectangle available for the text block. */
		Common::Rect rect;
		/** Red component of the text color. */
		byte red;
		/** Green component of the text color. */
		byte green;
		/** Blue component of the text color. */
		byte blue;
		/** Whether each line is horizontally centered in the block. */
		bool centered;
	};
	typedef Registry::User User;
	/** Engine used to install the detected platform's default arrow. */
	TrinketEngine &_vm;
	/** Save manager used for launcher user records. */
	Common::SaveFileManager &_manager;
	/** Target-specific filename of the launcher user registry. */
	Common::String _saveName;
	/** Launcher user records in display order. */
	Common::Array<User> _users;
	/** Next persistent launcher user identifier. */
	uint _nextID = 1;
	/** One-based game slot selected for a launch request. */
	uint _slot = 1;
	/** Selected user-list index, or minus one with no selection. */
	int _selected = -1;
	/** Index of the first user visible in the scrolling list. */
	int _first = 0;
	/** Whether the launcher is showing its startup screen. */
	bool _startup = true;
	/** Whether the current name edit creates a new user. */
	bool _newEntry = false;
	/** Whether the name-deletion confirmation dialog is open. */
	bool _confirmDelete = false;
	/** Whether a launcher content page is being displayed. */
	bool _content = false;
	/** Whether launcher music is enabled in the user registry. */
	bool _musicEnabled = true;
	/** Whether launcher sound effects are enabled in the user registry. */
	bool _soundEnabled = true;
	/** Whether the registry selects automatic launcher updates. */
	bool _updatesAutomatic = true;
	/** Whether player selection is being handled by the game-specific sign-in flow. */
	bool _gameSignin = false;
	/** Whether the current product page offers a game launch action. */
	bool _productPlay = false;
	/** Cursor resource selected while hovering over a launcher button. */
	Graphics::WinCursorGroup *_hoverCursor = nullptr;
	/** Whether the pointer currently selects launcher button feedback. */
	bool _overButton = false;
	/** Music resource ID selected by the launcher layout. */
	uint _defaultMusic = 0;
	/** Launcher music volume from zero through one hundred. */
	uint _musicVolume = 100;
	/** Resource path of the currently selected launcher music. */
	Common::String _musicPath;
	/** Player name currently being edited. */
	Common::String _typed;
	/** Canonical launcher resource directory. */
	Common::Path _root;
	/** Common launcher images, animations and fonts. */
	mutable Common::NEResources _common;
	/** Player sign-in resource library. */
	mutable Common::NEResources _signin;
	/** Selected product's launcher resource library. */
	mutable Common::NEResources _product;
	/** Brand navigation resource library. */
	mutable Common::NEResources _brands;
	/** Parent-information resource library. */
	mutable Common::NEResources _parents;
	/** Launcher speech, music and sound-effect library. */
	mutable Common::NEResources _audio;
	/** Installed news content resource library. */
	mutable Common::NEResources _news;
	/** Resource libraries for the six launcher world pages. */
	mutable Common::NEResources _worldLibs[6];
	/** Launcher FontMan registrations released after all launcher font consumers. */
	OM::FontCache _fonts;
	/** Player sign-in layout and interaction settings. */
	OM::XMLParser _layout;
	/** Installed launcher startup layout. */
	OM::XMLParser _startupLayout;
	/** Launcher home-page layout. */
	OM::XMLParser _homeLayout;
	/** Parent-information page layout. */
	OM::XMLParser _parentsLayout;
	/** Layouts for the six launcher demo worlds. */
	OM::XMLParser _demoLayouts[6];
	/** Installed news-page layout. */
	OM::XMLParser _newsLayout;
	/** XML document supplying the current content page. */
	const OM::XMLParser *_contentLayout = nullptr;
	/** XML document retained for content-page back navigation. */
	const OM::XMLParser *_previousLayout = nullptr;
	/** XML branch supplying the current content page. */
	Common::String _contentBranch;
	/** Previous content branch used for back navigation. */
	Common::String _previousBranch;
	/** Current page of a content tile grid. */
	uint _gridPage = 0;
	/** Decoder for the content page's video preview. */
	Video::BinkDecoder _contentMovie;
	/** Most recent decoded preview frame. */
	Graphics::Surface _movieFrame;
	/** Preview position in launcher game pixels. */
	Common::Point _moviePosition;
	/** Selected launcher world index, or minus one outside a world page. */
	int _world = -1;
	/** Text blocks laid out for the active content page. */
	Common::Array<TextBlock> _textBlocks;
	/** Link actions indexed by content-object input actions. */
	Common::Array<Common::String> _links;
	/** Parameters corresponding to the content link actions. */
	Common::Array<Common::String> _linkParameters;
	/** Persistent launcher frame and navigation sprites. */
	Shell::Scene _frameScene;
	/** Sprite for the frame's vertical navigation tab. */
	Shell::Sprite *_vertTab = nullptr;
	/** Sprite for the frame's horizontal navigation tab. */
	Shell::Sprite *_horizTab = nullptr;
	/** Frame sprite used to launch the selected installed product. */
	Shell::Sprite *_playCD = nullptr;
	/** Sprites used by the current sign-in or content screen. */
	Shell::Scene _scene;
	/** Sprites displayed by the current launcher modal dialog. */
	Shell::Scene _dialogScene;
	/** Font used to draw player names in the sign-in list. */
	const Graphics::Font *_listFont = nullptr;
	/** FontMan-owned font used for launcher dialog text. */
	const Graphics::Font *_dialogFont = nullptr;
	/** Playback state for launcher prompts and speech. */
	OM::Sound _voice;
	/** Playback state for launcher background music. */
	OM::Sound _music;
	/** Playback state for launcher button feedback. */
	OM::Sound _click;
	/** Confirmation button for the current sign-in state. */
	Shell::Sprite *_ok = nullptr;
	/** Button used to begin a new player-name entry. */
	Shell::Sprite *_newButton = nullptr;
	/** Game-pixel rectangle containing visible player names. */
	Common::Rect _listRect;
	/** Launcher directory name beneath the installed game data root. */
	static constexpr const char *kRoot = "LAUNCHER";
	/** Executable resource path containing launcher cursor images. */
	static constexpr const char *kCursorExecutable = "win/TLCLauncher.exe";
	/** Path of the common launcher resource library. */
	static constexpr const char *kCommon = "rsc/tlccommon.rsc";
	/** Path of the launcher sign-in resource library. */
	static constexpr const char *kSignin = "rsc/tlcsignin.rsc";
	/** Path of the player sign-in layout document. */
	static constexpr const char *kSigninXML = "rsc/TLCSignin.xml";
	/** Path of the launcher home-page layout document. */
	static constexpr const char *kHomeXML = "rsc/TLCHome.xml";
	/** Path of the parent-information layout document. */
	static constexpr const char *kParentsXML = "rsc/TLCParents.xml";
	/** Path of the brand navigation resource library. */
	static constexpr const char *kBrands = "rsc/tlcbrands.rsc";
	/** Path of the parent-information resource library. */
	static constexpr const char *kParents = "rsc/tlcparents.rsc";
	/** Path of the launcher audio resource library. */
	static constexpr const char *kAudio = "rsc/tlcaudio.rsc";
	/** Path of the installed news resource library. */
	static constexpr const char *kNews = "USERS/rsc/tlcwhatsnew.rsc";
	/** Directory used to locate launcher video previews. */
	static constexpr const char *kMovieDirectory = "TLCLauncher";
	/** Preview movie names indexed by launcher world. */
	static constexpr const char *kWorldMovies[6] = {
		"RRandI.bik",
		"mvjopening.bik",
		"Clue640.bik",
		"zoom_movie1.bik",
		"OT32211.bik",
		"CSD_final.bik",
	};
	/** Resource-library paths indexed by launcher world. */
	static constexpr const char *kWorldLibraries[6] = {
		"rsc/tlcrrworld.rsc",
		"rsc/tlcsfworld.rsc",
		"rsc/tlccfworld.rsc",
		"rsc/tlczoworld.rsc",
		"rsc/tlcotworld.rsc",
		"rsc/tlccsworld.rsc",
	};
	/** Demo layout paths indexed by launcher world. */
	static constexpr const char *kDemoXML[6] = {
		"TLCRRDemos.xml",
		"TLCSFDemos.xml",
		"TLCCFDemos.xml",
		"TLCZODemos.xml",
		"TLCOTDemos.xml",
		"TLCCSDemos.xml",
	};
	/** Path of the installed startup layout document. */
	static constexpr const char *kStartupXML = "USERS/installed/TLCLstartup.xml";
	/** Pattern mapping an audio library and resource ID to a music path. */
	static constexpr const char *kMusicResource = "%s/%u.wav";
	/** Resource path of the default launcher click sound. */
	static constexpr const char *kClickSound = "common/1001.wav";
	/** Pattern locating a product's launcher resource library. */
	static constexpr const char *kProductRSC = "PRODUCT/rsc/%s.rsc";
	/** Pattern locating a product's installed launcher layout. */
	static constexpr const char *kProductXML = "PRODUCT/installed/%s.xml";
	/** XML branch containing sign-in keyboard controls. */
	static constexpr const char *kSigninBranch = "launcher/signin/keyboard/";
	/** Resource path of the common launcher background image. */
	static constexpr const char *kBackground = "common/3000.jpg";
	/** Pattern mapping a launcher library and resource ID to an animation path. */
	static constexpr const char *kAnimation = "%s/%d.ao";
	/** Speech prompt played when entering a player name. */
	static constexpr const char *kEnterSpeech = "signin/4081.wav";
	/** Speech prompt played before removing a player name. */
	static constexpr const char *kRemoveSpeech = "signin/4071.wav";
	/** Resource path of the launcher dialog font. */
	static constexpr const char *kDialogFont = "common/15018.nfnt";
	/** Read user records and launcher preferences from the target registry. */
	bool loadUsers();
	/** Write user records and launcher preferences to the target registry. */
	bool storeUsers() const;
	/** Build the installed launcher startup screen. */
	bool showStartup();
	/** Build the product screen selected by an XML branch. */
	bool showProductScreen(const Common::String &branch);
	/** Build the player sign-in screen and its current list selection. */
	bool showSignin();
	/** Create persistent launcher navigation and product controls. */
	bool buildFrame();
	/** Build a content page from the supplied XML document and branch. */
	bool showContent(const OM::XMLParser &xml, const Common::String &branch);
	/** Execute a content link with its authored parameters. */
	void followLink(const Common::String &link, const Common::String &parameters = Common::String());
	/** Create the content objects described by an authored content script. */
	bool addContentScript(const OM::XMLParser &xml, const Common::String &script);
	/** Lay out a page of content tiles from XML. */
	bool addContentGrid(const OM::XMLParser &xml);
	/** Select a launcher world and build its content page. */
	bool showWorld(uint world);
	/** Load a video preview at a launcher game-pixel position. */
	void playContentMovie(const Common::String &name, int x, int y);
	/** Stop the active video preview and clear its frame. */
	void stopContentMovie();
	/** Show the standard notice for an unavailable launcher action. */
	static void notifyNotImplemented();
	/** Draw the current content page's laid-out text blocks. */
	void drawContentText(OM::Canvas &canvas);
	/** Create one text block from an XML element and its layout attributes. */
	bool addContentText(const OM::XMLParser &xml, uint node);
	/** Start the selected launcher music if its preference permits playback. */
	void startMusic();
	/** Resolve a library resource ID and select it for launcher music. */
	void selectMusic(uint id, const char *library);
	/** Create a frame sprite at a position with an optional input action. */
	static Shell::Sprite *addFrameObject(Shell::Scene &scene, int id, int x, int y, int action = 0);
	/** Apply the standard launcher button frame and pointer-feedback policy. */
	static void configureButton(Shell::Sprite &sprite);
	/** Install the authored hover cursor over buttons or restore the engine's default arrow. */
	void updateCursor(bool overButton);
	/** Build the player-name deletion confirmation dialog. */
	void showDeleteDialog();
	/** Resolve a named list color through the current launcher layout. */
	uint32 listColor(const OM::Canvas &canvas, const char *name) const;
	/** Create a launcher sprite from an XML branch and resource library. */
	Shell::Sprite *add(const OM::XMLParser &xml, const Common::String &branch, const char *library, int action);
	/** Append an accepted character to the current player-name edit. */
	void addCharacter(char character);
	/** Select an existing user whose name matches the current edit. */
	void matchName();
	/** Select a player-list index and update list scrolling and button state. */
	void select(int index);
	/** Return whether the current player-name selection or edit can be accepted. */
	bool valid() const;
	/** Compare launcher user names for alphabetical list order. */
	static bool userLess(const User &left, const User &right) { return left.name < right.name; }
};

} // End of namespace Launcher
} // End of namespace Trinket

#endif
