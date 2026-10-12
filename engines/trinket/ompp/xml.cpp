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

#include "trinket/ompp/xml.h"

#include "common/archive.h"
#include "common/memstream.h"
#include "common/ptr.h"
#include "common/textconsole.h"

namespace Trinket {
namespace OM {

constexpr uint32 XMLParser::kMaxSourceSize;

Common::String XMLParser::escape(const Common::String &value) {
	Common::String result;
	for (uint i = 0; i < value.size(); i++) {
		switch (value[i]) {
		case '&':
			result += "&amp;";
			break;
		case '<':
			result += "&lt;";
			break;
		case '>':
			result += "&gt;";
			break;
		case '"':
			result += "&quot;";
			break;
		case '\'':
			result += "&apos;";
			break;
		default:
			result += value[i];
			break;
		}
	}
	return result;
}

bool XMLParser::loadFile(const Common::Path &filename) {
	return loadStream(SearchMan.createReadStreamForMember(filename), filename.toString());
}

bool XMLParser::loadFile(const Common::FSNode &node) {
	return loadStream(node.createReadStream(), node.getName());
}

bool XMLParser::loadBuffer(const byte *buffer, uint32 size, DisposeAfterUse::Flag disposable) {
	return loadStream(new Common::MemoryReadStream(buffer, size, disposable), "Memory Stream");
}

bool XMLParser::loadStream(Common::SeekableReadStream *stream, const Common::String &name) {
	Common::ScopedPtr<Common::SeekableReadStream> input(stream);
	Common::XMLParser::close();
	_validMarkup = false;
	_sourceName = name;
	if (!input || input->size() < 0 || kMaxSourceSize < input->size() || !input->seek(0))
		return false;
	Common::Array<byte> bytes;
	bytes.resize(input->size());
	if (!bytes.empty() && (input->read(bytes.data(), bytes.size()) != bytes.size() || input->err()))
		return false;
	Common::String source;
	if (!bytes.empty()) {
		// Embedded NUL must not silently truncate the document.
		if (memchr(bytes.data(), 0, bytes.size()))
			return false;
		source = Common::String(reinterpret_cast<const char *>(bytes.data()), bytes.size());
	}
	Common::String guarded = guardText(source);
	_validMarkup = validateMarkup(guarded);
	Common::MemoryReadStream prepared(reinterpret_cast<const byte *>(guarded.c_str()), guarded.size());
	return Common::XMLParser::loadStream(guarded.empty() ? new Common::MemoryReadStream(nullptr, 0) : prepared.readStream(guarded.size()), name);
}

bool XMLParser::parse() {
	// Reject invalid markup before the shared parser formats an unsafe tag span.
	if (!_validMarkup) {
		cleanup();
		warning("Trinket: invalid XML markup in %s", _sourceName.c_str());
		return false;
	}
	return Common::XMLParser::parse();
}

bool XMLParser::validateMarkup(const Common::String &source) {
	Common::Array<Common::String> tags;
	bool headerSeen = false;
	uint pos = 0;
	while (pos < source.size()) {
		if (source[pos] != '<') {
			pos += 1;
			continue;
		}
		if (source.substr(pos, 4) == "<!--") {
			uint end = source.find("-->", pos + 4);
			if (end == Common::String::npos)
				return false;
			pos = end + 3;
			continue;
		}
		pos += 1;
		if (pos == source.size())
			return false;
		bool closing = source[pos] == '/', header = source[pos] == '?';
		if (closing || header)
			pos += 1;
		uint start = pos;
		while (pos < source.size() && isMarkupNameChar(source[pos]))
			pos += 1;
		if (start == pos)
			return false;
		Common::String name = source.substr(start, pos - start);
		if (header && (headerSeen || name != "xml" || !tags.empty()))
			return false;
		Common::StringMap attributes;
		bool selfClosing = false;
		for (;;) {
			while (pos < source.size() && Common::isSpace(source[pos]))
				pos += 1;
			if (pos == source.size())
				return false;
			if (source[pos] == '/' || (header && source[pos] == '?')) {
				selfClosing = true;
				pos += 1;
			}
			if (pos < source.size() && source[pos] == '>') {
				pos += 1;
				break;
			}
			if (closing || selfClosing)
				return false;
			start = pos;
			while (pos < source.size() && isMarkupNameChar(source[pos]))
				pos += 1;
			if (start == pos)
				return false;
			Common::String attribute = source.substr(start, pos - start);
			if (attributes.contains(attribute))
				return false;
			attributes[attribute] = "";
			while (pos < source.size() && Common::isSpace(source[pos]))
				pos += 1;
			if (pos == source.size() || source[pos] != '=')
				return false;
			pos += 1;
			while (pos < source.size() && Common::isSpace(source[pos]))
				pos += 1;
			if (pos == source.size())
				return false;
			if (source[pos] == '\'' || source[pos] == '"') {
				char quote = source[pos];
				pos += 1;
				while (pos < source.size() && source[pos] != quote)
					pos += 1;
				if (pos == source.size())
					return false;
				pos += 1;
			} else {
				start = pos;
				while (pos < source.size() && isMarkupNameChar(source[pos]))
					pos += 1;
				if (start == pos)
					return false;
			}
		}
		if (header) {
			if (!selfClosing)
				return false;
			headerSeen = true;
		} else if (closing) {
			if (selfClosing || tags.empty() || tags.back() != name)
				return false;
			tags.pop_back();
		} else if (!selfClosing) {
			if (MAX_XML_DEPTH <= tags.size())
				return false;
			tags.push_back(name);
		}
	}
	return headerSeen && tags.empty();
}

Common::String XMLParser::guardText(const Common::String &source) {
	Common::String result;
	uint header = 0;
	while (header < source.size()) {
		if (Common::isSpace(source[header])) {
			header += 1;
		} else if (source.substr(header, 4) == "<!--") {
			uint end = source.find("-->", header + 4);
			if (end == Common::String::npos)
				break;
			header = end + 3;
		} else {
			break;
		}
	}
	// Authored game XML omits the declaration required by Common's parser.
	if (source.substr(header, 5) != "<?xml")
		result = "<?xml version=\"1.0\"?>\n";
	uint pos = 0;
	int depth = 0;
	while (pos < source.size()) {
		uint start = pos;
		if (source[pos] == '<') {
			if (source.substr(pos, 4) == "<!--") {
				uint end = source.find("-->", pos + 4);
				pos = end == Common::String::npos ? source.size() : end + 3;
			} else {
				char quote = 0;
				while (pos < source.size()) {
					char c = source[pos];
					pos += 1;
					if (quote) {
						if (c == quote)
							quote = 0;
					} else if (c == '\'' || c == '"') {
						quote = c;
					} else if (c == '>') {
						if (start + 1 < pos && source[start + 1] == '/')
							depth -= 1;
						else if (start + 1 < pos && source[start + 1] != '?' && source[start + 1] != '!' && source[pos - 2] != '/')
							depth += 1;
						break;
					}
				}
			}
		} else {
			bool hasText = false, hasLineBreak = false;
			while (pos < source.size() && source[pos] != '<') {
				hasText |= !Common::isSpace(source[pos]);
				hasLineBreak |= source[pos] == '\n' || source[pos] == '\r' || source[pos] == '\t';
				pos += 1;
			}
			// Keep formatting between elements out of property values. Spaces in leaf values are significant.
			if (0 < depth && (hasText || !hasLineBreak))
				result += '~';
		}
		result += source.substr(start, pos - start);
	}
	return result;
}

} // namespace OM
} // namespace Trinket
