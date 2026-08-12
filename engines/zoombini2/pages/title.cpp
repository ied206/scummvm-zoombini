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

#include "zoombini2/pages/title.h"
#include "zoombini2/gfx.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

TitleScreen::TitleScreen(Zoombini2Engine *engine)
	: Page(engine), _background(nullptr), _clicked(false), _deadline(0), _musicId(-1) {
	_pageId = kPageTitleScreen;
}

TitleScreen::~TitleScreen() {
	// Unload music
	SoundManager *sound = _engine->getSoundManager();
	if (sound && _musicId >= 0) {
		sound->unload(_musicId);
	}

	delete _background;
}

void TitleScreen::init() {
	debug(1, "TitleScreen::init");
	_deadline = _engine->getGameTickCount() + 10000;

	// Load the static title background.
	_background = new BitBlock();
	if (!_background->load(Common::Path("bmp/story_intro/title_screen"))) {
		warning("TitleScreen: Failed to load title background");
	}

	// Play the title music until the page is dismissed.
	SoundManager *sound = _engine->getSoundManager();
	if (sound) {
		_musicId = sound->load(true, Common::Path("sounds/music/Booliewood_Level1.wav"), true);
		if (_musicId >= 0) {
			sound->playLoop(_musicId);
			sound->setVolume(_musicId, sound->_volumeMusic);
		}
	}
}

void TitleScreen::update() {
	if (_clicked)
		return;

	if (_deadline < _engine->getGameTickCount()
	    || _engine->isMouseClicked() || _engine->getLastKeyPressed()) {
		_clicked = true;
		_engine->requestPageChange(kPageMenuOptions);
	}
}

void TitleScreen::draw(Graphics::ManagedSurface *screen) {
	if (_background) {
		_background->drawToSurface(screen, 0, 0);
	}
}

void TitleScreen::handleClick(const Common::Point &pos) {
	(void)pos;
	_clicked = true;
	_engine->requestPageChange(kPageMenuOptions);
}

} // End of namespace Zoombini2
