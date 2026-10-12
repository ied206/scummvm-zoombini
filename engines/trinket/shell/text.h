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

#ifndef TRINKET_SHELL_TEXT_H
#define TRINKET_SHELL_TEXT_H

#include "common/array.h"
#include "common/str.h"
#include "graphics/font.h"

namespace Trinket {
namespace Shell {

/**
 * RText layout using @ref Graphics::Font metrics.
 * Keep native byte-range splitting for whitespace and game/launcher CR handling;
 * @ref Graphics::Font::wordWrapText produces different line ranges.
 * Preserve native final single-character suppression for layout fidelity.
 */
class Text {
public:
	enum Rules {
		kGame,
		kLauncher,
	};
	static bool wrap(const Graphics::Font &font, Common::String text, int width, Common::Array<Common::String> &lines,
					 Rules rules, bool wrapping = true, bool outlined = false);
	static int firstBaseline(int height, int lineCount, int ascent, int descent, int leading);
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
