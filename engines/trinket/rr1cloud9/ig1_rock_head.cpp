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

#include "trinket/rr1cloud9/ig1_rock_head.h"

#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

int RockHead::number(const Common::String &key) const { return _host._page->number(key); }

void RockHead::line(const Common::String &actors, const Common::String &sounds) {
	_host.say(_host._page->text(actors), number(sounds));
}

void RockHead::group(const Common::String &actors, const Common::String &sounds, int count) {
	_host.speeches(actors, sounds, count);
}

bool RockHead::character(const char *name, const char *prefix, int pose) {
	if (_host._actors.contains(name)) {
		delete _host._actors[name];
		_host._actors.erase(name);
	}
	Common::String suffix = Common::String::format(".%d", pose);
	Common::String base(prefix);
	int settle = number(base + "SetID" + suffix);
	int lips = number(base + "LipsID" + suffix);
	if (!lips)
		lips = settle;
	Common::ScopedPtr<Shell::Character> actor(new Shell::Character(_host._scene, _host._vm.mixer(), _host._vm.resources(), _host._vm.rnd(), _host));
	if (!actor->create(settle ? settle : lips, number(base + "Z")))
		return false;
	actor->movable = false;
	for (const char *key : {"registerFidgetsToSettle", "registerSpeechToSettle", "loadSpeechAnimsInBkgd", "animateSettled"})
		actor->set(key, {}, Core::ScriptValue(0));
	Core::ScriptValue result;
	actor->play("setSpeechAnims", {Core::ScriptValue(lips), Core::ScriptValue(1), Core::ScriptValue(1), Core::ScriptValue(number(base + "BodyID" + suffix)), Core::ScriptValue(0), Core::ScriptValue(0)}, result);
	actor->play("addFidgetGroup", {Core::ScriptValue(1), Core::ScriptValue(5), Core::ScriptValue(10)}, result);
	actor->play("addFidget", {Core::ScriptValue(1), Core::ScriptValue(settle ? settle : lips)}, result);
	if (number(base + "Fidg1ID" + suffix)) {
		actor->play("addFidgetGroup", {Core::ScriptValue(2), Core::ScriptValue(15), Core::ScriptValue(30)}, result);
		for (int i = 1; i <= 2; i++)
			actor->play("addFidget", {Core::ScriptValue(2), Core::ScriptValue(number(base + Common::String::format("Fidg%dID", i) + suffix))}, result);
	}
	_host._actors[name] = actor.release();
	return true;
}

bool RockHead::companions(int pose) {
	if (!character("sam", "kSam", pose) || !character("paige", "kPaige", pose))
		return false;
	if (_host._chester)
		_host._scene.remove(_host._chester);
	_host._chester = _host._scene.add(_host.characterResource(number(Common::String::format("kChesterFidgID.%d", pose)),
															  Shell::Character::Resources::kAnimation),
									  number("gClosedChesterZ"));
	return _host._chester != nullptr;
}

bool RockHead::rock() {
	int pose = _host._world.get("R1IG1HatHere") ? 2 : 1;
	if (!character("rock", "kRock", pose))
		return false;
	Core::ScriptValue result;
	_host._actors["rock"]->play("removeFidgetGroup", {Core::ScriptValue(1)}, result);
	if (pose == 1) {
		_host._actors["rock"]->play("addFidgetGroup", {Core::ScriptValue(1), Core::ScriptValue(5), Core::ScriptValue(10)}, result);
		_host._actors["rock"]->play("addFidget", {Core::ScriptValue(1), Core::ScriptValue(number("kRockFdgtID"))}, result);
	}
	return true;
}

bool RockHead::enter() {
	World &world = _host._world;
	if (!world.get("R1IG1State")) {
		world.set("R1IG1State", 1);
		world.set("R1IG1HoopHere", 1);
	}
	_state = CLIP(world.get("R1IG1State"), 1, 4);
	if (_state == 1 && world.get("item:6"))
		_state = 2;
	if (_state < 3 && world.get("R1IG1LadderHere"))
		_state = 3;
	if (_state == 3 && !world.get("R1IG1HoopHere"))
		_state = 4;
	world.set("R1IG1State", _state);
	_firstVisit = !world.visits("r1ig1");
	int pending = world.get("R1IG1Pending");
	_pose = pending == 8 || pending == 2 ? 2 : 1;
	if (!character("reader", "kReader", _pose) || !companions(_pose) || !rock())
		return false;
	auto sprite = [this](const char *key, const char *depth) {
		return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), number(depth));
	};
	_ladder = sprite("kLadderID", "kLadderZ");
	_hat = sprite("kHatID", "kHatZ");
	_hoop = sprite("kHoop2ID", "kHoopZ");
	if (!_ladder || !_hat || !_hoop)
		return false;
	_ladder->visible = world.get("R1IG1LadderHere");
	_hat->visible = world.get("R1IG1HatHere");
	_hoop->visible = _hat->visible && world.get("R1IG1HoopHere");
	_hoop->boxTouchy = true;
	if (pending == 6 || pending == 8)
		_phase = pending == 6 ? kLadderWait : kHatWait;
	else if (pending == 2)
		_phase = kCollect;
	else if (world.get("previousLocation") == 20) {
		_host._actors["reader"]->playAnim(number("kReaderWalkInID"));
		_host._actors["sam"]->playAnim(number("kSamWalkInID"));
		_host._actors["paige"]->setVisible(false);
		_host._chester->visible = false;
		_phase = kArrival;
	} else
		entrySpeech();
	return true;
}

void RockHead::entrySpeech() {
	World &world = _host._world;
	if (_firstVisit)
		group("kEnterCharID", "kEnterSpchID", number("kMaxEntrySpchs"));
	else if (world.get("previousLocation") == 20) {
		int count = world.get("R1IG1RainbowSlideCount") + 1;
		world.set("R1IG1RainbowSlideCount", count);
		Common::String suffix = Common::String::format(".%d", count == 1 ? 1 : _host._vm.rnd().number(2, 3));
		group("kReEnterCharIDs" + suffix, "kReEnterSpchIDs" + suffix, number("kMaxReEnterSpchs" + suffix));
	}
}

void RockHead::climb() {
	_host.closeChest();
	_host._actors["reader"]->playAnim(number(_pose == 1 ? "kReaderClimbID" : "kReaderDecendID"));
	_phase = kReaderClimb;
}

void RockHead::update(uint32 time) {
	if (!_clock) {
		_start = _idle = time;
		_clock = true;
	}
	if (_host._nextSpeech < _host._speech.size()) {
		_idle = time;
		return;
	}
	World &world = _host._world;
	if (_phase == kArrival) {
		if (_host._actors["reader"]->isBusy() || _host._actors["sam"]->isBusy())
			return;
		_host._actors["paige"]->setVisible(true);
		_host._chester->visible = true;
		entrySpeech();
		_phase = kIdle;
	} else if (_phase == kReaderClimb) {
		if (_host._actors["reader"]->isBusy())
			return;
		if (!character("reader", "kReader", 3 - _pose))
			error("RR1: cannot load Reader's climbing pose");
		_host._actors["sam"]->playAnim(number(_pose == 1 ? "kSamClimbID" : "kSamDecendID"));
		_phase = kSamClimb;
	} else if (_phase == kSamClimb) {
		if (_host._actors["sam"]->isBusy())
			return;
		_pose = 3 - _pose;
		if (!companions(_pose))
			error("RR1: cannot load summit companions");
		_phase = kIdle;
	} else if (_phase == kLadderWait || _phase == kHatWait) {
		if (time - _start < 1500)
			return;
		if (_phase == kLadderWait) {
			_ladder->visible = true;
			world.set("R1IG1LadderHere", 1);
			world.set("R1IG1Pending", 0);
			_state = 3;
			world.set("R1IG1State", _state);
			group("kLadderCharID", "kLadderSpchID", 2);
			_phase = kSpeech;
		} else {
			_hat->visible = true;
			Core::ScriptValue result;
			_host._actors["rock"]->play("removeFidgetGroup", {Core::ScriptValue(1)}, result);
			group("kHatCharID", "kHatSpchID", number("kNumHatSpchs"));
			_phase = kHatSpeech;
		}
	} else if (_phase == kHatSpeech) {
		world.set("R1IG1HatHere", 1);
		world.set("R1IG1Pending", 0);
		_hoop->visible = world.get("R1IG1HoopHere");
		_host.playSounds({static_cast<uint16>(number("kHoopPresentedID"))});
		if (!rock())
			error("RR1: cannot load Rock Head's hat pose");
		line("kHoopPresentedCharID", "kHoopPresentedSpchID");
		_phase = kSpeech;
	} else if (_phase == kCollect) {
		if (world.get("R1IG1Pending") == 2) {
			world.set("item:2", MIN(25, world.get("item:2") + 1));
			_host.putAway();
			world.set("R1IG1HoopHere", 0);
			world.set("R1IG1Pending", 0);
		}
		_hoop->visible = false;
		_state = 4;
		world.set("R1IG1State", _state);
		_phase = kIdle;
	} else if (_phase == kSpeech)
		_phase = kIdle;
	else if (_phase == kIdle && !_host._practice && static_cast<uint32>(number("gIdleHelpTime")) <= time - _idle) {
		Common::String suffix = Common::String::format(".%d", _state < 3 ? 1 : _pose + 1);
		suffix += Common::String::format(".%d", _host._vm.rnd().number(1, number("kMaxIdleIndex" + suffix)));
		group("kIdleCharID" + suffix, "kIdleSpchID" + suffix, number("kMaxIdleIndexSpeeches" + suffix));
		_idle = time;
	}
}

void RockHead::mainSpeech() {
	if (_state == 1) {
		int choice = _host._vm.rnd().number(1, _host._world.visits("r1ca1") ? 4 : 3);
		if (!_host._world.visits("r1ca1") && choice == 3)
			choice = 4;
		Common::String suffix = Common::String::format(".1.%d", choice);
		group("kMainSpchCharIDs" + suffix, "kMainSpchIDs" + suffix, number("kMaxMainSpchs" + suffix));
	} else if (_state == 2)
		line("kMainSpchCharIDs.2", "kMainSpchIDs.2");
	else if (_state == 3 && _pose == 1) {
		Common::String suffix = Common::String::format(".3.%d", _host._vm.rnd().number(1, 2));
		line("kMainSpchCharIDs" + suffix, "kMainSpchIDs" + suffix);
	} else
		rockSpeech();
}

void RockHead::rockSpeech() {
	World &world = _host._world;
	world.set("talkedToRockHead", 1);
	if (_state == 1) {
		mainSpeech();
		return;
	}
	Common::String suffix;
	if (_state == 2)
		suffix = ".2.1";
	else if (_state == 3 && !world.get("R1IG1HatHere")) {
		if (_pose == 1) {
			line("kRockCharID.3.1", "kRockSpchID.3.1");
			return;
		}
		int count = world.get("R1IG1RockClicks") + 1;
		world.set("R1IG1RockClicks", count);
		int choice;
		if (count == 1)
			choice = 1 < world.get("R1LN2State") ? 2 : 1;
		else {
			if (_moreClicks == 1)
				choice = world.get("item:8") ? 5 : (world.visits("r1cn3") ? 4 : 3);
			else {
				if (4 < _moreClicks)
					_moreClicks = 2;
				choice = _moreClicks + 4;
			}
			_moreClicks += 1;
		}
		suffix = Common::String::format(".3.2.%d", choice);
	} else
		suffix = Common::String::format(".4.%d", _pose);
	group("kRockCharID" + suffix, "kRockSpchID" + suffix, number("kMaxRockSpch" + suffix));
}

void RockHead::hoopClick() {
	if (_pose == 1) {
		_host.playSounds({static_cast<uint16>(number("WAVE_COMSOFTCLICK"))});
		Common::String suffix = Common::String::format(".%d", _host._vm.rnd().number(1, number("kMaxHoopSpchs")));
		group("kHoopCharID" + suffix, "kHoopSpchID" + suffix, number("kNumHoopSpchs" + suffix));
	} else if (_host._world.get("R1IG1HatHere") && !_host._practice) {
		_host.playSounds({static_cast<uint16>(number("WAVE_COMPICKUP"))});
		_host._world.set("R1IG1Pending", 2);
		line("kGetHoopCharID", "kGetHoopSpchID");
		_phase = kCollect;
	} else
		rockSpeech();
}

bool RockHead::hotspot(const char *prefix, const Common::String &suffix, Common::Point point) const {
	Common::String base(prefix);
	return Common::Rect(number(base + "HSX1" + suffix), number(base + "HSY1" + suffix),
						number(base + "HSX2" + suffix), number(base + "HSY2" + suffix))
		.contains(point);
}

bool RockHead::click(Common::Point point) {
	_idle = _host._time;
	if (busy())
		return true;
	if (_host._world.get("R1IG1HoopHere") && (_hoop->covers(point) ||
											  (!_host._world.get("R1IG1HatHere") && hotspot("kHoop", "", point)))) {
		hoopClick();
		return true;
	}
	for (auto &entry : _host._actors) {
		if (!entry._value->bounds().contains(point))
			continue;
		if (entry._key == "rock")
			rockSpeech();
		else if (entry._key == "paige") {
			int state = _state;
			int choice = _host._vm.rnd().number(1, number(Common::String::format("kMaxHelpSpchs.%d", state)));
			if (state == 3 && _host._world.get("R1IG1HatHere")) {
				state = 4;
				choice = 1;
			}
			Common::String suffix = Common::String::format(".%d.%d", state, choice);
			line("kHelpSpchCharIDs" + suffix, "kHelpSpchIDs" + suffix);
		} else
			mainSpeech();
		return true;
	}
	if (_ladder->visible && (hotspot("kLadder", _pose == 1 ? ".1" : ".2", point) || hotspot("kLadder", ".3", point))) {
		climb();
		return true;
	}
	if (_pose == 2) {
		for (const auto &exit : _host._page->exits()) {
			if (exit.rect.contains(point)) {
				climb();
				return true;
			}
		}
	}
	return false;
}

bool RockHead::donate(uint item) {
	if (busy() || _host._practice)
		return false;
	_host.interrupt();
	World &world = _host._world;
	if ((item == 6 && !_ladder->visible) || (item == 8 && _pose == 2 && !_hat->visible)) {
		Common::String key = Common::String::format("item:%u", item);
		if (world.get(key) <= 0)
			return false;
		world.set(key, world.get(key) - 1);
		world.set("R1IG1Pending", item);
		_host.closeChest();
		_start = _host._time;
		_clock = true;
		_phase = item == 6 ? kLadderWait : kHatWait;
		return true;
	}
	if (_pose == 1 && item == 8)
		line("kRockCharID.2.1.1", "kRockSpchID.2.1.1");
	else if (_pose == 2 && world.get("talkedToRockHead")) {
		Common::String suffix = Common::String::format(".%d", _host._vm.rnd().number(1, number("kMaxWrongColorSpchs")));
		line("kWrongColorCharID" + suffix, "kWrongColorSpchID" + suffix);
	} else {
		Common::String suffix = Common::String::format(".%d", _host._vm.rnd().number(1, number("kMaxItemSpchs")));
		line("kItemCharSpch" + suffix, "kItemSpchsID" + suffix);
	}
	return false;
}

} // namespace RR1Cloud9
} // namespace Trinket
