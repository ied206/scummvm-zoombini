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

#ifndef TRINKET_RR1CLOUD9_SIGNIN_H
#define TRINKET_RR1CLOUD9_SIGNIN_H

#include "trinket/ompp/font.h"
#include "trinket/ompp/sound.h"
#include "trinket/shell/scene.h"

namespace Trinket {

class TrinketEngine;
namespace RR1Cloud9 {

/** RR's sorted name list, typed-prefix matching and sign-in dialogue. */
class Signin {
public:
	enum Request {
		kNone,
		kStart,
		kPractice,
		kQuit,
		kDelete,
	};
	Signin(TrinketEngine &vm, Shell::Scene &scene);
	bool enter();
	Request event(const Common::Event &event, uint32 time);
	Request update();
	void draw(OM::Canvas &canvas, uint32 time);
	Common::String selectedName() const;
	const Common::String &deletedName() const { return _deleted; }
	void refresh();
	void stopSound();

private:
	enum Mode {
		kBrowse00 = 0,
		kMatch01 = 1,
		kDraft02 = 2,
		kNew03 = 3,
	};
	TrinketEngine &_vm;
	Shell::Scene &_scene;
	Shell::Scene _dialog;
	OM::Sound _voice, _effect;
	Common::ScopedPtr<OM::NFNTFont> _font;
	Common::Array<Common::String> _entries;
	Common::String _typed, _deleted;
	Common::Array<Common::String> _speech;
	Shell::Sprite *_start = nullptr, *_new = nullptr;
	Shell::Sprite *_dialogActors[2] = {};
	Mode _mode = kBrowse00;
	int _selected = 0, _first = 0;
	int _dialogKind = 0;
	int _lastClickRow = -1;
	uint32 _lastClickTime = 0;
	bool _valid = false, _starting = false;
	static constexpr int kMaxNames = 99, kMaxLength = 16, kRows = 3;
	int _rowHeight = 20;
	void centerSelection();
	void buttons();
	void newEntry();
	void addCharacter(char character);
	void deleteCharacter();
	void move(int direction, bool page);
	void start();
	void dialog(int kind);
	void speak(uint16 id);
	void rematch();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
