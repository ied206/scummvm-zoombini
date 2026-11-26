#include <cxxtest/TestSuite.h>

#include "common/endian.h"
#include "common/memstream.h"
#include "trinket/zoombini3_world.h"

#ifdef ENABLE_ZOOMBINI3

class TrinketPlatformSaveTestSuite : public CxxTest::TestSuite {
private:
	/** Build a Macintosh fixture from independently specified native file offsets, including nonzero padding and history bytes. */
	static Common::Array<byte> macFixture() {
		Common::Array<byte> data;
		data.resize(7630);
		memset(data.data(), 0, data.size());
		data[0] = 12;
		WRITE_BE_UINT32(data.data() + 1, 0x01020304);
		WRITE_BE_UINT32(data.data() + 9, 0x11223344);
		for (uint i = 0; i < 7; i++) {
			byte *activity = data.data() + 45 + 120 * i;
			activity[0] = i + 1;
			for (uint j = 1; j < 24; j++)
				activity[j] = i * 24 + j;
			for (uint j = 24; j < 96; j += 4)
				WRITE_BE_UINT32(activity + j, 0x10203040 + i * 256 + j);
			WRITE_BE_UINT32(activity + 44, 2);
			activity[96] = 1;
			activity[97] = 2;
			activity[98] = 3;
			activity[99] = 0xD0 + i;
			for (uint j = 100; j < 120; j += 4)
				WRITE_BE_UINT32(activity + j, 0x50607080 + i * 256 + j);
		}
		for (uint i = 0; i < 10; i++)
			data[885 + i] = 17 + i;
		for (uint i = 0; i < 240; i++) {
			byte *token = data.data() + 895 + 28 * i;
			WRITE_BE_UINT32(token, 11 + i % 3);
			for (uint j = 0; j < 6; j++)
				WRITE_BE_UINT32(token + 4 + j * 4, 0x12345600 + i + j);
		}
		WRITE_BE_UINT32(data.data() + 7615, 0x76543210);
		memcpy(data.data() + 7619, "PLAYER_END\n", 11);
		return data;
	}

public:
	/** Macintosh round trips preserve mixed-width fields, byte history, BE32 tokens and every activity padding byte. */
	void testMacNativeLayout() {
		const Common::Array<byte> data = macFixture();
		Common::MemoryReadStream input(data.data(), data.size());
		TS_ASSERT_EQUALS(Trinket::Zoombini3::World::detectSavePlatform(input), Common::kPlatformMacintosh);
		TS_ASSERT_EQUALS(input.pos(), 0);
		Trinket::Zoombini3::World world;
		TS_ASSERT(world.read(input, Common::kPlatformMacintosh));
		TS_ASSERT_EQUALS(world.currentLocation(), 12);
		TS_ASSERT_EQUALS(world.visitCount(9), 0x01020304);
		TS_ASSERT_EQUALS(world.seenMovie(9), 26);
		TS_ASSERT_EQUALS(world.token(239).location, 13);
		TS_ASSERT_EQUALS(world.token(239).attributes[5], 0x123456F4);
		Common::MemoryWriteStreamDynamic output(DisposeAfterUse::YES);
		TS_ASSERT(world.write(output, Common::kPlatformMacintosh));
		TS_ASSERT_EQUALS(output.size(), data.size());
		TS_ASSERT_SAME_DATA(output.getData(), data.data(), data.size());
	}

	/** Platform conversion preserves full-width values and byte-array order while narrowing Windows byte fields for Macintosh. */
	void testCrossPlatformConversion() {
		Trinket::Zoombini3::World world;
		world.setAutoEnabled(0, 0x1234);
		world.setSeenMovie(3, 0x5678);
		world.setVisitedCount(9, 0x12345678);
		world.setTokenAttribute(17, 3, -123456789);
		Common::MemoryWriteStreamDynamic windows(DisposeAfterUse::YES);
		TS_ASSERT(world.write(windows));
		Common::MemoryReadStream winInput(windows.getData(), windows.size());
		TS_ASSERT_EQUALS(Trinket::Zoombini3::World::detectSavePlatform(winInput), Common::kPlatformWindows);
		Common::MemoryWriteStreamDynamic mac(DisposeAfterUse::YES);
		TS_ASSERT(world.write(mac, Common::kPlatformMacintosh));
		TS_ASSERT_EQUALS(windows.size() - mac.size(), 89);
		Common::MemoryReadStream macInput(mac.getData(), mac.size());
		TS_ASSERT_EQUALS(Trinket::Zoombini3::World::detectSavePlatform(macInput), Common::kPlatformMacintosh);
		Trinket::Zoombini3::World converted;
		TS_ASSERT(converted.read(macInput, Common::kPlatformMacintosh));
		TS_ASSERT_EQUALS(converted.autoEnabled(0), 0x34);
		TS_ASSERT_EQUALS(converted.seenMovie(3), 0x78);
		TS_ASSERT_EQUALS(converted.visitCount(9), 0x12345678);
		TS_ASSERT_EQUALS(converted.token(17).attributes[3], -123456789);
		Common::MemoryWriteStreamDynamic back(DisposeAfterUse::YES);
		TS_ASSERT(converted.write(back));
		TS_ASSERT_EQUALS(READ_LE_UINT32(back.getData() + 48), 0x34);
		TS_ASSERT_EQUALS(READ_LE_UINT32(back.getData() + 956), 0x78);
		TS_ASSERT_EQUALS(READ_LE_UINT32(back.getData() + 4), 0x12345678);
	}

	/** Detection rejects truncation and trailing data, and failed decoding leaves the active world unchanged. */
	void testMalformedSaveAndRollback() {
		Common::Array<byte> data = macFixture();
		Trinket::Zoombini3::World world;
		world.setLocation(Trinket::Zoombini3::kBarn07);
		const uint sizes[] = {0, 7618, 7619, 7629};
		for (uint size : sizes) {
			Common::MemoryReadStream input(data.data(), size);
			TS_ASSERT_EQUALS(Trinket::Zoombini3::World::detectSavePlatform(input), Common::kPlatformUnknown);
			TS_ASSERT_EQUALS(input.pos(), 0);
			TS_ASSERT(!world.read(input, Common::kPlatformMacintosh));
			TS_ASSERT_EQUALS(world.currentLocation(), Trinket::Zoombini3::kBarn07);
		}
		data.push_back(1);
		Common::MemoryReadStream trailing(data.data(), data.size());
		TS_ASSERT_EQUALS(Trinket::Zoombini3::World::detectSavePlatform(trailing), Common::kPlatformUnknown);
	}
};

#endif
