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

#include "trinket/rr1cloud9/resources.h"

#include "common/fs.h"
#include "common/memstream.h"
#include "trinket/compression/nc1.h"

namespace Trinket {
namespace RR1Cloud9 {

constexpr const char *Resources::kR1comName;
constexpr const char *Resources::kCompreName;
constexpr const char *Resources::kComName;
constexpr const char *Resources::kComsfxName;
constexpr const char *Resources::kComtunzName;

constexpr const char *Resources::kInstalledDirectory;
constexpr const char *Resources::kResourceDirectory;
constexpr const char *Resources::kSharedResourceDirectory;
constexpr const char *Resources::kScriptDirectory;
constexpr const char *Resources::kSharedScriptDirectory;
constexpr const char *Resources::kMainScript;
constexpr const char *Resources::kLibraryExtension;
constexpr const char *Resources::kSampleLibrary;
constexpr const char *Resources::kSampleDirectory;
constexpr const char *Resources::kCommonLibrary;
constexpr const char *Resources::kResolvedNameFormat;
constexpr const char *Resources::kAnimationTag;
constexpr const char *Resources::kPaletteTag;
constexpr const char *Resources::kBackgroundTag;
constexpr const char *Resources::kSoundTag;
constexpr const char *Resources::kFontTag;
constexpr const char *Resources::kLipsTag;
constexpr const char *Resources::kTextTag;
constexpr const char *Resources::kAnimationExtension;
constexpr const char *Resources::kPaletteExtension;
constexpr const char *Resources::kBackgroundExtension;
constexpr const char *Resources::kSoundExtension;
constexpr const char *Resources::kFontExtension;
constexpr const char *Resources::kLipsExtension;
constexpr const char *Resources::kTextExtension;

Common::SeekableReadStream *Resources::unpack(Common::SeekableReadStream *stream, const Common::String &tag) {
	if (!stream || (!tag.equals(kAnimationTag) && !tag.equals(kPaletteTag)))
		return stream;
	if (stream->size() < 4 || stream->readUint32BE() != MKTAG('N', 'C', '1', 26)) {
		stream->seek(0);
		return stream;
	}
	Common::ScopedPtr<Common::SeekableReadStream> source(stream);
	if (source->size() < 17 || UINT32_MAX < static_cast<uint64>(source->size()))
		return nullptr;
	const uint32 size = source->readUint32LE();
	// Each three-byte match expands by at most 255 bytes. Check before allocating.
	const uint32 packedSize = source->size() - 16;
	if (!size || (tag.equals(kPaletteTag) && size != 1536) || 1 + static_cast<uint64>(packedSize - 1) * 255 / 3 < size)
		return nullptr;
	source->seek(16);
	Common::Array<byte> packed;
	packed.resize(packedSize);
	if (source->read(packed.data(), packedSize) != packedSize)
		return nullptr;
	byte *decoded = static_cast<byte *>(malloc(size));
	if (!decoded)
		return nullptr;
	if (!decompressNC1(packed.data(), packedSize, decoded, size)) {
		free(decoded);
		return nullptr;
	}
	return new Common::MemoryReadStream(decoded, size, DisposeAfterUse::YES);
}

bool Resources::load(Common::Path &gamePath) {
	_libraries.clear();
	_resolvedNames.clear();
	_directories.clear();
	Common::FSNode root(gamePath);
	Common::FSNode installed = root.getChild(kInstalledDirectory);
	if (installed.getChild(kScriptDirectory).getChild(kMainScript).exists())
		gamePath = installed.getPath();
	else if (root.getName().equalsIgnoreCase(kInstalledDirectory) && root.getParent().getChild(kResourceDirectory).isDirectory())
		root = root.getParent();
	for (const Common::Path &base : {gamePath, root.getPath()}) {
		_directories.push_back(base.join(kResourceDirectory));
		_directories.push_back(base.join(kSharedResourceDirectory));
		_directories.push_back(base.join(kScriptDirectory));
		_directories.push_back(base.join(kSharedScriptDirectory));
	}
	Common::ScopedPtr<Common::SeekableReadStream> script(open(kMainScript));
	return script != nullptr;
}

Common::SeekableReadStream *Resources::openLibrary(const Common::String &library, const Common::String &tag, uint id) const {
	auto found = _libraries.find(library);
	if (found != _libraries.end())
		return unpack(found->_value->getResource(tag, id), tag);
	for (const Common::Path &directory : _directories) {
		Common::Path path = directory.join(library + kLibraryExtension);
		if (!Common::FSNode(path).exists())
			continue;
		Common::SharedPtr<Common::NEResources> resource(new Common::NEResources());
		if (!resource->loadFromEXE(Common::FSNode(path).createReadStream()))
			return nullptr;
		_libraries[library] = resource;
		return unpack(resource->getResource(tag, id), tag);
	}
	return nullptr;
}

Common::String Resources::cacheKey(const Common::String &name) const {
	Common::String key = OM::ResourceProvider::cacheKey(name);
	return _resolvedNames.getValOrDefault(key, key);
}

Common::SeekableReadStream *Resources::open(const Common::String &name) const {
	if (name.contains("..") || name.contains('\\') || name.contains(':'))
		return nullptr;
	uint slash = name.find('/');
	if (slash == Common::String::npos) {
		for (const Common::Path &directory : _directories) {
			Common::FSNode file(directory.join(name));
			if (!file.exists() || file.isDirectory())
				continue;
			Common::SeekableReadStream *stream = file.createReadStream();
			if (stream)
				return stream;
		}
		return nullptr;
	}
	Common::String library = name.substr(0, slash);
	library.toLowercase();
	if (library.equals(kSampleLibrary)) {
		Common::String filename = name.substr(slash + 1);
		if (filename.contains('/') || filename.empty())
			return nullptr;
		for (const Common::Path &directory : _directories) {
			Common::FSNode file(directory.join(kSampleDirectory).join(filename));
			if (file.exists() && !file.isDirectory()) {
				Common::SeekableReadStream *stream = file.createReadStream();
				if (stream)
					return stream;
			}
		}
		return nullptr;
	}
	const char *extension = strrchr(name.c_str(), '.');
	if (!extension)
		return nullptr;
	const char *tag = nullptr;
	if (!strcmp(extension, kAnimationExtension))
		tag = kAnimationTag;
	else if (!strcmp(extension, kPaletteExtension))
		tag = kPaletteTag;
	else if (!strcmp(extension, kBackgroundExtension))
		tag = kBackgroundTag;
	else if (!strcmp(extension, kSoundExtension))
		tag = kSoundTag;
	else if (!strcmp(extension, kFontExtension))
		tag = kFontTag;
	else if (!strcmp(extension, kLipsExtension))
		tag = kLipsTag;
	else if (!strcmp(extension, kTextExtension))
		tag = kTextTag;
	if (!tag)
		return nullptr;
	uint id = 0;
	for (const char *p = name.c_str() + slash + 1; p < extension; p++) {
		if (*p < '0' || '9' < *p || 32767 < id)
			return nullptr;
		id = id * 10 + *p - '0';
	}
	if (!id || 32767 < id)
		return nullptr;
	if (library.equals(kCommonLibrary)) {
		for (const char *common : {kR1comName, kCompreName, kComName, kComsfxName, kComtunzName}) {
			Common::SeekableReadStream *stream = openLibrary(common, tag, id);
			if (stream) {
				_resolvedNames[OM::ResourceProvider::cacheKey(name)] = Common::String::format(kResolvedNameFormat, common, id, extension);
				return stream;
			}
		}
		return nullptr;
	}
	return openLibrary(library, tag, id);
}

} // namespace RR1Cloud9
} // namespace Trinket
