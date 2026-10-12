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

bool TextObject::create(const Core::ScriptArguments &arguments) {
	if (arguments.size() < 3 || 6 < arguments.size())
		return false;
	_text = arguments[0].text();
	_point = Common::Point(arguments[1].integer(), arguments[2].integer());
	_usePoint = arguments.size() < 5 || arguments[4].text().empty();
	if (_usePoint) {
		_center = 3 < arguments.size() && arguments[3].integer() != 0;
	} else {
		x = _point.x;
		y = _point.y;
		_width = arguments[3].integer();
		_height = arguments[4].integer();
		_wrap = 5 < arguments.size() && arguments[5].integer() != 0;
	}
	updateBox();
	return true;
}

void TextObject::updateBox() {
	if (!_usePoint)
		return;
	Common::Rect box = font().inkBounds(_text);
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
	Common::String path = _resources.resourcePath(resource, "nfnt");
	Common::ScopedPtr<Common::SeekableReadStream> stream(_resources.open(path));
	Common::SharedPtr<OM::NFNTFont> font(stream ? OM::NFNTFont::load(*stream) : nullptr);
	if (!font)
		return false;
	_resourceFont = font;
	_fontPath = path;
	updateBox();
	return true;
}

bool TextObject::setColor(const Core::ScriptArguments &arguments, const Core::ScriptValue &value, uint16 *color) {
	if (arguments.size() != 2)
		return false;
	color[0] = arguments[0].integer();
	color[1] = arguments[1].integer();
	color[2] = value.integer();
	return true;
}

bool TextObject::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property == "text")
		result = Core::ScriptValue::fromText(_text);
	else if (property == "userData")
		result = Core::ScriptValue::fromText(_userData);
	else if (property == "center")
		result = Core::ScriptValue(static_cast<int32>(_center));
	else if (property == "wrap")
		result = Core::ScriptValue(static_cast<int32>(_wrap));
	else if (property == "outline")
		result = Core::ScriptValue(static_cast<int32>(_outline));
	else if (property == "pointX" || (_usePoint && property == "x"))
		result = Core::ScriptValue(static_cast<int32>(_point.x));
	else if (property == "pointY" || (_usePoint && property == "y"))
		result = Core::ScriptValue(static_cast<int32>(_point.y));
	else if (property == "fontAscent")
		result = Core::ScriptValue(font().ascent());
	else if (property == "fontDescent")
		result = Core::ScriptValue(font().descent());
	else if (property == "fontLeading")
		result = Core::ScriptValue(font().getFontHeight() - font().ascent() - font().descent());
	else
		return Core::Trinket::get(property, arguments, result);
	return true;
}

bool TextObject::set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) {
	if (property == "text") {
		_text = value.text();
		if (2 <= _text.size() && _text.firstChar() == '"' && _text.lastChar() == '"')
			_text = _text.substr(1, _text.size() - 2);
	} else if (property == "userData") {
		_userData = value.text();
	} else if (property == "font") {
		return arguments.empty() && setFont(value.text());
	} else if (property == "color") {
		return setColor(arguments, value, _color);
	} else if (property == "outlineColor") {
		return setColor(arguments, value, _outlineColor);
	} else if (property == "center") {
		_center = value.integer() != 0;
	} else if (property == "outline") {
		_outline = value.integer() != 0;
	} else if (property == "wrap") {
		if (!_usePoint)
			_wrap = value.integer() != 0;
	} else if (property == "pointX" || (_usePoint && property == "x")) {
		_point.x = value.integer();
	} else if (property == "pointY" || (_usePoint && property == "y")) {
		_point.y = value.integer();
	} else {
		return Core::Trinket::set(property, arguments, value);
	}
	updateBox();
	return true;
}

Core::ScriptableObject *TextObject::clone() const {
	Common::ScopedPtr<TextObject> copy(new TextObject(_resources, _defaultFont));
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
	int leading = font().getFontHeight() - font().ascent() - font().descent();
	int baseline = y - box.top + Text::firstBaseline(_height, lines.size(), font().ascent(), font().descent(), leading);
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
						font().drawString(&surface, lines[i], left + dx, baseline - font().ascent() + dy, _width + 2, outline,
										  Graphics::kTextAlignLeft, 0, false, true);
				}
			}
		}
		font().drawString(&surface, lines[i], left, baseline - font().ascent(), _width + 2, color, Graphics::kTextAlignLeft, 0, false, true);
		baseline += font().getFontHeight();
	}
}

} // namespace Shell
} // namespace Trinket
