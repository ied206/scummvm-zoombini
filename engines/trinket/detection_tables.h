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

static const TrinketGameDescription gameDescriptions[] = {
	// Zoombinis: Island Odyssey
	// v1.0US
	{
		{
			"zoombini3",
			"v1.0US",
			AD_ENTRY2s("root.inx", "2ca0ed3bc2cbccd73d0e557ce52082ea", 45660,
					   "z3p1.xml", "8ba6e53eba7afff758ed3caf829c454e", 1455),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOMIDI),
		},
		GF_Z3_10,
	},
	// Zoombinis: Island Odyssey
	// v1.0UK: British
	{
		{
			"zoombini3",
			"v1.0UK",
			AD_ENTRY2s("root.inx", "2ca0ed3bc2cbccd73d0e557ce52082ea", 45660,
					   "z3p1.xml", "f2a4d4287f2b169fed46985ee8790325", 1456),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO1(GUIO_NOMIDI),
		},
		GF_Z3_10,
	},
	// Zoombinis: Island Odyssey
	// v1.0SE: Swedish
	{
		{
			"zoombini3",
			"v1.0SE",
			AD_ENTRY2s("root.inx", "01d1705d427102cf7a943c7a76f72ca3", 45660,
					   "z3p1.xml", "c6fbd15de8d41156feea2ee8a6d0a04b", 1439),
			Common::SV_SWE,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO1(GUIO_NOMIDI),
		},
		GF_Z3_10,
	},
	// Reader Rabbit 1st Grade: Capers on Cloud Nine!
	{
		{
			"rr1cloud9",
			"",
			AD_ENTRY2s("r1main.mps", "a60557ed6ef34be47fcccdc8a8003fc8", 75859,
					   "r1cn1.mps", "228d014d75c471ed4491346911533fa6", 102497),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOMIDI),
		},
		GF_RR1_CLOUD9,
	},
	{AD_TABLE_END_MARKER, 0},
};

} // End of namespace Trinket

#endif // TRINKET_DETECTION_TABLES_H
