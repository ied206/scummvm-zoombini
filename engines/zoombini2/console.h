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

#include "zoombini2/pages/transition_maptrans.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

enum PageId : int;

/** Parse bounded decimal or 0x-prefixed integer text without reporting diagnostics. */
class StringParser {
public:
	enum class Result {
		kSuccess,
		kEmptyInput,
		kInvalidInput,
		kTrailingCharacters,
		kConversionFailure,
		kOutOfRange
	};

	/** Leave @p result unchanged when parsing fails. */
	static Result parseSignedInt(const char *text, int32 &result);
	/** Leave @p result unchanged when parsing fails. */
	static Result parseUnsignedInt(const char *text, uint32 &result);
};

/** Debug console for the Mountain Rescue engine. */
class Zoombini2Console : public GUI::Debugger {
public:
	/** Bind the console to @p vm without taking ownership. */
	explicit Zoombini2Console(Zoombini2Engine *vm);
	/** Release the console without touching the bound engine. */
	~Zoombini2Console() override;

private:
	/** Select a route transition or practice puzzle through @ref Zoombini2Console::Cmd_Go. */
	static constexpr const char *kCmdGo = "go";
	/** Inspect puzzle answers, manage remaining chances, or complete the active puzzle through @ref Zoombini2Console::Cmd_Puzzle. */
	static constexpr const char *kCmdPuzzle = "puzzle";
	/** Alias of @ref Zoombini2Console::kCmdPuzzle, handled by @ref Zoombini2Console::Cmd_Puzzle. */
	static constexpr const char *kCmdPage = "page";
	static constexpr const char *kSubCmdGoXfer = "xfer";
	static constexpr const char *kSubCmdGoPractice = "practice";
	bool Cmd_Go(int argc, const char **argv);
	bool CmdSub_GoXfer(int argc, const char **argv);
	bool CmdSub_GoPractice(int argc, const char **argv);
	const TransitionMapTrans::RouteDest *findGoDestination(const char *name, bool puzzleOnly);
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
	/**
	 * Preview resources or the page's drop-acceptance area mask through @ref Zoombini2Console::Cmd_Draw.
	 * Debug views open in dialogs so the active page remains available when the preview closes.
	 */
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
	/** Plot a point, line, or rectangle on a white debug canvas through @ref Zoombini2Console::Cmd_Plot. */
	static constexpr const char *kCmdPlot = "plot";
	/** Plot one pixel. */
	static constexpr const char *kSubCmdPlotPoint = "point";
	/** Plot a line between two inclusive endpoints. */
	static constexpr const char *kSubCmdPlotLine = "line";
	/** Plot a rectangle outline with exclusive right and bottom coordinates. */
	static constexpr const char *kSubCmdPlotRect = "rect";
	/** Dispatch the plot command to its selected subcommand. */
	bool Cmd_Plot(int argc, const char **argv);
	/** Open the debug dialog showing one diagnostic pixel. */
	bool CmdSub_PlotPoint(int argc, const char **argv);
	/** Open the debug dialog showing one diagnostic line. */
	bool CmdSub_PlotLine(int argc, const char **argv);
	/** Open the debug dialog showing one diagnostic rectangle outline. */
	bool CmdSub_PlotRect(int argc, const char **argv);
	/** Parse a signed coordinate pair without allowing values to wrap when passed to surface drawing. */
	bool parsePlotPoint(const char *xText, const char *yText, Common::Point32 &point);
	/** Parse a 24-bit RGB color, accepting 0xRRGGBB or #RRGGBB independently of the destination pixel format. */
	bool parsePlotColor(const char *text, uint32 &color);
	bool parseSignedInt(const char *text, int32 &result);
	bool parseUnsignedInt(const char *text, uint32 &result);
	void reportIntegerParseFailure(StringParser::Result result, bool unsignedValue, const char *text);
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
