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

#include "trinket/zoombini3_options.h"

#include "common/debug.h"
#include "common/file.h"
#include "trinket/shell/text.h"

namespace Trinket {
namespace Zoombini3 {

constexpr const char *Options::kAsset;
constexpr const char *Options::kSound;
constexpr const char *Options::kCreditsXML;
constexpr const char *Options::kCreditsFont;
constexpr const char *Options::kCreditsBackground;
constexpr const char *Options::kNewGameFont;
constexpr const char *Options::kSlotHeadingFont;
constexpr int Options::kPanelAction;
constexpr int Options::kCheckboxAction;
constexpr int Options::kFirstSliderAction;

Options::Options(const OM::ResourceProvider &resources, Audio::Mixer &mixer, World &world) : _resources(resources), _world(world),
																							 _scene(resources), _checkbox(_scene), _music(_scene), _dialogue(_scene), _effects(_scene), _click(mixer), _preview(mixer) {
}

bool Options::addButton(uint resource, Action action, int verticalOffset) {
	Shell::Sprite *button = _scene.add(Common::String::format(kAsset, resource), 3, action);
	if (!button)
		return false;
	button->managed = true;
	button->setFrame(1);
	button->y += verticalOffset;
	// ROptions captures each button's current rectangle before hover can move it.
	button->hitRect = Common::Rect(button->x, button->y, button->x + button->width(), button->y + button->height());
	_buttons.push_back(button);
	return true;
}

bool Options::load(bool practice, int currentMusicVolume, const Shell::SaveFiles::Player &player) {
	if (_loaded)
		return false;
	if (!practice && (!player.count || 6 < player.count || !player.current || player.count < player.current))
		return false;
	_player = player;
	_currentMusicVolume = currentMusicVolume;
	_currentSaveName = Common::String::format("%s_%u", player.name.c_str(), player.current);
	const uint panels[] = {
		437,
		405,
		499,
	};
	for (uint i = 0; i < ARRAYSIZE(panels); i++) {
		Shell::Sprite *panel = _scene.add(Common::String::format(kAsset, panels[i]), i ? 3 : 2, kPanelAction);
		if (!panel)
			return false;
		panel->managed = true;
		panel->setFrame(1);
	}
	if (!practice && (!addButton(407, kNewGame01, 0) || !addButton(408, kLoadGame02, 0)))
		return false;
	if (!_checkbox.load(3, kCheckboxAction, !_world.isHelpAudioOff(), practice))
		return false;
	int offset = practice ? -_checkbox.height() : 0;
	if (!addButton(406, kQuit04, offset) || !addButton(409, kCredits03, offset) || !addButton(401, kClose05, 0))
		return false;
	OptionsSlider *sliders[] = {
		&_music,
		&_dialogue,
		&_effects,
	};
	const int volumes[] = {
		currentMusicVolume,
		_world.dialogueVolume(),
		_world.effectsVolume(),
	};
	for (uint i = 0; i < ARRAYSIZE(sliders); i++) {
		if (!sliders[i]->load(static_cast<OptionsSlider::Channel>(i), 3, kFirstSliderAction + 10 * i))
			return false;
		sliders[i]->setPercentage(volumes[i]);
	}
	_loaded = true;
	return true;
}

void Options::playClick() {
	_click.play(_resources, Common::String::format(kSound, static_cast<uint>(kResSound401_Click)), Audio::Mixer::kSFXSoundType);
}

void Options::syncVolumes(int musicVolume) {
	_currentMusicVolume = musicVolume;
	_music.setPercentage(musicVolume);
	_dialogue.setPercentage(_world.dialogueVolume());
	_effects.setPercentage(_world.effectsVolume());
}

void Options::playPreview(bool dialogue) {
	_preview.stop();
	_preview.play(_resources, Common::String::format(kSound, static_cast<uint>(dialogue ? kResSound440_DialoguePreview : kResSound439_EffectsPreview)),
				  dialogue ? Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType);
}

Options::Result Options::event(const Common::Event &event, uint32 time) {
	Result result;
	if (_credits) {
		if (event.type == Common::EVENT_LBUTTONDOWN) {
			returnToMain(event.mouse, time);
			result.music = kRestoreMusic02;
		}
		return result;
	}
	if (_newGameScene)
		return newGameEvent(event, time);
	if (_slotScene)
		return slotEvent(event, time);
	if (!_loaded || (event.type != Common::EVENT_MOUSEMOVE && event.type != Common::EVENT_LBUTTONDOWN && event.type != Common::EVENT_LBUTTONUP))
		return result;
	int hit = _scene.hitTest(event.mouse);
	if (event.type == Common::EVENT_LBUTTONDOWN && !hit) {
		result.action = kClose05;
		return result;
	}
	int action = _scene.event(event);
	for (uint i = 0; i < _buttons.size(); i++) {
		Shell::Sprite *button = _buttons[i];
		uint frame = 1;
		if (button->pressed)
			frame = 2;
		else if (button->hovered)
			frame = 4;
		button->setFrame(frame);
		if (event.type == Common::EVENT_LBUTTONDOWN && hit == button->action)
			playClick();
	}
	uint checkbox = _checkbox.event(event);
	if (checkbox & OptionsCheckbox::kClickSound02)
		playClick();
	if (checkbox & OptionsCheckbox::kClicked01)
		_world.setHelpAudioOff(!_checkbox.isOn());
	OptionsSlider *sliders[] = {
		&_music,
		&_dialogue,
		&_effects,
	};
	for (uint i = 0; i < ARRAYSIZE(sliders); i++) {
		uint notification = sliders[i]->event(event, time);
		if (notification & OptionsSlider::kValueChanged01) {
			int volume = sliders[i]->percentage();
			if (i == OptionsSlider::kMusic00) {
				_currentMusicVolume = volume;
				_world.setMusicVolume(volume);
				result.volumeChanges |= kMusic01;
			} else {
				_preview.stop();
				if (i == OptionsSlider::kDialogue01) {
					_world.setDialogueVolume(volume);
					result.volumeChanges |= kDialogue02;
				} else {
					_world.setEffectsVolume(volume);
					result.volumeChanges |= kEffects04;
				}
			}
		}
		if ((notification & OptionsSlider::kActivated02) && i != OptionsSlider::kMusic00)
			playPreview(i == OptionsSlider::kDialogue01);
	}
	if (kNewGame01 <= action && action <= kClose05)
		result.action = static_cast<Action>(action);
	if (result.action == kCredits03) {
		result.action = kNone00;
		if (openCredits(time))
			result.music = kStartCredits01;
		else
			warning("Could not load Credits resources");
	} else if (result.action == kNewGame01) {
		result.action = kNone00;
		if (!openNewGame())
			warning("Could not load New Game confirmation resources");
	} else if (result.action == kLoadGame02) {
		result.action = kNone00;
		if (!openSlots(false))
			warning("Could not load saved game selection resources");
	}
	return result;
}

bool Options::overControl(Common::Point point) const {
	if (_credits)
		return false;
	if (_slotScene) {
		int hit = _slotScene->hitTest(point);
		return hit && hit != kPanelAction;
	}
	int hit = _newGameScene ? _newGameScene->hitTest(point) : _scene.hitTest(point);
	return hit && hit != kPanelAction;
}

bool Options::openCredits(uint32 time) {
	Common::File xml;
	Common::ScopedPtr<Common::SeekableReadStream> fontStream(_resources.open(kCreditsFont));
	Common::ScopedPtr<OM::NFNTFont> font(fontStream ? OM::NFNTFont::load(*fontStream) : nullptr);
	if (!xml.open(Common::Path(kCreditsXML)) || !font)
		return false;
	Common::ScopedPtr<Shell::CreditText> credits(new Shell::CreditText(520, 420));
	Common::ScopedPtr<Shell::Scene> scene(new Shell::Scene(_resources));
	if (!credits->load(xml, *font, _resources, kAsset) || !scene->add(kCreditsBackground, 0))
		return false;
	_preview.stop();
	credits->start(time, 1000);
	_credits.reset(credits.release());
	_creditsScene.reset(scene.release());
	return true;
}

bool Options::openNewGame() {
	Common::ScopedPtr<Common::SeekableReadStream> stream(_resources.open(kNewGameFont));
	Common::SharedPtr<OM::NFNTFont> font(stream ? OM::NFNTFont::load(*stream) : nullptr);
	if (!font)
		return false;
	Common::ScopedPtr<Shell::Scene> scene(new Shell::Scene(_resources));
	const uint panels[] = {
		437,
		411,
		499,
	};
	for (uint i = 0; i < ARRAYSIZE(panels); i++) {
		Shell::Sprite *panel = scene->add(Common::String::format(kAsset, panels[i]), i ? 3 : 2, kPanelAction);
		if (!panel)
			return false;
		panel->managed = true;
		panel->setFrame(1);
	}
	Common::Array<Shell::Sprite *> buttons;
	const uint resources[] = {
		402,
		403,
		401,
	};
	const ConfirmationAction actions[] = {
		kConfirmNew01,
		kCancelNew02,
		kReturnNew03,
	};
	for (uint i = 0; i < ARRAYSIZE(resources); i++) {
		Shell::Sprite *button = scene->add(Common::String::format(kAsset, resources[i]), 3, actions[i]);
		if (!button)
			return false;
		button->managed = true;
		button->setFrame(1);
		button->hitRect = Common::Rect(button->x, button->y, button->x + button->width(), button->y + button->height());
		buttons.push_back(button);
	}
	_preview.stop();
	_newGameButtons = buttons;
	_newGameFont = font;
	_newGameScene.reset(scene.release());
	return true;
}

Options::Result Options::newGameEvent(const Common::Event &event, uint32 time) {
	Result result;
	if (event.type != Common::EVENT_MOUSEMOVE && event.type != Common::EVENT_LBUTTONDOWN && event.type != Common::EVENT_LBUTTONUP)
		return result;
	int hit = _newGameScene->hitTest(event.mouse);
	if (event.type == Common::EVENT_LBUTTONDOWN && !hit) {
		result.action = kClose05;
		return result;
	}
	int action = _newGameScene->event(event);
	for (uint i = 0; i < _newGameButtons.size(); i++) {
		Shell::Sprite *button = _newGameButtons[i];
		if (button->pressed)
			button->setFrame(2);
		else
			button->setFrame(button->hovered ? 4 : 1);
		if (event.type == Common::EVENT_LBUTTONDOWN && hit == button->action)
			playClick();
	}
	if (action == kConfirmNew01) {
		if (_player.count < 6)
			result.action = kNewGame01;
		else if (!openSlots(true))
			warning("Could not load saved game replacement resources");
	} else if (action == kCancelNew02 || action == kReturnNew03)
		returnToMain(event.mouse, time);
	return result;
}

void Options::returnToMain(Common::Point pointer, uint32 time) {
	_preview.stop();
	_credits.reset();
	_creditsScene.reset();
	_newGameButtons.clear();
	_newGameScene.reset();
	_slotButtons.clear();
	_slotScene.reset();
	_slotHover = _slotSelection = nullptr;
	_checkbox.setOn(!_world.isHelpAudioOff(), pointer);
	_music.setPercentage(_currentMusicVolume);
	_dialogue.setPercentage(_world.dialogueVolume());
	_effects.setPercentage(_world.effectsVolume());
	// Reestablish hover without sending the closing press into the restored panel.
	Common::Event hover;
	hover.type = Common::EVENT_MOUSEMOVE;
	hover.mouse = pointer;
	_scene.event(hover);
	_music.event(hover, time);
	_dialogue.event(hover, time);
	_effects.event(hover, time);
	for (uint i = 0; i < _buttons.size(); i++)
		_buttons[i]->setFrame(_buttons[i]->hovered ? 4 : 1);
}

void Options::draw(OM::Canvas &canvas, uint32 time) {
	if (_slotScene) {
		_slotScene->drawSprites(canvas, INT_MIN, INT_MAX);
		drawLabel(canvas, *_slotHeadingFont, Common::String::format("%s's Saved Games", _player.name.c_str()), _slotHeadingRect);
		for (uint i = 0; i < _slotLabels.size(); i++)
			drawLabel(canvas, *_slotFont, _slotLabels[i], _slotLabelRects[i]);
		return;
	}
	if (_newGameScene) {
		_newGameScene->drawSprites(canvas, INT_MIN, INT_MAX);
		// RText's rectangle constructor centers this unwrapped, read-only save name.
		drawLabel(canvas, *_newGameFont, _currentSaveName, Common::Rect(0, 260, 800, 300));
		return;
	}
	if (_credits) {
		_credits->update(time);
		_creditsScene->draw(canvas);
		_credits->draw(canvas, 140, 80);
		return;
	}
	_scene.drawSprites(canvas, INT_MIN, INT_MAX);
}

void Options::drawLabel(OM::Canvas &canvas, const OM::NFNTFont &font, const Common::String &text, const Common::Rect &rect) {
	int baseline = rect.top + Shell::Text::firstBaseline(rect.height(), 1, font.ascent(), font.descent(), 0);
	uint32 color = canvas.surface().format.RGBToColor(500 / 256, 450 / 256, 7500 / 256);
	font.drawString(&canvas.surface(), text, rect.left, baseline - font.ascent(), rect.width(), color, Graphics::kTextAlignCenter, 0, false, true);
}

Common::Point Options::slotOrigin(uint index) {
	return Common::Point(156 + 150 * (index % 3), 163 + 120 * (index / 3));
}

uint Options::slotThumbnail(int location) {
	if (kCatapult01 <= location && location <= kBarn07)
		return 414 + location;
	switch (location) {
	case kRecruitment11:
		return 423;
	case kIsland13:
		return 424;
	case kMap12:
		return 438;
	default:
		return 0;
	}
}

bool Options::openSlots(bool replace) {
	Common::ScopedPtr<Common::SeekableReadStream> heading(_resources.open(kSlotHeadingFont));
	Common::ScopedPtr<Common::SeekableReadStream> font(_resources.open(kCreditsFont));
	Common::SharedPtr<OM::NFNTFont> headingFont(heading ? OM::NFNTFont::load(*heading) : nullptr);
	Common::SharedPtr<OM::NFNTFont> slotFont(font ? OM::NFNTFont::load(*font) : nullptr);
	if (!headingFont || !slotFont)
		return false;
	Common::ScopedPtr<Shell::Scene> scene(new Shell::Scene(_resources));
	Common::Rect headingRect;
	const uint panels[] = {
		437,
		412,
		499,
	};
	for (uint i = 0; i < ARRAYSIZE(panels); i++) {
		Shell::Sprite *panel = scene->add(Common::String::format(kAsset, panels[i]), i ? 3 : 2, kPanelAction);
		if (!panel)
			return false;
		panel->managed = true;
		panel->setFrame(panels[i] == 499 ? 2 : 1);
		if (panels[i] == 412)
			headingRect = Common::Rect(panel->x, panel->y, panel->x + panel->width(), panel->y + panel->height() - 10);
	}
	Common::Array<Common::Rect> rects;
	Common::Array<Common::String> labels;
	Common::String name = _player.name;
	for (uint i = 0; i < 6; i++) {
		Common::Point origin = slotOrigin(i);
		Shell::Sprite *card = scene->add(Common::String::format(kAsset, 413), 3, kPanelAction);
		if (!card)
			return false;
		card->managed = true;
		card->setFrame(1);
		card->x = origin.x;
		card->y = origin.y;
		if (_player.count <= i)
			continue;
		uint thumbnail = slotThumbnail(i + 1 == _player.current ? _world.currentLocation() : _player.locations[i]);
		if (!thumbnail)
			continue;
		Shell::Sprite *picture = scene->add(Common::String::format(kAsset, thumbnail), 3, kFirstSlot10 + i);
		if (!picture)
			return false;
		picture->managed = true;
		picture->setFrame(1);
		picture->x = origin.x + 14;
		picture->y = origin.y + 9;
		picture->actionOnMouseDown = true;
		int top = origin.y + card->height() - 12;
		if (!i) {
			while (!name.empty() && card->width() < slotFont->getStringWidth(name + "_1"))
				name.deleteLastChar();
		}
		labels.push_back(Common::String::format("%s_%u", name.c_str(), i + 1));
		rects.push_back(Common::Rect(origin.x, top, origin.x + card->width(), top + 25));
	}
	Shell::Sprite *hover = scene->add(Common::String::format(kAsset, 414), 3);
	Shell::Sprite *selection = scene->add(Common::String::format(kAsset, 414), 3);
	if (!hover || !selection)
		return false;
	hover->managed = selection->managed = true;
	hover->setFrame(1);
	selection->setFrame(1);
	hover->visible = false;
	Common::Point origin = slotOrigin(_player.current - 1);
	selection->x = origin.x + 4;
	selection->y = origin.y - 1;
	Common::Array<Shell::Sprite *> buttons;
	const uint resources[] = {
		404,
		401,
	};
	const SlotAction actions[] = {
		kAcceptSlot01,
		kReturnSlots02,
	};
	for (uint i = 0; i < ARRAYSIZE(resources); i++) {
		Shell::Sprite *button = scene->add(Common::String::format(kAsset, resources[i]), 3, actions[i]);
		if (!button)
			return false;
		button->managed = true;
		button->setFrame(1);
		button->hitRect = Common::Rect(button->x, button->y, button->x + button->width(), button->y + button->height());
		buttons.push_back(button);
	}
	_preview.stop();
	_newGameButtons.clear();
	_newGameScene.reset();
	_slotHeadingFont = headingFont;
	_slotFont = slotFont;
	_slotScene.reset(scene.release());
	_slotButtons = buttons;
	_slotLabels = labels;
	_slotLabelRects = rects;
	_slotHeadingRect = headingRect;
	_slotHover = hover;
	_slotSelection = selection;
	_selectedSlot = _player.current;
	_replaceSelectedGame = replace;
	if (replace)
		_preview.play(_resources, Common::String::format(kSound, static_cast<uint>(kResSound402_Replace)), Audio::Mixer::kSFXSoundType);
	return true;
}

Options::Result Options::slotEvent(const Common::Event &event, uint32 time) {
	Result result;
	if (event.type != Common::EVENT_MOUSEMOVE && event.type != Common::EVENT_LBUTTONDOWN && event.type != Common::EVENT_LBUTTONUP)
		return result;
	int hit = _slotScene->hitTest(event.mouse);
	if (event.type == Common::EVENT_LBUTTONDOWN && !hit) {
		result.action = kClose05;
		return result;
	}
	int action = _slotScene->event(event);
	_slotHover->visible = kFirstSlot10 <= hit && hit < kFirstSlot10 + static_cast<int>(_player.count);
	if (_slotHover->visible) {
		Common::Point origin = slotOrigin(hit - kFirstSlot10);
		_slotHover->x = origin.x + 4;
		_slotHover->y = origin.y - 1;
	}
	for (uint i = 0; i < _slotButtons.size(); i++) {
		Shell::Sprite *button = _slotButtons[i];
		if (button->pressed)
			button->setFrame(2);
		else
			button->setFrame(button->hovered ? 4 : 1);
		if (event.type == Common::EVENT_LBUTTONDOWN && hit == button->action)
			playClick();
	}
	if (kFirstSlot10 <= action && action < kFirstSlot10 + static_cast<int>(_player.count)) {
		_selectedSlot = action - kFirstSlot10 + 1;
		Common::Point origin = slotOrigin(_selectedSlot - 1);
		_slotSelection->x = origin.x + 4;
		_slotSelection->y = origin.y - 1;
	} else if (action == kAcceptSlot01) {
		if (_selectedSlot == _player.current && !_replaceSelectedGame) {
			result.action = kClose05;
		} else {
			result.action = kLoadGame02;
			result.selectedSlot = _selectedSlot;
			result.replaceSelectedGame = _replaceSelectedGame;
		}
	} else if (action == kReturnSlots02) {
		returnToMain(event.mouse, time);
	}
	return result;
}

} // namespace Zoombini3
} // namespace Trinket
