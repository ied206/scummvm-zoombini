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

#ifndef TRINKET_RR1CLOUD9_CA1_CLOUD_FLOORS_H
#define TRINKET_RR1CLOUD9_CA1_CLOUD_FLOORS_H

#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** CA1: fill word gaps before a cloud brick reaches each floor's checkpoint. */
class CloudFloors {
public:
	explicit CloudFloors(PageScene &host);
	~CloudFloors();
	bool enter();
	void advance(uint32 time);
	void update(uint32 time);
	void draw(OM::Canvas &canvas);
	bool click(Common::Point point);
	bool event(const Common::Event &event);
	bool busy() const { return _phase != kTravelling || _correctRoute || 0 <= _dragged; }
	void pause(bool paused);
	void resume(uint32 elapsed) {
		_nextTick += elapsed;
		_idle += elapsed;
	}

private:
	enum Phase {
		kIntroduction,
		kTargetSpeech,
		kTravelling,
		kWrongWord,
		kWrongTarget,
		kCorrect,
		kReach,
		kDoneSpeech,
		kDescend,
		kRewardSpeech,
		kToss,
		kMarbleFlash,
		kNext,
	};
	struct Tile {
		Shell::Sprite *sprite = nullptr;
		Shell::TextObject *text = nullptr;
		Common::String value;
		Common::Point home;
		int slot = -1;
	};
	struct Slot {
		Tile tile;
		Common::String expected;
		int answer = -1;
		bool locked = false;
	};
	PageScene &_host;
	Shell::ResourcePaths _paths;
	OM::XMLParser _data;
	Common::SharedPtr<OM::NFNTFont> _font;
	Common::StringArray _pictures, _words;
	Common::Array<Slot> _slots[3];
	Common::Array<Tile> _answers;
	Common::String _targets[3];
	Shell::Sprite *_belts[3] = {}, *_masks[3] = {}, *_pieces[3] = {}, *_picturesAO[3] = {}, *_bricks[3] = {}, *_reaches[3] = {};
	Shell::Sprite *_brick = nullptr, *_highlight = nullptr, *_ladder = nullptr, *_marble = nullptr, *_descend = nullptr, *_toss = nullptr;
	Common::Array<Shell::Sprite *> _pausedSprites;
	Common::Point _points[5], _dragOffset;
	Graphics::Surface _cursors[5];
	Common::Point _mouse;
	int _cursor = -1;
	bool _paused = false;
	int _level = 1, _dataset = 1, _set = 1, _floor = 1, _state = 1, _brickCount = 0;
	int _from = 0, _to = 1, _end = 3, _dragged = -1, _mistakes = 0;
	int _oops = 0, _workerClicks = 0, _ladderClicks = 0, _mainClicks = 0, _paigeClicks = 0, _marbleSpeech = 0, _idleIndex = 0;
	bool _moving = false, _correctRoute = false, _ladderHere = true, _alternateOops = true;
	uint32 _nextTick = 0, _idle = 0;
	Phase _phase = kIntroduction;
	int number(const Common::String &key) const;
	int indexed(const char *key, int index) const;
	Shell::Sprite *sprite(const Common::String &key, int depth);
	bool worker(int pose);
	bool loadList(int resource, Common::StringArray &list);
	static int index(const Common::StringArray &list, const Common::String &word);
	static Common::Rect bounds(Shell::Sprite *sprite);
	bool createTile(Tile &tile, const Common::String &value, int length, Common::Point position, int depth);
	void deleteTile(Tile &tile);
	void drawTile(Tile &tile, OM::Canvas &canvas);
	void clearAnswers();
	void clearFloor(int floor);
	void say(const char *actor, const Common::String &key);
	void speech(const Common::String &actor, uint resource);
	void speeches(const Common::String &actors, const Common::String &resources, uint count);
	void updateCursor();
	void releaseCursor();
	void sayWord(const Common::String &word);
	void sound(const Common::String &key);
	void persist();
	void assess(bool correct);
	bool setup();
	void next();
	void initPath();
	void segment(int from, int to);
	void checkpoint();
	void reject();
	void fall();
	void finish();
	void drop();
	void home(int answer);
	void collect(const char *item);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
