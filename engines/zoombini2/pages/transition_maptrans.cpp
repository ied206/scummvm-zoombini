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
#include "common/util.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/transition_maptrans.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *TransitionMapTrans::kSpeechFormat;
constexpr const char *TransitionMapTrans::kRouteFormat;
constexpr const char *TransitionMapTrans::kMusicPath;
constexpr const char *TransitionMapTrans::kMapTransitionOverlayPathFormat;
constexpr const char *TransitionMapTrans::kMapTransitionBackgroundPathFormat;
constexpr int TransitionMapTrans::kFinalPageRescueThreshold;

const TransitionMapTrans::RouteDest TransitionMapTrans::kRouteDestinations[] = {
	{"crazyturtle", PageId::kPageCrazyTurtle_01, PageId::kPageZombiniville_00, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kAnyCount},
	{"waterslide", PageId::kPageWaterslide_02, PageId::kPageCrazyTurtle_01, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kAnyCount},
	{"aquacube", PageId::kPageAquacube_03, PageId::kPageWaterslide_02, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kAnyCount},
	{"rescue1", PageId::kPageRescue1_04, PageId::kPageAquacube_03, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kAnyCount},
	{"mysticmarsh", PageId::kPageMysticMarsh_05, PageId::kPageRescue1_04, Zoombini2Engine::RouteBranch::kRight02, RouteRescueCondition::kAnyCount},
	{"magicwall", PageId::kPageMagicWall_06, PageId::kPageRescue1_04, Zoombini2Engine::RouteBranch::kLeft01, RouteRescueCondition::kAnyCount},
	{"walloffleens", PageId::kPageWallOfFleens_07, PageId::kPageMysticMarsh_05, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kAnyCount},
	{"cheznorf", PageId::kPageChezNorf_08, PageId::kPageMagicWall_06, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kAnyCount},
	{"rescue2", PageId::kPageRescue2_09, PageId::kPageWallOfFleens_07, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kAnyCount},
	{"rescue2norf", PageId::kPageRescue2_09, PageId::kPageChezNorf_08, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kAnyCount},
	{"snowboard", PageId::kPageSnowboard_10, PageId::kPageRescue2_09, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kAnyCount},
	{"boolies", PageId::kPageBoolies_11, PageId::kPageSnowboard_10, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kAnyCount},
	{"booliewood", PageId::kPageBooliewood_12, PageId::kPageBoolies_11, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kBelowFinalThreshold},
	{"final", PageId::kPageFinal_23, PageId::kPageBoolies_11, Zoombini2Engine::RouteBranch::kNone00, RouteRescueCondition::kAtOrAboveFinalThreshold}};

// ============================================================================
// TransitionMapTrans - route-map transition.
// ============================================================================

TransitionMapTrans::TransitionMapTrans(Zoombini2Engine *vm)
	: TransitionBase(vm) {
	_pageId = PageId::kPageMapTrans_22;
}

TransitionMapTrans::~TransitionMapTrans() {
	cleanupSpeech();
	cleanupPaths();
}

void TransitionMapTrans::queueSpeech(const Common::String &name) {
	SoundManager *sound = _vm->getSoundManager();
	if (!sound)
		return;
	const Common::Path path(Common::String::format(kSpeechFormat, name.c_str()));
	_speechIds.push_back(sound->load(true, path, false));
}

void TransitionMapTrans::queueTravelSpeech(PageId srcPage) {
	GameState *state = _vm->_state;
	if (!state)
		return;

	switch (srcPage) {
	case PageId::kPageZombiniville_00:
		if (!state->hasPageVisit(PageId::kPageCrazyTurtle_01, 1)) {
			queueSpeech(kSpeechTur11);
		} else {
			if (state->hasPageVisit(PageId::kPageBooliewood_12, 1))
				queueSpeech(kSpeechZbv31_3);
			else
				queueSpeech(kSpeechZbv31_2);
			queueSpeech(Common::String::format(kSpeechTur21Format, getRandomBinarySpeechVariant()));
		}
		break;
	case PageId::kPageCrazyTurtle_01:
		if (state->hasPageVisit(PageId::kPageWaterslide_02, 1))
			queueSpeech(Common::String::format(kSpeechWsl21Format, getRandomBinarySpeechVariant()));
		else
			queueSpeech(kSpeechWsl11);
		break;
	case PageId::kPageWaterslide_02:
		if (!state->hasPageVisit(PageId::kPageAquacube_03, 1)) {
			queueSpeech(kSpeechAqu11_1_1);
			queueSpeech(kSpeechAqu11_1_2);
			queueSpeech(kSpeechAqu11_1_3);
		} else {
			queueSpeech(Common::String::format(kSpeechAqu21Format, getRandomBinarySpeechVariant()));
		}
		break;
	case PageId::kPageAquacube_03:
		queueSpeech(kSpeechAqu31);
		break;
	case PageId::kPageRescue1_04:
		if (_vm->_routeDirection == Zoombini2Engine::RouteBranch::kLeft01) {
			if (state->hasPageVisit(PageId::kPageMagicWall_06, 1))
				queueSpeech(Common::String::format(kSpeechMgw21Format, getRandomBinarySpeechVariant()));
			else
				queueSpeech(kSpeechMgw11);
		} else {
			if (state->hasPageVisit(PageId::kPageMysticMarsh_05, 1))
				queueSpeech(Common::String::format(kSpeechMym21Format, getRandomBinarySpeechVariant()));
			else
				queueSpeech(kSpeechMym11);
		}
		break;
	case PageId::kPageMysticMarsh_05:
		if (state->hasPageVisit(PageId::kPageWallOfFleens_07, 1))
			queueSpeech(Common::String::format(kSpeechWlf21Format, getRandomBinarySpeechVariant()));
		else
			queueSpeech(kSpeechWlf11);
		break;
	case PageId::kPageMagicWall_06:
		if (!state->hasPageVisit(PageId::kPageChezNorf_08, 1)) {
			queueSpeech(kSpeechCzn11_1);
			queueSpeech(kSpeechCzn11_2);
		} else {
			queueSpeech(Common::String::format(kSpeechCzn21Format, getRandomBinarySpeechVariant()));
		}
		break;
	case PageId::kPageWallOfFleens_07:
		queueSpeech(kSpeechBc211);
		break;
	case PageId::kPageChezNorf_08:
		queueSpeech(kSpeechBc212);
		break;
	case PageId::kPageRescue2_09:
		if (!state->hasPageVisit(PageId::kPageSnowboard_10, 1)) {
			queueSpeech(kSpeechSwb11);
			queueSpeech(kSpeechSwb11B);
		} else {
			queueSpeech(Common::String::format(kSpeechSwb21Format, getRandomBinarySpeechVariant()));
		}
		break;
	case PageId::kPageSnowboard_10:
		if (!state->hasPageVisit(PageId::kPageBoolies_11, 1)) {
			queueSpeech(kSpeechBlp11);
			queueSpeech(kSpeechBlp11B);
		} else {
			queueSpeech(Common::String::format(kSpeechBlp21Format, getRandomBinarySpeechVariant()));
		}
		break;
	case PageId::kPageBoolies_11:
		if (!state->hasPageVisit(PageId::kPageBooliewood_12, 1)) {
			queueSpeech(kSpeechBlw11_1);
			queueSpeech(kSpeechBlw11_3);
		} else {
			queueSpeech(Common::String::format(kSpeechBlw12Format, _vm->_rnd->getRandomNumber(2) + 1));
		}
		break;
	default:
		break;
	}
}

int TransitionMapTrans::getRandomBinarySpeechVariant() {
	return _vm->_rnd->getRandomNumber(1) == 0 ? 2 : 1;
}

void TransitionMapTrans::updateSpeechQueue() {
	SoundManager *sound = _vm->getSoundManager();
	if (!sound) {
		_activeSpeechIndex = -1;
		_nextSpeechIndex = _speechIds.size();
		return;
	}
	if (_activeSpeechIndex != -1) {
		const int soundId = _speechIds[_activeSpeechIndex];
		if (0 <= soundId && sound->isPlaying(soundId))
			return;
		if (0 <= soundId)
			sound->unload(soundId);
		_speechIds[_activeSpeechIndex] = -1;
		_activeSpeechIndex = -1;
	}
	while (_nextSpeechIndex < _speechIds.size()) {
		const uint speechIndex = _nextSpeechIndex;
		_nextSpeechIndex += 1;
		if (_speechIds[speechIndex] < 0)
			continue;
		_activeSpeechIndex = static_cast<int>(speechIndex);
		sound->playWithVolume(_speechIds[speechIndex], sound->getSpeechVolume());
		return;
	}
}

bool TransitionMapTrans::hasPendingSpeech() const {
	return _activeSpeechIndex != -1 || _nextSpeechIndex < _speechIds.size();
}

void TransitionMapTrans::cleanupSpeech() {
	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		for (uint i = 0; i < _speechIds.size(); i++) {
			if (0 <= _speechIds[i]) {
				sound->stop(_speechIds[i]);
				sound->unload(_speechIds[i]);
			}
		}
	}
	_speechIds.clear();
	_nextSpeechIndex = 0;
	_activeSpeechIndex = -1;
}

void TransitionMapTrans::cleanupPaths() {
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[i];
		if (!zoombini)
			continue;
		zoombini->clearMovement();
		zoombini->resetAnimation();
		zoombini->setHidden(false);
	}
}

void TransitionMapTrans::init() {
	debug(1, "MapTransition::init (source=%d, route=%d)",
		  static_cast<int>(_vm->_mapTransitSrcPageId), static_cast<int>(_vm->_routeDirection));

	const PageId src = _vm->_mapTransitSrcPageId;
	const Zoombini2Engine::RouteBranch routeBranch = _vm->_routeDirection;
	_vm->_skipMode = false;
	_transitionFinished = false;
	cleanupSpeech();

	// Select the map region from the source page.
	int mapRegion;
	if (PageId::kPageZombiniville_00 <= src && src <= PageId::kPageAquacube_03)
		mapRegion = 1;
	else if (PageId::kPageRescue1_04 <= src && src <= PageId::kPageChezNorf_08)
		mapRegion = 2;
	else
		mapRegion = 3;

	// Select the walking path from the source page.
	Common::String patName;
	switch (src) {
	case PageId::kPageZombiniville_00:
		patName = "tr1 - map1.pat";
		break;
	case PageId::kPageCrazyTurtle_01:
		patName = "tr2 - map1.pat";
		break;
	case PageId::kPageWaterslide_02:
		patName = "tr3 - map1.pat";
		break;
	case PageId::kPageAquacube_03:
		patName = "tr4 - map1.pat";
		break;
	case PageId::kPageRescue1_04:
		switch (routeBranch) {
		case Zoombini2Engine::RouteBranch::kLeft01:
			patName = "tr5 - map2.pat";
			break;
		case Zoombini2Engine::RouteBranch::kRight02:
			patName = "tr8 - map2.pat";
			break;
		default:
			break;
		}
		break;
	case PageId::kPageMysticMarsh_05:
		patName = "tr9 - map2.pat";
		break;
	case PageId::kPageMagicWall_06:
		patName = "tr6 - map2.pat";
		break;
	case PageId::kPageWallOfFleens_07:
		patName = "tr10 - map2.pat";
		break;
	case PageId::kPageChezNorf_08:
		patName = "tr7 - map2.pat";
		break;
	case PageId::kPageRescue2_09:
		patName = "tr11 - map3.pat";
		break;
	case PageId::kPageSnowboard_10:
		patName = "tr12 - map3.pat";
		break;
	case PageId::kPageBoolies_11:
		patName = "tr13 - map3.pat";
		break;
	default:
		patName = "Transition02.pat";
		break;
	}
	_patPath = Common::Path(Common::String::format(kRouteFormat, patName.c_str()));

	_targetPageId = getDestPage(src, routeBranch, _vm->_state->_rescuedBoolieCount);
	if (_targetPageId == PageId::kPageNone_M1) {
		warning("MapTransition: refusing invalid source/branch combination (%d, %d)",
				static_cast<int>(src), static_cast<int>(routeBranch));
		_vm->requestPageChange(src);
		return;
	}

	// Compose the background and overlays for this transition.
	composeMapTransitionBackground(src, mapRegion);

	if (!_vm->_gfx->getCommonZoombiniAnimation(ZmbAnimationKind::kMapTransTiny))
		warning("MapTransition: Failed to load PitiZomb3.anm");

	// Preserve the previous page grid for cleanup, then install the tiny MapTrans travel grid.
	const uint32 now = _vm->getTotalPlayTime();
	const uint numZoombinis = _vm->_state->_activeZoombinis.size();
	for (uint i = 0; i < numZoombinis; i++) {
		ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[i];
		zoombini->clearMovement();
		zoombini->setHidden(true);
		zoombini->setInputEnabled(false);
		zoombini->startDirectionTrackedAnimation(now);
		zoombini->setActiveAnimation(ZmbAnimationKind::kMapTransTiny);
	}
	_nextWalkIndex = 0;
	_nextWalkTime = 0;
	_completedCount = 0;

	debug(1, "MapTransition: source=%d -> target=%d (region %d, path=%s, zoombinis=%u)",
		  static_cast<int>(src), static_cast<int>(_targetPageId), mapRegion, _patPath.toString().c_str(), numZoombinis);
	queueTravelSpeech(src);

	// Play transition music
	startPageMusic(Common::Path(kMusicPath));
	updateSpeechQueue();
}

void TransitionMapTrans::composeMapTransitionBackground(PageId srcPageId, int mapRegion) {
	const Common::String bgPath = Common::String::format(kMapTransitionBackgroundPathFormat, mapRegion);
	if (!_vm->_gfx->loadBackground(bgPath, Gfx::kScreenSize)) {
		warning("MapTransition: Failed to load background %s", bgPath.c_str());
		ManagedSurface32 *fallback = _vm->_gfx->createBackground(Gfx::kScreenSize);
		_vm->_gfx->fillRect(fallback, Common::Rect32(0, 0, Gfx::kScreenSize.width, Gfx::kScreenSize.height), 0);
	}

	drawMapOverlays(_vm->_gfx->getBackgroundSurface(), srcPageId, mapRegion);
}

/** Draw one map-overlay RLE sprite retained for the current page. */
void TransitionMapTrans::drawOverlaySprite(ManagedSurface32 *dst, const Common::String &name, const Common::Point32 &pos) {
	const Common::String overlayPath = Common::String::format(kMapTransitionOverlayPathFormat, name.c_str());

	RleBlock *overlay = _vm->_gfx->loadPageRleBlock(overlayPath);
	if (overlay) {
		_vm->_gfx->drawRleBlock(dst, overlay, pos);
	} else {
		debug(2, "MapTransition: overlay '%s' not found", name.c_str());
	}
}

/**
 * Draw map overlay segments and icons based on visited-page state.
 *
 * Key pattern for each overlay:
 *   Draw if dstPageId was already visited during the current game,
 *   or if the current transition starts at srcPageId and that page is visited
 *   but dstPageId has not been reached at this level yet.
 *
 * Route direction at the Rescue Site I fork is tracked through @ref GameState::hasPageVisit.
 * Magic Wall visit kind 1 selects the upper path and Mystic Marsh selects the lower path.
 */
void TransitionMapTrans::drawMapOverlays(ManagedSurface32 *dst, PageId srcPageId, int mapRegion) {
	GameState *gs = _vm->_state;

	// Helper lambda: standard overlay visibility check.
	// "Show this path piece if destSurface was previously visited,
	// OR if we're currently transitioning and haven't arrived yet."
	auto visible = [&](PageId segmentSrcPageId, PageId dstPageId) -> bool {
		return gs->isPageVisited(dstPageId) || (srcPageId == segmentSrcPageId && gs->isPageVisited(segmentSrcPageId) && !gs->hasPageVisit(dstPageId, 1));
	};

	switch (mapRegion) {
	case 1: {
		// Map region one covers ShelterZombiniville through Rescue Site I.
		const bool seg01vis = visible(PageId::kPageZombiniville_00, PageId::kPageCrazyTurtle_01);
		const bool seg02vis = visible(PageId::kPageCrazyTurtle_01, PageId::kPageWaterslide_02);
		const bool seg03vis = visible(PageId::kPageWaterslide_02, PageId::kPageAquacube_03);
		const bool seg04vis = visible(PageId::kPageAquacube_03, PageId::kPageRescue1_04);

		if (seg01vis)
			drawOverlaySprite(dst, kOverlaySegment01, Common::Point32(264, 206));
		if (seg02vis)
			drawOverlaySprite(dst, kOverlaySegment02, Common::Point32(369, 94));
		if (seg03vis)
			drawOverlaySprite(dst, kOverlaySegment03, Common::Point32(520, 74));
		if (seg04vis)
			drawOverlaySprite(dst, kOverlaySegment03, Common::Point32(632, 156));

		// Icons
		if (seg01vis) {
			drawOverlaySprite(dst, kOverlayIcon01, Common::Point32(259, 302));
			drawOverlaySprite(dst, kOverlayIcon02, Common::Point32(281, 131));
		}
		if (seg02vis)
			drawOverlaySprite(dst, kOverlayIcon03, Common::Point32(421, 26));
		if (seg03vis)
			drawOverlaySprite(dst, kOverlayIcon04, Common::Point32(564, 99));
		if (seg04vis)
			drawOverlaySprite(dst, kOverlayIcon05, Common::Point32(653, 191));
		break;
	}

	case 2: {
		// Map region two covers both routes between the rescue sites.
		const bool northRoute = gs->hasPageVisit(PageId::kPageMagicWall_06, 1) || _vm->_routeDirection == Zoombini2Engine::RouteBranch::kLeft01;
		const bool southRoute = gs->hasPageVisit(PageId::kPageMysticMarsh_05, 1) || _vm->_routeDirection == Zoombini2Engine::RouteBranch::kRight02;

		// Unconditional: start of route from Rescue1
		drawOverlaySprite(dst, kOverlaySegment03, Common::Point32(-18, 198));
		drawOverlaySprite(dst, kOverlaySegment04, Common::Point32(99, 280));

		// Top path: fork, Magic Wall, Chez Norf, then Rescue Site II.
		if (northRoute && visible(PageId::kPageRescue1_04, PageId::kPageMagicWall_06))
			drawOverlaySprite(dst, kOverlaySegment05a, Common::Point32(201, 260));

		// Magic Wall to Chez Norf segment.
		const bool czNorfSeg = gs->isPageVisited(PageId::kPageChezNorf_08) ||
							   (srcPageId == PageId::kPageMagicWall_06 && gs->isPageVisited(PageId::kPageMagicWall_06) && !gs->hasPageVisit(PageId::kPageChezNorf_08, 1));
		if (czNorfSeg)
			drawOverlaySprite(dst, kOverlaySegment06a, Common::Point32(310, 230));

		// Chez Norf to Rescue Site II segment.
		if (gs->hasPageVisit(PageId::kPageChezNorf_08, 1)) {
			if (gs->isPageVisited(PageId::kPageRescue2_09) || (srcPageId == PageId::kPageChezNorf_08 && gs->isPageVisited(PageId::kPageChezNorf_08) && !gs->hasPageVisit(PageId::kPageRescue2_09, 1)))
				drawOverlaySprite(dst, kOverlaySegment07a, Common::Point32(480, 233));
		}

		// Bottom path: fork, Mystic Marsh, Wall of Fleens, then Rescue Site II.
		if (southRoute && visible(PageId::kPageRescue1_04, PageId::kPageMysticMarsh_05))
			drawOverlaySprite(dst, kOverlaySegment05b, Common::Point32(164, 376));

		// Mystic Marsh to Wall of Fleens segment.
		const bool wofSeg = gs->isPageVisited(PageId::kPageWallOfFleens_07) ||
							(srcPageId == PageId::kPageMysticMarsh_05 && gs->isPageVisited(PageId::kPageMysticMarsh_05) && !gs->hasPageVisit(PageId::kPageWallOfFleens_07, 1));
		if (wofSeg)
			drawOverlaySprite(dst, kOverlaySegment06b, Common::Point32(339, 476));

		// Wall of Fleens to Rescue Site II segment.
		if (gs->hasPageVisit(PageId::kPageWallOfFleens_07, 1)) {
			if (gs->isPageVisited(PageId::kPageRescue2_09) || (srcPageId == PageId::kPageWallOfFleens_07 && gs->isPageVisited(PageId::kPageWallOfFleens_07) && !gs->hasPageVisit(PageId::kPageRescue2_09, 1)))
				drawOverlaySprite(dst, kOverlaySegment07b, Common::Point32(512, 360));
		}

		// These route icons are always visible in region two.
		drawOverlaySprite(dst, kOverlayIcon04, Common::Point32(27, 223));
		drawOverlaySprite(dst, kOverlayIcon05, Common::Point32(116, 315));

		// Top route icons
		if (northRoute && visible(PageId::kPageRescue1_04, PageId::kPageMagicWall_06))
			drawOverlaySprite(dst, kOverlayIcon06a, Common::Point32(259, 210));
		if (czNorfSeg)
			drawOverlaySprite(dst, kOverlayIcon07a, Common::Point32(440, 170));

		// Rescue2 icon (reachable from either path)
		if (gs->isPageVisited(PageId::kPageRescue2_09) ||
			(srcPageId == PageId::kPageChezNorf_08 && gs->isPageVisited(PageId::kPageChezNorf_08) && !gs->hasPageVisit(PageId::kPageRescue2_09, 1)) ||
			(srcPageId == PageId::kPageWallOfFleens_07 && gs->isPageVisited(PageId::kPageWallOfFleens_07) && !gs->hasPageVisit(PageId::kPageRescue2_09, 1)))
			drawOverlaySprite(dst, kOverlayIcon08, Common::Point32(476, 271));

		// Bottom route icons
		if (southRoute && visible(PageId::kPageRescue1_04, PageId::kPageMysticMarsh_05))
			drawOverlaySprite(dst, kOverlayIcon06b, Common::Point32(290, 418));
		if (visible(PageId::kPageMysticMarsh_05, PageId::kPageWallOfFleens_07))
			drawOverlaySprite(dst, kOverlayIcon07b, Common::Point32(443, 430));
		break;
	}

	case 3: {
		// Map region three covers Rescue Site II through the finale.
		const bool northRouteVisited = gs->hasPageVisit(PageId::kPageMagicWall_06, 1);
		const bool czNorfFlag = gs->hasPageVisit(PageId::kPageChezNorf_08, 1);
		const bool wofFlag = gs->hasPageVisit(PageId::kPageWallOfFleens_07, 1);

		// Previous route segments (show which path was taken)
		if (northRouteVisited) {
			drawOverlaySprite(dst, kOverlaySegment05a, Common::Point32(-27, 418));
			drawOverlaySprite(dst, kOverlaySegment06a, Common::Point32(87, 386));
		}
		if (czNorfFlag)
			drawOverlaySprite(dst, kOverlaySegment07a, Common::Point32(251, 383));
		if (wofFlag)
			drawOverlaySprite(dst, kOverlaySegment07b, Common::Point32(293, 515));

		// Rescue Site II to Snowboard Gulch is always visible.
		drawOverlaySprite(dst, kOverlaySegment08, Common::Point32(313, 407));

		// Snowboard Gulch to Boolie Boggle.
		if (visible(PageId::kPageSnowboard_10, PageId::kPageBoolies_11))
			drawOverlaySprite(dst, kOverlaySegment09, Common::Point32(434, 328));
		// Boolie Boggle to the finale.
		if (visible(PageId::kPageBoolies_11, PageId::kPageBooliewood_12))
			drawOverlaySprite(dst, kOverlaySegment10, Common::Point32(527, 111));

		// Prior-route icons remain conditional on saved progress.
		if (northRouteVisited)
			drawOverlaySprite(dst, kOverlayIcon06a, Common::Point32(33, 366));
		if (czNorfFlag)
			drawOverlaySprite(dst, kOverlayIcon07a, Common::Point32(214, 326));

		// Unconditional icons
		drawOverlaySprite(dst, kOverlayIcon08, Common::Point32(252, 426));
		drawOverlaySprite(dst, kOverlayIcon07b, Common::Point32(66, 574));
		drawOverlaySprite(dst, kOverlayIcon09, Common::Point32(367, 367));

		// Conditional icons
		if (visible(PageId::kPageSnowboard_10, PageId::kPageBoolies_11))
			drawOverlaySprite(dst, kOverlayIcon10, Common::Point32(469, 273));
		if (visible(PageId::kPageBoolies_11, PageId::kPageBooliewood_12))
			drawOverlaySprite(dst, kOverlayIcon11, Common::Point32(608, -12));
		break;
	}

	default:
		// Use the first map region as a safe fallback.
		drawOverlaySprite(dst, kOverlaySegment01, Common::Point32(264, 206));
		drawOverlaySprite(dst, kOverlayIcon01, Common::Point32(259, 302));
		drawOverlaySprite(dst, kOverlayIcon02, Common::Point32(281, 131));
		break;
	}
}

/** Start and advance one independent path per Zoombini with an 800-millisecond stagger. */
void TransitionMapTrans::walkZoombinis() {
	uint32 now = _vm->getTotalPlayTime();
	const uint numZoombinis = _vm->_state->_activeZoombinis.size();

	// Start the next Zoombini walking if its strict 800-millisecond gate has passed.
	if (_nextWalkIndex < numZoombinis && _nextWalkTime < now) {
		_nextWalkTime = now + 800;

		PathObject *path = _vm->loadPagePATPath(_patPath);
		if (path) {
			ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[_nextWalkIndex];
			zoombini->setHidden(false);
			path->setStepValueForAllSegments(2);
			zoombini->startMovement(path, now);
		} else {
			warning("MapTransition: cannot start walker %u without path '%s'", _nextWalkIndex, _patPath.toString().c_str());
			_completedCount += 1;
		}
		_nextWalkIndex += 1;
	}

	// Update all walking zoombinis
	for (uint i = 0; i < numZoombinis; i++) {
		ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[i];
		if (!zoombini->hasMovementPath())
			continue;
		if (zoombini->isMovementFinished()) {
			zoombini->setHidden(true);
			zoombini->clearMovement();
			_completedCount += 1;
			continue;
		}
		zoombini->advanceMovement(now, Common::Point32(13, 13));
		zoombini->updateAnimation(now);
	}

	updateSpeechQueue();
	if (numZoombinis <= _completedCount && !hasPendingSpeech())
		finishTransition();
}

void TransitionMapTrans::onUpdate() {
	walkZoombinis();
}

PageId TransitionMapTrans::getPostTransitionPage() const {
	GameState *gameState = _vm->_state;
	if (!gameState)
		return _targetPageId;

	const PageId src = _vm->_mapTransitSrcPageId;
	if (src == PageId::kPageAquacube_03 && !gameState->hasPlayedRescue1Movie()) {
		const int zoombiniCount = static_cast<int>(_vm->_state->_activeZoombinis.size()) + gameState->_rescue1ArrivalCount;
		if (kRescue1MovieMinimumZoombinis <= zoombiniCount)
			return PageId::kPageCutsceneSecond_20;
	}

	if ((src == PageId::kPageWallOfFleens_07 || src == PageId::kPageChezNorf_08) && !gameState->hasPlayedRescue2Movie()) {
		return PageId::kPageCutsceneThird_21;
	}

	return _targetPageId;
}

void TransitionMapTrans::finishTransition() {
	if (_transitionFinished)
		return;
	_transitionFinished = true;

	if (_vm->_mapTransitSrcPageId == PageId::kPageBoolies_11 && !_vm->isDebugMapTransition()) {
		_vm->_state->recordBooliesCompletion();
		_targetPageId = getDestPage(PageId::kPageBoolies_11, _vm->_routeDirection, _vm->_state->_rescuedBoolieCount);
	}
	switch (_targetPageId) {
	case PageId::kPageBooliewood_12:
	case PageId::kPageFinal_23:
		if (_vm->isDebugMapTransition())
			_vm->_state->stashActiveZoombinis();
		break;
	default:
		break;
	}

	const PageId nextPage = getPostTransitionPage();
	switch (_targetPageId) {
	case PageId::kPageCrazyTurtle_01:
	case PageId::kPageWaterslide_02:
	case PageId::kPageAquacube_03:
	case PageId::kPageMysticMarsh_05:
	case PageId::kPageMagicWall_06:
	case PageId::kPageWallOfFleens_07:
	case PageId::kPageChezNorf_08:
	case PageId::kPageSnowboard_10:
	case PageId::kPageBoolies_11:
		if (_vm->inJourneyMode()) {
			GameState *state = _vm->_state;
			const int storedLevel = state->getPageLevel(_targetPageId);
			const int activeLevel = state->activatePageLevel(_targetPageId);
			if (storedLevel != 0 && (storedLevel < 1 || 4 < storedLevel)) {
				warning("MapTransition: dest page %d has invalid stored level %d; using level %d", static_cast<int>(_targetPageId), storedLevel, activeLevel);
			}
			debug(1, "MapTransition: dest page %d selects stored level %d", static_cast<int>(_targetPageId), activeLevel);
		}
		break;
	default:
		break;
	}
	_vm->_mapTransitSrcPageId = _targetPageId;
	_vm->requestPageChange(nextPage);
}

void TransitionMapTrans::onRenderContent(ManagedSurface32 *screen) {
	// Draw composited background (map + overlays)
	_vm->_gfx->drawBackground(screen, Common::Point32(0, 0));
}

void TransitionMapTrans::onRenderActors(ManagedSurface32 *screen) {
	// Sort walkers by vertical position so lower sprites overlap higher ones.
	Common::Array<uint> drawOrder;
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		const ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[i];
		if (zoombini->hasMovementPath() && !zoombini->isHidden())
			drawOrder.push_back(i);
	}
	ZoombiniRunner::sortDrawOrderByY(_vm->_state->_activeZoombinis, drawOrder);

	// Draw each zoombini in Y-sorted order
	for (uint i = 0; i < drawOrder.size(); i++)
		_vm->_gfx->drawZoombiniRunner(screen, _vm->_state->_activeZoombinis[drawOrder[i]]);
}

void TransitionMapTrans::onActorsRendered() {
	for (uint i = 0; i < _vm->_state->_activeZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _vm->_state->_activeZoombinis[i];
		if (zoombini->hasMovementPath() && !zoombini->isHidden())
			zoombini->advanceAnimationAfterDraw();
	}
}

EventHandleResult TransitionMapTrans::onKeyDown(const Common::KeyState &key, bool repeat) {
	(void)repeat;
	if (key.keycode != Common::KEYCODE_SPACE)
		return EventHandleResult::kPassthrough;
	finishTransition();
	return EventHandleResult::kConsumed;
}
EventHandleResult TransitionMapTrans::onLButtonUp(const Common::Point &pos) {
	(void)pos;
	finishTransition();
	return EventHandleResult::kConsumed;
}

const TransitionMapTrans::RouteDest *TransitionMapTrans::getRouteDestinations(uint &count) {
	count = ARRAYSIZE(kRouteDestinations);
	return kRouteDestinations;
}

PageId TransitionMapTrans::getDestPage(PageId src, Zoombini2Engine::RouteBranch routeBranch, int rescuedBoolies) {
	uint destinationCount = 0;
	const RouteDest *destinations = getRouteDestinations(destinationCount);
	for (uint i = 0; i < destinationCount; i++) {
		const RouteDest &routeDest = destinations[i];
		if (routeDest.source != src)
			continue;

		if (src == PageId::kPageRescue1_04 && routeDest.branch != routeBranch)
			continue;

		if (routeDest.rescueCondition == RouteRescueCondition::kBelowFinalThreshold &&
			kFinalPageRescueThreshold <= rescuedBoolies)
			continue;
		if (routeDest.rescueCondition == RouteRescueCondition::kAtOrAboveFinalThreshold && rescuedBoolies < kFinalPageRescueThreshold)
			continue;

		return routeDest.target;
	}

	return PageId::kPageNone_M1;
}

} // End of namespace Zoombini2
