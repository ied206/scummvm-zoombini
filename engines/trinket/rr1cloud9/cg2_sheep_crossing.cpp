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

constexpr const char *SheepCrossing::kR1CG2StateKey;
constexpr const char *SheepCrossing::kItem5Key;
constexpr const char *SheepCrossing::kR1CG2IdleCountKey;
constexpr const char *SheepCrossing::kR1CG2MainClicksKey;
constexpr const char *SheepCrossing::kR1CG2SheepClicksKey;
constexpr const char *SheepCrossing::kMaxSheepKey;
constexpr const char *SheepCrossing::kSheepLocSignedIndexKey;
constexpr const char *SheepCrossing::kSheepIDKey;
constexpr const char *SheepCrossing::k3Name;
constexpr const char *SheepCrossing::kSheepStandingIDKey;
constexpr const char *SheepCrossing::k1Name;
constexpr const char *SheepCrossing::k4Name;
constexpr const char *SheepCrossing::k2Name;
constexpr const char *SheepCrossing::kBlowWhistleIDKey;
constexpr const char *SheepCrossing::kSamZKey;
constexpr const char *SheepCrossing::kR1CG2WhistlePendingKey;
constexpr const char *SheepCrossing::kEnterCharIDKey;
constexpr const char *SheepCrossing::kEnterSpchIDKey;
constexpr const char *SheepCrossing::kMaxEntrySpchsKey;
constexpr const char *SheepCrossing::kR1CG2State2CountKey;
constexpr const char *SheepCrossing::kPreviousLocationKey;
constexpr const char *SheepCrossing::kR1CG2State3CountKey;
constexpr const char *SheepCrossing::kReEnterCharIDsKey;
constexpr const char *SheepCrossing::kReEnterSpchIDsKey;
constexpr const char *SheepCrossing::kNumReEnterSpchsKey;
constexpr const char *SheepCrossing::kSignedIndexSignedIndexFormat;
constexpr const char *SheepCrossing::kSamName;
constexpr const char *SheepCrossing::kSheepStandingSFXIDKey;
constexpr const char *SheepCrossing::kSheepLocUnsignedIndexKey;
constexpr const char *SheepCrossing::kR1CG2BlowCountKey;
constexpr const char *SheepCrossing::kWorksCharIDKey;
constexpr const char *SheepCrossing::kWorksSpchIDKey;
constexpr const char *SheepCrossing::kGIdleHelpTimeName;
constexpr const char *SheepCrossing::kMaxIdleIndexKey;
constexpr const char *SheepCrossing::kIdleCharIDKey;
constexpr const char *SheepCrossing::kIdleSpchIDKey;
constexpr const char *SheepCrossing::kMaxIdleIndexSpeechesKey;
constexpr const char *SheepCrossing::kSignedIndexFormat;
constexpr const char *SheepCrossing::kPreviousLocations1Key;
constexpr const char *SheepCrossing::kMaxSheepIndexKey;
constexpr const char *SheepCrossing::kMaxMainIndexKey;
constexpr const char *SheepCrossing::kSheepCharIDKey;
constexpr const char *SheepCrossing::kMainCharIDKey;
constexpr const char *SheepCrossing::kSheepSpchIDKey;
constexpr const char *SheepCrossing::kMainSpchIDKey;
constexpr const char *SheepCrossing::kMaxSheepIndexSpeechesKey;
constexpr const char *SheepCrossing::kMaxMainIndexSpeechesKey;
constexpr const char *SheepCrossing::kHSX1Name;
constexpr const char *SheepCrossing::kHSY1Name;
constexpr const char *SheepCrossing::kHSX2Name;
constexpr const char *SheepCrossing::kHSY2Name;
constexpr const char *SheepCrossing::kSignKey;
constexpr const char *SheepCrossing::kWAVECOMSOFTCLICKName;
constexpr const char *SheepCrossing::kSignCharIDKey;
constexpr const char *SheepCrossing::kSignSpchIDKey;
constexpr const char *SheepCrossing::kSheepKey;
constexpr const char *SheepCrossing::kPaigeName;
constexpr const char *SheepCrossing::kNumHelpSpchsKey;
constexpr const char *SheepCrossing::kHelpSpchCharIDsKey;
constexpr const char *SheepCrossing::kHelpSpchIDsKey;
constexpr const char *SheepCrossing::kINV1WhistleKey;
constexpr const char *SheepCrossing::kWrongItemCharIDKey;
constexpr const char *SheepCrossing::kWrongItemSpchIDKey;
constexpr const char *SheepCrossing::kMaxItemSpchsKey;
constexpr const char *SheepCrossing::kItemCharSpchKey;
constexpr const char *SheepCrossing::kItemSpchsIDKey;

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
	_state = CLIP(world.get(kR1CG2StateKey), 1, 3);
	if ((_state == 1 && world.get(kItem5Key)) || world.visits(Page::kPageCN4)) {
		_state = world.visits(Page::kPageCN4) ? 3 : 2;
		for (const char *key : {kR1CG2IdleCountKey, kR1CG2MainClicksKey, kR1CG2SheepClicksKey})
			world.set(key, 0);
	}
	world.set(kR1CG2StateKey, _state);
	for (int i = 1; i <= number(kMaxSheepKey); i++) {
		Common::String prefix = Common::String::format(kSheepLocSignedIndexKey, i);
		auto *walk = _host._scene.add(_host.characterResource(number(kSheepIDKey), Shell::Character::Resources::kAnimation), number(prefix + k3Name));
		auto *stand = _host._scene.add(_host.characterResource(number(kSheepStandingIDKey), Shell::Character::Resources::kAnimation), number(prefix + k3Name));
		if (!walk || !stand)
			return false;
		walk->y = stand->y = number(prefix + k1Name);
		stand->x = number(prefix + k4Name);
		walk->setFrame(number(prefix + k2Name));
		walk->playAnimation();
		stand->visible = false;
		_walking.push_back(walk);
		_standing.push_back(stand);
	}
	_whistle = _host._scene.add(_host.characterResource(number(kBlowWhistleIDKey), Shell::Character::Resources::kAnimation), number(kSamZKey));
	if (!_whistle)
		return false;
	_whistle->visible = false;
	_sound = new OM::Sound(_host._vm.mixer());
	_sound->play(_host._vm.resources(), _host.characterResource(number(kSheepIDKey), Shell::Character::Resources::kSound), Audio::Mixer::kSFXSoundType, 0);
	if (world.get(kR1CG2WhistlePendingKey))
		_phase = kWait;
	else if (!world.visits(Page::kPageCG2))
		group(kEnterCharIDKey, kEnterSpchIDKey, kMaxEntrySpchsKey, "");
	else {
		int choice = 0;
		if (_state == 1)
			choice = _host._vm.rnd().number(1, 2);
		else if (_state == 2)
			choice = increment(kR1CG2State2CountKey) == 1 ? 1 : 2;
		else if (world.get(kPreviousLocationKey) == 2 && increment(kR1CG2State3CountKey) == 1)
			choice = 1;
		if (choice)
			group(kReEnterCharIDsKey, kReEnterSpchIDsKey, kNumReEnterSpchsKey, Common::String::format(kSignedIndexSignedIndexFormat, _state, choice));
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
		_host._actors[kSamName]->setVisible(false);
		_whistle->startAnimation(1);
		_phase = kBlow;
	} else if (_phase == kBlow) {
		if (_whistle->isRunning())
			return;
		_whistle->visible = false;
		_host._actors[kSamName]->setVisible(true);
		_sound->stop();
		for (uint i = 0; i < _walking.size(); i++) {
			_walking[i]->visible = false;
			_walking[i]->stopAnimation();
			_standing[i]->visible = true;
			_standing[i]->startAnimation(1);
		}
		_host.playSounds({static_cast<uint16>(number(kSheepStandingSFXIDKey))});
		_phase = kStand;
	} else if (_phase == kStand) {
		for (auto *sheep : _standing) {
			if (sheep->isRunning())
				return;
		}
		for (uint i = 0; i < _standing.size(); i++) {
			Common::String prefix = Common::String::format(kSheepLocUnsignedIndexKey, i + 1);
			_standing[i]->setFrame(_standing[i]->getNumberOfFrames());
			_standing[i]->x = number(prefix + k4Name) + 12;
			_standing[i]->y = number(prefix + k1Name) - 49;
		}
		_moving = false;
		_host._world.set(kR1CG2WhistlePendingKey, 0);
		if (increment(kR1CG2BlowCountKey) == 1)
			_host.speeches(kWorksCharIDKey, kWorksSpchIDKey, 2);
		_phase = kSpeech;
	} else if (_phase == kSpeech)
		_phase = kIdle;
	else if (!_host._practice && static_cast<uint32>(number(kGIdleHelpTimeName)) <= time - _idle) {
		int choice = _host._vm.rnd().number(3, number(kMaxIdleIndexKey));
		if (increment(kR1CG2IdleCountKey) == 1 && _state < 3)
			choice = _state;
		group(kIdleCharIDKey, kIdleSpchIDKey, kMaxIdleIndexSpeechesKey, Common::String::format(kSignedIndexFormat, choice));
		_idle = time;
	}
}

void SheepCrossing::talk(bool sheep) {
	if (sheep && !_moving) {
		_destination = _host._page->text(kPreviousLocations1Key);
		return;
	}
	int choice = _host._vm.rnd().number(sheep ? 5 : 4, number(sheep ? kMaxSheepIndexKey : kMaxMainIndexKey));
	int count = increment(sheep ? kR1CG2SheepClicksKey : kR1CG2MainClicksKey);
	if (count == 1) {
		if (_state == 1)
			choice = 1;
		else if (_state == 2 && _moving)
			choice = 2;
		else if (sheep && _state == 3)
			choice = 4;
	} else if (count == 2 && _state == 2 && _moving)
		choice = 3;
	group(sheep ? kSheepCharIDKey : kMainCharIDKey, sheep ? kSheepSpchIDKey : kMainSpchIDKey,
		  sheep ? kMaxSheepIndexSpeechesKey : kMaxMainIndexSpeechesKey, Common::String::format(kSignedIndexFormat, choice));
}

bool SheepCrossing::click(Common::Point point) {
	_idle = _host._time;
	auto hotspot = [this, point](const char *prefix) {
		Common::String base(prefix);
		return Common::Rect(number(base + kHSX1Name), number(base + kHSY1Name), number(base + kHSX2Name), number(base + kHSY2Name)).contains(point);
	};
	if (hotspot(kSignKey)) {
		_host.playSounds({static_cast<uint16>(number(kWAVECOMSOFTCLICKName))});
		_host.say(_host._page->text(kSignCharIDKey), number(kSignSpchIDKey));
		return true;
	}
	if (hotspot(kSheepKey)) {
		talk(true);
		return true;
	}
	for (auto &entry : _host._actors) {
		if (!entry._value->bounds().contains(point))
			continue;
		if (entry._key.equals(kPaigeName)) {
			Common::String suffix = Common::String::format(kSignedIndexFormat, _state);
			suffix += Common::String::format(kSignedIndexFormat, _host._vm.rnd().number(1, number(kNumHelpSpchsKey + suffix)));
			_host.say(_host._page->text(kHelpSpchCharIDsKey + suffix), number(kHelpSpchIDsKey + suffix));
		} else
			talk(false);
		return true;
	}
	for (const auto &exit : _host._page->exits()) {
		if (_moving && exit.destination.equals(_host._page->text(kPreviousLocations1Key)) && exit.rect.contains(point)) {
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
	if (_moving && item == static_cast<uint>(number(kINV1WhistleKey)) && _host._world.get(kItem5Key)) {
		_host.closeChest();
		_host._world.set(kR1CG2WhistlePendingKey, 1);
		_start = _host._time;
		_clock = true;
		_phase = kWait;
		return true;
	}
	if (_moving)
		_host.say(_host._page->text(kWrongItemCharIDKey), number(kWrongItemSpchIDKey));
	else {
		int choice = _host._vm.rnd().number(1, number(kMaxItemSpchsKey));
		if (item == static_cast<uint>(number(kINV1WhistleKey)))
			choice = 3;
		Common::String suffix = Common::String::format(kSignedIndexFormat, choice);
		_host.say(_host._page->text(kItemCharSpchKey + suffix), number(kItemSpchsIDKey + suffix));
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
