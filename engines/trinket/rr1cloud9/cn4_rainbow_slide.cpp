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

#include "trinket/rr1cloud9/cn4_rainbow_slide.h"

#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

int RainbowSlide::number(const Common::String &key) const { return _host._page->number(key); }

void RainbowSlide::setState(int state) {
	_state = state;
	if (!_host._practice)
		_host._world.set("R1CN4GameState", state);
}

void RainbowSlide::group(const char *characters, const char *speech, int count, int index) {
	Common::String suffix = index ? Common::String::format(".%d", index) : Common::String();
	_host.speeches(Common::String(characters) + suffix, Common::String(speech) + suffix, count);
}

bool RainbowSlide::enter() {
	World &world = _host._world;
	_state = CLIP(world.get("R1CN4GameState"), 1, 4);
	if (_state == 1 && world.get("item:7")) {
		setState(2);
		if (1 < world.get("R1CN4CarrieClicks"))
			world.set("R1CN4CarrieClicks", 1);
	}
	auto sprite = [this](const char *key, const char *depth) {
		return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), number(depth));
	};
	if (_state < 3) {
		if (!_host.actor("carrie", "kCarrie") || !sprite("kStackID", "kStackZ") || !sprite("kStuffID", "kStuffZ"))
			return false;
		auto *carrie = _host._actors["carrie"];
		Core::ScriptValue result;
		carrie->set("animateSettled", {}, Core::ScriptValue(0));
		carrie->play("addFidgetGroup", {Core::ScriptValue(2), Core::ScriptValue(0.25), Core::ScriptValue(1)}, result);
		carrie->play("addFidget", {Core::ScriptValue(2), Core::ScriptValue(number("kCarrieWrapID"))}, result);
	}
	_bands = sprite("kBandsID", "kBandsZ");
	_wagon = sprite("kWagonID", "kWagonZ");
	_walk = _host._scene.add(_host.characterResource(number("kWalkToSlideID"), Shell::Character::Resources::kAnimation), 1);
	if (!_bands || !_wagon || !_walk)
		return false;
	_bands->visible = _state < 4;
	_wagon->visible = _walk->visible = false;
	if (world.get("R1CN4WagonPending")) {
		_phase = kDeliveryWait;
	} else if (world.get("R1CN4BandsPending")) {
		group("kBandsCharID", "kBandsSpchID", 2);
		_phase = kCollect;
	} else if (world.get("R1CN4ExitPending")) {
		_phase = kExitSpeech;
	} else if (!world.visits("r1cn4")) {
		group("kEnterCharID", "kEnterSpchID", number("kMaxEntrySpchs"));
	} else if (_state < 4) {
		bool speak = _state == 3;
		if (_state < 3) {
			Common::String key = Common::String::format("R1CN4State%dCount", _state);
			world.set(key, world.get(key) + 1);
			speak = world.get(key) == 1;
		}
		if (speak)
			group("kReEnterCharIDs", "kReEnterSpchIDs", number(Common::String::format("kMaxReEnterSpchs.%d", _state)), _state);
	}
	return true;
}

void RainbowSlide::update(uint32 time) {
	if (!_clock) {
		_start = _idle = time;
		_clock = true;
	}
	if (_host._nextSpeech < _host._speech.size()) {
		_idle = time;
		return;
	}
	World &world = _host._world;
	if (_phase == kDeliveryWait) {
		if (time - _start < 1500)
			return;
		if (_host._actors.contains("carrie")) {
			Core::ScriptValue result;
			_host._actors["carrie"]->play("removeFidgetGroup", {Core::ScriptValue(2)}, result);
		}
		_wagon->visible = true;
		world.set("R1CN4WagonHere", 1);
		world.set("R1CN4WagonPending", 0);
		group("kWagonCharID", "kWagonSpchID", number("kMaxWagonSpch"));
		setState(3);
		_phase = kDeliverySpeech;
	} else if (_phase == kDeliverySpeech) {
		_phase = kIdle;
	} else if (_phase == kCollect) {
		if (world.get("R1CN4BandsPending")) {
			world.set("item:1", MIN(25, world.get("item:1") + 1));
			_host.putAway();
			world.set("R1CN4BandsPending", 0);
		}
		_bands->visible = false;
		setState(4);
		_phase = kIdle;
	} else if (_phase == kExitSpeech) {
		_host._actors["reader"]->setVisible(false);
		_host._actors["sam"]->setVisible(false);
		_walk->visible = true;
		_walk->startAnimation(1);
		_phase = kExitAnimation;
	} else if (_phase == kExitAnimation) {
		if (!_walk->isRunning())
			_phase = kExitReady;
	} else if (_phase == kIdle && !_host._practice && static_cast<uint32>(number("gIdleHelpTime")) <= time - _idle) {
		int choice = _host._vm.rnd().number(1, number("kMaxIdleIndex"));
		group("kIdleCharID", "kIdleSpchID", number(Common::String::format("kMaxIdleIndexSpeeches.%d", choice)), choice);
		_idle = time;
	}
}

void RainbowSlide::carrieClick() {
	World &world = _host._world;
	int count = world.get("R1CN4CarrieClicks") + 1;
	world.set("R1CN4CarrieClicks", count);
	int groupIndex = 6;
	if (_state < 3 && count == 1)
		groupIndex = _state;
	else if (_state == 1) {
		_host.say(_host._page->text("kCarrieCharID.3.1"), number("kCarrieSpchID.3.1"));
		for (int i = world.visits("r1ia3") ? 2 : 4; i <= (world.visits("r1ia3") ? 3 : 4); i++)
			_host.say(_host._page->text(Common::String::format("kCarrieCharID.3.%d", i)), number(Common::String::format("kCarrieSpchID.3.%d", i)));
		return;
	} else if (_state == 2)
		groupIndex = _host._vm.rnd().number(4, 5);
	group("kCarrieCharID", "kCarrieSpchID", number(Common::String::format("kMaxCarrieSpch.%d", groupIndex)), groupIndex);
}

bool RainbowSlide::click(Common::Point point) {
	_idle = _host._time;
	if (busy())
		return true;
	if (_bands->visible && _bands->covers(point)) {
		if (_state < 3)
			carrieClick();
		else if (!_host._practice) {
			_host.playSounds({static_cast<uint16>(number("WAVE_COMPICKUP"))});
			_host._world.set("R1CN4BandsPending", 1);
			group("kBandsCharID", "kBandsSpchID", 2);
			_phase = kCollect;
		}
		return true;
	}
	for (auto &entry : _host._actors) {
		if (!entry._value->bounds().contains(point))
			continue;
		if (entry._key == "carrie")
			carrieClick();
		else if (entry._key == "paige") {
			int choice = _state == 1 ? _host._vm.rnd().number(1, 2) : _state + 1;
			_host.say(_host._page->text(Common::String::format("kHelpCharID.%d", choice)), number(Common::String::format("kHelpSpchID.%d", choice)));
		} else {
			int choice;
			if (_state == 1) {
				choice = _host._vm.rnd().number(1, _host._world.visits("r1ia3") ? 3 : 2);
				if (!_host._world.visits("r1ia3") && choice == 2)
					choice = 3;
			} else
				choice = _state < 4 ? _state + 2 : _host._vm.rnd().number(6, 7);
			group("kMainCharID", "kMainSpchID", number(Common::String::format("kMaxMainSpch.%d", choice)), choice);
		}
		return true;
	}
	for (const auto &exit : _host._page->exits()) {
		if (exit.destination == "r1rss" && exit.rect.contains(point)) {
			World &world = _host._world;
			int count = world.get("R1CN4SlideClicks") + 1;
			world.set("R1CN4SlideClicks", count);
			world.set("R1CN4ExitPending", 1);
			if (count == 1)
				group("kSlideCharID", "kSlideSpchID", number("kMaxSlideSpch"));
			_phase = kExitSpeech;
			return true;
		}
	}
	return false;
}

bool RainbowSlide::donate(uint item) {
	if (_host._practice || busy())
		return false;
	_host.interrupt();
	if (item != static_cast<uint>(number("kINV1Wagon")) || 3 <= _state) {
		int groupIndex = _state < 3 ? 1 : 2;
		group("kNoUseCharID", "kNoUseSpchID", number(Common::String::format("kMaxNoUseSpch.%d", groupIndex)), groupIndex);
		return false;
	}
	World &world = _host._world;
	if (world.get("item:7") <= 0)
		return false;
	world.set("item:7", world.get("item:7") - 1);
	world.set("R1CN4WagonPending", 1);
	_host.closeChest();
	_start = _host._time;
	_clock = true;
	_phase = kDeliveryWait;
	return true;
}

void RainbowSlide::pause(bool paused, uint32 elapsed) {
	if (!paused) {
		_start += elapsed;
		_idle += elapsed;
	}
	if (_phase == kExitAnimation)
		_walk->pauseAnimation(paused);
}

Common::String RainbowSlide::destination() const {
	return _phase == kExitReady ? "r1rss" : Common::String();
}

} // namespace RR1Cloud9
} // namespace Trinket
