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

constexpr const char *RainbowSlide::kR1CN4GameStateKey;
constexpr const char *RainbowSlide::kSignedIndexFormat;
constexpr const char *RainbowSlide::kItem7Key;
constexpr const char *RainbowSlide::kR1CN4CarrieClicksKey;
constexpr const char *RainbowSlide::kCarrieActor;
constexpr const char *RainbowSlide::kCarrieKey;
constexpr const char *RainbowSlide::kStackIDKey;
constexpr const char *RainbowSlide::kStackZKey;
constexpr const char *RainbowSlide::kStuffIDKey;
constexpr const char *RainbowSlide::kStuffZKey;
constexpr const char *RainbowSlide::kPropertyAnimateSettled;
constexpr const char *RainbowSlide::kMethodAddFidgetGroup;
constexpr const char *RainbowSlide::kMethodAddFidget;
constexpr const char *RainbowSlide::kCarrieWrapIDKey;
constexpr const char *RainbowSlide::kBandsIDKey;
constexpr const char *RainbowSlide::kBandsZKey;
constexpr const char *RainbowSlide::kWagonIDKey;
constexpr const char *RainbowSlide::kWagonZKey;
constexpr const char *RainbowSlide::kWalkToSlideIDKey;
constexpr const char *RainbowSlide::kR1CN4WagonPendingKey;
constexpr const char *RainbowSlide::kR1CN4BandsPendingKey;
constexpr const char *RainbowSlide::kBandsCharIDKey;
constexpr const char *RainbowSlide::kBandsSpchIDKey;
constexpr const char *RainbowSlide::kR1CN4ExitPendingKey;
constexpr const char *RainbowSlide::kEnterCharIDKey;
constexpr const char *RainbowSlide::kEnterSpchIDKey;
constexpr const char *RainbowSlide::kMaxEntrySpchsKey;
constexpr const char *RainbowSlide::kR1CN4StateSignedIndexCountKey;
constexpr const char *RainbowSlide::kReEnterCharIDsKey;
constexpr const char *RainbowSlide::kReEnterSpchIDsKey;
constexpr const char *RainbowSlide::kMaxReEnterSpchsSignedIndexKey;
constexpr const char *RainbowSlide::kMethodRemoveFidgetGroup;
constexpr const char *RainbowSlide::kR1CN4WagonHereKey;
constexpr const char *RainbowSlide::kWagonCharIDKey;
constexpr const char *RainbowSlide::kWagonSpchIDKey;
constexpr const char *RainbowSlide::kMaxWagonSpchKey;
constexpr const char *RainbowSlide::kItem1Key;
constexpr const char *RainbowSlide::kReaderName;
constexpr const char *RainbowSlide::kSamName;
constexpr const char *RainbowSlide::kGIdleHelpTimeName;
constexpr const char *RainbowSlide::kMaxIdleIndexKey;
constexpr const char *RainbowSlide::kIdleCharIDKey;
constexpr const char *RainbowSlide::kIdleSpchIDKey;
constexpr const char *RainbowSlide::kMaxIdleIndexSpeechesSignedIndexKey;
constexpr const char *RainbowSlide::kCarrieCharID31Key;
constexpr const char *RainbowSlide::kCarrieSpchID31Key;
constexpr const char *RainbowSlide::kCarrieCharID3SignedIndexKey;
constexpr const char *RainbowSlide::kCarrieSpchID3SignedIndexKey;
constexpr const char *RainbowSlide::kCarrieCharIDKey;
constexpr const char *RainbowSlide::kCarrieSpchIDKey;
constexpr const char *RainbowSlide::kMaxCarrieSpchSignedIndexKey;
constexpr const char *RainbowSlide::kWAVECOMPICKUPName;
constexpr const char *RainbowSlide::kPaigeName;
constexpr const char *RainbowSlide::kHelpCharIDSignedIndexKey;
constexpr const char *RainbowSlide::kHelpSpchIDSignedIndexKey;
constexpr const char *RainbowSlide::kMainCharIDKey;
constexpr const char *RainbowSlide::kMainSpchIDKey;
constexpr const char *RainbowSlide::kMaxMainSpchSignedIndexKey;
constexpr const char *RainbowSlide::kR1CN4SlideClicksKey;
constexpr const char *RainbowSlide::kSlideCharIDKey;
constexpr const char *RainbowSlide::kSlideSpchIDKey;
constexpr const char *RainbowSlide::kMaxSlideSpchKey;
constexpr const char *RainbowSlide::kINV1WagonKey;
constexpr const char *RainbowSlide::kNoUseCharIDKey;
constexpr const char *RainbowSlide::kNoUseSpchIDKey;
constexpr const char *RainbowSlide::kMaxNoUseSpchSignedIndexKey;

int RainbowSlide::number(const Common::String &key) const { return _host._page->number(key); }

void RainbowSlide::setState(int state) {
	_state = state;
	if (!_host._practice)
		_host._world.set(kR1CN4GameStateKey, state);
}

void RainbowSlide::group(const char *characters, const char *speech, int count, int index) {
	Common::String suffix = index ? Common::String::format(kSignedIndexFormat, index) : Common::String();
	_host.speeches(Common::String(characters) + suffix, Common::String(speech) + suffix, count);
}

bool RainbowSlide::enter() {
	World &world = _host._world;
	_state = CLIP(world.get(kR1CN4GameStateKey), 1, 4);
	if (_state == 1 && world.get(kItem7Key)) {
		setState(2);
		if (1 < world.get(kR1CN4CarrieClicksKey))
			world.set(kR1CN4CarrieClicksKey, 1);
	}
	auto sprite = [this](const char *key, const char *depth) {
		return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), number(depth));
	};
	if (_state < 3) {
		if (!_host.actor(kCarrieActor, kCarrieKey) || !sprite(kStackIDKey, kStackZKey) || !sprite(kStuffIDKey, kStuffZKey))
			return false;
		auto *carrie = _host._actors[kCarrieActor];
		Core::ScriptValue result;
		carrie->set(kPropertyAnimateSettled, {}, Core::ScriptValue(0));
		carrie->play(kMethodAddFidgetGroup, {Core::ScriptValue(2), Core::ScriptValue(0.25), Core::ScriptValue(1)}, result);
		carrie->play(kMethodAddFidget, {Core::ScriptValue(2), Core::ScriptValue(number(kCarrieWrapIDKey))}, result);
	}
	_bands = sprite(kBandsIDKey, kBandsZKey);
	_wagon = sprite(kWagonIDKey, kWagonZKey);
	_walk = _host._scene.add(_host.characterResource(number(kWalkToSlideIDKey), Shell::Character::Resources::kAnimation), 1);
	if (!_bands || !_wagon || !_walk)
		return false;
	_bands->visible = _state < 4;
	_wagon->visible = _walk->visible = false;
	if (world.get(kR1CN4WagonPendingKey)) {
		_phase = kDeliveryWait;
	} else if (world.get(kR1CN4BandsPendingKey)) {
		group(kBandsCharIDKey, kBandsSpchIDKey, 2);
		_phase = kCollect;
	} else if (world.get(kR1CN4ExitPendingKey)) {
		_phase = kExitSpeech;
	} else if (!world.visits(Page::kPageCN4)) {
		group(kEnterCharIDKey, kEnterSpchIDKey, number(kMaxEntrySpchsKey));
	} else if (_state < 4) {
		bool speak = _state == 3;
		if (_state < 3) {
			Common::String key = Common::String::format(kR1CN4StateSignedIndexCountKey, _state);
			world.set(key, world.get(key) + 1);
			speak = world.get(key) == 1;
		}
		if (speak)
			group(kReEnterCharIDsKey, kReEnterSpchIDsKey, number(Common::String::format(kMaxReEnterSpchsSignedIndexKey, _state)), _state);
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
		if (_host._actors.contains(kCarrieActor)) {
			Core::ScriptValue result;
			_host._actors[kCarrieActor]->play(kMethodRemoveFidgetGroup, {Core::ScriptValue(2)}, result);
		}
		_wagon->visible = true;
		world.set(kR1CN4WagonHereKey, 1);
		world.set(kR1CN4WagonPendingKey, 0);
		group(kWagonCharIDKey, kWagonSpchIDKey, number(kMaxWagonSpchKey));
		setState(3);
		_phase = kDeliverySpeech;
	} else if (_phase == kDeliverySpeech) {
		_phase = kIdle;
	} else if (_phase == kCollect) {
		if (world.get(kR1CN4BandsPendingKey)) {
			world.set(kItem1Key, MIN(25, world.get(kItem1Key) + 1));
			_host.putAway();
			world.set(kR1CN4BandsPendingKey, 0);
		}
		_bands->visible = false;
		setState(4);
		_phase = kIdle;
	} else if (_phase == kExitSpeech) {
		_host._actors[kReaderName]->setVisible(false);
		_host._actors[kSamName]->setVisible(false);
		_walk->visible = true;
		_walk->startAnimation(1);
		_phase = kExitAnimation;
	} else if (_phase == kExitAnimation) {
		if (!_walk->isRunning())
			_phase = kExitReady;
	} else if (_phase == kIdle && !_host._practice && static_cast<uint32>(number(kGIdleHelpTimeName)) <= time - _idle) {
		int choice = _host._vm.rnd().number(1, number(kMaxIdleIndexKey));
		group(kIdleCharIDKey, kIdleSpchIDKey, number(Common::String::format(kMaxIdleIndexSpeechesSignedIndexKey, choice)), choice);
		_idle = time;
	}
}

void RainbowSlide::carrieClick() {
	World &world = _host._world;
	int count = world.get(kR1CN4CarrieClicksKey) + 1;
	world.set(kR1CN4CarrieClicksKey, count);
	int groupIndex = 6;
	if (_state < 3 && count == 1)
		groupIndex = _state;
	else if (_state == 1) {
		_host.say(_host._page->text(kCarrieCharID31Key), number(kCarrieSpchID31Key));
		for (int i = world.visits(Page::kPageIA3) ? 2 : 4; i <= (world.visits(Page::kPageIA3) ? 3 : 4); i++)
			_host.say(_host._page->text(Common::String::format(kCarrieCharID3SignedIndexKey, i)), number(Common::String::format(kCarrieSpchID3SignedIndexKey, i)));
		return;
	} else if (_state == 2)
		groupIndex = _host._vm.rnd().number(4, 5);
	group(kCarrieCharIDKey, kCarrieSpchIDKey, number(Common::String::format(kMaxCarrieSpchSignedIndexKey, groupIndex)), groupIndex);
}

bool RainbowSlide::click(Common::Point point) {
	_idle = _host._time;
	if (busy())
		return true;
	if (_bands->visible && _bands->covers(point)) {
		if (_state < 3)
			carrieClick();
		else if (!_host._practice) {
			_host.playSounds({static_cast<uint16>(number(kWAVECOMPICKUPName))});
			_host._world.set(kR1CN4BandsPendingKey, 1);
			group(kBandsCharIDKey, kBandsSpchIDKey, 2);
			_phase = kCollect;
		}
		return true;
	}
	for (auto &entry : _host._actors) {
		if (!entry._value->bounds().contains(point))
			continue;
		if (entry._key.equals(kCarrieActor))
			carrieClick();
		else if (entry._key.equals(kPaigeName)) {
			int choice = _state == 1 ? _host._vm.rnd().number(1, 2) : _state + 1;
			_host.say(_host._page->text(Common::String::format(kHelpCharIDSignedIndexKey, choice)), number(Common::String::format(kHelpSpchIDSignedIndexKey, choice)));
		} else {
			int choice;
			if (_state == 1) {
				choice = _host._vm.rnd().number(1, _host._world.visits(Page::kPageIA3) ? 3 : 2);
				if (!_host._world.visits(Page::kPageIA3) && choice == 2)
					choice = 3;
			} else
				choice = _state < 4 ? _state + 2 : _host._vm.rnd().number(6, 7);
			group(kMainCharIDKey, kMainSpchIDKey, number(Common::String::format(kMaxMainSpchSignedIndexKey, choice)), choice);
		}
		return true;
	}
	for (const auto &exit : _host._page->exits()) {
		if (exit.destination.equals(Page::kPageRSS) && exit.rect.contains(point)) {
			World &world = _host._world;
			int count = world.get(kR1CN4SlideClicksKey) + 1;
			world.set(kR1CN4SlideClicksKey, count);
			world.set(kR1CN4ExitPendingKey, 1);
			if (count == 1)
				group(kSlideCharIDKey, kSlideSpchIDKey, number(kMaxSlideSpchKey));
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
	if (item != static_cast<uint>(number(kINV1WagonKey)) || 3 <= _state) {
		int groupIndex = _state < 3 ? 1 : 2;
		group(kNoUseCharIDKey, kNoUseSpchIDKey, number(Common::String::format(kMaxNoUseSpchSignedIndexKey, groupIndex)), groupIndex);
		return false;
	}
	World &world = _host._world;
	if (world.get(kItem7Key) <= 0)
		return false;
	world.set(kItem7Key, world.get(kItem7Key) - 1);
	world.set(kR1CN4WagonPendingKey, 1);
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
	return _phase == kExitReady ? Page::kPageRSS : Common::String();
}

} // namespace RR1Cloud9
} // namespace Trinket
