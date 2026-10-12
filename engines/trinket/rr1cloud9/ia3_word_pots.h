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

#ifndef TRINKET_RR1CLOUD9_IA3_WORD_POTS_H
#define TRINKET_RR1CLOUD9_IA3_WORD_POTS_H

#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** IA3: exchange word parts in three pots and pack the requested word. */
class WordPots {
public:
	explicit WordPots(PageScene &host);
	~WordPots();
	bool enter();
	void advance(uint32 time);
	void update(uint32 time);
	void draw(OM::Canvas &canvas);
	bool click(Common::Point point);
	bool event(const Common::Event &event);
	bool busy() const { return _phase != kReady || _dragging; }
	void skipSpeech();
	void pause(bool paused);
	void resume(uint32 elapsed) {
		_start += elapsed;
		_idle += elapsed;
	}

private:
	enum Phase {
		kReady,
		kIntroduction,
		kUnpacking,
		kGrowing,
		kCorrect,
		kTargetOut,
		kBatchSpeech,
		kRedWagonOut,
		kRedWagonSpeech,
		kMarbleSpeech,
		kMarbleFlash,
		kWagonOut,
		kTargetIn,
		kWagonIn,
		kNextSpeech,
	};
	PageScene &_host;
	Shell::ResourcePaths _paths;
	OM::XMLParser _data;
	Common::SharedPtr<OM::NFNTFont> _font, _potFont;
	Common::StringArray _words, _types, _pictures;
	Common::String _word, _targetWord, _parts[2][3];
	bool _alternate[3] = {};
	Shell::Sprite *_pots[3] = {}, *_user = nullptr, *_target = nullptr, *_box = nullptr, *_mask = nullptr;
	Shell::Sprite *_grow = nullptr, *_shrink = nullptr, *_arrow = nullptr, *_packages = nullptr;
	Shell::Sprite *_redWagon = nullptr, *_redMask = nullptr, *_oldWagon = nullptr, *_oldMask = nullptr, *_marble = nullptr;
	Shell::TextObject *_userText = nullptr, *_targetText = nullptr, *_oldText = nullptr, *_newText = nullptr;
	Common::Array<Shell::Sprite *> _pausedSprites;
	Common::Point _home, _dragOffset, _growHome, _shrinkHome, _arrowHome;
	Common::Point _boxHome, _maskHome, _wagonHome, _oldWagonHome, _oldMaskHome, _packagesHome;
	Common::Point _answerSize;
	static constexpr int kAnswerGrowOffset = -140;
	int _level = 1, _dataset = 1, _set = 1, _state = 1, _round = 1;
	int _transformations = 0, _attempts = 1, _wrongSpeech = 0, _help = 0, _pot = -1;
	int _completedRound = 0, _idleIndex = 0, _nextSpeechIndex = 1;
	bool _dragging = false, _showWords = false, _wagonHere = true, _showMarble = false;
	uint32 _start = 0, _idle = 0, _idleDelay = 5000;
	Phase _phase = kReady;
	int number(const Common::String &key) const;
	int indexed(const char *key, int index) const;
	Shell::Sprite *sprite(const char *key, int depth);
	Shell::TextObject *text(bool pot);
	static void setText(Shell::TextObject *text, const Common::String &value, int x, int y);
	static Common::Rect bounds(Shell::Sprite *sprite);
	Common::Rect answerBounds() const;
	static Common::Point frameOffset(Shell::Sprite *sprite);
	static void placeAnswer(Shell::Sprite *sprite, Common::Point origin);
	bool loadList(int resource, Common::StringArray &list);
	static int index(const Common::StringArray &list, const Common::String &word);
	int picture(const Common::String &word) const;
	void say(const char *actor, const Common::String &key);
	void group(const char *actors, const Common::String &key, int first = 1);
	void sayWord(const Common::String &word);
	void sound(const char *key);
	void persist();
	bool setup();
	void present();
	void targetPosition(int offset);
	void wagonPosition(int offset);
	void unpack();
	void transform(int pot);
	void grown();
	void drop();
	void solved();
	void next();
	void prepareNext();
	void announceNext();
	void help();
	void bloomHelp();
	void hover(Common::Point point);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
