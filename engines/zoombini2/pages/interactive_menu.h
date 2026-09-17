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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ZOOMBINI2_PAGES_INTERACTIVE_MENU_H
#define ZOOMBINI2_PAGES_INTERACTIVE_MENU_H

#include "common/rect.h"
#include "common/str.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/interactive_base.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;
class SaveFileList;
enum class DialogMsgBoxButton;

/** Internal state of the original sign-in page owner. */
enum class MenuScreenState {
	/** Display the saved-game list and its primary actions. */
	kMain00 = 0,
	/** Display the volume options panel. */
	kOptions01 = 1
};

/**
 * Sign-in page containing a four-row sorted saved-game list.
 *
 * The page retains visual resources, buttons, sounds, the options panel, and a
 * @ref SaveFileList. The list manages filename selection and editing semantics.
 */
class InteractiveMenu : public InteractiveBase {
public:
	/** Construct the sign-in page for @p vm. */
	explicit InteractiveMenu(Zoombini2Engine *vm);
	/** Release the loaded resources and controls retained by this page. */
	~InteractiveMenu() override;

	/** Load sign-in resources and scan the available profiles. */
	void init() override;
	/** Process text input and hover state for the active panel. */
	void onUpdate() override;
	/** Draw the saved-game list or the volume panel managed by this page. */
	void onRenderContent(ManagedSurface32 *screen) override;
	void onRenderForeground(ManagedSurface32 *screen) override;
	/** Dispatch a click to the list, buttons, or the volume panel managed by this page. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	bool hasActiveDialog() const override { return _state == MenuScreenState::kOptions01; }

private:
	/** Resource path. */
	static constexpr const char *kBackgroundPath = "#bmp/menu/background";
	/** Resource path. */
	static constexpr const char *kSelectorNormalPath = "bmp/menu/PARTIEs - selector NORMAL";
	/** Resource path. */
	static constexpr const char *kSelectorHighlightPath = "bmp/menu/PARTIEs - selector HILITE";
	/** Resource path. */
	static constexpr const char *kSelectionBarPath = "bmp/menu/barre-cache.rb";
	/** Resource path. */
	static constexpr const char *kBlipSoundPath = "sounds/blip.wav";
	/** Resource path. */
	static constexpr const char *kTypeSoundPath = "sounds/fx/i-bs5.wav";
	/** Resource path. */
	static constexpr const char *kDeleteSoundPath = "sounds/fx/del.wav";
	/** Resource path. */
	static constexpr const char *kArrowUpNormalPath = "bmp/menu/PARTIEs - ArrowUP NORMAL";
	/** Resource path. */
	static constexpr const char *kArrowUpHighlightPath = "bmp/menu/PARTIEs - ArrowUP HIGHLIGHT";
	/** Resource path. */
	static constexpr const char *kArrowDownNormalPath = "bmp/menu/PARTIEs - ArrowDOWN NORMAL";
	/** Resource path. */
	static constexpr const char *kArrowDownHighlightPath = "bmp/menu/PARTIEs - ArrowDOWN HILITE";
	/** Resource path. */
	static constexpr const char *kStartNormalPath = "bmp/menu/Start Normal";
	/** Resource path. */
	static constexpr const char *kStartHighlightPath = "bmp/menu/Start Highlight";
	/** Resource path. */
	static constexpr const char *kStartDisabledPath = "bmp/menu/Start Gray";
	/** Resource path. */
	static constexpr const char *kOptionsNormalPath = "bmp/menu/PANEL - Options  NORMAL";
	/** Resource path. */
	static constexpr const char *kOptionsHighlightPath = "bmp/menu/PANEL - Options  HIGHLIGHT";
	/** Resource path. */
	static constexpr const char *kNewNormalPath = "bmp/menu/New Normal";
	/** Resource path. */
	static constexpr const char *kNewHighlightPath = "bmp/menu/New Highlight";
	/** Resource path. */
	static constexpr const char *kNewDisabledPath = "bmp/menu/New Gray";
	/** Resource path. */
	static constexpr const char *kPracticeNormalPath = "bmp/menu/PANEL - Entrainement NORMAL";
	/** Resource path. */
	static constexpr const char *kPracticeHighlightPath = "bmp/menu/PANEL - Entraine HILITE";
	/** Resource path. */
	static constexpr const char *kQuitNormalPath = "bmp/menu/PANEL - Quitter NORMAL";
	/** Resource path. */
	static constexpr const char *kQuitHighlightPath = "bmp/menu/PANEL - Quitter HIGHLIGHT";
	/** Resource path. */
	static constexpr const char *kDeleteConfirmationPath = "bmp/menu/Quit_panel_text_suppr";
	/** Resource path. */
	static constexpr const char *kQuitConfirmationPath = "bmp/menu/Quit_panel_text_quit";

	void updateButtonAvailability();
	EventHandleResult handleVolumePanelInput(const Common::Point &pos, bool mouseReleased);
	bool _volumePanelMouseDown = false;

	/** Screen origin of the saved-profile list. */
	static constexpr Common::Point32 kFileListPos = Common::Point32(157, 286);
	static constexpr Size32 kSelectorSize = Size32(520, 201);
	static constexpr const char *kValidNameCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";

	/** Panel currently accepting input. */
	MenuScreenState _state = MenuScreenState::kMain00;
	/** Normal saved-game list frame. */
	BitBlock *_selectorNormal = nullptr;
	/** Highlighted saved-game list frame. */
	BitBlock *_selectorHilite = nullptr;
	/** Bar drawn behind the selected save row. */
	RleBlock *_selectionBar = nullptr;
	/** Profile list and editor state managed by this menu screen. */
	SaveFileList *_fileList = nullptr;
	/** Primary menu controls indexed by menu button ID. */
	UIButton *_buttons[kMenuButtonCount] = {};

	/** Menu selection sound. */
	int _blipSoundId = -1;
	/** Profile-name typing sound. */
	int _typeSoundId = -1;
	/** Profile deletion sound. */
	int _deleteSoundId = -1;
	/** Shared map music handle requested by this page. */
	int _mapMusicId = -1;

	/** Volume panel managed by this page while options are open. */
	VolumePanel *_volumePanel = nullptr;
	/** Selected profile retained while the shared delete confirmation is active. */
	Common::String _pendingDeleteProfileName;

	/** Load page graphics and sounds. */
	void loadResources();
	/** Create the primary menu controls. */
	void loadButtons();
	/** Refresh the profile list from the savegame manager. */
	void scanSaveFiles();
	/** Draw the main sign-in panel. */
	void drawMain(ManagedSurface32 *screen);
	/** Draw primary controls and update their retained hover transitions. */
	void drawButtonsAndUpdateHover(ManagedSurface32 *screen, const Common::Point32 &mousePos);

	/** Return the primary button at @p pos, or `-1` when none is hit. */
	int hitTestButton(const Common::Point &pos) const;
	/** Return whether @p pos falls inside the saved-game selector. */
	bool isInSelectorArea(const Common::Point &pos) const;
	/** Execute the action associated with @p buttonId. */
	void handleButtonClick(int buttonId);
	/** Apply one queued key code to profile-name editing. */
	void handleKeyInput(uint32 keyCode);
	/** Load the selected profile, or create it when it does not exist. */
	void startSelectedSave();
	/** Open deletion confirmation for the selected profile. */
	void requestDeleteConfirmation();
	/** Open the shared quit confirmation. */
	void requestQuitConfirmation();
	/** Apply the shared delete-confirmation result. */
	void handleDeleteConfirmation(DialogMsgBoxButton button);
	/** Apply the shared quit-confirmation result. */
	void handleQuitConfirmation(DialogMsgBoxButton button);

	/** Open the volume options panel. */
	void openOptionsDialog();
	/** Close options and optionally apply the panel values. */
	void closeOptionsDialog(bool applyChanges);
	/** Apply either panel or stored volume values, optionally persisting them for this target. */
	void applyOptionVolumes(bool usePanelValues, bool persistChanges);

	/** Delete the profile retained for the active confirmation. */
	void deleteSelectedSave();
	/** Play @p soundId when it names a loaded sound. */
	void playSound(int soundId);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_INTERACTIVE_MENU_H
