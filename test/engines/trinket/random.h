#include <cxxtest/TestSuite.h>

#include "common/config-manager.h"
#include "test/system/null_osystem.h"
#include "trinket/metaengine.h"
#include "trinket/shell/random.h"

#if NULL_OSYSTEM_IS_AVAILABLE && !defined(ENABLE_EVENTRECORDER)

class TrinketRandomTestSuite : public CxxTest::TestSuite {
public:
	/** Install a deterministic backend for the Common::RandomSource owned by each wrapper. */
	void setUp() {
		Common::install_null_g_system();
		ConfMan.setInt("random_seed", 1, Common::ConfigManager::kTransientDomain);
		ConfMan.setInt(TrinketMetaEngine::kOptionPrngAlgorithm, 0, Common::ConfigManager::kTransientDomain);
	}
	/** Release the test backend and temporary PRNG settings. */
	void tearDown() {
		ConfMan.removeKey("random_seed", Common::ConfigManager::kTransientDomain);
		ConfMan.removeKey(TrinketMetaEngine::kOptionPrngAlgorithm, Common::ConfigManager::kTransientDomain);
		Common::uninstall_null_g_system();
	}
	/** Native MSVCRT and Metrowerks MSL vectors differ from the first seed-one draw. */
	void testNativePlatformVectors() {
		Trinket::Shell::Random windows("windows", Common::kPlatformWindows);
		Trinket::Shell::Random mac("mac", Common::kPlatformMacintosh);
		const int winExpected[] = {41, 18467, 6334, 26500, 19169};
		const int macExpected[] = {16838, 5758, 10113, 17515, 31051};
		for (uint i = 0; i < 5; i++) {
			TS_ASSERT_EQUALS(windows.getRandomNumber(32767), winExpected[i]);
			TS_ASSERT_EQUALS(mac.getRandomNumber(32767), macExpected[i]);
		}
		mac.setSeed(0);
		TS_ASSERT_EQUALS(mac.getSeed(), 0U);
		TS_ASSERT_EQUALS(mac.getRandomNumber(32767), 0);
		TS_ASSERT_EQUALS(mac.getSeed(), 12345U);
	}
	/** Macintosh draws retain Shell's singleton, reversed-range and exclusion consumption contracts. */
	void testMacConsumption() {
		Trinket::Shell::Random rnd("mac", Common::kPlatformMacintosh);
		TS_ASSERT_EQUALS(rnd.getRandomNumberRng(7, 7), 7);
		TS_ASSERT_EQUALS(rnd.getSeed(), 1U);
		TS_ASSERT_EQUALS(rnd.getRandomNumberExcluding(0), 0);
		TS_ASSERT_EQUALS(rnd.getSeed(), 1U);
		TS_ASSERT_EQUALS(rnd.getRandomNumber(0), 0);
		TS_ASSERT_EQUALS(rnd.getSeed(), 1103527590U);
		TS_ASSERT_EQUALS(rnd.getRandomNumberRng(5, 1), 4);
		rnd.setSeed(1);
		TS_ASSERT_EQUALS(rnd.getRandomNumberExcluding(5, 3), 4);
		TS_ASSERT_EQUALS(rnd.getSeed(), 1103527590U);
	}
	/** ScummVM standard mode remains independent of the detected game platform. */
	void testStandardMode() {
		Trinket::Shell::Random windows("windows", Common::kPlatformWindows);
		Trinket::Shell::Random mac("mac", Common::kPlatformMacintosh);
		windows.setAlgorithm(Trinket::Shell::PrngAlgorithmKind::kStandardPrng);
		mac.setAlgorithm(Trinket::Shell::PrngAlgorithmKind::kStandardPrng);
		windows.setSeed(1234);
		mac.setSeed(1234);
		for (uint i = 0; i < 20; i++)
			TS_ASSERT_EQUALS(windows.getRandomNumber(1000000), mac.getRandomNumber(1000000));
	}
};

#endif
