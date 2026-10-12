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
		_vm.music().start(Common::String::format("%stunz/%u.wav", _name.c_str(), resource), volume);
}

bool PageScene::enterQueen() {
	if (!actor("queen", "kQueen"))
		return false;
	const int savedState = MAX(1, _world.get("R1CN2State"));
	int state = savedState;
	if (state == 2 && _world.visits("r1in4"))
		state = 3;
	if (state == 3 && _world.get("talkedToInventor"))
		state = 4;
	uint collected = 0;
	const char *const items[] = {
		"RBands",
		"HulaHoop",
		"Pillow",
	};
	for (const char *item : items) {
		int id = _page->number(Common::String("kINV1") + item);
		if (_world.get(Common::String::format("item:%d", id)) || _world.get(Common::String("R1LN2") + item + "Delivered"))
			collected += 1;
	}
	if (4 <= state)
		state = MAX(state, 4 + static_cast<int>(collected));
	const int reentry = state == savedState ? _world.get("R1CN2Reentry") + 1 : 0;
	_world.set("R1CN2State", state);
	_world.set("R1CN2Reentry", reentry);
	// The portable save route index for the catapult return page, r1rss.
	const bool fromCatapult = _world.get("previousLocation") == 20;
	if (fromCatapult)
		_world.set("R1CN2CatapultReentry", _world.get("R1CN2CatapultReentry") + 1);
	_world.set("R1CN3FromStreet", 1);
	Shell::Sprite *litter = _scene.add(characterResource(_page->number("kLitterID"), kAnimation), _page->number("kLitterZ"));
	Shell::Sprite *sign = _scene.add(characterResource(_page->number("kSignID"), kAnimation), _page->number("kSignZ"));
	if (!litter || !sign)
		return false;
	litter->visible = 3 <= state;
	litter->setFrame(state < 5 ? 1 : state < 7 ? 2
											   : 3);
	_ambient = _scene.add(characterResource(_page->number("kAmbientIDs.1"), kAnimation), _page->number("kAmbientZ"));
	if (!_ambient)
		return false;
	if (fromCatapult) {
		_actors["reader"]->playAnim(_page->number("kReaderWalkInID"));
		_actors["sam"]->playAnim(_page->number("kSamWalkInID"));
		_queenArriving = true;
	}
	int pending = _world.get("R1CN2WelcomePending");
	if (_world.get("R1CN2ExitPending")) {
		say("reader", _page->number("kByeID"));
		_queenPhase = kQueenExitSpeech;
	} else if (pending == 3) {
		queenMovieFinished();
	} else if (pending == 2) {
		_queenPhase = kQueenMovie;
	} else if (pending || state == 1) {
		say("sam", _page->number("kEnterSpchIDs"));
		_queenPhase = kQueenGreeting;
		_world.set("R1CN2WelcomePending", 1);
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
	if (_ambientWaiting && static_cast<uint32>(_page->number("kDelayTime")) <= time - _ambientStart) {
		_queenAmbientChoice = chooseAction(_ambientChoices);
		if (_queenAmbientChoice == 1) {
			_ambient->startAnimation(1);
		} else {
			_queenAmbient = _scene.add(characterResource(_page->number(Common::String::format("kAmbientIDs.%u", _queenAmbientChoice)), kAnimation), _page->number("kAmbientZ"));
			if (_queenAmbient)
				_queenAmbient->startAnimation(1);
		}
		_ambientWaiting = false;
	}
}

void PageScene::queenGroup(uint group, const char *actors) {
	for (uint i = 0; actors[i]; i++) {
		const char *actor = "queen";
		if (actors[i] == 'r')
			actor = "reader";
		else if (actors[i] == 's')
			actor = "sam";
		say(actor, _page->number(Common::String::format("kReEnterSpchIDs.%u.%u", group, i + 1)));
	}
}

void PageScene::queenReentry(int state, int reentry, bool fromCatapult) {
	if (fromCatapult) {
		int count = _world.get("R1CN2CatapultReentry");
		if (4 < count)
			count = 1;
		if (count == 1)
			queenGroup(1, "rsr");
		else
			say(count == 3 ? "reader" : "sam", _page->number(Common::String::format("kReEnterSpchIDs.%d", count)));
		_world.set("R1CN2CatapultReentry", count == 4 ? 1 : count);
	}
	// Catapult dialogue precedes, rather than replaces, the progress report.
	if (state == 2 && reentry == 1) {
		queenGroup(5, "qrq");
	} else if (state == 3 && reentry == 0) {
		queenGroup(6, "qrq");
	} else if (state == 4) {
		if (reentry == 0) {
			queenGroup(7, "qsrsrsrsrsq");
		} else if (reentry == 1) {
			queenGroup(8, "qr");
		} else if (reentry == 2) {
			queenGroup(9, "qr");
			_world.set("R1CN2Reentry", 0);
		}
	} else if (state == 5 && reentry == 0) {
		queenGroup(10, "qrs");
	} else if (state == 6) {
		uint missing = 0;
		if (!hasItem("HulaHoop", "R1LN2HulaHoopDelivered"))
			missing = 1;
		else if (!hasItem("RBands", "R1LN2RBandsDelivered"))
			missing = 2;
		else if (!hasItem("Pillow", "R1LN2PillowDelivered"))
			missing = 3;
		if (reentry == 0) {
			say("queen", _page->number("kReEnterSpchIDs.11.1"));
			if (missing)
				say(missing == 2 ? "sam" : "reader", _page->number(Common::String::format("kReEnterSpchIDs.11.%u", missing + 1)));
			say("queen", _page->number("kReEnterSpchIDs.11.5"));
		} else if (missing) {
			say("queen", _page->number(Common::String::format("kReEnterSpchIDs.12.%u", missing)));
			say("reader", _page->number("kReEnterSpchIDs.12.4"));
		}
	} else if (state == 7) {
		queenGroup(reentry == 0 ? 13 : 14, reentry == 0 ? "qqsrq" : "qqr");
	}
}

void PageScene::queenWelcome() {
	_world.set("R1CN2State", 2);
	_world.set("R1CN2Reentry", 0);
	_speech.clear();
	_nextSpeech = 0;
	speeches("kWelcomeCharID.1", "kWelcomeSpchID.1", _page->number("kNumWelcomeSpch.1"));
	speeches("kWelcomeCharID.2", "kWelcomeSpchID.2", _page->number("kNumWelcomeSpch.2"));
	_queenSongIndex = _speech.empty() ? 0 : _speech.size() - 1;
	_queenPhase = kQueenWelcome;
}

void PageScene::updateQueen() {
	if (_queenPhase == kQueenWelcome && !_queenSongStarted && _queenSongIndex <= _nextSpeech) {
		music(_page->number("kSongIntroID"));
		_queenSongStarted = true;
	}
	if (_nextSpeech < _speech.size())
		return;
	if (_queenPhase == kQueenGreeting) {
		queenWelcome();
	} else if (_queenPhase == kQueenWelcome) {
		_queenPhase = kQueenMovie;
		_world.set("R1CN2WelcomePending", 2);
	} else if (_queenPhase == kQueenAfterMovie) {
		_queenPhase = kQueenIdle;
		_world.set("R1CN2WelcomePending", 0);
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
	if (time - _idleStart < static_cast<uint32>(_page->number("gIdleHelpTime")))
		return;
	interrupt();
	uint choice = _vm.rnd().number(1, _world.get("R1CN2State") == 1 ? 2 : 3);
	uint group = choice == 1 ? 1 : 5 - choice;
	Common::String prefix = Common::String::format("kIdleSpchIDs.%u", group);
	say(choice == 1 ? "reader" : "sam", _page->number(prefix + ".1"));
	say(choice == 1 ? "sam" : "reader", _page->number(prefix + ".2"));
}

void PageScene::queenMovieFinished() {
	interrupt();
	music(_page->number("kBackgroundMusicIDs.2"));
	speeches("kWelcomeCharID.4", "kWelcomeSpchID.4", _page->number("kNumWelcomeSpch.4"));
	speeches("kWelcomeCharID.5", "kWelcomeSpchID.5", _page->number("kNumWelcomeSpch.5"));
	_queenPhase = kQueenAfterMovie;
	_world.set("R1CN2WelcomePending", 3);
}

Common::String PageScene::queenClick(Common::Point point) {
	if (!_world.get("R1CN2SignClickCount")) {
		int direction = _page->number("kNavLocCN3Sign");
		int count = _page->number(Common::String::format("kNumNavHotspots.%d", direction));
		for (int i = 1; i <= count; i++) {
			Common::String suffix = Common::String::format(".%d.%d", direction, i);
			Common::Rect rect(_page->number("kNavHSX1" + suffix), _page->number("kNavHSY1" + suffix),
							  _page->number("kNavHSX2" + suffix), _page->number("kNavHSY2" + suffix));
			if (!rect.contains(point))
				continue;
			_world.set("R1CN2SignClickCount", 1);
			_world.set("R1CN2ExitPending", 1);
			say("reader", _page->number("kByeID"));
			_queenPhase = kQueenExitSpeech;
			return "handled";
		}
	}
	int state = _world.get("R1CN2State");
	if (_actors["queen"]->bounds().contains(point)) {
		if (state == 1) {
			queenWelcome();
		} else if (state == 2 && !_queenClicks) {
			const char *const actors[] = {
				"queen",
				"reader",
				"queen",
				"sam",
			};
			for (uint i = 0; i < ARRAYSIZE(actors); i++)
				say(actors[i], _page->number(Common::String::format("kInstrSpchIDs.2.%u", i + 1)));
		} else if (state == 3 && !_queenClicks) {
			say("queen", _page->number("kInstrSpchIDs.3"));
		} else if (!_queenClicks) {
			uint group = 0;
			if (state == 7)
				group = 9;
			else if (!hasItem("HulaHoop", "R1LN2HulaHoopDelivered"))
				group = 4;
			else if (_world.visits("r1cg1") && !hasItem("Whistle"))
				group = 6;
			else if (!hasItem("RBands", "R1LN2RBandsDelivered"))
				group = 7;
			else if (!hasItem("Pillow", "R1LN2PillowDelivered"))
				group = 8;
			if (group) {
				const char *first = group == 9 ? "queen" : group == 4 || group == 8 ? "sam"
																					: "reader";
				say(first, _page->number(Common::String::format("kInstrSpchIDs.%u.1", group)));
				say(group == 9 ? "reader" : "queen", _page->number(Common::String::format("kInstrSpchIDs.%u.2", group)));
			}
			if (group == 4) {
				say("queen", _page->number("kInstrSpchIDs.4.3"));
				if (_world.get("talkedToRockHead") && !hasItem("Ladder", "R1IG1LadderHere")) {
					say("reader", _page->number("kInstrSpchIDs.5.1"));
					say("queen", _page->number("kInstrSpchIDs.5.2"));
				}
			} else if (group == 7 && _world.visits("r1cn4") && !hasItem("Wagon", "R1CN4WagonHere")) {
				say("sam", _page->number("kInstrSpchIDs.7.3"));
				say("queen", _page->number("kInstrSpchIDs.7.4"));
			}
		} else if (_queenClicks) {
			uint group = (_queenClicks - 1) % 3 + 10;
			say(group == 11 ? "reader" : "sam", _page->number(Common::String::format("kInstrSpchIDs.%u.1", group)));
			say(group == 12 ? "queen" : "reader", _page->number(Common::String::format("kInstrSpchIDs.%u.2", group)));
			if (group != 12)
				say("queen", _page->number(Common::String::format("kInstrSpchIDs.%u.3", group)));
		}
		_queenClicks += 1;
		return "handled";
	}
	if (_actors["paige"]->bounds().contains(point)) {
		uint group = state == 2 ? 1 : state == 7 ? 5
												 : _paigeClicks % 3 + 2;
		say("paige", _page->number(Common::String::format("kHelpSpchIDs.%u", group)));
		_paigeClicks += 1;
		return "handled";
	}
	if (_actors["reader"]->bounds().contains(point) || _actors["sam"]->bounds().contains(point)) {
		if (!_mainClicks && (state == 2 || state == 3 || state == 7)) {
			uint group = state == 7 ? 10 : state - 1;
			say(state == 3 ? "sam" : "reader", _page->number(Common::String::format("kMainSpchIDs.%u", group)));
		} else if (state != 7) {
			uint group = _mainClicks ? _vm.rnd().number(6, state < 4 ? 7 : 9) : state - 1;
			bool readerFirst = group == 3 || group == 6 || group == 8;
			say(readerFirst ? "reader" : "sam", _page->number(Common::String::format("kMainSpchIDs.%u.1", group)));
			say(readerFirst ? "sam" : "reader", _page->number(Common::String::format("kMainSpchIDs.%u.2", group)));
		}
		_mainClicks += 1;
		return "handled";
	}
	return Common::String();
}

bool PageScene::hasItem(const Common::String &name, const Common::String &delivered) const {
	uint id = _page->number("kINV1" + name);
	return _world.get(Common::String::format("item:%u", id)) > 0 || (!delivered.empty() && _world.get(delivered) > 0);
}

} // namespace RR1Cloud9
} // namespace Trinket
