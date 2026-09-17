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

constexpr const char *DialogDebug::kTitleFontPath;

DialogDebug::DialogDebug(Zoombini2Engine *vm)
	: DialogBase(vm) {
	_savedScreen = _vm->_gfx->createSurface(ManagedSurface32::kScreenSize);

	_titleFont = new BitmapFont(_vm);
	if (!_titleFont->load(Common::Path(kTitleFontPath), 255, 255, 255))
		warning("DialogDebug: Failed to load title font");
}

DialogDebug::~DialogDebug() {
	close();

	delete _savedScreen;
	delete _titleFont;
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
			_titleText = Common::String::format("[AreaMask] page(%d)", page->getPageId());
		else
			_titleText = Common::String::format("[AreaMask] page(%d) no area mask", page->getPageId());
	} else if (cmd._type == DialogDebugCommand::Type::kDrawAnimation) {
		if (cmd._animPath.toString().hasSuffixIgnoreCase(".rb")) {
			_sprite = new RleBlock(_vm);
			if (!_sprite->loadFromFile(cmd._animPath)) {
				freeAnimation();
				return false;
			}
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

	_pauseStartTime = g_system->getMillis();
	_vm->_isPaused = true;
	_vm->_pauseTimeStart = _pauseStartTime;
	if (_vm->getSoundManager())
		_vm->getSoundManager()->pauseAll();

	_isActive = true;
	return true;
}

void DialogDebug::close() {
	if (!_isActive)
		return;

	_isActive = false;
	_viewType = DialogDebugCommand::Type::kNone;
	freeAnimation();
	_vm->addPauseTime(g_system->getMillis() - _pauseStartTime);
	_vm->_isPaused = false;
	if (_vm->getSoundManager())
		_vm->getSoundManager()->resumeAll();
}

void DialogDebug::freeAnimation() {
	delete _animation;
	_animation = nullptr;
	delete _sprite;
	_sprite = nullptr;
	_frameIndex = 0;
}

int DialogDebug::getFrameCount() const {
	if (_sprite)
		return 1;
	if (_animation)
		return _animation->getFrameCount();
	return 0;
}

const RleBlock *DialogDebug::getCurrentFrame() const {
	if (_sprite)
		return _sprite->isValid() ? _sprite : nullptr;
	if (_animation && 0 <= _frameIndex && _frameIndex < _animation->getFrameCount())
		return _animation->getFrame(_frameIndex);
	return nullptr;
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
		_vm->_gfx->drawRleBlock(screen, getCurrentFrame(), Common::Point32(0, 0));
	} else {
		screen->copyFrom(*_savedScreen);
	}

	if (_titleFont && _titleFont->isLoaded() && !_titleText.empty()) {
		static constexpr int kTitleHeight = 22;
		_vm->_gfx->fillRect(screen, Common::Rect32(0, 0, ManagedSurface32::kScreenSize.width, kTitleHeight), 0);
		_titleFont->drawString(screen, Common::Point32(8, 3), _titleText, _vm->getAlphaLUT());
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
