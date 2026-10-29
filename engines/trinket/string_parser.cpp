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

#include "trinket/string_parser.h"
#include "common/util.h"

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>

namespace Trinket {

const StringParser::ParseConf StringParser::kDecimalProfile = {false, false};
const StringParser::ParseConf StringParser::kDecimalOrHexProfile = {true, false};
const StringParser::ParseConf StringParser::kNumericPrefixProfile = {true, true};

StringParser::Result StringParser::parseSignedInt(const char *text, int32 &result, const ParseConf &profile) {
	if (!text || text[0] == '\0')
		return Result::kEmptyInput;

	const char *digits = text;
	while (Common::isSpace(digits[0]))
		digits += 1;
	if (digits[0] == '+' || digits[0] == '-')
		digits += 1;

	int base = 10;
	if (profile.allowHexadecimal && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X'))
		base = 16;

	char *endPtr = nullptr;
	errno = 0;
	const long parsed = strtol(text, &endPtr, base);

	if (errno == ERANGE)
		return Result::kOutOfRange;
	if (errno != 0)
		return Result::kConversionFailure;
	if (endPtr == text)
		return Result::kInvalidInput;
	if (!profile.allowTrailingCharacters && *endPtr != '\0')
		return Result::kTrailingCharacters;
	if (parsed < INT32_MIN || INT32_MAX < parsed)
		return Result::kOutOfRange;

	result = static_cast<int32>(parsed);
	return Result::kSuccess;
}

StringParser::Result StringParser::parseUnsignedInt(const char *text, uint32 &result, const ParseConf &profile) {
	if (!text || text[0] == '\0')
		return Result::kEmptyInput;

	const char *digits = text;
	while (Common::isSpace(digits[0]))
		digits += 1;
	if (digits[0] == '-')
		return Result::kInvalidInput;
	if (digits[0] == '+')
		digits += 1;

	int base = 10;
	if (profile.allowHexadecimal && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X'))
		base = 16;

	char *endPtr = nullptr;
	errno = 0;
	const unsigned long parsed = strtoul(text, &endPtr, base);

	if (errno == ERANGE)
		return Result::kOutOfRange;
	if (errno != 0)
		return Result::kConversionFailure;
	if (endPtr == text)
		return Result::kInvalidInput;
	if (!profile.allowTrailingCharacters && *endPtr != '\0')
		return Result::kTrailingCharacters;
	if (UINT32_MAX < parsed)
		return Result::kOutOfRange;

	result = static_cast<uint32>(parsed);
	return Result::kSuccess;
}

StringParser::Result StringParser::parseFloat(const char *text, float &result, const ParseConf &profile) {
	if (!text || text[0] == '\0')
		return Result::kEmptyInput;

	char *endPtr = nullptr;
	errno = 0;
	const float parsed = strtof(text, &endPtr);
	Result status = checkReal(text, endPtr, errno, profile);
	if (status == Result::kSuccess)
		result = parsed;
	return status;
}

StringParser::Result StringParser::parseDouble(const char *text, double &result, const ParseConf &profile) {
	if (!text || text[0] == '\0')
		return Result::kEmptyInput;

	char *endPtr = nullptr;
	errno = 0;
	const double parsed = strtod(text, &endPtr);
	Result status = checkReal(text, endPtr, errno, profile);
	if (status == Result::kSuccess)
		result = parsed;
	return status;
}

int32 StringParser::parseInt32Prefix(const char *text) {
	if (!text)
		return 0;
	while (Common::isSpace(*text))
		text += 1;
	bool negative = *text == '-';
	if (*text == '+' || negative)
		text += 1;
	uint32 value = 0;
	// Native integer parameters stop at the first nondigit: "1e3" -> digits "1" -> 1.
	// Parsing a real first would turn the same text into 1000.
	while (Common::isDigit(*text)) {
		value = value * 10 + (*text - '0');
		text += 1;
	}
	if (negative)
		value = 0U - value;
	if (INT32_MAX < value)
		return INT32_MIN + static_cast<int32>(value - INT32_MAX - 1);
	return static_cast<int32>(value);
}

double StringParser::parseDoublePrefix(const char *text) {
	if (!text)
		return 0;
	const char *digits = text;
	while (Common::isSpace(*digits))
		digits += 1;
	bool negative = *digits == '-';
	if (*digits == '+' || negative)
		digits += 1;
	if (!Common::isDigit(*digits) && *digits != '.')
		return 0;
	if (digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X'))
		return negative ? -0.0 : 0.0;
	char *endPtr = nullptr;
	double value = strtod(text, &endPtr);
	return endPtr == text ? 0.0 : value;
}

StringParser::Result StringParser::checkReal(const char *text, const char *endPtr, int parseErr, const ParseConf &profile) {
	if (parseErr == ERANGE)
		return Result::kOutOfRange;
	if (parseErr != 0)
		return Result::kConversionFailure;
	if (endPtr == text)
		return Result::kInvalidInput;

	const char *digits = text;
	while (Common::isSpace(digits[0]))
		digits += 1;
	if (digits[0] == '+' || digits[0] == '-')
		digits += 1;
	if (!profile.allowHexadecimal && digits[0] == '0' && (digits[1] == 'x' || digits[1] == 'X'))
		return Result::kTrailingCharacters;
	if (!profile.allowTrailingCharacters && *endPtr != '\0')
		return Result::kTrailingCharacters;
	return Result::kSuccess;
}

} // End of namespace Trinket
