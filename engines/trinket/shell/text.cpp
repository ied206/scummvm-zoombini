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

#include "trinket/shell/text.h"

namespace Trinket {
namespace Shell {

bool Text::wrap(const Graphics::Font &font, Common::String text, int width, Common::Array<Common::String> &lines,
				Rules rules, bool wrapping, bool outlined) {
	lines.clear();
	if (rules == kLauncher) {
		for (uint i = 0; i < text.size(); i++) {
			if (text[i] == '\\') {
				// The original pre-parser cannot advance past other backslash sequences.
				if (text.size() <= i + 1 || text[i + 1] != 'n')
					return false;
				text[i] = ' ';
				text[i + 1] = '\n';
			}
		}
	}
	if (!wrapping) {
		lines.push_back(text);
		return true;
	}
	int position = 0, start = 0, end = -1, lastSpace = 0;
	bool inSpace = false;
	while (position < static_cast<int>(text.size())) {
		char c = text[position];
		if (c == '\n' || (c == '\r' && rules == kGame)) {
			lines.push_back(text.substr(start, position - start));
			position += 1;
			start = position;
			inSpace = false;
		} else if (c == '\r' && rules == kLauncher) {
			position += 1;
		} else if (c == ' ' || c == '\t') {
			if (start == position) {
				start += 1;
			} else {
				if (!inSpace) {
					lastSpace = position;
					end = position - 1;
				}
				inSpace = true;
			}
			position += 1;
		} else if (font.getStringWidth(text.substr(start, position - start + 1)) + (outlined ? 2 : 0) <= width) {
			position += 1;
			inSpace = false;
		} else {
			if (start < lastSpace) {
				lines.push_back(text.substr(start, lastSpace - start));
				position = lastSpace;
				start = lastSpace + 1;
			} else {
				// Reject a box that cannot hold one glyph instead of looping forever.
				if (position == start) {
					lines.clear();
					return false;
				}
				lines.push_back(text.substr(start, position - start));
				start = position;
			}
			inSpace = false;
		}
	}
	if (!inSpace)
		end = position - 1;
	// A final one-character line can disappear, even after an explicit newline.
	// This appears to be an aesthetic choice to avoid an isolated final glyph;
	// retain the native behavior. Empty final lines are still kept.
	if (start != end || lines.empty())
		lines.push_back(end < start ? Common::String() : text.substr(start, end - start + 1));
	return true;
}

int Text::firstBaseline(int height, int lineCount, int ascent, int descent, int leading) {
	int baseline = height / 2 - (lineCount - 1) / 2 * ascent - (lineCount - 1) * leading / 2 - lineCount / 2 * descent;
	if (lineCount % 2)
		baseline += ascent / 2 - descent / 2;
	return baseline;
}

} // namespace Shell
} // namespace Trinket
