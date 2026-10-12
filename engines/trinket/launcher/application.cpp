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

namespace Trinket {
namespace Launcher {

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

Application::Application(Common::SaveFileManager &manager, const Common::String &target, Audio::Mixer &mixer) : _manager(manager), _saveName(target + ".trinket-launcher-users.xml"),
																												_frameScene(*this), _scene(*this), _dialogScene(*this), _voice(mixer), _music(mixer), _click(mixer) {
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
	delete _arrowCursor;
	_arrowCursor = Graphics::makeDefaultWinCursor();
	delete _hoverCursor;
	_hoverCursor = nullptr;
	Common::PEResources executable;
	if (executable.loadFromEXE(Common::FSNode(root.join(kCursorExecutable)).createReadStream()))
		_hoverCursor = Graphics::WinCursorGroup::createCursorGroup(&executable, 1);
	_pointerOverButton = false;
	CursorMan.replaceCursor(_arrowCursor);
	OM::XMLParser startup;
	if (!startup.loadFile(Common::FSNode(root.join(kStartupXML))) || !startup.parse())
		return false;
	_defaultMusic = startup.number("startup/music");
	if (!_common.load(root.join(kCommon)) || !_signin.load(root.join(kSignin)) ||
		!_brands.load(root.join(kBrands)) || !_parents.load(root.join(kParents)) || !_audio.load(root.join(kAudio)) ||
		!_product.load(root.join(Common::String::format(kProductRSC, product)))) {
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
	_gameSignin = _startupLayout.get("startup/signin/execute") == "no";
	for (uint i = 0; i < ARRAYSIZE(kDemoXML); i++) {
		if (!_worldLibraries[i].load(root.join(kWorldLibraries[i])))
			return false;
		if (!_demoLayouts[i].loadFile(Common::FSNode(root.join("rsc").join(kDemoXML[i]))) || !_demoLayouts[i].parse())
			return false;
	}
	if (!_news.load(root.join(kNews)))
		return false;
	Common::ScopedPtr<Common::SeekableReadStream> news(_news.open("LXML", 10002));
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
	Common::ScopedPtr<Common::SeekableReadStream> fontStream(open(kDialogFont));
	_dialogFont.reset(fontStream ? OM::NFNTFont::load(*fontStream) : nullptr);
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
	uint id = atoi(name.c_str() + slash + 1);
	const char *tag = nullptr;
	if (name.hasSuffix(".ao"))
		tag = "ASEQ";
	else if (name.hasSuffix(".rgb"))
		tag = "RRGB";
	else if (name.hasSuffix(".jpg"))
		tag = "JPEG";
	else if (name.hasSuffix(".wav"))
		tag = "WAVE";
	else if (name.hasSuffix(".nfnt"))
		tag = "NFNT";
	if (!tag || !id)
		return nullptr;
	if (name.hasPrefix("signin/"))
		return _signin.open(tag, id);
	if (name.hasPrefix("product/")) {
		Common::SeekableReadStream *stream = _product.open(tag, id);
		if (stream)
			return stream;
	}
	Common::SeekableReadStream *stream = _common.open(tag, id);
	if (!stream)
		stream = _brands.open(tag, id);
	if (!stream)
		stream = _parents.open(tag, id);
	if (!stream)
		stream = _audio.open(tag, id);
	if (!stream)
		stream = _news.open(tag, id);
	for (uint i = 0; !stream && i < ARRAYSIZE(_worldLibraries); i++)
		stream = _worldLibraries[i].open(tag, id);
	return stream;
}

void Application::configureButton(Shell::Sprite &sprite) {
	// Launcher buttons use idle/click/hover/disabled logical frames 1/2/3/4.
	sprite.flyButton = sprite.button;
	sprite.hoverFrame = sprite.asset->pictureForFrame(2) < 0 ? 1 : 2;
	sprite.disabledFrame = 3;
}

Shell::Sprite *Application::addFrameObject(Shell::Scene &scene, int id, int x, int y, int action) {
	Shell::Sprite *sprite = scene.add(Common::String::format(kAnimation, "common", id), action ? 1 : 0, action, action != 0);
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
		{3005, 406, 5}};
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
	_verticalTab = addFrameObject(_frameScene, 3006, 665, 61);
	_horizontalTab = addFrameObject(_frameScene, 3007, 8, 443);
	_playCD = addFrameObject(_frameScene, 3051, 685, 15, 1006);
	if (!_verticalTab || !_horizontalTab || !_playCD)
		return false;
	_playCD->visible = _content;
	_horizontalTab->visible = false;
	Common::String branch = "launcher/cBar/cmdButtons/brandOrder/id";
	const Common::Array<OM::XMLParser::Element> &elements = _parentsLayout.elements();
	int index = 0;
	for (uint i = 0; i < elements.size(); i++) {
		if (elements[i].path != branch)
			continue;
		Common::String button = "launcher/cBar/cmdButtons/" + elements[i].value + "/cb";
		int x = _parentsLayout.number("launcher/cBar/cmdButtonArea/x") + 115 * index;
		int y = _parentsLayout.number("launcher/cBar/cmdButtonArea/y");
		if (!addFrameObject(_frameScene, _parentsLayout.number(button + "/id"), x, y, 1010 + index))
			return false;
		index += 1;
	}
	if (index != 6)
		warning("Trinket: launcher brand order contains %d entries", index);
	return index == 6;
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
		if (_musicPath != path) {
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
		found |= xml.elements()[i].path == branch;
	if (!found) {
		warning("Trinket: missing launcher content %s", branch.c_str());
		return false;
	}
	_previousLayout = _contentLayout;
	_previousBranch = _contentBranch;
	if (_contentLayout != &xml || _contentBranch != branch)
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
	uint music = branch == "launcher/startup/screen" ? xml.number("launcher/music") : 0;
	for (uint i = 0; i < elements.size(); i++) {
		if (elements[i].parent < 0 || elements[elements[i].parent].path != branch)
			continue;
		Common::String condition = xml.child(i, "condition");
		if (condition == "fileMac" || condition == "noFileMac" || condition == "install")
			continue;
		if (condition == "file" || condition == "fileWin" || condition == "noFile" || condition == "noFileWin") {
			bool exists = Common::FSNode(_root.getParent().getParent().join(kMovieDirectory).join(xml.child(i, "filename"))).exists();
			if (exists == condition.hasPrefix("noFile"))
				continue;
		}
		Common::String type = xml.child(i, "type");
		if (type == "music") {
			music = atoi(xml.child(i, "id").c_str());
		} else if (type == "text") {
			if (!addContentText(xml, i))
				return false;
		} else if (type == "toon" || type == "fob") {
			int action = 0;
			if (type == "fob") {
				action = kContentLink2000 + _links.size();
				_links.push_back(xml.child(i, "target"));
				_linkParameters.push_back(xml.child(i, "parameters"));
			}
			Shell::Sprite *sprite = addFrameObject(_scene, atoi(xml.child(i, "id").c_str()),
												   atoi(xml.child(i, "x").c_str()), atoi(xml.child(i, "y").c_str()), action);
			if (!sprite)
				return false;
			if (xml.child(i, "class") == "checkbox") {
				bool checked = (xml.child(i, "checkedIf/value") == "auto") == _updatesAutomatic;
				// The selected option is disabled and displays its checked frame.
				sprite->enabled = !checked;
				sprite->disabledFrame = 4;
			}
			if (type == "toon") {
				int frame = atoi(xml.child(i, "startFrame").c_str());
				if (0 < frame)
					sprite->setFrame(frame);
			}
		}
	}
	for (uint i = 0; i < elements.size(); i++) {
		if (0 <= elements[i].parent && elements[elements[i].parent].path == branch && xml.child(i, "type") == "initScript") {
			if (!addContentScript(xml, xml.child(i, "target")))
				return false;
		}
	}
	_musicVolume = 100;
	selectMusic(music, "common");
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
		if (elements[i].path.hasPrefix("launcher/cBar/cmdButtons/") && elements[i].path.hasSuffix("/cb"))
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
		_links.push_back("xml:" + xml.child(node, "xmlFile"));
		_linkParameters.push_back(xml.child(node, "dest"));
		if (!addFrameObject(_scene, atoi(xml.child(node, "id").c_str()), positions[count - 1][i * 2] - 20,
							positions[count - 1][i * 2 + 1], action))
			return false;
	}
	if (6 < buttons.size()) {
		Shell::Sprite *left = addFrameObject(_scene, xml.number("launcher/cBar/left/id"),
											 xml.number("launcher/cBar/left/x"), xml.number("launcher/cBar/left/y"), kGridPrevious1900);
		Shell::Sprite *right = addFrameObject(_scene, xml.number("launcher/cBar/right/id"),
											  xml.number("launcher/cBar/right/x"), xml.number("launcher/cBar/right/y"), kGridNext1901);
		if (!left || !right)
			return false;
		left->visible = 0 < first;
		right->visible = first + count < buttons.size();
	}
	return true;
}

bool Application::addContentScript(const OM::XMLParser &xml, const Common::String &script) {
	if (script.hasSuffix("Brand")) {
		// Demo sparkles only appear after a new download; the local catalogue starts clear.
		return true;
	}
	if (script.hasSuffix("Demos"))
		return addContentGrid(xml);
	if (script == "initMyCDRROMs") {
		// Disc-only product packs have no installed-product icon.
		Common::ScopedPtr<Common::SeekableReadStream> icon(_product.open("ASEQ", 2051));
		if (!icon)
			return true;
		int action = kContentLink2000 + _links.size();
		_links.push_back("product:startup");
		_linkParameters.push_back("");
		Shell::Sprite *button = _scene.add(Common::String::format(kAnimation, "product", 2051), 1, action, true);
		if (!button)
			return false;
		button->x = 290;
		button->y = 195;
		configureButton(*button);
		return true;
	}
	if (script == "initUpdates") {
		TextBlock block;
		Common::ScopedPtr<Common::SeekableReadStream> stream(open(kDialogFont));
		block.font.reset(stream ? OM::NFNTFont::load(*stream) : nullptr);
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
		"initRRWorld",
		"initSFWorld",
		"initCFWorld",
		"initZOWorld",
		"initOTWorld",
		"initCSWorld",
	};
	for (uint i = 0; i < ARRAYSIZE(worlds); i++) {
		if (script == worlds[i])
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
		{0, 24051, 528, 293, 3}, {0, 24059, 453, 254, 2}, {1, 25051, 605, 245, 3}, {1, 25055, 70, 167, 1}, {1, 25060, 557, 143, 2}, {1, 25067, 565, 52, 1}, {2, 26051, 488, 149, 1}, {2, 26058, 87, 131, 1}, {2, 26064, 551, 167, 2}, {2, 26069, 49, 184, 2}, {3, 27051, 276, 372, 1}, {3, 27052, 558, 378, 1}, {3, 27054, 630, 341, 1}, {3, 27055, 117, 381, 1}, {4, 29051, 56, 98, 3}, {4, 29053, 561, 140, 1}, {4, 29054, 599, 158, 2}, {4, 29055, 547, 199, 2}};
	static constexpr int screens[6][6] = {
		{24100, 383, 409, 15, 114, 84}, {25100, 49, 251, 9, 246, 149}, {26100, 310, 68, 37, 200, 59}, {27100, 121, 215, 31, 271, 69}, {29100, 63, 47, 4, 175, 105}, {28100, 57, 402, 20, 98, 88}};
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
	if (link.hasPrefix("xml:")) {
		Common::String file = link.substr(4);
		if (file.equalsIgnoreCase("TLCParents.xml"))
			showContent(_parentsLayout, parameters);
		else if (file.equalsIgnoreCase("TLCHome.xml"))
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
	if (link == "rscript:homeButtHandler")
		showContent(_homeLayout, "launcher/startup/screen");
	else if (link == "rscript:initParentsArea")
		showContent(_parentsLayout, "launcher/content/mainmenu");
	else if (link == "rscript:initProductGrid")
		showContent(_parentsLayout, "launcher/content/productGrid3to7");
	else if (link == "rscript:setUpdatesAuto" || link == "rscript:setUpdatesManual") {
		_updatesAutomatic = link == "rscript:setUpdatesAuto";
		if (!storeUsers())
			warning("Trinket: could not save launcher update preference");
		Common::String branch = _contentBranch;
		showContent(*_contentLayout, branch);
	} else if (link == "rscript:goBack" && _previousLayout) {
		Common::String branch = _previousBranch;
		showContent(*_previousLayout, branch);
	} else if (link.hasPrefix("startup:"))
		showProductScreen(link.substr(8));
	else if (link == "product:startup")
		showStartup();
	else if (link == "rscript:stopMovie")
		stopContentMovie();
	else if (link.hasPrefix("bink:"))
		playContentMovie(link.substr(5), atoi(parameters.c_str()), 75);
	else if (link.hasPrefix("launcher/startup/"))
		showContent(_homeLayout, link);
	else if (link.hasPrefix("launcher/content/"))
		showContent(_contentLayout ? *_contentLayout : _parentsLayout, link);
	else
		notifyNotImplemented();
}

Shell::Sprite *Application::add(const OM::XMLParser &xml, const Common::String &branch, const char *library, int action) {
	int id = xml.number(branch + "/id");
	if (id < 1)
		return nullptr;
	Shell::Sprite *sprite = _scene.add(Common::String::format(kAnimation, library, id), 1, action, action != 0);
	if (sprite) {
		sprite->x = xml.number(branch + "/x");
		sprite->y = xml.number(branch + "/y");
		configureButton(*sprite);
	}
	return sprite;
}

bool Application::showStartup() {
	return showProductScreen("startup/screen");
}

bool Application::showProductScreen(const Common::String &branch) {
	bool found = false;
	for (const OM::XMLParser::Element &element : _startupLayout.elements())
		found |= element.path == branch;
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
	_verticalTab->setFrame(1);
	_horizontalTab->setFrame(1);
	_horizontalTab->visible = false;
	_ok = _newButton = nullptr;
	_startup = true;
	_productPlay = false;
	if (!_scene.setBackground(kBackground))
		return false;
	const auto &elements = _startupLayout.elements();
	for (uint i = 0; i < elements.size(); i++) {
		if (elements[i].parent < 0 || elements[elements[i].parent].path != branch)
			continue;
		Common::String condition = _startupLayout.child(i, "condition");
		if (!condition.empty() && condition != "all")
			continue;
		Common::String type = _startupLayout.child(i, "type");
		if (type != "scene" && type != "toon" && type != "fob")
			continue;
		int id = atoi(_startupLayout.child(i, "id").c_str());
		int action = 0;
		if (type == "fob") {
			if (_startupLayout.child(i, "class") == "play") {
				action = 1;
				_productPlay = true;
			} else {
				action = kContentLink2000 + _links.size();
				_links.push_back(_startupLayout.child(i, "class") == "link" ? _startupLayout.child(i, "target") : "external:");
				_linkParameters.push_back(_startupLayout.child(i, "parameters"));
			}
		}
		Shell::Sprite *sprite = _scene.add(Common::String::format(kAnimation, "product", id), 1, action, action != 0);
		if (!sprite) {
			if (type == "scene" && _scene.setBackground(Common::String::format("product/%d.jpg", id)))
				continue;
			return false;
		}
		sprite->x = atoi(_startupLayout.child(i, "x").c_str());
		sprite->y = atoi(_startupLayout.child(i, "y").c_str());
		configureButton(*sprite);
		int frame = atoi(_startupLayout.child(i, "startFrame").c_str());
		if (0 < frame)
			sprite->setFrame(frame);
	}
	_musicVolume = 100;
	Common::String music = _startupLayout.get("startup/music");
	selectMusic(music.empty() ? _defaultMusic : atoi(music.c_str()), "product");
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
	if (!_scene.setBackground(kBackground) || !add(_layout, branch + "background", "signin", 0) ||
		!add(_layout, branch + "nameBox", "signin", 0))
		return false;
	_ok = add(_layout, branch + "ok", "signin", 1);
	_newButton = add(_layout, branch + "newName", "signin", 2);
	if (!_ok || !_newButton || !add(_layout, branch + "cancel", "signin", 3) ||
		!add(_layout, branch + "upArrow", "signin", 4) || !add(_layout, branch + "dnArrow", "signin", 5))
		return false;
	add(_layout, branch + "backspace", "signin", 6);
	add(_layout, branch + "space", "signin", 7);
	for (char c = 'a'; c <= 'z'; c++) {
		if (!add(_layout, branch + Common::String(c), "signin", c))
			return false;
	}
	int x = _layout.number(branch + "nameList/x"), y = _layout.number(branch + "nameList/y");
	_listRect = Common::Rect(x, y, x + _layout.number(branch + "nameList/w"), y + _layout.number(branch + "nameList/h"));
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
		if (_users[i].name == _typed)
			return false;
	}
	return true;
}

bool Application::commitName() {
	if (!valid())
		return false;
	Common::String name = selectedName();
	for (uint i = 0; i < _users.size(); i++) {
		if (_users[i].name == name)
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
	if (_pointerOverButton == overButton)
		return;
	_pointerOverButton = overButton;
	if (overButton && _hoverCursor && !_hoverCursor->cursors.empty())
		CursorMan.replaceCursor(_hoverCursor->cursors[0].cursor);
	else if (_arrowCursor)
		CursorMan.replaceCursor(_arrowCursor);
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
			uint vertical = _verticalTab->getFrame(), horizontal = _horizontalTab->getFrame();
			bool horizontalVisible = _horizontalTab->visible;
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
			_verticalTab->setFrame(vertical ? vertical : 1);
			_horizontalTab->setFrame(horizontal ? horizontal : 1);
			_horizontalTab->visible = horizontalVisible;
			if (!storeUsers())
				warning("Trinket: could not save launcher audio preferences");
		} else if (frameAction == 1003 || frameAction == 1005) {
			showContent(_homeLayout, frameAction == 1003 ? "launcher/startup/screen" : "launcher/startup/help");
			_verticalTab->setFrame(frameAction == 1003 ? 2 : 4);
			_horizontalTab->setFrame(1);
			_horizontalTab->visible = false;
		} else if (frameAction == 1004) {
			showContent(_newsLayout, "launcher/content/whatsNew");
			_verticalTab->setFrame(3);
			_horizontalTab->visible = false;
		} else if (frameAction == 1006) {
			showStartup();
		} else if (1010 <= frameAction && frameAction < 1016) {
			static const char *const destinations[] = {
				"rrBrand",
				"sfBrand",
				"cfBrand",
				"zoBrand",
				"otBrand",
				"csBrand",
			};
			showContent(_parentsLayout, Common::String("launcher/content/") + destinations[frameAction - 1010]);
			_verticalTab->setFrame(1);
			_horizontalTab->setFrame(frameAction - 1010 + 2);
			_horizontalTab->visible = true;
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
			showContent(_homeLayout, "launcher/startup/screen");
			_verticalTab->setFrame(2);
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
	Shell::Sprite *box = _dialogScene.add(Common::String::format(kAnimation, "common", 1042), 0);
	Shell::Sprite *yes = _dialogScene.add(Common::String::format(kAnimation, "common", 1045), 1, 1, true);
	Shell::Sprite *no = _dialogScene.add(Common::String::format(kAnimation, "common", 1046), 1, 2, true);
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
	Common::String path = Common::String(kSigninBranch) + "nameList/" + name;
	return canvas.surface().format.RGBToColor(_layout.number(path + "/r") >> 8,
											  _layout.number(path + "/g") >> 8, _layout.number(path + "/b") >> 8);
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
		list.fillRect(Common::Rect(list.w, list.h), listColor(canvas, "bc"));
		int lineHeight = _listFont->getFontHeight();
		int descent = lineHeight - _listFont->getFontAscent();
		Common::Array<Common::String> names;
		for (uint i = 0; i < _users.size(); i++)
			names.push_back(_users[i].name);
		int selected = _selected, first = _first;
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
			uint32 color = listColor(canvas, index == selected ? "stc" : "ntc");
			if (index == selected)
				list.fillRect(Common::Rect(0, y, list.w, MIN<int>(list.h, y + lineHeight)), listColor(canvas, "hbc"));
			bool matched = index == selected && !_newEntry && _selected >= 0 && _typed != names[index];
			if (matched)
				color = listColor(canvas, "mtc");
			_listFont->drawString(&list, names[index], 10, y, list.w - 10, color, Graphics::kTextAlignLeft, 0, false);
			if (matched)
				_listFont->drawString(&list, _typed, 10, y, list.w - 10, listColor(canvas, "stc"), Graphics::kTextAlignLeft, 0, false);
		}
	}
	if (_confirmDelete) {
		_dialogScene.drawSprites(canvas, INT_MIN, INT_MAX);
		Graphics::Surface textArea = canvas.surface().getSubArea(Common::Rect(277, 218, 522, 336));
		Common::Array<Common::String> lines;
		if (!Shell::Text::wrap(*_dialogFont, _layout.get(Common::String(kSigninBranch) + "dialogs/delete/msg"), textArea.w,
							   lines, Shell::Text::kLauncher))
			return;
		int count = lines.size();
		int leading = _dialogFont->getFontHeight() - _dialogFont->ascent() - _dialogFont->descent();
		int baseline = Shell::Text::firstBaseline(textArea.h, count, _dialogFont->ascent(), _dialogFont->descent(), leading);
		for (int i = 0; i < count; i++) {
			_dialogFont->drawString(&textArea, lines[i], 0, baseline - _dialogFont->ascent(), textArea.w,
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
	int x = atoi(xml.child(node, "x").c_str()), y = atoi(xml.child(node, "y").c_str());
	int w = atoi(xml.child(node, "w").c_str()), h = atoi(xml.child(node, "h").c_str());
	block.rect = Common::Rect(x, y, x + w, y + h);
	block.rect.clip(Common::Rect(800, 600));
	if (block.rect.isEmpty())
		return false;
	Common::ScopedPtr<Common::SeekableReadStream> stream(open("common/" + xml.child(node, "font") + ".nfnt"));
	block.font.reset(stream ? OM::NFNTFont::load(*stream) : nullptr);
	if (!block.font)
		return false;
	Common::String text = xml.child(node, "content");
	if (text.size() >= 2 && text.firstChar() == '"' && text.lastChar() == '"')
		text = text.substr(1, text.size() - 2);
	if (!Shell::Text::wrap(*block.font, text, block.rect.width(), block.lines, Shell::Text::kLauncher))
		return false;
	block.red = atoi(xml.child(node, "color/r").c_str()) >> 8;
	block.green = atoi(xml.child(node, "color/g").c_str()) >> 8;
	block.blue = atoi(xml.child(node, "color/b").c_str()) >> 8;
	block.centered = xml.child(node, "center") == "true";
	_textBlocks.push_back(block);
	return true;
}

} // namespace Launcher
} // namespace Trinket
