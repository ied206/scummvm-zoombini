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

#include "trinket/rr1cloud9/page.h"

#include "common/ptr.h"

namespace Trinket {
namespace RR1Cloud9 {

const Core::Script::Variable *Page::constant(const Common::String &name, uint depth) const {
	if (8 <= depth)
		return nullptr;
	const Core::Script::Variable *value = _script.find(name);
	if (value && (value->type == 2 || value->type == 3))
		return value;
	const auto &operands = _script.operands();
	const auto &variables = _script.variables();
	for (const auto &instruction : _script.instructions()) {
		if (instruction.opcode != 18 || instruction.operand < 0 || operands.size() <= static_cast<uint>(instruction.operand + 2))
			continue;
		int target = operands[instruction.operand], source = operands[instruction.operand + 1];
		if (target < 0 || source < 0 || variables.size() <= static_cast<uint>(target) || variables.size() <= static_cast<uint>(source))
			continue;
		if (variables[target].name == name && operands[instruction.operand + 2] == -1) {
			if (variables[source].classification == 1)
				return &variables[source];
			return constant(variables[source].name, depth + 1);
		}
	}
	return nullptr;
}

Common::String Page::text(const Common::String &name) const {
	const auto *value = constant(name);
	if (!value || value->type != 2)
		return Common::String();
	if (value->classification == 1 && value->name.size() >= 2 && value->name[0] == '"')
		return value->name.substr(1, value->name.size() - 2);
	return value->extra;
}

int Page::number(const Common::String &name) const {
	const auto *value = constant(name);
	return value && value->type == 3 ? value->integer : 0;
}

bool Page::load(const OM::ResourceProvider &resources, const Common::String &name) {
	_exits.clear();
	Common::ScopedPtr<Common::SeekableReadStream> stream(resources.open(name + ".mps"));
	if (!stream || !_script.load(*stream))
		return false;
	int count = number("kNumPrevLocs");
	if (count < 0 || 32 < count)
		return false;
	for (int direction = 1; direction <= count; direction++) {
		Common::String destination = text(Common::String::format("kPreviousLocations.%d", direction));
		int hotspots = number(Common::String::format("kNumNavHotspots.%d", direction));
		if (hotspots < 0 || 32 < hotspots)
			return false;
		for (int i = 1; i <= hotspots; i++) {
			Common::String suffix = Common::String::format(".%d.%d", direction, i);
			int left = number("kNavHSX1" + suffix), top = number("kNavHSY1" + suffix);
			int right = number("kNavHSX2" + suffix), bottom = number("kNavHSY2" + suffix);
			if (left < 0 || top < 0 || 640 < right || 480 < bottom || right < left || bottom < top)
				return false;
			Exit exit;
			exit.rect = Common::Rect(left, top, right, bottom);
			exit.destination = destination;
			exit.cursor = number(Common::String::format("kNavHSCursors.%d", direction));
			if (!destination.empty() && !exit.rect.isEmpty())
				_exits.push_back(exit);
		}
	}
	return true;
}

} // namespace RR1Cloud9
} // namespace Trinket
