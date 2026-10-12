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

#include "trinket/zoombini3_quit.h"

#include "common/debug.h"

namespace Trinket {
namespace Zoombini3 {

constexpr const char *QuitDialog::kAsset;
constexpr const char *QuitDialog::kSound;

QuitDialog::QuitDialog(const OM::ResourceProvider &resources, Audio::Mixer &mixer) : _resources(resources), _scene(resources),
																					 _click(mixer), _voice(mixer) {
	_graphics[0] = _graphics[1] = nullptr;
	_buttons[0] = _buttons[1] = nullptr;
}

bool QuitDialog::load() {
	if (!_scene.add(Common::String::format(kAsset, 500), 0))
		return false;
	_idle = _scene.add(Common::String::format(kAsset, 503), 1);
	if (!_idle)
		return false;
	for (uint i = 0; i < 2; i++) {
		Shell::Sprite *button = _scene.add(Common::String::format(kAsset, 1 + i), 3, 1 + i);
		_buttons[i] = button;
		_graphics[i] = _scene.add(Common::String::format(kAsset, 501 + i), 1);
		if (!button || !_graphics[i])
			return false;
		button->managed = true;
		button->setFrame(1);
		button->hitRect = Common::Rect(button->x, button->y, button->x + button->width(), button->y + button->height());
		_graphics[i]->setFrame(1);
		_graphics[i]->visible = false;
	}
	return true;
}

void QuitDialog::setHover(int hover) {
	if (_hover == hover)
		return;
	if (0 <= _hover) {
		_graphics[_hover]->pauseAnimation(true);
		_graphics[_hover]->setFrame(1);
		_graphics[_hover]->visible = false;
	}
	_hover = hover;
	_idle->visible = hover < 0;
	if (0 <= hover) {
		_graphics[hover]->visible = true;
		_graphics[hover]->startAnimation(-1);
	}
}

QuitDialog::Result QuitDialog::event(const Common::Event &event) {
	if (event.type == Common::EVENT_KEYDOWN) {
		int key = event.kbd.ascii;
		if ('A' <= key && key <= 'Z')
			key += 'a' - 'A';
		if (key == 'y')
			return kQuit00;
		if (key == 'n')
			return kResume01;
		return kPending;
	}
	if (event.type != Common::EVENT_MOUSEMOVE && event.type != Common::EVENT_LBUTTONDOWN && event.type != Common::EVENT_LBUTTONUP)
		return kPending;
	int action = _scene.event(event);
	int hit = _scene.hitTest(event.mouse);
	if (event.type == Common::EVENT_LBUTTONDOWN && hit)
		_click.play(_resources, Common::String::format(kSound, static_cast<uint>(kResSoundCommon0001_Click)), Audio::Mixer::kSFXSoundType);
	if (event.type == Common::EVENT_MOUSEMOVE || event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_LBUTTONUP) {
		for (uint i = 0; i < 2; i++) {
			uint frame = 1;
			if (_buttons[i]->pressed)
				frame = 2;
			else if (_buttons[i]->hovered)
				frame = 4;
			_buttons[i]->setFrame(frame);
		}
		setHover(hit - 1);
	}
	if (action == 1)
		return kQuit00;
	if (action == 2)
		return kResume01;
	return kPending;
}

void QuitDialog::draw(OM::Canvas &canvas, uint32 time) {
	Common::Array<uint16> sounds;
	_scene.update(time, sounds);
	for (uint i = 0; i < sounds.size(); i++) {
		if (!_voice.play(_resources, Common::String::format(kSound, sounds[i]), Audio::Mixer::kSFXSoundType))
			warning("Could not play quit dialog sound %u", sounds[i]);
	}
	_scene.drawSprites(canvas, 0, 3);
}

} // namespace Zoombini3
} // namespace Trinket
