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

#ifndef ZOOMBINI2_PAGES_INTERACTIVE_WORLDMAP_H
#define ZOOMBINI2_PAGES_INTERACTIVE_WORLDMAP_H

#include "zoombini2/pages/interactive_base.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;
class BitmapFont;
class VolumePanel;

enum WorldMapMode {
	/** Allow direct puzzle selection at one shared difficulty. */
	kWorldMapPractice,
	/** Show route progress and continue the active saved game. */
	kWorldMapSavedGame
};

/**
 * Mountain map with distinct practice and saved-game modes.
 *
 * Practice mode enables puzzle icons and selects one difficulty for the whole
 * route. Saved-game mode enables visited route hubs and draws each route
 * segment at the difficulty stored by the active game.
 */
class WorldMapPage : public InteractivePage {
public:
	/** Construct a mountain map in @p mode for @p engine. */
	WorldMapPage(Zoombini2Engine *engine, WorldMapMode mode);
	/** Release map resources, controls, and modal panels. */
	~WorldMapPage() override;

	/** Load map resources and derive availability from the selected mode. */
	void init() override;
	/** Update icon, legend, button, and modal hover state. */
	void update() override;
	/** Draw the map, route progress, statistics, and active modal panel. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Dispatch a click to a map icon, control, legend tab, or modal panel. */
	void handleClick(const Common::Point &pos) override;

private:
	/** Number of selectable map icons. */
	static const int kNumIcons = 13;
	/** Number of title overlays corresponding to map icons. */
	static const int kNumTitles = 13;
	/** Number of path-segment slots, including one alternate slot. */
	static const int kNumSegments = 14;
	/** Number of segment graphics tiers, including the neutral tier. */
	static const int kNumDiffTiers = 4;
	/** Number of legend bitmaps, including the inactive legend. */
	static const int kNumLegends = 4;

	/** Icon hit-test rectangles in map coordinates. */
	struct IconRect {
		int16 x, y, w, h;
	};
	static const IconRect kIconHitRects[kNumIcons];

	/** Title sprite draw positions. */
	struct TitlePos {
		int16 x, y;
	};
	static const TitlePos kTitlePositions[kNumTitles];

	/** Route-segment draw positions. */
	struct SegmentPos {
		int16 x, y;
	};
	static const SegmentPos kSegmentPositions[kNumSegments];

	/** Vertical positions for the four saved-game statistics. */
	static const int kStatLabelY[4];

	/**
	 * Segment-to-world mapping for saved-game per-world difficulty drawing.
	 * Index = segment slot, value = world ID whose difficulty to use.
	 * Slot 12 is unused in saved-game mode; slot 13 occupies its position.
	 */
	static const int kSegToWorld[kNumSegments];

	/** One bottom-panel button with either RLE or bit-block visuals. */
	struct MapButton {
		/** Hit-test rectangle components. */
		int x, y, w, h;
		/** Whether the button accepts clicks. */
		bool enabled;
		/** Whether the pointer is currently over the button. */
		bool hovered;
		/** Whether this button uses RLE sprites instead of bit blocks. */
		bool isRle;
		/** Normal RLE visual when @ref MapButton::isRle is true. */
		RleBlock *normalRle;
		/** Hovered RLE visual when @ref MapButton::isRle is true. */
		RleBlock *hiliteRle;
		/** Optional disabled RLE visual. */
		RleBlock *grayRle;
		/** Normal bit-block visual when @ref MapButton::isRle is false. */
		BitBlock *normalBB;
		/** Hovered bit-block visual when @ref MapButton::isRle is false. */
		BitBlock *hiliteBB;

		/** Initialize an enabled button with no loaded visuals. */
		MapButton() : x(0), y(0), w(0), h(0), enabled(true), hovered(false),
		              isRle(false), normalRle(nullptr), hiliteRle(nullptr),
		              grayRle(nullptr), normalBB(nullptr), hiliteBB(nullptr) {}
	};

	/** Number of controls in the bottom panel. */
	static const int kNumButtons = 4;

	/** Practice or saved-game behavior selected at construction. */
	WorldMapMode _mode;
	/** Currently hovered icon, or `-1` when none is hovered. */
	int _hoveredIcon;
	/** Practice or selected-world difficulty in the range one through three. */
	int _currentDifficulty;
	/** Hovered difficulty legend, or zero when none is hovered. */
	int _hoveredLegendTab;

	/** Mountain map background. */
	BitBlock *_background;
	/** One colored or disabled icon for each map destination. */
	RleBlock *_icons[kNumIcons];
	/** Title overlay corresponding to each map icon. */
	RleBlock *_titles[kNumTitles];
	/** Path graphics indexed by difficulty tier and segment slot. */
	RleBlock *_segments[kNumDiffTiers][kNumSegments];
	/** Practice-mode instruction panel. */
	RleBlock *_statsPractice;
	/** Saved-game progress panel. */
	RleBlock *_statsSavedGame;
	/** Legend graphics indexed by inactive or active difficulty. */
	BitBlock *_legends[kNumLegends];
	/** White font used for saved-game statistics. */
	BitmapFont *_whiteFont;

	/** Whether each icon accepts clicks in the current mode and progress state. */
	bool _iconClickable[kNumIcons];
	/** Whether each icon uses its colored rather than disabled visual. */
	bool _iconColored[kNumIcons];
	/** Remaining, first-board, second-board, and completed Zoombini counts. */
	int _stats[4];

	/** Bottom-panel controls. */
	MapButton _buttons[kNumButtons];

	/** Menu selection sound. */
	int _blipSoundId;
	/** Shared engine-owned map music handle. */
	int _mapMusicId;

	/** Owned volume panel while options are open. */
	VolumePanel *_volumePanel;
	/** Whether quit confirmation is active. */
	bool _showQuitDialog;
	/** Quit confirmation panel origin. */
	Common::Point32 _quitDialogPosition;
	/** Hovered quit confirmation control, or zero. */
	int _quitDialogButtonHover;
	/** Quit panel without a highlighted action. */
	RleBlock *_quitPanelNothing;
	/** Quit panel with OK highlighted. */
	RleBlock *_quitPanelOk;
	/** Quit panel with Cancel highlighted. */
	RleBlock *_quitPanelCancel;
	/** Quit confirmation text. */
	BitBlock *_quitTextQuit;
	/** Saved pixels restored when quit confirmation closes. */
	Graphics::ManagedSurface *_quitDialogBackground;

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
	/** Return the difficulty legend tab at @p x and @p y, or zero. */
	int hitTestLegendTab(int x, int y) const;

	/** Return whether this page uses direct practice selection. */
	bool isPracticeMode() const { return _mode == kWorldMapPractice; }
	/** Draw every route segment at the practice difficulty. */
	void drawPracticeSegments(Graphics::ManagedSurface *screen, const byte (*lut)[256]);
	/** Draw visited route segments at their stored world difficulties. */
	void drawSavedGameSegments(Graphics::ManagedSurface *screen, const byte (*lut)[256]);

	/** Open the volume panel. */
	void openVolumePanel();
	/** Close the volume panel and optionally apply its values. */
	void closeVolumePanel(bool applyChanges);
	/** Apply either panel or stored volume values to the engine. */
	void applyVolumePanelVolumes(bool usePanelValues);
	/** Open quit confirmation. */
	void openQuitDialog();
	/** Close quit confirmation. */
	void closeQuitDialog();
	/** Draw quit confirmation over the saved map background. */
	void drawQuitDialog(Graphics::ManagedSurface *screen);
	/** Return the quit confirmation control at @p x and @p y, or zero. */
	int hitTestQuitDialog(int x, int y) const;

	/** Title resource paths indexed by icon. */
	static const char *const kTitleFiles[kNumTitles];
	/** Route-segment directories indexed by difficulty tier. */
	static const char *const kSegmentDirs[kNumDiffTiers];
	/** Route-segment filenames indexed by segment slot. */
	static const char *const kSegmentFiles[kNumSegments];
	/** Legend resource paths indexed by inactive or active difficulty. */
	static const char *const kLegendFiles[kNumLegends];
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_INTERACTIVE_WORLDMAP_H
