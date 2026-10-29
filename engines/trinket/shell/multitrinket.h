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

#ifndef TRINKET_SHELL_MULTITRINKET_H
#define TRINKET_SHELL_MULTITRINKET_H

#include "trinket/core/trinket.h"

namespace Trinket {
namespace Shell {

/** Track child trinkets in a composite; subclasses determine when children are deleted. */
class MultiTrinket : public Core::Trinket, private Core::Trinket::ChildObserver {
public:
	/** Detach child observers before the container is destroyed. */
	~MultiTrinket() override;
	/** Read child count, user data or an inherited Trinket property. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign user data or an inherited Trinket property. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Attach a child and insert it into the container's depth order. */
	bool addTrinket(Core::Trinket &child);
	/** Detach a child and remove it from the depth-ordered list. */
	void removeTrinket(Core::Trinket &child);
	/** Detach all children without destroying them. */
	void clearTrinkets();
	/** Return the children in drawing order. */
	const Common::Array<Core::Trinket *> &trinkets() const { return _children; }

private:
	/** Script identifier for user Data. */
	static constexpr const char *kUserDataName = "userData";

	/** Attached children sorted by drawing depth. */
	Common::Array<Core::Trinket *> _children;
	/** Arbitrary script text attached to the container. */
	Common::String _userData;
	/** Insert an attached child at its current drawing depth. */
	void insert(Core::Trinket &child);
	/** Remove a child that is being destroyed from the drawing list. */
	void childDestroyed(Core::Trinket &child) override;
	/** Reinsert a child after its drawing depth changes. */
	void childDepthChanged(Core::Trinket &child) override;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
