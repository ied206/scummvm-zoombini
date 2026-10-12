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

#ifndef TRINKET_RR1CLOUD9_IA2_SANDWICH_H
#define TRINKET_RR1CLOUD9_IA2_SANDWICH_H

#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** IA2 exact-payment activity and sandwich assembly. */
class Sandwich {
public:
	explicit Sandwich(PageScene &host);
	~Sandwich();
	bool enter();
	void update(uint32 time);
	void draw(OM::Canvas &canvas);
	bool click(Common::Point point);
	bool event(const Common::Event &event);
	bool busy() const { return _phase != kReady || _dragged; }
	void skipSpeech();
	void resume(uint32 elapsed) { _bunStart += elapsed; }

private:
	enum Phase {
		kReady,
		kSelecting,
		kPaying,
		kClosing,
		kReward,
	};
	struct Coin {
		Shell::Sprite *sprite = nullptr;
		int type = 0;
		bool paid = false;
	};
	struct Topping {
		Shell::Sprite *sprite = nullptr;
		int item = 0, column = 0;
	};
	PageScene &_host;
	Shell::ResourcePaths _paths;
	Common::SharedPtr<OM::NFNTFont> _font;
	OM::XMLParser _data;
	Common::StringArray _names;
	Shell::TextObject *_itemText = nullptr, *_costText = nullptr, *_totalText = nullptr;
	Shell::Sprite *_bell = nullptr, *_register = nullptr, *_indicator = nullptr, *_glow = nullptr;
	Shell::Sprite *_claw = nullptr, *_top = nullptr, *_bottom = nullptr, *_item = nullptr, *_marble = nullptr;
	Shell::Sprite *_arrows[2] = {}, *_dragged = nullptr;
	Common::Array<Coin> _coins;
	Common::Array<Topping> _toppings;
	Common::Point _dragOffset;
	int _dragCoin = -1;
	int _level = 1, _dataset = 0, _set = 1, _current = 29, _price = 0, _total = 0, _wrong = 0;
	int _itemDatasets[30] = {}, _supply[4] = {}, _placed[30] = {}, _columns[5] = {};
	int _columnHeights[5] = {};
	uint32 _unlocked = 0, _bunStart = 0;
	int _bunY = 0, _bunTarget = 0;
	bool _selected = false, _totalVisible = false, _showMarble = false;
	bool _shortHelp = false, _excessHelp = false, _firstTopping = false, _ringHelp = false;
	uint _mainClicks = 0, _clawClicks = 0;
	Phase _phase = kReady;
	int number(const Common::String &key) const;
	int indexed(const char *key, int index) const;
	Shell::Sprite *sprite(const char *resource, const char *depth);
	Shell::TextObject *text(int x, int y, int width = 0, int height = 0);
	void say(const char *actor, const Common::String &key);
	void sayPrice(int price);
	void sound(const char *key);
	bool loadNames();
	bool readData();
	bool showItem();
	bool startPayment();
	void clearCoins();
	void layoutCoins();
	void refreshTotal(bool reveal);
	void ringBell();
	void wrongPrice();
	void unlock();
	void assess();
	void finishSandwich();
	void award();
	void newSandwich();
	void help(bool paige);
	void drop(Common::Point point);
	void persist();
	Common::Rect counter(int type) const;
	static Common::String priceText(int price);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
