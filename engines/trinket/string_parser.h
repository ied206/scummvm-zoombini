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

#ifndef TRINKET_STRING_PARSER_H
#define TRINKET_STRING_PARSER_H

#include "common/scummsys.h"

namespace Trinket {

/** Numeric text conversion with explicit syntax profiles. */
class StringParser {
public:
	/** Result of parsing integer or floating-point text. */
	enum class Result {
		/** The input parsed according to its profile and the output value was updated. */
		kSuccess,
		/** The input is null or empty. */
		kEmptyInput,
		/** No number was parsed, or unsigned input had a leading minus sign. */
		kInvalidInput,
		/** The profile rejects hexadecimal input or characters remaining after the number. */
		kTrailingCharacters,
		/** The conversion routine reported a non-range error. */
		kConversionFailure,
		/** The value is outside the destination type's range. */
		kOutOfRange
	};

	/** Parsing options for accepted numeric syntax. */
	struct ParseConf {
		/** Whether a 0x-prefixed hexadecimal value is accepted. */
		bool allowHexadecimal;
		/** Whether characters may remain after a successfully parsed numeric prefix. */
		bool allowTrailingCharacters;
	};

	/** Profile requiring complete input and disabling hexadecimal numeric syntax. */
	static const ParseConf kDecimalProfile;
	/** Profile requiring complete input and allowing 0x-prefixed hexadecimal values. */
	static const ParseConf kDecimalOrHexProfile;
	/** Profile accepting a numeric prefix, including hexadecimal values, before arbitrary trailing text. */
	static const ParseConf kNumericPrefixProfile;

	/** Parse signed 32-bit integer text and leave @p result unchanged on failure. */
	static Result parseSignedInt(const char *text, int32 &result, const ParseConf &profile);
	/** Parse unsigned 32-bit integer text and leave @p result unchanged on failure. */
	static Result parseUnsignedInt(const char *text, uint32 &result, const ParseConf &profile);
	/** Parse directly to float with strtof and leave @p result unchanged on failure, including underflow. */
	static Result parseFloat(const char *text, float &result, const ParseConf &profile);
	/** Parse directly to double with strtod and leave @p result unchanged on failure, including underflow. */
	static Result parseDouble(const char *text, double &result, const ParseConf &profile);
	/** Read a decimal integer prefix with signed 32-bit wrapping, returning zero when no digits are present. */
	static int32 parseInt32Prefix(const char *text);
	/** Read a decimal real prefix, returning zero on invalid input and retaining overflow and underflow results. */
	static double parseDoublePrefix(const char *text);

private:
	/** Check a real conversion's error code, consumed text and syntax profile before publishing its value. */
	static Result checkReal(const char *text, const char *endPtr, int parseErr, const ParseConf &profile);
};

} // End of namespace Trinket

#endif
