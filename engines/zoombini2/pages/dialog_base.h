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

#ifndef ZOOMBINI2_PAGES_DIALOG_BASE_H
#define ZOOMBINI2_PAGES_DIALOG_BASE_H

#include "zoombini2/pages/page_base.h"

namespace Zoombini2 {

/** Modal page that retains the dispatched page and consumes its input. */
class DialogBase : public PageBase {
public:
	/** Bind this modal page to its owning @p vm. */
	explicit DialogBase(Zoombini2Engine *vm) : PageBase(vm, PageCategory::kDialog) {}
	/** Release resources owned by the concrete dialog. */
	~DialogBase() override {}
	/** Dialog setup is driven by the opener rather than the page dispatcher. */
	void init() override {}
	/** Dialogs have no dispatcher update step; their opener and the three-button controls drive them. */
	void onUpdate() override {}
	/** Return whether this dialog currently owns drawing and input. */
	virtual bool isActive() const = 0;
	/** Close the dialog and restore any retained page state. */
	virtual void close() = 0;
	/** Draw the dialog over @p screen. */
	virtual void onRenderScene(ManagedSurface32 *screen) override = 0;
	/** Handle a game-space click while the modal blocks its underlying page. */
	virtual EventHandleResult onLButtonDown(const Common::Point &pos) override = 0;
	/** Update dialog hover state for @p pos. */
	virtual EventHandleResult onMouseMove(const Common::Point &pos) override = 0;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_DIALOG_BASE_H
