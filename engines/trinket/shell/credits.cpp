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

#include "trinket/shell/credits.h"

#include "common/ptr.h"
#include "common/substream.h"
#include "common/tokenizer.h"
#include "trinket/core/animation.h"
#include "trinket/ompp/xml.h"

namespace Trinket {
namespace Shell {

CreditText::CreditText(int width, int height) : _width(width), _height(height) {
}

Common::String CreditText::textContent(const Common::String &source) {
	Common::String result, token;
	bool quoted = false;
	for (uint i = 0; i <= source.size(); i++) {
		char c = i < source.size() ? source[i] : 0;
		if (c == '"') {
			quoted = !quoted;
		} else if (!c || (!quoted && (c == ' ' || c == '\t' || c == '\r' || c == '\n'))) {
			if (!token.empty()) {
				result += token + ' ';
				token.clear();
			}
		} else {
			token += c;
		}
	}
	for (uint i = 0; i < result.size(); i++)
		if (static_cast<byte>(result[i]) == 194)
			result.setChar(' ', i);
	return result;
}

bool CreditText::loadImage(const OM::ResourceProvider &resources, const char *pattern, uint image, Graphics::Surface &surface) {
	Common::String path = Common::String::format(pattern, image);
	Common::ScopedPtr<Common::SeekableReadStream> stream(resources.open(path));
	Core::Animation animation;
	if (!stream || !animation.load(*stream))
		return false;
	Common::ScopedPtr<Common::SeekableReadStream> palette(resources.open(path.substr(0, path.size() - 3) + ".rgb"));
	if (palette && !animation.loadPalette(*palette))
		return false;
	return animation.decode(0, surface);
}

bool CreditText::load(Common::SeekableReadStream &stream, const OM::NFNTFont &font, const OM::ResourceProvider &resources, const char *imagePattern) {
	if (_content.getPixels() || _width <= 0 || _height <= 0)
		return false;
	OM::XMLParser xml;
	if (stream.pos() < 0 || stream.size() < stream.pos() || 0xFFFFFFFFULL < static_cast<uint64>(stream.size()) ||
		!xml.loadStream(new Common::SeekableSubReadStream(&stream, stream.pos(), stream.size(), DisposeAfterUse::NO)) || !xml.parse())
		return false;
	Common::Array<Entry> entries;
	const auto &nodes = xml.elements();
	int y = 0, maximumNameWidth = 0;
	for (uint i = 0; i < nodes.size(); i++) {
		const auto &node = nodes[i];
		if (node.path == "Root_Element/Section") {
			Entry entry;
			entry.y = y + 24;
			entry.x = -1;
			entry.text = node.attributes.getValOrDefault("title");
			entries.push_back(entry);
			y += 72;
		} else if (node.path == "Root_Element/Section/member") {
			Entry entry;
			entry.y = y;
			entry.text = textContent(xml.child(i, "title"));
			entry.name = textContent(xml.child(i, "name"));
			maximumNameWidth = MAX(maximumNameWidth, font.getStringWidth(entry.name));
			entries.push_back(entry);
			y += 24;
		} else if (node.path == "Root_Element/case1") {
			y += 40;
		} else if (node.path == "Root_Element/case1/text") {
			Entry entry;
			entry.y = y;
			entry.text = textContent(node.value);
			entries.push_back(entry);
			y += 16;
		} else if (node.path == "Root_Element/case1/imgpair") {
			Common::StringTokenizer values(node.value);
			int height = 0;
			for (uint n = 0; n < 2; n++) {
				Entry entry;
				if (values.empty())
					return false;
				entry.image = atoi(values.nextToken().c_str());
				if (values.empty() || entry.image < 0)
					return false;
				entry.x = atoi(values.nextToken().c_str());
				entry.y = y;
				Graphics::Surface image;
				if (!loadImage(resources, imagePattern, entry.image, image))
					return false;
				height = MAX<int>(height, image.h);
				image.free();
				entries.push_back(entry);
			}
			_maximumStep = MAX(_maximumStep, height);
			y += height;
		}
	}
	if (entries.empty() || 30000 < y + _height)
		return false;
	_finish = y - _height + 24;
	OM::Canvas content(_width, MAX(y + 24, _height + _maximumStep));
	content.surface().fillRect(Common::Rect(content.surface().w, content.surface().h), 0);
	uint32 black = content.surface().format.ARGBToColor(255, 0, 0, 0);
	for (uint i = 0; i < entries.size(); i++) {
		const Entry &entry = entries[i];
		int x = entry.x;
		if (x == -1)
			x = (_width - font.getStringWidth(entry.text)) / 2;
		if (0 <= entry.image) {
			Graphics::Surface image;
			if (!loadImage(resources, imagePattern, entry.image, image))
				return false;
			content.blit(image, x, entry.y);
			image.free();
		} else {
			font.drawString(&content.surface(), entry.text, x, entry.y, _width - x, black, Graphics::kTextAlignLeft, 0, false, true);
			int nameX = _width - maximumNameWidth - 2;
			font.drawString(&content.surface(), entry.name, nameX, entry.y, _width - nameX, black, Graphics::kTextAlignLeft, 0, false, true);
		}
	}
	_content.copyFrom(content.surface());
	return true;
}

void CreditText::start(uint32 now, uint32 delay) {
	_started = true;
	if (delay)
		_deadline = now + delay;
	else
		_running = true;
}

bool CreditText::update(uint32 now) {
	if (_deadline && _deadline < now) {
		_running = true;
		_deadline = 0;
		_lastUpdate = now;
	}
	if (!_running || _paused)
		return false;
	uint32 elapsed = _lastUpdate ? now - _lastUpdate : 0;
	if (!_lastUpdate)
		_lastUpdate = now;
	uint32 step = elapsed * 60 / 1000;
	if (!step)
		return false;
	_position += MIN<uint32>(step, _maximumStep);
	_lastUpdate = now;
	if (_finish <= _position) {
		_running = false;
		return true;
	}
	return false;
}

void CreditText::pause(uint32 now) {
	_paused = true;
	_pauseTime = now;
}

void CreditText::resume(uint32 now) {
	_paused = false;
	if (_deadline)
		_deadline += now - _pauseTime;
	else if (_running)
		_lastUpdate += now - _pauseTime;
}

void CreditText::draw(OM::Canvas &canvas, int x, int y) const {
	if (!_started || !_content.getPixels() || _content.h <= _position)
		return;
	Graphics::Surface visible = _content.getSubArea(Common::Rect(0, _position, _width, MIN<int>(_position + _height, _content.h)));
	canvas.blit(visible, x, y);
}

} // namespace Shell
} // namespace Trinket
