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
	if (choices.empty()) {
		for (uint i = 1; i <= count; i++)
			choices.push_back(i);
	}
	// Even the last remaining candidate consumes a native rand() draw.
	uint index = _vm._rnd->getRandomNumberExcluding(choices.size());
	uint choice = choices[index];
	choices.remove_at(index);
	return choice;
}

bool PageScene::enterHub() {
	_ambient = _scene.add(characterResource(_page->number(kAmbientIDKey), kAnimation), _page->number(kAmbientZKey));
	if (!_ambient)
		return false;
	_ambient->visible = false;
	_ambientSound = new OM::Sound(_vm.mixer());
	if (!_world.visits(_name)) {
		say(kSamName, _page->number(kEnterSpchIDs1Key));
		say(kSamName, _page->number(kEnterSpchIDs2Key));
		say(kReaderName, _page->number(kEnterSpchIDs3Key));
	} else if (!_world.visits(Page::kPageLN2)) {
		say(kSamName, _page->number(kReEnterSpchIDsA1Key));
		say(kReaderName, _page->number(kReEnterSpchIDsA2Key));
	} else if (_vm._rnd->getRandomNumberRng(1, 2) == 2) {
		switch (_vm._rnd->getRandomNumberRng(1, 4)) {
		case 1:
			say(kReaderName, _page->number(kReEnterSpchIDs11Key));
			say(kSamName, _page->number(kReEnterSpchIDs12Key));
			say(kReaderName, _page->number(kReEnterSpchIDs13Key));
			break;
		case 2:
			say(kReaderName, _page->number(kReEnterSpchIDs2Key));
			break;
		case 3:
			say(kSamName, _page->number(kReEnterSpchIDsA1Key));
			break;
		case 4:
			say(kReaderName, _page->number(kReEnterSpchIDs4Key));
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
	if (_ambientWaiting && static_cast<uint32>(_page->number(kDelayTimeKey)) <= time - _ambientStart) {
		uint choice = chooseAction(_ambientChoices);
		_ambientSound->play(_vm.resources(), characterResource(_page->number(Common::String::format(kAmbientIDsUnsignedIndexKey, choice)), kSound),
							Audio::Mixer::kSFXSoundType, 1, _page->number(kAmbientVolumeKey));
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
	Common::String prefix = Common::String::format(kIdleSpchIDsUnsignedIndexKey, choice);
	say(choice == 3 ? kReaderName : kSamName, _page->number(prefix + k1Name));
	say(choice == 3 ? kSamName : kReaderName, _page->number(prefix + k2Name));
	if (choice == 3)
		say(kSamName, _page->number(prefix + k3Name));
}

Common::String PageScene::hubClick(Common::Point point) {
	for (const auto &actor : _actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		if (actor._key.equals(kPaigeName)) {
			if (!_world.visits(Page::kPageLN2))
				say(kPaigeName, _page->number(kPaigeHelpID1Key));
			else {
				if (static_cast<uint>(_page->number(kMaxPaigeIndexKey)) < _paigeClicks)
					_paigeClicks = 0;
				say(kPaigeName, _page->number(kPaigeHelpID21Key) + _paigeClicks);
				_paigeClicks += 1;
			}
		} else {
			_mainClicks += 1;
			if (_mainClicks == 3) {
				say(kReaderName, _page->number(kReEnterSpchIDs11Key));
				say(kSamName, _page->number(kReEnterSpchIDs12Key));
				say(kReaderName, _page->number(kReEnterSpchIDs13Key));
				_mainClicks = 0;
			} else {
				Common::String prefix = Common::String::format(kMainSpchIDsUnsignedIndexKey, _mainClicks);
				say(_mainClicks == 1 ? kReaderName : kSamName, _page->number(prefix + k1Name));
				say(_mainClicks == 1 ? kSamName : kReaderName, _page->number(prefix + k2Name));
			}
		}
		_idleWaiting = false;
		return kHandledName;
	}
	return Common::String();
}

Common::String PageScene::groveClick(Common::Point point) {
	for (const auto &actor : _actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		if (actor._key.equals(kPaigeName)) {
			say(kPaigeName, _page->number(kPaigeHelpIDKey));
		} else {
			_mainClicks += 1;
			if (static_cast<uint>(_page->number(kMaxMainCharIndexKey)) < _mainClicks)
				_mainClicks = 1;
			Common::String prefix = Common::String::format(kMainSpchIDUnsignedIndexKey, _mainClicks);
			say(_page->text(kMainCharID1Key), _page->number(prefix + k1Name));
			say(_page->text(kMainCharID2Key), _page->number(prefix + k2Name));
		}
		return kHandledName;
	}
	return Common::String();
}

} // namespace RR1Cloud9
} // namespace Trinket
