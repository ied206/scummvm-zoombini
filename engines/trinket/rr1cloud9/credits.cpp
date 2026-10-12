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

#include "trinket/rr1cloud9/credits.h"

#include "common/tokenizer.h"
#include "trinket/ompp/xml.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

Credits::Credits(TrinketEngine &vm) : _vm(vm), _voice(vm.mixer()), _paths(vm.resources()), _scene(vm.resources()), _images(vm.resources()) {}

Credits::~Credits() {
	for (auto &row : _rows)
		for (auto *text : row.text)
			delete text;
}

void Credits::addRow(int height) {
	Row row;
	row.offset = _length;
	row.height = height;
	_rows.push_back(row);
	_length += height + 2;
}

void Credits::addText(const Common::String &value, int x, bool center) {
	Shell::TextObject *text = new Shell::TextObject(_paths, _font);
	text->create({Core::ScriptValue::fromText(value), Core::ScriptValue(x), Core::ScriptValue(396),
				  Core::ScriptValue(535 - x), Core::ScriptValue(_rows.back().height), Core::ScriptValue(0)});
	text->set("center", {}, Core::ScriptValue(center));
	text->set("color", {Core::ScriptValue(65535), Core::ScriptValue(63487)}, Core::ScriptValue(59647));
	_rows.back().text.push_back(text);
}

bool Credits::enter(bool ending) {
	Common::ScopedPtr<Common::SeekableReadStream> font(_vm.resources().open("common/1014.nfnt"));
	_font.reset(font ? OM::NFNTFont::load(*font) : nullptr);
	OM::XMLParser xml;
	if (!_font || !xml.loadStream(_vm.resources().open("r1credits.xml")) || !xml.parse() || !_scene.add("common/6550.ao", 0))
		return false;
	if (ending) {
		if (!_scene.add("common/6501.ao", 1, 1, true) || !_scene.add("common/6502.ao", 1, 2, true))
			return false;
	} else if (!_scene.add("common/6503.ao", 1, 3, true)) {
		return false;
	}
	const auto &elements = xml.elements();
	for (uint i = 0; i < elements.size(); i++) {
		const auto &element = elements[i];
		if (element.path == "Root_Element/Section") {
			addRow();
			addRow();
			addText(element.attributes.getValOrDefault("title"), 120, true);
			addRow();
		} else if (element.path == "Root_Element/Section/member") {
			addRow();
			addText(xml.child(i, "title"));
			addText(xml.child(i, "name"), 350);
		} else if (element.path == "Root_Element/case1") {
			addRow(16);
		} else if (element.path == "Root_Element/case1/text") {
			addRow(16);
			addText(element.value);
		} else if (element.path == "Root_Element/case1/imgpair") {
			addRow(16);
			Common::StringTokenizer tokens(element.value);
			while (!tokens.empty()) {
				int id = atoi(tokens.nextToken().c_str());
				if (tokens.empty())
					return false;
				int x = atoi(tokens.nextToken().c_str());
				auto *image = _images.add(Common::String::format("common/%d.ao", id), 0);
				if (!image)
					return false;
				image->x = x;
				_rows.back().images.push_back(image);
				_rows.back().height = MAX(_rows.back().height, image->height());
			}
			_length = _rows.back().offset + _rows.back().height + 2;
		}
	}
	if (_rows.empty())
		return false;
	_vm.music().start("common/1050.wav", 100);
	if (ending)
		_voice.play(_vm.resources(), "comspch/1913.wav", Audio::Mixer::kSpeechSoundType);
	return true;
}

void Credits::draw(OM::Canvas &canvas, uint32 time) {
	if (!_started) {
		_due = time + 1;
		_started = true;
	} else if (_scroll < _length - 2 && static_cast<int32>(time - _due) >= 0) {
		_scroll += 2;
		_due = time + 30;
	}
	Common::Array<uint16> sounds;
	_scene.update(time, sounds);
	_scene.draw(canvas);
	_contents.surface().copyRectToSurface(canvas.surface(), 0, 0, Common::Rect(640, 480));
	for (auto &row : _rows) {
		int y = 396 + row.offset - _scroll;
		for (auto *image : row.images) {
			image->y = y;
			image->visible = y < 396 && 84 < y + row.height;
		}
		if (396 <= y || y + row.height <= 84)
			continue;
		for (auto *text : row.text) {
			text->y = y;
			text->draw(_contents);
		}
	}
	_images.drawSprites(_contents, INT_MIN, INT_MAX);
	canvas.surface().copyRectToSurface(_contents.surface(), 120, 84, Common::Rect(120, 84, 535, 396));
}

} // namespace RR1Cloud9
} // namespace Trinket
