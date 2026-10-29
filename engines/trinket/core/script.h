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
		/** Encoded instruction operation. */
		byte opcode;
		/** Index of the instruction's operand list in the compiled image. */
		int16 operand;
	};
	struct Variable {
		/** Variable form selecting a name, literal, path, slice or expression. */
		int32 classification = 0;
		/** Serialized value type selecting string, signed integer or real storage. */
		int32 type = 0;
		/** Signed 32-bit payload used by serialized integer variables. */
		int32 integer = 0;
		/** Upper heap index associated with a family of dotted variable names. */
		int32 range = 0;
		/** Heap index followed when this variable is an alias. */
		int32 reference = 0;
		/** Operand count or text length, interpreted according to the variable classification. */
		int32 count = 0;
		/** Alias marker; minus one redirects resolution through the reference index. */
		int32 indirect = 0;
		/** Authored variable name or literal text. */
		Common::String name;
		/** Stored string value, procedure marker or encoded expression text. */
		Common::String extra;
		/** Double-precision value parsed once from a serialized type-4 decimal string. */
		double real = 0;
		/** Heap indices used by expressions and segmented variable names. */
		Common::Array<int16> array;
	};
	/** Replace the compiled image, returning false for invalid sizes, types or trailing data. */
	bool load(Common::SeekableReadStream &stream);
	/** Return the first variable with the supplied authored name, or return null. */
	const Variable *find(const Common::String &name) const;
	/** Return the decoded instruction stream. */
	const Common::Array<Instruction> &instructions() const { return _instructions; }
	/** Return the compiled operand-index table. */
	const Common::Array<int16> &operands() const { return _operands; }
	/** Return the decoded heap variables, including the three reserved entries. */
	const Common::Array<Variable> &variables() const { return _vars; }

private:
	/** Serialized marker values selecting the byte order of an MPS image. */
	enum ByteOrder {
		kByteOrder01_BigEndian = 1,
		kByteOrder02_LittleEndian = 2
	};

	/** Script identifier for sYSTEM. */
	static constexpr const char *kSYSTEMName = "SYSTEM";
	/** Script identifier for rECEIVER. */
	static constexpr const char *kRECEIVERName = "RECEIVER";
	/** Script identifier for rESULT. */
	static constexpr const char *kRESULTName = "RESULT";
	/** Script identifier for wIN. */
	static constexpr const char *kWINName = "WIN";

	/** Decoded instructions in execution order. */
	Common::Array<Instruction> _instructions;
	/** Variable indices and terminators used by instruction operand lists. */
	Common::Array<int16> _operands;
	/** Decoded variables indexed by compiled heap references. */
	Common::Array<Variable> _vars;
	/** Read a signed 32-bit field in the image's byte order. */
	static int32 readInt(Common::SeekableReadStream &stream, bool bigEndian) {
		return bigEndian ? stream.readSint32BE() : stream.readSint32LE();
	}
	/** Read a signed 16-bit field in the image's byte order. */
	static int16 readShort(Common::SeekableReadStream &stream, bool bigEndian) {
		return bigEndian ? stream.readSint16BE() : stream.readSint16LE();
	}
	/** Read a length-prefixed string, rejecting a length outside the remaining stream. */
	static bool readString(Common::SeekableReadStream &stream, bool bigEndian, Common::String &value);
};

} // End of namespace Core
} // End of namespace Trinket

#endif
