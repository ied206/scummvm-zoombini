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

#ifndef TRINKET_SHELL_CREDITS_H
#define TRINKET_SHELL_CREDITS_H

#include "trinket/ompp/canvas.h"
#include "trinket/ompp/font.h"
#include "trinket/ompp/resource.h"

namespace Trinket {
namespace Shell {

class Scene;

/** Lay out and scroll credit text and images within a fixed viewport. */
class CreditText {
public:
	/** Configure the visible width and height of scrolling credits. */
	CreditText(int width, int height);
	/** Free the rendered credit-content pixels. */
	~CreditText() { _content.free(); }
	/** Render XML credit entries using the supplied font and image resources. */
	bool load(Common::SeekableReadStream &xml, const OM::NFNTFont &font, const OM::ResourceProvider &resources, const char *imagePattern);
	/** Begin scrolling after a millisecond delay. */
	void start(uint32 now, uint32 delay);
	/** Advance scrolling at a millisecond timestamp and report whether it remains active. */
	bool update(uint32 now);
	/** Pause scrolling and record the millisecond timestamp. */
	void pause(uint32 now);
	/** Resume scrolling while excluding the paused interval from timing. */
	void resume(uint32 now);
	/** Draw the visible credit content at a game-pixel position. */
	void draw(OM::Canvas &canvas, int x, int y) const;
	/** Return the vertical scrolling position in pixels. */
	int position() const { return _position; }
	/** Return the scrolling position at which the credits finish. */
	int finishPosition() const { return _finish; }
	/** Return the maximum number of pixels advanced in one update. */
	int maximumStep() const { return _maximumStep; }

private:
	/** XML element or attribute name for root Element/Section. */
	static constexpr const char *kXMLRootElementSection = "Root_Element/Section";
	/** XML element or attribute name for title. */
	static constexpr const char *kXMLTitle = "title";
	/** XML element or attribute name for root Element/Section/member. */
	static constexpr const char *kXMLRootElementSectionMember = "Root_Element/Section/member";
	/** XML element or attribute name for name. */
	static constexpr const char *kXMLName = "name";
	/** XML element or attribute name for root Element/case1. */
	static constexpr const char *kXMLRootElementCase1 = "Root_Element/case1";
	/** XML element or attribute name for root Element/case1/text. */
	static constexpr const char *kXMLRootElementCase1Text = "Root_Element/case1/text";
	/** XML element or attribute name for root Element/case1/imgpair. */
	static constexpr const char *kXMLRootElementCase1Imgpair = "Root_Element/case1/imgpair";

	struct Entry {
		/** Text displayed by this credit entry. */
		Common::String text;
		/** Optional authored entry name used for credit layout. */
		Common::String name;
		/** Vertical position of this entry in the scrolling content, in pixels. */
		int y = 0;
		/** Horizontal position of this entry in the scrolling content, in pixels. */
		int x = 0;
		/** Image resource ID, or minus one for a text-only entry. */
		int image = -1;
	};
	/** Pre-rendered surface containing the complete scrolling credits. */
	Graphics::Surface _content;
	/** Visible credit viewport width in pixels. */
	int _width;
	/** Visible credit viewport height in pixels. */
	int _height;
	/** Current vertical scroll position in pixels. */
	int _position = 0;
	/** Vertical scroll position that ends playback. */
	int _finish = 0;
	/** Maximum scrolling distance in pixels per update. */
	int _maximumStep = 24;
	/** Millisecond timestamp when the initial delay ends. */
	uint32 _deadline = 0;
	/** Millisecond timestamp used to calculate the next scrolling step. */
	uint32 _lastUpdate = 0;
	/** Millisecond timestamp when scrolling was paused. */
	uint32 _pauseTime = 0;
	/** Whether credit playback remains active. */
	bool _running = false;
	/** Whether scrolling is temporarily suspended. */
	bool _paused = false;
	/** Whether the initial delay has elapsed and scrolling has begun. */
	bool _started = false;
	/** Decode the supported markup within a credit text entry. */
	static Common::String textContent(const Common::String &source);
	/** Copy a cached credit image into the supplied composition surface. */
	static bool loadImage(Scene &images, const char *pattern, uint image, Graphics::Surface &surface);
	/** Prevent accidental copies of this stateful object. */
	CreditText(const CreditText &) = delete;
	/** Prevent accidental copies of this stateful object. */
	CreditText &operator=(const CreditText &) = delete;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
