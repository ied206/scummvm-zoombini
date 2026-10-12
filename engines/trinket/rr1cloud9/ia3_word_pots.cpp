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

#include "trinket/rr1cloud9/ia3_word_pots.h"

#include "common/tokenizer.h"
#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

WordPots::WordPots(PageScene &host) : _host(host), _paths(host._vm.resources()) {}
WordPots::~WordPots() {
	delete _userText;
	delete _targetText;
	delete _oldText;
	delete _newText;
}

int WordPots::number(const Common::String &key) const { return _host._page->number(key); }

int WordPots::indexed(const char *key, int i) const { return number(Common::String::format("%s.%d", key, i)); }

void WordPots::say(const char *actor, const Common::String &key) {
	_host.delaySpeech(1);
	_host.say(actor, number(key));
	_host.delaySpeech(1);
}

void WordPots::sound(const char *key) { _host.playSounds({static_cast<uint16>(number(key))}); }

void WordPots::group(const char *actors, const Common::String &key, int first) {
	for (int i = 0; actors[i]; i++) {
		const char *actor = actors[i] == 'r' ? "reader" : "bloom";
		if (actors[i] == 's')
			actor = "sam";
		say(actor, Common::String::format("%s.%d", key.c_str(), first + i));
	}
}

Shell::Sprite *WordPots::sprite(const char *key, int depth) {
	return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), depth);
}

Shell::TextObject *WordPots::text(bool pot) {
	Shell::TextObject *result = new Shell::TextObject(_paths, pot ? _potFont : _font);
	result->create({Core::ScriptValue::fromText(" "), Core::ScriptValue(0), Core::ScriptValue(0), Core::ScriptValue(1)});
	result->set("center", {}, Core::ScriptValue(1));
	result->set("color", {Core::ScriptValue(pot ? 65535 : 6317), Core::ScriptValue(pot ? 63487 : 4212)},
				Core::ScriptValue(pot ? 59647 : 2106));
	return result;
}

void WordPots::setText(Shell::TextObject *text, const Common::String &value, int x, int y) {
	text->set("text", {}, Core::ScriptValue::fromText(value));
	text->set("x", {}, Core::ScriptValue(x));
	text->set("y", {}, Core::ScriptValue(y));
}

Common::Rect WordPots::bounds(Shell::Sprite *sprite) {
	return Common::Rect(sprite->x, sprite->y, sprite->x + sprite->width(), sprite->y + sprite->height());
}

Common::Point WordPots::frameOffset(Shell::Sprite *sprite) {
	uint frame = 0;
	for (const auto &step : sprite->asset->sequence(0)) {
		if (step.picture == -1)
			break;
		if (0 <= step.picture) {
			frame += 1;
			if (frame == sprite->getFrame())
				return Common::Point(step.x, step.y);
		}
	}
	return Common::Point();
}

void WordPots::placeAnswer(Shell::Sprite *sprite, Common::Point origin) {
	Common::Point offset = frameOffset(sprite);
	sprite->x = origin.x + offset.x;
	sprite->y = origin.y + offset.y;
}

bool WordPots::loadList(int resource, Common::StringArray &list) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_host._vm.resources().open(Common::String::format("r1ia3/%d.txt", resource)));
	if (!stream || 65536 < stream->size())
		return false;
	Common::String line;
	while (stream->pos() < stream->size()) {
		char c = stream->readByte();
		if (line.empty() && c == '*')
			return !list.empty();
		if (c == '\r') {
			uint comma = line.find(',');
			list.push_back(comma == Common::String::npos ? line : line.substr(0, comma));
			line.clear();
		} else if (c && c != '\n') {
			line += c;
		}
	}
	return false;
}

int WordPots::index(const Common::StringArray &list, const Common::String &word) {
	for (uint i = 0; i < list.size(); i++)
		if (word.equalsIgnoreCase(list[i]))
			return i;
	return -1;
}

int WordPots::picture(const Common::String &word) const {
	int at = index(_words, word);
	if (at < 0)
		return 0;
	if (_types[at] == "p") {
		int frame = index(_pictures, word);
		return frame < 0 ? 0 : frame + 3;
	}
	return _types[at] == "?" ? 2 : 1;
}

void WordPots::sayWord(const Common::String &word) {
	int at = index(_words, word);
	if (0 <= at) {
		_host.delaySpeech(1);
		_host.say("bloom", number("kWordNameSpchID") + at);
		_host.delaySpeech(1);
	}
}

bool WordPots::enter() {
	Common::ScopedPtr<Common::SeekableReadStream> font(_host._vm.resources().open("common/1016.nfnt"));
	Common::ScopedPtr<Common::SeekableReadStream> potFont(_host._vm.resources().open("common/1014.nfnt"));
	_font.reset(font ? OM::NFNTFont::load(*font) : nullptr);
	_potFont.reset(potFont ? OM::NFNTFont::load(*potFont) : nullptr);
	if (!_font || !_potFont || !_data.loadStream(_host._vm.resources().open("r1ia3.xml")) || !_data.parse() ||
		!loadList(number("TEXT_IA3WORDS"), _words) || !loadList(number("TEXT_IA3WORDTYPES"), _types) ||
		!loadList(number("TEXT_IA3PICTUREWORDS"), _pictures) || _words.size() != _types.size() || !_host.actor("bloom", "kBloom"))
		return false;
	_level = CLIP(_host._world.get("R1IA3Level"), 1, 3);
	_dataset = CLIP(_host._world.get("R1IA3Dataset"), 1, indexed("kMaxDataSets", _level));
	_set = _host._world.get("R1IA3Set");
	if (_set < 1 || 3 < _set)
		_set = _host._vm.rnd().number(1, 3);
	_state = CLIP(_host._world.get("R1IA3State"), 1, 4);
	if (_state == 3)
		_state = 2;
	_round = MAX(1, _host._world.get("R1IA3Round"));
	_wagonHere = !_host._world.get("R1IA3State") || _host._world.get("R1IA3WagonHere");
	_showMarble = _host._world.get("R1IA3ShowMarble");
	if (_host._practice) {
		_state = 4;
		_round = 5;
		_wagonHere = _showMarble = false;
		if (_host._chester)
			_host._chester->visible = false;
	}
	_user = sprite("kItemsID", 100);
	_target = sprite("kItemsID", 92);
	_box = sprite("kTargetBoxID", 91);
	_mask = sprite("kTargetMaskID", 95);
	_grow = sprite("kGrowID", 2);
	_shrink = sprite("kShrinkID", 2);
	_arrow = sprite("kArrowID", 4);
	_packages = sprite("kPackagesID", 7);
	_redWagon = sprite("kRedWagonID", 6);
	_redMask = sprite("kRedWagonMaskID", 8);
	_oldWagon = sprite("kOldWagonID", 6);
	_oldMask = sprite("kOldWagonMaskID", 8);
	_marble = sprite("kMarbleID", 10);
	for (int i = 0; i < 3; i++) {
		_pots[i] = sprite(Common::String::format("kPotIDs.%d", i + 1).c_str(), 3);
		if (!_pots[i])
			return false;
	}
	if (!_user || !_target || !_box || !_mask || !_grow || !_shrink || !_arrow || !_packages ||
		!_redWagon || !_redMask || !_oldWagon || !_oldMask || !_marble)
		return false;
	_userText = text(false);
	_targetText = text(false);
	_oldText = text(true);
	_newText = text(true);
	_user->setFrame(1);
	_answerSize = Common::Point(_user->width(), _user->height());
	_growHome = Common::Point(_grow->x, _grow->y);
	_shrinkHome = Common::Point(_shrink->x, _shrink->y);
	_arrowHome = Common::Point(_arrow->x, _arrow->y);
	_boxHome = Common::Point(_box->x, _box->y);
	_maskHome = Common::Point(_mask->x + 1, _mask->y - 1);
	_wagonHome = Common::Point(_redWagon->x, _redWagon->y);
	_oldWagonHome = Common::Point(_oldWagon->x, _oldWagon->y);
	_oldMaskHome = Common::Point(_oldMask->x, _oldMask->y);
	_packagesHome = Common::Point(_packages->x, _packages->y);
	_marble->x = number("kMarbleX");
	_marble->y = number("kMarbleY");
	if (!setup())
		return false;
	if (_host._practice) {
		for (const char *key : {"kInstrSpchIDs.6", "kInstrSpchIDs.8", "kInstrSpchIDs.5.2", "kInstrSpchIDs.7"})
			say("bloom", key);
	} else if (_state == 1) {
		say("reader", "kEnterSpchIDs");
		_phase = kIntroduction;
	} else {
		int choice = _host._vm.rnd().number(1, 3);
		if (choice == 3)
			group("rb", "kReEnterSpchIDs.3");
		else
			say("bloom", Common::String::format("kReEnterSpchIDs.%d", choice));
	}
	if (_state == 4) {
		say("bloom", "kNextSpchIDs.1");
		sayWord(_targetWord);
	} else if (1 < _state) {
		bloomHelp();
	}
	return true;
}

void WordPots::persist() {
	if (_host._practice)
		return;
	_host._world.set("R1IA3State", _state);
	_host._world.set("R1IA3Level", _level);
	_host._world.set("R1IA3Dataset", _dataset);
	_host._world.set("R1IA3Set", _set);
	_host._world.set("R1IA3Round", _round);
	_host._world.set("R1IA3WagonHere", _wagonHere);
	_host._world.set("R1IA3ShowMarble", _showMarble);
}

bool WordPots::setup() {
	Common::StringTokenizer tokens(_data.get(Common::String::format("DataSet/Level_%d/WordList_%d/Set_%d", _level, _dataset, _set)));
	Common::StringArray words;
	while (!tokens.empty())
		words.push_back(tokens.nextToken());
	if (words.size() != 8 || !picture(words[0]) || !picture(words[1]))
		return false;
	for (int i = 0; i < 3; i++) {
		_parts[0][i] = words[2 + i];
		_parts[1][i] = words[5 + i];
		_alternate[i] = false;
	}
	_word = words[0];
	_targetWord = words[1];
	_user->setFrame(picture(_word));
	_target->setFrame(picture(_targetWord));
	int package = (_round - 1) % 4 + 1;
	_home = Common::Point(indexed("kPackageX", package), indexed("kPackageY", package));
	placeAnswer(_user, _home);
	_transformations = 0;
	_attempts = 1;
	_pot = -1;
	_dragging = _showWords = false;
	_grow->visible = _shrink->visible = _arrow->visible = false;
	_phase = kReady;
	_idle = _host._time;
	targetPosition(0);
	wagonPosition(0);
	present();
	persist();
	return true;
}

void WordPots::present() {
	int package = (_round - 1) % 4 + 1;
	_packages->setFrame(_state == 1 ? package : package + 1);
	_packages->visible = _state == 1 || package < 4;
	_user->visible = 1 < _state;
	_target->visible = _state == 4;
	_mask->visible = _state == 4 && _level == 3;
	_redWagon->visible = _wagonHere;
	_redWagon->x = _wagonHome.x - (4 < _round ? number("kRedWagonMoveOffset") : 0);
	_redMask->visible = _wagonHere && _round <= 4;
	_oldWagon->visible = _oldMask->visible = 4 < _round;
	_marble->visible = _showMarble;
}

void WordPots::targetPosition(int offset) {
	_box->x = _boxHome.x + offset;
	_box->y = _boxHome.y;
	_mask->x = _maskHome.x + offset;
	_mask->y = _maskHome.y;
	placeAnswer(_target, Common::Point(number("kTargetAnswerX") + offset, number("kTargetAnswerY")));
}

void WordPots::wagonPosition(int offset) {
	_oldWagon->x = _oldWagonHome.x + offset;
	_oldMask->x = _oldMaskHome.x + offset;
	_packages->x = _packagesHome.x + offset;
}

void WordPots::unpack() {
	_state = 2;
	group("brbbrbsb", "kInstrSpchIDs.1");
	_phase = kUnpacking;
	persist();
}

void WordPots::transform(int pot) {
	_pot = pot;
	_transformations += 1;
	Common::String old = _word;
	_alternate[pot] = !_alternate[pot];
	_word.clear();
	for (int i = 0; i < 3; i++)
		_word += _parts[_alternate[i] ? 1 : 0][i];
	_user->setFrame(picture(_word));
	int x = indexed("kContainerX", pot + 1), y = indexed("kContainerY", pot + 1);
	placeAnswer(_user, Common::Point(x + (indexed("kContainerW", pot + 1) - _answerSize.x) / 2,
									 y + (indexed("kContainerH", pot + 1) - _answerSize.y) / 2 + kAnswerGrowOffset));
	int offset = pot * 100;
	setText(_oldText, old, number("kOldWordX") + offset, number("kOldWordY"));
	setText(_newText, _word, number("kNewWordX") + offset, number("kNewWordY"));
	// The script places these text boxes by their top-left corner after changing the text.
	_oldText->x = number("kOldWordX") + offset;
	_oldText->y = number("kOldWordY");
	_newText->x = number("kNewWordX") + offset;
	_newText->y = number("kNewWordY");
	_arrow->x = _arrowHome.x + offset;
	_arrow->y = _arrowHome.y;
	_user->visible = _arrow->visible = _showWords = false;
	_shrink->visible = false;
	_grow->startAnimation();
	_grow->x = _growHome.x + offset;
	_grow->y = _growHome.y;
	_grow->visible = true;
	_phase = kGrowing;
	sound("kDropSfxID");
}

void WordPots::grown() {
	_grow->visible = false;
	_shrink->setFrame(1);
	_shrink->x = _shrinkHome.x + _pot * 100;
	_shrink->y = _shrinkHome.y;
	_shrink->visible = _user->visible = _arrow->visible = _showWords = true;
	sayWord(_word);
	if (_state == 2) {
		_state = 3;
		group("bbr", "kInstrSpchIDs.3");
		group("bss", "kInstrSpchIDs.3", 4 + _pot * 3);
		group("bb", "kInstrSpchIDs.3", 13);
	} else if (_state == 3) {
		_state = 4;
		_target->visible = true;
		_mask->visible = _level == 3;
		group("bbbbsb", "kInstrSpchIDs.5");
	}
	_phase = kReady;
	_idle = _host._time;
	persist();
}

void WordPots::drop() {
	_dragging = false;
	Common::Rect answer = answerBounds();
	int selected = -1, area = 0;
	for (int i = 0; i < 4; i++) {
		Common::Rect rect = bounds(_box);
		if (i < 3) {
			int x = indexed("kContainerX", i + 1), y = indexed("kContainerY", i + 1);
			rect = Common::Rect(x, y, x + indexed("kContainerW", i + 1), y + indexed("kContainerH", i + 1));
		}
		if (!rect.intersects(answer))
			continue;
		rect.clip(answer);
		int overlap = rect.width() * rect.height();
		if (area < overlap) {
			selected = i;
			area = overlap;
		}
	}
	if (0 <= selected && selected < 3) {
		transform(selected);
		return;
	}
	placeAnswer(_user, _home);
	if (selected != 3 || _state < 4) {
		sound("kGoHomeSfxID");
		return;
	}
	if (_word == _targetWord) {
		solved();
		return;
	}
	sound("kWrongSfxID");
	_wrongSpeech = _wrongSpeech % 3 + 1;
	if (_wrongSpeech == 1)
		group("bb", "kIncorrectSpchIDs");
	else if (_wrongSpeech == 2)
		group("br", "kIncorrectSpchIDs", 3);
	else
		say("bloom", "kIncorrectSpchIDs.5");
	_attempts += 1;
	if (2 < _attempts)
		help();
}

void WordPots::solved() {
	_completedRound = _round;
	_user->visible = _showWords = _arrow->visible = false;
	if (!_host._practice) {
		bool imperfect = indexed("kOptimumTransformations", _level) < _transformations;
		int outcome = imperfect ? 3 : 1;
		World &world = _host._world;
		int streak = world.get("R1IA3StreakKind") == outcome ? world.get("R1IA3Streak") + 1 : 1;
		int threshold = imperfect ? number("kAutolevelDownCount") : number("kAutolevelUpCount");
		world.recordAssessment("R1IA3", _level, outcome, number("kAutolevelUpCount"), number("kAutolevelDownCount"), streak);
		world.set("R1IA3StreakKind", outcome);
		world.set("R1IA3Streak", threshold <= streak ? 0 : streak);
		if (threshold <= streak && !world.get("R1IA3FixedLevel"))
			_level = CLIP(_level + (imperfect ? -1 : 1), 1, 3);
		if (4 < _round && _round % 4 == 0)
			_showMarble = true;
	}
	_dataset = _round == 1 ? _host._vm.rnd().number(1, indexed("kMaxDataSets", _level)) : _dataset + 1;
	if (indexed("kMaxDataSets", _level) < _dataset)
		_dataset = 1;
	_set = _host._vm.rnd().number(1, 3);
	_round += 1;
	// Save the next problem together with its reward so a reload cannot award it twice.
	persist();
	_host.queueSound(number("kCorrectSfxID"));
	say("bloom", Common::String::format("kCorrectSpchIDs.%d", _host._vm.rnd().number(1, 4)));
	_phase = kCorrect;
}

Common::Rect WordPots::answerBounds() const {
	Common::Rect result = bounds(_user);
	Common::Point origin = Common::Point(_user->x, _user->y) - frameOffset(_user);
	Common::Rect label(origin.x, origin.y + number("kAnswerTextOffsetY"),
					   origin.x + _answerSize.x, origin.y + _answerSize.y + number("kAnswerTextOffsetY"));
	result.extend(label);
	return result;
}

void WordPots::next() {
	if (_completedRound % 4 == 0 && _oldWagon->visible) {
		_phase = kWagonOut;
		_start = _host._time;
		sound("kRedWagonSfxID");
	} else {
		prepareNext();
	}
}

void WordPots::prepareNext() {
	if (!setup())
		warning("RR1: cannot load IA3 level %d dataset %d set %d", _level, _dataset, _set);
	_phase = kTargetIn;
	_start = _host._time;
	targetPosition(number("kTargetMoveOffset"));
	_user->visible = false;
	int package = (_round - 1) % 4 + 1;
	_packages->visible = true;
	_packages->setFrame(package);
	if (package == 1)
		wagonPosition(number("kOldWagonMoveOffset"));
}

void WordPots::announceNext() {
	say("bloom", "kNextSpchIDs.1");
	sayWord(_targetWord);
	_phase = kNextSpeech;
}

void WordPots::help() {
	_help = _help % 9 + 1;
	if (_help == 5 && _level != 1)
		_help = 6;
	if (_help == 6 && _level == 1)
		_help = 7;
	say("paige", Common::String::format("kHelpSpchIDs.1.%d", _help));
}

void WordPots::bloomHelp() {
	if (_state == 1) {
		unpack();
	} else if (_state < 4) {
		say("bloom", _state == 2 ? "kInstrSpchIDs.2" : "kInstrSpchIDs.4");
	} else {
		int choice = _host._vm.rnd().number(1, 7);
		if (choice <= 5)
			say("bloom", Common::String::format("kInstrSpchIDs.%d", choice == 1 ? 12 : choice + 4));
		else
			group(choice == 6 ? "rb" : "sbr", choice == 6 ? "kInstrSpchIDs.10" : "kInstrSpchIDs.11");
	}
}

bool WordPots::click(Common::Point point) {
	_idle = _host._time;
	if (_state == 1) {
		unpack();
		return true;
	}
	if (_user->visible && bounds(_user).contains(point)) {
		_host.interrupt();
		_dragging = true;
		_dragOffset = Common::Point(point.x - _user->x, point.y - _user->y);
		if (0 <= _pot) {
			_shrink->startAnimation();
			_shrink->visible = true;
			_pot = -1;
		} else {
			sayWord(_word);
		}
		sound("kPickUpSfxID");
		return true;
	}
	if (_showMarble && _marble->covers(point)) {
		_showMarble = _marble->visible = false;
		Common::String key = Common::String::format("item:%d", number("kINV1Marble1"));
		_host._world.set(key, _host._world.get(key) + 1);
		_host.putAway();
		persist();
		return true;
	}
	if (_redWagon->visible && _redWagon->covers(point)) {
		if (4 < _round) {
			_wagonHere = _redWagon->visible = false;
			Common::String key = Common::String::format("item:%d", number("kINV1Wagon"));
			_host._world.set(key, _host._world.get(key) + 1);
			_host.putAway();
			group("rb", "kWagonSpchIDs.2", 3);
			persist();
		} else {
			group("sb", "kWagonSpchIDs.2");
		}
		return true;
	}
	if (_oldWagon->visible && _oldWagon->covers(point)) {
		if (_wagonHere)
			group("sb", "kWagonSpchIDs.1");
		else if (!_host._practice)
			say("reader", "kWagonSpchIDs.1.3");
		return true;
	}
	if (!_host._practice && _host._actors["bloom"]->bounds().contains(point)) {
		bloomHelp();
		return true;
	}
	if (_host._actors["paige"]->bounds().contains(point)) {
		help();
		return true;
	}
	if (!_host._practice && (_host._actors["reader"]->bounds().contains(point) || _host._actors["sam"]->bounds().contains(point))) {
		int choice = _host._vm.rnd().number(1, 4);
		static const char *const actors[] = {
			"rb",
			"sbr",
			"rb",
			"sr",
		};
		group(actors[choice - 1], Common::String::format("kMainSpchIDs.%d", choice));
		return true;
	}
	if (bounds(_box).contains(point)) {
		if (_state == 4)
			sayWord(_targetWord);
		return true;
	}
	for (int i = 0; i < 3; i++) {
		int x = indexed("kContainerX", i + 1), y = indexed("kContainerY", i + 1);
		if (Common::Rect(x, y, x + indexed("kContainerW", i + 1), y + indexed("kContainerH", i + 1)).contains(point)) {
			say("bloom", _level == 1 ? Common::String::format("kPot%dSpchID", i + 1) : Common::String("kPotSpchID"));
			return true;
		}
	}
	return false;
}

void WordPots::hover(Common::Point point) {
	for (auto *pot : _pots) {
		Common::Point position(pot->x, pot->y);
		pot->setFrame(bounds(pot).contains(point) ? 2 : 1);
		pot->x = position.x;
		pot->y = position.y;
	}
	_box->setFrame(bounds(_box).contains(point) ? 2 : 1);
	_box->x = _boxHome.x;
	_box->y = _boxHome.y;
}

bool WordPots::event(const Common::Event &event) {
	if (!_dragging) {
		if (_phase == kReady && event.type == Common::EVENT_MOUSEMOVE)
			hover(event.mouse);
		return false;
	}
	if (event.type == Common::EVENT_MOUSEMOVE || event.type == Common::EVENT_LBUTTONUP) {
		_user->x = event.mouse.x - _dragOffset.x;
		_user->y = event.mouse.y - _dragOffset.y;
		if (event.type == Common::EVENT_LBUTTONUP) {
			_host.interrupt();
			drop();
		}
		return true;
	}
	return event.type == Common::EVENT_LBUTTONDOWN;
}

void WordPots::advance(uint32 time) {
	if (_host._nextSpeech < _host._speech.size() || (_host._speaking && _host._speaking->isBusy()))
		_idle = time;
	if (_pot < 0 && !_shrink->isRunning())
		_shrink->visible = false;
}

void WordPots::update(uint32 time) {
	if (_host._nextSpeech < _host._speech.size())
		return;
	if (_phase == kIntroduction) {
		unpack();
	} else if (_phase == kUnpacking) {
		present();
		_host.queueSound(number("kPackageOpenID"));
		say("bloom", "kInstrSpchIDs.1.9");
		_phase = kReady;
	} else if (_phase == kGrowing && !_grow->isRunning()) {
		grown();
	} else if (_phase == kCorrect) {
		_phase = kTargetOut;
		_start = time;
	} else if (_phase == kTargetOut) {
		uint step = MIN<uint>(12, (time - _start) * 12 / 1000);
		targetPosition(number("kTargetMoveOffset") * step / 12);
		if (step < 12)
			return;
		if (_completedRound % 4 == 0) {
			say("bloom", Common::String::format("kCorrectSpchIDs.%d", _host._vm.rnd().number(6, 7)));
		} else {
			say("bloom", "kCorrectSpchIDs.5");
		}
		_phase = kBatchSpeech;
	} else if (_phase == kBatchSpeech) {
		if (_completedRound == 4) {
			_redMask->visible = false;
			sound("kRedWagonSfxID");
			_phase = kRedWagonOut;
			_start = time;
		} else if (_completedRound % 4 == 0 && !_host._practice) {
			bool existing = _marble->visible;
			_marble->visible = true;
			say("bloom", existing ? "kCorrectSpchIDs.9" : "kCorrectSpchIDs.8");
			_phase = existing ? kRedWagonSpeech : kMarbleSpeech;
		} else {
			next();
		}
	} else if (_phase == kRedWagonOut) {
		int step = MIN<uint>(12, (time - _start) * 12 / 1000);
		_redWagon->x = _wagonHome.x - number("kRedWagonMoveOffset") * step / 12;
		if (step == 12) {
			say("bloom", "kCorrectSpchIDs.10");
			_phase = kRedWagonSpeech;
		}
	} else if (_phase == kRedWagonSpeech) {
		next();
	} else if (_phase == kMarbleSpeech) {
		_marble->setFrame(3);
		_host.queueSound(number("kMarble1ID"));
		_phase = kMarbleFlash;
	} else if (_phase == kMarbleFlash) {
		_marble->setFrame(1);
		next();
	} else if (_phase == kWagonOut) {
		int step = MIN<uint>(12, (time - _start) * 12 / 1000);
		wagonPosition(number("kOldWagonMoveOffset") * step / 12);
		if (step == 12)
			prepareNext();
	} else if (_phase == kTargetIn) {
		int step = MIN<uint>(12, (time - _start) * 12 / 1000);
		targetPosition(number("kTargetMoveOffset") * (12 - step) / 12);
		if (step == 12) {
			if (_completedRound % 4 == 0) {
				_phase = kWagonIn;
				_start = time;
				sound("kRedWagonSfxID");
			} else {
				announceNext();
			}
		}
	} else if (_phase == kWagonIn) {
		int step = MIN<uint>(12, (time - _start) * 12 / 1000);
		wagonPosition(number("kOldWagonMoveOffset") * (12 - step) / 12);
		if (step == 12)
			announceNext();
	} else if (_phase == kNextSpeech) {
		present();
		sound("kPackageOpenID");
		if (_host._vm.rnd().number(1, 4) == 1) {
			_nextSpeechIndex += 1;
			if (number("kMaxCorrectSpeech") < _nextSpeechIndex)
				_nextSpeechIndex = 2;
			say("bloom", Common::String::format("kNextSpchIDs.%d", _nextSpeechIndex));
		}
		_phase = kReady;
		_idle = time;
	} else if (!_host._practice && _phase == kReady && !_dragging && _idleDelay <= time - _idle) {
		_idle = time;
		_idleDelay = 30000;
		_idleIndex += 1;
		if (_state < 4)
			bloomHelp();
		else if (_idleIndex % 2 == 1)
			say("bloom", "kIdleSpchIDs.1");
		else
			group("rs", "kIdleSpchIDs.2");
	}
}

void WordPots::draw(OM::Canvas &canvas) {
	_host._scene.drawBackground(canvas);
	_host._scene.drawSprites(canvas, INT_MIN, 4);
	if (_showWords) {
		_oldText->draw(canvas);
		_newText->draw(canvas);
	}
	_host._scene.drawSprites(canvas, 5, 94);
	if (_target->visible) {
		Common::Point offset = frameOffset(_target);
		setText(_targetText, _targetWord, _target->x - offset.x + _answerSize.x / 2,
				_target->y - offset.y + _answerSize.y / 2 + number("kAnswerTextOffsetY"));
		_targetText->draw(canvas);
	}
	_host._scene.drawSprites(canvas, 95, 100);
	if (_user->visible) {
		Common::Point offset = frameOffset(_user);
		setText(_userText, _word, _user->x - offset.x + _answerSize.x / 2,
				_user->y - offset.y + _answerSize.y / 2 + number("kAnswerTextOffsetY"));
		_userText->draw(canvas);
	}
	_host._scene.drawSprites(canvas, 101, INT_MAX);
}

void WordPots::skipSpeech() { _host.interrupt(); }

void WordPots::pause(bool paused) {
	if (paused) {
		_pausedSprites.clear();
		for (auto *sprite : {_grow, _shrink}) {
			if (sprite && sprite->isRunning()) {
				_pausedSprites.push_back(sprite);
				sprite->pauseAnimation(true);
			}
		}
	} else {
		for (auto *sprite : _pausedSprites)
			sprite->pauseAnimation(false);
		_pausedSprites.clear();
	}
}

} // namespace RR1Cloud9
} // namespace Trinket
