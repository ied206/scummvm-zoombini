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

/** Composite trinket membership; concrete owners control child destruction. */
class MultiTrinket : public Core::Trinket, private Core::Trinket::ChildObserver {
public:
	~MultiTrinket() override;
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool addTrinket(Core::Trinket &child);
	void removeTrinket(Core::Trinket &child);
	void clearTrinkets();
	const Common::Array<Core::Trinket *> &trinkets() const { return _children; }

private:
	Common::Array<Core::Trinket *> _children;
	Common::String _userData;
	void insert(Core::Trinket &child);
	void childDestroyed(Core::Trinket &child) override;
	void childDepthChanged(Core::Trinket &child) override;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
