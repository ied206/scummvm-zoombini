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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_RR1CLOUD9_CREDITS_H
#define TRINKET_RR1CLOUD9_CREDITS_H

#include "trinket/ompp/sound.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {

class TrinketEngine;
namespace RR1Cloud9 {

/** Authored XML credits, shared by the ending and the options panel. */
class Credits {
public:
	explicit Credits(TrinketEngine &vm);
	~Credits();
	bool enter(bool ending);
	void draw(OM::Canvas &canvas, uint32 time);
	int event(const Common::Event &event) { return _scene.event(event); }

private:
	struct Row {
		Common::Array<Shell::TextObject *> text;
		Common::Array<Shell::Sprite *> images;
		int offset = 0, height = 24;
	};
	TrinketEngine &_vm;
	OM::Sound _voice;
	Shell::ResourcePaths _paths;
	Shell::Scene _scene, _images;
	Common::SharedPtr<OM::NFNTFont> _font;
	Common::Array<Row> _rows;
	OM::Canvas _contents{640, 480};
	int _scroll = 0, _length = 0;
	uint32 _due = 0;
	bool _started = false;
	void addRow(int height = 24);
	void addText(const Common::String &value, int x = 120, bool center = false);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
