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

/** Display script-controlled text with its font, bounds and alignment. */
class TextObject : public Core::Trinket {
public:
	/** Create text using resource-path and font services and the supplied default font. */
	TextObject(const ResourcePaths &resources, OM::FontCache &fonts, const Common::SharedPtr<OM::NFNTFont> &defaultFont) : _resources(resources), _fonts(fonts), _defaultFont(defaultFont) {}
	/** Configure text, font and positioning from the script constructor arguments. */
	bool create(const Core::ScriptArguments &sargs);
	/** Read text, font, color, alignment and inherited geometry properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign text properties and recalculate the drawing box when needed. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Create another text object with the same content and layout settings. */
	Core::ScriptableObject *clone() const override;
	/** Return the current text box width in game pixels. */
	int width() override { return _width; }
	/** Return the current text box height in game pixels. */
	int height() override { return _height; }
	/** Draw wrapped, aligned text with its optional outline. */
	void draw(OM::Canvas &canvas) const;

private:
	/** Script property name for text. */
	static constexpr const char *kPropertyText = "text";
	/** Script property name for user Data. */
	static constexpr const char *kPropertyUserData = "userData";
	/** Script property name for center. */
	static constexpr const char *kPropertyCenter = "center";
	/** Script property name for wrap. */
	static constexpr const char *kPropertyWrap = "wrap";
	/** Script property name for outline. */
	static constexpr const char *kPropertyOutline = "outline";
	/** Script property name for point X. */
	static constexpr const char *kPropertyPointX = "pointX";
	/** Script property name for x. */
	static constexpr const char *kPropertyX = "x";
	/** Script property name for point Y. */
	static constexpr const char *kPropertyPointY = "pointY";
	/** Script property name for y. */
	static constexpr const char *kPropertyY = "y";
	/** Script property name for font Ascent. */
	static constexpr const char *kPropertyFontAscent = "fontAscent";
	/** Script property name for font Descent. */
	static constexpr const char *kPropertyFontDescent = "fontDescent";
	/** Script property name for font Leading. */
	static constexpr const char *kPropertyFontLeading = "fontLeading";
	/** Script property name for font. */
	static constexpr const char *kPropertyFont = "font";
	/** Script property name for color. */
	static constexpr const char *kPropertyColor = "color";
	/** Script property name for outline Color. */
	static constexpr const char *kPropertyOutlineColor = "outlineColor";

	/** Extension used to resolve bitmap fonts. */
	static constexpr const char *kFontExtension = "nfnt";

	/** Resource-path resolver used to select text fonts. */
	const ResourcePaths &_resources;
	/** Shared strike cache belonging to this resource namespace. */
	OM::FontCache &_fonts;
	/** Font used when no explicit font resource was selected. */
	Common::SharedPtr<OM::NFNTFont> _defaultFont;
	/** Decoded font selected by the current font resource path. */
	Common::SharedPtr<OM::NFNTFont> _resourceFont;
	/** Explicit font resource path, empty when using the default font. */
	Common::String _fontPath;
	/** Text content displayed by the object. */
	Common::String _text;
	/** Arbitrary script text attached to the text object. */
	Common::String _userData;
	/** Authored anchor position in game pixels. */
	Common::Point _point;
	/** Text box width in game pixels. */
	int _width = 0;
	/** Text box height in game pixels. */
	int _height = 0;
	/** Left ink bearing accounted for when placing text in the box. */
	int _inkLeft = 0;
	/** Whether layout derives its box from the anchor point rather than explicit bounds. */
	bool _usePoint = false;
	/** Whether text lines are horizontally centered. */
	bool _center = true;
	/** Whether lines wrap to the current text box width. */
	bool _wrap = false;
	/** Whether a contrasting outline is drawn around glyphs. */
	bool _outline = false;
	/** Text RGB components in the zero-to-65535 script color range. */
	uint16 _color[3] = {};
	/** Outline RGB components in the zero-to-65535 script color range. */
	uint16 _outlineColor[3] = {};
	/** Return the selected resource font or the default font. */
	const OM::NFNTFont &font() const { return _fontPath.empty() ? *_defaultFont : *_resourceFont; }
	/** Recalculate text bounds from the content, font and positioning mode. */
	void updateBox();
	/** Load and select a font resource, returning false if it cannot be decoded. */
	bool setFont(const Common::String &resource);
	/** Assign one or all script RGB components after validating the argument count. */
	static bool setColor(const Core::ScriptArguments &sargs, const Core::ScriptValue &value, uint16 *color);
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
