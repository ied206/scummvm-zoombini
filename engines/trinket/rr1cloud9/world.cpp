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

#include "trinket/rr1cloud9/world.h"

namespace Trinket {
namespace RR1Cloud9 {

void World::recordAssessment(const Common::String &activity, int level, int outcome, int up, int down, int streak) {
	Common::String key = Common::String::format("%sStats:%d:", activity.c_str(), level);
	set(key + "total", MIN(get(key + "total") + 1, 1000000));
	if (outcome == 1)
		set(key + "correct", MIN(get(key + "correct") + 1, 1000000));
	int count = MIN(get(activity + "AssessmentCount") + 1, 20);
	if (outcome == 1 && up <= streak) {
		set(key + "color", 1);
		count = 0;
	} else if (outcome == 3 && down <= streak) {
		set(key + "color", 2);
		count = 0;
	} else if (MIN(up, down) <= count) {
		set(key + "color", 3);
	}
	set(activity + "AssessmentCount", count);
}

bool World::save(Common::WriteStream &stream) const {
	stream.writeUint16LE(_values.size());
	for (const auto &entry : _values) {
		stream.writeByte(entry._key.size());
		stream.write(entry._key.c_str(), entry._key.size());
		stream.writeSint32LE(entry._value);
	}
	return !stream.err();
}

bool World::load(Common::ReadStream &stream) {
	Common::HashMap<Common::String, int> values;
	uint count = stream.readUint16LE();
	if (512 < count)
		return false;
	for (uint i = 0; i < count; i++) {
		uint length = stream.readByte();
		if (!length || 80 < length)
			return false;
		char key[81];
		if (stream.read(key, length) != length)
			return false;
		key[length] = 0;
		if (strlen(key) != length || values.contains(key))
			return false;
		values[key] = stream.readSint32LE();
	}
	if (stream.err() || stream.eos())
		return false;
	_values = values;
	return true;
}

} // namespace RR1Cloud9
} // namespace Trinket
