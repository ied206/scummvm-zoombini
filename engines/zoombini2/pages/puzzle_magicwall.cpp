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

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_magicwall.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// PuzzleMagicWall - color-coded maze navigation puzzle.
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
// ============================================================================

// Color names matching resource file naming convention
const char *PuzzleMagicWall::kColorNames[kColorCount] = {
	"blue",
	"green",
	"navy",
	"orange",
	"purple",
	"red",
	"rose",
	"turquoise",
	"violet",
	"yellow"};

// Path animation duration (ms)
static const uint32 kPathAnimDuration = 2000;

// Gate animation duration (ms)
static const uint32 kGateAnimDuration = 500;

// Minimap position
static const Common::Point32 kMinimapPos(620, 40);

// Maze clickable regions for directing zoombinis
static const Common::Rect kPathButtons[4] = {
	Common::Rect(50, 200, 200, 350),  // Path 0 (top-left)
	Common::Rect(250, 200, 400, 350), // Path 1 (top-right)
	Common::Rect(50, 400, 200, 550),  // Path 2 (bottom-left)
	Common::Rect(250, 400, 400, 550)  // Path 3 (bottom-right)
};

PuzzleMagicWall::PuzzleMagicWall(Zoombini2Engine *vm)
	: PuzzleBase(vm, kPageMagicWall),
	  _state(kStateInit),
	  _currentLevel(0),
	  _activeSlot(-1),
	  _destSlot(-1),
	  _capturedCount(0),
	  _miniMapImage(nullptr),
	  _miniMapDotImage(nullptr),
	  _glowwormImage(nullptr),
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
		_dotImage[i] = nullptr;
		_bugImage[i] = nullptr;
		_miniLightImage[i] = nullptr;
	}
	for (int i = 0; i < 4; i++) {
		_gateAnims[i] = nullptr;
		_exitPaths[i] = nullptr;
		_bougePaths[i] = nullptr;
		_slots[i].zoombiniIdx = -1;
		_slots[i].pathProgress = 0;
		_slots[i].targetColor = -1;
		_slots[i].captured = false;
		_slots[i].pos = Common::Point32();
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

PuzzleMagicWall::~PuzzleMagicWall() {
	SoundManager *snd = _vm->getSoundManager();

	// Unload music
	if (_musicId >= 0) {
		snd->stop(_musicId);
		snd->unload(_musicId);
	}

	// Unload sound effects
	for (int i = 0; i < 4; i++) {
		if (_sndApproval[i] >= 0)
			snd->unload(_sndApproval[i]);
		if (_sndHint[i] >= 0)
			snd->unload(_sndHint[i]);
	}
	for (int i = 0; i < 2; i++) {
		if (_sndError[i] >= 0)
			snd->unload(_sndError[i]);
	}
	if (_sndGateOpen >= 0)
		snd->unload(_sndGateOpen);
	if (_sndZoombiniMove >= 0)
		snd->unload(_sndZoombiniMove);

	for (int i = 0; i < kColorCount; i++) {
		delete _dotImage[i];
		delete _bugImage[i];
		delete _miniLightImage[i];
	}
	delete _miniMapImage;
	delete _miniMapDotImage;
	delete _glowwormImage;
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

void PuzzleMagicWall::init() {
	// Call base init for background and zoombini loading
	PuzzleBase::init();

	// Start the Beetle Bug Alley music.
	if (SoundManager *snd = _vm->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("#sounds/music/06-BB01.wav"), true);
		if (_musicId >= 0) {
			snd->playLoop(_musicId);
			snd->setVolume(_musicId, snd->_volumeMusic);
		}

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

	int level = CLIP(_vm->getGameState()->_level, 1, 3);

	debug(1, "PuzzleMagicWall::init - level %d", level);

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
	_stateTimer = _vm->getGameTickCount();
}

void PuzzleMagicWall::loadResources() {
	// Load color dot sprites (DOT-{color}.rb)
	for (int i = 0; i < kColorCount; i++) {
		Common::Path dotPath(Common::String::format("bmp/magic_wall/DOT-%s", kColorNames[i]));
		_dotImage[i] = new RleBlock();
		if (!_dotImage[i]->loadFromFile(dotPath)) {
			debug(2, "PuzzleMagicWall: Failed to load DOT-%s", kColorNames[i]);
			delete _dotImage[i];
			_dotImage[i] = nullptr;
		}
	}

	// Load color bug sprites (bug_c_{color}.rb)
	for (int i = 0; i < kColorCount; i++) {
		Common::Path bugPath(Common::String::format("bmp/magic_wall/bug_c_%s", kColorNames[i]));
		_bugImage[i] = new RleBlock();
		if (!_bugImage[i]->loadFromFile(bugPath)) {
			debug(2, "PuzzleMagicWall: Failed to load bug_c_%s", kColorNames[i]);
			delete _bugImage[i];
			_bugImage[i] = nullptr;
		}
	}

	// Load minimap sprites
	Common::Path miniMapPath("bmp/magic_wall/mini-map");
	_miniMapImage = new RleBlock();
	if (!_miniMapImage->loadFromFile(miniMapPath)) {
		delete _miniMapImage;
		_miniMapImage = nullptr;
	}

	Common::Path miniMapDotPath("bmp/magic_wall/mini-map-dot");
	_miniMapDotImage = new RleBlock();
	if (!_miniMapDotImage->loadFromFile(miniMapDotPath)) {
		delete _miniMapDotImage;
		_miniMapDotImage = nullptr;
	}

	// Load minimap lights for each color
	for (int i = 0; i < kColorCount; i++) {
		Common::Path lightPath(Common::String::format("bmp/magic_wall/mini-light-%s", kColorNames[i]));
		_miniLightImage[i] = new RleBlock();
		if (!_miniLightImage[i]->loadFromFile(lightPath)) {
			delete _miniLightImage[i];
			_miniLightImage[i] = nullptr;
		}
	}

	// Load glowworm (le_vier_luisant.bb)
	Common::Path glowwormPath("bmp/magic_wall/le_vier_luisant");
	_glowwormImage = new RleBlock();
	if (!_glowwormImage->loadFromFile(glowwormPath)) {
		delete _glowwormImage;
		_glowwormImage = nullptr;
	}

	// Load glowworm animation (le_vier.an)
	Common::Path glowwormAnimPath("bmp/magic_wall/le_vier");
	_glowwormAnim = new Animation();
	if (!_glowwormAnim->loadFromFile(glowwormAnimPath)) {
		delete _glowwormAnim;
		_glowwormAnim = nullptr;
	}

	// Load gate animations (porte-A/B/C/D.an)
	const char *gateNames[] = {"porte-A", "porte-B", "porte-C", "porte-D"};
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

	// Load exit and internal movement paths.
	for (int i = 0; i < 4; i++) {
		Common::Path exitPath(Common::String::format("bmp/magic_wall/PAT/EXIT%d.PAT", i + 1));
		_exitPaths[i] = PathObject::loadFromPAT(exitPath);
		if (!_exitPaths[i]) {
			debug(2, "PuzzleMagicWall: Failed to load EXIT%d.PAT", i + 1);
		}

		Common::Path bougePath(Common::String::format("bmp/magic_wall/PAT/BOUGE%d.PAT", i + 1));
		_bougePaths[i] = PathObject::loadFromPAT(bougePath);
		if (!_bougePaths[i]) {
			debug(2, "PuzzleMagicWall: Failed to load BOUGE%d.PAT", i + 1);
		}
	}

	debug(2, "PuzzleMagicWall: Resources loaded");
}

void PuzzleMagicWall::setupMaze() {
	// Set up gate positions for the selected maze level.

	// Gate positions (approximate, based on typical maze layout)
	_gates[0].pos = Common::Point32(150, 280);
	_gates[0].open = false;

	_gates[1].pos = Common::Point32(350, 280);
	_gates[1].open = false;

	_gates[2].pos = Common::Point32(150, 420);
	_gates[2].open = false;

	_gates[3].pos = Common::Point32(350, 420);
	_gates[3].open = false;

	debug(2, "PuzzleMagicWall: Maze setup for level %d", _currentLevel);
}

void PuzzleMagicWall::placeColorDots() {
	_colorDots.clear();

	// Fixed marker positions for the beetles
	static const Common::Point32 kColorDotPos[] = {
		Common::Point32(150, 200), Common::Point32(350, 200), Common::Point32(150, 400), Common::Point32(350, 400)};
	int colors[] = {kColorBlue, kColorGreen, kColorRed, kColorYellow};

	for (int i = 0; i < 4; i++) {
		ColorDot dot;
		dot.colorIdx = colors[i];
		dot.pos = kColorDotPos[i];
		dot.lightOn = false;
		_colorDots.push_back(dot);
	}

	debug(2, "PuzzleMagicWall: Placed 4 color markers");
}

void PuzzleMagicWall::placeColorBugs() {
	// Place color bugs to guide zoombinis
	// Each bug matches a dot color

	_colorBugs.clear();

	Common::RandomSource rnd("magicwall_bugs");

	// Match bugs to dots
	for (uint i = 0; i < _colorDots.size(); i++) {
		ColorBug bug;
		bug.colorIdx = _colorDots[i].colorIdx;
		// Place bug near but not on top of dot
		bug.pos = Common::Point32(
			_colorDots[i].pos.x + rnd.getRandomNumberRng(-50, 50), _colorDots[i].pos.y + rnd.getRandomNumberRng(-30, 30));
		bug.active = true;
		_colorBugs.push_back(bug);
	}

	debug(2, "PuzzleMagicWall: Placed %d color bugs", (int)_colorBugs.size());
}

void PuzzleMagicWall::assignZoombiniSlots() {
	// Assign beetles (zoombinis) to the 4 internal slots
	int zoomIdx = 0;
	for (int slot = 0; slot < 4; slot++) {
		if (zoomIdx < (int)_puzzleZoombinis.size()) {
			_slots[slot].zoombiniIdx = zoomIdx;
			_slots[slot].pathProgress = 100;
			_slots[slot].pos = _colorDots[slot].pos;
			_slots[slot].path = nullptr;

			// Assign a color to this zoombini
			_zoombiniColors[zoomIdx] = (zoomIdx % kColorCount);

			debug(2, "PuzzleMagicWall: Slot %d -> Beetle (Zoombini %d, Color %d)", slot, zoomIdx, _zoombiniColors[zoomIdx]);
			zoomIdx++;
		}
	}
}

void PuzzleMagicWall::setupTablets() {
	_tablets.clear();

	// Define some tablets that move beetles between slots
	// In a real implementation, these would be loaded from data
	struct TabletDef {
		Common::Rect rect;
		int src, dst, pathIdx;
	};

	TabletDef defs[] = {
		{Common::Rect(120, 150, 180, 200), 0, 1, 0},
		{Common::Rect(320, 150, 380, 200), 1, 2, 1},
		{Common::Rect(120, 350, 180, 400), 2, 3, 2},
		{Common::Rect(320, 350, 380, 400), 3, 0, 3}};

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

	debug(2, "PuzzleMagicWall: Setup %d tablets", (int)_tablets.size());
}

void PuzzleMagicWall::startZoombiniPath(int slotIdx) {
	// This function is now used to move beetles between slots or to exit
	if (slotIdx < 0 || slotIdx >= 8)
		return;

	ZoombiniSlot &slot = _slots[slotIdx];
	if (slot.zoombiniIdx < 0)
		return;

	uint32 now = _vm->getGameTickCount();
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

void PuzzleMagicWall::advanceZoombiniPath(int slotIdx) {
	// Advance the selected Zoombini along its active path.

	if (slotIdx < 0 || slotIdx >= 8)
		return;

	ZoombiniSlot &slot = _slots[slotIdx];
	if (slot.zoombiniIdx < 0 || slot.captured)
		return;

	uint32 now = _vm->getGameTickCount();

	// Use PathObject for bezier path evaluation if available
	if (slot.path) {
		Common::Point32 pos;
		bool stillMoving = slot.path->advance(now, pos);
		slot.pos = pos;

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
		const Common::Point32 startPos(50 + slotIdx * 100, 550);
		const Common::Point32 endPos(50 + slotIdx * 100, 50);
		slot.pos = Common::Point32(
			startPos.x + static_cast<int32>((endPos.x - startPos.x) * progress),
			startPos.y + static_cast<int32>((endPos.y - startPos.y) * progress));
	}
}

bool PuzzleMagicWall::checkSlotComplete(int slotIdx) {
	if (slotIdx < 0 || slotIdx >= 8)
		return false;

	return _slots[slotIdx].pathProgress >= 100;
}

void PuzzleMagicWall::completeSlot(int slotIdx) {
	// Finish path animation for beetle in slotIdx
	if (slotIdx < 0 || slotIdx >= 8)
		return;

	ZoombiniSlot &slot = _slots[slotIdx];
	int zoomIdx = slot.zoombiniIdx;

	if (slotIdx >= 4) {
		// Reached exit
		slot.captured = true; // We should probably use a different flag now
		if (zoomIdx >= 0 && zoomIdx < (int)_puzzleZoombinis.size()) {
			_puzzleZoombinis[zoomIdx]->_puzzleStatus = 1;
		}
		_capturedCount++;
		debug(1, "PuzzleMagicWall: Beetle %d exited", zoomIdx);
	} else {
		// Moved to another slot
		if (_destSlot >= 0 && _destSlot < 8) {
			_slots[_destSlot].zoombiniIdx = zoomIdx;
			_slots[_destSlot].pathProgress = 100;
			if (_slots[_destSlot].path) {
				_slots[_destSlot].pos = Common::Point32();
			} else {
				_slots[_destSlot].pos = _colorDots[_destSlot % 4].pos;
			}

			// Clear current slot
			slot.zoombiniIdx = -1;
			slot.pathProgress = 0;
			debug(1, "PuzzleMagicWall: Beetle %d moved to slot %d", zoomIdx, _destSlot);
		}
	}

	updateLights();

	// Play approval sound
	if (SoundManager *snd = _vm->getSoundManager()) {
		if (_sndApproval[_nextApprovalIdx] >= 0) {
			snd->play(_sndApproval[_nextApprovalIdx]);
		}
		_nextApprovalIdx = (_nextApprovalIdx + 1) % 4;
	}
}

void PuzzleMagicWall::updateLights() {
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

int PuzzleMagicWall::countCaptured() const {
	int count = 0;
	for (int i = 0; i < 4; i++) {
		if (_slots[i].captured)
			count++;
	}
	return count;
}

void PuzzleMagicWall::onUpdate() {
	uint32 now = _vm->getGameTickCount();
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
		debug(1, "PuzzleMagicWall: All zoombinis captured (%d)", countCaptured());
		_state = kStateDone;
		_stateTimer = now;
		break;

	case kStateDone:
		// Wait before transitioning out
		if (elapsed > 2000) {
			debug(1, "PuzzleMagicWall: Complete, returning to map");
			_vm->_returningFromPuzzle = true;
			_vm->_mapTransitionSourcePageId = kPageMagicWall;
			_vm->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void PuzzleMagicWall::onRenderBackground(ManagedSurface32 *screen) {
	if (_background)
		_background->drawToSurface(screen, Common::Point32(0, 0));
}

void PuzzleMagicWall::onRenderScene(ManagedSurface32 *screen) {

	// Draw maze elements
	drawMazeLevel(screen, _currentLevel);
	drawColorDots(screen);
	drawColorBugs(screen);
	drawTablets(screen);
	drawWallLever(screen);
	drawGates(screen);
	drawMinimap(screen);


	// Draw debug info
#if 0
	// Debug: Draw clickable regions
	for (int i = 0; i < 4; i++) {
		screen->frameRect(kPathButtons[i], 0xFFFF00);
	}
#endif
}

void PuzzleMagicWall::drawMazeLevel(ManagedSurface32 *screen, int level) {
	// The page background is already drawn before the maze overlays.

	// Draw some maze structure indicators (stub)
	// Real implementation would use maze graph data
	uint32 wallColor = 0x404040; // Dark grey

	// Simple maze walls (horizontal)
	screen->hLine(100, 200, 500, wallColor);
	screen->hLine(100, 350, 500, wallColor);
	screen->hLine(100, 500, 500, wallColor);

	// Simple maze walls (vertical)
	screen->vLine(100, 200, 500, wallColor);
	screen->vLine(300, 200, 500, wallColor);
	screen->vLine(500, 200, 500, wallColor);
}

void PuzzleMagicWall::drawColorDots(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	for (uint i = 0; i < _colorDots.size(); i++) {
		const ColorDot &dot = _colorDots[i];
		RleBlock *image = _dotImage[dot.colorIdx];

		if (image) {
			image->drawToScreen(screen, dot.pos, lut);
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
			screen->fillRect(
				Common::Rect(
					static_cast<int16>(dot.pos.x - 8), static_cast<int16>(dot.pos.y - 8),
					static_cast<int16>(dot.pos.x + 8), static_cast<int16>(dot.pos.y + 8)),
				colors[dot.colorIdx % 10]);
		}

		if (dot.lightOn) {
			// Draw light above dot
			screen->fillRect(
				Common::Rect(
					static_cast<int16>(dot.pos.x - 4), static_cast<int16>(dot.pos.y - 20),
					static_cast<int16>(dot.pos.x + 4), static_cast<int16>(dot.pos.y - 12)),
				0x00FFFF);
		}
	}
}

void PuzzleMagicWall::drawColorBugs(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	for (uint i = 0; i < _colorBugs.size(); i++) {
		const ColorBug &bug = _colorBugs[i];
		if (!bug.active)
			continue;

		RleBlock *image = _bugImage[bug.colorIdx];
		if (image) {
			image->drawToScreen(screen, bug.pos, lut);
		}
	}
}

void PuzzleMagicWall::drawTablets(ManagedSurface32 *screen) {
	for (uint i = 0; i < _tablets.size(); i++) {
		const Tablet &t = _tablets[i];
		screen->fillRect(t.rect, 0x808080); // Grey stone
		screen->frameRect(t.rect, 0x000000);
	}
}

void PuzzleMagicWall::drawWallLever(ManagedSurface32 *screen) {
	screen->fillRect(_wallLever, 0xCCAA00); // Gold lever
	screen->frameRect(_wallLever, 0x000000);
}

void PuzzleMagicWall::drawMinimap(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	// Draw minimap background
	if (_miniMapImage) {
		_miniMapImage->drawToScreen(screen, kMinimapPos, lut);
	}

	// Draw dots on minimap showing zoombini positions
	for (int i = 0; i < 4; i++) {
		const ZoombiniSlot &slot = _slots[i];
		if (slot.zoombiniIdx >= 0 && !slot.captured) {
			int dotColor = slot.targetColor;
			if (dotColor >= 0 && dotColor < kColorCount && _miniLightImage[dotColor]) {
				// Scale slot position to minimap
				const Common::Point32 miniPos(
					kMinimapPos.x + 10 + (slot.pos.x * 80 / 640),
					kMinimapPos.y + 10 + (slot.pos.y * 60 / 480));
				_miniLightImage[dotColor]->drawToScreen(screen, miniPos, lut);
			}
		}
	}
}

void PuzzleMagicWall::drawGates(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	for (int i = 0; i < 4; i++) {
		const Gate &gate = _gates[i];
		Animation *anim = _gateAnims[i];

		if (anim) {
			// Select animation frame based on gate state
			// Open gates show frame 1, closed gates show frame 0
			int frameIdx = gate.open ? 1 : 0;
			const RleBlock *frame = anim->getFrame(frameIdx);
			if (frame) {
				frame->drawToScreen(screen, gate.pos, lut);
			}
		} else {
			// Fallback: draw simple rectangle
			uint32 color = gate.open ? 0x00FF00 : 0xFF0000;
			screen->fillRect(
				Common::Rect(
					static_cast<int16>(gate.pos.x - 10), static_cast<int16>(gate.pos.y - 20),
					static_cast<int16>(gate.pos.x + 10), static_cast<int16>(gate.pos.y + 20)),
				color);
		}
	}
}

void PuzzleMagicWall::onRenderActors(ManagedSurface32 *screen) {
	const AlphaBlendLUT &lut = _vm->getAlphaLUT();

	// Draw zoombinis in their slots
	for (int i = 0; i < 8; i++) {
		const ZoombiniSlot &slot = _slots[i];
		if (slot.zoombiniIdx < 0 || (i >= 4 && slot.captured))
			continue;

		const ZoombiniState *z = _puzzleZoombinis[slot.zoombiniIdx];

		// Draw zoombini at current position
		if (_zoombiniAnimation) {
			_zoombiniAnimation->drawZoombini(screen, z->_traits, slot.pos, 0, 0, lut);
		}
	}
}

EventHandleResult PuzzleMagicWall::onLButtonDown(const Common::Point &pos) {
	if (_state != kStateIdle)
		return EventHandleResult::kPassthrough;

	// Check tablets
	for (uint i = 0; i < _tablets.size(); i++) {
		const Tablet &t = _tablets[i];
		if (t.rect.contains(pos)) {
			// Check if there is a beetle in the source slot
			if (_slots[t.sourceSlot].zoombiniIdx >= 0) {
				debug(2, "PuzzleMagicWall: Tablet %d clicked, moving beetle from %d to %d",
					  i, t.sourceSlot, t.destSlot);

				_destSlot = t.destSlot;
				_slots[t.sourceSlot].path = t.path;
				startZoombiniPath(t.sourceSlot);
				return EventHandleResult::kConsumed;
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
			debug(2, "PuzzleMagicWall: Wall lever pressed, all lights on! Opening doors.");
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
			debug(2, "PuzzleMagicWall: Lever pressed but not all lights are on.");
		}
		return EventHandleResult::kConsumed;
	}

	debug(2, "PuzzleMagicWall: Click at %d,%d", pos.x, pos.y);
	return EventHandleResult::kPassthrough;
}

} // End of namespace Zoombini2
