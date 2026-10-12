#include <cxxtest/TestSuite.h>

#include "trinket/compression/lzo1x.h"

class LZO1XTestSuite : public CxxTest::TestSuite {
private:
	static constexpr byte kGuard = 165;

	void checkSmallBlock(const byte *packed, size_t packedSize, const byte *expected, size_t expectedSize) {
		byte guarded[256];
		TS_ASSERT_LESS_THAN(expectedSize + 34, sizeof(guarded));
		for (size_t capacity = 0; capacity <= expectedSize + 16; capacity++) {
			memset(guarded, kGuard, sizeof(guarded));
			size_t written = 123;
			const Trinket::LZO1XStatus status = Trinket::decompressLZO1X(packed, packedSize, guarded + 8, capacity, written);
			TS_ASSERT_LESS_THAN_EQUALS(written, capacity);
			if (capacity < expectedSize) {
				TS_ASSERT(status == Trinket::LZO1XStatus::kOutputOverflow);
			} else {
				TS_ASSERT(status == Trinket::LZO1XStatus::kSuccess);
				TS_ASSERT_EQUALS(written, expectedSize);
			}
			TS_ASSERT_SAME_DATA(guarded + 8, expected, written);
			for (size_t i = 0; i < 8; i++)
				TS_ASSERT_EQUALS(guarded[i], kGuard);
			for (size_t i = 8 + written; i < sizeof(guarded); i++)
				TS_ASSERT_EQUALS(guarded[i], kGuard);
		}
		for (size_t size = 0; size < packedSize; size++) {
			memset(guarded, kGuard, sizeof(guarded));
			size_t written = 123;
			const Trinket::LZO1XStatus status = Trinket::decompressLZO1X(packed, size, guarded + 8, expectedSize, written);
			TS_ASSERT(status == Trinket::LZO1XStatus::kInputUnderrun);
			TS_ASSERT_LESS_THAN_EQUALS(written, expectedSize);
			TS_ASSERT_SAME_DATA(guarded + 8, expected, written);
			for (size_t i = 0; i < 8; i++)
				TS_ASSERT_EQUALS(guarded[i], kGuard);
			for (size_t i = 8 + written; i < sizeof(guarded); i++)
				TS_ASSERT_EQUALS(guarded[i], kGuard);
		}
	}

public:
	void testEmptyAndNullBuffers() {
		const byte packed[] = { 17, 0, 0 };
		byte output = kGuard;
		size_t written = 123;
		TS_ASSERT(Trinket::decompressLZO1X(packed, sizeof(packed), nullptr, 0, written) == Trinket::LZO1XStatus::kSuccess);
		TS_ASSERT_EQUALS(written, 0u);
		TS_ASSERT(Trinket::decompressLZO1X(nullptr, 0, nullptr, 0, written) == Trinket::LZO1XStatus::kInputUnderrun);
		TS_ASSERT_EQUALS(written, 0u);
		TS_ASSERT(Trinket::decompressLZO1X(nullptr, 1, &output, 1, written) == Trinket::LZO1XStatus::kInvalidArgument);
		TS_ASSERT_EQUALS(written, 0u);
		TS_ASSERT(Trinket::decompressLZO1X(packed, sizeof(packed), nullptr, 1, written) == Trinket::LZO1XStatus::kInvalidArgument);
		TS_ASSERT_EQUALS(written, 0u);
		TS_ASSERT_EQUALS(output, kGuard);
	}

	void testInitialLiteralBoundary() {
		const byte expected[] = { 'A', 'B', 'C', 'D', 'E' };
		for (size_t count = 1; count <= sizeof(expected); count++) {
			byte packed[9] = {};
			packed[0] = static_cast<byte>(17 + count);
			memcpy(packed + 1, expected, count);
			packed[count + 1] = 17;
			checkSmallBlock(packed, count + 4, expected, count);
		}
	}

	void testOverlappingCompactMatches() {
		const byte expected[] = { 'A', 'B', 'A', 'B', 'A', 'B', 'A', 'B', 'A', 'B' };
		for (unsigned length = 3; length <= 8; length++) {
			const byte packed[] = { 19, 'A', 'B', static_cast<byte>(((length - 1) << 5) | 4), 0, 17, 0, 0 };
			checkSmallBlock(packed, sizeof(packed), expected, length + 2);
		}
	}

	void testShortMatchesAfterTrailingLiterals() {
		const byte packed[] = { 18, 'A', 1, 0, 'B', 2, 0, 'C', 'D', 3, 0, 'E', 'F', 'G', 17, 0, 0 };
		const byte expected[] = { 'A', 'A', 'A', 'B', 'B', 'B', 'C', 'D', 'D', 'D', 'E', 'F', 'G' };
		checkSmallBlock(packed, sizeof(packed), expected, sizeof(expected));
	}

	void testStandaloneLiteralAfterMatch() {
		const byte packed[] = { 18, 'A', 64, 0, 1, 'B', 'C', 'D', 'E', 17, 0, 0 };
		const byte expected[] = { 'A', 'A', 'A', 'A', 'B', 'C', 'D', 'E' };
		checkSmallBlock(packed, sizeof(packed), expected, sizeof(expected));
	}

	void testMediumMatchAndTrailingLiterals() {
		const byte packed[] = { 21, 'A', 'B', 'C', 'D', 38, 15, 0, 'X', 'Y', 'Z', 0, 0, 17, 0, 0 };
		const byte expected[] = { 'A', 'B', 'C', 'D', 'A', 'B', 'C', 'D', 'A', 'B', 'C', 'D', 'X', 'Y', 'Z', 'Z', 'Z' };
		checkSmallBlock(packed, sizeof(packed), expected, sizeof(expected));
	}

	void testExtendedMatchLength() {
		const byte packed[] = { 18, 'Q', 32, 1, 0, 0, 17, 0, 0 };
		byte expected[35];
		memset(expected, 'Q', sizeof(expected));
		checkSmallBlock(packed, sizeof(packed), expected, sizeof(expected));

		const byte longer[] = { 18, 'R', 32, 0, 1, 0, 0, 17, 0, 0 };
		byte output[292];
		memset(output, kGuard, sizeof(output));
		size_t written = 0;
		TS_ASSERT(Trinket::decompressLZO1X(longer, sizeof(longer), output + 1, 290, written) == Trinket::LZO1XStatus::kSuccess);
		TS_ASSERT_EQUALS(written, 290u);
		for (size_t i = 1; i <= written; i++)
			TS_ASSERT_EQUALS(output[i], 'R');
		TS_ASSERT_EQUALS(output[0], kGuard);
		TS_ASSERT_EQUALS(output[291], kGuard);
	}

	void testLongLiteralAndDistanceFamilies() {
		byte packed[50256] = {};
		byte output[50032];
		size_t pos = 1;
		size_t remaining = 50000 - 18;
		while (255 < remaining) {
			packed[pos] = 0;
			pos += 1;
			remaining -= 255;
		}
		packed[pos] = static_cast<byte>(remaining);
		pos += 1;
		const size_t literals = pos;
		for (size_t i = 0; i < 50000; i++)
			packed[pos + i] = static_cast<byte>((i * 13 + i / 255) % 256);
		pos += 50000;
		// A short match after a long literal, then both far-distance ranges.
		const byte matches[] = { 0, 0, 17, 4, 0, 25, 0, 0, 25, 252, 255, 17, 0, 0 };
		memcpy(packed + pos, matches, sizeof(matches));
		pos += sizeof(matches);
		memset(output, kGuard, sizeof(output));
		size_t written = 0;
		TS_ASSERT(Trinket::decompressLZO1X(packed, pos, output + 1, 50030, written) == Trinket::LZO1XStatus::kSuccess);
		TS_ASSERT_EQUALS(written, 50012u);
		TS_ASSERT_SAME_DATA(output + 1, packed + literals, 50000);
		const size_t distances[] = { 2049, 16385, 32768, 49151 };
		for (size_t match = 0; match < 4; match++) {
			const size_t start = 50000 + match * 3;
			TS_ASSERT_SAME_DATA(output + 1 + start, output + 1 + start - distances[match], 3);
		}
		TS_ASSERT_EQUALS(output[0], kGuard);
		for (size_t i = written + 1; i < sizeof(output); i++)
			TS_ASSERT_EQUALS(output[i], kGuard);
	}

	void testInvalidBackReferences() {
		const byte packed[][7] = {
			{ 18, 'A', 4, 0, 17, 0, 0 },
			{ 18, 'A', 68, 0, 17, 0, 0 },
			{ 18, 'A', 33, 4, 0, 17, 0 },
			{ 18, 'A', 17, 4, 0, 17, 0 }
		};
		for (size_t i = 0; i < 4; i++) {
			byte output[8];
			memset(output, kGuard, sizeof(output));
			size_t written = 0;
			TS_ASSERT(Trinket::decompressLZO1X(packed[i], sizeof(packed[i]), output, sizeof(output), written) ==
				Trinket::LZO1XStatus::kInvalidBackReference);
			TS_ASSERT_EQUALS(written, 1u);
			TS_ASSERT_EQUALS(output[0], 'A');
			for (size_t j = 1; j < sizeof(output); j++)
				TS_ASSERT_EQUALS(output[j], kGuard);
		}
	}

	void testMissingEndMarkerAndTrailingInput() {
		const byte packed[] = { 18, 'A', 17, 0, 0, 99 };
		byte output[8];
		memset(output, kGuard, sizeof(output));
		size_t written = 0;
		TS_ASSERT(Trinket::decompressLZO1X(packed, 2, output, sizeof(output), written) == Trinket::LZO1XStatus::kInputUnderrun);
		TS_ASSERT_EQUALS(written, 1u);
		TS_ASSERT(Trinket::decompressLZO1X(packed, sizeof(packed), output, sizeof(output), written) == Trinket::LZO1XStatus::kTrailingInput);
		TS_ASSERT_EQUALS(written, 1u);
		TS_ASSERT_EQUALS(output[0], 'A');
		for (size_t i = 1; i < sizeof(output); i++)
			TS_ASSERT_EQUALS(output[i], kGuard);
	}
};
