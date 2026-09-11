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
#include "common/file.h"
#include "common/tokenizer.h"

#include "zoombini2/graphics.h"
#include "zoombini2/scripts.h"
#include "zoombini2/pages/transition_maptrans.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

static const int kRescue1MovieMinimumZoombinis = 8;

// ============================================================================
// TransitionMapTrans - route-map transition.
// ============================================================================

TransitionMapTrans::TransitionMapTrans(Zoombini2Engine *vm)
	: TransitionBase(vm) {
	_pageId = kPageMapTrans;
}

TransitionMapTrans::~TransitionMapTrans() {
	if (_musicId >= 0) {
		SoundManager *snd = _vm->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	cleanupPaths();
	delete _compositedBg;
}

void TransitionMapTrans::cleanupPaths() {
	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		ZoombiniState *zoombini = _vm->_globalZoombinis[i];
		if (!zoombini)
			continue;
		zoombini->clearMovement();
		zoombini->resetAnimation();
		zoombini->_hidden = false;
	}
}

/** Load, draw, and release one cached map-overlay RLE sprite. */
void TransitionMapTrans::drawOverlaySprite(Graphics::ManagedSurface *dst, const Common::String &name, const Common::Point32 &pos) {
	Common::Path overlayPath(Common::String::format("bmp/maptrans/%s.bmp", name.c_str()));

	RleBlock overlay(_vm);
	if (overlay.load(overlayPath)) {
		overlay.drawToScreen(dst, pos, _vm->getAlphaLUT());
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
 * Route direction at the page-4 fork is tracked through @ref GameState::hasPageVisit.
 * Page 6 visit kind 1 selects the upper path and page 5 selects the lower path.
 */

void TransitionMapTrans::drawMapOverlays(Graphics::ManagedSurface *dst, PageId src, int mapRegion) {
	GameState *gs = _vm->getGameState();
	const RouteBranch routeBranch = _vm->_routeDirection;

	// Helper lambda: standard overlay visibility check.
	// "Show this path piece if destination was previously visited,
	// OR if we're currently transitioning and haven't arrived yet."
	auto visible = [&](int srcPageId, int dstPageId) -> bool {
		return gs->isPageVisited(dstPageId) || (src == srcPageId && gs->isPageVisited(srcPageId) && !gs->hasPageVisit(dstPageId, 1));
	};

	switch (mapRegion) {
	case 1: {
		// Map region one covers ShelterZombiniville through Rescue Site I.
		bool seg01vis = visible(0, 1);
		bool seg02vis = visible(1, 2);
		bool seg03vis = visible(2, 3);
		bool seg04vis = visible(3, 4);

		if (seg01vis) {
			drawOverlaySprite(dst, "bigmap_segment_01", Common::Point32(264, 206));
		}
		if (seg02vis) {
			drawOverlaySprite(dst, "bigmap_segment_02", Common::Point32(369, 94));
		}
		if (seg03vis) {
			drawOverlaySprite(dst, "bigmap_segment_03", Common::Point32(520, 74));
		}
		if (seg04vis) {
			drawOverlaySprite(dst, "bigmap_segment_03", Common::Point32(632, 156));
		}

		// Icons
		if (seg01vis) {
			drawOverlaySprite(dst, "bigmap_icon_01", Common::Point32(259, 302));
			drawOverlaySprite(dst, "bigmap_icon_02", Common::Point32(281, 131));
		}
		if (seg02vis) {
			drawOverlaySprite(dst, "bigmap_icon_03", Common::Point32(421, 26));
		}
		if (seg03vis) {
			drawOverlaySprite(dst, "bigmap_icon_04", Common::Point32(564, 99));
		}
		if (seg04vis) {
			drawOverlaySprite(dst, "bigmap_icon_05", Common::Point32(653, 191));
		}
		break;
	}

	case 2: {
		// Map region two covers both routes between the rescue sites.
		bool northRoute = gs->hasPageVisit(6, 1) || routeBranch == RouteBranch::kLeft01;
		bool southRoute = gs->hasPageVisit(5, 1) || routeBranch == RouteBranch::kRight02;

		// Unconditional: start of route from Rescue1
		drawOverlaySprite(dst, "bigmap_segment_03", Common::Point32(-18, 198));
		drawOverlaySprite(dst, "bigmap_segment_04", Common::Point32(99, 280));

		// Top path: fork, Magic Wall, Chez Norf, then Rescue Site II.
		if (northRoute && visible(4, 6)) {
			drawOverlaySprite(dst, "bigmap_segment_05a", Common::Point32(201, 260));
		}

		// Magic Wall to Chez Norf segment.
		bool czNorfSeg = gs->isPageVisited(8) || (src == 6 && gs->isPageVisited(6) && !gs->hasPageVisit(8, 1));
		if (czNorfSeg) {
			drawOverlaySprite(dst, "bigmap_segment_06a", Common::Point32(310, 230));
		}

		// Chez Norf to Rescue Site II segment.
		if (gs->hasPageVisit(8, 1)) {
			if (gs->isPageVisited(9) || (src == 8 && gs->isPageVisited(8) && !gs->hasPageVisit(9, 1))) {
				drawOverlaySprite(dst, "bigmap_segment_07a", Common::Point32(480, 233));
			}
		}

		// Bottom path: fork, Mystic Marsh, Wall of Fleens, then Rescue Site II.
		if (southRoute && visible(4, 5)) {
			drawOverlaySprite(dst, "bigmap_segment_05b", Common::Point32(164, 376));
		}

		// Mystic Marsh to Wall of Fleens segment.
		bool wofSeg = gs->isPageVisited(7) || (src == 5 && gs->isPageVisited(5) && !gs->hasPageVisit(7, 1));
		if (wofSeg) {
			drawOverlaySprite(dst, "bigmap_segment_06b", Common::Point32(339, 476));
		}

		// Wall of Fleens to Rescue Site II segment.
		if (gs->hasPageVisit(7, 1)) {
			if (gs->isPageVisited(9) || (src == 7 && gs->isPageVisited(7) && !gs->hasPageVisit(9, 1))) {
				drawOverlaySprite(dst, "bigmap_segment_07b", Common::Point32(512, 360));
			}
		}

		// These route icons are always visible in region two.
		drawOverlaySprite(dst, "bigmap_icon_04", Common::Point32(27, 223));
		drawOverlaySprite(dst, "bigmap_icon_05", Common::Point32(116, 315));

		// Top route icons
		if (northRoute && visible(4, 6)) {
			drawOverlaySprite(dst, "bigmap_icon_06a", Common::Point32(259, 210));
		}
		if (czNorfSeg) {
			drawOverlaySprite(dst, "bigmap_icon_07a", Common::Point32(440, 170));
		}

		// Rescue2 icon (reachable from either path)
		if (gs->isPageVisited(9) ||
			(src == 8 && gs->isPageVisited(8) && !gs->hasPageVisit(9, 1)) ||
			(src == 7 && gs->isPageVisited(7) && !gs->hasPageVisit(9, 1))) {
			drawOverlaySprite(dst, "bigmap_icon_08", Common::Point32(476, 271));
		}

		// Bottom route icons
		if (southRoute && visible(4, 5)) {
			drawOverlaySprite(dst, "bigmap_icon_06b", Common::Point32(290, 418));
		}
		if (visible(5, 7)) {
			drawOverlaySprite(dst, "bigmap_icon_07b", Common::Point32(443, 430));
		}
		break;
	}

	case 3: {
		// Map region three covers Rescue Site II through the finale.
		bool northRouteVisited = gs->hasPageVisit(6, 1);
		bool czNorfFlag = gs->hasPageVisit(8, 1);
		bool wofFlag = gs->hasPageVisit(7, 1);

		// Previous route segments (show which path was taken)
		if (northRouteVisited) {
			drawOverlaySprite(dst, "bigmap_segment_05a", Common::Point32(-27, 418));
			drawOverlaySprite(dst, "bigmap_segment_06a", Common::Point32(87, 386));
		}
		if (czNorfFlag) {
			drawOverlaySprite(dst, "bigmap_segment_07a", Common::Point32(251, 383));
		}
		if (wofFlag) {
			drawOverlaySprite(dst, "bigmap_segment_07b", Common::Point32(293, 515));
		}

		// Rescue Site II to Snowboard Gulch is always visible.
		drawOverlaySprite(dst, "bigmap_segment_08", Common::Point32(313, 407));

		// Snowboard Gulch to Boolie Boggle.
		if (visible(10, 11)) {
			drawOverlaySprite(dst, "bigmap_segment_09", Common::Point32(434, 328));
		}
		// Boolie Boggle to the finale.
		if (visible(11, 12)) {
			drawOverlaySprite(dst, "bigmap_segment_10", Common::Point32(527, 111));
		}

		// Prior-route icons remain conditional on saved progress.
		if (northRouteVisited) {
			drawOverlaySprite(dst, "bigmap_icon_06a", Common::Point32(33, 366));
		}
		if (czNorfFlag) {
			drawOverlaySprite(dst, "bigmap_icon_07a", Common::Point32(214, 326));
		}

		// Unconditional icons
		drawOverlaySprite(dst, "bigmap_icon_08", Common::Point32(252, 426));
		drawOverlaySprite(dst, "bigmap_icon_07b", Common::Point32(66, 574));
		drawOverlaySprite(dst, "bigmap_icon_09", Common::Point32(367, 367));

		// Conditional icons
		if (visible(10, 11)) {
			drawOverlaySprite(dst, "bigmap_icon_10", Common::Point32(469, 273));
		}
		if (visible(11, 12)) {
			drawOverlaySprite(dst, "bigmap_icon_11", Common::Point32(608, -12));
		}
		break;
	}

	default:
		// Use the first map region as a safe fallback.
		drawOverlaySprite(dst, "bigmap_segment_01", Common::Point32(264, 206));
		drawOverlaySprite(dst, "bigmap_icon_01", Common::Point32(259, 302));
		drawOverlaySprite(dst, "bigmap_icon_02", Common::Point32(281, 131));
		break;
	}
}

void TransitionMapTrans::init() {
	debug(1, "MapTransition::init (source=%d, route=%d)",
		  static_cast<int>(_vm->_mapTransitionSourcePageId), static_cast<int>(_vm->_routeDirection));

	const PageId src = _vm->_mapTransitionSourcePageId;
	const RouteBranch routeBranch = _vm->_routeDirection;
	_vm->_skipMode = false;
	_transitionFinished = false;

	// Select the map region from the source page.
	int mapRegion;
	if (kPageZombiniville <= src && src <= kPageAquacube)
		mapRegion = 1;
	else if (kPageRescue1 <= src && src <= kPageChezNorf)
		mapRegion = 2;
	else
		mapRegion = 3;

	// Select the walking path from the source page.
	Common::String patName;
		switch (src) {
	case kPageZombiniville:
		patName = "tr1 - map1.pat";
		break;
	case kPageCrazyTurtle:
		patName = "tr2 - map1.pat";
		break;
	case kPageWaterslide:
		patName = "tr3 - map1.pat";
		break;
	case kPageAquacube:
		patName = "tr4 - map1.pat";
		break;
	case kPageRescue1:
		if (routeBranch == RouteBranch::kLeft01)
			patName = "tr5 - map2.pat";
		else if (routeBranch == RouteBranch::kRight02)
			patName = "tr8 - map2.pat";
		break;
	case kPageMysticMarsh:
		patName = "tr9 - map2.pat";
		break;
	case kPageMagicWall:
		patName = "tr6 - map2.pat";
		break;
	case kPageWallOfFleens:
		patName = "tr10 - map2.pat";
		break;
	case kPageChezNorf:
		patName = "tr7 - map2.pat";
		break;
	case kPageRescue2:
		patName = "tr11 - map3.pat";
		break;
	case kPageSnowboard:
		patName = "tr12 - map3.pat";
		break;
	case kPageBoolies:
		patName = "tr13 - map3.pat";
		break;
	default:
		patName = "Transition02.pat";
		break;
	}
	_patPath = Common::Path(Common::String::format("bmp/maptrans/%s", patName.c_str()));

	_targetPageId = getDestPage(src, routeBranch, _vm->getGameState()->_rescuedBoolieCount);
	if (_targetPageId == kPageNone) {
		warning("MapTransition: refusing invalid source/branch combination (%d, %d)",
				static_cast<int>(src), static_cast<int>(routeBranch));
		_vm->requestPageChange(src);
		return;
	}

	// Load and composite the background
	BitBlock bg(_vm);
	Common::String bgPath = Common::String::format("#bmp/maptrans/bigmap_background_%d", mapRegion);

	delete _compositedBg;
	_compositedBg = new Graphics::ManagedSurface(kScreenWidth, kScreenHeight,
												 Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));

	if (bg.load(Common::Path(bgPath))) {
		bg.drawToSurface(_compositedBg, Common::Point32(0, 0));
	} else {
		warning("MapTransition: Failed to load background %s", bgPath.c_str());
		_compositedBg->fillRect(Common::Rect(kScreenWidth, kScreenHeight), 0);
	}

	// Draw overlay sprites onto background
	drawMapOverlays(_compositedBg, src, mapRegion);

	_zoombiniAnimation = _vm->loadZoombiniAnimation(Common::Path("bmp/zombis/littleZomb.anm"));
	if (!_zoombiniAnimation)
		warning("MapTransition: Failed to load littleZomb.anm");

	// Initialize the runtime state owned by each walking Zoombini.
	cleanupPaths();
	int numZoombinis = static_cast<int>(_vm->_globalZoombinis.size());
	for (int i = 0; i < numZoombinis; i++) {
		ZoombiniState *zoombini = _vm->_globalZoombinis[i];
		zoombini->setDefaultAnimation(_zoombiniAnimation, 88);
		zoombini->_hidden = true;
	}
	_nextWalkIndex = 0;
	_nextWalkTime = 0;
	_completedCount = 0;

	debug(1, "MapTransition: source=%d -> target=%d (region %d, path=%s, zoombinis=%d)",
		  src, _targetPageId, mapRegion, _patPath.toString().c_str(), numZoombinis);

	// Play transition music
	_musicId = _vm->getSoundManager()->load(true,
											Common::Path("#sounds/music/ZMR-Transition.wav"), true);
	if (_musicId >= 0)
		_vm->getSoundManager()->play(_musicId);
}

/** Start and advance one independent path per Zoombini with an 800-millisecond stagger. */
void TransitionMapTrans::walkZoombinis() {
	uint32 now = _vm->getGameTickCount();
	int numZoombinis = static_cast<int>(_vm->_globalZoombinis.size());

	// Start the next zoombini walking if it's time
	if (_nextWalkIndex < numZoombinis && now > _nextWalkTime) {
		_nextWalkTime = now + 800;

		PathObject *path = PathObject::loadFromPAT(_vm, _patPath);
		if (path) {
			ZoombiniState *zoombini = _vm->_globalZoombinis[_nextWalkIndex];
			zoombini->_hidden = false;
			zoombini->startMovement(path, now);
			zoombini->startDirectionTrackedAnimation(now, 50);
		}
		_nextWalkIndex += 1;
	}

	// Update all walking zoombinis
	for (int i = 0; i < numZoombinis; i++) {
		ZoombiniState *zoombini = _vm->_globalZoombinis[i];
		if (!zoombini->_movementPath)
			continue;
		if (!zoombini->advanceMovement(now, Common::Point32(13, 13), true))
			_completedCount += 1;
		zoombini->updateAnimation(now);
	}

	// When all zoombinis are done, signal transition completion
	if (_completedCount >= numZoombinis) {
		_vm->_skipMode = true;
	}
}

void TransitionMapTrans::onUpdate() {
	walkZoombinis();

	// Space skips the walking animation but preserves the destination gate.
	if (_vm->_skipMode) {
		finishTransition();
	}
}

PageId TransitionMapTrans::getPostTransitionPage() const {
	GameState *gameState = _vm->getGameState();
	if (!gameState)
		return _targetPageId;

	const PageId src = _vm->_mapTransitionSourcePageId;
	if (src == kPageAquacube && !gameState->hasPlayedRescue1Movie()) {
		const int zoombiniCount = static_cast<int>(_vm->_globalZoombinis.size()) + gameState->_rescue1ArrivalCount;
		if (kRescue1MovieMinimumZoombinis <= zoombiniCount)
			return kPageCutsceneSecond;
	}

	if ((src == kPageWallOfFleens || src == kPageChezNorf) && !gameState->hasPlayedRescue2Movie()) {
		return kPageCutsceneThird;
	}

	return _targetPageId;
}

void TransitionMapTrans::finishTransition() {
	if (_transitionFinished)
		return;
	_transitionFinished = true;

	const PageId nextPage = getPostTransitionPage();
	_vm->_mapTransitionSourcePageId = _targetPageId;
	_vm->requestPageChange(nextPage);
}

void TransitionMapTrans::onRenderScene(ManagedSurface32 *screen) {
	// Draw composited background (map + overlays)
	if (_compositedBg)
		screen->blitFrom(*_compositedBg, Common::Point32(0, 0));
}

void TransitionMapTrans::onRenderActors(ManagedSurface32 *screen) {
	// Sort walkers by vertical position so lower sprites overlap higher ones.
	int numZoombinis = static_cast<int>(_vm->_globalZoombinis.size());

	// Build sort order by Y
	Common::Array<int> drawOrder;
	for (int i = 0; i < numZoombinis; i++) {
		const ZoombiniState *zoombini = _vm->_globalZoombinis[i];
		if (zoombini->_movementPath && !zoombini->_hidden)
			drawOrder.push_back(i);
	}

	// The party is small enough for a direct pairwise sort.
	for (uint i = 0; i < drawOrder.size(); i++) {
		for (uint j = i + 1; j < drawOrder.size(); j++) {
			if (_vm->_globalZoombinis[drawOrder[j]]->_screenPos.y < _vm->_globalZoombinis[drawOrder[i]]->_screenPos.y)
				SWAP(drawOrder[i], drawOrder[j]);
		}
	}

	// Draw each zoombini in Y-sorted order
	for (uint i = 0; i < drawOrder.size(); i++)
		_vm->_globalZoombinis[drawOrder[i]]->draw(screen, _vm->getAlphaLUT());
}

EventHandleResult TransitionMapTrans::onKeyDown(const Common::KeyState &key, bool repeat) {
	(void)repeat;
	if (key.keycode != Common::KEYCODE_SPACE)
		return EventHandleResult::kPassthrough;
	finishTransition();
	return EventHandleResult::kConsumed;
}
EventHandleResult TransitionMapTrans::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	// Skip map animation on click
	finishTransition();
	return EventHandleResult::kConsumed;
}

PageId TransitionMapTrans::getDestPage(PageId src, RouteBranch routeBranch, int rescuedBoolies) {
	switch (src) {
	case kPageZombiniville:
		return kPageCrazyTurtle;
	case kPageCrazyTurtle:
		return kPageWaterslide;
	case kPageWaterslide:
		return kPageAquacube;
	case kPageAquacube:
		return kPageRescue1;
	case kPageRescue1: {
		if (routeBranch == RouteBranch::kLeft01)
			return kPageMagicWall;
		else if (routeBranch == RouteBranch::kRight02)
			return kPageMysticMarsh;
		return kPageNone;
	}
	case kPageMysticMarsh:
		return kPageWallOfFleens;
	case kPageMagicWall:
		return kPageChezNorf;
	case kPageWallOfFleens:
	case kPageChezNorf:
		return kPageRescue2;
	case kPageRescue2:
		return kPageSnowboard;
	case kPageSnowboard:
		return kPageBoolies;
	case kPageBoolies:
		return rescuedBoolies < 400 ? kPageBooliewood : kPageFinal;
	default:
		return kPageNone;
	}
}

} // End of namespace Zoombini2
