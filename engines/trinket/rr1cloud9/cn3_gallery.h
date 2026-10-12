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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_RR1CLOUD9_CN3_GALLERY_H
#define TRINKET_RR1CLOUD9_CN3_GALLERY_H

#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** CN3 gallery, flipbook preview and artist story transitions. */
class Gallery {
public:
	explicit Gallery(PageScene &host) : _host(host) {}
	~Gallery();
	bool enter();
	void advance(uint32 time);
	void update(uint32 time);
	void draw(OM::Canvas &canvas);
	bool click(Common::Point point);
	bool event(const Common::Event &event);
	void pause(bool paused);
	void resume(uint32 time);
	bool busy() const { return _phase != kIdle || _destination != 0; }
	bool wantsMovie() const { return _phase == kMovie; }
	int destination() const { return _destination; }

private:
	enum Phase {
		kIdle,
		kIntroduction,
		kBeforeMovie,
		kMovie,
		kOfferSpeech,
		kTakeOff,
		kCollectSpeech,
	};
	PageScene &_host;
	Shell::Sprite *_frames[5] = {}, *_easel = nullptr, *_hat = nullptr, *_takeOff = nullptr;
	Common::Array<Graphics::Surface *> _samples[3];
	Graphics::Surface _edited[5];
	int _editedFrame[5] = {};
	Common::Point _positions[5];
	int _maximum[5] = {}, _painting = 1, _frame = 1, _hover = 0, _state = 1;
	int _hatClicks = 0, _idleIndex = 0, _destination = 0;
	uint32 _nextFrame = 0, _idle = 0;
	bool _clockStarted = false, _takeOffPaused = false;
	Phase _phase = kIdle;
	int number(const Common::String &key) const;
	int indexed(const char *key, int index) const;
	void say(const char *actor, const Common::String &key);
	void group(const char *key, const char *actors);
	bool artist(int pose);
	void artistSpeech();
	void mainSpeech();
	void hatClick();
	void choose(int painting);
	void highlight();
	void setState(int state);
	void collect();
	static bool loadImage(Common::SeekableReadStream &stream, Graphics::Surface &image);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
