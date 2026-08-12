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

#include "base/plugins.h"
#include "engines/advancedDetector.h"
#include "common/debug.h"
#include "common/str.h"
#include "common/md5.h"
#include "common/stream.h"

#include "zoombini2/detection.h"

static const PlainGameDescriptor zoombini2Games[] = {
	{"zoombini2", "Zoombinis: Mountain Rescue"},
	{nullptr, nullptr}
};

namespace Zoombini2 {

static const ADGameDescription gameDescriptions[] = {
	// Zoombinis: Mountain Rescue - Korean release
	// Requires both Data/ (main resources) and INSTALL/HD/ (cached backgrounds)
	{
		"zoombini2",
		nullptr,
		AD_ENTRY3s("Data/Bmp/ZOMBIS/littleZomb.anm", "45a25d37d7c01a5dd4eea814340aad32", 1451723,
		           "Data/Sounds/DW-Zville.wav", "40433989cb65343cd9c9389d6ed93718", 258166,
		           "INSTALL/HD/Bmp/Map/background.bb", "019d27367ef3cd4fc83ab77ac8c4f217", 1440024),
		Common::KO_KOR,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	// Zoombinis: Mountain Rescue - English release
	// Requires both Data/ (main resources) and INSTALL/HD/ (cached backgrounds)
	{
		"zoombini2",
		nullptr,
		AD_ENTRY3s("Data/Bmp/ZOMBIS/littleZomb.anm", "45a25d37d7c01a5dd4eea814340aad32", 1451723,
		           "Data/Sounds/DW-Zville.wav", "e49a4979f87022f6cc681ee88df43472", 199762,
		           "INSTALL/HD/Bmp/Map/background.bb", "8810734b173c40bac5863997a7196f12", 1440024),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_UNSTABLE,
		GUIO1(GUIO_NOMIDI)
	},

	AD_TABLE_END_MARKER
};

} // End of namespace Zoombini2

static const char *const directoryGlobs[] = {
	"Data",
	"INSTALL",
	"HD",
	"Bmp",
	"ZOMBIS",
	"Sounds",
	"movies",
	nullptr
};

class Zoombini2MetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	Zoombini2MetaEngineDetection() : AdvancedMetaEngineDetection(Zoombini2::gameDescriptions, zoombini2Games) {
		_maxScanDepth = 5;  // Supports INSTALL/HD/Bmp/ZOMBIS and Data/Bmp/ZOMBIS
		_directoryGlobs = directoryGlobs;
		_flags = kADFlagMatchFullPaths;
	}

	const char *getName() const override {
		return "zoombini2";
	}

	const char *getEngineName() const override {
		return "Zoombinis: Mountain Rescue";
	}

	const char *getOriginalCopyright() const override {
		return "Zoombinis: Mountain Rescue (C) 2001 The Learning Company";
	}
};

REGISTER_PLUGIN_STATIC(ZOOMBINI2_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Zoombini2MetaEngineDetection);
