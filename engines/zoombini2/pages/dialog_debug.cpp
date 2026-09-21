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

#include "zoombini2/pages/dialog_debug.h"
#include "zoombini2/graphics.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

#include "common/debug.h"
#include "common/system.h"

namespace Zoombini2 {

DialogDebug::DialogDebug(Zoombini2Engine *vm)
	: DialogBase(vm) {
	_savedScreen = _vm->_gfx->createSurface(ManagedSurface32::kScreenSize);

	if (!_vm->_gfx->loadTextFont(Gfx::TextColor::kWhite03))
		warning("DialogDebug: Failed to load title font");
}

DialogDebug::~DialogDebug() {
	close();

	delete _savedScreen;
}

bool DialogDebug::open(const DialogDebugCommand &cmd) {
	if (_isActive)
		return false;

	freeAnimation();
	_viewType = DialogDebugCommand::Type::kNone;
	_titleText.clear();

	if (cmd._type == DialogDebugCommand::Type::kDrawAreaMask) {
		PageBase *page = _vm->getCurrentPage();
		if (!page)
			return false;

		_vm->_gfx->captureScreen(_savedScreen);
		_vm->_gfx->maskRejectedArea(_savedScreen, page->getAreaMask());

		if (page->hasAreaMask())
			_titleText = Common::String::format("[AreaMask] page(%d)", static_cast<int>(page->getPageId()));
		else
			_titleText = Common::String::format("[AreaMask] page(%d) no area mask", static_cast<int>(page->getPageId()));
	} else if (cmd._type == DialogDebugCommand::Type::kDrawAnimation) {
		if (cmd._animPath.toString().hasSuffixIgnoreCase(".rb")) {
			if (!_vm->_gfx->loadPageRleBlock(cmd._animPath.toString('/'))) {
				freeAnimation();
				return false;
			}
			_singleFrameSprite = true;
		} else {
			_animation = new Animation(_vm);
			if (!_animation->loadFromFile(cmd._animPath)) {
				freeAnimation();
				return false;
			}
		}

		if (getFrameCount() <= cmd._startFrame) {
			freeAnimation();
			return false;
		}
		_frameIndex = cmd._startFrame;
		_animPath = cmd._animPath;
		updateAnimationTitle();
	} else {
		return false;
	}

	_viewType = cmd._type;

	_vm->setDialogPaused(true);

	_isActive = true;
	return true;
}

void DialogDebug::close() {
	if (!_isActive)
		return;

	_isActive = false;
	_viewType = DialogDebugCommand::Type::kNone;
	freeAnimation();
	_vm->setDialogPaused(false);
}

void DialogDebug::freeAnimation() {
	delete _animation;
	_animation = nullptr;
	_singleFrameSprite = false;
	_frameIndex = 0;
}

int DialogDebug::getFrameCount() const {
	if (_singleFrameSprite)
		return 1;
	if (_animation)
		return _animation->getFrameCount();
	return 0;
}

void DialogDebug::updateAnimationTitle() {
	_titleText = Common::String::format("[Animation] %s frame(%d/%d)", _animPath.toString().c_str(), _frameIndex, getFrameCount());
}

void DialogDebug::onRenderContent(ManagedSurface32 *screen) {
	if (!_isActive)
		return;

	if (_viewType == DialogDebugCommand::Type::kDrawAnimation) {
		const uint32 white = screen->format.ARGBToColor(255, 255, 255, 255);
		_vm->_gfx->fillRect(screen, Common::Rect32(ManagedSurface32::kScreenSize.width, ManagedSurface32::kScreenSize.height), white);
		if (_singleFrameSprite)
			_vm->_gfx->drawPageRleBlock(screen, _animPath.toString('/'), Common::Point32(0, 0));
		else if (_animation && 0 <= _frameIndex && _frameIndex < _animation->getFrameCount())
			_vm->_gfx->drawRleBlock(screen, _animation->getFrame(_frameIndex), Common::Point32(0, 0));
	} else {
		screen->copyFrom(*_savedScreen);
	}

	if (_vm->_gfx->hasTextFont(Gfx::TextColor::kWhite03) && !_titleText.empty()) {
		static constexpr int kTitleHeight = 22;
		_vm->_gfx->fillRect(screen, Common::Rect32(0, 0, ManagedSurface32::kScreenSize.width, kTitleHeight), 0);
		_vm->_gfx->drawText(screen, Gfx::TextColor::kWhite03, Common::Point32(8, 3), _titleText);
	}
}

EventHandleResult DialogDebug::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	if (!_isActive)
		return EventHandleResult::kPassthrough;
	close();
	return EventHandleResult::kConsumed;
}

EventHandleResult DialogDebug::onMouseMove(const Common::Point &pos) {
	(void)pos;
	return _isActive ? EventHandleResult::kConsumed : EventHandleResult::kPassthrough;
}

EventHandleResult DialogDebug::onKeyDown(const Common::KeyState &key, bool repeat) {
	(void)repeat;
	if (!isActive())
		return EventHandleResult::kPassthrough;
	if (key.keycode == Common::KEYCODE_ESCAPE) {
		close();
		return EventHandleResult::kConsumed;
	}
	if (_viewType == DialogDebugCommand::Type::kDrawAnimation) {
		if (key.keycode == Common::KEYCODE_LEFT && 0 < _frameIndex) {
			_frameIndex -= 1;
			updateAnimationTitle();
		} else if (key.keycode == Common::KEYCODE_RIGHT && _frameIndex + 1 < getFrameCount()) {
			_frameIndex += 1;
			updateAnimationTitle();
		}
	}
	return EventHandleResult::kConsumed;
}

} // End of namespace Zoombini2
