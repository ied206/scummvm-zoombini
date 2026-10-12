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

#include "trinket/rr1cloud9/cg2_sheep_crossing.h"

#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

int SheepCrossing::number(const Common::String &key) const { return _host._page->number(key); }

int SheepCrossing::increment(const char *key) {
	int value = _host._world.get(key) + 1;
	_host._world.set(key, value);
	return value;
}

void SheepCrossing::group(const char *actors, const char *sounds, const char *count, const Common::String &suffix) {
	_host.speeches(Common::String(actors) + suffix, Common::String(sounds) + suffix, number(Common::String(count) + suffix));
}

bool SheepCrossing::enter() {
	World &world = _host._world;
	_state = CLIP(world.get("R1CG2State"), 1, 3);
	if ((_state == 1 && world.get("item:5")) || world.visits("r1cn4")) {
		_state = world.visits("r1cn4") ? 3 : 2;
		for (const char *key : {"R1CG2IdleCount", "R1CG2MainClicks", "R1CG2SheepClicks"})
			world.set(key, 0);
	}
	world.set("R1CG2State", _state);
	for (int i = 1; i <= number("kMaxSheep"); i++) {
		Common::String prefix = Common::String::format("kSheepLoc.%d.", i);
		auto *walk = _host._scene.add(_host.characterResource(number("kSheepID"), Shell::Character::Resources::kAnimation), number(prefix + "3"));
		auto *stand = _host._scene.add(_host.characterResource(number("kSheepStandingID"), Shell::Character::Resources::kAnimation), number(prefix + "3"));
		if (!walk || !stand)
			return false;
		walk->y = stand->y = number(prefix + "1");
		stand->x = number(prefix + "4");
		walk->setFrame(number(prefix + "2"));
		walk->playAnimation();
		stand->visible = false;
		_walking.push_back(walk);
		_standing.push_back(stand);
	}
	_whistle = _host._scene.add(_host.characterResource(number("kBlowWhistleID"), Shell::Character::Resources::kAnimation), number("kSamZ"));
	if (!_whistle)
		return false;
	_whistle->visible = false;
	_sound = new OM::Sound(_host._vm.mixer());
	_sound->play(_host._vm.resources(), _host.characterResource(number("kSheepID"), Shell::Character::Resources::kSound), Audio::Mixer::kSFXSoundType, 0);
	if (world.get("R1CG2WhistlePending"))
		_phase = kWait;
	else if (!world.visits("r1cg2"))
		group("kEnterCharID", "kEnterSpchID", "kMaxEntrySpchs", "");
	else {
		int choice = 0;
		if (_state == 1)
			choice = _host._vm.rnd().number(1, 2);
		else if (_state == 2)
			choice = increment("R1CG2State2Count") == 1 ? 1 : 2;
		else if (world.get("previousLocation") == 2 && increment("R1CG2State3Count") == 1)
			choice = 1;
		if (choice)
			group("kReEnterCharIDs", "kReEnterSpchIDs", "kNumReEnterSpchs", Common::String::format(".%d.%d", _state, choice));
	}
	return true;
}

void SheepCrossing::update(uint32 time) {
	if (!_clock) {
		_start = _idle = time;
		_clock = true;
	}
	if (_host._nextSpeech < _host._speech.size()) {
		_idle = time;
		return;
	}
	if (_phase == kWait) {
		if (time - _start < 1500)
			return;
		_whistle->visible = true;
		_host._actors["sam"]->setVisible(false);
		_whistle->startAnimation(1);
		_phase = kBlow;
	} else if (_phase == kBlow) {
		if (_whistle->isRunning())
			return;
		_whistle->visible = false;
		_host._actors["sam"]->setVisible(true);
		_sound->stop();
		for (uint i = 0; i < _walking.size(); i++) {
			_walking[i]->visible = false;
			_walking[i]->stopAnimation();
			_standing[i]->visible = true;
			_standing[i]->startAnimation(1);
		}
		_host.playSounds({static_cast<uint16>(number("kSheepStandingSFXID"))});
		_phase = kStand;
	} else if (_phase == kStand) {
		for (auto *sheep : _standing)
			if (sheep->isRunning())
				return;
		for (uint i = 0; i < _standing.size(); i++) {
			Common::String prefix = Common::String::format("kSheepLoc.%u.", i + 1);
			_standing[i]->setFrame(_standing[i]->getNumberOfFrames());
			_standing[i]->x = number(prefix + "4") + 12;
			_standing[i]->y = number(prefix + "1") - 49;
		}
		_moving = false;
		_host._world.set("R1CG2WhistlePending", 0);
		if (increment("R1CG2BlowCount") == 1)
			_host.speeches("kWorksCharID", "kWorksSpchID", 2);
		_phase = kSpeech;
	} else if (_phase == kSpeech)
		_phase = kIdle;
	else if (!_host._practice && static_cast<uint32>(number("gIdleHelpTime")) <= time - _idle) {
		int choice = _host._vm.rnd().number(3, number("kMaxIdleIndex"));
		if (increment("R1CG2IdleCount") == 1 && _state < 3)
			choice = _state;
		group("kIdleCharID", "kIdleSpchID", "kMaxIdleIndexSpeeches", Common::String::format(".%d", choice));
		_idle = time;
	}
}

void SheepCrossing::talk(bool sheep) {
	if (sheep && !_moving) {
		_destination = _host._page->text("kPreviousLocations.1");
		return;
	}
	int choice = _host._vm.rnd().number(sheep ? 5 : 4, number(sheep ? "kMaxSheepIndex" : "kMaxMainIndex"));
	int count = increment(sheep ? "R1CG2SheepClicks" : "R1CG2MainClicks");
	if (count == 1) {
		if (_state == 1)
			choice = 1;
		else if (_state == 2 && _moving)
			choice = 2;
		else if (sheep && _state == 3)
			choice = 4;
	} else if (count == 2 && _state == 2 && _moving)
		choice = 3;
	group(sheep ? "kSheepCharID" : "kMainCharID", sheep ? "kSheepSpchID" : "kMainSpchID",
		  sheep ? "kMaxSheepIndexSpeeches" : "kMaxMainIndexSpeeches", Common::String::format(".%d", choice));
}

bool SheepCrossing::click(Common::Point point) {
	_idle = _host._time;
	auto hotspot = [this, point](const char *prefix) {
		Common::String base(prefix);
		return Common::Rect(number(base + "HSX1"), number(base + "HSY1"), number(base + "HSX2"), number(base + "HSY2")).contains(point);
	};
	if (hotspot("kSign")) {
		_host.playSounds({static_cast<uint16>(number("WAVE_COMSOFTCLICK"))});
		_host.say(_host._page->text("kSignCharID"), number("kSignSpchID"));
		return true;
	}
	if (hotspot("kSheep")) {
		talk(true);
		return true;
	}
	for (auto &entry : _host._actors) {
		if (!entry._value->bounds().contains(point))
			continue;
		if (entry._key == "paige") {
			Common::String suffix = Common::String::format(".%d", _state);
			suffix += Common::String::format(".%d", _host._vm.rnd().number(1, number("kNumHelpSpchs" + suffix)));
			_host.say(_host._page->text("kHelpSpchCharIDs" + suffix), number("kHelpSpchIDs" + suffix));
		} else
			talk(false);
		return true;
	}
	for (const auto &exit : _host._page->exits()) {
		if (_moving && exit.destination == _host._page->text("kPreviousLocations.1") && exit.rect.contains(point)) {
			talk(true);
			return true;
		}
	}
	return false;
}

bool SheepCrossing::donate(uint item) {
	if (busy() || _host._practice)
		return false;
	_host.interrupt();
	if (_moving && item == static_cast<uint>(number("kINV1Whistle")) && _host._world.get("item:5")) {
		_host.closeChest();
		_host._world.set("R1CG2WhistlePending", 1);
		_start = _host._time;
		_clock = true;
		_phase = kWait;
		return true;
	}
	if (_moving)
		_host.say(_host._page->text("kWrongItemCharID"), number("kWrongItemSpchID"));
	else {
		int choice = _host._vm.rnd().number(1, number("kMaxItemSpchs"));
		if (item == static_cast<uint>(number("kINV1Whistle")))
			choice = 3;
		Common::String suffix = Common::String::format(".%d", choice);
		_host.say(_host._page->text("kItemCharSpch" + suffix), number("kItemSpchsID" + suffix));
	}
	return false;
}

void SheepCrossing::pause(bool paused, uint32 elapsed) {
	if (!paused) {
		_start += elapsed;
		_idle += elapsed;
	}
	for (auto *sprite : _walking)
		sprite->pauseAnimation(paused);
	for (auto *sprite : _standing)
		sprite->pauseAnimation(paused);
	_whistle->pauseAnimation(paused);
	if (_sound)
		_sound->pause(paused);
}
} // namespace RR1Cloud9
} // namespace Trinket
