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

#include "zoombini2/graphics.h"
#include "zoombini2/pages/transition_credits.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

CreditsPage::CreditsPage(Zoombini2Engine *engine)
	: TransitionPage(engine),
	  _endTime(0),
	  _scrollY(0.0f),
	  _maxScrollY(0),
	  _scrollActive(false),
	  _redrawNeeded(false),
	  _lastUpdateTime(0),
	  _initialWait(false),
	  _background(nullptr),
	  _musicId(-1),
	  _finished(false) {
	_pageId = kPageCredits;
}

CreditsPage::~CreditsPage() {
	if (_musicId >= 0) {
		SoundManager *sm = _engine->getSoundManager();
		sm->stop(_musicId);
		sm->unload(_musicId);
	}
	delete _background;
}

void CreditsPage::init() {
	debug(1, "CreditsPage::init");
	static constexpr uint32 kInitialHoldMilliseconds = 4000;

	_background = new BitBlock();
	if (!_background->load(Common::Path("bmp/credits/credits"))) {
		warning("CreditsPage: Failed to load bmp/credits/credits");
	}

	_maxScrollY = (600 < _background->getHeight()) ? (_background->getHeight() - 600) : 0;

	_scrollY = 0.0f;
	_scrollActive = true;
	_redrawNeeded = true;
	_initialWait = true;

	_endTime = _engine->getGameTickCount() + kInitialHoldMilliseconds;
	_lastUpdateTime = _engine->getGameTickCount();

	SoundManager *sm = _engine->getSoundManager();
	_musicId = sm->load(true, Common::Path("sounds/music/ZMR-Transition.wav"), true);
	if (_musicId >= 0) {
		sm->playLoop(_musicId);
		sm->setVolume(_musicId, sm->_volumeMusic);
	}

	_finished = false;
}

void CreditsPage::update() {
	static constexpr float kScrollPixelsPerMillisecond = 0.03f;
	static constexpr uint32 kEndHoldMilliseconds = 10000;

	uint32 now = _engine->getGameTickCount();

	if (_scrollActive) {
		if (_initialWait) {
			if (now > _endTime)
				_initialWait = false;
		} else {
			_redrawNeeded = true;
			_scrollY += static_cast<float>(now - _lastUpdateTime) * kScrollPixelsPerMillisecond;
		}

		if (static_cast<float>(_maxScrollY) < _scrollY) {
			_scrollY = static_cast<float>(_maxScrollY);
			_scrollActive = false;
			_endTime = now + kEndHoldMilliseconds;
		}

		_lastUpdateTime = now;
	} else {
		if (now > _endTime)
			_finished = true;
	}

	if (_finished)
		Engine::quitGame();
}

void CreditsPage::draw(Graphics::ManagedSurface *screen) {
	if (!_redrawNeeded)
		return;
	_redrawNeeded = false;

	if (!_background)
		return;

	const int y = static_cast<int>(_scrollY);
	const int bmpW = _background->getWidth();
	const int bmpH = _background->getHeight();
	int srcBottom = y + 600;
	if (bmpH < srcBottom)
		srcBottom = bmpH;

	_background->drawSubRect(screen, 0, 0, Common::Rect(0, y, bmpW, srcBottom));
}

void CreditsPage::handleClick(const Common::Point &pos) {
	(void)pos;
	if (!_finished) {
		_finished = true;
		Engine::quitGame();
	}
}

} // End of namespace Zoombini2
