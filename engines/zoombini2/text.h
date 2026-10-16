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

#ifndef ZOOMBINI2_TEXT_H
#define ZOOMBINI2_TEXT_H

#include "common/hashmap.h"
#include "common/language.h"
#include "common/noncopyable.h"
#include "common/str-enc.h"
#include "common/str.h"
#include "common/ustr.h"

#include "graphics/font.h"

#include "zoombini2/graphics.h"

namespace Zoombini2 {

class Zoombini2Engine;

/**
 * Localized UI messages, game text fonts, and release-encoded conversion.
 * Retains the bitmap strip and resolves the font for one @ref Gfx::FontKind.
 * Selects added UI messages in the game language, using English when a translation is unavailable.
 */
class Text : private Common::NonCopyable {
public:
	/** Semantic keys for UI messages added by ScummVM. */
	enum Key {
		/** Practice-map label for the restored level 4 tab. */
		kRestoredLevelLabel,
		/** Help-dialog notice shown when level 4 has no help sheet. */
		kMissingLevel4Help,
		/** Confirmation before overwriting a save whose name differs only in letter case. */
		kConfirmCaseCollision,
		/** Confirmation before opening a save without retaining progress. */
		kConfirmReadOnlyLoad
	};

	/** Create the text manager for @p vm and select its localized UI messages. */
	explicit Text(Zoombini2Engine *vm);
	/** Release cached messages and the bitmap strip. */
	~Text();

	/** Return the cached message for @p key in the game language or English, valid until this text manager is destroyed. */
	const Common::U32String &getLocalizedString(Key key) const;

	/** Return the main text encoding for @p language. */
	static Common::CodePage gameCodePage(Common::Language language);
	/** Decode release-encoded game bytes to Unicode with the release encoding. */
	static Common::U32String toU32String(const Common::String &str, Common::Language language);
	/** Encode Unicode text to release-encoded game bytes with the release encoding. */
	static Common::String fromU32String(const Common::U32String &str, Common::Language language);

	/** Load the shared text strip on demand; failed loads can be retried by the caller. */
	bool loadTextFont();
	/** Return whether the shared text strip is ready. */
	bool hasTextFont() const;
	/** Return the loaded strip, or null when it is unavailable. */
	BmtFont *bmtFont() const;
	/** Resolve the font selected by @p kind, or null when it is unavailable. */
	const Graphics::Font *resolveFont(Gfx::FontKind kind) const;

private:
	/** Translations for one UI message, indexed by game language. */
	typedef Common::HashMap<Common::Language, const char32_t *, Common::Hash<int>> LanguageStrings;
	/** UI message translations indexed first by semantic key, then by language. */
	typedef Common::HashMap<Key, LanguageStrings, Common::Hash<int>> LocalizedStringMap;
	/** Cached Unicode messages indexed by semantic key. */
	typedef Common::HashMap<Key, Common::U32String, Common::Hash<int>> StringMap;

	/** Build the UI message table, including an English fallback for every key. */
	static LocalizedStringMap createLocalizedStrings();

	/** Bitmap strip shared by all UI text colors. */
	static constexpr const char *kTextFontPath = "bmp/typo";
	/** Single coverage-mask glyph set tinted per draw, retained until graphics shutdown. */
	BmtFont *_bmtFont = nullptr;
	/**
	 * Whether the shared UI font load has been attempted once for this game instance.
	 * This flag cache only the load decision so the font file loads once while callers query per frame.
	 */
	mutable bool _uiFontResolved = false;
	/**
	 * Whether the theme supplies a language extra font for this game instance.
	 * This flag cache only the load decision so the font file loads once while callers query per frame.
	 */
	mutable bool _uiFontHasExtra = false;
	/** Game-language UI messages selected at construction, with English used for missing translations. */
	StringMap _strMap;
	/** Borrowed game instance used for resource resolution. */
	Zoombini2Engine *_vm;
};

} // End of namespace Zoombini2

#endif
