/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_SHELL_XML_FILE_H
#define TRINKET_SHELL_XML_FILE_H

#include "trinket/core/object.h"
#include "trinket/ompp/xml.h"

namespace Trinket {
namespace Shell {

/** RXMLFile's document cursor and indexed content strings. */
class XMLFile : public Core::ScriptableObject {
public:
	class Factory : public Core::ObjectRegistry::Factory {
	public:
		Core::ScriptableObject *create(const Core::ScriptArguments &arguments) override;
	};
	bool load(const Common::String &path);
	bool loadText(const Common::String &text);
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;

private:
	Common::Array<OM::XMLParser::Element> _elements;
	Common::String _filename;
	int _selected = -1;
	void addNode(const Common::String &name, const Common::String &value = Common::String());
	bool write() const;
	void writeNode(Common::String &text, uint index) const;
	Common::String tag(uint index) const;
	Common::Array<int> children() const;
	Common::Array<Common::String> content() const;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
