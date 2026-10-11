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

#include "graphics/fontman.h"
#include "gui/ThemeEngine.h"
#include "gui/gui-manager.h"

#include "zoombini2/text.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *Text::kTextFontPath;

Text::Text(Zoombini2Engine *vm) : _vm(vm) {
}

Text::~Text() {
	delete _bmtFont;
}

Common::CodePage Text::gameCodePage(Common::Language language) {
	switch (language) {
	case Common::KO_KOR:
		return Common::kWindows949;
	case Common::PL_POL:
		return Common::kWindows1250;
	case Common::HE_ISR:
		return Common::kWindows1255;
	case Common::EN_ANY:
	case Common::EN_GRB:
	case Common::EN_USA:
	default:
		return Common::kWindows1252;
	}
}

Common::U32String Text::toU32String(const Common::String &str, Common::Language language) {
	return str.decode(gameCodePage(language));
}

Common::String Text::fromU32String(const Common::U32String &str, Common::Language language) {
	return str.encode(gameCodePage(language));
}

bool Text::loadTextFont() {
	if (hasTextFont())
		return true;
	if (!_bmtFont)
		_bmtFont = new BmtFont(_vm);
	return _bmtFont->load(Common::Path(kTextFontPath));
}

bool Text::hasTextFont() const {
	return _bmtFont && _bmtFont->isLoaded();
}

BmtFont *Text::bmtFont() const {
	return _bmtFont && _bmtFont->isLoaded() ? _bmtFont : nullptr;
}

const Graphics::Font *Text::resolveFont(Gfx::FontKind kind) const {
	switch (kind) {
	case Gfx::FontKind::kBmtFont:
		return bmtFont();
	case Gfx::FontKind::kGuiFont:
		// Cache the load decision here so the font file loads once.
		if (!_uiFontResolved) {
			_uiFontResolved = true;
			if (g_gui.theme()->loadExtraFont(GUI::ThemeEngine::kFontStyleNormal, _vm->getLanguage()))
				_uiFontHasExtra = true;
		}
		// Query both pointers fresh on every call because either owner can replace them.
		if (_uiFontHasExtra)
			return g_gui.theme()->getFont(GUI::ThemeEngine::kFontStyleLangExtra);
		return FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);
	case Gfx::FontKind::kDebugTitleFont:
		return FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	case Gfx::FontKind::kDebugFont:
	default:
		return FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	}
}

} // End of namespace Zoombini2
