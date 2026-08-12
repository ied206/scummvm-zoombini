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

#include "common/debug.h"
#include "common/str.h"
#include "common/path.h"

#include "zoombini2/pages/worldmap.h"
#include "zoombini2/gfx.h"
#include "zoombini2/ui.h"
#include "zoombini2/sound.h"
#include "zoombini2/game_state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// Static world-map geometry and resource tables.
// ============================================================================

/**
 * Icon hit-test rectangles {x, y, w, h}.
 * Each entry supplies the icon origin and hit-test dimensions.
 */
const WorldMapPage::IconRect WorldMapPage::kIconHitRects[kNumIcons] = {
	{155, 343,  39, 75},   //  0 Zombiniville
	{168, 259,  41, 42},   //  1 CrazyTurtle
	{237, 206,  60, 50},   //  2 Waterslide
	{305, 241,  54, 46},   //  3 Aquacube
	{355, 290,  54, 45},   //  4 Rescue1
	{441, 341,  29, 38},   //  5 MysticMarsh
	{424, 235,  30, 33},   //  6 MagicWall
	{517, 346,  42, 44},   //  7 WallOfFleens
	{515, 215,  31, 47},   //  8 ChezNorf
	{531, 277,  58, 44},   //  9 Rescue2
	{591, 237,  48, 27},   // 10 Snowboard
	{642, 190,  43, 45},   // 11 Boolies
	{709,  48,  89, 76}    // 12 Booliewood
};

/**
 * Title sprite draw positions {x, y}.
 * Each entry supplies the title origin for one world icon.
 */
const WorldMapPage::TitlePos WorldMapPage::kTitlePositions[kNumTitles] = {
	{ 35, 414},   //  0 Zombiniville
	{  0, 222},   //  1 CrazyTurtle
	{140,  96},   //  2 Waterslide
	{224, 287},   //  3 Aquacube
	{406, 299},   //  4 Rescue1
	{358, 385},   //  5 MysticMarsh
	{352,  96},   //  6 MagicWall
	{545, 379},   //  7 WallOfFleens
	{464,  91},   //  8 ChezNorf
	{579, 307},   //  9 Rescue2
	{642, 247},   // 10 Snowboard
	{695, 192},   // 11 Boolies
	{575,  44}    // 12 Booliewood
};

/** Stat label Y positions. */
const int WorldMapPage::kStatLabelY[4] = { 15, 39, 63, 90 };

/**
 * Title sprite filenames per world (0-12).
 */
const char *const WorldMapPage::kTitleFiles[kNumTitles] = {
	"bmp/map/Title01",     //  0 Zombiniville
	"bmp/map/Title02",     //  1 CrazyTurtle
	"bmp/map/Title03",     //  2 Waterslide
	"bmp/map/Title04",     //  3 Aquacube
	"bmp/map/Title05",     //  4 Rescue1
	"bmp/map/Title06a",    //  5 MysticMarsh
	"bmp/map/Title06b",    //  6 MagicWall
	"bmp/map/Title07a",    //  7 WallOfFleens
	"bmp/map/Title07b",    //  8 ChezNorf
	"bmp/map/Title08",     //  9 Rescue2
	"bmp/map/Title09",     // 10 Snowboard
	"bmp/map/Title10",     // 11 Boolies
	"bmp/map/Title12"      // 12 Booliewood
};

/**
 * Segment draw positions {x, y}.
 * Slots 12 and 13 intentionally share a position.
 * Slot 13 duplicates slot 12 (both 661, 101).
 */
const WorldMapPage::SegmentPos WorldMapPage::kSegmentPositions[kNumSegments] = {
	{151, 285},   //  0 segment_01
	{204, 230},   //  1 segment_02
	{271, 222},   //  2 segment_03
	{342, 266},   //  3 segment_04
	{377, 314},   //  4 segment_05a
	{386, 256},   //  5 segment_05b
	{456, 361},   //  6 segment_06a
	{442, 239},   //  7 segment_06b
	{540, 289},   //  8 segment_07a
	{529, 244},   //  9 segment_07b
	{575, 249},   // 10 segment_08
	{623, 212},   // 11 segment_09
	{661, 101},   // 12 segment_10
	{661, 101}    // 13 duplicate of segment_10
};

/**
 * Segment-to-world mapping for saved-game per-world difficulty drawing.
 * The saved-game route uses each mapped world's stored difficulty.
 * Index = segment slot, value = world ID (0-12).
 * Slot 12 is unused in saved-game mode.
 */
const int WorldMapPage::kSegToWorld[kNumSegments] = {
	1,   //  0: CrazyTurtle
	2,   //  1: Waterslide
	3,   //  2: Aquacube
	3,   //  3: Aquacube
	5,   //  4: MysticMarsh
	6,   //  5: MagicWall
	7,   //  6: WallOfFleens
	8,   //  7: ChezNorf
	7,   //  8: WallOfFleens
	8,   //  9: ChezNorf
	10,  // 10: Snowboard
	11,  // 11: Boolies
	11,  // 12: unused in saved-game mode
	11   // 13: Boolies duplicate
};

const char *const WorldMapPage::kSegmentDirs[kNumDiffTiers] = {
	"bmp/map/04 neutral segments",   // 0 = neutral (unvisited)
	"bmp/map/01 Easy segments",      // 1 = easy difficulty
	"bmp/map/02 Medium segments",    // 2 = medium difficulty
	"bmp/map/03 Hard segments"       // 3 = hard difficulty
};

const char *const WorldMapPage::kSegmentFiles[kNumSegments] = {
	"segment_01",   //  0
	"segment_02",   //  1
	"segment_03",   //  2
	"segment_04",   //  3
	"segment_05a",  //  4
	"segment_05b",  //  5
	"segment_06a",  //  6
	"segment_06b",  //  7
	"segment_07a",  //  8
	"segment_07b",  //  9
	"segment_08",   // 10
	"segment_09",   // 11
	"segment_10",   // 12
	"segment_10"    // 13 (duplicate)
};

const char *const WorldMapPage::kLegendFiles[kNumLegends] = {
	"bmp/map/map_legend_off",      // 0
	"bmp/map/map_legend_level1",   // 1
	"bmp/map/map_legend_level2",   // 2
	"bmp/map/map_legend_level3"    // 3
};

// ============================================================================
// Construction / Destruction
// ============================================================================

WorldMapPage::WorldMapPage(Zoombini2Engine *engine, WorldMapMode mode)
	: Page(engine), _mode(mode), _hoveredIcon(-1),
	  _currentDifficulty(1), _hoveredLegendTab(0), _background(nullptr),
	  _statsPractice(nullptr), _statsSavedGame(nullptr), _whiteFont(nullptr),
	  _blipSoundId(-1), _mapMusicId(-1), _volumePanel(nullptr),
	  _showQuitDialog(false), _quitDialogX(0), _quitDialogY(0),
	  _quitDialogButtonHover(0), _quitPanelNothing(nullptr),
	  _quitPanelOk(nullptr), _quitPanelCancel(nullptr), _quitTextQuit(nullptr) {
	_pageId = kPageWorldMap;

	for (int i = 0; i < kNumIcons; i++) {
		_icons[i] = nullptr;
		_iconClickable[i] = false;
		_iconColored[i] = false;
	}
	for (int i = 0; i < kNumTitles; i++) {
		_titles[i] = nullptr;
	}
	for (int tier = 0; tier < kNumDiffTiers; tier++) {
		for (int slot = 0; slot < kNumSegments; slot++) {
			_segments[tier][slot] = nullptr;
		}
	}
	for (int i = 0; i < kNumLegends; i++) {
		_legends[i] = nullptr;
	}
	for (int i = 0; i < kNumButtons; i++) {
		_buttons[i].x = 0;
		_buttons[i].y = 0;
		_buttons[i].w = 0;
		_buttons[i].h = 0;
		_buttons[i].enabled = false;
		_buttons[i].hovered = false;
		_buttons[i].isRle = false;
		_buttons[i].normalRle = nullptr;
		_buttons[i].hiliteRle = nullptr;
		_buttons[i].grayRle = nullptr;
		_buttons[i].normalBB = nullptr;
		_buttons[i].hiliteBB = nullptr;
	}
	memset(_stats, 0, sizeof(_stats));
}

WorldMapPage::~WorldMapPage() {
	delete _background;
	for (int i = 0; i < kNumIcons; i++) {
		delete _icons[i];
	}
	for (int i = 0; i < kNumTitles; i++) {
		delete _titles[i];
	}
	for (int tier = 0; tier < kNumDiffTiers; tier++) {
		for (int slot = 0; slot < kNumSegments; slot++) {
			delete _segments[tier][slot];
		}
	}
	delete _statsPractice;
	delete _statsSavedGame;
	for (int i = 0; i < kNumLegends; i++) {
		delete _legends[i];
	}
	delete _whiteFont;

	for (int i = 0; i < kNumButtons; i++) {
		delete _buttons[i].normalRle;
		delete _buttons[i].hiliteRle;
		delete _buttons[i].grayRle;
		delete _buttons[i].normalBB;
		delete _buttons[i].hiliteBB;
	}

	SoundManager *sm = _engine->getSoundManager();
	if (sm && 0 <= _blipSoundId)
		sm->unload(_blipSoundId);

	delete _volumePanel;
	delete _quitPanelNothing;
	delete _quitPanelOk;
	delete _quitPanelCancel;
	delete _quitTextQuit;
}

// ============================================================================
// Initialization
// ============================================================================

void WorldMapPage::init() {
	debug(1, "WorldMapPage::init (mode=%d)", _mode);
	SoundManager *sm = _engine->getSoundManager();
	GameState *gs = _engine->getGameState();

	// The saved-game map always exposes the starting hub.
	if (!isPracticeMode()) {
		gs->_worldDataA[0] = 1;
	}

	// --- Background ---
	_background = new BitBlock();
	if (!_background->load(Common::Path("bmp/map/background"))) {
		warning("WorldMapPage: Failed to load map background");
	}

	// --- World icons (one per world: normal or gray, decided by mode) ---
	setupIcons();

	// --- Title overlays ---
	for (int i = 0; i < kNumTitles; i++) {
		_titles[i] = new RleBlock();
		if (!_titles[i]->load(Common::Path(kTitleFiles[i]))) {
			warning("WorldMapPage: Failed to load title %d!", i);
		}
	}

	// Path segment overlays contain 4 tiers with 14 slots each.
	loadSegments();

	// --- Stats overlays (both always loaded) ---
	// Practice and saved-game maps use different statistics panels.
	_statsPractice = new RleBlock();
	_statsPractice->load(Common::Path("bmp/map/stats_scr3"));
	_statsSavedGame = new RleBlock();
	_statsSavedGame->load(Common::Path("bmp/map/stats_scr1"));

	// --- Legend bitmaps (all 4: off, level1, level2, level3) ---
	for (int i = 0; i < kNumLegends; i++) {
		_legends[i] = new BitBlock();
		_legends[i]->load(Common::Path(kLegendFiles[i]));
	}

	// --- White bitmap font for stats ---
	_whiteFont = new BitmapFont();
	_whiteFont->load(Common::Path("bmp/typo"), 255, 255, 255);

	// --- Set initial difficulty ---
	if (isPracticeMode()) {
		_currentDifficulty = 1;
	} else {
		// Saved-game routes use per-world difficulties.
		_currentDifficulty = gs->getDifficulty();
	}

	// --- Audio ---
	_blipSoundId = sm->load(false, Common::Path("sounds/blip.wav"), false);

	_mapMusicId = _engine->ensureMapMusic();

	// --- Compute stats ---
	computeStats();

	// --- Bottom panel buttons ---
	loadButtons();
}

// ============================================================================
// Icon setup decides color and clickability for each mode.
// ============================================================================

void WorldMapPage::setupIcons() {
	GameState *gs = _engine->getGameState();

	if (isPracticeMode()) {
		// Practice mode:
		// Hubs (0,4,9,12): gray, not clickable.
		// Puzzles (1-3, 5-8, 10-11): colored, clickable.
		for (int i = 0; i < kNumIcons; i++) {
			bool isHub = (i == 0 || i == 4 || i == 9 || i == 12);
			_iconColored[i] = !isHub;
			_iconClickable[i] = !isHub;

			Common::String path;
			if (isHub) {
				path = Common::String::format("bmp/map/icon%02dgray", i);
			} else {
				path = Common::String::format("bmp/map/icon%02d", i);
			}
			_icons[i] = new RleBlock();
			if (!_icons[i]->load(Common::Path(path))) {
				warning("WorldMapPage: Failed to load icon %d at %s!", i, path.c_str());
			}
		}
	} else {
		// Saved-game mode:
		if (400 <= gs->_counterDword) {
			// Endgame state: all gray except Booliewood (icon 12).
			for (int i = 0; i < kNumIcons; i++) {
				_iconClickable[i] = false;
				_iconColored[i] = false;

				Common::String path;
				if (i == 12) {
					path = Common::String::format("bmp/map/icon%02d", i);
					_iconClickable[i] = true;
					_iconColored[i] = true;
				} else {
					path = Common::String::format("bmp/map/icon%02dgray", i);
				}
				_icons[i] = new RleBlock();
				_icons[i]->load(Common::Path(path));
			}
		} else {
			// Normal progress uses each world's visited state.
			for (int i = 0; i < kNumIcons; i++) {
				_iconClickable[i] = false;
				_iconColored[i] = false;

				Common::String path;
				if (gs->isWorldVisitedAtDiff(i, 1)) {
					path = Common::String::format("bmp/map/icon%02d", i);
					_iconColored[i] = true;
					// Only hub icons become clickable when visited.
					if (i == 0 || i == 4 || i == 9 || i == 12) {
						_iconClickable[i] = true;
					}
				} else {
					path = Common::String::format("bmp/map/icon%02dgray", i);
				}
				_icons[i] = new RleBlock();
				if (!_icons[i]->load(Common::Path(path))) {
					warning("WorldMapPage: Failed to load icon %d at %s!", i, path.c_str());
				}
			}
		}
	}
}

// ============================================================================
// Segment loading
// ============================================================================

void WorldMapPage::loadSegments() {
	for (int tier = 0; tier < kNumDiffTiers; tier++) {
		for (int slot = 0; slot < kNumSegments; slot++) {
			Common::String path = Common::String::format(
				"%s/%s", kSegmentDirs[tier], kSegmentFiles[slot]);
			_segments[tier][slot] = new RleBlock();
			if (!_segments[tier][slot]->load(Common::Path(path))) {
				warning("WorldMapPage: Failed to load segment tier=%d slot=%d path=%s!",
				        tier, slot, path.c_str());
			}
		}
	}
}

// ============================================================================
// Button loading
// ============================================================================

void WorldMapPage::loadButtons() {
	struct ButtonSetup {
		int x, y, w, h;
		bool isRle;
		const char *normalName;
		const char *hiliteName;
		const char *grayName;
	};

	const ButtonSetup setup[kNumButtons] = {
		// Button 0: Files / Parties
		{27,  561, 145, 39, false,
		 "bmp/map/PANEL NL - Parties NORMAL",
		 "bmp/map/PANEL NL - Parties HILITE", nullptr},
		// Button 1: Options
		{175, 561, 145, 39, false,
		 "bmp/map/PANEL NL - Options NORMAL",
		 "bmp/map/PANEL NL - Options HILITE", nullptr},
		// Button 2: Game in practice mode or Entraine in saved-game mode
		isPracticeMode()
			? ButtonSetup{468, 561, 145, 39, true,
			              "bmp/map/PANEL NL - Game NORMAL",
			              "bmp/map/PANEL NL - Game HILITE",
			              "bmp/map/PANEL NL - Game Gray"}
			: ButtonSetup{468, 561, 145, 39, false,
			              "bmp/map/PANEL NL - Entraine NORMAL",
			              "bmp/map/PANEL NL - Entraine HILITE", nullptr},
		// Button 3: Quitter
		{613, 561, 145, 39, true,
		 "bmp/map/PANEL NL - Quitter NORMAL",
		 "bmp/map/PANEL NL - Quitter HILITE", nullptr},
	};

	GameState *gs = _engine->getGameState();

	for (int i = 0; i < kNumButtons; i++) {
		const ButtonSetup &s = setup[i];
		MapButton &btn = _buttons[i];

		btn.x = s.x;  btn.y = s.y;
		btn.w = s.w;  btn.h = s.h;
		btn.isRle = s.isRle;
		btn.hovered = false;

		// Button 2 is always enabled for a saved game.
		// Practice can switch back to the active game only after one exists.
		if (i == 2 && isPracticeMode()) {
			btn.enabled = gs && !gs->_playerName.empty();
		} else {
			btn.enabled = true;
		}

		if (s.isRle) {
			btn.normalRle = new RleBlock();
			if (!btn.normalRle->load(Common::Path(s.normalName)))
				debug(2, "WorldMapPage: button %d normal rle failed", i);

			btn.hiliteRle = new RleBlock();
			if (!btn.hiliteRle->load(Common::Path(s.hiliteName)))
				debug(2, "WorldMapPage: button %d hilite rle failed", i);

			if (s.grayName) {
				btn.grayRle = new RleBlock();
				if (!btn.grayRle->load(Common::Path(s.grayName)))
					debug(2, "WorldMapPage: button %d gray rle failed", i);
			}
		} else {
			btn.normalBB = new BitBlock();
			if (!btn.normalBB->load(Common::Path(s.normalName)))
				debug(2, "WorldMapPage: button %d normal bb failed", i);

			btn.hiliteBB = new BitBlock();
			if (!btn.hiliteBB->load(Common::Path(s.hiliteName)))
				debug(2, "WorldMapPage: button %d hilite bb failed", i);
		}
	}
}

// ============================================================================
// Stats computation
// ============================================================================

void WorldMapPage::computeStats() {
	GameState *gs = _engine->getGameState();

	// The four rows are remaining, board A, board B, and completed counts.
	int boardA = 0;
	int boardB = 0;

	for (int i = 0; i < kBoardSize - kBoardCols; i++) {
		if (gs->_boardA[i] != nullptr)
			boardA++;
		if (gs->_boardB[i] != nullptr)
			boardB++;
	}

	_stats[3] = gs->_statC;
	_stats[2] = boardB;
	_stats[1] = boardA;
	_stats[0] = kMaxCombinations - _stats[3] - _stats[2] - _stats[1];
}

// ============================================================================
// Update
// ============================================================================

void WorldMapPage::update() {
	Common::Point mouse = _engine->getMousePos();

	if (_showQuitDialog) {
		_quitDialogButtonHover = hitTestQuitDialog(mouse.x, mouse.y);
		return;
	}

	if (_volumePanel) {
		if (_engine->getLastKeyPressed() == Common::KEYCODE_ESCAPE)
			closeVolumePanel(false);
		return;
	}

	_hoveredIcon = hitTestIcon(mouse);

	// Legend tabs are available in practice mode only.
	if (isPracticeMode()) {
		_hoveredLegendTab = hitTestLegendTab(mouse.x, mouse.y);
	} else {
		_hoveredLegendTab = 0;
	}

	for (int i = 0; i < kNumButtons; i++) {
		if (!_buttons[i].enabled) {
			_buttons[i].hovered = false;
			continue;
		}
		const MapButton &b = _buttons[i];
		_buttons[i].hovered = (mouse.x > b.x && mouse.x < b.x + b.w &&
		                       mouse.y > b.y && mouse.y < b.y + b.h);
	}
}

// ============================================================================
// Draw
// ============================================================================

void WorldMapPage::draw(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();
	GameState *gs = _engine->getGameState();

	// 1. Background
	if (_background) {
		_background->drawToSurface(screen, 0, 0);
	}

	// 2. Path segments
	if (isPracticeMode()) {
		drawPracticeSegments(screen, lut);
	} else {
		drawSavedGameSegments(screen, lut);
	}

	// 3. Stats panel and text
	if (isPracticeMode()) {
		// New-game mode: only draw stats_scr3 panel, no numbers or name.
		// Practice mode shows instructions without player statistics.
		if (_statsPractice && _statsPractice->isValid()) {
			_statsPractice->drawToScreen(screen, 10, 10, lut);
		}
	} else {
		// Load-game mode: draw stats_scr1 panel + player name + stat numbers.
		// Saved-game mode shows the player name and four progress rows.
		if (_statsSavedGame && _statsSavedGame->isValid()) {
			_statsSavedGame->drawToScreen(screen, 10, 10, lut);
		}

		if (_whiteFont && _whiteFont->isLoaded()) {
			// Player name centered at Y=10, min X=240
			const Common::String &name = gs->_playerName;
			int nameWidth = _whiteFont->getStringWidth(name);
			int nameX = 400 - nameWidth / 2;
			if (nameX < 240)
				nameX = 240;
			_whiteFont->drawString(screen, nameX, 10, name, lut);

			// Stat numbers (right-aligned at x=226, Y from kStatLabelY)
			for (int i = 0; i < 4; i++) {
				Common::String valStr = Common::String::format("%d", _stats[i]);
				int valWidth = _whiteFont->getStringWidth(valStr);
				_whiteFont->drawString(screen, 226 - valWidth, kStatLabelY[i],
				                       valStr, lut);
			}
		}
	}

	// 4. Practice legend
	// The legend highlights only the tab currently under the mouse.
	if (isPracticeMode()) {
		// Always draw legend_off base
		if (_legends[0] && _legends[0]->getWidth() > 0) {
			_legends[0]->drawToSurface(screen, 590, 427);
		}
		// The selected difficulty affects the route. Only the hovered tab is overlaid.
		int tabToShow = _hoveredLegendTab;
		if (1 <= tabToShow && tabToShow <= 3) {
			if (_legends[tabToShow] && _legends[tabToShow]->getWidth() > 0) {
				_legends[tabToShow]->drawToSurface(screen, 590, 427);
			}
		}
	}

	// 5. World icons
	for (int i = 0; i < kNumIcons; i++) {
		RleBlock *icon = _icons[i];
		if (icon && icon->isValid()) {
			icon->drawToScreen(screen, kIconHitRects[i].x,
			                   kIconHitRects[i].y, lut);
		}
	}

	// 6. Title overlay for hovered icon
	if (0 <= _hoveredIcon && _hoveredIcon < kNumTitles) {
		RleBlock *title = _titles[_hoveredIcon];
		if (title && title->isValid()) {
			title->drawToScreen(screen,
			                    kTitlePositions[_hoveredIcon].x,
			                    kTitlePositions[_hoveredIcon].y, lut);
		}
	}

	// 7. Bottom panel buttons
	for (int i = 0; i < kNumButtons; i++) {
		const MapButton &btn = _buttons[i];
		if (btn.isRle) {
			RleBlock *img = nullptr;
			if (!btn.enabled && btn.grayRle)
				img = btn.grayRle;
			else if (btn.hovered && btn.hiliteRle)
				img = btn.hiliteRle;
			else
				img = btn.normalRle;
			if (img && img->isValid())
				img->drawToScreen(screen, btn.x, btn.y, lut);
		} else {
			BitBlock *img = (btn.hovered && btn.hiliteBB) ? btn.hiliteBB : btn.normalBB;
			if (img && img->getWidth() > 0)
				img->drawToSurface(screen, btn.x, btn.y);
		}
	}

	// 8. Volume panel (if visible)
	if (_volumePanel) {
		const byte (*alphaLUT)[256] = _engine->getAlphaLUT();
		Common::Point mouse = _engine->getMousePos();
		const VolumePanelResult result = _volumePanel->drawAndHandleInput(screen,
		        mouse.x, mouse.y, _engine->isMouseDown(), _engine->isMouseClicked(),
		        alphaLUT);
		if (result == kVolumePanelChanged)
			applyVolumePanelVolumes(true);
		else if (result == kVolumePanelApply)
			closeVolumePanel(true);
		else if (result == kVolumePanelCancel)
			closeVolumePanel(false);
	}

	// 9. Quit dialog (if visible)
	if (_showQuitDialog) {
		drawQuitDialog(screen);
	}
}

// ============================================================================
// Practice route drawing
// ============================================================================

void WorldMapPage::drawPracticeSegments(Graphics::ManagedSurface *screen, const byte (*lut)[256]) {
	// Practice uses one difficulty for the full route and skips duplicate slot 13.
	int tier = _currentDifficulty;
	if (tier < 0 || kNumDiffTiers <= tier)
		tier = 0;

	for (int slot = 0; slot < kNumSegments; slot++) {
		if (slot == 13)
			continue;
		RleBlock *seg = _segments[tier][slot];
		if (seg && seg->isValid()) {
			seg->drawToScreen(screen,
			                  kSegmentPositions[slot].x,
			                  kSegmentPositions[slot].y, lut);
		}
	}
}

// ============================================================================
// Saved-game route drawing
// ============================================================================

void WorldMapPage::drawSavedGameSegments(Graphics::ManagedSurface *screen, const byte (*lut)[256]) {
	// Draw specific slots using each world's stored difficulty.
	// Draw order: 10, 1, 2, 3, 5, 4, 7, 6, 9, 8, 0, 11, 13
	// (Skips slot 12, draws slot 13 instead at same position.)
	GameState *gs = _engine->getGameState();

	static const int drawOrder[] = { 10, 1, 2, 3, 5, 4, 7, 6, 9, 8, 0, 11, 13 };
	for (int idx = 0; idx < 13; idx++) {
		int slot = drawOrder[idx];
		int worldId = kSegToWorld[slot];
		int tier = gs->_worldDataA[worldId];
		if (tier < 0 || kNumDiffTiers <= tier)
			tier = 0;

		RleBlock *seg = _segments[tier][slot];
		if (seg && seg->isValid()) {
			seg->drawToScreen(screen,
			                  kSegmentPositions[slot].x,
			                  kSegmentPositions[slot].y, lut);
		}
	}
}

// ============================================================================
// Click handling
// ============================================================================

void WorldMapPage::handleClick(const Common::Point &pos) {
	if (_showQuitDialog) {
		int quitBtn = hitTestQuitDialog(pos.x, pos.y);
		if (quitBtn == 1) {
			closeQuitDialog();
			_engine->setQuitAfterCredits(true);
			_engine->requestPageChange(kPageCredits);
		} else if (quitBtn == 2) {
			closeQuitDialog();
		}
		return;
	}

	if (_volumePanel) {
		return;
	}

	// Check practice difficulty tabs.
	if (isPracticeMode()) {
		int tab = hitTestLegendTab(pos.x, pos.y);
		if (1 <= tab && tab <= 3) {
			_currentDifficulty = tab;
			if (0 <= _blipSoundId)
				_engine->getSoundManager()->play(_blipSoundId);
			return;
		}
	}

	// Check buttons first
	int btnClicked = hitTestButton(pos);
	if (0 <= btnClicked) {
		if (0 <= _blipSoundId)
			_engine->getSoundManager()->play(_blipSoundId);

		switch (btnClicked) {
		case 0:
			debug(1, "WorldMapPage: Files button clicked");
			_engine->requestPageChange(kPageMenuOptions);
			break;
		case 1:
			debug(1, "WorldMapPage: Options button clicked");
			openVolumePanel();
			break;
		case 2:
			debug(1, "WorldMapPage: Game/Entraine button clicked (mode=%d)", _mode);
			if (isPracticeMode()) {
				_engine->requestPageChange(kPageMenuLoad);
			} else {
				_engine->requestPageChange(kPageMenuNew);
			}
			break;
		case 3:
			debug(1, "WorldMapPage: Quit button clicked");
			openQuitDialog();
			break;
		default:
			break;
		}
		return;
	}

	// Check icon clicks
	int clicked = hitTestIcon(pos);

	if (0 <= clicked && _iconClickable[clicked]) {
		debug(1, "WorldMapPage: Clicked world icon %d", clicked);

		if (0 <= _blipSoundId) {
			_engine->getSoundManager()->play(_blipSoundId);
		}

		// Saved-game navigation uses the selected world's stored difficulty.
		GameState *gs = _engine->getGameState();
		if (!isPracticeMode()) {
			_currentDifficulty = gs->_worldDataA[clicked];
		}
		gs->_gameMode = _currentDifficulty;

		switch (clicked) {
		case 0:
			_engine->requestPageChange(kPageZombiniville);
			break;
		case 4:
			_engine->requestPageChange(kPageRescue1);
			break;
		case 9:
			_engine->requestPageChange(kPageRescue2);
			break;
		case 12:
			_engine->requestPageChange(kPageBooliewood);
			break;
		default:
			_engine->_returningFromPuzzle = false;
			_engine->_maptransSourceWorld = clicked;
			_engine->requestPageChange(clicked);
			break;
		}
		return;
	}
}

// ============================================================================
// Hit-testing
// ============================================================================

int WorldMapPage::hitTestButton(const Common::Point &pos) const {
	for (int i = 0; i < kNumButtons; i++) {
		if (!_buttons[i].enabled)
			continue;
		const MapButton &b = _buttons[i];
		if (pos.x > b.x && pos.x < b.x + b.w &&
		    pos.y > b.y && pos.y < b.y + b.h)
			return i;
	}
	return -1;
}

int WorldMapPage::hitTestIcon(const Common::Point &pos) const {
	if (isPracticeMode()) {
		// Practice mode checks puzzle icons only.
		for (int i = 1; i < 12; i++) {
			if (!_iconClickable[i])
				continue;
			const IconRect &r = kIconHitRects[i];
			if (pos.x > r.x && pos.x < r.x + r.w &&
			    pos.y > r.y && pos.y < r.y + r.h) {
				return i;
			}
		}
	} else {
		// Saved-game mode checks all enabled hub and final icons.
		for (int i = 0; i < kNumIcons; i++) {
			if (!_iconClickable[i])
				continue;
			const IconRect &r = kIconHitRects[i];
			if (pos.x > r.x && pos.x < r.x + r.w &&
			    pos.y > r.y && pos.y < r.y + r.h) {
				return i;
			}
		}
	}
	return -1;
}

/**
 * Hit-test practice difficulty tabs.
 * Returns 1-3 for the difficulty tab, or 0 if none hit.
 */
int WorldMapPage::hitTestLegendTab(int x, int y) const {
	// Level 1: x in (605, 734), y in (430, 449)
	if (x > 605 && x < 734 && y > 430 && y < 449)
		return 1;
	// Level 2: x in (605, 734), y in (450, 468)
	if (x > 605 && x < 734 && y > 450 && y < 468)
		return 2;
	// Level 3: x in (597, 734), y in (472, 488)
	if (x > 597 && x < 734 && y > 472 && y < 488)
		return 3;
	return 0;
}

// ============================================================================
// Dialogs
// ============================================================================

void WorldMapPage::openVolumePanel() {
	if (!_volumePanel) {
		_volumePanel = new VolumePanel();
		_volumePanel->init();
		SoundManager *sound = _engine->getSoundManager();
		if (sound)
			_volumePanel->setInitialVolumes(sound->_volumeMusic, sound->_volumeSFX,
			                                sound->_volumeSpeech);
	}
}

void WorldMapPage::closeVolumePanel(bool applyChanges) {
	if (!_volumePanel)
		return;
	applyVolumePanelVolumes(applyChanges);
	delete _volumePanel;
	_volumePanel = nullptr;
}

void WorldMapPage::applyVolumePanelVolumes(bool usePanelValues) {
	if (!_volumePanel)
		return;
	SoundManager *sound = _engine->getSoundManager();
	if (!sound)
		return;
	sound->_volumeMusic = usePanelValues ? _volumePanel->getMusicVolume() :
	                                      _volumePanel->getInitialMusicVolume();
	sound->_volumeSFX = usePanelValues ? _volumePanel->getSfxVolume() :
	                                    _volumePanel->getInitialSfxVolume();
	sound->_volumeSpeech = usePanelValues ? _volumePanel->getSpeechVolume() :
	                                       _volumePanel->getInitialSpeechVolume();
	_engine->setMapMusicVolume(sound->_volumeMusic);
}

void WorldMapPage::openQuitDialog() {
	_showQuitDialog = true;
	_quitDialogButtonHover = 0;

	if (!_quitPanelNothing) {
		_quitPanelNothing = new RleBlock();
		if (!_quitPanelNothing->loadFromFile(Common::Path("bmp/menu/QUIT_panel_nothing.rb"))) {
			warning("WorldMapPage: Failed to load QUIT_panel_nothing");
		}
	}
	if (!_quitPanelOk) {
		_quitPanelOk = new RleBlock();
		if (!_quitPanelOk->loadFromFile(Common::Path("bmp/menu/QUIT_panel_ok.rb"))) {
			warning("WorldMapPage: Failed to load QUIT_panel_ok");
		}
	}
	if (!_quitPanelCancel) {
		_quitPanelCancel = new RleBlock();
		if (!_quitPanelCancel->loadFromFile(Common::Path("bmp/menu/QUIT_panel_cancel.rb"))) {
			warning("WorldMapPage: Failed to load QUIT_panel_cancel");
		}
	}

	if (!_quitTextQuit) {
		_quitTextQuit = new BitBlock();
		_quitTextQuit->load(Common::Path("bmp/menu/Quit_panel_text_quit"));
	}

	if (_quitPanelOk && _quitPanelOk->getWidth() > 0) {
		_quitDialogX = 400 - _quitPanelOk->getWidth() / 2;
		_quitDialogY = 300 - _quitPanelOk->getHeight() / 2;
	} else {
		_quitDialogX = 200;
		_quitDialogY = 200;
	}
}

void WorldMapPage::closeQuitDialog() {
	_showQuitDialog = false;
	_quitDialogButtonHover = 0;
}

void WorldMapPage::drawQuitDialog(Graphics::ManagedSurface *screen) {
	const byte (*alphaLUT)[256] = _engine->getAlphaLUT();

	RleBlock *panel = nullptr;
	switch (_quitDialogButtonHover) {
	case 1:  panel = _quitPanelOk;      break;
	case 2:  panel = _quitPanelCancel;   break;
	default: panel = _quitPanelNothing;  break;
	}

	if (panel) {
		panel->drawToScreen(screen, _quitDialogX, _quitDialogY, alphaLUT);
	}

	if (_quitTextQuit) {
		_quitTextQuit->drawToSurface(screen, _quitDialogX, _quitDialogY);
	}
}

int WorldMapPage::hitTestQuitDialog(int x, int y) const {
	if (x > _quitDialogX + 207 && x < _quitDialogX + 272 &&
	    y > _quitDialogY + 77 && y < _quitDialogY + 145) {
		return 1;  // OK
	}
	if (x > _quitDialogX + 287 && x < _quitDialogX + 352 &&
	    y > _quitDialogY + 77 && y < _quitDialogY + 145) {
		return 2;  // Cancel
	}
	return 0;
}

} // End of namespace Zoombini2
