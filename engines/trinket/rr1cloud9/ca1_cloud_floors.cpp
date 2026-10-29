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

#include "trinket/rr1cloud9/ca1_cloud_floors.h"

#include "common/tokenizer.h"
#include "graphics/cursorman.h"
#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

constexpr const char *CloudFloors::kNameSignedIndexFormat;
constexpr const char *CloudFloors::kUnsignedIndexFormat;
constexpr const char *CloudFloors::kPropertyWorker;
constexpr const char *CloudFloors::kCWSetIDKey;
constexpr const char *CloudFloors::kCWZKey;
constexpr const char *CloudFloors::kRegisterFidgetsToSettleName;
constexpr const char *CloudFloors::kRegisterSpeechToSettleName;
constexpr const char *CloudFloors::kLoadSpeechAnimsInBkgdName;
constexpr const char *CloudFloors::kAnimateSettledName;
constexpr const char *CloudFloors::kMethodSetSpeechAnims;
constexpr const char *CloudFloors::kCWLipsIDKey;
constexpr const char *CloudFloors::kCWBodyIDKey;
constexpr const char *CloudFloors::kMethodAddFidgetGroup;
constexpr const char *CloudFloors::kMethodAddFidget;
constexpr const char *CloudFloors::kCWFidgIDSignedIndexSignedIndexKey;
constexpr const char *CloudFloors::kCA1WordsIDKey;
constexpr const char *CloudFloors::kTileBoxIDKey;
constexpr const char *CloudFloors::kPropertyCenter;
constexpr const char *CloudFloors::kPropertyColor;
constexpr const char *CloudFloors::kTEXTCA1WTILESName;
constexpr const char *CloudFloors::kTEXTCA1WORDSName;
constexpr const char *CloudFloors::kR1CA1LevelKey;
constexpr const char *CloudFloors::kR1CA1DatasetKey;
constexpr const char *CloudFloors::kR1CA1SetKey;
constexpr const char *CloudFloors::kNumSetsKey;
constexpr const char *CloudFloors::kMaxDataSetsKey;
constexpr const char *CloudFloors::kR1CA1StateKey;
constexpr const char *CloudFloors::kR1CA1RoundKey;
constexpr const char *CloudFloors::kR1CA1LadderHereKey;
constexpr const char *CloudFloors::kCloudHiliteIDKey;
constexpr const char *CloudFloors::kBelt1IDKey;
constexpr const char *CloudFloors::kMask1IDSignedIndexKey;
constexpr const char *CloudFloors::kCloudPieceIDSignedIndexKey;
constexpr const char *CloudFloors::kWordTileIDKey;
constexpr const char *CloudFloors::kBrickIDKey;
constexpr const char *CloudFloors::kCloudBrickZKey;
constexpr const char *CloudFloors::kReachIDSignedIndexKey;
constexpr const char *CloudFloors::kCloudBrickXName;
constexpr const char *CloudFloors::kCloudBrickYName;
constexpr const char *CloudFloors::kNumElevatorsKey;
constexpr const char *CloudFloors::kElevator0IDKey;
constexpr const char *CloudFloors::kCatcherIDKey;
constexpr const char *CloudFloors::kLadderIDKey;
constexpr const char *CloudFloors::kMarbleIDKey;
constexpr const char *CloudFloors::kDownLadderIDKey;
constexpr const char *CloudFloors::kTossIDKey;
constexpr const char *CloudFloors::kMarbleXKey;
constexpr const char *CloudFloors::kMarbleYKey;
constexpr const char *CloudFloors::kPEnterSpchIDSignedIndexKey;
constexpr const char *CloudFloors::kEnterCharIDKey;
constexpr const char *CloudFloors::kEnterSpchIDKey;
constexpr const char *CloudFloors::kMaxEntrySpchsKey;
constexpr const char *CloudFloors::kReEnterSpchIDs1SignedIndexKey;
constexpr const char *CloudFloors::kR1CA1ReentryKey;
constexpr const char *CloudFloors::kReEnterSpchIDs21Key;
constexpr const char *CloudFloors::kPropertyDataSetLevelSignedIndexWordListSignedIndexSetSignedIndex;
constexpr const char *CloudFloors::kNumWordsKey;
constexpr const char *CloudFloors::kTargetXSignedIndexSignedIndexKey;
constexpr const char *CloudFloors::kTargetYKey;
constexpr const char *CloudFloors::kAnswerStartXKey;
constexpr const char *CloudFloors::kAnswerYKey;
constexpr const char *CloudFloors::kR1CA1StreakKindKey;
constexpr const char *CloudFloors::kR1CA1StreakKey;
constexpr const char *CloudFloors::kR1CA1Key;
constexpr const char *CloudFloors::kR1CA1FixedLevelKey;
constexpr const char *CloudFloors::kEndPtKey;
constexpr const char *CloudFloors::kXSignedIndexSignedIndexFormat;
constexpr const char *CloudFloors::kYSignedIndexSignedIndexFormat;
constexpr const char *CloudFloors::kX36Name;
constexpr const char *CloudFloors::kY36Name;
constexpr const char *CloudFloors::kPathDXKey;
constexpr const char *CloudFloors::kPathDYKey;
constexpr const char *CloudFloors::kPaigeName;
constexpr const char *CloudFloors::kReaderName;
constexpr const char *CloudFloors::kSamName;
constexpr const char *CloudFloors::kLadderHSWKey;
constexpr const char *CloudFloors::kLadderHSHKey;
constexpr const char *CloudFloors::kCorrectSfxIDKey;
constexpr const char *CloudFloors::kCongratsCharIDSignedIndexKey;
constexpr const char *CloudFloors::kCongratsSpchIDKey;
constexpr const char *CloudFloors::kMaxOopsSpeechesKey;
constexpr const char *CloudFloors::kOopsCharIDSignedIndexKey;
constexpr const char *CloudFloors::kOopsSpchIDKey;
constexpr const char *CloudFloors::kCA1SFX10IDSignedIndexKey;
constexpr const char *CloudFloors::kPropertyVisible;
constexpr const char *CloudFloors::kReachSfxIDSignedIndexKey;
constexpr const char *CloudFloors::kAlreadySpchID1Key;
constexpr const char *CloudFloors::kAlreadySpchID2Key;
constexpr const char *CloudFloors::kMaxMarbleSpeechesKey;
constexpr const char *CloudFloors::kHaveMarbleSpchIDSignedIndexKey;
constexpr const char *CloudFloors::kDropSfxIDKey;
constexpr const char *CloudFloors::kGoHomeSfxIDKey;
constexpr const char *CloudFloors::kItemSignedIndexFormat;
constexpr const char *CloudFloors::kCOMPickUpSfxIDKey;
constexpr const char *CloudFloors::kR1CA1MarbleClickKey;
constexpr const char *CloudFloors::kCOMSomeUseIDKey;
constexpr const char *CloudFloors::kINV1Marble5Key;
constexpr const char *CloudFloors::kLadderCharID1Key;
constexpr const char *CloudFloors::kLadderSpchID1Key;
constexpr const char *CloudFloors::kLadderSpchID1SignedIndexKey;
constexpr const char *CloudFloors::kLadderCharID2Key;
constexpr const char *CloudFloors::kLadderSpchID2Key;
constexpr const char *CloudFloors::kINV1LadderKey;
constexpr const char *CloudFloors::kPickUpSfxIDKey;
constexpr const char *CloudFloors::kCWSpchIDSignedIndexKey;
constexpr const char *CloudFloors::kPaigeHelpIDKey;
constexpr const char *CloudFloors::kMaxMainCharIndexKey;
constexpr const char *CloudFloors::kSignedIndexSignedIndexFormat;
constexpr const char *CloudFloors::kMainCharIDKey;
constexpr const char *CloudFloors::kMainSpchIDKey;
constexpr const char *CloudFloors::kDoneSpchIDKey;
constexpr const char *CloudFloors::kCA1SFX07IDKey;
constexpr const char *CloudFloors::kThanksSpchID1Key;
constexpr const char *CloudFloors::kThanksSpchID2Key;
constexpr const char *CloudFloors::kMarble5IDKey;
constexpr const char *CloudFloors::kMaxIdleIndexKey;
constexpr const char *CloudFloors::kIdleCharIDSignedIndexKey;
constexpr const char *CloudFloors::kIdleSpchIDKey;
constexpr const char *CloudFloors::kPropertyText;
constexpr const char *CloudFloors::kPropertyX;
constexpr const char *CloudFloors::kPropertyY;
constexpr const char *CloudFloors::kTextOffsetYKey;

constexpr const char *CloudFloors::kTextFormat;
constexpr const char *CloudFloors::kDigitFormat;
constexpr const char *CloudFloors::kFont;
constexpr const char *CloudFloors::kDataXML;

CloudFloors::CloudFloors(PageScene &host) : _host(host), _paths(host._vm.resources()) {}
CloudFloors::~CloudFloors() {
	releaseCursor();
	for (auto &cursor : _cursors)
		cursor.free();
	for (auto &answer : _answers)
		delete answer.text;
	for (auto &floor : _slots) {
		for (auto &slot : floor)
			delete slot.tile.text;
	}
}

int CloudFloors::number(const Common::String &key) const { return _host._page->number(key); }

int CloudFloors::indexed(const char *key, int i) const { return number(Common::String::format(kNameSignedIndexFormat, key, i)); }

void CloudFloors::say(const char *actor, const Common::String &key) { speech(actor, number(key)); }

void CloudFloors::speech(const Common::String &actor, uint resource) {
	_host.delaySpeech(1);
	_host.say(actor, resource);
	_host.delaySpeech(1);
}

void CloudFloors::speeches(const Common::String &actors, const Common::String &resources, uint count) {
	for (uint i = 1; i <= count; i++) {
		Common::String suffix = Common::String::format(kUnsignedIndexFormat, i);
		speech(_host._page->text(actors + suffix), number(resources + suffix));
	}
}

void CloudFloors::sound(const Common::String &key) { _host.playSounds({static_cast<uint16>(number(key))}); }

Shell::Sprite *CloudFloors::sprite(const Common::String &key, int depth) {
	return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), depth);
}

Common::Rect CloudFloors::bounds(Shell::Sprite *sprite) {
	return Common::Rect(sprite->x, sprite->y, sprite->x + sprite->width(), sprite->y + sprite->height());
}

bool CloudFloors::worker(int pose) {
	if (_host._actors.contains(kPropertyWorker)) {
		delete _host._actors[kPropertyWorker];
		_host._actors.erase(kPropertyWorker);
	}
	Common::ScopedPtr<Shell::Character> actor(new Shell::Character(_host._scene, _host._vm.mixer(), _host._vm.resources(), *_host._vm._rnd, _host));
	if (!actor->create(indexed(kCWSetIDKey, pose), number(kCWZKey)))
		return false;
	actor->movable = false;
	for (const char *key : {kRegisterFidgetsToSettleName, kRegisterSpeechToSettleName, kLoadSpeechAnimsInBkgdName, kAnimateSettledName})
		actor->set(key, {}, Core::ScriptValue(0));
	Core::ScriptValue result;
	actor->play(kMethodSetSpeechAnims, {Core::ScriptValue(indexed(kCWLipsIDKey, pose)), Core::ScriptValue(1), Core::ScriptValue(1),
						 Core::ScriptValue(indexed(kCWBodyIDKey, pose)), Core::ScriptValue(0), Core::ScriptValue(0)}, result);
	actor->play(kMethodAddFidgetGroup, {Core::ScriptValue(1), Core::ScriptValue(5), Core::ScriptValue(10)}, result);
	actor->play(kMethodAddFidget, {Core::ScriptValue(1), Core::ScriptValue(indexed(kCWSetIDKey, pose))}, result);
	actor->play(kMethodAddFidgetGroup, {Core::ScriptValue(2), Core::ScriptValue(15), Core::ScriptValue(30)}, result);
	for (int i = 1; i <= (pose == 1 ? 3 : 2); i++)
		actor->play(kMethodAddFidget, {Core::ScriptValue(2), Core::ScriptValue(number(Common::String::format(kCWFidgIDSignedIndexSignedIndexKey, pose, i)))}, result);
	_host._actors[kPropertyWorker] = actor.release();
	return true;
}

bool CloudFloors::loadList(int resource, Common::StringArray &list) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_host._vm.resources().open(Common::String::format(kTextFormat, resource)));
	if (!stream || 65536 < stream->size())
		return false;
	Common::String line;
	while (stream->pos() < stream->size()) {
		char c = stream->readByte();
		if (line.empty() && c == '*')
			return !list.empty();
		if (c == '\r') {
			list.push_back(line);
			line.clear();
		} else if (c && c != '\n') {
			line += c;
		}
	}
	return false;
}

int CloudFloors::index(const Common::StringArray &list, const Common::String &word) {
	for (uint i = 0; i < list.size(); i++) {
		if (word.equalsIgnoreCase(list[i]))
			return i;
	}
	return -1;
}

void CloudFloors::sayWord(const Common::String &word) {
	int at = index(_words, word);
	if (0 <= at)
		speech(kPropertyWorker, number(kCA1WordsIDKey) + at);
}

bool CloudFloors::createTile(Tile &tile, const Common::String &value, int length, Common::Point position, int depth) {
	tile.sprite = sprite(kTileBoxIDKey, depth);
	if (!tile.sprite)
		return false;
	tile.sprite->setFrame(length);
	tile.sprite->x = position.x;
	tile.sprite->y = position.y;
	tile.home = position;
	tile.value = value;
	tile.text = new Shell::TextObject(_paths, _host._vm.fonts(), _font);
	tile.text->create({Core::ScriptValue::fromText(" "), Core::ScriptValue(0), Core::ScriptValue(0), Core::ScriptValue(1)});
	tile.text->set(kPropertyCenter, {}, Core::ScriptValue(1));
	tile.text->set(kPropertyColor, {Core::ScriptValue(6317), Core::ScriptValue(4212)}, Core::ScriptValue(2106));
	return true;
}

void CloudFloors::deleteTile(Tile &tile) {
	_host._scene.remove(tile.sprite);
	delete tile.text;
}

void CloudFloors::clearAnswers() {
	for (auto &answer : _answers)
		deleteTile(answer);
	_answers.clear();
	_dragged = -1;
}

void CloudFloors::clearFloor(int floor) {
	for (auto &slot : _slots[floor])
		deleteTile(slot.tile);
	_slots[floor].clear();
	_targets[floor].clear();
	_picturesAO[floor]->visible = false;
}

bool CloudFloors::enter() {
	for (uint i = 0; i < ARRAYSIZE(_cursors); i++) {
		Common::String path = Common::String::format(kDigitFormat, 4061 + i);
		const Core::Animation *animation = _host._scene.loadAnimation(path);
		if (!animation || !animation->picture(0))
			return false;
		_cursors[i].copyFrom(*animation->picture(0));
	}
	_font = _host._vm.fonts().load(kFont);
	if (!_font || !_data.loadStream(_host._vm.resources().open(kDataXML)) || !_data.parse() ||
		!loadList(number(kTEXTCA1WTILESName), _pictures) || !loadList(number(kTEXTCA1WORDSName), _words))
		return false;
	World &world = _host._world;
	_level = CLIP(world.get(kR1CA1LevelKey), 1, 3);
	_dataset = world.get(kR1CA1DatasetKey);
	_set = CLIP(world.get(kR1CA1SetKey), 1, indexed(kNumSetsKey, _level));
	if (_dataset < 1 || indexed(kMaxDataSetsKey, _level) < _dataset) {
		_dataset = _host._vm._rnd->getRandomNumberRng(1, indexed(kMaxDataSetsKey, _level));
		_set = _host._vm._rnd->getRandomNumberRng(1, indexed(kNumSetsKey, _level));
	}
	_state = CLIP(world.get(kR1CA1StateKey), 1, 3);
	_brickCount = CLIP(world.get(kR1CA1RoundKey), 0, 3);
	_ladderHere = !world.get(kR1CA1StateKey) || world.get(kR1CA1LadderHereKey);
	if (_host._practice) {
		_state = 2;
		_ladderHere = false;
		if (_host._chester)
			_host._chester->visible = false;
	}
	if (!worker(_state == 1 ? 1 : 2))
		return false;
	_highlight = sprite(kCloudHiliteIDKey, 5);
	for (int i = 0; i < 3; i++) {
		_belts[i] = _host._scene.add(_host.characterResource(number(kBelt1IDKey) + i, Shell::Character::Resources::kAnimation), 3);
		_masks[i] = sprite(Common::String::format(kMask1IDSignedIndexKey, i + 1), 4);
		_pieces[i] = sprite(Common::String::format(kCloudPieceIDSignedIndexKey, i + 1), 17);
		_picturesAO[i] = sprite(kWordTileIDKey, 5);
		_bricks[i] = sprite(kBrickIDKey, indexed(kCloudBrickZKey, i + 1));
		_reaches[i] = sprite(Common::String::format(kReachIDSignedIndexKey, i + 1), 75);
		if (!_belts[i] || !_masks[i] || !_pieces[i] || !_picturesAO[i] || !_bricks[i] || !_reaches[i])
			return false;
		_picturesAO[i]->visible = _reaches[i]->visible = false;
		_bricks[i]->x = indexed(kCloudBrickXName, i + 1);
		_bricks[i]->y = number(kCloudBrickYName);
		_bricks[i]->visible = i < _brickCount;
	}
	for (int i = 0; i < number(kNumElevatorsKey); i++) {
		if (!_host._scene.add(_host.characterResource(number(kElevator0IDKey) + i, Shell::Character::Resources::kAnimation), 12))
			return false;
	}
	if (!sprite(kCatcherIDKey, 12))
		return false;
	_brick = sprite(kBrickIDKey, 7);
	_ladder = sprite(kLadderIDKey, 60);
	_marble = sprite(kMarbleIDKey, 100);
	_descend = sprite(kDownLadderIDKey, 75);
	_toss = sprite(kTossIDKey, 75);
	if (!_brick || !_highlight || !_ladder || !_marble || !_descend || !_toss)
		return false;
	_brick->visible = _descend->visible = _toss->visible = false;
	_ladder->visible = _ladderHere;
	_marble->visible = _state == 3;
	_marble->colorRemap = PageScene::marbleColor(4);
	_marble->x = number(kMarbleXKey);
	_marble->y = number(kMarbleYKey);
	if (_host._practice) {
		for (int i = 1; i <= 3; i++)
			say(kPropertyWorker, Common::String::format(kPEnterSpchIDSignedIndexKey, i));
	} else if (world.visits(Page::kPageCA1) == 0) {
		speeches(kEnterCharIDKey, kEnterSpchIDKey, number(kMaxEntrySpchsKey));
	} else if (_state == 1) {
		say(kPropertyWorker, Common::String::format(kReEnterSpchIDs1SignedIndexKey, _host._vm._rnd->getRandomNumberRng(1, 2)));
	} else if (world.get(kR1CA1ReentryKey)) {
		say(kPropertyWorker, kReEnterSpchIDs21Key);
		world.set(kR1CA1ReentryKey, 0);
	}
	return setup();
}

void CloudFloors::persist() {
	if (_host._practice)
		return;
	_host._world.set(kR1CA1StateKey, _state);
	_host._world.set(kR1CA1RoundKey, _brickCount);
	_host._world.set(kR1CA1LevelKey, _level);
	_host._world.set(kR1CA1DatasetKey, _dataset);
	_host._world.set(kR1CA1SetKey, _set);
	_host._world.set(kR1CA1LadderHereKey, _ladderHere);
}

bool CloudFloors::setup() {
	Common::StringTokenizer tokens(_data.get(Common::String::format(kPropertyDataSetLevelSignedIndexWordListSignedIndexSetSignedIndex, _level, _dataset, _set)));
	Common::StringArray words;
	// The script reads a fixed field count; some US records have an extra trailing zero.
	while (!tokens.empty() && words.size() < static_cast<uint>(indexed(kNumWordsKey, _level)))
		words.push_back(tokens.nextToken());
	if (words.size() != static_cast<uint>(indexed(kNumWordsKey, _level)))
		return false;
	Common::String pattern = words[0];
	Common::String target;
	Common::StringArray parts;
	uint missing = 1;
	int letters = 0;
	for (char c : pattern) {
		Common::String part(c);
		if (c == '1' || c == '2') {
			if (words.size() <= missing || words[missing].size() != static_cast<uint>(c - '0'))
				return false;
			part = words[missing++];
		}
		parts.push_back(part);
		target += part;
	}
	if (target.size() < 3 || 7 < target.size() || index(_pictures, target) < 0 || index(_words, target) < 0)
		return false;
	for (uint i = 1; i < words.size(); i++) {
		if (!words[i].equals("0")) {
			if (words[i].empty() || 2 < words[i].size())
				return false;
			letters += words[i].size();
		}
	}
	if (letters < 5 || 8 < letters)
		return false;
	clearAnswers();
	if (_floor == 1) {
		for (int i = 0; i < 3; i++)
			clearFloor(i);
	} else {
		clearFloor(_floor - 1);
	}
	int frame = target.size() - 2;
	_masks[_floor - 1]->setFrame(frame);
	_pieces[_floor - 1]->setFrame(frame);
	_masks[_floor - 1]->visible = true;
	_highlight->setFrame(_floor);
	_highlight->visible = true;
	Shell::Sprite *picture = _picturesAO[_floor - 1];
	picture->setFrame(index(_pictures, target) + 1);
	Common::Point center = bounds(_highlight).center();
	picture->x = center.x - picture->width() / 2;
	picture->y = center.y - picture->height() / 2;
	picture->visible = true;
	_targets[_floor - 1] = target;
	int x = number(Common::String::format(kTargetXSignedIndexSignedIndexKey, _floor, frame));
	int y = indexed(kTargetYKey, _floor);
	for (uint i = 0; i < parts.size(); i++) {
		Slot slot;
		slot.expected = parts[i];
		slot.locked = pattern[i] != '1' && pattern[i] != '2';
		if (!createTile(slot.tile, slot.locked ? parts[i] : Common::String(), parts[i].size(), Common::Point(x, y), 22))
			return false;
		_slots[_floor - 1].push_back(slot);
		x += parts[i].size() == 2 ? 55 : 28;
	}
	for (uint i = 1; i < words.size(); i++) {
		if (words[i].equals("0"))
			continue;
		Tile tile;
		if (!createTile(tile, words[i], words[i].size(), Common::Point(), 27))
			return false;
		_answers.push_back(tile);
	}
	Common::Array<int> order;
	for (uint i = 0; i < _answers.size(); i++)
		order.push_back(i);
	bool adjacent;
	do {
		for (uint i = 0; i <= order.size() / 2; i++) {
			int a = _host._vm._rnd->getRandomNumberRng(0, order.size() - 1);
			int b = _host._vm._rnd->getRandomNumberExcluding(order.size(), a);
			SWAP(order[a], order[b]);
		}
		adjacent = false;
		for (uint i = 1; i < order.size(); i++)
			adjacent |= order[i - 1] + 1 == order[i];
	} while (adjacent);
	x = indexed(kAnswerStartXKey, letters);
	for (int i : order) {
		_answers[i].home = Common::Point(x, number(kAnswerYKey));
		home(i);
		x += _answers[i].value.size() == 2 ? 57 : 30;
	}
	_mistakes = 0;
	_moving = false;
	_phase = kTargetSpeech;
	sayWord(target);
	persist();
	return true;
}

void CloudFloors::assess(bool correct) {
	if (_host._practice)
		return;
	World &world = _host._world;
	int outcome = correct ? 1 : 3;
	int streak = world.get(kR1CA1StreakKindKey) == outcome ? world.get(kR1CA1StreakKey) + 1 : 1;
	world.recordAssessment(kR1CA1Key, _level, outcome, 3, 3, streak);
	world.set(kR1CA1StreakKindKey, outcome);
	world.set(kR1CA1StreakKey, streak == 3 ? 0 : streak);
	int previous = _level;
	if (3 <= streak && !world.get(kR1CA1FixedLevelKey))
		_level = CLIP(_level + (correct ? 1 : -1), 1, 3);
	if (previous != _level) {
		_dataset = _host._vm._rnd->getRandomNumberRng(1, indexed(kMaxDataSetsKey, _level));
		_set = _host._vm._rnd->getRandomNumberRng(1, indexed(kNumSetsKey, _level));
	}
}

void CloudFloors::initPath() {
	_end = indexed(kEndPtKey, _floor);
	for (int i = 0; i <= _end; i++)
		_points[i] = Common::Point(number(Common::String::format(kXSignedIndexSignedIndexFormat, _floor, i + 1)),
								   number(Common::String::format(kYSignedIndexSignedIndexFormat, _floor, i + 1)));
	if (_floor == 3 && _brickCount == 3)
		_points[_end] = Common::Point(number(kX36Name), number(kY36Name));
	_correctRoute = false;
	_nextTick = _host._time + 42;
	segment(0, 1);
	for (auto *belt : _belts) {
		if (!belt->isRunning())
			belt->startAnimation(-1);
	}
	_phase = kTravelling;
	_idle = _host._time;
}

void CloudFloors::segment(int from, int to) {
	_from = from;
	_to = to;
	_brick->x = _points[from].x;
	_brick->y = _points[from].y;
	_brick->visible = true;
	_moving = true;
}

void CloudFloors::advance(uint32 time) {
	updateCursor();
	if (_host._nextSpeech < _host._speech.size() || (_host._speaking && _host._speaking->isBusy()))
		_idle = time;
	if (!_moving || static_cast<int32>(time - _nextTick) < 0)
		return;
	_nextTick += 42;
	if (21 < static_cast<int32>(time - _nextTick))
		_nextTick = time + 42;
	Common::Point start = _points[_from];
	Common::Point end = _points[_to];
	bool reached;
	if (start.y == end.y) {
		_brick->x += start.x < end.x ? number(kPathDXKey) : -number(kPathDXKey);
		reached = start.x < end.x ? end.x <= _brick->x : _brick->x <= end.x;
	} else {
		_brick->y += start.y < end.y ? number(kPathDYKey) : -number(kPathDYKey);
		reached = start.y < end.y ? end.y <= _brick->y : _brick->y <= end.y;
	}
	if (!reached)
		return;
	if (_to == _end) {
		_moving = false;
		_brick->visible = false;
		finish();
	} else if (_to == 1) {
		_moving = false;
		checkpoint();
	} else if (_to == 0) {
		segment(0, 1);
	} else if (_to == _end - 1) {
		if (_end == 3)
			segment(0, 1);
		else
			segment(_to, 0);
	} else {
		segment(_to, _to + 1);
	}
}

void CloudFloors::releaseCursor() {
	if (0 <= _cursor) {
		CursorMan.popCursor();
		_cursor = -1;
	}
}

void CloudFloors::updateCursor() {
	if (_paused || _host._chestOpen) {
		releaseCursor();
		return;
	}
	int cursor = 0;
	if (0 <= _dragged) {
		cursor = 4;
	} else if (_phase == kTravelling && !_correctRoute) {
		for (auto &tile : _answers) {
			if (bounds(tile.sprite).contains(_mouse) && (tile.slot < 0 || !_slots[_floor - 1][tile.slot].locked))
				cursor = 3;
		}
		if (cursor == 0) {
			for (const char *name : {kPropertyWorker, kPaigeName, kReaderName, kSamName}) {
				if (_host._actors[name]->visible && _host._actors[name]->bounds().contains(_mouse))
					cursor = 1;
			}
			for (int i = 0; i < _floor; i++) {
				if (_picturesAO[i]->visible && bounds(_picturesAO[i]).contains(_mouse))
					cursor = 1;
				for (auto &slot : _slots[i]) {
					if (slot.locked && bounds(slot.tile.sprite).contains(_mouse))
						cursor = 1;
				}
			}
		}
		if ((_marble->visible && _marble->covers(_mouse)) ||
			(_ladderHere && Common::Rect(_ladder->x, _ladder->y, _ladder->x + number(kLadderHSWKey), _ladder->y + number(kLadderHSHKey)).contains(_mouse)) ||
			(_host._chester && _host._chester->visible && _host._chester->covers(_mouse)))
			cursor = 2;
		Shell::Sprite *map = _host._scene.hitSprite(_mouse, 100, 100, true);
		if (map && map->action == 1)
			cursor = 2;
	}
	if (cursor == _cursor)
		return;
	static const Common::Point hotspots[] = {
		Common::Point(1, 2),
		Common::Point(1, 1),
		Common::Point(7, 1),
		Common::Point(7, 5),
		Common::Point(11, 6),
	};
	if (_cursor < 0)
		CursorMan.pushCursor(_cursors[cursor], hotspots[cursor].x, hotspots[cursor].y, 0);
	else
		CursorMan.replaceCursor(_cursors[cursor], hotspots[cursor].x, hotspots[cursor].y, 0);
	_cursor = cursor;
}

void CloudFloors::checkpoint() {
	int occupied = 0;
	int correct = 0;
	Common::String word;
	for (auto &slot : _slots[_floor - 1]) {
		Common::String value = slot.tile.value;
		if (0 <= slot.answer)
			value = _answers[slot.answer].value;
		if (!value.empty())
			occupied += 1;
		if (value.equals(slot.expected)) {
			slot.locked = true;
			correct += 1;
		}
		word += value;
	}
	if (occupied != static_cast<int>(_slots[_floor - 1].size())) {
		fall();
		return;
	}
	_host.interrupt();
	if (occupied == correct) {
		if (!_mistakes)
			assess(true);
		sayWord(word);
		sound(kCorrectSfxIDKey);
		if (_floor == 3) {
			int i = _host._vm._rnd->getRandomNumberRng(1, 5);
			speech(_host._page->text(Common::String::format(kCongratsCharIDSignedIndexKey, i)), indexed(kCongratsSpchIDKey, i));
		}
		_phase = kCorrect;
	} else {
		_mistakes += 1;
		assess(false);
		if (_level < 3)
			sayWord(word);
		_phase = kWrongWord;
	}
}

void CloudFloors::reject() {
	for (auto &slot : _slots[_floor - 1]) {
		if (!slot.locked && 0 <= slot.answer) {
			home(slot.answer);
			slot.answer = -1;
		}
	}
	_alternateOops = !_alternateOops;
	if (_alternateOops) {
		_oops = _oops % number(kMaxOopsSpeechesKey) + 1;
		speech(_host._page->text(Common::String::format(kOopsCharIDSignedIndexKey, _oops)), indexed(kOopsSpchIDKey, _oops));
	}
	sayWord(_targets[_floor - 1]);
	_phase = kWrongTarget;
}

void CloudFloors::fall() {
	sound(Common::String::format(kCA1SFX10IDSignedIndexKey, _floor));
	segment(1, 2);
	_phase = kTravelling;
}

void CloudFloors::finish() {
	_host.interrupt();
	if (_floor != 3 || _host._practice) {
		next();
		return;
	}
	_brickCount = MIN(3, _brickCount + 1);
	if (_state == 1) {
		// The third brick earns the ladder even if a save interrupts the descent.
		if (_brickCount == 3)
			_state = 2;
		_host._actors[kPropertyWorker]->set(kPropertyVisible, {}, Core::ScriptValue(0));
		_reaches[_brickCount - 1]->visible = true;
		_reaches[_brickCount - 1]->startAnimation();
		sound(Common::String::format(kReachSfxIDSignedIndexKey, _brickCount));
		_phase = kReach;
	} else if (_state == 3) {
		say(kPropertyWorker, kAlreadySpchID1Key);
		say(kPropertyWorker, kAlreadySpchID2Key);
		_phase = kNext;
	} else {
		_state = 3;
		_marbleSpeech = _marbleSpeech % number(kMaxMarbleSpeechesKey) + 1;
		say(kPropertyWorker, Common::String::format(kHaveMarbleSpchIDSignedIndexKey, _marbleSpeech));
		_phase = kRewardSpeech;
	}
	persist();
}

void CloudFloors::next() {
	for (auto &slot : _slots[_floor - 1]) {
		slot.tile.value = slot.expected;
		slot.answer = -1;
	}
	_floor = _floor % 3 + 1;
	_dataset += 1;
	if (indexed(kMaxDataSetsKey, _level) < _dataset) {
		_dataset = 1;
		_set = _set % indexed(kNumSetsKey, _level) + 1;
	}
	if (!setup())
		warning("RR1: cannot load CA1 level %d dataset %d set %d", _level, _dataset, _set);
}

void CloudFloors::home(int answer) {
	Tile &tile = _answers[answer];
	tile.sprite->x = tile.home.x;
	tile.sprite->y = tile.home.y;
	tile.slot = -1;
	_host._scene.setDepth(tile.sprite, 27);
}

void CloudFloors::drop() {
	Tile &tile = _answers[_dragged];
	int best = -1;
	int area = 0;
	for (uint i = 0; i < _slots[_floor - 1].size(); i++) {
		Slot &slot = _slots[_floor - 1][i];
		if (slot.locked || slot.expected.size() != tile.value.size())
			continue;
		Common::Rect overlap = bounds(slot.tile.sprite);
		if (!overlap.intersects(bounds(tile.sprite)))
			continue;
		overlap.clip(bounds(tile.sprite));
		int size = overlap.width() * overlap.height();
		if (area < size) {
			area = size;
			best = i;
		}
	}
	if (0 <= best) {
		Slot &slot = _slots[_floor - 1][best];
		if (0 <= slot.answer)
			home(slot.answer);
		slot.answer = _dragged;
		tile.slot = best;
		tile.sprite->x = slot.tile.home.x;
		tile.sprite->y = slot.tile.home.y;
		_host._scene.setDepth(tile.sprite, 27);
		sound(kDropSfxIDKey);
	} else {
		home(_dragged);
		sound(kGoHomeSfxIDKey);
	}
	_dragged = -1;
}

void CloudFloors::collect(const char *item) {
	Common::String key = Common::String::format(kItemSignedIndexFormat, number(item));
	_host._world.set(key, MIN(25, _host._world.get(key) + 1));
	_host.putAway();
	sound(kCOMPickUpSfxIDKey);
	persist();
}

bool CloudFloors::click(Common::Point point) {
	if (_phase != kTravelling) {
		_host.interrupt();
		return true;
	}
	_idle = _host._time;
	if (_marble->visible && _marble->covers(point)) {
		_state = 2;
		_marble->visible = false;
		if (!_host._world.get(kR1CA1MarbleClickKey)) {
			_host._world.set(kR1CA1MarbleClickKey, 1);
			say(kSamName, kCOMSomeUseIDKey);
		}
		collect(kINV1Marble5Key);
		return true;
	}
	if (_ladderHere && Common::Rect(_ladder->x, _ladder->y, _ladder->x + number(kLadderHSWKey), _ladder->y + number(kLadderHSHKey)).contains(point)) {
		if (_state == 1) {
			_ladderClicks = _ladderClicks == 3 ? 2 : _ladderClicks + 1;
			if (_ladderClicks == 1)
				speeches(kLadderCharID1Key, kLadderSpchID1Key, 2);
			else
				say(kPropertyWorker, Common::String::format(kLadderSpchID1SignedIndexKey, _ladderClicks + 1));
		} else {
			speeches(kLadderCharID2Key, kLadderSpchID2Key, 2);
			_ladderHere = _ladder->visible = false;
			collect(kINV1LadderKey);
		}
		return true;
	}
	for (uint i = 0; i < _answers.size(); i++) {
		Tile &tile = _answers[i];
		if (!bounds(tile.sprite).contains(point) || (0 <= tile.slot && _slots[_floor - 1][tile.slot].locked))
			continue;
		if (0 <= tile.slot)
			_slots[_floor - 1][tile.slot].answer = -1;
		tile.slot = -1;
		_dragged = i;
		_dragOffset = point - Common::Point(tile.sprite->x, tile.sprite->y);
		_host._scene.setDepth(tile.sprite, 100);
		sound(kPickUpSfxIDKey);
		sayWord(tile.value);
		return true;
	}
	if (_host._actors[kPropertyWorker]->bounds().contains(point)) {
		_workerClicks = _workerClicks % 4 + 1;
		say(kPropertyWorker, Common::String::format(kCWSpchIDSignedIndexKey, _workerClicks));
		return true;
	}
	if (_host._actors[kPaigeName]->bounds().contains(point)) {
		_paigeClicks = (_paigeClicks + 1) % 3;
		speech(kPaigeName, number(kPaigeHelpIDKey) + _paigeClicks);
		return true;
	}
	if (!_host._practice && (_host._actors[kReaderName]->bounds().contains(point) || _host._actors[kSamName]->bounds().contains(point))) {
		int group = _ladderHere && _host._world.visits(Page::kPageIG1) ? 1 : 2;
		_mainClicks = _mainClicks % indexed(kMaxMainCharIndexKey, group) + 1;
		Common::String suffix = Common::String::format(kSignedIndexSignedIndexFormat, group, _mainClicks);
		speech(_host._page->text(kMainCharIDKey + suffix), number(kMainSpchIDKey + suffix));
		return true;
	}
	for (int i = 0; i < _floor; i++) {
		if (_picturesAO[i]->visible && bounds(_picturesAO[i]).contains(point)) {
			sayWord(_targets[i]);
			return true;
		}
		for (auto &slot : _slots[i]) {
			if (slot.locked && bounds(slot.tile.sprite).contains(point)) {
				sayWord(_targets[i]);
				return true;
			}
		}
	}
	return false;
}

bool CloudFloors::event(const Common::Event &event) {
	if (event.type == Common::EVENT_MOUSEMOVE || event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP)
		_mouse = event.mouse;
	if (_dragged < 0)
		return false;
	if (event.type == Common::EVENT_MOUSEMOVE || event.type == Common::EVENT_LBUTTONUP) {
		Tile &tile = _answers[_dragged];
		tile.sprite->x = event.mouse.x - _dragOffset.x;
		tile.sprite->y = event.mouse.y - _dragOffset.y;
		if (event.type == Common::EVENT_LBUTTONUP)
			drop();
		return true;
	}
	return event.type == Common::EVENT_LBUTTONDOWN;
}

void CloudFloors::update(uint32 time) {
	if (_host._nextSpeech < _host._speech.size())
		return;
	switch (_phase) {
	case kTargetSpeech:
		initPath();
		break;
	case kWrongWord:
		reject();
		break;
	case kWrongTarget:
		fall();
		break;
	case kCorrect:
		_masks[_floor - 1]->visible = false;
		_correctRoute = true;
		segment(1, _end);
		_phase = kTravelling;
		break;
	case kReach:
		if (_reaches[_brickCount - 1]->isRunning())
			break;
		_reaches[_brickCount - 1]->visible = false;
		_bricks[_brickCount - 1]->visible = true;
		_host._actors[kPropertyWorker]->set(kPropertyVisible, {}, Core::ScriptValue(1));
		if (_brickCount == 3) {
			say(kPropertyWorker, kDoneSpchIDKey);
			_phase = kDoneSpeech;
		} else {
			next();
		}
		break;
	case kDoneSpeech:
		_host._actors[kPropertyWorker]->set(kPropertyVisible, {}, Core::ScriptValue(0));
		_descend->visible = true;
		_descend->startAnimation();
		sound(kCA1SFX07IDKey);
		_phase = kDescend;
		break;
	case kDescend:
		if (_descend->isRunning())
			break;
		_state = 2;
		_descend->visible = false;
		worker(2);
		say(kPropertyWorker, kThanksSpchID1Key);
		say(kPropertyWorker, kThanksSpchID2Key);
		persist();
		_phase = kNext;
		break;
	case kRewardSpeech:
		_host._actors[kPropertyWorker]->set(kPropertyVisible, {}, Core::ScriptValue(0));
		_toss->visible = true;
		_toss->startAnimation();
		sound(kTossIDKey);
		_phase = kToss;
		break;
	case kToss:
		if (_toss->isRunning())
			break;
		_toss->visible = false;
		_marble->visible = true;
		_marble->setFrame(3);
		_host._actors[kPropertyWorker]->set(kPropertyVisible, {}, Core::ScriptValue(1));
		_host.queueSound(number(kMarble5IDKey));
		_phase = kMarbleFlash;
		break;
	case kMarbleFlash:
		_marble->setFrame(1);
		next();
		break;
	case kNext:
		next();
		break;
	case kTravelling:
		if (!_host._practice && !_correctRoute && _dragged < 0 && 30000 <= time - _idle) {
			_idle = time;
			_idleIndex = _idleIndex % number(kMaxIdleIndexKey) + 1;
			_host.say(_host._page->text(Common::String::format(kIdleCharIDSignedIndexKey, _idleIndex)), indexed(kIdleSpchIDKey, _idleIndex));
		}
		break;
	default:
		break;
	}
}

void CloudFloors::drawTile(Tile &tile, OM::Canvas &canvas) {
	if (!tile.sprite->visible || tile.value.empty())
		return;
	Common::String value = tile.value;
	if (value.size() == 2)
		value = value.substr(0, 1) + "  " + value.substr(1);
	Common::Point center = bounds(tile.sprite).center();
	tile.text->set(kPropertyText, {}, Core::ScriptValue::fromText(value));
	tile.text->set(kPropertyX, {}, Core::ScriptValue(center.x));
	tile.text->set(kPropertyY, {}, Core::ScriptValue(center.y - number(kTextOffsetYKey)));
	tile.text->draw(canvas);
}

void CloudFloors::draw(OM::Canvas &canvas) {
	_host._scene.drawBackground(canvas);
	_host._scene.drawSprites(canvas, INT_MIN, 27);
	for (auto &floor : _slots) {
		for (auto &slot : floor)
			drawTile(slot.tile, canvas);
	}
	for (uint i = 0; i < _answers.size(); i++) {
		if (static_cast<int>(i) != _dragged)
			drawTile(_answers[i], canvas);
	}
	_host._scene.drawSprites(canvas, 28, 100);
	if (0 <= _dragged)
		drawTile(_answers[_dragged], canvas);
	_host._scene.drawSprites(canvas, 101, INT_MAX);
}

void CloudFloors::pause(bool paused) {
	_paused = paused;
	if (paused)
		releaseCursor();
	if (paused) {
		_pausedSprites.clear();
		for (auto *sprite : {_belts[0], _belts[1], _belts[2], _reaches[0], _reaches[1], _reaches[2], _descend, _toss}) {
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
