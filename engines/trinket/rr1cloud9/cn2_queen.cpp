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

void PageScene::music(uint resource) {
	uint volume = _vm.music().volume();
	bool enabled = _vm.music().isEnabled();
	_vm.music().reset();
	_vm.music().setEnabled(enabled);
	if (resource)
		_vm.music().start(Common::String::format(kPageMusicFormat, _name.c_str(), resource), volume);
}

bool PageScene::enterQueen() {
	if (!actor(kQueenName, kQueenKey))
		return false;
	const int savedState = MAX(1, _world.get(kR1CN2StateKey));
	int state = savedState;
	if (state == 2 && _world.visits(Page::kPageIN4))
		state = 3;
	if (state == 3 && _world.get(kTalkedToInventorKey))
		state = 4;
	uint collected = 0;
	const char *const items[] = {
		kRBandsName,
		kHulaHoopName,
		kPillowName,
	};
	for (const char *item : items) {
		int id = _page->number(Common::String(kINV1Key) + item);
		if (_world.get(Common::String::format(kItemSignedIndexKey, id)) || _world.get(Common::String(kR1LN2Key) + item + kDeliveredKey))
			collected += 1;
	}
	if (4 <= state)
		state = MAX(state, 4 + static_cast<int>(collected));
	const int reentry = state == savedState ? _world.get(kR1CN2ReentryKey) + 1 : 0;
	_world.set(kR1CN2StateKey, state);
	_world.set(kR1CN2ReentryKey, reentry);
	// The portable save route index for the catapult return page, r1rss.
	const bool fromCatapult = _world.get(kPreviousLocationKey) == 20;
	if (fromCatapult)
		_world.set(kR1CN2CatapultReentryKey, _world.get(kR1CN2CatapultReentryKey) + 1);
	_world.set(kR1CN3FromStreetKey, 1);
	Shell::Sprite *litter = _scene.add(characterResource(_page->number(kLitterIDKey), kAnimation), _page->number(kLitterZKey));
	Shell::Sprite *sign = _scene.add(characterResource(_page->number(kSignIDKey), kAnimation), _page->number(kSignZKey));
	if (!litter || !sign)
		return false;
	litter->visible = 3 <= state;
	int frame;
	if (state < 5)
		frame = 1;
	else if (state < 7)
		frame = 2;
	else
		frame = 3;
	litter->setFrame(frame);
	_ambient = _scene.add(characterResource(_page->number(kAmbientIDs1Key), kAnimation), _page->number(kAmbientZKey));
	if (!_ambient)
		return false;
	if (fromCatapult) {
		_actors[kReaderName]->playAnim(_page->number(kReaderWalkInIDKey));
		_actors[kSamName]->playAnim(_page->number(kSamWalkInIDKey));
		_queenArriving = true;
	}
	int pending = _world.get(kR1CN2WelcomePendingKey);
	if (_world.get(kR1CN2ExitPendingKey)) {
		say(kReaderName, _page->number(kByeIDKey));
		_queenPhase = kQueenExitSpeech;
	} else if (pending == 3) {
		queenMovieFinished();
	} else if (pending == 2) {
		_queenPhase = kQueenMovie;
	} else if (pending || state == 1) {
		say(kSamName, _page->number(kEnterSpchIDsKey));
		_queenPhase = kQueenGreeting;
		_world.set(kR1CN2WelcomePendingKey, 1);
	} else {
		queenReentry(state, reentry, fromCatapult);
	}
	return true;
}

void PageScene::updateQueenAmbient(uint32 time) {
	if (!_hubStarted) {
		_hubStarted = true;
		_ambientWaiting = true;
		_ambientStart = time;
	}
	Shell::Sprite *playing = _queenAmbientChoice == 1 ? _ambient : _queenAmbient;
	if (!_ambientWaiting && playing && !playing->isRunning()) {
		if (_queenAmbient)
			_scene.remove(_queenAmbient);
		_queenAmbient = nullptr;
		_queenAmbientChoice = 0;
		_ambientWaiting = true;
		_ambientStart = time;
	}
	if (_ambientWaiting && static_cast<uint32>(_page->number(kDelayTimeKey)) <= time - _ambientStart) {
		_queenAmbientChoice = chooseAction(_ambientChoices);
		if (_queenAmbientChoice == 1) {
			_ambient->startAnimation(1);
		} else {
			_queenAmbient = _scene.add(characterResource(_page->number(Common::String::format(kAmbientIDsUnsignedIndexKey, _queenAmbientChoice)),
				kAnimation), _page->number(kAmbientZKey));
			if (_queenAmbient)
				_queenAmbient->startAnimation(1);
		}
		_ambientWaiting = false;
	}
}

void PageScene::queenGroup(uint group, const char *actors) {
	for (uint i = 0; actors[i]; i++) {
		const char *actor = kQueenName;
		if (actors[i] == 'r')
			actor = kReaderName;
		else if (actors[i] == 's')
			actor = kSamName;
		say(actor, _page->number(Common::String::format(kReEnterSpchIDsUnsignedIndexUnsignedIndexKey, group, i + 1)));
	}
}

void PageScene::queenReentry(int state, int reentry, bool fromCatapult) {
	if (fromCatapult) {
		int count = _world.get(kR1CN2CatapultReentryKey);
		if (4 < count)
			count = 1;
		if (count == 1)
			queenGroup(1, kRsrName);
		else
			say(count == 3 ? kReaderName : kSamName, _page->number(Common::String::format(kReEnterSpchIDsSignedIndexKey, count)));
		_world.set(kR1CN2CatapultReentryKey, count == 4 ? 1 : count);
	}
	// Catapult dialogue precedes, rather than replaces, the progress report.
	if (state == 2 && reentry == 1) {
		queenGroup(5, kQrqName);
	} else if (state == 3 && reentry == 0) {
		queenGroup(6, kQrqName);
	} else if (state == 4) {
		if (reentry == 0) {
			queenGroup(7, kQsrsrsrsrsqName);
		} else if (reentry == 1) {
			queenGroup(8, kQrName);
		} else if (reentry == 2) {
			queenGroup(9, kQrName);
			_world.set(kR1CN2ReentryKey, 0);
		}
	} else if (state == 5 && reentry == 0) {
		queenGroup(10, kQrsName);
	} else if (state == 6) {
		uint missing = 0;
		if (!hasItem(kHulaHoopName, kR1LN2HulaHoopDeliveredKey))
			missing = 1;
		else if (!hasItem(kRBandsName, kR1LN2RBandsDeliveredKey))
			missing = 2;
		else if (!hasItem(kPillowName, kR1LN2PillowDeliveredKey))
			missing = 3;
		if (reentry == 0) {
			say(kQueenName, _page->number(kReEnterSpchIDs111Key));
			if (missing)
				say(missing == 2 ? kSamName : kReaderName, _page->number(Common::String::format(kReEnterSpchIDs11UnsignedIndexKey, missing + 1)));
			say(kQueenName, _page->number(kReEnterSpchIDs115Key));
		} else if (missing) {
			say(kQueenName, _page->number(Common::String::format(kReEnterSpchIDs12UnsignedIndexKey, missing)));
			say(kReaderName, _page->number(kReEnterSpchIDs124Key));
		}
	} else if (state == 7) {
		queenGroup(reentry == 0 ? 13 : 14, reentry == 0 ? kQqsrqName : kQqrName);
	}
}

void PageScene::queenWelcome() {
	_world.set(kR1CN2StateKey, 2);
	_world.set(kR1CN2ReentryKey, 0);
	_speech.clear();
	_nextSpeech = 0;
	speeches(kWelcomeCharID1Key, kWelcomeSpchID1Key, _page->number(kNumWelcomeSpch1Key));
	speeches(kWelcomeCharID2Key, kWelcomeSpchID2Key, _page->number(kNumWelcomeSpch2Key));
	_queenSongIndex = _speech.empty() ? 0 : _speech.size() - 1;
	_queenPhase = kQueenWelcome;
}

void PageScene::updateQueen() {
	if (_queenPhase == kQueenWelcome && !_queenSongStarted && _queenSongIndex <= _nextSpeech) {
		music(_page->number(kSongIntroIDKey));
		_queenSongStarted = true;
	}
	if (_nextSpeech < _speech.size())
		return;
	if (_queenPhase == kQueenGreeting) {
		queenWelcome();
	} else if (_queenPhase == kQueenWelcome) {
		_queenPhase = kQueenMovie;
		_world.set(kR1CN2WelcomePendingKey, 2);
	} else if (_queenPhase == kQueenAfterMovie) {
		_queenPhase = kQueenIdle;
		_world.set(kR1CN2WelcomePendingKey, 0);
	} else if (_queenPhase == kQueenExitSpeech) {
		_queenPhase = kQueenExitReady;
	}
}

void PageScene::queenIdle(uint32 time) {
	if (_practice || _queenPhase != kQueenIdle) {
		_idleWaiting = false;
		return;
	}
	if (!_idleWaiting) {
		_idleWaiting = true;
		_idleStart = time;
		return;
	}
	if (time - _idleStart < static_cast<uint32>(_page->number(kGIdleHelpTimeName)))
		return;
	interrupt();
	uint choice = _vm.rnd().number(1, _world.get(kR1CN2StateKey) == 1 ? 2 : 3);
	uint group = choice == 1 ? 1 : 5 - choice;
	Common::String prefix = Common::String::format(kIdleSpchIDsUnsignedIndexKey, group);
	say(choice == 1 ? kReaderName : kSamName, _page->number(prefix + k1Name));
	say(choice == 1 ? kSamName : kReaderName, _page->number(prefix + k2Name));
}

void PageScene::queenMovieFinished() {
	interrupt();
	music(_page->number(kBackgroundMusicIDs2Key));
	speeches(kWelcomeCharID4Key, kWelcomeSpchID4Key, _page->number(kNumWelcomeSpch4Key));
	speeches(kWelcomeCharID5Key, kWelcomeSpchID5Key, _page->number(kNumWelcomeSpch5Key));
	_queenPhase = kQueenAfterMovie;
	_world.set(kR1CN2WelcomePendingKey, 3);
}

Common::String PageScene::queenClick(Common::Point point) {
	if (!_world.get(kR1CN2SignClickCountKey)) {
		int direction = _page->number(kNavLocCN3SignKey);
		int count = _page->number(Common::String::format(kNumNavHotspotsSignedIndexKey, direction));
		for (int i = 1; i <= count; i++) {
			Common::String suffix = Common::String::format(kSignedIndexSignedIndexFormat, direction, i);
			Common::Rect rect(_page->number(kNavHSX1Key + suffix), _page->number(kNavHSY1Key + suffix),
							  _page->number(kNavHSX2Key + suffix), _page->number(kNavHSY2Key + suffix));
			if (!rect.contains(point))
				continue;
			_world.set(kR1CN2SignClickCountKey, 1);
			_world.set(kR1CN2ExitPendingKey, 1);
			say(kReaderName, _page->number(kByeIDKey));
			_queenPhase = kQueenExitSpeech;
			return kHandledName;
		}
	}
	int state = _world.get(kR1CN2StateKey);
	if (_actors[kQueenName]->bounds().contains(point)) {
		if (state == 1) {
			queenWelcome();
		} else if (state == 2 && !_queenClicks) {
			const char *const actors[] = {
				kQueenName,
				kReaderName,
				kQueenName,
				kSamName,
			};
			for (uint i = 0; i < ARRAYSIZE(actors); i++)
				say(actors[i], _page->number(Common::String::format(kInstrSpchIDs2UnsignedIndexKey, i + 1)));
		} else if (state == 3 && !_queenClicks) {
			say(kQueenName, _page->number(kInstrSpchIDs3Key));
		} else if (!_queenClicks) {
			uint group = 0;
			if (state == 7)
				group = 9;
			else if (!hasItem(kHulaHoopName, kR1LN2HulaHoopDeliveredKey))
				group = 4;
			else if (_world.visits(Page::kPageCG1) && !hasItem(kWhistleKey))
				group = 6;
			else if (!hasItem(kRBandsName, kR1LN2RBandsDeliveredKey))
				group = 7;
			else if (!hasItem(kPillowName, kR1LN2PillowDeliveredKey))
				group = 8;
			if (group) {
				const char *first;
				if (group == 9)
					first = kQueenName;
				else if (group == 4 || group == 8)
					first = kSamName;
				else
					first = kReaderName;
				say(first, _page->number(Common::String::format(kInstrSpchIDsUnsignedIndex1Key, group)));
				say(group == 9 ? kReaderName : kQueenName, _page->number(Common::String::format(kInstrSpchIDsUnsignedIndex2Key, group)));
			}
			if (group == 4) {
				say(kQueenName, _page->number(kInstrSpchIDs43Key));
				if (_world.get(kTalkedToRockHeadKey) && !hasItem(kLadderKey, kR1IG1LadderHereKey)) {
					say(kReaderName, _page->number(kInstrSpchIDs51Key));
					say(kQueenName, _page->number(kInstrSpchIDs52Key));
				}
			} else if (group == 7 && _world.visits(Page::kPageCN4) && !hasItem(kWagonKey, kR1CN4WagonHereKey)) {
				say(kSamName, _page->number(kInstrSpchIDs73Key));
				say(kQueenName, _page->number(kInstrSpchIDs74Key));
			}
		} else if (_queenClicks) {
			uint group = (_queenClicks - 1) % 3 + 10;
			say(group == 11 ? kReaderName : kSamName, _page->number(Common::String::format(kInstrSpchIDsUnsignedIndex1Key, group)));
			say(group == 12 ? kQueenName : kReaderName, _page->number(Common::String::format(kInstrSpchIDsUnsignedIndex2Key, group)));
			if (group != 12)
				say(kQueenName, _page->number(Common::String::format(kInstrSpchIDsUnsignedIndex3Key, group)));
		}
		_queenClicks += 1;
		return kHandledName;
	}
	if (_actors[kPaigeName]->bounds().contains(point)) {
		uint group;
		if (state == 2)
			group = 1;
		else if (state == 7)
			group = 5;
		else
			group = _paigeClicks % 3 + 2;
		say(kPaigeName, _page->number(Common::String::format(kHelpSpchIDsUnsignedIndexKey, group)));
		_paigeClicks += 1;
		return kHandledName;
	}
	if (_actors[kReaderName]->bounds().contains(point) || _actors[kSamName]->bounds().contains(point)) {
		if (!_mainClicks && (state == 2 || state == 3 || state == 7)) {
			uint group = state == 7 ? 10 : state - 1;
			say(state == 3 ? kSamName : kReaderName, _page->number(Common::String::format(kMainSpchIDsUnsignedIndexKey, group)));
		} else if (state != 7) {
			uint group;
			if (_mainClicks) {
				int last = 9;
				if (state < 4)
					last = 7;
				group = _vm.rnd().number(6, last);
			} else {
				group = state - 1;
			}
			bool readerFirst = group == 3 || group == 6 || group == 8;
			say(readerFirst ? kReaderName : kSamName, _page->number(Common::String::format(kMainSpchIDsUnsignedIndex1Key, group)));
			say(readerFirst ? kSamName : kReaderName, _page->number(Common::String::format(kMainSpchIDsUnsignedIndex2Key, group)));
		}
		_mainClicks += 1;
		return kHandledName;
	}
	return Common::String();
}

bool PageScene::hasItem(const Common::String &name, const Common::String &delivered) const {
	uint id = _page->number(kINV1Key + name);
	return _world.get(Common::String::format(kItemUnsignedIndexKey, id)) > 0 || (!delivered.empty() && _world.get(delivered) > 0);
}

} // namespace RR1Cloud9
} // namespace Trinket
