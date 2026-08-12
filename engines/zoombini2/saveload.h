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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ZOOMBINI2_SAVELOAD_H
#define ZOOMBINI2_SAVELOAD_H

#include "common/array.h"
#include "common/str-array.h"

namespace Common {
class SaveFileManager;
}

namespace Zoombini2 {

class GameState;
class Zoombini;

/** Target-scoped storage for the original game's named player profiles. */
class Zoombini2SavegameManager {
public:
	static constexpr int kMaximumProfileNameLength = 16;

	Zoombini2SavegameManager(Common::SaveFileManager *saveFileManager, const Common::String &target);

	Common::StringArray listProfiles() const;
	bool saveProfile(const Common::String &profileName, const GameState &state, const Common::Array<Zoombini *> *globalRoster = nullptr) const;
	bool loadProfile(const Common::String &profileName, GameState &state) const;
	bool deleteProfile(const Common::String &profileName) const;
	bool renameProfile(const Common::String &oldProfileName, const Common::String &newProfileName) const;
	bool profileExists(const Common::String &profileName) const;

	static bool isValidProfileName(const Common::String &profileName);

private:
	Common::String makeSaveFileName(const Common::String &profileName) const;
	static void addProfileSorted(Common::StringArray &profiles, const Common::String &profileName);
	bool verifySaveData(const Common::String &saveFileName, const byte *data, uint32 size) const;

	Common::SaveFileManager *_saveFileManager;
	Common::String _target;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_SAVELOAD_H
