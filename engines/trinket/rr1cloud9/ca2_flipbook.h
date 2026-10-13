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

#ifndef TRINKET_RR1CLOUD9_CA2_FLIPBOOK_H
#define TRINKET_RR1CLOUD9_CA2_FLIPBOOK_H

#include "trinket/ompp/font.h"
#include "trinket/rr1cloud9/artwork.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** CA2 drawing, onion skin and flipbook playback. */
class Flipbook {
public:
	explicit Flipbook(PageScene &host) : _host(host) {}
	~Flipbook();
	bool enter();
	bool hasCustomCursor() const { return 0 <= _cursor || _cursorHidden; }
	void checkpoint();
	void advance(uint32 time);
	void update(uint32 time);
	void draw(OM::Canvas &canvas);
	bool click(Common::Point point);
	bool event(const Common::Event &event);
	bool busy() const { return _dragging || _turn != 0; }
	void pause(bool paused);
	void resume(uint32 time);

private:
	enum Tool {
		kPaint,
		kFill,
		kErase,
		kStamp,
	};
	PageScene &_host;
	Artwork _practiceArtwork;
	Common::Array<uint16> _pixels, _onion, _undo;
	Graphics::Surface _image, _stampImage;
	Graphics::Surface _colored[8];
	Graphics::Surface _cursors[4];
	Common::Point _cursorHotspots[4];
	int _cursor = -1;
	bool _paused = false, _cursorHidden = false;
	Common::ScopedPtr<OM::NFNTFont> _font;
	Shell::Sprite *_colors[12] = {}, *_stamps[6] = {}, *_tools[3] = {};
	Shell::Sprite *_next = nullptr, *_previous = nullptr, *_play = nullptr, *_bomb = nullptr, *_undoButton = nullptr, *_print = nullptr, *_flip = nullptr;
	Common::Point _last, _mouse;
	Shell::Sprite *_pressed = nullptr;
	int _book = 4, _frame = 1, _maximum = 1, _color = 0, _stamp = -1, _turn = 0, _state = 2;
	Tool _tool = kPaint, _previousTool = kPaint;
	bool _dirty = false, _painted = false, _dragging = false, _playing = false, _ready = false, _helpPending = true;
	uint32 _nextFrame = 0, _idle = 0;
	uint32 _helpStart = 0, _pauseStart = 0;
	uint _helpIndex = 0, _helpPhase = 0;
	bool _idleWaiting = false;
	int _mainClicks = 0;
	Artwork &artwork();
	int number(const Common::String &key) const;
	void used(const char *key);
	void say(const char *actor, const Common::String &key);
	void group(const Common::String &key, const char *actors);
	bool loadFrame();
	void buttons();
	void cancelHelp();
	void updateHelp(uint32 time);
	void updateCursor();
	void releaseCursor();
	void turn(int direction);
	void stop();
	void selectTool(Tool tool);
	bool stampImage();
	bool coloredImage(Shell::Sprite &sprite, int resource, Graphics::Surface &image) const;
	uint16 color() const;
	void stroke(Common::Point from, Common::Point to);
	void fill(Common::Point point);
	void stamp(Common::Point point);
	void paige();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
