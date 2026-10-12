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
#include "common/str.h"
#include "common/translation.h"
#include "common/util.h"
#include "engines/advancedDetector.h"

#include "trinket/detection.h"
#include "trinket/detection_tables.h"

static const PlainGameDescriptor trinketGames[] = {
	{"zoombini3", "Zoombinis: Island Odyssey"},
	{"rr1cloud9", "Reader Rabbit 1st Grade: Capers on Cloud Nine!"},
	{nullptr, nullptr},
};

static const char *const directoryGlobs[] = {
	"hd",
	"install",
	"rsc",
	"scripts",
	"r1scripts",
	"r1rsc",
	"Zoombinis Island Odyssey",
	nullptr,
};

class TrinketMetaEngineDetection : public AdvancedMetaEngineDetection<Trinket::TrinketGameDescription> {
public:
	TrinketMetaEngineDetection() : AdvancedMetaEngineDetection(Trinket::gameDescriptions, trinketGames) {
		_md5Bytes = 5000;
		_maxScanDepth = 4;
		_directoryGlobs = directoryGlobs;
	}

	const char *getName() const override {
		return "trinket";
	}

	const char *getEngineName() const override {
		return "Trinket";
	}

	const char *getOriginalCopyright() const override {
		return "Trinket games (C) The Learning Company / Riverdeep Interactive Learning Limited";
	}

	DetectedGame toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo = nullptr) const override {
		DetectedGame game = AdvancedMetaEngineDetection::toDetectedGame(adGame, extraInfo);

		// Like Mohawk's Zoombinis detector, retain the release in the target ID.
		Common::String versionTag;
		for (const char *c = adGame.desc->extra; c && *c; ++c) {
			if (Common::isAlnum(*c))
				versionTag += *c;
			else if (*c == '_' || *c == '-')
				versionTag += '-';
		}
		versionTag.toLowercase();
		if (!versionTag.empty()) {
			game.preferredTarget += "-";
			game.preferredTarget += versionTag;
		}

		// Unsupported entries normally use extra as an error message, not a
		// version. Keep the release visible while reporting the missing runtime.
		const char *title = nullptr;
		for (const PlainGameDescriptor *entry = trinketGames; entry->gameId; entry++)
			if (!strcmp(entry->gameId, adGame.desc->gameId))
				title = entry->description;
		game.description = DetectedGame(getName(), adGame.desc->gameId, title,
										adGame.desc->language, adGame.desc->platform, adGame.desc->extra, false)
							   .description;
		if (adGame.desc->flags & ADGF_UNSUPPORTED)
			game.extra = _s("Zoombinis: Island Odyssey is not yet implemented.");
		return game;
	}
};

REGISTER_PLUGIN_STATIC(TRINKET_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, TrinketMetaEngineDetection);
