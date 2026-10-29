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

#ifndef TRINKET_CORE_TRINKET_H
#define TRINKET_CORE_TRINKET_H

#include "trinket/core/object.h"

namespace Trinket {
namespace Core {

/** Scriptable object participating in the visual and input system. */
class Trinket : public ScriptableObject {
public:
	/** Create an enabled, visible object and declare its pointer events. */
	Trinket();
	/** Notify the parent observer that this child is being destroyed. */
	~Trinket() override;
	/** Horizontal position in the parent's game-pixel coordinate space. */
	int x = 0;
	/** Vertical position in the parent's game-pixel coordinate space. */
	int y = 0;
	/** Drawing depth used to order siblings. */
	int z = 0;
	/** Script-assigned integer used to identify this object. */
	int tag = 0;
	/** Whether the object participates in drawing. */
	bool visible = true;
	/** Whether the object accepts pointer interaction. */
	bool enabled = true;
	/** Whether script-controlled movement is allowed. */
	bool movable = true;
	/** Authored cursor resource name used while hovering over the object. */
	Common::String spotCursor = "0";
	/** Authored cursor resource name used while dragging the object. */
	Common::String grabCursor = "0";
	/** Return the object's width in game pixels. */
	virtual int width() { return 0; }
	/** Return the object's height in game pixels. */
	virtual int height() { return 0; }
	/** Read geometry, visibility or another supported Trinket property. */
	bool get(const Common::String &property, const ScriptArguments &sargs, ScriptValue &result) override;
	/** Assign a supported Trinket property and notify the parent when depth changes. */
	bool set(const Common::String &property, const ScriptArguments &sargs, const ScriptValue &value) override;
	/** Handle object placement, offsets and cursor-selection script methods. */
	bool play(const Common::String &method, const ScriptArguments &sargs, ScriptValue &result) override;
	/** Return the parent object, or null when detached. */
	Trinket *parent() const { return _parent; }

	class ChildObserver {
	public:
		/** Allow destruction through the child-observer interface. */
		virtual ~ChildObserver() {}
		/** Notify the parent before a child becomes unavailable. */
		virtual void childDestroyed(Trinket &child) = 0;
		/** Notify the parent that a child's drawing order must be updated. */
		virtual void childDepthChanged(Trinket &child) = 0;
	};

	/** Set the parent and the observer notified of child changes. */
	void attach(Trinket *parent, ChildObserver *observer);

private:
	/** Script event name for mouse Down. */
	static constexpr const char *kEventMouseDown = "mouseDown";
	/** Script event name for mouse Up. */
	static constexpr const char *kEventMouseUp = "mouseUp";
	/** Script event name for dbl Click. */
	static constexpr const char *kEventDblClick = "dblClick";
	/** Script event name for fly On. */
	static constexpr const char *kEventFlyOn = "flyOn";
	/** Script event name for fly Off. */
	static constexpr const char *kEventFlyOff = "flyOff";
	/** Script property name for x. */
	static constexpr const char *kPropertyX = "x";
	/** Script property name for y. */
	static constexpr const char *kPropertyY = "y";
	/** Script property name for z. */
	static constexpr const char *kPropertyZ = "z";
	/** Script property name for visible. */
	static constexpr const char *kPropertyVisible = "visible";
	/** Script property name for touchy. */
	static constexpr const char *kPropertyTouchy = "touchy";
	/** Script property name for movable. */
	static constexpr const char *kPropertyMovable = "movable";
	/** Script property name for tag. */
	static constexpr const char *kPropertyTag = "tag";
	/** Script property name for width. */
	static constexpr const char *kPropertyWidth = "width";
	/** Script property name for height. */
	static constexpr const char *kPropertyHeight = "height";
	/** Script method name for set AOCursor. */
	static constexpr const char *kMethodSetAOCursor = "setAOCursor";
	/** Script method name for place. */
	static constexpr const char *kMethodPlace = "place";
	/** Script identifier for offset. */
	static constexpr const char *kOffsetName = "offset";

	/** Parent that supplies the object's coordinate space. */
	Trinket *_parent = nullptr;
	/** Parent observer notified of destruction and depth changes. */
	ChildObserver *_childObserver = nullptr;
};

} // End of namespace Core
} // End of namespace Trinket

#endif
