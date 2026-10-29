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

#include "trinket/string_parser.h"

namespace Trinket {
namespace Core {

constexpr const char *Script::kSYSTEMName;
constexpr const char *Script::kRECEIVERName;
constexpr const char *Script::kRESULTName;
constexpr const char *Script::kWINName;

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
	_vars.clear();
	byte byteOrder = stream.readByte();
	if (byteOrder != kByteOrder01_BigEndian && byteOrder != kByteOrder02_LittleEndian)
		return false;
	bool bigEndian = byteOrder == kByteOrder01_BigEndian;
	uint32 count = readInt(stream, bigEndian);
	if (count > (stream.size() - stream.pos()) / 3)
		return false;
	for (uint32 i = 0; i < count; i++) {
		Instruction inst;
		inst.opcode = stream.readByte();
		inst.operand = readShort(stream, bigEndian);
		_instructions.push_back(inst);
	}
	count = readInt(stream, bigEndian);
	if (count > (stream.size() - stream.pos()) / 2)
		return false;
	for (uint32 i = 0; i < count; i++)
		_operands.push_back(readShort(stream, bigEndian));
	count = readInt(stream, bigEndian);
	if (count < 3 || 32768 < count || count - 3 > (stream.size() - stream.pos()) / 36)
		return false;
	_vars.resize(count);
	_vars[0].name = kSYSTEMName;
	_vars[1].name = kRECEIVERName;
	_vars[2].name = kRESULTName;
	_vars[0].type = _vars[1].type = _vars[2].type = 2;
	_vars[0].extra = kWINName;
	_vars[0].indirect = 1;
	_vars[1].indirect = -1;
	for (uint32 i = 3; i < count; i++) {
		Variable &var = _vars[i];
		var.classification = readInt(stream, bigEndian);
		var.type = readInt(stream, bigEndian);
		if (var.type == 3) {
			var.integer = readInt(stream, bigEndian);
		} else if (var.type == 4) {
			int16 length = readShort(stream, bigEndian);
			if (length < 0 || stream.size() - stream.pos() < length)
				return false;
			Common::String text;
			for (int j = 0; j < length; j++)
				text += static_cast<char>(stream.readByte());
			var.real = StringParser::parseDoublePrefix(text.c_str());
		} else if (var.type < 0 || 4 < var.type) {
			return false;
		}
		var.range = readInt(stream, bigEndian);
		var.reference = readInt(stream, bigEndian);
		var.count = readInt(stream, bigEndian);
		var.indirect = readInt(stream, bigEndian);
		if (!readString(stream, bigEndian, var.name) || !readString(stream, bigEndian, var.extra))
			return false;
		int32 size = readInt(stream, bigEndian);
		if (size < -1 || stream.size() - stream.pos() < static_cast<int64>(size) * 2)
			return false;
		for (int32 j = 0; j < size; j++)
		var.array.push_back(readShort(stream, bigEndian));
	}
	return !stream.err() && !stream.eos() && stream.pos() == stream.size();
}

const Script::Variable *Script::find(const Common::String &name) const {
	for (uint i = 0; i < _vars.size(); i++) {
		if (_vars[i].name.equals(name))
			return &_vars[i];
	}
	return nullptr;
}

} // namespace Core
} // namespace Trinket
