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

constexpr const char *RockHead::kSignedIndexFormat;
constexpr const char *RockHead::kSetIDName;
constexpr const char *RockHead::kLipsIDName;
constexpr const char *RockHead::kZName;
constexpr const char *RockHead::kRegisterFidgetsToSettleName;
constexpr const char *RockHead::kRegisterSpeechToSettleName;
constexpr const char *RockHead::kLoadSpeechAnimsInBkgdName;
constexpr const char *RockHead::kAnimateSettledName;
constexpr const char *RockHead::kMethodSetSpeechAnims;
constexpr const char *RockHead::kBodyIDSuffix;
constexpr const char *RockHead::kMethodAddFidgetGroup;
constexpr const char *RockHead::kMethodAddFidget;
constexpr const char *RockHead::kFidg1IDName;
constexpr const char *RockHead::kFidgetIDSuffixFormat;
constexpr const char *RockHead::kSamName;
constexpr const char *RockHead::kSamKey;
constexpr const char *RockHead::kPaigeName;
constexpr const char *RockHead::kPaigeKey;
constexpr const char *RockHead::kChesterFidgIDSignedIndexKey;
constexpr const char *RockHead::kGClosedChesterZName;
constexpr const char *RockHead::kR1IG1HatHereKey;
constexpr const char *RockHead::kRockActor;
constexpr const char *RockHead::kRockKey;
constexpr const char *RockHead::kMethodRemoveFidgetGroup;
constexpr const char *RockHead::kRockFdgtIDKey;
constexpr const char *RockHead::kR1IG1StateKey;
constexpr const char *RockHead::kR1IG1HoopHereKey;
constexpr const char *RockHead::kItem6Key;
constexpr const char *RockHead::kR1IG1LadderHereKey;
constexpr const char *RockHead::kR1IG1PendingKey;
constexpr const char *RockHead::kReaderName;
constexpr const char *RockHead::kReaderKey;
constexpr const char *RockHead::kLadderIDKey;
constexpr const char *RockHead::kLadderZKey;
constexpr const char *RockHead::kHatIDKey;
constexpr const char *RockHead::kHatZKey;
constexpr const char *RockHead::kHoop2IDKey;
constexpr const char *RockHead::kHoopZKey;
constexpr const char *RockHead::kPreviousLocationKey;
constexpr const char *RockHead::kReaderWalkInIDKey;
constexpr const char *RockHead::kSamWalkInIDKey;
constexpr const char *RockHead::kEnterCharIDKey;
constexpr const char *RockHead::kEnterSpchIDKey;
constexpr const char *RockHead::kMaxEntrySpchsKey;
constexpr const char *RockHead::kR1IG1RainbowSlideCountKey;
constexpr const char *RockHead::kReEnterCharIDsKey;
constexpr const char *RockHead::kReEnterSpchIDsKey;
constexpr const char *RockHead::kMaxReEnterSpchsKey;
constexpr const char *RockHead::kReaderClimbIDKey;
constexpr const char *RockHead::kReaderDecendIDKey;
constexpr const char *RockHead::kSamClimbIDKey;
constexpr const char *RockHead::kSamDecendIDKey;
constexpr const char *RockHead::kLadderCharIDKey;
constexpr const char *RockHead::kLadderSpchIDKey;
constexpr const char *RockHead::kHatCharIDKey;
constexpr const char *RockHead::kHatSpchIDKey;
constexpr const char *RockHead::kNumHatSpchsKey;
constexpr const char *RockHead::kHoopPresentedIDKey;
constexpr const char *RockHead::kHoopPresentedCharIDKey;
constexpr const char *RockHead::kHoopPresentedSpchIDKey;
constexpr const char *RockHead::kItem2Key;
constexpr const char *RockHead::kGIdleHelpTimeName;
constexpr const char *RockHead::kMaxIdleIndexKey;
constexpr const char *RockHead::kIdleCharIDKey;
constexpr const char *RockHead::kIdleSpchIDKey;
constexpr const char *RockHead::kMaxIdleIndexSpeechesKey;
constexpr const char *RockHead::k1SignedIndexFormat;
constexpr const char *RockHead::kMainSpchCharIDsKey;
constexpr const char *RockHead::kMainSpchIDsKey;
constexpr const char *RockHead::kMaxMainSpchsKey;
constexpr const char *RockHead::kMainSpchCharIDs2Key;
constexpr const char *RockHead::kMainSpchIDs2Key;
constexpr const char *RockHead::k3SignedIndexFormat;
constexpr const char *RockHead::kTalkedToRockHeadKey;
constexpr const char *RockHead::k21Name;
constexpr const char *RockHead::kRockCharID31Key;
constexpr const char *RockHead::kRockSpchID31Key;
constexpr const char *RockHead::kR1IG1RockClicksKey;
constexpr const char *RockHead::kR1LN2StateKey;
constexpr const char *RockHead::kItem8Key;
constexpr const char *RockHead::k32SignedIndexFormat;
constexpr const char *RockHead::k4SignedIndexFormat;
constexpr const char *RockHead::kRockCharIDKey;
constexpr const char *RockHead::kRockSpchIDKey;
constexpr const char *RockHead::kMaxRockSpchKey;
constexpr const char *RockHead::kWAVECOMSOFTCLICKName;
constexpr const char *RockHead::kMaxHoopSpchsKey;
constexpr const char *RockHead::kHoopCharIDKey;
constexpr const char *RockHead::kHoopSpchIDKey;
constexpr const char *RockHead::kNumHoopSpchsKey;
constexpr const char *RockHead::kWAVECOMPICKUPName;
constexpr const char *RockHead::kGetHoopCharIDKey;
constexpr const char *RockHead::kGetHoopSpchIDKey;
constexpr const char *RockHead::kHSX1Name;
constexpr const char *RockHead::kHSY1Name;
constexpr const char *RockHead::kHSX2Name;
constexpr const char *RockHead::kHSY2Name;
constexpr const char *RockHead::kHoopKey;
constexpr const char *RockHead::kMaxHelpSpchsSignedIndexKey;
constexpr const char *RockHead::kSignedIndexSignedIndexFormat;
constexpr const char *RockHead::kHelpSpchCharIDsKey;
constexpr const char *RockHead::kHelpSpchIDsKey;
constexpr const char *RockHead::kLadderKey;
constexpr const char *RockHead::k1Name;
constexpr const char *RockHead::k2Name;
constexpr const char *RockHead::k3Name;
constexpr const char *RockHead::kItemUnsignedIndexFormat;
constexpr const char *RockHead::kRockCharID211Key;
constexpr const char *RockHead::kRockSpchID211Key;
constexpr const char *RockHead::kMaxWrongColorSpchsKey;
constexpr const char *RockHead::kWrongColorCharIDKey;
constexpr const char *RockHead::kWrongColorSpchIDKey;
constexpr const char *RockHead::kMaxItemSpchsKey;
constexpr const char *RockHead::kItemCharSpchKey;
constexpr const char *RockHead::kItemSpchsIDKey;

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
	Common::String suffix = Common::String::format(kSignedIndexFormat, pose);
	Common::String base(prefix);
	int settle = number(base + kSetIDName + suffix);
	int lips = number(base + kLipsIDName + suffix);
	if (!lips)
		lips = settle;
	Common::ScopedPtr<Shell::Character> actor(new Shell::Character(_host._scene, _host._vm.mixer(), _host._vm.resources(), *_host._vm._rnd, _host));
	if (!actor->create(settle ? settle : lips, number(base + kZName)))
		return false;
	actor->movable = false;
	for (const char *key : {kRegisterFidgetsToSettleName, kRegisterSpeechToSettleName, kLoadSpeechAnimsInBkgdName, kAnimateSettledName})
		actor->set(key, {}, Core::ScriptValue(0));
	Core::ScriptValue result;
	actor->play(kMethodSetSpeechAnims, {Core::ScriptValue(lips), Core::ScriptValue(1), Core::ScriptValue(1),
						 Core::ScriptValue(number(base + kBodyIDSuffix + suffix)), Core::ScriptValue(0), Core::ScriptValue(0)}, result);
	actor->play(kMethodAddFidgetGroup, {Core::ScriptValue(1), Core::ScriptValue(5), Core::ScriptValue(10)}, result);
	actor->play(kMethodAddFidget, {Core::ScriptValue(1), Core::ScriptValue(settle ? settle : lips)}, result);
	if (number(base + kFidg1IDName + suffix)) {
		actor->play(kMethodAddFidgetGroup, {Core::ScriptValue(2), Core::ScriptValue(15), Core::ScriptValue(30)}, result);
		for (int i = 1; i <= 2; i++)
			actor->play(kMethodAddFidget, {Core::ScriptValue(2), Core::ScriptValue(number(base + Common::String::format(kFidgetIDSuffixFormat, i) + suffix))}, result);
	}
	_host._actors[name] = actor.release();
	return true;
}

bool RockHead::companions(int pose) {
	if (!character(kSamName, kSamKey, pose) || !character(kPaigeName, kPaigeKey, pose))
		return false;
	if (_host._chester)
		_host._scene.remove(_host._chester);
	_host._chester = _host._scene.add(_host.characterResource(number(Common::String::format(kChesterFidgIDSignedIndexKey, pose)),
															  Shell::Character::Resources::kAnimation),
									  number(kGClosedChesterZName));
	return _host._chester != nullptr;
}

bool RockHead::rock() {
	int pose = _host._world.get(kR1IG1HatHereKey) ? 2 : 1;
	if (!character(kRockActor, kRockKey, pose))
		return false;
	Core::ScriptValue result;
	_host._actors[kRockActor]->play(kMethodRemoveFidgetGroup, {Core::ScriptValue(1)}, result);
	if (pose == 1) {
		_host._actors[kRockActor]->play(kMethodAddFidgetGroup, {Core::ScriptValue(1), Core::ScriptValue(5), Core::ScriptValue(10)}, result);
		_host._actors[kRockActor]->play(kMethodAddFidget, {Core::ScriptValue(1), Core::ScriptValue(number(kRockFdgtIDKey))}, result);
	}
	return true;
}

bool RockHead::enter() {
	World &world = _host._world;
	if (!world.get(kR1IG1StateKey)) {
		world.set(kR1IG1StateKey, 1);
		world.set(kR1IG1HoopHereKey, 1);
	}
	_state = CLIP(world.get(kR1IG1StateKey), 1, 4);
	if (_state == 1 && world.get(kItem6Key))
		_state = 2;
	if (_state < 3 && world.get(kR1IG1LadderHereKey))
		_state = 3;
	if (_state == 3 && !world.get(kR1IG1HoopHereKey))
		_state = 4;
	world.set(kR1IG1StateKey, _state);
	_firstVisit = !world.visits(Page::kPageIG1);
	int pending = world.get(kR1IG1PendingKey);
	_pose = pending == 8 || pending == 2 ? 2 : 1;
	if (!character(kReaderName, kReaderKey, _pose) || !companions(_pose) || !rock())
		return false;
	auto sprite = [this](const char *key, const char *depth) {
		return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), number(depth));
	};
	_ladder = sprite(kLadderIDKey, kLadderZKey);
	_hat = sprite(kHatIDKey, kHatZKey);
	_hoop = sprite(kHoop2IDKey, kHoopZKey);
	if (!_ladder || !_hat || !_hoop)
		return false;
	_ladder->visible = world.get(kR1IG1LadderHereKey);
	_hat->visible = world.get(kR1IG1HatHereKey);
	_hoop->visible = _hat->visible && world.get(kR1IG1HoopHereKey);
	_hoop->boxTouchy = true;
	if (pending == 6 || pending == 8)
		_phase = pending == 6 ? kLadderWait : kHatWait;
	else if (pending == 2)
		_phase = kCollect;
	else if (world.get(kPreviousLocationKey) == 20) {
		_host._actors[kReaderName]->playAnim(number(kReaderWalkInIDKey));
		_host._actors[kSamName]->playAnim(number(kSamWalkInIDKey));
		_host._actors[kPaigeName]->setVisible(false);
		_host._chester->visible = false;
		_phase = kArrival;
	} else
		entrySpeech();
	return true;
}

void RockHead::entrySpeech() {
	World &world = _host._world;
	if (_firstVisit)
		group(kEnterCharIDKey, kEnterSpchIDKey, number(kMaxEntrySpchsKey));
	else if (world.get(kPreviousLocationKey) == 20) {
		int count = world.get(kR1IG1RainbowSlideCountKey) + 1;
		world.set(kR1IG1RainbowSlideCountKey, count);
		Common::String suffix = Common::String::format(kSignedIndexFormat, count == 1 ? 1 : _host._vm._rnd->getRandomNumberRng(2, 3));
		group(kReEnterCharIDsKey + suffix, kReEnterSpchIDsKey + suffix, number(kMaxReEnterSpchsKey + suffix));
	}
}

void RockHead::climb() {
	_host.closeChest();
	_host._actors[kReaderName]->playAnim(number(_pose == 1 ? kReaderClimbIDKey : kReaderDecendIDKey));
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
		if (_host._actors[kReaderName]->isBusy() || _host._actors[kSamName]->isBusy())
			return;
		_host._actors[kPaigeName]->setVisible(true);
		_host._chester->visible = true;
		entrySpeech();
		_phase = kIdle;
	} else if (_phase == kReaderClimb) {
		if (_host._actors[kReaderName]->isBusy())
			return;
		if (!character(kReaderName, kReaderKey, 3 - _pose))
			error("RR1: cannot load Reader's climbing pose");
		_host._actors[kSamName]->playAnim(number(_pose == 1 ? kSamClimbIDKey : kSamDecendIDKey));
		_phase = kSamClimb;
	} else if (_phase == kSamClimb) {
		if (_host._actors[kSamName]->isBusy())
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
			world.set(kR1IG1LadderHereKey, 1);
			world.set(kR1IG1PendingKey, 0);
			_state = 3;
			world.set(kR1IG1StateKey, _state);
			group(kLadderCharIDKey, kLadderSpchIDKey, 2);
			_phase = kSpeech;
		} else {
			_hat->visible = true;
			Core::ScriptValue result;
			_host._actors[kRockActor]->play(kMethodRemoveFidgetGroup, {Core::ScriptValue(1)}, result);
			group(kHatCharIDKey, kHatSpchIDKey, number(kNumHatSpchsKey));
			_phase = kHatSpeech;
		}
	} else if (_phase == kHatSpeech) {
		world.set(kR1IG1HatHereKey, 1);
		world.set(kR1IG1PendingKey, 0);
		_hoop->visible = world.get(kR1IG1HoopHereKey);
		_host.playSounds({static_cast<uint16>(number(kHoopPresentedIDKey))});
		if (!rock())
			error("RR1: cannot load Rock Head's hat pose");
		line(kHoopPresentedCharIDKey, kHoopPresentedSpchIDKey);
		_phase = kSpeech;
	} else if (_phase == kCollect) {
		if (world.get(kR1IG1PendingKey) == 2) {
			world.set(kItem2Key, MIN(25, world.get(kItem2Key) + 1));
			_host.putAway();
			world.set(kR1IG1HoopHereKey, 0);
			world.set(kR1IG1PendingKey, 0);
		}
		_hoop->visible = false;
		_state = 4;
		world.set(kR1IG1StateKey, _state);
		_phase = kIdle;
	} else if (_phase == kSpeech)
		_phase = kIdle;
	else if (_phase == kIdle && !_host._practice && static_cast<uint32>(number(kGIdleHelpTimeName)) <= time - _idle) {
		Common::String suffix = Common::String::format(kSignedIndexFormat, _state < 3 ? 1 : _pose + 1);
		suffix += Common::String::format(kSignedIndexFormat, _host._vm._rnd->getRandomNumberRng(1, number(kMaxIdleIndexKey + suffix)));
		group(kIdleCharIDKey + suffix, kIdleSpchIDKey + suffix, number(kMaxIdleIndexSpeechesKey + suffix));
		_idle = time;
	}
}

void RockHead::mainSpeech() {
	if (_state == 1) {
		int choice = _host._vm._rnd->getRandomNumberRng(1, _host._world.visits(Page::kPageCA1) ? 4 : 3);
		if (!_host._world.visits(Page::kPageCA1) && choice == 3)
			choice = 4;
		Common::String suffix = Common::String::format(k1SignedIndexFormat, choice);
		group(kMainSpchCharIDsKey + suffix, kMainSpchIDsKey + suffix, number(kMaxMainSpchsKey + suffix));
	} else if (_state == 2)
		line(kMainSpchCharIDs2Key, kMainSpchIDs2Key);
	else if (_state == 3 && _pose == 1) {
		Common::String suffix = Common::String::format(k3SignedIndexFormat, _host._vm._rnd->getRandomNumberRng(1, 2));
		line(kMainSpchCharIDsKey + suffix, kMainSpchIDsKey + suffix);
	} else
		rockSpeech();
}

void RockHead::rockSpeech() {
	World &world = _host._world;
	world.set(kTalkedToRockHeadKey, 1);
	if (_state == 1) {
		mainSpeech();
		return;
	}
	Common::String suffix;
	if (_state == 2)
		suffix = k21Name;
	else if (_state == 3 && !world.get(kR1IG1HatHereKey)) {
		if (_pose == 1) {
			line(kRockCharID31Key, kRockSpchID31Key);
			return;
		}
		int count = world.get(kR1IG1RockClicksKey) + 1;
		world.set(kR1IG1RockClicksKey, count);
		int choice;
		if (count == 1)
			choice = 1 < world.get(kR1LN2StateKey) ? 2 : 1;
		else {
			if (_moreClicks == 1) {
				if (world.get(kItem8Key))
					choice = 5;
				else if (world.visits(Page::kPageCN3))
					choice = 4;
				else
					choice = 3;
			} else {
				if (4 < _moreClicks)
					_moreClicks = 2;
				choice = _moreClicks + 4;
			}
			_moreClicks += 1;
		}
		suffix = Common::String::format(k32SignedIndexFormat, choice);
	} else
		suffix = Common::String::format(k4SignedIndexFormat, _pose);
	group(kRockCharIDKey + suffix, kRockSpchIDKey + suffix, number(kMaxRockSpchKey + suffix));
}

void RockHead::hoopClick() {
	if (_pose == 1) {
		_host.playSounds({static_cast<uint16>(number(kWAVECOMSOFTCLICKName))});
		Common::String suffix = Common::String::format(kSignedIndexFormat, _host._vm._rnd->getRandomNumberRng(1, number(kMaxHoopSpchsKey)));
		group(kHoopCharIDKey + suffix, kHoopSpchIDKey + suffix, number(kNumHoopSpchsKey + suffix));
	} else if (_host._world.get(kR1IG1HatHereKey) && !_host._practice) {
		_host.playSounds({static_cast<uint16>(number(kWAVECOMPICKUPName))});
		_host._world.set(kR1IG1PendingKey, 2);
		line(kGetHoopCharIDKey, kGetHoopSpchIDKey);
		_phase = kCollect;
	} else
		rockSpeech();
}

bool RockHead::hotspot(const char *prefix, const Common::String &suffix, Common::Point point) const {
	Common::String base(prefix);
	return Common::Rect(number(base + kHSX1Name + suffix), number(base + kHSY1Name + suffix),
						number(base + kHSX2Name + suffix), number(base + kHSY2Name + suffix))
		.contains(point);
}

bool RockHead::click(Common::Point point) {
	_idle = _host._time;
	if (busy())
		return true;
	if (_host._world.get(kR1IG1HoopHereKey) && (_hoop->covers(point) ||
											  (!_host._world.get(kR1IG1HatHereKey) && hotspot(kHoopKey, "", point)))) {
		hoopClick();
		return true;
	}
	for (auto &entry : _host._actors) {
		if (!entry._value->bounds().contains(point))
			continue;
		if (entry._key.equals(kRockActor))
			rockSpeech();
		else if (entry._key.equals(kPaigeName)) {
			int state = _state;
			int choice = _host._vm._rnd->getRandomNumberRng(1, number(Common::String::format(kMaxHelpSpchsSignedIndexKey, state)));
			if (state == 3 && _host._world.get(kR1IG1HatHereKey)) {
				state = 4;
				choice = 1;
			}
			Common::String suffix = Common::String::format(kSignedIndexSignedIndexFormat, state, choice);
			line(kHelpSpchCharIDsKey + suffix, kHelpSpchIDsKey + suffix);
		} else
			mainSpeech();
		return true;
	}
	if (_ladder->visible && (hotspot(kLadderKey, _pose == 1 ? k1Name : k2Name, point) || hotspot(kLadderKey, k3Name, point))) {
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
		Common::String key = Common::String::format(kItemUnsignedIndexFormat, item);
		if (world.get(key) <= 0)
			return false;
		world.set(key, world.get(key) - 1);
		world.set(kR1IG1PendingKey, item);
		_host.closeChest();
		_start = _host._time;
		_clock = true;
		_phase = item == 6 ? kLadderWait : kHatWait;
		return true;
	}
	if (_pose == 1 && item == 8)
		line(kRockCharID211Key, kRockSpchID211Key);
	else if (_pose == 2 && world.get(kTalkedToRockHeadKey)) {
		Common::String suffix = Common::String::format(kSignedIndexFormat, _host._vm._rnd->getRandomNumberRng(1, number(kMaxWrongColorSpchsKey)));
		line(kWrongColorCharIDKey + suffix, kWrongColorSpchIDKey + suffix);
	} else {
		Common::String suffix = Common::String::format(kSignedIndexFormat, _host._vm._rnd->getRandomNumberRng(1, number(kMaxItemSpchsKey)));
		line(kItemCharSpchKey + suffix, kItemSpchsIDKey + suffix);
	}
	return false;
}

} // namespace RR1Cloud9
} // namespace Trinket
