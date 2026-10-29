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

#include "trinket/rr1cloud9/la2_music_patterns.h"

#include "common/tokenizer.h"
#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

constexpr const char *MusicPatterns::kNameSignedIndexFormat;
constexpr const char *MusicPatterns::kUnsignedIndexFormat;
constexpr const char *MusicPatterns::kKNameCharIDsSignedIndexFormat;
constexpr const char *MusicPatterns::kKNameSpchIDsSignedIndexFormat;
constexpr const char *MusicPatterns::kPropertyGrid;
constexpr const char *MusicPatterns::kLocalKey;
constexpr const char *MusicPatterns::kPropertyZ;
constexpr const char *MusicPatterns::kGridZKey;
constexpr const char *MusicPatterns::kPropertyAnimateSettled;
constexpr const char *MusicPatterns::kR1LA2LevelKey;
constexpr const char *MusicPatterns::kR1LA2DatasetKey;
constexpr const char *MusicPatterns::kR1LA2CountKey;
constexpr const char *MusicPatterns::kR1LA2StateKey;
constexpr const char *MusicPatterns::kR1LA2PillowHereKey;
constexpr const char *MusicPatterns::kR1LA2MarbleSignedIndexCountKey;
constexpr const char *MusicPatterns::kDoorIDSignedIndexKey;
constexpr const char *MusicPatterns::kDoorZKey;
constexpr const char *MusicPatterns::kColumnLightsIDKey;
constexpr const char *MusicPatterns::kColumnLightXKey;
constexpr const char *MusicPatterns::kColumnLightDeltaXKey;
constexpr const char *MusicPatterns::kColumnLightYKey;
constexpr const char *MusicPatterns::kLockIDKey;
constexpr const char *MusicPatterns::kLockZKey;
constexpr const char *MusicPatterns::kGridXKey;
constexpr const char *MusicPatterns::kGridDeltaXKey;
constexpr const char *MusicPatterns::kGridYKey;
constexpr const char *MusicPatterns::kGridDeltaYKey;
constexpr const char *MusicPatterns::kButtonIDKey;
constexpr const char *MusicPatterns::kDoorLightsIDKey;
constexpr const char *MusicPatterns::kActivateDoorIDKey;
constexpr const char *MusicPatterns::kSpinIDKey;
constexpr const char *MusicPatterns::kSpinZKey;
constexpr const char *MusicPatterns::kPillowIDKey;
constexpr const char *MusicPatterns::kPillowZKey;
constexpr const char *MusicPatterns::kJewelIDKey;
constexpr const char *MusicPatterns::kJewelZKey;
constexpr const char *MusicPatterns::kSparkleIDKey;
constexpr const char *MusicPatterns::kTrayMaskIDKey;
constexpr const char *MusicPatterns::kTrayMaskZKey;
constexpr const char *MusicPatterns::kSparkleDeltaYKey;
constexpr const char *MusicPatterns::kJewelDeltaYKey;
constexpr const char *MusicPatterns::kReEnterSpchIDs41Key;
constexpr const char *MusicPatterns::kLocalSpchIDs411Key;
constexpr const char *MusicPatterns::kLocalName;
constexpr const char *MusicPatterns::kLocalCharIDs16Key;
constexpr const char *MusicPatterns::kLocalSpchIDs18Key;
constexpr const char *MusicPatterns::kMaxLightsKey;
constexpr const char *MusicPatterns::kReaderName;
constexpr const char *MusicPatterns::kReEnterSpchIDs31Key;
constexpr const char *MusicPatterns::kReEnterSpchIDsSignedIndexSignedIndexKey;
constexpr const char *MusicPatterns::kSamName;
constexpr const char *MusicPatterns::kReEnterSpchIDs42Key;
constexpr const char *MusicPatterns::kReEnterSpchIDs51Key;
constexpr const char *MusicPatterns::kReEnterSpchIDs61Key;
constexpr const char *MusicPatterns::kPropertyDataSetLevelSignedIndexWordListSignedIndexSet1;
constexpr const char *MusicPatterns::kNumGivenKey;
constexpr const char *MusicPatterns::kMarbleIDSignedIndexKey;
constexpr const char *MusicPatterns::kMarbleZKey;
constexpr const char *MusicPatterns::kMarbleDeltaZKey;
constexpr const char *MusicPatterns::kMarbleXKey;
constexpr const char *MusicPatterns::kMarbleDeltaXKey;
constexpr const char *MusicPatterns::kMarbleYKey;
constexpr const char *MusicPatterns::kMarbleDeltaYKey;
constexpr const char *MusicPatterns::kINV1Marble1Key;
constexpr const char *MusicPatterns::kItemUnsignedIndexFormat;
constexpr const char *MusicPatterns::kSceneSfxIDKey;
constexpr const char *MusicPatterns::kGoHomeSfxIDKey;
constexpr const char *MusicPatterns::kSnapSfxIDKey;
constexpr const char *MusicPatterns::kColumnLightBadIDKey;
constexpr const char *MusicPatterns::kMarbleSingFrameKey;
constexpr const char *MusicPatterns::kMarbleIDKey;
constexpr const char *MusicPatterns::kBlankSfxIDKey;
constexpr const char *MusicPatterns::kMarbleEyeCloseFrameKey;
constexpr const char *MusicPatterns::kWrongSpchIDsSignedIndexKey;
constexpr const char *MusicPatterns::kSceneCharIDs2Key;
constexpr const char *MusicPatterns::kSceneSpchIDs2Key;
constexpr const char *MusicPatterns::kNoMoveSpchIDSignedIndexKey;
constexpr const char *MusicPatterns::kR1LA2StreakKindKey;
constexpr const char *MusicPatterns::kR1LA2StreakKey;
constexpr const char *MusicPatterns::kAutolevelDownCountKey;
constexpr const char *MusicPatterns::kAutolevelUpCountKey;
constexpr const char *MusicPatterns::kR1LA2Key;
constexpr const char *MusicPatterns::kR1LA2FixedLevelKey;
constexpr const char *MusicPatterns::kHaveSpch11Key;
constexpr const char *MusicPatterns::kGChesterOpenIDName;
constexpr const char *MusicPatterns::kGClosedChesterZName;
constexpr const char *MusicPatterns::kItemSignedIndexKey;
constexpr const char *MusicPatterns::kHaveSpch21Key;
constexpr const char *MusicPatterns::kHaveSpch31Key;
constexpr const char *MusicPatterns::kHaveSpch22Key;
constexpr const char *MusicPatterns::kHaveSpch32Key;
constexpr const char *MusicPatterns::kSceneSpchIDs1SignedIndexKey;
constexpr const char *MusicPatterns::kEmptyGridSfxIDKey;
constexpr const char *MusicPatterns::kCongratsSpchIDSignedIndexKey;
constexpr const char *MusicPatterns::kDoorSfxID1Key;
constexpr const char *MusicPatterns::kDoorSfxID2Key;
constexpr const char *MusicPatterns::kDoorOpenName;
constexpr const char *MusicPatterns::kFanFareSfxIDKey;
constexpr const char *MusicPatterns::kLastDoorCharIDsKey;
constexpr const char *MusicPatterns::kLastDoorSpchIDsKey;
constexpr const char *MusicPatterns::kDoorOpenCharIDs4SignedIndexKey;
constexpr const char *MusicPatterns::kDoorOpenSpchIDs4Key;
constexpr const char *MusicPatterns::kNewPatternSfxIDKey;
constexpr const char *MusicPatterns::kIdleSignedIndexKey;
constexpr const char *MusicPatterns::kCharIDSignedIndexFormat;
constexpr const char *MusicPatterns::kSpchIDSignedIndexFormat;
constexpr const char *MusicPatterns::kPaigeName;
constexpr const char *MusicPatterns::kPaigeSpchIDs2SignedIndexKey;
constexpr const char *MusicPatterns::kPaigeSpchIDs3Key;
constexpr const char *MusicPatterns::kPaigeSpchIDs42Key;
constexpr const char *MusicPatterns::kPaigeSpchIDs411Key;
constexpr const char *MusicPatterns::kPaigeSpchIDs412Key;
constexpr const char *MusicPatterns::kPaigeSpchIDs413Key;
constexpr const char *MusicPatterns::kWrongSfxIDKey;
constexpr const char *MusicPatterns::kPlayName;
constexpr const char *MusicPatterns::kPickUpSfxIDKey;
constexpr const char *MusicPatterns::kCOMSoftClickSfxIDKey;
constexpr const char *MusicPatterns::kDoorClkSpchIDs4Key;
constexpr const char *MusicPatterns::kLocalSpchIDs31Key;
constexpr const char *MusicPatterns::kLocalSpchIDs32Key;
constexpr const char *MusicPatterns::kDoorClkName;
constexpr const char *MusicPatterns::kMaxDoorClkSpchKey;
constexpr const char *MusicPatterns::kINV1PillowKey;
constexpr const char *MusicPatterns::kJewelDropSfxIDKey;
constexpr const char *MusicPatterns::kPillowSpchIDKey;
constexpr const char *MusicPatterns::kLightSpchIDKey;
constexpr const char *MusicPatterns::kJewelName;
constexpr const char *MusicPatterns::kNumJewelSpchsKey;
constexpr const char *MusicPatterns::kSceneName;
constexpr const char *MusicPatterns::kLocalSpchIDs511Key;
constexpr const char *MusicPatterns::kLocalSpchIDs512Key;
constexpr const char *MusicPatterns::kLocalSpchIDs52SignedIndexKey;
constexpr const char *MusicPatterns::kSppName;
constexpr const char *MusicPatterns::kRgName;
constexpr const char *MusicPatterns::kRggName;
constexpr const char *MusicPatterns::kSgrName;
constexpr const char *MusicPatterns::kMainSpchIDsSignedIndexSignedIndexUnsignedIndexKey;

constexpr const char *MusicPatterns::kDataXML;

MusicPatterns::~MusicPatterns() {
	for (auto *note : _notes)
		delete note;
}

int MusicPatterns::number(const Common::String &key) const { return _host._page->number(key); }

int MusicPatterns::indexed(const char *key, int index) const { return number(Common::String::format(kNameSignedIndexFormat, key, index)); }

Shell::Sprite *MusicPatterns::sprite(const Common::String &key, int depth) {
	return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), depth);
}

void MusicPatterns::say(const char *actor, const Common::String &key) { speech(actor, number(key)); }

void MusicPatterns::speech(const Common::String &actor, uint resource) {
	_host.delaySpeech(1);
	_host.say(actor, resource);
	_host.delaySpeech(1);
}

void MusicPatterns::speeches(const Common::String &actors, const Common::String &resources, uint count) {
	for (uint i = 1; i <= count; i++) {
		Common::String suffix = Common::String::format(kUnsignedIndexFormat, i);
		speech(_host._page->text(actors + suffix), number(resources + suffix));
	}
}

void MusicPatterns::sound(const Common::String &key) { _host.playSounds({static_cast<uint16>(number(key))}); }

void MusicPatterns::group(const char *prefix, int index, int count) {
	speeches(Common::String::format(kKNameCharIDsSignedIndexFormat, prefix, index), Common::String::format(kKNameSpchIDsSignedIndexFormat, prefix, index), count);
}

bool MusicPatterns::enter() {
	if (!_data.loadStream(_host._vm.resources().open(kDataXML)) || !_data.parse() || !_host.actor(kPropertyGrid, kLocalKey, false))
		return false;
	_host._actors[kPropertyGrid]->set(kPropertyZ, {}, Core::ScriptValue(number(kGridZKey)));
	_host._actors[kPropertyGrid]->set(kPropertyAnimateSettled, {}, Core::ScriptValue(0));
	World &world = _host._world;
	_level = CLIP(world.get(kR1LA2LevelKey), 1, 3);
	_dataset = CLIP(world.get(kR1LA2DatasetKey), 1, 9);
	_count = CLIP(world.get(kR1LA2CountKey), 0, 1000000);
	_state = CLIP(world.get(kR1LA2StateKey), 3, 5);
	_pillowHere = !world.get(kR1LA2StateKey) || world.get(kR1LA2PillowHereKey);
	for (int row = 0; row < 5; row++) {
		_bank[row] = CLIP(world.get(Common::String::format(kR1LA2MarbleSignedIndexCountKey, row + 1)), 0, 8);
		_notes[row] = new OM::Sound(_host._vm.mixer());
	}
	if (_host._practice) {
		_count = 10;
		_state = 5;
		_pillowHere = false;
		for (int &bank : _bank)
			bank = 8;
		if (_host._chester)
			_host._chester->visible = false;
	}
	for (int i = 0; i < 3; i++) {
		_doors[i] = sprite(Common::String::format(kDoorIDSignedIndexKey, i + 1), indexed(kDoorZKey, i + 1));
		if (!_doors[i])
			return false;
		_doors[i]->boxTouchy = true;
	}
	for (int column = 0; column < 12; column++) {
		_lights[column] = sprite(kColumnLightsIDKey, 20);
		if (!_lights[column])
			return false;
		_lights[column]->x = number(kColumnLightXKey) + column * number(kColumnLightDeltaXKey);
		_lights[column]->y = number(kColumnLightYKey);
		if (8 <= column)
			continue;
		for (int row = 0; row < 5; row++) {
			_locks[row][column] = sprite(kLockIDKey, number(kLockZKey));
			if (!_locks[row][column])
				return false;
			_locks[row][column]->x = number(kGridXKey) + column * number(kGridDeltaXKey);
			_locks[row][column]->y = number(kGridYKey) + row * number(kGridDeltaYKey);
		}
	}
	_button = sprite(kButtonIDKey, 20);
	_doorLights = sprite(kDoorLightsIDKey, 20);
	_activate = sprite(kActivateDoorIDKey, 20);
	_spin = sprite(kSpinIDKey, number(kSpinZKey));
	_pillow = sprite(kPillowIDKey, number(kPillowZKey));
	_jewel = sprite(kJewelIDKey, number(kJewelZKey));
	_sparkle = sprite(kSparkleIDKey, number(kJewelZKey) + 1);
	if (!_button || !_doorLights || !_activate || !_spin || !_pillow || !_jewel || !_sparkle || !sprite(kTrayMaskIDKey, number(kTrayMaskZKey)))
		return false;
	_activate->visible = _spin->visible = false;
	_pillow->visible = _pillowHere;
	_sparkle->y += number(kSparkleDeltaYKey);
	if (!_pillowHere) {
		_jewel->y += number(kJewelDeltaYKey);
		_sparkle->y += number(kJewelDeltaYKey);
	}
	_jewel->visible = _sparkle->visible = !_host._practice;
	_sparkle->startAnimation(-1);
	doors();
	if (!setup())
		return false;
	const bool firstVisit = !world.visits(Page::kPageLA2);
	_inventoryHelp = !_host._practice && (firstVisit || _state != 5);
	_playOnEntry = !firstVisit && _state == 5;
	if (_host._practice && firstVisit) {
		say(kPropertyGrid, kReEnterSpchIDs41Key);
		say(kPropertyGrid, kLocalSpchIDs411Key);
	} else if (firstVisit) {
		group(kLocalName, 1, 7);
		speech(_host._page->text(kLocalCharIDs16Key), number(kLocalSpchIDs18Key));
	} else {
		int greeting = _host._vm._rnd->getRandomNumberRng(2, _count < number(kMaxLightsKey) ? 4 : 3);
		if (greeting == 3)
			say(kReaderName, kReEnterSpchIDs31Key);
		say(kPropertyGrid, Common::String::format(kReEnterSpchIDsSignedIndexSignedIndexKey, greeting, greeting == 3 ? 2 : 1));
		if (greeting == 4)
			say(kSamName, kReEnterSpchIDs42Key);
		if (_inventoryHelp)
			say(_state == 3 ? kPropertyGrid : kSamName, _state == 3 ? kReEnterSpchIDs51Key : kReEnterSpchIDs61Key);
	}
	_idle = _host._time;
	return true;
}

void MusicPatterns::draw(OM::Canvas &canvas) {
	_host._scene.drawBackground(canvas);
	const int x = number(kGridXKey);
	const int height = number(kGridDeltaYKey);
	const int width = 12 * number(kGridDeltaXKey);
	for (uint row = 0; row < 5; row++) {
		const int y = number(kGridYKey) + row * height;
		canvas.recolorRect(Common::Rect(x, y, x + width, y + height), PageScene::marbleColor(row));
	}
	_host._scene.drawSprites(canvas, INT_MIN, INT_MAX);
}

void MusicPatterns::clearMarbles() {
	for (auto &marble : _marbles)
		_host._scene.remove(marble.sprite);
	_marbles.clear();
	_dragged = -1;
}

bool MusicPatterns::setup() {
	Common::StringTokenizer tokens(_data.get(Common::String::format(kPropertyDataSetLevelSignedIndexWordListSignedIndexSet1, _level, _dataset)));
	memset(_target, 0, sizeof(_target));
	memset(_needed, 0, sizeof(_needed));
	_given = 0;
	for (int column = 0; column < indexed(kNumGivenKey, _level); column++) {
		if (tokens.empty())
			return false;
		Common::String chord = tokens.nextToken();
		if (chord.equals("0"))
			continue;
		// Authored nonempty columns form a prefix, followed by zero padding.
		if (column != _given)
			return false;
		for (char note : chord) {
			if (note < '1' || '5' < note)
				return false;
			_target[note - '1'][column] = true;
		}
		_given += 1;
	}
	if (!_given)
		return false;
	clearMarbles();
	for (int row = 0; row < 5; row++) {
		for (int column = _given; column < 12; column++) {
			_target[row][column] = _target[row][column - _given];
			if (_target[row][column])
				_needed[row] += 1;
		}
		for (int column = 0; column < 8; column++) {
			_locks[row][column]->visible = column < _given && _target[row][column];
			if (!_locks[row][column]->visible)
				continue;
			Marble marble;
			marble.row = row;
			marble.column = column;
			marble.fixed = true;
			marble.sprite = sprite(Common::String::format(kMarbleIDSignedIndexKey, row + 1), 5);
			if (!marble.sprite)
				return false;
			marble.sprite->colorRemap = PageScene::marbleColor(row);
			marble.sprite->x = _locks[row][column]->x;
			marble.sprite->y = _locks[row][column]->y;
			_marbles.push_back(marble);
		}
		for (int i = 0; i < _bank[row]; i++) {
			Marble marble;
			marble.row = row;
			marble.depth = number(kMarbleZKey) - i * number(kMarbleDeltaZKey);
			marble.home = Common::Point(indexed(kMarbleXKey, row + 1) - i * number(kMarbleDeltaXKey),
										number(kMarbleYKey) - i * number(kMarbleDeltaYKey));
			marble.sprite = sprite(Common::String::format(kMarbleIDSignedIndexKey, row + 1), marble.depth);
			if (!marble.sprite)
				return false;
			marble.sprite->colorRemap = PageScene::marbleColor(row);
			marble.sprite->x = marble.home.x;
			marble.sprite->y = marble.home.y;
			_marbles.push_back(marble);
		}
	}
	if (3 < _state) {
		_state = 5;
		for (int row = 0; row < 5; row++) {
			if (_bank[row] < _needed[row])
				_state = 4;
		}
	}
	lights();
	persist();
	return true;
}

void MusicPatterns::persist() {
	if (_host._practice)
		return;
	World &world = _host._world;
	world.set(kR1LA2LevelKey, _level);
	world.set(kR1LA2DatasetKey, _dataset);
	world.set(kR1LA2CountKey, _count);
	world.set(kR1LA2StateKey, _state);
	world.set(kR1LA2PillowHereKey, _pillowHere);
	for (int row = 0; row < 5; row++)
		world.set(Common::String::format(kR1LA2MarbleSignedIndexCountKey, row + 1), _bank[row]);
}

bool MusicPatterns::donate(uint item) {
	uint first = number(kINV1Marble1Key);
	if (_host._practice || item < first || first + 5 <= item)
		return false;
	_host.interrupt();
	_donated = false;
	_firstDonation = _state == 3;
	for (int row = 0; row < 5; row++) {
		Common::String key = Common::String::format(kItemUnsignedIndexFormat, first + row);
		int available = MAX(0, _host._world.get(key));
		int take = MIN(available, 8 - _bank[row]);
		_donated |= 0 < take;
		_bank[row] += take;
		_host._world.set(key, available - take);
	}
	if (_donated) {
		_state = 4;
		if (!setup())
			error("RR1: cannot reset LA2 pattern");
		sound(kSceneSfxIDKey);
	}
	closeInventory();
	return true;
}

void MusicPatterns::closeInventory() {
	_host.closeChest(false);
	if (_host._chester)
		_host._chester->visible = false;
	_closeStart = 0;
	_phase = kChestClosing;
}

void MusicPatterns::skipSpeech() {
	// The inventory inspection lines and its close delay are non-interruptible.
	if (_phase != kInventorySpeech && _phase != kChestOpening && _phase != kBackpackOpening && _phase != kChestClosing)
		_host.interrupt();
}

void MusicPatterns::resume(uint32 elapsed) {
	_idle += elapsed;
	if (_phase == kChestClosing && _closeStart)
		_closeStart += elapsed;
}

int MusicPatterns::resident(int row, int column) const {
	for (uint i = 0; i < _marbles.size(); i++) {
		if (_marbles[i].row == row && _marbles[i].column == column)
			return i;
	}
	return -1;
}

void MusicPatterns::home(int index) {
	Marble &marble = _marbles[index];
	marble.column = -1;
	marble.sprite->x = marble.home.x;
	marble.sprite->y = marble.home.y;
	marble.sprite->setFrame(1);
	_host._scene.setDepth(marble.sprite, marble.depth);
}

void MusicPatterns::drop() {
	Marble &marble = _marbles[_dragged];
	Common::Rect picture(marble.sprite->x, marble.sprite->y, marble.sprite->x + marble.sprite->width(), marble.sprite->y + marble.sprite->height());
	int best = 0;
	int column = -1;
	for (int c = _given; c < 12; c++) {
		for (int row = 0; row < 5; row++) {
			int x = number(kGridXKey) + c * number(kGridDeltaXKey);
			int y = number(kGridYKey) + row * number(kGridDeltaYKey);
			Common::Rect overlap(x, y, x + number(kGridDeltaXKey), y + number(kGridDeltaYKey));
			if (!overlap.intersects(picture))
				continue;
			overlap.clip(picture);
			int area = overlap.width() * overlap.height();
			if (best < area) {
				best = area;
				column = c;
			}
		}
	}
	if (column < 0) {
		home(_dragged);
		sound(kGoHomeSfxIDKey);
	} else {
		int previous = resident(marble.row, column);
		if (0 <= previous)
			home(previous);
		marble.column = column;
		marble.sprite->x = number(kGridXKey) + column * number(kGridDeltaXKey);
		marble.sprite->y = number(kGridYKey) + marble.row * number(kGridDeltaYKey);
		marble.sprite->setFrame(1);
		_host._scene.setDepth(marble.sprite, number(kGridZKey) + 1);
		sound(kSnapSfxIDKey);
	}
	_dragged = -1;
	lights();
}

void MusicPatterns::lights() {
	for (int column = 0; column < 12; column++) {
		int count = 0;
		int frame = 1;
		for (int row = 0; row < 5; row++) {
			if (0 <= resident(row, column)) {
				count += 1;
				frame = row + 2;
			}
		}
		_lights[column]->colorRemap = OM::Canvas::ColorRemap::kNone;
		if (count == 1)
			_lights[column]->colorRemap = PageScene::marbleColor(frame - 2);
		_lights[column]->visible = _level != 3;
		_lights[column]->setFrame(1 < count ? number(kColumnLightBadIDKey) : frame);
	}
}

void MusicPatterns::doors() {
	for (int i = 0; i < 3; i++) {
		int progress = CLIP(_count - i * 3, 0, 3);
		_doors[i]->visible = progress < 3;
		_doors[i]->setFrame(MIN(progress + 1, 3));
	}
	_doorLights->setFrame(MIN(_count, 9) + 1);
}

void MusicPatterns::play(Result result, int first, int last, int soloRow) {
	_result = result;
	_column = first;
	_lastColumn = last;
	_soloRow = soloRow;
	_phase = kPlayStart;
}

void MusicPatterns::playColumn() {
	bool empty = true;
	for (int row = 0; row < 5; row++) {
		if (0 <= _soloRow && row != _soloRow)
			continue;
		int index = resident(row, _column);
		if (index < 0)
			continue;
		empty = false;
		_marbles[index].sprite->setFrame(number(kMarbleSingFrameKey));
		_notes[row]->play(_host._vm.resources(), _host.characterResource(indexed(kMarbleIDKey, row + 1), Shell::Character::Resources::kSound),
						  Audio::Mixer::kSFXSoundType);
	}
	if (empty)
		_notes[0]->play(_host._vm.resources(), _host.characterResource(number(kBlankSfxIDKey), Shell::Character::Resources::kSound), Audio::Mixer::kSFXSoundType);
	_lights[_column]->setFrame(_lights[_column]->getFrame() + 7);
}

void MusicPatterns::endColumn() {
	for (auto *note : _notes)
		note->stop();
	for (auto &marble : _marbles) {
		if (marble.column == _column)
			marble.sprite->setFrame(number(kMarbleEyeCloseFrameKey));
	}
	lights();
}

void MusicPatterns::finishPlayback() {
	if (_result == kCorrect) {
		_spin->visible = true;
		_spin->startAnimation(1);
		_phase = kSpin;
	} else if (_result == kWrong) {
		say(kPropertyGrid, Common::String::format(kWrongSpchIDsSignedIndexKey, _host._vm._rnd->getRandomNumberRng(1, 3)));
		_phase = kWrongSpeech;
	} else {
		if (_result == kListen && _helpAfterPlayback) {
			speeches(kSceneCharIDs2Key, kSceneSpchIDs2Key, 3);
			_helpAfterPlayback = false;
		}
		if (_result == kLocked)
			say(kPropertyGrid, Common::String::format(kNoMoveSpchIDSignedIndexKey, _host._vm._rnd->getRandomNumberRng(1, 2)));
		_phase = kIdle;
		_idle = _host._time;
	}
}

void MusicPatterns::assess() {
	if (_host._practice || _count == 9)
		return;
	World &world = _host._world;
	int outcome = _mistakes ? 3 : 1;
	int streak = world.get(kR1LA2StreakKindKey) == outcome ? world.get(kR1LA2StreakKey) + 1 : 1;
	int threshold = indexed(_mistakes ? kAutolevelDownCountKey : kAutolevelUpCountKey, _level);
	world.recordAssessment(kR1LA2Key, _level, outcome, indexed(kAutolevelUpCountKey, _level), indexed(kAutolevelDownCountKey, _level), streak);
	world.set(kR1LA2StreakKindKey, outcome);
	world.set(kR1LA2StreakKey, threshold <= streak ? 0 : streak);
	if (threshold <= streak && !world.get(kR1LA2FixedLevelKey))
		_level = CLIP(_level + (_mistakes ? -1 : 1), 1, 3);
}

void MusicPatterns::accept() {
	_count = MIN(_count + 1, 1000000);
	assess();
	_dataset = _dataset % 9 + 1;
	// Commit the next problem with the reward before the interruptible door sequence.
	persist();
	if (_count <= 9) {
		_activate->visible = true;
		_activate->startAnimation(1);
		sound(kActivateDoorIDKey);
		_phase = kDoor;
	} else {
		_phase = kNext;
	}
}

void MusicPatterns::advance(uint32 time) {
	if (_host._nextSpeech < _host._speech.size() || (_host._speaking && _host._speaking->isBusy()))
		_idle = time;
}

void MusicPatterns::update(uint32 time) {
	if (_host._nextSpeech < _host._speech.size())
		return;
	if (_phase == kEntryGreeting) {
		if (_inventoryHelp) {
			say(kReaderName, kHaveSpch11Key);
			_phase = kInventoryStart;
		} else if (_playOnEntry) {
			play(kListen);
		} else {
			_phase = kIdle;
		}
	} else if (_phase == kInventoryStart) {
		_chestOpening = sprite(kGChesterOpenIDName, number(kGClosedChesterZName));
		if (_chestOpening) {
			_chestOpening->startAnimation(1);
			if (_host._chester)
				_host._chester->visible = false;
		}
		_phase = kChestOpening;
	} else if (_phase == kChestOpening) {
		if (_chestOpening && _chestOpening->isRunning())
			return;
		_host._scene.remove(_chestOpening);
		_chestOpening = nullptr;
		_host.openChest();
		_phase = kBackpackOpening;
	} else if (_phase == kBackpackOpening) {
		if (!_host._chestOpen)
			return;
		_haveMarbles = false;
		for (int row = 0; row < 5; row++)
			_haveMarbles |= 0 < _host._world.get(Common::String::format(kItemSignedIndexKey, number(kINV1Marble1Key) + row));
		say(kSamName, _haveMarbles ? kHaveSpch21Key : kHaveSpch31Key);
		say(kReaderName, _haveMarbles ? kHaveSpch22Key : kHaveSpch32Key);
		_phase = kInventorySpeech;
	} else if (_phase == kInventorySpeech) {
		if (_haveMarbles) {
			_phase = kIdle;
			_idle = time;
		} else {
			closeInventory();
		}
	} else if (_phase == kChestClosing) {
		if (!_closeStart)
			_closeStart = time;
		if (time - _closeStart < 500)
			return;
		if (_host._chester)
			_host._chester->visible = true;
		if (_donated) {
			if (_firstDonation) {
				for (int i = 1; i <= 3; i++)
					say(kPropertyGrid, Common::String::format(kSceneSpchIDs1SignedIndexKey, i));
			}
			_helpAfterPlayback = _state == 4;
			_donated = false;
			_phase = kDonationSpeech;
		} else {
			_phase = kIdle;
		}
		_idle = time;
	} else if (_phase == kIntroduction || _phase == kDonationSpeech) {
		play(kListen);
	} else if (_phase == kPlayStart) {
		playColumn();
		_phase = kPlayback;
	} else if (_phase == kPlayback) {
		for (auto *note : _notes) {
			if (note->isPlaying())
				return;
		}
		endColumn();
		_column += 1;
		if (_column < _lastColumn)
			playColumn();
		else
			finishPlayback();
	} else if (_phase == kWrongSpeech) {
		_mistakes += 1;
		for (uint i = 0; i < _marbles.size(); i++) {
			const Marble &marble = _marbles[i];
			if (!marble.fixed && 0 <= marble.column && !_target[marble.row][marble.column])
				home(i);
		}
		sound(kEmptyGridSfxIDKey);
		lights();
		_phase = kIdle;
	} else if (_phase == kSpin && !_spin->isRunning()) {
		_spin->visible = false;
		say(kPropertyGrid, Common::String::format(kCongratsSpchIDSignedIndexKey, _host._vm._rnd->getRandomNumberRng(1, 4)));
		_phase = kCongratulations;
	} else if (_phase == kCongratulations) {
		accept();
	} else if (_phase == kDoor && !_activate->isRunning()) {
		_activate->visible = false;
		doors();
		sound(_count % 3 ? kDoorSfxID1Key : kDoorSfxID2Key);
		if (_count <= 3)
			group(kDoorOpenName, _count, _count == 2 ? 2 : 4);
		else if (_count == 9 && _pillowHere) {
			sound(kFanFareSfxIDKey);
			speeches(kLastDoorCharIDsKey, kLastDoorSpchIDsKey, 4);
		} else {
			int line = _host._vm._rnd->getRandomNumberRng(1, 3);
			speech(_host._page->text(Common::String::format(kDoorOpenCharIDs4SignedIndexKey, line)), indexed(kDoorOpenSpchIDs4Key, line));
		}
		_phase = kNext;
	} else if (_phase == kNext) {
		_mistakes = 0;
		if (!setup())
			error("RR1: cannot load LA2 pattern");
		sound(kNewPatternSfxIDKey);
		_phase = kIntroduction;
	} else if (!_host._practice && _phase == kIdle && !_host._chestOpen && 30000 <= time - _idle) {
		bool enough = _state == 5;
		_idleIndex = _idleIndex % (enough ? 3 : 2) + 1;
		Common::String prefix = Common::String::format(kIdleSignedIndexKey, enough ? 2 : 1);
		int count = 2;
		if (enough) {
			if (_idleIndex == 1)
				count = 1;
		} else if (_idleIndex == 2) {
			count = 1;
		}
		speeches(prefix + Common::String::format(kCharIDSignedIndexFormat, _idleIndex),
				 prefix + Common::String::format(kSpchIDSignedIndexFormat, _idleIndex), count);
		_idle = time;
	}
}

void MusicPatterns::hint() {
	_paigeClicks = _paigeClicks % 2 + 1;
	if (_state == 3) {
		say(kPaigeName, Common::String::format(kPaigeSpchIDs2SignedIndexKey, _paigeClicks));
	} else if (_state == 4) {
		say(kPaigeName, kPaigeSpchIDs3Key);
	} else {
		if (_paigeClicks == 2 && _level != 3)
			say(kPaigeName, kPaigeSpchIDs42Key);
		else {
			say(kPaigeName, kPaigeSpchIDs411Key);
			if (_level != 3)
				say(kPaigeName, kPaigeSpchIDs412Key);
			say(kSamName, kPaigeSpchIDs413Key);
		}
	}
	_idle = _host._time;
}

bool MusicPatterns::click(Common::Point point) {
	_idle = _host._time;
	if (_button->covers(point)) {
		sound(kButtonIDKey);
		bool complete = true;
		bool correct = true;
		for (int column = 0; column < 12; column++) {
			int count = 0;
			int needed = 0;
			for (int row = 0; row < 5; row++) {
				bool present = 0 <= resident(row, column);
				count += present;
				needed += _target[row][column];
				if (present && !_target[row][column])
					correct = false;
			}
			complete &= 0 < count && (_level != 3 || needed <= count);
		}
		if (complete && !correct)
			sound(kWrongSfxIDKey);
		if (!complete) {
			_playClicks = _playClicks % 3 + 1;
			int lines = 2;
			if (_playClicks == 1)
				lines = _level == 3 ? 3 : 4;
			else if (_playClicks == 2)
				lines = 1;
			group(kPlayName, _playClicks, lines);
		}
		Result result = kListen;
		if (complete)
			result = correct ? kCorrect : kWrong;
		play(result);
		return true;
	}
	Shell::Sprite *hit = _host._scene.hitSprite(point);
	for (uint i = 0; i < _marbles.size(); i++) {
		Marble &marble = _marbles[i];
		if (marble.sprite != hit)
			continue;
		if (marble.fixed) {
			play(kLocked, marble.column, marble.column + 1, marble.row);
		} else {
			_dragged = i;
			_dragOffset = point - Common::Point(marble.sprite->x, marble.sprite->y);
			marble.column = -1;
			marble.sprite->setFrame(2);
			_host._scene.setDepth(marble.sprite, 190);
			sound(kPickUpSfxIDKey);
			lights();
		}
		return true;
	}
	for (auto *door : _doors) {
		if (door->visible && door->covers(point)) {
			sound(kCOMSoftClickSfxIDKey);
			_doorClicks = _doorClicks % 3 + 1;
			if (_state == 3) {
				say(kSamName, kDoorClkSpchIDs4Key);
				say(kPropertyGrid, kLocalSpchIDs31Key);
				say(kPropertyGrid, kLocalSpchIDs32Key);
			} else {
				group(kDoorClkName, _doorClicks, indexed(kMaxDoorClkSpchKey, _doorClicks));
			}
			return true;
		}
	}
	if (9 <= _count && _pillowHere && _pillow->covers(point)) {
		_pillowHere = _pillow->visible = false;
		Common::String key = Common::String::format(kItemSignedIndexKey, number(kINV1PillowKey));
		_host._world.set(key, MIN(25, _host._world.get(key) + 1));
		_host.putAway();
		_jewel->y += number(kJewelDeltaYKey);
		_sparkle->y += number(kJewelDeltaYKey);
		sound(kJewelDropSfxIDKey);
		say(kReaderName, kPillowSpchIDKey);
		persist();
		return true;
	}
	for (auto *light : _lights) {
		if (light->visible && light->covers(point)) {
			say(kPropertyGrid, kLightSpchIDKey);
			return true;
		}
	}
	if (_jewel->visible && _jewel->covers(point)) {
		_jewelClicks = MIN(_jewelClicks + 1, 4);
		sound(kCOMSoftClickSfxIDKey);
		group(kJewelName, _jewelClicks, indexed(kNumJewelSpchsKey, _jewelClicks));
		return true;
	}
	if (_host._actors[kPaigeName]->bounds().contains(point)) {
		hint();
		return true;
	}
	if (_host._actors[kPropertyGrid]->bounds().contains(point)) {
		_localClicks += 1;
		if (_state == 3) {
			say(kPropertyGrid, kLocalSpchIDs31Key);
			say(kPropertyGrid, kLocalSpchIDs32Key);
		} else if (_localClicks == 1 || _state == 4) {
			say(kPropertyGrid, kLocalSpchIDs411Key);
			if (_state == 4)
				group(kSceneName, 2, 2);
		} else if (_localClicks == 2) {
			say(kPropertyGrid, kLocalSpchIDs511Key);
			if (_level != 3)
				say(kPropertyGrid, kLocalSpchIDs512Key);
		} else {
			say(kPropertyGrid, Common::String::format(kLocalSpchIDs52SignedIndexKey, _host._vm._rnd->getRandomNumberRng(1, 6)));
		}
		return true;
	}
	if (!_host._practice && (_host._actors[kReaderName]->bounds().contains(point) || _host._actors[kSamName]->bounds().contains(point))) {
		if (_state == 4) {
			group(kSceneName, 2, 2);
		} else {
			_mainClicks = _mainClicks % (_state == 5 ? 3 : 2) + 1;
			const char *actors = nullptr;
			if (_state == 3)
				actors = _mainClicks == 1 ? kSppName : kRgName;
			else if (_mainClicks == 1)
				actors = kRggName;
			else
				actors = _mainClicks == 2 ? kRgName : kSgrName;
			for (uint i = 0; actors[i]; i++) {
				const char *actor = kPropertyGrid;
				if (actors[i] == 'r')
					actor = kReaderName;
				else if (actors[i] == 's')
					actor = kSamName;
				else if (actors[i] == 'p')
					actor = kPaigeName;
				say(actor, Common::String::format(kMainSpchIDsSignedIndexSignedIndexUnsignedIndexKey, _state == 3 ? 3 : 4, _mainClicks, i + 1));
			}
		}
		return true;
	}
	return false;
}

bool MusicPatterns::event(const Common::Event &event) {
	if (0 <= _dragged && event.type == Common::EVENT_MOUSEMOVE) {
		_marbles[_dragged].sprite->x = event.mouse.x - _dragOffset.x;
		_marbles[_dragged].sprite->y = event.mouse.y - _dragOffset.y;
		return true;
	}
	if (0 <= _dragged && event.type == Common::EVENT_LBUTTONUP) {
		_marbles[_dragged].sprite->x = event.mouse.x - _dragOffset.x;
		_marbles[_dragged].sprite->y = event.mouse.y - _dragOffset.y;
		drop();
		return true;
	}
	if (_phase == kIdle && event.type == Common::EVENT_MOUSEMOVE) {
		Shell::Sprite *hit = _host._scene.hitSprite(event.mouse);
		_button->setFrame(hit == _button ? 2 : 1);
		for (auto &marble : _marbles) {
			if (!marble.fixed)
				marble.sprite->setFrame(hit == marble.sprite ? 2 : 1);
		}
	}
	return false;
}

void MusicPatterns::pause(bool paused) {
	for (auto *note : _notes) {
		if (note)
			note->pause(paused);
	}
	if (paused) {
		_pausedSprites.clear();
		for (auto *animation : {_spin, _activate, _sparkle, _chestOpening}) {
			if (animation && animation->isRunning()) {
				_pausedSprites.push_back(animation);
				animation->pauseAnimation(true);
			}
		}
	} else {
		for (auto *animation : _pausedSprites)
			animation->pauseAnimation(false);
		_pausedSprites.clear();
	}
}

} // namespace RR1Cloud9
} // namespace Trinket
