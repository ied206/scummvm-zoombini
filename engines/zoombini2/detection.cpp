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
#include "common/translation.h"
#include "common/util.h"
#include "engines/advancedDetector.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "zoombini2/detection_tables.h"

static constexpr PlainGameDescriptor zoombini2Games[] = {
	{"zoombini2", "Zoombinis: Mountain Rescue"},
	{nullptr, nullptr},
};

static constexpr const char *directoryGlobs[] = {
	"Data",
	"INSTALL",
	"HD",
	"Bmp",
	"Map",
	"ZOMBIS",
	"Sounds",
	"movies",
	nullptr,
};

class Zoombini2MetaEngineDetection : public AdvancedMetaEngineDetection<Zoombini2::Zoombini2GameDescription> {
public:
	/** Configure release detection for the disc and installed resource directory layouts. */
	Zoombini2MetaEngineDetection()
		: AdvancedMetaEngineDetection(Zoombini2::gameDescriptions, zoombini2Games) {
		_maxScanDepth = 5; // Supports INSTALL/HD/Bmp/ZOMBIS and Data/Bmp/ZOMBIS
		_directoryGlobs = directoryGlobs;
		_flags = kADFlagMatchFullPaths | kADFlagPreferFallbackDetection;
	}

	/** Return the engine identifier used by the launcher. */
	const char *getName() const override {
		return "zoombini2";
	}

	/** Return the game's display name. */
	const char *getEngineName() const override {
		return "Zoombinis: Mountain Rescue";
	}

	/** Return the original game's copyright notice. */
	const char *getOriginalCopyright() const override {
		return "Zoombinis: Mountain Rescue (C) 2001 The Learning Company";
	}

	/** Add a normalized release tag to the preferred launcher target name. */
	DetectedGame toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const override;
	/** Show installer extraction instructions during interactive detection, before registration. */
	DetectedGames detectGames(const Common::FSList &fslist, uint32 skipADFlags = 0, bool skipIncomplete = false) override;
	/** Recognize known installer discs and identify unknown releases by their resource layout. */
	ADDetectedGame fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const override;

private:
	/**
	 * Return the match with its actual file fingerprints if all required files satisfy the description.
	 * @param allFiles Files indexed by their detector paths.
	 * @param description Description containing the required paths and any size or hash constraints.
	 */
	ADDetectedGame matchFallbackFiles(const FileMap &allFiles, const ADGameDescription &description) const;
};

DetectedGames Zoombini2MetaEngineDetection::detectGames(const Common::FSList &fslist, uint32 skipADFlags, bool skipIncomplete) {
	const DetectedGames matches = AdvancedMetaEngineDetection::detectGames(fslist, skipADFlags, skipIncomplete);
	DetectedGames games;
	for (const DetectedGame &game : matches) {
		if (game.gameSupportLevel == kUnsupportedGame) {
			if (skipADFlags & ADGF_UNSUPPORTED)
				continue;
			const Common::U32String &inst = _(game.extra);
			if (GUI::GuiManager::hasInstance() && g_gui.isActive()) {
				GUI::MessageDialog dialog(inst);
				dialog.runModal();
			} else {
				warning("%s", inst.encode(Common::kUtf8).c_str());
			}
		}
		games.push_back(game);
	}
	return games;
}

ADDetectedGame Zoombini2MetaEngineDetection::matchFallbackFiles(const FileMap &allFiles, const ADGameDescription &description) const {
	ADDetectedGame game(&description);
	for (const ADGameFileDescription *fileDesc = description.filesDescriptions; fileDesc->fileName; fileDesc++) {
		const Common::Path path(fileDesc->fileName);
		FileProperties fileProp;
		if (!allFiles.contains(path) || !getFileProperties(allFiles, kMD5Head, path, fileProp) || fileProp.size <= 0 ||
			(fileDesc->fileSize != AD_NO_SIZE && fileProp.size != fileDesc->fileSize) || (fileDesc->md5 && fileProp.md5 != fileDesc->md5)) {
			return ADDetectedGame();
		}
		game.matchedFiles[path] = fileProp;
	}
	return game;
}

ADDetectedGame Zoombini2MetaEngineDetection::fallbackDetect(const FileMap &allFiles, const Common::FSList &fslist, ADDetectedGameExtraInfo **extra) const {
	for (const Zoombini2::Zoombini2GameDescription &installerDesc : Zoombini2::kPackagedInstallers) {
		ADDetectedGame game = matchFallbackFiles(allFiles, installerDesc.desc);
		if (!game.desc)
			continue;
		if (extra) {
			*extra = new ADDetectedGameExtraInfo();
			(*extra)->gameName = Common::String::format("%s Installer", getEngineName());
			(*extra)->targetID = "zoombini2-installer";
		}
		return game;
	}
	for (const Zoombini2::Zoombini2GameDescription &release : Zoombini2::kFuzzyGameDescriptions) {
		ADDetectedGame game = matchFallbackFiles(allFiles, release.desc);
		if (!game.desc)
			continue;
		game.hasUnknownFiles = true;
		return game;
	}
	return ADDetectedGame();
}

DetectedGame Zoombini2MetaEngineDetection::toDetectedGame(const ADDetectedGame &adGame, ADDetectedGameExtraInfo *extraInfo) const {
	DetectedGame game = AdvancedMetaEngineDetection::toDetectedGame(adGame, extraInfo);
	const Zoombini2::Zoombini2GameDescription *gameDesc = reinterpret_cast<const Zoombini2::Zoombini2GameDescription *>(adGame.desc);
	// Installer entries store extraction instructions in extra.
	// Skip release suffix generation to keep those instructions out of the target ID.
	if (gameDesc->features & Zoombini2::GF_Z2_INSTALLER)
		return game;

	if (game.gameId == "zoombini2" && adGame.desc->extra && *adGame.desc->extra) {
		// Keep each release target stable when multiple versions share the same platform and language.
		static constexpr char kDemoSuffix[] = " Demo";
		const bool isDemo = (adGame.desc->flags & ADGF_DEMO) != 0;
		Common::String versionExtra = adGame.desc->extra;
		if (isDemo && versionExtra.hasSuffix(kDemoSuffix))
			versionExtra.erase(versionExtra.size() - (sizeof(kDemoSuffix) - 1));

		Common::String versionTag;
		for (const char *character = versionExtra.c_str(); *character; character++) {
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
