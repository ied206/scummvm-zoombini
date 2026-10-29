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

constexpr const char *Catapult::kSignedIndexFormat;
constexpr const char *Catapult::kR1LG2PetSleepingKey;
constexpr const char *Catapult::kR1LG2CatapultReadyKey;
constexpr const char *Catapult::kR1LG2FeedPendingKey;
constexpr const char *Catapult::kPropertyPet1;
constexpr const char *Catapult::kGPetName;
constexpr const char *Catapult::kPropertyZ;
constexpr const char *Catapult::kPetZKey;
constexpr const char *Catapult::kBowlIDKey;
constexpr const char *Catapult::kBowlZKey;
constexpr const char *Catapult::kSandwichIDKey;
constexpr const char *Catapult::kSandwichZKey;
constexpr const char *Catapult::kEatingIDKey;
constexpr const char *Catapult::kGPetSleepIDName;
constexpr const char *Catapult::kClumpStaticIDKey;
constexpr const char *Catapult::kClumpZKey;
constexpr const char *Catapult::kClumpIDKey;
constexpr const char *Catapult::kCatapultIDKey;
constexpr const char *Catapult::kCatapultZKey;
constexpr const char *Catapult::kExitAnimIDKey;
constexpr const char *Catapult::kR1LG2ExitPendingKey;
constexpr const char *Catapult::kEnterCharIDKey;
constexpr const char *Catapult::kEnterSpchIDKey;
constexpr const char *Catapult::kMaxEntrySpchsKey;
constexpr const char *Catapult::kReEnterCharIDsKey;
constexpr const char *Catapult::kReEnterSpchIDsKey;
constexpr const char *Catapult::kMaxReEnterSpchsKey;
constexpr const char *Catapult::kPetSleepIDKey;
constexpr const char *Catapult::kFeedingSFXIDKey;
constexpr const char *Catapult::kR1LG2FeedCreatureKey;
constexpr const char *Catapult::kSandwichCharIDKey;
constexpr const char *Catapult::kSandwichSpchIDKey;
constexpr const char *Catapult::kMaxSandwichSpchsKey;
constexpr const char *Catapult::kPostEatCharIDKey;
constexpr const char *Catapult::kPostEatSpchIDKey;
constexpr const char *Catapult::kMaxPostEatSpchsKey;
constexpr const char *Catapult::kLetsGoCharIDKey;
constexpr const char *Catapult::kLetsGoSpchIDKey;
constexpr const char *Catapult::kReaderName;
constexpr const char *Catapult::kSamName;
constexpr const char *Catapult::kGIdleHelpTimeName;
constexpr const char *Catapult::kIdleCharIDKey;
constexpr const char *Catapult::kIdleSpchIDKey;
constexpr const char *Catapult::kMaxIdleSpchsKey;
constexpr const char *Catapult::kINV1FoodKey;
constexpr const char *Catapult::kItem9Key;
constexpr const char *Catapult::kItemCharSpch2Key;
constexpr const char *Catapult::kItemSpchsID2Key;
constexpr const char *Catapult::kMaxItemSpchs2Key;
constexpr const char *Catapult::kItemCharSpchKey;
constexpr const char *Catapult::kItemSpchsIDKey;
constexpr const char *Catapult::kItemSpchCountKey;
constexpr const char *Catapult::kMaxItemSpchsKey;
constexpr const char *Catapult::kR1LG2RodeCatapultKey;
constexpr const char *Catapult::kFirstExitCharIDKey;
constexpr const char *Catapult::kFirstExitSpchIDKey;
constexpr const char *Catapult::kSubSeqExitCharIDKey;
constexpr const char *Catapult::kSubSeqExitSpchIDKey;
constexpr const char *Catapult::kBowlCharIDKey;
constexpr const char *Catapult::kBowlSpchIDKey;
constexpr const char *Catapult::kMaxBowlSpchsKey;
constexpr const char *Catapult::kPetCharIDsKey;
constexpr const char *Catapult::kPetSpchIDsKey;
constexpr const char *Catapult::kMaxPetSpchsKey;
constexpr const char *Catapult::kPaigeName;
constexpr const char *Catapult::k21Name;
constexpr const char *Catapult::k1SignedIndexFormat;
constexpr const char *Catapult::kMaxHelpSpchs1Key;
constexpr const char *Catapult::kHelpSpchCharIDsKey;
constexpr const char *Catapult::kHelpSpchIDsKey;
constexpr const char *Catapult::kMainSpchCharIDsKey;
constexpr const char *Catapult::kMainSpchIDsKey;
constexpr const char *Catapult::kMaxMainSpchsKey;
constexpr const char *Catapult::kDontExitCharIDsKey;
constexpr const char *Catapult::kDontExitSpchIDsKey;
constexpr const char *Catapult::kMaxDontExitSpchsKey;
constexpr const char *Catapult::kNumDontExitSpchsKey;

int Catapult::number(const Common::String &key) const { return _host._page->number(key); }

void Catapult::line(const char *actors, const char *sounds, int index) {
	Common::String suffix = index ? Common::String::format(kSignedIndexFormat, index) : Common::String();
	_host.say(_host._page->text(Common::String(actors) + suffix), number(Common::String(sounds) + suffix));
}

void Catapult::group(const char *actors, const char *sounds, const char *count, int index) {
	Common::String suffix = index ? Common::String::format(kSignedIndexFormat, index) : Common::String();
	_host.speeches(Common::String(actors) + suffix, Common::String(sounds) + suffix, number(Common::String(count) + suffix));
}

bool Catapult::enter() {
	World &world = _host._world;
	_sleeping = world.get(kR1LG2PetSleepingKey) && world.get(kR1LG2CatapultReadyKey) && !world.get(kR1LG2FeedPendingKey);
	if (!world.get(kR1LG2FeedPendingKey))
		world.set(kR1LG2PetSleepingKey, _sleeping);
	if (!_host.actor(kPropertyPet1, kGPetName))
		return false;
	_host._actors[kPropertyPet1]->set(kPropertyZ, {}, Core::ScriptValue(number(kPetZKey)));
	_host._actors[kPropertyPet1]->setVisible(!_sleeping);
	auto sprite = [this](const char *key, const char *depth) {
		return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), number(depth));
	};
	_bowl = sprite(kBowlIDKey, kBowlZKey);
	_sandwich = sprite(kSandwichIDKey, kSandwichZKey);
	_eating = sprite(kEatingIDKey, kPetZKey);
	_sleep = sprite(kGPetSleepIDName, kPetZKey);
	_clump = sprite(kClumpStaticIDKey, kClumpZKey);
	_clumpAnim = sprite(kClumpIDKey, kClumpZKey);
	_catapult = sprite(kCatapultIDKey, kCatapultZKey);
	_exit = sprite(kExitAnimIDKey, kCatapultZKey);
	if (!_bowl || !_sandwich || !_eating || !_sleep || !_clump || !_clumpAnim || !_catapult || !_exit)
		return false;
	_bowl->visible = !_sleeping;
	_sleep->visible = _sleeping;
	_sandwich->visible = _eating->visible = _clumpAnim->visible = _exit->visible = false;
	if (world.get(kR1LG2ExitPendingKey)) {
		_phase = kExitSpeech;
	} else if (world.get(kR1LG2FeedPendingKey)) {
		_phase = kFeedWait;
	} else if (!world.visits(Page::kPageLG2))
		group(kEnterCharIDKey, kEnterSpchIDKey, kMaxEntrySpchsKey);
	else if (!_sleeping && world.visits(Page::kPageLG2) < 4)
		group(kReEnterCharIDsKey, kReEnterSpchIDsKey, kMaxReEnterSpchsKey, world.visits(Page::kPageLG2));
	else if (_sleeping)
		group(kReEnterCharIDsKey, kReEnterSpchIDsKey, kMaxReEnterSpchsKey, 4);
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
			_host.playSounds({static_cast<uint16>(number(kPetSleepIDKey))});
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
		_host.playSounds({static_cast<uint16>(number(kFeedingSFXIDKey))});
		_sandwich->visible = true;
		if (world.get(kR1LG2FeedPendingKey) == 1) {
			world.set(kR1LG2FeedCreatureKey, world.get(kR1LG2FeedCreatureKey) + 1);
			world.set(kR1LG2PetSleepingKey, 1);
			world.set(kR1LG2CatapultReadyKey, 1);
			world.set(kR1LG2FeedPendingKey, 2);
		}
		int count = world.get(kR1LG2FeedCreatureKey);
		line(kSandwichCharIDKey, kSandwichSpchIDKey, count == 1 ? 1 : _host._vm._rnd->getRandomNumberRng(1, number(kMaxSandwichSpchsKey)));
		_phase = kFeedSpeech;
	} else if (_phase == kFeedSpeech) {
		_eating->visible = true;
		_host._actors[kPropertyPet1]->setVisible(false);
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
		world.set(kR1LG2FeedPendingKey, 0);
		int count = world.get(kR1LG2FeedCreatureKey);
		line(kPostEatCharIDKey, kPostEatSpchIDKey, _host._vm._rnd->getRandomNumberRng(1, number(kMaxPostEatSpchsKey)));
		if (count == 1)
			line(kLetsGoCharIDKey, kLetsGoSpchIDKey);
		_phase = kSpeech;
	} else if (_phase == kSpeech)
		_phase = kIdle;
	else if (_phase == kExitSpeech) {
		_clumpAnim->stopAnimation();
		_clumpAnim->visible = _catapult->visible = false;
		_clump->visible = _exit->visible = true;
		_host._actors[kReaderName]->setVisible(false);
		_host._actors[kSamName]->setVisible(false);
		_exit->startAnimation(1);
		_phase = kExiting;
	} else if (_phase == kExiting) {
		if (!_exit->isRunning())
			_phase = kExitReady;
	} else if (_phase == kIdle && !_host._practice && static_cast<uint32>(number(kGIdleHelpTimeName)) <= time - _idle) {
		group(kIdleCharIDKey, kIdleSpchIDKey, kMaxIdleSpchsKey, _sleeping ? 4 : _host._vm._rnd->getRandomNumberRng(1, 3));
		_idle = time;
	}
}

bool Catapult::donate(uint item) {
	if (busy() || _host._practice)
		return false;
	_host.interrupt();
	if (!_sleeping && item == static_cast<uint>(number(kINV1FoodKey)) && 0 < _host._world.get(kItem9Key)) {
		_host._world.set(kItem9Key, _host._world.get(kItem9Key) - 1);
		_host._world.set(kR1LG2FeedPendingKey, 1);
		_host.closeChest();
		_start = _host._time;
		_phase = kFeedWait;
		return true;
	}
	if (_sleeping)
		line(kItemCharSpch2Key, kItemSpchsID2Key, _host._vm._rnd->getRandomNumberRng(1, number(kMaxItemSpchs2Key)));
	else
		group(kItemCharSpchKey, kItemSpchsIDKey, kItemSpchCountKey, _host._vm._rnd->getRandomNumberRng(1, number(kMaxItemSpchsKey)));
	return false;
}

void Catapult::ride() {
	World &world = _host._world;
	int count = world.get(kR1LG2RodeCatapultKey) + 1;
	world.set(kR1LG2RodeCatapultKey, count);
	world.set(kR1LG2CatapultReadyKey, 0);
	world.set(kR1LG2ExitPendingKey, 1);
	if (count == 1)
		line(kFirstExitCharIDKey, kFirstExitSpchIDKey);
	else
		line(kSubSeqExitCharIDKey, kSubSeqExitSpchIDKey, _host._vm._rnd->getRandomNumberRng(1, 2));
	_phase = kExitSpeech;
}

bool Catapult::click(Common::Point point) {
	_idle = _host._time;
	if (_bowl->visible && _bowl->covers(point)) {
		int choice = _host._vm._rnd->getRandomNumberRng(1, 2);
		if (choice == 2 && !_host._world.get(kItem9Key))
			choice = 3;
		group(kBowlCharIDKey, kBowlSpchIDKey, kMaxBowlSpchsKey, choice);
		return true;
	}
	if (_sleep->visible && _sleep->covers(point))
		return true;
	for (auto &entry : _host._actors) {
		if (entry._key.equals(kPropertyPet1) && _sleeping)
			continue;
		if (!entry._value->bounds().contains(point))
			continue;
		if (entry._key.equals(kPropertyPet1))
			group(kPetCharIDsKey, kPetSpchIDsKey, kMaxPetSpchsKey, _host._vm._rnd->getRandomNumberRng(1, 3));
		else if (entry._key.equals(kPaigeName)) {
			Common::String suffix;
			if (_sleeping)
				suffix = k21Name;
			else
				suffix = Common::String::format(k1SignedIndexFormat, _host._vm._rnd->getRandomNumberRng(1, number(kMaxHelpSpchs1Key)));
			_host.say(_host._page->text(kHelpSpchCharIDsKey + suffix), number(kHelpSpchIDsKey + suffix));
		} else {
			int choice = 7;
			if (!_sleeping) {
				if (_host._world.get(kR1LG2RodeCatapultKey))
					choice = _host._vm._rnd->getRandomNumberRng(4, 6);
				else {
					choice = _host._vm._rnd->getRandomNumberRng(1, 2) == 2 ? 3 : 1;
					if (choice == 1 && _host._world.get(kItem9Key))
						choice = 2;
				}
			}
			group(kMainSpchCharIDsKey, kMainSpchIDsKey, kMaxMainSpchsKey, choice);
		}
		return true;
	}
	for (const auto &exit : _host._page->exits()) {
		if (exit.destination.equals(Page::kPageRSS) && exit.rect.contains(point)) {
			if (_sleeping)
				ride();
			else
				group(kDontExitCharIDsKey, kDontExitSpchIDsKey, kMaxDontExitSpchsKey, _host._vm._rnd->getRandomNumberRng(1, number(kNumDontExitSpchsKey)));
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
	return _phase == kExitReady ? Page::kPageRSS : Common::String();
}
} // namespace RR1Cloud9
} // namespace Trinket
