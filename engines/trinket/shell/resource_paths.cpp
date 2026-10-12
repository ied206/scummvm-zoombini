/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/shell/resource_paths.h"

namespace Trinket {
namespace Shell {

Common::String ResourcePaths::normalize(const Common::String &name) {
	Common::String normalized = name;
	normalized.replace('\\', '/');
	normalized.toLowercase();
	return normalized;
}

void ResourcePaths::addPath(const Common::String &path) {
	Common::String normalized = normalize(path);
	while (!normalized.empty() && normalized.hasSuffix("/"))
		normalized.deleteLastChar();
	_paths.push_back(normalized);
}

void ResourcePaths::use(const Common::String &directory) {
	Common::String path = normalize(directory);
	if (path == "..") {
		uint slash = _directory.findLastOf('/');
		_directory = slash == Common::String::npos ? Common::String() : _directory.substr(0, slash);
	} else if (path.hasPrefix("/")) {
		_directory = path.substr(1);
	} else {
		_directory += path;
	}
}

Common::String ResourcePaths::find(const Common::String &name, Common::SeekableReadStream *&stream) const {
	Common::String normalized = normalize(name);
	bool absolute = normalized.hasPrefix("/");
	Common::String candidate = absolute ? normalized.substr(1) : normalized;
	if (!absolute && !_directory.empty())
		candidate = _directory + "/" + candidate;
	stream = _provider.open(candidate);
	if (stream)
		return candidate;
	uint slash = normalized.findLastOf('/');
	for (uint i = 0; i < _paths.size(); i++) {
		candidate = _paths[i] + (absolute ? Common::String() : Common::String("/")) + normalized;
		stream = _provider.open(candidate);
		if (stream)
			return candidate;
		if (slash != Common::String::npos) {
			candidate = _paths[i] + "/" + normalized.substr(slash + 1);
			stream = _provider.open(candidate);
			if (stream)
				return candidate;
		}
	}
	return Common::String();
}

Common::SeekableReadStream *ResourcePaths::open(const Common::String &name) const {
	Common::SeekableReadStream *stream = nullptr;
	find(name, stream);
	return stream;
}

Common::String ResourcePaths::resolve(const Common::String &name) const {
	Common::SeekableReadStream *stream = nullptr;
	Common::String resolved = find(name, stream);
	delete stream;
	return resolved;
}

Common::String ResourcePaths::resourcePath(const Common::String &id, const Common::String &extension) const {
	Common::String name = normalize(id);
	Common::String suffix = extension.hasPrefix(".") ? extension : "." + extension;
	if (!name.hasSuffix(suffix))
		name += suffix;
	Common::String resolved = resolve(name);
	// Consumers reopen this virtual path without applying the active MPS directory again.
	if (!resolved.empty())
		resolved = "/" + resolved;
	return resolved;
}

} // namespace Shell
} // namespace Trinket
