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

#ifndef ZOOMBINI2_PAGES_WORLDMAP_H
#define ZOOMBINI2_PAGES_WORLDMAP_H

#include "zoombini2/pages/page.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;
class BitmapFont;
class VolumePanel;

enum WorldMapMode {
	kWorldMapPractice,
	kWorldMapSavedGame
};

/**
 * Mountain map with distinct practice and saved-game modes.
 *
 * Practice mode enables puzzle icons and selects one difficulty for the whole
 * route. Saved-game mode enables visited route hubs and draws each route
 * segment at the difficulty stored by the active game.
 */
class WorldMapPage : public Page {
public:
	WorldMapPage(Zoombini2Engine *engine, WorldMapMode mode);
	~WorldMapPage() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	static const int kNumIcons = 13;
	static const int kNumTitles = 13;
	static const int kNumSegments = 14;   ///< Path segment slots (13 unique + 1 duplicate)
	static const int kNumDiffTiers = 4;   ///< Difficulty tiers: neutral, easy, medium, hard
	static const int kNumLegends = 4;     ///< Legend bitmaps: off, level1, level2, level3

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

	/** Stat label Y positions. */
	static const int kStatLabelY[4];

	/**
	 * Segment-to-world mapping for saved-game per-world difficulty drawing.
	 * Index = segment slot, value = world ID whose difficulty to use.
	 * Slot 12 is unused in saved-game mode; slot 13 occupies its position.
	 */
	static const int kSegToWorld[kNumSegments];

	/**
	 * Bottom panel button.
	 */
	struct MapButton {
		int x, y, w, h;      ///< Hit-test area
		bool enabled;         ///< Whether button responds to input
		bool hovered;         ///< Currently under mouse
		bool isRle;           ///< true draws RleBlock, false draws BitBlock
		RleBlock *normalRle;  ///< RleBlock normal state (isRle=true)
		RleBlock *hiliteRle;  ///< RleBlock hover state
		RleBlock *grayRle;    ///< RleBlock disabled state (may be null)
		BitBlock *normalBB;   ///< BitBlock normal state (isRle=false)
		BitBlock *hiliteBB;   ///< BitBlock hover state

		MapButton() : x(0), y(0), w(0), h(0), enabled(true), hovered(false),
		              isRle(false), normalRle(nullptr), hiliteRle(nullptr),
		              grayRle(nullptr), normalBB(nullptr), hiliteBB(nullptr) {}
	};

	static const int kNumButtons = 4;

	WorldMapMode _mode;
	int _hoveredIcon;         ///< Currently hovered icon (-1 = none)
	int _currentDifficulty;   ///< Practice or selected-world difficulty, 1-3
	int _hoveredLegendTab;    ///< 0=none, 1-3=difficulty tab being hovered

	// Graphics resources
	BitBlock *_background;
	RleBlock *_icons[kNumIcons];                      ///< One icon per world (normal or gray, decided at init)
	RleBlock *_titles[kNumTitles];                    ///< Title text overlay sprites
	RleBlock *_segments[kNumDiffTiers][kNumSegments]; ///< Path segments [tier][slot]
	RleBlock *_statsPractice;                         ///< Practice instruction panel
	RleBlock *_statsSavedGame;                        ///< Saved-game progress panel
	BitBlock *_legends[kNumLegends];                  ///< Legend bitmaps: [0]=off, [1-3]=level1-3
	BitmapFont *_whiteFont;                           ///< White (255,255,255) font for stats

	// State
	bool _iconClickable[kNumIcons];   ///< Whether each icon responds to clicks
	bool _iconColored[kNumIcons];     ///< Whether each icon is drawn colored (vs gray)
	int _stats[4];                    ///< 0=remaining, 1=boardA, 2=boardB, 3=completed

	// Bottom panel buttons
	MapButton _buttons[kNumButtons];

	// Audio
	int _blipSoundId;
	int _mapMusicId;          ///< Shared engine-owned map music

	// Dialogs
	VolumePanel *_volumePanel;
	bool _showQuitDialog;
	int _quitDialogX, _quitDialogY;
	int _quitDialogButtonHover;
	RleBlock *_quitPanelNothing;
	RleBlock *_quitPanelOk;
	RleBlock *_quitPanelCancel;
	BitBlock *_quitTextQuit;

	void computeStats();
	void setupIcons();
	void loadSegments();
	void loadButtons();
	int hitTestIcon(const Common::Point &pos) const;
	int hitTestButton(const Common::Point &pos) const;
	int hitTestLegendTab(int x, int y) const;

	bool isPracticeMode() const { return _mode == kWorldMapPractice; }
	void drawPracticeSegments(Graphics::ManagedSurface *screen, const byte (*lut)[256]);
	void drawSavedGameSegments(Graphics::ManagedSurface *screen, const byte (*lut)[256]);

	// Dialog methods
	void openVolumePanel();
	void closeVolumePanel(bool applyChanges);
	void applyVolumePanelVolumes(bool usePanelValues);
	void openQuitDialog();
	void closeQuitDialog();
	void drawQuitDialog(Graphics::ManagedSurface *screen);
	int hitTestQuitDialog(int x, int y) const;

	static const char *const kTitleFiles[kNumTitles];
	static const char *const kSegmentDirs[kNumDiffTiers];
	static const char *const kSegmentFiles[kNumSegments];
	static const char *const kLegendFiles[kNumLegends];
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_WORLDMAP_H
