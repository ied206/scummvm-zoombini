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

#ifndef TRINKET_RR1CLOUD9_OPTIONS_H
#define TRINKET_RR1CLOUD9_OPTIONS_H

#include "trinket/ompp/sound.h"
#include "trinket/rr1cloud9/world.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {

class TrinketEngine;
namespace RR1Cloud9 {

/** RR1's modal settings, activity selection and earned progress panels. */
class Options {
public:
	enum Request {
		kNone,
		kClose,
		kSignin,
		kQuit,
		kCredits,
		kMovie,
		kLocation,
		kPrint,
	};
	Options(TrinketEngine &vm, World &world, const Common::String &name, int location, bool practice);
	~Options();
	bool enter();
	void draw(OM::Canvas &canvas, uint32 time);
	Request event(const Common::Event &event);
	int destination() const { return _destination; }
	int movie() const { return _movie; }
	bool changedCurrentLevel() const { return _changedCurrent; }
	void stopSound() { _voice.stop(); }
	static const char *movieFile(int index);

private:
	enum Panel {
		kMain,
		kLevels,
		kReport,
		kMovies,
	};
	TrinketEngine &_vm;
	World &_world;
	Shell::ResourcePaths _paths;
	Shell::Scene _scene, _dialog;
	OM::Sound _voice, _effect;
	Common::SharedPtr<OM::NFNTFont> _font, _smallFont;
	Common::Array<Shell::TextObject *> _texts;
	Common::String _name;
	int _location, _destination = 0, _movie = -1, _pendingRow = -1, _pendingLevel = 0;
	bool _practice, _changedCurrent = false;
	Panel _panel = kMain;
	struct Control {
		Shell::Sprite *sprite;
		uint rest, pressed;
	};
	Common::Array<Control> _controls;
	static const int kLocations[8];
	static const char *const kActivities[8];
	static Common::String prefix(int row);
	void clearText();
	bool show(Panel panel);
	Shell::Sprite *add(int resource, int action = 0, int x = -1, int y = -1);
	void text(const Common::String &value, const Common::Rect &rect, bool small = false);
	void say(int resource);
	void control(Shell::Sprite *sprite, uint rest, uint pressed);
	void setLevel(int row, int level);
	Request activate(int action);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
