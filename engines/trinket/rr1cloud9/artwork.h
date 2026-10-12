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

#ifndef TRINKET_RR1CLOUD9_ARTWORK_H
#define TRINKET_RR1CLOUD9_ARTWORK_H

#include "common/array.h"
#include "common/stream.h"
#include "graphics/surface.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace RR1Cloud9 {

/** The five CA2 books. Empty pixels mean the CD sample (or a blank page). */
class Artwork {
public:
	static constexpr int kWidth = 320, kHeight = 214, kVisibleHeight = 212, kFrames = 18, kBooks = 5;
	static constexpr uint16 kWhite = 32767;
	struct Frame {
		Common::Array<uint16> pixels;
		bool marked = false;
	};
	Frame &frame(int book, int page) { return _frames[(book - 1) * kFrames + page - 1]; }
	const Frame &frame(int book, int page) const { return _frames[(book - 1) * kFrames + page - 1]; }
	void clear();
	int markedCount() const;
	bool image(const OM::ResourceProvider &resources, int book, int page, Common::Array<uint16> &pixels) const;
	bool load(Common::ReadStream &stream);
	bool save(Common::WriteStream &stream) const;
	static void render(const Common::Array<uint16> &pixels, Graphics::Surface &surface, bool preview = false);

private:
	Frame _frames[kBooks * kFrames];
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
