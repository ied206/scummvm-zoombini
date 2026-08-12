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

#include <string.h>

#include "common/debug.h"
#include "common/memstream.h"
#include "common/savefile.h"

#include "zoombini2/game_state.h"
#include "zoombini2/saveload.h"

namespace Zoombini2 {

Zoombini2SavegameManager::Zoombini2SavegameManager(Common::SaveFileManager *saveFileManager, const Common::String &target)
	: _saveFileManager(saveFileManager), _target(target.empty() ? "zoombini2" : target) {
}

Common::String Zoombini2SavegameManager::makeSaveFileName(const Common::String &profileName) const {
	return Common::String::format("%s-%s.mk", _target.c_str(), profileName.c_str());
}

bool Zoombini2SavegameManager::isValidProfileName(const Common::String &profileName) {
	if (profileName.empty() || kMaximumProfileNameLength < static_cast<int>(profileName.size()) ||
	        profileName.firstChar() == ' ' || profileName.lastChar() == ' ')
		return false;

	bool previousWasSpace = false;
	for (uint i = 0; i < profileName.size(); i++) {
		const char character = profileName[i];
		const bool isLetter = ('A' <= character && character <= 'Z') || ('a' <= character && character <= 'z');
		const bool isDigit = '0' <= character && character <= '9';
		const bool isSpace = character == ' ';
		if ((!isLetter && !isDigit && !isSpace) || (isSpace && previousWasSpace))
			return false;
		previousWasSpace = isSpace;
	}
	return true;
}

void Zoombini2SavegameManager::addProfileSorted(Common::StringArray &profiles, const Common::String &profileName) {
	uint index = 0;
	while (index < profiles.size() && profiles[index].compareToIgnoreCase(profileName) < 0)
		index += 1;
	if (index < profiles.size() && profiles[index].equalsIgnoreCase(profileName))
		return;
	profiles.insert_at(index, profileName);
}

Common::StringArray Zoombini2SavegameManager::listProfiles() const {
	Common::StringArray profiles;
	if (!_saveFileManager)
		return profiles;

	const Common::String prefix = _target + "-";
	const Common::String suffix = ".mk";
	const Common::StringArray saveFiles = _saveFileManager->listSavefiles(prefix + "*" + suffix);
	for (uint i = 0; i < saveFiles.size(); i++) {
		const Common::String &saveFileName = saveFiles[i];
		if (saveFileName.size() <= prefix.size() + suffix.size())
			continue;
		if (!saveFileName.substr(0, prefix.size()).equalsIgnoreCase(prefix) ||
		        !saveFileName.substr(saveFileName.size() - suffix.size()).equalsIgnoreCase(suffix))
			continue;

		const Common::String profileName = saveFileName.substr(prefix.size(), saveFileName.size() - prefix.size() - suffix.size());
		if (!isValidProfileName(profileName)) {
			warning("Zoombini2SavegameManager: Ignoring invalid profile save '%s'", saveFileName.c_str());
			continue;
		}
		addProfileSorted(profiles, profileName);
	}
	return profiles;
}

bool Zoombini2SavegameManager::verifySaveData(const Common::String &saveFileName, const byte *data, uint32 size) const {
	Common::InSaveFile *stream = _saveFileManager->openForLoading(saveFileName);
	if (!stream)
		return false;

	bool matches = stream->size() == size;
	byte buffer[4096];
	for (uint32 offset = 0; matches && offset < size;) {
		const uint32 amount = MIN<uint32>(sizeof(buffer), size - offset);
		matches = stream->read(buffer, amount) == amount && memcmp(buffer, data + offset, amount) == 0 && !stream->err();
		offset += amount;
	}
	delete stream;
	return matches;
}

bool Zoombini2SavegameManager::saveProfile(const Common::String &profileName, const GameState &state,
		const Common::Array<Zoombini *> *globalRoster) const {
	if (!_saveFileManager || !isValidProfileName(profileName))
		return false;

	Common::MemoryWriteStreamDynamic data(DisposeAfterUse::YES);
	if (!state.save(&data, globalRoster))
		return false;

	const Common::String saveFileName = makeSaveFileName(profileName);
	Common::OutSaveFile *stream = _saveFileManager->openForSaving(saveFileName, false);
	if (!stream) {
		warning("Zoombini2SavegameManager: Could not open '%s' for writing", saveFileName.c_str());
		return false;
	}

	bool ok = stream->write(data.getData(), data.size()) == data.size();
	stream->finalize();
	ok = ok && !stream->err();
	delete stream;
	if (ok)
		ok = verifySaveData(saveFileName, data.getData(), data.size());

	if (ok)
		debug(1, "Saved Zoombini2 profile to %s", saveFileName.c_str());
	else
		warning("Zoombini2SavegameManager: Failed to write '%s'", saveFileName.c_str());
	return ok;
}

bool Zoombini2SavegameManager::loadProfile(const Common::String &profileName, GameState &state) const {
	if (!_saveFileManager || !isValidProfileName(profileName))
		return false;

	const Common::String saveFileName = makeSaveFileName(profileName);
	Common::InSaveFile *stream = _saveFileManager->openForLoading(saveFileName);
	if (!stream) {
		warning("Zoombini2SavegameManager: Could not open '%s' for reading", saveFileName.c_str());
		return false;
	}

	const bool ok = state.load(stream);
	delete stream;
	if (ok)
		debug(1, "Loaded Zoombini2 profile from %s", saveFileName.c_str());
	else
		warning("Zoombini2SavegameManager: Failed to load '%s'", saveFileName.c_str());
	return ok;
}

bool Zoombini2SavegameManager::deleteProfile(const Common::String &profileName) const {
	if (!_saveFileManager || !isValidProfileName(profileName))
		return false;
	return _saveFileManager->removeSavefile(makeSaveFileName(profileName));
}

bool Zoombini2SavegameManager::renameProfile(const Common::String &oldProfileName, const Common::String &newProfileName) const {
	if (!_saveFileManager || !isValidProfileName(oldProfileName) || !isValidProfileName(newProfileName))
		return false;
	if (oldProfileName == newProfileName)
		return true;
	if (oldProfileName.equalsIgnoreCase(newProfileName) || profileExists(newProfileName))
		return false;

	GameState renamedState;
	if (!loadProfile(oldProfileName, renamedState))
		return false;
	renamedState._playerName = newProfileName;
	if (!saveProfile(newProfileName, renamedState))
		return false;
	if (deleteProfile(oldProfileName))
		return true;

	deleteProfile(newProfileName);
	return false;
}

bool Zoombini2SavegameManager::profileExists(const Common::String &profileName) const {
	return _saveFileManager && isValidProfileName(profileName) && _saveFileManager->exists(makeSaveFileName(profileName));
}

} // End of namespace Zoombini2
