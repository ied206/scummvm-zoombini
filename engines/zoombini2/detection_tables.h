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

static constexpr Zoombini2GameDescription gameDescriptions[] = {
	// Zoombinis: Mountain Rescue
	// English Windows, v1.0
	// Build Date: 2001-09-09 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.0US",
			AD_ENTRY2s("INSTALL/HD/Bmp/Map/background.bb", "8810734b173c40bac5863997a7196f12", 1440024,
					   "INSTALL/HD/zoombini2.exe", "e3e8951dc399ffb571fdfa46f5ae3a0c", 647168),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT),
		},
		GF_Z2_V10,
	},
	// Zoombinis: Mountain Rescue
	// Dutch (Belgium and Netherlands) Windows, v1.0
	// Build Date: 2001-09-09 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.0NL",
			AD_ENTRY1s("INSTALL/HD/Bmp/Map/background.bb", "cfc9d681ca98b4aa639fdd43a21dce8e", 1440024),
			Common::NL_NLD,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GAMEOPTION_HELP_PAGE_COLOR_KEYING),
		},
		GF_Z2_V10 | GF_Z2_SOLID_HELP_PAGES | GF_Z2_SOLID_HELP_FRAME,
	},
	// Zoombinis: Mountain Rescue
	// Hebrew Windows, v1.0
	// Build Date: 2003-06-23 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.0HE",
			AD_ENTRY1s("INSTALL/HD/Bmp/Map/background.bb", "b2b60177a0fdbb315961f86d4ef8d06d", 1440024),
			Common::HE_ISR,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GAMEOPTION_HELP_PAGE_COLOR_KEYING),
		},
		GF_Z2_V10 | GF_Z2_SOLID_HELP_PAGES,
	},
	// Zoombinis: Mountain Rescue
	// English Windows, v1.1
	// Build Date: 2001-11-30 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.1US",
			AD_ENTRY2s("INSTALL/HD/Bmp/Map/background.bb", "8810734b173c40bac5863997a7196f12", 1440024,
					   "Data/Bmp/Menu/Option - dialogues Normal.rb", "e7d39b38548a648dedd3b72c684e09a7", 81756),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT),
		},
		GF_Z2_V11,
	},
	// Zoombinis: Mountain Rescue
	// English Macintosh, v1.1; also the 2003 Adventure Workshop disc.
	// Build Date: 2001-11-27
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.1US",
			AD_ENTRY2s("Data/Bmp/Map/background.bb", "8810734b173c40bac5863997a7196f12", 1440024,
					   "Zoombinis Mountain Rescue", "r:c4b2f9ca1a9245089ae66f1d77c69794", 3604),
			Common::EN_USA,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT),
		},
		GF_Z2_V11,
	},
	// Zoombinis: Mountain Rescue
	// British English Windows, v1.1 family
	// Build Date: 2001-11-30 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.1UK",
			AD_ENTRY2s("INSTALL/HD/Bmp/Map/background.bb", "8810734b173c40bac5863997a7196f12", 1440024,
					   "Data/Sounds/ZBV11.wav", "e4a61e4c288975cd2066bb46e102b0d3", 467420),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT),
		},
		GF_Z2_V11,
	},
	// Zoombinis: Mountain Rescue
	// British English Macintosh, v1.1.
	// Build Date: 2001-11-27
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.1UK",
			AD_ENTRY2s("Data/Bmp/Map/background.bb", "8810734b173c40bac5863997a7196f12", 1440024,
					   "Zoombinis 2", "r:1ac2511b19c9a9079e08f75cf76802a0", 3492),
			Common::EN_GRB,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO1(GUIO_NOASPECT),
		},
		GF_Z2_V11,
	},
	// Zoombinis: Mountain Rescue
	// Korean Windows, v1.1, ArisuMedia
	// Build Date: 2002-04-08 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.1KR",
			AD_ENTRY1s("INSTALL/HD/Bmp/Map/background.bb", "019d27367ef3cd4fc83ab77ac8c4f217", 1440024),
			Common::KO_KOR,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GAMEOPTION_HELP_PAGE_COLOR_KEYING),
		},
		GF_Z2_V11 | GF_Z2_SOLID_HELP_PAGES,
	},
	// Zoombinis: Mountain Rescue
	// Swedish Windows, v1.1
	// Build Date: 2001-11-30 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.1SE",
			AD_ENTRY2s("INSTALL/HD/Bmp/Map/background.bb", "8810734b173c40bac5863997a7196f12", 1440024,
					   "INSTALL/HD/zoombini2.exe", "0b7546aa8ba6d3f0ad05d37c3276d676", 2395248),
			Common::SV_SWE,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT),
		},
		GF_Z2_V11,
	},
	// Zoombinis: Mountain Rescue
	// Polish Windows, v1.1
	// Build Date: 2001-11-30 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.1PL",
			AD_ENTRY1s("Bmp/Map/background.bb", "b58e398807cd459d563cb6338c3fa32b", 1440024),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO1(GUIO_NOASPECT),
		},
		GF_Z2_V11,
	},
	// Zoombinis: Mountain Rescue
	// English Windows, Prerelease US Demo
	// Uses flat 'Bmp/' layout, unlike final releases.
	// Build Date: 2001-07-30 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"Prerelease US Demo",
			AD_ENTRY1s("Bmp/story_intro/title_screen.bb", "e82d7fb66bce4e2fc811cac72322e925", 1440024),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_DEMO,
			GUIO1(GUIO_NOASPECT),
		},
		GF_Z2_V10,
	},

	{AD_TABLE_END_MARKER, GF_NONE},
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_DETECTION_TABLES_H
