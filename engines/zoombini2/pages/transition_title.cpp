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

#include "zoombini2/graphics.h"
#include "zoombini2/pages/transition_title.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

TransitionTitle::TransitionTitle(Zoombini2Engine *vm)
	: TransitionBase(vm) {
	_pageId = kPageTitleScreen;
}

TransitionTitle::~TransitionTitle() {
	// Unload music
	SoundManager *sound = _vm->getSoundManager();
	if (sound && _musicId >= 0) {
		sound->unload(_musicId);
	}

	delete _background;
}

void TransitionTitle::init() {
	debug(1, "TitleScreen::init");
	_deadline = _vm->getGameTickCount() + 10000;

	// Load the static title background.
	_background = new BitBlock(_vm);
	if (!_background->load(Common::Path("bmp/story_intro/title_screen"))) {
		warning("TitleScreen: Failed to load title background");
	}

	// Play the title music until the page is dismissed.
	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		_musicId = sound->load(true, Common::Path("#sounds/music/Booliewood_Level1.wav"), true);
		if (_musicId >= 0) {
			sound->playLoop(_musicId);
			sound->setVolume(_musicId, sound->_volumeMusic);
		}
	}
}

void TransitionTitle::onUpdate() {
	if (_clicked)
		return;

	if (_deadline < _vm->getGameTickCount())
		dismiss();
}

void TransitionTitle::onRenderScene(ManagedSurface32 *screen) {
	if (_background) {
		_background->drawToSurface(screen, Common::Point32(0, 0));
	}
}

EventHandleResult TransitionTitle::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	return dismiss();
}

EventHandleResult TransitionTitle::onKeyDown(const Common::KeyState &key, bool repeat) {
	(void)key;
	(void)repeat;
	return dismiss();
}

EventHandleResult TransitionTitle::dismiss() {
	_clicked = true;
	_vm->requestPageChange(kPageMenuOptions);
	return EventHandleResult::kConsumed;
}

} // End of namespace Zoombini2
