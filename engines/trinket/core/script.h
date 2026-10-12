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

#ifndef TRINKET_CORE_SCRIPT_H
#define TRINKET_CORE_SCRIPT_H

#include "common/array.h"
#include "common/str.h"
#include "common/stream.h"

namespace Trinket {

/** Trinket Core: Z3 tcx.dll, Reader Rabbit trinketcore.dll, and launcher TLCLLTrinketcore.dll. */
namespace Core {

/** Compiled MPS image, with file-local indices and both serialized byte orders. */
class Script {
public:
	struct Instruction {
		byte opcode;
		int16 operand;
	};
	struct Variable {
		int32 classification = 0, type = 0, integer = 0;
		int32 range = 0, reference = 0, count = 0, indirect = 0;
		Common::String name, extra, real;
		Common::Array<int16> array;
	};
	bool load(Common::SeekableReadStream &stream);
	const Variable *find(const Common::String &name) const;
	const Common::Array<Instruction> &instructions() const { return _instructions; }
	const Common::Array<int16> &operands() const { return _operands; }
	const Common::Array<Variable> &variables() const { return _variables; }

private:
	Common::Array<Instruction> _instructions;
	Common::Array<int16> _operands;
	Common::Array<Variable> _variables;
	static int32 readInt(Common::SeekableReadStream &stream, bool bigEndian) {
		return bigEndian ? stream.readSint32BE() : stream.readSint32LE();
	}
	static int16 readShort(Common::SeekableReadStream &stream, bool bigEndian) {
		return bigEndian ? stream.readSint16BE() : stream.readSint16LE();
	}
	static bool readString(Common::SeekableReadStream &stream, bool bigEndian, Common::String &value);
};

} // End of namespace Core
} // End of namespace Trinket

#endif
