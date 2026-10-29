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

#ifndef TRINKET_RR1CLOUD9_RESOURCES_H
#define TRINKET_RR1CLOUD9_RESOURCES_H

#include "common/formats/winexe_ne.h"
#include "common/ptr.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace RR1Cloud9 {

/**
 * RR1Cloud9 game assets use NE-format RSC libraries.
 * Each library has its own resource table and tag-to-type mapping, read by @ref Common::NEResources.
 * Explicit library names keep page resources with overlapping IDs separate.
 * Libraries are cached on demand; the caller must delete each returned payload stream.
 */
class Resources : public OM::ResourceProvider {
public:
	/** Locate the canonical game directories and initialize resource-library lookup. */
	bool load(Common::Path &gamePath);
	/**
	 * Resolve library/id.extension to an NE resource; r1map/4300.ao means r1map.rsc, ASEQ, ID 4300.
	 * Extensions select the resource tag, and the first path component selects the library.
	 * common searches r1com, compre, com, comsfx and comtunz in that order.
	 * Names have no leading slash; Z3's @ref Trinket::Shell::ResourcePaths root syntax does not apply here.
	 * Bare filenames and samples/filename open loose files instead of NE entries.
	 * Expand NC1-wrapped ASEQ/RRGB payloads before returning them to OM consumers.
	 */
	Common::SeekableReadStream *open(const Common::String &name) const override;
	/** Use the actual library selected by a successful common-resource lookup. */
	Common::String cacheKey(const Common::String &name) const override;

private:
	/** Script identifier for r1com. */
	static constexpr const char *kR1comName = "r1com";
	/** Script identifier for compre. */
	static constexpr const char *kCompreName = "compre";
	/** Script identifier for com. */
	static constexpr const char *kComName = "com";
	/** Script identifier for comsfx. */
	static constexpr const char *kComsfxName = "comsfx";
	/** Script identifier for comtunz. */
	static constexpr const char *kComtunzName = "comtunz";

	/** Installed data directory on the original disc. */
	static constexpr const char *kInstalledDirectory = "HD";
	/** Directory containing game resource libraries. */
	static constexpr const char *kResourceDirectory = "R1RSC";
	/** Directory containing shared resource libraries. */
	static constexpr const char *kSharedResourceDirectory = "RSC";
	/** Directory containing game page scripts. */
	static constexpr const char *kScriptDirectory = "R1scripts";
	/** Fallback directory containing page scripts. */
	static constexpr const char *kSharedScriptDirectory = "scripts";
	/** Entry script used to verify the resource root. */
	static constexpr const char *kMainScript = "r1main.mps";
	/** Extension of NE resource libraries. */
	static constexpr const char *kLibraryExtension = ".rsc";
	/** Virtual library for loose sample artwork. */
	static constexpr const char *kSampleLibrary = "samples";
	/** Directory containing loose sample artwork. */
	static constexpr const char *kSampleDirectory = "Samples";
	/** Virtual alias for ordered shared-library lookup. */
	static constexpr const char *kCommonLibrary = "common";
	/** Resolved library, resource ID and extension for cache identity. */
	static constexpr const char *kResolvedNameFormat = "%s/%u%s";
	/** Resource tag for animation sequences. */
	static constexpr const char *kAnimationTag = "ASEQ";
	/** Resource tag for palettes. */
	static constexpr const char *kPaletteTag = "RRGB";
	/** Resource tag for background images. */
	static constexpr const char *kBackgroundTag = "BK16";
	/** Resource tag for audio. */
	static constexpr const char *kSoundTag = "WAVE";
	/** Resource tag for bitmap fonts. */
	static constexpr const char *kFontTag = "NFNT";
	/** Resource tag for lip animation. */
	static constexpr const char *kLipsTag = "LIPS";
	/** Resource tag for text. */
	static constexpr const char *kTextTag = "TEXT";
	/** Virtual resource extension for animation sequences. */
	static constexpr const char *kAnimationExtension = ".ao";
	/** Virtual resource extension for palettes. */
	static constexpr const char *kPaletteExtension = ".rgb";
	/** Virtual resource extension for background images. */
	static constexpr const char *kBackgroundExtension = ".bk16";
	/** Virtual resource extension for audio. */
	static constexpr const char *kSoundExtension = ".wav";
	/** Virtual resource extension for bitmap fonts. */
	static constexpr const char *kFontExtension = ".nfnt";
	/** Virtual resource extension for lip animation. */
	static constexpr const char *kLipsExtension = ".lips";
	/** Virtual resource extension for text. */
	static constexpr const char *kTextExtension = ".txt";

	/** Expand supported NC1 resource wrappers, returning the original stream when no wrapper is present. */
	static Common::SeekableReadStream *unpack(Common::SeekableReadStream *stream, const Common::String &tag);
	/** Game resource directories searched in precedence order. */
	Common::Array<Common::Path> _directories;
	/** NE resource libraries cached by their normalized path. */
	mutable Common::HashMap<Common::String, Common::SharedPtr<Common::NEResources>> _libraries;
	/** Successful common aliases, cleared with the provider's library namespace. */
	mutable Common::HashMap<Common::String, Common::String> _resolvedNames;
	/** Open a typed resource ID from the selected NE library. */
	Common::SeekableReadStream *openLibrary(const Common::String &library, const Common::String &tag, uint id) const;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
