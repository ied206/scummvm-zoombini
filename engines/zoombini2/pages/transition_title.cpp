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

constexpr const char *TransitionTitle::kBackgroundPath;
constexpr const char *TransitionTitle::kMusicPath;
constexpr const char *TransitionTitle::kDemoMusicPath;

TransitionTitle::TransitionTitle(Zoombini2Engine *vm)
	: TransitionBase(vm) {
	_pageId = kPageTitleScreen;
}

void TransitionTitle::init() {
	debug(1, "TitleScreen::init");
	_deadline = _vm->getGameTickCount() + 10000;

	// Load the static title background.
	if (!_vm->_gfx->loadPageBitBlock(kBackgroundPath)) {
		warning("TitleScreen: Failed to load title background");
	}

	// Play the title music until the page is dismissed.
	startPageMusic(Common::Path(_vm->isDemo() ? kDemoMusicPath : kMusicPath));
}

void TransitionTitle::onUpdate() {
	if (_clicked)
		return;

	if (_deadline < _vm->getGameTickCount())
		dismiss();
}

void TransitionTitle::onRenderContent(ManagedSurface32 *screen) {
	_vm->_gfx->drawPageBitBlock(screen, kBackgroundPath, Common::Point32(0, 0));
}

EventHandleResult TransitionTitle::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	if (_vm->isDemo())
		return EventHandleResult::kConsumed;
	return dismiss();
}

EventHandleResult TransitionTitle::onLButtonUp(const Common::Point &pos) {
	(void)pos;
	if (_vm->isDemo())
		return dismiss();
	return EventHandleResult::kPassthrough;
}

EventHandleResult TransitionTitle::onKeyDown(const Common::KeyState &key, bool repeat) {
	(void)key;
	(void)repeat;
	return dismiss();
}

EventHandleResult TransitionTitle::dismiss() {
	_clicked = true;
	_vm->requestPageChange(_vm->isDemo() ? kPageWaterslide : kPageMenuOptions);
	return EventHandleResult::kConsumed;
}

} // End of namespace Zoombini2
