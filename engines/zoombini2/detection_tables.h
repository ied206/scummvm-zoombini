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

#include "common/translation.h"
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
			AD_ENTRY2s("Data/Bmp/story_intro/title_screen.bb", "e4744f42813f036773e8d48b9c583b16", 1440024,
					   "Data/Bmp/Menu/Option - dialogues Normal.rb", "10d3959ca5190057fb0a2913345d81f4", 103452),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
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
			AD_ENTRY1s("Data/Bmp/story_intro/title_screen.bb", "01b2b85b22e0a77f4d8eaadb5afaf832", 1440024),
			Common::NL_NLD,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOASPECT, GUIO_NOLANG, GAMEOPTION_HELP_PAGE_COLOR_KEYING),
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
			AD_ENTRY1s("Data/Bmp/story_intro/title_screen.bb", "c63af90f0298c73afe2e85307e6609cd", 1440024),
			Common::HE_ISR,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOASPECT, GUIO_NOLANG, GAMEOPTION_HELP_PAGE_COLOR_KEYING),
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
			AD_ENTRY2s("Data/Bmp/story_intro/title_screen.bb", "e4744f42813f036773e8d48b9c583b16", 1440024,
					   "Data/Bmp/Menu/Option - dialogues Normal.rb", "e7d39b38548a648dedd3b72c684e09a7", 81756),
			Common::EN_USA,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
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
			AD_ENTRY1s("Bmp/story_intro/title_screen.bb", "e4744f42813f036773e8d48b9c583b16", 1440024),
			Common::EN_USA,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
		},
		GF_Z2_V11,
	},
	// Zoombinis: Mountain Rescue
	// British English Windows, v1.1
	// Build Date: 2001-11-30 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.1UK",
			// Three exact matches take precedence over the shared US v1.0 title/menu pair.
			AD_ENTRY3s("Data/Bmp/story_intro/title_screen.bb", "e4744f42813f036773e8d48b9c583b16", 1440024,
					   "Data/Bmp/Menu/Option - dialogues Normal.rb", "10d3959ca5190057fb0a2913345d81f4", 103452,
					   "Data/Sounds/ZBV11.wav", "e4a61e4c288975cd2066bb46e102b0d3", 467420),
			Common::EN_GRB,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
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
			AD_ENTRY2s("Bmp/story_intro/title_screen.bb", "e4744f42813f036773e8d48b9c583b16", 1440024,
					   "Sounds/ZBV11.wav", "e4a61e4c288975cd2066bb46e102b0d3", 467420),
			Common::EN_GRB,
			Common::kPlatformMacintosh,
			ADGF_UNSTABLE,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
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
			AD_ENTRY1s("Data/Bmp/story_intro/title_screen.bb", "0a6f0d7bad69ca12cf35101840a345c7", 1440024),
			Common::KO_KOR,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO3(GUIO_NOASPECT, GUIO_NOLANG, GAMEOPTION_HELP_PAGE_COLOR_KEYING),
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
			AD_ENTRY1s("Data/Bmp/story_intro/title_screen.bb", "8f8c78f2e127c718d1bdd2a8a0949a43", 1440024),
			Common::SV_SWE,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
		},
		GF_Z2_V11,
	},
	// Zoombinis: Mountain Rescue
	// Polish Windows, v1.1
	// Disc layouts require extracting Inno Setup archives.
	// Build Date: 2001-11-30 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.1PL",
			AD_ENTRY1s("Bmp/story_intro/title_screen.bb", "291986f837a49738a23d1d92fc78c2a5", 1440024),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
		},
		GF_Z2_V11,
	},
	// Zoombinis: Mountain Rescue
	// Norwegian Windows, v1.1 family
	// Build Date: 2001-11-30 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.1NO",
			AD_ENTRY1s("Data/Bmp/story_intro/title_screen.bb", "7c587bc44ed3dbd8cae1372e17fbb7f0", 1440024),
			Common::NB_NOR,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
		},
		GF_Z2_V11,
	},
	// Zoombinis: Mountain Rescue
	// Russian Windows, v1.1 family; disc and installed layouts with Data.
	// Disc layouts require extracting setup.exe's app payload into INSTALL/HD.
	// Unextracted discs receive extraction guidance from the installer fallback.
	// The installed executable supplies required name and keyboard tables.
	// Build Date: 2001-11-30 UTC
	// From Hajin Jang
	{
		{
			"zoombini2",
			"v1.1RU",
			AD_ENTRY2s("Data/Bmp/story_intro/title_screen.bb", "bbb3d6cf5b54b3286c4bf861f43a77fd", 1440024,
					   "zoombini2.exe", "1f07e410fdccac259eb68d997f38ede7", 790609),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_NO_FLAGS,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
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
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
		},
		GF_Z2_V10,
	},

	{AD_TABLE_END_MARKER, GF_NONE},
};

/**
 * Packaged installer signatures used by fallback detection to provide extraction instructions.
 */
static constexpr Zoombini2GameDescription kPackagedInstallers[] = {
	// Zoombinis: Mountain Rescue
	// Polish Windows, v1.1
	// UDF exposes the BIN names expected by innounp.
	// Packaged Date: 2008-01-23
	// From Hajin Jang
	{
		{
			"zoombini2",
			// I18N: Keep the command, filenames and the literal {app} directory name unchanged.
			_s("This v1.1PL disc dump is a packaged Inno Setup installer. "
			   "Extract resources with \"innounp -x -a setup.exe\", then add the extracted \"{app}\" directory to ScummVM."),
			AD_ENTRY3s("setup.exe", "c2d9a4c1acaeb52b7416e6888bf0401f", 480300,
					   "setup-1.bin", "693367f0a821ffce129e9c923ca500d3", 209234432,
					   "setup-2.bin", "041a8c19054cdbca0fe642057ff6bcf8", 73070177),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
		},
		GF_Z2_INSTALLER,
	},
	// Zoombinis: Mountain Rescue
	// Polish Windows, v1.1
	// ISO9660 exposes underscore BIN names that must be renamed for innounp.
	// Packaged Date: 2008-01-23
	// From Hajin Jang
	{
		{
			"zoombini2",
			// I18N: Keep the command, filenames and the literal {app} directory name unchanged.
			_s("This v1.1PL disc dump is a packaged Inno Setup installer. "
			   "Rename SETUP_1.BIN/SETUP_2.BIN to setup-1.bin/setup-2.bin. "
			   "Then extract resources with \"innounp -x -a setup.exe\", then register the extracted \"{app}\" directory to ScummVM."),
			AD_ENTRY3s("setup.exe", "c2d9a4c1acaeb52b7416e6888bf0401f", 480300,
					   "setup_1.bin", "693367f0a821ffce129e9c923ca500d3", 209234432,
					   "setup_2.bin", "041a8c19054cdbca0fe642057ff6bcf8", 73070177),
			Common::PL_POL,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
		},
		GF_Z2_INSTALLER,
	},
	// Zoombinis: Mountain Rescue
	// Russian Windows, v1.1 family
	// Disc Data is loose; installed resources are packed in setup.exe.
	// Packaged Date: 2004-08-16
	// From Hajin Jang
	{
		{
			"zoombini2",
			// I18N: Keep the command, filenames and directory names unchanged, including the literal {app} directory.
			_s("This v1.1RU disc packaged some resources files into its Inno Setup installer. "
			   "Run \"innounp -x -a setup.exe\", rename extracted \"{app}\" into \"INSTALL/HD\", "
			   "then register the disc dump root directory to ScummVM."),
			AD_ENTRY2s("setup.exe", "97648880a50fcdc39aa245c25c9e2971", 68650082,
					   "Data/Bmp/story_intro/title_screen.bb", "bbb3d6cf5b54b3286c4bf861f43a77fd", 1440024),
			Common::RU_RUS,
			Common::kPlatformWindows,
			ADGF_UNSUPPORTED,
			GUIO2(GUIO_NOASPECT, GUIO_NOLANG),
		},
		GF_Z2_INSTALLER,
	},
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_DETECTION_TABLES_H
