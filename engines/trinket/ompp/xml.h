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

#ifndef TRINKET_OMPP_XML_H
#define TRINKET_OMPP_XML_H

#include "common/array.h"
#include "common/formats/xmlparser.h"

namespace Trinket {
namespace OM {

/** Slash-addressed XML properties used by launcher layout and user records. */
class XMLParser : public Common::XMLParser {
public:
	struct Element {
		Common::String path, value;
		Common::StringMap attributes;
		int parent;
	};
	XMLParser() { setAllowText(); }
	/** Encode text or an attribute value without escaping introduced entity markers. */
	static Common::String escape(const Common::String &value);
	bool loadFile(const Common::Path &filename);
	bool loadFile(const Common::FSNode &node);
	bool loadStream(Common::SeekableReadStream *stream, const Common::String &name = "File Stream");
	bool loadBuffer(const byte *buffer, uint32 size, DisposeAfterUse::Flag disposable = DisposeAfterUse::NO);
	bool parse();
	const Common::Array<Element> &elements() const { return _elements; }
	Common::String child(uint parent, const Common::String &name) const {
		if (parent >= _elements.size())
			return Common::String();
		Common::String wanted = _elements[parent].path + "/" + name;
		for (uint i = parent + 1; i < _elements.size(); i++) {
			if (_elements[i].parent < static_cast<int>(parent))
				break;
			if (_elements[i].path == wanted)
				return _elements[i].value;
		}
		return Common::String();
	}
	Common::String get(const Common::String &path) const { return _values.getValOrDefault(path); }
	int number(const Common::String &path) const { return atoi(get(path).c_str()); }
	const Common::StringMap &values() const { return _values; }

protected:
	bool isValidNameChar(char c) override {
		return Common::XMLParser::isValidNameChar(c) || c == '-';
	}
	struct Layout : XMLKeyLayout {
		bool doCallback(Common::XMLParser *, ParserNode *) override { return true; }
	};
	void buildLayout() override { _XMLkeys = new Layout; }
	bool handleUnknownKey(ParserNode *node) override {
		node->layout = _XMLkeys;
		return keyCallback(node);
	}
	bool keyCallback(ParserNode *node) override {
		if (_values.size() >= 4096 || _elements.size() >= 16384)
			return false;
		_stack.push_back(node->name);
		_values[path()] = "";
		Element element;
		element.path = path();
		element.attributes = node->values;
		element.parent = _parents.empty() ? -1 : _parents.back();
		_parents.push_back(_elements.size());
		_elements.push_back(element);
		return true;
	}
	bool closedKeyCallback(ParserNode *node) override {
		if (!node->header && !_stack.empty()) {
			_stack.pop_back();
			_parents.pop_back();
		}
		return true;
	}
	bool textCallback(const Common::String &value) override {
		// The input adapter guards text against the base parser's whitespace skipping.
		if (value.empty() || value[0] != '~')
			return false;
		Common::String text = value.substr(1);
		_values[path()] += text;
		if (!_parents.empty())
			_elements[_parents.back()].value += text;
		return true;
	}
	void cleanup() override {
		_stack.clear();
		_values.clear();
		_parents.clear();
		_elements.clear();
	}

private:
	static constexpr uint32 kMaxSourceSize = 1024 * 1024;
	static Common::String guardText(const Common::String &source);
	static bool validateMarkup(const Common::String &source);
	static bool isMarkupNameChar(char c) { return Common::isAlnum(c) || c == '_' || c == '-'; }
	bool _validMarkup = false;
	Common::String _sourceName;
	// Preserve repeated siblings and document order for launcher content areas.
	Common::Array<Element> _elements;
	Common::Array<int> _parents;
	Common::StringMap _values;
	Common::Array<Common::String> _stack;
	Common::String path() const {
		Common::String result;
		for (uint i = 0; i < _stack.size(); i++) {
			if (i)
				result += '/';
			result += _stack[i];
		}
		return result;
	}
};

} // End of namespace OM
} // End of namespace Trinket

#endif
