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

#include "trinket/launcher/application.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "common/formats/winexe_pe.h"
#include "common/fs.h"
#include "common/ptr.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "trinket/shell/text.h"
#include "trinket/string_parser.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace Launcher {

constexpr const char *Application::kPropertyStartupMusic;
constexpr const char *Application::kPropertyStartupSigninExecute;
constexpr const char *Application::kPropertyNo;
constexpr const char *Application::kLXMLName;
constexpr const char *Application::kASEQName;
constexpr const char *Application::kRRGBName;
constexpr const char *Application::kJPEGName;
constexpr const char *Application::kWAVEName;
constexpr const char *Application::kNFNTName;
constexpr const char *Application::kCommonName;
constexpr const char *Application::kLauncherCBarCmdButtonsBrandOrderIdName;
constexpr const char *Application::kLauncherCBarCmdButtonsName;
constexpr const char *Application::kCbName;
constexpr const char *Application::kLauncherCBarCmdButtonAreaXName;
constexpr const char *Application::kLauncherCBarCmdButtonAreaYName;
constexpr const char *Application::kIdName;
constexpr const char *Application::kLauncherStartupScreenName;
constexpr const char *Application::kLauncherMusicName;
constexpr const char *Application::kXMLCondition;
constexpr const char *Application::kFileMacName;
constexpr const char *Application::kNoFileMacName;
constexpr const char *Application::kInstallName;
constexpr const char *Application::kFileName;
constexpr const char *Application::kFileWinName;
constexpr const char *Application::kNoFileName;
constexpr const char *Application::kNoFileWinName;
constexpr const char *Application::kXMLFilename;
constexpr const char *Application::kXMLType;
constexpr const char *Application::kClassMusic;
constexpr const char *Application::kXMLId;
constexpr const char *Application::kClassText;
constexpr const char *Application::kClassToon;
constexpr const char *Application::kClassFob;
constexpr const char *Application::kXMLTarget;
constexpr const char *Application::kXMLParameters;
constexpr const char *Application::kXMLX;
constexpr const char *Application::kXMLY;
constexpr const char *Application::kXMLClass;
constexpr const char *Application::kXMLCheckbox;
constexpr const char *Application::kXMLCheckedIfValue;
constexpr const char *Application::kXMLAuto;
constexpr const char *Application::kXMLStartFrame;
constexpr const char *Application::kXMLInitScript;
constexpr const char *Application::kXMLXml;
constexpr const char *Application::kXMLXmlFile;
constexpr const char *Application::kXMLDest;
constexpr const char *Application::kLauncherCBarLeftIdName;
constexpr const char *Application::kLauncherCBarLeftXName;
constexpr const char *Application::kLauncherCBarLeftYName;
constexpr const char *Application::kLauncherCBarRightIdName;
constexpr const char *Application::kLauncherCBarRightXName;
constexpr const char *Application::kLauncherCBarRightYName;
constexpr const char *Application::kBrandName;
constexpr const char *Application::kDemosName;
constexpr const char *Application::kInitMyCDRROMsName;
constexpr const char *Application::kProductStartupName;
constexpr const char *Application::kProductName;
constexpr const char *Application::kInitUpdatesName;
constexpr const char *Application::kInitRRWorldName;
constexpr const char *Application::kInitSFWorldName;
constexpr const char *Application::kInitCFWorldName;
constexpr const char *Application::kInitZOWorldName;
constexpr const char *Application::kInitOTWorldName;
constexpr const char *Application::kInitCSWorldName;
constexpr const char *Application::kRscriptHomeButtHandlerName;
constexpr const char *Application::kRscriptInitParentsAreaName;
constexpr const char *Application::kLauncherContentMainmenuName;
constexpr const char *Application::kRscriptInitProductGridName;
constexpr const char *Application::kLauncherContentProductGrid3to7Name;
constexpr const char *Application::kRscriptSetUpdatesAutoName;
constexpr const char *Application::kRscriptSetUpdatesManualName;
constexpr const char *Application::kRscriptGoBackName;
constexpr const char *Application::kStartupName;
constexpr const char *Application::kRscriptStopMovieName;
constexpr const char *Application::kBinkName;
constexpr const char *Application::kLauncherStartupName;
constexpr const char *Application::kLauncherContentName;
constexpr const char *Application::kXName;
constexpr const char *Application::kYName;
constexpr const char *Application::kStartupScreenName;
constexpr const char *Application::kAllName;
constexpr const char *Application::kClassScene;
constexpr const char *Application::kXMLPlay;
constexpr const char *Application::kXMLLink;
constexpr const char *Application::kXMLExternal;
constexpr const char *Application::kClassProductSignedIndexJpg;
constexpr const char *Application::kBackgroundName;
constexpr const char *Application::kSigninName;
constexpr const char *Application::kNameBoxName;
constexpr const char *Application::kOkName;
constexpr const char *Application::kNewNameName;
constexpr const char *Application::kCancelName;
constexpr const char *Application::kUpArrowName;
constexpr const char *Application::kDnArrowName;
constexpr const char *Application::kBackspaceName;
constexpr const char *Application::kSpaceName;
constexpr const char *Application::kNameListXName;
constexpr const char *Application::kNameListYName;
constexpr const char *Application::kNameListWName;
constexpr const char *Application::kNameListHName;
constexpr const char *Application::kLauncherStartupHelpName;
constexpr const char *Application::kLauncherContentWhatsNewName;
constexpr const char *Application::kRrBrandName;
constexpr const char *Application::kSfBrandName;
constexpr const char *Application::kCfBrandName;
constexpr const char *Application::kZoBrandName;
constexpr const char *Application::kOtBrandName;
constexpr const char *Application::kCsBrandName;
constexpr const char *Application::kNameListName;
constexpr const char *Application::kRName;
constexpr const char *Application::kGName;
constexpr const char *Application::kBName;
constexpr const char *Application::kBcName;
constexpr const char *Application::kStcName;
constexpr const char *Application::kNtcName;
constexpr const char *Application::kHbcName;
constexpr const char *Application::kMtcName;
constexpr const char *Application::kPropertyDialogsDeleteMsg;
constexpr const char *Application::kXMLW;
constexpr const char *Application::kXMLH;
constexpr const char *Application::kXMLFont;
constexpr const char *Application::kXMLContent;
constexpr const char *Application::kXMLColorR;
constexpr const char *Application::kXMLColorG;
constexpr const char *Application::kXMLColorB;
constexpr const char *Application::kXMLCenter;
constexpr const char *Application::kXMLTrue;

constexpr const char *Application::kUsersFileSuffix;
constexpr const char *Application::kResourceDirectory;
constexpr const char *Application::kAnimationExtension;
constexpr const char *Application::kPaletteExtension;
constexpr const char *Application::kImageExtension;
constexpr const char *Application::kSoundExtension;
constexpr const char *Application::kFontExtension;
constexpr const char *Application::kSigninLibraryPrefix;
constexpr const char *Application::kProductLibraryPrefix;
constexpr const char *Application::kCommonLibraryPrefix;
constexpr const char *Application::kParentsXMLName;
constexpr const char *Application::kHomeXMLName;

constexpr const char *Application::kRoot;
constexpr const char *Application::kCursorExecutable;
constexpr const char *Application::kCommon;
constexpr const char *Application::kSignin;
constexpr const char *Application::kSigninXML;
constexpr const char *Application::kHomeXML;
constexpr const char *Application::kParentsXML;
constexpr const char *Application::kBrands;
constexpr const char *Application::kParents;
constexpr const char *Application::kAudio;
constexpr const char *Application::kNews;
constexpr const char *Application::kMovieDirectory;
constexpr const char *Application::kWorldMovies[6];
constexpr const char *Application::kWorldLibraries[6];
constexpr const char *Application::kDemoXML[6];
constexpr const char *Application::kStartupXML;
constexpr const char *Application::kMusicResource;
constexpr const char *Application::kClickSound;
constexpr const char *Application::kProductRSC;
constexpr const char *Application::kProductXML;
constexpr const char *Application::kSigninBranch;
constexpr const char *Application::kBackground;
constexpr const char *Application::kAnimation;
constexpr const char *Application::kEnterSpeech;
constexpr const char *Application::kRemoveSpeech;
constexpr const char *Application::kDialogFont;

Application::Application(TrinketEngine &vm, Common::SaveFileManager &manager, const Common::String &target, Audio::Mixer &mixer)
	: _vm(vm), _manager(manager), _saveName(target + kUsersFileSuffix),
	  _fonts(*this), _frameScene(*this), _scene(*this), _dialogScene(*this), _voice(mixer), _music(mixer), _click(mixer) {
}

bool Application::loadUsers() {
	Registry registry;
	if (!registry.load(_manager, _saveName))
		return false;
	_users = registry.users;
	_nextID = registry.nextID;
	_musicEnabled = registry.musicEnabled;
	_soundEnabled = registry.soundEnabled;
	_updatesAutomatic = registry.updatesAutomatic;
	return true;
}

bool Application::storeUsers() const {
	Registry registry;
	registry.users = _users;
	registry.nextID = _nextID;
	registry.musicEnabled = _musicEnabled;
	registry.soundEnabled = _soundEnabled;
	registry.updatesAutomatic = _updatesAutomatic;
	return registry.store(_manager, _saveName);
}

bool Application::load(const Common::Path &gamePath, const char *product) {
	// Intentionally replace Windows SYSTEM_FONT to avoid host-specific font files.
	// FontManager owns this font; game-provided NFNT fonts retain their original rendering.
	_listFont = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	Common::Path root = gamePath.join(kRoot);
	_root = root;
	delete _hoverCursor;
	_hoverCursor = nullptr;
	Common::PEResources executable;
	if (executable.loadFromEXE(Common::FSNode(root.join(kCursorExecutable)).createReadStream()))
		_hoverCursor = Graphics::WinCursorGroup::createCursorGroup(&executable, 1);
	_overButton = false;
	_vm.setDefaultCursor();
	OM::XMLParser startup;
	if (!startup.loadFile(Common::FSNode(root.join(kStartupXML))) || !startup.parse())
		return false;
	Common::String defaultMusicText = startup.get(kPropertyStartupMusic);
	_defaultMusic = 0;
	if (!defaultMusicText.empty()) {
		int32 defaultMusic = 0;
		if (StringParser::parseSignedInt(defaultMusicText.c_str(), defaultMusic, StringParser::kDecimalProfile) !=
			StringParser::Result::kSuccess || defaultMusic < 0)
			return false;
		_defaultMusic = static_cast<uint>(defaultMusic);
	}
	if (!_common.loadFromEXE(Common::FSNode(root.join(kCommon)).createReadStream()) ||
		!_signin.loadFromEXE(Common::FSNode(root.join(kSignin)).createReadStream()) ||
		!_brands.loadFromEXE(Common::FSNode(root.join(kBrands)).createReadStream()) ||
		!_parents.loadFromEXE(Common::FSNode(root.join(kParents)).createReadStream()) ||
		!_audio.loadFromEXE(Common::FSNode(root.join(kAudio)).createReadStream()) ||
		!_product.loadFromEXE(Common::FSNode(root.join(Common::String::format(kProductRSC, product))).createReadStream())) {
		warning("Trinket: failed to load launcher NE libraries from %s", root.toString().c_str());
		return false;
	}
	if (!_homeLayout.loadFile(Common::FSNode(root.join(kHomeXML))) || !_homeLayout.parse() ||
		!_parentsLayout.loadFile(Common::FSNode(root.join(kParentsXML))) || !_parentsLayout.parse() ||
		!_layout.loadFile(Common::FSNode(root.join(kSigninXML))) || !_layout.parse() ||
		!_startupLayout.loadFile(Common::FSNode(root.join(Common::String::format(kProductXML, product)))) || !_startupLayout.parse()) {
		warning("Trinket: failed to load launcher XML");
		return false;
	}
	if (!loadUsers()) {
		warning("Trinket: failed to load launcher users");
		return false;
	}
	_gameSignin = _startupLayout.get(kPropertyStartupSigninExecute).equals(kPropertyNo);
	for (uint i = 0; i < ARRAYSIZE(kDemoXML); i++) {
		if (!_worldLibs[i].loadFromEXE(Common::FSNode(root.join(kWorldLibraries[i])).createReadStream()))
			return false;
		if (!_demoLayouts[i].loadFile(Common::FSNode(root.join(kResourceDirectory).join(kDemoXML[i]))) || !_demoLayouts[i].parse())
			return false;
	}
	if (!_news.loadFromEXE(Common::FSNode(root.join(kNews)).createReadStream()))
		return false;
	Common::ScopedPtr<Common::SeekableReadStream> news(_news.getResource(Common::String(kLXMLName), 10002));
	if (!news)
		return false;
	Common::String newsXML;
	while (!news->eos()) {
		char c = news->readByte();
		if (!c)
			break;
		newsXML += c;
	}
	if (!_newsLayout.loadBuffer(reinterpret_cast<const byte *>(newsXML.c_str()), newsXML.size()) || !_newsLayout.parse())
		return false;
	_dialogFont = _fonts.load(kDialogFont);
	if (!_dialogFont) {
		warning("Trinket: failed to load launcher dialog font %s", kDialogFont);
		return false;
	}
	if (!buildFrame() || !showStartup()) {
		warning("Trinket: failed to construct launcher startup scene");
		return false;
	}
	return true;
}

Common::SeekableReadStream *Application::open(const Common::String &name) const {
	uint slash = name.find('/');
	if (slash == Common::String::npos)
		return nullptr;
	Common::String idText = name.substr(slash + 1);
	uint extension = idText.find('.');
	if (extension != Common::String::npos)
		idText = idText.substr(0, extension);
	uint32 id = 0;
	if (StringParser::parseUnsignedInt(idText.c_str(), id, StringParser::kDecimalProfile) != StringParser::Result::kSuccess)
		return nullptr;
	const char *tag = nullptr;
	if (name.hasSuffix(kAnimationExtension))
		tag = kASEQName;
	else if (name.hasSuffix(kPaletteExtension))
		tag = kRRGBName;
	else if (name.hasSuffix(kImageExtension))
		tag = kJPEGName;
	else if (name.hasSuffix(kSoundExtension))
		tag = kWAVEName;
	else if (name.hasSuffix(kFontExtension))
		tag = kNFNTName;
	if (!tag || !id)
		return nullptr;
	const Common::WinResourceID type = Common::String(tag);
	if (name.hasPrefix(kSigninLibraryPrefix))
		return _signin.getResource(type, id);
	if (name.hasPrefix(kProductLibraryPrefix)) {
		Common::SeekableReadStream *stream = _product.getResource(type, id);
		if (stream)
			return stream;
	}
	Common::SeekableReadStream *stream = _common.getResource(type, id);
	if (!stream)
		stream = _brands.getResource(type, id);
	if (!stream)
		stream = _parents.getResource(type, id);
	if (!stream)
		stream = _audio.getResource(type, id);
	if (!stream)
		stream = _news.getResource(type, id);
	for (uint i = 0; !stream && i < ARRAYSIZE(_worldLibs); i++)
		stream = _worldLibs[i].getResource(type, id);
	return stream;
}

void Application::configureButton(Shell::Sprite &sprite) {
	// Launcher buttons use idle/click/hover/disabled logical frames 1/2/3/4.
	sprite.flyButton = sprite.button;
	sprite.hoverFrame = sprite.asset->pictureForFrame(2) < 0 ? 1 : 2;
	sprite.disabledFrame = 3;
}

Shell::Sprite *Application::addFrameObject(Shell::Scene &scene, int id, int x, int y, int action) {
	Shell::Sprite *sprite = scene.add(Common::String::format(kAnimation, kCommonName, id), action ? 1 : 0, action, action != 0);
	if (sprite) {
		sprite->x = x;
		sprite->y = y;
		configureButton(*sprite);
	} else {
		warning("Trinket: missing launcher animation %d", id);
	}
	return sprite;
}

bool Application::buildFrame() {
	_frameScene.clear();
	static const int stamps[][3] = {
		{3004, 665, 61},
		{3002, 37, 61},
		{3001, 37, 32},
		{3003, 37, 407},
		{3005, 406, 5},
	};
	for (uint i = 0; i < ARRAYSIZE(stamps); i++) {
		if (!addFrameObject(_frameScene, stamps[i][0], stamps[i][1], stamps[i][2]))
			return false;
	}
	if (!addFrameObject(_frameScene, 3052, 697, 147, 1000) ||
		!addFrameObject(_frameScene, _musicEnabled ? 3053 : 3054, _musicEnabled ? 683 : 681, _musicEnabled ? 110 : 108, 1001) ||
		!addFrameObject(_frameScene, _soundEnabled ? 3055 : 3056, _soundEnabled ? 727 : 725, _soundEnabled ? 110 : 108, 1002) ||
		!addFrameObject(_frameScene, 3057, 686, 205, 1003) ||
		!addFrameObject(_frameScene, 3058, 686, 287, 1004) ||
		!addFrameObject(_frameScene, 3059, 686, 369, 1005))
		return false;
	_vertTab = addFrameObject(_frameScene, 3006, 665, 61);
	_horizTab = addFrameObject(_frameScene, 3007, 8, 443);
	_playCD = addFrameObject(_frameScene, 3051, 685, 15, 1006);
	if (!_vertTab || !_horizTab || !_playCD)
		return false;
	_playCD->visible = _content;
	_horizTab->visible = false;
	Common::String branch = kLauncherCBarCmdButtonsBrandOrderIdName;
	const Common::Array<OM::XMLParser::Element> &elements = _parentsLayout.elements();
	int brandOrderIdx = 0;
	for (uint i = 0; i < elements.size(); i++) {
		if (!elements[i].path.equals(branch))
			continue;
		Common::String button = kLauncherCBarCmdButtonsName + elements[i].value + kCbName;
		int32 areaX = 0;
		int32 y = 0;
		int32 id = 0;
		if (_parentsLayout.number(kLauncherCBarCmdButtonAreaXName, areaX) != StringParser::Result::kSuccess ||
			_parentsLayout.number(kLauncherCBarCmdButtonAreaYName, y) != StringParser::Result::kSuccess ||
			_parentsLayout.number(button + kIdName, id) != StringParser::Result::kSuccess)
			return false;
		int x = areaX + 115 * brandOrderIdx;
		int action = 1010 + brandOrderIdx;
		if (!addFrameObject(_frameScene, id, x, y, action))
			return false;
		brandOrderIdx += 1;
	}
	if (brandOrderIdx != kMaxBrandOrderButtonCount)
		warning("Trinket: launcher brand order contains %d entries", brandOrderIdx);
	return brandOrderIdx == kMaxBrandOrderButtonCount;
}

void Application::startMusic() {
	uint volume = _musicEnabled && !_contentMovie.isVideoLoaded() ? _musicVolume : 0;
	if (!_musicPath.empty() && !_music.isPlaying()) {
		if (!_music.play(*this, _musicPath, Audio::Mixer::kMusicSoundType, 0, volume))
			warning("Trinket: could not play launcher music %s", _musicPath.c_str());
		else
			debug(1, "Trinket launcher music: %s, volume %u", _musicPath.c_str(), volume);
	} else {
		_music.setVolume(volume);
		debug(1, "Trinket launcher music volume: %u (track retained)", volume);
	}
}

void Application::selectMusic(uint id, const char *library) {
	// A zero ID keeps the current ditty; selecting the same track preserves its position.
	if (id) {
		Common::String path = Common::String::format(kMusicResource, library, id);
		if (!_musicPath.equals(path)) {
			_music.stop();
			_musicPath = path;
		}
	}
	startMusic();
}

void Application::suspend() {
	updateCursor(false);
	stopContentMovie();
	_voice.stop();
	_music.stop();
	_click.stop();
}

bool Application::showContent(const OM::XMLParser &xml, const Common::String &branch) {
	bool found = false;
	for (uint i = 0; i < xml.elements().size(); i++)
		found |= xml.elements()[i].path.equals(branch);
	if (!found) {
		warning("Trinket: missing launcher content %s", branch.c_str());
		return false;
	}
	_previousLayout = _contentLayout;
	_previousBranch = _contentBranch;
	if (_contentLayout != &xml || !_contentBranch.equals(branch))
		_gridPage = 0;
	_contentLayout = &xml;
	_contentBranch = branch;
	stopContentMovie();
	_world = -1;
	_voice.stop();
	_scene.clear();
	_ok = _newButton = nullptr;
	_content = _startup = true;
	_textBlocks.clear();
	_links.clear();
	_linkParameters.clear();
	_playCD->visible = true;
	if (!_scene.setBackground(kBackground))
		return false;
	const Common::Array<OM::XMLParser::Element> &elements = xml.elements();
	uint music = 0;
	if (branch.equals(kLauncherStartupScreenName)) {
		Common::String musicText = xml.get(kLauncherMusicName);
		if (!musicText.empty()) {
			int32 parsedMusic = 0;
			if (StringParser::parseSignedInt(musicText.c_str(), parsedMusic, StringParser::kDecimalProfile) !=
				StringParser::Result::kSuccess || parsedMusic < 0)
				return false;
			music = static_cast<uint>(parsedMusic);
		}
	}
	for (uint i = 0; i < elements.size(); i++) {
		if (elements[i].parent < 0 || !elements[elements[i].parent].path.equals(branch))
			continue;
		Common::String condition = xml.child(i, kXMLCondition);
		if (condition.equals(kFileMacName) || condition.equals(kNoFileMacName) || condition.equals(kInstallName))
			continue;
		if (condition.equals(kFileName) || condition.equals(kFileWinName) || condition.equals(kNoFileName) || condition.equals(kNoFileWinName)) {
			bool exists = Common::FSNode(_root.getParent().getParent().join(kMovieDirectory).join(xml.child(i, kXMLFilename))).exists();
			if (exists == condition.hasPrefix(kNoFileName))
				continue;
		}
		Common::String type = xml.child(i, kXMLType);
		if (type.equals(kClassMusic)) {
			int32 parsedMusic = 0;
			Common::String musicText = xml.child(i, kXMLId);
			if (StringParser::parseSignedInt(musicText.c_str(), parsedMusic, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
				parsedMusic < 0)
				return false;
			music = static_cast<uint>(parsedMusic);
		} else if (type.equals(kClassText)) {
			if (!addContentText(xml, i))
				return false;
		} else if (type.equals(kClassToon) || type.equals(kClassFob)) {
			int action = 0;
			if (type.equals(kClassFob)) {
				action = kContentLink2000 + _links.size();
				_links.push_back(xml.child(i, kXMLTarget));
				_linkParameters.push_back(xml.child(i, kXMLParameters));
			}
			int32 id = 0;
			int32 x = 0;
			int32 y = 0;
			if (StringParser::parseSignedInt(xml.child(i, kXMLId).c_str(), id, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
				StringParser::parseSignedInt(xml.child(i, kXMLX).c_str(), x, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
				StringParser::parseSignedInt(xml.child(i, kXMLY).c_str(), y, StringParser::kDecimalProfile) != StringParser::Result::kSuccess)
				return false;
			Shell::Sprite *sprite = addFrameObject(_scene, id, x, y, action);
			if (!sprite)
				return false;
			if (xml.child(i, kXMLClass).equals(kXMLCheckbox)) {
				bool checked = (xml.child(i, kXMLCheckedIfValue).equals(kXMLAuto)) == _updatesAutomatic;
				// The selected option is disabled and displays its checked frame.
				sprite->enabled = !checked;
				sprite->disabledFrame = 4;
			}
			if (type.equals(kClassToon)) {
				int32 frame = 0;
				Common::String frameText = xml.child(i, kXMLStartFrame);
				if (!frameText.empty() && StringParser::parseSignedInt(frameText.c_str(), frame, StringParser::kDecimalProfile) !=
					StringParser::Result::kSuccess)
					return false;
				if (0 < frame)
					sprite->setFrame(frame);
			}
		}
	}
	for (uint i = 0; i < elements.size(); i++) {
		if (0 <= elements[i].parent && elements[elements[i].parent].path.equals(branch) && xml.child(i, kXMLType).equals(kXMLInitScript)) {
			if (!addContentScript(xml, xml.child(i, kXMLTarget)))
				return false;
		}
	}
	_musicVolume = 100;
	selectMusic(music, kCommonName);
	debug(1, "Trinket launcher content: %s", branch.c_str());
	return true;
}

void Application::notifyNotImplemented() {
	// Keep external launcher services as explicit no-ops without changing scenes.
	g_system->displayMessageOnOSD(_("Not Implemented"));
}

bool Application::addContentGrid(const OM::XMLParser &xml) {
	Common::Array<uint> buttons;
	const Common::Array<OM::XMLParser::Element> &elements = xml.elements();
	for (uint i = 0; i < elements.size(); i++) {
		if (elements[i].path.hasPrefix(kLauncherCBarCmdButtonsName) && elements[i].path.hasSuffix(kCbName))
			buttons.push_back(i);
	}
	uint first = MIN<uint>(_gridPage * 6, buttons.size());
	uint count = MIN<uint>(6, buttons.size() - first);
	static constexpr int positions[6][12] = {
		{310, 195},
		{245, 195, 375, 195},
		{180, 195, 310, 195, 440, 195},
		{245, 120, 375, 120, 245, 270, 375, 270},
		{245, 120, 375, 120, 180, 270, 310, 270, 440, 270},
		{180, 120, 310, 120, 440, 120, 180, 270, 310, 270, 440, 270}};
	for (uint i = 0; i < count; i++) {
		uint node = buttons[first + i];
		int action = kContentLink2000 + _links.size();
		_links.push_back(kXMLXml + xml.child(node, kXMLXmlFile));
		_linkParameters.push_back(xml.child(node, kXMLDest));
		Common::String idText = xml.child(node, kXMLId);
		int32 id = 0;
		if (StringParser::parseSignedInt(idText.c_str(), id, StringParser::kDecimalProfile) != StringParser::Result::kSuccess)
			return false;
		int x = positions[count - 1][i * 2] - 20;
		int y = positions[count - 1][i * 2 + 1];
		if (!addFrameObject(_scene, id, x, y, action))
			return false;
	}
	if (6 < buttons.size()) {
		int32 leftId = 0;
		int32 leftX = 0;
		int32 leftY = 0;
		int32 rightId = 0;
		int32 rightX = 0;
		int32 rightY = 0;
		if (xml.number(kLauncherCBarLeftIdName, leftId) != StringParser::Result::kSuccess ||
			xml.number(kLauncherCBarLeftXName, leftX) != StringParser::Result::kSuccess ||
			xml.number(kLauncherCBarLeftYName, leftY) != StringParser::Result::kSuccess ||
			xml.number(kLauncherCBarRightIdName, rightId) != StringParser::Result::kSuccess ||
			xml.number(kLauncherCBarRightXName, rightX) != StringParser::Result::kSuccess ||
			xml.number(kLauncherCBarRightYName, rightY) != StringParser::Result::kSuccess)
			return false;
		Shell::Sprite *left = addFrameObject(_scene, leftId, leftX, leftY, kGridPrevious1900);
		Shell::Sprite *right = addFrameObject(_scene, rightId, rightX, rightY, kGridNext1901);
		if (!left || !right)
			return false;
		left->visible = 0 < first;
		right->visible = first + count < buttons.size();
	}
	return true;
}

bool Application::addContentScript(const OM::XMLParser &xml, const Common::String &script) {
	if (script.hasSuffix(kBrandName)) {
		// Demo sparkles only appear after a new download; the local catalogue starts clear.
		return true;
	}
	if (script.hasSuffix(kDemosName))
		return addContentGrid(xml);
	if (script.equals(kInitMyCDRROMsName)) {
		// Disc-only product packs have no installed-product icon.
		Common::ScopedPtr<Common::SeekableReadStream> icon(_product.getResource(Common::String(kASEQName), 2051));
		if (!icon)
			return true;
		int action = kContentLink2000 + _links.size();
		_links.push_back(kProductStartupName);
		_linkParameters.push_back("");
		Shell::Sprite *button = _scene.add(Common::String::format(kAnimation, kProductName, 2051), 1, action, true);
		if (!button)
			return false;
		button->x = 290;
		button->y = 195;
		configureButton(*button);
		return true;
	}
	if (script.equals(kInitUpdatesName)) {
		TextBlock block;
		block.font = _fonts.load(kDialogFont);
		if (!block.font)
			return false;
		block.lines.push_back("Your products are all up to date.");
		block.rect = Common::Rect(50, 100, 650, 130);
		block.red = 57;
		block.green = 12;
		block.blue = 107;
		block.centered = false;
		_textBlocks.push_back(block);
		return true;
	}
	static constexpr const char *worlds[] = {
		kInitRRWorldName,
		kInitSFWorldName,
		kInitCFWorldName,
		kInitZOWorldName,
		kInitOTWorldName,
		kInitCSWorldName,
	};
	for (uint i = 0; i < ARRAYSIZE(worlds); i++) {
		if (script.equals(worlds[i]))
			return showWorld(i);
	}
	warning("Trinket: unknown launcher content script %s", script.c_str());
	return false;
}

void Application::stopContentMovie() {
	_contentMovie.close();
	_movieFrame.free();
	_music.setVolume(_musicEnabled ? _musicVolume : 0);
}

void Application::playContentMovie(const Common::String &name, int x, int y) {
	stopContentMovie();
	Common::SeekableReadStream *stream = Common::FSNode(_root.getParent().getParent().join(kMovieDirectory).join(name)).createReadStream();
	if (!stream || !_contentMovie.loadStream(stream)) {
		warning("Trinket: missing launcher movie %s", name.c_str());
		return;
	}
	_moviePosition = Common::Point(x, y);
	_contentMovie.setVolume(_soundEnabled ? 255 : 0);
	_music.setVolume(0);
	_contentMovie.start();
}

bool Application::showWorld(uint world) {
	static constexpr int characters[][5] = {
		{0, 24051, 528, 293, 3},
		{0, 24059, 453, 254, 2},
		{1, 25051, 605, 245, 3},
		{1, 25055, 70, 167, 1},
		{1, 25060, 557, 143, 2},
		{1, 25067, 565, 52, 1},
		{2, 26051, 488, 149, 1},
		{2, 26058, 87, 131, 1},
		{2, 26064, 551, 167, 2},
		{2, 26069, 49, 184, 2},
		{3, 27051, 276, 372, 1},
		{3, 27052, 558, 378, 1},
		{3, 27054, 630, 341, 1},
		{3, 27055, 117, 381, 1},
		{4, 29051, 56, 98, 3},
		{4, 29053, 561, 140, 1},
		{4, 29054, 599, 158, 2},
		{4, 29055, 547, 199, 2},
	};
	static constexpr int screens[6][6] = {
		{24100, 383, 409, 15, 114, 84},
		{25100, 49, 251, 9, 246, 149},
		{26100, 310, 68, 37, 200, 59},
		{27100, 121, 215, 31, 271, 69},
		{29100, 63, 47, 4, 175, 105},
		{28100, 57, 402, 20, 98, 88},
	};
	_world = world;
	for (uint i = 0; i < ARRAYSIZE(characters); i++) {
		if (characters[i][0] != static_cast<int>(world))
			continue;
		Shell::Sprite *sprite = addFrameObject(_scene, characters[i][1], characters[i][2], characters[i][3]);
		if (!sprite)
			return false;
		_scene.setDepth(sprite, characters[i][4]);
	}
	Shell::Sprite *screen = addFrameObject(_scene, screens[world][0], screens[world][1], screens[world][2]);
	if (!screen)
		return false;
	screen->flyButton = false;
	screen->setFrame(screens[world][3]);
	_scene.setDepth(screen, 4);
	playContentMovie(kWorldMovies[world], screens[world][4], screens[world][5]);
	return true;
}

void Application::followLink(const Common::String &link, const Common::String &parameters) {
	if (link.hasPrefix(kXMLXml)) {
		Common::String file = link.substr(4);
		if (file.equalsIgnoreCase(kParentsXMLName))
			showContent(_parentsLayout, parameters);
		else if (file.equalsIgnoreCase(kHomeXMLName))
			showContent(_homeLayout, parameters);
		else {
			for (uint i = 0; i < ARRAYSIZE(kDemoXML); i++) {
				if (file.equalsIgnoreCase(kDemoXML[i])) {
					showContent(_demoLayouts[i], parameters);
					return;
				}
			}
			warning("Trinket: unknown launcher XML %s", file.c_str());
		}
		return;
	}
	if (link.equals(kRscriptHomeButtHandlerName))
		showContent(_homeLayout, kLauncherStartupScreenName);
	else if (link.equals(kRscriptInitParentsAreaName))
		showContent(_parentsLayout, kLauncherContentMainmenuName);
	else if (link.equals(kRscriptInitProductGridName))
		showContent(_parentsLayout, kLauncherContentProductGrid3to7Name);
	else if (link.equals(kRscriptSetUpdatesAutoName) || link.equals(kRscriptSetUpdatesManualName)) {
		_updatesAutomatic = link.equals(kRscriptSetUpdatesAutoName);
		if (!storeUsers())
			warning("Trinket: could not save launcher update preference");
		Common::String branch = _contentBranch;
		showContent(*_contentLayout, branch);
	} else if (link.equals(kRscriptGoBackName) && _previousLayout) {
		Common::String branch = _previousBranch;
		showContent(*_previousLayout, branch);
	} else if (link.hasPrefix(kStartupName))
		showProductScreen(link.substr(8));
	else if (link.equals(kProductStartupName))
		showStartup();
	else if (link.equals(kRscriptStopMovieName))
		stopContentMovie();
	else if (link.hasPrefix(kBinkName)) {
		int32 movieId = 0;
		if (StringParser::parseSignedInt(parameters.c_str(), movieId, StringParser::kDecimalProfile) == StringParser::Result::kSuccess &&
			movieId >= 0)
			playContentMovie(link.substr(5), movieId, 75);
		else
			warning("Trinket: invalid launcher movie ID %s", parameters.c_str());
	} else if (link.hasPrefix(kLauncherStartupName))
		showContent(_homeLayout, link);
	else if (link.hasPrefix(kLauncherContentName))
		showContent(_contentLayout ? *_contentLayout : _parentsLayout, link);
	else
		notifyNotImplemented();
}

Shell::Sprite *Application::add(const OM::XMLParser &xml, const Common::String &branch, const char *library, int action) {
	int32 id = 0;
	int32 x = 0;
	int32 y = 0;
	if (xml.number(branch + kIdName, id) != StringParser::Result::kSuccess ||
		xml.number(branch + kXName, x) != StringParser::Result::kSuccess ||
		xml.number(branch + kYName, y) != StringParser::Result::kSuccess)
		return nullptr;
	if (id < 1)
		return nullptr;
	Shell::Sprite *sprite = _scene.add(Common::String::format(kAnimation, library, id), 1, action, action != 0);
	if (sprite) {
		sprite->x = x;
		sprite->y = y;
		configureButton(*sprite);
	}
	return sprite;
}

bool Application::showStartup() {
	return showProductScreen(kStartupScreenName);
}

bool Application::showProductScreen(const Common::String &branch) {
	bool found = false;
	for (const OM::XMLParser::Element &element : _startupLayout.elements())
		found |= element.path.equals(branch);
	if (!found)
		return false;
	stopContentMovie();
	_voice.stop();
	_scene.clear();
	_content = false;
	_textBlocks.clear();
	_links.clear();
	_linkParameters.clear();
	_playCD->visible = false;
	_vertTab->setFrame(1);
	_horizTab->setFrame(1);
	_horizTab->visible = false;
	_ok = _newButton = nullptr;
	_startup = true;
	_productPlay = false;
	if (!_scene.setBackground(kBackground))
		return false;
	const auto &elements = _startupLayout.elements();
	for (uint i = 0; i < elements.size(); i++) {
		if (elements[i].parent < 0 || !elements[elements[i].parent].path.equals(branch))
			continue;
		Common::String condition = _startupLayout.child(i, kXMLCondition);
		if (!condition.empty() && !condition.equals(kAllName))
			continue;
		Common::String type = _startupLayout.child(i, kXMLType);
		if (!type.equals(kClassScene) && !type.equals(kClassToon) && !type.equals(kClassFob))
			continue;
		int32 id = 0;
		int32 x = 0;
		int32 y = 0;
		int32 frame = 0;
		Common::String idText = _startupLayout.child(i, kXMLId);
		Common::String xText = _startupLayout.child(i, kXMLX);
		Common::String yText = _startupLayout.child(i, kXMLY);
		Common::String frameText = _startupLayout.child(i, kXMLStartFrame);
		if (StringParser::parseSignedInt(idText.c_str(), id, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
			StringParser::parseSignedInt(xText.c_str(), x, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
			StringParser::parseSignedInt(yText.c_str(), y, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
			(!frameText.empty() && StringParser::parseSignedInt(frameText.c_str(), frame, StringParser::kDecimalProfile) !=
				StringParser::Result::kSuccess))
			return false;
		int action = 0;
		if (type.equals(kClassFob)) {
			if (_startupLayout.child(i, kXMLClass).equals(kXMLPlay)) {
				action = 1;
				_productPlay = true;
			} else {
				action = kContentLink2000 + _links.size();
				_links.push_back(_startupLayout.child(i, kXMLClass).equals(kXMLLink) ? _startupLayout.child(i, kXMLTarget) : kXMLExternal);
				_linkParameters.push_back(_startupLayout.child(i, kXMLParameters));
			}
		}
		Shell::Sprite *sprite = _scene.add(Common::String::format(kAnimation, kProductName, id), 1, action, action != 0);
		if (!sprite) {
			if (type.equals(kClassScene) && _scene.setBackground(Common::String::format(kClassProductSignedIndexJpg, id)))
				continue;
			return false;
		}
		sprite->x = x;
		sprite->y = y;
		configureButton(*sprite);
		if (0 < frame)
			sprite->setFrame(frame);
	}
	_musicVolume = 100;
	Common::String music = _startupLayout.get(kPropertyStartupMusic);
	uint musicId = _defaultMusic;
	if (!music.empty()) {
		int32 parsedMusicId = 0;
		if (StringParser::parseSignedInt(music.c_str(), parsedMusicId, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
			parsedMusicId < 0)
			return false;
		musicId = static_cast<uint>(parsedMusicId);
	}
	selectMusic(musicId, kProductName);
	return true;
}

bool Application::showSignin() {
	_scene.clear();
	_textBlocks.clear();
	_links.clear();
	_startup = false;
	_content = false;
	_playCD->visible = false;
	Common::String branch(kSigninBranch);
	if (!_scene.setBackground(kBackground) || !add(_layout, branch + kBackgroundName, kSigninName, 0) ||
		!add(_layout, branch + kNameBoxName, kSigninName, 0))
		return false;
	_ok = add(_layout, branch + kOkName, kSigninName, 1);
	_newButton = add(_layout, branch + kNewNameName, kSigninName, 2);
	if (!_ok || !_newButton || !add(_layout, branch + kCancelName, kSigninName, 3) ||
		!add(_layout, branch + kUpArrowName, kSigninName, 4) || !add(_layout, branch + kDnArrowName, kSigninName, 5))
		return false;
	add(_layout, branch + kBackspaceName, kSigninName, 6);
	add(_layout, branch + kSpaceName, kSigninName, 7);
	for (char c = 'a'; c <= 'z'; c++) {
		if (!add(_layout, branch + Common::String(c), kSigninName, c))
			return false;
	}
	int32 x = 0;
	int32 y = 0;
	int32 width = 0;
	int32 height = 0;
	if (_layout.number(branch + kNameListXName, x) != StringParser::Result::kSuccess ||
		_layout.number(branch + kNameListYName, y) != StringParser::Result::kSuccess ||
		_layout.number(branch + kNameListWName, width) != StringParser::Result::kSuccess ||
		_layout.number(branch + kNameListHName, height) != StringParser::Result::kSuccess)
		return false;
	_listRect = Common::Rect(x, y, x + width, y + height);
	_newEntry = _users.empty();
	if (!_users.empty() && _selected < 0)
		select(0);
	if (_soundEnabled)
		_voice.play(*this, kEnterSpeech, Audio::Mixer::kSpeechSoundType);
	_musicVolume = 30;
	startMusic();
	return true;
}

void Application::resume() {
	_voice.stop();
	if (_gameSignin)
		showStartup();
	else
		showSignin();
}

void Application::select(int index) {
	if (index < 0 || static_cast<int>(_users.size()) <= index)
		return;
	_selected = index;
	_typed = _users[index].name;
	_newEntry = false;
	_first = MIN(_first, index);
	if (_first + 3 <= index)
		_first = index - 2;
}

void Application::matchName() {
	_selected = -1;
	for (uint i = 0; i < _users.size(); i++) {
		if (_users[i].name.hasPrefix(_typed)) {
			_selected = i;
			_first = MIN(_first, _selected);
			if (_first + 3 <= _selected)
				_first = _selected - 2;
			break;
		}
	}
}

void Application::addCharacter(char character) {
	if (_typed.size() >= 21 || (character == ' ' && (_typed.empty() || _typed.lastChar() == ' ')))
		return;
	if (_typed.empty() || _typed.lastChar() == ' ')
		character = toupper(static_cast<unsigned char>(character));
	_typed += character;
	matchName();
}

Common::String Application::selectedName() const {
	if (!_newEntry && 0 <= _selected)
		return _users[_selected].name;
	return _typed;
}

bool Application::valid() const {
	if (_typed.empty())
		return false;
	if (!_newEntry && 0 <= _selected)
		return true;
	if (_users.size() >= 99)
		return false;
	for (uint i = 0; i < _users.size(); i++) {
		if (_users[i].name.equals(_typed))
			return false;
	}
	return true;
}

bool Application::commitName() {
	if (!valid())
		return false;
	Common::String name = selectedName();
	for (uint i = 0; i < _users.size(); i++) {
		if (_users[i].name.equals(name))
			return true;
	}
	User user;
	user.id = _nextID;
	user.name = name;
	_nextID += 1;
	_users.push_back(user);
	if (!storeUsers()) {
		_users.pop_back();
		_nextID -= 1;
		return false;
	}
	Common::sort(_users.begin(), _users.end(), userLess);
	_newEntry = false;
	matchName();
	return true;
}

bool Application::removeName() {
	if (_selected < 0)
		return false;
	User previous = _users[_selected];
	_users.remove_at(_selected);
	if (!storeUsers()) {
		_users.push_back(previous);
		Common::sort(_users.begin(), _users.end(), userLess);
		return false;
	}
	_selected = -1;
	_first = 0;
	_typed.clear();
	_newEntry = _users.empty();
	if (!_users.empty())
		select(0);
	return true;
}

void Application::updateCursor(bool overButton) {
	if (_overButton == overButton)
		return;
	_overButton = overButton;
	if (overButton && _hoverCursor && !_hoverCursor->cursors.empty())
		CursorMan.replaceCursor(_hoverCursor->cursors[0].cursor);
	else
		_vm.setDefaultCursor();
}

Application::Request Application::event(const Common::Event &event) {
	if (event.type == Common::EVENT_MOUSEMOVE || event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP) {
		bool overButton = _confirmDelete ? _dialogScene.hitTest(event.mouse) != 0 : (_frameScene.hitTest(event.mouse) != 0 || _scene.hitTest(event.mouse) != 0);
		updateCursor(overButton);
	}
	if (event.type == Common::EVENT_MOUSEMOVE)
		debug(5, "Trinket launcher pointer: %d,%d; content=%d frame=%d", event.mouse.x, event.mouse.y,
			  _scene.hitTest(event.mouse), _frameScene.hitTest(event.mouse));
	if (!_confirmDelete) {
		int frameAction = _frameScene.event(event);
		if (_soundEnabled && _frameScene.pressFeedback())
			_click.play(*this, kClickSound, Audio::Mixer::kSFXSoundType);
		if (frameAction == 1000)
			return kQuit05;
		if (frameAction == 1001 || frameAction == 1002) {
			uint vertical = _vertTab->getFrame();
			uint horizontal = _horizTab->getFrame();
			bool horizVisible = _horizTab->visible;
			if (frameAction == 1001) {
				_musicEnabled = !_musicEnabled;
				// The original volume toggle does not rewind the looping ditty.
				_musicVolume = 100;
				startMusic();
			} else {
				_soundEnabled = !_soundEnabled;
				_contentMovie.setVolume(_soundEnabled ? 255 : 0);
				if (!_soundEnabled) {
					_voice.stop();
					_click.stop();
				}
			}
			if (!buildFrame())
				error("Could not restore launcher controls");
			_vertTab->setFrame(vertical ? vertical : 1);
			_horizTab->setFrame(horizontal ? horizontal : 1);
			_horizTab->visible = horizVisible;
			if (!storeUsers())
				warning("Trinket: could not save launcher audio preferences");
		} else if (frameAction == 1003 || frameAction == 1005) {
			showContent(_homeLayout, frameAction == 1003 ? kLauncherStartupScreenName : kLauncherStartupHelpName);
			_vertTab->setFrame(frameAction == 1003 ? 2 : 4);
			_horizTab->setFrame(1);
			_horizTab->visible = false;
		} else if (frameAction == 1004) {
			showContent(_newsLayout, kLauncherContentWhatsNewName);
			_vertTab->setFrame(3);
			_horizTab->visible = false;
		} else if (frameAction == 1006) {
			showStartup();
		} else if (1010 <= frameAction && frameAction < 1016) {
			static constexpr const char *destinations[] = {
				kRrBrandName,
				kSfBrandName,
				kCfBrandName,
				kZoBrandName,
				kOtBrandName,
				kCsBrandName,
			};
			showContent(_parentsLayout, Common::String(kLauncherContentName) + destinations[frameAction - 1010]);
			_vertTab->setFrame(1);
			_horizTab->setFrame(frameAction - 1010 + 2);
			_horizTab->visible = true;
		}
		if (frameAction)
			return kNone00;
	}
	int action = _confirmDelete ? _dialogScene.event(event) : _scene.event(event);
	if (!action && _content && 0 <= _world && event.type == Common::EVENT_LBUTTONUP && Common::Rect(50, 50, 650, 400).contains(event.mouse))
		action = kWorldMovie1902;
	if (!_confirmDelete && _soundEnabled && _scene.pressFeedback())
		_click.play(*this, kClickSound, Audio::Mixer::kSFXSoundType);
	Common::KeyCode key = event.type == Common::EVENT_KEYDOWN ? event.kbd.keycode : Common::KEYCODE_INVALID;
	if (event.type == Common::EVENT_KEYDOWN && 32 <= event.kbd.ascii && event.kbd.ascii < 127 &&
		!(event.kbd.flags & (Common::KBD_CTRL | Common::KBD_ALT)))
		key = Common::KEYCODE_INVALID;
	if (_confirmDelete) {
		if (action == 1 || key == Common::KEYCODE_RETURN) {
			_confirmDelete = false;
			_dialogScene.clear();
			return kDelete02;
		}
		if (action == 2 || key == Common::KEYCODE_ESCAPE) {
			_confirmDelete = false;
			_dialogScene.clear();
		}
		return kNone00;
	}
	if (_content) {
		if (action == kWorldMovie1902 && 0 <= _world) {
			if (_contentMovie.isVideoLoaded())
				stopContentMovie();
			else
				playContentMovie(kWorldMovies[_world], _moviePosition.x, _moviePosition.y);
		} else if (action == kGridPrevious1900 || action == kGridNext1901) {
			if (action == kGridPrevious1900 && 0 < _gridPage)
				_gridPage -= 1;
			else if (action == kGridNext1901)
				_gridPage += 1;
			Common::String branch = _contentBranch;
			showContent(*_contentLayout, branch);
		} else if (kContentLink2000 <= action && action - kContentLink2000 < static_cast<int>(_links.size())) {
			Common::String link = _links[action - kContentLink2000];
			Common::String parameters = _linkParameters[action - kContentLink2000];
			followLink(link, parameters);
		} else if (key == Common::KEYCODE_ESCAPE) {
			showStartup();
		}
		return kNone00;
	}
	if (_startup) {
		if (action == 1 || (_productPlay && key == Common::KEYCODE_RETURN)) {
			if (_gameSignin)
				return kStartGame06;
			if (!showSignin())
				warning("Trinket: could not load the sign-in screen");
		} else if (action == 4 || key == Common::KEYCODE_ESCAPE) {
			showContent(_homeLayout, kLauncherStartupScreenName);
			_vertTab->setFrame(2);
		} else if (kContentLink2000 <= action && action - kContentLink2000 < static_cast<int>(_links.size())) {
			Common::String link = _links[action - kContentLink2000];
			Common::String parameters = _linkParameters[action - kContentLink2000];
			followLink(link, parameters);
		}
		return kNone00;
	}
	if (event.type == Common::EVENT_LBUTTONUP && _listRect.contains(event.mouse)) {
		int descent = _listFont->getFontHeight() - _listFont->getFontAscent();
		int row = (event.mouse.y - _listRect.top - descent) / _listFont->getFontHeight();
		if (0 <= row && row < 3)
			select(_first + row);
		return kNone00;
	}
	if (action == 1 || key == Common::KEYCODE_RETURN)
		return valid() ? kPlay01 : kNone00;
	if (action == 2) {
		if (_users.size() < 99) {
			_typed.clear();
			_selected = -1;
			_newEntry = true;
		}
	} else if (action == 3 || key == Common::KEYCODE_ESCAPE) {
		showStartup();
	} else if (action == 4 || key == Common::KEYCODE_UP) {
		select(_selected - 1);
	} else if (action == 5 || key == Common::KEYCODE_DOWN) {
		select(_selected + 1);
	} else if (key == Common::KEYCODE_PAGEUP) {
		select(MAX(0, _selected - 3));
	} else if (key == Common::KEYCODE_PAGEDOWN) {
		select(MIN<int>(_users.size() - 1, _selected + 3));
	} else if (key == Common::KEYCODE_r && (event.kbd.flags & Common::KBD_CTRL) && 0 <= _selected) {
		showDeleteDialog();
	} else if (key == Common::KEYCODE_F7 && 0 <= _selected) {
		return kNewGame03;
	} else if (Common::KEYCODE_F1 <= key && key <= Common::KEYCODE_F6 && 0 <= _selected) {
		_slot = key - Common::KEYCODE_F1 + 1;
		return kSlot04;
	} else if (action == 6 || key == Common::KEYCODE_BACKSPACE || key == Common::KEYCODE_DELETE) {
		if (!_typed.empty())
			_typed.deleteLastChar();
		matchName();
	} else if (action == 7) {
		addCharacter(' ');
	} else if ('a' <= action && action <= 'z') {
		addCharacter(action);
	} else if (event.type == Common::EVENT_KEYDOWN && !(event.kbd.flags & (Common::KBD_CTRL | Common::KBD_ALT))) {
		char c = event.kbd.ascii;
		if (Common::isAlnum(c) || c == ' ' || c == '-' || c == '.')
			addCharacter(c);
	}
	return kNone00;
}

void Application::showDeleteDialog() {
	_dialogScene.clear();
	Shell::Sprite *box = _dialogScene.add(Common::String::format(kAnimation, kCommonName, 1042), 0);
	Shell::Sprite *yes = _dialogScene.add(Common::String::format(kAnimation, kCommonName, 1045), 1, 1, true);
	Shell::Sprite *no = _dialogScene.add(Common::String::format(kAnimation, kCommonName, 1046), 1, 2, true);
	if (!box || !yes || !no) {
		_dialogScene.clear();
		warning("Trinket: failed to load the launcher delete dialog");
		return;
	}
	configureButton(*yes);
	configureButton(*no);
	box->x = 257;
	box->y = 205;
	int spacer = (286 - 70 - no->width()) / 3;
	yes->x = 257 + spacer;
	yes->y = 342;
	no->x = 257 + spacer * 2 + 70;
	no->y = 342;
	_confirmDelete = true;
	if (_soundEnabled)
		_voice.play(*this, kRemoveSpeech, Audio::Mixer::kSpeechSoundType);
}

uint32 Application::listColor(const OM::Canvas &canvas, const char *name) const {
	Common::String path = Common::String(kSigninBranch) + kNameListName + name;
	int32 red = 0;
	int32 green = 0;
	int32 blue = 0;
	if (_layout.number(path + kRName, red) != StringParser::Result::kSuccess ||
		_layout.number(path + kGName, green) != StringParser::Result::kSuccess ||
		_layout.number(path + kBName, blue) != StringParser::Result::kSuccess) {
		warning("Trinket: invalid launcher list color at %s", path.c_str());
		return canvas.surface().format.RGBToColor(0, 0, 0);
	}
	return canvas.surface().format.RGBToColor(red >> 8, green >> 8, blue >> 8);
}

void Application::draw(OM::Canvas &canvas) {
	if (_contentMovie.isVideoLoaded()) {
		if (_contentMovie.endOfVideo())
			stopContentMovie();
		else if (_contentMovie.needsUpdate()) {
			const Graphics::Surface *frame = _contentMovie.decodeNextFrame();
			if (frame) {
				Common::ScopedPtr<Graphics::Surface> converted(frame->convertTo(canvas.surface().format));
				_movieFrame.free();
				_movieFrame.copyFrom(*converted);
				converted->free();
			}
		}
	}
	if (_ok)
		_ok->enabled = valid();
	if (_newButton)
		_newButton->enabled = _users.size() < 99 && !_newEntry;
	_scene.drawBackground(canvas);
	_frameScene.drawSprites(canvas, INT_MIN, 0);
	_scene.drawSprites(canvas, INT_MIN, INT_MAX);
	_frameScene.drawSprites(canvas, 1, INT_MAX);
	if (_content)
		drawContentText(canvas);
	if (_movieFrame.getPixels())
		canvas.blit(_movieFrame, _moviePosition.x, _moviePosition.y);
	if (!_startup) {
		Graphics::Surface list = canvas.surface().getSubArea(_listRect);
		list.fillRect(Common::Rect(list.w, list.h), listColor(canvas, kBcName));
		int lineHeight = _listFont->getFontHeight();
		int descent = lineHeight - _listFont->getFontAscent();
		Common::Array<Common::String> names;
		for (uint i = 0; i < _users.size(); i++)
			names.push_back(_users[i].name);
		int selected = _selected;
		int first = _first;
		if (_newEntry || selected < 0) {
			selected = 0;
			while (selected < static_cast<int>(names.size()) && names[selected] < _typed)
				selected += 1;
			names.insert_at(selected, _typed);
			first = MIN(first, selected);
			if (first + 3 <= selected)
				first = selected - 2;
		}
		for (int row = 0; row < 3; row++) {
			int index = first + row;
			if (static_cast<int>(names.size()) <= index)
				break;
			int y = descent + row * lineHeight;
			uint32 color = listColor(canvas, index == selected ? kStcName : kNtcName);
			if (index == selected)
				list.fillRect(Common::Rect(0, y, list.w, MIN<int>(list.h, y + lineHeight)), listColor(canvas, kHbcName));
			bool matched = index == selected && !_newEntry && _selected >= 0 && !_typed.equals(names[index]);
			if (matched)
				color = listColor(canvas, kMtcName);
			_listFont->drawString(&list, names[index], 10, y, list.w - 10, color, Graphics::kTextAlignLeft, 0, false);
			if (matched)
				_listFont->drawString(&list, _typed, 10, y, list.w - 10, listColor(canvas, kStcName), Graphics::kTextAlignLeft, 0, false);
		}
	}
	if (_confirmDelete) {
		_dialogScene.drawSprites(canvas, INT_MIN, INT_MAX);
		Graphics::Surface textArea = canvas.surface().getSubArea(Common::Rect(277, 218, 522, 336));
		Common::Array<Common::String> lines;
		if (!Shell::Text::wrap(*_dialogFont, _layout.get(Common::String(kSigninBranch) + kPropertyDialogsDeleteMsg), textArea.w,
							   lines, Shell::Text::kLauncher))
			return;
		int count = lines.size();
		int leading = _dialogFont->getFontHeight() - _dialogFont->getFontAscent() - _dialogFont->getFontDescent();
		int baseline = Shell::Text::firstBaseline(textArea.h, count, _dialogFont->getFontAscent(), _dialogFont->getFontDescent(), leading);
		for (int i = 0; i < count; i++) {
			_dialogFont->drawString(&textArea, lines[i], 0, baseline - _dialogFont->getFontAscent(), textArea.w,
									canvas.surface().format.RGBToColor(57, 12, 107), Graphics::kTextAlignLeft, 0, false, true);
			baseline += _dialogFont->getFontHeight();
		}
	}
}

void Application::drawContentText(OM::Canvas &canvas) {
	for (uint i = 0; i < _textBlocks.size(); i++) {
		const TextBlock &block = _textBlocks[i];
		Graphics::Surface area = canvas.surface().getSubArea(block.rect);
		uint32 color = area.format.RGBToColor(block.red, block.green, block.blue);
		for (uint line = 0; line < block.lines.size() && line * block.font->getFontHeight() < static_cast<uint>(area.h); line++)
			block.font->drawString(&area, block.lines[line], 0, line * block.font->getFontHeight(), area.w, color,
								   block.centered ? Graphics::kTextAlignCenter : Graphics::kTextAlignLeft, 0, false, true);
	}
}

bool Application::addContentText(const OM::XMLParser &xml, uint node) {
	TextBlock block;
	int32 x = 0;
	int32 y = 0;
	int32 width = 0;
	int32 height = 0;
	int32 red = 0;
	int32 green = 0;
	int32 blue = 0;
	if (StringParser::parseSignedInt(xml.child(node, kXMLX).c_str(), x, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
		StringParser::parseSignedInt(xml.child(node, kXMLY).c_str(), y, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
		StringParser::parseSignedInt(xml.child(node, kXMLW).c_str(), width, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
		StringParser::parseSignedInt(xml.child(node, kXMLH).c_str(), height, StringParser::kDecimalProfile) != StringParser::Result::kSuccess)
		return false;
	block.rect = Common::Rect(x, y, x + width, y + height);
	block.rect.clip(Common::Rect(800, 600));
	if (block.rect.isEmpty())
		return false;
	block.font = _fonts.load(kCommonLibraryPrefix + xml.child(node, kXMLFont) + kFontExtension);
	if (!block.font)
		return false;
	Common::String text = xml.child(node, kXMLContent);
	if (text.size() >= 2 && text.firstChar() == '"' && text.lastChar() == '"')
		text = text.substr(1, text.size() - 2);
	if (!Shell::Text::wrap(*block.font, text, block.rect.width(), block.lines, Shell::Text::kLauncher))
		return false;
	if (StringParser::parseSignedInt(xml.child(node, kXMLColorR).c_str(), red, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
		StringParser::parseSignedInt(xml.child(node, kXMLColorG).c_str(), green, StringParser::kDecimalProfile) != StringParser::Result::kSuccess ||
		StringParser::parseSignedInt(xml.child(node, kXMLColorB).c_str(), blue, StringParser::kDecimalProfile) != StringParser::Result::kSuccess)
		return false;
	block.red = red >> 8;
	block.green = green >> 8;
	block.blue = blue >> 8;
	block.centered = xml.child(node, kXMLCenter).equals(kXMLTrue);
	_textBlocks.push_back(block);
	return true;
}

} // namespace Launcher
} // namespace Trinket
