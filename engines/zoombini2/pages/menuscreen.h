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

#ifndef ZOOMBINI2_PAGES_MENUSCREEN_H
#define ZOOMBINI2_PAGES_MENUSCREEN_H

#include "zoombini2/pages/page.h"
#include "zoombini2/ui.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;
class SaveFileList;

enum SaveMenuState {
	kSaveMenuMain = 0,
	kSaveMenuOptions = 1,
	kSaveMenuConfirm = 2
};

enum SaveMenuConfirmType {
	kSaveMenuConfirmNone = 0,
	kSaveMenuConfirmDelete = 1,
	kSaveMenuConfirmQuit = 2
};

/**
 * Save-file menu containing a four-row sorted filename list.
 *
 * The page owns visual resources, buttons, sounds, the options panel, and a
 * @ref SaveFileList. The list owns filename selection and editing semantics.
 */
class MenuScreenPage : public Page {
public:
	explicit MenuScreenPage(Zoombini2Engine *engine);
	~MenuScreenPage() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	static const int kFileListX = 157;
	static const int kFileListY = 286;
	static const int kSelectorWidth = 520;
	static const int kSelectorHeight = 201;
	static const char *const kValidNameCharacters;

	SaveMenuState _state;
	BitBlock *_background;
	BitBlock *_selectorNormal;
	BitBlock *_selectorHilite;
	RleBlock *_selectionBar;
	SaveFileList *_fileList;
	UIButton *_buttons[kMenuButtonCount];

	int _blipSoundId;
	int _typeSoundId;
	int _deleteSoundId;
	int _mapMusicId;

	VolumePanel *_volumePanel;
	SaveMenuConfirmType _confirmType;
	RleBlock *_confirmPanelNothing;
	RleBlock *_confirmPanelOk;
	RleBlock *_confirmPanelCancel;
	BitBlock *_confirmTextDelete;
	BitBlock *_confirmTextQuit;
	int _confirmButtonHover;
	int _confirmX;
	int _confirmY;

	void loadResources();
	void loadButtons();
	void scanSaveFiles();
	void drawMain(Graphics::ManagedSurface *screen);
	void drawButtons(Graphics::ManagedSurface *screen, int mouseX, int mouseY);

	int hitTestButton(const Common::Point &pos) const;
	bool isInSelectorArea(const Common::Point &pos) const;
	void handleButtonClick(int buttonId);
	void handleKeyInput(uint32 keyCode);
	void startSelectedSave();
	void requestDeleteConfirmation();

	void openOptionsDialog();
	void closeOptionsDialog(bool applyChanges);
	void applyOptionVolumes(bool usePanelValues);

	void openConfirmDialog(SaveMenuConfirmType type);
	void closeConfirmDialog();
	void drawConfirmDialog(Graphics::ManagedSurface *screen);
	int hitTestConfirmDialog(int x, int y) const;
	void handleConfirmClick(int buttonId);
	void deleteSelectedSave();
	void playSound(int soundId);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_MENUSCREEN_H
