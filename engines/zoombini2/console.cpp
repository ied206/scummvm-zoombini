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

#include "zoombini2/console.h"

#include "common/file.h"
#include "common/fs.h"
#include "common/ptr.h"
#include "common/textconsole.h"
#include "common/util.h"

#ifdef USE_PNG
#include "image/png.h"
#endif

#include "zoombini2/graphics.h"
#include "zoombini2/pages/dialog_debug.h"
#include "zoombini2/pages/interactive_map.h"
#include "zoombini2/pages/page_base.h"
#include "zoombini2/pages/puzzle_base.h"
#include "zoombini2/pages/transition_maptrans.h"
#include "zoombini2/random.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/text_manager.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr RGBColor Zoombini2Console::kDefaultPlotColor;
constexpr const char *Zoombini2Console::kDebugDumpPathFormat;

const Zoombini2Console::ResourceTypeInfo Zoombini2Console::kResourceTypes[] = {
	{Zoombini2Engine::PageResourceType::kAreaMask, "areamask", "areamask"},
	{Zoombini2Engine::PageResourceType::kBitmap, "bitmap", "bb"},
	{Zoombini2Engine::PageResourceType::kBitmap, "bitmap", "bmp"},
	{Zoombini2Engine::PageResourceType::kSprite, "sprite", "rb"},
	{Zoombini2Engine::PageResourceType::kAnimation, "animation", "an"},
	{Zoombini2Engine::PageResourceType::kZoombiniAnimation, "zoombini", "anm"},
	{Zoombini2Engine::PageResourceType::kPath, "path", "pat"},
	{Zoombini2Engine::PageResourceType::kSound, "sound", "wav"},
	{Zoombini2Engine::PageResourceType::kVideo, "video", "bik"},
	{Zoombini2Engine::PageResourceType::kLayout, "layout", "ztl"},
	{Zoombini2Engine::PageResourceType::kUnknown, "unknown", "unknown"}};

Zoombini2Console::Zoombini2Console(Zoombini2Engine *vm)
	: GUI::Debugger(), _vm(vm) {
	registerCmd(kCmdSfx, WRAP_METHOD(Zoombini2Console, Cmd_Sfx));
	registerCmd(kCmdBgm, WRAP_METHOD(Zoombini2Console, Cmd_Bgm));
	registerCmd(kCmdGo, WRAP_METHOD(Zoombini2Console, Cmd_Go));
	registerCmd(kCmdDraw, WRAP_METHOD(Zoombini2Console, Cmd_Draw));
	registerCmd(kCmdDump, WRAP_METHOD(Zoombini2Console, Cmd_Dump));
	registerCmd(kCmdList, WRAP_METHOD(Zoombini2Console, Cmd_List));
	registerCmd(kCmdState, WRAP_METHOD(Zoombini2Console, Cmd_State));
	registerCmd(kCmdManShortcuts, WRAP_METHOD(Zoombini2Console, Cmd_ManShortcuts));
	registerCmd(kCmdPlot, WRAP_METHOD(Zoombini2Console, Cmd_Plot));
	registerCmd(kCmdBuiltinDebug, WRAP_METHOD(Zoombini2Console, Cmd_BuiltinDebug));
	registerCmd(kCmdManBuiltinDebug, WRAP_METHOD(Zoombini2Console, Cmd_ManBuiltinDebug));
	registerCmd(kCmdPuzzle, WRAP_METHOD(Zoombini2Console, Cmd_Puzzle));
	registerCmd(kCmdPage, WRAP_METHOD(Zoombini2Console, Cmd_Puzzle));
}

Zoombini2Console::~Zoombini2Console() {
}

bool Zoombini2Console::parseSignedInt(const char *text, int32 &result) {
	const StringParser::Result parseResult = StringParser::parseSignedInt(text, result, StringParser::kDecimalOrHexProfile);
	if (parseResult == StringParser::Result::kSuccess)
		return true;

	reportIntegerParseFailure(parseResult, false, text);
	return false;
}

bool Zoombini2Console::parseUnsignedInt(const char *text, uint32 &result) {
	const StringParser::Result parseResult = StringParser::parseUnsignedInt(text, result, StringParser::kDecimalOrHexProfile);
	if (parseResult == StringParser::Result::kSuccess)
		return true;

	reportIntegerParseFailure(parseResult, true, text);
	return false;
}

void Zoombini2Console::reportIntegerParseFailure(StringParser::Result result, bool unsignedValue, const char *text) {
	const char *parseFunction = unsignedValue ? "parseUnsignedInt" : "parseSignedInt";
	const char *type = unsignedValue ? "uint32" : "int32";
	switch (result) {
	case StringParser::Result::kEmptyInput:
		warning("%s: Empty string\n", parseFunction);
		break;
	case StringParser::Result::kInvalidInput:
		warning("%s: '%s' is not a valid %s\n", parseFunction, text, type);
		break;
	case StringParser::Result::kTrailingCharacters:
		warning("%s: '%s' contains invalid characters\n", parseFunction, text);
		break;
	case StringParser::Result::kConversionFailure:
		warning("%s: %s conversion failed in '%s'\n", parseFunction, type, text);
		break;
	case StringParser::Result::kOutOfRange:
		if (unsignedValue)
			warning("parseUnsignedInt: uint32 overflow in '%s'\n", text);
		else
			warning("parseSignedInt: int32 overflow or underflow in '%s'\n", text);
		break;
	case StringParser::Result::kSuccess:
		return;
	}
	debugPrintf("Cannot parse %s(%s) (hex supported with 0x prefix)\n", type, text ? text : "(null)");
}

bool Zoombini2Console::isHelpOption(const char *arg) {
	return arg && (scumm_stricmp(arg, "--help") == 0 || scumm_stricmp(arg, "-h") == 0);
}

bool Zoombini2Console::hasHelpOption(int argc, const char **argv) {
	for (int i = 1; i < argc; i++) {
		if (isHelpOption(argv[i]))
			return true;
	}

	return false;
}

void Zoombini2Console::printHelpOption() {
	debugPrintf("  -h, --help  Show this help text and exit.\n");
	debugPrintf("\n");
}

bool Zoombini2Console::Cmd_Sfx(int argc, const char **argv) {
	if (argc == 2 && isHelpOption(argv[1])) {
		debugPrintf("Control sound-effect playback.\n");
		debugPrintf("Usage: %s <%s|%s> [arguments]\n\n", kCmdSfx, kSubCmdSfxPlay, kSubCmdSfxStop);
		debugPrintf("  %s <wav-path>  Play a WAV resource below %s.\n", kSubCmdSfxPlay, kSfxResourcePrefix);
		debugPrintf("  %s             Stop all active sound effects.\n\n", kSubCmdSfxStop);
		debugPrintf("These commands do not change saved game state, music, or speech playback.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}
	if (argc < 2) {
		debugPrintf("Usage: %s <%s|%s> [arguments]\n\n", kCmdSfx, kSubCmdSfxPlay, kSubCmdSfxStop);
		return true;
	}
	if (scumm_stricmp(argv[1], kSubCmdSfxPlay) == 0)
		return CmdSub_SfxPlay(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdSfxStop) == 0)
		return CmdSub_SfxStop(argc, argv);

	debugPrintf("Unknown SFX subcommand '%s'. Use %s --help.\n\n", argv[1], kCmdSfx);
	return true;
}

bool Zoombini2Console::CmdSub_SfxPlay(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Play a sound-effect WAV resource.\n");
		debugPrintf("Usage: %s %s <%s...wav>\n\n", kCmdSfx, kSubCmdSfxPlay, kSfxResourcePrefix);
		debugPrintf("Use the logical resource path, for example sounds/fx/03-BS01.wav.\n");
		debugPrintf("Any active sound effects are stopped before the selected resource is played.\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}
	if (argc != 3) {
		debugPrintf("Usage: %s %s <%s...wav>\n\n", kCmdSfx, kSubCmdSfxPlay, kSfxResourcePrefix);
		return true;
	}

	const Common::Path soundPath(argv[2]);
	const Common::String soundPathString = soundPath.toString('/');
	if (!soundPathString.hasPrefixIgnoreCase(kSfxResourcePrefix) || !soundPathString.hasSuffixIgnoreCase(kWavResourceSuffix) ||
		soundPathString.contains("/../")) {
		debugPrintf("Sound path must be a WAV resource below %s.\n\n", kSfxResourcePrefix);
		return true;
	}
	if (!_vm->hasResource(soundPathString)) {
		debugPrintf("Cannot find sound resource '%s'.\n\n", soundPathString.c_str());
		return true;
	}

	SoundManager *sound = _vm->getSoundManager();
	if (!sound) {
		debugPrintf("Sound playback is unavailable.\n\n");
		return true;
	}
	sound->stopAll(SoundCategory::kSFX01);
	if (0 <= _debugSfxId) {
		sound->unload(_debugSfxId);
		_debugSfxId = -1;
	}
	_debugSfxId = sound->load(false, soundPath, false);
	sound->play(_debugSfxId);
	return false;
}

bool Zoombini2Console::CmdSub_SfxStop(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Stop all active sound effects without changing music, speech, or saved game state.\n");
		debugPrintf("Usage: %s %s\n\n", kCmdSfx, kSubCmdSfxStop);
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}
	if (argc != 2) {
		debugPrintf("Usage: %s %s\n\n", kCmdSfx, kSubCmdSfxStop);
		return true;
	}

	debugPrintf("Stopping sound effects.\n\n");
	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		sound->stopAll(SoundCategory::kSFX01);
		if (0 <= _debugSfxId) {
			sound->unload(_debugSfxId);
			_debugSfxId = -1;
		}
	}
	return true;
}

bool Zoombini2Console::Cmd_Bgm(int argc, const char **argv) {
	if (argc == 2 && isHelpOption(argv[1])) {
		debugPrintf("Control background music playback.\n");
		debugPrintf("Usage: %s <%s|%s> [arguments]\n\n", kCmdBgm, kSubCmdBgmPlay, kSubCmdBgmStop);
		debugPrintf("  %s <wav-path>  Stream and loop a WAV resource below %s or %s.\n", kSubCmdBgmPlay, kBgmResourcePrefix,
					kInstalledBgmResourcePrefix);
		debugPrintf("  %s             Stop all active background music.\n\n", kSubCmdBgmStop);
		debugPrintf("These commands do not change saved game state, sound effects, or speech playback.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}
	if (argc < 2) {
		debugPrintf("Usage: %s <%s|%s> [arguments]\n\n", kCmdBgm, kSubCmdBgmPlay, kSubCmdBgmStop);
		return true;
	}
	if (scumm_stricmp(argv[1], kSubCmdBgmPlay) == 0)
		return CmdSub_BgmPlay(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdBgmStop) == 0)
		return CmdSub_BgmStop(argc, argv);

	debugPrintf("Unknown BGM subcommand '%s'. Use %s --help.\n\n", argv[1], kCmdBgm);
	return true;
}

bool Zoombini2Console::CmdSub_BgmPlay(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Stream and loop a background-music WAV resource.\n");
		debugPrintf("Usage: %s %s <%s...wav|%s...wav>\n\n", kCmdBgm, kSubCmdBgmPlay, kBgmResourcePrefix,
					kInstalledBgmResourcePrefix);
		debugPrintf("Use the installed-root marker for tracks under INSTALL/HD, for example #sounds/music/01-BS06.wav.\n");
		debugPrintf("Any active background music is stopped before the selected resource is played.\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}
	if (argc != 3) {
		debugPrintf("Usage: %s %s <%s...wav|%s...wav>\n\n", kCmdBgm, kSubCmdBgmPlay, kBgmResourcePrefix,
					kInstalledBgmResourcePrefix);
		return true;
	}

	const Common::Path bgmPath(argv[2]);
	const Common::String bgmPathString = bgmPath.toString('/');
	const bool hasBgmPrefix = bgmPathString.hasPrefixIgnoreCase(kBgmResourcePrefix) ||
							  bgmPathString.hasPrefixIgnoreCase(kInstalledBgmResourcePrefix);
	if (!hasBgmPrefix || !bgmPathString.hasSuffixIgnoreCase(kWavResourceSuffix) || bgmPathString.contains("/../")) {
		debugPrintf("BGM path must be a WAV resource below %s or %s.\n\n", kBgmResourcePrefix, kInstalledBgmResourcePrefix);
		return true;
	}
	if (!_vm->hasResource(bgmPathString)) {
		debugPrintf("Cannot find BGM resource '%s'.\n\n", bgmPathString.c_str());
		return true;
	}

	SoundManager *sound = _vm->getSoundManager();
	if (!sound) {
		debugPrintf("Sound playback is unavailable.\n\n");
		return true;
	}
	sound->stopAll(SoundCategory::kMusic00);
	if (0 <= _debugBgmId) {
		sound->unload(_debugBgmId);
		_debugBgmId = -1;
	}
	_debugBgmId = sound->load(true, bgmPath, true);
	sound->play(_debugBgmId);
	return false;
}

bool Zoombini2Console::CmdSub_BgmStop(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Stop all active background music without changing sound effects, speech, or saved game state.\n");
		debugPrintf("Usage: %s %s\n\n", kCmdBgm, kSubCmdBgmStop);
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}
	if (argc != 2) {
		debugPrintf("Usage: %s %s\n\n", kCmdBgm, kSubCmdBgmStop);
		return true;
	}

	debugPrintf("Stopping background music.\n\n");
	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		sound->stopAll(SoundCategory::kMusic00);
		if (0 <= _debugBgmId) {
			sound->unload(_debugBgmId);
			_debugBgmId = -1;
		}
	}
	return true;
}

bool Zoombini2Console::Cmd_BuiltinDebug(int argc, const char **argv) {
	const Common::String fileName = _vm->getDebugZoombiniSetFileName();
	if (argc < 2 || hasHelpOption(argc, argv)) {
		debugPrintf("Run an original Zoombinis Mountain Rescue developer action.\n");
		debugPrintf("Usage: %s <export|import|complete>\n\n", kCmdBuiltinDebug);
		debugPrintf("  export    Run F2: write the current party's traits to %s in the save location.\n", fileName.c_str());
		debugPrintf("  import    Run F3: read that file and apply traits to existing party members.\n");
		debugPrintf("  complete  Run P: mark the party free through the active page's completion path.\n\n");
		debugPrintf("Debugger actions run independently of the in-game hotkey setting.\n");
		debugPrintf("Use %s for the in-game keys and page limits.\n\n", kCmdManBuiltinDebug);
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}
	if (argc != 2) {
		debugPrintf("Usage: %s <export|import|complete>\n\n", kCmdBuiltinDebug);
		return true;
	}
	if (scumm_stricmp(argv[1], "export") == 0) {
		_vm->runBuiltinDebugAction(Zoombini2Engine::BuiltinDebugAction::kExportZoombiniSet);
		debugPrintf("F2 export to %s in the save location attempted. Empty parties produce no file.\n\n", fileName.c_str());
	} else if (scumm_stricmp(argv[1], "import") == 0) {
		_vm->runBuiltinDebugAction(Zoombini2Engine::BuiltinDebugAction::kImportZoombiniSet);
		debugPrintf("F3 import from %s attempted. Missing or invalid input leaves the party unchanged.\n\n", fileName.c_str());
	} else if (scumm_stricmp(argv[1], "complete") == 0) {
		if (_vm->runBuiltinDebugAction(Zoombini2Engine::BuiltinDebugAction::kCompletePuzzle))
			debugPrintf("P completion state applied. The active page handles departure.\n\n");
		else
			debugPrintf("P is unavailable on the active shelter or finale page.\n\n");
	} else {
		debugPrintf("Unknown developer action '%s'. Use %s --help.\n\n", argv[1], kCmdBuiltinDebug);
	}
	return true;
}

bool Zoombini2Console::Cmd_ManBuiltinDebug(int argc, const char **argv) {
	const Common::String fileName = _vm->getDebugZoombiniSetFileName();
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Print the original Zoombinis Mountain Rescue developer-key manual.\n");
		debugPrintf("Usage: %s\n\n", kCmdManBuiltinDebug);
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}
	if (argc != 1) {
		debugPrintf("Usage: %s\n\n", kCmdManBuiltinDebug);
		return true;
	}
	debugPrintf("Zoombinis Mountain Rescue developer keys\n");
	debugPrintf("  In-game hotkeys: %s (enable in the engine options).\n", _vm->areBuiltinDebugHotkeysEnabled() ? "enabled" : "disabled");
	debugPrintf("  Close the ScummVM debugger before using a game key.\n\n");
	debugPrintf("[Global keys]\n");
	debugPrintf("  F2  Export the active party to %s in ScummVM's configured save location.\n", fileName.c_str());
	debugPrintf("      Count, then one Feet Nose Hair Eyes decimal tuple per member.\n");
	debugPrintf("      The file is uncompressed text and can be edited before F3 imports it.\n");
	debugPrintf("      An empty party writes nothing.\n");
	debugPrintf("  F3  Read whitespace-delimited tuples from the same target-prefixed file.\n");
	debugPrintf("      Update at most the current party size; do not create or remove members.\n");
	debugPrintf("      Validate the entire file first: at most 4096 bytes, 1-16 members, trait values 1-5.\n");
	debugPrintf("      Incomplete, invalid, or extra data rejects the file without changing the party.\n");
	debugPrintf("  P   While held, mark the active party free for normal puzzle departure.\n");
	debugPrintf("      The changed puzzle state remains after the key is released.\n");
	debugPrintf("      Unavailable in Zombiniville, both rescue sites, Booliewood, and the finale.\n");
	debugPrintf("      Wall of Fleens also receives perfect-clear eligibility.\n\n");
	debugPrintf("[Chez Norf only]\n");
	debugPrintf("  C   While held, show the generated food values, table labels, and clue counts.\n");
	debugPrintf("      The overlay changes no puzzle state.\n\n");
	debugPrintf("The direct F2, F3, and P actions are available through %s.\n\n", kCmdBuiltinDebug);
	return true;
}

const TransitionMapTrans::RouteDest *Zoombini2Console::findGoDest(const char *name, bool puzzleOnly) {
	int pageNumber = -1;
	const char *digits = name;
	while (Common::isSpace(digits[0]))
		digits += 1;
	bool numeric = false;
	if (Common::isDigit(digits[0]) || digits[0] == '+' || digits[0] == '-')
		numeric = parseSignedInt(name, pageNumber);
	uint destinationCount = 0;
	const TransitionMapTrans::RouteDest *destinations = TransitionMapTrans::getRouteDestinations(destinationCount);
	for (uint i = 0; i < destinationCount; i++) {
		const TransitionMapTrans::RouteDest &destination = destinations[i];
		if (puzzleOnly && InteractiveMap::getPracticePartySize(destination.target) == 0)
			continue;
		if (scumm_stricmp(name, destination.name) == 0 || (numeric && static_cast<int>(destination.target) == pageNumber))
			return &destination;
	}
	return nullptr;
}

void Zoombini2Console::printGoDestinations(bool puzzleOnly) {
	uint destinationCount = 0;
	const TransitionMapTrans::RouteDest *destinations = TransitionMapTrans::getRouteDestinations(destinationCount);
	for (uint i = 0; i < destinationCount; i++) {
		const TransitionMapTrans::RouteDest &destination = destinations[i];
		if (puzzleOnly && InteractiveMap::getPracticePartySize(destination.target) == 0)
			continue;
		debugPrintf("  %-15s page %2d%s\n", destination.name, static_cast<int>(destination.target),
					puzzleOnly ? " (restored level 4)" : "");
	}
}

bool Zoombini2Console::Cmd_Go(int argc, const char **argv) {
	if (argc < 2 || isHelpOption(argv[1])) {
		debugPrintf("Navigate directly to a route transition or practice puzzle.\n");
		debugPrintf("Usage: %s <%s|%s> ...\n\n", kCmdGo, kSubCmdGoXfer, kSubCmdGoPractice);
		debugPrintf("  go xfer <destination> [level]  Jump to the route-map transition.\n");
		debugPrintf("  go practice <puzzle> <level> [count]  Start a practice puzzle.\n");
		debugPrintf("Use a subcommand without arguments to list destinations.\n\n");
		return true;
	}
	if (scumm_stricmp(argv[1], kSubCmdGoXfer) == 0)
		return CmdSub_GoXfer(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdGoPractice) == 0)
		return CmdSub_GoPractice(argc, argv);
	debugPrintf("Unknown go subcommand '%s'. Use go --help.\n\n", argv[1]);
	return true;
}

bool Zoombini2Console::CmdSub_GoPractice(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc < 4 || 5 < argc) {
		debugPrintf("Usage: go practice <puzzle> <level> [count]\n");
		debugPrintf("Puzzle accepts a name or page number. Level is 1-3, or restored level 4.\n");
		debugPrintf("Count defaults to the route party size: 16 before Rescue I, 8 afterward.\n");
		debugPrintf("Available puzzles:\n");
		printGoDestinations(true);
		debugPrintf("\n");
		return true;
	}
	if (!_vm->hasActiveGamePage()) {
		debugPrintf("No active Zoombini2 game page.\n\n");
		return true;
	}
	const TransitionMapTrans::RouteDest *destination = findGoDest(argv[2], true);
	if (!destination) {
		debugPrintf("Unknown puzzle '%s'. Use go practice without arguments for the list.\n\n", argv[2]);
		return true;
	}
	uint level = 0;
	if (!parseUnsignedInt(argv[3], level) || level < 1 || 4 < level) {
		debugPrintf("Invalid level '%s'. Must be 1-4.\n\n", argv[3]);
		return true;
	}
	const uint maxPartySize = InteractiveMap::getPracticePartySize(destination->target);
	uint count = maxPartySize;
	if (argc == 5 && (!parseUnsignedInt(argv[4], count) || count < 1 || maxPartySize < count)) {
		debugPrintf("Invalid count '%s'. Must be 1-%u for this route.\n\n", argv[4], maxPartySize);
		return true;
	}
	_vm->queueDebugPracticeLaunch(destination->target, static_cast<int>(level), count);
	debugPrintf("Starting practice page %d at level %u with %u Zoombinis.\n\n", static_cast<int>(destination->target), level, count);
	return false;
}

bool Zoombini2Console::CmdSub_GoXfer(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc < 3 || 4 < argc) {
		debugPrintf("Usage: go xfer <destination> [level]\n");
		debugPrintf("Destination accepts a name or page number. An explicit level starts isolated practice.\n");
		debugPrintf("Without a level, the current journey or practice level is retained.\n");
		debugPrintf("Rescue sites use seated members; surplus and uncompleted arrival parties return to the village.\n");
		debugPrintf("Available destinations:\n");
		printGoDestinations(false);
		debugPrintf("\n");
		return true;
	}
	if (!_vm->hasActiveGamePage()) {
		debugPrintf("No active Zoombini2 game page.\n\n");
		return true;
	}
	const TransitionMapTrans::RouteDest *destination = findGoDest(argv[2], false);
	if (!destination) {
		debugPrintf("Unknown destination '%s'. Use go xfer without arguments for the list.\n\n", argv[2]);
		return true;
	}
	uint level = 0;
	if (argc == 4 && (!parseUnsignedInt(argv[3], level) || level < 1 || 4 < level)) {
		debugPrintf("Invalid level '%s'. Must be 1-4.\n\n", argv[3]);
		return true;
	}
	const Zoombini2Engine::DebugRouteRequestResult result =
		_vm->requestDebugMapTransition(destination->target, destination->source, destination->branch, level);
	switch (result) {
	case Zoombini2Engine::DebugRouteRequestResult::kQueued:
		debugPrintf("Jumping through the route transition to page %d.\n\n", static_cast<int>(destination->target));
		return false;
	case Zoombini2Engine::DebugRouteRequestResult::kNoActivePage:
		debugPrintf("No active Zoombini2 game page.\n\n");
		return true;
	case Zoombini2Engine::DebugRouteRequestResult::kNoTravelingParty:
		debugPrintf("No traveling party. Select Zoombinis first, or supply a level for isolated practice.\n\n");
		return true;
	case Zoombini2Engine::DebugRouteRequestResult::kInvalidLevel:
		debugPrintf("Invalid level '%u'. Must be 1-4.\n\n", level);
		return true;
	case Zoombini2Engine::DebugRouteRequestResult::kPracticeRequiresPuzzle:
		debugPrintf("Practice transitions need a puzzle destination; shelters have no practice level.\n\n");
		return true;
	case Zoombini2Engine::DebugRouteRequestResult::kUnreachableDestination:
		debugPrintf("Destination page %d is not reachable with the current rescue count.\n\n", static_cast<int>(destination->target));
		return true;
	}
	return true;
}

bool Zoombini2Console::Cmd_Draw(int argc, const char **argv) {
	if (argc == 2 && isHelpOption(argv[1])) {
		debugPrintf("Inspect debug views of the active page and resources.\n");
		debugPrintf("Usage: %s <subcommand> [arguments]\n\n", kCmdDraw);
		debugPrintf("Subcommands:\n");
		debugPrintf("  %s\n", kSubCmdDrawAreaMask);
		debugPrintf("      Show the active page with rejected drop areas darkened.\n\n");
		debugPrintf("  %s <path> [start-frame]\n", kSubCmdDrawSprites);
		debugPrintf("      Show a numbered sprite sheet from an .an or .rb resource. Alias: sprite.\n\n");
		debugPrintf("  %s <path>\n", kSubCmdDrawImage);
		debugPrintf("      Show a decoded bitmap (.bb/.bmp) over white.\n\n");
		debugPrintf("  %s <path> [color]\n", kSubCmdDrawCurve);
		debugPrintf("      Draw a PAT path over the current screen.\n\n");
		debugPrintf("  %s <path> | --text \"message\"\n", kSubCmdDrawMsgBox);
		debugPrintf("      Preview a bitmap message or UTF-8 UI text without a game callback.\n\n");
		printResourcePathHelp();
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 2) {
		debugPrintf("Usage: %s <%s|%s|%s|%s|%s> [arguments]\n", kCmdDraw, kSubCmdDrawAreaMask, kSubCmdDrawSprites,
					kSubCmdDrawImage, kSubCmdDrawCurve, kSubCmdDrawMsgBox);
		debugPrintf("\n");
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdDrawAreaMask) == 0)
		return CmdSub_DrawAreaMask(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDrawSprites) == 0 || scumm_stricmp(argv[1], kSubCmdDrawSprite) == 0)
		return CmdSub_DrawSprites(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDrawImage) == 0)
		return CmdSub_DrawImage(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDrawCurve) == 0)
		return CmdSub_DrawCurve(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDrawMsgBox) == 0)
		return CmdSub_DrawMsgBox(argc, argv);

	debugPrintf("Unknown %s subcommand '%s'.\n", kCmdDraw, argv[1]);
	debugPrintf("Usage: %s <%s|%s|%s|%s|%s> [arguments]\n", kCmdDraw, kSubCmdDrawAreaMask, kSubCmdDrawSprites,
				kSubCmdDrawImage, kSubCmdDrawCurve, kSubCmdDrawMsgBox);
	debugPrintf("\n");
	return true;
}

bool Zoombini2Console::CmdSub_DrawAreaMask(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Show the active staging area of this page.\n");
		debugPrintf("Usage: %s %s\n\n", kCmdDraw, kSubCmdDrawAreaMask);
		debugPrintf("Areas outside the are mask are darkened.\n");
		debugPrintf("Click or press ESC while the dialog is shown to close it.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc != 2) {
		debugPrintf("Usage: %s %s\n", kCmdDraw, kSubCmdDrawAreaMask);
		debugPrintf("\n");
		return true;
	}

	if (!_vm->getActivePage()) {
		debugPrintf("No active Zoombini2 page.\n");
		debugPrintf("\n");
		return true;
	}

	DialogDebugCommand cmd;
	cmd.setDrawAreaMask();
	if (!_vm->openDebugDialog(cmd))
		return true;
	return false;
}

bool Zoombini2Console::CmdSub_DrawSprites(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Show a numbered sprite sheet from an .an or .rb resource in the debug dialog.\n");
		debugPrintf("Usage: %s %s <path> [start-frame]\n\n", kCmdDraw, kSubCmdDrawSprites);
		debugPrintf("Alias: %s %s <path> [start-frame]\n\n", kCmdDraw, kSubCmdDrawSprite);
		debugPrintf("<path> is an animation (.an) or sprite (.rb) resource path,\n");
		debugPrintf("with or without its extension. Frames are numbered from 0;\n");
		debugPrintf("[start-frame] defaults to 0 and selects the first frame on the sheet.\n");
		debugPrintf("Frames retain their aspect ratio and shrink only when larger than the preview cells.\n");
		debugPrintf("Use the Left/Right keys to browse sheets when the frames do not fit on one screen.\n");
		debugPrintf("Click or press ESC while the dialog is shown to close it.\n\n");
		printResourcePathHelp();
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 3 || 4 < argc) {
		debugPrintf("Usage: %s %s <path> [start-frame]\n\n", kCmdDraw, kSubCmdDrawSprites);
		printResourcePathHelp();
		return true;
	}

	uint startFrame = 0;
	if (argc == 4 && (!parseUnsignedInt(argv[3], startFrame) || INT_MAX < startFrame)) {
		debugPrintf("Cannot parse argument %s\n", argv[3]);
		debugPrintf("\n");
		return true;
	}

	DialogDebugCommand cmd;
	Common::Path path;
	if (!parseDebugResourcePath(argv[2], path))
		return true;
	cmd.setDrawSprites(path, static_cast<int>(startFrame));
	if (!_vm->openDebugDialog(cmd)) {
		debugPrintf("Cannot open sprite sheet '%s' at frame %u\n", path.toString('/').c_str(), startFrame);
		debugPrintf("\n");
		return true;
	}
	return false;
}

bool Zoombini2Console::CmdSub_DrawCurve(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc < 3 || 4 < argc) {
		debugPrintf("Draw a PAT path over the current screen.\n");
		debugPrintf("Usage: %s %s <path> [color]\n\n", kCmdDraw, kSubCmdDrawCurve);
		debugPrintf("Use a .pat resource path. Every segment is drawn at its original screen coordinates.\n");
		debugPrintf("Quote paths containing spaces.\n");
		debugPrintf("Color is 24-bit RGB (0xRRGGBB or #RRGGBB), default: white.\n");
		debugPrintf("Click or press ESC to close the dialog.\n\n");
		printResourcePathHelp();
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	Common::Path path;
	RGBColor color = kDefaultPlotColor;
	if (!parseDebugResourcePath(argv[2], path) || (argc == 4 && !parsePlotColor(argv[3], color)))
		return true;
	DialogDebugCommand cmd;
	cmd.setDrawCurve(path, color);
	if (!_vm->openDebugDialog(cmd)) {
		debugPrintf("Cannot draw PAT path '%s'.\n\n", path.toString('/').c_str());
		return true;
	}
	return false;
}

bool Zoombini2Console::Cmd_Plot(int argc, const char **argv) {
	if (argc < 2 || (argc == 2 && isHelpOption(argv[1]))) {
		debugPrintf("Plot geometry over the current screen.\n");
		debugPrintf("Usage: %s <subcommand> [arguments]\n\n", kCmdPlot);
		debugPrintf("Subcommands:\n");
		debugPrintf("  %s <x> <y> [color]\n", kSubCmdPlotPoint);
		debugPrintf("      Plot one pixel within the game screen.\n");
		debugPrintf("  %s <x0> <y0> <x1> <y1> [color]\n", kSubCmdPlotLine);
		debugPrintf("      Plot a line between two inclusive endpoints.\n");
		debugPrintf("  %s <x1> <y1> <x2> <y2> [color]\n", kSubCmdPlotRect);
		debugPrintf("      Plot a rectangle outline; right and bottom coordinates are exclusive.\n");
		debugPrintf("  %s <x0> <y0> <cx0> <cy0> <cx1> <cy1> <x1> <y1> [color]\n", kSubCmdPlotCurve);
		debugPrintf("      Plot a cubic Bezier curve from its endpoints and two control points.\n\n");
		debugPrintf("Coordinates are 0-based signed 16-bit values; geometry is clipped to the screen.\n");
		debugPrintf("Integer arguments accept a 0x prefix. Color is 24-bit RGB (0xRRGGBB or #RRGGBB), default: white.\n");
		debugPrintf("Click or press ESC to close the dialog.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdPlotPoint) == 0)
		return CmdSub_PlotPoint(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdPlotLine) == 0)
		return CmdSub_PlotLine(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdPlotRect) == 0)
		return CmdSub_PlotRect(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdPlotCurve) == 0)
		return CmdSub_PlotCurve(argc, argv);

	debugPrintf("Unknown %s subcommand '%s'. Use %s, %s, %s, or %s.\n\n", kCmdPlot, argv[1],
				kSubCmdPlotPoint, kSubCmdPlotLine, kSubCmdPlotRect, kSubCmdPlotCurve);
	return true;
}

bool Zoombini2Console::parsePlotPoint(const char *xText, const char *yText, Common::Point32 &point) {
	int32 x = 0;
	int32 y = 0;
	if (!parseSignedInt(xText, x) || !parseSignedInt(yText, y))
		return false;
	if (x < INT16_MIN || INT16_MAX < x || y < INT16_MIN || INT16_MAX < y) {
		debugPrintf("Coordinates must be in the signed 16-bit range.\n");
		return false;
	}
	point = Common::Point32(x, y);
	return true;
}

bool Zoombini2Console::parsePlotColor(const char *text, RGBColor &color) {
	uint32 parsed = 0;
	if (text && text[0] == '#') {
		if (strlen(text) != 7) {
			debugPrintf("A #RRGGBB color must contain exactly six hexadecimal digits.\n");
			return false;
		}
		const Common::String hexadecimal = Common::String::format("0x%s", text + 1);
		if (!parseUnsignedInt(hexadecimal.c_str(), parsed))
			return false;
	} else if (!parseUnsignedInt(text, parsed)) {
		return false;
	}
	if (0xFFFFFF < parsed) {
		debugPrintf("Color must be a 24-bit RGB value (0xRRGGBB or #RRGGBB).\n");
		return false;
	}
	color = RGBColor(static_cast<byte>(parsed >> 16), static_cast<byte>(parsed >> 8), static_cast<byte>(parsed));
	return true;
}

bool Zoombini2Console::CmdSub_PlotPoint(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc < 4 || 5 < argc) {
		debugPrintf("Plot one pixel over the current screen.\n");
		debugPrintf("Usage: %s %s <x> <y> [color]\n\n", kCmdPlot, kSubCmdPlotPoint);
		debugPrintf("Coordinates must be within the 800x600 game screen.\n");
		debugPrintf("Color is 24-bit RGB (0xRRGGBB or #RRGGBB), default: white.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	Common::Point32 point;
	RGBColor color = kDefaultPlotColor;
	if (!parsePlotPoint(argv[2], argv[3], point) || (argc == 5 && !parsePlotColor(argv[4], color))) {
		debugPrintf("\n");
		return true;
	}
	if (point.x < 0 || Gfx::kScreenSize.width <= point.x || point.y < 0 || Gfx::kScreenSize.height <= point.y) {
		debugPrintf("Coordinates out of bounds (screen size: %d x %d).\n\n", Gfx::kScreenSize.width, Gfx::kScreenSize.height);
		return true;
	}

	DialogDebugCommand cmd;
	cmd.setPlotPoint(point, color);
	if (!_vm->openDebugDialog(cmd)) {
		debugPrintf("Cannot open the plot debug dialog.\n\n");
		return true;
	}
	return false;
}

bool Zoombini2Console::CmdSub_PlotLine(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc < 6 || 7 < argc) {
		debugPrintf("Plot a line over the current screen.\n");
		debugPrintf("Usage: %s %s <x0> <y0> <x1> <y1> [color]\n\n", kCmdPlot, kSubCmdPlotLine);
		debugPrintf("Endpoints are inclusive signed 16-bit coordinates; the line is clipped to the screen.\n");
		debugPrintf("Color is 24-bit RGB (0xRRGGBB or #RRGGBB), default: white.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	Common::Point32 start;
	Common::Point32 end;
	RGBColor color = kDefaultPlotColor;
	if (!parsePlotPoint(argv[2], argv[3], start) || !parsePlotPoint(argv[4], argv[5], end) || (argc == 7 && !parsePlotColor(argv[6], color))) {
		debugPrintf("\n");
		return true;
	}

	DialogDebugCommand cmd;
	cmd.setPlotLine(start, end, color);
	if (!_vm->openDebugDialog(cmd)) {
		debugPrintf("Cannot open the plot debug dialog.\n\n");
		return true;
	}
	return false;
}

bool Zoombini2Console::CmdSub_PlotRect(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc < 6 || 7 < argc) {
		debugPrintf("Plot a rectangle outline over the current screen.\n");
		debugPrintf("Usage: %s %s <x1> <y1> <x2> <y2> [color]\n\n", kCmdPlot, kSubCmdPlotRect);
		debugPrintf("Coordinates are signed 16-bit values; x1 < x2 and y1 < y2 are required.\n");
		debugPrintf("Right and bottom coordinates are exclusive; edges are clipped to the screen.\n");
		debugPrintf("Color is 24-bit RGB (0xRRGGBB or #RRGGBB), default: white.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	Common::Point32 topLeft;
	Common::Point32 bottomRight;
	RGBColor color = kDefaultPlotColor;
	if (!parsePlotPoint(argv[2], argv[3], topLeft) || !parsePlotPoint(argv[4], argv[5], bottomRight) || (argc == 7 && !parsePlotColor(argv[6], color))) {
		debugPrintf("\n");
		return true;
	}
	if (bottomRight.x <= topLeft.x || bottomRight.y <= topLeft.y) {
		debugPrintf("Invalid rectangle coordinates: x1 < x2 and y1 < y2 are required.\n\n");
		return true;
	}

	DialogDebugCommand cmd;
	cmd.setPlotRect(topLeft, bottomRight, color);
	if (!_vm->openDebugDialog(cmd)) {
		debugPrintf("Cannot open the plot debug dialog.\n\n");
		return true;
	}
	return false;
}

bool Zoombini2Console::CmdSub_PlotCurve(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc < 10 || 11 < argc) {
		debugPrintf("Plot a cubic Bezier curve over the current screen.\n");
		debugPrintf("Usage: %s %s <x0> <y0> <cx0> <cy0> <cx1> <cy1> <x1> <y1> [color]\n\n", kCmdPlot, kSubCmdPlotCurve);
		debugPrintf("Example: %s %s 100 450 150 50 600 50 700 450 #00FFFF\n\n", kCmdPlot, kSubCmdPlotCurve);
		debugPrintf("(x0,y0) and (x1,y1) are the endpoints; (cx0,cy0) and (cx1,cy1) are the control points.\n");
		debugPrintf("Coordinates are signed 16-bit values; the curve is clipped to the screen.\n");
		debugPrintf("Color is 24-bit RGB (0xRRGGBB or #RRGGBB), default: white.\n");
		debugPrintf("Click or press ESC to close the dialog.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	Common::Point32 start;
	Common::Point32 ctrl0;
	Common::Point32 ctrl1;
	Common::Point32 end;
	RGBColor color = kDefaultPlotColor;
	if (!parsePlotPoint(argv[2], argv[3], start) || !parsePlotPoint(argv[4], argv[5], ctrl0) ||
		!parsePlotPoint(argv[6], argv[7], ctrl1) || !parsePlotPoint(argv[8], argv[9], end) ||
		(argc == 11 && !parsePlotColor(argv[10], color))) {
		debugPrintf("\n");
		return true;
	}
	DialogDebugCommand cmd;
	cmd.setPlotCurve(start, ctrl0, ctrl1, end, color);
	if (!_vm->openDebugDialog(cmd)) {
		debugPrintf("Cannot open the plot debug dialog.\n\n");
		return true;
	}
	return false;
}

bool Zoombini2Console::Cmd_Puzzle(int argc, const char **argv) {
	if (argc < 2 || isHelpOption(argv[1])) {
		debugPrintf("Inspect or control the active puzzle.\nUsage: page|puzzle <subcommand> [arguments]\n\n");
		debugPrintf("  retry                        Retry the same puzzle from the beginning.\n");
		debugPrintf("  finish                       Accept the party and depart.\n");
		debugPrintf("  answer                       Print the generated rules/answer without changing state.\n");
		debugPrintf("  chance|chances [get]          Show remaining chances and other resource budgets.\n");
		debugPrintf("  chance|chances set <remaining> Set a supported finite budget.\n\n");
		const PuzzleBase *puzzle = dynamic_cast<PuzzleBase *>(_vm->getActivePage());
		const char *chanceSupport = "unsupported";
		if (puzzle && puzzle->debugCanSetChances())
			chanceSupport = "supported";
		debugPrintf("Chance setting is %s in the active page state.\n", chanceSupport);
		debugPrintf("Commands work in practice and journey modes, independently of the in-game debug hotkey flag.\n");
		debugPrintf("Use puzzle <subcommand> --help for details.\n\n");
		return true;
	}
	if (scumm_stricmp(argv[1], "retry") == 0)
		return CmdSub_PuzzleRetry(argc, argv);
	if (scumm_stricmp(argv[1], "finish") == 0)
		return CmdSub_PuzzleFinish(argc, argv);
	if (scumm_stricmp(argv[1], "answer") == 0)
		return CmdSub_PuzzleAnswer(argc, argv);
	if (scumm_stricmp(argv[1], "chance") == 0 || scumm_stricmp(argv[1], "chances") == 0)
		return CmdSub_PuzzleChance(argc, argv);
	debugPrintf("Unknown puzzle subcommand '%s'. Use puzzle --help.\n\n", argv[1]);
	return true;
}

bool Zoombini2Console::CmdSub_PuzzleRetry(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc != 2) {
		debugPrintf("Usage: page|puzzle retry\nRetry the current puzzle with its original party, level and generated rules.\n");
		debugPrintf("Progress and chances return to their state at puzzle entry. Works in practice and journey modes.\n\n");
		return true;
	}
	if (!dynamic_cast<PuzzleBase *>(_vm->getActivePage())) {
		debugPrintf("Current page is not a puzzle page.\n\n");
		return true;
	}
	if (!_vm->debugRetryPuzzle()) {
		debugPrintf("Cannot retry while a page change is pending or no puzzle entry checkpoint is available.\n\n");
		return true;
	}
	debugPrintf("Retrying the current puzzle.\n\n");
	return false;
}

bool Zoombini2Console::CmdSub_PuzzleFinish(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc != 2) {
		debugPrintf("Usage: page|puzzle finish\nAccept the whole party and trigger regular departure.\n");
		debugPrintf("Practice mode returns to its map; journey mode retains the accepted party and advances along the route.\n\n");
		return true;
	}
	PuzzleBase *puzzle = dynamic_cast<PuzzleBase *>(_vm->getActivePage());
	if (!puzzle) {
		debugPrintf("Current page is not a puzzle page.\n\n");
		return true;
	}
	puzzle->debugForceFinish();
	debugPrintf("Departure triggered.\n\n");
	return true;
}

bool Zoombini2Console::CmdSub_PuzzleAnswer(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc != 2) {
		debugPrintf("Usage: page|puzzle answer\nPrint the current generated answer without changing game state or RNG.\n\n");
		return true;
	}
	const PuzzleBase *puzzle = dynamic_cast<PuzzleBase *>(_vm->getActivePage());
	if (!puzzle) {
		debugPrintf("Current page is not a puzzle page.\n\n");
		return true;
	}
	const Common::String answer = formatPuzzleAnswer(puzzle->debugGetAnswer());
	debugPrintf("%s", answer.c_str());
	if (answer.empty() || answer.lastChar() != '\n')
		debugPrintf("\n");
	debugPrintf("\n");
	return true;
}

Common::String Zoombini2Console::formatPuzzleAnswer(const Common::String &answer) {
	static constexpr uint kLineWidth = 88;
	Common::String formatted;
	size_t lineStart = 0;
	while (lineStart < answer.size()) {
		const size_t lineEnd = answer.findFirstOf('\n', lineStart);
		const size_t lineLength = lineEnd == Common::String::npos ? answer.size() - lineStart : lineEnd - lineStart;
		Common::String line = answer.substr(lineStart, lineLength);
		const size_t firstWord = line.findFirstNotOf(' ');
		const size_t indentation = firstWord == Common::String::npos ? line.size() : firstWord;
		const Common::String continuation = line.substr(0, indentation) + "  ";
		while (kLineWidth < line.size()) {
			size_t breakPosition = line.findLastOf(' ', kLineWidth);
			if (breakPosition == Common::String::npos || breakPosition <= indentation)
				breakPosition = line.findFirstOf(' ', kLineWidth);
			if (breakPosition == Common::String::npos)
				break;
			formatted += line.substr(0, breakPosition) + '\n';
			const size_t nextWord = line.findFirstNotOf(' ', breakPosition);
			if (nextWord == Common::String::npos) {
				line.clear();
				break;
			}
			line = continuation + line.substr(nextWord);
		}
		formatted += line;
		if (lineEnd == Common::String::npos)
			break;
		formatted += '\n';
		lineStart = lineEnd + 1;
	}
	return formatted;
}

bool Zoombini2Console::CmdSub_PuzzleChance(int argc, const char **argv) {
	PuzzleBase *puzzle = dynamic_cast<PuzzleBase *>(_vm->getActivePage());
	const bool query = argc == 2 || (argc == 3 && scumm_stricmp(argv[2], "get") == 0);
	const bool set = 2 < argc && scumm_stricmp(argv[2], "set") == 0;
	if (hasHelpOption(argc, argv) || (!query && !set)) {
		debugPrintf("Usage: page|puzzle chance|chances [get|set <remaining>]\n");
		debugPrintf("Without an argument, show the chance model and remaining count.\n");
		debugPrintf("Set accepts a decimal or 0x-prefixed integer from one through the opportunity limit.\n");
		const char *chanceSupport = "unsupported";
		if (puzzle && puzzle->debugCanSetChances())
			chanceSupport = "supported";
		debugPrintf("Setting is %s in the active page state.\n\n", chanceSupport);
		return true;
	}
	PuzzleChanceInfo info = puzzle ? puzzle->debugGetChances() : PuzzleChanceInfo();
	if (set) {
		if (argc != 4) {
			debugPrintf("Usage: page|puzzle chance|chances set <remaining>\n\n");
			return true;
		}
		uint remaining;
		if (!parseUnsignedInt(argv[3], remaining) || INT_MAX < remaining) {
			debugPrintf("Invalid remaining chances '%s'. Must be a positive integer.\n\n", argv[3]);
			return true;
		}
		if (!puzzle || !puzzle->debugCanSetChances() || info.opportunities < 0) {
			debugPrintf("The active page state does not support adjusting chances.\n\n");
			return true;
		}
		if (remaining == 0 || static_cast<uint>(info.opportunities) < remaining) {
			debugPrintf("Remaining chances must be between 1 and %d.\n\n", info.opportunities);
			return true;
		}
		if (!puzzle->debugSetChances(static_cast<int>(remaining))) {
			debugPrintf("The active puzzle cannot adjust chances in its current state.\n\n");
			return true;
		}
		info = puzzle->debugGetChances();
		debugPrintf("Chances left set to %d.\n\n", info.chancesLeft());
	}
	debugPrintf("Chance type: %s\n", PuzzleChanceInfo::typeName(info.type));
	if (0 <= info.opportunities) {
		debugPrintf("  One chance is used per [%s].\n", info.unit ? info.unit : "attempt");
		debugPrintf("  Opportunities: %d\n  Chances left:  %d\n  Chances used:  %d\n", info.opportunities, info.chancesLeft(), info.used);
	} else {
		switch (info.type) {
		case PuzzleChanceInfo::Type::kInfinite:
			debugPrintf("  The player can try without any chance limitation.\n");
			break;
		case PuzzleChanceInfo::Type::kNone:
			debugPrintf("  This page is not a puzzle.\n");
			break;
		default:
			debugPrintf("  This puzzle has no single countable chance budget.\n");
			break;
		}
	}
	if (puzzle)
		debugPrintf("%s", puzzle->debugGetChanceDetails().c_str());
	debugPrintf("\n");
	return true;
}

bool Zoombini2Console::parseDebugResourcePath(const char *text, Common::Path &path) {
	Common::String normalized(text);
	if (!normalized.empty() && normalized[0] == '@') {
		uint32 handle = 0;
		if (!parseUnsignedInt(text + 1, handle))
			return false;
		const Common::Array<Zoombini2Engine::PageResource> &resources = _vm->getPageResources();
		if (handle == 0 || resources.size() < handle) {
			debugPrintf("Unknown resource handle '%s'. Use list resource for this page's handles.\n\n", text);
			return false;
		}
		normalized = resources[handle - 1].path.toString('/');
	}
	normalized.replace('\\', '/');
	Common::String relative = normalized;
	if (!relative.empty() && relative[0] == '#')
		relative = relative.substr(1);
	if (relative.empty() || relative[0] == '/' || relative.contains(':') || relative == ".." || relative.hasPrefix("../") ||
		relative.contains("/../") || relative.hasSuffix("/..")) {
		debugPrintf("Use a relative game resource path; absolute paths and parent traversal are unavailable.\n\n");
		return false;
	}
	path = Common::Path(normalized, '/');
	return true;
}

void Zoombini2Console::printResourcePathHelp() {
	debugPrintf("Use list resources to view the currently loaded resources.\n");
	debugPrintf("A listed @number can replace <path>; handles reset on page transitions.\n\n");
}

bool Zoombini2Console::CmdSub_DrawImage(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc != 3) {
		debugPrintf("Preview a decoded bitmap over white in the debug dialog.\n");
		debugPrintf("Usage: %s %s <path>\n\n", kCmdDraw, kSubCmdDrawImage);
		debugPrintf("Use a logical .bb/.bmp path or an extensionless bitmap name.\n");
		debugPrintf("Prefix installed resources with #. Large images are clipped to the game screen.\n");
		debugPrintf("Click or press ESC to close the preview.\n\n");
		printResourcePathHelp();
		printHelpOption();
		return true;
	}
	Common::Path path;
	if (!parseDebugResourcePath(argv[2], path))
		return true;
	DialogDebugCommand cmd;
	cmd.setDrawImage(path);
	if (!_vm->openDebugDialog(cmd)) {
		debugPrintf("Cannot open image '%s', or another dialog is active.\n\n", path.toString('/').c_str());
		return true;
	}
	return false;
}

bool Zoombini2Console::CmdSub_DrawMsgBox(int argc, const char **argv) {
	if ((argc == 3 && isHelpOption(argv[2])) || (argc != 3 && argc != 4)) {
		debugPrintf("Preview a confirmation dialog.\n");
		debugPrintf("Usage: %s %s <path>\n", kCmdDraw, kSubCmdDrawMsgBox);
		debugPrintf("       %s %s --text \"message\"\n\n", kCmdDraw, kSubCmdDrawMsgBox);
		debugPrintf("The preview has no game callback. Both buttons only close it.\n\n");
		printResourcePathHelp();
		printHelpOption();
		return true;
	}
	bool queued = false;
	if (argc == 4 && scumm_stricmp(argv[2], "--text") == 0) {
		const Common::U32String text(argv[3], Common::kUtf8);
		if (text.empty()) {
			debugPrintf("The message text is empty.\n\n");
			return true;
		}
		queued = _vm->requestUiTextMsgBox(text, nullptr);
	} else if (argc == 3 && scumm_stricmp(argv[2], "--text") != 0) {
		Common::Path path;
		if (!parseDebugResourcePath(argv[2], path))
			return true;
		BitBlock image(_vm);
		if (!image.load(path)) {
			debugPrintf("Cannot decode message image '%s'.\n\n", path.toString('/').c_str());
			return true;
		}
		queued = _vm->requestMsgBox(path, nullptr);
	} else {
		debugPrintf("Use %s %s --help for the supported message forms.\n\n", kCmdDraw, kSubCmdDrawMsgBox);
		return true;
	}
	if (!queued) {
		debugPrintf("Cannot open the message preview while another dialog is active.\n\n");
		return true;
	}
	return false;
}

bool Zoombini2Console::loadDebugAnimation(const Common::Path &path, Animation &animation, RleBlock &sprite, bool &singleFrame) {
	singleFrame = path.toString('/').hasSuffixIgnoreCase(".rb");
	const bool loaded = singleFrame ? sprite.loadFromFile(path) : animation.loadFromFile(path);
	if (!loaded || (!singleFrame && animation.getFrameCount() == 0)) {
		debugPrintf("Cannot decode animation '%s'. Use an .an path, an extensionless animation name, or an explicit .rb path.\n\n",
					path.toString('/').c_str());
		return false;
	}
	return true;
}

bool Zoombini2Console::checkPNGExportSupport() {
#ifdef USE_PNG
	return true;
#else
	debugPrintf("Bitmap export requires a build with PNG support.\n\n");
	return false;
#endif
}

bool Zoombini2Console::validateDebugImageSize(const Size32 &size) {
	static constexpr uint64 kMaximumExportPixels = 32 * 1024 * 1024;
	if (size.width <= 0 || size.height <= 0 || INT16_MAX < size.width || INT16_MAX < size.height ||
		kMaximumExportPixels < static_cast<uint64>(size.width) * static_cast<uint64>(size.height)) {
		debugPrintf("Image dimensions %d x %d exceed the export surface limits.\n\n", size.width, size.height);
		return false;
	}
	return true;
}

Common::String Zoombini2Console::makeDebugDumpStem(const Common::String &name) {
	Common::String stem;
	for (uint i = 0; i < name.size() && stem.size() < 96; i++) {
		const char c = name[i];
		stem += Common::isAlnum(c) || c == '-' || c == '_' ? c : '_';
	}
	if (stem.empty())
		stem = "resource";
	return stem;
}

bool Zoombini2Console::openDebugDump(Common::DumpFile &out, const Common::String &stem, const char *extension, Common::Path &path) {
	for (uint sequence = 1; sequence <= 9999; sequence++) {
		path = Common::Path(Common::String::format(kDebugDumpPathFormat, stem.c_str(), sequence, extension), '/');
		if (Common::FSNode(path).exists())
			continue;
		if (out.open(path, true))
			return true;
		debugPrintf("Cannot create dump '%s'.\n\n", path.toString('/').c_str());
		return false;
	}
	debugPrintf("No unused numbered dump name remains for '%s'.\n\n", stem.c_str());
	return false;
}

bool Zoombini2Console::exportDebugSurface(const Graphics::Surface &surface, const Common::String &stem) {
#ifndef USE_PNG
	(void)surface;
	(void)stem;
	return false;
#else
	Common::DumpFile out;
	Common::Path path;
	if (!openDebugDump(out, stem, "png", path))
		return false;
	const bool written = Image::writePNG(out, surface);
	const bool okay = written && out.flush() && !out.err();
	out.close();
	debugPrintf("%s %s (%d x %d).\n", okay ? "Exported" : "Failed to export", path.toString('/').c_str(), surface.w, surface.h);
	return okay;
#endif
}

bool Zoombini2Console::Cmd_Dump(int argc, const char **argv) {
	if (argc < 2 || (argc == 2 && isHelpOption(argv[1]))) {
		debugPrintf("Export decoded Z2 resources or the live journey state.\n");
		debugPrintf("Usage: %s <image|animation|save> [arguments]\n\n", kCmdDump);
		debugPrintf("  image <path>                Export one bitmap over white as PNG.\n");
		debugPrintf("  animation <path>            Export every decoded frame over white as PNG. Alias: anm.\n");
		debugPrintf("  save                        Export the live journey in the .mk layout.\n\n");
		debugPrintf("Numbered files below dumps/ preserve existing exports.\n\n");
		printResourcePathHelp();
		printHelpOption();
		return true;
	}
	if (scumm_stricmp(argv[1], "image") == 0)
		return CmdSub_DumpImage(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDumpAnimation) == 0 || scumm_stricmp(argv[1], kSubCmdDumpAnimationAlias) == 0)
		return CmdSub_DumpAnimation(argc, argv);
	if (scumm_stricmp(argv[1], "save") == 0)
		return CmdSub_DumpSave(argc, argv);
	debugPrintf("Unknown dump subcommand '%s'. Use %s --help.\n\n", argv[1], kCmdDump);
	return true;
}

bool Zoombini2Console::CmdSub_DumpImage(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc != 3) {
		debugPrintf("Export one decoded bitmap composited over white.\n");
		debugPrintf("Usage: %s image <path>\n\n", kCmdDump);
		debugPrintf("The bitmap resolver prefers a matching .bb cache for .bmp or extensionless names.\n");
		debugPrintf("Bitmap resources are always exported as PNG and require PNG support in this build.\n\n");
		printResourcePathHelp();
		printHelpOption();
		return true;
	}
	if (!checkPNGExportSupport())
		return true;
	Common::Path path;
	if (!parseDebugResourcePath(argv[2], path))
		return true;
	BitBlock image(_vm);
	if (!image.load(path)) {
		debugPrintf("Cannot decode image '%s'.\n\n", path.toString('/').c_str());
		return true;
	}
	if (!validateDebugImageSize(image.getSize()))
		return true;
	Common::ScopedPtr<ManagedSurface32> surface(_vm->_gfx->createSurface(image.getSize()));
	surface->fillRect(Common::Rect(surface->w, surface->h), surface->format.ARGBToColor(255, 255, 255, 255));
	_vm->_gfx->drawBitBlock(surface.get(), &image, Common::Point32());
	exportDebugSurface(surface->rawSurface(), makeDebugDumpStem(path.toString('/')));
	debugPrintf("\n");
	return true;
}

bool Zoombini2Console::CmdSub_DumpAnimation(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc != 3) {
		debugPrintf("Export every decoded animation frame or one explicit RB sprite, composited over white.\n");
		debugPrintf("Usage: %s animation <path>\n\n", kCmdDump);
		debugPrintf("Alias: %s %s <path>\n\n", kCmdDump, kSubCmdDumpAnimationAlias);
		debugPrintf("Frame names use zero-based indices; all images are exported as PNG.\n");
		debugPrintf("PNG support is required in this build.\n");
		debugPrintf("Recoverable animation prefixes follow the existing decoder's salvage policy.\n\n");
		printResourcePathHelp();
		printHelpOption();
		return true;
	}
	if (!checkPNGExportSupport())
		return true;
	Common::Path path;
	if (!parseDebugResourcePath(argv[2], path))
		return true;
	Animation animation(_vm);
	RleBlock sprite(_vm);
	bool singleFrame = false;
	if (!loadDebugAnimation(path, animation, sprite, singleFrame))
		return true;
	const int count = singleFrame ? 1 : animation.getFrameCount();
	const Common::String stem = makeDebugDumpStem(path.toString('/'));
	int exported = 0;
	for (int i = 0; i < count; i++) {
		const RleBlock *frame = singleFrame ? &sprite : animation.getFrame(i);
		if (!frame || !validateDebugImageSize(frame->getSize()))
			break;
		Common::ScopedPtr<ManagedSurface32> surface(_vm->_gfx->createSurface(frame->getSize()));
		surface->fillRect(Common::Rect(surface->w, surface->h), surface->format.ARGBToColor(255, 255, 255, 255));
		_vm->_gfx->drawRleBlock(surface.get(), frame, Common::Point32());
		const Common::String frameStem = Common::String::format("%s_frame%04d", stem.c_str(), i);
		if (!exportDebugSurface(surface->rawSurface(), frameStem))
			break;
		exported += 1;
	}
	debugPrintf("Exported %d of %d decoded frames.\n\n", exported, count);
	return true;
}

bool Zoombini2Console::CmdSub_DumpSave(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc != 2) {
		debugPrintf("Export the live journey state using the existing .mk serializer.\n");
		debugPrintf("Usage: %s save\n\n", kCmdDump);
		debugPrintf("The export includes unsaved changes and follows the serializer's active-party rules.\n");
		debugPrintf("It writes a fresh dump file without writing the player's stored save.\n");
		debugPrintf("A named journey must be active; practice mode is unavailable.\n\n");
		printHelpOption();
		return true;
	}
	if (!_vm->_state || _vm->inPracticeMode() || _vm->_state->getPlayerName().empty()) {
		debugPrintf("No named journey is active.\n\n");
		return true;
	}
	Common::DumpFile out;
	Common::Path path;
	const Common::String stem = "state_" + makeDebugDumpStem(_vm->_state->getPlayerName());
	if (!openDebugDump(out, stem, "mk", path))
		return true;
	const bool okay = _vm->_state->save(&out) && out.flush() && !out.err();
	const int64 size = out.pos();
	out.close();
	debugPrintf("%s live state to %s (%lld bytes).\n\n", okay ? "Exported" : "Failed to export",
				path.toString('/').c_str(), static_cast<long long int>(size));
	return true;
}

bool Zoombini2Console::Cmd_List(int argc, const char **argv) {
	if (argc < 2 || (argc == 2 && isHelpOption(argv[1]))) {
		debugPrintf("List Z2 resource accesses and registered general-object runners.\n");
		debugPrintf("Usage: %s <resource|runners> [arguments]\n\n", kCmdList);
		debugPrintf("  resource [type] [filter]  List resources used during the current page lifetime.\n");
		debugPrintf("  runners                   List the current page-layer runner registry.\n\n");
		printResourcePathHelp();
		printHelpOption();
		return true;
	}
	if (scumm_stricmp(argv[1], "resource") == 0 || scumm_stricmp(argv[1], "resources") == 0)
		return CmdSub_ListResource(argc, argv);
	if (scumm_stricmp(argv[1], "runners") == 0)
		return CmdSub_ListRunners(argc, argv);
	debugPrintf("Unknown list subcommand '%s'. Use %s --help.\n\n", argv[1], kCmdList);
	return true;
}

const char *Zoombini2Console::getResourceTypeName(Zoombini2Engine::PageResourceType type) {
	for (uint i = 0; i < ARRAYSIZE(kResourceTypes); i++) {
		if (kResourceTypes[i].type == type)
			return kResourceTypes[i].name;
	}
	return "unknown";
}

bool Zoombini2Console::parseResourceType(const char *text, Zoombini2Engine::PageResourceType &type) {
	for (uint i = 0; i < ARRAYSIZE(kResourceTypes); i++) {
		const ResourceTypeInfo &info = kResourceTypes[i];
		if (scumm_stricmp(text, info.name) == 0 || scumm_stricmp(text, info.extension) == 0) {
			type = info.type;
			return true;
		}
	}
	return false;
}

bool Zoombini2Console::CmdSub_ListResource(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc < 2 || 4 < argc) {
		debugPrintf("List successful resource file opens and cache requests since the last page transition.\n");
		debugPrintf("Usage: %s resource [type] [filter]\n\n", kCmdList);
		debugPrintf("Types: bitmap (bb/bmp), sprite (rb), animation (an), zoombini (anm),\n");
		debugPrintf("       areamask, path (pat), sound (wav), video (bik), layout (ztl), unknown, all.\n");
		debugPrintf("Type names and extension aliases ignore case; all includes every type.\n");
		debugPrintf("The optional filter matches any part of the path, ignoring case.\n");
		debugPrintf("A single argument that is not a type is treated as a path filter.\n");
		debugPrintf("Paths name the resolved files, including their extensions.\n");
		debugPrintf("Includes dialogs and console previews; an opened file need not still have a live stream.\n");
		debugPrintf("Cache labels describe successful requests, including hits on shared caches.\n");
		debugPrintf("Counters show opens, successful graphics/PAT decodes, cache requests and cache hits.\n");
		debugPrintf("Graphics payload bytes include active background/mask and page/shared graphics caches, excluding allocation overhead.\n");
		debugPrintf("File opens and cache requests for the same resolved path share one entry.\n");
		debugPrintf("Filtering retains handle numbers; new resources append without renumbering them.\n");
		debugPrintf("The resources subcommand is an alias for resource.\n\n");
		printResourcePathHelp();
		printHelpOption();
		return true;
	}
	Zoombini2Engine::PageResourceType type = Zoombini2Engine::PageResourceType::kUnknown;
	bool filterByType = false;
	Common::String filter;
	if (2 < argc) {
		if (scumm_stricmp(argv[2], "all") == 0) {
			if (argc == 4)
				filter = argv[3];
		} else if (parseResourceType(argv[2], type)) {
			filterByType = true;
			if (argc == 4)
				filter = argv[3];
		} else if (argc == 3) {
			filter = argv[2];
		} else {
			debugPrintf("Unknown resource type '%s'. Use %s resource --help.\n\n", argv[2], kCmdList);
			return true;
		}
	}
	const Common::Array<Zoombini2Engine::PageResource> &resources = _vm->getPageResources();
	filter.toLowercase();
	debugPrintf("Page %d: %u recorded resource paths.\n", static_cast<int>(_vm->getActivePageId()), resources.size());
	uint shown = 0;
	for (uint i = 0; i < resources.size(); i++) {
		const Zoombini2Engine::PageResource &resource = resources[i];
		if (filterByType && resource.type != type)
			continue;
		const Common::String name = resource.path.toString('/');
		Common::String match(name);
		match.toLowercase();
		if (!filter.empty() && !match.contains(filter))
			continue;
		debugPrintf("  @%-3u [%s] [%s%s%s] %s\n", i + 1, getResourceTypeName(resource.type), resource.opened ? "open" : "",
					resource.opened && resource.cacheUsed ? ", " : "",
					resource.cacheUsed ? "cache" : "", name.c_str());
		debugPrintf("       opens=%u decodes=%u requests=%u hits=%u\n", resource.openCount, resource.decodeCount, resource.cacheRequests, resource.cacheHits);
		shown += 1;
	}
	debugPrintf("%u matching paths. Use @number in place of a resource path; handles reset on page transitions.\n\n", shown);
	debugPrintf("Retained graphics payload: %" PRIu64 " bytes (all graphics caches, independent of filter).\n\n", _vm->_gfx->getCachedDataSize());
	return true;
}

bool Zoombini2Console::CmdSub_ListRunners(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc != 2) {
		debugPrintf("List registered general-object animation runners without advancing them.\n");
		debugPrintf("Usage: %s runners\n\n", kCmdList);
		debugPrintf("Lists layer/index, position, active state, mode, timing/frame indices, and input flags.\n");
		debugPrintf("Standalone page-local runners are outside the layer registry. Use state get party for Zoombinis.\n\n");
		printHelpOption();
		return true;
	}
	if (!_vm->getActivePage() || !_vm->_gfx->getPageLayerStack()) {
		debugPrintf("No active page-layer registry.\n\n");
		return true;
	}
	const PageLayerStack *stack = _vm->_gfx->getPageLayerStack();
	debugPrintf("Page %d: %u layers; registered general-object runners only.\n", static_cast<int>(_vm->getActivePageId()), stack->getLayerCount());
	for (uint layerIndex = 0; layerIndex < stack->getLayerCount(); layerIndex++) {
		const PageLayer *layer = stack->getLayer(static_cast<int>(layerIndex));
		if (!layer)
			continue;
		debugPrintf("  layer %u: %u runners, scroll=%d\n", layerIndex, layer->getAnimationRunnerCount(), layer->getScrollX());
		for (uint i = 0; i < layer->getAnimationRunnerCount(); i++) {
			const AnimationRunner *runner = layer->getAnimationRunner(static_cast<int>(i));
			if (!runner)
				continue;
			const Common::Point32 position = runner->getPosition();
			const Common::Rect32 &hitRect = runner->getHitRect();
			debugPrintf("    [%u] pos=(%d,%d) active=%d mode=%d timed=%d frame=%d input=%d hitTest=%d\n", i, position.x, position.y,
						runner->isActive(), static_cast<int>(runner->getMode()), runner->getCurrentTimedEntryIndex(), runner->getCurrentFrameIndex(),
						runner->isInputEnabled(), runner->isHitTestEnabled());
			debugPrintf("         hitRect=(%d,%d,%d,%d)\n", hitRect.left, hitRect.top, hitRect.right, hitRect.bottom);
		}
	}
	debugPrintf("\n");
	return true;
}

bool Zoombini2Console::printDebugStateGroup(const Common::String &group) {
	const GameState &state = *_vm->_state;
	if (group.equalsIgnoreCase("page")) {
		debugPrintf("page: current=%d gameplay=%d level=%d mode=%s\n", static_cast<int>(_vm->getActivePageId()),
					static_cast<int>(state._currentPage), state.getLevel(), _vm->inPracticeMode() ? "practice" : "journey");
	} else if (group.equalsIgnoreCase("progress")) {
		debugPrintf("progress: rescue1=%u rescue2=%u booliewood=%u rescuedBoolies=%d rescue1Arrivals=%d completedZoombinis=%d\n",
					state._hasReachedRescue1, state._hasReachedRescue2, state._hasReachedBooliewood, state._rescuedBoolieCount,
					state._rescue1ArrivalCount, state._completedZoombiniCount);
		for (int i = 0; i < state.getPageLevelCount(); i++) {
			const PageId page = static_cast<PageId>(i);
			const int level = state.getPageLevel(page);
			const uint visit1 = state.getPageVisitCount(page, 1);
			const uint visit2 = state.getPageVisitCount(page, 2);
			const uint visit3 = state.getPageVisitCount(page, 3);
			if (level == 0 && state._perfectClearCount[i] == 0 && visit1 == 0 && visit2 == 0 && visit3 == 0)
				continue;
			debugPrintf("  page %d: level=%d perfectClears=%d visits=(%u,%u,%u)\n", i, level, state._perfectClearCount[i],
						visit1, visit2, visit3);
		}
	} else if (group.equalsIgnoreCase("population")) {
		const Zoombini2PopulationSummary population = state.getPopulationSummary();
		debugPrintf("population: Zombiniville=%d rescue1=%d rescue2=%d Booliewood=%d savedRoster=%d\n", population._zombinivilleCount,
					population._rescue1Count, population._rescue2Count, population._booliewoodCount, population._activePartyCount);
	} else if (group.equalsIgnoreCase("party")) {
		debugPrintf("party: live=%u saved=%u\n", state._activeZoombinis.size(), state._savedRoster.size());
		for (uint i = 0; i < state._activeZoombinis.size(); i++) {
			const ZoombiniRunner *zoombini = state._activeZoombinis[i];
			if (!zoombini) {
				debugPrintf("  [%u] unavailable\n", i);
				continue;
			}
			const Common::String encodedName = zoombini->getName();
			const Common::String name = TextManager::toU32String(encodedName, _vm->getLanguage()).encode(Common::kUtf8);
			const ZmbTrait &traits = zoombini->getTraits();
			debugPrintf("  [%u] %s: feet=%u nose=%u hair=%u eyes=%u\n", i, name.c_str(), traits._feet, traits._nose, traits._hair, traits._eyes);
		}
	} else if (group.equalsIgnoreCase("audio")) {
		debugPrintf("audio: music=%d sfx=%d speech=%d\n", _vm->getMusicVolume(), _vm->getSFXVolume(), _vm->getSpeechVolume());
	} else if (group.equalsIgnoreCase("rng")) {
		if (_vm->_rnd)
			debugPrintf("rng: current state=0x%08X\n", _vm->_rnd->getSeed());
		else
			debugPrintf("rng: unavailable\n");
	} else {
		debugPrintf("Unknown state group '%s'. Use %s --help.\n", group.c_str(), kCmdState);
		return false;
	}
	return true;
}

bool Zoombini2Console::Cmd_State(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || (argc == 2 && scumm_stricmp(argv[1], "help") == 0)) {
		debugPrintf("Read the current Z2 game and runtime state.\n");
		debugPrintf("Usage: %s [get [page|progress|population|party|audio|rng]]\n\n", kCmdState);
		debugPrintf("The default command prints every group. Queries do not advance RNG or mutate game state.\n");
		debugPrintf("The party group lists live actors; population includes the retained saved-roster count.\n\n");
		printHelpOption();
		return true;
	}
	if (3 < argc || (1 < argc && scumm_stricmp(argv[1], "get") != 0)) {
		debugPrintf("Usage: %s [get [page|progress|population|party|audio|rng]]\n\n", kCmdState);
		return true;
	}
	if (!_vm->_state) {
		debugPrintf("Game state is unavailable.\n\n");
		return true;
	}
	if (argc == 3) {
		printDebugStateGroup(argv[2]);
	} else {
		static constexpr const char *kGroups[] = {
			"page",
			"progress",
			"population",
			"party",
			"audio",
			"rng"};
		for (const char *group : kGroups)
			printDebugStateGroup(group);
	}
	debugPrintf("\n");
	return true;
}

bool Zoombini2Console::Cmd_ManShortcuts(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc != 1) {
		debugPrintf("Print the keyboard shortcuts currently implemented by Z2.\n");
		debugPrintf("Usage: %s\n\n", kCmdManShortcuts);
		printHelpOption();
		return true;
	}
	debugPrintf("Zoombinis Mountain Rescue keyboard shortcuts\n\n");
	debugPrintf("[Global]\n");
	debugPrintf("  F5          Open the ScummVM global menu.\n");
	debugPrintf("  Ctrl+K      Toggle the selected/active save's automatic-write lock when the option is enabled (%s).\n",
				_vm->isSavefileReadOnlyToggleEnabled() ? "enabled" : "disabled");
	debugPrintf("  F2/F3/P/C   See %s for the optional original developer keys.\n\n", kCmdManBuiltinDebug);
	debugPrintf("[Sign-in]\n");
	debugPrintf("  Up/Down     Change the selected save.\n");
	debugPrintf("  PgUp/PgDn   Scroll the save list.\n");
	debugPrintf("  Enter       Start the selected save or confirm the edited name.\n");
	debugPrintf("  Backspace   Remove the last name character while editing.\n");
	debugPrintf("  Delete      Request deletion confirmation for the selected save.\n\n");
	debugPrintf("[Map transition]\n");
	debugPrintf("  Space       Finish the current transition.\n\n");
	debugPrintf("[Debug previews]\n");
	debugPrintf("  Esc/Click   Close the debug dialog.\n");
	debugPrintf("  Left/Right  Step animation frames.\n\n");
	debugPrintf("[Enhanced shortcuts] (%s)\n", _vm->useEnhancedKbdShortcuts() ? "enabled" : "disabled");
	debugPrintf("  Map:        Ctrl+P switches journey/practice modes; 1-3 select practice level.\n");
	debugPrintf("              4 selects the restored practice level where supported.\n");
	debugPrintf("  Zombiniville: Delete removes the held Zoombini; Shift+Delete removes the whole party.\n");
	debugPrintf("  Message box: Enter accepts; Esc cancels.\n\n");
	debugPrintf("Map volume-panel Esc cancels that panel. The help overlay uses its on-screen controls.\n\n");
	return true;
}

} // End of namespace Zoombini2
