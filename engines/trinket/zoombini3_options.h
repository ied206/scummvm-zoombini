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

#ifndef TRINKET_ZOOMBINI3_OPTIONS_H
#define TRINKET_ZOOMBINI3_OPTIONS_H

#include "common/ptr.h"
#include "trinket/ompp/sound.h"
#include "trinket/shell/credits.h"
#include "trinket/shell/savefiles.h"
#include "trinket/zoombini3_checkbox.h"
#include "trinket/zoombini3_slider.h"
#include "trinket/zoombini3_world.h"

namespace Trinket {
namespace Zoombini3 {

/** Panels of the original ROptions object. Game changes belong to its owner. */
class Options {
public:
	enum Action {
		kNone00 = 0,
		kNewGame01 = 1,
		kLoadGame02 = 2,
		kCredits03 = 3,
		kQuit04 = 4,
		kClose05 = 5,
	};
	enum VolumeChange {
		kMusic01 = 1,
		kDialogue02 = 2,
		kEffects04 = 4,
	};
	enum MusicRequest {
		kKeepMusic00 = 0,
		kStartCredits01 = 1,
		kRestoreMusic02 = 2,
	};

	/** Sound resource IDs owned by the options panel. */
	enum PageResourceId : int16 {
		kResSound401_Click = 401,
		kResSound402_Replace = 402,
		kResSound403_CreditsMusic = 403,
		kResSound439_EffectsPreview = 439,
		kResSound440_DialoguePreview = 440,
	};
	struct Result {
		Action action = kNone00;
		uint volumeChanges = 0;
		MusicRequest music = kKeepMusic00;
		uint selectedSlot = 0;
		bool replaceSelectedGame = false;
	};
	Options(const OM::ResourceProvider &resources, Audio::Mixer &mixer, World &world);
	bool load(bool practice, int currentMusicVolume, const Shell::SaveFiles::Player &player);
	Result event(const Common::Event &event, uint32 time);
	void draw(OM::Canvas &canvas, uint32 time = 0);
	bool showingCredits() const { return _credits.get() != nullptr; }
	bool showingNewGame() const { return _newGameScene.get() != nullptr; }
	bool showingSlots() const { return _slotScene.get() != nullptr; }
	bool overControl(Common::Point point) const;
	void stopPreview() { _preview.stop(); }
	void syncVolumes(int musicVolume);

private:
	enum ConfirmationAction {
		kConfirmNew01 = 1,
		kCancelNew02 = 2,
		kReturnNew03 = 3,
	};
	enum SlotAction {
		kAcceptSlot01 = 1,
		kReturnSlots02 = 2,
		kFirstSlot10 = 10,
	};
	static constexpr const char *kAsset = "/z3options/%u.ao";
	static constexpr const char *kSound = "/z3options/%u.wav";
	static constexpr const char *kCreditsXML = "z3credits.xml";
	static constexpr const char *kCreditsFont = "/z3common/351.nfnt";
	static constexpr const char *kCreditsBackground = "/z3options/431.ao";
	static constexpr const char *kNewGameFont = "/z3common/354.nfnt";
	static constexpr const char *kSlotHeadingFont = "/z3common/353.nfnt";
	static constexpr int kPanelAction = 100;
	static constexpr int kCheckboxAction = 20;
	static constexpr int kFirstSliderAction = 30;
	const OM::ResourceProvider &_resources;
	World &_world;
	Shell::Scene _scene;
	OptionsCheckbox _checkbox;
	OptionsSlider _music, _dialogue, _effects;
	OM::Sound _click, _preview;
	Common::Array<Shell::Sprite *> _buttons;
	Common::ScopedPtr<Shell::Scene> _creditsScene;
	Common::ScopedPtr<Shell::CreditText> _credits;
	Common::ScopedPtr<Shell::Scene> _newGameScene;
	Common::Array<Shell::Sprite *> _newGameButtons;
	Common::SharedPtr<OM::NFNTFont> _newGameFont;
	Common::String _currentSaveName;
	Shell::SaveFiles::Player _player;
	Common::ScopedPtr<Shell::Scene> _slotScene;
	Common::Array<Shell::Sprite *> _slotButtons;
	Shell::Sprite *_slotHover = nullptr, *_slotSelection = nullptr;
	Common::SharedPtr<OM::NFNTFont> _slotHeadingFont, _slotFont;
	Common::Rect _slotHeadingRect;
	Common::Array<Common::Rect> _slotLabelRects;
	Common::Array<Common::String> _slotLabels;
	uint _selectedSlot = 0;
	bool _replaceSelectedGame = false;
	int _currentMusicVolume = 0;
	bool _loaded = false;
	bool addButton(uint resource, Action action, int verticalOffset);
	void playPreview(bool dialogue);
	void playClick();
	bool openCredits(uint32 time);
	bool openNewGame();
	Result newGameEvent(const Common::Event &event, uint32 time);
	bool openSlots(bool replace);
	Result slotEvent(const Common::Event &event, uint32 time);
	static Common::Point slotOrigin(uint index);
	static uint slotThumbnail(int location);
	static void drawLabel(OM::Canvas &canvas, const OM::NFNTFont &font, const Common::String &text, const Common::Rect &rect);
	void returnToMain(Common::Point pointer, uint32 time);
	Options(const Options &) = delete;
	Options &operator=(const Options &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
