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

#include "common/textconsole.h"
#include "common/util.h"

#include "zoombini2/pages/dialog_debug.h"
#include "zoombini2/pages/interactive_map.h"
#include "zoombini2/pages/puzzle_base.h"
#include "zoombini2/pages/transition_maptrans.h"
#include "zoombini2/scripts.h"
#include "zoombini2/zoombini2.h"

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

namespace Zoombini2 {

namespace {

/** Parse bounded decimal or 0x-prefixed integer arguments for debugger commands. */
class ConsoleArgumentParser {
public:
	static bool parseSignedInt(GUI::Debugger &debugger, const char *str, int32 &result);
	static bool parseUnsignedInt(GUI::Debugger &debugger, const char *str, uint32 &result);

private:
	static bool reportFailure(GUI::Debugger &debugger, const char *type, const char *str);
};

bool ConsoleArgumentParser::reportFailure(GUI::Debugger &debugger, const char *type, const char *str) {
	debugger.debugPrintf("Cannot parse %s(%s) (hex supported with 0x prefix)\n", type, str ? str : "(null)");
	return false;
}

bool ConsoleArgumentParser::parseSignedInt(GUI::Debugger &debugger, const char *str, int32 &result) {
	if (!str || str[0] == '\0') {
		warning("parseSignedInt: Empty string\n");
		return reportFailure(debugger, "int32", str);
	}

	const char *digits = str;
	while (Common::isSpace(digits[0]))
		digits += 1;

	// Check if it's a hexadecimal number (starts with "0x" or "0X")
	int base = 10;
	if (digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X'))
		base = 16;

	char *endPtr = nullptr;
	errno = 0;
	const long parsed = strtol(str, &endPtr, base);

	// Check for conversion errors
	if (errno == ERANGE) {
		warning("parseSignedInt: int32 overflow or underflow in '%s'\n", str);
		return reportFailure(debugger, "int32", str);
	}
	if (errno != 0) {
		warning("parseSignedInt: int32 conversion failed in '%s'\n", str);
		return reportFailure(debugger, "int32", str);
	}

	// Check if any characters were converted
	if (endPtr == str) {
		warning("parseSignedInt: '%s' is not a valid int32\n", str);
		return reportFailure(debugger, "int32", str);
	}

	// Check if there are trailing characters
	if (*endPtr != '\0') {
		warning("parseSignedInt: '%s' contains invalid characters\n", str);
		return reportFailure(debugger, "int32", str);
	}
	if (parsed < INT32_MIN || INT32_MAX < parsed) {
		warning("parseSignedInt: int32 overflow or underflow in '%s'\n", str);
		return reportFailure(debugger, "int32", str);
	}

	result = static_cast<int32>(parsed);
	return true;
}

bool ConsoleArgumentParser::parseUnsignedInt(GUI::Debugger &debugger, const char *str, uint32 &result) {
	if (!str || str[0] == '\0') {
		warning("parseUnsignedInt: Empty string\n");
		return reportFailure(debugger, "uint32", str);
	}

	const char *digits = str;
	while (Common::isSpace(digits[0]))
		digits += 1;
	if (digits[0] == '-') {
		warning("parseUnsignedInt: '%s' is not a valid uint32\n", str);
		return reportFailure(debugger, "uint32", str);
	}

	// Check if it's a hexadecimal number (starts with "0x" or "0X")
	int base = 10;
	if (digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X'))
		base = 16;

	char *endPtr = nullptr;
	errno = 0;
	const unsigned long parsed = strtoul(str, &endPtr, base);

	// Check for conversion errors
	if (errno == ERANGE) {
		warning("parseUnsignedInt: uint32 overflow in '%s'\n", str);
		return reportFailure(debugger, "uint32", str);
	}
	if (errno != 0) {
		warning("parseUnsignedInt: uint32 conversion failed in '%s'\n", str);
		return reportFailure(debugger, "uint32", str);
	}

	// Check if any characters were converted
	if (endPtr == str) {
		warning("parseUnsignedInt: '%s' is not a valid uint32\n", str);
		return reportFailure(debugger, "uint32", str);
	}

	// Check if there are trailing characters
	if (*endPtr != '\0') {
		warning("parseUnsignedInt: '%s' contains invalid characters\n", str);
		return reportFailure(debugger, "uint32", str);
	}
	if (UINT32_MAX < parsed) {
		warning("parseUnsignedInt: uint32 overflow in '%s'\n", str);
		return reportFailure(debugger, "uint32", str);
	}

	result = static_cast<uint32>(parsed);
	return true;
}

} // End of anonymous namespace

const Zoombini2Console::GoDestination Zoombini2Console::kGoDestinations[] = {
	{"crazyturtle", kPageCrazyTurtle, kPageZombiniville, Zoombini2Engine::RouteBranch::kNone00},
	{"waterslide", kPageWaterslide, kPageCrazyTurtle, Zoombini2Engine::RouteBranch::kNone00},
	{"aquacube", kPageAquacube, kPageWaterslide, Zoombini2Engine::RouteBranch::kNone00},
	{"rescue1", kPageRescue1, kPageAquacube, Zoombini2Engine::RouteBranch::kNone00},
	{"mysticmarsh", kPageMysticMarsh, kPageRescue1, Zoombini2Engine::RouteBranch::kRight02},
	{"magicwall", kPageMagicWall, kPageRescue1, Zoombini2Engine::RouteBranch::kLeft01},
	{"walloffleens", kPageWallOfFleens, kPageMysticMarsh, Zoombini2Engine::RouteBranch::kNone00},
	{"cheznorf", kPageChezNorf, kPageMagicWall, Zoombini2Engine::RouteBranch::kNone00},
	{"rescue2", kPageRescue2, kPageWallOfFleens, Zoombini2Engine::RouteBranch::kNone00},
	{"rescue2norf", kPageRescue2, kPageChezNorf, Zoombini2Engine::RouteBranch::kNone00},
	{"snowboard", kPageSnowboard, kPageRescue2, Zoombini2Engine::RouteBranch::kNone00},
	{"boolies", kPageBoolies, kPageSnowboard, Zoombini2Engine::RouteBranch::kNone00},
	{"booliewood", kPageBooliewood, kPageBoolies, Zoombini2Engine::RouteBranch::kNone00},
	{"final", kPageFinal, kPageBoolies, Zoombini2Engine::RouteBranch::kNone00},
};

Zoombini2Console::Zoombini2Console(Zoombini2Engine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd(kCmdGo, WRAP_METHOD(Zoombini2Console, Cmd_Go));
	registerCmd(kCmdDraw, WRAP_METHOD(Zoombini2Console, Cmd_Draw));
	registerCmd(kCmdBuiltinDebug, WRAP_METHOD(Zoombini2Console, Cmd_BuiltinDebug));
	registerCmd(kCmdManBuiltinDebug, WRAP_METHOD(Zoombini2Console, Cmd_ManBuiltinDebug));
	registerCmd("puzzle", WRAP_METHOD(Zoombini2Console, Cmd_Puzzle));
	registerCmd("page", WRAP_METHOD(Zoombini2Console, Cmd_Puzzle));
}

Zoombini2Console::~Zoombini2Console() {
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

const Zoombini2Console::GoDestination *Zoombini2Console::findGoDestPage(const char *name, bool puzzleOnly) {
	int pageNumber = -1;
	const char *digits = name;
	while (Common::isSpace(digits[0]))
		digits += 1;
	bool numeric = false;
	if (Common::isDigit(digits[0]) || digits[0] == '+' || digits[0] == '-')
		numeric = ConsoleArgumentParser::parseSignedInt(*this, name, pageNumber);
	for (uint i = 0; i < ARRAYSIZE(kGoDestinations); i++) {
		const GoDestination &destination = kGoDestinations[i];
		if (puzzleOnly && InteractiveMap::getPracticePartySize(destination.target) == 0)
			continue;
		if (scumm_stricmp(name, destination.name) == 0 || (numeric && static_cast<int>(destination.target) == pageNumber))
			return &destination;
	}
	return nullptr;
}

void Zoombini2Console::printGoDestinations(bool puzzleOnly) {
	for (uint i = 0; i < ARRAYSIZE(kGoDestinations); i++) {
		const GoDestination &destination = kGoDestinations[i];
		if (puzzleOnly && InteractiveMap::getPracticePartySize(destination.target) == 0)
			continue;
		debugPrintf("  %-15s page %2d%s\n", destination.name, static_cast<int>(destination.target),
					puzzleOnly && Zoombini2Engine::supportsInternalPracticeLevel4(destination.target) ? " (internal level 4)" : "");
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
		debugPrintf("Puzzle accepts a name or page number. Level is 1-3, or internal 4 where listed.\n");
		debugPrintf("Count defaults to the route party size: 16 before Rescue I, 8 afterward.\n");
		debugPrintf("Available puzzles:\n");
		printGoDestinations(true);
		debugPrintf("\n");
		return true;
	}
	if (!_vm->_state || !_vm->getCurrentPage()) {
		debugPrintf("No active Zoombini2 game page.\n\n");
		return true;
	}
	const GoDestination *destination = findGoDestPage(argv[2], true);
	if (!destination) {
		debugPrintf("Unknown puzzle '%s'. Use go practice without arguments for the list.\n\n", argv[2]);
		return true;
	}
	uint level = 0;
	if (!ConsoleArgumentParser::parseUnsignedInt(*this, argv[3], level) || level < 1 || 4 < level) {
		debugPrintf("Invalid level '%s'. Must be 1-4.\n\n", argv[3]);
		return true;
	}
	if (level == 4 && !Zoombini2Engine::supportsInternalPracticeLevel4(destination->target)) {
		debugPrintf("Page %d has no safely playable internal level 4.\n\n", static_cast<int>(destination->target));
		return true;
	}
	const uint maxPartySize = InteractiveMap::getPracticePartySize(destination->target);
	uint count = maxPartySize;
	if (argc == 5 && (!ConsoleArgumentParser::parseUnsignedInt(*this, argv[4], count) || count < 1 || maxPartySize < count)) {
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
		debugPrintf("Without a level, the current saved-adventure or practice difficulty is retained.\n");
		debugPrintf("Available destinations:\n");
		printGoDestinations(false);
		debugPrintf("\n");
		return true;
	}
	if (!_vm->_state || !_vm->getCurrentPage()) {
		debugPrintf("No active Zoombini2 game page.\n\n");
		return true;
	}
	const GoDestination *destination = findGoDestPage(argv[2], false);
	if (!destination) {
		debugPrintf("Unknown destination '%s'. Use go xfer without arguments for the list.\n\n", argv[2]);
		return true;
	}
	uint level = 0;
	if (argc == 4 && (!ConsoleArgumentParser::parseUnsignedInt(*this, argv[3], level) || level < 1 || 4 < level)) {
		debugPrintf("Invalid level '%s'. Must be 1-4.\n\n", argv[3]);
		return true;
	}
	if (level == 4 && !Zoombini2Engine::supportsInternalPracticeLevel4(destination->target)) {
		debugPrintf("Page %d has no safely playable internal level 4.\n\n", static_cast<int>(destination->target));
		return true;
	}
	if (InteractiveMap::getPracticePartySize(destination->target) == 0 && (level != 0 || !_vm->_isSavedGame)) {
		debugPrintf("Practice transitions need a puzzle destination; shelters have no practice level.\n\n");
		return true;
	}
	if (level == 0 && !_vm->_isSavedGame && _vm->_state->getLevel() == 4 && !Zoombini2Engine::supportsInternalPracticeLevel4(destination->target) &&
			InteractiveMap::getPracticePartySize(destination->target) != 0) {
		debugPrintf("Current practice level 4 is unavailable for page %d.\n\n", static_cast<int>(destination->target));
		return true;
	}
	const int rescuedBoolies = level == 0 ? _vm->_state->_rescuedBoolieCount : 0;
	const PageId routedTarget = TransitionMapTrans::getDestPage(destination->source, destination->branch, rescuedBoolies);
	if (routedTarget != destination->target) {
		debugPrintf("Destination page %d is not reachable with the current rescue count.\n\n", static_cast<int>(destination->target));
		return true;
	}
	if (level != 0) {
		_vm->_isSavedGame = false;
		_vm->_debugXferPracticeLevel = static_cast<int>(level);
		_vm->_debugXferPracticeTarget = destination->target;
		_vm->_debugXferResetState = true;
	} else if (_vm->_isSavedGame && dynamic_cast<PuzzleBase *>(_vm->getCurrentPage())) {
		for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
			if (_vm->_state->_activeZoombinis[i])
				_vm->_state->_activeZoombinis[i]->setCanAdvanceFromPage(true);
		}
	} else if (!_vm->_isSavedGame) {
		_vm->_debugXferPracticeLevel = _vm->_state->getLevel();
		_vm->_debugXferPracticeTarget = destination->target;
	}
	_vm->_mapTransitionSourcePageId = destination->source;
	_vm->_routeDirection = destination->branch;
	_vm->_debugXferDestination = InteractiveMap::getPracticePartySize(destination->target) != 0 ? destination->target : destination->source;
	_vm->requestPageChange(kPageMapTrans);
	debugPrintf("Jumping through the route transition to page %d.\n\n", static_cast<int>(destination->target));
	return false;
}

bool Zoombini2Console::Cmd_Draw(int argc, const char **argv) {
	if (argc == 2 && isHelpOption(argv[1])) {
		debugPrintf("Inspect debug views of the active page and resources.\n");
		debugPrintf("Usage: %s <subcommand> [arguments]\n\n", kCmdDraw);
		debugPrintf("Subcommands:\n");
		debugPrintf("  %s\n", kSubCmdDrawAreaMask);
		debugPrintf("      Show the active page only where its area mask accepts drops,\n");
		debugPrintf("      masking the rest with black.\n\n");
		debugPrintf("  %s <path> [start-frame]\n", kSubCmdDrawAnimation);
		debugPrintf("      Show frames of an animation (.an) or sprite (.rb) resource.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 2) {
		debugPrintf("Usage: %s <%s|%s> [arguments]\n", kCmdDraw, kSubCmdDrawAreaMask, kSubCmdDrawAnimation);
		debugPrintf("\n");
		return true;
	}

	if (scumm_stricmp(argv[1], kSubCmdDrawAreaMask) == 0)
		return CmdSub_DrawAreaMask(argc, argv);
	if (scumm_stricmp(argv[1], kSubCmdDrawAnimation) == 0)
		return CmdSub_DrawAnimation(argc, argv);

	debugPrintf("Unknown %s subcommand '%s'.\n", kCmdDraw, argv[1]);
	debugPrintf("Usage: %s <%s|%s> [arguments]\n", kCmdDraw, kSubCmdDrawAreaMask, kSubCmdDrawAnimation);
	debugPrintf("\n");
	return true;
}

bool Zoombini2Console::CmdSub_DrawAreaMask(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Show the active page through its drop-acceptance area mask in the debug dialog.\n");
		debugPrintf("Usage: %s %s\n\n", kCmdDraw, kSubCmdDrawAreaMask);
		debugPrintf("Only mask bytes with a marked bit remain visible.\n");
		debugPrintf("The other pixels are masked with black.\n");
		debugPrintf("If the active page has no area mask, the debug display is\n");
		debugPrintf("fully black and shows a diagnostic message.\n");
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

	if (!_vm->getCurrentPage()) {
		debugPrintf("No active Zoombini2 page\n");
		debugPrintf("\n");
		return true;
	}

	DialogDebugCommand cmd;
	cmd.setDrawAreaMask();
	if (!_vm->openDebugDialog(cmd))
		return true;
	return false;
}

bool Zoombini2Console::CmdSub_DrawAnimation(int argc, const char **argv) {
	if (hasHelpOption(argc, argv)) {
		debugPrintf("Show one frame of an animation or sprite resource in the debug dialog.\n");
		debugPrintf("Usage: %s %s <path> [start-frame]\n\n", kCmdDraw, kSubCmdDrawAnimation);
		debugPrintf("<path> is an animation (.an) or sprite (.rb) resource path,\n");
		debugPrintf("with or without its extension. Frames are numbered from 0;\n");
		debugPrintf("[start-frame] defaults to 0. Use the Left/Right keys to\n");
		debugPrintf("step frames. Click or press ESC while the dialog is shown\n");
		debugPrintf("to close it.\n\n");
		debugPrintf("Options:\n");
		printHelpOption();
		return true;
	}

	if (argc < 3 || 4 < argc) {
		debugPrintf("Usage: %s %s <path> [start-frame]\n", kCmdDraw, kSubCmdDrawAnimation);
		debugPrintf("\n");
		return true;
	}

	uint startFrame = 0;
	if (argc == 4 && (!ConsoleArgumentParser::parseUnsignedInt(*this, argv[3], startFrame) || INT_MAX < startFrame)) {
		debugPrintf("Cannot parse argument %s\n", argv[3]);
		debugPrintf("\n");
		return true;
	}

	DialogDebugCommand cmd;
	cmd.setDrawAnimation(Common::Path(argv[2]), static_cast<int>(startFrame));
	if (!_vm->openDebugDialog(cmd)) {
		debugPrintf("Cannot open animation '%s' at frame %u\n", argv[2], startFrame);
		debugPrintf("\n");
		return true;
	}
	return false;
}

bool Zoombini2Console::Cmd_Puzzle(int argc, const char **argv) {
	if (argc < 2 || isHelpOption(argv[1])) {
		debugPrintf("Inspect or control the active puzzle.\nUsage: page|puzzle <subcommand> [arguments]\n\n");
		debugPrintf("  finish                       Accept the party and depart.\n");
		debugPrintf("  answer                       Print the generated rules/answer without changing state.\n");
		debugPrintf("  chance|chances [get]          Show remaining chances and other resource budgets.\n");
		debugPrintf("  chance|chances set <remaining> Set a supported finite budget.\n\n");
		const PuzzleBase *puzzle = dynamic_cast<PuzzleBase *>(_vm->getCurrentPage());
		const char *chanceSupport = "unsupported";
		if (puzzle && puzzle->debugCanSetChances())
			chanceSupport = "supported";
		debugPrintf("Chance setting is %s in the active page state.\n", chanceSupport);
		debugPrintf("Commands work in practice and saved adventures, independently of the in-game debug hotkey flag.\n");
		debugPrintf("Use puzzle <subcommand> --help for details.\n\n");
		return true;
	}
	if (scumm_stricmp(argv[1], "finish") == 0)
		return CmdSub_PuzzleFinish(argc, argv);
	if (scumm_stricmp(argv[1], "answer") == 0)
		return CmdSub_PuzzleAnswer(argc, argv);
	if (scumm_stricmp(argv[1], "chance") == 0 || scumm_stricmp(argv[1], "chances") == 0)
		return CmdSub_PuzzleChance(argc, argv);
	debugPrintf("Unknown puzzle subcommand '%s'. Use puzzle --help.\n\n", argv[1]);
	return true;
}

bool Zoombini2Console::CmdSub_PuzzleFinish(int argc, const char **argv) {
	if (hasHelpOption(argc, argv) || argc != 2) {
		debugPrintf("Usage: page|puzzle finish\nAccept the whole party and trigger regular departure.\n");
		debugPrintf("Practice returns to its map; saved adventures retain the accepted party and advance along the route.\n\n");
		return true;
	}
	PuzzleBase *puzzle = dynamic_cast<PuzzleBase *>(_vm->getCurrentPage());
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
	const PuzzleBase *puzzle = dynamic_cast<PuzzleBase *>(_vm->getCurrentPage());
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
	PuzzleBase *puzzle = dynamic_cast<PuzzleBase *>(_vm->getCurrentPage());
	const bool query = argc == 2 || (argc == 3 && scumm_stricmp(argv[2], "get") == 0);
	const bool set = 2 < argc && scumm_stricmp(argv[2], "set") == 0;
	if (hasHelpOption(argc, argv) || (!query && !set)) {
		debugPrintf("Usage: page|puzzle chance|chances [get|set <remaining>]\n");
		debugPrintf("Without an argument, show the chance model and remaining count.\n");
		debugPrintf("Set accepts a decimal or 0x-prefixed integer from zero through the opportunity limit.\n");
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
		if (!ConsoleArgumentParser::parseUnsignedInt(*this, argv[3], remaining) || INT_MAX < remaining) {
			debugPrintf("Invalid remaining chances '%s'. Must be a non-negative integer.\n\n", argv[3]);
			return true;
		}
		if (!puzzle || !puzzle->debugCanSetChances() || info.opportunities < 0) {
			debugPrintf("The active page state does not support adjusting chances.\n\n");
			return true;
		}
		if (info.opportunities < 0 || static_cast<uint>(info.opportunities) < remaining) {
			debugPrintf("Remaining chances must be between 0 and %d.\n\n", info.opportunities);
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
	} else if (info.type == PuzzleChanceInfo::Type::kInfinite) {
		debugPrintf("  The player can try without any chance limitation.\n");
	} else if (info.type == PuzzleChanceInfo::Type::kNone) {
		debugPrintf("  This page is not a puzzle.\n");
	} else {
		debugPrintf("  This puzzle has no single countable chance budget.\n");
	}
	if (puzzle)
		debugPrintf("%s", puzzle->debugGetChanceDetails().c_str());
	debugPrintf("\n");
	return true;
}

} // End of namespace Zoombini2
