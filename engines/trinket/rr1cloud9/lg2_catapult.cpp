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

#include "trinket/rr1cloud9/lg2_catapult.h"

#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

int Catapult::number(const Common::String &key) const { return _host._page->number(key); }

void Catapult::line(const char *actors, const char *sounds, int index) {
	Common::String suffix = index ? Common::String::format(".%d", index) : Common::String();
	_host.say(_host._page->text(Common::String(actors) + suffix), number(Common::String(sounds) + suffix));
}

void Catapult::group(const char *actors, const char *sounds, const char *count, int index) {
	Common::String suffix = index ? Common::String::format(".%d", index) : Common::String();
	_host.speeches(Common::String(actors) + suffix, Common::String(sounds) + suffix, number(Common::String(count) + suffix));
}

bool Catapult::enter() {
	World &world = _host._world;
	_sleeping = world.get("R1LG2PetSleeping") && world.get("R1LG2CatapultReady") && !world.get("R1LG2FeedPending");
	if (!world.get("R1LG2FeedPending"))
		world.set("R1LG2PetSleeping", _sleeping);
	if (!_host.actor("pet.1", "gPet"))
		return false;
	_host._actors["pet.1"]->set("z", {}, Core::ScriptValue(number("kPetZ")));
	_host._actors["pet.1"]->setVisible(!_sleeping);
	auto sprite = [this](const char *key, const char *depth) {
		return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), number(depth));
	};
	_bowl = sprite("kBowlID", "kBowlZ");
	_sandwich = sprite("kSandwichID", "kSandwichZ");
	_eating = sprite("kEatingID", "kPetZ");
	_sleep = sprite("gPetSleepID", "kPetZ");
	_clump = sprite("kClumpStaticID", "kClumpZ");
	_clumpAnim = sprite("kClumpID", "kClumpZ");
	_catapult = sprite("kCatapultID", "kCatapultZ");
	_exit = sprite("kExitAnimID", "kCatapultZ");
	if (!_bowl || !_sandwich || !_eating || !_sleep || !_clump || !_clumpAnim || !_catapult || !_exit)
		return false;
	_bowl->visible = !_sleeping;
	_sleep->visible = _sleeping;
	_sandwich->visible = _eating->visible = _clumpAnim->visible = _exit->visible = false;
	if (world.get("R1LG2ExitPending")) {
		_phase = kExitSpeech;
	} else if (world.get("R1LG2FeedPending")) {
		_phase = kFeedWait;
	} else if (!world.visits("r1lg2"))
		group("kEnterCharID", "kEnterSpchID", "kMaxEntrySpchs");
	else if (!_sleeping && world.visits("r1lg2") < 4)
		group("kReEnterCharIDs", "kReEnterSpchIDs", "kMaxReEnterSpchs", world.visits("r1lg2"));
	else if (_sleeping)
		group("kReEnterCharIDs", "kReEnterSpchIDs", "kMaxReEnterSpchs", 4);
	return true;
}

void Catapult::advance(uint32 time) {
	if (!_clock) {
		_start = _idle = _ambientStart = _sleepStart = time;
		_clock = true;
	}
	if (_phase != kExiting && _phase != kExitReady) {
		if (_ambientPlaying && !_clumpAnim->isRunning()) {
			_ambientPlaying = false;
			_clumpAnim->visible = false;
			_clump->visible = _catapult->visible = true;
			_ambientStart = time;
			_ambientDelay = 2000;
		} else if (!_ambientPlaying && _ambientDelay <= time - _ambientStart) {
			_clumpAnim->visible = true;
			_clump->visible = _catapult->visible = false;
			_clumpAnim->startAnimation(1);
			_ambientPlaying = true;
		}
	}
	if (_sleep->visible) {
		if (_snoring && !_sleep->isRunning()) {
			_snoring = false;
			_sleepStart = time;
		} else if (!_snoring && 5000 <= time - _sleepStart) {
			_sleep->startAnimation(1);
			_host.playSounds({static_cast<uint16>(number("kPetSleepID"))});
			_snoring = true;
		}
	}
}

void Catapult::update(uint32 time) {
	if (_host._nextSpeech < _host._speech.size()) {
		_idle = time;
		return;
	}
	World &world = _host._world;
	if (_phase == kFeedWait) {
		if (time - _start < 1500)
			return;
		_host.playSounds({static_cast<uint16>(number("kFeedingSFXID"))});
		_sandwich->visible = true;
		if (world.get("R1LG2FeedPending") == 1) {
			world.set("R1LG2FeedCreature", world.get("R1LG2FeedCreature") + 1);
			world.set("R1LG2PetSleeping", 1);
			world.set("R1LG2CatapultReady", 1);
			world.set("R1LG2FeedPending", 2);
		}
		int count = world.get("R1LG2FeedCreature");
		line("kSandwichCharID", "kSandwichSpchID", count == 1 ? 1 : _host._vm.rnd().number(1, number("kMaxSandwichSpchs")));
		_phase = kFeedSpeech;
	} else if (_phase == kFeedSpeech) {
		_eating->visible = true;
		_host._actors["pet.1"]->setVisible(false);
		_sandwich->visible = _bowl->visible = false;
		_eating->startAnimation(1);
		_phase = kEating;
	} else if (_phase == kEating) {
		if (_eating->isRunning())
			return;
		_eating->visible = false;
		_sleep->visible = true;
		_sleepStart = time;
		_sleeping = true;
		world.set("R1LG2FeedPending", 0);
		int count = world.get("R1LG2FeedCreature");
		line("kPostEatCharID", "kPostEatSpchID", _host._vm.rnd().number(1, number("kMaxPostEatSpchs")));
		if (count == 1)
			line("kLetsGoCharID", "kLetsGoSpchID");
		_phase = kSpeech;
	} else if (_phase == kSpeech)
		_phase = kIdle;
	else if (_phase == kExitSpeech) {
		_clumpAnim->stopAnimation();
		_clumpAnim->visible = _catapult->visible = false;
		_clump->visible = _exit->visible = true;
		_host._actors["reader"]->setVisible(false);
		_host._actors["sam"]->setVisible(false);
		_exit->startAnimation(1);
		_phase = kExiting;
	} else if (_phase == kExiting) {
		if (!_exit->isRunning())
			_phase = kExitReady;
	} else if (_phase == kIdle && !_host._practice && static_cast<uint32>(number("gIdleHelpTime")) <= time - _idle) {
		group("kIdleCharID", "kIdleSpchID", "kMaxIdleSpchs", _sleeping ? 4 : _host._vm.rnd().number(1, 3));
		_idle = time;
	}
}

bool Catapult::donate(uint item) {
	if (busy() || _host._practice)
		return false;
	_host.interrupt();
	if (!_sleeping && item == static_cast<uint>(number("kINV1Food")) && 0 < _host._world.get("item:9")) {
		_host._world.set("item:9", _host._world.get("item:9") - 1);
		_host._world.set("R1LG2FeedPending", 1);
		_host.closeChest();
		_start = _host._time;
		_phase = kFeedWait;
		return true;
	}
	if (_sleeping)
		line("kItemCharSpch2", "kItemSpchsID2", _host._vm.rnd().number(1, number("kMaxItemSpchs2")));
	else
		group("kItemCharSpch", "kItemSpchsID", "kItemSpchCount", _host._vm.rnd().number(1, number("kMaxItemSpchs")));
	return false;
}

void Catapult::ride() {
	World &world = _host._world;
	int count = world.get("R1LG2RodeCatapult") + 1;
	world.set("R1LG2RodeCatapult", count);
	world.set("R1LG2CatapultReady", 0);
	world.set("R1LG2ExitPending", 1);
	if (count == 1)
		line("kFirstExitCharID", "kFirstExitSpchID");
	else
		line("kSubSeqExitCharID", "kSubSeqExitSpchID", _host._vm.rnd().number(1, 2));
	_phase = kExitSpeech;
}

bool Catapult::click(Common::Point point) {
	_idle = _host._time;
	if (_bowl->visible && _bowl->covers(point)) {
		int choice = _host._vm.rnd().number(1, 2);
		if (choice == 2 && !_host._world.get("item:9"))
			choice = 3;
		group("kBowlCharID", "kBowlSpchID", "kMaxBowlSpchs", choice);
		return true;
	}
	if (_sleep->visible && _sleep->covers(point))
		return true;
	for (auto &entry : _host._actors) {
		if (entry._key == "pet.1" && _sleeping)
			continue;
		if (!entry._value->bounds().contains(point))
			continue;
		if (entry._key == "pet.1")
			group("kPetCharIDs", "kPetSpchIDs", "kMaxPetSpchs", _host._vm.rnd().number(1, 3));
		else if (entry._key == "paige") {
			Common::String suffix = _sleeping ? ".2.1" : Common::String::format(".1.%d", _host._vm.rnd().number(1, number("kMaxHelpSpchs.1")));
			_host.say(_host._page->text("kHelpSpchCharIDs" + suffix), number("kHelpSpchIDs" + suffix));
		} else {
			int choice = 7;
			if (!_sleeping) {
				if (_host._world.get("R1LG2RodeCatapult"))
					choice = _host._vm.rnd().number(4, 6);
				else {
					choice = _host._vm.rnd().number(1, 2) == 2 ? 3 : 1;
					if (choice == 1 && _host._world.get("item:9"))
						choice = 2;
				}
			}
			group("kMainSpchCharIDs", "kMainSpchIDs", "kMaxMainSpchs", choice);
		}
		return true;
	}
	for (const auto &exit : _host._page->exits()) {
		if (exit.destination == "r1rss" && exit.rect.contains(point)) {
			if (_sleeping)
				ride();
			else
				group("kDontExitCharIDs", "kDontExitSpchIDs", "kMaxDontExitSpchs", _host._vm.rnd().number(1, number("kNumDontExitSpchs")));
			return true;
		}
	}
	return false;
}

void Catapult::pause(bool paused, uint32 elapsed) {
	if (!paused) {
		_start += elapsed;
		_idle += elapsed;
		_ambientStart += elapsed;
		_sleepStart += elapsed;
	}
	for (auto *sprite : {_sleep, _eating, _clumpAnim, _exit})
		sprite->pauseAnimation(paused);
}

Common::String Catapult::destination() const {
	return _phase == kExitReady ? "r1rss" : Common::String();
}
} // namespace RR1Cloud9
} // namespace Trinket
