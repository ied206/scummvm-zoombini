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

#include "common/algorithm.h"

#include "zoombini2/gfx.h"
#include "zoombini2/pages/save_file_list.h"
#include "zoombini2/ui.h"

namespace Zoombini2 {

SaveFileList::SaveFileList(int x, int y, RleBlock *selectionBar)
	: _x(x), _y(y), _selectionBar(selectionBar), _defaultFont(nullptr),
	  _matchFont(nullptr), _editFont(nullptr), _editState(kEditIdle),
	  _selectedIndex(0), _scrollOffset(0), _validSelection(false) {
}

SaveFileList::~SaveFileList() {
	delete _defaultFont;
	delete _matchFont;
	delete _editFont;
}

bool SaveFileList::init() {
	_defaultFont = new BitmapFont();
	_matchFont = new BitmapFont();
	_editFont = new BitmapFont();

	const Common::Path fontPath("bmp/typo");
	return _defaultFont->load(fontPath, 16, 16, 16) &&
	       _matchFont->load(fontPath, 0, 0, 255) &&
	       _editFont->load(fontPath, 0, 255, 0);
}

void SaveFileList::clear() {
	_items.clear();
	_editBuffer.clear();
	_editState = kEditIdle;
	_selectedIndex = 0;
	_scrollOffset = 0;
	_validSelection = false;
}

bool SaveFileList::addItemSorted(const Common::String &name) {
	if (kMaximumItems <= static_cast<int>(_items.size()) || name.empty() ||
	    kMaximumNameLength < static_cast<int>(name.size()))
		return false;

	insertItem(findInsertionPoint(name), name);
	_selectedIndex = 0;
	_scrollOffset = 0;
	_validSelection = true;
	return true;
}

void SaveFileList::draw(Graphics::ManagedSurface *screen, const byte alphaLUT[256][256]) const {
	for (int row = 0; row < kVisibleRows; ++row) {
		const int itemIndex = _scrollOffset + row;
		if (static_cast<int>(_items.size()) <= itemIndex)
			break;

		const bool selected = itemIndex == _selectedIndex;
		if (selected && _selectionBar && _selectionBar->isValid()) {
			_selectionBar->drawToScreen(screen, _x + kSelectionOffsetX,
			                            _y + kSelectionOffsetY + row * kRowStride,
			                            alphaLUT);
		}

		BitmapFont *font = _defaultFont;
		if (selected && _editState == kEditPrefixMatch)
			font = _matchFont;
		else if (selected && kEditProvisional <= _editState)
			font = _editFont;

		if (font && font->isLoaded()) {
			font->drawString(screen, _x + kSelectionOffsetX + kTextOffsetX,
			                 _y + kSelectionOffsetY + kTextOffsetY + row * kRowStride,
			                 _items[itemIndex], alphaLUT);
		}
	}
}

bool SaveFileList::handleClick(const Common::Point &pos) {
	if (kEditPrefixMatch < _editState || !_selectionBar || !_selectionBar->isValid())
		return false;

	const int left = _x + kSelectionOffsetX;
	const int top = _y + kSelectionOffsetY;
	const int width = _selectionBar->getWidth();
	const int height = _selectionBar->getHeight() * kVisibleRows;
	if (pos.x < left || left + width <= pos.x || pos.y < top || top + height <= pos.y)
		return false;

	const int row = (pos.y - top) / kRowStride;
	const int itemIndex = _scrollOffset + row;
	if (row < 0 || kVisibleRows <= row || static_cast<int>(_items.size()) <= itemIndex)
		return false;

	_selectedIndex = itemIndex;
	_editBuffer.clear();
	_editState = kEditIdle;
	_validSelection = true;
	return true;
}

SaveFileList::TextInputResult SaveFileList::handleCharacter(char c) {
	if (!canAppendCharacter(c))
		return kTextRejected;

	c = normalizeCharacter(c);
	_editBuffer += c;

	if (_editState <= kEditPrefixMatch) {
		const int matchingIndex = findPrefix(_editBuffer);
		if (0 <= matchingIndex) {
			_selectedIndex = matchingIndex;
			_editState = kEditPrefixMatch;
		} else {
			if (kMaximumItems <= static_cast<int>(_items.size())) {
				_editBuffer.deleteLastChar();
				return kTextListFull;
			}

			_selectedIndex = findInsertionPoint(_editBuffer);
			insertItem(_selectedIndex, _editBuffer);
			_editState = kEditProvisional;
		}
		_validSelection = true;
	} else if (_editState == kEditProvisional) {
		_items[_selectedIndex] = _editBuffer;
		_validSelection = true;
	} else {
		_items[_selectedIndex] = _editBuffer;
		_validSelection = !isDuplicate(_editBuffer, _selectedIndex);
	}

	revealSelection();
	return kTextAccepted;
}

bool SaveFileList::handleBackspace() {
	if (_editBuffer.empty() && _editState != kEditExplicit)
		return false;
	if (_editBuffer.empty()) {
		removeItem(_selectedIndex);
		_editState = kEditIdle;
		clampSelection();
		_validSelection = !_items.empty();
		revealSelection();
		return true;
	}

	_editBuffer.deleteLastChar();

	if (_editState == kEditPrefixMatch) {
		if (_editBuffer.empty()) {
			_editState = kEditIdle;
			_validSelection = !_items.empty();
		} else {
			const int matchingIndex = findPrefix(_editBuffer);
			if (0 <= matchingIndex)
				_selectedIndex = matchingIndex;
			_validSelection = 0 <= matchingIndex;
		}
	} else if (_editState == kEditProvisional) {
		if (_editBuffer.empty()) {
			removeItem(_selectedIndex);
			_editState = kEditIdle;
			_selectedIndex = 0;
			_scrollOffset = 0;
			_validSelection = !_items.empty();
		} else {
			const int provisionalIndex = _selectedIndex;
			const int matchingIndex = findPrefix(_editBuffer, provisionalIndex);
			if (0 <= matchingIndex) {
				removeItem(provisionalIndex);
				_selectedIndex = matchingIndex;
				if (provisionalIndex < matchingIndex)
					--_selectedIndex;
				_editState = kEditPrefixMatch;
			} else {
				_items[_selectedIndex] = _editBuffer;
			}
			_validSelection = true;
		}
	} else if (_editState == kEditExplicit) {
		if (_editBuffer.empty()) {
			removeItem(_selectedIndex);
			_editState = kEditIdle;
			clampSelection();
			_validSelection = !_items.empty();
		} else {
			_items[_selectedIndex] = _editBuffer;
			_validSelection = !isDuplicate(_editBuffer, _selectedIndex);
		}
	}

	revealSelection();
	return true;
}

void SaveFileList::beginOrConfirmNewEntry() {
	if (!canBeginNewEntry())
		return;

	if (_editState == kEditIdle) {
		insertItem(0, Common::String());
		_selectedIndex = 0;
		_scrollOffset = 0;
		_editBuffer.clear();
		_editState = kEditExplicit;
		_validSelection = false;
	} else if (_editState == kEditPrefixMatch) {
		insertItem(_selectedIndex, _editBuffer);
		_editState = kEditExplicit;
		_validSelection = !isDuplicate(_editBuffer, _selectedIndex);
	} else if (_editState == kEditProvisional) {
		_editState = kEditExplicit;
		_validSelection = !isDuplicate(_editBuffer, _selectedIndex);
	}

	revealSelection();
}

void SaveFileList::moveSelectionUp() {
	if (!canMoveUp())
		return;
	--_selectedIndex;
	_editBuffer.clear();
	revealSelection();
}

void SaveFileList::moveSelectionDown() {
	if (!canMoveDown())
		return;
	++_selectedIndex;
	_editBuffer.clear();
	revealSelection();
}

void SaveFileList::scrollPageUp() {
	if (!canPageUp())
		return;
	_scrollOffset = MAX(0, _scrollOffset - kVisibleRows);
	if (_scrollOffset + kVisibleRows <= _selectedIndex)
		_selectedIndex = _scrollOffset + kVisibleRows - 1;
}

void SaveFileList::scrollPageDown() {
	if (!canPageDown())
		return;
	_scrollOffset = MIN(static_cast<int>(_items.size()) - kVisibleRows,
	                    _scrollOffset + kVisibleRows);
	if (_selectedIndex < _scrollOffset)
		_selectedIndex = _scrollOffset;
}

bool SaveFileList::canMoveUp() const {
	return _editState == kEditIdle && 0 < _selectedIndex;
}

bool SaveFileList::canMoveDown() const {
	return _editState == kEditIdle && _selectedIndex + 1 < static_cast<int>(_items.size());
}

bool SaveFileList::canPageUp() const {
	return _editState == kEditIdle && 0 < _scrollOffset;
}

bool SaveFileList::canPageDown() const {
	return _editState == kEditIdle && _scrollOffset + kVisibleRows < static_cast<int>(_items.size());
}

bool SaveFileList::canBeginNewEntry() const {
	return _items.size() < kMaximumItems && !isEditing();
}

Common::String SaveFileList::getSelectedName() const {
	if (!_validSelection || _selectedIndex < 0 || static_cast<int>(_items.size()) <= _selectedIndex)
		return Common::String();
	return _items[_selectedIndex];
}

void SaveFileList::deleteSelected() {
	if (_selectedIndex < 0 || static_cast<int>(_items.size()) <= _selectedIndex)
		return;

	removeItem(_selectedIndex);
	_editBuffer.clear();
	_editState = kEditIdle;
	clampSelection();
	_validSelection = !_items.empty();
}

int SaveFileList::findInsertionPoint(const Common::String &name) const {
	int index = 0;
	while (index < static_cast<int>(_items.size()) && _items[index].compareToIgnoreCase(name) < 0)
		++index;
	return index;
}

int SaveFileList::findPrefix(const Common::String &prefix, int ignoredIndex) const {
	for (int i = 0; i < static_cast<int>(_items.size()); ++i) {
		if (i != ignoredIndex && _items[i].hasPrefixIgnoreCase(prefix))
			return i;
	}
	return -1;
}

bool SaveFileList::isDuplicate(const Common::String &name, int ignoredIndex) const {
	for (int i = 0; i < static_cast<int>(_items.size()); ++i) {
		if (i != ignoredIndex && _items[i].equalsIgnoreCase(name))
			return true;
	}
	return false;
}

bool SaveFileList::canAppendCharacter(char c) const {
	if (kMaximumNameLength <= static_cast<int>(_editBuffer.size()))
		return false;
	if (c == ' ' && (_editBuffer.empty() || _editBuffer.lastChar() == ' '))
		return false;

	Common::String prospective = _editBuffer;
	prospective += normalizeCharacter(c);
	if (!_defaultFont || !_defaultFont->isLoaded() || !_selectionBar || !_selectionBar->isValid())
		return true;
	return _defaultFont->getStringWidth(prospective) <= _selectionBar->getWidth() - 5;
}

char SaveFileList::normalizeCharacter(char c) const {
	const bool capitalize = _editBuffer.empty() || _editBuffer.lastChar() == ' ';
	if ('a' <= c && c <= 'z')
		return capitalize ? c - 'a' + 'A' : c;
	if ('A' <= c && c <= 'Z')
		return capitalize ? c : c - 'A' + 'a';
	return c;
}

void SaveFileList::insertItem(int index, const Common::String &name) {
	_items.insert_at(index, name);
}

void SaveFileList::removeItem(int index) {
	if (0 <= index && index < static_cast<int>(_items.size()))
		_items.remove_at(index);
}

void SaveFileList::revealSelection() {
	if (_items.empty()) {
		_selectedIndex = 0;
		_scrollOffset = 0;
		return;
	}

	clampSelection();
	if (_selectedIndex < _scrollOffset || _scrollOffset + kVisibleRows <= _selectedIndex) {
		_scrollOffset = _selectedIndex - kVisibleRows / 2;
		_scrollOffset = MAX(0, _scrollOffset);
		_scrollOffset = MIN(_scrollOffset, MAX(0, static_cast<int>(_items.size()) - kVisibleRows));
	}
}

void SaveFileList::clampSelection() {
	if (_items.empty()) {
		_selectedIndex = 0;
		_scrollOffset = 0;
		return;
	}

	_selectedIndex = CLIP(_selectedIndex, 0, static_cast<int>(_items.size()) - 1);
	_scrollOffset = CLIP(_scrollOffset, 0, MAX(0, static_cast<int>(_items.size()) - kVisibleRows));
}

} // End of namespace Zoombini2
