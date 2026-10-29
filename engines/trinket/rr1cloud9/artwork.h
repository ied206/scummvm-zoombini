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
	/** Stored artwork frame width in pixels. */
	static constexpr int kWidth = 320;
	/** Stored artwork frame height in pixels. */
	static constexpr int kHeight = 214;
	/** Number of frame rows shown by the drawing interface. */
	static constexpr int kVisibleHeight = 212;
	/** Number of frames in each flipbook. */
	static constexpr int kFrames = 18;
	/** Number of flipbooks retained in the game state. */
	static constexpr int kBooks = 5;
	/** RGB555 value used for blank white artwork pixels. */
	static constexpr uint16 kWhite = 32767;
	struct Frame {
		/** RGB555 artwork pixels, empty when using a disc sample or blank page. */
		Common::Array<uint16> pixels;
		/** Whether this frame is marked in the saved book state. */
		bool marked = false;
	};
	/** Return a frame using one-based book and page numbers. */
	Frame &frame(int book, int page) { return _frames[(book - 1) * kFrames + page - 1]; }
	/** Return a frame using one-based book and page numbers. */
	const Frame &frame(int book, int page) const { return _frames[(book - 1) * kFrames + page - 1]; }
	/** Clear all edited pixels and saved frame marks. */
	void clear();
	/** Count frames marked in the current book state. */
	int markedCount() const;
	/** Copy a frame's edited pixels or load its disc sample when no edit exists. */
	bool image(const OM::ResourceProvider &resources, int book, int page, Common::Array<uint16> &pixels) const;
	/** Decode the saved flipbook frames and their marks. */
	bool load(Common::ReadStream &stream);
	/** Serialize edited frame pixels and saved marks. */
	bool save(Common::WriteStream &stream) const;
	/** Convert RGB555 artwork to a drawable surface, optionally at preview size. */
	static void render(const Common::Array<uint16> &pixels, Graphics::Surface &surface, bool preview = false);

private:
	/** Sample artwork path indexed by book and page. */
	static constexpr const char *kSampleFormat = "samples/SampleB%dI%02d.DAT";

	/** Saved artwork indexed by book and then frame. */
	Frame _frames[kBooks * kFrames];
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
