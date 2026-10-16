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

#include "common/ptr.h"
#include "common/system.h"

namespace Zoombini2 {

constexpr int DialogDebug::kSheetPadding;
constexpr int DialogDebug::kSheetFrameLimit;
constexpr int DialogDebug::kSheetSourceLimit;
constexpr const char *DialogDebug::kSheetKeyLegend;

DialogDebug::DialogDebug(Zoombini2Engine *vm)
	: DialogBase(vm) {
	_savedScreen = _vm->_gfx->createSurface(Gfx::kScreenSize);
}

DialogDebug::~DialogDebug() {
	close();

	delete _savedScreen;
}

bool DialogDebug::open(const DialogDebugCommand &cmd) {
	if (_isActive)
		return false;

	freeAnimation();
	freeImage();
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
	case DialogDebugCommand::Type::kDrawSprites: {
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

		if (cmd._startFrame < 0 || getFrameCount() <= cmd._startFrame) {
			freeAnimation();
			return false;
		}
		_sheetStartFrame = cmd._startFrame;
		_animPath = cmd._animPath;
		prepareAnimationSheet();
		break;
	}
	case DialogDebugCommand::Type::kDrawImage: {
		_image = new BitBlock(_vm);
		if (!_image->load(cmd._imagePath)) {
			freeImage();
			return false;
		}
		_titleText = Common::String::format("[Image] %s (%d x %d)", cmd._imagePath.toString('/').c_str(), _image->getWidth(), _image->getHeight());
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
	freeImage();
	_vm->setDialogPaused(false);
}

void DialogDebug::freeAnimation() {
	delete _animation;
	_animation = nullptr;
	_singleFrameSprite = false;
	_sheetStartFrame = 0;
	_sheetFrameCapacity = 0;
	_sheetColumns = 0;
	_sheetCellSize = Size32();
}

void DialogDebug::freeImage() {
	delete _image;
	_image = nullptr;
}

int DialogDebug::getFrameCount() const {
	if (_singleFrameSprite)
		return 1;
	if (_animation)
		return _animation->getFrameCount();
	return 0;
}

const RleBlock *DialogDebug::getAnimationFrame(int index) const {
	if (index < 0 || getFrameCount() <= index)
		return nullptr;
	if (_singleFrameSprite)
		return _vm->_gfx->loadPageRleBlock(_animPath.toString('/'));
	return _animation->getFrame(index);
}

Size32 DialogDebug::getSheetFrameSize(const Size32 &size) {
	if (size.width <= kSheetFrameLimit && size.height <= kSheetFrameLimit)
		return size;
	if (size.height <= size.width)
		return Size32(kSheetFrameLimit, MAX(1, static_cast<int>(static_cast<int64>(size.height) * kSheetFrameLimit / size.width)));
	return Size32(MAX(1, static_cast<int>(static_cast<int64>(size.width) * kSheetFrameLimit / size.height)), kSheetFrameLimit);
}

void DialogDebug::prepareAnimationSheet() {
	Gfx *gfx = _vm->_gfx;
	Gfx::TextConf conf;
	conf.fontKind = Gfx::FontKind::kDebugFont;
	conf.fillColor = Gfx::PredefinedColor::kBlack.toPixel(_savedScreen->format);
	const int labelHeight = gfx->getFontHeight(conf);
	int cellWidth = 1;
	int frameHeight = 1;
	for (int i = 0; i < getFrameCount(); i++) {
		const RleBlock *frame = getAnimationFrame(i);
		if (!frame)
			continue;
		const Size32 sourceSize(MIN(frame->getWidth(), kSheetSourceLimit), MIN(frame->getHeight(), kSheetSourceLimit));
		const Size32 previewSize = getSheetFrameSize(sourceSize);
		const Common::String label = Common::String::format("%d: %d x %d", i, frame->getWidth(), frame->getHeight());
		cellWidth = MAX(cellWidth, MAX(previewSize.width, gfx->getTextWidth(label, conf)));
		frameHeight = MAX(frameHeight, previewSize.height);
	}
	_sheetCellSize = Size32(cellWidth + kSheetPadding * 2, frameHeight + labelHeight + kSheetPadding * 3);
	_sheetColumns = MAX(1, (_savedScreen->w - kSheetPadding * 2) / _sheetCellSize.width);
	const int rows = MAX(1, (_savedScreen->h - getTitleHeight() - kSheetPadding * 2) / _sheetCellSize.height);
	_sheetFrameCapacity = _sheetColumns * rows;
	const uint32 white = _savedScreen->format.ARGBToColor(255, 255, 255, 255);
	const uint32 border = _savedScreen->format.ARGBToColor(255, 192, 192, 192);
	_savedScreen->fillRect(Common::Rect(_savedScreen->w, _savedScreen->h), white);
	const int endFrame = MIN(getFrameCount(), _sheetStartFrame + _sheetFrameCapacity);
	for (int i = _sheetStartFrame; i < endFrame; i++) {
		const RleBlock *frame = getAnimationFrame(i);
		if (!frame)
			continue;
		const int cellIndex = i - _sheetStartFrame;
		const int x = kSheetPadding + cellIndex % _sheetColumns * _sheetCellSize.width;
		const int y = kSheetPadding + cellIndex / _sheetColumns * _sheetCellSize.height;
		gfx->frameRect(_savedScreen, Common::Rect(x, y, x + _sheetCellSize.width, y + _sheetCellSize.height), border);
		const Common::String label = Common::String::format("%d: %d x %d", i, frame->getWidth(), frame->getHeight());
		gfx->drawString(_savedScreen, label, Common::Point32(x + kSheetPadding, y + kSheetPadding), cellWidth, conf);
		const Size32 sourceSize(MIN(frame->getWidth(), kSheetSourceLimit), MIN(frame->getHeight(), kSheetSourceLimit));
		const Size32 previewSize = getSheetFrameSize(sourceSize);
		const Common::Point32 pos(x + (_sheetCellSize.width - previewSize.width) / 2, y + labelHeight + kSheetPadding * 2);
		if (frame->getWidth() == previewSize.width && frame->getHeight() == previewSize.height) {
			const Common::Rect32 clip(pos.x, pos.y, pos.x + previewSize.width, pos.y + previewSize.height);
			gfx->drawRleBlockClipped(_savedScreen, frame, pos, clip);
		} else {
			if (sourceSize.width != frame->getWidth() || sourceSize.height != frame->getHeight())
				warning("DialogDebug: clipping oversized sheet frame %d (%d x %d)", i, frame->getWidth(), frame->getHeight());
			Common::ScopedPtr<ManagedSurface32> source(gfx->createSurface(sourceSize));
			source->fillRect(Common::Rect(source->w, source->h), white);
			gfx->drawRleBlock(source.get(), frame, Common::Point32());
			_savedScreen->blitFrom(source->rawSurface(), Common::Rect(source->w, source->h),
								   Common::Rect(pos.x, pos.y, pos.x + previewSize.width, pos.y + previewSize.height));
		}
	}
	updateAnimationTitle();
}

void DialogDebug::updateAnimationTitle() {
	const int lastFrame = MIN(getFrameCount(), _sheetStartFrame + _sheetFrameCapacity) - 1;
	_titleText = Common::String::format("[Sprite Sheet] frames(%d-%d/%d) %s", _sheetStartFrame, lastFrame, getFrameCount(),
										_animPath.toString('/').c_str());
}

void DialogDebug::preparePlot(const DialogDebugCommand &cmd) {
	// Overlay the primitive on a snapshot of the current screen so the
	// coordinate can be read against the live game scene.
	_vm->_gfx->captureScreen(_savedScreen);
	const uint32 color = cmd._plotColor.toPixel(_savedScreen->format);
	const Common::String colorText = cmd._plotColor.toString();
	const Common::Point32 &start = cmd._plotStart;
	const Common::Point32 &end = cmd._plotEnd;

	switch (cmd._type) {
	case DialogDebugCommand::Type::kPlotPoint:
		_vm->_gfx->fillRect(_savedScreen, Common::Rect32(start.x, start.y, start.x + 1, start.y + 1), color);
		_titleText = Common::String::format("[Plot Point] at (%d, %d) color(%s)", start.x, start.y, colorText.c_str());
		break;
	case DialogDebugCommand::Type::kPlotLine:
		_vm->_gfx->drawLine(_savedScreen, start, end, color);
		_titleText = Common::String::format("[Plot Line] from (%d, %d) to (%d, %d) color(%s)", start.x, start.y, end.x, end.y, colorText.c_str());
		break;
	case DialogDebugCommand::Type::kPlotRect: {
		_vm->_gfx->frameRect(_savedScreen, Common::Rect(start.x, start.y, end.x, end.y), color);
		_titleText = Common::String::format("[Plot Rect] from (%d, %d) to (%d, %d) color(%s)", start.x, start.y, end.x, end.y, colorText.c_str());
		break;
	}
	default:
		break;
	}
}

int DialogDebug::getTitleHeight() const {
	Gfx::TextConf conf;
	conf.fontKind = Gfx::FontKind::kDebugTitleFont;
	const int titleFontHeight = _vm->_gfx->getFontHeight(conf);
	conf.fontKind = Gfx::FontKind::kDebugFont;
	const int legendFontHeight = _vm->_gfx->getFontHeight(conf);
	return MAX(kTitleHeight, kTitleY + MAX(titleFontHeight, legendFontHeight) + kTitleY);
}

void DialogDebug::drawTitleText(ManagedSurface32 *screen) const {
	if (_titleText.empty())
		return;
	_vm->_gfx->fillRect(screen, Common::Rect32(0, 0, Gfx::kScreenSize.width, getTitleHeight()), 0);
	Gfx::TextConf conf;
	conf.fontKind = Gfx::FontKind::kDebugTitleFont;
	conf.fillColor = Gfx::PredefinedColor::kWhite03.toPixel(screen->format);
	int maxWidth = Gfx::kNoMaxWidth;
	if (_viewType == DialogDebugCommand::Type::kDrawSprites) {
		Gfx::TextConf legendConf;
		legendConf.fontKind = Gfx::FontKind::kDebugFont;
		const Common::String legend = Common::String(kSheetKeyLegend) + " [ESC] close";
		maxWidth = screen->w - _vm->_gfx->getTextWidth(legend, legendConf) - kTitleX * 3;
	}
	_vm->_gfx->drawString(screen, _titleText, Common::Point32(kTitleX, kTitleY), maxWidth, conf);
}

void DialogDebug::drawEscText(ManagedSurface32 *screen, const Common::String &keyLegend) const {
	static constexpr const char *kEscText = "[ESC] close";
	Common::String text;
	if (keyLegend.empty()) {
		text = kEscText;
	} else {
		text = keyLegend;
		if (keyLegend.lastChar() != ' ')
			text += ' ';
		text += kEscText;
	}
	Gfx::TextConf conf;
	conf.fontKind = Gfx::FontKind::kDebugFont;
	conf.fillColor = Gfx::PredefinedColor::kWhite03.toPixel(screen->format);
	const int escWidth = _vm->_gfx->getTextWidth(text, conf);
	_vm->_gfx->drawString(screen, text,
						  Common::Point32(Gfx::kScreenSize.width - escWidth - kTitleX, kTitleY), Gfx::kNoMaxWidth, conf);
}

void DialogDebug::onRenderContent(ManagedSurface32 *screen) {
	if (!_isActive)
		return;

	switch (_viewType) {
	case DialogDebugCommand::Type::kDrawImage: {
		const uint32 white = _savedScreen->format.ARGBToColor(255, 255, 255, 255);
		_vm->_gfx->fillRect(_savedScreen, Common::Rect32(Gfx::kScreenSize.width, Gfx::kScreenSize.height), white);
		_vm->_gfx->drawBitBlock(_savedScreen, _image, Common::Point32(0, 0));
		break;
	}
	default:
		break;
	}

	if (_viewType == DialogDebugCommand::Type::kDrawImage || _viewType == DialogDebugCommand::Type::kDrawSprites ||
		_viewType == DialogDebugCommand::Type::kDrawAreaMask)
		screen->copyRectToSurface(_savedScreen->rawSurface(), 0, getTitleHeight(), Common::Rect32(_savedScreen->w, _savedScreen->h));
	else
		screen->copyFrom(*_savedScreen);

	drawTitleText(screen);
	if (_viewType == DialogDebugCommand::Type::kDrawSprites)
		drawEscText(screen, kSheetKeyLegend);
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
	if (_viewType == DialogDebugCommand::Type::kDrawSprites) {
		switch (key.keycode) {
		case Common::KEYCODE_LEFT:
			if (0 < _sheetStartFrame) {
				_sheetStartFrame = MAX(0, _sheetStartFrame - _sheetFrameCapacity);
				prepareAnimationSheet();
			}
			break;
		case Common::KEYCODE_RIGHT:
			if (_sheetStartFrame + _sheetFrameCapacity < getFrameCount()) {
				_sheetStartFrame += _sheetFrameCapacity;
				prepareAnimationSheet();
			}
			break;
		default:
			break;
		}
	}
	return EventHandleResult::kConsumed;
}

} // End of namespace Zoombini2
