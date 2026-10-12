/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/shell/xml_file.h"

#include "common/archive.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/system.h"

namespace Trinket {
namespace Shell {

Core::ScriptableObject *XMLFile::Factory::create(const Core::ScriptArguments &arguments) {
	if (arguments.size() != 1)
		return nullptr;
	XMLFile *file = new XMLFile();
	if (!file->load(arguments[0].text())) {
		delete file;
		return nullptr;
	}
	return file;
}

bool XMLFile::load(const Common::String &path) {
	_filename = path;
	if (!_filename.contains('.'))
		_filename += ".xml";
	Common::ScopedPtr<Common::SeekableReadStream> stream(SearchMan.createReadStreamForMember(Common::Path(_filename)));
	if (!stream)
		stream.reset(g_system->getSavefileManager()->openForLoading(Common::Path(_filename).getLastComponent().toString()));
	_elements.clear();
	_selected = -1;
	if (stream) {
		OM::XMLParser document;
		if (!document.loadStream(stream.release(), _filename) || !document.parse() || document.elements().empty())
			return false;
		_elements = document.elements();
	} else {
		OM::XMLParser::Element root;
		root.parent = -1;
		_elements.push_back(root);
	}
	_selected = 0;
	return true;
}

bool XMLFile::loadText(const Common::String &text) {
	OM::XMLParser document;
	_filename.clear();
	_selected = -1;
	_elements.clear();
	if (!document.loadBuffer(reinterpret_cast<const byte *>(text.c_str()), text.size()) || !document.parse() || document.elements().empty())
		return false;
	_elements = document.elements();
	_selected = 0;
	return true;
}

void XMLFile::addNode(const Common::String &name, const Common::String &value) {
	if (_selected < 0)
		return;
	uint next = _selected + 1;
	while (next < _elements.size() && _selected <= _elements[next].parent)
		next += 1;
	OM::XMLParser::Element node;
	node.parent = _selected;
	node.path = _elements[_selected].path;
	if (!node.path.empty())
		node.path += '/';
	node.path += name;
	node.value = value;
	for (uint i = next; i < _elements.size(); i++)
		if (static_cast<int>(next) <= _elements[i].parent)
			_elements[i].parent += 1;
	_elements.insert_at(next, node);
}

void XMLFile::writeNode(Common::String &text, uint index) const {
	Common::String name = tag(index);
	if (!name.empty()) {
		text += "<" + name;
		for (auto it = _elements[index].attributes.begin(); it != _elements[index].attributes.end(); ++it)
			text += " " + it->_key + "=\"" + OM::XMLParser::escape(it->_value) + "\"";
		text += ">" + OM::XMLParser::escape(_elements[index].value);
	}
	for (uint i = index + 1; i < _elements.size(); i++) {
		if (_elements[i].parent < static_cast<int>(index))
			break;
		if (_elements[i].parent == static_cast<int>(index))
			writeNode(text, i);
	}
	if (!name.empty())
		text += "</" + name + ">\n";
}

bool XMLFile::write() const {
	if (_filename.empty() || _elements.empty())
		return true;
	Common::String text = "<?xml version=\"1.0\"?>\n";
	writeNode(text, 0);
	Common::ScopedPtr<Common::OutSaveFile> stream(
		g_system->getSavefileManager()->openForSaving(Common::Path(_filename).getLastComponent().toString()));
	if (!stream || stream->write(text.c_str(), text.size()) != text.size())
		return false;
	stream->finalize();
	return !stream->err();
}

Common::String XMLFile::tag(uint index) const {
	const Common::String &path = _elements[index].path;
	uint slash = path.findLastOf('/');
	return slash == Common::String::npos ? path : path.substr(slash + 1);
}

Common::Array<int> XMLFile::children() const {
	Common::Array<int> result;
	if (_selected < 0)
		return result;
	const Common::Array<OM::XMLParser::Element> &elements = _elements;
	for (uint i = _selected + 1; i < elements.size(); i++) {
		if (elements[i].parent < _selected)
			break;
		if (elements[i].parent == _selected)
			result.push_back(i);
	}
	return result;
}

Common::Array<Common::String> XMLFile::content() const {
	Common::Array<Common::String> result;
	if (_selected < 0)
		return result;
	const Common::String &text = _elements[_selected].value;
	uint position = 0;
	while (position < text.size()) {
		while (position < text.size() && Common::isSpace(text[position]))
			position += 1;
		if (text.size() <= position)
			break;
		char quote = text[position] == '"' || text[position] == '\'' ? text[position++] : 0;
		uint start = position;
		while (position < text.size() && (quote ? text[position] != quote : !Common::isSpace(text[position])))
			position += 1;
		result.push_back(text.substr(start, position - start));
		if (quote && position < text.size())
			position += 1;
	}
	return result;
}

bool XMLFile::get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (property == "childCount") {
		result = Core::ScriptValue(static_cast<int32>(children().size()));
	} else if (property == "nodeTag") {
		result = Core::ScriptValue(_selected < 0 ? Common::String() : tag(_selected));
	} else if (property == "contentCount") {
		result = Core::ScriptValue(static_cast<int32>(content().size()));
	} else if (property == "contentString") {
		Common::Array<Common::String> values = content();
		int index = arguments.empty() ? 0 : MAX<int32>(0, arguments[0].integer() - 1);
		result = Core::ScriptValue::fromText(static_cast<uint>(index) < values.size() ? values[index] : Common::String());
	} else if (property == "attributeValue" && !arguments.empty()) {
		Common::String value;
		if (0 <= _selected)
			value = _elements[_selected].attributes.getValOrDefault(arguments[0].text());
		result = Core::ScriptValue::fromText(value);
	} else if (property == "getVariable" && !arguments.empty()) {
		Common::String name = arguments[0].text();
		if (1 < arguments.size() && 0 < arguments[1].integer())
			name += Common::String::format(".%d", arguments[1].integer());
		Common::String value;
		for (int child : children())
			if (tag(child) == name) {
				value = _elements[child].value;
				break;
			}
		result = Core::ScriptValue::fromText(value);
	} else {
		return ScriptableObject::get(property, arguments, result);
	}
	return true;
}

bool XMLFile::play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) {
	if (method == "addNode" && arguments.size() == 1) {
		addNode(arguments[0].text());
	} else if (method == "addVariable" && arguments.size() == 2) {
		addNode(arguments[0].text(), arguments[1].text());
	} else if (method == "write" && arguments.empty()) {
		if (!write())
			return false;
	} else if (method == "selectRoot") {
		_selected = _elements.empty() ? -1 : 0;
	} else if (method == "selectParent") {
		if (0 <= _selected && 0 <= _elements[_selected].parent)
			_selected = _elements[_selected].parent;
	} else if (method == "selectChild" && !arguments.empty()) {
		Common::Array<int> values = children();
		int index = MAX<int32>(0, arguments[0].integer() - 1);
		if (static_cast<uint>(index) < values.size())
			_selected = values[index];
	} else if (method == "selectNode" && !arguments.empty()) {
		if (1 < arguments.size() && arguments[1].integer())
			_selected = _elements.empty() ? -1 : 0;
		Common::String name = arguments[0].text();
		if (_selected != 0 || _elements.empty() || name != tag(0)) {
			Common::Array<int> values = children();
			for (uint i = 0; i < values.size(); i++) {
				if (tag(values[i]) == name) {
					_selected = values[i];
					break;
				}
			}
		}
	} else {
		return ScriptableObject::play(method, arguments, result);
	}
	complete();
	return true;
}

} // namespace Shell
} // namespace Trinket
