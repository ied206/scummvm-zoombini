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

Common::SeekableReadStream *Resources::unpack(Common::SeekableReadStream *stream, const Common::String &tag) {
	if (!stream || (tag != "ASEQ" && tag != "RRGB"))
		return stream;
	if (stream->size() < 4 || stream->readUint32BE() != MKTAG('N', 'C', '1', 26)) {
		stream->seek(0);
		return stream;
	}
	Common::ScopedPtr<Common::SeekableReadStream> source(stream);
	if (source->size() < 17 || 0xffffffffULL < static_cast<uint64>(source->size()))
		return nullptr;
	const uint32 size = source->readUint32LE();
	// Each three-byte match expands by at most 255 bytes. Check before allocating.
	const uint32 packedSize = source->size() - 16;
	if (!size || (tag == "RRGB" && size != 1536) || 1 + static_cast<uint64>(packedSize - 1) * 255 / 3 < size)
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
	_directories.clear();
	Common::FSNode root(gamePath);
	Common::FSNode installed = root.getChild("HD");
	if (installed.getChild("R1scripts").getChild("r1main.mps").exists())
		gamePath = installed.getPath();
	else if (root.getName().equalsIgnoreCase("HD") && root.getParent().getChild("R1RSC").isDirectory())
		root = root.getParent();
	for (const Common::Path &base : {gamePath, root.getPath()}) {
		_directories.push_back(base.join("R1RSC"));
		_directories.push_back(base.join("RSC"));
		_directories.push_back(base.join("R1scripts"));
		_directories.push_back(base.join("scripts"));
	}
	Common::ScopedPtr<Common::SeekableReadStream> script(open("r1main.mps"));
	return script != nullptr;
}

Common::SeekableReadStream *Resources::openLibrary(const Common::String &library, const Common::String &tag, uint id) const {
	auto found = _libraries.find(library);
	if (found != _libraries.end())
		return unpack(found->_value->open(tag, id), tag);
	for (const Common::Path &directory : _directories) {
		Common::Path path = directory.join(library + ".rsc");
		if (!Common::FSNode(path).exists())
			continue;
		Common::SharedPtr<OM::NEResourceLibrary> resource(new OM::NEResourceLibrary());
		if (!resource->load(path))
			return nullptr;
		_libraries[library] = resource;
		return unpack(resource->open(tag, id), tag);
	}
	return nullptr;
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
	if (library == "samples") {
		Common::String filename = name.substr(slash + 1);
		if (filename.contains('/') || filename.empty())
			return nullptr;
		for (const Common::Path &directory : _directories) {
			Common::FSNode file(directory.join("Samples").join(filename));
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
	if (!strcmp(extension, ".ao"))
		tag = "ASEQ";
	else if (!strcmp(extension, ".rgb"))
		tag = "RRGB";
	else if (!strcmp(extension, ".bk16"))
		tag = "BK16";
	else if (!strcmp(extension, ".wav"))
		tag = "WAVE";
	else if (!strcmp(extension, ".nfnt"))
		tag = "NFNT";
	else if (!strcmp(extension, ".lips"))
		tag = "LIPS";
	else if (!strcmp(extension, ".txt"))
		tag = "TEXT";
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
	if (library == "common") {
		for (const char *common : {"r1com", "compre", "com", "comsfx", "comtunz"}) {
			Common::SeekableReadStream *stream = openLibrary(common, tag, id);
			if (stream)
				return stream;
		}
		return nullptr;
	}
	return openLibrary(library, tag, id);
}

} // namespace RR1Cloud9
} // namespace Trinket
