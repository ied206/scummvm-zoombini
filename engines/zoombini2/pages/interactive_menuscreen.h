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

#ifndef ZOOMBINI2_PAGES_INTERACTIVE_MENUSCREEN_H
#define ZOOMBINI2_PAGES_INTERACTIVE_MENUSCREEN_H

#include "common/rect.h"

#include "zoombini2/pages/interactive_base.h"
#include "zoombini2/ui.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;
class SaveFileList;

enum SaveMenuState {
	/** Display the saved-game list and its primary actions. */
	kSaveMenuMain00 = 0,
	/** Display the volume options panel. */
	kSaveMenuOptions01 = 1,
	/** Display a destructive-action confirmation panel. */
	kSaveMenuConfirm02 = 2
};

enum SaveMenuConfirmType {
	/** No confirmation request is active. */
	kSaveMenuConfirmNone00 = 0,
	/** Confirm deletion of the selected profile. */
	kSaveMenuConfirmDelete01 = 1,
	/** Confirm leaving the game. */
	kSaveMenuConfirmQuit02 = 2
};

/**
 * Sign-in screen containing a four-row sorted saved-game list.
 *
 * The page owns visual resources, buttons, sounds, the options panel, and a
 * @ref SaveFileList. The list owns filename selection and editing semantics.
 */
class MenuScreenPage : public InteractivePage {
public:
	/** Construct the sign-in page for @p engine. */
	explicit MenuScreenPage(Zoombini2Engine *engine);
	/** Release the loaded resources and owned controls. */
	~MenuScreenPage() override;

	/** Load sign-in resources and scan the available profiles. */
	void init() override;
	/** Process text input and hover state for the active panel. */
	void update() override;
	/** Draw the saved-game list or the active modal panel. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Dispatch a click to the list, buttons, or active modal panel. */
	void handleClick(const Common::Point &pos) override;

private:
	/** Screen origin of the saved-profile list. */
	static const Common::Point32 kFileListPosition;
	static const int kSelectorWidth = 520;
	static const int kSelectorHeight = 201;
	static const char *const kValidNameCharacters;

	/** Panel currently accepting input. */
	SaveMenuState _state;
	/** Full-screen sign-in background. */
	BitBlock *_background;
	/** Normal saved-game list frame. */
	BitBlock *_selectorNormal;
	/** Highlighted saved-game list frame. */
	BitBlock *_selectorHilite;
	/** Bar drawn behind the selected save row. */
	RleBlock *_selectionBar;
	/** Owned profile list and editor state. */
	SaveFileList *_fileList;
	/** Primary menu controls indexed by menu button ID. */
	UIButton *_buttons[kMenuButtonCount];

	/** Menu selection sound. */
	int _blipSoundId;
	/** Profile-name typing sound. */
	int _typeSoundId;
	/** Profile deletion sound. */
	int _deleteSoundId;
	/** Shared map music handle requested by this page. */
	int _mapMusicId;

	/** Owned volume panel while options are open. */
	VolumePanel *_volumePanel;
	/** Confirmation action currently being presented. */
	SaveMenuConfirmType _confirmType;
	/** Confirmation panel background without a highlighted button. */
	RleBlock *_confirmPanelNothing;
	/** Confirmation panel background with OK highlighted. */
	RleBlock *_confirmPanelOk;
	/** Confirmation panel background with Cancel highlighted. */
	RleBlock *_confirmPanelCancel;
	/** Delete-confirmation text. */
	BitBlock *_confirmTextDelete;
	/** Quit-confirmation text. */
	BitBlock *_confirmTextQuit;
	/** Hovered confirmation button, or zero when neither is hovered. */
	int _confirmButtonHover;
	/** Signed screen origin of the confirmation panel. */
	Common::Point32 _confirmPosition;
	/** Saved pixels restored when the confirmation panel closes. */
	Graphics::ManagedSurface *_confirmDialogBackground;

	/** Load page graphics, sounds, and modal resources. */
	void loadResources();
	/** Create the primary menu controls. */
	void loadButtons();
	/** Refresh the profile list from the savegame manager. */
	void scanSaveFiles();
	/** Draw the main sign-in panel. */
	void drawMain(Graphics::ManagedSurface *screen);
	/** Draw primary controls using the supplied mouse position. */
	void drawButtons(Graphics::ManagedSurface *screen, int mouseX, int mouseY);

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

	/** Open the volume options panel. */
	void openOptionsDialog();
	/** Close options and optionally apply the panel values. */
	void closeOptionsDialog(bool applyChanges);
	/** Apply either panel or stored volume values to the engine. */
	void applyOptionVolumes(bool usePanelValues);

	/** Open the confirmation panel for @p type. */
	void openConfirmDialog(SaveMenuConfirmType type);
	/** Close the active confirmation panel. */
	void closeConfirmDialog();
	/** Draw the active confirmation panel. */
	void drawConfirmDialog(Graphics::ManagedSurface *screen);
	/** Return the confirmation control at @p x and @p y, or zero. */
	int hitTestConfirmDialog(int x, int y) const;
	/** Dispatch @p buttonId within the active confirmation panel. */
	void handleConfirmClick(int buttonId);
	/** Delete the selected profile and refresh the list. */
	void deleteSelectedSave();
	/** Play @p soundId when it names a loaded sound. */
	void playSound(int soundId);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_INTERACTIVE_MENUSCREEN_H
