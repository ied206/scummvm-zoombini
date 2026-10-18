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

#include "common/callback.h"
#include "common/debug.h"
#include "common/path.h"
#include "common/str.h"

#include "graphics/font.h"
#include "zoombini2/graphics.h"
#include "zoombini2/pages/dialog_msgbox.h"
#include "zoombini2/pages/interactive_map.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/text_manager.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *InteractiveMap::kBackgroundPath;
constexpr const char *InteractiveMap::kPracticeStatsPath;
constexpr const char *InteractiveMap::kJourneyStatsPath;
constexpr const char *InteractiveMap::kBlipSoundPath;
constexpr const char *InteractiveMap::kDisabledIconFormat;
constexpr const char *InteractiveMap::kIconFormat;
constexpr const char *InteractiveMap::kSegmentPathFormat;
constexpr const char *InteractiveMap::kFilesNormalPath;
constexpr const char *InteractiveMap::kFilesHighlightPath;
constexpr const char *InteractiveMap::kOptionsNormalPath;
constexpr const char *InteractiveMap::kOptionsHighlightPath;
constexpr const char *InteractiveMap::kGameNormalPath;
constexpr const char *InteractiveMap::kGameHighlightPath;
constexpr const char *InteractiveMap::kGameDisabledPath;
constexpr const char *InteractiveMap::kPracticeNormalPath;
constexpr const char *InteractiveMap::kPracticeHighlightPath;
constexpr const char *InteractiveMap::kQuitNormalPath;
constexpr const char *InteractiveMap::kQuitHighlightPath;
constexpr const char *InteractiveMap::kQuitConfirmationPath;

/**
 * Title sprite draw positions {x, y}.
 * Each entry supplies the title origin for one page icon.
 */
constexpr Common::Point32 InteractiveMap::kTitlePos[kNumTitles];

/** Stat label Y positions. */
constexpr int InteractiveMap::kStatLabelY[4];

/**
 * Title sprite filenames per page (0-12).
 */
constexpr const char *InteractiveMap::kTitleFiles[];

/**
 * Segment draw positions {x, y}.
 * Slots 12 and 13 intentionally share a position.
 * Slot 13 duplicates slot 12 (both 661, 101).
 */
constexpr Common::Point32 InteractiveMap::kSegmentPos[kNumSegments];

/** Journey route drawing order and difficulty sources. */
constexpr InteractiveMap::RouteSegmentDraw InteractiveMap::kJourneyRouteSegments[];
constexpr const char *InteractiveMap::kSegmentDirs[];

// Branch resources use b-before-a slot order rather than alphabetical suffix order.
constexpr const char *InteractiveMap::kSegmentFiles[];
constexpr const char *InteractiveMap::kLegendFiles[];
constexpr RGBColor InteractiveMap::kL4Color;

// ============================================================================
// Static map-screen geometry and resource tables.
// ============================================================================

/**
 * Icon hit-test rectangles {left, top, right, bottom}.
 * The right and bottom edges remain exclusive, matching the original hit test.
 */
const Common::Rect InteractiveMap::kIconHitRects[kNumIcons] = {
	Common::Rect(155, 343, 194, 418), //<  0 Zombiniville
	Common::Rect(168, 259, 209, 301), //<  1 CrazyTurtle
	Common::Rect(237, 206, 297, 256), //<  2 Waterslide
	Common::Rect(305, 241, 359, 287), //<  3 Aquacube
	Common::Rect(355, 290, 409, 335), //<  4 RescueSite1
	Common::Rect(441, 341, 470, 379), //<  5 MysticMarsh
	Common::Rect(424, 235, 454, 268), //<  6 MagicWall
	Common::Rect(517, 346, 559, 390), //<  7 WallOfFleens
	Common::Rect(515, 215, 546, 262), //<  8 ChezNorf
	Common::Rect(531, 277, 589, 321), //<  9 RescueSite2
	Common::Rect(591, 237, 639, 264), //< 10 Snowboard
	Common::Rect(642, 190, 685, 235), //< 11 Boolies
	Common::Rect(709, 48, 798, 124),  //< 12 Booliewood
};
// ============================================================================
// Construction / Destruction
// ============================================================================

InteractiveMap::InteractiveMap(Zoombini2Engine *vm, GameMode mode)
	: InteractiveBase(vm), _mode(mode) {
	if (inPracticeMode())
		_pageId = PageId::kPageMapPractice_M3;
	else
		_pageId = PageId::kPageMapJourney_M4;
}

InteractiveMap::~InteractiveMap() {
	SoundManager *sm = _vm->getSoundManager();
	if (sm && 0 <= _blipSoundId)
		sm->unload(_blipSoundId);

	delete _volumePanel;
}

// ============================================================================
// Initialization
// ============================================================================

void InteractiveMap::init() {
	debug(1, "MapScreenPage::init (mode=%s)", inPracticeMode() ? "practice" : "journey");
	SoundManager *sm = _vm->getSoundManager();
	GameState *gs = _vm->_state;
	if (inJourneyMode() && _vm->inJourneyMode())
		_vm->writeGameSave(gs->getPlayerName());
	_vm->_state->clearActiveZoombinis();
	_vm->setGameMode(_mode);
	if (inPracticeMode()) {
		PageId practicePageId = PageId::kPageNone_M1;
		int practiceLevel = 0;
		uint practicePartySize = 0;
		if (_vm->takePracticePuzzleLaunch(practicePageId, practiceLevel, practicePartySize)) {
			_currentLevel = practiceLevel;
			gs->_level = _currentLevel;
			_vm->_returningFromPuzzle = false;
			createPracticeParty(_vm, practicePageId, practicePartySize);
			_vm->_mapTransitSrcPageId = practicePageId;
			_vm->requestPageChange(practicePageId);
			return;
		}
	}

	// The journey map always exposes the starting hub.
	if (inJourneyMode()) {
		gs->setPageLevel(PageId::kPageZombiniville_00, 1);
	}

	// --- Background ---
	if (!_vm->_gfx->loadBackground(kBackgroundPath)) {
		warning("MapScreenPage: Failed to load map background");
	}

	// --- Page icons (one per route page: normal or gray, decided by mode) ---
	setupIcons();

	// --- Title overlays ---
	for (int i = 0; i < kNumTitles; i++) {
		const Common::String path(kTitleFiles[i]);
		if (_vm->_gfx->loadPageRleBlock(path))
			_titles[i] = path;
		else {
			_titles[i].clear();
			warning("MapScreenPage: Failed to load title %d!", i);
		}
	}

	// Path segment overlays contain 4 tiers with 14 slots each.
	loadSegments();

	// --- Stats overlays (both always loaded) ---
	// Practice and journey maps use different statistics panels.
	if (_vm->_gfx->loadPageRleBlock(kPracticeStatsPath))
		_statsPracticePath = kPracticeStatsPath;
	else
		_statsPracticePath.clear();
	if (_vm->_gfx->loadPageRleBlock(kJourneyStatsPath))
		_statsJourneyPath = kJourneyStatsPath;
	else
		_statsJourneyPath.clear();

	// --- Legend bitmaps (all 4: off, level1, level2, level3) ---
	for (int i = 0; i < kNumLegends; i++) {
		const Common::String path(kLegendFiles[i]);
		if (_vm->_gfx->loadPageBitBlock(path))
			_legends[i] = path;
		else
			_legends[i].clear();
	}

	// --- Set initial level ---
	if (inPracticeMode()) {
		_currentLevel = _vm->getPracticeLevel();
		if (_currentLevel == 4 && !_vm->allowCutL4PracticePuzzles()) {
			_currentLevel = 3;
			_vm->setPracticeLevel(_currentLevel);
		}
	} else {
		// Journey routes use per-page levels.
		_currentLevel = gs->getLevel();
	}

	// --- Audio ---
	_blipSoundId = sm->load(false, Common::Path(kBlipSoundPath), false);

	startMapMusic();

	// --- Compute stats ---
	computeStats();

	// --- Bottom panel buttons ---
	loadButtons();
}

// ============================================================================
// Icon setup decides color and clickability for each mode.
// ============================================================================

void InteractiveMap::setupIcons() {
	GameState *gs = _vm->_state;

	if (inPracticeMode()) {
		// Practice mode:
		// Hubs (0,4,9,12): gray, not clickable.
		// Puzzles (1-3, 5-8, 10-11): colored, clickable.
		for (int i = 0; i < kNumIcons; i++) {
			bool isHub = (i == 0 || i == 4 || i == 9 || i == 12);
			_iconColored[i] = !isHub;
			_iconClickable[i] = !isHub;

			Common::String path;
			if (isHub) {
				path = Common::String::format(kDisabledIconFormat, i);
			} else {
				path = Common::String::format(kIconFormat, i);
			}
			_icons[i] = _vm->_gfx->loadPageRleBlock(path) ? path : Common::String();
			if (_icons[i].empty()) {
				warning("MapScreenPage: Failed to load icon %d at %s!", i, path.c_str());
			}
		}
	} else {
		// Journey mode:
		if (400 <= gs->_rescuedBoolieCount) {
			// Endgame state: all gray except Booliewood (icon 12).
			for (int i = 0; i < kNumIcons; i++) {
				_iconClickable[i] = false;
				_iconColored[i] = false;

				Common::String path;
				if (i == 12) {
					path = Common::String::format(kIconFormat, i);
					_iconClickable[i] = true;
					_iconColored[i] = true;
				} else {
					path = Common::String::format(kDisabledIconFormat, i);
				}
				_icons[i] = _vm->_gfx->loadPageRleBlock(path) ? path : Common::String();
			}
		} else {
			// Normal progress uses each page's visited state.
			for (int i = 0; i < kNumIcons; i++) {
				_iconClickable[i] = false;
				_iconColored[i] = false;

				Common::String path;
				if (gs->hasPageVisit(static_cast<PageId>(i), 1)) {
					path = Common::String::format(kIconFormat, i);
					_iconColored[i] = true;
					// Only hub icons become clickable when visited.
					if (i == 0 || i == 4 || i == 9 || i == 12) {
						_iconClickable[i] = true;
					}
				} else {
					path = Common::String::format(kDisabledIconFormat, i);
				}
				_icons[i] = _vm->_gfx->loadPageRleBlock(path) ? path : Common::String();
				if (_icons[i].empty()) {
					warning("MapScreenPage: Failed to load icon %d at %s!", i, path.c_str());
				}
			}
		}
	}
}

bool InteractiveMap::practiceCandidateFitsPack(const Zoombini2Engine *vm, const ZmbTrait &traits) {
	int counts[ZmbTrait::kTraitKindCount][ZmbTrait::kTraitValueCount + 1] = {};
	if (!traits.hasValidValues())
		return false;
	for (int traitOrdinal = 0; traitOrdinal < ZmbTrait::kTraitKindCount; traitOrdinal++) {
		const ZmbTrait::TraitKind traitIndex = static_cast<ZmbTrait::TraitKind>(traitOrdinal);
		counts[traitOrdinal][traits.getValue(traitIndex)] += 1;
	}

	const uint16 candidateHash = traits.calculateHash();
	int matchingCombinations = 0;
	for (uint i = 0; i < vm->_state->_activeZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = vm->_state->_activeZoombinis[i];
		for (int traitOrdinal = 0; traitOrdinal < ZmbTrait::kTraitKindCount; traitOrdinal++) {
			const ZmbTrait::TraitKind traitIndex = static_cast<ZmbTrait::TraitKind>(traitOrdinal);
			const byte value = zoombini->getTraits().getValue(traitIndex);
			if (value < 1 || ZmbTrait::kTraitValueCount < value)
				return false;
			counts[traitOrdinal][value] += 1;
		}
		if (zoombini->getTraitHash() == candidateHash)
			matchingCombinations += 1;
	}

	for (int traitOrdinal = 0; traitOrdinal < ZmbTrait::kTraitKindCount; traitOrdinal++) {
		for (int value = 1; value <= ZmbTrait::kTraitValueCount; value++) {
			if (5 < counts[traitOrdinal][value])
				return false;
		}
	}
	return matchingCombinations < 2;
}

void InteractiveMap::createPracticeParty(Zoombini2Engine *vm, PageId pageId, uint partySize) {
	const uint maxPartySize = getPracticePartySize(pageId);
	vm->_state->clearActiveZoombinis();
	if (maxPartySize == 0)
		return;
	if (partySize == 0)
		partySize = maxPartySize;

	while (vm->_state->_activeZoombinis.size() < partySize) {
		const byte hair = static_cast<byte>(vm->_rnd->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1);
		const byte eyes = static_cast<byte>(vm->_rnd->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1);
		const byte nose = static_cast<byte>(vm->_rnd->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1);
		const byte feet = static_cast<byte>(vm->_rnd->getRandomNumber(ZmbTrait::kTraitValueCount - 1) + 1);
		const ZmbTrait traits(feet, nose, hair, eyes);
		if (!practiceCandidateFitsPack(vm, traits))
			continue;

		ZoombiniRunner *zoombini = new ZoombiniRunner(vm);
		zoombini->setTraits(traits);
		zoombini->setInputEnabled(true);
		zoombini->setCanAdvanceFromPage(false);
		zoombini->setAnimationCell(33);
		vm->_state->_activeZoombinis.push_back(zoombini);
	}

	for (uint i = 0; i < vm->_state->_activeZoombinis.size(); i++) {
		const Common::String name = GameState::generateZoombiniName(*vm->_rnd);
		vm->_state->_activeZoombinis[i]->setName(name.c_str());
	}
}

// ============================================================================
// Segment loading
// ============================================================================

void InteractiveMap::loadSegments() {
	for (int tier = 0; tier < kNumLevelTiers; tier++) {
		for (int slot = 0; slot < kNumSegments; slot++) {
			Common::String path = Common::String::format(
				kSegmentPathFormat, kSegmentDirs[tier], kSegmentFiles[slot]);
			_segments[tier][slot] = _vm->_gfx->loadPageRleBlock(path) ? path : Common::String();
			if (_segments[tier][slot].empty()) {
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
		/** Button hit rectangle in map-page pixels. */
		Common::Rect rect;
		/** Whether this button loads RLE sprites rather than uncompressed bitmaps. */
		bool isRle;
		/** Resource key for the button's normal appearance. */
		const char *normalName;
		/** Resource key for the button's hover appearance. */
		const char *hiliteName;
		/** Resource key for the disabled appearance, or null when none is supplied. */
		const char *grayName;
	};

	ButtonSetup setup[kNumButtons] = {
		// Button 0: Files / Parties
		{Common::Rect(27, 561, 172, 600), false,
		 kFilesNormalPath,
		 kFilesHighlightPath, nullptr},
		// Button 1: Options
		{Common::Rect(175, 561, 320, 600), false,
		 kOptionsNormalPath,
		 kOptionsHighlightPath, nullptr},
		{},
		// Button 3: Quitter
		{Common::Rect(613, 561, 758, 600), true,
		 kQuitNormalPath,
		 kQuitHighlightPath, nullptr},
	};
	// Button 2: Game in practice mode or Entraine in journey mode
	if (inPracticeMode())
		setup[2] = ButtonSetup{Common::Rect(468, 561, 613, 600), true,
							   kGameNormalPath,
							   kGameHighlightPath,
							   kGameDisabledPath};
	else
		setup[2] = ButtonSetup{Common::Rect(468, 561, 613, 600), false,
							   kPracticeNormalPath,
							   kPracticeHighlightPath, nullptr};

	GameState *gs = _vm->_state;

	for (int i = 0; i < kNumButtons; i++) {
		const ButtonSetup &s = setup[i];
		MapButton &btn = _buttons[i];

		btn.rect = s.rect;
		btn.isRle = s.isRle;
		btn.hovered = false;

		// Button 2 is always enabled for a saved game.
		// Practice can switch back to the active game only after one exists.
		if (i == 2 && inPracticeMode()) {
			btn.enabled = gs && !gs->getPlayerName().empty();
		} else {
			btn.enabled = true;
		}

		if (s.isRle) {
			btn.normalPath = _vm->_gfx->loadPageRleBlock(s.normalName) ? s.normalName : "";
			if (btn.normalPath.empty())
				debug(2, "MapScreenPage: button %d normal rle failed", i);

			btn.hilitePath = _vm->_gfx->loadPageRleBlock(s.hiliteName) ? s.hiliteName : "";
			if (btn.hilitePath.empty())
				debug(2, "MapScreenPage: button %d hilite rle failed", i);

			if (s.grayName) {
				btn.grayPath = _vm->_gfx->loadPageRleBlock(s.grayName) ? s.grayName : "";
				if (btn.grayPath.empty())
					debug(2, "MapScreenPage: button %d gray rle failed", i);
			}
		} else {
			btn.normalPath = _vm->_gfx->loadPageBitBlock(s.normalName) ? s.normalName : "";
			if (btn.normalPath.empty())
				debug(2, "MapScreenPage: button %d normal bb failed", i);

			btn.hilitePath = _vm->_gfx->loadPageBitBlock(s.hiliteName) ? s.hiliteName : "";
			if (btn.hilitePath.empty())
				debug(2, "MapScreenPage: button %d hilite bb failed", i);
		}
	}
}

// ============================================================================
// Stats computation
// ============================================================================

void InteractiveMap::computeStats() {
	GameState *gs = _vm->_state;

	// The four rows are remaining, Rescue I storage, Rescue II storage, and completed counts.
	int rescue1StorageCount = 0;
	int rescue2StorageCount = 0;

	for (int i = 0; i < GameState::kStorageSize - GameState::kStorageCols; i++) {
		if (gs->_rescue1Storage[i] != nullptr)
			rescue1StorageCount += 1;
		if (gs->_rescue2Storage[i] != nullptr)
			rescue2StorageCount += 1;
	}

	_stats[3] = gs->_completedZoombiniCount;
	_stats[2] = rescue2StorageCount;
	_stats[1] = rescue1StorageCount;
	_stats[0] = TraitComboTable::kZoombiniCombinationCount - _stats[3] - _stats[2] - _stats[1];
}

// ============================================================================
// Update
// ============================================================================

void InteractiveMap::onUpdate() {
	if (inPracticeMode() && _currentLevel == 4 && !_vm->allowCutL4PracticePuzzles()) {
		_currentLevel = 3;
		_vm->setPracticeLevel(_currentLevel);
	}
	const Common::Point32 mousePos = _vm->getMousePos();
	const Common::Point mouseEventPos(mousePos.x, mousePos.y);

	if (_volumePanel) {
		return;
	}

	_hoveredIcon = hitTestIcon(mouseEventPos);

	// Legend tabs are available in practice mode only.
	if (inPracticeMode()) {
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

void InteractiveMap::onRenderContent(ManagedSurface32 *screen) {
	GameState *gs = _vm->_state;

	// 1. Background
	_vm->_gfx->drawBackground(screen, Common::Point32(0, 0));

	// 2. Path segments
	if (inPracticeMode()) {
		drawPracticeSegments(screen);
	} else {
		drawJourneySegments(screen);
	}

	// 3. Stats panel and text
	if (inPracticeMode()) {
		// New-game mode: only draw stats_scr3 panel, no numbers or name.
		// Practice mode shows instructions without player statistics.
		if (!_statsPracticePath.empty())
			_vm->_gfx->drawPageRleBlock(screen, _statsPracticePath, Common::Point32(10, 10));
	} else {
		// Load-game mode: draw stats_scr1 panel + player name + stat numbers.
		// Journey mode shows the player name and four progress rows.
		if (!_statsJourneyPath.empty())
			_vm->_gfx->drawPageRleBlock(screen, _statsJourneyPath, Common::Point32(10, 10));

		Gfx::TextConf conf;
		conf.fontKind = Gfx::FontKind::kBmtFont;
		conf.fillColor = Gfx::PredefinedColor::kWhite03.toPixel(screen->format);
		// Player name centered at Y=10, min X=240
		const Common::String &name = gs->getPlayerName();
		int nameWidth = _vm->_gfx->getTextWidth(name, conf);
		int nameX = 400 - nameWidth / 2;
		if (nameX < 240)
			nameX = 240;
		_vm->_gfx->drawString(screen, name, Common::Point32(nameX, 10), Gfx::kNoMaxWidth, conf);

		// Stat numbers (right-aligned at x=226, Y from kStatLabelY)
		for (int i = 0; i < ARRAYSIZE(_stats); i++) {
			Common::String valStr = Common::String::format("%d", _stats[i]);
			int valWidth = _vm->_gfx->getTextWidth(valStr, conf);
			_vm->_gfx->drawString(screen, valStr, Common::Point32(226 - valWidth, kStatLabelY[i]), Gfx::kNoMaxWidth, conf);
		}
	}

	// 4. Practice legend
	// The legend highlights only the tab currently under the mouse.
	if (inPracticeMode()) {
		// Always draw legend_off base
		if (!_legends[0].empty())
			_vm->_gfx->drawPageBitBlock(screen, _legends[0], Common::Point32(590, 427));
		// The selected level affects the route. Only the hovered tab is overlaid.
		int tabToShow = _hoveredLegendTab;
		if (1 <= tabToShow && tabToShow <= 3) {
			if (!_legends[tabToShow].empty())
				_vm->_gfx->drawPageBitBlock(screen, _legends[tabToShow], Common::Point32(590, 427));
		}
		if (_vm->allowCutL4PracticePuzzles()) {
			drawL4Legend(screen);
		}
	}

	// 5. Page icons
	for (int i = 0; i < kNumIcons; i++) {
		if (!_icons[i].empty())
			_vm->_gfx->drawPageRleBlock(screen, _icons[i], Common::Point32(kIconHitRects[i].left, kIconHitRects[i].top));
	}

	// 6. Title overlay for hovered icon
	if (0 <= _hoveredIcon && _hoveredIcon < kNumTitles) {
		if (!_titles[_hoveredIcon].empty())
			_vm->_gfx->drawPageRleBlock(screen, _titles[_hoveredIcon], kTitlePos[_hoveredIcon]);
	}

	// 7. Bottom panel buttons
	for (int i = 0; i < kNumButtons; i++) {
		const MapButton &btn = _buttons[i];
		if (btn.isRle) {
			Common::String imagePath = btn.normalPath;
			if (!btn.enabled && !btn.grayPath.empty())
				imagePath = btn.grayPath;
			else if (btn.hovered && !btn.hilitePath.empty())
				imagePath = btn.hilitePath;
			if (!imagePath.empty())
				_vm->_gfx->drawPageRleBlock(screen, imagePath, Common::Point32(btn.rect.left, btn.rect.top));
		} else {
			const Common::String &imagePath = btn.hovered && !btn.hilitePath.empty() ? btn.hilitePath : btn.normalPath;
			if (!imagePath.empty())
				_vm->_gfx->drawPageBitBlock(screen, imagePath, Common::Point32(btn.rect.left, btn.rect.top));
		}
	}
}

void InteractiveMap::onRenderForeground(ManagedSurface32 *screen) {
	if (_volumePanel) {
		const Common::Point32 mousePos(_vm->getMousePos());
		_volumePanel->draw(screen, mousePos, _vm->getAlphaLUT());
	}
}

// ============================================================================
// Practice route drawing
// ============================================================================

void InteractiveMap::drawPracticeSegments(ManagedSurface32 *screen) {
	// Practice skips duplicate slot 13 and draws every route at the selected level.

	for (int slot = 0; slot < kNumSegments; slot++) {
		if (slot == 13)
			continue;
		const int level = _currentLevel;
		const int tier = MIN(level, kNumLevelTiers - 1);
		if (_segments[tier][slot].empty())
			continue;
		if (level == 4) {
			RleBlock *segment = _vm->_gfx->loadPageRleBlock(_segments[tier][slot]);
			if (segment)
				segment->drawToScreenSolidColor(screen, kSegmentPos[slot], kL4Color, _vm->getAlphaLUT());
		} else {
			_vm->_gfx->drawPageRleBlock(screen, _segments[tier][slot], kSegmentPos[slot]);
		}
	}
}

bool InteractiveMap::selectPracticeLevel(int level) {
	if (level < 1 || 4 < level || (level == 4 && !_vm->allowCutL4PracticePuzzles()))
		return false;
	_currentLevel = level;
	_vm->setPracticeLevel(level);
	if (0 <= _blipSoundId)
		_vm->getSoundManager()->play(_blipSoundId);
	return true;
}

void InteractiveMap::drawL4Legend(ManagedSurface32 *screen) const {
	if (!screen)
		return;

	const uint32 fillColor = kL4Color.toPixel(screen->format);
	const uint32 outlineColor = screen->format.ARGBToColor(255, 255, 242, 0); // #FFF200

	_vm->_gfx->fillSlantedRect(screen, Common::Rect32(kL4TabLeft, kL4TabTop, kL4TabRight, kL4TabBottom), -kL4TabSlant, fillColor, true);

	// Frame a slanted rect on hover.
	if (_hoveredLegendTab == 4) {
		_vm->_gfx->drawLine(screen, Common::Point32(kL4TabLeft, kL4TabTop), Common::Point32(kL4TabRight, kL4TabTop),
							outlineColor, kL4TabBorderThickness, true);
		_vm->_gfx->drawLine(screen, Common::Point32(kL4TabLeft - kL4TabSlant, kL4TabBottom), Common::Point32(kL4TabRight - kL4TabSlant, kL4TabBottom),
							outlineColor, kL4TabBorderThickness, true);
		_vm->_gfx->drawLine(screen, Common::Point32(kL4TabLeft, kL4TabTop), Common::Point32(kL4TabLeft - kL4TabSlant, kL4TabBottom),
							outlineColor, kL4TabBorderThickness, true);
		_vm->_gfx->drawLine(screen, Common::Point32(kL4TabRight, kL4TabTop), Common::Point32(kL4TabRight - kL4TabSlant, kL4TabBottom),
							outlineColor, kL4TabBorderThickness, true);
	}

	// Draw a label explaining restoration of cut level - Level4.
	Gfx::TextConf conf;
	conf.fontKind = Gfx::FontKind::kGuiFont;
	conf.fillColor = fillColor;
	conf.outlineEffect = true;
	conf.outlineColor = outlineColor;
	const int fontHeight = _vm->_gfx->getFontHeight(conf);
	if (fontHeight <= 0)
		return;

	const Common::U32String &label = _vm->_text->getLocalizedString(TextManager::kRestoredLevelLabel);

	const int textX = kL4TabRight + kL4TabTextMargin;
	int textY = kL4TabTop + (kL4TabBottom - kL4TabTop - fontHeight) / 2;
	// In some releases, L1-L3 label can take up to two lines.
	// Our L4 label should be moved to bottom a bit in that case.
	switch (_vm->getLanguage()) {
	case Common::KO_KOR:
		textY += 12;
		break;
	case Common::PL_POL:
		textY += 8;
		break;
	default:
		break;
	}

	int textRightEdge;
	switch (_vm->getLanguage()) {
	case Common::KO_KOR:
		textRightEdge = kL4TabTextRightEdgeKo;
		break;
	default:
		textRightEdge = kL4TabTextRightEdgeEn;
		break;
	}
	const int maxWidth = textRightEdge - textX - kL4TabTextMargin;
	if (maxWidth <= 0)
		return;

	switch (_vm->getLanguage()) {
	case Common::KO_KOR:
		conf.hAlign = Graphics::kTextAlignCenter;
		break;
	default:
		conf.hAlign = Graphics::kTextAlignLeft;
		break;
	}
	_vm->_gfx->drawString(screen, label, Common::Point32(textX, textY), maxWidth, conf);
}

// ============================================================================
// Journey route drawing
// ============================================================================

void InteractiveMap::drawJourneySegments(ManagedSurface32 *screen) {
	GameState *gs = _vm->_state;

	for (int i = 0; i < ARRAYSIZE(kJourneyRouteSegments); i++) {
		const RouteSegmentDraw &segment = kJourneyRouteSegments[i];
		const int slot = segment.segmentSlot;
		int tier = gs->getPageLevel(segment.levelPageId);
		if (tier < 0 || kNumLevelTiers <= tier)
			tier = 0;

		if (!_segments[tier][slot].empty())
			_vm->_gfx->drawPageRleBlock(screen, _segments[tier][slot], kSegmentPos[slot]);
	}
}

// ============================================================================
// Click handling
// ============================================================================

EventHandleResult InteractiveMap::handleVolumePanelInput(const Common::Point &pos, bool mouseReleased) {
	if (!_volumePanel)
		return EventHandleResult::kPassthrough;
	const VolumePanelResult result = _volumePanel->handleMouseInput(Common::Point32(pos), _volumePanelMouseDown, mouseReleased);
	switch (result) {
	case kVolumePanelChanged:
		applyVolumePanelVolumes(true, false);
		_volumePanel->playPreviewSound();
		break;
	case kVolumePanelApply:
		closeVolumePanel(true);
		break;
	case kVolumePanelCancel:
		closeVolumePanel(false);
		break;
	default:
		break;
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
	if (_volumePanel) {
		if (key.keycode == Common::KEYCODE_ESCAPE)
			closeVolumePanel(false);
		return EventHandleResult::kConsumed;
	}
	if (!_vm->useEnhancedKbdShortcuts())
		return EventHandleResult::kPassthrough;
	if (repeat)
		return EventHandleResult::kConsumed;
	if ((key.flags & Common::KBD_CTRL) != 0 && key.keycode == Common::KEYCODE_p) {
		if (inPracticeMode()) {
			if (_buttons[2].enabled)
				_vm->requestPageChange(PageId::kPageMapJourney_M4);
		} else {
			_vm->requestPageChange(PageId::kPageMapPractice_M3);
		}
		return EventHandleResult::kConsumed;
	}
	if (inPracticeMode() && key.hasFlags(0)) {
		int level = 0;
		switch (key.keycode) {
		case Common::KEYCODE_1:
		case Common::KEYCODE_KP1:
			level = 1;
			break;
		case Common::KEYCODE_2:
		case Common::KEYCODE_KP2:
			level = 2;
			break;
		case Common::KEYCODE_3:
		case Common::KEYCODE_KP3:
			level = 3;
			break;
		case Common::KEYCODE_4:
		case Common::KEYCODE_KP4:
			level = 4;
			break;
		default:
			break;
		}
		if (selectPracticeLevel(level))
			return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}
EventHandleResult InteractiveMap::onLButtonDown(const Common::Point &pos) {
	if (_volumePanel) {
		_volumePanelMouseDown = true;
		_volumePanel->handleMouseInput(Common::Point32(pos), true, false);
		return EventHandleResult::kConsumed;
	}

	// Check practice level tabs.
	if (inPracticeMode()) {
		const int tab = hitTestLegendTab(Common::Point32(pos));
		if (selectPracticeLevel(tab)) {
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
			_vm->requestPageChange(PageId::kPageMenuOptions_M2);
			break;
		case 1:
			debug(1, "MapScreenPage: Options button clicked");
			openVolumePanel();
			break;
		case 2:
			debug(1, "MapScreenPage: Game/Entraine button clicked (mode=%s)", inPracticeMode() ? "practice" : "journey");
			if (inPracticeMode()) {
				_vm->requestPageChange(PageId::kPageMapJourney_M4);
			} else {
				_vm->requestPageChange(PageId::kPageMapPractice_M3);
			}
			break;
		case 3:
			debug(1, "MapScreenPage: Quit button clicked");
			requestQuitConfirmation();
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

		// Journey navigation uses the selected page's stored level.
		GameState *gs = _vm->_state;
		if (inJourneyMode()) {
			_currentLevel = gs->getPageLevel(static_cast<PageId>(clicked));
		}
		if (_currentLevel == 0)
			return EventHandleResult::kConsumed;
		gs->_level = _currentLevel;

		switch (clicked) {
		case 0:
			_vm->requestPageChange(PageId::kPageZombiniville_00);
			break;
		case 4:
			_vm->requestPageChange(PageId::kPageRescue1_04);
			break;
		case 9:
			_vm->requestPageChange(PageId::kPageRescue2_09);
			break;
		case 12:
			_vm->requestPageChange(PageId::kPageBooliewood_12);
			break;
		default:
			_vm->_returningFromPuzzle = false;
			if (inPracticeMode())
				createPracticeParty(_vm, static_cast<PageId>(clicked));
			_vm->_mapTransitSrcPageId = static_cast<PageId>(clicked);
			_vm->requestPageChange(static_cast<PageId>(clicked));
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
	if (inPracticeMode()) {
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
		// Journey mode checks all enabled hub and final icons.
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
 * Returns 1-4 for the level tab, or 0 if none hit.
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
	if (_vm->allowCutL4PracticePuzzles() && kL4TabLeft - kL4TabSlant < pos.x && pos.x < kL4TabRight &&
		kL4TabTop < pos.y && pos.y < kL4TabBottom)
		return 4;
	return 0;
}

// ============================================================================
// Dialogs
// ============================================================================

void InteractiveMap::openVolumePanel() {
	if (!_volumePanel) {
		_volumePanel = new VolumePanel(_vm);
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

void InteractiveMap::requestQuitConfirmation() {
	_vm->requestMsgBox(Common::Path(kQuitConfirmationPath),
					   new Common::Callback<InteractiveMap, DialogMsgBoxButton>(this, &InteractiveMap::handleQuitConfirmation));
}

void InteractiveMap::handleQuitConfirmation(DialogMsgBoxButton button) {
	if (button == DialogMsgBoxButton::kOkay01)
		_vm->requestPageChange(PageId::kPageCredits_16);
}

uint InteractiveMap::getPracticePartySize(PageId pageId) {
	switch (pageId) {
	case PageId::kPageCrazyTurtle_01:
	case PageId::kPageWaterslide_02:
	case PageId::kPageAquacube_03:
		return kMaxPackSize;
	case PageId::kPageMysticMarsh_05:
	case PageId::kPageMagicWall_06:
	case PageId::kPageWallOfFleens_07:
	case PageId::kPageChezNorf_08:
	case PageId::kPageSnowboard_10:
	case PageId::kPageBoolies_11:
		return kPostRescuePackSize;
	default:
		return 0;
	}
}

} // End of namespace Zoombini2
