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
#include "common/random.h"

#include "zoombini2/pages/magicwall.h"
#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"
#include "zoombini2/sound.h"

namespace Zoombini2 {

// ============================================================================
// MagicWallPuzzle — Color-coded maze navigation puzzle.
//
// Original: MagicWall__Init_41D9B0, MagicWall__MoveZoombiniOnPath_41C6C0,
//           MagicWall__AdvanceZoombiniPath_41CAD0, MagicWall__CheckAllPathsDone_41CB80.
//
// Core mechanics:
//   - 4 zoombini slots corresponding to 4 directional paths
//   - Color-coded dots mark destinations (11 colors)
//   - Colored bugs guide zoombinis to matching colored dots
//   - Each path uses EXIT%d.PAT for final exit + BOUGE%d.PAT for movement
//   - Glowworm (le_vier) provides hints
//   - 4 gates (porte-A/B/C/D) control access between maze sections
//   - Goal: Guide zoombinis through maze to their matching color exits
//
// Path system:
//   - Slot 0-3: Uses EXIT(slot+1).PAT and BOUGE(slot+1).PAT
//   - Slot 4-7: Uses EXIT(slot-3).PAT (exit only, no movement)
//   - Path files contain CurveSegment bezier data for smooth animation
//
// Original object layout (0x1B4 bytes):
//   - +72 to +91: PathObject pointers for exit paths
//   - +92 to +111: PathObject pointers for bouge (movement) paths
//   - +268: animating flag
//   - +78 to +82: Zoombini slot indices
// ============================================================================

// Color names matching resource file naming convention
const char *MagicWallPuzzle::kColorNames[kColorCount] = {
	"blue",
	"green",
	"navy",
	"orange",
	"purple",
	"red",
	"rose",
	"turquoise",
	"violet",
	"yellow"
};

// Path animation duration (ms)
static const uint32 kPathAnimDuration = 2000;

// Gate animation duration (ms)
static const uint32 kGateAnimDuration = 500;

// Minimap position
static const int kMinimapX = 620;
static const int kMinimapY = 40;

// Maze clickable regions for directing zoombinis
static const Common::Rect kPathButtons[4] = {
	Common::Rect(50, 200, 200, 350),    // Path 0 (top-left)
	Common::Rect(250, 200, 400, 350),   // Path 1 (top-right)
	Common::Rect(50, 400, 200, 550),    // Path 2 (bottom-left)
	Common::Rect(250, 400, 400, 550)    // Path 3 (bottom-right)
};

MagicWallPuzzle::MagicWallPuzzle(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageMagicWall),
	_state(kStateInit),
	  _currentLevel(0),
	  _activeSlot(-1),
	  _destSlot(-1),
	  _capturedCount(0),
	  _miniMapGfx(nullptr),
	  _miniMapDotGfx(nullptr),
	  _glowwormGfx(nullptr),
	  _glowwormAnim(nullptr),
	_musicId(-1),
	  _sndGateOpen(-1),
	  _sndZoombiniMove(-1),
	  _nextApprovalIdx(0),
	  _wallLever(0, 0, 0, 0) {

	for (int i = 0; i < 4; i++) {
		_sndApproval[i] = -1;
		_sndHint[i] = -1;
	}
	for (int i = 0; i < 2; i++) {
		_sndError[i] = -1;
	}

	for (int i = 0; i < kColorCount; i++) {
		_dotGfx[i] = nullptr;
		_bugGfx[i] = nullptr;
		_miniLightGfx[i] = nullptr;
	}
	for (int i = 0; i < 4; i++) {
		_gateAnims[i] = nullptr;
		_exitPaths[i] = nullptr;
		_bougePaths[i] = nullptr;
		_slots[i].zoombiniIdx = -1;
		_slots[i].pathProgress = 0;
		_slots[i].targetColor = -1;
		_slots[i].captured = false;
		_slots[i].x = 0;
		_slots[i].y = 0;
		_slots[i].path = nullptr;
		_slots[i].pathStartTime = 0;
		_gates[i].gateIdx = i;
		_gates[i].open = false;
		_gates[i].animStart = 0;
	}
	for (int i = 0; i < 5; i++) {
		_crystalAnims[i] = nullptr;
	}
}

MagicWallPuzzle::~MagicWallPuzzle() {
	SoundManager *snd = _engine->getSoundManager();

	// Unload music
	if (_musicId >= 0) {
		snd->stop(_musicId);
		snd->unload(_musicId);
	}

	// Unload sound effects
	for (int i = 0; i < 4; i++) {
		if (_sndApproval[i] >= 0) snd->unload(_sndApproval[i]);
		if (_sndHint[i] >= 0) snd->unload(_sndHint[i]);
	}
	for (int i = 0; i < 2; i++) {
		if (_sndError[i] >= 0) snd->unload(_sndError[i]);
	}
	if (_sndGateOpen >= 0) snd->unload(_sndGateOpen);
	if (_sndZoombiniMove >= 0) snd->unload(_sndZoombiniMove);

	for (int i = 0; i < kColorCount; i++) {
		delete _dotGfx[i];
		delete _bugGfx[i];
		delete _miniLightGfx[i];
	}
	delete _miniMapGfx;
	delete _miniMapDotGfx;
	delete _glowwormGfx;
	delete _glowwormAnim;
	for (int i = 0; i < 4; i++) {
		delete _gateAnims[i];
		delete _exitPaths[i];
		delete _bougePaths[i];
	}
	for (int i = 0; i < 5; i++) {
		delete _crystalAnims[i];
	}
}

void MagicWallPuzzle::init() {
	// Call base init for background and zoombini loading
	PuzzlePage::init();

	// BGM: 06-BB01.wav (IDA: MagicWall__Init_41D9B0)
	if (SoundManager *snd = _engine->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("sounds/music/06-BB01.wav"), true);
		if (_musicId >= 0) { snd->playLoop(_musicId); snd->setVolume(_musicId, snd->_volumeMusic); }

		// Load sound effects
		// Approval sounds (success): 6-A1.wav through 6-A4.wav
		for (int i = 0; i < 4; i++) {
			Common::Path path(Common::String::format("sounds/6-A%d.wav", i + 1));
			_sndApproval[i] = snd->load(false, path, false);
		}

		// Error sounds: 6-E1.wav, 6-E2.wav
		for (int i = 0; i < 2; i++) {
			Common::Path path(Common::String::format("sounds/6-E%d.wav", i + 1));
			_sndError[i] = snd->load(false, path, false);
		}

		// Hint sounds: 6-H1.wav through 6-H4.wav
		for (int i = 0; i < 4; i++) {
			Common::Path path(Common::String::format("sounds/6-H%d.wav", i + 1));
			_sndHint[i] = snd->load(false, path, false);
		}

		// Sound effects: gate open, movement
		_sndGateOpen = snd->load(false, Common::Path("sounds/fx/06-BS01.wav"), false);
		_sndZoombiniMove = snd->load(false, Common::Path("sounds/fx/06-BS02.wav"), false);
	}

	int diff = CLIP(_engine->getGameState()->_gameMode, 1, 3);

	debug(1, "MagicWallPuzzle::init — difficulty %d", diff);

	// Load resources
	loadResources();

	// Setup maze elements
	setupMaze();

	// Place color dots in maze
	placeColorDots();

	// Place color bugs
	placeColorBugs();

	// Setup tablets
	setupTablets();

	// Assign zoombinis to slots
	assignZoombiniSlots();

	_capturedCount = 0;
	_activeSlot = -1;
	_state = kStateIdle;
	_stateTimer = _engine->getGameTickCount();
}

void MagicWallPuzzle::loadResources() {
	// Load color dot sprites (DOT-{color}.rb)
	for (int i = 0; i < kColorCount; i++) {
		Common::Path dotPath(Common::String::format("bmp/magic_wall/DOT-%s", kColorNames[i]));
		_dotGfx[i] = new RleBlock();
		if (!_dotGfx[i]->loadFromFile(dotPath)) {
			debug(2, "MagicWallPuzzle: Failed to load DOT-%s", kColorNames[i]);
			delete _dotGfx[i];
			_dotGfx[i] = nullptr;
		}
	}

	// Load color bug sprites (bug_c_{color}.rb)
	for (int i = 0; i < kColorCount; i++) {
		Common::Path bugPath(Common::String::format("bmp/magic_wall/bug_c_%s", kColorNames[i]));
		_bugGfx[i] = new RleBlock();
		if (!_bugGfx[i]->loadFromFile(bugPath)) {
			debug(2, "MagicWallPuzzle: Failed to load bug_c_%s", kColorNames[i]);
			delete _bugGfx[i];
			_bugGfx[i] = nullptr;
		}
	}

	// Load minimap sprites
	Common::Path miniMapPath("bmp/magic_wall/mini-map");
	_miniMapGfx = new RleBlock();
	if (!_miniMapGfx->loadFromFile(miniMapPath)) {
		delete _miniMapGfx;
		_miniMapGfx = nullptr;
	}

	Common::Path miniMapDotPath("bmp/magic_wall/mini-map-dot");
	_miniMapDotGfx = new RleBlock();
	if (!_miniMapDotGfx->loadFromFile(miniMapDotPath)) {
		delete _miniMapDotGfx;
		_miniMapDotGfx = nullptr;
	}

	// Load minimap lights for each color
	for (int i = 0; i < kColorCount; i++) {
		Common::Path lightPath(Common::String::format("bmp/magic_wall/mini-light-%s", kColorNames[i]));
		_miniLightGfx[i] = new RleBlock();
		if (!_miniLightGfx[i]->loadFromFile(lightPath)) {
			delete _miniLightGfx[i];
			_miniLightGfx[i] = nullptr;
		}
	}

	// Load glowworm (le_vier_luisant.bb)
	Common::Path glowwormPath("bmp/magic_wall/le_vier_luisant");
	_glowwormGfx = new RleBlock();
	if (!_glowwormGfx->loadFromFile(glowwormPath)) {
		delete _glowwormGfx;
		_glowwormGfx = nullptr;
	}

	// Load glowworm animation (le_vier.an)
	Common::Path glowwormAnimPath("bmp/magic_wall/le_vier");
	_glowwormAnim = new Animation();
	if (!_glowwormAnim->loadFromFile(glowwormAnimPath)) {
		delete _glowwormAnim;
		_glowwormAnim = nullptr;
	}

	// Load gate animations (porte-A/B/C/D.an)
	const char *gateNames[] = { "porte-A", "porte-B", "porte-C", "porte-D" };
	for (int i = 0; i < 4; i++) {
		Common::Path gatePath(Common::String::format("bmp/magic_wall/%s", gateNames[i]));
		_gateAnims[i] = new Animation();
		if (!_gateAnims[i]->loadFromFile(gatePath)) {
			delete _gateAnims[i];
			_gateAnims[i] = nullptr;
		}
	}

	// Load crystal animations (Crystal1-5.an)
	for (int i = 0; i < 5; i++) {
		Common::Path crystalPath(Common::String::format("bmp/magic_wall/Crystal%d", i + 1));
		_crystalAnims[i] = new Animation();
		if (!_crystalAnims[i]->loadFromFile(crystalPath)) {
			delete _crystalAnims[i];
			_crystalAnims[i] = nullptr;
		}
	}

	// Load PAT bezier path files (EXIT1-4.PAT, BOUGE1-4.PAT)
	// Original: MagicWall__Init_41D9B0 loads these at offsets +72 to +111
	for (int i = 0; i < 4; i++) {
		Common::Path exitPath(Common::String::format("bmp/magic_wall/PAT/EXIT%d.PAT", i + 1));
		_exitPaths[i] = PathObject::loadFromPAT(exitPath);
		if (!_exitPaths[i]) {
			debug(2, "MagicWallPuzzle: Failed to load EXIT%d.PAT", i + 1);
		}

		Common::Path bougePath(Common::String::format("bmp/magic_wall/PAT/BOUGE%d.PAT", i + 1));
		_bougePaths[i] = PathObject::loadFromPAT(bougePath);
		if (!_bougePaths[i]) {
			debug(2, "MagicWallPuzzle: Failed to load BOUGE%d.PAT", i + 1);
		}
	}

	debug(2, "MagicWallPuzzle: Resources loaded");
}

void MagicWallPuzzle::setupMaze() {
	// Setup gate positions based on difficulty level
	// Original: MagicWall__DrawMazeLevel_41B530 positions gates in maze

	// Gate positions (approximate, based on typical maze layout)
	_gates[0].x = 150;
	_gates[0].y = 280;
	_gates[0].open = false;

	_gates[1].x = 350;
	_gates[1].y = 280;
	_gates[1].open = false;

	_gates[2].x = 150;
	_gates[2].y = 420;
	_gates[2].open = false;

	_gates[3].x = 350;
	_gates[3].y = 420;
	_gates[3].open = false;

	debug(2, "MagicWallPuzzle: Maze setup for level %d", _currentLevel);
}

void MagicWallPuzzle::placeColorDots() {
	_colorDots.clear();

	// Fixed marker positions for the beetles
	int posX[] = { 150, 350, 150, 350 };
	int posY[] = { 200, 200, 400, 400 };
	int colors[] = { kColorBlue, kColorGreen, kColorRed, kColorYellow };

	for (int i = 0; i < 4; i++) {
		ColorDot dot;
		dot.colorIdx = colors[i];
		dot.x = posX[i];
		dot.y = posY[i];
		dot.lightOn = false;
		_colorDots.push_back(dot);
	}

	debug(2, "MagicWallPuzzle: Placed 4 color markers");
}

void MagicWallPuzzle::placeColorBugs() {
	// Place color bugs to guide zoombinis
	// Each bug matches a dot color

	_colorBugs.clear();

	Common::RandomSource rnd("magicwall_bugs");

	// Match bugs to dots
	for (uint i = 0; i < _colorDots.size(); i++) {
		ColorBug bug;
		bug.colorIdx = _colorDots[i].colorIdx;
		// Place bug near but not on top of dot
		bug.x = _colorDots[i].x + rnd.getRandomNumberRng(-50, 50);
		bug.y = _colorDots[i].y + rnd.getRandomNumberRng(-30, 30);
		bug.active = true;
		_colorBugs.push_back(bug);
	}

	debug(2, "MagicWallPuzzle: Placed %d color bugs", (int)_colorBugs.size());
}

void MagicWallPuzzle::assignZoombiniSlots() {
	// Assign beetles (zoombinis) to the 4 internal slots
	int zoomIdx = 0;
	for (int slot = 0; slot < 4; slot++) {
		if (zoomIdx < (int)_puzzleZoombinis.size()) {
			_slots[slot].zoombiniIdx = zoomIdx;
			_slots[slot].pathProgress = 100;
			_slots[slot].x = _colorDots[slot].x;
			_slots[slot].y = _colorDots[slot].y;
			_slots[slot].path = nullptr;

			// Assign a color to this zoombini
			_zoombiniColors[zoomIdx] = (zoomIdx % kColorCount);

			debug(2, "MagicWallPuzzle: Slot %d -> Beetle (Zoombini %d, Color %d)", slot, zoomIdx, _zoombiniColors[zoomIdx]);
			zoomIdx++;
		}
	}
}

void MagicWallPuzzle::setupTablets() {
	_tablets.clear();

	// Define some tablets that move beetles between slots
	// In a real implementation, these would be loaded from data
	struct TabletDef {
		Common::Rect rect;
		int src, dst, pathIdx;
	};

	TabletDef defs[] = {
		{ Common::Rect(120, 150, 180, 200), 0, 1, 0 },
		{ Common::Rect(320, 150, 380, 200), 1, 2, 1 },
		{ Common::Rect(120, 350, 180, 400), 2, 3, 2 },
		{ Common::Rect(320, 350, 380, 400), 3, 0, 3 }
	};

	for (const auto &d : defs) {
		Tablet t;
		t.rect = d.rect;
		t.sourceSlot = d.src;
		t.destSlot = d.dst;
		t.path = _bougePaths[d.pathIdx];
		_tablets.push_back(t);
	}

	// Wall lever to exit the puzzle
	_wallLever = Common::Rect(550, 200, 600, 300);

	debug(2, "MagicWallPuzzle: Setup %d tablets", (int)_tablets.size());
}

void MagicWallPuzzle::startZoombiniPath(int slotIdx) {
	// This function is now used to move beetles between slots or to exit
	if (slotIdx < 0 || slotIdx >= 8)
		return;

	ZoombiniSlot &slot = _slots[slotIdx];
	if (slot.zoombiniIdx < 0)
		return;

	uint32 now = _engine->getGameTickCount();
	slot.pathProgress = 0;
	_activeSlot = slotIdx;
	_state = kStateZoombiniMoving;

	_stateTimer = now;
	slot.pathStartTime = now;

	// This is handled by the caller who provides the path
	if (slot.path) {
		slot.path->start(now);
	}
}

void MagicWallPuzzle::advanceZoombiniPath(int slotIdx) {
	// Advance zoombini along the path
	// Original: MagicWall__AdvanceZoombiniPath_41CAD0

	if (slotIdx < 0 || slotIdx >= 8)
		return;

	ZoombiniSlot &slot = _slots[slotIdx];
	if (slot.zoombiniIdx < 0 || slot.captured)
		return;

	uint32 now = _engine->getGameTickCount();

	// Use PathObject for bezier path evaluation if available
	if (slot.path) {
		int outX, outY;
		bool stillMoving = slot.path->advance(now, outX, outY);
		slot.x = outX;
		slot.y = outY;

		if (!stillMoving) {
			slot.pathProgress = 100;
		} else {
			// Estimate progress based on elapsed time (for UI purposes)
			uint32 elapsed = now - slot.pathStartTime;
			slot.pathProgress = MIN((int)(elapsed / 20), 99);
		}
	} else {
		// Fallback to linear interpolation if PAT not loaded
		uint32 elapsed = now - _stateTimer;
		float progress = (float)elapsed / kPathAnimDuration;

		if (progress >= 1.0f) {
			slot.pathProgress = 100;
		} else {
			slot.pathProgress = (int)(progress * 100);
		}

		// Linear interpolation as fallback
		int startX = 50 + slotIdx * 100;
		int startY = 550;
		int endX = 50 + slotIdx * 100;
		int endY = 50;  // Exit at top

		slot.x = startX + (int)((endX - startX) * progress);
		slot.y = startY + (int)((endY - startY) * progress);
	}
}

bool MagicWallPuzzle::checkSlotComplete(int slotIdx) {
	if (slotIdx < 0 || slotIdx >= 8)
		return false;

	return _slots[slotIdx].pathProgress >= 100;
}

void MagicWallPuzzle::completeSlot(int slotIdx) {
	// Finish path animation for beetle in slotIdx
	if (slotIdx < 0 || slotIdx >= 8)
		return;

	ZoombiniSlot &slot = _slots[slotIdx];
	int zoomIdx = slot.zoombiniIdx;

	if (slotIdx >= 4) {
		// Reached exit
		slot.captured = true; // We should probably use a different flag now
		if (zoomIdx >= 0 && zoomIdx < (int)_puzzleZoombinis.size()) {
			_puzzleZoombinis[zoomIdx]->_freeStatus = 1; // Captured
		}
		_capturedCount++;
		debug(1, "MagicWallPuzzle: Beetle %d exited", zoomIdx);
	} else {
		// Moved to another slot
		if (_destSlot >= 0 && _destSlot < 8) {
			_slots[_destSlot].zoombiniIdx = zoomIdx;
			_slots[_destSlot].pathProgress = 100;
			_slots[_destSlot].x = _slots[_destSlot].path ? 0 : _colorDots[_destSlot % 4].x; // simplified
			_slots[_destSlot].y = _slots[_destSlot].path ? 0 : _colorDots[_destSlot % 4].y;
			
			// Clear current slot
			slot.zoombiniIdx = -1;
			slot.pathProgress = 0;
			debug(1, "MagicWallPuzzle: Beetle %d moved to slot %d", zoomIdx, _destSlot);
		}
	}

	updateLights();

	// Play approval sound
	if (SoundManager *snd = _engine->getSoundManager()) {
		if (_sndApproval[_nextApprovalIdx] >= 0) {
			snd->play(_sndApproval[_nextApprovalIdx]);
		}
		_nextApprovalIdx = (_nextApprovalIdx + 1) % 4;
	}
}

void MagicWallPuzzle::updateLights() {
	for (uint i = 0; i < _colorDots.size(); i++) {
		int zoomIdx = _slots[i].zoombiniIdx;
		if (zoomIdx >= 0 && zoomIdx < 16) {
			if (_zoombiniColors[zoomIdx] == _colorDots[i].colorIdx) {
				_colorDots[i].lightOn = true;
			} else {
				_colorDots[i].lightOn = false;
			}
		} else {
			_colorDots[i].lightOn = false;
		}
	}
}

int MagicWallPuzzle::countCaptured() const {
	int count = 0;
	for (int i = 0; i < 4; i++) {
		if (_slots[i].captured)
			count++;
	}
	return count;
}

void MagicWallPuzzle::update() {
	uint32 now = _engine->getGameTickCount();
	uint32 elapsed = now - _stateTimer;

	switch (_state) {
	case kStateInit:
		// Should not happen after init()
		break;

	case kStateIdle:
		// Waiting for player input - handled in handleClick()
		break;

	case kStateZoombiniMoving:
		// Animate zoombini movement along path
		if (_activeSlot >= 0) {
			advanceZoombiniPath(_activeSlot);

			if (checkSlotComplete(_activeSlot)) {
				completeSlot(_activeSlot);

				// Check if all captured
				if (countCaptured() >= 4 || countCaptured() >= (int)_puzzleZoombinis.size()) {
					_state = kStateComplete;
					_stateTimer = now;
				} else {
					_activeSlot = -1;
					_state = kStateIdle;
				}
			}
		} else {
			_state = kStateIdle;
		}
		break;

	case kStateGateOpening:
		// Gate animation
		if (elapsed > kGateAnimDuration) {
			_state = kStateIdle;
		}
		break;

	case kStateComplete:
		debug(1, "MagicWallPuzzle: All zoombinis captured (%d)", countCaptured());
		_state = kStateDone;
		_stateTimer = now;
		break;

	case kStateDone:
		// Wait before transitioning out
		if (elapsed > 2000) {
			debug(1, "MagicWallPuzzle: Complete, returning to map");
			_engine->_returningFromPuzzle = true;
			_engine->_maptransSourceWorld = kPageMagicWall;
			_engine->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void MagicWallPuzzle::draw(Graphics::ManagedSurface *screen) {
	// Draw background
	if (_background)
		_background->drawToSurface(screen, 0, 0);

	// Draw maze elements
	drawMazeLevel(screen, _currentLevel);
	drawColorDots(screen);
	drawColorBugs(screen);
	drawTablets(screen);
	drawWallLever(screen);
	drawGates(screen);
	drawMinimap(screen);
	drawZoombinis(screen);

	// Draw debug info
#if 0
	// Debug: Draw clickable regions
	for (int i = 0; i < 4; i++) {
		screen->frameRect(kPathButtons[i], 0xFFFF00);
	}
#endif
}

void MagicWallPuzzle::drawMazeLevel(Graphics::ManagedSurface *screen, int level) {
	// Original: MagicWall__DrawMazeLevel_41B530
	// Draws background, overlay, edges (white ripple lines), and dots

	// Background already drawn above

	// Draw some maze structure indicators (stub)
	// Real implementation would use maze graph data
	uint32 wallColor = 0x404040;  // Dark grey

	// Simple maze walls (horizontal)
	screen->hLine(100, 200, 500, wallColor);
	screen->hLine(100, 350, 500, wallColor);
	screen->hLine(100, 500, 500, wallColor);

	// Simple maze walls (vertical)
	screen->vLine(100, 200, 500, wallColor);
	screen->vLine(300, 200, 500, wallColor);
	screen->vLine(500, 200, 500, wallColor);
}

void MagicWallPuzzle::drawColorDots(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (uint i = 0; i < _colorDots.size(); i++) {
		const ColorDot &dot = _colorDots[i];
		RleBlock *gfx = _dotGfx[dot.colorIdx];

		if (gfx) {
			gfx->drawToScreen(screen, dot.x, dot.y, lut);
		} else {
			// Fallback: draw colored circle
			uint32 colors[] = {
				0x0000FF, // blue
				0x00FF00, // green
				0x000080, // navy
				0xFF8000, // orange
				0x800080, // purple
				0xFF0000, // red
				0xFF80C0, // rose
				0x00FFFF, // turquoise
				0x8000FF, // violet
				0xFFFF00  // yellow
			};
			screen->fillRect(Common::Rect(dot.x - 8, dot.y - 8, dot.x + 8, dot.y + 8),
							 colors[dot.colorIdx % 10]);
		}

		if (dot.lightOn) {
			// Draw light above dot
			screen->fillRect(Common::Rect(dot.x - 4, dot.y - 20, dot.x + 4, dot.y - 12), 0x00FFFF);
		}
	}
}

void MagicWallPuzzle::drawColorBugs(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (uint i = 0; i < _colorBugs.size(); i++) {
		const ColorBug &bug = _colorBugs[i];
		if (!bug.active)
			continue;

		RleBlock *gfx = _bugGfx[bug.colorIdx];
		if (gfx) {
			gfx->drawToScreen(screen, bug.x, bug.y, lut);
		}
	}
}

void MagicWallPuzzle::drawTablets(Graphics::ManagedSurface *screen) {
	for (uint i = 0; i < _tablets.size(); i++) {
		const Tablet &t = _tablets[i];
		screen->fillRect(t.rect, 0x808080); // Grey stone
		screen->frameRect(t.rect, 0x000000);
	}
}

void MagicWallPuzzle::drawWallLever(Graphics::ManagedSurface *screen) {
	screen->fillRect(_wallLever, 0xCCAA00); // Gold lever
	screen->frameRect(_wallLever, 0x000000);
}

void MagicWallPuzzle::drawMinimap(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw minimap background
	if (_miniMapGfx) {
		_miniMapGfx->drawToScreen(screen, kMinimapX, kMinimapY, lut);
	}

	// Draw dots on minimap showing zoombini positions
	for (int i = 0; i < 4; i++) {
		const ZoombiniSlot &slot = _slots[i];
		if (slot.zoombiniIdx >= 0 && !slot.captured) {
			int dotColor = slot.targetColor;
			if (dotColor >= 0 && dotColor < kColorCount && _miniLightGfx[dotColor]) {
				// Scale slot position to minimap
				int miniX = kMinimapX + 10 + (slot.x * 80 / 640);
				int miniY = kMinimapY + 10 + (slot.y * 60 / 480);
				_miniLightGfx[dotColor]->drawToScreen(screen, miniX, miniY, lut);
			}
		}
	}
}

void MagicWallPuzzle::drawGates(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	for (int i = 0; i < 4; i++) {
		const Gate &gate = _gates[i];
		Animation *anim = _gateAnims[i];

		if (anim) {
			// Select animation frame based on gate state
			// Open gates show frame 1, closed gates show frame 0
			int frameIdx = gate.open ? 1 : 0;
			const RleBlock *frame = anim->getFrame(frameIdx);
			if (frame) {
				frame->drawToScreen(screen, gate.x, gate.y, lut);
			}
		} else {
			// Fallback: draw simple rectangle
			uint32 color = gate.open ? 0x00FF00 : 0xFF0000;
			screen->fillRect(Common::Rect(gate.x - 10, gate.y - 20, gate.x + 10, gate.y + 20), color);
		}
	}
}

void MagicWallPuzzle::drawZoombinis(Graphics::ManagedSurface *screen) {
	const byte (*lut)[256] = _engine->getAlphaLUT();

	// Draw zoombinis in their slots
	for (int i = 0; i < 8; i++) {
		const ZoombiniSlot &slot = _slots[i];
		if (slot.zoombiniIdx < 0 || (i >= 4 && slot.captured))
			continue;

		const Zoombini *z = _puzzleZoombinis[slot.zoombiniIdx];

		// Draw zoombini at current position
		if (_zoombiniGfx) {
			int baseIdx = 0;  // Standing still

			// Body
			const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
			if (frame)
				frame->drawToScreen(screen, slot.x, slot.y, lut);

			// Features
			const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
			for (int feat = 1; feat <= 4; feat++) {
				int featIdx = baseIdx + feat * ZoombiniGfx::kDim2 + features[feat - 1];
				frame = _zoombiniGfx->getFrame(featIdx, 0);
				if (frame)
					frame->drawToScreen(screen, slot.x, slot.y, lut);
			}
		}
	}
}

void MagicWallPuzzle::handleClick(const Common::Point &pos) {
	if (_state != kStateIdle)
		return;

	// Check tablets
	for (uint i = 0; i < _tablets.size(); i++) {
		const Tablet &t = _tablets[i];
		if (t.rect.contains(pos)) {
			// Check if there is a beetle in the source slot
			if (_slots[t.sourceSlot].zoombiniIdx >= 0) {
				debug(2, "MagicWallPuzzle: Tablet %d clicked, moving beetle from %d to %d", 
					  i, t.sourceSlot, t.destSlot);
				
				_destSlot = t.destSlot;
				_slots[t.sourceSlot].path = t.path;
				startZoombiniPath(t.sourceSlot);
				return;
			}
		}
	}

	// Check wall lever
	if (_wallLever.contains(pos)) {
		// Only work if all lights are on
		bool allOn = true;
		for (uint i = 0; i < _colorDots.size(); i++) {
			if (!_colorDots[i].lightOn) {
				allOn = false;
				break;
			}
		}

		if (allOn) {
			debug(2, "MagicWallPuzzle: Wall lever pressed, all lights on! Opening doors.");
			// Start exit sequence for all beetles
			for (int i = 0; i < 4; i++) {
				if (_slots[i].zoombiniIdx >= 0) {
					_destSlot = i + 4;
					_slots[i].path = _exitPaths[i];
					startZoombiniPath(i);
					// Note: This will only move one at a time in current update loop
					// A better implementation would handle multiple simultaneous movements
					break; 
				}
			}
		} else {
			debug(2, "MagicWallPuzzle: Lever pressed but not all lights are on.");
		}
		return;
	}

	debug(2, "MagicWallPuzzle: Click at %d,%d", pos.x, pos.y);
}

} // End of namespace Zoombini2
