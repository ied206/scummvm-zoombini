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
#include "common/rect.h"

#include "zoombini2/pages/credits.h"
#include "zoombini2/gfx.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// CreditsPage — end credits (page ID 16).
// Original: Credits__Init_417E60 (0x417E60), object size 0x20 (32 bytes).
//
// The credits are a single tall bitmap (bmp/credits/credits.bmp) that
// scrolls upward at 30 pixels per second.  The FrameHandler controls
// three phases:
//   1. Initial wait  — 4 000 ms before scrolling begins.
//   2. Active scroll — scrollY advances at 30 px/sec via float delta time.
//   3. End hold      — 10 000 ms after reaching the bottom, then done.
// Any left-click or skip signal exits immediately.
//
// Key globals in the original (not reproduced here):
//   dword_4A876C  — BGM sound handle (here: _musicId)
//   dword_571D64  — global skip flag (here: engine click/key event)
//   byte_572A63   — transition-to-title flag (here: _finished flag)
// ============================================================================

CreditsPage::CreditsPage(Zoombini2Engine *engine, bool quitAfterCredits)
	: Page(engine),
	  _endTime(0),
	  _scrollY(0.0f),
	  _maxScrollY(0),
	  _scrollActive(false),
	  _redrawNeeded(false),
	  _lastUpdateTime(0),
	  _initialWait(false),
	  _background(nullptr),
	  _musicId(-1),
	  _finished(false),
	  _quitAfterCredits(quitAfterCredits) {
	_pageId = kPageCredits;
}

CreditsPage::~CreditsPage() {
	// Credits__Destructor_417E20: unload BGM, free bitblock.
	if (_musicId >= 0) {
		SoundManager *sm = _engine->getSoundManager();
		sm->stop(_musicId);
		sm->unload(_musicId);
	}
	delete _background;
}

void CreditsPage::init() {
	debug(1, "CreditsPage::init");

	// Credits__Init_417E60 (0x417E60):
	// Load credits bitmap — tall image scrolled vertically.
	// Original: Scene__LoadBackground_457BC0(1, "#bmp\credits\credits.bmp")
	_background = new BitBlock();
	if (!_background->load(Common::Path("bmp/credits/credits"))) {
		warning("CreditsPage: Failed to load bmp/credits/credits");
	}

	// maxScrollY = bitmap_height - 600 (screen height).
	// Original: *(this + 3) = *(_DWORD *)(*(_DWORD *)(**(_DWORD **)(v6 + 4) + 40) + 16) - 600
	_maxScrollY = (_background->getHeight() > 600) ? (_background->getHeight() - 600) : 0;

	// Reset scroll state.
	_scrollY = 0.0f;
	_scrollActive = true;
	_redrawNeeded = true;
	_initialWait = true;

	// Initial 4-second pause before scrolling begins.
	// Original: *this = Game__GetTime_469040() + 4000
	_endTime = _engine->getGameTickCount() + 4000;
	_lastUpdateTime = _engine->getGameTickCount();

	// BGM: ZMR-Transition.wav looped.
	// Original: CSaianSound__LoadWrapper(1, "#sounds\music\ZMR-Transition.wav", 1)
	//           CSaianSoundBuffer__PlayLoopVol(g_volumeMusic)
	SoundManager *sm = _engine->getSoundManager();
	_musicId = sm->load(true, Common::Path("sounds/music/ZMR-Transition.wav"), true);
	if (_musicId >= 0) {
		sm->playLoop(_musicId);
		sm->setVolume(_musicId, sm->_volumeMusic);
	}

	_finished = false;
}

void CreditsPage::update() {
	// Credits__FrameHandler_417D30 (0x417D30):
	// Drives the three-phase scroll sequence and detects exit conditions.

	uint32 now = _engine->getGameTickCount();

	if (_scrollActive) {
		if (_initialWait) {
			// Phase 1: hold for 4 seconds.
			// Original: if (GetTime() > this[0]) this[24] = 0
			if (now > _endTime)
				_initialWait = false;
		} else {
			// Phase 2: advance scroll position.
			// Original: scrollY += (v7 - v8) * 30.0 * 0.001
			_redrawNeeded = true;
			_scrollY += static_cast<float>(now - _lastUpdateTime) * 30.0f * 0.001f;
		}

		// Clamp and transition to end-hold phase.
		// Original: if (this[3] < this[2]) { clamp; this[16]=0; this[0]=GetTime+10000 }
		if (_scrollY >= static_cast<float>(_maxScrollY)) {
			_scrollY = static_cast<float>(_maxScrollY);
			_scrollActive = false;
			_endTime = now + 10000;
		}

		// Always update delta-time base while scroll phase is active.
		// Original: *(this + 5) = GetTime()   (unconditional inside if-scrollActive)
		_lastUpdateTime = now;
	} else {
		// Phase 3: wait 10 seconds at the bottom, then finish.
		// Original: v6 = (GetTime() > this[0])
		if (now > _endTime)
			_finished = true;
	}

	if (_finished) {
		if (_quitAfterCredits)
			Engine::quitGame();
		else
			_engine->requestPageChange(kPageTitleScreen);
	}
}

void CreditsPage::draw(Graphics::ManagedSurface *screen) {
	// Credits__FrameHandler_417D30 draw section:
	// if (this[17]) { Scene__DrawBgRegion(…, 0, (int64)scrollY); this[17]=0 }
	//
	// Uses double buffering like the original: only redraws when _redrawNeeded
	// is set. The frame buffer persists between frames via needsScreenClear().
	if (!_redrawNeeded)
		return;
	_redrawNeeded = false;

	if (!_background)
		return;

	int y = static_cast<int>(_scrollY);
	int bmpW = _background->getWidth();
	int bmpH = _background->getHeight();
	int srcBottom = y + 600;
	if (srcBottom > bmpH)
		srcBottom = bmpH;

	// Draw the visible 800×600 window starting at (0, scrollY).
	_background->drawSubRect(screen, 0, 0, Common::Rect(0, y, bmpW, srcBottom));
}

void CreditsPage::handleClick(const Common::Point &pos) {
	// Any click skips credits immediately.
	// Original: if (*a5 == 1 || dword_571D64 != -1) return 1
	if (!_finished) {
		_finished = true;
		if (_quitAfterCredits)
			Engine::quitGame();
		else
			_engine->requestPageChange(kPageTitleScreen);
	}
}

} // End of namespace Zoombini2
