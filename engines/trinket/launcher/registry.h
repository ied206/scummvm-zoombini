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

#ifndef TRINKET_LAUNCHER_REGISTRY_H
#define TRINKET_LAUNCHER_REGISTRY_H

#include "common/array.h"
#include "common/savefile.h"

namespace Trinket {
namespace Launcher {

/** Launcher identity records; no graphics or game instance is needed to edit them. */
class Registry {
public:
	struct User {
		/** Persistent launcher user identifier, independent of display order. */
		uint id;
		/** Player name shown in the launcher. */
		Common::String name;
	};
	/** Registered launcher users in their current list order. */
	Common::Array<User> users;
	/** Next persistent user identifier available for allocation. */
	uint nextID = 1;
	/** Whether launcher background music is enabled. */
	bool musicEnabled = true;
	/** Whether launcher sound effects are enabled. */
	bool soundEnabled = true;
	/** Whether the stored launcher preference requests automatic updates. */
	bool updatesAutomatic = true;
	/** Read launcher users and preferences from the save manager, returning false for invalid XML. */
	bool load(Common::SaveFileManager &manager, const Common::String &file);
	/** Write launcher users and preferences through the save manager. */
	bool store(Common::SaveFileManager &manager, const Common::String &file) const;
	/** Compare user names for alphabetical display order. */
	static bool less(const User &left, const User &right) { return left.name < right.name; }

private:
	/** Closing launcher registry elements. */
	static constexpr const char *kDocumentFooter = "</users>\n</launcher>\n";
	/** Serialized player identifier and escaped name. */
	static constexpr const char *kUserFormat = "<user%u><name>%s</name></user%u>\n";
	/** Next player identifier and opening user list. */
	static constexpr const char *kUsersHeaderFormat = "<nextID>%u</nextID>\n<users>\n";
	/** Serialized launcher update mode. */
	static constexpr const char *kUpdateModeFormat = "<updateMode>%s</updateMode>\n";
	/** Serialized launcher music and sound-effect settings. */
	static constexpr const char *kSoundSettingsFormat = "<musicState>%d</musicState>\n<sfxState>%d</sfxState>\n";
	/** XML declaration and opening launcher registry element. */
	static constexpr const char *kDocumentHeader = "<?xml version=\"1.0\" standalone=\"yes\"?>\n<launcher>\n";
	/** XML path of the next player identifier. */
	static constexpr const char *kNextIDPath = "launcher/nextID";
	/** XML path of the launcher music setting. */
	static constexpr const char *kMusicStatePath = "launcher/musicState";
	/** XML path of the launcher sound-effect setting. */
	static constexpr const char *kEffectsStatePath = "launcher/sfxState";
	/** XML path of the launcher update mode. */
	static constexpr const char *kUpdateModePath = "launcher/updateMode";
	/** Stored value selecting manual launcher updates. */
	static constexpr const char *kManualUpdateMode = "manual";
	/** XML path prefix preceding each player identifier. */
	static constexpr const char *kUserPathPrefix = "launcher/users/user";
	/** Path suffix selecting a registered player name. */
	static constexpr const char *kNameElementSuffix = "/name";
	/** Stored value selecting automatic launcher updates. */
	static constexpr const char *kAutomaticUpdateMode = "auto";

	/** Parse a nonnegative launcher identifier without changing the output on failure. */
	static bool parseID(const Common::String &value, uint &id);
};

} // End of namespace Launcher
} // End of namespace Trinket

#endif
