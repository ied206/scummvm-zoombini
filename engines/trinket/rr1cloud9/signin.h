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
	/** Connect the game's player-selection interface to the engine and scene. */
	Signin(TrinketEngine &vm, Shell::Scene &scene);
	/** Load sign-in controls and populate registered player names. */
	bool enter();
	/** Handle name selection, editing and confirmation input. */
	Request event(const Common::Event &event, uint32 time);
	/** Advance deferred sign-in state and return a pending game request. */
	Request update();
	/** Advance and draw sign-in and confirmation sprites at a millisecond timestamp. */
	void draw(OM::Canvas &canvas, uint32 time);
	/** Return the currently selected or newly edited player name. */
	Common::String selectedName() const;
	/** Return the name removed by the most recent deletion request. */
	const Common::String &deletedName() const { return _deleted; }
	/** Reload player names and refresh list selection. */
	void refresh();
	/** Stop sign-in prompts and feedback sounds. */
	void stopSound();

private:
	/** Background image for player sign-in. */
	static constexpr const char *kBackground = "signin/5000.bk16";
	/** Sign-in button path indexed by resource ID. */
	static constexpr const char *kButtonFormat = "sipre/%d.ao";
	/** Font for the player name list. */
	static constexpr const char *kNameFont = "common/1018.nfnt";
	/** Sign-in speech path indexed by resource ID. */
	static constexpr const char *kSpeechFormat = "signin/%u.wav";
	/** List of names with recorded pronunciation. */
	static constexpr const char *kRecordedNames = "signinnames/8888.txt";
	/** Library prefix for recorded player names. */
	static constexpr const char *kNameSpeechPrefix = "signinnames/";
	/** Extension of recorded name speech. */
	static constexpr const char *kSoundExtension = ".wav";
	/** Pause sound appended after a recorded name. */
	static constexpr const char *kNameSpeechPause = "signin/5001.wav";
	/** Background shared by sign-in confirmation dialogs. */
	static constexpr const char *kDialogBackground = "common/1905.ao";
	/** Prompt animation for removing a player. */
	static constexpr const char *kRemovePrompt = "signin/5150.ao";
	/** Prompt shown when the player limit is reached. */
	static constexpr const char *kNameLimitPrompt = "signin/5152.ao";
	/** Accept button in sign-in confirmation dialogs. */
	static constexpr const char *kDialogAccept = "common/1861.ao";
	/** Cancel button in sign-in confirmation dialogs. */
	static constexpr const char *kDialogCancel = "common/1862.ao";
	/** Dismiss button in sign-in information dialogs. */
	static constexpr const char *kDialogDismiss = "common/1863.ao";
	/** Dialog character path indexed by resource ID. */
	static constexpr const char *kDialogActorFormat = "common/%d.ao";
	/** Sound played for a sign-in control click. */
	static constexpr const char *kClickSound = "common/1801.wav";

	enum Mode {
		kBrowse00 = 0,
		kMatch01 = 1,
		kDraft02 = 2,
		kNew03 = 3,
	};
	/** Engine supplying player storage, sign-in resources and audio. */
	TrinketEngine &_vm;
	/** Scene displaying the sign-in controls and player list. */
	Shell::Scene &_scene;
	/** Sprites displayed by the active sign-in confirmation dialog. */
	Shell::Scene _dialog;
	/** Playback state for spoken sign-in prompts. */
	OM::Sound _voice;
	/** Playback state for sign-in control sounds. */
	OM::Sound _effect;
	/** Sign-in strike shared with other game consumers of the same resource. */
	Common::SharedPtr<OM::NFNTFont> _font;
	/** Registered player names in list order. */
	Common::Array<Common::String> _entries;
	/** Player name currently being edited. */
	Common::String _typed;
	/** Player name retained for a confirmed deletion request. */
	Common::String _deleted;
	/** Queued spoken prompt resource paths. */
	Common::Array<Common::String> _speech;
	/** Button used to start the selected player's game. */
	Shell::Sprite *_start = nullptr;
	/** Button used to begin entering a new player name. */
	Shell::Sprite *_new = nullptr;
	/** Character sprites used in sign-in confirmations. */
	Shell::Sprite *_dialogActors[2] = {};
	/** Current name browsing, editing or confirmation state. */
	Mode _mode = kBrowse00;
	/** Current selected player-list index. */
	int _selected = 0;
	/** Index of the first name visible in the list. */
	int _first = 0;
	/** Current confirmation dialog kind. */
	int _dialogKind = 0;
	/** List row used by the previous click, or minus one when unset. */
	int _lastClickRow = -1;
	/** Millisecond timestamp used to recognize a second click on the same row. */
	uint32 _lastClickTime = 0;
	/** Whether the current selection or edited name can be accepted. */
	bool _valid = false;
	/** Whether sign-in completion is waiting for its final prompt or transition. */
	bool _starting = false;
	/** Maximum number of registered player names accepted by the sign-in interface. */
	static constexpr int kMaxNames = 99;
	/** Maximum number of bytes accepted in a player name. */
	static constexpr int kMaxLength = 16;
	/** Number of player-name rows visible at once. */
	static constexpr int kRows = 3;
	/** Vertical distance between visible name rows in pixels. */
	int _rowHeight = 20;
	/** Adjust list scrolling to keep the selected name visible. */
	void centerSelection();
	/** Refresh button availability and feedback for the current sign-in state. */
	void buttons();
	/** Begin editing a new player name. */
	void newEntry();
	/** Append an accepted character to the edited player name. */
	void addCharacter(char character);
	/** Remove the final character from the edited player name. */
	void deleteCharacter();
	/** Move name selection by a row or visible page. */
	void move(int direction, bool page);
	/** Accept the selected name and begin the final sign-in transition. */
	void start();
	/** Build a sign-in confirmation dialog of the requested kind. */
	void dialog(int kind);
	/** Queue a spoken sign-in prompt by resource ID. */
	void speak(uint16 id);
	/** Match the edited name against existing player entries. */
	void rematch();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
