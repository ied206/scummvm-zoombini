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
	/** Create an interactive rectangle at the supplied bounds and depth. */
	HotSpot(const Common::Rect &area, int depth) : _size(area.width(), area.height()) {
		x = area.left;
		y = area.top;
		z = depth;
		movable = false;
	}
	/** Return the hotspot width in game pixels. */
	int width() override { return _size.x; }
	/** Return the hotspot height in game pixels. */
	int height() override { return _size.y; }
	/** Return the hotspot rectangle in its parent's coordinate space. */
	Common::Rect bounds() const { return Common::Rect(x, y, x + _size.x, y + _size.y); }
	/** Read hotspot user data or an inherited geometry property. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override {
		if (property.equals("outlined") || property.equals("filled")) {
			result = Core::ScriptValue(static_cast<int32>(property.equals("outlined") ? _outlined : _filled));
			return true;
		}
		if (!property.equals("userData"))
			return Core::Trinket::get(property, sargs, result);
		result = Core::ScriptValue::fromText(_userData);
		return true;
	}
	/** Change hotspot user data, drawing flags or inherited geometry. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override {
		if (property.equals("outlined") || property.equals("filled")) {
			if (property.equals("outlined"))
				_outlined = value.toInt32() != 0;
			else
				_filled = value.toInt32() != 0;
			return true;
		}
		if (!property.equals("userData"))
			return Core::Trinket::set(property, sargs, value);
		_userData = value.toText();
		return true;
	}

	class Factory : public Core::ObjectRegistry::Factory {
	public:
		/** Create a rectangular hotspot from four bounds and a drawing depth. */
		Core::ScriptableObject *create(const Core::ScriptArguments &sargs) override {
			if (sargs.size() != 5)
				return nullptr;
			return new HotSpot(Common::Rect(sargs[0].toInt32(), sargs[1].toInt32(), sargs[2].toInt32(), sargs[3].toInt32()),
							   sargs[4].toInt32());
		}
	};
	/** Select a hotspot cursor or dispatch an inherited script method. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override {
		if (method.equals("setAOCursor") && !sargs.empty()) {
			spotCursor = sargs[0].toText();
			return true;
		}
		return Core::Trinket::play(method, sargs, result);
	}
	/** Draw the requested black outline or fill inside the clipped hotspot bounds. */
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
	/** Width and height of the interactive rectangle in game pixels. */
	Common::Point _size;
	/** Arbitrary script text attached to this hotspot. */
	Common::String _userData;
	/** Whether the hotspot draws a black border. */
	bool _outlined = false;
	/** Whether the hotspot fills its rectangle with black. */
	bool _filled = false;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
