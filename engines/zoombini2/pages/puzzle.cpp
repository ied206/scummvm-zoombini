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

#include "zoombini2/pages/puzzle.h"
#include "zoombini2/gfx.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// PuzzlePage — base puzzle implementation.
//
// Each puzzle has its own resource directory under bmp/{puzzleName}/.
// Two interaction models (from original CPage vtable):
//   Blocking: logic runs entirely in init(), update() is nullsub
//   Non-blocking: update() override for frame-by-frame logic (ChezNorf, WallOfFleens)
//
// Current state: Base implementation with auto-advance for flow testing.
// Individual puzzle logic to be implemented in derived classes.
// ============================================================================

// Auto-advance delay in milliseconds (for unimplemented puzzles)
static const uint32 kAutoAdvanceDelay = 5000;

static const struct {
	int id;
	const char *name;
	const char *dir;
	const char *bgName;  // Background BMP name (without bmp/ prefix or .bmp extension)
} kPuzzleInfo[] = {
	// Background paths from IDA: each puzzle's Init function loads via Scene__LoadBackground
	{ kPageCrazyTurtle,  "CrazyTurtle",  "crazy_turtle",    "crazy_turtle/background" },
	{ kPageWaterslide,   "Waterslide",   "waterslide",      "waterslide/waterslides" },
	{ kPageAquacube,     "Aquacube",     "aquacube",         "aquacube/background" },
	{ kPageMysticMarsh,  "MysticMarsh",  "mystic_marsh",    "mystic_marsh/background1" },
	{ kPageMagicWall,    "MagicWall",    "magic_wall",      "magic_wall/magic wall" },
	{ kPageWallOfFleens, "WallOfFleens", "wall_of_fleens",  "wall_of_fleens/background" },
	{ kPageChezNorf,     "ChezNorf",     "chez_norf",       "chez_norf/baquegund" },
	{ kPageSnowboard,    "Snowboard",    "snowboard",       "snowboard/snowboard-EASY" },
	{ kPageBoolies,      "Boolies",      "boolies",         "Boolies/background" },
	{ kPageBooliewood,   "Booliewood",   "booliewood",      "booliewood/background" },
	{ 0, nullptr, nullptr, nullptr }
};

/* static */
const char *PuzzlePage::getPuzzleName(int puzzleId) {
	for (int i = 0; kPuzzleInfo[i].name; i++) {
		if (kPuzzleInfo[i].id == puzzleId)
			return kPuzzleInfo[i].name;
	}
	return "Unknown";
}

/* static */
const char *PuzzlePage::getPuzzleDir(int puzzleId) {
	for (int i = 0; kPuzzleInfo[i].dir; i++) {
		if (kPuzzleInfo[i].id == puzzleId)
			return kPuzzleInfo[i].dir;
	}
	return nullptr;
}

PuzzlePage::PuzzlePage(Zoombini2Engine *engine, int puzzleId)
	: Page(engine), _puzzleId(puzzleId), _background(nullptr),
	  _zoombiniGfx(nullptr), _puzzleState(0), _stateTimer(0) {
	_pageId = puzzleId;
}

PuzzlePage::~PuzzlePage() {
	delete _background;
	delete _zoombiniGfx;
}

void PuzzlePage::init() {
	const char *name = getPuzzleName(_puzzleId);
	debug(1, "PuzzlePage::init — %s (page %d)", name, _puzzleId);

	// Load puzzle background using correct per-puzzle paths from IDA
	const char *bgName = nullptr;
	for (int i = 0; kPuzzleInfo[i].name; i++) {
		if (kPuzzleInfo[i].id == _puzzleId) {
			bgName = kPuzzleInfo[i].bgName;
			break;
		}
	}

	if (bgName) {
		Common::Path bgPath(Common::String::format("bmp/%s", bgName));
		_background = new BitBlock();
		if (!_background->load(bgPath)) {
			debug(1, "PuzzlePage: Failed to load background for %s", name);
			delete _background;
			_background = nullptr;
		}
	}

	// Load zoombini sprite graphics for display
	_zoombiniGfx = new ZoombiniGfx();
	if (!_zoombiniGfx->loadFromFile(Common::Path("bmp/zombis/littleZomb.anm"))) {
		debug(1, "PuzzlePage: Failed to load zoombini graphics");
		delete _zoombiniGfx;
		_zoombiniGfx = nullptr;
	}

	// Transfer zoombinis from global to puzzle
	_puzzleZoombinis.clear();
	for (uint i = 0; i < _engine->_globalZoombinis.size(); i++) {
		_puzzleZoombinis.push_back(_engine->_globalZoombinis[i]);
	}

	_puzzleState = 0;
	_stateTimer = _engine->getGameTickCount();
}

void PuzzlePage::update() {
	// Each puzzle type has its own update logic:
	//   Blocking puzzles (CrazyTurtle, Waterslide, etc.) run logic in init()
	//   Non-blocking (ChezNorf, WallOfFleens) use per-frame tick via update()
	// Stub: auto-advance after kAutoAdvanceDelay ms for testing flow progression
	uint32 elapsed = _engine->getGameTickCount() - _stateTimer;

	if (_puzzleState == 0) {
		if (elapsed > kAutoAdvanceDelay) {
			debug(1, "PuzzlePage: %s — auto-advance (stub)", getPuzzleName(_puzzleId));
			_puzzleState = 1;
			_engine->_returningFromPuzzle = true;
			_engine->_maptransSourceWorld = _puzzleId;
			_engine->requestPageChange(kPageMapTrans);
		}
	}
}

void PuzzlePage::draw(Graphics::ManagedSurface *screen) {
	if (_background) {
		_background->drawToSurface(screen, 0, 0);
	}

	// Draw zoombinis in a simple row (stub visualization)
	if (_zoombiniGfx && !_puzzleZoombinis.empty()) {
		const byte (*lut)[256] = _engine->getAlphaLUT();
		int numZoombinis = MIN((int)_puzzleZoombinis.size(), 16);
		int startX = 100;
		int startY = 500;
		int spacing = 40;

		for (int i = 0; i < numZoombinis; i++) {
			const Zoombini *z = _puzzleZoombinis[i];

			// Cell 0 = standing still, facing right
			int baseIdx = 0;

			// Body: [0][0][0]
			const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
			int x = startX + i * spacing;
			int y = startY;
			if (frame)
				frame->drawToScreen(screen, x, y, lut);

			// Features 1..4 (Hair, Eyes, Nose, Feet)
			const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
			for (int slot = 1; slot <= 4; slot++) {
				int featIdx = baseIdx + slot * ZoombiniGfx::kDim2 + features[slot - 1];
				frame = _zoombiniGfx->getFrame(featIdx, 0);
				if (frame)
					frame->drawToScreen(screen, x, y, lut);
			}
		}
	}

	// Per-puzzle scene elements, animations, and zoombini sprites
	// require individual puzzle class implementations.
}

void PuzzlePage::handleClick(const Common::Point &pos) {
	// Click to skip — advance puzzle immediately (stub for testing)
	if (_puzzleState == 0) {
		debug(1, "PuzzlePage: %s — click skip", getPuzzleName(_puzzleId));
		_puzzleState = 1;
		_engine->_returningFromPuzzle = true;
		_engine->_maptransSourceWorld = _puzzleId;
		_engine->requestPageChange(kPageMapTrans);
	}
}

} // End of namespace Zoombini2
