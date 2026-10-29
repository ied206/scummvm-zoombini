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

#include "trinket/rr1cloud9/la1_gumball.h"

#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/shell/text.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

constexpr const char *Gumball::kGumActor;
constexpr const char *Gumball::kSamName;
constexpr const char *Gumball::kReaderName;
constexpr const char *Gumball::kNameUnsignedIndexFormat;
constexpr const char *Gumball::kPartsListIDKey;
constexpr const char *Gumball::kWordsListIDKey;
constexpr const char *Gumball::kTLC18FontKey;
constexpr const char *Gumball::kR1LA1LevelKey;
constexpr const char *Gumball::kR1LA1DatasetKey;
constexpr const char *Gumball::kR1LA1StateKey;
constexpr const char *Gumball::kR1LA1CCountKey;
constexpr const char *Gumball::kR1LA1TCCountKey;
constexpr const char *Gumball::kGumKey;
constexpr const char *Gumball::kMethodSetSpeechAnims;
constexpr const char *Gumball::kGumLipsIDKey;
constexpr const char *Gumball::kGumSetIDKey;
constexpr const char *Gumball::kMethodAddFidgetGroup;
constexpr const char *Gumball::kFidgetMinTime2Key;
constexpr const char *Gumball::kFidgetMaxTime2Key;
constexpr const char *Gumball::kMethodAddFidget;
constexpr const char *Gumball::kGumFidg1IDKey;
constexpr const char *Gumball::kTokenReleaseIDKey;
constexpr const char *Gumball::kTokenReleaseZKey;
constexpr const char *Gumball::kTokenIDKey;
constexpr const char *Gumball::kTokenZKey;
constexpr const char *Gumball::kSliderInIDKey;
constexpr const char *Gumball::kSliderSettleZKey;
constexpr const char *Gumball::kSliderZKey;
constexpr const char *Gumball::kSliderOutIDKey;
constexpr const char *Gumball::kWhistleSparkleIDKey;
constexpr const char *Gumball::kItemZKey;
constexpr const char *Gumball::kWhistleRollIDKey;
constexpr const char *Gumball::kMarbleIDKey;
constexpr const char *Gumball::kCandyRollIDKey;
constexpr const char *Gumball::kBlowWhistleIDKey;
constexpr const char *Gumball::kSamAnimZKey;
constexpr const char *Gumball::kEatCandyIDKey;
constexpr const char *Gumball::kMarbleRollIDKey;
constexpr const char *Gumball::kTokenReleaseXKey;
constexpr const char *Gumball::kTokenReleaseClosedKey;
constexpr const char *Gumball::kTokenXKey;
constexpr const char *Gumball::kWhistleSettleFrameKey;
constexpr const char *Gumball::kCandySettleFrameKey;
constexpr const char *Gumball::kMarbleXKey;
constexpr const char *Gumball::kMarbleYKey;
constexpr const char *Gumball::kR1LA1ShowMarbleKey;
constexpr const char *Gumball::kFirstRampYKey;
constexpr const char *Gumball::kBValidRampUnsignedIndexFormat;
constexpr const char *Gumball::kRampRowsKey;
constexpr const char *Gumball::kRampRowChangeSignedIndexKey;
constexpr const char *Gumball::kRampDeltaYKey;
constexpr const char *Gumball::kRampIDKey;
constexpr const char *Gumball::kRampZKey;
constexpr const char *Gumball::kRampXValuesSignedIndexKey;
constexpr const char *Gumball::kCorrTokenZKey;
constexpr const char *Gumball::kCorrTokenXUnsignedIndexKey;
constexpr const char *Gumball::kCorrTokenYKey;
constexpr const char *Gumball::kInstructionSpchIDs14Key;
constexpr const char *Gumball::kInstructionSpchIDs15Key;
constexpr const char *Gumball::kRsName;
constexpr const char *Gumball::kEnterSpchIDsKey;
constexpr const char *Gumball::kSgrsName;
constexpr const char *Gumball::kReEnterSpchIDs2Key;
constexpr const char *Gumball::kRsggName;
constexpr const char *Gumball::kReEnterSpchIDs3Key;
constexpr const char *Gumball::kRgName;
constexpr const char *Gumball::kReEnterSpchIDs41Key;
constexpr const char *Gumball::kGgName;
constexpr const char *Gumball::kReEnterSpchIDs43Key;
constexpr const char *Gumball::kSggName;
constexpr const char *Gumball::kReEnterSpchIDs44Key;
constexpr const char *Gumball::kMaxDataSetsSignedIndexKey;
constexpr const char *Gumball::kNumSetsKey;
constexpr const char *Gumball::kRightFixedRampSignedIndexUnsignedIndexKey;
constexpr const char *Gumball::kLeftFixedRampSignedIndexUnsignedIndexKey;
constexpr const char *Gumball::kFixedRamp3SignedIndexSignedIndexKey;
constexpr const char *Gumball::kRampRightKey;
constexpr const char *Gumball::kRampLeftKey;
constexpr const char *Gumball::kTokenXValuesUnsignedIndexKey;
constexpr const char *Gumball::kTokenYValuesUnsignedIndexKey;
constexpr const char *Gumball::kPropertyColor;
constexpr const char *Gumball::kPropertyDataSetLevelSignedIndexDataSetSignedIndexSetSignedIndex;
constexpr const char *Gumball::kWordsXUnsignedIndexKey;
constexpr const char *Gumball::kWordsYKey;
constexpr const char *Gumball::kPartsXStartKey;
constexpr const char *Gumball::kPartsYKey;
constexpr const char *Gumball::kInitSentenceSpchIDKey;
constexpr const char *Gumball::kWordBlankSFXKey;
constexpr const char *Gumball::kIdleSpchIDsSignedIndexKey;
constexpr const char *Gumball::kIdleSpchIDsSignedIndex1Key;
constexpr const char *Gumball::kIdleSpchIDsSignedIndex2Key;
constexpr const char *Gumball::kR1LA1HistoryCountKey;
constexpr const char *Gumball::kR1LA1HistorySignedIndexKey;
constexpr const char *Gumball::kR1LA1Key;
constexpr const char *Gumball::kR1LA1FixedLevelKey;
constexpr const char *Gumball::kSliderSFXKey;
constexpr const char *Gumball::kTokenReleaseOpenKey;
constexpr const char *Gumball::kReleaseTokenSFXKey;
constexpr const char *Gumball::kTokenBumpSFXKey;
constexpr const char *Gumball::kCorrectSpchIDsUnsignedIndexKey;
constexpr const char *Gumball::kWrongSpchIDsUnsignedIndexKey;
constexpr const char *Gumball::kPaigeName;
constexpr const char *Gumball::kPaigeSpchIDs1Key;
constexpr const char *Gumball::kAutoHelpIDsUnsignedIndexKey;
constexpr const char *Gumball::kOopsSpchIDsUnsignedIndexKey;
constexpr const char *Gumball::kCorrectSFXKey;
constexpr const char *Gumball::kWrongSFXKey;
constexpr const char *Gumball::kGrgName;
constexpr const char *Gumball::kInstructionSpchIDs1Key;
constexpr const char *Gumball::kTokenReleaseHighlightKey;
constexpr const char *Gumball::kInstructionSpchIDs16Key;
constexpr const char *Gumball::kTokenRollSFXKey;
constexpr const char *Gumball::kPathDelayKey;
constexpr const char *Gumball::kPathDXKey;
constexpr const char *Gumball::kPathDYKey;
constexpr const char *Gumball::kGumCycleSFXKey;
constexpr const char *Gumball::kGotWhistleSpchIDKey;
constexpr const char *Gumball::kMarbleSFXKey;
constexpr const char *Gumball::kGotMarbleSpchIDsSignedIndexKey;
constexpr const char *Gumball::kRrName;
constexpr const char *Gumball::kPutAwaySpchIDsKey;
constexpr const char *Gumball::kEatCandySpchIDsSignedIndexKey;
constexpr const char *Gumball::kEatCandySpchIDsUnsignedIndexKey;
constexpr const char *Gumball::kEatCandySpchIDs7Key;
constexpr const char *Gumball::kEatCandySpchIDs8Key;
constexpr const char *Gumball::kEatCandySpchIDs9Key;
constexpr const char *Gumball::kCOMPickUpSfxIDKey;
constexpr const char *Gumball::kINV1WhistleKey;
constexpr const char *Gumball::kINV1Marble4Key;
constexpr const char *Gumball::kItemUnsignedIndexFormat;
constexpr const char *Gumball::kPutAwaySpchIDs2Key;
constexpr const char *Gumball::kInstructionSpchIDs3SignedIndexKey;
constexpr const char *Gumball::kInstructionSpchIDs2SignedIndexKey;
constexpr const char *Gumball::kInstructionSpchIDs24Key;
constexpr const char *Gumball::kInstructionSpchIDs25Key;
constexpr const char *Gumball::kInstructionSpchIDs26Key;
constexpr const char *Gumball::kPaigeSpchIDs2Key;
constexpr const char *Gumball::kPaigeSpchIDs3Key;
constexpr const char *Gumball::kPaigeSpchIDs4Key;
constexpr const char *Gumball::kPaigeSpchIDsSignedIndexKey;
constexpr const char *Gumball::kPaigeSpchIDs11Key;
constexpr const char *Gumball::kPaigeSpchState4IDsSignedIndexKey;
constexpr const char *Gumball::kRampLockedSFXKey;
constexpr const char *Gumball::kPaigeSpchIDs13Key;
constexpr const char *Gumball::kRampFlipSFXKey;
constexpr const char *Gumball::kGumHSX1Key;
constexpr const char *Gumball::kGumHSY1Key;
constexpr const char *Gumball::kGumHSX2Key;
constexpr const char *Gumball::kGumHSY2Key;
constexpr const char *Gumball::kMainSpchIDs3Key;
constexpr const char *Gumball::kEnterSpchIDs3Key;
constexpr const char *Gumball::kMainSpchIDsSignedIndexKey;
constexpr const char *Gumball::kIdleSpchIDs1Key;
constexpr const char *Gumball::kMainSpchIDs4Key;
constexpr const char *Gumball::kMainSpchIDs5Key;
constexpr const char *Gumball::kRampRHighlightKey;
constexpr const char *Gumball::kRampLHighlightKey;

constexpr const char *Gumball::kTextFormat;
constexpr const char *Gumball::kFontFormat;
constexpr const char *Gumball::kDataXML;

Gumball::Gumball(PageScene &host) : _host(host), _paths(host._vm.resources()) {}

Gumball::~Gumball() {
	clearText();
}

int Gumball::number(const Common::String &key) const {
	return _host._page->number(key);
}

Shell::Sprite *Gumball::sprite(const char *resource, const char *depth) {
	Shell::Sprite *result = _host._scene.add(_host.characterResource(number(resource), Shell::Character::Resources::kAnimation), number(depth));
	if (result)
		_sprites.push_back(result);
	return result;
}

void Gumball::sound(const char *key) {
	_host.playSounds({static_cast<uint16>(number(key))});
}

void Gumball::say(const char *actor, const Common::String &key) {
	_host.delaySpeech(1);
	_host.say(actor, number(key));
	_host.delaySpeech(1);
}

void Gumball::group(const char *actors, const Common::String &key) {
	for (uint i = 0; actors[i]; i++) {
		const char *actor = kGumActor;
		if (actors[i] == 's')
			actor = kSamName;
		else if (actors[i] == 'r')
			actor = kReaderName;
		say(actor, Common::String::format(kNameUnsignedIndexFormat, key.c_str(), i + 1));
	}
}

bool Gumball::loadList(uint resource, Common::StringArray &list) {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_host._vm.resources().open(Common::String::format(kTextFormat, resource)));
	if (!stream || 65536 < stream->size())
		return false;
	Common::String line;
	while (stream->pos() < stream->size()) {
		char c = stream->readByte();
		if (line.empty() && c == '*')
			return !list.empty();
		if (c == '\r') {
			uint comma = line.find(',');
			if (comma != Common::String::npos)
				line = line.substr(0, comma);
			list.push_back(line);
			line.clear();
		} else if (c && c != '\n') {
			line += c;
		}
	}
	return false;
}

int Gumball::wordIndex(const Common::StringArray &list, Common::String word) {
	while (!word.empty() && !Common::isAlnum(word.lastChar()))
		word.deleteLastChar();
	for (uint i = 0; i < list.size(); i++) {
		if (word.equalsIgnoreCase(list[i]))
			return i;
	}
	return -1;
}

uint Gumball::wordSpeech(const Common::String &word, bool part) const {
	int index = wordIndex(part ? _partsList : _wordsList, word);
	if (index < 0)
		return 0;
	const char *key;
	if (part)
		key = kPartsListIDKey;
	else
		key = kWordsListIDKey;
	return number(key) + index;
}

bool Gumball::enter() {
	_font = _host._vm.fonts().load(Common::String::format(kFontFormat, number(kTLC18FontKey)));
	if (!_font || !_data.loadStream(_host._vm.resources().open(kDataXML)) || !_data.parse() ||
		!loadList(number(kWordsListIDKey), _wordsList) || !loadList(number(kPartsListIDKey), _partsList))
		return false;
	_level = CLIP(_host._world.get(kR1LA1LevelKey), 1, 3);
	_dataset = _host._world.get(kR1LA1DatasetKey);
	_state = MAX(1, _host._world.get(kR1LA1StateKey));
	_correct = _host._world.get(kR1LA1CCountKey);
	_total = _host._world.get(kR1LA1TCCountKey);
	if (_host._practice) {
		_state = 2;
		_correct = _total = 0;
		if (_host._chester)
			_host._chester->visible = false;
		if (!_dataset)
			_dataset = 1;
	} else if (!_dataset) {
		randomDataset();
	}
	if (!_host.actor(kGumActor, kGumKey, false))
		return false;
	Core::ScriptValue result;
	_host._actors[kGumActor]->play(kMethodSetSpeechAnims, {Core::ScriptValue(number(kGumLipsIDKey)), Core::ScriptValue(1), Core::ScriptValue(1),
						 Core::ScriptValue(number(kGumSetIDKey)), Core::ScriptValue(0), Core::ScriptValue(0)}, result);
	_host._actors[kGumActor]->play(kMethodAddFidgetGroup, {Core::ScriptValue(2), Core::ScriptValue(number(kFidgetMinTime2Key)),
		Core::ScriptValue(number(kFidgetMaxTime2Key))}, result);
	_host._actors[kGumActor]->play(kMethodAddFidget, {Core::ScriptValue(2), Core::ScriptValue(number(kGumFidg1IDKey))}, result);
	_release = sprite(kTokenReleaseIDKey, kTokenReleaseZKey);
	_token = sprite(kTokenIDKey, kTokenZKey);
	_movingToken = sprite(kTokenIDKey, kTokenZKey);
	_tray = sprite(kSliderInIDKey, kSliderSettleZKey);
	_trayIn = sprite(kSliderInIDKey, kSliderZKey);
	_trayOut = sprite(kSliderOutIDKey, kSliderZKey);
	_sparkle = sprite(kWhistleSparkleIDKey, kItemZKey);
	_whistle = sprite(kWhistleRollIDKey, kItemZKey);
	_marble = sprite(kMarbleIDKey, kItemZKey);
	_candy = sprite(kCandyRollIDKey, kItemZKey);
	_blow = sprite(kBlowWhistleIDKey, kSamAnimZKey);
	_eat = sprite(kEatCandyIDKey, kSamAnimZKey);
	_rolls[kWhistle] = sprite(kWhistleRollIDKey, kItemZKey);
	_rolls[kMarble] = sprite(kMarbleRollIDKey, kItemZKey);
	_rolls[kCandy] = sprite(kCandyRollIDKey, kItemZKey);
	if (_sprites.size() != 15)
		return false;
	for (auto *item : _sprites)
		item->visible = false;
	_release->visible = true;
	_release->x = number(kTokenReleaseXKey);
	_release->setFrame(number(kTokenReleaseClosedKey));
	_token->x = number(kTokenXKey);
	_whistle->setFrame(number(kWhistleSettleFrameKey));
	_candy->setFrame(number(kCandySettleFrameKey));
	_marble->colorRemap = PageScene::marbleColor(3);
	_rolls[kMarble]->colorRemap = PageScene::marbleColor(3);
	_marble->x = number(kMarbleXKey);
	_marble->y = number(kMarbleYKey);
	_host._scene.setDepth(_marble, number(kItemZKey) + 1);
	_sparkle->setFrame(MIN(3, _total + 1));
	_sparkle->visible = !_host._practice && _state < 3;
	_whistle->visible = !_host._practice && _state == 3;
	_marble->visible = !_host._practice && _host._world.get(kR1LA1ShowMarbleKey);
	int j = 1;
	int y = number(kFirstRampYKey);
	for (uint node = 1; node <= 22; node++) {
		if (!number(Common::String::format(kBValidRampUnsignedIndexFormat, node)))
			continue;
		for (int row = 1; row <= number(kRampRowsKey); row++) {
			if (j == number(Common::String::format(kRampRowChangeSignedIndexKey, row)))
				y += number(kRampDeltaYKey);
		}
		Shell::Sprite *ramp = sprite(kRampIDKey, kRampZKey);
		if (!ramp)
			return false;
		ramp->x = number(Common::String::format(kRampXValuesSignedIndexKey, j));
		ramp->y = y;
		_ramps[node].sprite = ramp;
		j += 1;
	}
	for (uint i = 0; i < 3; i++) {
		_correctTokens[i] = sprite(kTokenIDKey, kCorrTokenZKey);
		if (!_correctTokens[i])
			return false;
		_correctTokens[i]->x = number(Common::String::format(kCorrTokenXUnsignedIndexKey, i + 1));
		_correctTokens[i]->y = number(kCorrTokenYKey);
		_correctTokens[i]->visible = false;
	}
	setupRamps();
	if (!puzzle())
		return false;
	if (_host._practice) {
		say(kGumActor, kInstructionSpchIDs14Key);
		say(kReaderName, kInstructionSpchIDs15Key);
	} else if (_state == 1) {
		group(kRsName, kEnterSpchIDsKey);
	} else if (_state == 2) {
		group(kSgrsName, kReEnterSpchIDs2Key);
	} else if (_state == 3) {
		group(kRsggName, kReEnterSpchIDs3Key);
	} else {
		int choice = _host._vm._rnd->getRandomNumberRng(1, 3);
		if (choice == 1)
			group(kRgName, kReEnterSpchIDs41Key);
		else if (choice == 2)
			group(kGgName, kReEnterSpchIDs43Key);
		else
			group(kSggName, kReEnterSpchIDs44Key);
	}
	persist();
	return true;
}

void Gumball::randomDataset() {
	_dataset = _host._vm._rnd->getRandomNumberRng(1, number(Common::String::format(kMaxDataSetsSignedIndexKey, _level)));
	_set = _host._vm._rnd->getRandomNumberRng(1, number(kNumSetsKey));
}

void Gumball::setupRamps() {
	for (uint i = 1; i <= 22; i++) {
		Ramp &ramp = _ramps[i];
		if (!ramp.sprite)
			continue;
		ramp.right = _host._vm._rnd->getRandomNumberRng(1, 2) == 2;
		ramp.locked = false;
		if (_level < 3) {
			for (uint j = 1; j <= 2; j++) {
				if (number(Common::String::format(kRightFixedRampSignedIndexUnsignedIndexKey, _level, j)) == static_cast<int>(i)) {
					ramp.locked = true;
					ramp.right = true;
				}
				if (number(Common::String::format(kLeftFixedRampSignedIndexUnsignedIndexKey, _level, j)) == static_cast<int>(i)) {
					ramp.locked = true;
					ramp.right = false;
				}
			}
		}
	}
	if (_level == 3) {
		// Keep the previous candidate index when changing to the lower ramp group.
		int previous = 0;
		for (uint j = 1; j <= 3; j++) {
			int candidate = 1 + _host._vm._rnd->getRandomNumberExcluding(j < 3 ? 5 : 4, previous - 1);
			int node = number(Common::String::format(kFixedRamp3SignedIndexSignedIndexKey, j < 3 ? 1 : 2, candidate));
			Ramp &ramp = _ramps[node];
			ramp.locked = true;
			ramp.right = _host._vm._rnd->getRandomNumberRng(1, 2) == 2;
			previous = candidate;
			if (node == 8)
				ramp.right = true;
			if (node == 9)
				ramp.right = false;
		}
	}
	rampFrames();
}

void Gumball::rampFrames() {
	bool rolling = _phase == kRelease || _phase == kBump || _phase == kRolling;
	for (auto &ramp : _ramps) {
		if (ramp.sprite)
			ramp.sprite->setFrame((ramp.right ? number(kRampRightKey) : number(kRampLeftKey)) + (ramp.locked || rolling ? 2 : 0));
	}
}

void Gumball::buildPath() {
	static const uint edges[18][2] = {
		{0, 0}, {2, 3}, {4, 5}, {5, 6}, {7, 8}, {8, 9}, {9, 10}, {14, 14}, {11, 12}, {12, 13}, {17, 17}, {14, 15}, {15, 16}, {16, 17}, {18, 18}, {19, 20}, {20, 21}, {22, 22}};
	_path.clear();
	uint node = 1;
	_path.push_back(node);
	while (node < 18) {
		node = edges[node][_ramps[node].right ? 1 : 0];
		_path.push_back(node);
	}
}

Common::Point Gumball::tokenPoint(uint node) const {
	return Common::Point(number(Common::String::format(kTokenXValuesUnsignedIndexKey, node)), number(Common::String::format(kTokenYValuesUnsignedIndexKey, node)));
}

void Gumball::clearText() {
	_readings.clear();
	for (auto *item : _text)
		delete item;
	_text.clear();
	_textSpeech.clear();
}

Shell::TextObject *Gumball::text(const Common::String &value, int x, int y, bool centered, uint speech) {
	Shell::TextObject *item = new Shell::TextObject(_paths, _host._vm.fonts(), _font);
	item->create({Core::ScriptValue::fromText(value), Core::ScriptValue(x), Core::ScriptValue(y), Core::ScriptValue(centered ? 1 : 0)});
	item->set(kPropertyColor, {Core::ScriptValue(6317), Core::ScriptValue(4212)}, Core::ScriptValue(2106));
	_text.push_back(item);
	_textSpeech.push_back(speech);
	return item;
}

bool Gumball::puzzle() {
	Common::String record = _data.get(Common::String::format(kPropertyDataSetLevelSignedIndexDataSetSignedIndexSetSignedIndex, _level, _dataset, _set));
	Common::String values[4];
	uint offset = 0;
	for (auto &value : values) {
		uint begin = record.find('"', offset);
		if (begin == Common::String::npos)
			return false;
		uint end = record.find('"', begin + 1);
		if (end == Common::String::npos)
			return false;
		value = record.substr(begin + 1, end - begin - 1);
		offset = end + 1;
	}
	uint blank = values[0].find('_');
	if (blank == Common::String::npos)
		return false;
	_parts[0] = values[0].substr(0, blank);
	_parts[0].trim();
	_parts[1] = " ________  ";
	_parts[2] = values[0].substr(blank + 1);
	_parts[2].trim();
	for (uint i = 0; i < 3; i++)
		_order[i] = i + 1;
	// The authored three-word permutation swaps exactly one pair.
	int first = _host._vm._rnd->getRandomNumberRng(1, 3) - 1;
	int second = _host._vm._rnd->getRandomNumberExcluding(3, first);
	SWAP(_order[first], _order[second]);
	for (uint i = 0; i < 3; i++) {
		_words[i] = values[_order[i]];
		if (!wordSpeech(_words[i], false))
			return false;
	}
	_imperfect = false;
	_chosenSlot = 0;
	sentence();
	return true;
}

void Gumball::sentence(int slot) {
	clearText();
	for (uint i = 0; i < 3; i++)
		text(_words[i], number(Common::String::format(kWordsXUnsignedIndexKey, i + 1)), number(kWordsYKey), true, wordSpeech(_words[i], false));
	Common::String blank = _parts[1];
	if (slot) {
		blank = " _";
		int spaces = Shell::Text::inkBounds(*_font, "  ").width() + 2;
		while (Shell::Text::inkBounds(*_font, blank).width() + 2 + spaces < Shell::Text::inkBounds(*_font, _words[slot - 1]).width() + 2)
			blank += '_';
		blank += "_  ";
	}
	int x = number(kPartsXStartKey);
	int y = number(kPartsYKey);
	for (uint i = 0; i < 3; i++) {
		Shell::TextObject *part = text(i == 1 ? blank : _parts[i], x, y, false, wordSpeech(_parts[i], true));
		if (i == 1 && slot) {
			int middleX = x + part->width() / 2;
			Shell::TextObject *word = text(_words[slot - 1], middleX, y + part->height() / 2, true, wordSpeech(_words[slot - 1], false));
			x = middleX + word->width() + 6;
		} else {
			x += part->width();
		}
	}
}

void Gumball::readSentence(int slot) {
	_readings.clear();
	if (slot && _order[slot - 1] == 1) {
		int offset = (_dataset - 1) * 3 + _set - 1;
		for (int i = 1; i < _level; i++)
			offset += 3 * number(Common::String::format(kMaxDataSetsSignedIndexKey, i));
		readText(number(kInitSentenceSpchIDKey) + offset, ((1U << _text.size()) - 1) & ~7U);
		return;
	}
	for (uint i = 0; i < 3; i++) {
		if (i != 1) {
			if (!_parts[i].equals("."))
				readText(wordSpeech(_parts[i], true), 1U << (3 + i + (slot && i == 2 ? 1 : 0)));
		} else if (slot) {
			readText(wordSpeech(_words[slot - 1], false), 1U << 5);
		} else {
			readText(number(kWordBlankSFXKey), 1U << 4, true);
		}
	}
}

void Gumball::readText(uint resource, uint mask, bool sound) {
	if (!resource)
		return;
	uint first = _host._speech.size();
	if (sound) {
		_host.queueSound(resource);
	} else {
		_host.delaySpeech(1);
		_host.say(kGumActor, resource);
		_host.delaySpeech(1);
	}
	TextReading reading = {first, _host._speech.size(), mask};
	_readings.push_back(reading);
}

uint Gumball::highlightedText() const {
	if (!_host._speechDelayStarted && !_host._speaking && !_host._queuedSound)
		return 0;
	uint index = _host._speechDelayStarted ? _host._nextSpeech : _host._nextSpeech - 1;
	for (const auto &reading : _readings) {
		if (reading.first <= index && index < reading.end)
			return reading.mask;
	}
	return 0;
}

void Gumball::idle(uint32 time) {
	if (_host._practice || _host._chestOpen || _host._putAwayPending || _host._putAwayActive)
		return;
	if (!_host._idleWaiting) {
		_host._idleWaiting = true;
		_host._idleStart = time;
	}
	if (time - _host._idleStart < 30000)
		return;
	_host._idleWaiting = false;
	int choice = _host._vm._rnd->getRandomNumberRng(1, 4);
	if (choice <= 2) {
		_host.say(choice == 1 ? kReaderName : kSamName, number(Common::String::format(kIdleSpchIDsSignedIndexKey, choice)));
	} else {
		_host.say(choice == 3 ? kSamName : kReaderName, number(Common::String::format(kIdleSpchIDsSignedIndex1Key, choice + 1)));
		_host.say(choice == 3 ? kReaderName : kSamName, number(Common::String::format(kIdleSpchIDsSignedIndex2Key, choice + 1)));
	}
}

void Gumball::persist() {
	if (_host._practice)
		return;
	_host._world.set(kR1LA1StateKey, _state);
	_host._world.set(kR1LA1CCountKey, _correct);
	_host._world.set(kR1LA1TCCountKey, _total);
	_host._world.set(kR1LA1LevelKey, _level);
	_host._world.set(kR1LA1DatasetKey, _dataset);
}

void Gumball::assess() {
	if (_host._practice)
		return;
	World &world = _host._world;
	int count = CLIP(world.get(kR1LA1HistoryCountKey), 0, 20);
	if (count == 20) {
		for (int i = 0; i < 19; i++)
			world.set(Common::String::format(kR1LA1HistorySignedIndexKey, i), world.get(Common::String::format(kR1LA1HistorySignedIndexKey, i + 1)));
		count = 19;
	}
	world.set(Common::String::format(kR1LA1HistorySignedIndexKey, count), _imperfect ? 3 : 1);
	count += 1;
	world.set(kR1LA1HistoryCountKey, count);
	int clean = 0;
	int imperfect = 0;
	for (int i = count - 1; 0 <= i; i--) {
		int outcome = world.get(Common::String::format(kR1LA1HistorySignedIndexKey, i));
		if (outcome == 1) {
			clean += 1;
			imperfect = 0;
		} else if (outcome == 3) {
			imperfect += 1;
			clean = 0;
		} else if (outcome == 2) {
			clean = imperfect = 0;
		}
		if (3 <= clean || 5 <= imperfect)
			break;
	}
	int previousLevel = _level;
	world.recordAssessment(kR1LA1Key, _level, _imperfect ? 3 : 1, 3, 5, MAX(clean, imperfect));
	if (3 <= clean || 5 <= imperfect) {
		world.set(kR1LA1HistoryCountKey, 0);
		if (!world.get(kR1LA1FixedLevelKey))
			_level = CLIP(_level + (3 <= clean ? 1 : -1), 1, 3);
	}
	if (previousLevel != _level)
		randomDataset();
	else {
		_dataset += 1;
		if (number(Common::String::format(kMaxDataSetsSignedIndexKey, _level)) < _dataset) {
			_dataset = 1;
			_set = _set % number(kNumSetsKey) + 1;
		}
	}
	persist();
}

void Gumball::sliderOut() {
	_trayOut->visible = true;
	_trayOut->startAnimation(1);
	sound(kSliderSFXKey);
	_phase = kTrayOut;
}

void Gumball::releaseToken() {
	_rampClicks = 0;
	_tokenClicked = true;
	_phase = kRelease;
	rampFrames();
	buildPath();
	_release->setFrame(number(kTokenReleaseOpenKey));
	_token->visible = false;
	sound(kReleaseTokenSFXKey);
	_deadline = _host._time + 300;
}

void Gumball::checkAnswer() {
	_phase = kAnswer;
	rampFrames();
	_release->setFrame(number(kTokenReleaseClosedKey));
	_movingToken->visible = false;
	_movingToken->stopAnimation();
	_rollingSound.reset();
	sound(kTokenBumpSFXKey);
	uint location = _path.back();
	_chosenSlot = 18 < location && location < 22 ? location - 18 : 0;
	_answerCorrect = _chosenSlot && _order[_chosenSlot - 1] == 1;
	if (_answerCorrect) {
		_total += 1;
		_correct += 1;
		_correctTokens[_chosenSlot - 1]->visible = true;
		_previousSlot = 0;
		_reward = kCandy;
		if (!_host._practice && _correct == 3) {
			_reward = _total == 3 ? kWhistle : kMarble;
			_correct = 0;
			if (_reward == kWhistle)
				_state = 3;
			else
				_host._world.set(kR1LA1ShowMarbleKey, 1);
		}
		sentence(_chosenSlot);
		readSentence(_chosenSlot);
		uint choice = _host.chooseAction(_correctChoices, 6);
		say(kGumActor, Common::String::format(kCorrectSpchIDsUnsignedIndexKey, choice));
		persist();
		// Commit the next question with the award, so a save during feedback cannot
		// replay a solved round while retaining its reward.
		assess();
	} else {
		_imperfect = true;
		_token->visible = true;
		if (_chosenSlot) {
			_wrong += 1;
			sentence(_chosenSlot);
			readSentence(_chosenSlot);
			uint choice = _host.chooseAction(_wrongChoices, 6);
			static constexpr const char *speakers[] = {
				kGumActor,
				kSamName,
				kReaderName,
				kGumActor,
				kSamName,
				kReaderName,
			};
			say(speakers[choice - 1], Common::String::format(kWrongSpchIDsUnsignedIndexKey, choice));
			if (_previousSlot == _chosenSlot)
				say(kPaigeName, kPaigeSpchIDs1Key);
			_previousSlot = _chosenSlot;
			if (_wrong == 4 && !_gumClicks && !_paigeClicks) {
				uint help = _host.chooseAction(_helpChoices, 4);
				say(help == 1 ? kGumActor : kPaigeName, Common::String::format(kAutoHelpIDsUnsignedIndexKey, help));
				_wrong = 0;
			}
		} else {
			_missLeft = location == 18 ? _missLeft + 1 : 0;
			_missRight = location == 22 ? _missRight + 1 : 0;
			if (_missLeft == 2 || _missRight == 2) {
				say(kPaigeName, kPaigeSpchIDs1Key);
				_missLeft = _missRight = 0;
			}
			uint choice = _host.chooseAction(_oopsChoices, 5);
			say(choice % 2 ? kSamName : kReaderName, Common::String::format(kOopsSpchIDsUnsignedIndexKey, choice));
		}
	}
	if (_chosenSlot) {
		_missLeft = _missRight = 0;
		sound(_answerCorrect ? kCorrectSFXKey : kWrongSFXKey);
	}
}

void Gumball::dispense() {
	_sparkle->setFrame(MIN(3, _total + 1));
	if (_reward == kWhistle)
		_sparkle->visible = false;
	_rolls[_reward]->visible = true;
	_rolls[_reward]->startAnimation(1);
	_phase = kRewardRoll;
}

void Gumball::nextPuzzle() {
	if (_host._practice) {
		_dataset += 1;
		if (number(Common::String::format(kMaxDataSetsSignedIndexKey, _level)) < _dataset) {
			_dataset = 1;
			_set = _set % number(kNumSetsKey) + 1;
		}
	}
	setupRamps();
	if (!puzzle())
		error("Trinket: missing LA1 dataset %d/%d/%d", _level, _dataset, _set);
	sliderOut();
}

void Gumball::update(uint32 time) {
	if (_phase != kReady)
		_host._idleWaiting = false;
	if (_host._nextSpeech < _host._speech.size())
		return;
	switch (_phase) {
	case kIntro:
		if (_state == 1) {
			group(kGrgName, kInstructionSpchIDs1Key);
			_phase = kInstructions;
		} else {
			sliderOut();
		}
		break;
	case kInstructions:
		_state = 2;
		_finishInstrs = true;
		persist();
		sliderOut();
		break;
	case kTrayOut:
		if (_trayOut->isRunning())
			break;
		_trayOut->visible = false;
		_tray->visible = _showText = _token->visible = true;
		if (_finishInstrs) {
			_release->setFrame(number(kTokenReleaseHighlightKey));
			say(kGumActor, kInstructionSpchIDs14Key);
			_finishInstrs = false;
			_phase = kTokenExplain;
		} else {
			_phase = kReady;
		}
		break;
	case kTokenExplain:
		_blinkStep = 0;
		_deadline = time + 100;
		_phase = kTokenBlink;
		break;
	case kTokenBlink:
		if (static_cast<int32>(time - _deadline) < 0)
			break;
		_blinkStep += 1;
		_token->visible = _blinkStep % 2 == 0;
		_deadline = time + 100;
		if (_blinkStep == 6) {
			_release->setFrame(number(kTokenReleaseClosedKey));
			say(kReaderName, kInstructionSpchIDs15Key);
			say(kGumActor, kInstructionSpchIDs16Key);
			_phase = kReady;
		}
		break;
	case kRelease:
		if (static_cast<int32>(time - _deadline) < 0)
			break;
		sound(kTokenBumpSFXKey);
		_deadline = time + 300;
		_phase = kBump;
		break;
	case kBump:
		if (static_cast<int32>(time - _deadline) < 0)
			break;
		_rollingSound.reset(new OM::Sound(_host._vm.mixer()));
		_rollingSound->play(_host._vm.resources(), _host.characterResource(number(kTokenRollSFXKey), Shell::Character::Resources::kSound),
							Audio::Mixer::kSFXSoundType, 0);
		_segment = 1;
		_movingToken->x = tokenPoint(_path[0]).x;
		_movingToken->y = tokenPoint(_path[0]).y;
		_movingToken->visible = true;
		_movingToken->startAnimation();
		_deadline = time + number(kPathDelayKey);
		_phase = kRolling;
		break;
	case kRolling: {
		if (static_cast<int32>(time - _deadline) < 0)
			break;
		uint delay = number(kPathDelayKey);
		_deadline += delay;
		if (static_cast<int32>(time - _deadline - delay / 2) > 0)
			_deadline = time + delay;
		Common::Point from = tokenPoint(_path[_segment - 1]);
		Common::Point to = tokenPoint(_path[_segment]);
		int dx = 0;
		if (from.x < to.x)
			dx = number(kPathDXKey);
		else if (to.x < from.x)
			dx = -number(kPathDXKey);
		_movingToken->x += dx;
		_movingToken->y += number(kPathDYKey);
		bool reachedX = dx < 0 ? _movingToken->x <= to.x : to.x <= _movingToken->x;
		if (reachedX && to.y <= _movingToken->y) {
			_segment += 1;
			if (_path.size() <= _segment) {
				checkAnswer();
			} else {
				_movingToken->x = to.x;
				_movingToken->y = to.y;
			}
		}
		break;
	}
	case kAnswer:
		if (!_answerCorrect) {
			sentence();
			_phase = kReady;
		} else {
			for (auto *token : _correctTokens)
				token->visible = false;
			_tray->visible = _showText = false;
			_trayIn->visible = true;
			_trayIn->startAnimation(1);
			sound(kSliderSFXKey);
			_phase = kTrayIn;
		}
		break;
	case kTrayIn:
		if (!_trayIn->isRunning()) {
			_trayIn->visible = false;
			sound(kGumCycleSFXKey);
			dispense();
		}
		break;
	case kRewardRoll:
		if (_rolls[_reward]->isRunning())
			break;
		_rolls[_reward]->visible = false;
		if (_reward == kWhistle) {
			_whistle->visible = true;
			say(kSamName, kGotWhistleSpchIDKey);
		} else if (_reward == kMarble) {
			_marble->visible = true;
			sound(kMarbleSFXKey);
			_marbleSpeech = _marbleSpeech % 3 + 1;
			say(_marbleSpeech == 1 ? kReaderName : kSamName, Common::String::format(kGotMarbleSpchIDsSignedIndexKey, _marbleSpeech));
		} else {
			_candy->visible = true;
		}
		_phase = kRewardSpeech;
		break;
	case kRewardSpeech:
		nextPuzzle();
		break;
	case kPickup: {
		Shell::Sprite *animation = _pickup == kWhistle ? _blow : _eat;
		if (animation->isRunning())
			break;
		animation->visible = false;
		_host._actors[kSamName]->setVisible(true);
		if (_pickup == kWhistle) {
			group(kRrName, kPutAwaySpchIDsKey);
		} else {
			_candyClicks += 1;
			if (_candyClicks < 5) {
				say(kSamName, Common::String::format(kEatCandySpchIDsSignedIndexKey, _candyClicks));
			} else {
				uint choice = _host.chooseAction(_yuckChoices, 4);
				if (choice <= 2)
					say(kSamName, Common::String::format(kEatCandySpchIDsUnsignedIndexKey, choice + 4));
				else if (choice == 3) {
					say(kSamName, kEatCandySpchIDs7Key);
					say(kReaderName, kEatCandySpchIDs8Key);
				} else {
					say(kReaderName, kEatCandySpchIDs9Key);
				}
			}
		}
		_phase = kPutAway;
		break;
	}
	case kPutAway:
		if (_pickup != kCandy)
			_host.putAway();
		_phase = kReady;
		break;
	case kReady:
		idle(time);
		break;
	}
}

void Gumball::pickup(Reward item) {
	_pickup = item;
	sound(kCOMPickUpSfxIDKey);
	if (item == kCandy) {
		_candy->visible = false;
	} else {
		uint id = number(item == kWhistle ? kINV1WhistleKey : kINV1Marble4Key);
		Common::String key = Common::String::format(kItemUnsignedIndexFormat, id);
		_host._world.set(key, _host._world.get(key) + 1);
		if (item == kWhistle) {
			_state = 4;
			_whistle->visible = false;
		} else {
			_marble->visible = false;
			_host._world.set(kR1LA1ShowMarbleKey, 0);
			if (_host._world.get(key) == 1)
				say(kReaderName, kPutAwaySpchIDs2Key);
			_phase = kPutAway;
			return;
		}
		persist();
	}
	_host._actors[kSamName]->setVisible(false);
	Shell::Sprite *animation = item == kWhistle ? _blow : _eat;
	animation->visible = true;
	animation->startAnimation(1);
	_phase = kPickup;
}

void Gumball::gumHelp() {
	_gumClicks += 1;
	if (2 < _state) {
		say(kGumActor, Common::String::format(kInstructionSpchIDs3SignedIndexKey, _host._vm._rnd->getRandomNumberRng(1, 2)));
		return;
	}
	int help = 0;
	if (!_rampClicked)
		help = 1;
	else if (!_tokenClicked)
		help = 2;
	else if (!_total)
		help = 3;
	if (help)
		say(kGumActor, Common::String::format(kInstructionSpchIDs2SignedIndexKey, help));
	if (!_sentenceClicked)
		say(kGumActor, kInstructionSpchIDs24Key);
	else if (!help) {
		say(kGumActor, kInstructionSpchIDs25Key);
		say(kGumActor, kInstructionSpchIDs26Key);
	}
}

void Gumball::paigeHelp() {
	_paigeClicks += 1;
	if (_state == 2) {
		if (!_rampClicked) {
			say(kPaigeName, kPaigeSpchIDs2Key);
			say(kPaigeName, kPaigeSpchIDs3Key);
		} else if (!_tokenClicked) {
			say(kPaigeName, kPaigeSpchIDs4Key);
		} else {
			say(kPaigeName, Common::String::format(kPaigeSpchIDsSignedIndexKey, _paigeClicks + 4));
		}
	} else if (_state == 3) {
		say(kPaigeName, kPaigeSpchIDs11Key);
	} else {
		say(kPaigeName, Common::String::format(kPaigeSpchState4IDsSignedIndexKey, _paigeClicks));
	}
	if ((_level == 3 && _state != 3 ? 6 : 3) <= _paigeClicks)
		_paigeClicks = 0;
}

bool Gumball::click(Common::Point point) {
	if (_whistle->visible && _whistle->covers(point)) {
		pickup(kWhistle);
		return true;
	}
	if (_marble->visible && _marble->covers(point)) {
		pickup(kMarble);
		return true;
	}
	if (_candy->visible && _candy->covers(point)) {
		pickup(kCandy);
		return true;
	}
	if (Common::Rect(_release->x, 0, _release->x + _release->width(), _token->y + _token->height()).contains(point)) {
		releaseToken();
		return true;
	}
	for (auto &ramp : _ramps) {
		if (!ramp.sprite || !ramp.sprite->covers(point))
			continue;
		if (ramp.locked) {
			sound(kRampLockedSFXKey);
			say(kPaigeName, kPaigeSpchIDs13Key);
		} else {
			_rampClicked = true;
			_rampClicks += 1;
			ramp.right = !ramp.right;
			rampFrames();
			hover(point);
			sound(kRampFlipSFXKey);
			if (!_tokenClicked && _rampClicks == 3) {
				_rampClicks = 0;
				say(kPaigeName, kPaigeSpchIDs4Key);
			}
		}
		return true;
	}
	if (_showText) {
		for (uint i = 0; i < _text.size(); i++) {
			auto *item = _text[i];
			if (!Common::Rect(item->x, item->y, item->x + item->width(), item->y + item->height()).contains(point))
				continue;
			_wordClicks += 1;
			if (i < 3) {
				_readings.clear();
				readText(wordSpeech(_words[i], false), 1U << i);
			} else {
				_sentenceClicked = true;
				readSentence();
			}
			if (!_tokenClicked && _wordClicks == 3) {
				_wordClicks = 0;
				say(kPaigeName, kPaigeSpchIDs4Key);
			}
			return true;
		}
	}
	Common::Rect gum(number(kGumHSX1Key), number(kGumHSY1Key), number(kGumHSX2Key), number(kGumHSY2Key));
	if (gum.contains(point)) {
		gumHelp();
		return true;
	}
	if (_host._actors[kPaigeName]->bounds().contains(point)) {
		paigeHelp();
		return true;
	}
	if (!_host._practice && (_host._actors[kSamName]->bounds().contains(point) || _host._actors[kReaderName]->bounds().contains(point))) {
		if (_state == 3) {
			say(kSamName, kMainSpchIDs3Key);
		} else {
			int choice = _host._vm._rnd->getRandomNumberRng(1, 3);
			if (_state == 2) {
				if (choice == 1)
					say(kSamName, kEnterSpchIDs3Key);
				else
					say(choice == 2 ? kReaderName : kSamName, Common::String::format(kMainSpchIDsSignedIndexKey, choice - 1));
			} else if (choice == 2) {
				say(kReaderName, kIdleSpchIDs1Key);
			} else {
				say(kSamName, choice == 1 ? kMainSpchIDs4Key : kMainSpchIDs5Key);
			}
		}
		return true;
	}
	return false;
}

void Gumball::hover(Common::Point point) {
	if (busy())
		return;
	rampFrames();
	for (auto &ramp : _ramps) {
		if (ramp.sprite && !ramp.locked && ramp.sprite->covers(point))
			ramp.sprite->setFrame(number(ramp.right ? kRampRHighlightKey : kRampLHighlightKey));
	}
	_release->setFrame(number(_release->covers(point) ? kTokenReleaseHighlightKey : kTokenReleaseClosedKey));
}

void Gumball::draw(OM::Canvas &canvas) {
	if (!_showText)
		return;
	uint mask = highlightedText();
	for (uint i = 0; i < _text.size(); i++) {
		bool highlight = (mask & (1U << i)) != 0;
		_text[i]->set(kPropertyColor, {Core::ScriptValue(highlight ? 61440 : 6317), Core::ScriptValue(highlight ? 61440 : 4212)},
					  Core::ScriptValue(highlight ? 61440 : 2106));
		_text[i]->draw(canvas);
	}
}

void Gumball::skipSpeech() {
	if (_phase == kTokenExplain || _phase == kTokenBlink) {
		_host.interrupt();
		_token->visible = true;
		_release->setFrame(number(kTokenReleaseClosedKey));
		_phase = kReady;
		return;
	}
	if (_phase == kIntro || _phase == kInstructions || _phase == kAnswer || _phase == kRewardSpeech || _phase == kPutAway)
		_host.interrupt();
}

void Gumball::pause(bool paused) {
	if (paused) {
		_pausedSprites.clear();
		for (auto *item : _sprites) {
			if (item->isRunning()) {
				_pausedSprites.push_back(item);
				item->pauseAnimation(true);
			}
		}
	} else {
		// A selected static frame is paused by design, not by the map or overlay.
		for (auto *item : _pausedSprites)
			item->pauseAnimation(false);
		_pausedSprites.clear();
	}
	if (_rollingSound)
		_rollingSound->pause(paused);
}

} // namespace RR1Cloud9
} // namespace Trinket
