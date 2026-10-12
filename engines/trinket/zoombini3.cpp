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

#include "trinket/zoombini3.h"

#include "common/archive.h"
#include "common/fs.h"
#include "trinket/launcher/version.h"
#include "trinket/shell/resource.h"
#include "trinket/trinket.h"
#include "trinket/zoombini3_game.h"
#include "trinket/zoombini3_savefiles.h"
#include "trinket/zoombini3_version.h"

namespace Trinket {
namespace Zoombini3 {

static OM::ResourceProvider *createResources() {
	return new Shell::IndexedResources();
}

static bool loadResources(OM::ResourceProvider &resources, Common::Path &dataPath) {
	Common::FSNode directory(dataPath);
	SearchMan.addSubDirectoryMatching(directory, "rsc");
	Common::FSNode installed = directory.getChild("HD");
	if (installed.getChild("rsc").getChild("root.inx").exists()) {
		directory = installed;
		SearchMan.addDirectory("trinket-installed-rsc", directory.getChild("rsc"));
	}
	dataPath = directory.getPath();
	SearchMan.addSubDirectoryMatching(directory, "scripts");
	return static_cast<Shell::IndexedResources &>(resources).load(Common::Path("root.inx"));
}

const Subengine &getSubengine() {
	static const Subengine subengine = {
		&Game::create, &createResources, &loadResources,
		"Zoombinis: Island Odyssey", &SaveFiles::create,
		OriginalVersions::kWindowsGame, Launcher::OriginalVersions::kUSInstalled,
		"sku381664-CD", kRecruitment11, 800, 600};
	return subengine;
}

} // namespace Zoombini3
} // namespace Trinket
