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

#include "zoombini2/pages/page_base.h"
#include "zoombini2/graphics.h"

namespace Zoombini2 {

PageBase::PageBase(Zoombini2Engine *vm, PageCategory pageCategory) : _vm(vm), _pageCategory(pageCategory) {
}

PageBase::~PageBase() {
}

EventHandleResult PageEventHandler::handleEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		return onLButtonDown(event.mouse);
	case Common::EVENT_LBUTTONUP:
		return onLButtonUp(event.mouse);
	case Common::EVENT_MOUSEMOVE:
		return onMouseMove(event.mouse);
	case Common::EVENT_KEYDOWN:
		return onKeyDown(event.kbd, event.kbdRepeat);
	case Common::EVENT_KEYUP:
		return onKeyUp(event.kbd);
	default:
		return EventHandleResult::kPassthrough;
	}
}

void PageBase::onFrame(ManagedSurface32 *screen, bool advanceState) {
	if (advanceState)
		onUpdate();
	renderFrame(screen, advanceState);
}

void PageBase::render(ManagedSurface32 *screen) {
	renderFrame(screen, false);
}

void PageBase::renderFrame(ManagedSurface32 *screen, bool advanceState) {
	if (needsScreenClear())
		screen->fillRect(Common::Rect32(screen->w, screen->h), 0);
	onRenderBackground(screen);
	onRenderScene(screen);
	onRenderActors(screen);
	if (advanceState)
		onActorsRendered();
	onRenderForeground(screen);
	if (advanceState)
		onPostRender();
}

} // End of namespace Zoombini2
