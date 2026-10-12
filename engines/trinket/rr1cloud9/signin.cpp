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

#include "trinket/rr1cloud9/signin.h"

#include "common/algorithm.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace RR1Cloud9 {

Signin::Signin(TrinketEngine &vm, Shell::Scene &scene) : _vm(vm), _scene(scene), _dialog(vm.resources()),
														 _voice(vm.mixer()), _effect(vm.mixer()) {}

bool Signin::enter() {
	_scene.clear();
	if (!_scene.setBackground("signin/5000.bk16"))
		return false;
	for (int i = 1; i <= 34; i++) {
		int id = i <= 6 ? 5003 + i : 5004 + i;
		if (i == 34)
			id = 5065;
		auto *button = _scene.add(Common::String::format("sipre/%d.ao", id), 1, i, true);
		if (!button)
			return false;
		if (i == 4)
			_new = button;
		if (i == 5)
			_start = button;
	}
	Common::ScopedPtr<Common::SeekableReadStream> font(_vm.resources().open("common/1018.nfnt"));
	if (font)
		_font.reset(OM::NFNTFont::load(*font));
	if (!_font)
		return false;
	_rowHeight = _font->ascent() + _font->descent() + _font->getFontLeading() - 3;
	refresh();
	speak(5100);
	return true;
}

void Signin::refresh() {
	_entries = _vm.playerNames();
	Common::sort(_entries.begin(), _entries.end());
	_selected = CLIP(_selected, 0, MAX(0, static_cast<int>(_entries.size()) - 1));
	_first = MIN(_first, MAX(0, static_cast<int>(_entries.size()) - kRows));
	_typed.clear();
	_mode = kBrowse00;
	_valid = !_entries.empty();
	buttons();
}

Common::String Signin::selectedName() const {
	return _valid && _selected < static_cast<int>(_entries.size()) ? _entries[_selected] : Common::String();
}

void Signin::centerSelection() {
	if (_selected < _first || _first + kRows <= _selected || static_cast<int>(_entries.size()) - kRows < _first)
		_first = CLIP(_selected - kRows / 2, 0, MAX(0, static_cast<int>(_entries.size()) - kRows));
}

void Signin::buttons() {
	if (_start)
		_start->enabled = _valid;
	if (_new)
		_new->enabled = _mode < kDraft02 && _entries.size() < kMaxNames;
}

void Signin::newEntry() {
	if (kMaxNames <= _entries.size()) {
		dialog(2);
		return;
	}
	if (kDraft02 <= _mode)
		return;
	if (_mode == kBrowse00) {
		_typed.clear();
		_selected = _first = 0;
	}
	_valid = !_typed.empty() && (_entries.empty() || _typed != _entries[_selected]);
	_entries.insert_at(_selected, _typed);
	_mode = kNew03;
	buttons();
}

void Signin::rematch() {
	int row = 0;
	while (row < static_cast<int>(_entries.size()) && _entries[row].compareTo(_typed) < 0 && !_entries[row].hasPrefix(_typed))
		row += 1;
	_selected = row;
	if (row < static_cast<int>(_entries.size()) && _entries[row].hasPrefix(_typed)) {
		_mode = kMatch01;
		_valid = true;
	} else {
		_entries.insert_at(row, _typed);
		_mode = kDraft02;
		_valid = true;
	}
	centerSelection();
}

void Signin::addCharacter(char character) {
	if (kMaxLength <= _typed.size() || (character == ' ' && (_typed.empty() || _typed.lastChar() == ' ')))
		return;
	if ('a' <= character && character <= 'z' && (_typed.empty() || _typed.lastChar() == ' '))
		character -= 'a' - 'A';
	_typed += character;
	if (_mode < kDraft02) {
		if (kMaxNames <= _entries.size()) {
			bool match = false;
			for (const auto &entry : _entries)
				match |= entry.hasPrefix(_typed);
			if (!match) {
				_typed.deleteLastChar();
				dialog(2);
				return;
			}
		}
		rematch();
	} else {
		_entries[_selected] = _typed;
		_valid = true;
		if (_mode == kNew03)
			for (uint i = 0; i < _entries.size(); i++)
				if (static_cast<int>(i) != _selected && _entries[i] == _typed)
					_valid = false;
	}
	buttons();
	if ('A' <= character && character <= 'Z')
		character += 'a' - 'A';
	if ('a' <= character && character <= 'z')
		speak(5002 + character - 'a');
}

void Signin::deleteCharacter() {
	if (_typed.empty() && _mode != kNew03)
		return;
	if (!_typed.empty())
		_typed.deleteLastChar();
	if (_mode == kDraft02 || (_mode == kNew03 && _typed.empty()))
		_entries.remove_at(_selected);
	if (_typed.empty()) {
		_mode = kBrowse00;
		_selected = CLIP(_selected, 0, MAX(0, static_cast<int>(_entries.size()) - 1));
		_valid = !_entries.empty();
	} else if (_mode != kNew03) {
		rematch();
	} else {
		_entries[_selected] = _typed;
		_valid = true;
		for (uint i = 0; i < _entries.size(); i++)
			if (static_cast<int>(i) != _selected && _entries[i] == _typed)
				_valid = false;
	}
	centerSelection();
	buttons();
	speak(5055);
}

void Signin::move(int direction, bool page) {
	if (_mode != kBrowse00 || _entries.empty())
		return;
	if (page) {
		_first = CLIP(_first + direction * kRows, 0, MAX(0, static_cast<int>(_entries.size()) - kRows));
		_selected = CLIP(_selected, _first, MIN(_first + kRows, static_cast<int>(_entries.size())) - 1);
	} else {
		_selected = CLIP(_selected + direction, 0, static_cast<int>(_entries.size()) - 1);
		centerSelection();
	}
}

void Signin::stopSound() {
	_voice.stop();
	_speech.clear();
}

void Signin::speak(uint16 id) {
	stopSound();
	_voice.play(_vm.resources(), Common::String::format("signin/%u.wav", id), Audio::Mixer::kSpeechSoundType, 1, 77);
}

void Signin::start() {
	if (!_valid)
		return;
	speak(5000);
	Common::ScopedPtr<Common::SeekableReadStream> names(_vm.resources().open("signinnames/8888.txt"));
	if (names) {
		while (names->pos() < names->size()) {
			Common::String line = names->readLine();
			uint separator = line.find('%');
			if (separator == Common::String::npos || !line.substr(0, separator).equalsIgnoreCase(selectedName()))
				continue;
			Common::String id = line.substr(separator + 1);
			id.trim();
			_speech.push_back("signinnames/" + id + ".wav");
			break;
		}
	}
	_speech.push_back("signin/5001.wav");
	_starting = true;
}

Signin::Request Signin::update() {
	if (_voice.isPlaying())
		return kNone;
	if (!_speech.empty()) {
		Common::String resource = _speech.front();
		_speech.remove_at(0);
		_voice.play(_vm.resources(), resource, Audio::Mixer::kSpeechSoundType, 1, 77);
	} else if (_starting) {
		_starting = false;
		return kStart;
	}
	return kNone;
}

void Signin::dialog(int kind) {
	_dialog.clear();
	_dialogActors[0] = _dialogActors[1] = nullptr;
	_dialogKind = kind;
	if (kind == 1) {
		_deleted = selectedName();
		_dialog.add("common/1905.ao", 0);
		_dialog.add("signin/5150.ao", 1);
		_dialog.add("common/1861.ao", 3, 1, true);
		_dialog.add("common/1862.ao", 3, 2, true);
		for (int i = 0; i < 2; i++) {
			_dialogActors[i] = _dialog.add(Common::String::format("common/%d.ao", 1902 + i), 2, i + 1);
			if (_dialogActors[i]) {
				_dialogActors[i]->startAnimation(-1);
				_dialogActors[i]->pauseAnimation(true);
			}
		}
		speak(5150);
	} else {
		_dialog.add("signin/5152.ao", 0);
		_dialog.add("common/1863.ao", 2, 2, true);
		speak(5152);
	}
}

Signin::Request Signin::event(const Common::Event &event, uint32 time) {
	if (_dialogKind) {
		bool hovered[2] = {
			_dialogActors[0] && _dialogActors[0]->hovered,
			_dialogActors[1] && _dialogActors[1]->hovered,
		};
		int action = _dialog.event(event);
		for (int i = 0; i < 2; i++) {
			auto *actor = _dialogActors[i];
			if (!actor || actor->hovered == hovered[i])
				continue;
			if (actor->hovered)
				actor->playAnimation();
			else {
				actor->pauseAnimation(true);
				actor->setFrame(1);
			}
		}
		if (event.type == Common::EVENT_KEYDOWN && event.kbd.keycode == Common::KEYCODE_ESCAPE)
			action = 2;
		if (!action)
			return kNone;
		bool remove = _dialogKind == 1 && action == 1;
		_dialogKind = 0;
		_dialog.clear();
		_dialogActors[0] = _dialogActors[1] = nullptr;
		stopSound();
		return remove ? kDelete : kNone;
	}
	if (_starting) {
		if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN) {
			stopSound();
			_starting = false;
			return kStart;
		}
		return kNone;
	}
	int action = _scene.event(event);
	if (event.type == Common::EVENT_KEYDOWN || event.type == Common::EVENT_LBUTTONDOWN)
		stopSound();
	if (action)
		_effect.play(_vm.resources(), "common/1801.wav", Audio::Mixer::kSFXSoundType, 1, 65);
	if (action == 1)
		move(-1, false);
	else if (action == 2)
		move(1, false);
	else if (action == 3)
		return kQuit;
	else if (action == 4)
		newEntry();
	else if (action == 5)
		start();
	else if (action == 6)
		return kPractice;
	else if (7 <= action && action <= 32)
		addCharacter('a' + action - 7);
	else if (action == 33)
		addCharacter(' ');
	else if (action == 34)
		deleteCharacter();
	if (event.type == Common::EVENT_LBUTTONDOWN && _mode < kDraft02 && Common::Rect(119, 126, 464, 192).contains(event.mouse)) {
		int row = _first + (event.mouse.y - 126) / _rowHeight;
		if (row < static_cast<int>(_entries.size())) {
			_selected = row;
			_mode = kBrowse00;
			_typed.clear();
			_valid = true;
			buttons();
			if (_lastClickRow == row && time - _lastClickTime < 500)
				start();
			_lastClickRow = row;
			_lastClickTime = time;
		}
	}
	if (event.type == Common::EVENT_KEYDOWN) {
		auto key = event.kbd.keycode;
		bool command = event.kbd.flags & (Common::KBD_CTRL | Common::KBD_META);
		if (key == Common::KEYCODE_ESCAPE || (command && key == Common::KEYCODE_q))
			return kQuit;
		if (command && key == Common::KEYCODE_r && _valid && _mode < kDraft02)
			dialog(1);
		else if (key == Common::KEYCODE_BACKSPACE || key == Common::KEYCODE_DELETE)
			deleteCharacter();
		else if (key == Common::KEYCODE_RETURN || key == Common::KEYCODE_KP_ENTER)
			start();
		else if (key == Common::KEYCODE_UP || key == Common::KEYCODE_DOWN)
			move(key == Common::KEYCODE_UP ? -1 : 1, false);
		else if (key == Common::KEYCODE_PAGEUP || key == Common::KEYCODE_PAGEDOWN)
			move(key == Common::KEYCODE_PAGEUP ? -1 : 1, true);
		else if (!command && !(event.kbd.flags & Common::KBD_ALT)) {
			uint c = event.kbd.ascii;
			if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == ' ' || c == '-' || c == '.')
				addCharacter(c);
		}
	}
	return kNone;
}

void Signin::draw(OM::Canvas &canvas, uint32 time) {
	Common::Array<uint16> sounds;
	_scene.update(time, sounds);
	_scene.draw(canvas);
	auto &surface = canvas.surface();
	surface.fillRect(Common::Rect(119, 126, 464, 192), surface.format.RGBToColor(255, 255, 255));
	for (int i = 0; i < kRows && _first + i < static_cast<int>(_entries.size()); i++) {
		bool selected = _first + i == _selected;
		int y = 126 + i * _rowHeight;
		if (selected)
			surface.fillRect(Common::Rect(119, y + _font->descent(), 464, y + _font->descent() + _rowHeight),
							 surface.format.RGBToColor(165, 232, 255));
		y += _rowHeight - _font->ascent();
		uint32 color = surface.format.RGBToColor(12, 25, 35);
		if (selected)
			color = _mode < kDraft02 ? surface.format.RGBToColor(7, 56, 112) : surface.format.RGBToColor(107, 7, 5);
		_font->drawString(&surface, _entries[_first + i], 129, y, 335, color);
		if (selected && _mode == kMatch01)
			_font->drawString(&surface, _typed, 129, y, 335, surface.format.RGBToColor(107, 7, 5));
	}
	if (_dialogKind) {
		_dialog.update(time, sounds);
		_dialog.drawSprites(canvas, 0, 3);
	}
}
} // namespace RR1Cloud9
} // namespace Trinket
