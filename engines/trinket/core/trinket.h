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
	Trinket();
	~Trinket() override;
	int x = 0, y = 0, z = 0;
	int tag = 0;
	bool visible = true, enabled = true, movable = true;
	Common::String spotCursor = "0", grabCursor = "0";
	virtual int width() { return 0; }
	virtual int height() { return 0; }
	bool get(const Common::String &property, const ScriptArguments &arguments, ScriptValue &result) override;
	bool set(const Common::String &property, const ScriptArguments &arguments, const ScriptValue &value) override;
	bool play(const Common::String &method, const ScriptArguments &arguments, ScriptValue &result) override;
	Trinket *parent() const { return _parent; }

	class ChildObserver {
	public:
		virtual ~ChildObserver() {}
		virtual void childDestroyed(Trinket &child) = 0;
		virtual void childDepthChanged(Trinket &child) = 0;
	};

	void attach(Trinket *parent, ChildObserver *observer);

private:
	Trinket *_parent = nullptr;
	ChildObserver *_childObserver = nullptr;
};

} // End of namespace Core
} // End of namespace Trinket

#endif
