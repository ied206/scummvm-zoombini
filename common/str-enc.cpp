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

#include "common/str.h"
#include "common/ustr.h"
#include "common/noncopyable.h"
#include "common/util.h"
#include "common/endian.h"
#include "common/error.h"
#include "common/system.h"
#include "common/enc-internal.h"
#include "common/file.h"

namespace Common {

// //TODO: This is a quick and dirty converter. Refactoring needed:
// 1. Original version has an option for performing strict / nonstrict
//    conversion for the 0xD800...0xDFFF interval
// 2. Original version returns a result code. This version does NOT
//    insert 'FFFD' on errors & does not inform caller on any errors
//
// More comprehensive one lives in wintermute/utils/convert_utf.cpp
void U32String::decodeUTF8(const char *src, uint32 len) {
	ensureCapacity(len, false);

	// The String class, and therefore the Font class as well, assume one
	// character is one byte, but in this case it's actually an UTF-8
	// string with up to 4 bytes per character. To work around this,
	// convert it to an U32String before drawing it, because our Font class
	// can handle that.
	for (uint i = 0; i < len;) {
		uint32 chr = 0;
		uint num = 1;

		if ((src[i] & 0xF8) == 0xF0) {
			num = 4;
		} else if ((src[i] & 0xF0) == 0xE0) {
			num = 3;
		} else if ((src[i] & 0xE0) == 0xC0) {
			num = 2;
		}

		if (len - i >= num) {
			switch (num) {
			case 4:
				chr |= (src[i++] & 0x07) << 18;
				chr |= (src[i++] & 0x3F) << 12;
				chr |= (src[i++] & 0x3F) << 6;
				chr |= (src[i++] & 0x3F);
				break;

			case 3:
				chr |= (src[i++] & 0x0F) << 12;
				chr |= (src[i++] & 0x3F) << 6;
				chr |= (src[i++] & 0x3F);
				break;

			case 2:
				chr |= (src[i++] & 0x1F) << 6;
				chr |= (src[i++] & 0x3F);
				break;

			default:
				chr = (src[i++] & 0x7F);
				break;
			}
		} else {
			break;
		}

		operator+=(chr);
	}
}

const uint16 invalidCode = 0xFFFD;

struct MacCJKSequence {
	/** Mac CJK character code, encoded in one or two bytes. */
	uint16 encoded = 0;
	/** Unicode code points, padded with zeroes after the sequence. */
	uint16 unicode[5] = {};
};

struct MacCJKSequenceTable : private NonCopyable {
	/** Number of elements in each owned array. */
	uint32 count = 0;
	/**
	 * Records sorted lexicographically by their Unicode sequences.
	 *
	 * @remarks
	 * Encoding from Unicode to MacCJK binary-searches these records for the longest matching sequence.
	 */
	const MacCJKSequence *records = nullptr;
	/**
	 * Indices into @ref MacCJKSequenceTable::records, sorted by the records' MacCJK character codes.
	 *
	 * @remarks
	 * Decoding from MacCJK to Unicode binary-searches these indices to find a record by its encoded character.
	 * One uint16 per record avoids linear scans, especially for MacKorean, without duplicating the records.
	 */
	const uint16 *encodedOrder = nullptr;

	/** Free the owned record and index arrays. */
	~MacCJKSequenceTable() {
		delete[] records;
		delete[] encodedOrder;
	}
};

static bool cjk_tables_loaded = false;
static const uint16 *windows932ConversionTable = 0;
static const uint16 *windows932ReverseConversionTable = 0;
static const uint16 *windows936ConversionTable = 0;
static const uint16 *windows936ReverseConversionTable = 0;
static const uint16 *windows949ConversionTable = 0;
static const uint16 *windows949ReverseConversionTable = 0;
static const uint16 *windows950ConversionTable = 0;
static uint16 *windows950ReverseConversionTable = 0;
static const uint16 *johabConversionTable = 0;
static const uint16 *johabReverseConversionTable = 0;
static const uint16 *traditional2SimplifiedChineseConversionTable = 0;
static const uint16 *macKoreanConversionTable = 0;
static const MacCJKSequenceTable *macKoreanSequenceTable = 0;
static uint16 *macKoreanReverseConversionTable = 0;
static const uint16 *macJapaneseConversionTable = 0;
static const MacCJKSequenceTable *macJapaneseSequenceTable = 0;
static uint16 *macJapaneseReverseConversionTable = 0;
static const uint16 *macChineseTradConversionTable = 0;
static const MacCJKSequenceTable *macChineseTradSequenceTable = 0;
static uint16 *macChineseTradReverseConversionTable = 0;
static const uint16 *macChineseSimpConversionTable = 0;
static const MacCJKSequenceTable *macChineseSimpSequenceTable = 0;
static uint16 *macChineseSimpReverseConversionTable = 0;

/**
 * Load a primary CJK lookup table from encoding.dat.
 *
 * Entries hold one Unicode code point, or zero for an unmapped character.
 * Mac mappings with several code points are loaded separately by loadMacCJKSequences().
 */
static const uint16 *loadCJKTable(File &f, int idx, size_t sz) {
	f.seek(16 + idx * 4);
	uint32 off = f.readUint32LE();
	f.seek(off);
	uint16 *res = new uint16[sz];
	f.read(res, 2 * sz);
#ifndef SCUMM_LITTLE_ENDIAN
	for (uint i = 0; i < sz; i++)
		res[i] = FROM_LE_16(res[i]);
#endif
	return res;
}

/**
 * Load an auxiliary Mac CJK mapping table for characters that expand into several Unicode code points.
 *
 * For example, MacKorean 0xAA41 maps to two Unicode code points: U+C6B4 + U+20DE.
 * Such mappings cannot fit in one uint16 entry of primary table.
 * Instead, they are looked up in this auxiliary sequence table.
 *
 * This follows ICU's separation of base (1:1) and extension (1:N) mappings:
 * https://unicode-org.github.io/icu/userguide/conversion/data.html#extension-and-delta-tables
 * - Decoding from MacCJK to Unicode checks the primary table first.
 * - Encoding from Unicode to MacCJK matches the longest sequence before a single code point.
 *
 * A uint32 count precedes records of six uint16 values:
 * the encoded character followed by up to five Unicode code points, padded with zeroes.
 * The records are followed by uint16 record indices sorted by encoded character.
 *
 * @return The loaded table, or nullptr on allocation failure or invalid offset, size or record indices.
 */
static const MacCJKSequenceTable *loadMacCJKSequences(File &f, int idx) {
	f.seek(16 + idx * 4);
	uint32 offset = f.readUint32LE();
	if (offset < 16 || f.size() < offset + 4)
		return nullptr;
	f.seek(offset);

	uint32 entries = f.readUint32LE();
	// Each distinct uint16 encoded character can have at most one record.
	if (0x10000 < entries || f.size() < offset + 4 + entries * 14)
		return nullptr;
	MacCJKSequenceTable *table = new MacCJKSequenceTable();
	if (!table)
		return nullptr;
	table->count = entries;
	MacCJKSequence *records = new MacCJKSequence[entries];
	table->records = records;
	uint16 *encodedOrder = new uint16[entries];
	table->encodedOrder = encodedOrder;
	if (!records || !encodedOrder) {
		delete table;
		return nullptr;
	}
	for (uint i = 0; i < entries; i++) {
		MacCJKSequence &record = records[i];
		record.encoded = f.readUint16LE();
		for (uint j = 0; j < ARRAYSIZE(record.unicode); j++)
			record.unicode[j] = f.readUint16LE();
	}
	for (uint i = 0; i < entries; i++) {
		encodedOrder[i] = f.readUint16LE();
		if (entries <= encodedOrder[i]) {
			delete table;
			return nullptr;
		}
	}
	return table;
}

static void loadChineseT2S(File &f) {
	f.seek(16 + 5 * 4);
	uint32 off = f.readUint32LE();
	f.seek(off);
	uint16 *res = new uint16[0x10000];
	for (uint i = 0; i < 0x10000; i++) {
		res[i] = i;
	}
	uint32 num_entries = f.readUint32LE();
	for (uint i = 0; i < num_entries; i++) {
		uint16 t = f.readUint16LE();
		uint16 s = f.readUint16LE();
		res[t] = s;
	}
	traditional2SimplifiedChineseConversionTable = res;
}

static void loadCJKTables() {
	File f;

	cjk_tables_loaded = true;

	if (!f.open("encoding.dat")) {
		warning("encoding.dat is not found. Support for CJK is disabled");
		return;
	}

	if (f.size() < 16 + 3 * 4) {
		warning("encoding.dat is invalid. Support for CJK is disabled");
		return;
	}

	if (f.readUint32BE() != MKTAG('S', 'C', 'V', 'M')
	    || f.readUint32BE() != MKTAG('E', 'N', 'C', 'D')) {
		warning("encoding.dat is invalid. Support for CJK is disabled");
		return;
	}

	int ver = f.readUint32LE();
	int num_tables = f.readUint32LE();

	// Version and number of tables.
	if (ver != 0 || num_tables < 3) {
		warning("encoding.dat is of incompatible version. Support for CJK is disabled");
		return;
	}

	windows932ConversionTable = loadCJKTable(f, 0, 47 * 192);
	windows949ConversionTable = loadCJKTable(f, 1, 0x7e * 0xb2);
	windows950ConversionTable = loadCJKTable(f, 2, 89 * 157);
	if (num_tables >= 4)
		johabConversionTable = loadCJKTable(f, 3, 80 * 188);
	if (num_tables >= 5)
		windows936ConversionTable = loadCJKTable(f, 4, 126 * 190);
	if (num_tables >= 6)
		loadChineseT2S(f);
	if (num_tables >= 14) {
		// Each primary Mac CJK table is followed by its auxiliary Unicode sequence table.
		macKoreanConversionTable = loadCJKTable(f, 6, 256 + 94 * 190);
		macKoreanSequenceTable = loadMacCJKSequences(f, 7);
		macJapaneseConversionTable = loadCJKTable(f, 8, 256 + 60 * 192);
		macJapaneseSequenceTable = loadMacCJKSequences(f, 9);
		macChineseTradConversionTable = loadCJKTable(f, 10, 256 + 92 * 157);
		macChineseTradSequenceTable = loadMacCJKSequences(f, 11);
		macChineseSimpConversionTable = loadCJKTable(f, 12, 256 + 92 * 94);
		macChineseSimpSequenceTable = loadMacCJKSequences(f, 13);
	}
}

void releaseCJKTables() {
	cjk_tables_loaded = false;
	delete[] windows932ConversionTable;
	windows932ConversionTable = 0;
	delete[] windows932ReverseConversionTable;
	windows932ReverseConversionTable = 0;
	delete[] windows936ConversionTable;
	windows936ConversionTable = 0;
	delete[] windows936ReverseConversionTable;
	windows936ReverseConversionTable = 0;
	delete[] windows949ConversionTable;
	windows949ConversionTable = 0;
	delete[] windows949ReverseConversionTable;
	windows949ReverseConversionTable = 0;
	delete[] windows950ConversionTable;
	windows950ConversionTable = 0;
	delete[] windows950ReverseConversionTable;
	windows950ReverseConversionTable = 0;
	delete[] johabConversionTable;
	johabConversionTable = 0;
	delete[] johabReverseConversionTable;
	johabReverseConversionTable = 0;
	delete[] traditional2SimplifiedChineseConversionTable;
	traditional2SimplifiedChineseConversionTable = 0;
	delete[] macKoreanConversionTable;
	macKoreanConversionTable = 0;
	delete macKoreanSequenceTable;
	macKoreanSequenceTable = 0;
	delete[] macKoreanReverseConversionTable;
	macKoreanReverseConversionTable = 0;
	delete[] macJapaneseConversionTable;
	macJapaneseConversionTable = 0;
	delete macJapaneseSequenceTable;
	macJapaneseSequenceTable = 0;
	delete[] macJapaneseReverseConversionTable;
	macJapaneseReverseConversionTable = 0;
	delete[] macChineseTradConversionTable;
	macChineseTradConversionTable = 0;
	delete macChineseTradSequenceTable;
	macChineseTradSequenceTable = 0;
	delete[] macChineseTradReverseConversionTable;
	macChineseTradReverseConversionTable = 0;
	delete[] macChineseSimpConversionTable;
	macChineseSimpConversionTable = 0;
	delete macChineseSimpSequenceTable;
	macChineseSimpSequenceTable = 0;
	delete[] macChineseSimpReverseConversionTable;
	macChineseSimpReverseConversionTable = 0;
}

void U32String::decodeWindows932(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint8 high = src[i++];

		if ((high & 0x80) == 0x00) {
			operator+=(high);
			continue;
		}

		// Katakana
		if (high >= 0xa1 && high <= 0xdf) {
			operator+=(high - 0xa1 + 0xFF61);
			continue;
		}

		if (i >= len) {
			operator+=(invalidCode);
			continue;
		}

		uint8 low = src[i++];
		if (low < 0x40) {
			operator+=(invalidCode);
			continue;
		}
		uint8 lowidx = low - 0x40;
		uint8 highidx;

		if (high >= 0x81 && high < 0x85)
			highidx = high - 0x81;
		else if (high >= 0x87 && high < 0xa0)
			highidx = high - 0x87 + 4;
		else if (high >= 0xe0 && high < 0xef)
			highidx = high - 0xe0 + 29;
		else if (high >= 0xfa && high < 0xfd)
			highidx = high - 0xfa + 44;
		else {
			operator+=(invalidCode);
			continue;
		}

		if (!windows932ConversionTable) {
			operator+=(invalidCode);
			continue;
		}

		// Main range
		uint16 val = windows932ConversionTable[highidx * 192 + lowidx];
		operator+=(val ? val : invalidCode);
	}
}

void U32String::decodeWindows936(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint8 high = src[i++];

		if ((high & 0x80) == 0x00) {
			operator+=(high);
			continue;
		}

		// Euro symbol
		if (high == 0x80) {
			operator+=(0x20ac);
			continue;
		}

		if (high == 0xff) {
			operator+=(invalidCode);
			continue;
		}

		uint8 low = src[i++];
		if (low < 0x40 || low == 0x7f || low == 0xff) {
			operator+=(invalidCode);
			continue;
		}
		uint8 lowidx = low > 0x7f ? low - 0x41 : low - 0x40;
		uint8 highidx = high - 0x81;

		if (!windows936ConversionTable) {
			operator+=(invalidCode);
			continue;
		}

		uint16 val = windows936ConversionTable[highidx * 190 + lowidx];
		operator+=(val ? val : invalidCode);
	}
}

static uint16 convertUHCToUCSReal(uint8 high, uint8 low) {
	uint lowidx = 0;
	if (low >= 0x41 && low < 0x5b)
		lowidx = low - 0x41;
	else if (low >= 0x61 && low < 0x7b)
		lowidx = low - 0x61 + 0x1a;
	else if (low >= 0x81 && low < 0xff)
		lowidx = low - 0x81 + 0x1a * 2;
	else
		return 0;
	if (!windows949ConversionTable)
		return 0;
	uint16 idx = (high - 0x81) * 0xb2 + lowidx;
	return windows949ConversionTable[idx];
}

uint16 convertUHCToUCS(uint8 high, uint8 low) {
	if (!cjk_tables_loaded)
		loadCJKTables();

	return convertUHCToUCSReal(high, low);
}


void U32String::decodeWindows949(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint8 high = src[i++];

		if ((high & 0x80) == 0x00) {
			operator+=(high);
			continue;
		}

		if (high == 0x80 || high == 0xff) {
			operator+=(invalidCode);
			continue;
		}

		if (i >= len) {
			operator+=(invalidCode);
			continue;
		}

		uint8 low = src[i++];
		uint16 val = convertUHCToUCSReal(high, low);

		operator+=(val ? val : invalidCode);
	}
}

void U32String::decodeWindows950(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint8 high = src[i++];

		if ((high & 0x80) == 0x00) {
			operator+=(high);
			continue;
		}

		// Euro symbol
		if (high == 0x80) {
			operator+=(0x20ac);
			continue;
		}

		if (high == 0xff) {
			operator+=(invalidCode);
			continue;
		}

		if (i >= len) {
			operator+=(invalidCode);
			continue;
		}

		uint8 low = src[i++];
		uint8 lowidx = low < 0x80 ? low - 0x40 : (low - 0xa1 + 0x3f);

		// Main range
		if (high >= 0xa1 && high < 0xfa) {
			uint16 val = windows950ConversionTable ?
				windows950ConversionTable[(high - 0xa1) * 157 + lowidx] : 0;
			operator+=(val ? val : invalidCode);
			continue;
		}

		// PUA range
		if (high <= 0x8d) {
			operator+=(0xeeb8 + 157 * (high-0x81) + lowidx);
			continue;
		}
		if (high <= 0xa0) {
			operator+=(0xe311 + (157 * (high-0x8e)) + lowidx);
			continue;
		}
		if (high >= 0xfa) {
			operator+=(0xe000 + (157 * (high-0xfa)) + lowidx);
			continue;
		}
	}
}

static uint16 convertJohabToUCSReal(uint8 high, uint8 low) {
	if (high >= 0x84 && high < 0xD4)
		high -= 0x84;
	else
		return 0;

	if (low >= 0x41 && low < 0x7F)
		low -= 0x41;
	else if (low >= 0x81 && low < 0xFF)
		low -= (0x81 - (0x7F - 0x41));
	else
		return 0;

	if (!johabConversionTable)
		return 0;

	uint16 idx = high * 188 + low;
	return johabConversionTable[idx];
}

void U32String::decodeJohab(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint8 high = src[i++];

		if ((high & 0x80) == 0x00) {
			operator+=(high);
			continue;
		}

		if (high == 0x80 || high == 0xff) {
			operator+=(invalidCode);
			continue;
		}

		if (i >= len) {
			operator+=(invalidCode);
			continue;
		}

		uint8 low = src[i++];
		uint16 val = convertJohabToUCSReal(high, low);

		operator+=(val ? val : invalidCode);
	}
}


StringEncodingResult String::encodeWindows932(const U32String &src, char errorChar) {
	StringEncodingResult encodingResult = kStringEncodingResultSucceeded;

	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!windows932ReverseConversionTable && windows932ConversionTable) {
		uint16 *rt = new uint16[0x10000]();
		for (uint highidx = 0; highidx < 47; highidx++) {
			uint8 high = 0;
			if (highidx < 4)
				high = highidx + 0x81;
			else if (highidx < 29)
				high = highidx + 0x87 - 4;
			else if (highidx < 44)
				high = highidx + 0xe0 - 29;
			else
				high = highidx + 0xfa - 44;

			for (uint lowidx = 0; lowidx < 192; lowidx++) {
				uint8 low = lowidx + 0x40;
				uint16 unicode = windows932ConversionTable[highidx * 192 + lowidx];

				rt[unicode] = (high << 8) | low;
			}
		}
		windows932ReverseConversionTable = rt;
	}

	for (uint i = 0; i < src.size();) {
		uint32 point = src[i++];

		if (point < 0x80) {
			operator+=(point);
			continue;
		}

		// Katakana
		if (point >= 0xff61 && point <= 0xff9f) {
			operator+=(0xa1 + (point - 0xFF61));
			continue;
		}

		if (point > 0x10000) {
			operator+=(errorChar);
			encodingResult = kStringEncodingResultHasErrors;
			continue;
		}

		if (!windows932ReverseConversionTable) {
			operator+=(errorChar);
			encodingResult = kStringEncodingResultHasErrors;
			continue;
		}

		uint16 rev = windows932ReverseConversionTable[point];
		if (rev != 0) {
			operator+=(rev >> 8);
			operator+=(rev & 0xff);
			continue;
		}

		// This codepage contains cyrillic, so no need to transliterate

		operator+=(errorChar);
		encodingResult = kStringEncodingResultHasErrors;
		continue;
	}

	return encodingResult;
}

StringEncodingResult String::encodeWindows936(const U32String &src, char errorChar) {
	StringEncodingResult encodingResult = kStringEncodingResultSucceeded;

	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!windows936ReverseConversionTable && windows936ConversionTable) {
		uint16 *rt = new uint16[0x10000]();
		for (uint highidx = 0; highidx < 190; highidx++) {
			uint8 high = highidx + 0x81;
			for (uint lowidx = 0; lowidx < 192; lowidx++) {
				uint8 low = lowidx + 0x40;
				if (low >= 0x7f)
					low++;
				uint16 unicode = windows936ConversionTable[highidx * 190 + lowidx];

				rt[unicode] = (high << 8) | low;
			}
		}
		windows936ReverseConversionTable = rt;
	}

	for (uint i = 0; i < src.size();) {
		uint32 point = src[i++];

		if (point < 0x80) {
			operator+=(point);
			continue;
		}

		if (point > 0x10000) {
			operator+=(errorChar);
			encodingResult = kStringEncodingResultHasErrors;
			continue;
		}

		if (!windows936ReverseConversionTable) {
			operator+=(errorChar);
			encodingResult = kStringEncodingResultHasErrors;
			continue;
		}

		uint16 rev = windows936ReverseConversionTable[point];
		if (rev != 0) {
			operator+=(rev >> 8);
			operator+=(rev & 0xff);
			continue;
		}

		// This codepage contains cyrillic, so no need to transliterate

		operator+=(errorChar);
		encodingResult = kStringEncodingResultHasErrors;
		continue;
	}

	return encodingResult;
}

StringEncodingResult String::encodeWindows949(const U32String &src, char errorChar) {
	StringEncodingResult encodingResult = kStringEncodingResultSucceeded;

	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!windows949ReverseConversionTable && windows949ConversionTable) {
		uint16 *rt = new uint16[0x10000]();

		for (uint lowidx = 0; lowidx < 0xb2; lowidx++) {
			uint8 low = 0;
			if (lowidx < 0x1a)
				low = 0x41 + lowidx;
			else if (lowidx < 0x1a * 2)
				low = 0x61 + lowidx - 0x1a;
			else
				low = 0x81 + lowidx - 0x1a * 2;

			for (uint highidx = 0; highidx < 0x7e; highidx++) {
				uint8 high = highidx + 0x81;
				uint16 unicode = windows949ConversionTable[highidx * 0xb2 + lowidx];

				rt[unicode] = (high << 8) | low;
			}
		}

		windows949ReverseConversionTable = rt;
	}

	for (uint i = 0; i < src.size();) {
		uint32 point = src[i++];

		if (point < 0x80) {
			operator+=(point);
			continue;
		}

		if (point > 0x10000 || !windows949ReverseConversionTable) {
			operator+=(errorChar);
			encodingResult = kStringEncodingResultHasErrors;
			continue;
		}

		uint16 rev = windows949ReverseConversionTable[point];
		if (rev == 0) {
			// This codepage contains cyrillic, so no need to transliterate
			operator+=(errorChar);
			encodingResult = kStringEncodingResultHasErrors;
			continue;
		}

		operator+=(rev >> 8);
		operator+=(rev & 0xff);
	}

	return encodingResult;
}

static const char g_cyrillicTransliterationTable[] = {
	' ', 'E', 'D', 'G', 'E', 'Z', 'I', 'I', 'J', 'L', 'N', 'C', 'K', 'I', 'U', 'D',
	'A', 'B', 'V', 'G', 'D', 'E', 'Z', 'Z', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'R', 'S', 'T', 'U', 'F', 'H', 'C', 'C', 'S', 'S', '\"', 'Y', '\'', 'E', 'U', 'A',
	'a', 'b', 'v', 'g', 'd', 'e', 'z', 'z', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p',
	'r', 's', 't', 'u', 'f', 'h', 'c', 'c', 's', 's', '\"', 'y', '\'', 'e', 'u', 'a',
	'e', 'e', 'd', 'g', 'e', 'z', 'i', 'i', 'j', 'l', 'n', 'c', 'k', 'i', 'u', 'd',
};

StringEncodingResult String::translitChar(U32String::value_type point, char errorChar) {
	if (point == 0xa0) {
		operator+=(' ');
		return kStringEncodingResultSucceeded;
	}

	if (point == 0xad) {
		operator+=('-');
		return kStringEncodingResultSucceeded;
	}

	if (point == 0x2116) {
		operator+=('N');
		return kStringEncodingResultSucceeded;
	}

	if (point >= 0x401 && point <= 0x45f) {
		operator+=(g_cyrillicTransliterationTable[point - 0x400]);
		return kStringEncodingResultSucceeded;
	}

	operator+=(errorChar);
	return kStringEncodingResultHasErrors;
}

StringEncodingResult String::encodeWindows950(const U32String &src, bool transliterate, char errorChar) {
	StringEncodingResult encodingResult = kStringEncodingResultSucceeded;

	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!windows950ReverseConversionTable && windows950ConversionTable) {
		uint16 *rt = new uint16[0x10000]();

		for (uint lowidx = 0; lowidx < 157; lowidx++) {
			uint8 low = 0;
			if (lowidx < 0x3f)
				low = 0x40 + lowidx;
			else
				low = 0xa1 + lowidx - 0x3f;

			for (uint highidx = 0; highidx < 89; highidx++) {
				uint8 high = highidx + 0xa1;
				uint16 unicode = windows950ConversionTable[highidx * 157 + lowidx];

				rt[unicode] = (high << 8) | low;
			}
		}

		windows950ReverseConversionTable = rt;
	}

	for (uint i = 0; i < src.size();) {
		uint32 point = src[i++];

		if (point < 0x80) {
			operator+=(point);
			continue;
		}

		if (point > 0x10000) {
			operator+=(errorChar);
			encodingResult = kStringEncodingResultHasErrors;
			continue;
		}

		// Euro symbol
		if (point == 0x20ac) {
			operator+=((char) 0x80);
			continue;
		}

		if (!windows950ReverseConversionTable) {
			operator+=(errorChar);
			encodingResult = kStringEncodingResultHasErrors;
			continue;
		}

		uint16 rev = windows950ReverseConversionTable[point];
		if (rev != 0) {
			operator+=(rev >> 8);
			operator+=(rev & 0xff);
			continue;
		}

		// PUA range
		if (point >= 0xe000 && point <= 0xf848) {
			byte lowidx = 0, high = 0, low = 0;
			if (point <= 0xe310) {
				high = (point - 0xe000) / 157 + 0xfa;
				lowidx = (point - 0xe000) % 157;
			} else if (point <= 0xeeb7) {
				high = (point - 0xe311) / 157 + 0x8e;
				lowidx = (point - 0xe311) % 157;
			} else if (point <= 0xf6b0) {
				high = (point - 0xeeb8) / 157 + 0x81;
				lowidx = (point - 0xeeb8) % 157;
			} else {
				high = (point - 0xf672) / 157 + 0xc6;
				lowidx = (point - 0xf672) % 157;
			}

			if (lowidx <= 0x3e)
				low = 0x40 + lowidx;
			else
				low = 0x62 + lowidx;

			operator+=(high);
			operator+=(low);
			windows950ReverseConversionTable[point] = (high << 8) | low;
			continue;
		}

		if (transliterate) {
			StringEncodingResult translitResult = translitChar(point, errorChar);
			if (translitResult != kStringEncodingResultSucceeded)
				encodingResult = translitResult;
			continue;
		}

		operator+=(errorChar);
		encodingResult = kStringEncodingResultHasErrors;
		continue;
	}

	return encodingResult;
}

StringEncodingResult String::encodeJohab(const U32String &src, char errorChar) {
	StringEncodingResult encodingResult = kStringEncodingResultSucceeded;

	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!johabReverseConversionTable && johabConversionTable) {
		uint16 *rt = new uint16[0x10000]();
		for (uint lowidx = 0; lowidx < 188; lowidx++) {
			uint8 low = 0;
			if (lowidx < (0x7F - 0x41))
				low = 0x41 + lowidx;
			else
				low = 0x81 + lowidx - (0x7F - 0x41);

			for (uint highidx = 0; highidx < 80; highidx++) {
				uint8 high = highidx + 0x84;
				uint16 unicode = johabConversionTable[highidx * 188 + lowidx];
				rt[unicode] = (high << 8) | low;
			}
		}
		johabReverseConversionTable = rt;
	}

	for (uint i = 0; i < src.size();) {
		uint32 point = src[i++];

		if (point < 0x80) {
			operator+=(point);
			continue;
		}

		if (point > 0x10000 || !johabReverseConversionTable) {
			operator+=(errorChar);
			encodingResult = kStringEncodingResultHasErrors;
			continue;
		}

		uint16 rev = johabReverseConversionTable[point];
		if (rev == 0) {
			operator+=(errorChar);
			encodingResult = kStringEncodingResultHasErrors;
			continue;
		}

		operator+=(rev >> 8);
		operator+=(rev & 0xff);
	}

	return encodingResult;
}

static uint16 convertMacKoreanToUnicode(uint16 code) {
	const uint16 *table = macKoreanConversionTable;
	if (!table)
		return 0;
	if (code < 256)
		return table[code];
	const uint high = code >> 8;
	const uint low = code & 0xff;
	if (high < 0xa1 || 0xfe < high || low < 0x41 || 0xfe < low)
		return 0;
	return table[256 + (high - 0xa1) * 190 + low - 0x41];
}

static uint16 convertMacJapaneseToUnicode(uint16 code) {
	const uint16 *table = macJapaneseConversionTable;
	if (!table)
		return 0;
	if (code < 256)
		return table[code];
	const uint high = code >> 8;
	const uint low = code & 0xff;
	if (low < 0x40 || low == 0x7f || 0xfc < low)
		return 0;
	uint row;
	if (0x81 <= high && high <= 0x9f)
		row = high - 0x81;
	else if (0xe0 <= high && high <= 0xfc)
		row = high - 0xe0 + 31;
	else
		return 0;
	return table[256 + row * 192 + low - 0x40];
}

static uint16 convertMacChineseTradToUnicode(uint16 code) {
	const uint16 *table = macChineseTradConversionTable;
	if (!table)
		return 0;
	if (code < 256)
		return table[code];
	const uint high = code >> 8;
	const uint low = code & 0xff;
	if (high < 0xa1 || 0xfc < high)
		return 0;
	uint column;
	if (0x40 <= low && low <= 0x7e)
		column = low - 0x40;
	else if (0xa1 <= low && low <= 0xfe)
		column = low - 0xa1 + 63;
	else
		return 0;
	return table[256 + (high - 0xa1) * 157 + column];
}

static uint16 convertMacChineseSimpToUnicode(uint16 code) {
	const uint16 *table = macChineseSimpConversionTable;
	if (!table)
		return 0;
	if (code < 256)
		return table[code];
	const uint high = code >> 8;
	const uint low = code & 0xff;
	if (high < 0xa1 || 0xfc < high || low < 0xa1 || 0xfe < low)
		return 0;
	return table[256 + (high - 0xa1) * 94 + low - 0xa1];
}

// Search the record indices sorted by encoded character.
static const MacCJKSequence *findMacCJKSequence(uint16 code, const MacCJKSequenceTable *table) {
	if (!table)
		return nullptr;
	uint first = 0;
	uint last = table->count;
	while (first < last) {
		const uint middle = first + (last - first) / 2;
		const MacCJKSequence &record = table->records[table->encodedOrder[middle]];
		if (record.encoded < code)
			first = middle + 1;
		else if (code < record.encoded)
			last = middle;
		else
			return &record;
	}
	return nullptr;
}

void U32String::decodeMacKorean(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint16 code = static_cast<uint8>(src[i]);
		i += 1;

		const bool lead = 0xa1 <= code && code <= 0xfe;
		if (lead) {
			if (i == len) {
				operator+=(invalidCode);
				continue;
			}

			const uint8 low = static_cast<uint8>(src[i]);
			const bool trail = (0xa1 <= low && low <= 0xfe) ||
				(code <= 0xad && ((0x41 <= low && low <= 0x7d) || (0x81 <= low && low <= 0xa0)));
			if (!trail) {
				operator+=(invalidCode);
				continue;
			}

			code = (code << 8) | low;
			i += 1;
		}

		const uint16 unicode = convertMacKoreanToUnicode(code);
		if (unicode || code == 0) {
			operator+=(unicode);
		} else {
			const MacCJKSequence *sequence = findMacCJKSequence(code, macKoreanSequenceTable);
			if (sequence) {
				for (uint j = 0; j < ARRAYSIZE(sequence->unicode) && sequence->unicode[j]; j++)
					operator+=(sequence->unicode[j]);
			} else {
				operator+=(invalidCode);
			}
		}
	}
}

void U32String::decodeMacJapanese(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint16 code = static_cast<uint8>(src[i]);
		i += 1;

		const bool lead = (0x81 <= code && code <= 0x9f) || (0xe0 <= code && code <= 0xfc);
		if (lead) {
			if (i == len) {
				operator+=(invalidCode);
				continue;
			}

			const uint8 low = static_cast<uint8>(src[i]);
			const bool trail = 0x40 <= low && low <= 0xfc && low != 0x7f;
			if (!trail) {
				operator+=(invalidCode);
				continue;
			}

			code = (code << 8) | low;
			i += 1;
		}

		const uint16 unicode = convertMacJapaneseToUnicode(code);
		if (unicode || code == 0) {
			operator+=(unicode);
		} else {
			const MacCJKSequence *sequence = findMacCJKSequence(code, macJapaneseSequenceTable);
			if (sequence) {
				for (uint j = 0; j < ARRAYSIZE(sequence->unicode) && sequence->unicode[j]; j++)
					operator+=(sequence->unicode[j]);
			} else {
				operator+=(invalidCode);
			}
		}
	}
}

void U32String::decodeMacChineseTrad(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint16 code = static_cast<uint8>(src[i]);
		i += 1;

		const bool lead = 0xa1 <= code && code <= 0xfc;
		if (lead) {
			if (i == len) {
				operator+=(invalidCode);
				continue;
			}

			const uint8 low = static_cast<uint8>(src[i]);
			const bool trail = (0x40 <= low && low <= 0x7e) || (0xa1 <= low && low <= 0xfe);
			if (!trail) {
				operator+=(invalidCode);
				continue;
			}

			code = (code << 8) | low;
			i += 1;
		}

		const uint16 unicode = convertMacChineseTradToUnicode(code);
		if (unicode || code == 0) {
			operator+=(unicode);
		} else {
			const MacCJKSequence *sequence = findMacCJKSequence(code, macChineseTradSequenceTable);
			if (sequence) {
				for (uint j = 0; j < ARRAYSIZE(sequence->unicode) && sequence->unicode[j]; j++)
					operator+=(sequence->unicode[j]);
			} else {
				operator+=(invalidCode);
			}
		}
	}
}

void U32String::decodeMacChineseSimp(const char *src, uint32 len) {
	ensureCapacity(len, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	for (uint i = 0; i < len;) {
		uint16 code = static_cast<uint8>(src[i]);
		i += 1;

		const bool lead = 0xa1 <= code && code <= 0xfc;
		if (lead) {
			if (i == len) {
				operator+=(invalidCode);
				continue;
			}

			const uint8 low = static_cast<uint8>(src[i]);
			const bool trail = 0xa1 <= low && low <= 0xfe;
			if (!trail) {
				operator+=(invalidCode);
				continue;
			}

			code = (code << 8) | low;
			i += 1;
		}

		const uint16 unicode = convertMacChineseSimpToUnicode(code);
		if (unicode || code == 0) {
			operator+=(unicode);
		} else {
			const MacCJKSequence *sequence = findMacCJKSequence(code, macChineseSimpSequenceTable);
			if (sequence) {
				for (uint j = 0; j < ARRAYSIZE(sequence->unicode) && sequence->unicode[j]; j++)
					operator+=(sequence->unicode[j]);
			} else {
				operator+=(invalidCode);
			}
		}
	}
}

// Like ICU, prefer the longest exact sequence over a single-code-point mapping.
// This retains Apple's grouping and variant hints.
static const MacCJKSequence *findMacCJKSequence(const U32String &src, uint position, const MacCJKSequenceTable *table, uint &length) {
	if (!table)
		return nullptr;
	for (length = MIN<uint>(5, src.size() - position); 2 <= length; length -= 1) {
		uint first = 0;
		uint last = table->count;
		while (first < last) {
			const uint middle = first + (last - first) / 2;
			const MacCJKSequence &record = table->records[middle];
			int comparison = 0;
			for (uint i = 0; i < length; i++) {
				if (!record.unicode[i] || record.unicode[i] < src[position + i]) {
					comparison = -1;
					break;
				}
				if (src[position + i] < record.unicode[i]) {
					comparison = 1;
					break;
				}
			}
			if (!comparison && length < ARRAYSIZE(record.unicode) && record.unicode[length])
				comparison = 1;
			if (comparison < 0)
				first = middle + 1;
			else if (0 < comparison)
				last = middle;
			else
				return &record;
		}
	}
	return nullptr;
}

StringEncodingResult String::encodeMacKorean(const U32String &src, char errorChar) {
	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!macKoreanReverseConversionTable) {
		macKoreanReverseConversionTable = new uint16[0x10000]();

		for (uint code = 0; code < 0x10000; code++) {
			const uint16 unicode = convertMacKoreanToUnicode(code);
			if (unicode)
				macKoreanReverseConversionTable[unicode] = code;
		}
	}

	StringEncodingResult result = kStringEncodingResultSucceeded;

	for (uint i = 0; i < src.size();) {
		uint length;
		const MacCJKSequence *sequence = findMacCJKSequence(src, i, macKoreanSequenceTable, length);
		uint16 code = 0;

		if (sequence) {
			code = sequence->encoded;
			i += length;
		} else {
			const uint32 point = src[i];
			i += 1;

			if (point < 0x10000)
				code = macKoreanReverseConversionTable[point];

			if (!code && point != 0) {
				operator+=(errorChar);
				result = kStringEncodingResultHasErrors;
				continue;
			}
		}

		if (0xff < code)
			operator+=(static_cast<char>(code >> 8));
		operator+=(static_cast<char>(code & 0xff));
	}

	return result;
}

StringEncodingResult String::encodeMacJapanese(const U32String &src, char errorChar) {
	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!macJapaneseReverseConversionTable) {
		macJapaneseReverseConversionTable = new uint16[0x10000]();

		for (uint code = 0; code < 0x10000; code++) {
			const uint16 unicode = convertMacJapaneseToUnicode(code);
			if (unicode)
				macJapaneseReverseConversionTable[unicode] = code;
		}
	}

	StringEncodingResult result = kStringEncodingResultSucceeded;

	for (uint i = 0; i < src.size();) {
		uint length;
		const MacCJKSequence *sequence = findMacCJKSequence(src, i, macJapaneseSequenceTable, length);
		uint16 code = 0;

		if (sequence) {
			code = sequence->encoded;
			i += length;
		} else {
			const uint32 point = src[i];
			i += 1;

			if (point < 0x10000)
				code = macJapaneseReverseConversionTable[point];

			if (!code && point != 0) {
				operator+=(errorChar);
				result = kStringEncodingResultHasErrors;
				continue;
			}
		}

		if (0xff < code)
			operator+=(static_cast<char>(code >> 8));
		operator+=(static_cast<char>(code & 0xff));
	}

	return result;
}

StringEncodingResult String::encodeMacChineseTrad(const U32String &src, char errorChar) {
	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!macChineseTradReverseConversionTable) {
		macChineseTradReverseConversionTable = new uint16[0x10000]();

		for (uint code = 0; code < 0x10000; code++) {
			const uint16 unicode = convertMacChineseTradToUnicode(code);
			if (unicode)
				macChineseTradReverseConversionTable[unicode] = code;
		}
	}

	StringEncodingResult result = kStringEncodingResultSucceeded;

	for (uint i = 0; i < src.size();) {
		uint length;
		const MacCJKSequence *sequence = findMacCJKSequence(src, i, macChineseTradSequenceTable, length);
		uint16 code = 0;

		if (sequence) {
			code = sequence->encoded;
			i += length;
		} else {
			const uint32 point = src[i];
			i += 1;

			if (point < 0x10000)
				code = macChineseTradReverseConversionTable[point];

			if (!code && point != 0) {
				operator+=(errorChar);
				result = kStringEncodingResultHasErrors;
				continue;
			}
		}

		if (0xff < code)
			operator+=(static_cast<char>(code >> 8));
		operator+=(static_cast<char>(code & 0xff));
	}

	return result;
}

StringEncodingResult String::encodeMacChineseSimp(const U32String &src, char errorChar) {
	ensureCapacity(src.size() * 2, false);

	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!macChineseSimpReverseConversionTable) {
		macChineseSimpReverseConversionTable = new uint16[0x10000]();

		for (uint code = 0; code < 0x10000; code++) {
			const uint16 unicode = convertMacChineseSimpToUnicode(code);
			if (unicode)
				macChineseSimpReverseConversionTable[unicode] = code;
		}
	}

	StringEncodingResult result = kStringEncodingResultSucceeded;

	for (uint i = 0; i < src.size();) {
		uint length;
		const MacCJKSequence *sequence = findMacCJKSequence(src, i, macChineseSimpSequenceTable, length);
		uint16 code = 0;

		if (sequence) {
			code = sequence->encoded;
			i += length;
		} else {
			const uint32 point = src[i];
			i += 1;

			if (point < 0x10000)
				code = macChineseSimpReverseConversionTable[point];

			if (!code && point != 0) {
				operator+=(errorChar);
				result = kStringEncodingResultHasErrors;
				continue;
			}
		}

		if (0xff < code)
			operator+=(static_cast<char>(code >> 8));
		operator+=(static_cast<char>(code & 0xff));
	}

	return result;
}

// //TODO: This is a quick and dirty converter. Refactoring needed:
// 1. Original version has an option for performing strict / nonstrict
//    conversion for the 0xD800...0xDFFF interval
// 2. Original version returns a result code. This version inserts '0xFFFD' if
//    character does not fit in 4 bytes & does not inform caller on any errors
//
// More comprehensive one lives in wintermute/utils/convert_utf.cpp
StringEncodingResult String::encodeUTF8(const U32String &src, char errorChar) {
	ensureCapacity(src.size(), false);
	static const uint8 firstByteMark[5] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0 };
	char writingBytes[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

	uint i = 0;
	while (i < src.size()) {
		unsigned short bytesToWrite = 0;
		const uint32 byteMask = 0xBF;
		const uint32 byteMark = 0x80;

		uint32 ch = src[i++];
		if (ch < (uint32)0x80) {
			bytesToWrite = 1;
		} else if (ch < (uint32)0x800) {
			bytesToWrite = 2;
		} else if (ch < (uint32)0x10000) {
			bytesToWrite = 3;
		} else if (ch <= 0x0010FFFF) {
			bytesToWrite = 4;
		} else {
			bytesToWrite = 3;
			ch = invalidCode;
		}

		char *pBytes = writingBytes + (4 - bytesToWrite);

		switch (bytesToWrite) {
		case 4:
			pBytes[3] = (char)((ch | byteMark) & byteMask);
			ch >>= 6;
			// fallthrough
		case 3:
			pBytes[2] = (char)((ch | byteMark) & byteMask);
			ch >>= 6;
			// fallthrough
		case 2:
			pBytes[1] = (char)((ch | byteMark) & byteMask);
			ch >>= 6;
			// fallthrough
		case 1:
			pBytes[0] = (char)(ch | firstByteMark[bytesToWrite]);
			break;
		default:
			break;
		}

		operator+=(pBytes);
	}

	return kStringEncodingResultSucceeded;
}

#define decodeUTF16Template(suffix, read)				\
Common::U32String U32String::decodeUTF16 ## suffix (const uint16 *start, uint len) {	\
	const uint16 *ptr = start;					\
	Common::U32String dst;						\
	dst.ensureCapacity(len, false);					\
									\
	while (len > 0) {						\
		uint16 c = read(ptr++);					\
		len--;							\
		if (c >= 0xD800 && c <= 0xDBFF && len > 0) {		\
			uint16 low = read(ptr);				\
			if (low >= 0xDC00 && low <= 0xDFFF) {		\
				/* low is OK, we can advance pointer */	\
				ptr++; len--;				\
				dst += ((c & 0x3ff) << 10)		\
					| (low & 0x3ff);		\
			} else {					\
				dst += invalidCode;			\
			}						\
			continue;					\
		}							\
									\
		if (c >= 0xD800 && c <= 0xDFFF) {			\
			dst += invalidCode;				\
			continue;					\
		}							\
		dst += c;						\
	}								\
									\
	return dst;							\
}

decodeUTF16Template(BE, READ_BE_UINT16)
decodeUTF16Template(LE, READ_LE_UINT16)
decodeUTF16Template(Native, READ_UINT16)

#define encodeUTF16Template(suffix, write)				\
uint16 *U32String::encodeUTF16 ## suffix (uint *len) const {		\
	uint16 *out = new uint16[_size * 2 + 1];			\
	uint16 *ptr = out;						\
									\
	for (uint i = 0; i < _size; i++) {				\
		uint32 c = _str[i];					\
		if (c < 0x10000) {					\
			write(ptr++, c);				\
			continue;					\
		}							\
		write (ptr++, 0xD800 | ((c >> 10) & 0x3ff));		\
		write (ptr++, 0xDC00 | (c & 0x3ff));			\
	}								\
									\
	write(ptr, 0);							\
	if (len)							\
		*len = ptr - out;					\
									\
	return out;							\
}

encodeUTF16Template(BE, WRITE_BE_UINT16)
encodeUTF16Template(LE, WRITE_LE_UINT16)
encodeUTF16Template(Native, WRITE_UINT16)

// Upper bound on unicode codepoint in any single-byte encoding. Must be divisible by 0x100 and be strictly above large codepoint
static const int kMaxCharSingleByte = 0x3000;

static const uint16 *
getConversionTable(CodePage page) {
	switch (page) {
	case kWindows1250:
		return kWindows1250ConversionTable;
	case kWindows1251:
		return kWindows1251ConversionTable;
	case kWindows1252:
		return kWindows1252ConversionTable;
	case kWindows1253:
		return kWindows1253ConversionTable;
	case kWindows1254:
		return kWindows1254ConversionTable;
	case kWindows1255:
		return kWindows1255ConversionTable;
	case kWindows1256:
		return kWindows1256ConversionTable;
	case kWindows1257:
		return kWindows1257ConversionTable;
	case kMacCentralEurope:
		return kMacCentralEuropeConversionTable;
	case kMacRoman:
		return kMacRomanConversionTable;
	case kISO8859_1:
		return kLatin1ConversionTable;
	case kISO8859_2:
		return kLatin2ConversionTable;
	case kISO8859_5:
		return kISO5ConversionTable;
	case kDos850:
		return kDos850ConversionTable;
	case kDos862:
		return kDos862ConversionTable;
	case kDos866:
		return kDos866ConversionTable;
	case kASCII:
		return kASCIIConversionTable;

	case kCodePageInvalid:
	// Multibyte encodings. Can't be represented in simple table way
	case kUtf8:
	case kWindows932:
	case kWindows936:
	case kWindows949:
	case kWindows950:
	case kMacKorean:
	case kMacJapanese:
	case kMacChineseTrad:
	case kMacChineseSimp:
	case kJohab:
		return nullptr;
	}
	return nullptr;
}

struct ReverseTablePrefixTreeLevel1 {
	struct ReverseTablePrefixTreeLevel2 *next[kMaxCharSingleByte / 0x100];
	bool valid;
};

struct ReverseTablePrefixTreeLevel2 {
	uint8 end[256];

	ReverseTablePrefixTreeLevel2() {
		memset(end, 0, sizeof(end));
	}
};

ReverseTablePrefixTreeLevel1 reverseTables[kLastEncoding + 1];

static const ReverseTablePrefixTreeLevel1 *
getReverseConversionTable(CodePage page) {
	if (reverseTables[page].valid)
		return &reverseTables[page];
	const uint16 *conversionTable = getConversionTable(page);
	if (!conversionTable)
		return nullptr;
	reverseTables[page].valid = true;
	for (uint i = 0; i < 0x80; i++) {
		uint32 c = conversionTable[i];
		if (c == 0 || c >= kMaxCharSingleByte)
			continue;
		if (!reverseTables[page].next[c >> 8]) {
			reverseTables[page].next[c >> 8] = new ReverseTablePrefixTreeLevel2();
		}

		reverseTables[page].next[c >> 8]->end[c&0xff] = i | 0x80;
	}

	return &reverseTables[page];
}

void U32String::decodeOneByte(const char *src, uint32 len, CodePage page) {
	const uint16 *conversionTable = getConversionTable(page);

	if (conversionTable == nullptr) {
		conversionTable = kASCIIConversionTable;
	}

	ensureCapacity(len, false);

	for (uint i = 0; i < len; ++i) {
		if ((src[i] & 0x80) == 0) {
			operator+=(src[i]);
			continue;
		}

		uint16 val = conversionTable[src[i] & 0x7f];
		operator+=(val ? val : invalidCode);
	}
}

StringEncodingResult String::encodeOneByte(const U32String &src, CodePage page, bool transliterate, char errorChar) {
	StringEncodingResult encodingResult = kStringEncodingResultSucceeded;

	const ReverseTablePrefixTreeLevel1 *conversionTable =
		getReverseConversionTable(page);

	ensureCapacity(src.size(), false);

	if (conversionTable == nullptr) {
		for (uint i = 0; i < src.size(); ++i) {
			uint32 c = src[i];
			if (c <= 0x7F) {
				operator+=((char)c);
				continue;
			}

			if (transliterate) {
				StringEncodingResult translitResult = translitChar(c, errorChar);
				if (translitResult != kStringEncodingResultSucceeded)
					encodingResult = translitResult;
			} else {
				operator+=(errorChar);
				encodingResult = kStringEncodingResultHasErrors;
			}
		}
		return encodingResult;
	}

	for (uint i = 0; i < src.size(); ++i) {
		uint32 c = src[i];
		if (c <= 0x7F) {
			operator+=((char)c);
			continue;
		}

		if (c >= kMaxCharSingleByte)
			continue;
		ReverseTablePrefixTreeLevel2 *l2 = conversionTable->next[c>>8];
		unsigned char uc = l2 ? l2->end[c&0xff] : 0;
		if (uc != 0) {
			operator+=((char)uc);
			continue;
		}

		if (transliterate) {
			StringEncodingResult translitResult = translitChar(c, errorChar);
			if (translitResult != kStringEncodingResultSucceeded)
				encodingResult = translitResult;
		} else {
			operator+=(errorChar);
			encodingResult = kStringEncodingResultHasErrors;
		}
	}

	return encodingResult;
}

StringEncodingResult String::encodeInternal(const U32String &src, CodePage page, char errorChar) {
	switch(page) {
	case kUtf8:
		return encodeUTF8(src, errorChar);
	case kWindows932:
		return encodeWindows932(src, errorChar);
	case kWindows936:
		return encodeWindows936(src, errorChar);
	case kWindows949:
		return encodeWindows949(src, errorChar);
	case kWindows950:
		return encodeWindows950(src, true, errorChar);
	case kMacKorean:
		return encodeMacKorean(src, errorChar);
	case kMacJapanese:
		return encodeMacJapanese(src, errorChar);
	case kMacChineseTrad:
		return encodeMacChineseTrad(src, errorChar);
	case kMacChineseSimp:
		return encodeMacChineseSimp(src, errorChar);
	case kJohab:
		return encodeJohab(src, errorChar);
	default:
		return encodeOneByte(src, page, true, errorChar);
	}
}

U32String convertToU32String(const char *str, CodePage page) {
	return String(str).decode(page);
}

U32String convertUtf8ToUtf32(const String &str) {
	return str.decode(kUtf8);
}

String convertFromU32String(const U32String &string, CodePage page) {
	return string.encode(page);
}

String convertUtf32ToUtf8(const U32String &u32str) {
	return u32str.encode(kUtf8);
}

void U32String::decodeInternal(const char *str, uint32 len, CodePage page) {
	assert(str);

	_storage[0] = 0;
	_size = 0;

	switch(page) {
	case kUtf8:
		decodeUTF8(str, len);
		break;
	case kWindows932:
		decodeWindows932(str, len);
		break;
	case kWindows936:
		decodeWindows936(str, len);
		break;
	case kWindows949:
		decodeWindows949(str, len);
		break;
	case kWindows950:
		decodeWindows950(str, len);
		break;
	case kMacKorean:
		decodeMacKorean(str, len);
		break;
	case kMacJapanese:
		decodeMacJapanese(str, len);
		break;
	case kMacChineseTrad:
		decodeMacChineseTrad(str, len);
		break;
	case kMacChineseSimp:
		decodeMacChineseSimp(str, len);
		break;
	case kJohab:
		decodeJohab(str, len);
		break;
	default:
		decodeOneByte(str, len, page);
		break;
	}
}

U32String String::decode(CodePage page) const {
	if (page == kCodePageInvalid ||
			page > kLastEncoding) {
		error("Invalid codepage");
	}

	U32String unicodeString;
	unicodeString.decodeInternal(_str, _size, page);
	return unicodeString;
}

String U32String::encode(CodePage page) const {
	String string;
	(void)encode(string, page, '?');
	return string;
}

StringEncodingResult U32String::encode(String &outString, CodePage page, char errorChar) const {
	if (page == kCodePageInvalid ||
			page > kLastEncoding) {
		error("Invalid codepage");
	}

	return outString.encodeInternal(*this, page, errorChar);
}

U32String U32String::transcodeChineseT2S() const {
	if (!cjk_tables_loaded)
		loadCJKTables();

	if (!traditional2SimplifiedChineseConversionTable)
		return *this;

	U32String simplified;
	for (uint32 i = 0; i < size(); i++) {
		uint32 ch = _str[i];
		if (ch >= 0x10000)
			simplified += ch;
		else
			simplified += traditional2SimplifiedChineseConversionTable[ch];
	}
	return simplified;
}

} // End of namespace Common
