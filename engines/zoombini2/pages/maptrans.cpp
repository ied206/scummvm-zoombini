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

#include "zoombini2/pages/maptrans.h"
#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

static const int kRescue1MovieMinimumZoombinis = 8;

// ============================================================================
// MapTransition — overworld map transition (page ID 22).
// Original: CL_maptrans__Init_4224A0 (0x4224A0), object size 0x110 (272 bytes).
// ============================================================================

MapTransition::MapTransition(Zoombini2Engine *engine)
	: Page(engine), _compositedBg(nullptr), _nextWalkIndex(0),
	  _nextWalkTime(0), _completedCount(0), _targetWorld(0),
	  _transitionFinished(false), _musicId(-1), _zoombiniGfx(nullptr) {
	_pageId = kPageMapTrans;
}

MapTransition::~MapTransition() {
	if (_musicId >= 0) {
		SoundManager *snd = _engine->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	cleanupPaths();
	delete _compositedBg;
	delete _zoombiniGfx;
}

void MapTransition::cleanupPaths() {
	for (uint i = 0; i < _zoombiniPaths.size(); i++)
		delete _zoombiniPaths[i];
	_zoombiniPaths.clear();
}

/**
 * Draw an overlay sprite (segment or icon) onto a surface.
 * Original: CL_maptrans__DrawOverlaySprite_422290.
 * Loads bmp/maptrans/<name>.bmp + <name>_a.bmp, draws, and frees.
 */
void MapTransition::drawOverlaySprite(Graphics::ManagedSurface *dst,
                                      const Common::String &name, int x, int y) {
	Common::Path colorPath(Common::String::format("bmp/maptrans/%s.bmp", name.c_str()));
	Common::Path alphaPath(Common::String::format("bmp/maptrans/%s_a.bmp", name.c_str()));

	BitBlock bit;
	if (bit.loadFromBMPPair(colorPath, alphaPath)) {
		bit.drawAlphaBlend(dst, x, y, _engine->getAlphaLUT());
	} else if (bit.load(Common::Path(Common::String::format("bmp/maptrans/%s", name.c_str())))) {
		bit.drawToSurface(dst, x, y);
	} else {
		debug(2, "MapTransition: overlay '%s' not found", name.c_str());
	}
}

/**
 * Draw map overlay segments and icons based on visited-world state.
 * Original: conditional overlay logic in CL_maptrans__Init_4224A0.
 *
 * Key pattern for each overlay:
 *   Draw if destWorld was already visited (past game),
 *   OR if we're currently transitioning FROM srcWorld and srcWorld is visited
 *   but destWorld hasn't been reached at this difficulty yet.
 *
 * Route direction (top/bottom) at world 4 fork is tracked via
 * GameState::isWorldVisitedAtDiff: world 6 diff1 = top path,
 * world 5 diff1 = bottom path.
 */
void MapTransition::drawMapOverlays(Graphics::ManagedSurface *dst, int source, int mapRegion) {
	GameState *gs = _engine->getGameState();
	int route = _engine->_routeDirection;

	// Helper lambda: standard overlay visibility check.
	// "Show this path piece if destination was previously visited,
	// OR if we're currently transitioning and haven't arrived yet."
	auto visible = [&](int srcWorld, int dstWorld) -> bool {
		return gs->isWorldVisited(dstWorld)
			|| (source == srcWorld && gs->isWorldVisited(srcWorld)
			    && !gs->isWorldVisitedAtDiff(dstWorld, 1));
	};

	switch (mapRegion) {
	case 1: {
		// Map region 1: Zombiniville → Rescue1 (transitions 0-3)
		bool seg01vis = visible(0, 1);
		bool seg02vis = visible(1, 2);
		bool seg03vis = visible(2, 3);
		bool seg04vis = visible(3, 4);

		if (seg01vis) {
			drawOverlaySprite(dst, "bigmap_segment_01", 264, 206);
		}
		if (seg02vis) {
			drawOverlaySprite(dst, "bigmap_segment_02", 369, 94);
		}
		if (seg03vis) {
			drawOverlaySprite(dst, "bigmap_segment_03", 520, 74);
		}
		if (seg04vis) {
			drawOverlaySprite(dst, "bigmap_segment_03", 632, 156);
		}

		// Icons
		if (seg01vis) {
			drawOverlaySprite(dst, "bigmap_icon_01", 259, 302);
			drawOverlaySprite(dst, "bigmap_icon_02", 281, 131);
		}
		if (seg02vis) {
			drawOverlaySprite(dst, "bigmap_icon_03", 421, 26);
		}
		if (seg03vis) {
			drawOverlaySprite(dst, "bigmap_icon_04", 564, 99);
		}
		if (seg04vis) {
			drawOverlaySprite(dst, "bigmap_icon_05", 653, 191);
		}
		break;
	}

	case 2: {
		// Map region 2: Rescue1 → Rescue2 (transitions 4-8, with fork)
		bool topRoute = gs->isWorldVisitedAtDiff(6, 1) || route == 1;
		bool botRoute = gs->isWorldVisitedAtDiff(5, 1) || route == 2;

		// Unconditional: start of route from Rescue1
		drawOverlaySprite(dst, "bigmap_segment_03", -18, 198);
		drawOverlaySprite(dst, "bigmap_segment_04", 99, 280);

		// Top path: fork → MagicWall → ChezNorf → Rescue2
		if (topRoute && visible(4, 6)) {
			drawOverlaySprite(dst, "bigmap_segment_05a", 201, 260);
		}

		// MagicWall → ChezNorf segment
		bool czNorfSeg = gs->isWorldVisited(8)
			|| (source == 6 && gs->isWorldVisited(6) && !gs->isWorldVisitedAtDiff(8, 1));
		if (czNorfSeg) {
			drawOverlaySprite(dst, "bigmap_segment_06a", 310, 230);
		}

		// ChezNorf → Rescue2 segment (requires ChezNorf reached)
		if (gs->isWorldVisitedAtDiff(8, 1)) {
			if (gs->isWorldVisited(9)
			    || (source == 8 && gs->isWorldVisited(8) && !gs->isWorldVisitedAtDiff(9, 1))) {
				drawOverlaySprite(dst, "bigmap_segment_07a", 480, 233);
			}
		}

		// Bottom path: fork → MysticMarsh → WallOfFleens → Rescue2
		if (botRoute && visible(4, 5)) {
			drawOverlaySprite(dst, "bigmap_segment_05b", 164, 376);
		}

		// MysticMarsh → WallOfFleens segment
		bool wofSeg = gs->isWorldVisited(7)
			|| (source == 5 && gs->isWorldVisited(5) && !gs->isWorldVisitedAtDiff(7, 1));
		if (wofSeg) {
			drawOverlaySprite(dst, "bigmap_segment_06b", 339, 476);
		}

		// WallOfFleens → Rescue2 segment (requires WallOfFleens reached)
		if (gs->isWorldVisitedAtDiff(7, 1)) {
			if (gs->isWorldVisited(9)
			    || (source == 7 && gs->isWorldVisited(7) && !gs->isWorldVisitedAtDiff(9, 1))) {
				drawOverlaySprite(dst, "bigmap_segment_07b", 512, 360);
			}
		}

		// Icons — unconditional
		drawOverlaySprite(dst, "bigmap_icon_04", 27, 223);
		drawOverlaySprite(dst, "bigmap_icon_05", 116, 315);

		// Top route icons
		if (topRoute && visible(4, 6)) {
			drawOverlaySprite(dst, "bigmap_icon_06a", 259, 210);
		}
		if (czNorfSeg) {
			drawOverlaySprite(dst, "bigmap_icon_07a", 440, 170);
		}

		// Rescue2 icon (reachable from either path)
		if (gs->isWorldVisited(9)
		    || (source == 8 && gs->isWorldVisited(8) && !gs->isWorldVisitedAtDiff(9, 1))
		    || (source == 7 && gs->isWorldVisited(7) && !gs->isWorldVisitedAtDiff(9, 1))) {
			drawOverlaySprite(dst, "bigmap_icon_08", 476, 271);
		}

		// Bottom route icons
		if (botRoute && visible(4, 5)) {
			drawOverlaySprite(dst, "bigmap_icon_06b", 290, 418);
		}
		if (visible(5, 7)) {
			drawOverlaySprite(dst, "bigmap_icon_07b", 443, 430);
		}
		break;
	}

	case 3: {
		// Map region 3: Rescue2 → Final (transitions 9-12)
		bool topRouteFlag = gs->isWorldVisitedAtDiff(6, 1);
		bool czNorfFlag = gs->isWorldVisitedAtDiff(8, 1);
		bool wofFlag = gs->isWorldVisitedAtDiff(7, 1);

		// Previous route segments (show which path was taken)
		if (topRouteFlag) {
			drawOverlaySprite(dst, "bigmap_segment_05a", -27, 418);
			drawOverlaySprite(dst, "bigmap_segment_06a", 87, 386);
		}
		if (czNorfFlag) {
			drawOverlaySprite(dst, "bigmap_segment_07a", 251, 383);
		}
		if (wofFlag) {
			drawOverlaySprite(dst, "bigmap_segment_07b", 293, 515);
		}

		// Unconditional: Rescue2 → Snowboard start
		drawOverlaySprite(dst, "bigmap_segment_08", 313, 407);

		// Snowboard → Boolies
		if (visible(10, 11)) {
			drawOverlaySprite(dst, "bigmap_segment_09", 434, 328);
		}
		// Boolies → Final
		if (visible(11, 12)) {
			drawOverlaySprite(dst, "bigmap_segment_10", 527, 111);
		}

		// Icons — conditional on route flags
		if (topRouteFlag) {
			drawOverlaySprite(dst, "bigmap_icon_06a", 33, 366);
		}
		if (czNorfFlag) {
			drawOverlaySprite(dst, "bigmap_icon_07a", 214, 326);
		}

		// Unconditional icons
		drawOverlaySprite(dst, "bigmap_icon_08", 252, 426);
		drawOverlaySprite(dst, "bigmap_icon_07b", 66, 574);
		drawOverlaySprite(dst, "bigmap_icon_09", 367, 367);

		// Conditional icons
		if (visible(10, 11)) {
			drawOverlaySprite(dst, "bigmap_icon_10", 469, 273);
		}
		if (visible(11, 12)) {
			drawOverlaySprite(dst, "bigmap_icon_11", 608, -12);
		}
		break;
	}

	default:
		// Fallback (default case from IDA)
		drawOverlaySprite(dst, "bigmap_segment_01", 264, 206);
		drawOverlaySprite(dst, "bigmap_icon_01", 259, 302);
		drawOverlaySprite(dst, "bigmap_icon_02", 281, 131);
		break;
	}
}

void MapTransition::init() {
	debug(1, "MapTransition::init (source=%d, route=%d)",
		_engine->_maptransSourceWorld, _engine->_routeDirection);

	int source = _engine->_maptransSourceWorld;
	int route = _engine->_routeDirection;
	_engine->_skipMode = false;
	_transitionFinished = false;

	// Select map region based on source world
	int mapRegion;
	if (source >= 0 && source <= 3)
		mapRegion = 1;
	else if (source >= 4 && source <= 8)
		mapRegion = 2;
	else
		mapRegion = 3;

	// Select PAT file based on source world
	// Original: CL_maptrans__Init_4224A0 switch on a2
	Common::String patName;
	switch (source) {
	case 0:  patName = "tr1 - map1.pat"; break;
	case 1:  patName = "tr2 - map1.pat"; break;
	case 2:  patName = "tr3 - map1.pat"; break;
	case 3:  patName = "tr4 - map1.pat"; break;
	case 4:
		patName = (route == 2) ? "tr8 - map2.pat" : "tr5 - map2.pat";
		break;
	case 5:  patName = "tr9 - map2.pat"; break;
	case 6:  patName = "tr6 - map2.pat"; break;
	case 7:  patName = "tr10 - map2.pat"; break;
	case 8:  patName = "tr7 - map2.pat"; break;
	case 9:  patName = "tr11 - map3.pat"; break;
	case 10: patName = "tr12 - map3.pat"; break;
	case 11: patName = "tr13 - map3.pat"; break;
	default: patName = "Transition02.pat"; break;
	}
	_patPath = Common::Path(Common::String::format("bmp/maptrans/%s", patName.c_str()));

	// Determine target world based on routing table
	switch (source) {
	case kPageZombiniville: _targetWorld = kPageCrazyTurtle; break;
	case kPageCrazyTurtle:  _targetWorld = kPageWaterslide; break;
	case kPageWaterslide:   _targetWorld = kPageAquacube; break;
	case kPageAquacube:     _targetWorld = kPageRescue1; break;
	case kPageRescue1:
		_targetWorld = (route == 2) ? kPageMysticMarsh : kPageMagicWall;
		break;
	case kPageMysticMarsh:  _targetWorld = kPageWallOfFleens; break;
	case kPageMagicWall:    _targetWorld = kPageChezNorf; break;
	case kPageWallOfFleens: _targetWorld = kPageRescue2; break;
	case kPageChezNorf:     _targetWorld = kPageRescue2; break;
	case kPageRescue2:      _targetWorld = kPageSnowboard; break;
	case kPageSnowboard:    _targetWorld = kPageBoolies; break;
	case kPageBoolies:      _targetWorld = kPageFinal; break;
	default:                _targetWorld = kPageZombiniville; break;
	}

	// Load and composite the background
	BitBlock bg;
	Common::String bgPath = Common::String::format("bmp/maptrans/bigmap_background_%d", mapRegion);

	delete _compositedBg;
	_compositedBg = new Graphics::ManagedSurface(kScreenWidth, kScreenHeight,
	                    Graphics::PixelFormat(4, 8, 8, 8, 8, 16, 8, 0, 24));

	if (bg.load(Common::Path(bgPath))) {
		bg.drawToSurface(_compositedBg, 0, 0);
	} else {
		warning("MapTransition: Failed to load background %s", bgPath.c_str());
		_compositedBg->fillRect(Common::Rect(kScreenWidth, kScreenHeight), 0);
	}

	// Draw overlay sprites onto background
	drawMapOverlays(_compositedBg, source, mapRegion);

	// Initialize zoombini walk state
	cleanupPaths();
	int numZoombinis = (int)_engine->_globalZoombinis.size();
	_zoombiniPaths.resize(numZoombinis, nullptr);
	_walkStates.resize(numZoombinis);
	for (int i = 0; i < numZoombinis; i++) {
		_walkStates[i].screenX = 0;
		_walkStates[i].screenY = 0;
		_walkStates[i].prevX = 0;
		_walkStates[i].prevY = 0;
		_walkStates[i].cellIndex = 88; // default: facing down
		_walkStates[i].active = false;
	}
	_nextWalkIndex = 0;
	_nextWalkTime = 0;
	_completedCount = 0;

	// Load zoombini walking sprites (littleZomb.anm)
	// Original: g_worldData at 0x50FB50, loaded by CompressGfxZomb_Read_45C4D0
	delete _zoombiniGfx;
	_zoombiniGfx = new ZoombiniGfx();
	if (!_zoombiniGfx->loadFromFile(Common::Path("bmp/zombis/littleZomb.anm"))) {
		warning("MapTransition: Failed to load littleZomb.anm");
		delete _zoombiniGfx;
		_zoombiniGfx = nullptr;
	}

	debug(1, "MapTransition: source=%d → target=%d (region %d, path=%s, zoombinis=%d)",
		source, _targetWorld, mapRegion, _patPath.toString().c_str(), numZoombinis);

	// Play transition music
	_musicId = _engine->getSoundManager()->load(true,
		Common::Path("sounds/music/ZMR-Transition.wav"), true);
	if (_musicId >= 0)
		_engine->getSoundManager()->play(_musicId);
}

/**
 * Walk zoombinis one-by-one along bezier paths.
 * Original: CL_maptrans__WalkZoombinis_421E80.
 *
 * 800ms delay between starting each zoombini.
 * Each zoombini gets its own PathObject loaded from the same PAT file.
 * When all zoombinis complete their path, sets skipMode=1.
 */
void MapTransition::walkZoombinis() {
	uint32 now = _engine->getGameTickCount();
	int numZoombinis = (int)_engine->_globalZoombinis.size();

	// Start the next zoombini walking if it's time
	if (_nextWalkIndex < numZoombinis && now > _nextWalkTime) {
		_nextWalkTime = now + 800;

		PathObject *path = PathObject::loadFromPAT(_patPath);
		if (path) {
			path->start(now);
			_zoombiniPaths[_nextWalkIndex] = path;

			// Initialize walk state with path start position
			CurveSegment *seg = path->segments[0];
			int sx = seg->posX >> 10;
			int sy = seg->posY >> 10;
			_walkStates[_nextWalkIndex].screenX = sx - 13;
			_walkStates[_nextWalkIndex].screenY = sy - 13;
			_walkStates[_nextWalkIndex].prevX = sx - 13;
			_walkStates[_nextWalkIndex].prevY = sy - 13;
			_walkStates[_nextWalkIndex].cellIndex = 88;
			_walkStates[_nextWalkIndex].active = true;
		}
		_nextWalkIndex++;
	}

	// Update all walking zoombinis
	for (int i = 0; i < numZoombinis; i++) {
		PathObject *path = _zoombiniPaths[i];
		if (!path)
			continue;

		int screenX, screenY;
		if (!path->advance(now, screenX, screenY)) {
			// Path complete — zoombini finished walking
			_walkStates[i].active = false;
			delete path;
			_zoombiniPaths[i] = nullptr;
			_completedCount++;
		} else {
			// Update position and compute direction
			// Original: CompressGfxZomb__LoadSprites_45C230(bezierX - 13, bezierY - 13)
			int newX = screenX - 13;
			int newY = screenY - 13;

			_walkStates[i].prevX = _walkStates[i].screenX;
			_walkStates[i].prevY = _walkStates[i].screenY;
			_walkStates[i].screenX = newX;
			_walkStates[i].screenY = newY;

			int dx = newX - _walkStates[i].prevX;
			int dy = newY - _walkStates[i].prevY;
			if (dx != 0 || dy != 0)
				_walkStates[i].cellIndex = computeDirectionCell(dx, dy);
		}
	}

	// When all zoombinis are done, signal transition completion
	if (_completedCount >= numZoombinis) {
		_engine->_skipMode = true;
	}
}

void MapTransition::update() {
	walkZoombinis();

	// Space skips the walking animation but preserves the destination gate.
	if (_engine->getLastKeyPressed() == Common::KEYCODE_SPACE || _engine->_skipMode) {
		finishTransition();
	}
}

int MapTransition::getPostTransitionPage() const {
	GameState *gameState = _engine->getGameState();
	if (!gameState)
		return _targetWorld;

	const int source = _engine->_maptransSourceWorld;
	if (source == kPageAquacube && !gameState->hasPlayedRescue1Movie()) {
		const int zoombiniCount = static_cast<int>(_engine->_globalZoombinis.size()) + gameState->_statA;
		if (kRescue1MovieMinimumZoombinis <= zoombiniCount)
			return kPageStoryBmp;
	}

	if ((source == kPageWallOfFleens || source == kPageChezNorf)
	    && !gameState->hasPlayedRescue2Movie()) {
		return kPageStoryAnim;
	}

	return _targetWorld;
}

void MapTransition::finishTransition() {
	if (_transitionFinished)
		return;
	_transitionFinished = true;

	const int nextPage = getPostTransitionPage();
	_engine->_maptransSourceWorld = _targetWorld;
	_engine->requestPageChange(nextPage);
}

/**
 * Compute direction cell index from movement delta.
 * Original: CompressGfxZomb__LoadSprites_45C230 angle→direction mapping.
 *
 * Uses acos of the normalized X component, divided by 22° to get one
 * of 16 compass directions, each mapped to a ZoombiniGfx cell index.
 */
int MapTransition::computeDirectionCell(int dx, int dy) const {
	// Direction → cell index table (indexed by direction + 8)
	// Original values from IDA: 0→66, 1→69, 2→99, 3→98, 4→88, 5→87,
	// 6→77, 7→74, ±8→44, -7→41, -6→11, -5→12, -4→22, -3→23, -2→33, -1→36
	static const int kDirTable[17] = {
		44, 41, 11, 12, 22, 23, 33, 36,  // directions -8..-1
		66,                                // direction 0 (right)
		69, 99, 98, 88, 87, 77, 74, 44    // directions 1..8
	};

	double dist = sqrt((double)(dx * dx + dy * dy));
	if (dist < 0.001)
		return 88; // no movement: default down

	double cosAngle = CLIP((double)dx / dist, -1.0, 1.0);
	double angleDeg = acos(cosAngle) * 180.0 / M_PI;
	int direction = (int)(angleDeg / 22.0);
	if (direction > 8)
		direction = 8;
	if (dy < 0)
		direction = -direction;

	return kDirTable[direction + 8];
}

/**
 * Draw a single zoombini at given position with 5 layered sprites.
 * Original: Zoombini__RenderAndAnimate_45D540.
 *
 * Layers: body [cellIndex][0][0], then features A..D at:
 *   [cellIndex][1..4][featureValue].
 * Each layer is an RleBlock from the ZoombiniGfx 3D cell array.
 */
void MapTransition::drawZoombiniSprite(Graphics::ManagedSurface *dst,
                                       int zoombiniIdx, int cellIndex,
                                       int x, int y) const {
	if (!_zoombiniGfx || zoombiniIdx < 0 ||
	    zoombiniIdx >= (int)_engine->_globalZoombinis.size())
		return;

	const Zoombini *z = _engine->_globalZoombinis[zoombiniIdx];
	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Base flat index for this direction: cellIndex * kDim1 * kDim2
	int baseIdx = cellIndex * ZoombiniGfx::kDim1 * ZoombiniGfx::kDim2;

	// Body: [cellIndex][0][0]
	const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
	if (frame)
		frame->drawToScreen(dst, x, y, lut);

	// Features 1..4 (Hair, Eyes, Nose, Feet)
	const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
	for (int slot = 1; slot <= 4; slot++) {
		int featIdx = baseIdx + slot * ZoombiniGfx::kDim2 + features[slot - 1];
		frame = _zoombiniGfx->getFrame(featIdx, 0);
		if (frame)
			frame->drawToScreen(dst, x, y, lut);
	}
}

void MapTransition::draw(Graphics::ManagedSurface *screen) {
	// Draw composited background (map + overlays)
	if (_compositedBg)
		screen->blitFrom(*_compositedBg, Common::Point(0, 0));

	// Draw zoombini sprites sorted by Y position for proper overlap.
	// Original: Zoombini__SortByYAndDraw_45DB40 — bubble sort by Y,
	// then Zoombini__RenderAndAnimate_45D540 per zoombini.
	int numZoombinis = (int)_engine->_globalZoombinis.size();

	// Build sort order by Y
	Common::Array<int> drawOrder;
	for (int i = 0; i < numZoombinis; i++) {
		if (_walkStates[i].active)
			drawOrder.push_back(i);
	}

	// Bubble sort by Y (matches original sort at 0x45DB40)
	for (uint i = 0; i < drawOrder.size(); i++) {
		for (uint j = i + 1; j < drawOrder.size(); j++) {
			if (_walkStates[drawOrder[j]].screenY < _walkStates[drawOrder[i]].screenY)
				SWAP(drawOrder[i], drawOrder[j]);
		}
	}

	// Draw each zoombini in Y-sorted order
	for (uint i = 0; i < drawOrder.size(); i++) {
		int idx = drawOrder[i];
		drawZoombiniSprite(screen, idx, _walkStates[idx].cellIndex,
		                   _walkStates[idx].screenX, _walkStates[idx].screenY);
	}
}

void MapTransition::handleClick(const Common::Point &pos) {
	(void)pos;
	// Skip map animation on click
	finishTransition();
}

} // End of namespace Zoombini2
