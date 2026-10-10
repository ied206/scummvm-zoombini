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

#ifndef ZOOMBINI2_PAGES_DIALOG_DEBUG_H
#define ZOOMBINI2_PAGES_DIALOG_DEBUG_H

#include "common/path.h"
#include "common/rect.h"
#include "common/str.h"
#include "zoombini2/pages/dialog_base.h"

namespace Zoombini2 {

class Zoombini2Engine;
class Animation;
class ManagedSurface32;

/**
 * One debug view request for @ref DialogDebug.
 *
 * Each view is selected by its command type and carries only the arguments
 * that view needs. Additional views slot in as new command types.
 */
struct DialogDebugCommand {
	/** Debug views selectable through the console draw and plot commands. */
	enum class Type {
		/** No view selected. */
		kNone,
		/** Show the active page through its area mask. */
		kDrawAreaMask,
		/** Show frames of an animation or sprite resource. */
		kDrawAnimation,
		/** Plot one pixel on a white canvas. */
		kPlotPoint,
		/** Plot a line on a white canvas. */
		kPlotLine,
		/** Plot a rectangle outline on a white canvas. */
		kPlotRect
	};

	/** Selected debug view. */
	Type _type = Type::kNone;
	/** Animation (.an) or sprite (.rb) resource path for @ref DialogDebugCommand::Type::kDrawAnimation. */
	Common::Path _animPath;
	/** Zero-based starting frame for @ref DialogDebugCommand::Type::kDrawAnimation. */
	int _startFrame = 0;
	/** Pixel position, first line endpoint, or rectangle's top-left corner for a plot command. */
	Common::Point32 _plotStart;
	/** Last inclusive line endpoint or exclusive bottom-right rectangle corner for a plot command. */
	Common::Point32 _plotEnd;
	/** Diagnostic RGB value in 0xRRGGBB form, independent of the screen format. */
	uint32 _plotColor = 0;

	/** Select a command that shows the active page through its area mask. */
	void setDrawAreaMask() { _type = Type::kDrawAreaMask; }
	/** Select a command that shows @p path from @p startFrame. */
	void setDrawAnimation(const Common::Path &path, int startFrame) {
		_type = Type::kDrawAnimation;
		_animPath = path;
		_startFrame = startFrame;
	}
	/** Plot @p point using a 24-bit RGB @p color. */
	void setPlotPoint(const Common::Point32 &point, uint32 color) {
		_type = Type::kPlotPoint;
		_plotStart = point;
		_plotColor = color;
	}
	/** Plot from @p start through @p end, including both endpoints, using a 24-bit RGB @p color. */
	void setPlotLine(const Common::Point32 &start, const Common::Point32 &end, uint32 color) {
		_type = Type::kPlotLine;
		_plotStart = start;
		_plotEnd = end;
		_plotColor = color;
	}
	/** Plot an outline from @p topLeft to the exclusive @p bottomRight corner using a 24-bit RGB @p color. */
	void setPlotRect(const Common::Point32 &topLeft, const Common::Point32 &bottomRight, uint32 color) {
		_type = Type::kPlotRect;
		_plotStart = topLeft;
		_plotEnd = bottomRight;
		_plotColor = color;
	}
};

/**
 * Modal debug view dispatched from the console draw and plot commands.
 *
 * The area-mask view snapshots the screen on open and masks rejected drop areas with black.
 * The animation view renders resource frames over a blank sheet and permits manual frame stepping.
 * Plot views retain a white canvas with one primitive drawn through the shared graphics interface.
 * Gameplay timing is paused until the view closes.
 * Every view closes on any click or ESC key.
 */
class DialogDebug : public DialogBase {
public:
	/** Bind one debug overlay to @p vm. */
	explicit DialogDebug(Zoombini2Engine *vm);
	/** Release the saved screen and loaded view resources. */
	~DialogDebug() override;

	/** Set up @p cmd and open the modal. */
	bool open(const DialogDebugCommand &cmd);
	/** Close the overlay and resume gameplay timing. */
	void close() override;

	/** Return whether this overlay is open and should receive modal input and rendering. */
	bool isActive() const override { return _isActive; }

	/** Draw the active debug view. */
	void onRenderContent(ManagedSurface32 *screen) override;

	/** Close the modal on any click while it is active. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Swallow pointer movement while the modal is active. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Close the modal on ESC and step animation frames with the arrow keys. */
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;

private:
	/** Release the loaded animation or sprite, if any. */
	void freeAnimation();
	/** Return the number of frames in the loaded animation view. */
	int getFrameCount() const;
	/** Refresh the animation title from the current frame index. */
	void updateAnimationTitle();
	/** Prepare the plot canvas and title, drawing rectangle edges before clipping so offscreen edges stay offscreen. */
	void preparePlot(const DialogDebugCommand &cmd);
	/** Draw the stored title line over a title bar at the top of @p screen. */
	void drawTitleText(ManagedSurface32 *screen) const;
	/** Draw the close legend right-aligned in the title bar, appended to @p keyLegend when given. */
	void drawEscText(ManagedSurface32 *screen, const Common::String &keyLegend = Common::String()) const;

	/** Whether the debug overlay is active. */
	bool _isActive = false;
	/** Active debug view. */
	DialogDebugCommand::Type _viewType = DialogDebugCommand::Type::kNone;
	/** Masked page snapshot or prepared plot canvas copied into each debug frame. */
	ManagedSurface32 *_savedScreen = nullptr;
	/** Title line describing the active view. */
	Common::String _titleText;
	/** Loaded animation for the animation view, or nullptr. */
	Animation *_animation = nullptr;
	/** Whether the active animation view uses one page-cached RLE sprite. */
	bool _singleFrameSprite = false;
	/** Resource path of the loaded animation or sprite. */
	Common::Path _animPath;
	/** Zero-based index of the displayed animation frame. */
	int _frameIndex = 0;
	/** Height of the debug title bar in pixels. */
	static constexpr int kTitleHeight = 22;
	/** Title text origin inside the debug title bar. */
	static constexpr int kTitleX = 8;
	static constexpr int kTitleY = 3;
	/** System tick captured when the overlay paused gameplay. */
};

} // End of namespace Zoombini2

#endif
