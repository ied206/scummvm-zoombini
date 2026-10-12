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
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

Sandwich::Sandwich(PageScene &host) : _host(host), _paths(host._vm.resources()) {}

Sandwich::~Sandwich() {
	delete _itemText;
	delete _costText;
	delete _totalText;
}

int Sandwich::number(const Common::String &key) const { return _host._page->number(key); }

int Sandwich::indexed(const char *key, int index) const {
	return number(Common::String::format("%s.%d", key, index));
}

Shell::Sprite *Sandwich::sprite(const char *resource, const char *depth) {
	return _host._scene.add(_host.characterResource(number(resource), Shell::Character::Resources::kAnimation), number(depth));
}

Shell::TextObject *Sandwich::text(int x, int y, int width, int height) {
	Shell::TextObject *result = new Shell::TextObject(_paths, _font);
	Core::ScriptArguments args = {Core::ScriptValue::fromText(" "), Core::ScriptValue(x), Core::ScriptValue(y)};
	if (width) {
		args.push_back(Core::ScriptValue(width));
		args.push_back(Core::ScriptValue(height));
	}
	args.push_back(Core::ScriptValue(1));
	result->create(args);
	result->set("color", {Core::ScriptValue(6317), Core::ScriptValue(4212)}, Core::ScriptValue(2106));
	return result;
}

void Sandwich::say(const char *actor, const Common::String &key) { _host.say(actor, number(key)); }

void Sandwich::sound(const char *key) { _host.playSounds({static_cast<uint16>(number(key))}); }

bool Sandwich::loadNames() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_host._vm.resources().open("r1ia2/19200.txt"));
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
	Common::ScopedPtr<Common::SeekableReadStream> font(_host._vm.resources().open("common/1018.nfnt"));
	_font.reset(font ? OM::NFNTFont::load(*font) : nullptr);
	if (!_font || !loadNames() || !_data.loadStream(_host._vm.resources().open("r1ia2.xml")) || !_data.parse())
		return false;
	_level = CLIP(_host._world.get("R1IA2Level"), 1, 3);
	_unlocked = _host._practice ? 0 : static_cast<uint32>(_host._world.get("R1IA2ItemsUnlocked"));
	_showMarble = !_host._practice && _host._world.get("R1IA2ShowMarble");
	_clawClicks = _host._vm.rnd().number(1, 3);
	if (!_host.actor("lizard", "kLizard"))
		return false;
	_bell = sprite("kBellID", "kBellZ");
	_register = sprite("kCashRegisterID", "kCashRegisterZ");
	_indicator = sprite("kIndicatorID", "kIndicatorZ");
	_glow = sprite("kGlowID", "kGlowZ");
	_claw = sprite("kClawID", "kClawZ");
	_top = sprite("kTopBunID", "kTopBunZ");
	_bottom = sprite("kBtmBunID", "kBtmBunZ");
	_item = sprite("kItemsID", "kItemsZ");
	_marble = sprite("kCOM1Marble3ID", "kMarbleZ");
	_arrows[0] = sprite("kArrowID.1", "kArrowZ");
	_arrows[1] = sprite("kArrowID.2", "kArrowZ");
	if (!_bell || !_register || !_indicator || !_glow || !_claw || !_top || !_bottom || !_item || !_marble || !_arrows[0] || !_arrows[1])
		return false;
	_bunY = _top->y;
	_glow->visible = false;
	_marble->x = number("kMarbleX");
	_marble->y = number("kMarbleY");
	_marble->visible = _showMarble;
	_itemText = text(number("kItemTextX"), number("kItemTextY"), number("kItemTextW"), number("kItemTextH"));
	// These coordinates replace the earlier constants in pLoadLocalAnims.
	_costText = text(370, 187, 70, 22);
	_totalText = text(number("kTotalTextX"), number("kTotalTextY"));
	_totalText->set("center", {}, Core::ScriptValue(0));
	if (_host._practice && _host._chester)
		_host._chester->visible = false;
	if (_host._practice || !_host._world.visits("r1ia2"))
		_host.speeches("kEnterCharID", "kEnterSpchID", number("kMaxEntrySpchs"));
	else
		say("lizard", Common::String::format("kReEnterSpchID.%d", _host._vm.rnd().number(1, 2)));
	return showItem();
}

bool Sandwich::readData() {
	Common::String record = _data.get(Common::String::format("DataSet/Level_%d/WordList_%d/Set_%d", _level, _dataset, _set));
	Common::StringTokenizer tokens(record);
	int values[5];
	for (int &value : values) {
		if (tokens.empty())
			return false;
		Common::String token = tokens.nextToken();
		for (char c : token)
			if (!Common::isDigit(c))
				return false;
		value = atoi(token.c_str());
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
		return Common::String::format("%d%c", price, 162);
	return Common::String::format("$%d.%02d", price / 100, price % 100);
}

bool Sandwich::showItem() {
	clearCoins();
	_item->setFrame(_current);
	_indicator->setFrame((_unlocked & (1u << (_current - 1))) ? 3 : 1);
	_itemText->set("text", {}, Core::ScriptValue::fromText(_names[_current - 1]));
	_dataset = _itemDatasets[_current - 1];
	if (!_dataset) {
		_dataset = _host._vm.rnd().number(1, indexed("kMaxDataSets", _level));
		_set = _host._vm.rnd().number(1, indexed("kNumSets", _level));
	}
	if (!readData())
		return false;
	_itemDatasets[_current - 1] = _dataset;
	_costText->set("text", {}, Core::ScriptValue::fromText(priceText(_price)));
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
			coin.sprite = _host._scene.add(_host.characterResource(indexed("kCoinID", type + 1), Shell::Character::Resources::kAnimation), 100);
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
	sound("kCoinSfx2ID");
	if (!_unlocked) {
		say("lizard", "kHereSpchID");
		say("lizard", "kRingSpchID");
	}
	return true;
}

Common::Rect Sandwich::counter(int type) const {
	int x = indexed("kCounterCoinX", type), y = indexed("kCounterCoinY", type);
	return Common::Rect(x, y, x + indexed("kCounterCoinW", type), y + indexed("kCounterCoinH", type));
}

void Sandwich::layoutCoins() {
	int home[4] = {}, paid[4] = {};
	for (const auto &coin : _coins) {
		if (coin.sprite == _dragged)
			continue;
		int type = coin.type;
		if (coin.paid) {
			Common::Rect rect = counter(type + 1);
			coin.sprite->x = rect.left + (rect.width() - coin.sprite->width()) / 2;
			coin.sprite->y = rect.bottom - coin.sprite->height() - paid[type] * number("kCoinDeltaY");
			_host._scene.setDepth(coin.sprite, number("kCoinZ") + paid[type]);
			paid[type] += 1;
		} else {
			coin.sprite->x = indexed("kRRCoinX", type + 1);
			coin.sprite->y = indexed("kRRCoinY", type + 1) - home[type] * number("kCoinDeltaY");
			_host._scene.setDepth(coin.sprite, number("kCoinZ") + home[type] * number("kCoinDeltaY"));
			home[type] += 1;
		}
	}
}

void Sandwich::refreshTotal(bool reveal) {
	_totalVisible = reveal;
	_totalText->set("text", {}, Core::ScriptValue::fromText(priceText(_total)));
	_totalText->set("x", {}, Core::ScriptValue(number("kTotalTextR") - _totalText->width()));
	if (_level == 1 && _total == _price && !_ringHelp) {
		say("sam", "kSamRingSpchID");
		_ringHelp = true;
	}
}

void Sandwich::sayPrice(int price) {
	int dollars = price / 100, cents = price % 100;
	if (dollars) {
		_host.say("lizard", number("kOneSpchID") + dollars - 1);
		say("lizard", dollars == 1 ? "kDollarSpchID" : "kDollarsSpchID");
		if (cents)
			say("lizard", "kAndSpchID");
	}
	if (cents) {
		if (20 <= cents)
			_host.say("lizard", number("kTwentySpchID") + cents / 10 - 2);
		int ones = cents < 20 ? cents : cents % 10;
		if (ones)
			_host.say("lizard", number("kOneSpchID") + ones - 1);
		say("lizard", cents == 1 ? "kCentSpchID" : "kCentsSpchID");
	} else if (!dollars) {
		say("reader", "kCOMZeroSpchID");
	}
}

void Sandwich::ringBell() {
	sound("kBellID");
	if (!_selected) {
		say("lizard", "kLizardSpchID.1.1");
		return;
	}
	if (!_total) {
		say("paige", "kPaigeSpchID.2.1");
		return;
	}
	refreshTotal(true);
	if (_total != _price) {
		wrongPrice();
		return;
	}
	say("lizard", Common::String::format("kCongratsSpchID.%d", _host._vm.rnd().number(1, 4)));
	sound("kCoinSfx1ID");
	_phase = kPaying;
}

void Sandwich::wrongPrice() {
	_wrong += 1;
	bool shortfall = _total < _price;
	bool &heard = shortfall ? _shortHelp : _excessHelp;
	if (!heard) {
		say("lizard", Common::String::format("%s.%d", shortfall ? "kNotEnoughSpchID" : "kTooMuchSpchID", _host._vm.rnd().number(1, 3)));
		say("lizard", "kWrongSpchID");
		sayPrice(_price);
		if (_level == 1) {
			say("lizard", "kAmountSpchID");
			sayPrice(_total);
		}
		heard = true;
	} else {
		say("lizard", shortfall ? "kShortSpchID.1" : "kMoreSpchID.1");
		sayPrice(ABS(_price - _total));
		say("lizard", shortfall ? "kShortSpchID.2" : "kMoreSpchID.2");
		say("lizard", shortfall ? "kAddSpchID" : "kTakeOffSpchID");
		sayPrice(ABS(_price - _total));
	}
	if (_wrong == 5) {
		int choice = _host._vm.rnd().number(1, 2);
		say("lizard", Common::String::format("kStumpedSpchID.%d.1", choice));
		if (choice == 2)
			say("lizard", "kStumpedSpchID.2.2");
	}
	if (_wrong == 7) {
		say("lizard", "kToughSpchID.1");
		say("lizard", "kToughSpchID.2");
	}
}

void Sandwich::assess() {
	if (_host._practice)
		return;
	World &world = _host._world;
	int outcome = _wrong ? 3 : 1;
	int count = world.get("R1IA2StreakKind") == outcome ? world.get("R1IA2Streak") + 1 : 1;
	world.recordAssessment("R1IA2", _level, outcome, 3, 3, count);
	world.set("R1IA2StreakKind", outcome);
	world.set("R1IA2Streak", count == 3 ? 0 : count);
	if (count < 3 || world.get("R1IA2FixedLevel"))
		return;
	int next = CLIP(_level + (_wrong ? -1 : 1), 1, 3);
	if (_level != next) {
		_level = next;
		for (int &dataset : _itemDatasets)
			dataset = 0;
		_dataset = _host._vm.rnd().number(1, indexed("kMaxDataSets", _level));
		_set = _host._vm.rnd().number(1, indexed("kNumSets", _level));
	}
}

void Sandwich::unlock() {
	assess();
	_unlocked |= 1u << (_current - 1);
	clearCoins();
	_indicator->setFrame(3);
	persist();
	sound("kUnlockSfxID");
	if (_host._vm.rnd().number(1, 2) == 1)
		say("lizard", Common::String::format("kCorrectSpchID.%d", _host._vm.rnd().number(1, 5)));
	uint count = 0;
	for (uint i = 0; i < 30; i++)
		if (_unlocked & (1u << i))
			count += 1;
	if (count == 1) {
		say("lizard", "kUnlockSpchID.1");
		say("lizard", "kUnlockSpchID.2");
	} else {
		say("lizard", count == 2 ? "kLizardSpchID.3.1" : "kUnlock3SpchID");
	}
	_phase = kReady;
}

void Sandwich::persist() {
	if (_host._practice)
		return;
	_host._world.set("R1IA2Level", _level);
	_host._world.set("R1IA2Dataset", _dataset);
	_host._world.set("R1IA2ItemsUnlocked", _unlocked);
	_host._world.set("R1IA2ShowMarble", _showMarble);
}

void Sandwich::finishSandwich() {
	_clawClicks = _clawClicks % 3 + 1;
	uint distinct = 0;
	for (int count : _placed)
		if (count)
			distinct += 1;
	if (distinct < 3) {
		if (!distinct) {
			if (_unlocked)
				_host.speeches("kNoTopCharID", "kNoTopSpchID", 1);
			else
				for (uint i = 2; i <= 3; i++)
					_host.say(_host._page->text(Common::String::format("kNoTopCharID.%u", i)), indexed("kNoTopSpchID", i));
		} else if (_clawClicks == 1) {
			_host.speeches("kNotDoneCharID.1", "kNotDoneSpchID.1", 2);
		} else {
			say("lizard", Common::String::format("kNotDoneSpchID.%u", _clawClicks));
		}
		return;
	}
	int top = number("kSandwichContainerY") + number("kSandwichContainerH");
	for (const auto &topping : _toppings)
		top = MIN(top, topping.sprite->y);
	_bunTarget = top + number("kBunExtraDeltaY") - _top->height();
	_bunStart = _host._time;
	_claw->setFrame(3);
	_top->setFrame(1);
	_phase = kClosing;
	sound("kClawID");
	sound("kDoneSfxID");
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
		Common::String food = Common::String::format("item:%d", number("kINV1Food"));
		_host._world.set(food, _host._world.get(food) + 1);
		_host.putAway();
		if (!_showMarble) {
			say("lizard", "kSpecialSpchID.1");
			say("lizard", "kSpecialSpchID.2");
			_showMarble = true;
			_marble->visible = true;
			sound("kCOM1Marble3ID");
		} else {
			say("sam", "kMarbleThereSpchID");
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
	sound("kNewBunSfxID");
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
		uint frames = number("kTopBunMoveFrames");
		uint frame = MIN<uint>(frames, static_cast<uint64>(time - _bunStart) * number("kTopBunMoveFPS") / 1000);
		_top->y = _bunY + (_bunTarget - _bunY) * static_cast<int>(frame) / static_cast<int>(frames);
		if (frame == frames) {
			say("lizard", "kStandSpchID");
			if (!_host._practice) {
				bool visited = _host._world.visits("r1lg2");
				int choice = _host._vm.rnd().number(1, visited ? 3 : 2);
				Common::String suffix = Common::String::format(".%d", choice);
				_host.say(_host._page->text((visited ? "kYesLG2CharID" : "kNoLG2CharID") + suffix),
						  number((visited ? "kYesLG2SpchID" : "kNoLG2SpchID") + suffix));
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
	uint distinct = 0, unlocked = 0;
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
			say("paige", Common::String::format("kPaigeSpchID.1.%u", choice));
		} else if (!_total)
			say("paige", "kPaigeSpchID.2.1");
		else
			say("paige", Common::String::format("kPaigeSpchID.3.%d", !unlocked ? 1 : 6 - _set));
	} else if (_selected) {
		say("lizard", _total ? "kLizardSpchID.6.1" : "kLizardSpchID.5.1");
		if (!_total)
			say("lizard", "kLizardSpchID.5.2");
	} else if (!unlocked) {
		say("lizard", "kLizardSpchID.1.1");
		say("lizard", "kLizardSpchID.1.2");
	} else if (unlocked == 1) {
		say("lizard", distinct ? "kLizardSpchID.2.2" : "kLizardSpchID.2.1");
	} else if (distinct < 3) {
		say("lizard", "kLizardSpchID.3.1");
		say("lizard", "kLizardSpchID.3.2");
	} else {
		say("lizard", "kLizardSpchID.4.1");
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
			_total -= indexed("kCoinValue", coin.type + 1);
		}
		refreshTotal(_level == 1);
		layoutCoins();
		_host._scene.setDepth(_dragged, 199);
		sound("kCOMPickUpSfxID");
		return true;
	}
	if (_showMarble && _marble->covers(point)) {
		_showMarble = _marble->visible = false;
		Common::String key = Common::String::format("item:%d", number("kINV1Marble3"));
		_host._world.set(key, _host._world.get(key) + 1);
		_host.putAway();
		persist();
		sound("kCOMPickUpSfxID");
		return true;
	}
	for (uint i = 0; i < 2; i++) {
		if (!_arrows[i]->covers(point))
			continue;
		if (_selected)
			say("lizard", "kRemoveSpchID");
		_current = (_current + (i ? 1 : 29) - 1) % 30 + 1;
		sound("kArrowClickSfxID");
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
			say("lizard", "kTotalSpchID");
		return true;
	}
	if (Common::Rect(number("kItemsHotspotX1"), number("kItemsHotspotY1"), number("kItemsHotspotX2"), number("kItemsHotspotY2")).contains(point)) {
		if (_unlocked & (1u << (_current - 1))) {
			if (number("kMaxItemPlaced") <= _placed[_current - 1] || number("kMaxItemAnswers") - 1 <= static_cast<int>(_toppings.size())) {
				say("lizard", "kLimitSpchID");
				return true;
			}
			if (!_item->covers(point))
				return true;
			_dragged = sprite("kItemsID", "kItemsAnswerZ");
			if (!_dragged)
				return true;
			_dragged->setFrame(_current);
			_dragOffset = point - Common::Point(_dragged->x, _dragged->y);
			_dragCoin = -1;
			_host._scene.setDepth(_dragged, 199);
			sound("kCOMPickUpSfxID");
		} else if (_selected) {
			_host.say("lizard", number("kItemsID") + _current - 1);
			say("lizard", "kBuySpchID");
			sayPrice(_price);
			say("lizard", _total ? "kRingSpchID" : "kHereSpchID");
		} else {
			sound("kCOMSelectSfxID");
			_host.say("lizard", number("kItemsID") + _current - 1);
			if (!_firstTopping) {
				say("reader", "kToppingSpchID");
				_firstTopping = true;
			}
			say("lizard", Common::String::format("kCommentSpchID.%d", _host._vm.rnd().number(1, 5)));
			say("lizard", "kCostSpchID");
			sayPrice(_price);
			_phase = kSelecting;
		}
		return true;
	}
	for (const auto &actor : _host._actors) {
		if (!actor._value->bounds().contains(point))
			continue;
		if (actor._key == "paige" || actor._key == "lizard") {
			help(actor._key == "paige");
		} else {
			_mainClicks = _mainClicks % 4 + 1;
			Common::String suffix = Common::String::format(".%u", _mainClicks);
			_host.speeches("kMainCharID" + suffix, "kMainSpchID" + suffix, 2);
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
		for (int i = 1; i <= 5; i++)
			if (counter(i).intersects(bounds))
				accepted = true;
		if (accepted) {
			Coin &coin = _coins[_dragCoin];
			coin.paid = true;
			_total += indexed("kCoinValue", coin.type + 1);
		}
		_dragged = nullptr;
		_dragCoin = -1;
		layoutCoins();
		refreshTotal(_level == 1);
	} else {
		int column = -1, overlap = 0;
		for (uint i = 0; i < 5; i++) {
			int x = indexed("kSandwichContainerX", i + 1), y = number("kSandwichContainerY");
			Common::Rect rect(x, y, x + number("kSandwichContainerW"), y + number("kSandwichContainerH"));
			if (!rect.intersects(bounds))
				continue;
			rect.clip(bounds);
			int addedHeight = _columns[i] ? number("kSandwichContainerDeltaY") : _dragged->height();
			if (_columnHeights[i] + addedHeight <= number("kSandwichContainerH") && overlap < rect.width() * rect.height()) {
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
			_dragged->x = indexed("kSandwichContainerX", column + 1) + (number("kSandwichContainerW") - _dragged->width()) / 2;
			_dragged->y = number("kSandwichContainerY") + number("kSandwichContainerH") - _dragged->height() -
						  _columns[column] * number("kSandwichContainerDeltaY");
			_host._scene.setDepth(_dragged, number("kItemsZ") + _toppings.size());
			_columnHeights[column] += _columns[column] ? number("kSandwichContainerDeltaY") : _dragged->height();
			_columns[column] += 1;
			_placed[_current - 1] += 1;
			accepted = true;
		} else {
			_host._scene.remove(_dragged);
		}
		_dragged = nullptr;
	}
	sound(accepted ? "kCOMDropSfxID" : "kCOMSnapHomeSfxID");
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
