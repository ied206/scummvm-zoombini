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

#ifndef ZOOMBINI2_CONSOLE_H
#define ZOOMBINI2_CONSOLE_H

#include "gui/debugger.h"

#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

enum PageId : int;

/**
 * Debug console for the Mountain Rescue engine.
 *
 * The @ref draw command mirrors the Zoombini 1 console layout so later
 * resource subcommands slot into the same dispatcher. The first
 * subcommand visualizes the drop-acceptance area mask, which plays the
 * role of the original walkability terrain bitmap.
 */
class Zoombini2Console : public GUI::Debugger {
public:
	/** Bind the console to @p vm without taking ownership. */
	explicit Zoombini2Console(Zoombini2Engine *vm);
	/** Release the console without touching the bound engine. */
	~Zoombini2Console() override;

private:
	struct GoDestination {
		const char *name;
		PageId target;
		PageId source;
		Zoombini2Engine::RouteBranch branch;
	};
	static const GoDestination kGoDestinations[];
	static constexpr const char *kCmdGo = "go";
	static constexpr const char *kSubCmdGoXfer = "xfer";
	static constexpr const char *kSubCmdGoPractice = "practice";
	bool Cmd_Go(int argc, const char **argv);
	bool CmdSub_GoXfer(int argc, const char **argv);
	bool CmdSub_GoPractice(int argc, const char **argv);
	const GoDestination *findGoDestPage(const char *name, bool puzzleOnly);
	void printGoDestinations(bool puzzleOnly);
	bool Cmd_Puzzle(int argc, const char **argv);
	bool CmdSub_PuzzleFinish(int argc, const char **argv);
	bool CmdSub_PuzzleAnswer(int argc, const char **argv);
	/** Wrap puzzle answers to the debugger width while retaining section indentation. */
	static Common::String formatPuzzleAnswer(const Common::String &answer);
	bool CmdSub_PuzzleChance(int argc, const char **argv);
	/** Invoke the original global developer actions through the debugger. */
	static constexpr const char *kCmdBuiltinDebug = "builtin_debug";
	/** Explain the original developer keys and their activation gate. */
	static constexpr const char *kCmdManBuiltinDebug = "man_builtin_debug";
	bool Cmd_BuiltinDebug(int argc, const char **argv);
	bool Cmd_ManBuiltinDebug(int argc, const char **argv);
	/** Top-level debugger command for drawing debug views. */
	static constexpr const char *kCmdDraw = "draw";
	/** Draw the active page through its drop-acceptance mask. */
	static constexpr const char *kSubCmdDrawAreaMask = "areamask";
	/** Draw frames of an animation or sprite resource. */
	static constexpr const char *kSubCmdDrawAnimation = "animation";

	/** Dispatch the draw command to its selected subcommand. */
	bool Cmd_Draw(int argc, const char **argv);
	/** Open the debug dialog showing the masked page. */
	bool CmdSub_DrawAreaMask(int argc, const char **argv);
	/** Open the debug dialog showing animation or sprite frames. */
	bool CmdSub_DrawAnimation(int argc, const char **argv);
	/** Return whether @p arg requests command help. */
	static bool isHelpOption(const char *arg);
	/** Return whether any command argument requests help. */
	static bool hasHelpOption(int argc, const char **argv);
	/** Print the standard help-option usage line. */
	void printHelpOption();

	/** Borrowed engine used to query the active page and its mask. */
	Zoombini2Engine *_vm;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_CONSOLE_H
