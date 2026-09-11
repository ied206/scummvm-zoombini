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

#include "zoombini2/pages/dialog_base.h"

namespace Graphics {
class ManagedSurface;
}

namespace Zoombini2 {

class BitBlock;
class RleBlock;

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
 * A request supplies only its text resource, position, text offset, and
 * completion callback. The dialog owns the panel resources, saved screen
 * rectangle, pause lifecycle, hover state, and exclusive input routing.
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

	/** Return whether a request is pending or its dialog is open. */
	bool isActive() const override { return _state != DialogMsgBoxState::kClosed00; }
	/** Restore retained state, release request resources, and resume gameplay. */
	void close() override;

	/** Open a pending request and draw its current panel. */
	void onRenderScene(ManagedSurface32 *screen) override;
	/** Consume button presses; activation occurs on release. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Invoke the hovered button's callback on release. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Update the button highlight for @p pos. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Consume keys while the modal is active. */
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	/** Consume key releases while the modal is active. */
	EventHandleResult onKeyUp(const Common::KeyState &key) override;

private:
	/** Load request-specific resources and retain the covered screen rectangle. */
	bool openDialog(ManagedSurface32 *screen);
	/** Release all resources loaded for the current request. */
	void releaseResources();
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
	Common::Path _textPath;
	/** Callback owned for the lifetime of the request. */
	Common::BaseCallback<DialogMsgBoxButton> *_callback = nullptr;
	/** Button currently under the pointer. */
	DialogMsgBoxButton _hoveredButton = DialogMsgBoxButton::kNone00;
	/** Whether the retained panel rectangle must be recomposed. */
	bool _redrawNeeded = false;
	/** Neutral, OK-highlighted, and Cancel-highlighted panel sprites. */
	RleBlock *_panels[3] = {};
	/** Request-specific text bitmap. */
	BitBlock *_textImage = nullptr;
	/** Saved pixels covered by the panel. */
	Graphics::ManagedSurface *_savedBackground = nullptr;
	/** System tick captured when the open dialog paused gameplay. */
	uint32 _pauseStartTime = 0;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_DIALOG_MSGBOX_H
