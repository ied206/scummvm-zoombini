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

#include "trinket/rr1cloud9/ia2_sandwich.h"

#include "common/tokenizer.h"
#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/string_parser.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

constexpr const char *Sandwich::kNameSignedIndexFormat;
constexpr const char *Sandwich::kPropertyColor;
constexpr const char *Sandwich::kR1IA2LevelKey;
constexpr const char *Sandwich::kR1IA2ItemsUnlockedKey;
constexpr const char *Sandwich::kR1IA2ShowMarbleKey;
constexpr const char *Sandwich::kLizardName;
constexpr const char *Sandwich::kLizardKey;
constexpr const char *Sandwich::kBellIDKey;
constexpr const char *Sandwich::kBellZKey;
constexpr const char *Sandwich::kCashRegisterIDKey;
constexpr const char *Sandwich::kCashRegisterZKey;
constexpr const char *Sandwich::kIndicatorIDKey;
constexpr const char *Sandwich::kIndicatorZKey;
constexpr const char *Sandwich::kGlowIDKey;
constexpr const char *Sandwich::kGlowZKey;
constexpr const char *Sandwich::kClawIDKey;
constexpr const char *Sandwich::kClawZKey;
constexpr const char *Sandwich::kTopBunIDKey;
constexpr const char *Sandwich::kTopBunZKey;
constexpr const char *Sandwich::kBtmBunIDKey;
constexpr const char *Sandwich::kBtmBunZKey;
constexpr const char *Sandwich::kItemsIDKey;
constexpr const char *Sandwich::kItemsZKey;
constexpr const char *Sandwich::kCOM1Marble3IDKey;
constexpr const char *Sandwich::kMarbleZKey;
constexpr const char *Sandwich::kArrowID1Key;
constexpr const char *Sandwich::kArrowZKey;
constexpr const char *Sandwich::kArrowID2Key;
constexpr const char *Sandwich::kMarbleXKey;
constexpr const char *Sandwich::kMarbleYKey;
constexpr const char *Sandwich::kItemTextXKey;
constexpr const char *Sandwich::kItemTextYKey;
constexpr const char *Sandwich::kItemTextWKey;
constexpr const char *Sandwich::kItemTextHKey;
constexpr const char *Sandwich::kTotalTextXKey;
constexpr const char *Sandwich::kTotalTextYKey;
constexpr const char *Sandwich::kPropertyCenter;
constexpr const char *Sandwich::kEnterCharIDKey;
constexpr const char *Sandwich::kEnterSpchIDKey;
constexpr const char *Sandwich::kMaxEntrySpchsKey;
constexpr const char *Sandwich::kReEnterSpchIDSignedIndexKey;
constexpr const char *Sandwich::kPropertyDataSetLevelSignedIndexWordListSignedIndexSetSignedIndex;
constexpr const char *Sandwich::kSignedIndexCFormat;
constexpr const char *Sandwich::kSignedIndexSignedIndexFormat;
constexpr const char *Sandwich::kPropertyText;
constexpr const char *Sandwich::kMaxDataSetsKey;
constexpr const char *Sandwich::kNumSetsKey;
constexpr const char *Sandwich::kCoinIDKey;
constexpr const char *Sandwich::kCoinSfx2IDKey;
constexpr const char *Sandwich::kHereSpchIDKey;
constexpr const char *Sandwich::kRingSpchIDKey;
constexpr const char *Sandwich::kCounterCoinXKey;
constexpr const char *Sandwich::kCounterCoinYKey;
constexpr const char *Sandwich::kCounterCoinWKey;
constexpr const char *Sandwich::kCounterCoinHKey;
constexpr const char *Sandwich::kCoinDeltaYKey;
constexpr const char *Sandwich::kCoinZKey;
constexpr const char *Sandwich::kRRCoinXKey;
constexpr const char *Sandwich::kRRCoinYKey;
constexpr const char *Sandwich::kPropertyX;
constexpr const char *Sandwich::kTotalTextRKey;
constexpr const char *Sandwich::kSamName;
constexpr const char *Sandwich::kSamRingSpchIDKey;
constexpr const char *Sandwich::kOneSpchIDKey;
constexpr const char *Sandwich::kDollarSpchIDKey;
constexpr const char *Sandwich::kDollarsSpchIDKey;
constexpr const char *Sandwich::kAndSpchIDKey;
constexpr const char *Sandwich::kTwentySpchIDKey;
constexpr const char *Sandwich::kCentSpchIDKey;
constexpr const char *Sandwich::kCentsSpchIDKey;
constexpr const char *Sandwich::kReaderName;
constexpr const char *Sandwich::kCOMZeroSpchIDKey;
constexpr const char *Sandwich::kLizardSpchID11Key;
constexpr const char *Sandwich::kPaigeName;
constexpr const char *Sandwich::kPaigeSpchID21Key;
constexpr const char *Sandwich::kCongratsSpchIDSignedIndexKey;
constexpr const char *Sandwich::kCoinSfx1IDKey;
constexpr const char *Sandwich::kNotEnoughSpchIDKey;
constexpr const char *Sandwich::kTooMuchSpchIDKey;
constexpr const char *Sandwich::kWrongSpchIDKey;
constexpr const char *Sandwich::kAmountSpchIDKey;
constexpr const char *Sandwich::kShortSpchID1Key;
constexpr const char *Sandwich::kMoreSpchID1Key;
constexpr const char *Sandwich::kShortSpchID2Key;
constexpr const char *Sandwich::kMoreSpchID2Key;
constexpr const char *Sandwich::kAddSpchIDKey;
constexpr const char *Sandwich::kTakeOffSpchIDKey;
constexpr const char *Sandwich::kStumpedSpchIDSignedIndex1Key;
constexpr const char *Sandwich::kStumpedSpchID22Key;
constexpr const char *Sandwich::kToughSpchID1Key;
constexpr const char *Sandwich::kToughSpchID2Key;
constexpr const char *Sandwich::kR1IA2StreakKindKey;
constexpr const char *Sandwich::kR1IA2StreakKey;
constexpr const char *Sandwich::kR1IA2Key;
constexpr const char *Sandwich::kR1IA2FixedLevelKey;
constexpr const char *Sandwich::kUnlockSfxIDKey;
constexpr const char *Sandwich::kCorrectSpchIDSignedIndexKey;
constexpr const char *Sandwich::kUnlockSpchID1Key;
constexpr const char *Sandwich::kUnlockSpchID2Key;
constexpr const char *Sandwich::kLizardSpchID31Key;
constexpr const char *Sandwich::kUnlock3SpchIDKey;
constexpr const char *Sandwich::kR1IA2DatasetKey;
constexpr const char *Sandwich::kNoTopCharIDKey;
constexpr const char *Sandwich::kNoTopSpchIDKey;
constexpr const char *Sandwich::kNoTopCharIDUnsignedIndexKey;
constexpr const char *Sandwich::kNotDoneCharID1Key;
constexpr const char *Sandwich::kNotDoneSpchID1Key;
constexpr const char *Sandwich::kNotDoneSpchIDUnsignedIndexKey;
constexpr const char *Sandwich::kSandwichContainerYKey;
constexpr const char *Sandwich::kSandwichContainerHKey;
constexpr const char *Sandwich::kBunExtraDeltaYKey;
constexpr const char *Sandwich::kDoneSfxIDKey;
constexpr const char *Sandwich::kItemSignedIndexFormat;
constexpr const char *Sandwich::kINV1FoodKey;
constexpr const char *Sandwich::kSpecialSpchID1Key;
constexpr const char *Sandwich::kSpecialSpchID2Key;
constexpr const char *Sandwich::kMarbleThereSpchIDKey;
constexpr const char *Sandwich::kNewBunSfxIDKey;
constexpr const char *Sandwich::kTopBunMoveFramesKey;
constexpr const char *Sandwich::kTopBunMoveFPSKey;
constexpr const char *Sandwich::kStandSpchIDKey;
constexpr const char *Sandwich::kSignedIndexFormat;
constexpr const char *Sandwich::kYesLG2CharIDKey;
constexpr const char *Sandwich::kNoLG2CharIDKey;
constexpr const char *Sandwich::kYesLG2SpchIDKey;
constexpr const char *Sandwich::kNoLG2SpchIDKey;
constexpr const char *Sandwich::kPaigeSpchID1UnsignedIndexKey;
constexpr const char *Sandwich::kPaigeSpchID3SignedIndexKey;
constexpr const char *Sandwich::kLizardSpchID61Key;
constexpr const char *Sandwich::kLizardSpchID51Key;
constexpr const char *Sandwich::kLizardSpchID52Key;
constexpr const char *Sandwich::kLizardSpchID12Key;
constexpr const char *Sandwich::kLizardSpchID22Key;
constexpr const char *Sandwich::kLizardSpchID21Key;
constexpr const char *Sandwich::kLizardSpchID32Key;
constexpr const char *Sandwich::kLizardSpchID41Key;
constexpr const char *Sandwich::kCoinValueKey;
constexpr const char *Sandwich::kCOMPickUpSfxIDKey;
constexpr const char *Sandwich::kINV1Marble3Key;
constexpr const char *Sandwich::kRemoveSpchIDKey;
constexpr const char *Sandwich::kArrowClickSfxIDKey;
constexpr const char *Sandwich::kTotalSpchIDKey;
constexpr const char *Sandwich::kItemsHotspotX1Key;
constexpr const char *Sandwich::kItemsHotspotY1Key;
constexpr const char *Sandwich::kItemsHotspotX2Key;
constexpr const char *Sandwich::kItemsHotspotY2Key;
constexpr const char *Sandwich::kMaxItemPlacedKey;
constexpr const char *Sandwich::kMaxItemAnswersKey;
constexpr const char *Sandwich::kLimitSpchIDKey;
constexpr const char *Sandwich::kItemsAnswerZKey;
constexpr const char *Sandwich::kBuySpchIDKey;
constexpr const char *Sandwich::kCOMSelectSfxIDKey;
constexpr const char *Sandwich::kToppingSpchIDKey;
constexpr const char *Sandwich::kCommentSpchIDSignedIndexKey;
constexpr const char *Sandwich::kCostSpchIDKey;
constexpr const char *Sandwich::kUnsignedIndexFormat;
constexpr const char *Sandwich::kMainCharIDKey;
constexpr const char *Sandwich::kMainSpchIDKey;
constexpr const char *Sandwich::kSandwichContainerXKey;
constexpr const char *Sandwich::kSandwichContainerWKey;
constexpr const char *Sandwich::kSandwichContainerDeltaYKey;
constexpr const char *Sandwich::kCOMDropSfxIDKey;
constexpr const char *Sandwich::kCOMSnapHomeSfxIDKey;

constexpr const char *Sandwich::kFont;
constexpr const char *Sandwich::kNamesText;
constexpr const char *Sandwich::kDataXML;

Sandwich::Sandwich(PageScene &host) : _host(host), _paths(host._vm.resources()) {}

Sandwich::~Sandwich() {
	delete _itemText;
	delete _costText;
	delete _totalText;
}

int Sandwich::number(const Common::String &key) const { return _host._page->number(key); }

int Sandwich::indexed(const char *key, int index) const {
	return number(Common::String::format(kNameSignedIndexFormat, key, index));
}

Shell::Sprite *Sandwich::sprite(const char *resource, const char *depth) {
	return _host._scene.add(_host.characterResource(number(resource), Shell::Character::Resources::kAnimation), number(depth));
}

Shell::TextObject *Sandwich::text(int x, int y, int width, int height) {
	Shell::TextObject *result = new Shell::TextObject(_paths, _host._vm.fonts(), _font);
	Core::ScriptArguments args = {Core::ScriptValue::fromText(" "), Core::ScriptValue(x), Core::ScriptValue(y)};
	if (width) {
		args.push_back(Core::ScriptValue(width));
		args.push_back(Core::ScriptValue(height));
	}
	args.push_back(Core::ScriptValue(1));
	result->create(args);
	result->set(kPropertyColor, {Core::ScriptValue(6317), Core::ScriptValue(4212)}, Core::ScriptValue(2106));
	return result;
}

void Sandwich::say(const char *actor, const Common::String &key) { _host.say(actor, number(key)); }

void Sandwich::sound(const char *key) { _host.playSounds({static_cast<uint16>(number(key))}); }

bool Sandwich::loadNames() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_host._vm.resources().open(kNamesText));
	if (!stream || 65536 < stream->size())
		return false;
	Common::String line;
	while (stream->pos() < stream->size()) {
		char c = stream->readByte();
		if (line.empty() && c == '*')
			break;
		if (c == '\r') {
			uint comma = line.find(',');
			_names.push_back(line.substr(0, comma));
			line.clear();
		} else if (c && c != '\n') {
			line += c;
		}
	}
	return _names.size() == 30;
}

bool Sandwich::enter() {
	_font = _host._vm.fonts().load(kFont);
	if (!_font || !loadNames() || !_data.loadStream(_host._vm.resources().open(kDataXML)) || !_data.parse())
		return false;
	_level = CLIP(_host._world.get(kR1IA2LevelKey), 1, 3);
	_unlocked = _host._practice ? 0 : static_cast<uint32>(_host._world.get(kR1IA2ItemsUnlockedKey));
	_showMarble = !_host._practice && _host._world.get(kR1IA2ShowMarbleKey);
	_clawClicks = _host._vm._rnd->getRandomNumberRng(1, 3);
	if (!_host.actor(kLizardName, kLizardKey))
		return false;
	_bell = sprite(kBellIDKey, kBellZKey);
	_register = sprite(kCashRegisterIDKey, kCashRegisterZKey);
	_indicator = sprite(kIndicatorIDKey, kIndicatorZKey);
	_glow = sprite(kGlowIDKey, kGlowZKey);
	_claw = sprite(kClawIDKey, kClawZKey);
	_top = sprite(kTopBunIDKey, kTopBunZKey);
	_bottom = sprite(kBtmBunIDKey, kBtmBunZKey);
	_item = sprite(kItemsIDKey, kItemsZKey);
	_marble = sprite(kCOM1Marble3IDKey, kMarbleZKey);
	_arrows[0] = sprite(kArrowID1Key, kArrowZKey);
	_arrows[1] = sprite(kArrowID2Key, kArrowZKey);
	if (!_bell || !_register || !_indicator || !_glow || !_claw || !_top || !_bottom || !_item || !_marble || !_arrows[0] || !_arrows[1])
		return false;
	_bunY = _top->y;
	_glow->visible = false;
	_marble->colorRemap = PageScene::marbleColor(2);
	_marble->x = number(kMarbleXKey);
	_marble->y = number(kMarbleYKey);
	_marble->visible = _showMarble;
	_itemText = text(number(kItemTextXKey), number(kItemTextYKey), number(kItemTextWKey), number(kItemTextHKey));
	// These coordinates replace the earlier constants in pLoadLocalAnims.
	_costText = text(370, 187, 70, 22);
	_totalText = text(number(kTotalTextXKey), number(kTotalTextYKey));
	_totalText->set(kPropertyCenter, {}, Core::ScriptValue(0));
	if (_host._practice && _host._chester)
		_host._chester->visible = false;
	if (_host._practice || !_host._world.visits(Page::kPageIA2))
		_host.speeches(kEnterCharIDKey, kEnterSpchIDKey, number(kMaxEntrySpchsKey));
	else
		say(kLizardName, Common::String::format(kReEnterSpchIDSignedIndexKey, _host._vm._rnd->getRandomNumberRng(1, 2)));
	return showItem();
}

bool Sandwich::readData() {
	Common::String record = _data.get(Common::String::format(kPropertyDataSetLevelSignedIndexWordListSignedIndexSetSignedIndex, _level, _dataset, _set));
	Common::StringTokenizer tokens(record);
	int values[5];
	for (int &value : values) {
		if (tokens.empty())
			return false;
		Common::String token = tokens.nextToken();
		for (char c : token) {
			if (!Common::isDigit(c))
				return false;
		}
		int32 parsedValue = 0;
		if (StringParser::parseSignedInt(token.c_str(), parsedValue, StringParser::kDecimalProfile) != StringParser::Result::kSuccess)
			return false;
		value = parsedValue;
	}
	if (!tokens.empty() || values[0] <= 0)
		return false;
	_price = values[0];
	for (uint i = 0; i < 4; i++) {
		if (values[i + 1] < 0 || 10 < values[i + 1])
			return false;
		_supply[i] = values[i + 1];
	}
	return true;
}

Common::String Sandwich::priceText(int price) {
	if (price < 100)
		return Common::String::format(kSignedIndexCFormat, price, 162);
	return Common::String::format(kSignedIndexSignedIndexFormat, price / 100, price % 100);
}

bool Sandwich::showItem() {
	clearCoins();
	_item->setFrame(_current);
	_indicator->setFrame((_unlocked & (1u << (_current - 1))) ? 3 : 1);
	_itemText->set(kPropertyText, {}, Core::ScriptValue::fromText(_names[_current - 1]));
	_dataset = _itemDatasets[_current - 1];
	if (!_dataset) {
		_dataset = _host._vm._rnd->getRandomNumberRng(1, indexed(kMaxDataSetsKey, _level));
		_set = _host._vm._rnd->getRandomNumberRng(1, indexed(kNumSetsKey, _level));
	}
	if (!readData())
		return false;
	_itemDatasets[_current - 1] = _dataset;
	_costText->set(kPropertyText, {}, Core::ScriptValue::fromText(priceText(_price)));
	_wrong = 0;
	persist();
	return true;
}

void Sandwich::clearCoins() {
	for (const auto &coin : _coins)
		_host._scene.remove(coin.sprite);
	_coins.clear();
	_selected = false;
	_total = 0;
	_totalVisible = false;
}

bool Sandwich::startPayment() {
	clearCoins();
	for (uint type = 0; type < 4; type++) {
		for (int i = 0; i < _supply[type]; i++) {
			Coin coin;
			coin.type = type;
			coin.sprite = _host._scene.add(_host.characterResource(indexed(kCoinIDKey, type + 1), Shell::Character::Resources::kAnimation), 100);
			if (!coin.sprite) {
				clearCoins();
				return false;
			}
			_coins.push_back(coin);
		}
	}
	_selected = true;
	_shortHelp = _excessHelp = false;
	_indicator->setFrame(2);
	layoutCoins();
	sound(kCoinSfx2IDKey);
	if (!_unlocked) {
		say(kLizardName, kHereSpchIDKey);
		say(kLizardName, kRingSpchIDKey);
	}
	return true;
}

Common::Rect Sandwich::counter(int type) const {
	int x = indexed(kCounterCoinXKey, type);
	int y = indexed(kCounterCoinYKey, type);
	return Common::Rect(x, y, x + indexed(kCounterCoinWKey, type), y + indexed(kCounterCoinHKey, type));
}

void Sandwich::layoutCoins() {
	int home[4] = {};
	int paid[4] = {};
	for (const auto &coin : _coins) {
		if (coin.sprite == _dragged)
			continue;
		int type = coin.type;
		if (coin.paid) {
			Common::Rect rect = counter(type + 1);
			coin.sprite->x = rect.left + (rect.width() - coin.sprite->width()) / 2;
			coin.sprite->y = rect.bottom - coin.sprite->height() - paid[type] * number(kCoinDeltaYKey);
			_host._scene.setDepth(coin.sprite, number(kCoinZKey) + paid[type]);
			paid[type] += 1;
		} else {
			coin.sprite->x = indexed(kRRCoinXKey, type + 1);
			coin.sprite->y = indexed(kRRCoinYKey, type + 1) - home[type] * number(kCoinDeltaYKey);
			_host._scene.setDepth(coin.sprite, number(kCoinZKey) + home[type] * number(kCoinDeltaYKey));
			home[type] += 1;
		}
	}
}

void Sandwich::refreshTotal(bool reveal) {
	_totalVisible = reveal;
	_totalText->set(kPropertyText, {}, Core::ScriptValue::fromText(priceText(_total)));
	_totalText->set(kPropertyX, {}, Core::ScriptValue(number(kTotalTextRKey) - _totalText->width()));
	if (_level == 1 && _total == _price && !_ringHelp) {
		say(kSamName, kSamRingSpchIDKey);
		_ringHelp = true;
	}
}

void Sandwich::sayPrice(int price) {
	int dollars = price / 100;
	int cents = price % 100;
	if (dollars) {
		_host.say(kLizardName, number(kOneSpchIDKey) + dollars - 1);
		say(kLizardName, dollars == 1 ? kDollarSpchIDKey : kDollarsSpchIDKey);
		if (cents)
			say(kLizardName, kAndSpchIDKey);
	}
	if (cents) {
		if (20 <= cents)
			_host.say(kLizardName, number(kTwentySpchIDKey) + cents / 10 - 2);
		int ones = cents < 20 ? cents : cents % 10;
		if (ones)
			_host.say(kLizardName, number(kOneSpchIDKey) + ones - 1);
		say(kLizardName, cents == 1 ? kCentSpchIDKey : kCentsSpchIDKey);
	} else if (!dollars) {
		say(kReaderName, kCOMZeroSpchIDKey);
	}
}

void Sandwich::ringBell() {
	sound(kBellIDKey);
	if (!_selected) {
		say(kLizardName, kLizardSpchID11Key);
		return;
	}
	if (!_total) {
		say(kPaigeName, kPaigeSpchID21Key);
		return;
	}
	refreshTotal(true);
	if (_total != _price) {
		wrongPrice();
		return;
	}
	say(kLizardName, Common::String::format(kCongratsSpchIDSignedIndexKey, _host._vm._rnd->getRandomNumberRng(1, 4)));
	sound(kCoinSfx1IDKey);
	_phase = kPaying;
}

void Sandwich::wrongPrice() {
	_wrong += 1;
	bool shortfall = _total < _price;
	bool &heard = shortfall ? _shortHelp : _excessHelp;
	if (!heard) {
		const char *speechKey;
		if (shortfall)
			speechKey = kNotEnoughSpchIDKey;
		else
			speechKey = kTooMuchSpchIDKey;
		say(kLizardName, Common::String::format(kNameSignedIndexFormat, speechKey, _host._vm._rnd->getRandomNumberRng(1, 3)));
		say(kLizardName, kWrongSpchIDKey);
		sayPrice(_price);
		if (_level == 1) {
			say(kLizardName, kAmountSpchIDKey);
			sayPrice(_total);
		}
		heard = true;
	} else {
		say(kLizardName, shortfall ? kShortSpchID1Key : kMoreSpchID1Key);
		sayPrice(ABS(_price - _total));
		say(kLizardName, shortfall ? kShortSpchID2Key : kMoreSpchID2Key);
		say(kLizardName, shortfall ? kAddSpchIDKey : kTakeOffSpchIDKey);
		sayPrice(ABS(_price - _total));
	}
	if (_wrong == 5) {
		int choice = _host._vm._rnd->getRandomNumberRng(1, 2);
		say(kLizardName, Common::String::format(kStumpedSpchIDSignedIndex1Key, choice));
		if (choice == 2)
			say(kLizardName, kStumpedSpchID22Key);
	}
	if (_wrong == 7) {
		say(kLizardName, kToughSpchID1Key);
		say(kLizardName, kToughSpchID2Key);
	}
}

void Sandwich::assess() {
	if (_host._practice)
		return;
	World &world = _host._world;
	int outcome = _wrong ? 3 : 1;
	int count = world.get(kR1IA2StreakKindKey) == outcome ? world.get(kR1IA2StreakKey) + 1 : 1;
	world.recordAssessment(kR1IA2Key, _level, outcome, 3, 3, count);
	world.set(kR1IA2StreakKindKey, outcome);
	world.set(kR1IA2StreakKey, count == 3 ? 0 : count);
	if (count < 3 || world.get(kR1IA2FixedLevelKey))
		return;
	int next = CLIP(_level + (_wrong ? -1 : 1), 1, 3);
	if (_level != next) {
		_level = next;
		for (int &dataset : _itemDatasets)
			dataset = 0;
		_dataset = _host._vm._rnd->getRandomNumberRng(1, indexed(kMaxDataSetsKey, _level));
		_set = _host._vm._rnd->getRandomNumberRng(1, indexed(kNumSetsKey, _level));
	}
}

void Sandwich::unlock() {
	assess();
	_unlocked |= 1u << (_current - 1);
	clearCoins();
	_indicator->setFrame(3);
	persist();
	sound(kUnlockSfxIDKey);
	if (_host._vm._rnd->getRandomNumberRng(1, 2) == 1)
		say(kLizardName, Common::String::format(kCorrectSpchIDSignedIndexKey, _host._vm._rnd->getRandomNumberRng(1, 5)));
	uint count = 0;
	for (uint i = 0; i < 30; i++) {
		if (_unlocked & (1u << i))
			count += 1;
	}
	if (count == 1) {
		say(kLizardName, kUnlockSpchID1Key);
		say(kLizardName, kUnlockSpchID2Key);
	} else {
		say(kLizardName, count == 2 ? kLizardSpchID31Key : kUnlock3SpchIDKey);
	}
	_phase = kReady;
}

void Sandwich::persist() {
	if (_host._practice)
		return;
	_host._world.set(kR1IA2LevelKey, _level);
	_host._world.set(kR1IA2DatasetKey, _dataset);
	_host._world.set(kR1IA2ItemsUnlockedKey, _unlocked);
	_host._world.set(kR1IA2ShowMarbleKey, _showMarble);
}

void Sandwich::finishSandwich() {
	_clawClicks = _clawClicks % 3 + 1;
	uint distinct = 0;
	for (int count : _placed) {
		if (count)
			distinct += 1;
	}
	if (distinct < 3) {
		if (!distinct) {
			if (_unlocked) {
				_host.speeches(kNoTopCharIDKey, kNoTopSpchIDKey, 1);
			} else {
				for (uint i = 2; i <= 3; i++)
					_host.say(_host._page->text(Common::String::format(kNoTopCharIDUnsignedIndexKey, i)), indexed(kNoTopSpchIDKey, i));
			}
		} else if (_clawClicks == 1) {
			_host.speeches(kNotDoneCharID1Key, kNotDoneSpchID1Key, 2);
		} else {
			say(kLizardName, Common::String::format(kNotDoneSpchIDUnsignedIndexKey, _clawClicks));
		}
		return;
	}
	int top = number(kSandwichContainerYKey) + number(kSandwichContainerHKey);
	for (const auto &topping : _toppings)
		top = MIN(top, topping.sprite->y);
	_bunTarget = top + number(kBunExtraDeltaYKey) - _top->height();
	_bunStart = _host._time;
	_claw->setFrame(3);
	_top->setFrame(1);
	_phase = kClosing;
	sound(kClawIDKey);
	sound(kDoneSfxIDKey);
}

void Sandwich::award() {
	clearCoins();
	_top->visible = _bottom->visible = false;
	for (const auto &topping : _toppings)
		_host._scene.remove(topping.sprite);
	_toppings.clear();
	_unlocked = 0;
	for (int &count : _placed)
		count = 0;
	for (int &count : _columns)
		count = 0;
	for (int &height : _columnHeights)
		height = 0;
	for (int &dataset : _itemDatasets)
		dataset = 0;
	if (!_host._practice) {
		Common::String food = Common::String::format(kItemSignedIndexFormat, number(kINV1FoodKey));
		_host._world.set(food, _host._world.get(food) + 1);
		_host.putAway();
		if (!_showMarble) {
			say(kLizardName, kSpecialSpchID1Key);
			say(kLizardName, kSpecialSpchID2Key);
			_showMarble = true;
			_marble->visible = true;
			sound(kCOM1Marble3IDKey);
		} else {
			say(kSamName, kMarbleThereSpchIDKey);
		}
	}
	// The consumed ingredients and awards are committed together. Presentation
	// can be interrupted by saving without allowing a second award on reload.
	persist();
	_phase = kReward;
}

void Sandwich::newSandwich() {
	_top->y = _bunY;
	_top->visible = _bottom->visible = true;
	_claw->setFrame(1);
	sound(kNewBunSfxIDKey);
	if (!showItem())
		error("Trinket: missing IA2 price data");
	_phase = kReady;
}

void Sandwich::update(uint32 time) {
	if (_host._nextSpeech < _host._speech.size())
		return;
	if (_phase == kSelecting) {
		if (!startPayment())
			error("Trinket: missing IA2 coin resource");
		_phase = kReady;
	} else if (_phase == kPaying) {
		unlock();
	} else if (_phase == kClosing) {
		uint frames = number(kTopBunMoveFramesKey);
		uint frame = MIN<uint>(frames, static_cast<uint64>(time - _bunStart) * number(kTopBunMoveFPSKey) / 1000);
		_top->y = _bunY + (_bunTarget - _bunY) * static_cast<int>(frame) / static_cast<int>(frames);
		if (frame == frames) {
			say(kLizardName, kStandSpchIDKey);
			if (!_host._practice) {
				bool visited = _host._world.visits(Page::kPageLG2);
				int choice = _host._vm._rnd->getRandomNumberRng(1, visited ? 3 : 2);
				Common::String suffix = Common::String::format(kSignedIndexFormat, choice);
				_host.say(_host._page->text((visited ? kYesLG2CharIDKey : kNoLG2CharIDKey) + suffix),
						  number((visited ? kYesLG2SpchIDKey : kNoLG2SpchIDKey) + suffix));
			}
			award();
		}
	} else if (_phase == kReward) {
		newSandwich();
	}
}

void Sandwich::skipSpeech() {
	if (_phase != kClosing && !_dragged)
		_host.interrupt();
}

void Sandwich::help(bool paige) {
	uint distinct = 0;
	uint unlocked = 0;
	for (uint i = 0; i < 30; i++) {
		if (_placed[i])
			distinct += 1;
		if (_unlocked & (1u << i))
			unlocked += 1;
	}
	if (paige) {
		if (!_selected) {
			uint choice = distinct < 3 ? 2 : 3;
			if (!distinct)
				choice = 1;
			say(kPaigeName, Common::String::format(kPaigeSpchID1UnsignedIndexKey, choice));
		} else if (!_total)
			say(kPaigeName, kPaigeSpchID21Key);
		else
			say(kPaigeName, Common::String::format(kPaigeSpchID3SignedIndexKey, !unlocked ? 1 : 6 - _set));
	} else if (_selected) {
		say(kLizardName, _total ? kLizardSpchID61Key : kLizardSpchID51Key);
		if (!_total)
			say(kLizardName, kLizardSpchID52Key);
	} else if (!unlocked) {
		say(kLizardName, kLizardSpchID11Key);
		say(kLizardName, kLizardSpchID12Key);
	} else if (unlocked == 1) {
		say(kLizardName, distinct ? kLizardSpchID22Key : kLizardSpchID21Key);
	} else if (distinct < 3) {
		say(kLizardName, kLizardSpchID31Key);
		say(kLizardName, kLizardSpchID32Key);
	} else {
		say(kLizardName, kLizardSpchID41Key);
	}
}

bool Sandwich::click(Common::Point point) {
	if (busy())
		return true;
	Shell::Sprite *hit = _host._scene.hitSprite(point);
	for (uint i = 0; i < _coins.size(); i++) {
		Coin &coin = _coins[i];
		if (hit != coin.sprite)
			continue;
		_dragged = coin.sprite;
		_dragCoin = i;
		_dragOffset = point - Common::Point(_dragged->x, _dragged->y);
		if (coin.paid) {
			coin.paid = false;
			_total -= indexed(kCoinValueKey, coin.type + 1);
		}
		refreshTotal(_level == 1);
		layoutCoins();
		_host._scene.setDepth(_dragged, 199);
		sound(kCOMPickUpSfxIDKey);
		return true;
	}
	if (_showMarble && _marble->covers(point)) {
		_showMarble = _marble->visible = false;
		Common::String key = Common::String::format(kItemSignedIndexFormat, number(kINV1Marble3Key));
		_host._world.set(key, _host._world.get(key) + 1);
		_host.putAway();
		persist();
		sound(kCOMPickUpSfxIDKey);
		return true;
	}
	for (uint i = 0; i < 2; i++) {
		if (!_arrows[i]->covers(point))
			continue;
		if (_selected)
			say(kLizardName, kRemoveSpchIDKey);
		_current = (_current + (i ? 1 : 29) - 1) % 30 + 1;
		sound(kArrowClickSfxIDKey);
		if (!showItem())
			error("Trinket: missing IA2 price data");
		return true;
	}
	if (_bell->covers(point)) {
		ringBell();
		return true;
	}
	if (_claw->covers(point) || _top->covers(point)) {
		finishSandwich();
		return true;
	}
	if (Common::Rect(_register->x, _register->y, _register->x + _register->width(), _register->y + _register->height()).contains(point)) {
		if (_level == 1)
			sayPrice(_total);
		else if (!_totalVisible)
			say(kLizardName, kTotalSpchIDKey);
		return true;
	}
	if (Common::Rect(number(kItemsHotspotX1Key), number(kItemsHotspotY1Key), number(kItemsHotspotX2Key), number(kItemsHotspotY2Key)).contains(point)) {
		if (_unlocked & (1u << (_current - 1))) {
			if (number(kMaxItemPlacedKey) <= _placed[_current - 1] || number(kMaxItemAnswersKey) - 1 <= static_cast<int>(_toppings.size())) {
				say(kLizardName, kLimitSpchIDKey);
				return true;
			}
			if (!_item->covers(point))
				return true;
			_dragged = sprite(kItemsIDKey, kItemsAnswerZKey);
			if (!_dragged)
				return true;
			_dragged->setFrame(_current);
			_dragOffset = point - Common::Point(_dragged->x, _dragged->y);
			_dragCoin = -1;
			_host._scene.setDepth(_dragged, 199);
			sound(kCOMPickUpSfxIDKey);
		} else if (_selected) {
			_host.say(kLizardName, number(kItemsIDKey) + _current - 1);
			say(kLizardName, kBuySpchIDKey);
			sayPrice(_price);
			say(kLizardName, _total ? kRingSpchIDKey : kHereSpchIDKey);
		} else {
			sound(kCOMSelectSfxIDKey);
			_host.say(kLizardName, number(kItemsIDKey) + _current - 1);
			if (!_firstTopping) {
				say(kReaderName, kToppingSpchIDKey);
				_firstTopping = true;
			}
			say(kLizardName, Common::String::format(kCommentSpchIDSignedIndexKey, _host._vm._rnd->getRandomNumberRng(1, 5)));
			say(kLizardName, kCostSpchIDKey);
			sayPrice(_price);
			_phase = kSelecting;
		}
		return true;
	}
	for (const auto &actor : _host._actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		if (actor._key.equals(kPaigeName) || actor._key.equals(kLizardName)) {
			help(actor._key.equals(kPaigeName));
		} else {
			_mainClicks = _mainClicks % 4 + 1;
			Common::String suffix = Common::String::format(kUnsignedIndexFormat, _mainClicks);
			_host.speeches(kMainCharIDKey + suffix, kMainSpchIDKey + suffix, 2);
		}
		return true;
	}
	return false;
}

void Sandwich::drop(Common::Point point) {
	_dragged->x = point.x - _dragOffset.x;
	_dragged->y = point.y - _dragOffset.y;
	Common::Rect bounds(_dragged->x, _dragged->y, _dragged->x + _dragged->width(), _dragged->y + _dragged->height());
	bool accepted = false;
	if (0 <= _dragCoin) {
		for (int i = 1; i <= 5; i++) {
			if (counter(i).intersects(bounds))
				accepted = true;
		}
		if (accepted) {
			Coin &coin = _coins[_dragCoin];
			coin.paid = true;
			_total += indexed(kCoinValueKey, coin.type + 1);
		}
		_dragged = nullptr;
		_dragCoin = -1;
		layoutCoins();
		refreshTotal(_level == 1);
	} else {
		int column = -1;
		int overlap = 0;
		for (uint i = 0; i < 5; i++) {
			int x = indexed(kSandwichContainerXKey, i + 1);
			int y = number(kSandwichContainerYKey);
			Common::Rect rect(x, y, x + number(kSandwichContainerWKey), y + number(kSandwichContainerHKey));
			if (!rect.intersects(bounds))
				continue;
			rect.clip(bounds);
			int addedHeight = _columns[i] ? number(kSandwichContainerDeltaYKey) : _dragged->height();
			if (_columnHeights[i] + addedHeight <= number(kSandwichContainerHKey) && overlap < rect.width() * rect.height()) {
				column = i;
				overlap = rect.width() * rect.height();
			}
		}
		if (0 <= column) {
			Topping topping;
			topping.sprite = _dragged;
			topping.item = _current;
			topping.column = column;
			_toppings.push_back(topping);
			_dragged->x = indexed(kSandwichContainerXKey, column + 1) + (number(kSandwichContainerWKey) - _dragged->width()) / 2;
			_dragged->y = number(kSandwichContainerYKey) + number(kSandwichContainerHKey) - _dragged->height() -
						  _columns[column] * number(kSandwichContainerDeltaYKey);
			_host._scene.setDepth(_dragged, number(kItemsZKey) + _toppings.size());
			_columnHeights[column] += _columns[column] ? number(kSandwichContainerDeltaYKey) : _dragged->height();
			_columns[column] += 1;
			_placed[_current - 1] += 1;
			accepted = true;
		} else {
			_host._scene.remove(_dragged);
		}
		_dragged = nullptr;
	}
	sound(accepted ? kCOMDropSfxIDKey : kCOMSnapHomeSfxIDKey);
}

bool Sandwich::event(const Common::Event &event) {
	if (_dragged) {
		if (event.type == Common::EVENT_MOUSEMOVE) {
			_dragged->x = event.mouse.x - _dragOffset.x;
			_dragged->y = event.mouse.y - _dragOffset.y;
		} else if (event.type == Common::EVENT_LBUTTONUP) {
			drop(event.mouse);
		}
		return true;
	}
	if (event.type == Common::EVENT_MOUSEMOVE && !busy()) {
		for (auto *arrow : _arrows)
			arrow->setFrame(arrow->covers(event.mouse) ? 2 : 1);
		_bell->setFrame(_bell->covers(event.mouse) ? 2 : 1);
		bool bun = _claw->covers(event.mouse) || _top->covers(event.mouse);
		_claw->setFrame(bun ? 2 : 1);
		_top->setFrame(bun ? 2 : 1);
		_glow->visible = Common::Rect(315, 70, 485, 215).contains(event.mouse);
	}
	return false;
}

void Sandwich::draw(OM::Canvas &canvas) {
	_host._scene.drawBackground(canvas);
	_host._scene.drawSprites(canvas, INT_MIN, 10);
	_itemText->draw(canvas);
	_costText->draw(canvas);
	_host._scene.drawSprites(canvas, 11, 75);
	if (_totalVisible)
		_totalText->draw(canvas);
	_host._scene.drawSprites(canvas, 76, INT_MAX);
}

} // namespace RR1Cloud9
} // namespace Trinket
