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

	switch (cmd._type) {
	case DialogDebugCommand::Type::kDrawAreaMask: {
		PageBase *page = _vm->getCurrentPage();
		if (!page)
			return false;

		_vm->_gfx->captureScreen(_savedScreen);
		_vm->_gfx->maskRejectedArea(_savedScreen, page->getAreaMask());

		if (page->hasAreaMask())
			_titleText = Common::String::format("[AreaMask] page(%d)", static_cast<int>(page->getPageId()));
		else
			_titleText = Common::String::format("[AreaMask] page(%d) no area mask", static_cast<int>(page->getPageId()));
		break;
	}
	case DialogDebugCommand::Type::kDrawAnimation: {
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
		break;
	}
	case DialogDebugCommand::Type::kPlotPoint:
	case DialogDebugCommand::Type::kPlotLine:
	case DialogDebugCommand::Type::kPlotRect:
		preparePlot(cmd);
		break;
	default:
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

void DialogDebug::preparePlot(const DialogDebugCommand &cmd) {
	const uint32 white = _savedScreen->format.RGBToColor(255, 255, 255);
	_vm->_gfx->fillRect(_savedScreen, Common::Rect32(ManagedSurface32::kScreenSize.width, ManagedSurface32::kScreenSize.height), white);
	const RGBColor rgb(static_cast<byte>(cmd._plotColor >> 16), static_cast<byte>(cmd._plotColor >> 8), static_cast<byte>(cmd._plotColor));
	const uint32 color = rgb.toPixel(_savedScreen->format);
	const Common::Point32 &start = cmd._plotStart;
	const Common::Point32 &end = cmd._plotEnd;

	switch (cmd._type) {
	case DialogDebugCommand::Type::kPlotPoint:
		_vm->_gfx->fillRect(_savedScreen, Common::Rect32(start.x, start.y, start.x + 1, start.y + 1), color);
		_titleText = Common::String::format("[Plot Point] at (%d, %d) color(0x%06X)", start.x, start.y, cmd._plotColor);
		break;
	case DialogDebugCommand::Type::kPlotLine:
		_vm->_gfx->drawLine(_savedScreen, start, end, color);
		_titleText = Common::String::format("[Plot Line] from (%d, %d) to (%d, %d) color(0x%06X)", start.x, start.y, end.x, end.y, cmd._plotColor);
		break;
	case DialogDebugCommand::Type::kPlotRect:
	default:
		_vm->_gfx->drawLine(_savedScreen, start, Common::Point32(end.x - 1, start.y), color);
		_vm->_gfx->drawLine(_savedScreen, Common::Point32(start.x, end.y - 1), Common::Point32(end.x - 1, end.y - 1), color);
		_vm->_gfx->drawLine(_savedScreen, start, Common::Point32(start.x, end.y - 1), color);
		_vm->_gfx->drawLine(_savedScreen, Common::Point32(end.x - 1, start.y), Common::Point32(end.x - 1, end.y - 1), color);
		_titleText = Common::String::format("[Plot Rect] from (%d, %d) to (%d, %d) color(0x%06X)", start.x, start.y, end.x, end.y, cmd._plotColor);
		break;
	}
}

void DialogDebug::drawTitleText(ManagedSurface32 *screen) const {
	if (_titleText.empty() || !_vm->_gfx->hasTextFont(Gfx::TextColor::kWhite03))
		return;
	_vm->_gfx->fillRect(screen, Common::Rect32(0, 0, ManagedSurface32::kScreenSize.width, kTitleHeight), 0);
	_vm->_gfx->drawText(screen, Gfx::TextColor::kWhite03, Common::Point32(kTitleX, kTitleY), _titleText);
}

void DialogDebug::drawEscText(ManagedSurface32 *screen, const Common::String &keyLegend) const {
	static constexpr const char *kEscText = "[ESC] close";
	if (!_vm->_gfx->hasTextFont(Gfx::TextColor::kWhite03))
		return;
	Common::String text;
	if (keyLegend.empty()) {
		text = kEscText;
	} else {
		text = keyLegend;
		if (keyLegend.lastChar() != ' ')
			text += ' ';
		text += kEscText;
	}
	const int escWidth = _vm->_gfx->getTextWidth(text, Gfx::TextColor::kWhite03);
	_vm->_gfx->drawText(screen, Gfx::TextColor::kWhite03,
						Common::Point32(ManagedSurface32::kScreenSize.width - escWidth - kTitleX, kTitleY), text);
}

void DialogDebug::onRenderContent(ManagedSurface32 *screen) {
	if (!_isActive)
		return;

	switch (_viewType) {
	case DialogDebugCommand::Type::kDrawAnimation: {
		const uint32 white = screen->format.ARGBToColor(255, 255, 255, 255);
		_vm->_gfx->fillRect(screen, Common::Rect32(ManagedSurface32::kScreenSize.width, ManagedSurface32::kScreenSize.height), white);
		if (_singleFrameSprite)
			_vm->_gfx->drawPageRleBlock(screen, _animPath.toString('/'), Common::Point32(0, 0));
		else if (_animation && 0 <= _frameIndex && _frameIndex < _animation->getFrameCount())
			_vm->_gfx->drawRleBlock(screen, _animation->getFrame(_frameIndex), Common::Point32(0, 0));
		break;
	}
	default:
		screen->copyFrom(*_savedScreen);
		break;
	}

	drawTitleText(screen);
	if (_viewType == DialogDebugCommand::Type::kDrawAnimation)
		drawEscText(screen, "[<-/->] frame");
	else
		drawEscText(screen);
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
