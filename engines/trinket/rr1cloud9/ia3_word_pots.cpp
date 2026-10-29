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

constexpr const char *WordPots::kNameSignedIndexFormat;
constexpr const char *WordPots::kSamName;
constexpr const char *WordPots::kPropertyCenter;
constexpr const char *WordPots::kPropertyColor;
constexpr const char *WordPots::kPropertyText;
constexpr const char *WordPots::kPropertyX;
constexpr const char *WordPots::kPropertyY;
constexpr const char *WordPots::kPName;
constexpr const char *WordPots::kBloomName;
constexpr const char *WordPots::kWordNameSpchIDKey;
constexpr const char *WordPots::kTEXTIA3WORDSName;
constexpr const char *WordPots::kTEXTIA3WORDTYPESName;
constexpr const char *WordPots::kTEXTIA3PICTUREWORDSName;
constexpr const char *WordPots::kBloomKey;
constexpr const char *WordPots::kR1IA3LevelKey;
constexpr const char *WordPots::kR1IA3DatasetKey;
constexpr const char *WordPots::kMaxDataSetsKey;
constexpr const char *WordPots::kR1IA3SetKey;
constexpr const char *WordPots::kR1IA3StateKey;
constexpr const char *WordPots::kR1IA3RoundKey;
constexpr const char *WordPots::kR1IA3WagonHereKey;
constexpr const char *WordPots::kR1IA3ShowMarbleKey;
constexpr const char *WordPots::kItemsIDKey;
constexpr const char *WordPots::kTargetBoxIDKey;
constexpr const char *WordPots::kTargetMaskIDKey;
constexpr const char *WordPots::kGrowIDKey;
constexpr const char *WordPots::kShrinkIDKey;
constexpr const char *WordPots::kArrowIDKey;
constexpr const char *WordPots::kPackagesIDKey;
constexpr const char *WordPots::kRedWagonIDKey;
constexpr const char *WordPots::kRedWagonMaskIDKey;
constexpr const char *WordPots::kOldWagonIDKey;
constexpr const char *WordPots::kOldWagonMaskIDKey;
constexpr const char *WordPots::kMarbleIDKey;
constexpr const char *WordPots::kPotIDsSignedIndexKey;
constexpr const char *WordPots::kMarbleXKey;
constexpr const char *WordPots::kMarbleYKey;
constexpr const char *WordPots::kInstrSpchIDs6Key;
constexpr const char *WordPots::kInstrSpchIDs8Key;
constexpr const char *WordPots::kInstrSpchIDs52Key;
constexpr const char *WordPots::kInstrSpchIDs7Key;
constexpr const char *WordPots::kReaderName;
constexpr const char *WordPots::kEnterSpchIDsKey;
constexpr const char *WordPots::kRbName;
constexpr const char *WordPots::kReEnterSpchIDs3Key;
constexpr const char *WordPots::kReEnterSpchIDsSignedIndexKey;
constexpr const char *WordPots::kNextSpchIDs1Key;
constexpr const char *WordPots::kPropertyDataSetLevelSignedIndexWordListSignedIndexSetSignedIndex;
constexpr const char *WordPots::kPackageXKey;
constexpr const char *WordPots::kPackageYKey;
constexpr const char *WordPots::kRedWagonMoveOffsetKey;
constexpr const char *WordPots::kTargetAnswerXKey;
constexpr const char *WordPots::kTargetAnswerYKey;
constexpr const char *WordPots::kBrbbrbsbName;
constexpr const char *WordPots::kInstrSpchIDs1Key;
constexpr const char *WordPots::kContainerXKey;
constexpr const char *WordPots::kContainerYKey;
constexpr const char *WordPots::kContainerWKey;
constexpr const char *WordPots::kContainerHKey;
constexpr const char *WordPots::kOldWordXKey;
constexpr const char *WordPots::kOldWordYKey;
constexpr const char *WordPots::kNewWordXKey;
constexpr const char *WordPots::kNewWordYKey;
constexpr const char *WordPots::kDropSfxIDKey;
constexpr const char *WordPots::kBbrName;
constexpr const char *WordPots::kInstrSpchIDs3Key;
constexpr const char *WordPots::kBssName;
constexpr const char *WordPots::kBbName;
constexpr const char *WordPots::kBbbbsbName;
constexpr const char *WordPots::kInstrSpchIDs5Key;
constexpr const char *WordPots::kGoHomeSfxIDKey;
constexpr const char *WordPots::kWrongSfxIDKey;
constexpr const char *WordPots::kIncorrectSpchIDsKey;
constexpr const char *WordPots::kBrName;
constexpr const char *WordPots::kIncorrectSpchIDs5Key;
constexpr const char *WordPots::kOptimumTransformationsKey;
constexpr const char *WordPots::kR1IA3StreakKindKey;
constexpr const char *WordPots::kR1IA3StreakKey;
constexpr const char *WordPots::kAutolevelDownCountKey;
constexpr const char *WordPots::kAutolevelUpCountKey;
constexpr const char *WordPots::kR1IA3Key;
constexpr const char *WordPots::kR1IA3FixedLevelKey;
constexpr const char *WordPots::kCorrectSfxIDKey;
constexpr const char *WordPots::kCorrectSpchIDsSignedIndexKey;
constexpr const char *WordPots::kAnswerTextOffsetYKey;
constexpr const char *WordPots::kRedWagonSfxIDKey;
constexpr const char *WordPots::kTargetMoveOffsetKey;
constexpr const char *WordPots::kOldWagonMoveOffsetKey;
constexpr const char *WordPots::kPaigeName;
constexpr const char *WordPots::kHelpSpchIDs1SignedIndexKey;
constexpr const char *WordPots::kInstrSpchIDs2Key;
constexpr const char *WordPots::kInstrSpchIDs4Key;
constexpr const char *WordPots::kInstrSpchIDsSignedIndexKey;
constexpr const char *WordPots::kSbrName;
constexpr const char *WordPots::kInstrSpchIDs10Key;
constexpr const char *WordPots::kInstrSpchIDs11Key;
constexpr const char *WordPots::kPickUpSfxIDKey;
constexpr const char *WordPots::kItemSignedIndexFormat;
constexpr const char *WordPots::kINV1Marble1Key;
constexpr const char *WordPots::kINV1WagonKey;
constexpr const char *WordPots::kWagonSpchIDs2Key;
constexpr const char *WordPots::kSbName;
constexpr const char *WordPots::kWagonSpchIDs1Key;
constexpr const char *WordPots::kWagonSpchIDs13Key;
constexpr const char *WordPots::kSrName;
constexpr const char *WordPots::kMainSpchIDsSignedIndexKey;
constexpr const char *WordPots::kPotSignedIndexSpchIDKey;
constexpr const char *WordPots::kPotSpchIDKey;
constexpr const char *WordPots::kPackageOpenIDKey;
constexpr const char *WordPots::kInstrSpchIDs19Key;
constexpr const char *WordPots::kCorrectSpchIDs5Key;
constexpr const char *WordPots::kCorrectSpchIDs9Key;
constexpr const char *WordPots::kCorrectSpchIDs8Key;
constexpr const char *WordPots::kCorrectSpchIDs10Key;
constexpr const char *WordPots::kMarble1IDKey;
constexpr const char *WordPots::kMaxCorrectSpeechKey;
constexpr const char *WordPots::kNextSpchIDsSignedIndexKey;
constexpr const char *WordPots::kIdleSpchIDs1Key;
constexpr const char *WordPots::kRsName;
constexpr const char *WordPots::kIdleSpchIDs2Key;

constexpr const char *WordPots::kTextFormat;
constexpr const char *WordPots::kFont;
constexpr const char *WordPots::kPotFont;
constexpr const char *WordPots::kDataXML;

WordPots::WordPots(PageScene &host) : _host(host), _paths(host._vm.resources()) {}
WordPots::~WordPots() {
	delete _userText;
	delete _targetText;
	delete _oldText;
	delete _newText;
}

int WordPots::number(const Common::String &key) const { return _host._page->number(key); }

int WordPots::indexed(const char *key, int i) const { return number(Common::String::format(kNameSignedIndexFormat, key, i)); }

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
			actor = kSamName;
		say(actor, Common::String::format(kNameSignedIndexFormat, key.c_str(), first + i));
	}
}

Shell::Sprite *WordPots::sprite(const char *key, int depth) {
	return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), depth);
}

Shell::TextObject *WordPots::text(bool pot) {
	Shell::TextObject *result = new Shell::TextObject(_paths, _host._vm.fonts(), pot ? _potFont : _font);
	result->create({Core::ScriptValue::fromText(" "), Core::ScriptValue(0), Core::ScriptValue(0), Core::ScriptValue(1)});
	result->set(kPropertyCenter, {}, Core::ScriptValue(1));
	result->set(kPropertyColor, {Core::ScriptValue(pot ? 65535 : 6317), Core::ScriptValue(pot ? 63487 : 4212)},
				Core::ScriptValue(pot ? 59647 : 2106));
	return result;
}

void WordPots::setText(Shell::TextObject *text, const Common::String &value, int x, int y) {
	text->set(kPropertyText, {}, Core::ScriptValue::fromText(value));
	text->set(kPropertyX, {}, Core::ScriptValue(x));
	text->set(kPropertyY, {}, Core::ScriptValue(y));
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
				return step.offset;
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
	Common::ScopedPtr<Common::SeekableReadStream> stream(_host._vm.resources().open(Common::String::format(kTextFormat, resource)));
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
	for (uint i = 0; i < list.size(); i++) {
		if (word.equalsIgnoreCase(list[i]))
			return i;
	}
	return -1;
}

int WordPots::picture(const Common::String &word) const {
	int at = index(_words, word);
	if (at < 0)
		return 0;
	if (_types[at].equals(kPName)) {
		int frame = index(_pictures, word);
		return frame < 0 ? 0 : frame + 3;
	}
	return _types[at].equals("?") ? 2 : 1;
}

void WordPots::sayWord(const Common::String &word) {
	int at = index(_words, word);
	if (0 <= at) {
		_host.delaySpeech(1);
		_host.say(kBloomName, number(kWordNameSpchIDKey) + at);
		_host.delaySpeech(1);
	}
}

bool WordPots::enter() {
	_font = _host._vm.fonts().load(kFont);
	_potFont = _host._vm.fonts().load(kPotFont);
	if (!_font || !_potFont || !_data.loadStream(_host._vm.resources().open(kDataXML)) || !_data.parse() ||
		!loadList(number(kTEXTIA3WORDSName), _words) || !loadList(number(kTEXTIA3WORDTYPESName), _types) ||
		!loadList(number(kTEXTIA3PICTUREWORDSName), _pictures) || _words.size() != _types.size() || !_host.actor(kBloomName, kBloomKey))
		return false;
	_level = CLIP(_host._world.get(kR1IA3LevelKey), 1, 3);
	_dataset = CLIP(_host._world.get(kR1IA3DatasetKey), 1, indexed(kMaxDataSetsKey, _level));
	_set = _host._world.get(kR1IA3SetKey);
	if (_set < 1 || 3 < _set)
		_set = _host._vm._rnd->getRandomNumberRng(1, 3);
	_state = CLIP(_host._world.get(kR1IA3StateKey), 1, 4);
	if (_state == 3)
		_state = 2;
	_round = MAX(1, _host._world.get(kR1IA3RoundKey));
	_wagonHere = !_host._world.get(kR1IA3StateKey) || _host._world.get(kR1IA3WagonHereKey);
	_showMarble = _host._world.get(kR1IA3ShowMarbleKey);
	if (_host._practice) {
		_state = 4;
		_round = 5;
		_wagonHere = _showMarble = false;
		if (_host._chester)
			_host._chester->visible = false;
	}
	_user = sprite(kItemsIDKey, 100);
	_target = sprite(kItemsIDKey, 92);
	_box = sprite(kTargetBoxIDKey, 91);
	_mask = sprite(kTargetMaskIDKey, 95);
	_grow = sprite(kGrowIDKey, 2);
	_shrink = sprite(kShrinkIDKey, 2);
	_arrow = sprite(kArrowIDKey, 4);
	_packages = sprite(kPackagesIDKey, 7);
	_redWagon = sprite(kRedWagonIDKey, 6);
	_redMask = sprite(kRedWagonMaskIDKey, 8);
	_oldWagon = sprite(kOldWagonIDKey, 6);
	_oldMask = sprite(kOldWagonMaskIDKey, 8);
	_marble = sprite(kMarbleIDKey, 10);
	for (int i = 0; i < 3; i++) {
		_pots[i] = sprite(Common::String::format(kPotIDsSignedIndexKey, i + 1).c_str(), 3);
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
	_marble->colorRemap = PageScene::marbleColor(0);
	_marble->x = number(kMarbleXKey);
	_marble->y = number(kMarbleYKey);
	if (!setup())
		return false;
	if (_host._practice) {
		for (const char *key : {kInstrSpchIDs6Key, kInstrSpchIDs8Key, kInstrSpchIDs52Key, kInstrSpchIDs7Key})
			say(kBloomName, key);
	} else if (_state == 1) {
		say(kReaderName, kEnterSpchIDsKey);
		_phase = kIntroduction;
	} else {
		int choice = _host._vm._rnd->getRandomNumberRng(1, 3);
		if (choice == 3)
			group(kRbName, kReEnterSpchIDs3Key);
		else
			say(kBloomName, Common::String::format(kReEnterSpchIDsSignedIndexKey, choice));
	}
	if (_state == 4) {
		say(kBloomName, kNextSpchIDs1Key);
		sayWord(_targetWord);
	} else if (1 < _state) {
		bloomHelp();
	}
	return true;
}

void WordPots::persist() {
	if (_host._practice)
		return;
	_host._world.set(kR1IA3StateKey, _state);
	_host._world.set(kR1IA3LevelKey, _level);
	_host._world.set(kR1IA3DatasetKey, _dataset);
	_host._world.set(kR1IA3SetKey, _set);
	_host._world.set(kR1IA3RoundKey, _round);
	_host._world.set(kR1IA3WagonHereKey, _wagonHere);
	_host._world.set(kR1IA3ShowMarbleKey, _showMarble);
}

bool WordPots::setup() {
	Common::StringTokenizer tokens(_data.get(Common::String::format(kPropertyDataSetLevelSignedIndexWordListSignedIndexSetSignedIndex, _level, _dataset, _set)));
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
	_home = Common::Point(indexed(kPackageXKey, package), indexed(kPackageYKey, package));
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
	_redWagon->x = _wagonHome.x - (4 < _round ? number(kRedWagonMoveOffsetKey) : 0);
	_redMask->visible = _wagonHere && _round <= 4;
	_oldWagon->visible = _oldMask->visible = 4 < _round;
	_marble->visible = _showMarble;
}

void WordPots::targetPosition(int offset) {
	_box->x = _boxHome.x + offset;
	_box->y = _boxHome.y;
	_mask->x = _maskHome.x + offset;
	_mask->y = _maskHome.y;
	placeAnswer(_target, Common::Point(number(kTargetAnswerXKey) + offset, number(kTargetAnswerYKey)));
}

void WordPots::wagonPosition(int offset) {
	_oldWagon->x = _oldWagonHome.x + offset;
	_oldMask->x = _oldMaskHome.x + offset;
	_packages->x = _packagesHome.x + offset;
}

void WordPots::unpack() {
	_state = 2;
	group(kBrbbrbsbName, kInstrSpchIDs1Key);
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
	int x = indexed(kContainerXKey, pot + 1);
	int y = indexed(kContainerYKey, pot + 1);
	placeAnswer(_user, Common::Point(x + (indexed(kContainerWKey, pot + 1) - _answerSize.x) / 2,
									 y + (indexed(kContainerHKey, pot + 1) - _answerSize.y) / 2 + kAnswerGrowOffset));
	int offset = pot * 100;
	setText(_oldText, old, number(kOldWordXKey) + offset, number(kOldWordYKey));
	setText(_newText, _word, number(kNewWordXKey) + offset, number(kNewWordYKey));
	// The script places these text boxes by their top-left corner after changing the text.
	_oldText->x = number(kOldWordXKey) + offset;
	_oldText->y = number(kOldWordYKey);
	_newText->x = number(kNewWordXKey) + offset;
	_newText->y = number(kNewWordYKey);
	_arrow->x = _arrowHome.x + offset;
	_arrow->y = _arrowHome.y;
	_user->visible = _arrow->visible = _showWords = false;
	_shrink->visible = false;
	_grow->startAnimation();
	_grow->x = _growHome.x + offset;
	_grow->y = _growHome.y;
	_grow->visible = true;
	_phase = kGrowing;
	sound(kDropSfxIDKey);
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
		group(kBbrName, kInstrSpchIDs3Key);
		group(kBssName, kInstrSpchIDs3Key, 4 + _pot * 3);
		group(kBbName, kInstrSpchIDs3Key, 13);
	} else if (_state == 3) {
		_state = 4;
		_target->visible = true;
		_mask->visible = _level == 3;
		group(kBbbbsbName, kInstrSpchIDs5Key);
	}
	_phase = kReady;
	_idle = _host._time;
	persist();
}

void WordPots::drop() {
	_dragging = false;
	Common::Rect answer = answerBounds();
	int selected = -1;
	int area = 0;
	for (int i = 0; i < 4; i++) {
		Common::Rect rect = bounds(_box);
		if (i < 3) {
			int x = indexed(kContainerXKey, i + 1);
			int y = indexed(kContainerYKey, i + 1);
			rect = Common::Rect(x, y, x + indexed(kContainerWKey, i + 1), y + indexed(kContainerHKey, i + 1));
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
		sound(kGoHomeSfxIDKey);
		return;
	}
	if (_word.equals(_targetWord)) {
		solved();
		return;
	}
	sound(kWrongSfxIDKey);
	_wrongSpeech = _wrongSpeech % 3 + 1;
	if (_wrongSpeech == 1)
		group(kBbName, kIncorrectSpchIDsKey);
	else if (_wrongSpeech == 2)
		group(kBrName, kIncorrectSpchIDsKey, 3);
	else
		say(kBloomName, kIncorrectSpchIDs5Key);
	_attempts += 1;
	if (2 < _attempts)
		help();
}

void WordPots::solved() {
	_completedRound = _round;
	_user->visible = _showWords = _arrow->visible = false;
	if (!_host._practice) {
		bool imperfect = indexed(kOptimumTransformationsKey, _level) < _transformations;
		int outcome = imperfect ? 3 : 1;
		World &world = _host._world;
		int streak = world.get(kR1IA3StreakKindKey) == outcome ? world.get(kR1IA3StreakKey) + 1 : 1;
		int threshold = imperfect ? number(kAutolevelDownCountKey) : number(kAutolevelUpCountKey);
		world.recordAssessment(kR1IA3Key, _level, outcome, number(kAutolevelUpCountKey), number(kAutolevelDownCountKey), streak);
		world.set(kR1IA3StreakKindKey, outcome);
		world.set(kR1IA3StreakKey, threshold <= streak ? 0 : streak);
		if (threshold <= streak && !world.get(kR1IA3FixedLevelKey))
			_level = CLIP(_level + (imperfect ? -1 : 1), 1, 3);
		if (4 < _round && _round % 4 == 0)
			_showMarble = true;
	}
	_dataset = _round == 1 ? _host._vm._rnd->getRandomNumberRng(1, indexed(kMaxDataSetsKey, _level)) : _dataset + 1;
	if (indexed(kMaxDataSetsKey, _level) < _dataset)
		_dataset = 1;
	_set = _host._vm._rnd->getRandomNumberRng(1, 3);
	_round += 1;
	// Save the next problem together with its reward so a reload cannot award it twice.
	persist();
	_host.queueSound(number(kCorrectSfxIDKey));
	say(kBloomName, Common::String::format(kCorrectSpchIDsSignedIndexKey, _host._vm._rnd->getRandomNumberRng(1, 4)));
	_phase = kCorrect;
}

Common::Rect WordPots::answerBounds() const {
	Common::Rect result = bounds(_user);
	Common::Point origin = Common::Point(_user->x, _user->y) - frameOffset(_user);
	Common::Rect label(origin.x, origin.y + number(kAnswerTextOffsetYKey),
					   origin.x + _answerSize.x, origin.y + _answerSize.y + number(kAnswerTextOffsetYKey));
	result.extend(label);
	return result;
}

void WordPots::next() {
	if (_completedRound % 4 == 0 && _oldWagon->visible) {
		_phase = kWagonOut;
		_start = _host._time;
		sound(kRedWagonSfxIDKey);
	} else {
		prepareNext();
	}
}

void WordPots::prepareNext() {
	if (!setup())
		warning("RR1: cannot load IA3 level %d dataset %d set %d", _level, _dataset, _set);
	_phase = kTargetIn;
	_start = _host._time;
	targetPosition(number(kTargetMoveOffsetKey));
	_user->visible = false;
	int package = (_round - 1) % 4 + 1;
	_packages->visible = true;
	_packages->setFrame(package);
	if (package == 1)
		wagonPosition(number(kOldWagonMoveOffsetKey));
}

void WordPots::announceNext() {
	say(kBloomName, kNextSpchIDs1Key);
	sayWord(_targetWord);
	_phase = kNextSpeech;
}

void WordPots::help() {
	_help = _help % 9 + 1;
	if (_help == 5 && _level != 1)
		_help = 6;
	if (_help == 6 && _level == 1)
		_help = 7;
	say(kPaigeName, Common::String::format(kHelpSpchIDs1SignedIndexKey, _help));
}

void WordPots::bloomHelp() {
	if (_state == 1) {
		unpack();
	} else if (_state < 4) {
		say(kBloomName, _state == 2 ? kInstrSpchIDs2Key : kInstrSpchIDs4Key);
	} else {
		int choice = _host._vm._rnd->getRandomNumberRng(1, 7);
		if (choice <= 5)
			say(kBloomName, Common::String::format(kInstrSpchIDsSignedIndexKey, choice == 1 ? 12 : choice + 4));
		else
			group(choice == 6 ? kRbName : kSbrName, choice == 6 ? kInstrSpchIDs10Key : kInstrSpchIDs11Key);
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
		sound(kPickUpSfxIDKey);
		return true;
	}
	if (_showMarble && _marble->covers(point)) {
		_showMarble = _marble->visible = false;
		Common::String key = Common::String::format(kItemSignedIndexFormat, number(kINV1Marble1Key));
		_host._world.set(key, _host._world.get(key) + 1);
		_host.putAway();
		persist();
		return true;
	}
	if (_redWagon->visible && _redWagon->covers(point)) {
		if (4 < _round) {
			_wagonHere = _redWagon->visible = false;
			Common::String key = Common::String::format(kItemSignedIndexFormat, number(kINV1WagonKey));
			_host._world.set(key, _host._world.get(key) + 1);
			_host.putAway();
			group(kRbName, kWagonSpchIDs2Key, 3);
			persist();
		} else {
			group(kSbName, kWagonSpchIDs2Key);
		}
		return true;
	}
	if (_oldWagon->visible && _oldWagon->covers(point)) {
		if (_wagonHere)
			group(kSbName, kWagonSpchIDs1Key);
		else if (!_host._practice)
			say(kReaderName, kWagonSpchIDs13Key);
		return true;
	}
	if (!_host._practice && _host._actors[kBloomName]->bounds().contains(point)) {
		bloomHelp();
		return true;
	}
	if (_host._actors[kPaigeName]->bounds().contains(point)) {
		help();
		return true;
	}
	if (!_host._practice && (_host._actors[kReaderName]->bounds().contains(point) || _host._actors[kSamName]->bounds().contains(point))) {
		int choice = _host._vm._rnd->getRandomNumberRng(1, 4);
		static constexpr const char *actors[] = {
			kRbName,
			kSbrName,
			kRbName,
			kSrName,
		};
		group(actors[choice - 1], Common::String::format(kMainSpchIDsSignedIndexKey, choice));
		return true;
	}
	if (bounds(_box).contains(point)) {
		if (_state == 4)
			sayWord(_targetWord);
		return true;
	}
	for (int i = 0; i < 3; i++) {
		int x = indexed(kContainerXKey, i + 1);
		int y = indexed(kContainerYKey, i + 1);
		if (Common::Rect(x, y, x + indexed(kContainerWKey, i + 1), y + indexed(kContainerHKey, i + 1)).contains(point)) {
			say(kBloomName, _level == 1 ? Common::String::format(kPotSignedIndexSpchIDKey, i + 1) : Common::String(kPotSpchIDKey));
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
		_host.queueSound(number(kPackageOpenIDKey));
		say(kBloomName, kInstrSpchIDs19Key);
		_phase = kReady;
	} else if (_phase == kGrowing && !_grow->isRunning()) {
		grown();
	} else if (_phase == kCorrect) {
		_phase = kTargetOut;
		_start = time;
	} else if (_phase == kTargetOut) {
		uint step = MIN<uint>(12, (time - _start) * 12 / 1000);
		targetPosition(number(kTargetMoveOffsetKey) * step / 12);
		if (step < 12)
			return;
		if (_completedRound % 4 == 0) {
			say(kBloomName, Common::String::format(kCorrectSpchIDsSignedIndexKey, _host._vm._rnd->getRandomNumberRng(6, 7)));
		} else {
			say(kBloomName, kCorrectSpchIDs5Key);
		}
		_phase = kBatchSpeech;
	} else if (_phase == kBatchSpeech) {
		if (_completedRound == 4) {
			_redMask->visible = false;
			sound(kRedWagonSfxIDKey);
			_phase = kRedWagonOut;
			_start = time;
		} else if (_completedRound % 4 == 0 && !_host._practice) {
			bool existing = _marble->visible;
			_marble->visible = true;
			say(kBloomName, existing ? kCorrectSpchIDs9Key : kCorrectSpchIDs8Key);
			_phase = existing ? kRedWagonSpeech : kMarbleSpeech;
		} else {
			next();
		}
	} else if (_phase == kRedWagonOut) {
		int step = MIN<uint>(12, (time - _start) * 12 / 1000);
		_redWagon->x = _wagonHome.x - number(kRedWagonMoveOffsetKey) * step / 12;
		if (step == 12) {
			say(kBloomName, kCorrectSpchIDs10Key);
			_phase = kRedWagonSpeech;
		}
	} else if (_phase == kRedWagonSpeech) {
		next();
	} else if (_phase == kMarbleSpeech) {
		_marble->setFrame(3);
		_host.queueSound(number(kMarble1IDKey));
		_phase = kMarbleFlash;
	} else if (_phase == kMarbleFlash) {
		_marble->setFrame(1);
		next();
	} else if (_phase == kWagonOut) {
		int step = MIN<uint>(12, (time - _start) * 12 / 1000);
		wagonPosition(number(kOldWagonMoveOffsetKey) * step / 12);
		if (step == 12)
			prepareNext();
	} else if (_phase == kTargetIn) {
		int step = MIN<uint>(12, (time - _start) * 12 / 1000);
		targetPosition(number(kTargetMoveOffsetKey) * (12 - step) / 12);
		if (step == 12) {
			if (_completedRound % 4 == 0) {
				_phase = kWagonIn;
				_start = time;
				sound(kRedWagonSfxIDKey);
			} else {
				announceNext();
			}
		}
	} else if (_phase == kWagonIn) {
		int step = MIN<uint>(12, (time - _start) * 12 / 1000);
		wagonPosition(number(kOldWagonMoveOffsetKey) * (12 - step) / 12);
		if (step == 12)
			announceNext();
	} else if (_phase == kNextSpeech) {
		present();
		sound(kPackageOpenIDKey);
		if (_host._vm._rnd->getRandomNumberRng(1, 4) == 1) {
			_nextSpeechIdx += 1;
			if (number(kMaxCorrectSpeechKey) < _nextSpeechIdx)
				_nextSpeechIdx = 2;
			say(kBloomName, Common::String::format(kNextSpchIDsSignedIndexKey, _nextSpeechIdx));
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
			say(kBloomName, kIdleSpchIDs1Key);
		else
			group(kRsName, kIdleSpchIDs2Key);
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
				_target->y - offset.y + _answerSize.y / 2 + number(kAnswerTextOffsetYKey));
		_targetText->draw(canvas);
	}
	_host._scene.drawSprites(canvas, 95, 100);
	if (_user->visible) {
		Common::Point offset = frameOffset(_user);
		setText(_userText, _word, _user->x - offset.x + _answerSize.x / 2,
				_user->y - offset.y + _answerSize.y / 2 + number(kAnswerTextOffsetYKey));
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
