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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "zoombini2/pages/dialog_msgbox.h"
#include "common/debug.h"
#include "common/system.h"
#include "zoombini2/graphics.h"
#include "zoombini2/sound.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *DialogMsgBox::kPanelPaths[3];

DialogMsgBox::DialogMsgBox(Zoombini2Engine *vm)
	: DialogBase(vm) {
}

DialogMsgBox::~DialogMsgBox() {
	close();
}

bool DialogMsgBox::request(const Common::Path &textPath, Common::BaseCallback<DialogMsgBoxButton> *callback, const Common::Point32 &position,
						   const Common::Point32 &textOffset) {
	if (isActive()) {
		delete callback;
		return false;
	}

	_textPath = textPath;
	_position = position;
	_textOffset = textOffset;
	_callback = callback;
	_hoveredButton = DialogMsgBoxButton::kNone00;
	_redrawNeeded = true;
	_state = DialogMsgBoxState::kPendingOpen01;
	return true;
}

bool DialogMsgBox::openDialog() {
	if (_state != DialogMsgBoxState::kPendingOpen01)
		return _state == DialogMsgBoxState::kOpen02;

	for (int i = 0; i < 3; i++)
		_panels[i] = _vm->loadRleBlock(kPanelPaths[i]);
	_textImage = _vm->loadBitBlock(_textPath.toString());

	if (!_panels[0] || !_panels[1] || !_panels[2] || !_textImage) {
		warning("DialogMsgBox: Failed to load confirmation resources for '%s'", _textPath.toString().c_str());
		close();
		return false;
	}

	const Size32 panelSize = _panels[1]->getSize();
	if (_position.x == -1)
		_position.x = ManagedSurface32::kScreenSize.width / 2 - panelSize.width / 2;
	if (_position.y == -1)
		_position.y = ManagedSurface32::kScreenSize.height / 2 - panelSize.height / 2;

	_savedBackground = _vm->_gfx->createSurface(panelSize);
	_vm->_gfx->captureScreenRegion(_savedBackground,
								   Common::Rect(_position.x, _position.y, _position.x + panelSize.width, _position.y + panelSize.height));

	_pauseStartTime = g_system->getMillis();
	_vm->_isPaused = true;
	_vm->_pauseTimeStart = _pauseStartTime;
	if (_vm->getSoundManager())
		_vm->getSoundManager()->pauseAll();
	_state = DialogMsgBoxState::kOpen02;
	return true;
}

void DialogMsgBox::close() {
	if (_state == DialogMsgBoxState::kOpen02) {
		if (_savedBackground && _vm->getCurrentScreen()) {
			_vm->_gfx->copyRegionToScreen(*_savedBackground, Common::Point(_position.x, _position.y));
		}
		_vm->addPauseTime(g_system->getMillis() - _pauseStartTime);
		_vm->_isPaused = false;
		if (_vm->getSoundManager())
			_vm->getSoundManager()->resumeAll();
	}

	_state = DialogMsgBoxState::kClosed00;
	_hoveredButton = DialogMsgBoxButton::kNone00;
	_redrawNeeded = false;
	delete _callback;
	_callback = nullptr;
	releaseResources();
}

void DialogMsgBox::releaseResources() {
	for (int i = 0; i < 3; i++) {
		delete _panels[i];
		_panels[i] = nullptr;
	}
	delete _textImage;
	_textImage = nullptr;
	delete _savedBackground;
	_savedBackground = nullptr;
}

DialogMsgBoxButton DialogMsgBox::hitTest(const Common::Point &pos) const {
	const int relativeX = pos.x - _position.x;
	const int relativeY = pos.y - _position.y;
	if (77 < relativeY && relativeY < 145) {
		if (207 < relativeX && relativeX < 272)
			return DialogMsgBoxButton::kOkay01;
		if (287 < relativeX && relativeX < 352)
			return DialogMsgBoxButton::kCancel02;
	}
	return DialogMsgBoxButton::kNone00;
}

void DialogMsgBox::activateButton(DialogMsgBoxButton button) {
	Common::BaseCallback<DialogMsgBoxButton> *callback = _callback;
	_callback = nullptr;
	close();
	if (callback) {
		(*callback)(button);
		delete callback;
	}
}

void DialogMsgBox::onRenderContent(ManagedSurface32 *screen) {
	if (!isActive() || !openDialog())
		return;

	const DialogMsgBoxButton hoveredButton = hitTest(Common::Point(_vm->getMousePos().x, _vm->getMousePos().y));
	if (hoveredButton != _hoveredButton) {
		_hoveredButton = hoveredButton;
		_redrawNeeded = true;
	}
	if (!_redrawNeeded)
		return;
	screen->copyRectToSurface(*_savedBackground, _position.x, _position.y, Common::Rect(_savedBackground->w, _savedBackground->h));
	_panels[static_cast<int>(_hoveredButton)]->drawToScreen(screen, _position, _vm->getAlphaLUT());
	_textImage->drawToSurface(screen, _position + _textOffset);
	_redrawNeeded = false;
}

EventHandleResult DialogMsgBox::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	return isActive() ? EventHandleResult::kConsumed : EventHandleResult::kPassthrough;
}

EventHandleResult DialogMsgBox::onLButtonUp(const Common::Point &pos) {
	if (!isActive())
		return EventHandleResult::kPassthrough;
	if (_state == DialogMsgBoxState::kOpen02) {
		const DialogMsgBoxButton button = hitTest(pos);
		if (button != DialogMsgBoxButton::kNone00)
			activateButton(button);
	}
	return EventHandleResult::kConsumed;
}

EventHandleResult DialogMsgBox::onMouseMove(const Common::Point &pos) {
	if (!isActive())
		return EventHandleResult::kPassthrough;
	if (_state == DialogMsgBoxState::kOpen02) {
		const DialogMsgBoxButton hoveredButton = hitTest(pos);
		if (hoveredButton != _hoveredButton) {
			_hoveredButton = hoveredButton;
			_redrawNeeded = true;
		}
	}
	return EventHandleResult::kConsumed;
}

EventHandleResult DialogMsgBox::onKeyDown(const Common::KeyState &key, bool repeat) {
	(void)repeat;
	if (!isActive())
		return EventHandleResult::kPassthrough;

	// ScummVM-only convenience shortcuts. The original message box is mouse-only.
	EventHandleResult result = EventHandleResult::kConsumed;
	if (_state == DialogMsgBoxState::kOpen02 && _vm->useEnhancedKbdShortcuts()) {
		const DialogKeyAction keyAction = classifyDialogKey(key);
		switch (keyAction) {
		case kDialogKeyAccept:
			activateButton(DialogMsgBoxButton::kOkay01);
			break;
		case kDialogKeyCancel:
			activateButton(DialogMsgBoxButton::kCancel02);
			break;
		default:
			break;
		}
	}
	return result;
}

EventHandleResult DialogMsgBox::onKeyUp(const Common::KeyState &key) {
	(void)key;
	return isActive() ? EventHandleResult::kConsumed : EventHandleResult::kPassthrough;
}

} // End of namespace Zoombini2
