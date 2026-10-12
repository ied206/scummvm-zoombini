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

#ifndef TRINKET_RR1CLOUD9_LA3_RAIN_GAME_H
#define TRINKET_RR1CLOUD9_LA3_RAIN_GAME_H

#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** LA3: sort falling rain gear into three moving attribute bins. */
class RainGame {
public:
	explicit RainGame(PageScene &host);
	~RainGame();
	bool enter();
	void advance(uint32 time);
	void update(uint32 time);
	void draw(OM::Canvas &canvas);
	void pause(bool paused, uint32 elapsed);
	bool event(const Common::Event &event);
	void click(Common::Point point);
	bool ending() const { return _phase == kEnding || _phase == kDone; }
	Common::String destination() const;

private:
	enum Phase {
		kGreeting,
		kBinIntro,
		kBinAttributes,
		kWaitDelay,
		kDelay,
		kBegin,
		kPlaying,
		kFeedback,
		kEnding,
		kDone,
	};
	struct Attributes {
		int shape = -1, color = -1, object = -1;
	};
	struct Answer {
		Shell::Sprite *sprite = nullptr;
		Common::String key;
		bool target = false;
	};
	PageScene &_host;
	OM::XMLParser _data;
	Shell::ResourcePaths _paths;
	Common::SharedPtr<OM::NFNTFont> _font;
	Shell::TextObject *_scoreText = nullptr;
	Shell::Sprite *_catcher = nullptr, *_feedback = nullptr, *_barrels = nullptr, *_highlight = nullptr;
	Shell::Sprite *_labels[6] = {}, *_flags[6][2] = {};
	Common::Point _catcherOrigin;
	Common::Array<Answer> _answers;
	Common::String _binKeys[3];
	Common::StringArray _words[3];
	Phase _phase = kGreeting;
	int _level = 1, _dataset = 1, _round = 0, _wins = 0, _score = 0;
	int _bin = 0, _catcherX = 0, _mouseX = 0, _ticks = 0, _emitted = -1;
	int _distractors = 0;
	bool _first = true;
	uint32 _moveDue = 0, _fallDue = 0, _delay = 0;
	int number(const Common::String &key) const;
	void say(const char *actor, const Common::String &key);
	Shell::Sprite *sprite(int id, int depth);
	bool loadWords(int id, Common::StringArray &words);
	static Attributes attribute(const Common::String &word);
	static Common::String key(const Attributes &first, const Attributes &second);
	static int label(const Attributes &attribute);
	bool setup();
	void persist();
	void start(uint32 time);
	void finish();
	void flags();
	void move(int delta);
	void position(int x);
	void attributes(int bin);
	void setupSpeech();
	void catchAnswer(uint index, int bin);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
