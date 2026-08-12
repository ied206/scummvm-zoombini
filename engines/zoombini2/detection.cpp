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
#include "zoombini2/detection_tables.h"

static const PlainGameDescriptor zoombini2Games[] = {
	{"zoombini2", "Zoombinis: Mountain Rescue"},
	{nullptr, nullptr}
};

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

class Zoombini2MetaEngineDetection : public AdvancedMetaEngineDetection<Zoombini2::Zoombini2GameDescription> {
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
