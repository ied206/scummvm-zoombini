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

constexpr const char *XMLFile::kPropertyChildCount;
constexpr const char *XMLFile::kPropertyNodeTag;
constexpr const char *XMLFile::kPropertyContentCount;
constexpr const char *XMLFile::kPropertyContentString;
constexpr const char *XMLFile::kPropertyAttributeValue;
constexpr const char *XMLFile::kPropertyGetVariable;
constexpr const char *XMLFile::kSignedIndexFormat;
constexpr const char *XMLFile::kMethodAddNode;
constexpr const char *XMLFile::kMethodAddVariable;
constexpr const char *XMLFile::kMethodWrite;
constexpr const char *XMLFile::kMethodSelectRoot;
constexpr const char *XMLFile::kMethodSelectParent;
constexpr const char *XMLFile::kMethodSelectChild;
constexpr const char *XMLFile::kMethodSelectNode;

constexpr const char *XMLFile::kFileExtension;

Core::ScriptableObject *XMLFile::Factory::create(const Core::ScriptArguments &sargs) {
	if (sargs.size() != 1)
		return nullptr;
	XMLFile *file = new XMLFile();
	if (!file->load(sargs[0].toText())) {
		delete file;
		return nullptr;
	}
	return file;
}

bool XMLFile::load(const Common::String &path) {
	_filename = path;
	if (!_filename.contains('.'))
		_filename += kFileExtension;
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
	for (uint i = next; i < _elements.size(); i++) {
		if (static_cast<int>(next) <= _elements[i].parent)
			_elements[i].parent += 1;
	}
	_elements.insert_at(next, node);
}

void XMLFile::writeNode(Common::String &text, uint index) const {
	Common::String name = tag(index);
	if (!name.empty()) {
		text += "<" + name;
		for (auto it = _elements[index].attrs.begin(); it != _elements[index].attrs.end(); ++it)
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

bool XMLFile::get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (property.equals(kPropertyChildCount)) {
		result = Core::ScriptValue(static_cast<int32>(children().size()));
	} else if (property.equals(kPropertyNodeTag)) {
		result = Core::ScriptValue(_selected < 0 ? Common::String() : tag(_selected));
	} else if (property.equals(kPropertyContentCount)) {
		result = Core::ScriptValue(static_cast<int32>(content().size()));
	} else if (property.equals(kPropertyContentString)) {
		Common::Array<Common::String> values = content();
		int index = sargs.empty() ? 0 : MAX<int32>(0, sargs[0].toInt32() - 1);
		result = Core::ScriptValue::fromText(static_cast<uint>(index) < values.size() ? values[index] : Common::String());
	} else if (property.equals(kPropertyAttributeValue) && !sargs.empty()) {
		Common::String value;
		if (0 <= _selected)
			value = _elements[_selected].attrs.getValOrDefault(sargs[0].toText());
		result = Core::ScriptValue::fromText(value);
	} else if (property.equals(kPropertyGetVariable) && !sargs.empty()) {
		Common::String name = sargs[0].toText();
		if (1 < sargs.size() && 0 < sargs[1].toInt32())
			name += Common::String::format(kSignedIndexFormat, sargs[1].toInt32());
		Common::String value;
		for (int child : children()) {
			if (tag(child).equals(name)) {
				value = _elements[child].value;
				break;
			}
		}
		result = Core::ScriptValue::fromText(value);
	} else {
		return ScriptableObject::get(property, sargs, result);
	}
	return true;
}

bool XMLFile::play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (method.equals(kMethodAddNode) && sargs.size() == 1) {
		addNode(sargs[0].toText());
	} else if (method.equals(kMethodAddVariable) && sargs.size() == 2) {
		addNode(sargs[0].toText(), sargs[1].toText());
	} else if (method.equals(kMethodWrite) && sargs.empty()) {
		if (!write())
			return false;
	} else if (method.equals(kMethodSelectRoot)) {
		_selected = _elements.empty() ? -1 : 0;
	} else if (method.equals(kMethodSelectParent)) {
		if (0 <= _selected && 0 <= _elements[_selected].parent)
			_selected = _elements[_selected].parent;
	} else if (method.equals(kMethodSelectChild) && !sargs.empty()) {
		Common::Array<int> values = children();
		int index = MAX<int32>(0, sargs[0].toInt32() - 1);
		if (static_cast<uint>(index) < values.size())
			_selected = values[index];
	} else if (method.equals(kMethodSelectNode) && !sargs.empty()) {
		if (1 < sargs.size() && sargs[1].toInt32())
			_selected = _elements.empty() ? -1 : 0;
		Common::String name = sargs[0].toText();
		if (_selected != 0 || _elements.empty() || !name.equals(tag(0))) {
			Common::Array<int> values = children();
			for (uint i = 0; i < values.size(); i++) {
				if (tag(values[i]).equals(name)) {
					_selected = values[i];
					break;
				}
			}
		}
	} else {
		return ScriptableObject::play(method, sargs, result);
	}
	complete();
	return true;
}

} // namespace Shell
} // namespace Trinket
