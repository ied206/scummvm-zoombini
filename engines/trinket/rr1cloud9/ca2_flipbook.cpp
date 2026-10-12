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

#include "trinket/rr1cloud9/ca2_flipbook.h"

#include "common/system.h"
#include "common/translation.h"
#include "graphics/cursorman.h"
#include "trinket/rr1cloud9/page_scene.h"
#include "trinket/rr1cloud9/print.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

Flipbook::~Flipbook() {
	releaseCursor();
	if (_ready)
		checkpoint();
	_image.free();
	_stampImage.free();
	for (auto &image : _colored)
		image.free();
	for (auto &image : _cursors)
		image.free();
}

Artwork &Flipbook::artwork() { return _host._practice ? _practiceArtwork : _host._world.artwork; }

int Flipbook::number(const Common::String &key) const { return _host._page->number(key); }

void Flipbook::used(const char *key) {
	if (!_host._practice)
		_host._world.set(Common::String("R1CA2") + key + "Used", 1);
}

void Flipbook::say(const char *actor, const Common::String &key) { _host.say(actor, number(key)); }

void Flipbook::group(const Common::String &key, const char *actors) {
	for (uint i = 0; actors[i]; i++)
		say(actors[i] == 'r' ? "reader" : actors[i] == 's' ? "sam"
														   : "paige",
			Common::String::format("%s.%u", key.c_str(), i + 1));
}

bool Flipbook::enter() {
	World &world = _host._world;
	_book = world.visits("r1ca2") ? CLIP(world.get("R1CA2Dataset"), 1, 5) : 4;
	_state = _book <= 3 ? 1 : 2;
	if (world.get(Common::String::format("R1CA2PicUsed%d", _book)))
		_state = 3;
	static constexpr int defaults[] = {
		18,
		18,
		14,
		1,
		1,
	};
	_maximum = world.get(Common::String::format("R1CA2PicMax%d", _book));
	_maximum = _maximum ? CLIP(_maximum, 1, 18) : defaults[_book - 1];
	auto sprite = [this](int id, int depth) {
		Shell::Sprite *s = _host._scene.add(_host.characterResource(id, Shell::Character::Resources::kAnimation), depth);
		if (s)
			s->boxTouchy = true;
		return s;
	};
	for (int i = 0; i < 12; i++) {
		_colors[i] = sprite(11190 + i, 10);
		if (!_colors[i])
			return false;
	}
	for (int i = 0; i < 6; i++) {
		_stamps[i] = sprite(11180 + i, 10);
		if (!_stamps[i])
			return false;
	}
	for (int i = 0; i < 3; i++) {
		_tools[i] = sprite(11150 + i * 2, 60);
		if (!_tools[i])
			return false;
	}
	_next = sprite(11215, 10);
	_previous = sprite(11216, 10);
	_play = sprite(11235, 10);
	_print = sprite(11225, 10);
	_bomb = sprite(11230, 10);
	_undoButton = sprite(11239, 10);
	_flip = sprite(11220, 50);
	if (!_next || !_previous || !_play || !_print || !_bomb || !_undoButton || !_flip)
		return false;
	_flip->visible = false;
	Common::ScopedPtr<Common::SeekableReadStream> font(_host._vm.resources().open("common/1018.nfnt"));
	_font.reset(font ? OM::NFNTFont::load(*font) : nullptr);
	if (!_font || !loadFrame())
		return false;
	static const char *const cursorKeys[] = {
		"kBrushCursor",
		"kFillCursor",
		"kEraserCursor",
		"kFingerCursorID",
	};
	for (uint i = 0; i < ARRAYSIZE(cursorKeys); i++) {
		Core::Animation animation;
		Common::String path = _host.characterResource(number(cursorKeys[i]), Shell::Character::Resources::kAnimation);
		Common::ScopedPtr<Common::SeekableReadStream> stream(_host._vm.resources().open(path));
		Common::ScopedPtr<Common::SeekableReadStream> palette(_host._vm.resources().open(path.substr(0, path.size() - 3) + ".rgb"));
		if (!stream || !animation.load(*stream) || (palette && !animation.loadPalette(*palette)) || !animation.decode(0, _cursors[i]))
			return false;
		if (i == 3)
			_cursorHotspots[i] = Common::Point(7, 1);
		else {
			if (!animation.celCount())
				return false;
			Common::Point first;
			uint pictures = 0;
			for (const auto &step : animation.sequence(0)) {
				if (step.picture < 0)
					continue;
				if (!pictures)
					first = Common::Point(step.x, step.y);
				else {
					_cursorHotspots[i] = Common::Point(step.x, step.y) - first;
					break;
				}
				pictures += 1;
			}
		}
	}
	for (const char *name : {"reader", "sam"})
		_host._actors[name]->set("animateSettled", {}, Core::ScriptValue(0));
	Common::String entry = Common::String::format("R1CA2Enter%dCount", _state);
	if (!world.get(entry))
		group(Common::String::format("kEnterSpchIDs.%d", _state), _state == 1 ? "r" : _state == 2 ? "sr"
																								  : "s");
	else {
		int choice = _host._vm.rnd().number(1, 3);
		Common::String suffix = Common::String::format(".%d.%d", _state, choice);
		_host.say(_host._page->text("kReEnterCharIDs" + suffix), number("kReEnterSpchIDs" + suffix));
	}
	if (!_host._practice) {
		world.set(entry, world.get(entry) + 1);
		world.set("R1CA2Dataset", _book);
	}
	_ready = true;
	buttons();
	return true;
}

bool Flipbook::loadFrame() {
	_dirty = _painted = false;
	_undo.clear();
	_onion.clear();
	if (!_playing && 1 < _frame && !artwork().image(_host._vm.resources(), _book, _frame - 1, _onion))
		return false;
	return artwork().image(_host._vm.resources(), _book, _frame, _pixels);
}

void Flipbook::checkpoint() {
	if (!_ready)
		return;
	auto &frame = artwork().frame(_book, _frame);
	if (_dirty) {
		frame.pixels = _pixels;
		_dirty = false;
	}
	if (_painted)
		frame.marked = true;
	for (int i = 1; i <= 18; i++)
		if (artwork().frame(_book, i).marked)
			_maximum = MAX(_maximum, i);
	if (_host._practice)
		return;
	World &world = _host._world;
	int count = artwork().markedCount();
	world.set("R1CN3NumFrames", count);
	world.set("R1CN3FromStreet", 0);
	world.set(Common::String::format("R1CA2PicMax%d", _book), _maximum);
	if (count) {
		world.set(Common::String::format("R1CA2PicUsed%d", _book), 1);
		if (!world.get("R1CN31stEdit"))
			world.set("R1CN31stEdit", 1);
	}
	if (1 < count)
		used("TwoPages");
}

void Flipbook::buttons() {
	_next->visible = !_playing && _frame < 18;
	_previous->visible = !_playing && 1 < _frame;
	auto frame = [this](Shell::Sprite *sprite, bool selected, bool stop) {
		uint value = 1;
		if (_pressed == sprite)
			value = 3;
		else if (sprite->covers(_mouse) || selected)
			value = 2;
		sprite->setFrame(value + (stop ? 3 : 0));
	};
	for (auto *sprite : {_next, _previous, _print, _bomb, _undoButton})
		frame(sprite, false, false);
	frame(_play, false, _playing);
	for (int i = 0; i < 3; i++)
		frame(_tools[i], _tool == i, false);
	for (int i = 0; i < 12; i++)
		frame(_colors[i], _color == i, false);
	for (int i = 0; i < 6; i++) {
		_stamps[i]->visible = _stamp != i;
		frame(_stamps[i], false, false);
	}
	if (_helpIndex == 1)
		_tools[0]->setFrame(_helpPhase == 2 ? 2 : 1);
	else if (_helpIndex == 2)
		_next->setFrame(2);
	else if (_helpIndex == 3)
		_tools[1]->setFrame(2);
	else if (_helpIndex == 4)
		for (auto *stamp : _stamps)
			stamp->setFrame(2);
	else if (_helpIndex == 5)
		_play->setFrame(2);
	for (int i = 0; i < 8; i++)
		coloredImage(*(i < 6 ? _stamps[i] : _tools[i - 6]), i < 6 ? 11180 + i : 11150 + (i - 6) * 2, _colored[i]);
	updateCursor();
}

void Flipbook::releaseCursor() {
	if (_cursor >= 0) {
		CursorMan.popCursor();
		_cursor = -1;
	}
	if (_cursorHidden) {
		CursorMan.showMouse(true);
		_cursorHidden = false;
	}
}

void Flipbook::updateCursor() {
	if (_paused) {
		releaseCursor();
		return;
	}
	bool hidden = _tool == kStamp && !_playing && !_turn;
	if (_cursorHidden != hidden) {
		CursorMan.showMouse(!hidden);
		_cursorHidden = hidden;
	}
	int cursor = -1;
	if (_tool != kStamp && Common::Rect(162, 83, 482, 295).contains(_mouse) && !_playing && !_turn)
		cursor = _tool;
	else {
		for (auto *sprite : {_next, _previous, _print, _bomb, _undoButton, _play})
			if (sprite->visible && sprite->covers(_mouse))
				cursor = 3;
		for (auto *sprite : _colors)
			if (sprite->covers(_mouse))
				cursor = 3;
		for (auto *sprite : _tools)
			if (sprite->covers(_mouse))
				cursor = 3;
		for (auto *sprite : _stamps)
			if (sprite->visible && sprite->covers(_mouse))
				cursor = 3;
	}
	if (cursor == _cursor)
		return;
	if (cursor < 0) {
		if (_cursor >= 0)
			CursorMan.popCursor();
	} else if (_cursor < 0)
		CursorMan.pushCursor(_cursors[cursor], _cursorHotspots[cursor].x, _cursorHotspots[cursor].y, 0);
	else
		CursorMan.replaceCursor(_cursors[cursor], _cursorHotspots[cursor].x, _cursorHotspots[cursor].y, 0);
	_cursor = cursor;
}

void Flipbook::stop() {
	if (!_playing)
		return;
	_playing = false;
	loadFrame();
	buttons();
}

void Flipbook::turn(int direction) {
	stop();
	if (busy() || _frame + direction < 1 || 18 < _frame + direction)
		return;
	checkpoint();
	_turn = direction;
	_flip->visible = true;
	_flip->startAnimation(1);
	_host.playSounds({static_cast<uint16>(number("kPageFlipID"))});
}

void Flipbook::advance(uint32 time) {
	if (_host._speaking || _host._nextSpeech < _host._speech.size() || busy() || _playing)
		_idleWaiting = false;
	if (_turn && !_flip->isRunning()) {
		_frame += _turn;
		bool forward = 0 < _turn;
		_turn = 0;
		_flip->visible = false;
		loadFrame();
		buttons();
		if (forward && !_host._practice) {
			int clicks = _host._world.get("R1CA2NextClicks");
			if (!clicks) {
				_host._world.set("R1CA2NextClicks", 1);
				group("kArrowSpchIDs.1", "psr");
			} else if (clicks == 1 && _frame == 3) {
				_host._world.set("R1CA2NextClicks", 2);
				say("reader", "kArrowSpchIDs.2.1");
			}
		}
	}
	if (_playing && 0 <= static_cast<int32>(time - _nextFrame)) {
		_nextFrame = time + 126;
		_frame = _frame % _maximum + 1;
		loadFrame();
	}
}

void Flipbook::update(uint32 time) {
	if (_host._nextSpeech < _host._speech.size())
		return;
	if (_helpPending) {
		_helpPending = false;
		static const char *const usedKeys[] = {
			"Paint",
			"TwoPages",
			"Fill",
			"Stamp",
			"Play",
		};
		for (uint i = 0; i < ARRAYSIZE(usedKeys); i++) {
			if (!_host._world.get(Common::String("R1CA2") + usedKeys[i] + "Used")) {
				_helpIndex = i + 1;
				_helpPhase = 1;
				_helpStart = time;
				buttons();
				break;
			}
		}
	}
	if (_helpIndex) {
		updateHelp(time);
		return;
	}
	if (!_idleWaiting) {
		_idle = time;
		_idleWaiting = true;
	}
	if (!busy() && !_playing && !_host._practice && 60000 <= time - _idle) {
		_idle = time;
		_idleWaiting = false;
		Common::String suffix = Common::String::format(".%u", _host._vm.rnd().number(1, 4));
		_host.say(_host._page->text("kIdleCharID" + suffix), number("kIdleSpchID" + suffix));
	}
}

void Flipbook::cancelHelp() {
	_helpIndex = _helpPhase = 0;
	_idleWaiting = false;
	buttons();
}

void Flipbook::updateHelp(uint32 time) {
	bool blink = _helpIndex == 1 && _tool == kPaint;
	if (_helpPhase == 1 && (!blink || 500 <= time - _helpStart)) {
		_helpPhase = 2;
		buttons();
		say("paige", Common::String::format("kHelpSpchIDs.%u", _helpIndex));
	} else if (_helpPhase == 2) {
		if (blink) {
			_helpPhase = 3;
			_helpStart = time;
			buttons();
		} else
			cancelHelp();
	} else if (_helpPhase == 3 && 500 <= time - _helpStart)
		cancelHelp();
}

void Flipbook::resume(uint32 time) {
	uint32 elapsed = time - _pauseStart;
	_nextFrame += elapsed;
	_idle += elapsed;
	_helpStart += elapsed;
}

void Flipbook::pause(bool paused) {
	_paused = paused;
	updateCursor();
	if (paused) {
		_pauseStart = _host._time;
		_dragging = false;
		checkpoint();
	}
	if (_turn)
		_flip->pauseAnimation(paused);
}

uint16 Flipbook::color() const {
	static constexpr byte red[] = {
		219,
		243,
		245,
		227,
		12,
		111,
		43,
		194,
		143,
		109,
		0,
		255,
	};
	static constexpr byte green[] = {
		16,
		181,
		121,
		219,
		111,
		206,
		50,
		194,
		70,
		79,
		0,
		255,
	};
	static constexpr byte blue[] = {
		45,
		183,
		12,
		35,
		33,
		123,
		181,
		237,
		195,
		59,
		0,
		255,
	};
	return ((red[_color] >> 3) << 10) | ((green[_color] >> 3) << 5) | (blue[_color] >> 3);
}

void Flipbook::selectTool(Tool tool) {
	_stamp = -1;
	_tool = tool;
	buttons();
}

bool Flipbook::stampImage() {
	uint frame = _stamps[_stamp]->getFrame();
	_stamps[_stamp]->setFrame(1);
	bool result = coloredImage(*_stamps[_stamp], 11180 + _stamp, _stampImage);
	_stamps[_stamp]->setFrame(frame);
	return result;
}

bool Flipbook::coloredImage(Shell::Sprite &sprite, int resource, Graphics::Surface &image) const {
	Common::ScopedPtr<Common::SeekableReadStream> rgb(_host._vm.resources().open(Common::String::format("r1ca2/%d.rgb", resource)));
	if (!rgb || rgb->size() != 1536)
		return false;
	byte palette[768];
	for (auto &component : palette) {
		component = rgb->readByte();
		rgb->readByte();
	}
	uint c = color();
	palette[9] = ((c >> 10) & 31) << 3;
	palette[10] = ((c >> 5) & 31) << 3;
	palette[11] = (c & 31) << 3;
	const Core::Animation &asset = *sprite.asset;
	image.free();
	return asset.decode(asset.pictureForFrame(MAX(1U, sprite.displayedFrame()) - 1), image, palette);
}

void Flipbook::stroke(Common::Point from, Common::Point to) {
	int dx = ABS(to.x - from.x), dy = -ABS(to.y - from.y), error = dx + dy;
	int sx = from.x < to.x ? 1 : -1, sy = from.y < to.y ? 1 : -1;
	int radius = _tool == kErase ? 8 : 2;
	uint16 ink = _tool == kErase ? Artwork::kWhite : color();
	for (;;) {
		for (int y = MAX(0, from.y - radius); y < MIN(Artwork::kVisibleHeight, from.y + radius); y++)
			for (int x = MAX(0, from.x - radius); x < MIN(Artwork::kWidth, from.x + radius); x++)
				if ((x - from.x) * (x - from.x) + (y - from.y) * (y - from.y) <= radius * radius)
					_pixels[y * Artwork::kWidth + x] = ink;
		if (from == to)
			break;
		int twice = error * 2;
		if (dy <= twice) {
			error += dy;
			from.x += sx;
		}
		if (twice <= dx) {
			error += dx;
			from.y += sy;
		}
	}
}

void Flipbook::fill(Common::Point point) {
	uint16 old = _pixels[point.y * Artwork::kWidth + point.x], ink = color();
	if (old == ink)
		return;
	Common::Array<int> pending;
	pending.push_back(point.y * Artwork::kWidth + point.x);
	_pixels[pending.back()] = ink;
	while (!pending.empty()) {
		int index = pending.back();
		pending.pop_back();
		int x = index % Artwork::kWidth, y = index / Artwork::kWidth;
		int neighbors[] = {
			x ? index - 1 : -1,
			x + 1 < Artwork::kWidth ? index + 1 : -1,
			y ? index - Artwork::kWidth : -1,
			y + 1 < Artwork::kVisibleHeight ? index + Artwork::kWidth : -1,
		};
		for (int next : neighbors)
			if (0 <= next && _pixels[next] == old) {
				_pixels[next] = ink;
				pending.push_back(next);
			}
	}
}

void Flipbook::stamp(Common::Point point) {
	int left = point.x - _stampImage.w / 2, top = point.y - _stampImage.h / 2;
	for (int y = MAX(0, top); y < MIN(Artwork::kVisibleHeight, top + _stampImage.h); y++)
		for (int x = MAX(0, left); x < MIN(Artwork::kWidth, left + _stampImage.w); x++) {
			byte a, r, g, b;
			_stampImage.format.colorToARGB(_stampImage.getPixel(x - left, y - top), a, r, g, b);
			if (a)
				_pixels[y * Artwork::kWidth + x] = ((r >> 3) << 10) | ((g >> 3) << 5) | (b >> 3);
		}
}

bool Flipbook::event(const Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN && (event.kbd.keycode == Common::KEYCODE_LEFT || event.kbd.keycode == Common::KEYCODE_RIGHT)) {
		turn(event.kbd.keycode == Common::KEYCODE_LEFT ? -1 : 1);
		return true;
	}
	if (event.type == Common::EVENT_MOUSEMOVE) {
		_mouse = event.mouse;
		buttons();
		if (_dragging) {
			Common::Point next = event.mouse - Common::Point(162, 83);
			stroke(_last, next);
			_last = next;
		}
		return _dragging;
	}
	if (event.type == Common::EVENT_LBUTTONUP) {
		_pressed = nullptr;
		buttons();
		bool handled = _dragging;
		_dragging = false;
		return handled;
	}
	if (event.type != Common::EVENT_LBUTTONDOWN || busy())
		return false;
	_mouse = event.mouse;
	Common::Rect bounds(162, 83, 482, 295);
	Common::Rect stampBounds(event.mouse.x - _stampImage.w / 2, event.mouse.y - _stampImage.h / 2,
							 event.mouse.x - _stampImage.w / 2 + _stampImage.w, event.mouse.y - _stampImage.h / 2 + _stampImage.h);
	if (_tool == kStamp ? !bounds.intersects(stampBounds) : !bounds.contains(event.mouse))
		return false;
	_host.interrupt();
	_helpPending = false;
	cancelHelp();
	_idle = _host._time;
	if (_playing) {
		stop();
		return true;
	}
	_undo = _pixels;
	_dirty = true;
	Common::Point point = event.mouse - Common::Point(162, 83);
	if (_tool == kStamp) {
		stamp(point);
		used("Stamp");
	} else if (_tool == kFill) {
		fill(point);
		used("Fill");
	} else {
		_last = point;
		_dragging = true;
		stroke(point, point + Common::Point(1, 1));
		used(_tool == kErase ? "Erase" : "Paint");
	}
	if (_tool != kErase)
		_painted = true;
	static const char *const sounds[] = {
		"kPaintSFXID",
		"kFillSFXID",
		"kEraseSFXID",
		"kStampSFXID",
	};
	_host.playSounds({static_cast<uint16>(number(sounds[_tool]))});
	return true;
}

void Flipbook::paige() {
	checkpoint();
	int count = artwork().markedCount(), choice;
	World &world = _host._world;
	if (1 < _frame && artwork().frame(_book, _frame - 1).marked && !world.get("R1CA2PaigeClicks")) {
		choice = 1;
		if (!_host._practice)
			world.set("R1CA2PaigeClicks", 1);
	} else if (count == 1)
		choice = _host._vm.rnd().number(2, 3);
	else
		choice = count < 4 && !_host._practice ? 4 : 5;
	say("paige", Common::String::format("kPaigeSpchIDs.%d", choice));
}

bool Flipbook::click(Common::Point point) {
	if (busy())
		return true;
	_idle = _host._time;
	_mouse = point;
	_helpPending = false;
	cancelHelp();
	_pressed = nullptr;
	for (auto *sprite : {_next, _previous, _print, _bomb, _undoButton, _play})
		if (sprite->visible && sprite->covers(point))
			_pressed = sprite;
	buttons();
	if (_play->covers(point)) {
		if (_playing)
			stop();
		else {
			checkpoint();
			_playing = true;
			_frame = 1;
			_nextFrame = _host._time;
			loadFrame();
			used("Play");
			buttons();
		}
		return true;
	}
	stop();
	if (_next->visible && _next->covers(point)) {
		turn(1);
		return true;
	}
	if (_previous->visible && _previous->covers(point)) {
		turn(-1);
		return true;
	}
	for (int i = 0; i < 12; i++)
		if (_colors[i]->covers(point)) {
			_color = i;
			if (_tool == kStamp)
				stampImage();
			buttons();
			_host.playSounds({static_cast<uint16>(number(Common::String::format("kColorSFXID.%d", i + 1)))});
			return true;
		}
	for (int i = 0; i < 3; i++)
		if (_tools[i]->covers(point)) {
			selectTool(static_cast<Tool>(i));
			return true;
		}
	for (int i = 0; i < 6; i++)
		if (_stamps[i]->visible && _stamps[i]->covers(point)) {
			if (_tool != kStamp)
				_previousTool = _tool;
			_stamp = i;
			if (stampImage())
				_tool = kStamp;
			else
				_stamp = -1;
			buttons();
			_host.playSounds({static_cast<uint16>(number("kPickShapeSFXID"))});
			return true;
		}
	if (_tool == kStamp)
		selectTool(_previousTool);
	if (_bomb->covers(point)) {
		Common::Array<uint16> previous = _pixels;
		artwork().frame(_book, _frame).pixels.clear();
		loadFrame();
		_undo = previous;
		used("Bomb");
		_host.playSounds({static_cast<uint16>(number("kExplodeSFXID"))});
		return true;
	}
	if (_undoButton->covers(point)) {
		if (!_undo.empty()) {
			SWAP(_pixels, _undo);
			_dirty = true;
		}
		_host.playSounds({static_cast<uint16>(number("kUndoSFXID"))});
		return true;
	}
	if (_print->covers(point)) {
		if (!_host._world.get("isPrinterOff")) {
			checkpoint();
			Print::book(artwork(), _host._vm.resources(), _book);
		}
		return true;
	}
	if (_host._actors["paige"]->bounds().contains(point)) {
		paige();
		return true;
	}
	if (_host._actors["reader"]->bounds().contains(point) || _host._actors["sam"]->bounds().contains(point)) {
		if (!_host._practice) {
			_mainClicks = _mainClicks % 7 + 1;
			static const char *const actors[] = {
				"s",
				"rs",
				"srr",
				"r",
				"s",
				"r",
				"s",
			};
			group(Common::String::format("kMainSpchIDs.%d", _mainClicks), actors[_mainClicks - 1]);
		}
		return true;
	}
	return false;
}

void Flipbook::draw(OM::Canvas &canvas) {
	_host._scene.drawBackground(canvas);
	if (!_turn) {
		Common::Array<uint16> display = _pixels;
		if (!_onion.empty())
			for (uint i = 0; i < display.size(); i++)
				if (display[i] == Artwork::kWhite && _onion[i] != Artwork::kWhite) {
					uint c = _onion[i];
					display[i] = (MIN(28U, ((c >> 10) & 31) + 25) << 10) | (MIN(28U, ((c >> 5) & 31) + 25) << 5) |
								 MIN(28U, (c & 31) + 25);
				}
		Artwork::render(display, _image);
		canvas.blit(_image, 162, 83);
	}
	_host._scene.drawSprites(canvas, INT_MIN, 10);
	for (int i = 0; i < 6; i++)
		if (_stamps[i]->visible && _colored[i].getPixels())
			canvas.blit(_colored[i], _stamps[i]->x, _stamps[i]->y);
	_host._scene.drawSprites(canvas, 11, 60);
	for (int i = 0; i < 2; i++)
		if (_colored[i + 6].getPixels())
			canvas.blit(_colored[i + 6], _tools[i]->x, _tools[i]->y);
	_host._scene.drawSprites(canvas, 61, INT_MAX);
	_font->drawString(&canvas.surface(), Common::String::format("%d", _frame), 295, 48, 60,
					  canvas.surface().format.RGBToColor(6317 >> 8, 4212 >> 8, 2106 >> 8), Graphics::kTextAlignCenter);
	if (_tool == kStamp && !_playing && !_turn)
		canvas.blit(_stampImage, _mouse.x - _stampImage.w / 2, _mouse.y - _stampImage.h / 2);
}
} // namespace RR1Cloud9
} // namespace Trinket
