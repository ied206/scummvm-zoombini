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

#ifndef ZOOMBINI2_PAGES_DIALOG_HELP_H
#define ZOOMBINI2_PAGES_DIALOG_HELP_H

#include "common/rect.h"
#include "common/str.h"
#include "zoombini2/pages/dialog_base.h"

namespace Graphics {
class Font;
}

namespace Zoombini2 {

class Zoombini2Engine;

/**
 * Displays context-sensitive help for one puzzle and difficulty level.
 *
 * @ref DialogHelp::kHelpPageFormat selects a sheet by page ID, difficulty suffix, and one-based sheet number.
 * The dialog pauses gameplay and retains a screen snapshot until it closes, when it restores the screen and resumes the clock.
 * A missing sheet leaves the help frame open; level 4 displays a ScummVM-added notice.
 */
class DialogHelp : public DialogBase {
public:
	/** Bind a help overlay to @p vm. */
	DialogHelp(Zoombini2Engine *vm);
	/** Close the active help sheet and release the saved screen. */
	~DialogHelp() override;

	/** Check whether the requested help-sheet path exists without loading or validating the bitmap. */
	bool isPageValid(PageId pageId, int level, int sheet);

	/**
	 * Save the current screen and open the frame at the first help sheet, even when that sheet is missing.
	 * @return False if this dialog is already open; otherwise true after pausing gameplay and opening the frame.
	 */
	bool open(PageId pageId, int level);
	/** Close the overlay, restore the saved screen, and resume gameplay timing. */
	void close() override;
	/** Return whether this overlay is open and should receive modal input and rendering. */
	bool isActive() const override { return _isActive; }
	/** Keep shared controls visible over the help frame. */
	bool drawsSidebarOnTop() const override { return true; }
	/** Draw the frame, current help sheet, and navigation controls. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Handle navigation or close-button input while the modal is active. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Close the overlay when a close-button press is released inside it. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Update navigation-control hover state for @p pos. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;

private:
	/** RLE frame drawn behind the active help sheet and navigation controls. */
	static constexpr const char *kHelpFramePath = "Bmp/MENU/help_screen_main.rb";
	/** RLE placeholder shown when a supported difficulty has no loaded help sheet. */
	static constexpr const char *kPlaceholderPath = "Bmp/MENU/help_screen_placeholder.rb";
	/** Default bit-block image for the OK button. */
	static constexpr const char *kOkButtonNormalPath = "Bmp/MENU/help_screen_okbutton_normal.bb";
	/** Alternate bit-block image drawn while the pointer hovers over the OK button. */
	static constexpr const char *kOkButtonPushedPath = "Bmp/MENU/help_screen_okbutton_pushed.bb";
	/** Enabled bit-block image for the previous-sheet arrow. */
	static constexpr const char *kLeftArrowNormalPath = "Bmp/MENU/help_screen_leftarro_norma.bb";
	/** Disabled bit-block image for the previous-sheet arrow. */
	static constexpr const char *kLeftArrowEmptyPath = "Bmp/MENU/help_screen_leftarro_empty.bb";
	/** Enabled bit-block image for the next-sheet arrow. */
	static constexpr const char *kRightArrowNormalPath = "Bmp/MENU/help_screen_rightarro_norma.bb";
	/** Disabled bit-block image for the next-sheet arrow. */
	static constexpr const char *kRightArrowEmptyPath = "Bmp/MENU/help_screen_rightarro_empty.bb";
	/** Format for page help-sheet paths: page ID, difficulty suffix, and one-based sheet number. */
	static constexpr const char *kHelpPageFormat = "Bmp/help/%02d_help_%s_%02d.bb";
	/** Localized notices added by ScummVM; the original engine provides no level 4 help message or help sheet. */
	static constexpr const char *kMissingHelpTextEnglish = "No help resource is available for level 4.";
	static constexpr const char *kMissingHelpTextKorean = u8"4단계는 도움말 리소스가 없습니다.";
	/** Layout values added by ScummVM to center its level 4 notice in the otherwise empty help frame. */
	static constexpr int kMissingHelpTextX = 135;
	static constexpr int kMissingHelpTextWidth = 530;
	static constexpr int kMissingHelpTextCenterY = 285;

	/** Replace the active help-sheet path with the requested sheet. */
	bool loadPage(PageId pageId, int level, int sheet);
	/** Forget the active help-sheet path. */
	void freePage();
	/** Return the help-resource filename suffix for @p level, or null when none exists. */
	static const char *getLevelHelpFileSuffix(int level);
	/** Select a localized font for the ScummVM-added level 4 notice, which has no original-engine equivalent. */
	void resolveUiFont();
	/** Draw the ScummVM-added level 4 notice; the original engine leaves the frame without text when no sheet exists. */
	void drawMissingHelpText(ManagedSurface32 *screen) const;

	/** Whether the help overlay is active. */
	bool _isActive = false;
	/** Page identifier used to resolve the active help resource. */
	PageId _currentPageId = kPageNone;
	/** Level used to resolve the active help resource. */
	int _currentLevel = -1;
	/** One-based help sheet currently displayed. */
	int _currentSheet = 1;
	/** Screen snapshot restored when the overlay closes. */
	ManagedSurface32 *_savedScreen = nullptr;
	/** Path of the active help-sheet bitmap in the graphics page cache. */
	Common::String _helpPagePath;
	/** UI font borrowed from the theme while a level-four help dialog is open. */
	const Graphics::Font *_uiFont = nullptr;
	/** Whether the active release and target enable help-sheet color keying. */
	bool _transparentHelpPages = false;
	/** Close-button hit rectangle. */
	const Common::Rect _okButtonRect = Common::Rect(597, 400, 671, 444);
	/** Previous-sheet button hit rectangle. */
	const Common::Rect _leftArrowRect = Common::Rect(135, 400, 181, 444);
	/** Next-sheet button hit rectangle. */
	const Common::Rect _rightArrowRect = Common::Rect(209, 400, 253, 444);
	/** Whether the pointer is over the close button. */
	bool _okButtonHovered = false;
	/** Whether the pointer is over the previous-sheet button. */
	bool _leftArrowHovered = false;
	/** Whether the pointer is over the next-sheet button. */
	bool _rightArrowHovered = false;
	/** Whether the close button press is waiting for its release. */
	bool _okButtonArmed = false;
	/** System tick captured when the help overlay paused gameplay. */
};

} // End of namespace Zoombini2

#endif
