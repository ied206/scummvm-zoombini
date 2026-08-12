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

class BitmapFont;
class RleBlock;

/**
 * Sorted save-name list used by the menu screen.
 *
 * The list owns its three text fonts. The selection bar is supplied by the
 * menu screen and remains owned by that screen. Names are filename stems,
 * limited to 16 characters, and the viewport contains four rows.
 */
class SaveFileList {
public:
	enum EditState {
		kEditIdle = 0,
		kEditPrefixMatch = 1,
		kEditProvisional = 2,
		kEditExplicit = 3
	};

	enum TextInputResult {
		kTextRejected = 0,
		kTextAccepted = 1,
		kTextListFull = 2
	};

	static const int kMaximumItems = 99;
	static const int kMaximumNameLength = 16;
	static const int kVisibleRows = 4;

	SaveFileList(int x, int y, RleBlock *selectionBar);
	~SaveFileList();

	bool init();
	void clear();
	bool addItemSorted(const Common::String &name);

	void draw(Graphics::ManagedSurface *screen, const byte alphaLUT[256][256]) const;
	bool handleClick(const Common::Point &pos);
	TextInputResult handleCharacter(char c);
	bool handleBackspace();
	void beginOrConfirmNewEntry();

	void moveSelectionUp();
	void moveSelectionDown();
	void scrollPageUp();
	void scrollPageDown();

	bool canMoveUp() const;
	bool canMoveDown() const;
	bool canPageUp() const;
	bool canPageDown() const;
	bool canBeginNewEntry() const;
	bool hasValidSelection() const { return _validSelection; }
	bool isEditing() const { return kEditProvisional <= _editState; }
	bool hasEditBuffer() const { return !_editBuffer.empty(); }

	Common::String getSelectedName() const;
	EditState getEditState() const { return _editState; }
	int getItemCount() const { return static_cast<int>(_items.size()); }
	int getSelectedIndex() const { return _selectedIndex; }
	int getScrollOffset() const { return _scrollOffset; }

	void deleteSelected();

private:
	static const int kSelectionOffsetX = 35;
	static const int kSelectionOffsetY = 57;
	static const int kRowStride = 27;
	static const int kTextOffsetX = 8;
	static const int kTextOffsetY = 7;

	int _x;
	int _y;
	RleBlock *_selectionBar;
	BitmapFont *_defaultFont;
	BitmapFont *_matchFont;
	BitmapFont *_editFont;

	Common::Array<Common::String> _items;
	Common::String _editBuffer;
	EditState _editState;
	int _selectedIndex;
	int _scrollOffset;
	bool _validSelection;

	int findInsertionPoint(const Common::String &name) const;
	int findPrefix(const Common::String &prefix, int ignoredIndex = -1) const;
	bool isDuplicate(const Common::String &name, int ignoredIndex) const;
	bool canAppendCharacter(char c) const;
	char normalizeCharacter(char c) const;
	void insertItem(int index, const Common::String &name);
	void removeItem(int index);
	void revealSelection();
	void clampSelection();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SAVE_FILE_LIST_H
