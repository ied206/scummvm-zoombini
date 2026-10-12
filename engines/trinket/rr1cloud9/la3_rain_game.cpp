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

#include "trinket/rr1cloud9/la3_rain_game.h"

#include "common/tokenizer.h"
#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

RainGame::RainGame(PageScene &host) : _host(host), _paths(host._vm.resources()) {}
RainGame::~RainGame() { delete _scoreText; }

int RainGame::number(const Common::String &key) const { return _host._page->number(key); }

void RainGame::say(const char *actor, const Common::String &key) { _host.say(actor, number(key)); }

Shell::Sprite *RainGame::sprite(int id, int depth) {
	return _host._scene.add(_host.characterResource(id, Shell::Character::Resources::kAnimation), depth);
}

bool RainGame::loadWords(int id, Common::StringArray &words) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_host._vm.resources().open(Common::String::format("r1la3/%d.txt", id)));
	if (!stream || 65536 < stream->size())
		return false;
	Common::String line;
	while (stream->pos() < stream->size()) {
		char c = stream->readByte();
		if (line.empty() && c == '*')
			return !words.empty();
		if (c == '\r') {
			uint comma = line.find(',');
			words.push_back(comma == Common::String::npos ? line : line.substr(0, comma));
			line.clear();
		} else if (c && c != '\n') {
			line += c;
		}
	}
	return false;
}

bool RainGame::enter() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_host._vm.resources().open("common/1018.nfnt"));
	_font.reset(stream ? OM::NFNTFont::load(*stream) : nullptr);
	if (!_font || !_data.loadStream(_host._vm.resources().open("r1la3.xml")) || !_data.parse())
		return false;
	for (int i = 0; i < 3; i++)
		if (!loadWords(27200 + i, _words[i]))
			return false;
	if (_words[0].size() != _words[1].size() || _words[0].size() != _words[2].size())
		return false;
	_scoreText = new Shell::TextObject(_paths, _font);
	_scoreText->create({Core::ScriptValue::fromText("0"), Core::ScriptValue(53), Core::ScriptValue(440), Core::ScriptValue(1)});
	_scoreText->set("color", {Core::ScriptValue(4095), Core::ScriptValue(4095)}, Core::ScriptValue(4095));
	_catcher = sprite(27080, 90);
	_feedback = sprite(27150, 91);
	_barrels = sprite(number("kBarrelsID"), number("kBarrelsZ"));
	_highlight = sprite(27170, 101);
	if (!_catcher || !_feedback || !_barrels || !_highlight)
		return false;
	_catcherOrigin = Common::Point(_catcher->x, _catcher->y);
	_catcher->visible = _feedback->visible = _highlight->visible = false;
	static const int offsets[] = {
		17,
		113,
		206,
	};
	for (int i = 0; i < 6; i++) {
		_labels[i] = sprite(27154, 92);
		if (!_labels[i])
			return false;
		_labels[i]->x += offsets[i % 3];
		_labels[i]->y += i < 3 ? 0 : 33;
		if (!_host._practice) {
			for (int j = 0; j < 2; j++) {
				_flags[i][j] = sprite(number(j ? "kFlag2ID" : "kFlag1ID"), number(j ? "kFlag2Z" : "kFlag1Z"));
				if (!_flags[i][j])
					return false;
				_flags[i][j]->x += number(Common::String::format("kFlagOffsetX.%d", i + 1));
				_flags[i][j]->y += number(Common::String::format("kFlagOffsetY.%d", i + 1));
			}
		}
	}
	_level = CLIP(_host._world.get("R1LA3Level"), 1, 3);
	_dataset = CLIP(_host._world.get("R1LA3Dataset"), 1, 15);
	_round = _host._practice ? 0 : _host._world.get("R1LA3Round");
	_wins = _host._practice ? 0 : CLIP(_host._world.get("R1LA3Wins"), 0, 6);
	flags();
	if (!setup())
		return false;
	if (_wins == 6) {
		_phase = kEnding;
		say("reader", "kLastSpchIDs.1");
		say("paige", "kLastSpchIDs.2");
		say("sam", "kLastSpchIDs.3");
	} else {
		setupSpeech();
	}
	return true;
}

RainGame::Attributes RainGame::attribute(const Common::String &word) {
	static const char *const shapes[] = {
		"trapezoid",
		"circle",
		"square",
		"triangle",
	};
	static const char *const colors[] = {
		"red",
		"yellow",
		"blue",
		"green",
	};
	static const char *const objects[] = {
		"boot",
		"umbrella",
		"hat",
		"coat",
	};
	Attributes value;
	for (int i = 0; i < 4; i++) {
		if (word == shapes[i])
			value.shape = i;
		if (word == colors[i])
			value.color = i;
		if (word == objects[i])
			value.object = i;
	}
	return value;
}

Common::String RainGame::key(const Attributes &first, const Attributes &second) {
	return Common::String::format("%d%d%d", (0 <= second.object ? second.object : first.object) + 1,
								  (0 <= second.color ? second.color : first.color) + 1, (0 <= second.shape ? second.shape : first.shape) + 1);
}

int RainGame::label(const Attributes &value) {
	if (0 <= value.shape)
		return value.shape + 9;
	if (0 <= value.color)
		return value.color + 5;
	static const int objects[] = {
		2,
		3,
		1,
		4,
	};
	return 0 <= value.object ? objects[value.object] : 0;
}

bool RainGame::setup() {
	Common::StringTokenizer tokens(_data.get(Common::String::format("DataSet/Level_%d/WordList_%d", _level, _dataset)));
	struct Group {
		int count;
		Attributes first, second;
	};
	Common::Array<Group> groups;
	int count = 0;
	while (!tokens.empty()) {
		Group group;
		group.count = atoi(tokens.nextToken().c_str());
		if (group.count < 1 || 24 < count + group.count || tokens.empty())
			return false;
		group.first = attribute(tokens.nextToken());
		if (1 < _level) {
			if (tokens.empty())
				return false;
			group.second = attribute(tokens.nextToken());
		}
		count += group.count;
		groups.push_back(group);
	}
	if (groups.size() != static_cast<uint>(_level == 1 ? 4 : 5))
		return false;
	for (auto &answer : _answers)
		_host._scene.remove(answer.sprite);
	_answers.clear();
	_answers.resize(count);
	position(0);
	_score = 0;
	_scoreText->set("text", {}, Core::ScriptValue::fromText("0"));
	for (uint i = 0; i < groups.size(); i++) {
		const Group &group = groups[i];
		Common::String identity = key(group.first, group.second);
		if (i < 3) {
			_binKeys[i] = identity;
			_labels[i]->setFrame(label(group.first));
			_labels[i]->visible = true;
			_labels[i + 3]->setFrame(MAX(1, label(group.second)));
			_labels[i + 3]->visible = 1 < _level;
		}
		for (int n = 0; n < group.count; n++) {
			int slot = _host._vm.rnd().draw() % (count - 1);
			while (_answers[slot].sprite)
				slot = (slot + 1) % count;
			int shape = 0 <= group.first.shape ? group.first.shape : group.second.shape;
			int color = 0 <= group.first.color ? group.first.color : group.second.color;
			int object = 0 <= group.first.object ? group.first.object : group.second.object;
			if (shape < 0)
				shape = _host._vm.rnd().draw() % 4;
			if (color < 0)
				color = _host._vm.rnd().draw() % 4;
			if (object < 0)
				object = _host._vm.rnd().draw() % 4;
			Answer &answer = _answers[slot];
			answer.sprite = sprite(27158, 100);
			if (!answer.sprite)
				return false;
			answer.sprite->setFrame(shape + 4 * (color + 4 * object) + 1);
			answer.sprite->y = -answer.sprite->height();
			answer.sprite->visible = false;
			answer.key = identity;
			answer.target = i < 3;
		}
	}
	persist();
	return true;
}

void RainGame::persist() {
	if (_host._practice)
		return;
	_host._world.set("R1LA3Level", _level);
	_host._world.set("R1LA3Dataset", _dataset);
	_host._world.set("R1LA3Round", _round);
	_host._world.set("R1LA3Wins", _wins);
}

void RainGame::flags() {
	for (int i = 0; i < 6; i++)
		for (int j = 0; j < 2; j++)
			if (_flags[i][j]) {
				_flags[i][j]->visible = (i < _wins) == (j == 1);
				if (_flags[i][j]->visible)
					_flags[i][j]->startAnimation(0);
				else
					_flags[i][j]->stopAnimation();
			}
}

void RainGame::attributes(int bin) {
	for (uint i = 0; i < _words[0].size(); i++)
		if (_words[0][i] == _binKeys[bin]) {
			for (int j = 1; j < 3; j++) {
				int resource = atoi(_words[j][i].c_str());
				if (0 < resource)
					_host.say("paige", resource);
			}
			return;
		}
}

void RainGame::setupSpeech() {
	if (_first) {
		say("reader", "kEnterSpchIDs.1");
		say("reader", "kEnterSpchIDs.2");
		say("sam", "kEnterSpchIDs.3");
		say("reader", "kEnterSpchIDs.4");
	}
	say("reader", _first ? "kSetupSpchIDs.1" : "kSetupSpchIDs.2");
	_bin = 0;
	_phase = kGreeting;
}

void RainGame::update(uint32 time) {
	if (_host._nextSpeech < _host._speech.size())
		return;
	if (_phase == kGreeting || _phase == kBinAttributes) {
		if (_bin < 3) {
			_highlight->setFrame(_bin + 1);
			_highlight->visible = true;
			say("paige", Common::String::format("kSetupSpchIDs.%d", _bin + 3));
			_phase = kBinIntro;
		} else {
			_highlight->visible = false;
			_delay = time;
			_phase = kDelay;
		}
	} else if (_phase == kBinIntro) {
		_highlight->visible = false;
		attributes(_bin);
		_bin += 1;
		_phase = kBinAttributes;
	} else if (_phase == kWaitDelay) {
		_delay = time;
		_phase = kDelay;
	} else if (_phase == kDelay && 3000 <= time - _delay) {
		say("reader", _first ? "kBeginSpchIDs.1" : "kBeginSpchIDs.2");
		_first = false;
		_phase = kBegin;
	} else if (_phase == kBegin) {
		start(time);
	} else if (_phase == kFeedback) {
		if (setup())
			setupSpeech();
	} else if (_phase == kEnding) {
		_phase = kDone;
	}
}

void RainGame::start(uint32 time) {
	position(0);
	_catcher->visible = true;
	_feedback->visible = _barrels->visible = false;
	_host._actors["reader"]->setVisible(false);
	_host._actors["sam"]->setVisible(false);
	_moveDue = time + 25;
	_fallDue = time + 50;
	_ticks = _score = 0;
	_emitted = -1;
	_phase = kPlaying;
	_host.music(number("kBackgroundMusicID2"));
}

void RainGame::position(int x) {
	int delta = x - _catcherX;
	_catcherX = x;
	_catcher->x += delta;
	_feedback->x += delta;
	for (int i = 0; i < 6; i++)
		_labels[i]->x += delta;
}

void RainGame::move(int delta) {
	int x = CLIP(_catcherX + delta, -325, 400);
	if (x == _catcherX)
		return;
	position(x);
	if (!_catcher->isRunning()) {
		_catcher->startAnimation(1);
		_host.playSounds({27080});
	}
}

void RainGame::advance(uint32 time) {
	if (_phase != kPlaying)
		return;
	if (static_cast<int32>(time - _moveDue) >= 0) {
		_moveDue += 25;
		if (0 < _mouseX) {
			int distance = _mouseX - _catcher->width() / 2 - _catcherOrigin.x - _catcherX - 20;
			if (10 < distance)
				move(10);
			else if (distance < -10)
				move(-10);
		}
	}
	if (static_cast<int32>(time - _fallDue) < 0)
		return;
	_fallDue += 50;
	_ticks += 1;
	if (_ticks % 40 == 0) {
		_emitted += 1;
		if (_emitted < static_cast<int>(_answers.size())) {
			auto *answer = _answers[_emitted].sprite;
			answer->x = 100 + _host._vm.rnd().draw() % 400;
			answer->y = -answer->height();
			answer->visible = true;
			_host.playSounds({27158});
		}
		_feedback->visible = false;
	}
	for (uint i = 0; i < _answers.size(); i++) {
		auto *answer = _answers[i].sprite;
		if (!answer->visible)
			continue;
		if (485 <= answer->y + answer->height() + 4) {
			if (i + 1 == _answers.size()) {
				finish();
				return;
			}
		} else {
			answer->y += 4;
		}
	}
	static const int binX[] = {
		230,
		322,
		418,
	};
	for (uint i = 0; i < _answers.size(); i++) {
		auto *answer = _answers[i].sprite;
		if (!answer->visible)
			continue;
		Common::Rect bounds(answer->x, answer->y, answer->x + answer->width(), answer->y + answer->height());
		int chosen = -1;
		for (int bin = 0; bin < 3; bin++)
			if (bounds.intersects(Common::Rect(binX[bin] + _catcherX, 322, binX[bin] + _catcherX + 50, 342))) {
				if (chosen < 0 || _answers[i].key == _binKeys[bin])
					chosen = bin;
			}
		if (0 <= chosen) {
			catchAnswer(i, chosen);
			if (i + 1 == _answers.size()) {
				finish();
				return;
			}
		}
	}
}

void RainGame::catchAnswer(uint index, int bin) {
	Answer &answer = _answers[index];
	bool correct = answer.key == _binKeys[bin];
	if (correct) {
		answer.sprite->visible = false;
		_score += 1;
		_scoreText->set("text", {}, Core::ScriptValue::fromText(Common::String::format("%d", _score)));
	} else {
		answer.sprite->y = 485 - answer.sprite->height();
		if (!answer.target && index + 1 < _answers.size()) {
			_distractors += 1;
			if (_distractors == 1) {
				_host.interrupt();
				say("paige", "kDistracterSpchID");
			}
		}
	}
	_host.playSounds({static_cast<uint16>(correct ? 27150 : 27152)});
	_feedback->setFrame(bin + 1);
	_feedback->visible = true;
}

void RainGame::finish() {
	_host.interrupt();
	position(0);
	_catcher->visible = _feedback->visible = false;
	_catcher->stopAnimation();
	_barrels->visible = true;
	_host._actors["reader"]->setVisible(true);
	_host._actors["sam"]->setVisible(true);
	_host.music(number("kBackgroundMusicID"));
	say("sam", "kOverSpchID");
	bool correct = number("kMinCorrect") <= _score;
	if (correct) {
		if (!_host._practice)
			_wins += 1;
		flags();
		if (_wins < 6) {
			int feedback = 1;
			if (_score < 14)
				feedback = 3;
			else if (_score < 16)
				feedback = 2;
			say("reader", Common::String::format("kMoreSpchIDs.%d", feedback));
		} else {
			say("reader", "kLastSpchIDs.1");
			say("paige", "kLastSpchIDs.2");
			say("sam", "kLastSpchIDs.3");
		}
	} else {
		say("reader", "kLessSpchIDs.1");
		int choice = _host._vm.rnd().number(1, 4);
		say("paige", Common::String::format("kLessSpchIDs.%d", choice * 2));
		say("sam", Common::String::format("kLessSpchIDs.%d", choice * 2 + 1));
	}
	if (!_host._practice) {
		World &world = _host._world;
		int outcome = correct ? 1 : 3;
		int streak = world.get("R1LA3StreakKind") == outcome ? world.get("R1LA3Streak") + 1 : 1;
		world.recordAssessment("R1LA3", _level, outcome, 2, 2, streak);
		world.set("R1LA3StreakKind", outcome);
		world.set("R1LA3Streak", streak == 2 ? 0 : streak);
		if (2 <= streak && !world.get("R1LA3FixedLevel"))
			_level = CLIP(_level + (correct ? 1 : -1), 1, 3);
	}
	_phase = _wins == 6 ? kEnding : kFeedback;
	if (_wins < 6) {
		int choice = _host._vm.rnd().number(1, 3);
		say(choice == 2 ? "sam" : "reader", Common::String::format("kDevSpchIDs.%d", choice));
		say("sam", "kSoonSpchID");
		_round += 1;
		_dataset = 15 < _round ? _host._vm.rnd().number(1, 15) : _dataset % 15 + 1;
	}
	persist();
}

bool RainGame::event(const Common::Event &event) {
	if (event.type == Common::EVENT_MOUSEMOVE && 0 < event.mouse.x)
		_mouseX = event.mouse.x;
	if (_phase == kPlaying && event.type == Common::EVENT_KEYDOWN &&
		(event.kbd.keycode == Common::KEYCODE_LEFT || event.kbd.keycode == Common::KEYCODE_RIGHT)) {
		_mouseX = 0;
		move(event.kbd.keycode == Common::KEYCODE_LEFT ? -10 : 10);
		return true;
	}
	return false;
}

void RainGame::click(Common::Point point) {
	if (ending())
		return;
	bool paige = _host._actors["paige"]->bounds().contains(point);
	if (_phase == kPlaying) {
		if (paige) {
			_host.interrupt();
			for (int i = 0; i < 3; i++) {
				say("paige", Common::String::format("kSetupSpchIDs.%d", i + 3));
				attributes(i);
			}
		}
		return;
	}
	_host.interrupt();
	if (_phase == kFeedback && !setup())
		return;
	_highlight->visible = false;
	if (paige) {
		say("paige", Common::String::format("kHelpSpchIDs.%d", _host._vm.rnd().number(1, 3)));
	} else if (_host._actors["reader"]->bounds().contains(point) || _host._actors["sam"]->bounds().contains(point)) {
		int choice = _host._vm.rnd().number(1, 2);
		say(choice == 1 ? "reader" : "sam", Common::String::format("kMainSpchIDs.%d.1", choice));
		say(choice == 1 ? "sam" : "reader", Common::String::format("kMainSpchIDs.%d.2", choice));
	}
	_phase = kWaitDelay;
}

void RainGame::pause(bool paused, uint32 elapsed) {
	_catcher->pauseAnimation(paused);
	for (auto &flag : _flags)
		for (auto *sprite : flag)
			if (sprite)
				sprite->pauseAnimation(paused);
	if (paused) {
		position(0);
	} else {
		_moveDue = _host._time + elapsed + 25;
		_fallDue = _host._time + elapsed + 50;
		_delay += elapsed;
	}
}

void RainGame::draw(OM::Canvas &canvas) {
	_host._scene.drawBackground(canvas);
	_host._scene.drawSprites(canvas, INT_MIN, 102);
	_scoreText->draw(canvas);
	_host._scene.drawSprites(canvas, 103, INT_MAX);
}

Common::String RainGame::destination() const { return _phase == kDone ? "r1end" : Common::String(); }

} // namespace RR1Cloud9
} // namespace Trinket
