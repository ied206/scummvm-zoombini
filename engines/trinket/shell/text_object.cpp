/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/shell/text_object.h"

#include "common/ptr.h"
#include "trinket/shell/text.h"

namespace Trinket {
namespace Shell {

constexpr const char *TextObject::kPropertyText;
constexpr const char *TextObject::kPropertyUserData;
constexpr const char *TextObject::kPropertyCenter;
constexpr const char *TextObject::kPropertyWrap;
constexpr const char *TextObject::kPropertyOutline;
constexpr const char *TextObject::kPropertyPointX;
constexpr const char *TextObject::kPropertyX;
constexpr const char *TextObject::kPropertyPointY;
constexpr const char *TextObject::kPropertyY;
constexpr const char *TextObject::kPropertyFontAscent;
constexpr const char *TextObject::kPropertyFontDescent;
constexpr const char *TextObject::kPropertyFontLeading;
constexpr const char *TextObject::kPropertyFont;
constexpr const char *TextObject::kPropertyColor;
constexpr const char *TextObject::kPropertyOutlineColor;

constexpr const char *TextObject::kFontExtension;

bool TextObject::create(const Core::ScriptArguments &sargs) {
	if (sargs.size() < 3 || 6 < sargs.size())
		return false;
	_text = sargs[0].toText();
	_point = Common::Point(sargs[1].toInt32(), sargs[2].toInt32());
	_usePoint = sargs.size() < 5 || sargs[4].toText().empty();
	if (_usePoint) {
		_center = 3 < sargs.size() && sargs[3].toInt32() != 0;
	} else {
		x = _point.x;
		y = _point.y;
		_width = sargs[3].toInt32();
		_height = sargs[4].toInt32();
		_wrap = 5 < sargs.size() && sargs[5].toInt32() != 0;
	}
	updateBox();
	return true;
}

void TextObject::updateBox() {
	if (!_usePoint)
		return;
	Common::Rect box = Text::inkBounds(font(), _text);
	box.right += 2;
	if (_outline)
		box.grow(1);
	_inkLeft = box.left;
	_width = box.width();
	_height = box.height();
	x = _point.x;
	y = _point.y;
	if (_center) {
		x -= _width / 2;
		y -= _height / 2;
	}
}

bool TextObject::setFont(const Common::String &resource) {
	Common::String path = _resources.resourcePath(resource, kFontExtension);
	const Graphics::Font *font = _fonts.load(path);
	if (!font)
		return false;
	_resourceFont = font;
	_fontPath = path;
	updateBox();
	return true;
}

bool TextObject::setColor(const Core::ScriptArguments &sargs, const Core::ScriptValue &value, uint16 *color) {
	if (sargs.size() != 2)
		return false;
	color[0] = sargs[0].toInt32();
	color[1] = sargs[1].toInt32();
	color[2] = value.toInt32();
	return true;
}

bool TextObject::get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (property.equals(kPropertyText))
		result = Core::ScriptValue::fromText(_text);
	else if (property.equals(kPropertyUserData))
		result = Core::ScriptValue::fromText(_userData);
	else if (property.equals(kPropertyCenter))
		result = Core::ScriptValue(static_cast<int32>(_center));
	else if (property.equals(kPropertyWrap))
		result = Core::ScriptValue(static_cast<int32>(_wrap));
	else if (property.equals(kPropertyOutline))
		result = Core::ScriptValue(static_cast<int32>(_outline));
	else if (property.equals(kPropertyPointX) || (_usePoint && property.equals(kPropertyX)))
		result = Core::ScriptValue(static_cast<int32>(_point.x));
	else if (property.equals(kPropertyPointY) || (_usePoint && property.equals(kPropertyY)))
		result = Core::ScriptValue(static_cast<int32>(_point.y));
	else if (property.equals(kPropertyFontAscent))
		result = Core::ScriptValue(font().getFontAscent());
	else if (property.equals(kPropertyFontDescent))
		result = Core::ScriptValue(font().getFontDescent());
	else if (property.equals(kPropertyFontLeading))
		result = Core::ScriptValue(font().getFontHeight() - font().getFontAscent() - font().getFontDescent());
	else
		return Core::Trinket::get(property, sargs, result);
	return true;
}

bool TextObject::set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) {
	if (property.equals(kPropertyText)) {
		_text = value.toText();
		if (2 <= _text.size() && _text.firstChar() == '"' && _text.lastChar() == '"')
			_text = _text.substr(1, _text.size() - 2);
	} else if (property.equals(kPropertyUserData)) {
		_userData = value.toText();
	} else if (property.equals(kPropertyFont)) {
		return sargs.empty() && setFont(value.toText());
	} else if (property.equals(kPropertyColor)) {
		return setColor(sargs, value, _color);
	} else if (property.equals(kPropertyOutlineColor)) {
		return setColor(sargs, value, _outlineColor);
	} else if (property.equals(kPropertyCenter)) {
		_center = value.toInt32() != 0;
	} else if (property.equals(kPropertyOutline)) {
		_outline = value.toInt32() != 0;
	} else if (property.equals(kPropertyWrap)) {
		if (!_usePoint)
			_wrap = value.toInt32() != 0;
	} else if (property.equals(kPropertyPointX) || (_usePoint && property.equals(kPropertyX))) {
		_point.x = value.toInt32();
	} else if (property.equals(kPropertyPointY) || (_usePoint && property.equals(kPropertyY))) {
		_point.y = value.toInt32();
	} else {
		return Core::Trinket::set(property, sargs, value);
	}
	updateBox();
	return true;
}

Core::ScriptableObject *TextObject::clone() const {
	Common::ScopedPtr<TextObject> copy(new TextObject(_resources, _fonts, _defaultFont));
	copy->_text = _text;
	copy->_userData = _userData;
	copy->_point = _point;
	copy->_usePoint = _usePoint;
	copy->_center = _center;
	copy->_wrap = _wrap;
	copy->_outline = _outline;
	copy->_width = _width;
	copy->_height = _height;
	copy->x = x;
	copy->y = y;
	copy->z = z;
	copy->tag = tag;
	copy->visible = visible;
	copy->enabled = enabled;
	copy->movable = movable;
	for (uint i = 0; i < 3; i++) {
		copy->_color[i] = _color[i];
		copy->_outlineColor[i] = _outlineColor[i];
	}
	if (!_fontPath.empty() && !copy->setFont(_fontPath))
		return nullptr;
	copy->updateBox();
	copy->spotCursor = spotCursor;
	copy->grabCursor = grabCursor;
	copyEventsTo(*copy);
	return copy.release();
}

void TextObject::draw(OM::Canvas &canvas) const {
	if (!visible || _width <= 0 || _height <= 0)
		return;
	Common::Rect box(x, y, x + _width, y + _height);
	box.clip(Common::Rect(canvas.surface().w, canvas.surface().h));
	if (box.isEmpty())
		return;
	Graphics::Surface surface = canvas.surface().getSubArea(box);
	Common::Array<Common::String> lines;
	if (!Text::wrap(font(), _text, _width, lines, Text::kGame, _wrap, _outline))
		return;
	int leading = font().getFontHeight() - font().getFontAscent() - font().getFontDescent();
	int baseline = y - box.top + Text::firstBaseline(_height, lines.size(), font().getFontAscent(), font().getFontDescent(), leading);
	uint32 color = surface.format.RGBToColor(_color[0] / 256, _color[1] / 256, _color[2] / 256);
	uint32 outline = surface.format.RGBToColor(_outlineColor[0] / 256, _outlineColor[1] / 256, _outlineColor[2] / 256);
	for (uint i = 0; i < lines.size(); i++) {
		int left = x - box.left - _inkLeft;
		if (_center)
			left = x - box.left + (_width - font().getStringWidth(lines[i])) / 2 - _inkLeft / 2;
		if (_outline) {
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
					if (dx || dy)
						font().drawString(&surface, lines[i], left + dx, baseline - font().getFontAscent() + dy, _width + 2, outline,
										  Graphics::kTextAlignLeft, 0, false, true);
				}
			}
		}
		font().drawString(&surface, lines[i], left, baseline - font().getFontAscent(), _width + 2, color, Graphics::kTextAlignLeft, 0, false, true);
		baseline += font().getFontHeight();
	}
}

} // namespace Shell
} // namespace Trinket
