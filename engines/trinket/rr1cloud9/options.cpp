/* ScummVM - Graphic Adventure Engine
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include "trinket/rr1cloud9/options.h"

#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

const int Options::kLocations[] = {
	5,
	17,
	19,
	18,
	12,
	11,
	13,
	6,
};
const char *const Options::kActivities[] = {
	"r1ca1",
	"r1ia1",
	"r1ia3",
	"r1ia2",
	"r1la2",
	"r1la1",
	"r1la3",
	"r1ca2",
};

Options::Options(TrinketEngine &vm, World &world, const Common::String &name, int location, bool practice) : _vm(vm), _world(world), _paths(vm.resources()), _scene(vm.resources()), _dialog(vm.resources()),
																											 _voice(vm.mixer()), _effect(vm.mixer()), _name(name), _location(location), _practice(practice) {}

Options::~Options() { clearText(); }

Common::String Options::prefix(int row) {
	Common::String value(kActivities[row]);
	value.toUppercase();
	return value;
}

const char *Options::movieFile(int index) {
	static const char *const files[] = {
		"r1mvop1.bik",
		"r1mvop2.bik",
		"rr1mvcq.bik",
		"r1mvdemo.bik",
		"r1mvcl1.bik",
		"r1mvclosing.bik",
	};
	return 0 <= index && index < 6 ? files[index] : "";
}

bool Options::enter() {
	Common::ScopedPtr<Common::SeekableReadStream> font(_vm.resources().open("common/1024.nfnt"));
	Common::ScopedPtr<Common::SeekableReadStream> small(_vm.resources().open("common/1018.nfnt"));
	_font.reset(font ? OM::NFNTFont::load(*font) : nullptr);
	_smallFont.reset(small ? OM::NFNTFont::load(*small) : nullptr);
	return _font && _smallFont && show(kMain);
}

void Options::clearText() {
	for (auto *item : _texts)
		delete item;
	_texts.clear();
}

Shell::Sprite *Options::add(int resource, int action, int x, int y) {
	auto *sprite = _scene.add(Common::String::format("%s/%d.ao", resource == 1853 ? "r1map" : "common", resource),
							  action ? 1 : 0, action, action != 0);
	if (sprite) {
		if (0 <= x)
			sprite->x = x;
		if (0 <= y)
			sprite->y = y;
	}
	return sprite;
}

void Options::text(const Common::String &value, const Common::Rect &rect, bool small) {
	auto *item = new Shell::TextObject(_paths, small ? _smallFont : _font);
	item->create({Core::ScriptValue::fromText(value), Core::ScriptValue(rect.left), Core::ScriptValue(rect.top),
				  Core::ScriptValue(rect.width()), Core::ScriptValue(rect.height()), Core::ScriptValue(0)});
	item->set("center", {}, Core::ScriptValue(1));
	item->set("color", {Core::ScriptValue(500), Core::ScriptValue(450)}, Core::ScriptValue(7500));
	_texts.push_back(item);
}

void Options::say(int resource) {
	_voice.stop();
	_voice.play(_vm.resources(), Common::String::format("comspch/%d.wav", resource), Audio::Mixer::kSpeechSoundType);
}

void Options::control(Shell::Sprite *sprite, uint rest, uint pressed) {
	if (!sprite)
		return;
	sprite->button = false;
	sprite->setFrame(rest);
	_controls.push_back({sprite, rest, pressed});
}

bool Options::show(Panel panel) {
	_voice.stop();
	_controls.clear();
	_scene.clear();
	clearText();
	_panel = panel;
	static const int backgrounds[] = {
		6000,
		6202,
		6102,
		6300,
	};
	if (!add(backgrounds[panel]))
		return false;
	if (panel == kMain) {
		for (int id : {6001, 6002, 6003, 6004, 6006, 6007})
			add(id, id);
		for (int id : {6010, 6014}) {
			auto *toggle = add(id, id);
			uint frame = _world.get(id == 6010 ? "isBackgroundMusicOff" : "isPrinterOff") ? 3 : 1;
			control(toggle, frame, frame + 1);
		}
	} else if (panel == kMovies) {
		for (int i = 0; i < 6; i++) {
			if (!_world.get(Common::String::format("movie:%d", i)))
				continue;
			add(6350 + i);
			add(6356 + i, 500 + i);
		}
		add(6008, 6007);
		add(6302, 1853);
		return true;
	} else {
		text(Common::String(panel == kLevels ? "Levels For " : "Progress Report For ") + (_practice ? "Practice Mode" : _name), Common::Rect(1, 1, 639, 30));
		static const int levelY[] = {
			72,
			112,
			153,
			194,
			234,
			275,
			315,
			356,
		};
		static const int reportY[] = {
			71,
			111,
			151,
			193,
			233,
			273,
			332,
			372,
		};
		static const int badgeY[] = {
			71,
			111,
			151,
			192,
			232,
			273,
		};
		static const int badges[] = {
			6114,
			6110,
			6112,
			6111,
		};
		for (int row = 0; row < 8; row++) {
			int y = panel == kLevels ? levelY[row] : reportY[row];
			bool enabled = _practice || _world.visits(kActivities[row]);
			auto *activity = add(6120, 100 + row, panel == kLevels ? 20 : 22, y);
			control(activity, enabled ? 1 : 3, enabled ? 2 : 3);
			if (_location == kLocations[row]) {
				auto *marker = add(6121, 0, 20, y - 2);
				if (marker)
					marker->z = 2;
			}
			if (panel == kLevels && row < 7) {
				int level = CLIP(_world.get(prefix(row) + "Level"), 1, 3);
				for (int n = 0; n < 3; n++) {
					auto *button = add(6210 + n, 200 + row * 3 + n, 406 + 38 * n, y);
					control(button, level == n + 1 ? 2 : 1, 2);
				}
				auto *automatic = add(6245, 300 + row, 571, y - 2);
				uint frame = _world.get(prefix(row) + "FixedLevel") ? 1 : 2;
				control(automatic, frame, 3 - frame);
			} else if (panel == kReport && row < 7) {
				for (int n = 1; n <= 3; n++) {
					Common::String key = Common::String::format("%sStats:%d:", prefix(row).c_str(), n);
					int x = 428 + 67 * (n - 1);
					if (row == 6) {
						text(Common::String::format("%d", _world.get(key + "correct")), Common::Rect(x, 327, x + 38, 368));
					} else if (_world.get(key + "total")) {
						int color = CLIP(_world.get(key + "color"), 0, 3);
						auto *badge = add(badges[color], 400 + color, x, badgeY[row]);
						if (badge)
							badge->button = false;
					}
				}
			} else if (panel == kReport) {
				int count = _world.get("R1CN3NumFrames");
				text(Common::String::format(count == 1 ? "%d Page Created" : "%d Pages Created", count), Common::Rect(425, 376, 617, 404), true);
			}
		}
		add(panel == kLevels ? 6250 : 6251, panel == kLevels ? 6002 : 6003);
		if (panel == kReport) {
			auto *print = add(6130, 6130);
			if (print)
				print->enabled = !_world.get("isPrinterOff");
		}
	}
	add(1853, 1853, 521, 422);
	return true;
}

void Options::setLevel(int row, int level) {
	Common::String key = prefix(row);
	_world.set(key + "Level", level);
	_world.set(key + "HistoryCount", 0);
	_world.set(key + "Streak", 0);
	_world.set(key + "StreakKind", 0);
	_world.set(key + "AssessmentCount", 0);
	if (_location == kLocations[row])
		_changedCurrent = true;
	show(kLevels);
}

Options::Request Options::activate(int action) {
	if (action == 1853) {
		if (_panel == kMain)
			return kClose;
		show(kMain);
	} else if (action == 6001)
		return _location == 0 ? kClose : kSignin;
	else if (action == 6007)
		return kQuit;
	else if (action == 6006)
		return kCredits;
	else if (action == 6002)
		show(kReport);
	else if (action == 6003)
		show(kLevels);
	else if (action == 6004)
		show(kMovies);
	else if (action == 6130 && !_world.get("isPrinterOff"))
		return kPrint;
	else if (action == 6010 || action == 6014) {
		Common::String key = action == 6010 ? "isBackgroundMusicOff" : "isPrinterOff";
		int off = !_world.get(key);
		_world.set(key, off);
		show(kMain);
		say((action == 6010 ? 6002 : 6021) - off);
	} else if (100 <= action && action < 108) {
		int row = action - 100;
		if (_practice || _world.visits(kActivities[row])) {
			_destination = kLocations[row];
			return kLocation;
		}
		say(6101);
	} else if (200 <= action && action < 221) {
		int row = (action - 200) / 3, level = (action - 200) % 3 + 1;
		if (!_world.visits(kActivities[row]))
			say(6113);
		else if (level != CLIP(_world.get(prefix(row) + "Level"), 1, 3)) {
			if (!_practice && _location == kLocations[row] && !_changedCurrent) {
				_pendingRow = row;
				_pendingLevel = level;
				_dialog.add("common/1905.ao", 0);
				_dialog.add("common/6140.ao", 1);
				_dialog.add("common/1861.ao", 2, 1, true);
				_dialog.add("common/1862.ao", 2, 2, true);
				say(6205);
			} else
				setLevel(row, level);
		}
	} else if (300 <= action && action < 307) {
		Common::String key = prefix(action - 300) + "FixedLevel";
		_world.set(key, !_world.get(key));
		show(kLevels);
	} else if (400 <= action && action < 404) {
		static const int voices[] = {
			6113,
			6110,
			6112,
			6111,
		};
		say(voices[action - 400]);
	} else if (500 <= action && action < 506 && _world.get(Common::String::format("movie:%d", action - 500))) {
		_movie = action - 500;
		return kMovie;
	}
	return kNone;
}

Options::Request Options::event(const Common::Event &event) {
	bool back = event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE;
	if (0 <= _pendingRow) {
		int action = back ? 2 : _dialog.event(event);
		if (action) {
			if (action == 1)
				setLevel(_pendingRow, _pendingLevel);
			_pendingRow = -1;
			_dialog.clear();
			_voice.stop();
		}
		return kNone;
	}
	int action = back ? 1853 : _scene.event(event);
	for (auto &item : _controls)
		item.sprite->setFrame(item.sprite->pressed ? item.pressed : item.rest);
	if (action)
		_effect.play(_vm.resources(), "common/1801.wav", Audio::Mixer::kSFXSoundType);
	return activate(action);
}

void Options::draw(OM::Canvas &canvas, uint32 time) {
	Common::Array<uint16> sounds;
	_scene.update(time, sounds);
	_scene.draw(canvas);
	for (auto *item : _texts)
		item->draw(canvas);
	if (0 <= _pendingRow) {
		_dialog.update(time, sounds);
		_dialog.drawSprites(canvas, INT_MIN, INT_MAX);
	}
}
} // namespace RR1Cloud9
} // namespace Trinket
