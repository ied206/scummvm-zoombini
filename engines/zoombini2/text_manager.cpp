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

#include "zoombini2/text_manager.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *TextManager::kTextFontPath;

TextManager::TextManager(Zoombini2Engine *vm)
	: _vm(vm) {
	static const LocalizedStringMap strings = createLocalizedStrings();
	const Common::Language language = _vm->getLanguage();
	for (LocalizedStringMap::const_iterator entry = strings.begin(); entry != strings.end(); entry++) {
		const LanguageStrings &translations = entry->_value;
		const LanguageStrings::const_iterator text = translations.find(language);
		if (text != translations.end())
			_strMap[entry->_key] = text->_value;
		else
			_strMap[entry->_key] = translations[Common::EN_USA];
	}
}

TextManager::~TextManager() {
	delete _bmtFont;
}

TextManager::LocalizedStringMap TextManager::createLocalizedStrings() {
	LocalizedStringMap strings;
	strings[kRestoredLevelLabel][Common::EN_USA] = U"VERY, VERY HARD";
	strings[kRestoredLevelLabel][Common::KO_KOR] = U"삭제되었던 4단계";

	strings[kMissingLevel4Help][Common::EN_USA] = U"No help resource is available for level 4.";
	strings[kMissingLevel4Help][Common::KO_KOR] = U"4단계는 도움말 리소스가 없습니다.";

	strings[kConfirmCaseCollision][Common::EN_USA] =
		U"A save that differs only in letter case exists.\nThe new save will overwrite it.\nAre you sure to overwrite?";
	strings[kConfirmCaseCollision][Common::KO_KOR] =
		U"대소문자만 다른 저장파일 이름이 있습니다.\n새 파일은 기존 저장파일을 덮어씁니다.\n덮어쓸까요?";

	strings[kConfirmReadOnlyLoad][Common::EN_USA] =
		U"This saved game will open in read-only mode.\nProgress will not be saved.\nContinue?";
	strings[kConfirmReadOnlyLoad][Common::KO_KOR] =
		U"이 저장파일은 읽기 전용 모드로 열립니다.\n진행 상황은 저장되지 않습니다.\n계속할까요?";
	return strings;
}

const Common::U32String &TextManager::getLocalizedString(Key key) const {
	return _strMap[key];
}

Common::CodePage TextManager::gameCodePage(Common::Language language) {
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

Common::U32String TextManager::toU32String(const Common::String &str, Common::Language language) {
	return str.decode(gameCodePage(language));
}

Common::String TextManager::fromU32String(const Common::U32String &str, Common::Language language) {
	return str.encode(gameCodePage(language));
}

bool TextManager::loadTextFont() {
	if (hasTextFont())
		return true;
	if (!_bmtFont)
		_bmtFont = new BmtFont(_vm);
	return _bmtFont->load(Common::Path(kTextFontPath));
}

bool TextManager::hasTextFont() const {
	return _bmtFont && _bmtFont->isLoaded();
}

BmtFont *TextManager::bmtFont() const {
	return _bmtFont && _bmtFont->isLoaded() ? _bmtFont : nullptr;
}

const Graphics::Font *TextManager::resolveFont(Gfx::FontKind kind) const {
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
