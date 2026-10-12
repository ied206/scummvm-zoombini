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

#include "trinket/launcher/registry.h"

#include "common/algorithm.h"
#include "common/ptr.h"
#include "trinket/ompp/xml.h"

namespace Trinket {
namespace Launcher {

bool Registry::parseID(const Common::String &value, uint &id) {
	id = 0;
	if (value.empty())
		return false;
	for (uint i = 0; i < value.size(); i++) {
		if (value[i] < '0' || '9' < value[i])
			return false;
		uint digit = value[i] - '0';
		if ((2147483647u - digit) / 10 < id)
			return false;
		id = id * 10 + digit;
	}
	return id != 0;
}

bool Registry::load(Common::SaveFileManager &manager, const Common::String &file) {
	Registry candidate;
	OM::XMLParser xml;
	Common::InSaveFile *stream = manager.openForLoading(file);
	if (!stream) {
		if (!manager.listSavefiles(file).empty())
			return false;
		*this = candidate;
		return true;
	}
	if (!xml.loadStream(stream) || !xml.parse())
		return false;
	if (!parseID(xml.get("launcher/nextID"), candidate.nextID))
		return false;
	candidate.musicEnabled = xml.get("launcher/musicState") != "0";
	candidate.soundEnabled = xml.get("launcher/sfxState") != "0";
	candidate.updatesAutomatic = xml.get("launcher/updateMode") != "manual";
	const Common::String prefix = "launcher/users/user";
	for (const auto &element : xml.elements()) {
		if (!element.path.hasPrefix(prefix) || !element.path.hasSuffix("/name"))
			continue;
		User user;
		Common::String id = element.path.substr(prefix.size(), element.path.size() - prefix.size() - 5);
		user.name = element.value;
		if (!parseID(id, user.id) || candidate.nextID <= user.id || user.name.empty() || 21 < user.name.size() || 99 <= candidate.users.size())
			return false;
		for (uint i = 0; i < user.name.size(); i++) {
			if (static_cast<byte>(user.name[i]) < 32)
				return false;
		}
		for (uint i = 0; i < candidate.users.size(); i++) {
			if (candidate.users[i].id == user.id || candidate.users[i].name == user.name)
				return false;
		}
		candidate.users.push_back(user);
	}
	Common::sort(candidate.users.begin(), candidate.users.end(), less);
	*this = candidate;
	return true;
}

bool Registry::store(Common::SaveFileManager &manager, const Common::String &file) const {
	Common::ScopedPtr<Common::OutSaveFile> stream(manager.openForSaving(file));
	if (!stream)
		return false;
	stream->writeString("<?xml version=\"1.0\" standalone=\"yes\"?>\n<launcher>\n");
	stream->writeString(Common::String::format("<musicState>%d</musicState>\n<sfxState>%d</sfxState>\n", musicEnabled, soundEnabled));
	stream->writeString(Common::String::format("<updateMode>%s</updateMode>\n", updatesAutomatic ? "auto" : "manual"));
	stream->writeString(Common::String::format("<nextID>%u</nextID>\n<users>\n", nextID));
	for (uint i = 0; i < users.size(); i++) {
		const User &user = users[i];
		stream->writeString(Common::String::format("<user%u><name>%s</name></user%u>\n", user.id, OM::XMLParser::escape(user.name).c_str(), user.id));
	}
	stream->writeString("</users>\n</launcher>\n");
	stream->finalize();
	return !stream->err();
}

} // namespace Launcher
} // namespace Trinket
