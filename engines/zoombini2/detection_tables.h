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

#ifndef ZOOMBINI2_DETECTION_TABLES_H
#define ZOOMBINI2_DETECTION_TABLES_H

#include "zoombini2/detection.h"

namespace Zoombini2 {

static const Zoombini2GameDescription gameDescriptions[] = {
	// Zoombinis: Mountain Rescue
	// English Windows, v1.0
	{
		{
			"zoombini2",
			"v1.0US",
			AD_ENTRY4s("Data/Bmp/ZOMBIS/littleZomb.anm", "45a25d37d7c01a5dd4eea814340aad32", 1451723,
			           "Data/Sounds/DW-Zville.wav", "e49a4979f87022f6cc681ee88df43472", 199762,
			           "INSTALL/HD/Bmp/Map/background.bb", "8810734b173c40bac5863997a7196f12", 1440024,
			           "INSTALL/HD/zoombini2.exe", "e3e8951dc399ffb571fdfa46f5ae3a0c", 647168),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GF_Z2_V10
	},
	// Zoombinis: Mountain Rescue
	// Dutch (Belgium and Netherlands) Windows, v1.0
	{
		{
			"zoombini2",
			"v1.0NL",
			AD_ENTRY4s("Data/Bmp/ZOMBIS/littleZomb.anm", "45a25d37d7c01a5dd4eea814340aad32", 1451723,
			           "Data/Sounds/DW-Zville.wav", "a0f6113ab354ee1e9a60194f8f107768", 340014,
			           "INSTALL/HD/Bmp/Map/background.bb", "cfc9d681ca98b4aa639fdd43a21dce8e", 1440024,
			           "INSTALL/HD/zoombini2.exe", "f8cf557d5f5075af8d0342b74922fe40", 1639009),
			Common::NL_NLD,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GF_Z2_V10
	},
	// Zoombinis: Mountain Rescue
	// Hebrew Windows, v1.0
	{
		{
			"zoombini2",
			"v1.0HE",
			AD_ENTRY4s("Data/Bmp/ZOMBIS/littleZomb.anm", "45a25d37d7c01a5dd4eea814340aad32", 1451723,
			           "Data/Sounds/DW-Zville.wav", "434c27cef80fb8fb01fbd3f2f1158329", 194238,
			           "INSTALL/HD/Bmp/Map/background.bb", "b2b60177a0fdbb315961f86d4ef8d06d", 1440024,
			           "INSTALL/HD/zoombini2.exe", "b566daeadb2c60a80ba26064860f911c", 815104),
			Common::HE_ISR,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GF_Z2_V10
	},
	// Zoombinis: Mountain Rescue
	// English Windows, v1.1
	{
		{
			"zoombini2",
			"v1.1US",
			AD_ENTRY4s("Data/Bmp/ZOMBIS/littleZomb.anm", "45a25d37d7c01a5dd4eea814340aad32", 1451723,
			           "Data/Sounds/DW-Zville.wav", "e49a4979f87022f6cc681ee88df43472", 199762,
			           "INSTALL/HD/Bmp/Map/background.bb", "8810734b173c40bac5863997a7196f12", 1440024,
			           "INSTALL/HD/zoombini2.exe", "df24a5559803c10f03b399b25950ea00", 725073),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GF_Z2_V11
	},
	// Zoombinis: Mountain Rescue
	// Korean Windows, v1.1, ArisuMedia
	{
		{
			"zoombini2",
			"v1.1KR",
			AD_ENTRY4s("Data/Bmp/ZOMBIS/littleZomb.anm", "45a25d37d7c01a5dd4eea814340aad32", 1451723,
			           "Data/Sounds/DW-Zville.wav", "40433989cb65343cd9c9389d6ed93718", 258166,
			           "INSTALL/HD/Bmp/Map/background.bb", "019d27367ef3cd4fc83ab77ac8c4f217", 1440024,
			           "INSTALL/HD/zoombini2.exe", "7e2e8636a7d32c5b9a7a0286ed55105e", 708684),
			Common::KO_KOR,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GF_Z2_V11
	},
	// Zoombinis: Mountain Rescue
	// Swedish Windows, v1.1
	{
		{
			"zoombini2",
			"v1.1SE",
			AD_ENTRY4s("Data/Bmp/ZOMBIS/littleZomb.anm", "45a25d37d7c01a5dd4eea814340aad32", 1451723,
			           "Data/Sounds/DW-Zville.wav", "3665cf01387befb4ab4f3da97c3cf6d2", 155692,
			           "INSTALL/HD/Bmp/Map/background.bb", "8810734b173c40bac5863997a7196f12", 1440024,
			           "INSTALL/HD/zoombini2.exe", "0b7546aa8ba6d3f0ad05d37c3276d676", 2395248),
			Common::SV_SWE,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GF_Z2_V11
	},
	// Zoombinis: Mountain Rescue
	// Polish Windows, v1.1
	{
		{
			"zoombini2",
			"v1.1PL",
			AD_ENTRY4s("Bmp/ZOMBIS/littleZomb.anm", "45a25d37d7c01a5dd4eea814340aad32", 1451723,
			           "Sounds/DW-Zville.wav", "8404c666da918c96a966485f16c9bf19", 163322,
			           "Bmp/Map/background.bb", "b58e398807cd459d563cb6338c3fa32b", 1440024,
			           "zoombini2.exe", "e7267a51d0edd95718ce9a849dd21e63", 729169),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT)
		},
		GF_Z2_V11
	},

	{AD_TABLE_END_MARKER, GF_NONE}
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_DETECTION_TABLES_H
