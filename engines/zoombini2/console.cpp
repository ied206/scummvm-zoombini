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

#include "zoombini2/pages/dialog_debug.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

Zoombini2Console::Zoombini2Console(Zoombini2Engine *vm) : GUI::Debugger(), _vm(vm) {
	registerCmd(kCmdDraw, WRAP_METHOD(Zoombini2Console, Cmd_Draw));
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

bool Zoombini2Console::parseNonNegativeInt(const char *str, int &result) {
	if (!str || *str == '\0')
		return false;

	uint value = 0;
	for (const char *p = str; *p != '\0'; p++) {
		if (*p < '0' || '9' < *p)
			return false;
		const uint digit = static_cast<uint>(*p - '0');
		if ((0xFFFFFFFFu - digit) / 10u < value)
			return false;
		value = value * 10u + digit;
	}
	if (0x7FFFFFFFu < value)
		return false;

	result = static_cast<int>(value);
	return true;
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
	if (!_vm->getDebugDialog()->open(cmd))
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

	int startFrame = 0;
	if (argc == 4 && !parseNonNegativeInt(argv[3], startFrame)) {
		debugPrintf("Cannot parse argument %s\n", argv[3]);
		debugPrintf("\n");
		return true;
	}

	DialogDebugCommand cmd;
	cmd.setDrawAnimation(Common::Path(argv[2]), startFrame);
	if (!_vm->getDebugDialog()->open(cmd)) {
		debugPrintf("Cannot open animation '%s' at frame %d\n", argv[2], startFrame);
		debugPrintf("\n");
		return true;
	}
	return false;
}

} // End of namespace Zoombini2
