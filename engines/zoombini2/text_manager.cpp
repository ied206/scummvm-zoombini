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

#include "common/array.h"
#include "common/ptr.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/fontman.h"
#include "gui/ThemeEngine.h"
#include "gui/gui-manager.h"

#include "zoombini2/text_manager.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *TextManager::kTextFontPath;
constexpr const char *TextManager::kBmtFontCacheName;
constexpr const char *TextManager::kRussianExecutablePath;

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
	if (language == Common::RU_RUS && !loadRussianTables())
		error("TextManager: Failed to load Russian name and keyboard tables from %s", kRussianExecutablePath);
}

TextManager::~TextManager() {
	FontMan.removeFontName(kBmtFontCacheName);
}

TextManager::LocalizedStringMap TextManager::createLocalizedStrings() {
	LocalizedStringMap strings;

	{
		LanguageStrings &strs = strings[kRestoredLevelLabel];
		// Brings original text for level 4 from the 'Logical Journey of the Zoombinis'
		strs[Common::EN_USA] = U"VERY, VERY HARD";
		strs[Common::DE_DEU] = U"TOTAL SCHWER";
		strs[Common::FR_FRA] = U"MISSION IMPOSSIBLE";
		strs[Common::JA_JPN] = U"とってもとっても難しい";
		strs[Common::PL_POL] = U"TRUDNIEJSZE NIŻ TRUDNE";
		// Exception: Korean text for level 4 have different translations in LJ and MR.
		strs[Common::KO_KOR] = U"삭제되었던 4단계"; // Custom texts to match sequel's level name translations.
	}

	{
		LanguageStrings &strs = strings[kMissingLevel4Help];
		strs[Common::EN_USA] = U"No help resource is available for level 4.";
		strs[Common::KO_KOR] = U"4단계는 도움말 리소스가 없습니다.";
	}

	{
		LanguageStrings &strs = strings[kConfirmCaseCollision];
		strs[Common::EN_USA] = U"A save that differs only in letter case exists.\nThe new save will overwrite it.\nAre you sure to overwrite?";
		strs[Common::KO_KOR] = U"대소문자만 다른 저장파일 이름이 있습니다.\n새 파일은 기존 저장파일을 덮어씁니다.\n덮어쓸까요?";
	}

	{
		LanguageStrings &strs = strings[kConfirmReadOnlyLoad];
		strs[Common::EN_USA] = U"This saved game will open in read-only mode.\nProgress will not be saved.\nContinue?";
		strs[Common::KO_KOR] = U"이 저장파일은 읽기 전용 모드로 열립니다.\n진행 상황은 저장되지 않습니다.\n계속할까요?";
	}

	return strings;
}

const Common::U32String &TextManager::getLocalizedString(Key key) const {
	return _strMap[key];
}

Common::U32String TextManager::toU32String(const Common::String &str) const {
	return str.decode(_vm->getRelInfo().getCodePage());
}

Common::String TextManager::fromU32String(const Common::U32String &str) const {
	return str.encode(_vm->getRelInfo().getCodePage());
}

Common::String TextManager::getZoombiniDisplayName(const Common::String &name) const {
	// Hebrew: Render Latin alphabets will be rendered as correlated Hebrew alphabets.
	if (_vm->isHebrew())
		return name;

	// Latin: Make every alphabet uppercase except 'i'.
	Common::String displayName = name;
	for (uint i = 0; i < displayName.size(); i++) {
		const char ch = displayName[i];
		if ('a' <= ch && ch <= 'z' && ch != 'i')
			displayName.setChar(toupper(ch), i);
	}
	return displayName;
}

bool TextManager::loadRussianTables() {
	static constexpr ExeTableSource kV11RussianSource = {kRussianExecutablePath, 790609, 0xB1000, 0xB1900};
	static constexpr uint kNameEntrySize = 8;
	Common::ScopedPtr<Common::SeekableReadStream> stream(_vm->openResourceFile(kV11RussianSource.fileName));
	if (!stream || stream->size() != kV11RussianSource.expectedSize)
		return false;

	Common::Array<byte> data;
	data.resize(kV11RussianSource.expectedSize);
	if (!stream->seek(0) || stream->read(data.data(), data.size()) != data.size() || stream->err())
		return false;
	if (data.size() < kV11RussianSource.namesOffset + ARRAYSIZE(_russianNames) * kNameEntrySize || data.size() < kV11RussianSource.keyboardOffset + sizeof(_russianKeyboard))
		return false;

	const char *names = reinterpret_cast<const char *>(data.data() + kV11RussianSource.namesOffset);
	for (uint i = 0; i < ARRAYSIZE(_russianNames); i++) {
		const char *name = names + i * kNameEntrySize;
		uint length = 0;
		while (length < kNameEntrySize && name[length] != 0) {
			if (static_cast<byte>(name[length]) < 0xC0)
				return false;
			length += 1;
		}
		if (length == 0 || length == kNameEntrySize)
			return false;
		_russianNames[i] = Common::String(name, length);
	}
	memcpy(_russianKeyboard, data.data() + kV11RussianSource.keyboardOffset, sizeof(_russianKeyboard));
	return true;
}

Common::String TextManager::generateZoombiniName() const {
	if (_vm->getLanguage() == Common::RU_RUS)
		return generateRussianZoombiniName();
	return generateLatinZoombiniName();
}

Common::String TextManager::generateLatinZoombiniName() const {
	static constexpr const char *kVowelPairs[] = {
		"a ", "a ", "a ", "e ", "e ", "e ", "e ", "i ", "i ", "i ",
		"o ", "o ", "o ", "u ", "u ", "y ", "ee", "oo", "yo", "ya",
		"ye", "ei", "ie", "ai", "ia", "au", "ua", "uo", "ou", "ae"};
	static constexpr char kSingleConsonants[] = "bbccdddfghjkkllmmnnprrssssttvwx";
	static constexpr char kEndings[] = "aeiou";
	static constexpr const char *kConsonantPairs[] = {
		"bl", "br", "ch", "cl", "cr", "dr", "dw", "fl", "fr", "gh",
		"gl", "gr", "kl", "kn", "kr", "kw", "ld", "mp", "nd", "nh",
		"nn", "ph", "pl", "pr", "qu", "qu", "rh", "rn", "sc", "sl",
		"sm", "sn", "sp", "sr", "st", "sw", "th", "tr", "tw"};

	Common::String name;
	const uint targetLength = _vm->_rnd->getRandomNumber(1) + 4; // 4 or 5
	bool useVowelPair = _vm->_rnd->getRandomNumber(98) + 1 < 40; // ~40%
	while (name.size() < targetLength) {
		bool usedConsonantPair = false;
		if (useVowelPair) {
			useVowelPair = false;
			const char *pair = kVowelPairs[_vm->_rnd->getRandomNumber(ARRAYSIZE(kVowelPairs) - 1)];
			name += pair[0];
			if (pair[1] != ' ')
				name += pair[1];
		} else {
			useVowelPair = true;
			if (1 < name.size() || _vm->_rnd->getRandomNumber(98) + 1 <= 33) { // ~33%
				const char *pair = kConsonantPairs[_vm->_rnd->getRandomNumber(ARRAYSIZE(kConsonantPairs) - 1)];
				name += pair[0];
				name += pair[1];
				usedConsonantPair = true;
			} else {
				name += kSingleConsonants[_vm->_rnd->getRandomNumber(ARRAYSIZE(kSingleConsonants) - 2)];
			}
		}
		if (usedConsonantPair && targetLength <= name.size())
			name.setChar(kEndings[_vm->_rnd->getRandomNumber(ARRAYSIZE(kEndings) - 2)], name.size() - 1);
		if (name.size() == 2 && name[0] == name[1])
			name.deleteLastChar();
	}
	return name;
}

Common::String TextManager::generateRussianZoombiniName() const {
	const uint index = _vm->_rnd->getRandomNumber(ARRAYSIZE(_russianNames) - 1);
	return _russianNames[index];
}

uint32 TextManager::mapRussianNameKey(uint32 keyCode) const {
	if (0x0410 <= keyCode && keyCode <= 0x044F)
		return keyCode - 0x0410 + 0xC0;
	if (32 <= keyCode && keyCode <= 127)
		return _russianKeyboard[keyCode];
	return keyCode;
}

bool TextManager::isValidNameCharacter(uint32 keyCode) const {
	/** Allowed release-byte characters for savefile-name keyboard entry. */
	static constexpr const char *kValidNameCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
	/** Characters accepted by the Hebrew release in addition to its Hebrew byte range. */
	static constexpr const char *kHebrewValidNameCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,.; ";
	const Common::Language language = _vm->getLanguage();
	if (language == Common::HE_ISR)
		return (0xE0 <= keyCode && keyCode <= 0xFA) || (keyCode < 128 && strchr(kHebrewValidNameCharacters, static_cast<char>(keyCode)));
	if (language == Common::RU_RUS)
		return (0xC0 <= keyCode && keyCode <= 0xFF) || ('0' <= keyCode && keyCode <= '9') || keyCode == ' ' || keyCode == '_';
	return keyCode < 128 && strchr(kValidNameCharacters, static_cast<char>(keyCode));
}

bool TextManager::loadTextFont() {
	if (hasTextFont())
		return true;
	BmtFont *font = new BmtFont(_vm);
	if (!font->load(Common::Path(kTextFontPath))) {
		delete font;
		return false;
	}
	FontMan.assignFontToName(kBmtFontCacheName, font);
	return true;
}

bool TextManager::hasTextFont() const {
	return bmtFont() != nullptr;
}

const BmtFont *TextManager::bmtFont() const {
	return static_cast<const BmtFont *>(FontMan.getFontByName(kBmtFontCacheName));
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
