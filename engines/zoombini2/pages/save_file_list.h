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

#ifndef ZOOMBINI2_PAGES_SAVE_FILE_LIST_H
#define ZOOMBINI2_PAGES_SAVE_FILE_LIST_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Graphics {
class ManagedSurface;
}

namespace Zoombini2 {

class AlphaBlendLUT;
class BitmapFont;
class RleBlock;
class ManagedSurface32;

/**
 * Sorted save-name list used by the menu screen.
 *
 * The list owns its three text fonts. The selection bar is supplied by the
 * menu screen and remains owned by that screen. Names are filename stems,
 * limited to 16 characters, and the viewport contains four rows.
 */
class SaveFileList {
public:
	/** Editing stages used by keyboard entry and prefix matching. */
	enum EditState {
		kEditIdle00 = 0,        ///< No profile name is being edited.
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

	/** Maximum number of profile rows. */
	static const int kMaximumItems = 99;
	/** Maximum number of characters in one profile name. */
	static const int kMaximumNameLength = 16;
	/** Number of rows visible in the list viewport. */
	static const int kVisibleRows = 4;

	/** Construct a list at @p x and @p y using the borrowed @p selectionBar. */
	SaveFileList(const Common::Point32 &pos, RleBlock *selectionBar);
	/** Release the list's three owned fonts. */
	~SaveFileList();

	/** Load the normal, prefix-match, and edit fonts. */
	bool init();
	/** Remove every item and reset selection and editing state. */
	void clear();
	/** Insert @p name in case-insensitive order unless it is invalid or duplicated. */
	bool addItemSorted(const Common::String &name);

	/** Draw the visible rows and the borrowed selection bar. */
	void draw(ManagedSurface32 *screen, const AlphaBlendLUT &alphaLUT) const;
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
	/** Return whether list capacity allows a new profile row. */
	bool canBeginNewEntry() const;
	/** Return whether selection refers to an existing list item. */
	bool hasValidSelection() const { return _validSelection; }
	/** Return whether a provisional or explicit new row is active. */
	bool isEditing() const { return kEditProvisional02 <= _editState; }
	/** Return whether the edit buffer contains at least one character. */
	bool hasEditBuffer() const { return !_editBuffer.empty(); }

	/** Return the selected existing name or active edit buffer. */
	Common::String getSelectedName() const;
	/** Return the active editing stage. */
	EditState getEditState() const { return _editState; }
	/** Return the number of profile rows. */
	int getItemCount() const { return static_cast<int>(_items.size()); }
	/** Return the selected row index. */
	int getSelectedIndex() const { return _selectedIndex; }
	/** Return the index of the first visible row. */
	int getScrollOffset() const { return _scrollOffset; }

	/** Remove the selected row and normalize the remaining selection. */
	void deleteSelected();

private:
	/** Selection-bar horizontal offset from the list origin. */
	static const int kSelectionOffsetX = 35;
	/** Selection-bar vertical offset from the list origin. */
	static const int kSelectionOffsetY = 57;
	/** Vertical distance between adjacent visible rows. */
	static const int kRowStride = 27;
	/** Text horizontal offset from the list origin. */
	static const int kTextOffsetX = 8;
	/** Text vertical offset from the list origin. */
	static const int kTextOffsetY = 7;

	/** Signed screen origin of the list. */
	Common::Point32 _pos;
	/** Borrowed selection-bar sprite owned by the menu screen. */
	RleBlock *_selectionBar;
	/** Font managed by this list for ordinary existing rows. */
	BitmapFont *_defaultFont;
	/** Font managed by this list for the row matching a typed prefix. */
	BitmapFont *_matchFont;
	/** Font managed by this list for a provisional or explicitly edited row. */
	BitmapFont *_editFont;

	/** Profile names in case-insensitive display order. */
	Common::Array<Common::String> _items;
	/** Text being used for prefix matching or a new profile row. */
	Common::String _editBuffer;
	/** Active editing stage. */
	EditState _editState;
	/** Selected row index. */
	int _selectedIndex;
	/** Index of the first visible row. */
	int _scrollOffset;
	/** Whether @ref SaveFileList::_selectedIndex names an existing item. */
	bool _validSelection;

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
	/** Insert @p name at @p index and select it. */
	void insertItem(int index, const Common::String &name);
	/** Remove the item at @p index. */
	void removeItem(int index);
	/** Adjust the viewport so the selected row is visible. */
	void revealSelection();
	/** Clamp selection and viewport after a list-size change. */
	void clampSelection();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SAVE_FILE_LIST_H
