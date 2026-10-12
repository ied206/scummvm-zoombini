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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

uint PageScene::chooseAction(Common::Array<uint> &choices, uint count) {
	if (choices.empty())
		for (uint i = 1; i <= count; i++)
			choices.push_back(i);
	// Even the last remaining candidate consumes a native rand() draw.
	uint index = _vm.rnd().excluding(choices.size());
	uint choice = choices[index];
	choices.remove_at(index);
	return choice;
}

bool PageScene::enterHub() {
	_ambient = _scene.add(characterResource(_page->number("kAmbientID"), kAnimation), _page->number("kAmbientZ"));
	if (!_ambient)
		return false;
	_ambient->visible = false;
	_ambientSound = new OM::Sound(_vm.mixer());
	if (!_world.visits(_name)) {
		say("sam", _page->number("kEnterSpchIDs.1"));
		say("sam", _page->number("kEnterSpchIDs.2"));
		say("reader", _page->number("kEnterSpchIDs.3"));
	} else if (!_world.visits("r1ln2")) {
		say("sam", _page->number("kReEnterSpchIDsA.1"));
		say("reader", _page->number("kReEnterSpchIDsA.2"));
	} else if (_vm.rnd().number(1, 2) == 2) {
		switch (_vm.rnd().number(1, 4)) {
		case 1:
			say("reader", _page->number("kReEnterSpchIDs.1.1"));
			say("sam", _page->number("kReEnterSpchIDs.1.2"));
			say("reader", _page->number("kReEnterSpchIDs.1.3"));
			break;
		case 2:
			say("reader", _page->number("kReEnterSpchIDs.2"));
			break;
		case 3:
			say("sam", _page->number("kReEnterSpchIDsA.1"));
			break;
		case 4:
			say("reader", _page->number("kReEnterSpchIDs.4"));
			break;
		}
	}
	return true;
}

void PageScene::updateHub(uint32 time) {
	if (!_hubStarted) {
		_hubStarted = true;
		_ambientWaiting = true;
		_ambientStart = time;
	}
	if (!_ambientWaiting && !_ambient->isRunning() && !_ambientSound->isPlaying()) {
		_ambientWaiting = true;
		_ambientStart = time;
		_ambient->visible = false;
	}
	if (_ambientWaiting && static_cast<uint32>(_page->number("kDelayTime")) <= time - _ambientStart) {
		uint choice = chooseAction(_ambientChoices);
		_ambientSound->play(_vm.resources(), characterResource(_page->number(Common::String::format("kAmbientIDs.%u", choice)), kSound),
							Audio::Mixer::kSFXSoundType, 1, _page->number("kAmbientVolume"));
		_ambient->visible = true;
		_ambient->startAnimation(1);
		_ambientWaiting = false;
	}
	if (_speaking || _nextSpeech < _speech.size() || _chestOpen)
		_idleWaiting = false;
}

void PageScene::hubIdle(uint32 time) {
	if (_chestOpen)
		return;
	// The authored idle queue starts after the speech queue finishes, not after mouse movement.
	if (!_idleWaiting) {
		_idleWaiting = true;
		_idleStart = time;
		return;
	}
	if (time - _idleStart < 15000)
		return;
	interrupt();
	_idleWaiting = false;
	uint choice = chooseAction(_idleChoices);
	Common::String prefix = Common::String::format("kIdleSpchIDs.%u", choice);
	say(choice == 3 ? "reader" : "sam", _page->number(prefix + ".1"));
	say(choice == 3 ? "sam" : "reader", _page->number(prefix + ".2"));
	if (choice == 3)
		say("sam", _page->number(prefix + ".3"));
}

Common::String PageScene::hubClick(Common::Point point) {
	for (const auto &actor : _actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		if (actor._key == "paige") {
			if (!_world.visits("r1ln2"))
				say("paige", _page->number("kPaigeHelpID.1"));
			else {
				if (static_cast<uint>(_page->number("kMaxPaigeIndex")) < _paigeClicks)
					_paigeClicks = 0;
				say("paige", _page->number("kPaigeHelpID.2.1") + _paigeClicks);
				_paigeClicks += 1;
			}
		} else {
			_mainClicks += 1;
			if (_mainClicks == 3) {
				say("reader", _page->number("kReEnterSpchIDs.1.1"));
				say("sam", _page->number("kReEnterSpchIDs.1.2"));
				say("reader", _page->number("kReEnterSpchIDs.1.3"));
				_mainClicks = 0;
			} else {
				Common::String prefix = Common::String::format("kMainSpchIDs.%u", _mainClicks);
				say(_mainClicks == 1 ? "reader" : "sam", _page->number(prefix + ".1"));
				say(_mainClicks == 1 ? "sam" : "reader", _page->number(prefix + ".2"));
			}
		}
		_idleWaiting = false;
		return "handled";
	}
	return Common::String();
}

Common::String PageScene::groveClick(Common::Point point) {
	for (const auto &actor : _actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		if (actor._key == "paige") {
			say("paige", _page->number("kPaigeHelpID"));
		} else {
			_mainClicks += 1;
			if (static_cast<uint>(_page->number("kMaxMainCharIndex")) < _mainClicks)
				_mainClicks = 1;
			Common::String prefix = Common::String::format("kMainSpchID.%u", _mainClicks);
			say(_page->text("kMainCharID.1"), _page->number(prefix + ".1"));
			say(_page->text("kMainCharID.2"), _page->number(prefix + ".2"));
		}
		return "handled";
	}
	return Common::String();
}

} // namespace RR1Cloud9
} // namespace Trinket
