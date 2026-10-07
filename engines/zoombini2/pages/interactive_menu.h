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

#ifndef ZOOMBINI2_PAGES_INTERACTIVE_MENU_H
#define ZOOMBINI2_PAGES_INTERACTIVE_MENU_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/ustr.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/interactive_base.h"

namespace Zoombini2 {

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
 * @ref InteractiveMenu::SaveFileList.
 * The list manages filename selection and editing semantics.
 */
class InteractiveMenu : public InteractiveBase {
public:
	/** Construct the sign-in page for @p vm. */
	explicit InteractiveMenu(Zoombini2Engine *vm);
	/** Release the loaded resources and controls retained by this page. */
	~InteractiveMenu() override;

	/** Load sign-in resources and scan the available savefiles. */
	void init() override;
	/** Process text input and hover state for the active panel. */
	void onUpdate() override;
	/** Draw the saved-game list or the volume panel managed by this page. */
	void onRenderContent(ManagedSurface32 *screen) override;
	void onRenderForeground(ManagedSurface32 *screen) override;
	/** Dispatch a click to the list, buttons, or the volume panel managed by this page. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Toggle the file row under a right-button press when the test control is enabled. */
	EventHandleResult onRButtonDown(const Common::Point &pos) override;
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	bool hasActiveDialog() const override { return _state == MenuScreenState::kOptions01; }

private:
	/**
	 * Sorted save-name list used by the menu screen.
	 *
	 * Text colors are shared through @ref Gfx::loadTextFont.
	 * The selection bar is loaded through @ref Gfx::drawPageRleBlock and remains in the page bitmap cache.
	 * Savefile names are filename stems for in-game `.mk` saves.
	 * Names are limited to 16 characters.
	 * The viewport contains four rows.
	 */
	class SaveFileList {
	public:
		/** Editing stages used by keyboard entry and prefix matching. */
		enum EditState {
			kEditIdle00 = 0,        ///< No savefile name is being edited.
			kEditPrefixMatch01 = 1, ///< Typed text currently selects an existing prefix match.
			kEditProvisional02 = 2, ///< A new row exists but has not been explicitly confirmed.
			kEditExplicit03 = 3     ///< The new row is in explicit text-entry mode.
		};

		/** Result of appending one keyboard character. */
		enum TextInputResult {
			kTextRejected00 = 0, ///< The character did not change the edit buffer.
			kTextAccepted01 = 1, ///< The character was appended or used for prefix selection.
			kTextListFull02 = 2  ///< A new entry could not be created because the list is full.
		};

		/** Maximum number of savefile rows. */
		static constexpr int kMaximumItems = 99;
		/** Maximum number of characters in one savefile name. */
		static constexpr int kMaximumNameLength = 16;
		/** Number of rows visible in the list viewport. */
		static constexpr int kVisibleRows = 4;

		/** Construct an empty savefile list at @p pos; @p vm supplies the shared text renderer. */
		SaveFileList(Zoombini2Engine *vm, const Common::Point32 &pos);

		/** Prepare the normal, prefix-match, edit, and read-only colors through @ref Gfx::loadTextFont. */
		bool init();
		/** Remove every item and reset selection and editing state. */
		void clear();
		/** Insert @p name and its @p readOnly status in case-sensitive order unless it is invalid or duplicated. */
		bool addItemSorted(const Common::String &name, bool readOnly);

		/** Draw the visible rows and highlight the selected row through the page bitmap cache. */
		void draw(ManagedSurface32 *screen) const;
		/** Select the row at @p pos and report whether the list consumed the click. */
		bool handleClick(const Common::Point &pos);
		/** Normalize and apply @p c to selection or the active edit buffer. */
		TextInputResult handleCharacter(char c);
		/** Remove one edit character or leave explicit editing. */
		bool handleBackspace();
		/** Begin a provisional row or confirm the current edit buffer. */
		void beginOrConfirmNewEntry();

		/** Move selection one item toward the start of the list. */
		void moveSelectionUp();
		/** Move selection one item toward the end of the list. */
		void moveSelectionDown();
		/** Move selection one visible page toward the start of the list. */
		void scrollPageUp();
		/** Move selection one visible page toward the end of the list. */
		void scrollPageDown();

		/** Return whether selection can move toward the start of the list. */
		bool canMoveUp() const;
		/** Return whether selection can move toward the end of the list. */
		bool canMoveDown() const;
		/** Return whether a preceding visible page exists. */
		bool canPageUp() const;
		/** Return whether a following visible page exists. */
		bool canPageDown() const;
		/** Return whether list capacity allows a new savefile row. */
		bool canBeginNewEntry() const;
		/** Return whether selection refers to an existing list item. */
		bool hasValidSelection() const { return _validSelection; }
		/** Return whether a provisional or explicit new row is active. */
		bool isEditing() const { return kEditProvisional02 <= _editState; }
		/** Return whether the selected existing item is displayed as read-only. */
		bool isSelectedReadOnly() const;
		/** Return whether the edit buffer contains at least one character. */
		bool hasEditBuffer() const { return !_editBuffer.empty(); }

		/** Return the selected existing name or active edit buffer. */
		Common::String getSelectedName() const;
		/** Return an existing name differing from the active new entry only by letter case. */
		Common::String getCaseInsensitiveConflictName() const;
		/** Return the active editing stage. */
		EditState getEditState() const { return _editState; }
		/** Return the number of savefile rows. */
		int getItemCount() const { return static_cast<int>(_items.size()); }
		/** Return the selected row index. */
		int getSelectedIndex() const { return _selectedIndex; }
		/** Return the index of the first visible row. */
		int getScrollOffset() const { return _scrollOffset; }

		/** Remove the selected row and normalize the remaining selection. */
		void deleteSelected();

	private:
		/** Selection-bar horizontal offset from the list origin. */
		static constexpr int kSelectionOffsetX = 35;
		/** Selection-bar vertical offset from the list origin. */
		static constexpr int kSelectionOffsetY = 57;
		/** Vertical distance between adjacent visible rows. */
		static constexpr int kRowStride = 27;
		/** Text horizontal offset from the list origin. */
		static constexpr int kTextOffsetX = 8;
		/** Text vertical offset from the list origin. */
		static constexpr int kTextOffsetY = 7;
		/** Height of the Hebrew release's bitmap-font strip. */
		static constexpr int kHebrewFontHeight = 20;

		/** Borrowed vm used to access the shared text renderer. */
		Zoombini2Engine *_vm;
		/** Signed screen origin of the list. */
		Common::Point32 _pos;

		/** Savefile names in case-sensitive display order. */
		Common::Array<Common::String> _items;
		/** Physical read-only status for each corresponding stored savefile name. */
		Common::Array<bool> _readOnly;
		/** Text being used for prefix matching or a new savefile row. */
		Common::String _editBuffer;
		/** Active editing stage. */
		EditState _editState = kEditIdle00;
		/** Selected row index. */
		int _selectedIndex = 0;
		/** Index of the first visible row. */
		int _scrollOffset = 0;
		/** Whether @ref InteractiveMenu::SaveFileList::_selectedIndex names an existing item. */
		bool _validSelection = false;

		/** Return the sorted insertion index for @p name. */
		int findInsertionPoint(const Common::String &name) const;
		/** Return the first prefix match excluding @p ignoredIndex, or -1. */
		int findPrefix(const Common::String &prefix, int ignoredIndex = -1) const;
		/** Return whether @p name duplicates an item other than @p ignoredIndex. */
		bool isDuplicate(const Common::String &name, int ignoredIndex) const;
		/** Return whether @p c can extend the active edit buffer. */
		bool canAppendCharacter(char c) const;
		/** Convert @p c to the list's accepted display form, or zero when invalid. */
		char normalizeCharacter(char c) const;
		/** Insert @p name and its @p readOnly status at @p index and select it. */
		void insertItem(int index, const Common::String &name, bool readOnly = false);
		/** Remove the item at @p index. */
		void removeItem(int index);
		/** Adjust the viewport so the selected row is visible. */
		void revealSelection();
		/** Clamp selection and viewport after a list-size change. */
		void clampSelection();
	};

	/** Bit-block background behind the saved-game selector and menu controls. */
	static constexpr const char *kBackgroundPath = "#bmp/menu/background";
	/** Normal BitBlock frame around the saved-game selector. */
	static constexpr const char *kSelectorNormalPath = "bmp/menu/PARTIEs - selector NORMAL";
	/** Highlighted BitBlock frame drawn when the pointer is over the saved-game selector. */
	static constexpr const char *kSelectorHighlightPath = "bmp/menu/PARTIEs - selector HILITE";
	/** RLE highlight bar drawn behind the selected saved-game row. */
	static constexpr const char *kSelectionBarPath = "bmp/menu/barre-cache.rb";
	/** Menu selection sound played for list, button, and hover changes. */
	static constexpr const char *kBlipSoundPath = "sounds/blip.wav";
	/** Sound effect played when a savefile-name character is accepted. */
	static constexpr const char *kTypeSoundPath = "sounds/fx/i-bs5.wav";
	/** Sound effect played when a savefile-name character is deleted. */
	static constexpr const char *kDeleteSoundPath = "sounds/fx/del.wav";
	/** Normal BitBlock sprite for moving the saved-game selection toward earlier rows. */
	static constexpr const char *kArrowUpNormalPath = "bmp/menu/PARTIEs - ArrowUP NORMAL";
	/** Hover BitBlock sprite for moving the saved-game selection toward earlier rows. */
	static constexpr const char *kArrowUpHighlightPath = "bmp/menu/PARTIEs - ArrowUP HIGHLIGHT";
	/** Normal BitBlock sprite for moving the saved-game selection toward later rows. */
	static constexpr const char *kArrowDownNormalPath = "bmp/menu/PARTIEs - ArrowDOWN NORMAL";
	/** Hover BitBlock sprite for moving the saved-game selection toward later rows. */
	static constexpr const char *kArrowDownHighlightPath = "bmp/menu/PARTIEs - ArrowDOWN HILITE";
	/** Normal RLE sprite for loading or continuing the selected saved game. */
	static constexpr const char *kStartNormalPath = "bmp/menu/Start Normal";
	/** Hover RLE sprite for loading or continuing the selected saved game. */
	static constexpr const char *kStartHighlightPath = "bmp/menu/Start Highlight";
	/** Disabled RLE sprite for Start when the list has no valid selection. */
	static constexpr const char *kStartDisabledPath = "bmp/menu/Start Gray";
	/** Shared transparency mask applied to the Start and New button sprites. */
	static constexpr const char *kButtonMaskPath = "bmp/menu/button-a";
	/** Normal BitBlock sprite for opening the volume options panel. */
	static constexpr const char *kOptionsNormalPath = "bmp/menu/PANEL - Options  NORMAL";
	/** Hover BitBlock sprite for opening the volume options panel. */
	static constexpr const char *kOptionsHighlightPath = "bmp/menu/PANEL - Options  HIGHLIGHT";
	/** Normal RLE sprite for beginning savefile-name entry. */
	static constexpr const char *kNewNormalPath = "bmp/menu/New Normal";
	/** Hover RLE sprite for beginning savefile-name entry. */
	static constexpr const char *kNewHighlightPath = "bmp/menu/New Highlight";
	/** Disabled RLE sprite for New when the saved-game list is full. */
	static constexpr const char *kNewDisabledPath = "bmp/menu/New Gray";
	/** Normal BitBlock sprite for switching to practice mode. */
	static constexpr const char *kPracticeNormalPath = "bmp/menu/PANEL - Entrainement NORMAL";
	/** Hover BitBlock sprite for switching to practice mode. */
	static constexpr const char *kPracticeHighlightPath = "bmp/menu/PANEL - Entraine HILITE";
	/** Normal BitBlock sprite for opening the quit confirmation. */
	static constexpr const char *kQuitNormalPath = "bmp/menu/PANEL - Quitter NORMAL";
	/** Hover BitBlock sprite for opening the quit confirmation. */
	static constexpr const char *kQuitHighlightPath = "bmp/menu/PANEL - Quitter HIGHLIGHT";
	/** Bit-block message shown when the Delete action confirms removing the selected savefile. */
	static constexpr const char *kDeleteConfirmationPath = "bmp/menu/Quit_panel_text_suppr";
	/** Bit-block message shown when the Quit action confirms leaving the menu for credits. */
	static constexpr const char *kQuitConfirmationPath = "bmp/menu/Quit_panel_text_quit";
	/** English confirmation text for a case-only save-name collision. */
	static constexpr const char *kCaseCollisionConfirmationEnglish =
		"A save that differs only in letter case exists.\nThe new save will overwrite it.\nAre you sure to overwrite?";
	/** Korean confirmation text for a case-only save-name collision. */
	static constexpr const char *kCaseCollisionConfirmationKorean =
		u8"대소문자만 다른 저장파일 이름이 있습니다.\n새 파일은 기존 저장파일을 덮어씁니다.\n덮어쓸까요?";
	/** English confirmation text for opening a save without progress writes. */
	static constexpr const char *kReadOnlyLoadConfirmationEnglish =
		"This saved game will open in read-only mode.\nProgress will not be saved.\nContinue?";
	/** Korean confirmation text for opening a save without progress writes. */
	static constexpr const char *kReadOnlyLoadConfirmationKorean =
		u8"이 저장파일은 읽기 전용 모드로 열립니다.\n진행 상황은 저장되지 않습니다.\n계속할까요?";

	void updateButtonAvailability();
	EventHandleResult handleVolumePanelInput(const Common::Point &pos, bool mouseReleased);
	bool _volumePanelMouseDown = false;

	/** Screen origin of the saved-game list. */
	static constexpr Common::Point32 kFileListPos = Common::Point32(157, 286);
	static constexpr Size32 kSelectorSize = Size32(520, 201);
	static constexpr const char *kValidNameCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
	/** Characters accepted by the Hebrew release in addition to its Hebrew byte range. */
	static constexpr const char *kHebrewValidNameCharacters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,.; ";

	/** Panel currently accepting input. */
	MenuScreenState _state = MenuScreenState::kMain00;
	/** Savefile list and editor state managed by this menu screen. */
	SaveFileList *_fileList = nullptr;
	/** Primary menu controls indexed by menu button ID. */
	UIButton *_buttons[kMenuButtonCount] = {};

	/** Menu selection sound. */
	int _blipSoundId = -1;
	/** Savefile-name typing sound. */
	int _typeSoundId = -1;
	/** Savefile deletion sound. */
	int _deleteSoundId = -1;

	/** Volume panel managed by this page while options are open. */
	VolumePanel *_volumePanel = nullptr;
	/** Selected savefile retained while the shared delete confirmation is active. */
	Common::String _pendingDeleteSavefileName;
	/** New embedded player name retained while the case-collision confirmation is active. */
	Common::String _pendingCaseCollisionPlayerName;
	/** Existing storage savefile retained while the case-collision confirmation is active. */
	Common::String _pendingCaseCollisionSavefileName;
	/** Existing savefile retained while its read-only load confirmation is active. */
	Common::String _pendingReadOnlySavefileName;

	/** Load page graphics and sounds. */
	void loadResources();
	/** Create the primary menu controls. */
	void loadButtons();
	/** Refresh the savefile list from the savegame manager. */
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
	/** Apply one queued key code to savefile-name editing. */
	void handleKeyInput(uint32 keyCode);
	/** Load the selected savefile, or create it when it does not exist. */
	void startSelectedSave();
	/** Load @p savefileName and continue only after its serialized state is valid. */
	void loadSelectedSave(const Common::String &savefileName);
	/** Initialize and save a new game under @p savefileName. */
	void startNewSave(const Common::String &playerName, const Common::String &savefileName);
	/** Continue into a loaded or newly created savefile. */
	void continueSelectedSave();
	/** Open deletion confirmation for the selected savefile. */
	void requestDeleteConfirmation();
	/** Open a confirmation before replacing a savefile whose name differs only by case. */
	void requestCaseCollisionConfirmation(const Common::String &playerName, const Common::String &savefileName);
	/** Open a confirmation before entering the selected red read-only savefile. */
	void requestReadOnlyLoadConfirmation(const Common::String &savefileName);
	/** Open the shared quit confirmation. */
	void requestQuitConfirmation();
	/** Apply the shared delete-confirmation result. */
	void handleDeleteConfirmation(DialogMsgBoxButton button);
	/** Apply the case-collision confirmation result. */
	void handleCaseCollisionConfirmation(DialogMsgBoxButton button);
	/** Apply the read-only load confirmation result. */
	void handleReadOnlyLoadConfirmation(DialogMsgBoxButton button);
	/** Apply the shared quit-confirmation result. */
	void handleQuitConfirmation(DialogMsgBoxButton button);
	/** Return release-appropriate case-collision confirmation text. */
	Common::U32String getCaseCollisionConfirmationText() const;
	/** Return release-appropriate read-only load confirmation text. */
	Common::U32String getReadOnlyLoadConfirmationText() const;

	/** Open the volume options panel. */
	void openOptionsDialog();
	/** Close options and optionally apply the panel values. */
	void closeOptionsDialog(bool applyChanges);
	/** Apply either panel or stored volume values, optionally persisting them for this target. */
	void applyOptionVolumes(bool usePanelValues, bool persistChanges);

	/** Delete the savefile retained for the active confirmation. */
	void deleteSelectedSave();
	/** Play @p soundId when it names a loaded sound. */
	void playSound(int soundId);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_INTERACTIVE_MENU_H
