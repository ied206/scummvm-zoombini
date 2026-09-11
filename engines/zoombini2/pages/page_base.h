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

#ifndef ZOOMBINI2_PAGES_PAGE_BASE_H
#define ZOOMBINI2_PAGES_PAGE_BASE_H

#include "common/events.h"
#include "common/rect.h"
#include "common/scummsys.h"

#include "graphics/managed_surface.h"

namespace Zoombini2 {

class Zoombini2Engine;
class ManagedSurface32;

/** Page ownership categories used by engine and sidebar policy. */
enum class PageCategory {
	/** Player-controlled map, shelter, menu, or puzzle. */
	kInteractive = 1,
	/** Timed travel, title, video, or credits page. */
	kTransition = 2,
	/** Modal overlay retaining its underlying page. */
	kDialog = 3,
};

/** Result returned by an input handler after it processes one event. */
enum class EventHandleResult {
	/** This handler did not claim the event. */
	kPassthrough,
	/** This handler claimed the event. */
	kConsumed,
};

/** Common ordered input dispatch for pages and their shared controls. */
class PageEventHandler {
public:
	virtual ~PageEventHandler() {}
	/** Dispatch one backend event to its typed callback. */
	EventHandleResult handleEvent(const Common::Event &event);
	/** Handle a left-button press. */
	virtual EventHandleResult onLButtonDown(const Common::Point &pos) {
		(void)pos;
		return EventHandleResult::kPassthrough;
	}
	/** Handle a left-button release. */
	virtual EventHandleResult onLButtonUp(const Common::Point &pos) {
		(void)pos;
		return EventHandleResult::kPassthrough;
	}
	/** Handle a pointer movement in game coordinates. */
	virtual EventHandleResult onMouseMove(const Common::Point &pos) {
		(void)pos;
		return EventHandleResult::kPassthrough;
	}
	/** Handle a key press with its backend repeat flag. */
	virtual EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) {
		(void)key;
		(void)repeat;
		return EventHandleResult::kPassthrough;
	}
	/** Handle a key release. */
	virtual EventHandleResult onKeyUp(const Common::KeyState &key) {
		(void)key;
		return EventHandleResult::kPassthrough;
	}
};

/**
 * Owns one screen-facing update, drawing, and input lifecycle.
 * The engine dispatches one regular page at a time. Modal dialogs derive from
 * this base but remain separately owned by the sidebar while retaining the
 * underlying dispatched page.
 */
class PageBase : public PageEventHandler {
public:
	/** Bind this page to its owning @p vm and record its @p pageCategory. */
	PageBase(Zoombini2Engine *vm, PageCategory pageCategory);
	/** Release resources owned by the concrete page. */
	virtual ~PageBase();

	/** Advance page state when permitted, then execute the complete render pass. */
	void onFrame(ManagedSurface32 *screen, bool advanceState);
	/** Recompose the selected visuals without advancing simulation or completion callbacks. */
	void render(ManagedSurface32 *screen);

	/** Initialize page-local state and resources. */
	virtual void init() = 0;
	/** Return whether the engine must clear the screen before @ref Page::render. */
	virtual bool needsScreenClear() const { return false; }
	/** Return whether a panel belonging to this page exclusively handles its input. */
	virtual bool hasActiveDialog() const { return false; }

	/** Return the numeric dispatcher identifier assigned by the concrete page. */
	int getPageId() const { return _pageId; }
	/** Return the ownership category recorded when this page was constructed. */
	PageCategory getCategory() const { return _pageCategory; }
	/** Return whether the shared sidebar is visible over this page. */
	virtual bool hasSidebar() const { return false; }
	/** Return whether the sidebar includes a Go button for this page. */
	virtual bool hasGoButton() const { return hasSidebar(); }
	/** Return whether the visible Go button currently accepts input. */
	virtual bool canUseGoButton() const { return hasGoButton(); }
	/** Return whether this page owns a shelter flow. */
	virtual bool isShelter() const { return false; }
	/** Apply any page-local state required by the global debug-completion hotkey. */
	virtual void applyDebugPuzzleCompletion() {}

protected:
	/** Advance this page's simulation before rendering. */
	virtual void onUpdate() {}
	/** Restore or draw this page's background pixels. */
	virtual void onRenderBackground(ManagedSurface32 *screen) { (void)screen; }
	/** Draw this page's scene elements behind its actors. */
	virtual void onRenderScene(ManagedSurface32 *screen) = 0;
	/** Draw actors at the page's actor boundary. */
	virtual void onRenderActors(ManagedSurface32 *screen) { (void)screen; }
	/** Advance actor completion work after drawing, before foreground composition, on an active frame only. */
	virtual void onActorsRendered() {}
	/** Draw this page's elements that cover its actors. */
	virtual void onRenderForeground(ManagedSurface32 *screen) { (void)screen; }
	/** Complete work after composition on an active frame only. */
	virtual void onPostRender() {}

	/** Reference to the engine interface. */
	Zoombini2Engine *_vm;
	/** Category assigned to this page for lifecycle and UI policy. */
	PageCategory _pageCategory;
	/** Numeric dispatcher identifier for this page. */
	int _pageId = -1;

private:
	/** Run visual hooks and, when requested, the two completion boundaries. */
	void renderFrame(ManagedSurface32 *screen, bool advanceState);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PAGE_BASE_H
