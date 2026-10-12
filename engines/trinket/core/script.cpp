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

#include "trinket/core/script.h"

namespace Trinket {
namespace Core {

bool Script::readString(Common::SeekableReadStream &stream, bool bigEndian, Common::String &value) {
	int32 length = readInt(stream, bigEndian);
	value.clear();
	if (length == -1)
		return true;
	if (length < 0 || stream.size() - stream.pos() < length)
		return false;
	for (int32 i = 0; i < length; i++)
		value += static_cast<char>(stream.readByte());
	return !stream.err() && !stream.eos();
}

bool Script::load(Common::SeekableReadStream &stream) {
	_instructions.clear();
	_operands.clear();
	_variables.clear();
	byte flag = stream.readByte();
	if (flag != 1 && flag != 2)
		return false;
	bool bigEndian = flag == 1;
	uint32 count = readInt(stream, bigEndian);
	if (count > (stream.size() - stream.pos()) / 3)
		return false;
	for (uint32 i = 0; i < count; i++) {
		Instruction instruction;
		instruction.opcode = stream.readByte();
		instruction.operand = readShort(stream, bigEndian);
		_instructions.push_back(instruction);
	}
	count = readInt(stream, bigEndian);
	if (count > (stream.size() - stream.pos()) / 2)
		return false;
	for (uint32 i = 0; i < count; i++)
		_operands.push_back(readShort(stream, bigEndian));
	count = readInt(stream, bigEndian);
	if (count < 3 || 32768 < count || count - 3 > (stream.size() - stream.pos()) / 36)
		return false;
	_variables.resize(count);
	_variables[0].name = "SYSTEM";
	_variables[1].name = "RECEIVER";
	_variables[2].name = "RESULT";
	_variables[0].type = _variables[1].type = _variables[2].type = 2;
	_variables[0].extra = "WIN";
	_variables[0].indirect = 1;
	_variables[1].indirect = -1;
	for (uint32 i = 3; i < count; i++) {
		Variable &variable = _variables[i];
		variable.classification = readInt(stream, bigEndian);
		variable.type = readInt(stream, bigEndian);
		if (variable.type == 3) {
			variable.integer = readInt(stream, bigEndian);
		} else if (variable.type == 4) {
			int16 length = readShort(stream, bigEndian);
			if (length < 0 || stream.size() - stream.pos() < length)
				return false;
			for (int j = 0; j < length; j++)
				variable.real += static_cast<char>(stream.readByte());
		} else if (variable.type < 0 || 4 < variable.type) {
			return false;
		}
		variable.range = readInt(stream, bigEndian);
		variable.reference = readInt(stream, bigEndian);
		variable.count = readInt(stream, bigEndian);
		variable.indirect = readInt(stream, bigEndian);
		if (!readString(stream, bigEndian, variable.name) || !readString(stream, bigEndian, variable.extra))
			return false;
		int32 size = readInt(stream, bigEndian);
		if (size < -1 || stream.size() - stream.pos() < static_cast<int64>(size) * 2)
			return false;
		for (int32 j = 0; j < size; j++)
			variable.array.push_back(readShort(stream, bigEndian));
	}
	return !stream.err() && !stream.eos() && stream.pos() == stream.size();
}

const Script::Variable *Script::find(const Common::String &name) const {
	for (uint i = 0; i < _variables.size(); i++) {
		if (_variables[i].name == name)
			return &_variables[i];
	}
	return nullptr;
}

} // namespace Core
} // namespace Trinket
