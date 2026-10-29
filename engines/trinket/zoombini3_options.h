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

/** Display options panels and report requested game and audio changes. */
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

	/** Sound resource IDs used by the options panel. */
	enum PageResourceId : int16 {
		kResSound401_Click = 401,
		kResSound402_Replace = 402,
		kResSound403_CreditsMusic = 403,
		kResSound439_EffectsPreview = 439,
		kResSound440_DialoguePreview = 440,
	};
	struct Result {
		/** Menu action requested by the most recent input event. */
		Action action = kNone00;
		/** Bit mask identifying volume controls changed by the event. */
		uint volumeChanges = 0;
		/** Requested change to background-music playback. */
		MusicRequest music = kKeepMusic00;
		/** One-based save slot selected by the menu. */
		uint selectedSlot = 0;
		/** Whether the selected slot should be replaced with a new game. */
		bool replaceSelGame = false;
	};
	/** Connect the options panels to resource, font, audio and saved world services. */
	Options(const OM::ResourceProvider &resources, OM::FontCache &fonts, Audio::Mixer &mixer, World &world);
	/** Build the options screen for practice mode, current music volume and player slots. */
	bool load(bool practice, int currentMusicVolume, const Shell::SaveFiles::Player &player);
	/** Handle options input and return requested menu and audio changes. */
	Result event(const Common::Event &event, uint32 time);
	/** Draw the active options panel or subdialog at a millisecond timestamp. */
	void draw(OM::Canvas &canvas, uint32 time = 0);
	/** Return whether the scrolling credits panel is open. */
	bool showingCredits() const { return _credits.get() != nullptr; }
	/** Return whether the new-game confirmation panel is open. */
	bool showingNewGame() const { return _newGameScene.get() != nullptr; }
	/** Return whether the save-slot selection panel is open. */
	bool showingSlots() const { return _slotScene.get() != nullptr; }
	/** Check whether a game-pixel point lies over an active options control. */
	bool overControl(Common::Point point) const;
	/** Stop the current dialogue or effects volume preview. */
	void stopPreview() { _preview.stop(); }
	/** Refresh the audio controls from the world and current music volume. */
	void syncVolumes(int musicVolume);

private:
	/** Format for script names or values: %s_%u. */
	static constexpr const char *kNameUnsignedIndexFormat = "%s_%u";
	/** Suffix identifying  1. */
	static constexpr const char *k1Suffix = "_1";

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
	/** Animation path pattern for options screen resources. */
	static constexpr const char *kAsset = "/z3options/%u.ao";
	/** Sound path pattern for options screen resources. */
	static constexpr const char *kSound = "/z3options/%u.wav";
	/** XML resource describing the credits content. */
	static constexpr const char *kCreditsXML = "z3credits.xml";
	/** Font resource used for the scrolling credits. */
	static constexpr const char *kCreditsFont = "/z3common/351.nfnt";
	/** Background animation resource used by the credits panel. */
	static constexpr const char *kCreditsBackground = "/z3options/431.ao";
	/** Font resource used by the new-game confirmation panel. */
	static constexpr const char *kNewGameFont = "/z3common/354.nfnt";
	/** Font resource used for save-slot headings. */
	static constexpr const char *kSlotHeadingFont = "/z3common/353.nfnt";
	/** Input action assigned to the options panel background. */
	static constexpr int kPanelAction = 100;
	/** Input action assigned to the background-music checkbox. */
	static constexpr int kCheckboxAction = 20;
	/** First action ID reserved for the three volume sliders. */
	static constexpr int kFirstSliderAction = 30;
	/** Provider used for options images, animations, fonts and sounds. */
	const OM::ResourceProvider &_resources;
	/** Shared game strikes; panels retain their selected fonts independently. */
	OM::FontCache &_fonts;
	/** Saved preferences and progress modified by the options screen. */
	World &_world;
	/** Sprites for the main options panel. */
	Shell::Scene _scene;
	/** Control that enables or disables background music. */
	OptionsCheckbox _checkbox;
	/** Slider controlling background-music volume. */
	OptionsSlider _music;
	/** Slider controlling dialogue volume. */
	OptionsSlider _dialogue;
	/** Slider controlling sound-effects volume. */
	OptionsSlider _effects;
	/** Playback state for menu click feedback. */
	OM::Sound _click;
	/** Playback state for dialogue or effects volume previews. */
	OM::Sound _preview;
	/** Main options action buttons. */
	Common::Array<Shell::Sprite *> _buttons;
	/** Sprites displayed behind the scrolling credits. */
	Common::ScopedPtr<Shell::Scene> _creditsScene;
	/** Scrolling credits content and timing state. */
	Common::ScopedPtr<Shell::CreditText> _credits;
	/** Sprites for the new-game confirmation panel. */
	Common::ScopedPtr<Shell::Scene> _newGameScene;
	/** Actions available in the new-game confirmation panel. */
	Common::Array<Shell::Sprite *> _newGameButtons;
	/** Font used to draw new-game confirmation text. */
	Common::SharedPtr<OM::NFNTFont> _newGameFont;
	/** Current player's name displayed by the options menu. */
	Common::String _curSaveName;
	/** Player registry record used to populate save slots. */
	Shell::SaveFiles::Player _player;
	/** Sprites for the save-slot selection panel. */
	Common::ScopedPtr<Shell::Scene> _slotScene;
	/** Selectable slot thumbnail buttons. */
	Common::Array<Shell::Sprite *> _slotButtons;
	/** Sprite highlighting the slot beneath the pointer. */
	Shell::Sprite *_slotHover = nullptr;
	/** Sprite marking the currently selected slot. */
	Shell::Sprite *_slotSelection = nullptr;
	/** Font used for the slot panel heading. */
	Common::SharedPtr<OM::NFNTFont> _slotHdrFont;
	/** Font used for individual slot labels. */
	Common::SharedPtr<OM::NFNTFont> _slotFont;
	/** Game-pixel rectangle of the slot panel heading. */
	Common::Rect _slotHdrRect;
	/** Game-pixel rectangles used to draw each slot label. */
	Common::Array<Common::Rect> _slotLabelRects;
	/** Displayed location labels for the player's slots. */
	Common::Array<Common::String> _slotLabels;
	/** One-based save slot selected by the player. */
	uint _selectedSlot = 0;
	/** Whether slot selection will replace the chosen game. */
	bool _replaceSelGame = false;
	/** Current music volume used to initialize or refresh its slider. */
	int _curMusicVol = 0;
	/** Whether the options screen has loaded successfully. */
	bool _loaded = false;
	/** Create a main options button with its action and vertical offset. */
	bool addButton(uint resource, Action action, int verticalOffset);
	/** Play a dialogue or effects sample at the selected volume. */
	void playPreview(bool dialogue);
	/** Play menu click feedback when sound is enabled. */
	void playClick();
	/** Build and start the scrolling credits panel. */
	bool openCredits(uint32 time);
	/** Build the new-game confirmation panel. */
	bool openNewGame();
	/** Handle confirmation-panel input and return its requested action. */
	Result newGameEvent(const Common::Event &event, uint32 time);
	/** Build save-slot selection for loading or replacing a game. */
	bool openSlots(bool replace);
	/** Handle slot selection and confirmation input. */
	Result slotEvent(const Common::Event &event, uint32 time);
	/** Return a slot thumbnail's game-pixel position. */
	static Common::Point slotOrigin(uint index);
	/** Map a saved location to its thumbnail resource ID. */
	static uint slotThumbnail(int location);
	/** Draw a text label within its game-pixel rectangle. */
	static void drawLabel(OM::Canvas &canvas, const OM::NFNTFont &font, const Common::String &text, const Common::Rect &rect);
	/** Close the current subpanel and refresh pointer feedback on the main panel. */
	void returnToMain(Common::Point pointer, uint32 time);
	/** Prevent accidental copies of this stateful object. */
	Options(const Options &) = delete;
	/** Prevent accidental copies of this stateful object. */
	Options &operator=(const Options &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
