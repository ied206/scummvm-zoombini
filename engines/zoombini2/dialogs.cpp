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

#include "common/config-manager.h"
#include "common/fs.h"
#include "common/gui_options.h"
#include "common/language.h"
#include "common/system.h"
#include "common/translation.h"
#include "common/util.h"

#include "gui/ThemeEval.h"
#include "gui/browser.h"
#include "gui/gui-manager.h"
#include "gui/message.h"
#include "gui/widgets/edittext.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/scrollcontainer.h"

#include "zoombini2/dialogs.h"
#include "zoombini2/graphics.h"
#include "zoombini2/metaengine.h"
#include "zoombini2/random.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr uint32 Zoombini2SaveManagementDialog::kEditSavefileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kDuplicateSavefileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kDeleteSavefileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kImportSavefileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kExportSavefileCommand;
constexpr uint32 Zoombini2SaveManagementDialog::kSavefileSelectionChangedCommand;
constexpr uint32 Zoombini2OptionsWidget::kManageSavefilesCommand;
constexpr uint32 Zoombini2OptionsWidget::kUnlockFrameRateCommand;

class Zoombini2SaveManagementDialog::SavefileListWidget : public GUI::ScrollContainerWidget {
public:
	/** Construct a save list that describes corrupted rows on hover. */
	SavefileListWidget(GUI::GuiObject *boss, int x, int y, int w, int h)
		: GUI::ScrollContainerWidget(boss, x, y, w, h) {
		setFlags(GUI::WIDGET_DYN_TOOLTIP);
	}

	/** Associate a row's current position and full height with its validation state. */
	void setRowState(uint row, GUI::Widget *widget, int height, bool corrupted) {
		if (_rows.size() <= row)
			_rows.resize(row + 1);

		_rows[row]._widget = widget;
		_rows[row]._height = height;
		_rows[row]._corrupted = corrupted;
	}

	/** Show corruption guidance when hovering over any part of a damaged row. */
	void handleTooltipUpdate(int x, int y) override {
		(void)x;
		setTooltip(Common::U32String());

		// Compare absolute positions so hit testing follows the scrolled rows.
		const int absY = getAbsY() + y;
		for (uint i = 0; i < _rows.size(); i++) {
			const RowState &row = _rows[i];
			if (!row._widget || !row._widget->isVisible())
				continue;

			const int rowTop = row._widget->getAbsY();
			if (rowTop <= absY && absY < rowTop + row._height) {
				if (row._corrupted)
					setTooltip(_("Corrupted save"));
				return;
			}
		}
	}

private:
	/** Visible row geometry and whether its savefile could be parsed. */
	struct RowState {
		/** First cell, whose position follows scrolling and layout changes. */
		GUI::Widget *_widget = nullptr;
		/** Full row height, including statistics on a second line. */
		int _height = 0;
		/** Whether the row represents an invalid savefile. */
		bool _corrupted = false;
	};

	/** Row references refreshed when the table is laid out. */
	Common::Array<RowState> _rows;
};

class Zoombini2OptionsWidget::SeparatorWidget : public GUI::Widget {
public:
	/** Construct a themed horizontal separator in the supplied GUI container. */
	SeparatorWidget(GUI::GuiObject *boss, const Common::String &name)
		: GUI::Widget(boss, name) {
		setFlags(GUI::WIDGET_ENABLED | GUI::WIDGET_CLEARBG);
	}

protected:
	/** Draw the themed separator across this widget's bounds. */
	void drawWidget() override {
		g_gui.theme()->drawLineSeparator(Common::Rect(_x, _y, _x + _w, _y + _h));
	}
};

class Zoombini2OptionsWidget::FrameRateNumberBox : public GUI::EditTextWidget {
public:
	/** Construct an integer-only frame-rate editor with the supplied initial FPS value. */
	FrameRateNumberBox(GUI::GuiObject *boss, const Common::String &name, int value, const Common::U32String &tooltip)
		: GUI::EditTextWidget(boss, name, Common::U32String::format("%d", value), tooltip), _value(value) {}

	/** Set the accepted FPS value and replace the editor's displayed text. */
	void setValue(int value) {
		_value = value;
		setEditString(Common::U32String::format("%d", value));
	}

	/** Return the edited FPS within configured limits, or the last accepted value for empty or invalid text. */
	int getValue() const {
		const Common::U32String &text = getEditString();
		if (text.empty())
			return _value;
		int value = 0;
		for (uint i = 0; i < text.size(); i++) {
			if (text[i] < '0' || '9' < text[i])
				return _value;
			value = MIN<int>(Zoombini2MetaEngine::kMaxFrameRate, value * 10 + static_cast<int>(text[i] - '0'));
		}
		return CLIP<int>(value, Zoombini2MetaEngine::kMinFrameRate, Zoombini2MetaEngine::kMaxFrameRate);
	}

protected:
	/** Accept decimal digits for the frame-rate entry. */
	bool isCharAllowed(Common::u32char_type_t character) const override {
		return '0' <= character && character <= '9';
	}

	/** Normalize the edited FPS text before releasing keyboard focus. */
	void lostFocusWidget() override {
		setValue(getValue());
		GUI::EditTextWidget::lostFocusWidget();
	}

private:
	/** Last accepted frame rate in FPS, used while the edit text is empty or invalid. */
	int _value;
};

Zoombini2MenuDialog::Zoombini2MenuDialog(Zoombini2Engine *vm)
	: MainMenuDialog(vm), _vm(vm) {
}

void Zoombini2MenuDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kOptionsCmd) {
		GUI::ConfigDialog configDialog;
		configDialog.runModal();

		_vm->applyGameSettings();
		_vm->syncSoundSettings();
		return;
	}

	MainMenuDialog::handleCommand(sender, cmd, data);
}

Zoombini2SavefileNameDialog::Zoombini2SavefileNameDialog(const Common::U32String &title, const Common::U32String &initialName, Common::Language language)
	: GUI::Dialog(0, 0, 1, 1) {
	_title = new GUI::StaticTextWidget(this, 0, 0, 1, 1, title, Graphics::kTextAlignStart);

	if (language == Common::HE_ISR)
		_nameHint = _("1-16 letters or spaces; , . ; also allowed");
	else
		_nameHint = _("Use 1 to 16 letters, digits, or spaces");

	// Defer sizing until the current theme and available overlay area are known.
	_descriptionScroller = new GUI::ScrollContainerWidget(this, 0, 0, 1, 1);
	_descriptionScroller->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);

	_edit = new GUI::EditTextWidget(this, 0, 0, 1, 1, false, initialName);

	_okButton = new GUI::ButtonWidget(this, 0, 0, 1, 1, _("OK"), Common::U32String(), GUI::kOKCmd);
	_cancelButton = new GUI::ButtonWidget(this, 0, 0, 1, 1, _("Cancel"), Common::U32String(), GUI::kCloseCmd);
}

Common::U32String Zoombini2SavefileNameDialog::getSavefileName() const {
	return _edit->getEditString();
}

void Zoombini2SavefileNameDialog::reflowLayout() {
	// Measure spacing and control heights using the current GUI scale and theme.
	const Common::Rect safeArea = g_system->getSafeOverlayArea();
	const float scale = g_gui.getScaleFactor();
	const int outerMargin = MAX(1, static_cast<int>(4 * scale));
	const int margin = MAX(4, static_cast<int>(8 * scale));
	const int gap = MAX(2, static_cast<int>(4 * scale));

	const Graphics::Font &font = g_gui.getFont(GUI::ThemeEngine::kFontStyleNormal);
	const int lineHeight = font.getFontHeight() + MAX(1, static_cast<int>(2 * scale));
	const int titleHeight = g_gui.getFontHeight();
	const int buttonHeight = MAX(g_gui.xmlEval()->getVar("Globals.Button.Height", 0), titleHeight + static_cast<int>(6 * scale));
	const int inputHeight = MAX(buttonHeight, font.getFontHeight() + static_cast<int>(8 * scale));
	const int scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", static_cast<int>(9 * scale));

	// Wrap the hint at the available width, reserving room for the scrollbar.
	_w = MIN<int>(safeArea.width() - 2 * outerMargin, static_cast<int>(360 * scale));
	const int contentWidth = _w - 2 * margin;
	const int descriptionWidth = contentWidth - scrollbarWidth;
	Common::Array<Common::U32String> lines;
	font.wordWrapText(_nameHint, descriptionWidth, lines);

	// Size the dialog for the wrapped hint, capped by the safe overlay area.
	const int scrollPadding = g_gui.xmlEval()->getVar("Globals.Font.Height", font.getFontHeight());
	const int fixedHeight = 2 * margin + titleHeight + 3 * gap + inputHeight + buttonHeight;
	_h = MIN<int>(safeArea.height() - 2 * outerMargin, fixedHeight + lines.size() * lineHeight + 2 * scrollPadding);
	_x = safeArea.left + (safeArea.width() - _w) / 2;
	_y = safeArea.top + (safeArea.height() - _h) / 2;

	_title->setPos(margin, margin);
	_title->setSize(contentWidth, titleHeight);
	_title->setTooltip(_title->getLabel());

	// Anchor the input and buttons at the bottom; the hint uses the remaining height.
	const int footerY = _h - margin - buttonHeight;
	const int inputY = footerY - gap - inputHeight;
	const int descriptionY = margin + titleHeight + gap;
	_descriptionScroller->setPos(margin, descriptionY);
	_descriptionScroller->setSize(descriptionWidth, MAX(1, inputY - gap - descriptionY));

	// Reuse line widgets across reflows and hide any lines no longer needed.
	while (_descLines.size() < lines.size()) {
		GUI::StaticTextWidget *line = new GUI::StaticTextWidget(_descriptionScroller, 0, 0, 1, 1, Common::U32String(), Graphics::kTextAlignStart,
																Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_descLines.push_back(line);
	}

	for (uint i = 0; i < _descLines.size(); i++) {
		const bool visible = i < lines.size();
		_descLines[i]->setVisible(visible);
		if (visible) {
			_descLines[i]->setPos(0, i * lineHeight);
			_descLines[i]->setSize(descriptionWidth, lineHeight);
			_descLines[i]->setLabel(lines[i]);
		}
	}

	_edit->setPos(margin, inputY);
	_edit->setSize(contentWidth, inputHeight);

	// Center equal-width buttons sized for their labels, within the dialog width.
	int buttonWidth = MAX(g_gui.getStringWidth(_okButton->getLabel()), g_gui.getStringWidth(_cancelButton->getLabel()));
	buttonWidth += 2 * margin;
	buttonWidth = MAX(buttonWidth, g_gui.xmlEval()->getVar("Globals.Button.Width", 0));
	buttonWidth = MIN(buttonWidth, (contentWidth - gap) / 2);
	const int buttonX = margin + (contentWidth - 2 * buttonWidth - gap) / 2;
	_okButton->setPos(buttonX, footerY);
	_okButton->setSize(buttonWidth, buttonHeight);
	_okButton->setTooltip(_okButton->getLabel());
	_cancelButton->setPos(buttonX + buttonWidth + gap, footerY);
	_cancelButton->setSize(buttonWidth, buttonHeight);
	_cancelButton->setTooltip(_cancelButton->getLabel());

	GUI::Dialog::reflowLayout();
}

void Zoombini2SavefileNameDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case GUI::kOKCmd:
		setResult(GUI::kOKCmd);
		close();
		break;
	case GUI::kCloseCmd:
		setResult(GUI::kCloseCmd);
		close();
		break;
	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
		break;
	}
}

Zoombini2SaveManagementDialog::Zoombini2SaveManagementDialog(const Common::String &domain)
	: GUI::Dialog(0, 0, 1, 1), _domain(domain), _savefileSelectionGroup(this, kSavefileSelectionChangedCommand) {
	_language = Common::parseLanguage(ConfMan.get("language", _domain));
	if (g_engine && ConfMan.getActiveDomainName() == _domain && Common::String(g_engine->getMetaEngine()->getName()) == "zoombini2") {
		const Zoombini2Engine *vm = static_cast<const Zoombini2Engine *>(g_engine);
		_language = vm->getLanguage();
	}

	_description = new GUI::StaticTextWidget(this, kDialogMargin, 8, kTableWidth, 24, true, _("Manage saved games"), Graphics::kTextAlignStart);

	static constexpr int kActionButtonGap = 6;
	static constexpr int kActionButtonCount = 5;
	static constexpr int kActionButtonWidth = (kTableWidth - (kActionButtonCount - 1) * kActionButtonGap) / kActionButtonCount;
	int actionX = kDialogMargin;
	_editButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, _("Rename"),
										Common::U32String(), kEditSavefileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_duplicateButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, _("Clone"),
											 Common::U32String(), kDuplicateSavefileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_importButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, _("Import"),
										  Common::U32String(), kImportSavefileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_exportButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kActionButtonWidth, kButtonHeight, true, _("Export"),
										  Common::U32String(), kExportSavefileCommand);
	actionX += kActionButtonWidth + kActionButtonGap;
	_deleteButton = new GUI::ButtonWidget(this, actionX, kActionButtonsTop, kTableWidth - actionX + kDialogMargin, kButtonHeight, true,
										  _("Delete"), Common::U32String(),
										  kDeleteSavefileCommand);

	_savefileHeader = new GUI::ContainerWidget(this, kDialogMargin, kHeaderTop, kTableWidth, kTableRowHeight, true);
	_savefileHeader->setBackgroundType(GUI::ThemeEngine::kWidgetBackgroundNo);
	_savefileColumnLabels[0] = new GUI::StaticTextWidget(_savefileHeader, kNameX, 0, kNameWidth, kTableRowHeight, true, _("Name"), Graphics::kTextAlignCenter);
	// I18N: Name of the Zoombini settlement.
	_savefileColumnLabels[1] = new GUI::StaticTextWidget(_savefileHeader, kZombinivilleX, 0, kZombinivilleWidth, kTableRowHeight, true, _("Zombiniville"),
														 Graphics::kTextAlignCenter);
	_savefileColumnLabels[2] = new GUI::StaticTextWidget(_savefileHeader, kRescue1X, 0, kRescueWidth, kTableRowHeight, true, _("Rescue I"), Graphics::kTextAlignCenter);
	_savefileColumnLabels[3] = new GUI::StaticTextWidget(_savefileHeader, kRescue2X, 0, kRescueWidth, kTableRowHeight, true, _("Rescue II"), Graphics::kTextAlignCenter);
	// I18N: Name of the Boolie shelter.
	_savefileColumnLabels[4] = new GUI::StaticTextWidget(_savefileHeader, kBooliewoodX, 0, kBooliewoodWidth, kTableRowHeight, true, _("Booliewood"),
														 Graphics::kTextAlignCenter);
	_savefileColumnLabels[5] = new GUI::StaticTextWidget(_savefileHeader, kActivePartyX, 0, kActivePartyWidth, kTableRowHeight, true, _("Active"),
														 Graphics::kTextAlignCenter);

	_savefileList = new SavefileListWidget(this, scaleDialogValue(kDialogMargin), scaleDialogValue(kListTop), scaleDialogValue(kTableWidth),
										   scaleDialogValue(kListHeight));
	_savefileTable = new GUI::ContainerWidget(_savefileList, 0, 0, kTableWidth, 0, true);
	const int fontHeight = MAX<int>(1, static_cast<int>(g_gui.getFontHeight() / g_gui.getScaleFactor()));
	const int selectionHeight = MIN<int>(kTableRowHeight, fontHeight);
	const int selectionOffset = (kTableRowHeight - selectionHeight) / 2;
	for (int i = 0; i < kMaximumSavefileRows; i++) {
		const int y = i * kTableRowHeight;
		_savefileSelectionButtons[i] = new GUI::RadiobuttonWidget(_savefileTable, kSelectionX, y + selectionOffset, kSelectionWidth, selectionHeight, true,
																  &_savefileSelectionGroup, i, Common::U32String(), Common::U32String());
		_savefileNameLabels[i] = new GUI::StaticTextWidget(_savefileTable, kNameX, y, kNameWidth, kTableRowHeight, true, Common::U32String(),
														   Graphics::kTextAlignLeft, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_zombinivilleLabels[i] = new GUI::StaticTextWidget(_savefileTable, kZombinivilleX, y, kZombinivilleWidth, kTableRowHeight, true,
														   Common::U32String(), Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_rescue1Labels[i] = new GUI::StaticTextWidget(_savefileTable, kRescue1X, y, kRescueWidth, kTableRowHeight, true, Common::U32String(),
													  Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_rescue2Labels[i] = new GUI::StaticTextWidget(_savefileTable, kRescue2X, y, kRescueWidth, kTableRowHeight, true, Common::U32String(),
													  Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_booliewoodLabels[i] = new GUI::StaticTextWidget(_savefileTable, kBooliewoodX, y, kBooliewoodWidth, kTableRowHeight, true, Common::U32String(),
														 Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
		_activePartyLabels[i] = new GUI::StaticTextWidget(_savefileTable, kActivePartyX, y, kActivePartyWidth, kTableRowHeight, true, Common::U32String(),
														  Graphics::kTextAlignCenter, Common::U32String(), GUI::ThemeEngine::kFontStyleNormal);
	}

	_closeButton = new GUI::ButtonWidget(this, kDialogMargin + (kTableWidth - 120) / 2, kBottomButtonsTop, 120, kButtonHeight, true, _("Close"),
										 Common::U32String(), GUI::kCloseCmd, Common::ASCII_ESCAPE);
}

void Zoombini2SaveManagementDialog::open() {
	GUI::Dialog::open();
	refreshSavefiles();
	_savefileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void Zoombini2SaveManagementDialog::reflowLayout() {
	// Reduce spacing on short overlays to leave more room for the save list.
	const Common::Rect safeArea = g_system->getSafeOverlayArea();
	const int outerMargin = MAX<int>(1, scaleDialogValue(4));
	int margin = MAX<int>(2, scaleDialogValue(6));
	int gap = MAX<int>(1, scaleDialogValue(4));
	int padding = MAX<int>(2, scaleDialogValue(6));
	if (safeArea.height() < scaleDialogValue(280)) {
		margin = MAX<int>(2, scaleDialogValue(4));
		gap = MAX<int>(1, scaleDialogValue(2));
		padding = MAX<int>(2, scaleDialogValue(4));
	}

	const int scrollbarWidth = g_gui.xmlEval()->getVar("Globals.Scrollbar.Width", scaleDialogValue(9));
	const int fontHeight = MAX(g_gui.getFontHeight(), g_gui.getFontHeight(GUI::ThemeEngine::kFontStyleNormal));
	const int lineHeight = fontHeight + gap;
	const int buttonHeight = MAX(g_gui.xmlEval()->getVar("Globals.Button.Height", 0), fontHeight + padding);

	// Fit and center the dialog, reserving scrollbar space beside the table.
	_w = MIN<int>(safeArea.width() - 2 * outerMargin, scaleDialogValue(kTableWidth + 2 * kDialogMargin) + scrollbarWidth);
	_h = MIN<int>(safeArea.height() - 2 * outerMargin, scaleDialogValue(kDialogHeight));
	_x = safeArea.left + (safeArea.width() - _w) / 2;
	_y = safeArea.top + (safeArea.height() - _h) / 2;
	const int contentWidth = _w - 2 * margin;
	const int tableWidth = contentWidth - scrollbarWidth;

	_description->setPos(margin, margin);
	_description->setSize(contentWidth, lineHeight);
	_description->setTooltip(_description->getLabel());

	// Size actions from their translated labels and wrap them onto another row as needed.
	GUI::ButtonWidget *const actions[] = {
		_editButton,
		_duplicateButton,
		_importButton,
		_exportButton,
		_deleteButton,
	};
	int actionX = margin;
	int actionY = margin + lineHeight + gap;
	const int maximumButtonWidth = (contentWidth - 3 * gap) / 4;
	for (uint i = 0; i < ARRAYSIZE(actions); i++) {
		GUI::ButtonWidget *button = actions[i];
		int width = MAX(g_gui.getStringWidth(button->getLabel()) + 2 * padding, scaleDialogValue(22));
		width = MIN(width, maximumButtonWidth);
		if (margin < actionX && margin + contentWidth < actionX + width) {
			actionX = margin;
			actionY += buttonHeight + gap;
		}

		button->setPos(actionX, actionY);
		button->setSize(width, buttonHeight);
		actionX += width + gap;
	}

	// Place the table header below the final action row.
	updateSavefileTableLayout(tableWidth, lineHeight, padding);
	const int headerY = actionY + buttonHeight + gap;
	_savefileHeader->setPos(margin, headerY);

	// Anchor Close at the bottom and give the list all space above it.
	const int footerY = _h - margin - buttonHeight;
	int closeWidth = g_gui.getStringWidth(_closeButton->getLabel()) + 2 * padding;
	closeWidth = MAX(closeWidth, g_gui.xmlEval()->getVar("Globals.Button.Width", 0));
	closeWidth = MIN(closeWidth, contentWidth);
	_closeButton->setPos(margin + (contentWidth - closeWidth) / 2, footerY);
	_closeButton->setSize(closeWidth, buttonHeight);
	_closeButton->setTooltip(_closeButton->getLabel());

	const int listY = headerY + _savefileHeader->getHeight();
	_savefileList->setPos(margin, listY);
	_savefileList->setSize(tableWidth, MAX(1, footerY - gap - listY));

	GUI::Dialog::reflowLayout();
}

void Zoombini2SaveManagementDialog::refreshSavefiles(const Common::String &selectedSavefile) {
	Common::String savefileNameToSelect = selectedSavefile;
	if (savefileNameToSelect.empty() && 0 <= _selectedSavefileIndex && _selectedSavefileIndex < static_cast<int>(_savefileNames.size()))
		savefileNameToSelect = _savefileNames[_selectedSavefileIndex];

	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	const Common::Array<Zoombini2SavefileSummary> summaries = savegameManager.listSavefileSummaries();
	_savefileNames.clear();
	_savefileRowCount = MIN<int>(summaries.size(), kMaximumSavefileRows);
	_selectedSavefileIndex = -1;
	for (int i = 0; i < _savefileRowCount; i++) {
		_savefileNames.push_back(summaries[i]._savefileName);
		if (!savefileNameToSelect.empty() && summaries[i]._savefileName.equalsIgnoreCase(savefileNameToSelect))
			_selectedSavefileIndex = i;
	}
	if (_selectedSavefileIndex < 0 && 0 < _savefileRowCount)
		_selectedSavefileIndex = 0;

	for (int i = 0; i < kMaximumSavefileRows; i++) {
		const bool visible = i < _savefileRowCount;
		_savefileSelectionButtons[i]->setVisible(visible);
		_savefileNameLabels[i]->setVisible(visible);
		_zombinivilleLabels[i]->setVisible(visible);
		_rescue1Labels[i]->setVisible(visible);
		_rescue2Labels[i]->setVisible(visible);
		_booliewoodLabels[i]->setVisible(visible);
		_activePartyLabels[i]->setVisible(visible);
		_savefileStateValid[i] = visible && summaries[i]._stateValid;
		if (!visible)
			continue;

		const Zoombini2SavefileSummary &summary = summaries[i];
		_savefileNameLabels[i]->setLabel(savegameManager.decodeSavefileName(summary._savefileName));
		if (summary._stateValid) {
			_zombinivilleLabels[i]->setLabel(Common::U32String::format("%d", summary._population._zombinivilleCount));
			_rescue1Labels[i]->setLabel(Common::U32String::format("%d", summary._population._rescue1Count));
			_rescue2Labels[i]->setLabel(Common::U32String::format("%d", summary._population._rescue2Count));
			_booliewoodLabels[i]->setLabel(Common::U32String::format("%d", summary._population._booliewoodCount));
			_activePartyLabels[i]->setLabel(Common::U32String::format("%d", summary._population._activePartyCount));
		} else {
			// I18N: Placeholder for a population count in an invalid saved game.
			_zombinivilleLabels[i]->setLabel(_("?"));
			_rescue1Labels[i]->setLabel(_("?"));
			_rescue2Labels[i]->setLabel(_("?"));
			_booliewoodLabels[i]->setLabel(_("?"));
			_activePartyLabels[i]->setLabel(_("?"));
		}

		GUI::ThemeEngine::FontColor fontColor = GUI::ThemeEngine::kFontColorOverride;
		if (summary._stateValid)
			fontColor = GUI::ThemeEngine::kFontColorNormal;
		GUI::StaticTextWidget *rowLabels[] = {
			_savefileNameLabels[i],
			_zombinivilleLabels[i],
			_rescue1Labels[i],
			_rescue2Labels[i],
			_booliewoodLabels[i],
			_activePartyLabels[i],
		};
		for (uint labelIndex = 0; labelIndex < ARRAYSIZE(rowLabels); labelIndex++) {
			rowLabels[labelIndex]->setFontColor(fontColor);
			rowLabels[labelIndex]->markAsDirty();
		}
	}

	_savefileSelectionGroup.setValue(_selectedSavefileIndex);
	reflowLayout();
	updateButtons();
}

void Zoombini2SaveManagementDialog::updateSavefileTableLayout(int tableWidth, int lineHeight, int padding) {
	// Measure each column from its heading and expected count width.
	const Graphics::Font &font = g_gui.getFont(GUI::ThemeEngine::kFontStyleNormal);
	const int selectionWidth = font.getFontHeight() + padding;
	int widths[6] = {
		scaleDialogValue(64),
		font.getStringWidth(Common::U32String(U"999")),
		font.getStringWidth(Common::U32String(U"999")),
		font.getStringWidth(Common::U32String(U"999")),
		font.getStringWidth(Common::U32String(U"999")),
		font.getStringWidth(Common::U32String(U"99")),
	};

	int requiredWidth = selectionWidth;
	for (uint i = 0; i < ARRAYSIZE(widths); i++) {
		widths[i] = MAX(widths[i], g_gui.getStringWidth(_savefileColumnLabels[i]->getLabel())) + padding;
		requiredWidth += widths[i];
	}

	// Keep counts beside the name when they fit; otherwise move them below it.
	const bool stacked = tableWidth < requiredWidth;
	int fixedWidth = selectionWidth;
	if (!stacked) {
		for (uint i = 1; i < ARRAYSIZE(widths); i++)
			fixedWidth += widths[i];
	}
	widths[0] = MAX(1, tableWidth - fixedWidth);

	int columnX[6] = {};
	int columnY[6] = {};
	int x = selectionWidth;
	for (uint i = 0; i < ARRAYSIZE(widths); i++) {
		if (stacked && 1 <= i) {
			// Divide the second line evenly, including any pixel-rounding remainder.
			const int statsWidth = tableWidth - selectionWidth;
			const int statIndex = i - 1;
			const int statCount = ARRAYSIZE(widths) - 1;
			columnX[i] = selectionWidth + statsWidth * statIndex / statCount;
			columnY[i] = lineHeight;
			widths[i] = statsWidth * (statIndex + 1) / statCount - statsWidth * statIndex / statCount;
		} else {
			columnX[i] = x;
			x += widths[i];
		}

		_savefileColumnLabels[i]->setPos(columnX[i], columnY[i]);
		_savefileColumnLabels[i]->setSize(widths[i], lineHeight);
		_savefileColumnLabels[i]->setTooltip(_savefileColumnLabels[i]->getLabel());
	}

	// Apply the same column positions and row height to the header and save entries.
	int rowHeight = lineHeight;
	if (stacked)
		rowHeight += lineHeight;
	_savefileHeader->setSize(tableWidth, rowHeight);
	_savefileTable->setSize(tableWidth, _savefileRowCount * rowHeight);

	for (int row = 0; row < kMaximumSavefileRows; row++) {
		const int y = row * rowHeight;
		_savefileSelectionButtons[row]->setPos(0, y + (lineHeight - font.getFontHeight()) / 2);
		_savefileSelectionButtons[row]->setSize(selectionWidth, font.getFontHeight());
		_savefileList->setRowState(row, _savefileNameLabels[row], rowHeight, !_savefileStateValid[row]);

		GUI::StaticTextWidget *const labels[] = {
			_savefileNameLabels[row],
			_zombinivilleLabels[row],
			_rescue1Labels[row],
			_rescue2Labels[row],
			_booliewoodLabels[row],
			_activePartyLabels[row],
		};
		for (uint i = 0; i < ARRAYSIZE(labels); i++) {
			labels[i]->setPos(columnX[i], y + columnY[i]);
			labels[i]->setSize(widths[i], lineHeight);

			// Preserve full labels on hover, with corruption guidance for invalid saves.
			if (row < _savefileRowCount && !_savefileStateValid[row])
				labels[i]->setTooltip(_("Corrupted save"));
			else
				labels[i]->setTooltip(labels[i]->getLabel());
		}
	}
}

void Zoombini2SaveManagementDialog::updateButtons() {
	const bool hasSelection = 0 <= _selectedSavefileIndex && _selectedSavefileIndex < _savefileRowCount;
	const bool stateValid = hasSelection && _savefileStateValid[_selectedSavefileIndex];
	const bool activeSavefile = hasSelection && isActiveSavefile(_savefileNames[_selectedSavefileIndex]);
	_editButton->setEnabled(stateValid && !activeSavefile);
	_duplicateButton->setEnabled(stateValid && _savefileRowCount < kMaximumSavefileRows);
	_importButton->setEnabled(true);
	_exportButton->setEnabled(stateValid);
	_deleteButton->setEnabled(hasSelection && !activeSavefile);

	// Restore label tooltips before adding guidance for the active save.
	_editButton->setTooltip(_editButton->getLabel());
	_duplicateButton->setTooltip(_duplicateButton->getLabel());
	_importButton->setTooltip(_importButton->getLabel());
	_exportButton->setTooltip(_exportButton->getLabel());
	_deleteButton->setTooltip(_deleteButton->getLabel());

	if (activeSavefile) {
		_editButton->setTooltip(_("Return to the launcher before renaming the active saved game."));
		_deleteButton->setTooltip(_("Return to the launcher before deleting the active saved game."));
	}
}

bool Zoombini2SaveManagementDialog::isActiveSavefile(const Common::String &savefileName) const {
	if (!g_engine || ConfMan.getActiveDomainName() != _domain || Common::String(g_engine->getMetaEngine()->getName()) != "zoombini2")
		return false;
	const Zoombini2Engine *vm = static_cast<const Zoombini2Engine *>(g_engine);
	return vm->isActiveGameSave(savefileName);
}

void Zoombini2SaveManagementDialog::renameSelectedSavefile() {
	if (_selectedSavefileIndex < 0 || static_cast<int>(_savefileNames.size()) <= _selectedSavefileIndex || !_savefileStateValid[_selectedSavefileIndex])
		return;

	const Common::String oldSavefileName = _savefileNames[_selectedSavefileIndex];
	if (isActiveSavefile(oldSavefileName))
		return;
	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	Zoombini2SavefileNameDialog nameDialog(_("Rename saved game"), savegameManager.decodeSavefileName(oldSavefileName), _language);
	if (nameDialog.runModal() != GUI::kOKCmd)
		return;

	Common::String newSavefileName;
	if (!savegameManager.encodeSavefileName(nameDialog.getSavefileName(), newSavefileName) ||
		!savegameManager.renameSavefile(oldSavefileName, newSavefileName)) {
		GUI::MessageDialog errorDialog(_("Invalid file name for saving"));
		errorDialog.runModal();
		return;
	}
	refreshSavefiles(newSavefileName);
	_savefileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void Zoombini2SaveManagementDialog::duplicateSelectedSavefile() {
	if (_selectedSavefileIndex < 0 || static_cast<int>(_savefileNames.size()) <= _selectedSavefileIndex || !_savefileStateValid[_selectedSavefileIndex] ||
		kMaximumSavefileRows <= _savefileRowCount)
		return;

	const Common::String srcSavefileName = _savefileNames[_selectedSavefileIndex];
	Zoombini2SavefileNameDialog nameDialog(_("Clone saved game"), Common::U32String(), _language);
	if (nameDialog.runModal() != GUI::kOKCmd)
		return;

	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	Common::String newSavefileName;
	if (!savegameManager.encodeSavefileName(nameDialog.getSavefileName(), newSavefileName) ||
		!savegameManager.duplicateSavefile(srcSavefileName, newSavefileName)) {
		GUI::MessageDialog errorDialog(_("Enter a unique name using the allowed characters for this language"));
		errorDialog.runModal();
		return;
	}
	refreshSavefiles(newSavefileName);
	_savefileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

void Zoombini2SaveManagementDialog::importSavefile() {
	GUI::BrowserDialog browser(_("Select one Zoombini2 .mk saved game"), false);
	if (browser.runModal() <= 0)
		return;

	const Common::FSNode src = browser.getResult();
	const Common::String srcName = src.getName();
	const size_t extensionPosition = srcName.findLastOf('.');
	if (!src.exists() || src.isDirectory() || extensionPosition == Common::String::npos ||
		!srcName.substr(extensionPosition).equalsIgnoreCase(".mk")) {
		GUI::MessageDialog errorDialog(_("Select one Zoombini2 .mk saved game"));
		errorDialog.runModal();
		return;
	}

	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	Common::String savefileName;
	Common::U32String displayName = srcName.substr(0, extensionPosition).decode(Common::kUtf8);
	bool validName = savegameManager.encodeSavefileName(displayName, savefileName) &&
					 Zoombini2SavegameManager::isValidSavefileName(savefileName, _language);
	while (!validName) {
		Zoombini2SavefileNameDialog nameDialog(_("Choose compatible save name"), displayName, _language);
		if (nameDialog.runModal() != GUI::kOKCmd)
			return;
		displayName = nameDialog.getSavefileName();
		validName = savegameManager.encodeSavefileName(displayName, savefileName) &&
					Zoombini2SavegameManager::isValidSavefileName(savefileName, _language);
		if (!validName) {
			GUI::MessageDialog errorDialog(_("The name contains characters unavailable in this language"));
			errorDialog.runModal();
		}
	}

	if (isActiveSavefile(savefileName)) {
		GUI::MessageDialog errorDialog(_("Return to the launcher before replacing the active saved game."));
		errorDialog.runModal();
		return;
	}
	if (savegameManager.savefileExists(savefileName)) {
		GUI::MessageDialog confirmation(_("A saved game with this name already exists. Replace it?"),
										_("Replace"), _("Cancel"));
		if (confirmation.runModal() != GUI::kMessageOK)
			return;
	}

	Common::SeekableReadStream *stream = src.createReadStream();
	const bool imported = savegameManager.importSavefile(savefileName, stream, true);
	delete stream;
	if (!imported) {
		GUI::MessageDialog errorDialog(_("The selected file is not a valid Zoombini2 saved game"));
		errorDialog.runModal();
		return;
	}

	refreshSavefiles(savefileName);
	_savefileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
	GUI::MessageDialog successDialog(_("The saved game was imported"));
	successDialog.runModal();
}

void Zoombini2SaveManagementDialog::exportSelectedSavefile() {
	if (_selectedSavefileIndex < 0 || static_cast<int>(_savefileNames.size()) <= _selectedSavefileIndex || !_savefileStateValid[_selectedSavefileIndex])
		return;

	GUI::BrowserDialog browser(_("Select the directory for the exported Zoombini2 .mk save"), true);
	if (browser.runModal() <= 0)
		return;

	const Common::FSNode directory = browser.getResult();
	if (!directory.isDirectory()) {
		GUI::MessageDialog errorDialog(_("The selected destination is not a directory"));
		errorDialog.runModal();
		return;
	}

	const Common::String savefileName = _savefileNames[_selectedSavefileIndex];
	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	const Common::String exportName = savegameManager.decodeSavefileName(savefileName).encode(Common::kUtf8) + ".mk";
	const Common::FSNode dest = findChild(directory, exportName);
	if (dest.exists()) {
		GUI::MessageDialog confirmation(_("The .mk file already exists. Replace it?"),
										_("Replace"), _("Cancel"));
		if (confirmation.runModal() != GUI::kMessageOK)
			return;
	}

	Common::SeekableWriteStream *stream = dest.createWriteStream(false);
	bool exported = stream && savegameManager.exportSavefile(savefileName, stream);
	if (stream) {
		stream->finalize();
		exported = exported && !stream->err();
	}
	delete stream;
	if (!exported) {
		GUI::MessageDialog errorDialog(_("Unable to export the selected saved game"));
		errorDialog.runModal();
		return;
	}

	GUI::MessageDialog successDialog(_("The saved game was exported as a Zoombini2 .mk file"));
	successDialog.runModal();
}

void Zoombini2SaveManagementDialog::deleteSelectedSavefile() {
	if (_selectedSavefileIndex < 0 || static_cast<int>(_savefileNames.size()) <= _selectedSavefileIndex)
		return;
	const Common::String savefileName = _savefileNames[_selectedSavefileIndex];
	if (isActiveSavefile(savefileName))
		return;

	GUI::MessageDialog confirmation(_("Do you really want to delete this saved game?"), _("Delete"),
									_("Cancel"));
	if (confirmation.runModal() != GUI::kMessageOK)
		return;

	Zoombini2SavegameManager savegameManager(g_system->getSavefileManager(), _domain, _language);
	if (!savegameManager.deleteSavefile(savefileName)) {
		GUI::MessageDialog errorDialog(_("Error deleting saved game"));
		errorDialog.runModal();
		return;
	}
	refreshSavefiles();
	_savefileList->reflowLayout();
	g_gui.scheduleTopDialogRedraw();
}

Common::FSNode Zoombini2SaveManagementDialog::findChild(const Common::FSNode &directory, const Common::String &name) {
	Common::FSNode direct = directory.getChild(name);
	if (direct.exists())
		return direct;

	Common::FSList children;
	if (directory.getChildren(children, Common::FSNode::kListFilesOnly)) {
		for (Common::FSList::const_iterator child = children.begin(); child != children.end(); child++) {
			if (child->getName().equalsIgnoreCase(name))
				return *child;
		}
	}
	return direct;
}

int Zoombini2SaveManagementDialog::scaleDialogValue(int value) {
	return 0 < value ? static_cast<int>(value * g_gui.getScaleFactor()) : value;
}

void Zoombini2SaveManagementDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kSavefileSelectionChangedCommand:
		if (data < static_cast<uint32>(_savefileRowCount))
			_selectedSavefileIndex = static_cast<int>(data);
		else
			_selectedSavefileIndex = -1;
		updateButtons();
		break;
	case kEditSavefileCommand:
		renameSelectedSavefile();
		break;
	case kDuplicateSavefileCommand:
		duplicateSelectedSavefile();
		break;
	case kImportSavefileCommand:
		importSavefile();
		break;
	case kExportSavefileCommand:
		exportSelectedSavefile();
		break;
	case kDeleteSavefileCommand:
		deleteSelectedSavefile();
		break;
	default:
		GUI::Dialog::handleCommand(sender, cmd, data);
		break;
	}
}

Zoombini2OptionsWidget::Zoombini2OptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &domain)
	: GUI::OptionsContainerWidget(boss, name, "Zoombini2EngineOptionsDialog", domain) {
	new SeparatorWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.SaveFilesSeparator");
	GUI::StaticTextWidget *header = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.SaveFilesHeader",
															  _("Save management"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	header->setAlign(Graphics::TextAlign::kTextAlignStart);
	GUI::ButtonWidget *manageSavefilesButton = new GUI::ButtonWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.ManageSavefiles",
																	 _("Saved games"), Common::U32String(), kManageSavefilesCommand);
	manageSavefilesButton->setTarget(this);
	_savefileReadOnlyToggleCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.SavefileReadOnlyToggle",
															  _("Enable savefile readonly toggle (Ctrl-K)"),
															  _("Ctrl+K or right-click toggles automatic save writes for one savefile during this game session. "
																"A file without write permission remains read-only."));

	new SeparatorWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayEnhancementsSeparator");
	header = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayEnhancements",
									   _("Gameplay Enhancements"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	header->setAlign(Graphics::TextAlign::kTextAlignStart);
	const Common::U32String audioOutputTooltip = _("Selects mono downmixing or stereo playback for game audio resources. "
												   "Changes apply to newly started sounds.");
	_audioOutputLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.AudioOutputLabel", _("Game audio:"), audioOutputTooltip);
	_audioOutputLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_audioOutputPopUp = new GUI::PopUpWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.AudioOutput", audioOutputTooltip);
	_audioOutputPopUp->appendEntry(_("Mono Downmix (Original)"), static_cast<uint32>(AudioOutputMode::kMonoDownmix));
	_audioOutputPopUp->appendEntry(_("Stereo Playback"), static_cast<uint32>(AudioOutputMode::kStereoPlayback));
	const Common::U32String bezierCurveDataTypeTooltip = _("Select datatype for Bezier curve computation.");
	_bezierCurveDataTypeLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.BezierCurveDataTypeLabel", _("Bezier curve datatype:"), bezierCurveDataTypeTooltip);
	_bezierCurveDataTypeLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_bezierCurveDataTypePopUp = new GUI::PopUpWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.BezierCurveDataType", bezierCurveDataTypeTooltip);
	_bezierCurveDataTypePopUp->appendEntry(_("Q10 fixed-point (Original)"), static_cast<uint32>(BezierCurveDataType::kQ10FixedPoint));
	_bezierCurveDataTypePopUp->appendEntry(_("F32 floating-point"), static_cast<uint32>(BezierCurveDataType::kF32FloatingPoint));
	_colorAssistLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.ColorAssistLabel",
												  _("Color assistance:"));
	_colorAssistLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_colorAssistPopUp = new GUI::PopUpWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.ColorAssist",
											 _("Adjusts Zoombini noses and puzzle colors."));
	_colorAssistPopUp->appendEntry(_("Original colors"), static_cast<uint32>(ColorAssistMode::kOriginal));
	_colorAssistPopUp->appendEntry(_("Enhanced color distinction"), static_cast<uint32>(ColorAssistMode::kEnhancedDistinction));
	_colorAssistPopUp->appendEntry(_("Red-green color blind assist"), static_cast<uint32>(ColorAssistMode::kRedGreenBlindAssist));
	_enhancedKbdShortcutsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.EnhancedKbdShortcuts",
															_("Enable enhanced keyboard shortcuts"),
															_("Enables some ScummVM-only keyboard shortcuts for quality of life improvements."));
	_wallOfFleensFixDepartStreakCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.WallOfFleensFixDepartStreak",
																   _("Magic Mirrors: Remove stray streaks from fleeing Fleens"),
																   _("Hides stray brown streaks in some Fleen departure frames in Magic Mirrors."));
	_mysticMarshFillCellGapsCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.MysticMarshFillCellGaps",
															   _("Bubble Bumpers: Fill gaps between cell sprites and edges"),
															   _("Fills narrow gaps between cell sprites and grid lines."));
	_mysticMarshExpandAreaMaskCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.MysticMarshExpandAreaMask",
																 _("Bubble Bumpers: Expand the staging area"),
																 _("Expands the area where Zoombinis can be reloated along the lower-left side."));
	_chezNorfClueSubtitlesCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.ChezNorfClueSubtitles",
															 _("Chez Norf: Show English subtitles on Norf clues"),
															 _("Shows the original English clues."));
	if (Common::checkGameGUIOption(GAMEOPTION_HELP_PAGE_COLOR_KEYING, ConfMan.get("guioptions", domain))) {
		_helpDialogTransparentBodyCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.HelpDialogTransparentBody",
																	 _("Remove solid color behind body in help dialogs"),
																	 _("Uses the help sheet's corner color as a transparency key for affected releases."));
	}

	new SeparatorWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayAdjustmentSeparator");
	header = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GameplayAdjustment",
									   _("Gameplay Adjustment"), Common::U32String(), GUI::ThemeEngine::kFontStyleBold);
	header->setAlign(Graphics::TextAlign::kTextAlignStart);
	_debugHotkeysCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.DebugHotkeys", _("Enable developer hotkeys"),
													_("Enables F2/F3 party exchange, P puzzle completion, and the Chez Norf C overlay."));
	_greedyWaterslideCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.GreedyWaterslide",
														_("Use alternate Pipes of Paloo pairing"),
														_("Selects the alternate pairing branch for level one."));
	_allowRestoreL4PracticePuzzlesCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.RestoreL4PracticePuzzles",
																	 _("Restore Level 4 puzzles in practice mode"),
																	 _("Make Level 4 accessible in practice-mode map legend. "
																	   "Even though the Level 4 was cut from the final release, most of its logic was retained in the original binary. "
																	   "Level 4 logic for Chez Norf, Snowboard Gulch was recovered from the demo, which actually serves as a beta binary. "
																	   "Mystic Marsh, Snowboard Gulch lacks the background resourcce for level 4; This option reconstructs them with best-effort. "));
	const Common::U32String prngAlgorithmTooltip = _("Selects the pseudo random number generator algorithm.");
	_prngAlgorithmLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.PrngAlgorithmLabel", _("PRNG Algorithm:"), prngAlgorithmTooltip);
	_prngAlgorithmLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_prngAlgorithmPopUp = new GUI::PopUpWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.PrngAlgorithm", prngAlgorithmTooltip);
	_prngAlgorithmPopUp->appendEntry(_("Original PRNG"), static_cast<uint32>(PrngAlgorithmKind::kOriginalPrng));
	_prngAlgorithmPopUp->appendEntry(_("ScummVM Standard PRNG"), static_cast<uint32>(PrngAlgorithmKind::kStandardPrng));
	_frameRateLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.FrameRateLabel", _("Frame rate (FPS):"));
	_frameRateLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_frameRateNumberBox = new FrameRateNumberBox(widgetsBoss(), "Zoombini2EngineOptionsDialog.FrameRate",
												 Zoombini2MetaEngine::kDefaultFrameRate,
												 _("Enter a whole number from 30 to 240 FPS."));
	_pacingLabel = new GUI::StaticTextWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.PacingLabel", _("Logic pacing:"));
	_pacingLabel->setAlign(Graphics::TextAlign::kTextAlignEnd);
	_pacingPopUp = new GUI::PopUpWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.Pacing",
										_("Selects the reference frame rate used to normalize logic pacing. "
										  "Affects panorama scrolling speed in Booliewood and how often fidget animations are triggered. "
										  "Does not affect the rendering frame rate."));
	_pacingPopUp->appendEntry(_("60Hz (LCD preset)"), static_cast<uint32>(LogicPacingMode::k60Hz));
	_pacingPopUp->appendEntry(_("75Hz (CRT preset)"), static_cast<uint32>(LogicPacingMode::k75Hz));
	_unlockFrameRateCheckbox = new GUI::CheckboxWidget(widgetsBoss(), "Zoombini2EngineOptionsDialog.UnlockFrameRate",
													   _("Unlock frame rate"),
													   _("Removes the engine frame-rate limit. Display VSync may still limit presentation."),
													   kUnlockFrameRateCommand);
	_unlockFrameRateCheckbox->setTarget(this);
}

void Zoombini2OptionsWidget::defineLayout(GUI::ThemeEval &layouts, const Common::String &layoutName, const Common::String &overlayedLayout) const {
	const int lineHeight = layouts.getVar("Globals.Line.Height");
	static constexpr int kOptionLabelPadding = 4;
	const GUI::StaticTextWidget *const optionLabels[] = {
		_audioOutputLabel,
		_bezierCurveDataTypeLabel,
		_colorAssistLabel,
		_prngAlgorithmLabel,
		_pacingLabel,
		_frameRateLabel,
	};
	int optionLabelWidth = 0;
	for (uint labelIdx = 0; labelIdx < ARRAYSIZE(optionLabels); labelIdx++)
		optionLabelWidth = MAX(optionLabelWidth, g_gui.getStringWidth(optionLabels[labelIdx]->getLabel()));
	optionLabelWidth += kOptionLabelPadding;
	layouts.addDialog(layoutName, overlayedLayout)
		.addLayout(GUI::ThemeLayout::kLayoutVertical)
		.addPadding(0, 0, 0, 0)
		.addSpace(10)
		.addWidget("SaveFilesSeparator", "", -1, 2)
		.addWidget("SaveFilesHeader", "", -1, lineHeight)
		.addWidget("ManageSavefiles", "Button")
		.addWidget("SavefileReadOnlyToggle", "Checkbox")
		.addSpace(10)
		.addWidget("GameplayEnhancementsSeparator", "", -1, 2)
		.addWidget("GameplayEnhancements", "", -1, lineHeight)
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("AudioOutputLabel", "", optionLabelWidth, lineHeight)
		.addWidget("AudioOutput", "PopUp")
		.closeLayout()
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("BezierCurveDataTypeLabel", "", optionLabelWidth, lineHeight)
		.addWidget("BezierCurveDataType", "PopUp")
		.closeLayout()
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("ColorAssistLabel", "", optionLabelWidth, lineHeight)
		.addWidget("ColorAssist", "", 210)
		.closeLayout()
		.addWidget("EnhancedKbdShortcuts", "Checkbox")
		.addWidget("WallOfFleensFixDepartStreak", "Checkbox")
		.addWidget("MysticMarshFillCellGaps", "Checkbox")
		.addWidget("MysticMarshExpandAreaMask", "Checkbox")
		.addWidget("ChezNorfClueSubtitles", "Checkbox")
		.addWidget("HelpDialogTransparentBody", "Checkbox")
		.addSpace(10)
		.addWidget("GameplayAdjustmentSeparator", "", -1, 2)
		.addWidget("GameplayAdjustment", "", -1)
		.addWidget("DebugHotkeys", "Checkbox")
		.addWidget("GreedyWaterslide", "Checkbox")
		.addWidget("RestoreL4PracticePuzzles", "Checkbox")
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("PrngAlgorithmLabel", "", optionLabelWidth, lineHeight)
		.addWidget("PrngAlgorithm", "PopUp")
		.closeLayout()
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("PacingLabel", "", optionLabelWidth, lineHeight)
		.addWidget("Pacing", "")
		.closeLayout()
		.addLayout(GUI::ThemeLayout::kLayoutHorizontal, 12)
		.addPadding(0, 0, 0, 0)
		.addWidget("FrameRateLabel", "", optionLabelWidth, lineHeight)
		.addWidget("FrameRate", "", 64)
		.closeLayout()
		.addWidget("UnlockFrameRate", "Checkbox")
		.closeLayout()
		.closeDialog();
}

void Zoombini2OptionsWidget::load() {
	_savefileReadOnlyToggleCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigEnableSavefileReadOnlyToggle, _domain));
	const int audioOutputValue = ConfMan.getInt(Zoombini2MetaEngine::kConfigAudioOutputMode, _domain);
	const AudioOutputMode audioOutputMode = static_cast<AudioOutputMode>(audioOutputValue);
	switch (audioOutputMode) {
	case AudioOutputMode::kMonoDownmix:
	case AudioOutputMode::kStereoPlayback:
		_audioOutputPopUp->setSelectedTag(static_cast<uint32>(audioOutputMode));
		break;
	default:
		warning("Zoombini2OptionsWidget::load(): Invalid %s(%d)", Zoombini2MetaEngine::kConfigAudioOutputMode, audioOutputValue);
		_audioOutputPopUp->setSelectedTag(static_cast<uint32>(AudioOutputMode::kMonoDownmix));
		break;
	}
	const int bezierCurveDataTypeValue = ConfMan.getInt(Zoombini2MetaEngine::kConfigBezierCurveDataType, _domain);
	const BezierCurveDataType bezierCurveDataType = static_cast<BezierCurveDataType>(bezierCurveDataTypeValue);
	switch (bezierCurveDataType) {
	case BezierCurveDataType::kQ10FixedPoint:
	case BezierCurveDataType::kF32FloatingPoint:
		_bezierCurveDataTypePopUp->setSelectedTag(static_cast<uint32>(bezierCurveDataType));
		break;
	default:
		warning("Zoombini2OptionsWidget::load(): Invalid %s(%d)", Zoombini2MetaEngine::kConfigBezierCurveDataType, bezierCurveDataTypeValue);
		_bezierCurveDataTypePopUp->setSelectedTag(static_cast<uint32>(BezierCurveDataType::kQ10FixedPoint));
		break;
	}
	const int colorAssistValue = ConfMan.getInt(Zoombini2MetaEngine::kConfigColorAssistMode, _domain);
	const ColorAssistMode colorAssistMode = static_cast<ColorAssistMode>(colorAssistValue);
	switch (colorAssistMode) {
	case ColorAssistMode::kOriginal:
	case ColorAssistMode::kEnhancedDistinction:
	case ColorAssistMode::kRedGreenBlindAssist:
		_colorAssistPopUp->setSelectedTag(static_cast<uint32>(colorAssistMode));
		break;
	default:
		warning("Zoombini2OptionsWidget::load(): Invalid %s(%d)", Zoombini2MetaEngine::kConfigColorAssistMode, colorAssistValue);
		_colorAssistPopUp->setSelectedTag(static_cast<uint32>(ColorAssistMode::kOriginal));
		break;
	}
	_enhancedKbdShortcutsCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigEnhancedKbdShortcuts, _domain));
	_wallOfFleensFixDepartStreakCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigWallOfFleensFixDepartStreak, _domain));
	_mysticMarshFillCellGapsCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigMysticMarshFillCellGaps, _domain));
	_mysticMarshExpandAreaMaskCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigMysticMarshExpandAreaMask, _domain));
	if (_helpDialogTransparentBodyCheckbox)
		_helpDialogTransparentBodyCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigHelpDialogTransparentBody, _domain));
	_debugHotkeysCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigDebugHotkeys, _domain));
	_chezNorfClueSubtitlesCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigChezNorfClueSubtitles, _domain));
	_greedyWaterslideCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigGreedyWaterslidePairing, _domain));
	_allowRestoreL4PracticePuzzlesCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigRestoreL4PracticePuzzles, _domain));
	_prngAlgorithmPopUp->setSelectedTag(ConfMan.getInt(Zoombini2MetaEngine::kConfigPrngAlgorithm, _domain));
	_pacingPopUp->setSelectedTag(ConfMan.getInt(Zoombini2MetaEngine::kConfigLogicPacingMode, _domain));
	const int frameRate = CLIP<int>(ConfMan.getInt(Zoombini2MetaEngine::kConfigFrameRate, _domain), Zoombini2MetaEngine::kMinFrameRate, Zoombini2MetaEngine::kMaxFrameRate);
	_frameRateNumberBox->setValue(frameRate);
	_unlockFrameRateCheckbox->setState(ConfMan.getBool(Zoombini2MetaEngine::kConfigUnlockFrameRate, _domain));
	updateFrameRateControls();
}

bool Zoombini2OptionsWidget::save() {
	const int frameRate = _frameRateNumberBox->getValue();
	_frameRateNumberBox->setValue(frameRate);

	const AudioOutputMode audioOutputMode = static_cast<AudioOutputMode>(_audioOutputPopUp->getSelectedTag());
	const BezierCurveDataType bezierCurveDataType = static_cast<BezierCurveDataType>(_bezierCurveDataTypePopUp->getSelectedTag());
	const ColorAssistMode colorAssistMode = static_cast<ColorAssistMode>(_colorAssistPopUp->getSelectedTag());
	const PrngAlgorithmKind prngAlgorithm = static_cast<PrngAlgorithmKind>(_prngAlgorithmPopUp->getSelectedTag());
	const LogicPacingMode logicPacingMode = static_cast<LogicPacingMode>(_pacingPopUp->getSelectedTag());

	ConfMan.setBool(Zoombini2MetaEngine::kConfigEnableSavefileReadOnlyToggle, _savefileReadOnlyToggleCheckbox->getState(), _domain);
	ConfMan.setInt(Zoombini2MetaEngine::kConfigAudioOutputMode, static_cast<int>(audioOutputMode), _domain);
	ConfMan.setInt(Zoombini2MetaEngine::kConfigBezierCurveDataType, static_cast<int>(bezierCurveDataType), _domain);
	ConfMan.setInt(Zoombini2MetaEngine::kConfigColorAssistMode, static_cast<int>(colorAssistMode), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigEnhancedKbdShortcuts, _enhancedKbdShortcutsCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigWallOfFleensFixDepartStreak, _wallOfFleensFixDepartStreakCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigMysticMarshFillCellGaps, _mysticMarshFillCellGapsCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigMysticMarshExpandAreaMask, _mysticMarshExpandAreaMaskCheckbox->getState(), _domain);
	if (_helpDialogTransparentBodyCheckbox)
		ConfMan.setBool(Zoombini2MetaEngine::kConfigHelpDialogTransparentBody, _helpDialogTransparentBodyCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigDebugHotkeys, _debugHotkeysCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigChezNorfClueSubtitles, _chezNorfClueSubtitlesCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigGreedyWaterslidePairing, _greedyWaterslideCheckbox->getState(), _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigRestoreL4PracticePuzzles, _allowRestoreL4PracticePuzzlesCheckbox->getState(), _domain);
	ConfMan.setInt(Zoombini2MetaEngine::kConfigPrngAlgorithm, static_cast<int>(prngAlgorithm), _domain);
	ConfMan.setInt(Zoombini2MetaEngine::kConfigLogicPacingMode, static_cast<int>(logicPacingMode), _domain);
	ConfMan.setInt(Zoombini2MetaEngine::kConfigFrameRate, frameRate, _domain);
	ConfMan.setBool(Zoombini2MetaEngine::kConfigUnlockFrameRate, _unlockFrameRateCheckbox->getState(), _domain);
	return true;
}

void Zoombini2OptionsWidget::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {
	if (cmd == kUnlockFrameRateCommand) {
		updateFrameRateControls();
		return;
	}
	if (cmd == kManageSavefilesCommand) {
		Zoombini2SaveManagementDialog dialog(_domain);
		dialog.runModal();
		return;
	}
	GUI::OptionsContainerWidget::handleCommand(sender, cmd, data);
}

void Zoombini2OptionsWidget::updateFrameRateControls() {
	const bool unlocked = _unlockFrameRateCheckbox->getState();
	_frameRateNumberBox->setEnabled(!unlocked);
}

} // End of namespace Zoombini2
