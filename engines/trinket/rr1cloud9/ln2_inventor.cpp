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
	static constexpr const char *names[] = {
		"",
		kRBandsName,
		kHulaHoopName,
		kPillowName,
	};
	return item <= 3 ? names[item] : "";
}

void PageScene::inventorGroup(const char *prefix, const char *actors) {
	for (uint i = 0; actors[i]; i++) {
		const char *actor = kIrvinName;
		if (actors[i] == 'r')
			actor = kReaderName;
		else if (actors[i] == 's')
			actor = kSamName;
		say(actor, _page->number(Common::String::format(kNameUnsignedIndexFormat, prefix, i + 1)));
	}
}

bool PageScene::enterInventor() {
	if (!actor(kIrvinName, kIrvinKey))
		return false;
	for (uint item = 1; item <= 3; item++) {
		Common::String prefix = item == 1 ? kRubberBandsKey : Common::String(kConstantPrefix) + partName(item);
		_parts[item - 1] = _scene.add(characterResource(_page->number(prefix + kIDName), kAnimation), _page->number(prefix + kZName));
		if (!_parts[item - 1])
			return false;
		_parts[item - 1]->visible = _world.get(Common::String(kR1LN2Key) + partName(item) + kDeliveredKey);
	}
	_ambient = _scene.add(characterResource(_page->number(kAmbientIDKey), kAnimation), _page->number(kAmbientZKey));
	if (!_ambient)
		return false;
	int savedState = MAX(1, _world.get(kR1LN2StateKey));
	int state = savedState;
	uint collected = 0;
	for (uint item = 1; item <= 3; item++) {
		if (_world.get(Common::String::format(kItemUnsignedIndexKey, item)))
			collected += 1;
	}
	if (2 <= state)
		state = MAX(state, 2 + static_cast<int>(collected));
	int reentry = state == savedState ? _world.get(kR1LN2ReentryKey) + 1 : 0;
	_world.set(kR1LN2StateKey, state);
	_world.set(kR1LN2ReentryKey, reentry);
	bool fromCatapult = _world.get(kPreviousLocationKey) == 10;
	if (fromCatapult)
		_world.set(kR1LN2CatapultReentryKey, _world.get(kR1LN2CatapultReentryKey) + 1);
	int pending = _world.get(kR1LN2IntroPendingKey);
	if (_world.get(kR1LN2MoviePendingKey)) {
		_inventorPhase = kInventorMovie;
	} else if (_world.get(kR1LN2DeliveryPendingKey)) {
		_inventorPhase = kInventorDeliveryWait;
	} else if (pending == 2) {
		_inventorPhase = kInventorIntro;
	} else if (pending || state == 1) {
		_world.set(kR1LN2StateKey, 1);
		inventorSpeech();
	} else if (fromCatapult && _world.get(kR1LN2CatapultReentryKey) == 1) {
		inventorGroup(kReEnterSpchIDs10Key, kIriName);
	} else if (state == 2 && reentry == 1) {
		inventorGroup(kReEnterSpchIDs1Key, kRriName);
	} else if (state == 3 && reentry == 0) {
		inventorGroup(kReEnterSpchIDs2Key, kSiName);
	} else if (state == 4 && reentry == 0) {
		inventorGroup(kReEnterSpchIDs3Key, kSiriName);
	} else if (state == 5) {
		inventorGroup(reentry == 0 ? kReEnterSpchIDs7Key : kReEnterSpchIDs8Key, reentry == 0 ? kSiiName : kIrName);
	} else {
		uint choice = _vm.rnd().number(1, 6);
		if (choice <= 3)
			inventorGroup(Common::String::format(kReEnterSpchIDsUnsignedIndexKey, choice + 3).c_str(), choice == 1 ? kIirName : kRiName);
	}
	return true;
}

void PageScene::inventorSpeech() {
	int state = _world.get(kR1LN2StateKey);
	if (state == 1) {
		_world.set(kR1LN2StateKey, 2);
		_world.set(kR1LN2ReentryKey, 0);
		_world.set(kTalkedToInventorKey, 1);
		_world.set(kR1LN2IntroPendingKey, 1);
		_inventorPhase = kInventorIntro;
		_irvinClicks = 0;
		inventorGroup(kInitialInstrSpchIDs1Key, kIirsiriisirirsrName);
		inventorGroup(kInitialInstrSpchIDs2Key, kIiisiName);
		// The two companions answer in parallel before Irvin's last line.
		say(kReaderName, _page->number(kInitialInstrSpchIDs27Key));
		if (!_speech.empty())
			_speech.back().together = true;
		say(kSamName, _page->number(kInitialInstrSpchIDs26Key));
		say(kIrvinName, _page->number(kInitialInstrSpchIDs28Key));
	} else if (state == 5) {
		say(kIrvinName, _page->number(kInstrSpchIDs5Key));
	} else {
		_irvinClicks = _irvinClicks % 3 + 1;
		const char *actors = kIiName;
		if (_irvinClicks == 1)
			actors = kRisName;
		else if (_irvinClicks == 2)
			actors = kRiName;
		inventorGroup(Common::String::format(kInstrSpchIDsUnsignedIndexKey, _irvinClicks + 1).c_str(), actors);
	}
}

void PageScene::inventorHint(uint item) {
	if (_highlight)
		_scene.remove(_highlight);
	Common::String prefix = Common::String::format(kCollectiblesUnsignedIndexFormat, item + 14);
	_highlight = _scene.add(Common::String::format(kInventoryAnimationFormat, _page->number(prefix + kInventoryImageField)), 203);
	if (_highlight) {
		_highlight->setFrame(2);
		_highlightDue = _time + 100;
		_highlight->x = _page->number(prefix + kInventoryXField);
		_highlight->y = _page->number(prefix + kInventoryYField) - _page->number(kChesterYOffsetKey);
	}
	static constexpr uint kHintIndices[] = {
		0,
		2,
		1,
		3,
	};
	say(kIrvinName, _page->number(Common::String::format(kInitialInstrSpchIDs3UnsignedIndexKey, kHintIndices[item])));
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
		_world.set(kR1LN2IntroPendingKey, 2);
		_world.set(kShowInventoryOutlinesKey, 1);
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
		say(kSamName, _page->number(kInitialInstrSpchIDs36Key));
		_inventorPhase = kInventorOutro;
		break;
	case kInventorOutro:
		_world.set(kR1LN2IntroPendingKey, 0);
		_inventorPhase = kInventorIdle;
		break;
	case kInventorDeliveryWait: {
		uint item = _world.get(kR1LN2DeliveryPendingKey);
		if (item < 1 || 3 < item) {
			_inventorPhase = kInventorIdle;
			break;
		}
		_world.set(Common::String(kR1LN2Key) + partName(item) + kDeliveredKey, 1);
		_parts[item - 1]->visible = true;
		static constexpr const char *kPrefixes[] = {
			"",
			kBandsName,
			kHoopName,
			kPillowName,
		};
		const char *prefix = kPrefixes[item];
		speeches(Common::String(kConstantPrefix) + prefix + kCharIDName, Common::String(kConstantPrefix) + prefix + kSpchIDName,
			_page->number(Common::String(kNumKey) + prefix + kSpchName));
		_inventorPhase = kInventorDeliverySpeech;
		break;
	}
	case kInventorDeliverySpeech:
		inventorMoreItems();
		_inventorPhase = kInventorMoreItems;
		break;
	case kInventorMoreItems: {
		_world.set(kR1LN2DeliveryPendingKey, 0);
		bool complete = true;
		for (uint item = 1; item <= 3; item++)
			complete &= _parts[item - 1]->visible;
		_inventorPhase = complete ? kInventorMovie : kInventorIdle;
		if (complete) {
			for (uint item = 1; item <= 3; item++) {
				_world.set(Common::String::format(kItemUnsignedIndexKey, item), 1);
				_world.set(Common::String(kR1LN2Key) + partName(item) + kDeliveredKey, 0);
			}
			_world.set(kR1LN2MoviePendingKey, 1);
		}
		break;
	}
	default:
		break;
	}
}

void PageScene::inventorMoreItems() {
	uint delivered = 0;
	for (auto *part : _parts) {
		if (part->visible)
			delivered += 1;
	}
	if (delivered == 3) {
		speeches(kItemsCharIDs2Key, kItemsSpchIDs2Key, _page->number(kNumItemSpch2Key));
		return;
	}
	say(_page->text(kItemsCharIDs11Key), _page->number(kItemsSpchIDs11Key));
	uint words = 0;
	for (uint item : {2U, 1U, 3U}) {
		if (_parts[item - 1]->visible)
			continue;
		if (words == 1 && delivered == 1) {
			say(_page->text(kItemsCharIDs15Key), _page->number(kItemsSpchIDs15Key));
			words += 1;
		}
		uint index = 4;
		if (item == 2)
			index = 2;
		else if (item == 1)
			index = 3;
		Common::String suffix = Common::String::format(k1UnsignedIndexFormat, index);
		say(_page->text(kItemsCharIDsKey + suffix), _page->number(kItemsSpchIDsKey + suffix));
		words += 1;
	}
}

void PageScene::inventorDrop(uint item) {
	interrupt();
	Common::String key = Common::String::format(kItemUnsignedIndexKey, item);
	if (item < 1 || 3 < item || !_world.get(key) || _world.get(Common::String(kR1LN2Key) + partName(item) + kDeliveredKey)) {
		say(_page->text(kNoUseCharIDKey), _page->number(kNoUseSpchIDKey));
		return;
	}
	_world.set(key, _world.get(key) - 1);
	_world.set(kR1LN2DeliveryPendingKey, item);
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
	if (time - _idleStart < static_cast<uint>(_page->number(kGIdleHelpTimeName)))
		return;
	interrupt();
	_idleWaiting = false;
	if (_world.get(kR1LN2StateKey) == 5)
		say(kReaderName, _page->number(kIdleSpchIDs3Key));
	else if (_vm.rnd().number(1, 2) == 1)
		inventorGroup(kIdleSpchIDs1Key, kRsName);
	else
		inventorGroup(kIdleSpchIDs2Key, kSrsName);
}

Common::String PageScene::inventorClick(Common::Point point) {
	for (const auto &actor : _actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		if (actor._key.equals(kIrvinName))
			inventorSpeech();
		else if (actor._key.equals(kPaigeName))
			say(kPaigeName, _page->number(_world.get(kR1LN2StateKey) == 5 ? kHelpSpchIDs2Key : kHelpSpchIDs1Key));
		else {
			_mainClicks = _mainClicks % 3 + 1;
			const char *actors = kSrName;
			if (_mainClicks == 1)
				actors = kRsName;
			else if (_mainClicks == 2)
				actors = kSrsName;
			inventorGroup(Common::String::format(kMainSpchIDsUnsignedIndexKey, _mainClicks).c_str(), actors);
		}
		_idleWaiting = false;
		return kHandledName;
	}
	return Common::String();
}

} // namespace RR1Cloud9
} // namespace Trinket
