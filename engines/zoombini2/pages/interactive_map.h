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

#ifndef ZOOMBINI2_PAGES_INTERACTIVE_MAP_SCREEN_H
#define ZOOMBINI2_PAGES_INTERACTIVE_MAP_SCREEN_H

#include "zoombini2/pages/interactive_base.h"

namespace Common {
class String;
}

namespace Zoombini2 {

class AlphaBlendLUT;
enum class DialogMsgBoxButton;
class VolumePanel;
struct ZmbTrait;

enum MapScreenMode {
	/** Allow direct puzzle selection at one shared level. */
	kMapScreenPractice,
	/** Show route progress and continue the active saved game. */
	kMapScreenSavedGame
};

/**
 * Mountain map with distinct practice and saved-game modes.
 *
 * Practice mode enables puzzle icons and selects one level for the whole
 * route. Saved-game mode enables visited route hubs and draws each route
 * segment at the level stored by the active game.
 */
class InteractiveMap : public InteractiveBase {
public:
	/** Construct a mountain map in @p mode for @p vm. */
	InteractiveMap(Zoombini2Engine *vm, MapScreenMode mode);
	/** Release map controls and the volume panel; bitmap resources remain in the graphics page cache. */
	~InteractiveMap() override;

	/** Load map resources and derive availability from the selected mode. */
	void init() override;
	/** Update icon, legend, button, and volume-panel hover state. */
	void onUpdate() override;
	/** Draw the map, route progress, statistics, and owned volume panel. */
	void onRenderContent(ManagedSurface32 *screen) override;
	void onRenderForeground(ManagedSurface32 *screen) override;
	/** Dispatch a click to a map icon, control, legend tab, or volume panel. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	/** Return the required practice-party size for @p pageId, or zero for a shelter or unsupported page. */
	static uint getPracticePartySize(PageId pageId);
	/** Create a route-sized or explicitly sized debugging party using the map's practice rules. */
	static void createPracticeParty(Zoombini2Engine *vm, PageId pageId, uint partySize = 0);
	bool hasActiveDialog() const override { return _volumePanel != nullptr; }

private:
	/** Resource path. */
	static constexpr const char *kBackgroundPath = "#bmp/Map/background";
	/** Resource path. */
	static constexpr const char *kPracticeStatsPath = "bmp/map/stats_scr3";
	/** Resource path. */
	static constexpr const char *kSavedGameStatsPath = "bmp/map/stats_scr1";
	/** Resource path. */
	static constexpr const char *kBlipSoundPath = "sounds/blip.wav";
	/** Resource path format. */
	static constexpr const char *kDisabledIconFormat = "bmp/map/icon%02dgray";
	/** Resource path format. */
	static constexpr const char *kIconFormat = "bmp/map/icon%02d";
	/** Resource path format. */
	static constexpr const char *kSegmentPathFormat = "%s/%s";
	/** Resource path. */
	static constexpr const char *kFilesNormalPath = "bmp/map/PANEL NL - Parties NORMAL";
	/** Resource path. */
	static constexpr const char *kFilesHighlightPath = "bmp/map/PANEL NL - Parties HILITE";
	/** Resource path. */
	static constexpr const char *kOptionsNormalPath = "bmp/map/PANEL NL - Options NORMAL";
	/** Resource path. */
	static constexpr const char *kOptionsHighlightPath = "bmp/map/PANEL NL - Options HILITE";
	/** Resource path. */
	static constexpr const char *kGameNormalPath = "bmp/map/PANEL NL - Game NORMAL";
	/** Resource path. */
	static constexpr const char *kGameHighlightPath = "bmp/map/PANEL NL - Game HILITE";
	/** Resource path. */
	static constexpr const char *kGameDisabledPath = "bmp/map/PANEL NL - Game Gray";
	/** Resource path. */
	static constexpr const char *kPracticeNormalPath = "bmp/map/PANEL NL - Entraine NORMAL";
	/** Resource path. */
	static constexpr const char *kPracticeHighlightPath = "bmp/map/PANEL NL - Entraine HILITE";
	/** Resource path. */
	static constexpr const char *kQuitNormalPath = "bmp/map/PANEL NL - Quitter NORMAL";
	/** Resource path. */
	static constexpr const char *kQuitHighlightPath = "bmp/map/PANEL NL - Quitter HILITE";
	/** Resource path. */
	static constexpr const char *kQuitConfirmationPath = "bmp/menu/Quit_panel_text_quit";

	EventHandleResult handleVolumePanelInput(const Common::Point &pos, bool mouseReleased);
	bool _volumePanelMouseDown = false;
	/** Number of selectable map icons. */
	static constexpr int kNumIcons = 13;
	/** Number of title overlays corresponding to map icons. */
	static constexpr int kNumTitles = 13;
	/** Number of path-segment slots, including one alternate slot. */
	static constexpr int kNumSegments = 14;
	/** Number of segment graphics tiers, including the neutral tier. */
	static constexpr int kNumLevelTiers = 4;
	/** Number of legend bitmaps, including the inactive legend. */
	static constexpr int kNumLegends = 4;
	/** Level 4 practice tab drawn beyond the three original legend rows. */
	static constexpr int kLevel4TabLeft = 590;
	static constexpr int kLevel4TabRight = 625;
	static constexpr int kLevel4TabTop = 497;
	static constexpr int kLevel4TabBottom = 508;
	static constexpr int kLevel4TabSlant = 3;

	/** Icon hit-test rectangles in map coordinates. */
	static const Common::Rect kIconHitRects[kNumIcons];

	/** Title sprite draw positions. */
	static constexpr Common::Point32 kTitlePos[kNumTitles] = {
		{35, 414},  //  0 ShelterZombiniville
		{0, 222},   //  1 CrazyTurtle
		{140, 96},  //  2 Waterslide
		{224, 287}, //  3 Aquacube
		{406, 299}, //  4 Rescue1
		{358, 385}, //  5 MysticMarsh
		{352, 96},  //  6 MagicWall
		{545, 379}, //  7 WallOfFleens
		{464, 91},  //  8 ChezNorf
		{579, 307}, //  9 Rescue2
		{642, 247}, // 10 Snowboard
		{695, 192}, // 11 Boolies
		{575, 44}   // 12 Booliewood
	};

	/** Route-segment draw positions. */
	static constexpr Common::Point32 kSegmentPos[kNumSegments] = {
		{151, 285}, //  0 segment_01
		{204, 230}, //  1 segment_02
		{271, 222}, //  2 segment_03
		{342, 266}, //  3 segment_04
		{377, 314}, //  4 segment_05b
		{386, 256}, //  5 segment_05a
		{456, 361}, //  6 segment_06b
		{442, 239}, //  7 segment_06a
		{540, 289}, //  8 segment_07b
		{529, 244}, //  9 segment_07a
		{575, 249}, // 10 segment_08
		{623, 212}, // 11 segment_09
		{661, 101}, // 12 segment_10
		{661, 101}  // 13 duplicate of segment_10
	};

	/** Vertical positions for the four saved-game statistics. */
	static constexpr int kStatLabelY[4] = {
		15,
		39,
		63,
		90,
	};

	/**
	 * Segment-to-page mapping for saved-game page-level drawing.
	 * Index = segment slot, value = page ID whose level to use.
	 * Slot 12 is unused in saved-game mode; slot 13 occupies its position.
	 */
	static constexpr PageId kSegmentPageIds[kNumSegments] = {
		kPageCrazyTurtle,  //  0: CrazyTurtle
		kPageWaterslide,   //  1: Waterslide
		kPageAquacube,     //  2: Aquacube
		kPageAquacube,     //  3: Aquacube
		kPageMysticMarsh,  //  4: MysticMarsh
		kPageMagicWall,    //  5: MagicWall
		kPageWallOfFleens, //  6: WallOfFleens
		kPageChezNorf,     //  7: ChezNorf
		kPageWallOfFleens, //  8: WallOfFleens
		kPageChezNorf,     //  9: ChezNorf
		kPageSnowboard,    // 10: Snowboard
		kPageBoolies,      // 11: Boolies
		kPageBoolies,      // 12: unused in saved-game mode
		kPageBoolies       // 13: Boolies duplicate
	};

	/** One bottom-panel button with either RLE or bit-block visuals. */
	struct MapButton {
		/** Hit-test rectangle in screen coordinates. */
		Common::Rect rect = Common::Rect();
		/** Whether the button accepts clicks. */
		bool enabled = true;
		/** Whether the pointer is currently over the button. */
		bool hovered = false;
		/** Whether this button uses RLE sprites instead of bit blocks. */
		bool isRle = false;
		/** Normal visual path in the graphics page cache. */
		Common::String normalPath;
		/** Hovered visual path in the graphics page cache. */
		Common::String hilitePath;
		/** Optional disabled visual path in the graphics page cache. */
		Common::String grayPath;

		/** Initialize an enabled button with no loaded visuals. */
		MapButton() = default;
	};

	/** Number of controls in the bottom panel. */
	static constexpr int kNumButtons = 4;

	/** Practice or saved-game behavior selected at construction. */
	MapScreenMode _mode;
	/** Currently hovered icon, or `-1` when none is hovered. */
	int _hoveredIcon = -1;
	/** Practice or selected-page level in the range one through four. */
	int _currentLevel = 1;
	/** Hovered level legend, or zero when none is hovered. */
	int _hoveredLegendTab = 0;

	/** One colored or disabled icon path for each map destination. */
	Common::String _icons[kNumIcons];
	/** Title overlay path corresponding to each map icon. */
	Common::String _titles[kNumTitles];
	/** Route graphic paths indexed by level tier and segment slot. */
	Common::String _segments[kNumLevelTiers][kNumSegments];
	/** Practice-mode instruction panel path. */
	Common::String _statsPracticePath;
	/** Saved-game progress panel path. */
	Common::String _statsSavedGamePath;
	/** Legend paths indexed by inactive or active level. */
	Common::String _legends[kNumLegends];

	/** Whether each icon accepts clicks in the current mode and progress state. */
	bool _iconClickable[kNumIcons] = {};
	/** Whether each icon uses its colored rather than disabled visual. */
	bool _iconColored[kNumIcons] = {};
	/** Remaining, first-board, second-board, and completed Zoombini counts. */
	int _stats[4] = {};

	/** Bottom-panel controls. */
	MapButton _buttons[kNumButtons] = {};

	/** Menu selection sound. */
	int _blipSoundId = -1;

	/** Volume panel managed by this page while options are open. */
	VolumePanel *_volumePanel = nullptr;

	/** Recompute progress statistics from the active game state. */
	void computeStats();
	/** Select colored, disabled, and clickable state for every icon. */
	void setupIcons();
	/** Load all route-segment graphics. */
	void loadSegments();
	/** Load and initialize the bottom-panel controls. */
	void loadButtons();
	/** Return the icon at @p pos, or `-1` when none is hit. */
	int hitTestIcon(const Common::Point &pos) const;
	/** Return the bottom-panel button at @p pos, or `-1`. */
	int hitTestButton(const Common::Point &pos) const;
	/** Return the level legend tab at @p pos, or zero. */
	int hitTestLegendTab(const Common::Point32 &pos) const;
	/** Apply a practice-map level after checking the optional level 4 tier. */
	bool selectPracticeLevel(int level);
	/** Return the effective practice difficulty for one puzzle. */
	int getPracticePuzzleLevel(PageId pageId) const;
	/** Draw the optional dark level 4 tab beneath the original legend. */
	void drawLevel4LegendTab(ManagedSurface32 *screen) const;

	/** Return whether this page uses direct practice selection. */
	bool isPracticeMode() const { return _mode == kMapScreenPractice; }
	/** Return whether @p traits satisfy the practice party's trait distribution limits. */
	static bool practiceCandidateFitsPack(const Zoombini2Engine *vm, const ZmbTrait &traits);
	/** Draw every route segment at the practice level. */
	void drawPracticeSegments(ManagedSurface32 *screen);
	/** Draw visited route segments at their stored page levels. */
	void drawSavedGameSegments(ManagedSurface32 *screen);

	/** Open the volume panel. */
	void openVolumePanel();
	/** Close the volume panel and optionally apply its values. */
	void closeVolumePanel(bool applyChanges);
	/** Apply either panel or stored volume values, optionally persisting them for this target. */
	void applyVolumePanelVolumes(bool usePanelValues, bool persistChanges);
	/** Request the shared quit confirmation. */
	void requestQuitConfirmation();
	/** Apply the shared quit-confirmation result. */
	void handleQuitConfirmation(DialogMsgBoxButton button);

	/** Title resource paths indexed by icon. */
	static constexpr const char *kTitleFiles[] = {
		"bmp/map/Title01",  //  0 ShelterZombiniville
		"bmp/map/Title02",  //  1 CrazyTurtle
		"bmp/map/Title03",  //  2 Waterslide
		"bmp/map/Title04",  //  3 Aquacube
		"bmp/map/Title05",  //  4 Rescue1
		"bmp/map/Title06a", //  5 MysticMarsh
		"bmp/map/Title06b", //  6 MagicWall
		"bmp/map/Title07a", //  7 WallOfFleens
		"bmp/map/Title07b", //  8 ChezNorf
		"bmp/map/Title08",  //  9 Rescue2
		"bmp/map/Title09",  // 10 Snowboard
		"bmp/map/Title10",  // 11 Boolies
		"bmp/map/Title12"   // 12 Booliewood
	};
	/** Route-segment directories indexed by level tier. */
	static constexpr const char *kSegmentDirs[] = {
		"bmp/map/04 neutral segments", // 0 = neutral (unvisited)
		"bmp/map/01 Easy segments",    // 1 = easy level
		"bmp/map/02 Medium segments",  // 2 = medium level
		"bmp/map/03 Hard segments"     // 3 = hard level
	};
	/** Route-segment filenames indexed by segment slot. */
	static constexpr const char *kSegmentFiles[] = {
		"segment_01",  //  0
		"segment_02",  //  1
		"segment_03",  //  2
		"segment_04",  //  3
		"segment_05b", //  4
		"segment_05a", //  5
		"segment_06b", //  6
		"segment_06a", //  7
		"segment_07b", //  8
		"segment_07a", //  9
		"segment_08",  // 10
		"segment_09",  // 11
		"segment_10",  // 12
		"segment_10"   // 13 (duplicate)
	};
	/** Legend resource paths indexed by inactive or active level. */
	static constexpr const char *kLegendFiles[] = {
		"bmp/map/map_legend_off",    // 0
		"bmp/map/map_legend_level1", // 1
		"bmp/map/map_legend_level2", // 2
		"bmp/map/map_legend_level3"  // 3
	};
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_INTERACTIVE_MAP_SCREEN_H
