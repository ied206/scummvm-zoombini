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

#include "trinket/rr1cloud9/la2_music_patterns.h"

#include "common/tokenizer.h"
#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

MusicPatterns::~MusicPatterns() {
	for (auto *note : _notes)
		delete note;
}

int MusicPatterns::number(const Common::String &key) const { return _host._page->number(key); }

int MusicPatterns::indexed(const char *key, int index) const { return number(Common::String::format("%s.%d", key, index)); }

Shell::Sprite *MusicPatterns::sprite(const Common::String &key, int depth) {
	return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), depth);
}

void MusicPatterns::say(const char *actor, const Common::String &key) { speech(actor, number(key)); }

void MusicPatterns::speech(const Common::String &actor, uint resource) {
	_host.delaySpeech(1);
	_host.say(actor, resource);
	_host.delaySpeech(1);
}

void MusicPatterns::speeches(const Common::String &actors, const Common::String &resources, uint count) {
	for (uint i = 1; i <= count; i++) {
		Common::String suffix = Common::String::format(".%u", i);
		speech(_host._page->text(actors + suffix), number(resources + suffix));
	}
}

void MusicPatterns::sound(const Common::String &key) { _host.playSounds({static_cast<uint16>(number(key))}); }

void MusicPatterns::group(const char *prefix, int index, int count) {
	speeches(Common::String::format("k%sCharIDs.%d", prefix, index), Common::String::format("k%sSpchIDs.%d", prefix, index), count);
}

bool MusicPatterns::enter() {
	if (!_data.loadStream(_host._vm.resources().open("r1la2.xml")) || !_data.parse() || !_host.actor("grid", "kLocal", false))
		return false;
	_host._actors["grid"]->set("z", {}, Core::ScriptValue(number("kGridZ")));
	_host._actors["grid"]->set("animateSettled", {}, Core::ScriptValue(0));
	World &world = _host._world;
	_level = CLIP(world.get("R1LA2Level"), 1, 3);
	_dataset = CLIP(world.get("R1LA2Dataset"), 1, 9);
	_count = CLIP(world.get("R1LA2Count"), 0, 1000000);
	_state = CLIP(world.get("R1LA2State"), 3, 5);
	_pillowHere = !world.get("R1LA2State") || world.get("R1LA2PillowHere");
	for (int row = 0; row < 5; row++) {
		_bank[row] = CLIP(world.get(Common::String::format("R1LA2Marble%dCount", row + 1)), 0, 8);
		_notes[row] = new OM::Sound(_host._vm.mixer());
	}
	if (_host._practice) {
		_count = 10;
		_state = 5;
		_pillowHere = false;
		for (int &bank : _bank)
			bank = 8;
		if (_host._chester)
			_host._chester->visible = false;
	}
	for (int i = 0; i < 3; i++) {
		_doors[i] = sprite(Common::String::format("kDoorID.%d", i + 1), indexed("kDoorZ", i + 1));
		if (!_doors[i])
			return false;
		_doors[i]->boxTouchy = true;
	}
	for (int column = 0; column < 12; column++) {
		_lights[column] = sprite("kColumnLightsID", 20);
		if (!_lights[column])
			return false;
		_lights[column]->x = number("kColumnLightX") + column * number("kColumnLightDeltaX");
		_lights[column]->y = number("kColumnLightY");
		if (8 <= column)
			continue;
		for (int row = 0; row < 5; row++) {
			_locks[row][column] = sprite("kLockID", number("kLockZ"));
			if (!_locks[row][column])
				return false;
			_locks[row][column]->x = number("kGridX") + column * number("kGridDeltaX");
			_locks[row][column]->y = number("kGridY") + row * number("kGridDeltaY");
		}
	}
	_button = sprite("kButtonID", 20);
	_doorLights = sprite("kDoorLightsID", 20);
	_activate = sprite("kActivateDoorID", 20);
	_spin = sprite("kSpinID", number("kSpinZ"));
	_pillow = sprite("kPillowID", number("kPillowZ"));
	_jewel = sprite("kJewelID", number("kJewelZ"));
	_sparkle = sprite("kSparkleID", number("kJewelZ") + 1);
	if (!_button || !_doorLights || !_activate || !_spin || !_pillow || !_jewel || !_sparkle || !sprite("kTrayMaskID", number("kTrayMaskZ")))
		return false;
	_activate->visible = _spin->visible = false;
	_pillow->visible = _pillowHere;
	_sparkle->y += number("kSparkleDeltaY");
	if (!_pillowHere) {
		_jewel->y += number("kJewelDeltaY");
		_sparkle->y += number("kJewelDeltaY");
	}
	_jewel->visible = _sparkle->visible = !_host._practice;
	_sparkle->startAnimation(-1);
	doors();
	if (!setup())
		return false;
	const bool firstVisit = !world.visits("r1la2");
	_inventoryHelp = !_host._practice && (firstVisit || _state != 5);
	_playOnEntry = !firstVisit && _state == 5;
	if (_host._practice && firstVisit) {
		say("grid", "kReEnterSpchIDs.4.1");
		say("grid", "kLocalSpchIDs.4.1.1");
	} else if (firstVisit) {
		group("Local", 1, 7);
		speech(_host._page->text("kLocalCharIDs.1.6"), number("kLocalSpchIDs.1.8"));
	} else {
		int greeting = _host._vm.rnd().number(2, _count < number("kMaxLights") ? 4 : 3);
		if (greeting == 3)
			say("reader", "kReEnterSpchIDs.3.1");
		say("grid", Common::String::format("kReEnterSpchIDs.%d.%d", greeting, greeting == 3 ? 2 : 1));
		if (greeting == 4)
			say("sam", "kReEnterSpchIDs.4.2");
		if (_inventoryHelp)
			say(_state == 3 ? "grid" : "sam", _state == 3 ? "kReEnterSpchIDs.5.1" : "kReEnterSpchIDs.6.1");
	}
	_idle = _host._time;
	return true;
}

void MusicPatterns::clearMarbles() {
	for (auto &marble : _marbles)
		_host._scene.remove(marble.sprite);
	_marbles.clear();
	_dragged = -1;
}

bool MusicPatterns::setup() {
	Common::StringTokenizer tokens(_data.get(Common::String::format("DataSet/Level_%d/WordList_%d/Set_1", _level, _dataset)));
	memset(_target, 0, sizeof(_target));
	memset(_needed, 0, sizeof(_needed));
	_given = 0;
	for (int column = 0; column < indexed("kNumGiven", _level); column++) {
		if (tokens.empty())
			return false;
		Common::String chord = tokens.nextToken();
		if (chord == "0")
			continue;
		// Authored nonempty columns form a prefix, followed by zero padding.
		if (column != _given)
			return false;
		for (char note : chord) {
			if (note < '1' || '5' < note)
				return false;
			_target[note - '1'][column] = true;
		}
		_given += 1;
	}
	if (!_given)
		return false;
	clearMarbles();
	for (int row = 0; row < 5; row++) {
		for (int column = _given; column < 12; column++) {
			_target[row][column] = _target[row][column - _given];
			if (_target[row][column])
				_needed[row] += 1;
		}
		for (int column = 0; column < 8; column++) {
			_locks[row][column]->visible = column < _given && _target[row][column];
			if (!_locks[row][column]->visible)
				continue;
			Marble marble;
			marble.row = row;
			marble.column = column;
			marble.fixed = true;
			marble.sprite = sprite(Common::String::format("kMarbleID.%d", row + 1), 5);
			if (!marble.sprite)
				return false;
			marble.sprite->x = _locks[row][column]->x;
			marble.sprite->y = _locks[row][column]->y;
			_marbles.push_back(marble);
		}
		for (int i = 0; i < _bank[row]; i++) {
			Marble marble;
			marble.row = row;
			marble.depth = number("kMarbleZ") - i * number("kMarbleDeltaZ");
			marble.home = Common::Point(indexed("kMarbleX", row + 1) - i * number("kMarbleDeltaX"),
										number("kMarbleY") - i * number("kMarbleDeltaY"));
			marble.sprite = sprite(Common::String::format("kMarbleID.%d", row + 1), marble.depth);
			if (!marble.sprite)
				return false;
			marble.sprite->x = marble.home.x;
			marble.sprite->y = marble.home.y;
			_marbles.push_back(marble);
		}
	}
	if (3 < _state) {
		_state = 5;
		for (int row = 0; row < 5; row++)
			if (_bank[row] < _needed[row])
				_state = 4;
	}
	lights();
	persist();
	return true;
}

void MusicPatterns::persist() {
	if (_host._practice)
		return;
	World &world = _host._world;
	world.set("R1LA2Level", _level);
	world.set("R1LA2Dataset", _dataset);
	world.set("R1LA2Count", _count);
	world.set("R1LA2State", _state);
	world.set("R1LA2PillowHere", _pillowHere);
	for (int row = 0; row < 5; row++)
		world.set(Common::String::format("R1LA2Marble%dCount", row + 1), _bank[row]);
}

bool MusicPatterns::donate(uint item) {
	uint first = number("kINV1Marble1");
	if (_host._practice || item < first || first + 5 <= item)
		return false;
	_host.interrupt();
	_donated = false;
	_firstDonation = _state == 3;
	for (int row = 0; row < 5; row++) {
		Common::String key = Common::String::format("item:%u", first + row);
		int available = MAX(0, _host._world.get(key));
		int take = MIN(available, 8 - _bank[row]);
		_donated |= 0 < take;
		_bank[row] += take;
		_host._world.set(key, available - take);
	}
	if (_donated) {
		_state = 4;
		if (!setup())
			error("RR1: cannot reset LA2 pattern");
		sound("kSceneSfxID");
	}
	closeInventory();
	return true;
}

void MusicPatterns::closeInventory() {
	_host.closeChest(false);
	if (_host._chester)
		_host._chester->visible = false;
	_closeStart = 0;
	_phase = kChestClosing;
}

void MusicPatterns::skipSpeech() {
	// The inventory inspection lines and its close delay are non-interruptible.
	if (_phase != kInventorySpeech && _phase != kChestOpening && _phase != kBackpackOpening && _phase != kChestClosing)
		_host.interrupt();
}

void MusicPatterns::resume(uint32 elapsed) {
	_idle += elapsed;
	if (_phase == kChestClosing && _closeStart)
		_closeStart += elapsed;
}

int MusicPatterns::resident(int row, int column) const {
	for (uint i = 0; i < _marbles.size(); i++)
		if (_marbles[i].row == row && _marbles[i].column == column)
			return i;
	return -1;
}

void MusicPatterns::home(int index) {
	Marble &marble = _marbles[index];
	marble.column = -1;
	marble.sprite->x = marble.home.x;
	marble.sprite->y = marble.home.y;
	marble.sprite->setFrame(1);
	_host._scene.setDepth(marble.sprite, marble.depth);
}

void MusicPatterns::drop() {
	Marble &marble = _marbles[_dragged];
	Common::Rect picture(marble.sprite->x, marble.sprite->y, marble.sprite->x + marble.sprite->width(), marble.sprite->y + marble.sprite->height());
	int best = 0, column = -1;
	for (int c = _given; c < 12; c++) {
		for (int row = 0; row < 5; row++) {
			int x = number("kGridX") + c * number("kGridDeltaX"), y = number("kGridY") + row * number("kGridDeltaY");
			Common::Rect overlap(x, y, x + number("kGridDeltaX"), y + number("kGridDeltaY"));
			if (!overlap.intersects(picture))
				continue;
			overlap.clip(picture);
			int area = overlap.width() * overlap.height();
			if (best < area) {
				best = area;
				column = c;
			}
		}
	}
	if (column < 0) {
		home(_dragged);
		sound("kGoHomeSfxID");
	} else {
		int previous = resident(marble.row, column);
		if (0 <= previous)
			home(previous);
		marble.column = column;
		marble.sprite->x = number("kGridX") + column * number("kGridDeltaX");
		marble.sprite->y = number("kGridY") + marble.row * number("kGridDeltaY");
		marble.sprite->setFrame(1);
		_host._scene.setDepth(marble.sprite, number("kGridZ") + 1);
		sound("kSnapSfxID");
	}
	_dragged = -1;
	lights();
}

void MusicPatterns::lights() {
	for (int column = 0; column < 12; column++) {
		int count = 0, frame = 1;
		for (int row = 0; row < 5; row++) {
			if (0 <= resident(row, column)) {
				count += 1;
				frame = row + 2;
			}
		}
		_lights[column]->visible = _level != 3;
		_lights[column]->setFrame(1 < count ? number("kColumnLightBadID") : frame);
	}
}

void MusicPatterns::doors() {
	for (int i = 0; i < 3; i++) {
		int progress = CLIP(_count - i * 3, 0, 3);
		_doors[i]->visible = progress < 3;
		_doors[i]->setFrame(MIN(progress + 1, 3));
	}
	_doorLights->setFrame(MIN(_count, 9) + 1);
}

void MusicPatterns::play(Result result, int first, int last, int soloRow) {
	_result = result;
	_column = first;
	_lastColumn = last;
	_soloRow = soloRow;
	_phase = kPlayStart;
}

void MusicPatterns::playColumn() {
	bool empty = true;
	for (int row = 0; row < 5; row++) {
		if (0 <= _soloRow && row != _soloRow)
			continue;
		int index = resident(row, _column);
		if (index < 0)
			continue;
		empty = false;
		_marbles[index].sprite->setFrame(number("kMarbleSingFrame"));
		_notes[row]->play(_host._vm.resources(), _host.characterResource(indexed("kMarbleID", row + 1), Shell::Character::Resources::kSound), Audio::Mixer::kSFXSoundType);
	}
	if (empty)
		_notes[0]->play(_host._vm.resources(), _host.characterResource(number("kBlankSfxID"), Shell::Character::Resources::kSound), Audio::Mixer::kSFXSoundType);
	_lights[_column]->setFrame(_lights[_column]->getFrame() + 7);
}

void MusicPatterns::endColumn() {
	for (auto *note : _notes)
		note->stop();
	for (auto &marble : _marbles)
		if (marble.column == _column)
			marble.sprite->setFrame(number("kMarbleEyeCloseFrame"));
	lights();
}

void MusicPatterns::finishPlayback() {
	if (_result == kCorrect) {
		_spin->visible = true;
		_spin->startAnimation(1);
		_phase = kSpin;
	} else if (_result == kWrong) {
		say("grid", Common::String::format("kWrongSpchIDs.%d", _host._vm.rnd().number(1, 3)));
		_phase = kWrongSpeech;
	} else {
		if (_result == kListen && _helpAfterPlayback) {
			speeches("kSceneCharIDs.2", "kSceneSpchIDs.2", 3);
			_helpAfterPlayback = false;
		}
		if (_result == kLocked)
			say("grid", Common::String::format("kNoMoveSpchID.%d", _host._vm.rnd().number(1, 2)));
		_phase = kIdle;
		_idle = _host._time;
	}
}

void MusicPatterns::assess() {
	if (_host._practice || _count == 9)
		return;
	World &world = _host._world;
	int outcome = _mistakes ? 3 : 1;
	int streak = world.get("R1LA2StreakKind") == outcome ? world.get("R1LA2Streak") + 1 : 1;
	int threshold = indexed(_mistakes ? "kAutolevelDownCount" : "kAutolevelUpCount", _level);
	world.recordAssessment("R1LA2", _level, outcome, indexed("kAutolevelUpCount", _level), indexed("kAutolevelDownCount", _level), streak);
	world.set("R1LA2StreakKind", outcome);
	world.set("R1LA2Streak", threshold <= streak ? 0 : streak);
	if (threshold <= streak && !world.get("R1LA2FixedLevel"))
		_level = CLIP(_level + (_mistakes ? -1 : 1), 1, 3);
}

void MusicPatterns::accept() {
	_count = MIN(_count + 1, 1000000);
	assess();
	_dataset = _dataset % 9 + 1;
	// Commit the next problem with the reward before the interruptible door sequence.
	persist();
	if (_count <= 9) {
		_activate->visible = true;
		_activate->startAnimation(1);
		sound("kActivateDoorID");
		_phase = kDoor;
	} else {
		_phase = kNext;
	}
}

void MusicPatterns::advance(uint32 time) {
	if (_host._nextSpeech < _host._speech.size() || (_host._speaking && _host._speaking->isBusy()))
		_idle = time;
}

void MusicPatterns::update(uint32 time) {
	if (_host._nextSpeech < _host._speech.size())
		return;
	if (_phase == kEntryGreeting) {
		if (_inventoryHelp) {
			say("reader", "kHaveSpch.1.1");
			_phase = kInventoryStart;
		} else if (_playOnEntry) {
			play(kListen);
		} else {
			_phase = kIdle;
		}
	} else if (_phase == kInventoryStart) {
		_chestOpening = sprite("gChesterOpenID", number("gClosedChesterZ"));
		if (_chestOpening) {
			_chestOpening->startAnimation(1);
			if (_host._chester)
				_host._chester->visible = false;
		}
		_phase = kChestOpening;
	} else if (_phase == kChestOpening) {
		if (_chestOpening && _chestOpening->isRunning())
			return;
		_host._scene.remove(_chestOpening);
		_chestOpening = nullptr;
		_host.openChest();
		_phase = kBackpackOpening;
	} else if (_phase == kBackpackOpening) {
		if (!_host._chestOpen)
			return;
		_haveMarbles = false;
		for (int row = 0; row < 5; row++)
			_haveMarbles |= 0 < _host._world.get(Common::String::format("item:%d", number("kINV1Marble1") + row));
		say("sam", _haveMarbles ? "kHaveSpch.2.1" : "kHaveSpch.3.1");
		say("reader", _haveMarbles ? "kHaveSpch.2.2" : "kHaveSpch.3.2");
		_phase = kInventorySpeech;
	} else if (_phase == kInventorySpeech) {
		if (_haveMarbles) {
			_phase = kIdle;
			_idle = time;
		} else {
			closeInventory();
		}
	} else if (_phase == kChestClosing) {
		if (!_closeStart)
			_closeStart = time;
		if (time - _closeStart < 500)
			return;
		if (_host._chester)
			_host._chester->visible = true;
		if (_donated) {
			if (_firstDonation) {
				for (int i = 1; i <= 3; i++)
					say("grid", Common::String::format("kSceneSpchIDs.1.%d", i));
			}
			_helpAfterPlayback = _state == 4;
			_donated = false;
			_phase = kDonationSpeech;
		} else {
			_phase = kIdle;
		}
		_idle = time;
	} else if (_phase == kIntroduction || _phase == kDonationSpeech) {
		play(kListen);
	} else if (_phase == kPlayStart) {
		playColumn();
		_phase = kPlayback;
	} else if (_phase == kPlayback) {
		for (auto *note : _notes)
			if (note->isPlaying())
				return;
		endColumn();
		_column += 1;
		if (_column < _lastColumn)
			playColumn();
		else
			finishPlayback();
	} else if (_phase == kWrongSpeech) {
		_mistakes += 1;
		for (uint i = 0; i < _marbles.size(); i++) {
			const Marble &marble = _marbles[i];
			if (!marble.fixed && 0 <= marble.column && !_target[marble.row][marble.column])
				home(i);
		}
		sound("kEmptyGridSfxID");
		lights();
		_phase = kIdle;
	} else if (_phase == kSpin && !_spin->isRunning()) {
		_spin->visible = false;
		say("grid", Common::String::format("kCongratsSpchID.%d", _host._vm.rnd().number(1, 4)));
		_phase = kCongratulations;
	} else if (_phase == kCongratulations) {
		accept();
	} else if (_phase == kDoor && !_activate->isRunning()) {
		_activate->visible = false;
		doors();
		sound(_count % 3 ? "kDoorSfxID.1" : "kDoorSfxID.2");
		if (_count <= 3)
			group("DoorOpen", _count, _count == 2 ? 2 : 4);
		else if (_count == 9 && _pillowHere) {
			sound("kFanFareSfxID");
			speeches("kLastDoorCharIDs", "kLastDoorSpchIDs", 4);
		} else {
			int line = _host._vm.rnd().number(1, 3);
			speech(_host._page->text(Common::String::format("kDoorOpenCharIDs.4.%d", line)), indexed("kDoorOpenSpchIDs.4", line));
		}
		_phase = kNext;
	} else if (_phase == kNext) {
		_mistakes = 0;
		if (!setup())
			error("RR1: cannot load LA2 pattern");
		sound("kNewPatternSfxID");
		_phase = kIntroduction;
	} else if (!_host._practice && _phase == kIdle && !_host._chestOpen && 30000 <= time - _idle) {
		bool enough = _state == 5;
		_idleIndex = _idleIndex % (enough ? 3 : 2) + 1;
		Common::String prefix = Common::String::format("kIdle%d", enough ? 2 : 1);
		speeches(prefix + Common::String::format("CharID.%d", _idleIndex), prefix + Common::String::format("SpchID.%d", _idleIndex),
				 (enough ? _idleIndex == 1 : _idleIndex == 2) ? 1 : 2);
		_idle = time;
	}
}

void MusicPatterns::hint() {
	_paigeClicks = _paigeClicks % 2 + 1;
	if (_state == 3) {
		say("paige", Common::String::format("kPaigeSpchIDs.2.%d", _paigeClicks));
	} else if (_state == 4) {
		say("paige", "kPaigeSpchIDs.3");
	} else {
		if (_paigeClicks == 2 && _level != 3)
			say("paige", "kPaigeSpchIDs.4.2");
		else {
			say("paige", "kPaigeSpchIDs.4.1.1");
			if (_level != 3)
				say("paige", "kPaigeSpchIDs.4.1.2");
			say("sam", "kPaigeSpchIDs.4.1.3");
		}
	}
	_idle = _host._time;
}

bool MusicPatterns::click(Common::Point point) {
	_idle = _host._time;
	if (_button->covers(point)) {
		sound("kButtonID");
		bool complete = true, correct = true;
		for (int column = 0; column < 12; column++) {
			int count = 0, needed = 0;
			for (int row = 0; row < 5; row++) {
				bool present = 0 <= resident(row, column);
				count += present;
				needed += _target[row][column];
				if (present && !_target[row][column])
					correct = false;
			}
			complete &= 0 < count && (_level != 3 || needed <= count);
		}
		if (complete && !correct)
			sound("kWrongSfxID");
		if (!complete) {
			_playClicks = _playClicks % 3 + 1;
			int lines = 2;
			if (_playClicks == 1)
				lines = _level == 3 ? 3 : 4;
			else if (_playClicks == 2)
				lines = 1;
			group("Play", _playClicks, lines);
		}
		Result result = kListen;
		if (complete)
			result = correct ? kCorrect : kWrong;
		play(result);
		return true;
	}
	Shell::Sprite *hit = _host._scene.hitSprite(point);
	for (uint i = 0; i < _marbles.size(); i++) {
		Marble &marble = _marbles[i];
		if (marble.sprite != hit)
			continue;
		if (marble.fixed) {
			play(kLocked, marble.column, marble.column + 1, marble.row);
		} else {
			_dragged = i;
			_dragOffset = point - Common::Point(marble.sprite->x, marble.sprite->y);
			marble.column = -1;
			marble.sprite->setFrame(2);
			_host._scene.setDepth(marble.sprite, 190);
			sound("kPickUpSfxID");
			lights();
		}
		return true;
	}
	for (auto *door : _doors) {
		if (door->visible && door->covers(point)) {
			sound("kCOMSoftClickSfxID");
			_doorClicks = _doorClicks % 3 + 1;
			if (_state == 3) {
				say("sam", "kDoorClkSpchIDs.4");
				say("grid", "kLocalSpchIDs.3.1");
				say("grid", "kLocalSpchIDs.3.2");
			} else {
				group("DoorClk", _doorClicks, indexed("kMaxDoorClkSpch", _doorClicks));
			}
			return true;
		}
	}
	if (9 <= _count && _pillowHere && _pillow->covers(point)) {
		_pillowHere = _pillow->visible = false;
		Common::String key = Common::String::format("item:%d", number("kINV1Pillow"));
		_host._world.set(key, MIN(25, _host._world.get(key) + 1));
		_host.putAway();
		_jewel->y += number("kJewelDeltaY");
		_sparkle->y += number("kJewelDeltaY");
		sound("kJewelDropSfxID");
		say("reader", "kPillowSpchID");
		persist();
		return true;
	}
	for (auto *light : _lights) {
		if (light->visible && light->covers(point)) {
			say("grid", "kLightSpchID");
			return true;
		}
	}
	if (_jewel->visible && _jewel->covers(point)) {
		_jewelClicks = MIN(_jewelClicks + 1, 4);
		sound("kCOMSoftClickSfxID");
		group("Jewel", _jewelClicks, indexed("kNumJewelSpchs", _jewelClicks));
		return true;
	}
	if (_host._actors["paige"]->bounds().contains(point)) {
		hint();
		return true;
	}
	if (_host._actors["grid"]->bounds().contains(point)) {
		_localClicks += 1;
		if (_state == 3) {
			say("grid", "kLocalSpchIDs.3.1");
			say("grid", "kLocalSpchIDs.3.2");
		} else if (_localClicks == 1 || _state == 4) {
			say("grid", "kLocalSpchIDs.4.1.1");
			if (_state == 4)
				group("Scene", 2, 2);
		} else if (_localClicks == 2) {
			say("grid", "kLocalSpchIDs.5.1.1");
			if (_level != 3)
				say("grid", "kLocalSpchIDs.5.1.2");
		} else {
			say("grid", Common::String::format("kLocalSpchIDs.5.2.%d", _host._vm.rnd().number(1, 6)));
		}
		return true;
	}
	if (!_host._practice && (_host._actors["reader"]->bounds().contains(point) || _host._actors["sam"]->bounds().contains(point))) {
		if (_state == 4) {
			group("Scene", 2, 2);
		} else {
			_mainClicks = _mainClicks % (_state == 5 ? 3 : 2) + 1;
			const char *actors = nullptr;
			if (_state == 3)
				actors = _mainClicks == 1 ? "spp" : "rg";
			else if (_mainClicks == 1)
				actors = "rgg";
			else
				actors = _mainClicks == 2 ? "rg" : "sgr";
			for (uint i = 0; actors[i]; i++) {
				const char *actor = "grid";
				if (actors[i] == 'r')
					actor = "reader";
				else if (actors[i] == 's')
					actor = "sam";
				else if (actors[i] == 'p')
					actor = "paige";
				say(actor, Common::String::format("kMainSpchIDs.%d.%d.%u", _state == 3 ? 3 : 4, _mainClicks, i + 1));
			}
		}
		return true;
	}
	return false;
}

bool MusicPatterns::event(const Common::Event &event) {
	if (0 <= _dragged && event.type == Common::EVENT_MOUSEMOVE) {
		_marbles[_dragged].sprite->x = event.mouse.x - _dragOffset.x;
		_marbles[_dragged].sprite->y = event.mouse.y - _dragOffset.y;
		return true;
	}
	if (0 <= _dragged && event.type == Common::EVENT_LBUTTONUP) {
		_marbles[_dragged].sprite->x = event.mouse.x - _dragOffset.x;
		_marbles[_dragged].sprite->y = event.mouse.y - _dragOffset.y;
		drop();
		return true;
	}
	if (_phase == kIdle && event.type == Common::EVENT_MOUSEMOVE) {
		Shell::Sprite *hit = _host._scene.hitSprite(event.mouse);
		_button->setFrame(hit == _button ? 2 : 1);
		for (auto &marble : _marbles)
			if (!marble.fixed)
				marble.sprite->setFrame(hit == marble.sprite ? 2 : 1);
	}
	return false;
}

void MusicPatterns::pause(bool paused) {
	for (auto *note : _notes)
		if (note)
			note->pause(paused);
	if (paused) {
		_pausedSprites.clear();
		for (auto *animation : {_spin, _activate, _sparkle, _chestOpening}) {
			if (animation && animation->isRunning()) {
				_pausedSprites.push_back(animation);
				animation->pauseAnimation(true);
			}
		}
	} else {
		for (auto *animation : _pausedSprites)
			animation->pauseAnimation(false);
		_pausedSprites.clear();
	}
}

} // namespace RR1Cloud9
} // namespace Trinket
