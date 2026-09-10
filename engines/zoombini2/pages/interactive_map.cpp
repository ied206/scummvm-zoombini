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
#include "common/path.h"
#include "common/str.h"
#include "common/system.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/interactive_map.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// Static map-screen geometry and resource tables.
// ============================================================================

/**
 * Icon hit-test rectangles {left, top, right, bottom}.
 * The right and bottom edges remain exclusive, matching the original hit test.
 */
const Common::Rect InteractiveMap::kIconHitRects[kNumIcons] = {
	Common::Rect(155, 343, 194, 418), //  0 ShelterZombiniville
	Common::Rect(168, 259, 209, 301), //  1 CrazyTurtle
	Common::Rect(237, 206, 297, 256), //  2 Waterslide
	Common::Rect(305, 241, 359, 287), //  3 Aquacube
	Common::Rect(355, 290, 409, 335), //  4 Rescue1
	Common::Rect(441, 341, 470, 379), //  5 MysticMarsh
	Common::Rect(424, 235, 454, 268), //  6 MagicWall
	Common::Rect(517, 346, 559, 390), //  7 WallOfFleens
	Common::Rect(515, 215, 546, 262), //  8 ChezNorf
	Common::Rect(531, 277, 589, 321), //  9 Rescue2
	Common::Rect(591, 237, 639, 264), // 10 Snowboard
	Common::Rect(642, 190, 685, 235), // 11 Boolies
	Common::Rect(709, 48, 798, 124)   // 12 Booliewood
};

/**
 * Title sprite draw positions {x, y}.
 * Each entry supplies the title origin for one page icon.
 */
const Common::Point32 InteractiveMap::kTitlePos[kNumTitles] = {
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

/** Stat label Y positions. */
const int InteractiveMap::kStatLabelY[4] = {15, 39, 63, 90};

/**
 * Title sprite filenames per page (0-12).
 */
const char *const InteractiveMap::kTitleFiles[kNumTitles] = {
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

/**
 * Segment draw positions {x, y}.
 * Slots 12 and 13 intentionally share a position.
 * Slot 13 duplicates slot 12 (both 661, 101).
 */
const Common::Point32 InteractiveMap::kSegmentPos[kNumSegments] = {
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

/**
 * Segment-to-page mapping for saved-game page-level drawing.
 * The saved-game route uses each mapped page's stored level.
 * Index = segment slot, value = page ID (0-12).
 * Slot 12 is unused in saved-game mode.
 */
const int InteractiveMap::kSegmentPageIds[kNumSegments] = {
	1,  //  0: CrazyTurtle
	2,  //  1: Waterslide
	3,  //  2: Aquacube
	3,  //  3: Aquacube
	5,  //  4: MysticMarsh
	6,  //  5: MagicWall
	7,  //  6: WallOfFleens
	8,  //  7: ChezNorf
	7,  //  8: WallOfFleens
	8,  //  9: ChezNorf
	10, // 10: Snowboard
	11, // 11: Boolies
	11, // 12: unused in saved-game mode
	11  // 13: Boolies duplicate
};

const char *const InteractiveMap::kSegmentDirs[kNumLevelTiers] = {
	"bmp/map/04 neutral segments", // 0 = neutral (unvisited)
	"bmp/map/01 Easy segments",    // 1 = easy level
	"bmp/map/02 Medium segments",  // 2 = medium level
	"bmp/map/03 Hard segments"     // 3 = hard level
};

// Branch resources use b-before-a slot order rather than alphabetical suffix order.
const char *const InteractiveMap::kSegmentFiles[kNumSegments] = {
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

const char *const InteractiveMap::kLegendFiles[kNumLegends] = {
	"bmp/map/map_legend_off",    // 0
	"bmp/map/map_legend_level1", // 1
	"bmp/map/map_legend_level2", // 2
	"bmp/map/map_legend_level3"  // 3
};

// ============================================================================
// Construction / Destruction
// ============================================================================

InteractiveMap::InteractiveMap(Zoombini2Engine *vm, MapScreenMode mode)
	: InteractiveBase(vm), _mode(mode), _hoveredIcon(-1),
	  _currentLevel(1), _hoveredLegendTab(0), _background(nullptr),
	  _statsPractice(nullptr), _statsSavedGame(nullptr), _whiteFont(nullptr),
	  _blipSoundId(-1), _mapMusicId(-1), _volumePanel(nullptr),
	  _showQuitDialog(false), _quitDialogPos(),
	  _quitDialogButtonHover(0), _quitPanelNothing(nullptr),
	  _quitPanelOk(nullptr), _quitPanelCancel(nullptr), _quitTextQuit(nullptr), _quitDialogBackground(nullptr) {
	_pageId = kPageMapScreen;

	for (int i = 0; i < kNumIcons; i++) {
		_icons[i] = nullptr;
		_iconClickable[i] = false;
		_iconColored[i] = false;
	}
	for (int i = 0; i < kNumTitles; i++) {
		_titles[i] = nullptr;
	}
	for (int tier = 0; tier < kNumLevelTiers; tier++) {
		for (int slot = 0; slot < kNumSegments; slot++) {
			_segments[tier][slot] = nullptr;
		}
	}
	for (int i = 0; i < kNumLegends; i++) {
		_legends[i] = nullptr;
	}
	for (int i = 0; i < kNumButtons; i++) {
		_buttons[i].rect = Common::Rect();
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

InteractiveMap::~InteractiveMap() {
	if (_showQuitDialog)
		closeQuitDialog();
	delete _background;
	for (int i = 0; i < kNumIcons; i++) {
		delete _icons[i];
	}
	for (int i = 0; i < kNumTitles; i++) {
		delete _titles[i];
	}
	for (int tier = 0; tier < kNumLevelTiers; tier++) {
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

	SoundManager *sm = _vm->getSoundManager();
	if (sm && 0 <= _blipSoundId)
		sm->unload(_blipSoundId);

	delete _volumePanel;
	delete _quitPanelNothing;
	delete _quitPanelOk;
	delete _quitPanelCancel;
	delete _quitTextQuit;
	delete _quitDialogBackground;
}

// ============================================================================
// Initialization
// ============================================================================

void InteractiveMap::init() {
	debug(1, "MapScreenPage::init (mode=%d)", _mode);
	SoundManager *sm = _vm->getSoundManager();
	GameState *gs = _vm->getGameState();
	if (!isPracticeMode() && _vm->_isSavedGame)
		_vm->writeGameSave(gs->_playerName);
	_vm->clearGlobalZoombinis();
	_vm->_isSavedGame = !isPracticeMode();

	// The saved-game map always exposes the starting hub.
	if (!isPracticeMode()) {
		gs->_pageLevel[0] = 1;
	}

	// --- Background ---
	_background = new BitBlock();
	if (!_background->load(Common::Path("#bmp/Map/background"))) {
		warning("MapScreenPage: Failed to load map background");
	}

	// --- Page icons (one per route page: normal or gray, decided by mode) ---
	setupIcons();

	// --- Title overlays ---
	for (int i = 0; i < kNumTitles; i++) {
		_titles[i] = new RleBlock();
		if (!_titles[i]->load(Common::Path(kTitleFiles[i]))) {
			warning("MapScreenPage: Failed to load title %d!", i);
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

	// --- Set initial level ---
	if (isPracticeMode()) {
		_currentLevel = 1;
	} else {
		// Saved-game routes use per-page levels.
		_currentLevel = gs->getLevel();
	}

	// --- Audio ---
	_blipSoundId = sm->load(false, Common::Path("sounds/blip.wav"), false);

	_mapMusicId = _vm->ensureMapMusic();

	// --- Compute stats ---
	computeStats();

	// --- Bottom panel buttons ---
	loadButtons();
}

// ============================================================================
// Icon setup decides color and clickability for each mode.
// ============================================================================

void InteractiveMap::setupIcons() {
	GameState *gs = _vm->getGameState();

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
				warning("MapScreenPage: Failed to load icon %d at %s!", i, path.c_str());
			}
		}
	} else {
		// Saved-game mode:
		if (400 <= gs->_rescuedBoolieCount) {
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
			// Normal progress uses each page's visited state.
			for (int i = 0; i < kNumIcons; i++) {
				_iconClickable[i] = false;
				_iconColored[i] = false;

				Common::String path;
				if (gs->hasPageVisit(i, 1)) {
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
					warning("MapScreenPage: Failed to load icon %d at %s!", i, path.c_str());
				}
			}
		}
	}
}

bool InteractiveMap::practiceCandidateFitsPack(const ZmbTrait &traits) const {
	int counts[ZmbTrait::kTraitCount][ZmbTrait::kTraitValueCount + 1] = {};
	if (!traits.hasValidValues())
		return false;
	for (int traitOrdinal = 0; traitOrdinal < ZmbTrait::kTraitCount; traitOrdinal++) {
		const ZmbTrait::TraitIndex traitIndex = static_cast<ZmbTrait::TraitIndex>(traitOrdinal);
		counts[traitOrdinal][traits.getValue(traitIndex)] += 1;
	}

	const uint16 candidateHash = traits.calculateHash();
	int matchingCombinations = 0;
	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		const ZoombiniState *zoombini = _vm->_globalZoombinis[i];
		for (int traitOrdinal = 0; traitOrdinal < ZmbTrait::kTraitCount; traitOrdinal++) {
			const ZmbTrait::TraitIndex traitIndex = static_cast<ZmbTrait::TraitIndex>(traitOrdinal);
			const byte value = zoombini->_traits.getValue(traitIndex);
			if (value < 1 || ZmbTrait::kTraitValueCount < value)
				return false;
			counts[traitOrdinal][value] += 1;
		}
		if (zoombini->_traitHash == candidateHash)
			matchingCombinations += 1;
	}

	for (int traitOrdinal = 0; traitOrdinal < ZmbTrait::kTraitCount; traitOrdinal++) {
		for (int value = 1; value <= ZmbTrait::kTraitValueCount; value++) {
			if (5 < counts[traitOrdinal][value])
				return false;
		}
	}
	return matchingCombinations < 2;
}

Common::String InteractiveMap::generatePracticeZoombiniName() const {
	static const char *const kVowelPairs[30] = {
		"a ",
		"a ",
		"a ",
		"e ",
		"e ",
		"e ",
		"e ",
		"i ",
		"i ",
		"i ",
		"o ",
		"o ",
		"o ",
		"u ",
		"u ",
		"y ",
		"ee",
		"oo",
		"yo",
		"ya",
		"ye",
		"ei",
		"ie",
		"ai",
		"ia",
		"au",
		"ua",
		"uo",
		"ou",
		"ae",
	};
	static const char kSingleConsonants[] = "bbccdddfghjkkllmmnnprrssssttvwx";
	static const char kEndings[] = "aeiou";
	static const char *const kConsonantPairs[39] = {
		"bl",
		"br",
		"ch",
		"cl",
		"cr",
		"dr",
		"dw",
		"fl",
		"fr",
		"gh",
		"gl",
		"gr",
		"kl",
		"kn",
		"kr",
		"kw",
		"ld",
		"mp",
		"nd",
		"nh",
		"nn",
		"ph",
		"pl",
		"pr",
		"qu",
		"qu",
		"rh",
		"rn",
		"sc",
		"sl",
		"sm",
		"sn",
		"sp",
		"sr",
		"st",
		"sw",
		"th",
		"tr",
		"tw",
	};

	Common::RandomSource *randomSrc = _vm->getRandom();
	char name[8] = {};
	const int targetLength = randomSrc->getRandomNumber(1) + 4;
	bool useVowelPair = randomSrc->getRandomNumber(98) + 1 < 40;
	int length = 0;
	while (length < targetLength) {
		bool usedConsonantPair = false;
		if (useVowelPair) {
			useVowelPair = false;
			const char *pair = kVowelPairs[randomSrc->getRandomNumber(29)];
			if (pair[1] != ' ') {
				name[length] = pair[0];
				length += 1;
				name[length] = pair[1];
				length += 1;
			} else {
				name[length] = pair[0];
				name[length] = pair[0];
				length += 1;
			}
		} else {
			useVowelPair = true;
			if (1 < length || randomSrc->getRandomNumber(98) + 1 <= 33) {
				const char *pair = kConsonantPairs[randomSrc->getRandomNumber(38)];
				name[length] = pair[0];
				length += 1;
				name[length] = pair[1];
				length += 1;
				usedConsonantPair = true;
			} else {
				name[length] = kSingleConsonants[randomSrc->getRandomNumber(30)];
				length += 1;
			}
		}
		if (usedConsonantPair && targetLength <= length)
			name[length - 1] = kEndings[randomSrc->getRandomNumber(4)];
		if (length == 2 && name[0] == name[1])
			length = 1;
	}
	return Common::String(name);
}

void InteractiveMap::createPracticeParty(int pageId) {
	const int partySize = getPracticePartySize(pageId);
	_vm->clearGlobalZoombinis();
	if (partySize == 0)
		return;

	Common::RandomSource *randomSrc = _vm->getRandom();
	while (static_cast<int>(_vm->_globalZoombinis.size()) < partySize) {
		const byte hair = static_cast<byte>(randomSrc->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1);
		const byte eyes = static_cast<byte>(randomSrc->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1);
		const byte nose = static_cast<byte>(randomSrc->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1);
		const byte feet = static_cast<byte>(randomSrc->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1);
		const ZmbTrait traits(feet, nose, hair, eyes);
		if (!practiceCandidateFitsPack(traits))
			continue;

		ZoombiniState *zoombini = new ZoombiniState();
		zoombini->setTraits(traits);
		zoombini->_inputEnabled = 1;
		zoombini->_puzzleStatus = 0;
		zoombini->_animationCell = 33;
		_vm->_globalZoombinis.push_back(zoombini);
	}

	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		const Common::String name = generatePracticeZoombiniName();
		Common::strlcpy(_vm->_globalZoombinis[i]->_name, name.c_str(), sizeof(_vm->_globalZoombinis[i]->_name));
	}
}

// ============================================================================
// Segment loading
// ============================================================================

void InteractiveMap::loadSegments() {
	for (int tier = 0; tier < kNumLevelTiers; tier++) {
		for (int slot = 0; slot < kNumSegments; slot++) {
			Common::String path = Common::String::format(
				"%s/%s", kSegmentDirs[tier], kSegmentFiles[slot]);
			_segments[tier][slot] = new RleBlock();
			if (!_segments[tier][slot]->load(Common::Path(path))) {
				warning("MapScreenPage: Failed to load segment tier=%d slot=%d path=%s!",
						tier, slot, path.c_str());
			}
		}
	}
}

// ============================================================================
// Button loading
// ============================================================================

void InteractiveMap::loadButtons() {
	struct ButtonSetup {
		Common::Rect rect;
		bool isRle;
		const char *normalName;
		const char *hiliteName;
		const char *grayName;
	};

	const ButtonSetup setup[kNumButtons] = {
		// Button 0: Files / Parties
		{Common::Rect(27, 561, 172, 600), false,
		 "bmp/map/PANEL NL - Parties NORMAL",
		 "bmp/map/PANEL NL - Parties HILITE", nullptr},
		// Button 1: Options
		{Common::Rect(175, 561, 320, 600), false,
		 "bmp/map/PANEL NL - Options NORMAL",
		 "bmp/map/PANEL NL - Options HILITE", nullptr},
		// Button 2: Game in practice mode or Entraine in saved-game mode
		isPracticeMode()
			? ButtonSetup{Common::Rect(468, 561, 613, 600), true,
						  "bmp/map/PANEL NL - Game NORMAL",
						  "bmp/map/PANEL NL - Game HILITE",
						  "bmp/map/PANEL NL - Game Gray"}
			: ButtonSetup{Common::Rect(468, 561, 613, 600), false,
						  "bmp/map/PANEL NL - Entraine NORMAL",
						  "bmp/map/PANEL NL - Entraine HILITE", nullptr},
		// Button 3: Quitter
		{Common::Rect(613, 561, 758, 600), true,
		 "bmp/map/PANEL NL - Quitter NORMAL",
		 "bmp/map/PANEL NL - Quitter HILITE", nullptr},
	};

	GameState *gs = _vm->getGameState();

	for (int i = 0; i < kNumButtons; i++) {
		const ButtonSetup &s = setup[i];
		MapButton &btn = _buttons[i];

		btn.rect = s.rect;
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
				debug(2, "MapScreenPage: button %d normal rle failed", i);

			btn.hiliteRle = new RleBlock();
			if (!btn.hiliteRle->load(Common::Path(s.hiliteName)))
				debug(2, "MapScreenPage: button %d hilite rle failed", i);

			if (s.grayName) {
				btn.grayRle = new RleBlock();
				if (!btn.grayRle->load(Common::Path(s.grayName)))
					debug(2, "MapScreenPage: button %d gray rle failed", i);
			}
		} else {
			btn.normalBB = new BitBlock();
			if (!btn.normalBB->load(Common::Path(s.normalName)))
				debug(2, "MapScreenPage: button %d normal bb failed", i);

			btn.hiliteBB = new BitBlock();
			if (!btn.hiliteBB->load(Common::Path(s.hiliteName)))
				debug(2, "MapScreenPage: button %d hilite bb failed", i);
		}
	}
}

// ============================================================================
// Stats computation
// ============================================================================

void InteractiveMap::computeStats() {
	GameState *gs = _vm->getGameState();

	// The four rows are remaining, board A, board B, and completed counts.
	int boardA = 0;
	int boardB = 0;

	for (int i = 0; i < kBoardSize - kBoardCols; i++) {
		if (gs->_rescue1Board[i] != nullptr)
			boardA += 1;
		if (gs->_rescue2Board[i] != nullptr)
			boardB += 1;
	}

	_stats[3] = gs->_completedZoombiniCount;
	_stats[2] = boardB;
	_stats[1] = boardA;
	_stats[0] = kMaxCombinations - _stats[3] - _stats[2] - _stats[1];
}

// ============================================================================
// Update
// ============================================================================

void InteractiveMap::onUpdate() {
	const Common::Point32 mousePos = _vm->getMousePos();
	const Common::Point mouseEventPos(mousePos.x, mousePos.y);

	if (_showQuitDialog) {
		_quitDialogButtonHover = hitTestQuitDialog(mousePos);
		return;
	}

	if (_volumePanel) {
		return;
	}

	_hoveredIcon = hitTestIcon(mouseEventPos);

	// Legend tabs are available in practice mode only.
	if (isPracticeMode()) {
		_hoveredLegendTab = hitTestLegendTab(mousePos);
	} else {
		_hoveredLegendTab = 0;
	}

	for (int i = 0; i < kNumButtons; i++) {
		if (!_buttons[i].enabled) {
			_buttons[i].hovered = false;
			continue;
		}
		const MapButton &b = _buttons[i];
		_buttons[i].hovered = (b.rect.left < mouseEventPos.x && mouseEventPos.x < b.rect.right &&
							   b.rect.top < mouseEventPos.y && mouseEventPos.y < b.rect.bottom);
	}
}

// ============================================================================
// Draw
// ============================================================================

void InteractiveMap::onRenderScene(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();
	GameState *gs = _vm->getGameState();

	// 1. Background
	if (_background) {
		_background->drawToSurface(screen, Common::Point32(0, 0));
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
			_statsPractice->drawToScreen(screen, Common::Point32(10, 10), lut);
		}
	} else {
		// Load-game mode: draw stats_scr1 panel + player name + stat numbers.
		// Saved-game mode shows the player name and four progress rows.
		if (_statsSavedGame && _statsSavedGame->isValid()) {
			_statsSavedGame->drawToScreen(screen, Common::Point32(10, 10), lut);
		}

		if (_whiteFont && _whiteFont->isLoaded()) {
			// Player name centered at Y=10, min X=240
			const Common::String &name = gs->_playerName;
			int nameWidth = _whiteFont->getStringWidth(name);
			int nameX = 400 - nameWidth / 2;
			if (nameX < 240)
				nameX = 240;
			_whiteFont->drawString(screen, Common::Point32(nameX, 10), name, lut);

			// Stat numbers (right-aligned at x=226, Y from kStatLabelY)
			for (int i = 0; i < 4; i++) {
				Common::String valStr = Common::String::format("%d", _stats[i]);
				int valWidth = _whiteFont->getStringWidth(valStr);
				_whiteFont->drawString(screen, Common::Point32(226 - valWidth, kStatLabelY[i]),
									   valStr, lut);
			}
		}
	}

	// 4. Practice legend
	// The legend highlights only the tab currently under the mouse.
	if (isPracticeMode()) {
		// Always draw legend_off base
		if (_legends[0] && _legends[0]->getWidth() > 0) {
			_legends[0]->drawToSurface(screen, Common::Point32(590, 427));
		}
		// The selected level affects the route. Only the hovered tab is overlaid.
		int tabToShow = _hoveredLegendTab;
		if (1 <= tabToShow && tabToShow <= 3) {
			if (_legends[tabToShow] && _legends[tabToShow]->getWidth() > 0) {
				_legends[tabToShow]->drawToSurface(screen, Common::Point32(590, 427));
			}
		}
	}

	// 5. Page icons
	for (int i = 0; i < kNumIcons; i++) {
		RleBlock *icon = _icons[i];
		if (icon && icon->isValid()) {
			icon->drawToScreen(screen, Common::Point32(kIconHitRects[i].left, kIconHitRects[i].top), lut);
		}
	}

	// 6. Title overlay for hovered icon
	if (0 <= _hoveredIcon && _hoveredIcon < kNumTitles) {
		RleBlock *title = _titles[_hoveredIcon];
		if (title && title->isValid()) {
			title->drawToScreen(screen, kTitlePos[_hoveredIcon], lut);
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
				img->drawToScreen(screen, Common::Point32(btn.rect.left, btn.rect.top), lut);
		} else {
			BitBlock *img = (btn.hovered && btn.hiliteBB) ? btn.hiliteBB : btn.normalBB;
			if (img && img->getWidth() > 0)
				img->drawToSurface(screen, Common::Point32(btn.rect.left, btn.rect.top));
		}
	}
}

void InteractiveMap::onRenderForeground(ManagedSurface32 *screen) {
	if (_volumePanel) {
		const Common::Point32 mousePos(_vm->getMousePos());
		_volumePanel->draw(screen, mousePos, _vm->getAlphaLUT());
	}
	// 9. Quit dialog (if visible)
	if (_showQuitDialog) {
		drawQuitDialog(screen);
	}
}

// ============================================================================
// Practice route drawing
// ============================================================================

void InteractiveMap::drawPracticeSegments(ManagedSurface32 *screen, const AlphaBlendLUT &lut) {
	// Practice uses one level for the full route and skips duplicate slot 13.
	int tier = _currentLevel;
	if (tier < 0 || kNumLevelTiers <= tier)
		tier = 0;

	for (int slot = 0; slot < kNumSegments; slot++) {
		if (slot == 13)
			continue;
		RleBlock *seg = _segments[tier][slot];
		if (seg && seg->isValid()) {
			seg->drawToScreen(screen, kSegmentPos[slot], lut);
		}
	}
}

// ============================================================================
// Saved-game route drawing
// ============================================================================

void InteractiveMap::drawSavedGameSegments(ManagedSurface32 *screen, const AlphaBlendLUT &lut) {
	// Draw specific slots using each page's stored level.
	// Draw order: 10, 1, 2, 3, 5, 4, 7, 6, 9, 8, 0, 11, 13
	// (Skips slot 12, draws slot 13 instead at same position.)
	GameState *gs = _vm->getGameState();

	static const int drawOrder[] = {10, 1, 2, 3, 5, 4, 7, 6, 9, 8, 0, 11, 13};
	for (int idx = 0; idx < 13; idx++) {
		int slot = drawOrder[idx];
		int pageId = kSegmentPageIds[slot];
		int tier = gs->_pageLevel[pageId];
		if (tier < 0 || kNumLevelTiers <= tier)
			tier = 0;

		RleBlock *seg = _segments[tier][slot];
		if (seg && seg->isValid()) {
			seg->drawToScreen(screen, kSegmentPos[slot], lut);
		}
	}
}

// ============================================================================
// Click handling
// ============================================================================

EventHandleResult InteractiveMap::handleVolumePanelInput(const Common::Point &pos, bool mouseReleased) {
	if (!_volumePanel)
		return EventHandleResult::kPassthrough;
	const VolumePanelResult result = _volumePanel->handleMouseInput(Common::Point32(pos), _volumePanelMouseDown, mouseReleased);
	if (result == kVolumePanelChanged) {
		applyVolumePanelVolumes(true, false);
		_volumePanel->playPreviewSound();
	} else if (result == kVolumePanelApply) {
		closeVolumePanel(true);
	} else if (result == kVolumePanelCancel) {
		closeVolumePanel(false);
	}
	return EventHandleResult::kConsumed;
}

EventHandleResult InteractiveMap::onLButtonUp(const Common::Point &pos) {
	_volumePanelMouseDown = false;
	return handleVolumePanelInput(pos, true);
}

EventHandleResult InteractiveMap::onMouseMove(const Common::Point &pos) {
	if (!_volumePanel)
		return EventHandleResult::kPassthrough;
	_volumePanel->handleMouseInput(Common::Point32(pos), _volumePanelMouseDown, false);
	return EventHandleResult::kConsumed;
}

EventHandleResult InteractiveMap::onKeyDown(const Common::KeyState &key, bool repeat) {
	(void)repeat;
	if (!_volumePanel && !_showQuitDialog)
		return EventHandleResult::kPassthrough;
	if (key.keycode == Common::KEYCODE_ESCAPE) {
		if (_volumePanel)
			closeVolumePanel(false);
		else
			closeQuitDialog();
	}
	return EventHandleResult::kConsumed;
}
EventHandleResult InteractiveMap::onLButtonDown(const Common::Point &pos) {
	if (_showQuitDialog) {
		int quitBtn = hitTestQuitDialog(Common::Point32(pos));
		if (quitBtn == 1) {
			closeQuitDialog();
			_vm->requestPageChange(kPageCredits);
		} else if (quitBtn == 2) {
			closeQuitDialog();
		}
		return EventHandleResult::kConsumed;
	}

	if (_volumePanel) {
		_volumePanelMouseDown = true;
		_volumePanel->handleMouseInput(Common::Point32(pos), true, false);
		return EventHandleResult::kConsumed;
	}

	// Check practice level tabs.
	if (isPracticeMode()) {
		int tab = hitTestLegendTab(Common::Point32(pos));
		if (1 <= tab && tab <= 3) {
			_currentLevel = tab;
			if (0 <= _blipSoundId)
				_vm->getSoundManager()->play(_blipSoundId);
			return EventHandleResult::kConsumed;
		}
	}

	// Check buttons first
	int btnClicked = hitTestButton(pos);
	if (0 <= btnClicked) {
		if (0 <= _blipSoundId)
			_vm->getSoundManager()->play(_blipSoundId);

		switch (btnClicked) {
		case 0:
			debug(1, "MapScreenPage: Files button clicked");
			_vm->requestPageChange(kPageMenuOptions);
			break;
		case 1:
			debug(1, "MapScreenPage: Options button clicked");
			openVolumePanel();
			break;
		case 2:
			debug(1, "MapScreenPage: Game/Entraine button clicked (mode=%d)", _mode);
			if (isPracticeMode()) {
				_vm->requestPageChange(kPageMenuLoad);
			} else {
				_vm->requestPageChange(kPageMenuPractice);
			}
			break;
		case 3:
			debug(1, "MapScreenPage: Quit button clicked");
			openQuitDialog();
			break;
		default:
			break;
		}
		return EventHandleResult::kConsumed;
	}

	// Check icon clicks
	int clicked = hitTestIcon(pos);

	if (0 <= clicked && _iconClickable[clicked]) {
		debug(1, "MapScreenPage: Clicked page icon %d", clicked);

		if (0 <= _blipSoundId) {
			_vm->getSoundManager()->play(_blipSoundId);
		}

		// Saved-game navigation uses the selected page's stored level.
		GameState *gs = _vm->getGameState();
		if (!isPracticeMode()) {
			_currentLevel = gs->_pageLevel[clicked];
		}
		if (_currentLevel == 0)
			return EventHandleResult::kConsumed;
		gs->_level = _currentLevel;

		switch (clicked) {
		case 0:
			_vm->requestPageChange(kPageZombiniville);
			break;
		case 4:
			_vm->requestPageChange(kPageRescue1);
			break;
		case 9:
			_vm->requestPageChange(kPageRescue2);
			break;
		case 12:
			_vm->requestPageChange(kPageBooliewood);
			break;
		default:
			_vm->_returningFromPuzzle = false;
			if (isPracticeMode())
				createPracticeParty(clicked);
			_vm->_mapTransitionSourcePageId = static_cast<PageId>(clicked);
			_vm->requestPageChange(clicked);
			break;
		}
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

// ============================================================================
// Hit-testing
// ============================================================================

int InteractiveMap::hitTestButton(const Common::Point &pos) const {
	for (int i = 0; i < kNumButtons; i++) {
		if (!_buttons[i].enabled)
			continue;
		const MapButton &b = _buttons[i];
		if (b.rect.left < pos.x && pos.x < b.rect.right &&
			b.rect.top < pos.y && pos.y < b.rect.bottom)
			return i;
	}
	return -1;
}

int InteractiveMap::hitTestIcon(const Common::Point &pos) const {
	if (isPracticeMode()) {
		// Practice mode checks puzzle icons only.
		for (int i = 1; i < 12; i++) {
			if (!_iconClickable[i])
				continue;
			const Common::Rect &r = kIconHitRects[i];
			if (r.left < pos.x && pos.x < r.right &&
				r.top < pos.y && pos.y < r.bottom) {
				return i;
			}
		}
	} else {
		// Saved-game mode checks all enabled hub and final icons.
		for (int i = 0; i < kNumIcons; i++) {
			if (!_iconClickable[i])
				continue;
			const Common::Rect &r = kIconHitRects[i];
			if (r.left < pos.x && pos.x < r.right &&
				r.top < pos.y && pos.y < r.bottom) {
				return i;
			}
		}
	}
	return -1;
}

/**
 * Hit-test practice level tabs.
 * Returns 1-3 for the level tab, or 0 if none hit.
 */

int InteractiveMap::hitTestLegendTab(const Common::Point32 &pos) const {
	// Level 1: x in (605, 734), y in (430, 449)
	if (605 < pos.x && pos.x < 734 && 430 < pos.y && pos.y < 449)
		return 1;
	// Level 2: x in (605, 734), y in (450, 468)
	if (605 < pos.x && pos.x < 734 && 450 < pos.y && pos.y < 468)
		return 2;
	// Level 3: x in (597, 734), y in (472, 488)
	if (597 < pos.x && pos.x < 734 && 472 < pos.y && pos.y < 488)
		return 3;
	return 0;
}

// ============================================================================
// Dialogs
// ============================================================================

void InteractiveMap::openVolumePanel() {
	if (!_volumePanel) {
		_volumePanel = new VolumePanel();
		_volumePanel->init(_vm->getSoundManager());
		_volumePanel->setInitialVolumes(_vm->getMusicVolume(), _vm->getSFXVolume(), _vm->getSpeechVolume());
	}
}

void InteractiveMap::closeVolumePanel(bool applyChanges) {
	if (!_volumePanel)
		return;
	applyVolumePanelVolumes(applyChanges, applyChanges);
	delete _volumePanel;
	_volumePanel = nullptr;
}

void InteractiveMap::applyVolumePanelVolumes(bool usePanelValues, bool persistChanges) {
	if (!_volumePanel)
		return;
	const int music = usePanelValues ? _volumePanel->getMusicVolume() : _volumePanel->getInitialMusicVolume();
	const int sfx = usePanelValues ? _volumePanel->getSfxVolume() : _volumePanel->getInitialSfxVolume();
	const int speech = usePanelValues ? _volumePanel->getSpeechVolume() : _volumePanel->getInitialSpeechVolume();
	if (persistChanges)
		_vm->saveSoundVolumes(music, sfx, speech);
	else
		_vm->previewSoundVolumes(music, sfx, speech);
}

void InteractiveMap::openQuitDialog() {
	if (_showQuitDialog)
		return;
	_quitDialogButtonHover = 0;

	if (!_quitPanelNothing) {
		_quitPanelNothing = new RleBlock();
		if (!_quitPanelNothing->loadFromFile(Common::Path("bmp/menu/QUIT_panel_nothing.rb"))) {
			warning("MapScreenPage: Failed to load QUIT_panel_nothing");
		}
	}
	if (!_quitPanelOk) {
		_quitPanelOk = new RleBlock();
		if (!_quitPanelOk->loadFromFile(Common::Path("bmp/menu/QUIT_panel_ok.rb"))) {
			warning("MapScreenPage: Failed to load QUIT_panel_ok");
		}
	}
	if (!_quitPanelCancel) {
		_quitPanelCancel = new RleBlock();
		if (!_quitPanelCancel->loadFromFile(Common::Path("bmp/menu/QUIT_panel_cancel.rb"))) {
			warning("MapScreenPage: Failed to load QUIT_panel_cancel");
		}
	}

	if (!_quitTextQuit) {
		_quitTextQuit = new BitBlock();
		_quitTextQuit->load(Common::Path("bmp/menu/Quit_panel_text_quit"));
	}

	if (_quitPanelOk && _quitPanelOk->getWidth() > 0) {
		_quitDialogPos = Common::Point32(400 - _quitPanelOk->getWidth() / 2, 300 - _quitPanelOk->getHeight() / 2);
	} else {
		_quitDialogPos = Common::Point32(200, 200);
	}

	if (!_quitDialogBackground)
		_quitDialogBackground = new Graphics::ManagedSurface();
	_quitDialogBackground->copyFrom(*_vm->getCurrentScreen());
	_showQuitDialog = true;
	_vm->_isPaused = true;
	_vm->_pauseTimeStart = g_system->getMillis();
	_vm->getSoundManager()->pauseAll();
}

void InteractiveMap::closeQuitDialog() {
	if (!_showQuitDialog)
		return;
	_showQuitDialog = false;
	_quitDialogButtonHover = 0;
	if (_quitDialogBackground)
		_vm->getCurrentScreen()->copyFrom(*_quitDialogBackground);
	_vm->addPauseTime(g_system->getMillis() - _vm->_pauseTimeStart);
	_vm->_isPaused = false;
	_vm->getSoundManager()->resumeAll();
}

void InteractiveMap::drawQuitDialog(ManagedSurface32 *screen) {
	const AlphaBlendLUT &alphaLUT = _vm->getAlphaLUT();
	if (_quitDialogBackground)
		screen->copyFrom(*_quitDialogBackground);

	RleBlock *panel = nullptr;
	switch (_quitDialogButtonHover) {
	case 1:
		panel = _quitPanelOk;
		break;
	case 2:
		panel = _quitPanelCancel;
		break;
	default:
		panel = _quitPanelNothing;
		break;
	}

	if (panel) {
		panel->drawToScreen(screen, _quitDialogPos, alphaLUT);
	}

	if (_quitTextQuit) {
		_quitTextQuit->drawToSurface(screen, Common::Point32(_quitDialogPos.x + 17, _quitDialogPos.y + 17));
	}
}

int InteractiveMap::hitTestQuitDialog(const Common::Point32 &pos) const {
	if (_quitDialogPos.x + 207 < pos.x && pos.x < _quitDialogPos.x + 272 &&
		_quitDialogPos.y + 77 < pos.y && pos.y < _quitDialogPos.y + 145) {
		return 1; // OK
	}
	if (_quitDialogPos.x + 287 < pos.x && pos.x < _quitDialogPos.x + 352 &&
		_quitDialogPos.y + 77 < pos.y && pos.y < _quitDialogPos.y + 145) {
		return 2; // Cancel
	}
	return 0;
}

int InteractiveMap::getPracticePartySize(int pageId) {
	switch (pageId) {
	case kPageCrazyTurtle:
	case kPageWaterSlide:
	case kPageAquaCube:
		return kMaxPackSize;
	case kPageMysticMarsh:
	case kPageMagicWall:
	case kPageWallOfFleens:
	case kPageChezNorf:
	case kPageSnowboard:
	case kPageBoolies:
		return kPostRescuePackSize;
	default:
		return 0;
	}
}

} // End of namespace Zoombini2
