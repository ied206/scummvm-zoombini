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

#ifndef TRINKET_RR1CLOUD9_LA2_MUSIC_PATTERNS_H
#define TRINKET_RR1CLOUD9_LA2_MUSIC_PATTERNS_H

#include "trinket/ompp/sound.h"
#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** LA2: repeat Marvin's musical patterns using reusable collected marbles. */
class MusicPatterns {
public:
	explicit MusicPatterns(PageScene &host) : _host(host) {}
	~MusicPatterns();
	bool enter();
	void advance(uint32 time);
	void update(uint32 time);
	bool click(Common::Point point);
	bool event(const Common::Event &event);
	bool donate(uint item);
	void skipSpeech();
	bool busy() const { return _phase != kIdle || 0 <= _dragged; }
	void pause(bool paused);
	void resume(uint32 elapsed);

private:
	enum Phase {
		kEntryGreeting,
		kInventoryStart,
		kChestOpening,
		kBackpackOpening,
		kInventorySpeech,
		kChestClosing,
		kDonationSpeech,
		kIntroduction,
		kIdle,
		kPlayStart,
		kPlayback,
		kWrongSpeech,
		kSpin,
		kCongratulations,
		kDoor,
		kNext,
	};
	enum Result {
		kListen,
		kWrong,
		kCorrect,
		kLocked,
	};
	struct Marble {
		Shell::Sprite *sprite = nullptr;
		Common::Point home;
		int row = 0, column = -1, depth = 0;
		bool fixed = false;
	};
	PageScene &_host;
	OM::XMLParser _data;
	Common::Array<Marble> _marbles;
	Common::Array<Shell::Sprite *> _pausedSprites;
	Shell::Sprite *_lights[12] = {}, *_locks[5][8] = {}, *_doors[3] = {};
	Shell::Sprite *_button = nullptr, *_doorLights = nullptr, *_activate = nullptr, *_spin = nullptr;
	Shell::Sprite *_pillow = nullptr, *_jewel = nullptr, *_sparkle = nullptr;
	Shell::Sprite *_chestOpening = nullptr;
	OM::Sound *_notes[5] = {};
	bool _target[5][12] = {};
	int _bank[5] = {}, _needed[5] = {};
	int _level = 1, _dataset = 1, _given = 0, _count = 0, _state = 3, _mistakes = 0;
	int _dragged = -1, _column = -1, _lastColumn = 12, _soloRow = -1;
	int _playClicks = 0, _doorClicks = 0, _localClicks = 0, _paigeClicks = 0, _mainClicks = 0, _jewelClicks = 0;
	int _idleIndex = 0;
	Common::Point _dragOffset;
	uint32 _idle = 0;
	uint32 _closeStart = 0;
	bool _inventoryHelp = false, _playOnEntry = false, _haveMarbles = false;
	bool _donated = false, _firstDonation = false, _helpAfterPlayback = false;
	bool _pillowHere = true;
	Phase _phase = kEntryGreeting;
	Result _result = kListen;
	int number(const Common::String &key) const;
	int indexed(const char *key, int index) const;
	Shell::Sprite *sprite(const Common::String &key, int depth);
	void say(const char *actor, const Common::String &key);
	void speech(const Common::String &actor, uint resource);
	void speeches(const Common::String &actors, const Common::String &resources, uint count);
	void sound(const Common::String &key);
	void group(const char *prefix, int index, int count);
	bool setup();
	void clearMarbles();
	void home(int index);
	void drop();
	int resident(int row, int column) const;
	void lights();
	void doors();
	void persist();
	void assess();
	void play(Result result, int first = 0, int last = 12, int soloRow = -1);
	void playColumn();
	void endColumn();
	void finishPlayback();
	void accept();
	void hint();
	void closeInventory();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
