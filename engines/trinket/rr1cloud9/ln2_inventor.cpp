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

const char *PageScene::partName(uint item) {
	static const char *const names[] = {
		"",
		"RBands",
		"HulaHoop",
		"Pillow",
	};
	return item <= 3 ? names[item] : "";
}

void PageScene::inventorGroup(const char *prefix, const char *actors) {
	for (uint i = 0; actors[i]; i++) {
		const char *actor = "irvin";
		if (actors[i] == 'r')
			actor = "reader";
		else if (actors[i] == 's')
			actor = "sam";
		say(actor, _page->number(Common::String::format("%s.%u", prefix, i + 1)));
	}
}

bool PageScene::enterInventor() {
	if (!actor("irvin", "kIrvin"))
		return false;
	for (uint item = 1; item <= 3; item++) {
		Common::String prefix = item == 1 ? "kRubberBands" : Common::String("k") + partName(item);
		_parts[item - 1] = _scene.add(characterResource(_page->number(prefix + "ID"), kAnimation), _page->number(prefix + "Z"));
		if (!_parts[item - 1])
			return false;
		_parts[item - 1]->visible = _world.get(Common::String("R1LN2") + partName(item) + "Delivered");
	}
	_ambient = _scene.add(characterResource(_page->number("kAmbientID"), kAnimation), _page->number("kAmbientZ"));
	if (!_ambient)
		return false;
	int savedState = MAX(1, _world.get("R1LN2State")), state = savedState;
	uint collected = 0;
	for (uint item = 1; item <= 3; item++)
		if (_world.get(Common::String::format("item:%u", item)))
			collected += 1;
	if (2 <= state)
		state = MAX(state, 2 + static_cast<int>(collected));
	int reentry = state == savedState ? _world.get("R1LN2Reentry") + 1 : 0;
	_world.set("R1LN2State", state);
	_world.set("R1LN2Reentry", reentry);
	bool fromCatapult = _world.get("previousLocation") == 10;
	if (fromCatapult)
		_world.set("R1LN2CatapultReentry", _world.get("R1LN2CatapultReentry") + 1);
	int pending = _world.get("R1LN2IntroPending");
	if (_world.get("R1LN2MoviePending")) {
		_inventorPhase = kInventorMovie;
	} else if (_world.get("R1LN2DeliveryPending")) {
		_inventorPhase = kInventorDeliveryWait;
	} else if (pending == 2) {
		_inventorPhase = kInventorIntro;
	} else if (pending || state == 1) {
		_world.set("R1LN2State", 1);
		inventorSpeech();
	} else if (fromCatapult && _world.get("R1LN2CatapultReentry") == 1) {
		inventorGroup("kReEnterSpchIDs.10", "iri");
	} else if (state == 2 && reentry == 1) {
		inventorGroup("kReEnterSpchIDs.1", "rri");
	} else if (state == 3 && reentry == 0) {
		inventorGroup("kReEnterSpchIDs.2", "si");
	} else if (state == 4 && reentry == 0) {
		inventorGroup("kReEnterSpchIDs.3", "siri");
	} else if (state == 5) {
		inventorGroup(reentry == 0 ? "kReEnterSpchIDs.7" : "kReEnterSpchIDs.8", reentry == 0 ? "sii" : "ir");
	} else {
		uint choice = _vm.rnd().number(1, 6);
		if (choice <= 3)
			inventorGroup(Common::String::format("kReEnterSpchIDs.%u", choice + 3).c_str(), choice == 1 ? "iir" : "ri");
	}
	return true;
}

void PageScene::inventorSpeech() {
	int state = _world.get("R1LN2State");
	if (state == 1) {
		_world.set("R1LN2State", 2);
		_world.set("R1LN2Reentry", 0);
		_world.set("talkedToInventor", 1);
		_world.set("R1LN2IntroPending", 1);
		_inventorPhase = kInventorIntro;
		_irvinClicks = 0;
		inventorGroup("kInitialInstrSpchIDs.1", "iirsiriisirirsr");
		inventorGroup("kInitialInstrSpchIDs.2", "iiisi");
		// The two companions answer in parallel before Irvin's last line.
		say("reader", _page->number("kInitialInstrSpchIDs.2.7"));
		if (!_speech.empty())
			_speech.back().together = true;
		say("sam", _page->number("kInitialInstrSpchIDs.2.6"));
		say("irvin", _page->number("kInitialInstrSpchIDs.2.8"));
	} else if (state == 5) {
		say("irvin", _page->number("kInstrSpchIDs.5"));
	} else {
		_irvinClicks = _irvinClicks % 3 + 1;
		const char *actors = "ii";
		if (_irvinClicks == 1)
			actors = "ris";
		else if (_irvinClicks == 2)
			actors = "ri";
		inventorGroup(Common::String::format("kInstrSpchIDs.%u", _irvinClicks + 1).c_str(), actors);
	}
}

void PageScene::inventorHint(uint item) {
	if (_highlight)
		_scene.remove(_highlight);
	Common::String prefix = Common::String::format("collectibles.%u.", item + 14);
	_highlight = _scene.add(Common::String::format("r1inventory/%d.ao", _page->number(prefix + "1")), 203);
	if (_highlight) {
		_highlight->setFrame(2);
		_highlightDue = _time + 100;
		_highlight->x = _page->number(prefix + "2");
		_highlight->y = _page->number(prefix + "3") - _page->number("kChesterYOffset");
	}
	static constexpr uint kHintIndices[] = {
		0,
		2,
		1,
		3,
	};
	say("irvin", _page->number(Common::String::format("kInitialInstrSpchIDs.3.%u", kHintIndices[item])));
}

void PageScene::updateInventorAmbient(uint32 time) {
	if (!_hubStarted) {
		_hubStarted = true;
		_ambientWaiting = true;
		_ambientStart = time;
	}
	if (_ambientWaiting) {
		if (1000 <= time - _ambientStart) {
			_ambient->startAnimation(1);
			_gearPlays = 1;
			_ambientWaiting = false;
		}
	} else if (!_ambient->isRunning()) {
		if (_gearPlays < 3) {
			_gearPlays += 1;
			_ambient->startAnimation(1);
		} else {
			_ambientWaiting = true;
			_ambientStart = time;
		}
	}
	if (_speaking || _speakingAlso || _nextSpeech < _speech.size() || _chestOpen || _inventorPhase != kInventorIdle)
		_idleWaiting = false;
}

void PageScene::updateInventor(uint32 time) {
	if (_nextSpeech < _speech.size())
		return;
	if (_inventorPhase == kInventorChestWait || _inventorPhase == kInventorCloseWait || _inventorPhase == kInventorDeliveryWait) {
		if (!_inventorTimerStarted) {
			_inventorTimerStarted = true;
			_inventorStart = time;
		}
		if (time - _inventorStart < (_inventorPhase == kInventorCloseWait ? 500U : 1500U))
			return;
		_inventorTimerStarted = false;
	}
	switch (_inventorPhase) {
	case kInventorIntro:
		_world.set("R1LN2IntroPending", 2);
		_world.set("ShowInventoryOutlines", 1);
		openChest();
		_inventorPhase = kInventorChestWait;
		break;
	case kInventorChestWait:
		inventorHint(2);
		_inventorPhase = kInventorHoop;
		break;
	case kInventorHoop:
		inventorHint(1);
		_inventorPhase = kInventorBands;
		break;
	case kInventorBands:
		inventorHint(3);
		_inventorPhase = kInventorPillow;
		break;
	case kInventorPillow:
		if (_highlight)
			_scene.remove(_highlight);
		_highlight = nullptr;
		closeChest();
		_inventorPhase = kInventorCloseWait;
		break;
	case kInventorCloseWait:
		say("sam", _page->number("kInitialInstrSpchIDs.3.6"));
		_inventorPhase = kInventorOutro;
		break;
	case kInventorOutro:
		_world.set("R1LN2IntroPending", 0);
		_inventorPhase = kInventorIdle;
		break;
	case kInventorDeliveryWait: {
		uint item = _world.get("R1LN2DeliveryPending");
		if (item < 1 || 3 < item) {
			_inventorPhase = kInventorIdle;
			break;
		}
		_world.set(Common::String("R1LN2") + partName(item) + "Delivered", 1);
		_parts[item - 1]->visible = true;
		static const char *const kPrefixes[] = {
			"",
			"Bands",
			"Hoop",
			"Pillow",
		};
		const char *prefix = kPrefixes[item];
		speeches(Common::String("k") + prefix + "CharID", Common::String("k") + prefix + "SpchID", _page->number(Common::String("kNum") + prefix + "Spch"));
		_inventorPhase = kInventorDeliverySpeech;
		break;
	}
	case kInventorDeliverySpeech:
		inventorMoreItems();
		_inventorPhase = kInventorMoreItems;
		break;
	case kInventorMoreItems: {
		_world.set("R1LN2DeliveryPending", 0);
		bool complete = true;
		for (uint item = 1; item <= 3; item++)
			complete &= _parts[item - 1]->visible;
		_inventorPhase = complete ? kInventorMovie : kInventorIdle;
		if (complete) {
			for (uint item = 1; item <= 3; item++) {
				_world.set(Common::String::format("item:%u", item), 1);
				_world.set(Common::String("R1LN2") + partName(item) + "Delivered", 0);
			}
			_world.set("R1LN2MoviePending", 1);
		}
		break;
	}
	default:
		break;
	}
}

void PageScene::inventorMoreItems() {
	uint delivered = 0;
	for (auto *part : _parts)
		if (part->visible)
			delivered += 1;
	if (delivered == 3) {
		speeches("kItemsCharIDs.2", "kItemsSpchIDs.2", _page->number("kNumItemSpch.2"));
		return;
	}
	say(_page->text("kItemsCharIDs.1.1"), _page->number("kItemsSpchIDs.1.1"));
	uint words = 0;
	for (uint item : {2U, 1U, 3U}) {
		if (_parts[item - 1]->visible)
			continue;
		if (words == 1 && delivered == 1) {
			say(_page->text("kItemsCharIDs.1.5"), _page->number("kItemsSpchIDs.1.5"));
			words += 1;
		}
		uint index = 4;
		if (item == 2)
			index = 2;
		else if (item == 1)
			index = 3;
		Common::String suffix = Common::String::format(".1.%u", index);
		say(_page->text("kItemsCharIDs" + suffix), _page->number("kItemsSpchIDs" + suffix));
		words += 1;
	}
}

void PageScene::inventorDrop(uint item) {
	interrupt();
	Common::String key = Common::String::format("item:%u", item);
	if (item < 1 || 3 < item || !_world.get(key) || _world.get(Common::String("R1LN2") + partName(item) + "Delivered")) {
		say(_page->text("kNoUseCharID"), _page->number("kNoUseSpchID"));
		return;
	}
	_world.set(key, _world.get(key) - 1);
	_world.set("R1LN2DeliveryPending", item);
	closeChest();
	_inventorPhase = kInventorDeliveryWait;
	_inventorTimerStarted = false;
}

void PageScene::inventorIdle(uint32 time) {
	if (_practice || _chestOpen || _inventorPhase != kInventorIdle)
		return;
	if (!_idleWaiting) {
		_idleStart = time;
		_idleWaiting = true;
		return;
	}
	if (time - _idleStart < static_cast<uint>(_page->number("gIdleHelpTime")))
		return;
	interrupt();
	_idleWaiting = false;
	if (_world.get("R1LN2State") == 5)
		say("reader", _page->number("kIdleSpchIDs.3"));
	else if (_vm.rnd().number(1, 2) == 1)
		inventorGroup("kIdleSpchIDs.1", "rs");
	else
		inventorGroup("kIdleSpchIDs.2", "srs");
}

Common::String PageScene::inventorClick(Common::Point point) {
	for (const auto &actor : _actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		if (actor._key == "irvin")
			inventorSpeech();
		else if (actor._key == "paige")
			say("paige", _page->number(_world.get("R1LN2State") == 5 ? "kHelpSpchIDs.2" : "kHelpSpchIDs.1"));
		else {
			_mainClicks = _mainClicks % 3 + 1;
			const char *actors = "sr";
			if (_mainClicks == 1)
				actors = "rs";
			else if (_mainClicks == 2)
				actors = "srs";
			inventorGroup(Common::String::format("kMainSpchIDs.%u", _mainClicks).c_str(), actors);
		}
		_idleWaiting = false;
		return "handled";
	}
	return Common::String();
}

} // namespace RR1Cloud9
} // namespace Trinket
