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

/** Coordinate mutually exclusive selection among registered script objects. */
class RadioGroup : public Core::ScriptableObject, private Core::ScriptableObject::Observer {
public:
	class Factory : public Core::ObjectRegistry::Factory {
	public:
		/** Configure radio-group creation with an optional script runtime. */
		Factory() = default;
		/** Configure radio-group creation with an optional script runtime. */
		explicit Factory(Core::ScriptRuntime &runtime) : _runtime(&runtime) {}
		/** Create a radio group using this factory's optional runtime. */
		Core::ScriptableObject *create(const Core::ScriptArguments &sargs) override { return new RadioGroup(_runtime); }

	private:
		/** Optional runtime used to resolve named radio members. */
		Core::ScriptRuntime *_runtime = nullptr;
	};

	/** Initialize exclusive sprite selection with optional named-member lookup. */
	explicit RadioGroup(Core::ScriptRuntime *runtime = nullptr);
	/** Detach member callbacks and destruction observers. */
	~RadioGroup() override;
	/** Register a sprite as an exclusive-choice member. */
	void add(Shell::Sprite &member);
	/** Select a valid member and update all member button states. */
	bool select(int index);
	/** Clear exclusive selection and reset member button states. */
	void clearSelection();
	/** Apply a member click and emit the group's script selection event. */
	void clicked(uint index);
	/** Return the selected member index, or minus one when none is selected. */
	int selection() const { return _selected; }
	/** Read the selected member and supported group properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign the selected member through the script property interface. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch member registration and selection-control script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;

private:
	/** Script event name for changed. */
	static constexpr const char *kEventChanged = "changed";
	/** Script identifier for mouse Down. */
	static constexpr const char *kMouseDownName = "mouseDown";
	/** Script identifier for fly On. */
	static constexpr const char *kFlyOnName = "flyOn";
	/** Script identifier for fly Off. */
	static constexpr const char *kFlyOffName = "flyOff";
	/** Script property name for selection. */
	static constexpr const char *kPropertySelection = "selection";
	/** Script property name for last. */
	static constexpr const char *kPropertyLast = "last";
	/** Script property name for group Name. */
	static constexpr const char *kPropertyGroupName = "groupName";
	/** Script method name for add. */
	static constexpr const char *kMethodAdd = "add";
	/** Script identifier for rAnimation. */
	static constexpr const char *kRAnimationName = "RAnimation";
	/** Script method name for clear. */
	static constexpr const char *kMethodClear = "clear";

	class MemberLink : public Core::ObjectLink {
	public:
		/** Connect a member's click or hover event to its radio group. */
		MemberLink(RadioGroup &group, int hover = 0) : _group(group), _hover(hover) {}
		/** Copy the member event callback. */
		Core::ObjectLink *clone() const override { return new MemberLink(_group, _hover); }
		/** Forward a member event to group selection or hover handling. */
		void execute(Core::ScriptableObject &source) override;

	private:
		/** Radio group receiving the member event. */
		RadioGroup &_group;
		/** Hover transition associated with this event, or the click marker. */
		int _hover;
	};
	/** Sprites participating in exclusive selection. */
	Common::Array<Shell::Sprite *> _members;
	/** Currently selected member index, or minus one when unset. */
	int _selected = -1;
	/** Previous selection retained during pointer feedback. */
	int _previous = -1;
	/** Script name associated with this radio group. */
	Common::String _name;
	/** Optional script runtime used to resolve named members. */
	Core::ScriptRuntime *_runtime;
	/** Remove a destroyed member and adjust selection indices. */
	void objectDestroyed(Core::ScriptableObject &object) override;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
