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

#include "trinket/rr1cloud9/cn3_gallery.h"

#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

constexpr const char *Gallery::kNameSignedIndexFormat;
constexpr const char *Gallery::kArtistKey;
constexpr const char *Gallery::kNameUnsignedIndexFormat;
constexpr const char *Gallery::kR1CN3StateKey;
constexpr const char *Gallery::kArtistSetIDKey;
constexpr const char *Gallery::kArtistZKey;
constexpr const char *Gallery::kRegisterFidgetsToSettleName;
constexpr const char *Gallery::kRegisterSpeechToSettleName;
constexpr const char *Gallery::kLoadSpeechAnimsInBkgdName;
constexpr const char *Gallery::kAnimateSettledName;
constexpr const char *Gallery::kMethodSetSpeechAnims;
constexpr const char *Gallery::kArtistLipsIDKey;
constexpr const char *Gallery::kArtistBodyIDKey;
constexpr const char *Gallery::kMethodAddFidgetGroup;
constexpr const char *Gallery::kMethodAddFidget;
constexpr const char *Gallery::kArtistFidg1IDKey;
constexpr const char *Gallery::kArtistFidg2IDKey;
constexpr const char *Gallery::kR1CN3NumFramesKey;
constexpr const char *Gallery::kNumMinPagesKey;
constexpr const char *Gallery::kR1CN3SaidAct4Key;
constexpr const char *Gallery::kASEQCN3EASELName;
constexpr const char *Gallery::kASEQCN3HATName;
constexpr const char *Gallery::kASEQCN3TAKEOFFName;
constexpr const char *Gallery::kASEQCN3PICFRAME1SignedIndexFormat;
constexpr const char *Gallery::kPicXKey;
constexpr const char *Gallery::kPicYKey;
constexpr const char *Gallery::kR1CA2PicMaxSignedIndexKey;
constexpr const char *Gallery::kSamplesSampleBSignedIndexISignedIndexDATPath;
constexpr const char *Gallery::kR1CA2DatasetKey;
constexpr const char *Gallery::kR1CN3HatPendingKey;
constexpr const char *Gallery::kR1CN3MoviePendingKey;
constexpr const char *Gallery::kEnterSpchIDsKey;
constexpr const char *Gallery::kSsrName;
constexpr const char *Gallery::kR1CN3FromStreetKey;
constexpr const char *Gallery::kPreviousLocationKey;
constexpr const char *Gallery::kReEnterSpchIDs2Key;
constexpr const char *Gallery::kAsaName;
constexpr const char *Gallery::kReEnterSpchIDs31Key;
constexpr const char *Gallery::kReEnterSpchIDs3Key;
constexpr const char *Gallery::kReEnterSpchIDs34Key;
constexpr const char *Gallery::kR1CN3SaidSS4Key;
constexpr const char *Gallery::kReEnterSpchIDs4Key;
constexpr const char *Gallery::kAaName;
constexpr const char *Gallery::kR1CN31stEditKey;
constexpr const char *Gallery::kReaderName;
constexpr const char *Gallery::kReEnterSpchIDsA31Key;
constexpr const char *Gallery::kReEnterSpchIDsA4Key;
constexpr const char *Gallery::kAaraName;
constexpr const char *Gallery::kReEnterSpchIDsA5Key;
constexpr const char *Gallery::kR1CA2PicUsedSignedIndexKey;
constexpr const char *Gallery::kItemSignedIndexFormat;
constexpr const char *Gallery::kINV1HatKey;
constexpr const char *Gallery::kPropertyVisible;
constexpr const char *Gallery::kSamName;
constexpr const char *Gallery::kReEnterSpchIDsA45Key;
constexpr const char *Gallery::kReEnterSpchIDsA46Key;
constexpr const char *Gallery::kTalkedToRockHeadKey;
constexpr const char *Gallery::kReEnterSpchIDsA47Key;
constexpr const char *Gallery::kIdleCharIDSignedIndexKey;
constexpr const char *Gallery::kIdleSpchIDSignedIndexKey;
constexpr const char *Gallery::kMaxIdleIndexSpeechesKey;
constexpr const char *Gallery::kArtistCharIDs1Key;
constexpr const char *Gallery::kArtistSpchIDs1Key;
constexpr const char *Gallery::kArtistSpchIDs21Key;
constexpr const char *Gallery::kArtistSpchIDs31Key;
constexpr const char *Gallery::kR1CN3FromSampleKey;
constexpr const char *Gallery::kArtistSpchIDs34Key;
constexpr const char *Gallery::kArtistSpchIDs32Key;
constexpr const char *Gallery::kAaaName;
constexpr const char *Gallery::kArtistSpchIDs33Key;
constexpr const char *Gallery::kArtistSpchIDs4Key;
constexpr const char *Gallery::kArName;
constexpr const char *Gallery::kMainCharIDs2SignedIndexKey;
constexpr const char *Gallery::kMainSpchIDs2SignedIndexKey;
constexpr const char *Gallery::kMaxMain2SpeechesKey;
constexpr const char *Gallery::kMainSpchIDs41Key;
constexpr const char *Gallery::kMainSpchIDs51Key;
constexpr const char *Gallery::kSaName;
constexpr const char *Gallery::kMainSpchIDs52Key;
constexpr const char *Gallery::kSrName;
constexpr const char *Gallery::kCOMSoftClickSfxIDKey;
constexpr const char *Gallery::kHatSpchIDs11Key;
constexpr const char *Gallery::kHatSpchIDs31Key;
constexpr const char *Gallery::kRaaasaaName;
constexpr const char *Gallery::kHatSpchIDs21Key;
constexpr const char *Gallery::kRaaaName;
constexpr const char *Gallery::kHatSpchIDs22Key;
constexpr const char *Gallery::kRaaName;
constexpr const char *Gallery::kHatSpchIDs4Key;
constexpr const char *Gallery::kSarName;
constexpr const char *Gallery::kEaselSpchIDKey;
constexpr const char *Gallery::kRraName;
constexpr const char *Gallery::kPaigeName;
constexpr const char *Gallery::kPaigeSpchIDsKey;

Gallery::~Gallery() {
	for (auto &image : _edited)
		image.free();
	for (auto &book : _samples) {
		for (auto *image : book) {
			image->free();
			delete image;
		}
	}
}

int Gallery::number(const Common::String &key) const { return _host._page->number(key); }

int Gallery::indexed(const char *key, int index) const { return number(Common::String::format(kNameSignedIndexFormat, key, index)); }

void Gallery::say(const char *actorName, const Common::String &key) { _host.say(actorName, number(key)); }

void Gallery::group(const char *key, const char *actors) {
	for (uint i = 0; actors[i]; i++) {
		const char *actorName;
		if (actors[i] == 'r')
			actorName = "reader";
		else if (actors[i] == 's')
			actorName = "sam";
		else
			actorName = kArtistKey;
		say(actorName, Common::String::format(kNameUnsignedIndexFormat, key, i + 1));
	}
}

void Gallery::setState(int state) {
	_state = state;
	if (!_host._practice)
		_host._world.set(kR1CN3StateKey, state);
}

bool Gallery::loadImage(Common::SeekableReadStream &stream, Graphics::Surface &image) {
	if (stream.size() < 10)
		return false;
	uint version = stream.readUint16LE();
	bool bigEndian = version == 256;
	if (version != 1 && !bigEndian)
		return false;
	uint32 size = bigEndian ? stream.readUint32BE() : stream.readUint32LE();
	uint width = bigEndian ? stream.readUint16BE() : stream.readUint16LE();
	uint height = bigEndian ? stream.readUint16BE() : stream.readUint16LE();
	if (!width || !height || 640 < width || 482 < height || size != 4 + width * height * 2 || stream.size() != size + 6)
		return false;
	image.free();
	image.create(151, 100, Graphics::PixelFormat::createFormatRGBA32());
	image.fillRect(Common::Rect(151, 100), image.format.RGBToColor(255, 255, 255));
	// Paint canvases include two extra rows. Scatter in source order, then clip
	// the preview to the requested rectangle; the last source pixel wins.
	for (uint y = 0; y < height; y++) {
		uint targetY = y * 102 / height;
		for (uint x = 0; x < width; x++) {
			uint color = bigEndian ? stream.readUint16BE() : stream.readUint16LE();
			if (targetY < 100) {
				uint r = (color >> 10) & 31;
				uint g = (color >> 5) & 31;
				uint b = color & 31;
				image.setPixel(x * 151 / width, targetY, image.format.RGBToColor((r << 3) | (r >> 2), (g << 3) | (g >> 2), (b << 3) | (b >> 2)));
			}
		}
	}
	return !stream.err() && !stream.eos();
}

bool Gallery::artist(int pose) {
	if (_host._actors.contains(kArtistKey)) {
		delete _host._actors[kArtistKey];
		_host._actors.erase(kArtistKey);
	}
	Common::ScopedPtr<Shell::Character> actor(new Shell::Character(_host._scene, _host._vm.mixer(), _host._vm.resources(), _host._vm.rnd(), _host));
	if (!actor->create(indexed(kArtistSetIDKey, pose), number(kArtistZKey)))
		return false;
	actor->movable = false;
	for (const char *key : {kRegisterFidgetsToSettleName, kRegisterSpeechToSettleName, kLoadSpeechAnimsInBkgdName, kAnimateSettledName})
		actor->set(key, {}, Core::ScriptValue(0));
	Core::ScriptValue result;
	actor->play(kMethodSetSpeechAnims, {Core::ScriptValue(indexed(kArtistLipsIDKey, pose)), Core::ScriptValue(pose == 1 ? 0 : 10), Core::ScriptValue(1),
						 Core::ScriptValue(number(kArtistBodyIDKey)), Core::ScriptValue(0), Core::ScriptValue(0)}, result);
	actor->play(kMethodAddFidgetGroup, {Core::ScriptValue(1), Core::ScriptValue(5), Core::ScriptValue(10)}, result);
	actor->play(kMethodAddFidget, {Core::ScriptValue(1), Core::ScriptValue(indexed(kArtistSetIDKey, pose))}, result);
	actor->play(kMethodAddFidgetGroup, {Core::ScriptValue(2), Core::ScriptValue(15), Core::ScriptValue(30)}, result);
	actor->play(kMethodAddFidget, {Core::ScriptValue(2), Core::ScriptValue(number(pose == 1 ? kArtistFidg1IDKey : kArtistFidg2IDKey))}, result);
	_host._actors[kArtistKey] = actor.release();
	return true;
}

bool Gallery::enter() {
	World &world = _host._world;
	_state = CLIP(world.get(kR1CN3StateKey), 1, 5);
	int edited = world.get(kR1CN3NumFramesKey);
	if ((_state == 2 && world.visits(Page::kPageCA2)) || _state == 3)
		setState(edited < number(kNumMinPagesKey) ? 3 : 4);
	bool offered = world.get(kR1CN3SaidAct4Key) != 0;
	if (!artist(_state < 5 && !offered ? 1 : 2))
		return false;
	auto sprite = [this](const Common::String &key, int depth) {
		return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), depth);
	};
	_easel = sprite(kASEQCN3EASELName, 2);
	_hat = sprite(kASEQCN3HATName, 75);
	_takeOff = sprite(kASEQCN3TAKEOFFName, 80);
	if (!_easel || !_hat || !_takeOff)
		return false;
	_hat->boxTouchy = true;
	_hat->visible = offered && _state < 5;
	_takeOff->visible = false;
	static constexpr int defaults[] = {
		18,
		18,
		14,
		1,
		1,
	};
	for (int i = 0; i < 5; i++) {
		_frames[i] = sprite(Common::String::format(kASEQCN3PICFRAME1SignedIndexFormat, i + 1), 15);
		if (!_frames[i])
			return false;
		_positions[i] = Common::Point(indexed(kPicXKey, i + 1), indexed(kPicYKey, i + 1));
		int saved = world.get(Common::String::format(kR1CA2PicMaxSignedIndexKey, i + 1));
		_maximum[i] = saved ? CLIP(saved, 1, 18) : defaults[i];
		if (i < 3) {
			for (int frame = 1; frame <= defaults[i]; frame++) {
				Common::ScopedPtr<Common::SeekableReadStream> stream(_host._vm.resources().open(
					Common::String::format(kSamplesSampleBSignedIndexISignedIndexDATPath, i + 1, frame)));
				auto *image = new Graphics::Surface();
				_samples[i].push_back(image);
				if (!stream || !loadImage(*stream, *image))
					return false;
			}
		}
	}
	_painting = CLIP(world.get(kR1CA2DatasetKey), 1, 5);
	highlight();
	if (world.get(kR1CN3HatPendingKey)) {
		_phase = kCollectSpeech;
	} else if (world.get(kR1CN3MoviePendingKey)) {
		_phase = kMovie;
	} else if (!world.visits(Page::kPageCN3)) {
		group(kEnterSpchIDsKey, kSsrName);
	} else if (world.get(kR1CN3FromStreetKey) || world.get(kPreviousLocationKey) == 2) {
		if (_state == 1)
			group(kEnterSpchIDsKey, kSsrName);
		else if (_state == 2)
			group(kReEnterSpchIDs2Key, kAsaName);
		else if (_state == 3) {
			say(kArtistKey, kReEnterSpchIDs31Key);
			_host.say(kArtistKey, indexed(kReEnterSpchIDs3Key, _host._vm.rnd().number(2, 3)));
			say(kArtistKey, kReEnterSpchIDs34Key);
		} else if (!world.get(kR1CN3SaidSS4Key)) {
			world.set(kR1CN3SaidSS4Key, 1);
			group(kReEnterSpchIDs4Key, kAaName);
		}
	} else if (_state == 3) {
		if (world.get(kR1CN31stEditKey) == 1 && 0 < edited) {
			world.set(kR1CN31stEditKey, 2);
			say(kReaderName, kReEnterSpchIDsA31Key);
		}
		artistSpeech();
	} else if (4 <= _state) {
		if (!offered) {
			group(kReEnterSpchIDsA4Key, kAaraName);
			_phase = kOfferSpeech;
		} else {
			_host.say(kArtistKey, indexed(kReEnterSpchIDsA5Key, world.visits(Page::kPageCN3) % 5 + 1));
		}
	}
	return true;
}

void Gallery::highlight() {
	for (int i = 0; i < 5; i++)
		_frames[i]->setFrame(i + 1 == _painting || i + 1 == _hover ? 2 : 1);
}

void Gallery::advance(uint32 time) {
	if (!_clockStarted) {
		_nextFrame = time + 126;
		_idle = time;
		_clockStarted = true;
	}
	if (_destination || _phase == kMovie || _phase == kBeforeMovie || static_cast<int32>(time - _nextFrame) < 0)
		return;
	_nextFrame = time + 126;
	_frame += 1;
	if (_maximum[_painting - 1] < _frame) {
		_frame = 1;
		do {
			_painting = _painting % 5 + 1;
		} while (3 < _painting && !_host._world.get(Common::String::format(kR1CA2PicUsedSignedIndexKey, _painting)));
		highlight();
	}
}

void Gallery::resume(uint32 time) {
	_nextFrame = time + 126;
	_idle = time;
}

void Gallery::pause(bool paused) {
	if (paused)
		_takeOffPaused = _takeOff->isRunning();
	if (_takeOffPaused)
		_takeOff->pauseAnimation(paused);
}

void Gallery::collect() {
	World &world = _host._world;
	if (world.get(kR1CN3HatPendingKey)) {
		Common::String key = Common::String::format(kItemSignedIndexFormat, number(kINV1HatKey));
		world.set(key, MIN(25, world.get(key) + 1));
		_host.putAway();
		world.set(kR1CN3HatPendingKey, 0);
	}
	_hat->visible = false;
	_phase = kIdle;
}

void Gallery::update(uint32 time) {
	if (_host._nextSpeech < _host._speech.size())
		return;
	if (_phase == kIntroduction) {
		setState(2);
		_phase = kIdle;
	} else if (_phase == kBeforeMovie) {
		_phase = kMovie;
	} else if (_phase == kOfferSpeech) {
		_host._actors[kArtistKey]->set(kPropertyVisible, {}, Core::ScriptValue(0));
		_takeOff->visible = true;
		_takeOff->startAnimation(1);
		_phase = kTakeOff;
	} else if (_phase == kTakeOff) {
		if (_takeOff->isRunning())
			return;
		artist(2);
		_takeOff->visible = false;
		_hat->visible = true;
		_host._world.set(kR1CN3SaidAct4Key, 1);
		say(kSamName, kReEnterSpchIDsA45Key);
		say(kArtistKey, kReEnterSpchIDsA46Key);
		if (_host._world.get(kTalkedToRockHeadKey))
			say(kReaderName, kReEnterSpchIDsA47Key);
		_phase = kIdle;
	} else if (_phase == kCollectSpeech) {
		collect();
	}
	if (_phase == kIdle && !_host._chestOpen && !_host._practice && 30000 <= time - _idle) {
		_idle = time;
		_idleIndex = _idleIndex % 3 + 1;
		_host.speeches(Common::String::format(kIdleCharIDSignedIndexKey, _idleIndex), Common::String::format(kIdleSpchIDSignedIndexKey, _idleIndex),
					   indexed(kMaxIdleIndexSpeechesKey, _idleIndex));
	}
}

void Gallery::artistSpeech() {
	if (_state == 1) {
		_host.speeches(kArtistCharIDs1Key, kArtistSpchIDs1Key, 5);
		_phase = kIntroduction;
	} else if (_state == 2) {
		say(kArtistKey, kArtistSpchIDs21Key);
		_host._world.set(kR1CN3MoviePendingKey, 1);
		_phase = kBeforeMovie;
	} else if (_state == 3) {
		int edited = _host._world.get(kR1CN3NumFramesKey);
		if (edited)
			_host.say(kArtistKey, indexed(kArtistSpchIDs31Key, _host._vm.rnd().number(1, 2)));
		if (!edited || _host._world.get(kR1CN3FromSampleKey))
			group(kArtistSpchIDs34Key, kAaName);
		else if (edited == 1)
			group(kArtistSpchIDs32Key, kAaaName);
		else if (edited < 6)
			group(kArtistSpchIDs33Key, kAaName);
	} else {
		group(kArtistSpchIDs4Key, kArName);
	}
}

void Gallery::mainSpeech() {
	if (_state == 1)
		artistSpeech();
	else if (_state < 4) {
		int choice = _host._vm.rnd().number(1, 2);
		_host.speeches(Common::String::format(kMainCharIDs2SignedIndexKey, choice), Common::String::format(kMainSpchIDs2SignedIndexKey, choice),
					   indexed(kMaxMain2SpeechesKey, choice));
	} else if (_state == 4 && _host._world.get(kTalkedToRockHeadKey))
		say(kReaderName, kMainSpchIDs41Key);
	else if (_host._vm.rnd().number(1, 2) == 1)
		group(kMainSpchIDs51Key, kSaName);
	else
		group(kMainSpchIDs52Key, kSrName);
}

void Gallery::hatClick() {
	_hatClicks += 1;
	_host.playSounds({static_cast<uint16>(number(kCOMSoftClickSfxIDKey))});
	if (_state == 1) {
		_hatClicks = 0;
		say(kSamName, kHatSpchIDs11Key);
		artistSpeech();
	} else if (_state < 4) {
		if (!_host._world.get(kTalkedToRockHeadKey))
			group(kHatSpchIDs31Key, kRaaasaaName);
		else if (_hatClicks == 1)
			group(kHatSpchIDs21Key, kRaaaName);
		else
			group(kHatSpchIDs22Key, kRaaName);
	} else if (_state == 4 && _hat->visible && !_host._practice) {
		setState(5);
		_host._world.set(kR1CN3HatPendingKey, 1);
		group(kHatSpchIDs4Key, kSarName);
		_phase = kCollectSpeech;
	}
}

void Gallery::choose(int painting) {
	_painting = painting;
	_frame = 1;
	_host._world.set(kR1CA2DatasetKey, painting);
	if (_state == 1) {
		group(kEaselSpchIDKey, kRraName);
		setState(2);
	}
	if (_state == 2)
		artistSpeech();
	else {
		_host._world.set(kR1CN3FromSampleKey, painting <= 3);
		_destination = 6;
	}
}

bool Gallery::click(Common::Point point) {
	_idle = _host._time;
	if (!_host._world.get(kR1CN3SaidAct4Key) && Common::Rect(510, 145, 590, 235).contains(point)) {
		hatClick();
		return true;
	}
	if (_hat->visible && _hat->covers(point)) {
		hatClick();
		return true;
	}
	for (const char *name : {kPaigeName, kReaderName, kSamName, kArtistKey}) {
		if (!_host._actors[name]->bounds().contains(point))
			continue;
		if (!strcmp(name, kPaigeName)) {
			int group;
			if (_state < 3)
				group = 1;
			else if (_state == 3)
				group = 2;
			else
				group = 3;
			_host.say(kPaigeName, indexed(kPaigeSpchIDsKey, group));
		} else if (!strcmp(name, kArtistKey))
			artistSpeech();
		else
			mainSpeech();
		return true;
	}
	const auto &exits = _host._page->exits();
	for (uint i = 0; i < exits.size(); i++) {
		if (!exits[i].rect.contains(point))
			continue;
		if (i == 0)
			_destination = 2;
		else
			choose(i == 1 ? 5 : i - 1);
		return true;
	}
	return false;
}

bool Gallery::event(const Common::Event &event) {
	if (event.type != Common::EVENT_MOUSEMOVE)
		return false;
	_hover = 0;
	_easel->setFrame(1);
	if (!busy()) {
		const auto &exits = _host._page->exits();
		for (uint i = 1; i < exits.size(); i++) {
			if (!exits[i].rect.contains(event.mouse))
				continue;
			if (i == 1)
				_easel->setFrame(2);
			else
				_hover = i - 1;
		}
	}
	highlight();
	return false;
}

void Gallery::draw(OM::Canvas &canvas) {
	_host._scene.drawBackground(canvas);
	_host._scene.drawSprites(canvas, INT_MIN, 15);
	for (int i = 0; i < 5; i++) {
		int frame = i + 1 == _painting ? _frame : 1;
		const auto &pixels = _host._world.artwork.frame(i + 1, frame).pixels;
		if (!pixels.empty()) {
			if (_editedFrame[i] != frame) {
				Artwork::render(pixels, _edited[i], true);
				_editedFrame[i] = frame;
			}
			canvas.blit(_edited[i], _positions[i].x, _positions[i].y);
		} else if (i < 3 && frame <= static_cast<int>(_samples[i].size())) {
			canvas.blit(*_samples[i][frame - 1], _positions[i].x, _positions[i].y);
		} else {
			canvas.surface().fillRect(Common::Rect(_positions[i].x, _positions[i].y, _positions[i].x + 151, _positions[i].y + 100),
									  canvas.surface().format.RGBToColor(255, 255, 255));
		}
	}
	_host._scene.drawSprites(canvas, 17, INT_MAX);
}

} // namespace RR1Cloud9
} // namespace Trinket
