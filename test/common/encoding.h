#include <cxxtest/TestSuite.h>

#include "common/str.h"
#include "common/ustr.h"
#include "../system/null_osystem.h"

// We support CJK on all the platforms but it relies on OSystem to read
// file which *in test environments* is available only on some platforms
#if NULL_OSYSTEM_IS_AVAILABLE
#define TEST_CJK 1
#else
#define TEST_CJK 0
#endif

class EncodingTestSuite : public CxxTest::TestSuite
{
public:
	void setUp() {
#if TEST_CJK
		Common::install_null_g_system();
#endif
	}

	void tearDown() {
#if TEST_CJK
		Common::uninstall_null_g_system();
#endif
	}

	void test_korean() {
#if TEST_CJK
		// Text: "게임중지 계속하려면 스페이스키를 치시오.".
		const byte utf8[] = {
			0xea, 0xb2, 0x8c, 0xec, 0x9e, 0x84, 0xec, 0xa4,
			0x91, 0xec, 0xa7, 0x80, 0x20, 0xea, 0xb3, 0x84,
			0xec, 0x86, 0x8d, 0xed, 0x95, 0x98, 0xeb, 0xa0,
			0xa4, 0xeb, 0xa9, 0xb4, 0x20, 0xec, 0x8a, 0xa4,
			0xed, 0x8e, 0x98, 0xec, 0x9d, 0xb4, 0xec, 0x8a,
			0xa4, 0xed, 0x82, 0xa4, 0xeb, 0xa5, 0xbc, 0x20,
			0xec, 0xb9, 0x98, 0xec, 0x8b, 0x9c, 0xec, 0x98,
			0xa4, 0x2e, 0x00,
		};

		const Common::u32char_type_t utf32[] = {
			0xac8c, 0xc784, 0xc911, 0xc9c0, 0x0020, 0xacc4,
			0xc18d, 0xd558, 0xb824, 0xba74, 0x0020, 0xc2a4,
			0xd398, 0xc774, 0xc2a4, 0xd0a4, 0xb97c, 0x0020,
			0xce58, 0xc2dc, 0xc624, 0x002e, 0
		};

		const byte uhc[] = {
			0xb0, 0xd4, 0xc0, 0xd3, 0xc1, 0xdf, 0xc1, 0xf6,
			0x20, 0xb0, 0xe8, 0xbc, 0xd3, 0xc7, 0xcf, 0xb7,
			0xc1, 0xb8, 0xe9, 0x20, 0xbd, 0xba, 0xc6, 0xe4,
			0xc0, 0xcc, 0xbd, 0xba, 0xc5, 0xb0, 0xb8, 0xa6,
			0x20, 0xc4, 0xa1, 0xbd, 0xc3, 0xbf, 0xc0, 0x2e,
			0x00
		};

		const byte macKorean[] = {
			0xb0, 0xd4, 0xc0, 0xd3, 0xc1, 0xdf, 0xc1, 0xf6,
			0x20, 0xb0, 0xe8, 0xbc, 0xd3, 0xc7, 0xcf, 0xb7,
			0xc1, 0xb8, 0xe9, 0x20, 0xbd, 0xba, 0xc6, 0xe4,
			0xc0, 0xcc, 0xbd, 0xba, 0xc5, 0xb0, 0xb8, 0xa6,
			0x20, 0xc4, 0xa1, 0xbd, 0xc3, 0xbf, 0xc0, 0x2e,
			0x00
		};

		Common::U32String ustr_from_utf8((const char *) utf8, Common::kUtf8);
		Common::U32String ustr_from_uhc((const char *) uhc, Common::kWindows949);
		Common::U32String ustr_from_mac((const char *) macKorean, Common::kMacKorean);
		Common::U32String ustr(utf32);
		Common::String utf8_to_uhc = ustr_from_utf8.encode(Common::kWindows949);
		Common::String uhc_to_utf8 = ustr_from_uhc.encode(Common::kUtf8);
		Common::String utf8_to_mac = ustr_from_utf8.encode(Common::kMacKorean);
		Common::String mac_to_utf8 = ustr_from_mac.encode(Common::kUtf8);
		TS_ASSERT_EQUALS(ustr_from_utf8, ustr);
		TS_ASSERT_EQUALS(ustr_from_uhc, ustr);
		TS_ASSERT_EQUALS(ustr_from_mac, ustr);
		TS_ASSERT(strcmp((const char *) utf8, uhc_to_utf8.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) uhc, utf8_to_uhc.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) utf8, mac_to_utf8.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) macKorean, utf8_to_mac.c_str()) == 0);
#endif
	}

	void test_chinese_traditional() {
#if TEST_CJK
		// Text: "遊戲暫停。按下空!白鍵繼續遊戲。".
		const byte utf8[] = {
			0xe9, 0x81, 0x8a, 0xe6, 0x88, 0xb2, 0xe6, 0x9a,
			0xab, 0xe5, 0x81, 0x9c, 0xe3, 0x80, 0x82, 0xe6,
			0x8c, 0x89, 0xe4, 0xb8, 0x8b, 0xe7, 0xa9, 0xba,
			0x21, 0xe7, 0x99, 0xbd, 0xe9, 0x8d, 0xb5, 0xe7,
			0xb9, 0xbc, 0xe7, 0xba, 0x8c, 0xe9, 0x81, 0x8a,
			0xe6, 0x88, 0xb2, 0xe3, 0x80, 0x82, 0x00,
		};

		const Common::u32char_type_t utf32[] = {
			0x904a, 0x6232, 0x66ab, 0x505c, 0x3002, 0x6309,
			0x4e0b, 0x7a7a, 0x0021, 0x767d, 0x9375, 0x7e7c,
			0x7e8c, 0x904a, 0x6232, 0x3002, 0
		};

		const byte big5[] = {
			0xb9, 0x43, 0xc0, 0xb8, 0xbc, 0xc8, 0xb0, 0xb1,
			0xa1, 0x43, 0xab, 0xf6, 0xa4, 0x55, 0xaa, 0xc5,
			0x21, 0xa5, 0xd5, 0xc1, 0xe4, 0xc4, 0x7e, 0xc4,
			0xf2, 0xb9, 0x43, 0xc0, 0xb8, 0xa1, 0x43, 0x00
		};

		const byte macChineseTrad[] = {
			0xb9, 0x43, 0xc0, 0xb8, 0xbc, 0xc8, 0xb0, 0xb1,
			0xa1, 0x43, 0xab, 0xf6, 0xa4, 0x55, 0xaa, 0xc5,
			0x21, 0xa5, 0xd5, 0xc1, 0xe4, 0xc4, 0x7e, 0xc4,
			0xf2, 0xb9, 0x43, 0xc0, 0xb8, 0xa1, 0x43, 0x00
		};
		Common::U32String ustr_from_utf8((const char *) utf8, Common::kUtf8);
		Common::U32String ustr_from_big5((const char *) big5, Common::kWindows950);
		Common::U32String ustr_from_mac((const char *) macChineseTrad, Common::kMacChineseTrad);
		Common::U32String ustr(utf32);
		Common::String utf8_to_big5 = ustr_from_utf8.encode(Common::kWindows950);
		Common::String big5_to_utf8 = ustr_from_big5.encode(Common::kUtf8);
		Common::String utf8_to_mac = ustr_from_utf8.encode(Common::kMacChineseTrad);
		Common::String mac_to_utf8 = ustr_from_mac.encode(Common::kUtf8);
		TS_ASSERT_EQUALS(ustr_from_utf8, ustr);
		TS_ASSERT_EQUALS(ustr_from_big5, ustr);
		TS_ASSERT_EQUALS(ustr_from_mac, ustr);
		TS_ASSERT(strcmp((const char *) utf8, big5_to_utf8.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) big5, utf8_to_big5.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) utf8, mac_to_utf8.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) macChineseTrad, utf8_to_mac.c_str()) == 0);
#endif
	}

	void test_chinese_simplified() {
#if TEST_CJK
		// Text: "游戏暂停。按下空!白键继续游戏。".
		const byte utf8[] = {
			0xe6, 0xb8, 0xb8, 0xe6, 0x88, 0x8f, 0xe6, 0x9a,
			0x82, 0xe5, 0x81, 0x9c, 0xe3, 0x80, 0x82, 0xe6,
			0x8c, 0x89, 0xe4, 0xb8, 0x8b, 0xe7, 0xa9, 0xba,
			0x21, 0xe7, 0x99, 0xbd, 0xe9, 0x94, 0xae, 0xe7,
			0xbb, 0xa7, 0xe7, 0xbb, 0xad, 0xe6, 0xb8, 0xb8,
			0xe6, 0x88, 0x8f, 0xe3, 0x80, 0x82, 0x00
		};

		const Common::u32char_type_t utf32[] = {
			0x6e38, 0x620f, 0x6682, 0x505c, 0x3002, 0x6309, 0x4e0b, 0x7a7a,
			0x0021, 0x767d, 0x952e, 0x7ee7, 0x7eed, 0x6e38, 0x620f, 0x3002,
			0
		};

		const byte gbk[] = {
			0xd3, 0xce, 0xcf, 0xb7, 0xd4, 0xdd, 0xcd, 0xa3,
			0xa1, 0xa3, 0xb0, 0xb4, 0xcf, 0xc2, 0xbf, 0xd5,
			0x21, 0xb0, 0xd7, 0xbc, 0xfc, 0xbc, 0xcc, 0xd0,
			0xf8, 0xd3, 0xce, 0xcf, 0xb7, 0xa1, 0xa3, 0x00
		};

		const byte macChineseSimp[] = {
			0xd3, 0xce, 0xcf, 0xb7, 0xd4, 0xdd, 0xcd, 0xa3,
			0xa1, 0xa3, 0xb0, 0xb4, 0xcf, 0xc2, 0xbf, 0xd5,
			0x21, 0xb0, 0xd7, 0xbc, 0xfc, 0xbc, 0xcc, 0xd0,
			0xf8, 0xd3, 0xce, 0xcf, 0xb7, 0xa1, 0xa3, 0x00
		};

		Common::U32String ustr_from_utf8((const char *) utf8, Common::kUtf8);
		Common::U32String ustr_from_gbk((const char *) gbk, Common::kWindows936);
		Common::U32String ustr_from_mac((const char *) macChineseSimp, Common::kMacChineseSimp);
		Common::U32String ustr(utf32);
		Common::String utf8_to_gbk = ustr_from_utf8.encode(Common::kWindows936);
		Common::String gbk_to_utf8 = ustr_from_gbk.encode(Common::kUtf8);
		Common::String utf8_to_mac = ustr_from_utf8.encode(Common::kMacChineseSimp);
		Common::String mac_to_utf8 = ustr_from_mac.encode(Common::kUtf8);
		TS_ASSERT_EQUALS(ustr_from_utf8, ustr);
		TS_ASSERT_EQUALS(ustr_from_gbk, ustr);
		TS_ASSERT_EQUALS(ustr_from_mac, ustr);
		TS_ASSERT(strcmp((const char *) utf8, gbk_to_utf8.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) gbk, utf8_to_gbk.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) utf8, mac_to_utf8.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) macChineseSimp, utf8_to_mac.c_str()) == 0);
#endif
	}

	void test_japanese() {
#if TEST_CJK
		// Text: "一時停止。続ける場合はスペースバーを押してください。".
		const byte utf8[] = {
			0xe4, 0xb8, 0x80, 0xe6, 0x99, 0x82, 0xe5, 0x81,
			0x9c, 0xe6, 0xad, 0xa2, 0xe3, 0x80, 0x82, 0xe7,
			0xb6, 0x9a, 0xe3, 0x81, 0x91, 0xe3, 0x82, 0x8b,
			0xe5, 0xa0, 0xb4, 0xe5, 0x90, 0x88, 0xe3, 0x81,
			0xaf, 0xe3, 0x82, 0xb9, 0xe3, 0x83, 0x9a, 0xe3,
			0x83, 0xbc, 0xe3, 0x82, 0xb9, 0xe3, 0x83, 0x90,
			0xe3, 0x83, 0xbc, 0xe3, 0x82, 0x92, 0xe6, 0x8a,
			0xbc, 0xe3, 0x81, 0x97, 0xe3, 0x81, 0xa6, 0xe3,
			0x81, 0x8f, 0xe3, 0x81, 0xa0, 0xe3, 0x81, 0x95,
			0xe3, 0x81, 0x84, 0xe3, 0x80, 0x82, 0
		};

		const Common::u32char_type_t utf32[] = {
			0x4e00, 0x6642, 0x505c, 0x6b62, 0x3002, 0x7d9a,
			0x3051, 0x308b, 0x5834, 0x5408, 0x306f, 0x30b9,
			0x30da, 0x30fc, 0x30b9, 0x30d0, 0x30fc, 0x3092,
			0x62bc, 0x3057, 0x3066, 0x304f, 0x3060, 0x3055,
			0x3044, 0x3002, 0
		};

		const byte cp932[] = {
			0x88, 0xea, 0x8e, 0x9e, 0x92, 0xe2, 0x8e, 0x7e,
			0x81, 0x42, 0x91, 0xb1, 0x82, 0xaf, 0x82, 0xe9,
			0x8f, 0xea, 0x8d, 0x87, 0x82, 0xcd, 0x83, 0x58,
			0x83, 0x79, 0x81, 0x5b, 0x83, 0x58, 0x83, 0x6f,
			0x81, 0x5b, 0x82, 0xf0, 0x89, 0x9f, 0x82, 0xb5,
			0x82, 0xc4, 0x82, 0xad, 0x82, 0xbe, 0x82, 0xb3,
			0x82, 0xa2, 0x81, 0x42, 0
		};

		const byte macJapanese[] = {
			0x88, 0xea, 0x8e, 0x9e, 0x92, 0xe2, 0x8e, 0x7e,
			0x81, 0x42, 0x91, 0xb1, 0x82, 0xaf, 0x82, 0xe9,
			0x8f, 0xea, 0x8d, 0x87, 0x82, 0xcd, 0x83, 0x58,
			0x83, 0x79, 0x81, 0x5b, 0x83, 0x58, 0x83, 0x6f,
			0x81, 0x5b, 0x82, 0xf0, 0x89, 0x9f, 0x82, 0xb5,
			0x82, 0xc4, 0x82, 0xad, 0x82, 0xbe, 0x82, 0xb3,
			0x82, 0xa2, 0x81, 0x42, 0x00
		};

		Common::U32String ustr_from_utf8((const char *) utf8, Common::kUtf8);
		Common::U32String ustr_from_cp932((const char *) cp932, Common::kWindows932);
		Common::U32String ustr_from_mac((const char *) macJapanese, Common::kMacJapanese);
		Common::U32String ustr(utf32);
		Common::String utf8_to_cp932 = ustr_from_utf8.encode(Common::kWindows932);
		Common::String cp932_to_utf8 = ustr_from_cp932.encode(Common::kUtf8);
		Common::String utf8_to_mac = ustr_from_utf8.encode(Common::kMacJapanese);
		Common::String mac_to_utf8 = ustr_from_mac.encode(Common::kUtf8);
		TS_ASSERT_EQUALS(ustr_from_utf8, ustr);
		TS_ASSERT_EQUALS(ustr_from_cp932, ustr);
		TS_ASSERT_EQUALS(ustr_from_mac, ustr);
		TS_ASSERT(strcmp((const char *) utf8, cp932_to_utf8.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) cp932, utf8_to_cp932.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) utf8, mac_to_utf8.c_str()) == 0);
		TS_ASSERT(strcmp((const char *) macJapanese, utf8_to_mac.c_str()) == 0);
#endif
	}

	void test_conversion_unicode_machine_endian() {
		//  |dolar|   cent    |     euro       |
		unsigned char utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0};
#ifdef SCUMM_BIG_ENDIAN
		//| dolar |  cent  |    euro   |
		unsigned char utf16be[] = {0, 0x24, 0, 0xA2, 0x20, 0xAC, 0, 0};
		//| dolar       |  cent        |    euro
		unsigned char utf32be[] = {0, 0, 0, 0x24, 0, 0, 0, 0xA2, 0, 0, 0x20, 0xAC, 0, 0, 0, 0};

		unsigned char *utf16 = utf16be;
		unsigned char *utf32 = utf32be;
#else
		//| dolar |  cent  |    euro   |
		unsigned char utf16le[] = {0x24, 0, 0xA2, 0, 0xAC, 0x20, 0, 0};
		//| dolar       |  cent        |    euro
		unsigned char utf32le[] = {0x24, 0, 0, 0, 0xA2, 0, 0, 0, 0xAC, 0x20, 0, 0, 0, 0, 0, 0};

		unsigned char *utf16 = utf16le;
		unsigned char *utf32 = utf32le;
#endif

		// UTF16 to UTF8
		Common::String resultstr8 = Common::U32String::decodeUTF16Native((uint16 *) utf16, 3).encode(Common::kUtf8);
		TS_ASSERT(resultstr8.c_str() != NULL);
		TS_ASSERT_EQUALS(memcmp(resultstr8.c_str(), utf8, 7), 0)

		// UTF32 to UTF8

		resultstr8 = Common::U32String((Common::u32char_type_t *) utf32, 3).encode(Common::kUtf8);
		TS_ASSERT(resultstr8.c_str() != NULL);
		TS_ASSERT_EQUALS(memcmp(resultstr8.c_str(), utf8, 7), 0);

		// UTF32 to UTF16
		uint16 *result16 = Common::U32String((Common::u32char_type_t *) utf32, 3).encodeUTF16Native(NULL);
		TS_ASSERT(result16 != NULL);
		TS_ASSERT_EQUALS(memcmp(result16, utf16, 8), 0);
		delete[] result16;

		// UTF8 to UTF16

		result16 = Common::U32String((char *) utf8, 6, Common::kUtf8).encodeUTF16Native(NULL);
		TS_ASSERT(result16 != NULL);
		TS_ASSERT_EQUALS(memcmp(result16, utf16, 8), 0);
		delete[] result16;

		// UTF8 to UTF32
		Common::U32String resultustr = Common::String((const char *) utf8, 6).decode(Common::kUtf8);
		TS_ASSERT_EQUALS(memcmp(resultustr.c_str(), utf32, 16), 0);

		// UTF16 to UTF32
		resultustr = Common::U32String::decodeUTF16Native((uint16 *) utf16, 3);
		TS_ASSERT_EQUALS(memcmp(resultustr.c_str(), utf32, 16), 0);
	}

	void test_conversion_unicode_big_endian() {
		//  |dolar|   cent    |     euro       |
		unsigned char utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0};
		//| dolar |  cent  |    euro   |
		unsigned char utf16be[] = {0, 0x24, 0, 0xA2, 0x20, 0xAC, 0, 0};

		// UTF16 to UTF8
		Common::String resultstr8 = Common::U32String::decodeUTF16BE((uint16 *) utf16be, 3).encode(Common::kUtf8);
		TS_ASSERT(resultstr8.c_str() != NULL);
		TS_ASSERT_EQUALS(memcmp(resultstr8.c_str(), utf8, 7), 0);


		// UTF8 to UTF16
		uint16 *result16 = Common::U32String((char *) utf8, 6, Common::kUtf8).encodeUTF16BE(NULL);
		TS_ASSERT(result16 != NULL);
		TS_ASSERT_EQUALS(memcmp(result16, utf16be, 8), 0);
		delete[] result16;

	}

	void test_conversion_unicode_little_endian() {
		//  |dolar|   cent    |     euro       |
		unsigned char utf8[] = {0x24, 0xC2, 0xA2, 0xE2, 0x82, 0xAC, 0};
		//| dolar |  cent  |    euro   |
		unsigned char utf16le[] = {0x24, 0, 0xA2, 0, 0xAC, 0x20, 0, 0};

		// UTF16 to UTF8
		Common::String resultstr8 = Common::U32String::decodeUTF16LE((uint16 *) utf16le, 3).encode(Common::kUtf8);
		TS_ASSERT(resultstr8.c_str() != NULL);
		TS_ASSERT_EQUALS(memcmp(resultstr8.c_str(), utf8, 7), 0);

		// UTF8 to UTF16
		uint16 *result16 = Common::U32String((char *) utf8, 6, Common::kUtf8).encodeUTF16LE(NULL);
		TS_ASSERT(result16 != NULL);
		TS_ASSERT_EQUALS(memcmp(result16, utf16le, 8), 0);
		delete[] result16;

	}

	void test_cyrillic_transliteration() {
		unsigned char utf8[] = {/* Z */0xD0, 0x97, /* d */ 0xD0, 0xB4, /* r */ 0xD1, 0x80, /* a */ 0xD0, 0xB0, /* v */ 0xD0, 0xB2, /* s */ 0xD1, 0x81, /* t */ 0xD1, 0x82, /* v */ 0xD0, 0xB2, /* u */ 0xD1, 0x83, /* j */ 0xD0, 0xB9, /* t */ 0xD1, 0x82, /* e */ 0xD0, 0xB5, 0};
		unsigned char iso_8859_5[] = {0xB7, 0xD4, 0xE0, 0xD0, 0xD2, 0xE1, 0xE2, 0xD2, 0xE3, 0xD9, 0xE2, 0xD5, 0};
		unsigned char ascii[] = "Zdravstvujte";

		Common::String result = Common::U32String((const char *) utf8, 24, Common::kUtf8).encode(Common::kASCII);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), ascii, 13), 0);

		result = Common::U32String((const char *) iso_8859_5, 12, Common::kISO8859_5).encode(Common::kASCII);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), ascii, 13), 0);

		result = Common::U32String((const char *) iso_8859_5, 12, Common::kISO8859_5).encode(Common::kUtf8);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), utf8, 25), 0);

		result = Common::U32String((const char *) utf8, 24, Common::kUtf8).encode(Common::kISO8859_5);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), iso_8859_5, 13), 0);

		// this should stay the same
		result = Common::U32String((const char *) ascii, 12, Common::kASCII).encode(Common::kISO8859_5);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), ascii, 13), 0);
	}

	void test_other_conversions() {
		unsigned char cp850[] = {0x99, 0xE0, 0xEA, 0x41, 0x64, 0};
		unsigned char utf8_1[] = {0xC3, 0x96, 0xC3, 0x93, 0xC3, 0x9B, 0x41, 0x64, 0};

		unsigned char iso_8859_2[] = {0xA9, 0xE1, 0x6C, 0x65, 0xE8, 0x65, 0x6B, 0};
		unsigned char utf8_2[] = {0xC5, 0xA0, 0xC3, 0xA1, 0x6C, 0x65, 0xC4, 0x8D, 0x65, 0x6B, 0};

		Common::String result = Common::U32String((const char *) cp850, sizeof(cp850)-1, Common::kDos850).encode(Common::kUtf8);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), utf8_1, sizeof(utf8_1)), 0);

		result = Common::U32String((const char *) utf8_1, sizeof(utf8_1)-1, Common::kUtf8).encode(Common::kDos850);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), cp850, sizeof(cp850)), 0);

		result = Common::U32String((const char *) iso_8859_2, sizeof(iso_8859_2)-1, Common::kISO8859_2).encode(Common::kUtf8);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), utf8_2, sizeof(utf8_2)), 0);

		result = Common::U32String((const char *) utf8_2, sizeof(utf8_2)-1, Common::kUtf8).encode(Common::kISO8859_2);
		TS_ASSERT_EQUALS(memcmp(result.c_str(), iso_8859_2, sizeof(iso_8859_2)), 0);
	}

	/** Check Korean single-byte additions, base Hangul, and extensions conflicting with CP949. */
	void test_mac_korean() {
#if TEST_CJK
		// Text: <NUL>\<NBSP>₩–<U+F87F>©＿<U+F87F><U+0085><U+009F>…<U+F87F>
		//       〜가힝「<U+F87F>1⃞<U+F87C>2⃞<U+F87C><U+F861>(A)운⃞
		const byte macKorean[] = {
			0x00, 0x5c, 0x80, 0x81, 0x82, 0x83, 0x84, 0x85,
			0x9f, 0xff, 0xa1, 0xad, 0xb0, 0xa1, 0xc8, 0xfe,
			0xa1, 0x41, 0xa2, 0xe6, 0xa2, 0xe7, 0xa3, 0x69,
			0xaa, 0x41
		};
		const Common::u32char_type_t unicode[] = {
			0x0000, 0x005c, 0x00a0, 0x20a9, 0x2013, 0xf87f, 0x00a9, 0xff3f,
			0xf87f, 0x0085, 0x009f, 0x2026, 0xf87f, 0x301c, 0xac00, 0xd79d,
			0x300c, 0xf87f, 0x0031, 0x20de, 0xf87c, 0x0032, 0x20de, 0xf87c,
			0xf861, 0x0028, 0x0041, 0x0029, 0xc6b4, 0x20de
		};
		const Common::String bytes((const char*) macKorean, sizeof(macKorean));
		const Common::U32String expected(unicode, ARRAYSIZE(unicode));
		TS_ASSERT_EQUALS(bytes.decode(Common::kMacKorean), expected);
		Common::String result;
		TS_ASSERT_EQUALS(expected.encode(result, Common::kMacKorean, '?'), Common::kStringEncodingResultSucceeded);
		TS_ASSERT_EQUALS(result, bytes);
		// Text: ₩A
		TS_ASSERT_EQUALS(Common::String("\x81" "A").decode(Common::kMacKorean), Common::U32String("₩A"));
#endif
	}

	/** Check Japanese symbols and Apple sequences conflicting with CP932. */
	void test_mac_japanese() {
#if TEST_CJK
		// Text: <NUL>¥\<NBSP>©™…<U+F87F>｡ﾟ
		//       —〜①㈰大⃝<U+F862>XIIIァ<U+F87E>あ<U+E000><U+E758><U+E98B>
		const byte macJapanese[] = {
			0x00, 0x5c, 0x80, 0xa0, 0xfd, 0xfe, 0xff, 0xa1,
			0xdf, 0x81, 0x5c, 0x81, 0x60, 0x85, 0x40, 0x87,
			0x40, 0x87, 0x91, 0x85, 0xab, 0xed, 0x40, 0x82,
			0xa0, 0xf0, 0x40, 0xfa, 0x40, 0xfc, 0xfc
		};
		const Common::u32char_type_t unicode[] = {
			0x0000, 0x00a5, 0x005c, 0x00a0, 0x00a9, 0x2122, 0x2026, 0xf87f,
			0xff61, 0xff9f, 0x2014, 0x301c, 0x2460, 0x3230, 0x5927, 0x20dd,
			0xf862, 0x0058, 0x0049, 0x0049, 0x0049, 0x30a1, 0xf87e, 0x3042,
			0xe000, 0xe758, 0xe98b
		};
		const Common::String bytes((const char*) macJapanese, sizeof(macJapanese));
		const Common::U32String expected(unicode, ARRAYSIZE(unicode));
		TS_ASSERT_EQUALS(bytes.decode(Common::kMacJapanese), expected);
		Common::String result;
		TS_ASSERT_EQUALS(expected.encode(result, Common::kMacJapanese, '?'), Common::kStringEncodingResultSucceeded);
		TS_ASSERT_EQUALS(result, bytes);
#endif
	}

	/** Check MacChineseTrad single-byte additions, Big5 characters, and variant sequences conflicting with CP950. */
	void test_mac_chinese_trad() {
#if TEST_CJK
		// Text: <NUL>\\<U+F87F><U+F880><U+F881><U+0083><U+009F><NBSP>©™…
		//       <IDEOGRAPHIC SPACE>，，<U+F87D>＿<U+F87F>一
		const byte macChineseTrad[] = {
			0x00, 0x5c, 0x80, 0x81, 0x82, 0x83, 0x9f, 0xa0,
			0xfd, 0xfe, 0xff, 0xa1, 0x40, 0xa1, 0x41, 0xa1,
			0x4d, 0xa1, 0x5a, 0xa4, 0x40
		};
		const Common::u32char_type_t unicode[] = {
			0x0000, 0x005c, 0x005c, 0xf87f, 0xf880, 0xf881, 0x0083, 0x009f,
			0x00a0, 0x00a9, 0x2122, 0x2026, 0x3000, 0xff0c, 0xff0c, 0xf87d,
			0xff3f, 0xf87f, 0x4e00
		};
		const Common::String bytes((const char*) macChineseTrad, sizeof(macChineseTrad));
		const Common::U32String expected(unicode, ARRAYSIZE(unicode));
		TS_ASSERT_EQUALS(bytes.decode(Common::kMacChineseTrad), expected);
		Common::String result;
		TS_ASSERT_EQUALS(expected.encode(result, Common::kMacChineseTrad, '?'), Common::kStringEncodingResultSucceeded);
		TS_ASSERT_EQUALS(result, bytes);
		// Unmapped extension decodes to �.
		TS_ASSERT_EQUALS(Common::String("\xc6\xa1").decode(Common::kMacChineseTrad), Common::U32String("�"));
#endif
	}

	/** Check MacChineseSimp additions and variant sequences conflicting with CP936, and unsupported GBK extensions. */
	void test_mac_chinese_simp() {
#if TEST_CJK
		// Text: <NUL>ü<U+F87F><U+F880><U+F881><U+0083><U+009F><NBSP>©™…
		//       <IDEOGRAPHIC SPACE>啊，<U+F87E>ǹ〜⋯
		const byte macChineseSimp[] = {
			0x00, 0x80, 0x81, 0x82, 0x83, 0x9f, 0xa0, 0xfd,
			0xfe, 0xff, 0xa1, 0xa1, 0xb0, 0xa1, 0xa6, 0xd9,
			0xa8, 0xbf, 0xa1, 0xab, 0xa1, 0xad
		};
		const Common::u32char_type_t unicode[] = {
			0x0000, 0x00fc, 0xf87f, 0xf880, 0xf881, 0x0083, 0x009f, 0x00a0,
			0x00a9, 0x2122, 0x2026, 0x3000, 0x554a, 0xff0c, 0xf87e, 0x01f9,
			0x301c, 0x22ef
		};
		const Common::String bytes((const char*) macChineseSimp, sizeof(macChineseSimp));
		const Common::U32String expected(unicode, ARRAYSIZE(unicode));
		TS_ASSERT_EQUALS(bytes.decode(Common::kMacChineseSimp), expected);
		Common::String result;
		TS_ASSERT_EQUALS(expected.encode(result, Common::kMacChineseSimp, '?'), Common::kStringEncodingResultSucceeded);
		TS_ASSERT_EQUALS(result, bytes);
		// Text: 丂 (GBK-only), replaced with "!" in MacChineseSimp.
		const Common::U32String gbkOnly = Common::String("\x81\x40").decode(Common::kWindows936);
		result.clear();
		TS_ASSERT_EQUALS(gbkOnly.encode(result, Common::kMacChineseSimp, '!'), Common::kStringEncodingResultHasErrors);
		TS_ASSERT_EQUALS(result, "!");
		// Text: <U+F880>@ (two single-byte characters).
		TS_ASSERT_EQUALS(Common::String("\x81\x40").decode(Common::kMacChineseSimp), Common::U32String(U"\uf880@"));
#endif
	}

	/** Check malformed bytes, truncated sequences, and characters outside either Mac repertoire. */
	void test_mac_cjk_errors() {
#if TEST_CJK
		// Decoded text: "�".
		TS_ASSERT_EQUALS(Common::String("\xa1").decode(Common::kMacKorean), Common::U32String("�"));
		// Decoded text: "�@A".
		TS_ASSERT_EQUALS(Common::String("\xa1\x40" "A").decode(Common::kMacKorean), Common::U32String("�@A"));
		// Decoded text: "�A".
		TS_ASSERT_EQUALS(Common::String("\xfe\xfe" "A").decode(Common::kMacKorean), Common::U32String("�A"));
		// Decoded text: "�".
		TS_ASSERT_EQUALS(Common::String("\x81").decode(Common::kMacJapanese), Common::U32String("�"));
		// Decoded text: "�<DEL>A".
		TS_ASSERT_EQUALS(Common::String("\x81\x7f" "A").decode(Common::kMacJapanese), Common::U32String("�\x7f" "A"));
		// Decoded text: "�A".
		TS_ASSERT_EQUALS(Common::String("\xef\x40" "A").decode(Common::kMacJapanese), Common::U32String("�A"));
		// Decoded text: "�".
		TS_ASSERT_EQUALS(Common::String("\xa1").decode(Common::kMacChineseTrad), Common::U32String("�"));
		// Decoded text: "�<DEL>A".
		TS_ASSERT_EQUALS(Common::String("\xa1\x7f" "A").decode(Common::kMacChineseTrad), Common::U32String("�\x7f" "A"));
		// Decoded text: "�A".
		TS_ASSERT_EQUALS(Common::String("\xfc\xfe" "A").decode(Common::kMacChineseTrad), Common::U32String("�A"));
		// Decoded text: "�".
		TS_ASSERT_EQUALS(Common::String("\xa1").decode(Common::kMacChineseSimp), Common::U32String("�"));
		// Decoded text: "�@A".
		TS_ASSERT_EQUALS(Common::String("\xa1\x40" "A").decode(Common::kMacChineseSimp), Common::U32String("�@A"));
		// Decoded text: "�A".
		TS_ASSERT_EQUALS(Common::String("\xfc\xfe" "A").decode(Common::kMacChineseSimp), Common::U32String("�A"));
		// Text: "𐀀😀<U+D800><U+F860>" -> "!!!!" in all four encodings.
		const Common::u32char_type_t unicode[] = {0x10000, 0x1f600, 0xd800, 0xf860};
		const Common::U32String unsupported(unicode, ARRAYSIZE(unicode));
		static constexpr Common::CodePage pages[] = {
			Common::kMacKorean,
			Common::kMacJapanese,
			Common::kMacChineseTrad,
			Common::kMacChineseSimp,
		};
		for (uint i = 0; i < ARRAYSIZE(pages); i++) {
			Common::String result;
			TS_ASSERT_EQUALS(unsupported.encode(result, pages[i], '!'), Common::kStringEncodingResultHasErrors);
			TS_ASSERT_EQUALS(result, "!!!!");
		}
#endif
	}
};
