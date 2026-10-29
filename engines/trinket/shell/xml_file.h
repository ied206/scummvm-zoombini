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
		/** Create a script-facing XML document from its constructor arguments. */
		Core::ScriptableObject *create(const Core::ScriptArguments &sargs) override;
	};
	/** Load an XML file and reset the selected element. */
	bool load(const Common::String &path);
	/** Parse XML text and reset the selected element. */
	bool loadText(const Common::String &text);
	/** Read the selected element's name, text, children or attributes. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Dispatch XML navigation, editing and file-writing script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;

private:
	/** Script property name for child Count. */
	static constexpr const char *kPropertyChildCount = "childCount";
	/** Script property name for node Tag. */
	static constexpr const char *kPropertyNodeTag = "nodeTag";
	/** Script property name for content Count. */
	static constexpr const char *kPropertyContentCount = "contentCount";
	/** Script property name for content String. */
	static constexpr const char *kPropertyContentString = "contentString";
	/** Script property name for attribute Value. */
	static constexpr const char *kPropertyAttributeValue = "attributeValue";
	/** Script property name for get Variable. */
	static constexpr const char *kPropertyGetVariable = "getVariable";
	/** Format for script names or values: .%d. */
	static constexpr const char *kSignedIndexFormat = ".%d";
	/** Script method name for add Node. */
	static constexpr const char *kMethodAddNode = "addNode";
	/** Script method name for add Variable. */
	static constexpr const char *kMethodAddVariable = "addVariable";
	/** Script method name for write. */
	static constexpr const char *kMethodWrite = "write";
	/** Script method name for select Root. */
	static constexpr const char *kMethodSelectRoot = "selectRoot";
	/** Script method name for select Parent. */
	static constexpr const char *kMethodSelectParent = "selectParent";
	/** Script method name for select Child. */
	static constexpr const char *kMethodSelectChild = "selectChild";
	/** Script method name for select Node. */
	static constexpr const char *kMethodSelectNode = "selectNode";

	/** Default extension for scripted XML files. */
	static constexpr const char *kFileExtension = ".xml";

	/** Editable document elements in parsing order. */
	Common::Array<OM::XMLParser::Element> _elements;
	/** Filename used when the document is written back. */
	Common::String _filename;
	/** Current element index, or minus one when no element is selected. */
	int _selected = -1;
	/** Append a named element beneath the current selection. */
	void addNode(const Common::String &name, const Common::String &value = Common::String());
	/** Serialize the current document to its selected filename. */
	bool write() const;
	/** Append one element and its descendants to the serialized document text. */
	void writeNode(Common::String &text, uint index) const;
	/** Return the local tag name of an element. */
	Common::String tag(uint index) const;
	/** Return the direct child indices of the selected element. */
	Common::Array<int> children() const;
	/** Return content values associated with the selected element. */
	Common::Array<Common::String> content() const;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
