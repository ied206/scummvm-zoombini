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

Gallery::~Gallery() {
	for (auto &image : _edited)
		image.free();
	for (auto &book : _samples)
		for (auto *image : book) {
			image->free();
			delete image;
		}
}

int Gallery::number(const Common::String &key) const { return _host._page->number(key); }

int Gallery::indexed(const char *key, int index) const { return number(Common::String::format("%s.%d", key, index)); }

void Gallery::say(const char *actorName, const Common::String &key) { _host.say(actorName, number(key)); }

void Gallery::group(const char *key, const char *actors) {
	for (uint i = 0; actors[i]; i++) {
		const char *actorName = actors[i] == 'r' ? "reader" : actors[i] == 's' ? "sam"
																			   : "artist";
		say(actorName, Common::String::format("%s.%u", key, i + 1));
	}
}

void Gallery::setState(int state) {
	_state = state;
	if (!_host._practice)
		_host._world.set("R1CN3State", state);
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
				uint r = (color >> 10) & 31, g = (color >> 5) & 31, b = color & 31;
				image.setPixel(x * 151 / width, targetY, image.format.RGBToColor((r << 3) | (r >> 2), (g << 3) | (g >> 2), (b << 3) | (b >> 2)));
			}
		}
	}
	return !stream.err() && !stream.eos();
}

bool Gallery::artist(int pose) {
	if (_host._actors.contains("artist")) {
		delete _host._actors["artist"];
		_host._actors.erase("artist");
	}
	Common::ScopedPtr<Shell::Character> actor(new Shell::Character(_host._scene, _host._vm.mixer(), _host._vm.resources(), _host._vm.rnd(), _host));
	if (!actor->create(indexed("kArtistSetID", pose), number("kArtistZ")))
		return false;
	actor->movable = false;
	for (const char *key : {"registerFidgetsToSettle", "registerSpeechToSettle", "loadSpeechAnimsInBkgd", "animateSettled"})
		actor->set(key, {}, Core::ScriptValue(0));
	Core::ScriptValue result;
	actor->play("setSpeechAnims", {Core::ScriptValue(indexed("kArtistLipsID", pose)), Core::ScriptValue(pose == 1 ? 0 : 10), Core::ScriptValue(1), Core::ScriptValue(number("kArtistBodyID")), Core::ScriptValue(0), Core::ScriptValue(0)}, result);
	actor->play("addFidgetGroup", {Core::ScriptValue(1), Core::ScriptValue(5), Core::ScriptValue(10)}, result);
	actor->play("addFidget", {Core::ScriptValue(1), Core::ScriptValue(indexed("kArtistSetID", pose))}, result);
	actor->play("addFidgetGroup", {Core::ScriptValue(2), Core::ScriptValue(15), Core::ScriptValue(30)}, result);
	actor->play("addFidget", {Core::ScriptValue(2), Core::ScriptValue(number(pose == 1 ? "kArtistFidg1ID" : "kArtistFidg2ID"))}, result);
	_host._actors["artist"] = actor.release();
	return true;
}

bool Gallery::enter() {
	World &world = _host._world;
	_state = CLIP(world.get("R1CN3State"), 1, 5);
	int edited = world.get("R1CN3NumFrames");
	if ((_state == 2 && world.visits("r1ca2")) || _state == 3)
		setState(edited < number("kNumMinPages") ? 3 : 4);
	bool offered = world.get("R1CN3SaidAct4") != 0;
	if (!artist(_state < 5 && !offered ? 1 : 2))
		return false;
	auto sprite = [this](const Common::String &key, int depth) {
		return _host._scene.add(_host.characterResource(number(key), Shell::Character::Resources::kAnimation), depth);
	};
	_easel = sprite("ASEQ_CN3EASEL", 2);
	_hat = sprite("ASEQ_CN3HAT", 75);
	_takeOff = sprite("ASEQ_CN3TAKEOFF", 80);
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
		_frames[i] = sprite(Common::String::format("ASEQ_CN3PICFRAME1.%d", i + 1), 15);
		if (!_frames[i])
			return false;
		_positions[i] = Common::Point(indexed("kPicX", i + 1), indexed("kPicY", i + 1));
		int saved = world.get(Common::String::format("R1CA2PicMax%d", i + 1));
		_maximum[i] = saved ? CLIP(saved, 1, 18) : defaults[i];
		if (i < 3) {
			for (int frame = 1; frame <= defaults[i]; frame++) {
				Common::ScopedPtr<Common::SeekableReadStream> stream(_host._vm.resources().open(
					Common::String::format("samples/SampleB%dI%02d.DAT", i + 1, frame)));
				auto *image = new Graphics::Surface();
				_samples[i].push_back(image);
				if (!stream || !loadImage(*stream, *image))
					return false;
			}
		}
	}
	_painting = CLIP(world.get("R1CA2Dataset"), 1, 5);
	highlight();
	if (world.get("R1CN3HatPending")) {
		_phase = kCollectSpeech;
	} else if (world.get("R1CN3MoviePending")) {
		_phase = kMovie;
	} else if (!world.visits("r1cn3")) {
		group("kEnterSpchIDs", "ssr");
	} else if (world.get("R1CN3FromStreet") || world.get("previousLocation") == 2) {
		if (_state == 1)
			group("kEnterSpchIDs", "ssr");
		else if (_state == 2)
			group("kReEnterSpchIDs.2", "asa");
		else if (_state == 3) {
			say("artist", "kReEnterSpchIDs.3.1");
			_host.say("artist", indexed("kReEnterSpchIDs.3", _host._vm.rnd().number(2, 3)));
			say("artist", "kReEnterSpchIDs.3.4");
		} else if (!world.get("R1CN3SaidSS4")) {
			world.set("R1CN3SaidSS4", 1);
			group("kReEnterSpchIDs.4", "aa");
		}
	} else if (_state == 3) {
		if (world.get("R1CN31stEdit") == 1 && 0 < edited) {
			world.set("R1CN31stEdit", 2);
			say("reader", "kReEnterSpchIDsA.3.1");
		}
		artistSpeech();
	} else if (4 <= _state) {
		if (!offered) {
			group("kReEnterSpchIDsA.4", "aara");
			_phase = kOfferSpeech;
		} else {
			_host.say("artist", indexed("kReEnterSpchIDsA.5", world.visits("r1cn3") % 5 + 1));
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
		} while (3 < _painting && !_host._world.get(Common::String::format("R1CA2PicUsed%d", _painting)));
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
	if (world.get("R1CN3HatPending")) {
		Common::String key = Common::String::format("item:%d", number("kINV1Hat"));
		world.set(key, MIN(25, world.get(key) + 1));
		_host.putAway();
		world.set("R1CN3HatPending", 0);
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
		_host._actors["artist"]->set("visible", {}, Core::ScriptValue(0));
		_takeOff->visible = true;
		_takeOff->startAnimation(1);
		_phase = kTakeOff;
	} else if (_phase == kTakeOff) {
		if (_takeOff->isRunning())
			return;
		artist(2);
		_takeOff->visible = false;
		_hat->visible = true;
		_host._world.set("R1CN3SaidAct4", 1);
		say("sam", "kReEnterSpchIDsA.4.5");
		say("artist", "kReEnterSpchIDsA.4.6");
		if (_host._world.get("talkedToRockHead"))
			say("reader", "kReEnterSpchIDsA.4.7");
		_phase = kIdle;
	} else if (_phase == kCollectSpeech) {
		collect();
	}
	if (_phase == kIdle && !_host._chestOpen && !_host._practice && 30000 <= time - _idle) {
		_idle = time;
		_idleIndex = _idleIndex % 3 + 1;
		_host.speeches(Common::String::format("kIdleCharID.%d", _idleIndex), Common::String::format("kIdleSpchID.%d", _idleIndex),
					   indexed("kMaxIdleIndexSpeeches", _idleIndex));
	}
}

void Gallery::artistSpeech() {
	if (_state == 1) {
		_host.speeches("kArtistCharIDs.1", "kArtistSpchIDs.1", 5);
		_phase = kIntroduction;
	} else if (_state == 2) {
		say("artist", "kArtistSpchIDs.2.1");
		_host._world.set("R1CN3MoviePending", 1);
		_phase = kBeforeMovie;
	} else if (_state == 3) {
		int edited = _host._world.get("R1CN3NumFrames");
		if (edited)
			_host.say("artist", indexed("kArtistSpchIDs.3.1", _host._vm.rnd().number(1, 2)));
		if (!edited || _host._world.get("R1CN3FromSample"))
			group("kArtistSpchIDs.3.4", "aa");
		else if (edited == 1)
			group("kArtistSpchIDs.3.2", "aaa");
		else if (edited < 6)
			group("kArtistSpchIDs.3.3", "aa");
	} else {
		group("kArtistSpchIDs.4", "ar");
	}
}

void Gallery::mainSpeech() {
	if (_state == 1)
		artistSpeech();
	else if (_state < 4) {
		int choice = _host._vm.rnd().number(1, 2);
		_host.speeches(Common::String::format("kMainCharIDs.2.%d", choice), Common::String::format("kMainSpchIDs.2.%d", choice),
					   indexed("kMaxMain2Speeches", choice));
	} else if (_state == 4 && _host._world.get("talkedToRockHead"))
		say("reader", "kMainSpchIDs.4.1");
	else if (_host._vm.rnd().number(1, 2) == 1)
		group("kMainSpchIDs.5.1", "sa");
	else
		group("kMainSpchIDs.5.2", "sr");
}

void Gallery::hatClick() {
	_hatClicks += 1;
	_host.playSounds({static_cast<uint16>(number("kCOMSoftClickSfxID"))});
	if (_state == 1) {
		_hatClicks = 0;
		say("sam", "kHatSpchIDs.1.1");
		artistSpeech();
	} else if (_state < 4) {
		if (!_host._world.get("talkedToRockHead"))
			group("kHatSpchIDs.3.1", "raaasaa");
		else if (_hatClicks == 1)
			group("kHatSpchIDs.2.1", "raaa");
		else
			group("kHatSpchIDs.2.2", "raa");
	} else if (_state == 4 && _hat->visible && !_host._practice) {
		setState(5);
		_host._world.set("R1CN3HatPending", 1);
		group("kHatSpchIDs.4", "sar");
		_phase = kCollectSpeech;
	}
}

void Gallery::choose(int painting) {
	_painting = painting;
	_frame = 1;
	_host._world.set("R1CA2Dataset", painting);
	if (_state == 1) {
		group("kEaselSpchID", "rra");
		setState(2);
	}
	if (_state == 2)
		artistSpeech();
	else {
		_host._world.set("R1CN3FromSample", painting <= 3);
		_destination = 6;
	}
}

bool Gallery::click(Common::Point point) {
	_idle = _host._time;
	if (!_host._world.get("R1CN3SaidAct4") && Common::Rect(510, 145, 590, 235).contains(point)) {
		hatClick();
		return true;
	}
	if (_hat->visible && _hat->covers(point)) {
		hatClick();
		return true;
	}
	for (const char *name : {"paige", "reader", "sam", "artist"}) {
		if (!_host._actors[name]->bounds().contains(point))
			continue;
		if (!strcmp(name, "paige"))
			_host.say("paige", indexed("kPaigeSpchIDs", _state < 3 ? 1 : _state == 3 ? 2
																					 : 3));
		else if (!strcmp(name, "artist"))
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
