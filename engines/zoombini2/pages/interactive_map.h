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
class BitBlock;
enum class DialogMsgBoxButton;
class RleBlock;
class BitmapFont;
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
	/** Release map resources, controls, and its owned volume panel. */
	~InteractiveMap() override;

	/** Load map resources and derive availability from the selected mode. */
	void init() override;
	/** Update icon, legend, button, and volume-panel hover state. */
	void onUpdate() override;
	/** Draw the map, route progress, statistics, and owned volume panel. */
	void onRenderScene(ManagedSurface32 *screen) override;
	void onRenderForeground(ManagedSurface32 *screen) override;
	/** Dispatch a click to a map icon, control, legend tab, or volume panel. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	/** Return the required practice-party size for @p pageId, or zero for a shelter or unsupported page. */
	static int getPracticePartySize(int pageId);
	bool hasActiveDialog() const override { return _volumePanel != nullptr; }

private:
	EventHandleResult handleVolumePanelInput(const Common::Point &pos, bool mouseReleased);
	bool _volumePanelMouseDown = false;
	/** Number of selectable map icons. */
	static const int kNumIcons = 13;
	/** Number of title overlays corresponding to map icons. */
	static const int kNumTitles = 13;
	/** Number of path-segment slots, including one alternate slot. */
	static const int kNumSegments = 14;
	/** Number of segment graphics tiers, including the neutral tier. */
	static const int kNumLevelTiers = 4;
	/** Number of legend bitmaps, including the inactive legend. */
	static const int kNumLegends = 4;

	/** Icon hit-test rectangles in map coordinates. */
	static const Common::Rect kIconHitRects[kNumIcons];

	/** Title sprite draw positions. */
	static const Common::Point32 kTitlePos[kNumTitles];

	/** Route-segment draw positions. */
	static const Common::Point32 kSegmentPos[kNumSegments];

	/** Vertical positions for the four saved-game statistics. */
	static const int kStatLabelY[4];

	/**
	 * Segment-to-page mapping for saved-game page-level drawing.
	 * Index = segment slot, value = page ID whose level to use.
	 * Slot 12 is unused in saved-game mode; slot 13 occupies its position.
	 */
	static const int kSegmentPageIds[kNumSegments];

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
		/** Normal RLE visual when @ref MapButton::isRle is true. */
		RleBlock *normalRle = nullptr;
		/** Hovered RLE visual when @ref MapButton::isRle is true. */
		RleBlock *hiliteRle = nullptr;
		/** Optional disabled RLE visual. */
		RleBlock *grayRle = nullptr;
		/** Normal bit-block visual when @ref MapButton::isRle is false. */
		BitBlock *normalBB = nullptr;
		/** Hovered bit-block visual when @ref MapButton::isRle is false. */
		BitBlock *hiliteBB = nullptr;

		/** Initialize an enabled button with no loaded visuals. */
		MapButton() = default;
	};

	/** Number of controls in the bottom panel. */
	static const int kNumButtons = 4;

	/** Practice or saved-game behavior selected at construction. */
	MapScreenMode _mode;
	/** Currently hovered icon, or `-1` when none is hovered. */
	int _hoveredIcon = -1;
	/** Practice or selected-page level in the range one through three. */
	int _currentLevel = 1;
	/** Hovered level legend, or zero when none is hovered. */
	int _hoveredLegendTab = 0;

	/** Mountain map background. */
	BitBlock *_background = nullptr;
	/** One colored or disabled icon for each map destination. */
	RleBlock *_icons[kNumIcons] = {};
	/** Title overlay corresponding to each map icon. */
	RleBlock *_titles[kNumTitles] = {};
	/** Path graphics indexed by level tier and segment slot. */
	RleBlock *_segments[kNumLevelTiers][kNumSegments] = {};
	/** Practice-mode instruction panel. */
	RleBlock *_statsPractice = nullptr;
	/** Saved-game progress panel. */
	RleBlock *_statsSavedGame = nullptr;
	/** Legend graphics indexed by inactive or active level. */
	BitBlock *_legends[kNumLegends] = {};
	/** White font used for saved-game statistics. */
	BitmapFont *_whiteFont = nullptr;

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
	/** Handle for the map-music stream shared across this game instance. */
	int _mapMusicId = -1;

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

	/** Return whether this page uses direct practice selection. */
	bool isPracticeMode() const { return _mode == kMapScreenPractice; }
	/** Replace the active roster with the route-sized practice party for @p pageId. */
	void createPracticeParty(int pageId);
	/** Return whether @p traits satisfy the practice party's trait distribution limits. */
	bool practiceCandidateFitsPack(const ZmbTrait &traits) const;
	/** Generate one name for a practice Zoombini. */
	Common::String generatePracticeZoombiniName() const;
	/** Draw every route segment at the practice level. */
	void drawPracticeSegments(ManagedSurface32 *screen, const AlphaBlendLUT &lut);
	/** Draw visited route segments at their stored page levels. */
	void drawSavedGameSegments(ManagedSurface32 *screen, const AlphaBlendLUT &lut);

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
	static const char *const kTitleFiles[kNumTitles];
	/** Route-segment directories indexed by level tier. */
	static const char *const kSegmentDirs[kNumLevelTiers];
	/** Route-segment filenames indexed by segment slot. */
	static const char *const kSegmentFiles[kNumSegments];
	/** Legend resource paths indexed by inactive or active level. */
	static const char *const kLegendFiles[kNumLegends];
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_INTERACTIVE_MAP_SCREEN_H
