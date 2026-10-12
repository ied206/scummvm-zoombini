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

/** RCreditText layout and scrolling; the owner handles music and completion links. */
class CreditText {
public:
	CreditText(int width, int height);
	~CreditText() { _content.free(); }
	bool load(Common::SeekableReadStream &xml, const OM::NFNTFont &font, const OM::ResourceProvider &resources, const char *imagePattern);
	void start(uint32 now, uint32 delay);
	bool update(uint32 now);
	void pause(uint32 now);
	void resume(uint32 now);
	void draw(OM::Canvas &canvas, int x, int y) const;
	int position() const { return _position; }
	int finishPosition() const { return _finish; }
	int maximumStep() const { return _maximumStep; }

private:
	struct Entry {
		Common::String text, name;
		int y = 0, x = 0, image = -1;
	};
	Graphics::Surface _content;
	int _width, _height;
	int _position = 0, _finish = 0, _maximumStep = 24;
	uint32 _deadline = 0, _lastUpdate = 0, _pauseTime = 0;
	bool _running = false, _paused = false, _started = false;
	static Common::String textContent(const Common::String &source);
	static bool loadImage(const OM::ResourceProvider &resources, const char *pattern, uint image, Graphics::Surface &surface);
	CreditText(const CreditText &) = delete;
	CreditText &operator=(const CreditText &) = delete;
};

} // End of namespace Shell
} // End of namespace Trinket

#endif
