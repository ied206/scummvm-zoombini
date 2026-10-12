/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_SHELL_HOTSPOT_H
#define TRINKET_SHELL_HOTSPOT_H

#include "common/rect.h"
#include "trinket/core/trinket.h"
#include "trinket/ompp/canvas.h"

namespace Trinket {
namespace Shell {

/** Rectangular native script object used for input and intersection queries. */
class HotSpot : public Core::Trinket {
public:
	HotSpot(const Common::Rect &area, int depth) : _size(area.width(), area.height()) {
		x = area.left;
		y = area.top;
		z = depth;
		movable = false;
	}
	int width() override { return _size.x; }
	int height() override { return _size.y; }
	Common::Rect bounds() const { return Common::Rect(x, y, x + _size.x, y + _size.y); }
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override {
		if (property == "outlined" || property == "filled") {
			result = Core::ScriptValue(static_cast<int32>(property == "outlined" ? _outlined : _filled));
			return true;
		}
		if (property != "userData")
			return Core::Trinket::get(property, arguments, result);
		result = Core::ScriptValue::fromText(_userData);
		return true;
	}
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override {
		if (property == "outlined" || property == "filled") {
			if (property == "outlined")
				_outlined = value.integer() != 0;
			else
				_filled = value.integer() != 0;
			return true;
		}
		if (property != "userData")
			return Core::Trinket::set(property, arguments, value);
		_userData = value.text();
		return true;
	}

	class Factory : public Core::ObjectRegistry::Factory {
	public:
		Core::ScriptableObject *create(const Core::ScriptArguments &arguments) override {
			if (arguments.size() != 5)
				return nullptr;
			return new HotSpot(Common::Rect(arguments[0].integer(), arguments[1].integer(), arguments[2].integer(), arguments[3].integer()),
							   arguments[4].integer());
		}
	};
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override {
		if (method == "setAOCursor" && !arguments.empty()) {
			spotCursor = arguments[0].text();
			return true;
		}
		return Core::Trinket::play(method, arguments, result);
	}
	void draw(OM::Canvas &canvas) const {
		if (!visible || (!_outlined && !_filled))
			return;
		Common::Rect rect = bounds();
		rect.clip(Common::Rect(canvas.surface().w, canvas.surface().h));
		uint32 color = canvas.surface().format.RGBToColor(0, 0, 0);
		if (_filled)
			canvas.surface().fillRect(rect, color);
		else
			canvas.surface().frameRect(rect, color);
	}

private:
	Common::Point _size;
	Common::String _userData;
	bool _outlined = false, _filled = false;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
