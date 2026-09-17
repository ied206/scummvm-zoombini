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
#include "common/util.h"
#include "engines/advancedDetector.h"
#include "zoombini2/detection_tables.h"

static constexpr PlainGameDescriptor zoombini2Games[] = {
	{"zoombini2", "Zoombinis: Mountain Rescue"},
	{nullptr, nullptr},
};

static constexpr char *const directoryGlobs[] = {
	"Data",
	"INSTALL",
	"HD",
	"Bmp",
	"ZOMBIS",
	"Sounds",
	"movies",
	nullptr,
};

class Zoombini2MetaEngineDetection : public AdvancedMetaEngineDetection<Zoombini2::Zoombini2GameDescription> {
public:
	Zoombini2MetaEngineDetection() : AdvancedMetaEngineDetection(Zoombini2::gameDescriptions, zoombini2Games) {
		_maxScanDepth = 5; // Supports INSTALL/HD/Bmp/ZOMBIS and Data/Bmp/ZOMBIS
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

	DetectedGame toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const override;
};

DetectedGame Zoombini2MetaEngineDetection::toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const {
	DetectedGame game = AdvancedMetaEngineDetection::toDetectedGame(adGame, extraInfo);
	const Zoombini2::Zoombini2GameDescription *zoombini2Desc = reinterpret_cast<const Zoombini2::Zoombini2GameDescription *>(adGame.desc);

	if ((zoombini2Desc->features & (Zoombini2::GF_Z2_V10 | Zoombini2::GF_Z2_V11)) && zoombini2Desc->desc.extra && *zoombini2Desc->desc.extra) {
		// Keep each release target stable when multiple versions share the same platform and language.
		Common::String versionTag;
		for (const char *character = zoombini2Desc->desc.extra; *character; character++) {
			if (Common::isAlnum(*character))
				versionTag += *character;
			else if (*character == '_')
				versionTag += '-';
		}
		versionTag.toLowercase();

		if (!versionTag.empty()) {
			game.preferredTarget += '-';
			game.preferredTarget += versionTag;
		}
	}

	return game;
}

REGISTER_PLUGIN_STATIC(ZOOMBINI2_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, Zoombini2MetaEngineDetection);
