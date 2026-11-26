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

#ifndef TRINKET_DETECTION_TABLES_H
#define TRINKET_DETECTION_TABLES_H

namespace Trinket {

// Build dates use PE/COFF (UTC) or PEF timestamps; actual build times are unverified.
static const TrinketGameDescription gameDescriptions[] = {
	// Zoombinis: Island Odyssey
	// v1.0US
	// Build Date: 2002-09-26 UTC
	// From Hajin Jang
	{
		{
			"zoombini3",
			"v1.0US",
			AD_ENTRY2s("root.inx", "2ca0ed3bc2cbccd73d0e557ce52082ea", 45660,
					   "z3p1.xml", "8ba6e53eba7afff758ed3caf829c454e", 1455),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOMIDI, GUIO_NOLANG),
		},
		GF_Z3_10,
	},
	// Zoombinis: Island Odyssey
	// v1.0UK: British
	// Build Date: 2002-09-26 UTC
	// From Hajin Jang
	{
		{
			"zoombini3",
			"v1.0UK",
			AD_ENTRY2s("root.inx", "2ca0ed3bc2cbccd73d0e557ce52082ea", 45660,
					   "z3p1.xml", "f2a4d4287f2b169fed46985ee8790325", 1456),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO2(GUIO_NOMIDI, GUIO_NOLANG),
		},
		GF_Z3_10,
	},
	// Zoombinis: Island Odyssey
	// v1.0SE: Swedish
	// Build Date: 2002-09-26 UTC
	// From Hajin Jang
	{
		{
			"zoombini3",
			"v1.0SE",
			AD_ENTRY2s("root.inx", "01d1705d427102cf7a943c7a76f72ca3", 45660,
					   "z3p1.xml", "c6fbd15de8d41156feea2ee8a6d0a04b", 1439),
			Common::SV_SWE,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO2(GUIO_NOMIDI, GUIO_NOLANG),
		},
		GF_Z3_10,
	},
	// Zoombinis: Island Odyssey
	// v1.0RU: Russian, Russobit-M / Nikita
	// Build Date: 2002-09-26 UTC; localized files dated 2004
	// From Hajin Jang
	{
		{
			"zoombini3",
			"v1.0RU",
			AD_ENTRY2s("root.inx", "b2d10d565dc3053a26a0f4bc939b13ea", 45950,
					   "z3p1.xml", "1cedc481c92c9711f5c898208d1e402a", 1450),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOLANG),
		},
		GF_Z3_10 | TrinketGameDescription::kZ3DirectStart,
	},
	// Zoombinis: Archipel en danger
	// v1.0FR: French, Mindscape hybrid disc
	// Build Date: 2002-09-26 UTC
	// From Hajin Jang
	{
		{
			"zoombini3",
			"v1.0FR",
			AD_ENTRY2s("root.inx", "8a18f6b88c190c6039818932f9aa544b", 45660,
					   "z3p1.xml", "4fd77e2bb0bbe005d360fdd23d58339c", 1440),
			Common::FR_FRA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOLANG),
		},
		GF_Z3_10 | TrinketGameDescription::kZ3DirectStart,
	},
	// Zoombinis: Island Odyssey
	// v1.0US: Macintosh PowerPC
	// Build Date: 2002-09-28
	// From Hajin Jang
	{
		{
			"zoombini3",
			"v1.0US",
			AD_ENTRY3s("root.inx", "2ca0ed3bc2cbccd73d0e557ce52082ea", 45660,
					   "z3p1.xml", "8ba6e53eba7afff758ed3caf829c454e", 1455,
					   "Zoombinis Island Odyssey", "rt:d140804107330a071409c5c8dd016855", 86584),
			Common::EN_USA,
			Common::kPlatformMacintosh,
			ADGF_TESTING,
			GUIO2(GUIO_NOMIDI, GUIO_NOLANG),
		},
		GF_Z3_10,
	},
	// Zoombinis: Island Odyssey
	// v1.0UK: Macintosh PowerPC, Mindscape hybrid disc
	// Build Date: 2002-09-28
	// From Hajin Jang
	{
		{
			"zoombini3",
			"v1.0UK",
			AD_ENTRY3s("root.inx", "2ca0ed3bc2cbccd73d0e557ce52082ea", 45660,
					   "z3p1.xml", "f2a4d4287f2b169fed46985ee8790325", 1456,
					   "Zoombinis Island Odyssey", "rt:d140804107330a071409c5c8dd016855", 86584),
			Common::EN_GRB,
			Common::kPlatformMacintosh,
			ADGF_UNSUPPORTED,
			GUIO2(GUIO_NOMIDI, GUIO_NOLANG),
		},
		GF_Z3_10,
	},
	// Zoombinis: Archipel en danger
	// v1.0FR: French, Macintosh PowerPC, Mindscape hybrid disc
	// From Hajin Jang
	{
		{
			"zoombini3",
			"v1.0FR",
			AD_ENTRY3s("root.inx", "8a18f6b88c190c6039818932f9aa544b", 45660,
					   "z3p1.xml", "4fd77e2bb0bbe005d360fdd23d58339c", 1440,
					   "Zoombinis Island Odyssey", "rt:d140804107330a071409c5c8dd016855", 86584),
			Common::FR_FRA,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOLANG),
		},
		GF_Z3_10 | TrinketGameDescription::kZ3DirectStart,
	},
	// Reader Rabbit 1st Grade: Capers on Cloud Nine!
	// Build Date: 2001-05-31 UTC
	// From Hajin Jang
	{
		{
			"rr1cloud9",
			"",
			AD_ENTRY2s("r1main.mps", "a60557ed6ef34be47fcccdc8a8003fc8", 75859,
					   "r1cn1.mps", "228d014d75c471ed4491346911533fa6", 102497),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO2(GUIO_NOMIDI, GUIO_NOLANG),
		},
		GF_RR1_CLOUD9,
	},
	// Reader Rabbit 1st Grade: Capers on Cloud Nine!
	// Macintosh PowerPC
	// Build Date: 2001-05-30
	// From Hajin Jang
	{
		{
			"rr1cloud9",
			"",
			AD_ENTRY3s("r1main.mps", "a60557ed6ef34be47fcccdc8a8003fc8", 75859,
					   "r1cn1.mps", "228d014d75c471ed4491346911533fa6", 102497,
					   "1st Grade Capers on Cloud Nine", "rt:8b3f400a008c60c488fcf32fed267604", 13171),
			Common::EN_USA,
			Common::kPlatformMacintosh,
			ADGF_TESTING,
			GUIO2(GUIO_NOMIDI, GUIO_NOLANG),
		},
		GF_RR1_CLOUD9,
	},
	{AD_TABLE_END_MARKER, 0},
};

/** Z3-only fallback layouts, with the platform marker first in each list. */
static const TrinketGameDescription kFuzzyZ3GameDescriptions[] = {
	// A hybrid may also contain the Windows executable.
	{
		{
			"zoombini3",
			"Unknown release",
			AD_ENTRY5s("Zoombinis Island Odyssey", nullptr, AD_NO_SIZE, "root.inx", nullptr, AD_NO_SIZE,
					   "z3common.bul", nullptr, AD_NO_SIZE, "z3p1.mps", nullptr, AD_NO_SIZE, "z3p1.xml", nullptr, AD_NO_SIZE),
			Common::UNK_LANG,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI),
		},
		TrinketGameDescription::kZ3DirectStart,
	},
	{
		{
			"zoombini3",
			"Unknown release",
			AD_ENTRY5s("Zoombinis Island Odyssey.exe", nullptr, AD_NO_SIZE, "root.inx", nullptr, AD_NO_SIZE,
					   "z3common.bul", nullptr, AD_NO_SIZE, "z3p1.mps", nullptr, AD_NO_SIZE, "z3p1.xml", nullptr, AD_NO_SIZE),
			Common::UNK_LANG,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI),
		},
		TrinketGameDescription::kZ3DirectStart,
	},
};

} // End of namespace Trinket

#endif // TRINKET_DETECTION_TABLES_H
