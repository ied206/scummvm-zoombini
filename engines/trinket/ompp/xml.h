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
#include "trinket/string_parser.h"

namespace Trinket {
namespace OM {

/** Slash-addressed XML properties used by launcher layout and user records. */
class XMLParser : public Common::XMLParser {
public:
	struct Element {
		/** Slash-separated path from the document root to this element. */
		Common::String path;
		/** Decoded text accumulated inside this element. */
		Common::String value;
		/** Decoded attribute names and values belonging to this element. */
		Common::StringMap attrs;
		/** Index of the parent element, or minus one for a root element. */
		int parent;
	};
	/** Enable element text collection for slash-addressed properties. */
	XMLParser() { setAllowText(); }
	/** Encode text or an attribute value without escaping introduced entity markers. */
	static Common::String escape(const Common::String &value);
	/** Load and validate XML from the supplied filesystem path or node. */
	bool loadFile(const Common::Path &filename);
	/** Load and validate XML from the supplied filesystem path or node. */
	bool loadFile(const Common::FSNode &node);
	/** Load XML from a stream and release the stream after reading it. */
	bool loadStream(Common::SeekableReadStream *stream, const Common::String &name = "File Stream");
	/** Load XML bytes and release the buffer only when requested by the disposal flag. */
	bool loadBuffer(const byte *buffer, uint32 size, DisposeAfterUse::Flag disposable = DisposeAfterUse::NO);
	/** Parse the validated source into path values and ordered element records. */
	bool parse();
	/** Return the elements in document order, including repeated siblings. */
	const Common::Array<Element> &elements() const { return _elements; }
	/** Return the named child element's text, or an empty string when it is absent. */
	Common::String child(uint parent, const Common::String &name) const {
		if (parent >= _elements.size())
			return Common::String();
		Common::String wanted = _elements[parent].path + "/" + name;
		for (uint i = parent + 1; i < _elements.size(); i++) {
			if (_elements[i].parent < static_cast<int>(parent))
				break;
			if (_elements[i].path.equals(wanted))
				return _elements[i].value;
		}
		return Common::String();
	}
	/** Return the text at a slash-separated path, or an empty string when it is absent. */
	Common::String get(const Common::String &path) const { return _values.getValOrDefault(path); }
	/** Parse a path value as a decimal integer and return its conversion status. */
	StringParser::Result number(const Common::String &path, int32 &result) const {
		return StringParser::parseSignedInt(get(path).c_str(), result, StringParser::kDecimalProfile);
	}
	/** Return the decoded path-to-text lookup table. */
	const Common::StringMap &values() const { return _values; }

protected:
	/** Accept ordinary XML name characters and hyphens. */
	bool isValidNameChar(char c) override {
		return Common::XMLParser::isValidNameChar(c) || c == '-';
	}
	struct Layout : XMLKeyLayout {
		/** Accept elements through the parser's generic key callbacks. */
		bool doCallback(Common::XMLParser *, ParserNode *) override { return true; }
	};
	/** Set up the generic XML element layout. */
	void buildLayout() override { _XMLkeys = new Layout; }
	/** Route an unlisted element through the generic key callback. */
	bool handleUnknownKey(ParserNode *node) override {
		node->layout = _XMLkeys;
		return keyCallback(node);
	}
	/** Record a newly opened element and its attributes, enforcing document-size limits. */
	bool keyCallback(ParserNode *node) override {
		if (_values.size() >= 4096 || _elements.size() >= 16384)
			return false;
		_stack.push_back(node->name);
		_values[path()] = "";
		Element element;
		element.path = path();
		element.attrs = node->values;
		element.parent = _parents.empty() ? -1 : _parents.back();
		_parents.push_back(_elements.size());
		_elements.push_back(element);
		return true;
	}
	/** Pop the current element and path after a closing tag. */
	bool closedKeyCallback(ParserNode *node) override {
		if (!node->header && !_stack.empty()) {
			_stack.pop_back();
			_parents.pop_back();
		}
		return true;
	}
	/** Append guarded element text to its path value and ordered element record. */
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
	/** Clear decoded elements, paths and nesting state before another parse. */
	void cleanup() override {
		_stack.clear();
		_values.clear();
		_parents.clear();
		_elements.clear();
	}

private:
	/** XML entity encoding an ampersand. */
	static constexpr const char *kAmpEntity = "&amp;";
	/** XML entity encoding a less-than sign. */
	static constexpr const char *kLessThanEntity = "&lt;";
	/** XML entity encoding a greater-than sign. */
	static constexpr const char *kGreaterThanEntity = "&gt;";
	/** XML entity encoding a double quote. */
	static constexpr const char *kQuoteEntity = "&quot;";
	/** XML entity encoding an apostrophe. */
	static constexpr const char *kApostropheEntity = "&apos;";
	/** Processing-instruction target of an XML declaration. */
	static constexpr const char *kDeclarationTarget = "xml";

	/** Maximum accepted XML source size in bytes. */
	static constexpr uint32 kMaxSourceSize = 1024 * 1024;
	/** Protect text runs from the base parser's whitespace skipping. */
	static Common::String guardText(const Common::String &source);
	/** Check declarations, nesting and markup before passing input to the base parser. */
	static bool validateMarkup(const Common::String &source);
	/** Recognize the restricted characters accepted in markup names. */
	static bool isMarkupNameChar(char c) { return Common::isAlnum(c) || c == '_' || c == '-'; }
	/** Whether the loaded source passed markup validation. */
	bool _validMarkup = false;
	/** Source label used when reporting XML parse errors. */
	Common::String _sourceName;
	// Preserve repeated siblings and document order for launcher content areas.
	/** Element records preserving repeated siblings and document order. */
	Common::Array<Element> _elements;
	/** Indices of the currently open elements, from root to innermost parent. */
	Common::Array<int> _parents;
	/** Decoded text indexed by slash-separated element path. */
	Common::StringMap _values;
	/** Names of the currently open elements used to assemble paths. */
	Common::Array<Common::String> _stack;
	/** Build the slash-separated path of the currently open element. */
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
