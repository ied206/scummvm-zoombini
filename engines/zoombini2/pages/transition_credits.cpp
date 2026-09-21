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

constexpr const char *TransitionCredits::kBackgroundPath;
constexpr const char *TransitionCredits::kMusicPath;
constexpr const char *TransitionCredits::kDemoBackgroundPath;
constexpr const char *TransitionCredits::kDemoMusicPath;

TransitionCredits::TransitionCredits(Zoombini2Engine *vm)
	: TransitionBase(vm) {
	_pageId = kPageCredits;
}

void TransitionCredits::init() {
	debug(1, "TransitionCredits::init");
	static constexpr uint32 kInitialHoldMilliseconds = 4000;

	const char *backgroundPath = _vm->isDemo() ? kDemoBackgroundPath : kBackgroundPath;
	const Size32 backgroundSize = _vm->_gfx->getPageBitBlockSize(backgroundPath);
	if (!_vm->isDemo() && (backgroundSize.width == 0 || backgroundSize.height == 0)) {
		warning("TransitionCredits: Failed to load credits background");
	}

	_maxScrollY = 0;
	if (ManagedSurface32::kScreenSize.height < backgroundSize.height)
		_maxScrollY = backgroundSize.height - ManagedSurface32::kScreenSize.height;

	_scrollY = 0.0f;
	_scrollActive = true;
	_redrawNeeded = true;
	_initialWait = true;

	_endTime = _vm->getGameTickCount() + kInitialHoldMilliseconds;
	_lastUpdateTime = _vm->getGameTickCount();

	startPageMusic(Common::Path(_vm->isDemo() ? kDemoMusicPath : kMusicPath));

	_finished = false;
}

void TransitionCredits::onUpdate() {
	if (_vm->isDemo())
		return;
	static constexpr float kScrollPixelsPerMillisecond = 0.03f;
	static constexpr uint32 kEndHoldMilliseconds = 10000;

	uint32 now = _vm->getGameTickCount();

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

void TransitionCredits::onRenderContent(ManagedSurface32 *screen) {
	if (!_redrawNeeded)
		return;
	_redrawNeeded = false;

	const char *backgroundPath = _vm->isDemo() ? kDemoBackgroundPath : kBackgroundPath;
	const Size32 backgroundSize = _vm->_gfx->getPageBitBlockSize(backgroundPath);
	if (backgroundSize.width == 0 || backgroundSize.height == 0)
		return;

	const int y = static_cast<int>(_scrollY);
	int srcBottom = y + ManagedSurface32::kScreenSize.height;
	if (backgroundSize.height < srcBottom)
		srcBottom = backgroundSize.height;

	_vm->_gfx->drawPageBitBlockSubRect(screen, backgroundPath, Common::Point32(0, 0), Common::Rect(0, y, backgroundSize.width, srcBottom));
}

EventHandleResult TransitionCredits::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	if (_vm->isDemo())
		return EventHandleResult::kConsumed;
	if (!_finished) {
		_finished = true;
		Engine::quitGame();
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

EventHandleResult TransitionCredits::onLButtonUp(const Common::Point &pos) {
	(void)pos;
	if (!_vm->isDemo())
		return EventHandleResult::kPassthrough;
	_finished = true;
	Engine::quitGame();
	return EventHandleResult::kConsumed;
}

} // End of namespace Zoombini2
