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

#include "zoombini2/graphics.h"
#include "zoombini2/pages/transition_maptrans.h"
#include "zoombini2/scripts.h"
#include "zoombini2/zoombini2.h"

namespace Common {
class DumpFile;
}

namespace Zoombini2 {

enum PageId : int;

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
	/** Play or stop sound effects through @ref Zoombini2Console::Cmd_Sfx. */
	static constexpr const char *kCmdSfx = "sfx";
	static constexpr const char *kSubCmdSfxPlay = "play";
	static constexpr const char *kSubCmdSfxStop = "stop";
	/** Sound-effect resources use this loose-file path prefix. */
	static constexpr const char *kSfxResourcePrefix = "sounds/fx/";
	/** Background music resources use these logical loose-file path prefixes. */
	static constexpr const char *kBgmResourcePrefix = "sounds/music/";
	static constexpr const char *kInstalledBgmResourcePrefix = "#sounds/music/";
	static constexpr const char *kWavResourceSuffix = ".wav";
	/** Play or stop background music through @ref Zoombini2Console::Cmd_Bgm. */
	static constexpr const char *kCmdBgm = "bgm";
	static constexpr const char *kSubCmdBgmPlay = "play";
	static constexpr const char *kSubCmdBgmStop = "stop";
	static constexpr const char *kSubCmdGoXfer = "xfer";
	static constexpr const char *kSubCmdGoPractice = "practice";
	bool Cmd_Sfx(int argc, const char **argv);
	bool CmdSub_SfxPlay(int argc, const char **argv);
	bool CmdSub_SfxStop(int argc, const char **argv);
	bool Cmd_Bgm(int argc, const char **argv);
	bool CmdSub_BgmPlay(int argc, const char **argv);
	bool CmdSub_BgmStop(int argc, const char **argv);
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
	/** Draw one loose-file bitmap. */
	static constexpr const char *kSubCmdDrawImage = "image";
	/** Preview a resource message or Unicode UI text in the game's confirmation dialog. */
	static constexpr const char *kSubCmdDrawMsgBox = "msgbox";

	/** Dispatch the draw command to its selected subcommand. */
	bool Cmd_Draw(int argc, const char **argv);
	/** Open the debug dialog showing the masked page. */
	bool CmdSub_DrawAreaMask(int argc, const char **argv);
	/** Open the debug dialog showing animation or sprite frames. */
	bool CmdSub_DrawAnimation(int argc, const char **argv);
	/** Open an independently decoded bitmap in the debug dialog. */
	bool CmdSub_DrawImage(int argc, const char **argv);
	/** Open a bitmap or Unicode message preview with no game callback. */
	bool CmdSub_DrawMsgBox(int argc, const char **argv);
	/** Export decoded resource images or the current saved-adventure state. */
	static constexpr const char *kCmdDump = "dump";
	/** Inspect decoded animation metadata or registered page-layer runners. */
	static constexpr const char *kCmdPrint = "print";
	/** Query game and runtime state without changing it. */
	static constexpr const char *kCmdState = "state";
	/** Describe the keyboard shortcuts implemented by Z2. */
	static constexpr const char *kCmdManShortcuts = "man_shortcuts";
	/** Numbered dump names preserve existing exports. */
	static constexpr const char *kDebugDumpPathFormat = "dumps/ZOOMBINI2_%s_%04u.%s";
	/** Dispatch the selected resource or save export. */
	bool Cmd_Dump(int argc, const char **argv);
	/** Render one bitmap over white and export it. */
	bool CmdSub_DumpImage(int argc, const char **argv);
	/** Render and export each decoded frame over white. */
	bool CmdSub_DumpAnimation(int argc, const char **argv);
	/** Serialize the live named adventure to a fresh dump file. */
	bool CmdSub_DumpSave(int argc, const char **argv);
	/** Dispatch the selected resource or runner query. */
	bool Cmd_Print(int argc, const char **argv);
	/** Print independently decoded frame counts and dimensions. */
	bool CmdSub_PrintAnimation(int argc, const char **argv);
	/** Print the registered general-object runners without stepping them. */
	bool CmdSub_PrintRunners(int argc, const char **argv);
	/** Print every supported state group or one requested group. */
	bool Cmd_State(int argc, const char **argv);
	/** Describe the current global and page-local keyboard handlers. */
	bool Cmd_ManShortcuts(int argc, const char **argv);
	/** Reject absolute paths and parent traversal before resolving a logical resource name. */
	bool parseDebugResourcePath(const char *text, Common::Path &path);
	/** Load an independent frame sequence or one explicit RB sprite. */
	bool loadDebugAnimation(const Common::Path &path, Animation &animation, RleBlock &sprite, bool &singleFrame);
	/** Parse the supported image format and diagnose unavailable PNG support. */
	bool parseDebugImageFormat(const char *text, bool &png);
	/** Validate the signed 16-bit surface boundary and limit export allocation. */
	bool validateDebugImageSize(const Size32 &size);
	/** Build a bounded filename component from a logical resource name. */
	static Common::String makeDebugDumpStem(const Common::String &name);
	/** Open a fresh numbered file below the dump directory. */
	bool openDebugDump(Common::DumpFile &out, const Common::String &stem, const char *extension, Common::Path &path);
	/** Write one rendered image and report its result. */
	bool exportDebugSurface(const Graphics::Surface &surface, const Common::String &stem, bool png);
	/** Print one supported group of read-only state fields. */
	bool printDebugStateGroup(const Common::String &group);
	/** Plot a point, line, or rectangle over the current screen through @ref Zoombini2Console::Cmd_Plot. */
	static constexpr const char *kCmdPlot = "plot";
	/** Plot one pixel. */
	static constexpr const char *kSubCmdPlotPoint = "point";
	/** Plot a line between two inclusive endpoints. */
	static constexpr const char *kSubCmdPlotLine = "line";
	/** Plot a rectangle outline with exclusive right and bottom coordinates. */
	static constexpr const char *kSubCmdPlotRect = "rect";
	/** Default plot color matching the white Z1 reference. */
	static constexpr RGBColor kDefaultPlotColor = RGBColor(255, 255, 255);
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
	bool parsePlotColor(const char *text, RGBColor &color);
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
	/** Sound record loaded by the latest SFX play command, or -1. */
	int _debugSfxId = -1;
	/** Streamed music record loaded by the latest BGM play command, or -1. */
	int _debugBgmId = -1;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_CONSOLE_H
