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

#ifndef TRINKET_RR1CLOUD9_LA1_GUMBALL_H
#define TRINKET_RR1CLOUD9_LA1_GUMBALL_H

#include "trinket/ompp/sound.h"
#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** LA1 word ramps, solved-round assessment and collectible presentation. */
class Gumball {
public:
	explicit Gumball(PageScene &host);
	~Gumball();
	bool enter();
	void update(uint32 time);
	void draw(OM::Canvas &canvas);
	bool click(Common::Point point);
	void hover(Common::Point point);
	bool busy() const { return _phase != kReady; }
	void skipSpeech();
	void pause(bool paused);
	void resume(uint32 elapsed) { _deadline += elapsed; }

private:
	enum Phase {
		kIntro,
		kInstructions,
		kTrayOut,
		kTokenExplain,
		kTokenBlink,
		kReady,
		kRelease,
		kBump,
		kRolling,
		kAnswer,
		kTrayIn,
		kRewardRoll,
		kRewardSpeech,
		kPickup,
		kPutAway,
	};
	enum Reward {
		kNoReward,
		kWhistle,
		kMarble,
		kCandy,
	};
	struct Ramp {
		Shell::Sprite *sprite = nullptr;
		bool right = false, locked = false;
	};
	PageScene &_host;
	Shell::ResourcePaths _paths;
	Common::SharedPtr<OM::NFNTFont> _font;
	OM::XMLParser _data;
	Common::StringArray _wordsList, _partsList;
	Common::Array<Shell::TextObject *> _text;
	Common::Array<uint> _textSpeech;
	struct TextReading {
		uint first, end, mask;
	};
	Common::Array<TextReading> _readings;
	Common::Array<Shell::Sprite *> _sprites;
	Common::Array<Shell::Sprite *> _pausedSprites;
	Ramp _ramps[23];
	Shell::Sprite *_release = nullptr, *_token = nullptr, *_movingToken = nullptr;
	Shell::Sprite *_tray = nullptr, *_trayIn = nullptr, *_trayOut = nullptr;
	Shell::Sprite *_sparkle = nullptr, *_whistle = nullptr, *_marble = nullptr, *_candy = nullptr;
	Shell::Sprite *_rolls[4] = {}, *_blow = nullptr, *_eat = nullptr;
	Shell::Sprite *_correctTokens[3] = {};
	Common::ScopedPtr<OM::Sound> _rollingSound;
	Common::Array<uint> _path;
	Common::Array<uint> _correctChoices, _wrongChoices, _oopsChoices, _helpChoices, _yuckChoices;
	Common::String _words[3], _parts[3];
	int _order[3] = {
		1,
		2,
		3,
	};
	int _level = 1, _dataset = 0, _set = 1, _state = 1;
	int _correct = 0, _total = 0, _wrong = 0, _previousSlot = 0, _missLeft = 0, _missRight = 0;
	int _gumClicks = 0, _paigeClicks = 0, _rampClicks = 0, _wordClicks = 0, _candyClicks = 0, _marbleSpeech = 0;
	bool _rampClicked = false, _tokenClicked = false, _sentenceClicked = false;
	bool _imperfect = false, _answerCorrect = false, _showText = false;
	bool _finishInstructions = false;
	int _chosenSlot = 0;
	uint _segment = 0;
	uint _blinkStep = 0;
	uint32 _deadline = 0;
	Phase _phase = kIntro;
	Reward _reward = kNoReward, _pickup = kNoReward;
	int number(const Common::String &key) const;
	Shell::Sprite *sprite(const char *resource, const char *depth);
	void sound(const char *key);
	void say(const char *actor, const Common::String &key);
	void group(const char *actors, const Common::String &key);
	bool loadList(uint resource, Common::StringArray &list);
	static int wordIndex(const Common::StringArray &list, Common::String word);
	uint wordSpeech(const Common::String &word, bool part) const;
	void randomDataset();
	void setupRamps();
	void rampFrames();
	void buildPath();
	Common::Point tokenPoint(uint node) const;
	bool puzzle();
	void clearText();
	Shell::TextObject *text(const Common::String &value, int x, int y, bool centered, uint speech);
	void sentence(int slot = 0);
	void readSentence(int slot = 0);
	void readText(uint resource, uint mask, bool sound = false);
	uint highlightedText() const;
	void idle(uint32 time);
	void sliderOut();
	void releaseToken();
	void checkAnswer();
	void assess();
	void persist();
	void dispense();
	void nextPuzzle();
	void pickup(Reward item);
	void gumHelp();
	void paigeHelp();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
