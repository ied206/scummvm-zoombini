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

#ifndef TRINKET_ZOOMBINI3_RADIO_H
#define TRINKET_ZOOMBINI3_RADIO_H

#include "trinket/core/script_runtime.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace Zoombini3 {

/** Scriptable radio group whose members remain owned by their scene. */
class RadioGroup : public Core::ScriptableObject, private Core::ScriptableObject::Observer {
public:
	class Factory : public Core::ObjectRegistry::Factory {
	public:
		Factory() = default;
		explicit Factory(Core::ScriptRuntime &runtime) : _runtime(&runtime) {}
		Core::ScriptableObject *create(const Core::ScriptArguments &arguments) override { return new RadioGroup(_runtime); }

	private:
		Core::ScriptRuntime *_runtime = nullptr;
	};

	explicit RadioGroup(Core::ScriptRuntime *runtime = nullptr);
	~RadioGroup() override;
	void add(Shell::Sprite &member);
	bool select(int index);
	void clearSelection();
	void clicked(uint index);
	int selection() const { return _selected; }
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;

private:
	class MemberLink : public Core::ObjectLink {
	public:
		MemberLink(RadioGroup &group, int hover = 0) : _group(group), _hover(hover) {}
		Core::ObjectLink *clone() const override { return new MemberLink(_group, _hover); }
		void execute(Core::ScriptableObject &source) override;

	private:
		RadioGroup &_group;
		int _hover;
	};
	Common::Array<Shell::Sprite *> _members;
	int _selected = -1, _previous = -1;
	Common::String _name;
	Core::ScriptRuntime *_runtime;
	void objectDestroyed(Core::ScriptableObject &object) override;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
