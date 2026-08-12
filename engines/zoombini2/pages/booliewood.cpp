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
#include "graphics/managed_surface.h"

#include "zoombini2/pages/booliewood.h"
#include "zoombini2/game_state.h"
#include "zoombini2/gfx.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// Theater seating layout — 23 seats arranged in staggered rows.
// Original uses 23 entries from unk_488A80 (28 bytes each), placed in a
// scrolling world with positions at unk_488F0C. Simplified to fixed screen
// positions in a theater-like arrangement.
// ============================================================================

// Seat positions (x, y) — 4 rows of decreasing count, theater style
static const int kSeatPositions[23][2] = {
	// Row 1 (front, 8 seats) — y=480
	{120, 480}, {170, 480}, {220, 480}, {270, 480},
	{340, 480}, {390, 480}, {440, 480}, {490, 480},
	// Row 2 (7 seats) — y=440
	{140, 440}, {190, 440}, {240, 440}, {310, 440},
	{360, 440}, {410, 440}, {460, 440},
	// Row 3 (5 seats) — y=400
	{160, 400}, {220, 400}, {290, 400}, {360, 400},
	{430, 400},
	// Row 4 (back, 3 seats) — y=360
	{210, 360}, {310, 360}, {410, 360},
};

// ============================================================================
// Constructor / Destructor
// ============================================================================

BooliewoodPage::BooliewoodPage(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageBooliewood),
	  _difficulty(1), _transitioning(false), _musicId(-1),
	  _overlay1(nullptr), _overlay2(nullptr) {
}

BooliewoodPage::~BooliewoodPage() {
	// Stop music
	if (_musicId >= 0) {
		SoundManager *snd = _engine->getSoundManager();
		if (snd) {
			snd->stop(_musicId);
			snd->unload(_musicId);
		}
	}

	delete _overlay1;
	delete _overlay2;
}

// ============================================================================
// Init
// ============================================================================

void BooliewoodPage::init() {
	PuzzlePage::init();
	debug(1, "BooliewoodPage::init");

	// Determine difficulty from engine gameMode
	// Original: this[8] = 4, then decrement based on global rescued count thresholds
	// (<300 -> 3, <200 -> 2, <100 -> 2, <50 -> 1)
	_difficulty = _engine->getGameState()->_gameMode;
	if (_difficulty < 1) _difficulty = 1;
	if (_difficulty > 4) _difficulty = 4;

	_transitioning = false;

	// BGM: Booliewood_Level1.wav (IDA: aSoundsMusicBoo at Booliewood__Init_40E600)
	SoundManager *snd = _engine->getSoundManager();
	if (snd) {
		_musicId = snd->load(true, Common::Path("sounds/music/Booliewood_Level1.wav"), true);
		if (_musicId >= 0) {
			snd->playLoop(_musicId);
			snd->setVolume(_musicId, snd->_volumeMusic);
		}
	}

	_stateTimer = _engine->getGameTickCount();
	_puzzleState = 0;
}

// ============================================================================
// Update
// ============================================================================

void BooliewoodPage::update() {
	if (_transitioning)
		return;

	uint32 elapsed = _engine->getGameTickCount() - _stateTimer;

	// Auto-advance after delay
	if (_puzzleState == 0 && elapsed > kAutoAdvanceMs) {
		debug(1, "BooliewoodPage: auto-advance");
		_transitioning = true;
		_engine->_returningFromPuzzle = true;
		_engine->_maptransSourceWorld = _pageId;
		_engine->requestPageChange(kPageMapTrans);
	}
}

// ============================================================================
// Draw
// ============================================================================

void BooliewoodPage::draw(Graphics::ManagedSurface *screen) {
	// Draw background
	if (_background) {
		_background->drawToSurface(screen, 0, 0);
	}

	// Draw zoombinis in theater seating arrangement
	if (!_zoombiniGfx || _puzzleZoombinis.empty())
		return;

	const byte (*lut)[256] = _engine->getAlphaLUT();
	int numToDraw = MIN((int)_puzzleZoombinis.size(), kNumSeats);

	for (int i = 0; i < numToDraw; i++) {
		const Zoombini *z = _puzzleZoombinis[i];
		int x = kSeatPositions[i][0];
		int y = kSeatPositions[i][1];

		// Body (standing idle, cell index 0)
		int baseIdx = 0;
		const RleBlock *frame = _zoombiniGfx->getFrame(baseIdx, 0);
		if (frame)
			frame->drawToScreen(screen, x, y, lut);

		// Features (Hair, Eyes, Nose, Feet)
		const byte features[4] = { z->_featureA, z->_featureB, z->_featureC, z->_featureD };
		for (int slot = 1; slot <= 4; slot++) {
			int featIdx = baseIdx + slot * ZoombiniGfx::kDim2 + features[slot - 1];
			frame = _zoombiniGfx->getFrame(featIdx, 0);
			if (frame)
				frame->drawToScreen(screen, x, y, lut);
		}
	}
}

// ============================================================================
// HandleClick
// ============================================================================

void BooliewoodPage::handleClick(const Common::Point &pos) {
	if (_transitioning)
		return;

	// Click anywhere to advance (skip the hub scene)
	if (_puzzleState == 0) {
		debug(1, "BooliewoodPage: click to advance");
		_transitioning = true;
		_engine->_returningFromPuzzle = true;
		_engine->_maptransSourceWorld = _pageId;
		_engine->requestPageChange(kPageMapTrans);
	}
}

} // End of namespace Zoombini2
