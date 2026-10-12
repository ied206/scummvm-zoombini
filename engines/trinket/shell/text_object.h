/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_SHELL_TEXT_OBJECT_H
#define TRINKET_SHELL_TEXT_OBJECT_H

#include "trinket/core/trinket.h"
#include "trinket/ompp/canvas.h"
#include "trinket/ompp/font.h"
#include "trinket/shell/resource_paths.h"

namespace Trinket {
namespace Shell {

/** Script-owned RText geometry, font resource and copied text. */
class TextObject : public Core::Trinket {
public:
	TextObject(const ResourcePaths &resources, const Common::SharedPtr<OM::NFNTFont> &defaultFont) : _resources(resources), _defaultFont(defaultFont) {}
	bool create(const Core::ScriptArguments &arguments);
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	Core::ScriptableObject *clone() const override;
	int width() override { return _width; }
	int height() override { return _height; }
	void draw(OM::Canvas &canvas) const;

private:
	const ResourcePaths &_resources;
	Common::SharedPtr<OM::NFNTFont> _defaultFont, _resourceFont;
	Common::String _fontPath, _text, _userData;
	Common::Point _point;
	int _width = 0, _height = 0, _inkLeft = 0;
	bool _usePoint = false, _center = true, _wrap = false, _outline = false;
	uint16 _color[3] = {}, _outlineColor[3] = {};
	const OM::NFNTFont &font() const { return _fontPath.empty() ? *_defaultFont : *_resourceFont; }
	void updateBox();
	bool setFont(const Common::String &resource);
	static bool setColor(const Core::ScriptArguments &arguments, const Core::ScriptValue &value, uint16 *color);
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
