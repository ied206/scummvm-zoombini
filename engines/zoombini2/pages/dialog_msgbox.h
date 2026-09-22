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

#ifndef ZOOMBINI2_PAGES_DIALOG_MSGBOX_H
#define ZOOMBINI2_PAGES_DIALOG_MSGBOX_H

#include "common/callback.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/ustr.h"

#include "zoombini2/pages/dialog_base.h"

namespace Graphics {
class Font;
}

namespace Zoombini2 {

/** Numeric result supplied to a shared confirmation callback. */
enum class DialogMsgBoxButton {
	kNone00 = 0,
	kOkay01 = 1,
	kCancel02 = 2
};

/** Lifecycle of the engine-owned confirmation dialog. */
enum class DialogMsgBoxState {
	kClosed00 = 0,
	kPendingOpen01 = 1,
	kOpen02 = 2
};

/**
 * Reusable two-button confirmation dialog shared by pages and controls.
 *
 * A request supplies either a game text resource or UI text, position, text
 * offset, and completion callback. The dialog borrows its panel resources from
 * the graphics page cache and retains the saved screen rectangle, pause
 * lifecycle, hover state, and exclusive input routing.
 */
class DialogMsgBox : public DialogBase {
public:
	/** Bind the reusable confirmation dialog to @p vm. */
	explicit DialogMsgBox(Zoombini2Engine *vm);
	/** Close the dialog and release any pending callback or resources. */
	~DialogMsgBox() override;

	/**
	 * Queue one confirmation request and take ownership of @p callback.
	 *
	 * A request made while another confirmation is active is rejected and its
	 * callback is deleted.
	 */
	bool request(const Common::Path &textPath, Common::BaseCallback<DialogMsgBoxButton> *callback,
				 const Common::Point32 &position = Common::Point32(-1, -1), const Common::Point32 &textOffset = Common::Point32(17, 17));

	/**
	 * Queue a confirmation whose text is drawn with the release language font.
	 */
	bool requestUiText(const Common::U32String &text, Common::BaseCallback<DialogMsgBoxButton> *callback, const Common::Point32 &position = Common::Point32(-1, -1));

	/** Return whether a request is pending or its dialog is open. */
	bool isActive() const override { return _state != DialogMsgBoxState::kClosed00; }
	/** Restore retained state, release request resources, and resume gameplay. */
	void close() override;

	/** Open a pending request and draw its current panel. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Consume button presses; activation occurs on release. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Invoke the hovered button's callback on release. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Update the button highlight for @p pos. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Handle enhanced keyboard shortcuts while the modal is active. */
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	/** Consume key releases while the modal is active. */
	EventHandleResult onKeyUp(const Common::KeyState &key) override;

private:
	/** Confirmation panel resource paths indexed by button state. */
	static constexpr const char *kPanelPaths[3] = {
		"bmp/menu/QUIT_panel_nothing.rb",
		"bmp/menu/QUIT_panel_ok.rb",
		"bmp/menu/QUIT_panel_cancel.rb",
	};
	/** Horizontal margin around UI text within the panel. */
	static constexpr int kUiTextMarginX = 14;
	/** Vertical offset of UI text within the panel. */
	static constexpr int kUiTextOffsetY = 12;
	/** Width reserved for wrapped UI text. */
	static constexpr int kUiTextWidth = 348;
	/** Height reserved for wrapped UI text above the buttons. */
	static constexpr int kUiTextHeight = 60;

	/** Begin a request after rejecting any conflicting active request. */
	bool beginRequest(Common::BaseCallback<DialogMsgBoxButton> *callback, const Common::Point32 &position);
	/** Load request-specific resources and retain the covered screen rectangle. */
	bool openDialog();
	/** Select the UI font for the current request from the release language. */
	void resolveUiFont();
	/** Release the saved screen pixels for the current request. */
	void releaseResources();
	/** Draw the current UI-font text request over the panel. */
	void drawUiText(ManagedSurface32 *screen) const;
	/** Return the dialog button under @p pos. */
	DialogMsgBoxButton hitTest(const Common::Point &pos) const;
	/** Close the dialog, then invoke the request callback with @p button. */
	void activateButton(DialogMsgBoxButton button);

	/** Current request lifecycle. */
	DialogMsgBoxState _state = DialogMsgBoxState::kClosed00;
	/** Requested screen origin; (-1,-1) centers the loaded panel. */
	Common::Point32 _position = Common::Point32(-1, -1);
	/** Text bitmap offset relative to @ref DialogMsgBox::_position. */
	Common::Point32 _textOffset = Common::Point32(17, 17);
	/** Request-specific text bitmap path. */
	Common::String _textPath;
	/** Request-specific text drawn with the release language font. */
	Common::U32String _uiText;
	/** UI font borrowed from the theme for the current request, or nullptr. */
	const Graphics::Font *_uiFont = nullptr;
	/** Callback owned for the lifetime of the request. */
	Common::BaseCallback<DialogMsgBoxButton> *_callback = nullptr;
	/** Button currently under the pointer. */
	DialogMsgBoxButton _hoveredButton = DialogMsgBoxButton::kNone00;
	/** Whether the retained panel rectangle must be recomposed. */
	bool _redrawNeeded = false;
	/** Saved pixels covered by the panel. */
	ManagedSurface32 *_savedBackground = nullptr;
	/** System tick captured when the open dialog paused gameplay. */
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_DIALOG_MSGBOX_H
