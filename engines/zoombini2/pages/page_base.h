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

#include "common/rect.h"
#include "common/scummsys.h"

#include "graphics/managed_surface.h"

namespace Zoombini2 {

class Zoombini2Engine;

/** Page ownership categories used by engine and sidebar policy. */
enum class PageCategory {
	kNone00 = 0,        ///< No page category.
	kInteractive01 = 1, ///< Player-controlled map, shelter, menu, or puzzle.
	kTransition02 = 2,  ///< Timed travel, title, video, or credits page.
	kDialog03 = 3       ///< Modal overlay retaining its underlying page.
};

/**
 * Owns one dispatched screen and its update, drawing, and input lifecycle.
 * Modal dialogs retain the underlying page and have a separate lifetime.
 */
class ZoombiniPage {
public:
	/** Bind this page to its owning @p engine. */
	ZoombiniPage(Zoombini2Engine *engine);
	/** Release resources owned by the concrete page. */
	virtual ~ZoombiniPage();

	/** Initialize page-local state and resources. */
	virtual void init() = 0;
	/** Advance page-local timers, animation, and transitions. */
	virtual void update() = 0;
	/** Draw the page into @p screen. */
	virtual void draw(Graphics::ManagedSurface *screen) = 0;
	/** Handle a game-space click at @p pos. */
	virtual void handleClick(const Common::Point &pos) {}

	/** Return whether the engine must clear the screen before @ref ZoombiniPage::draw. */
	virtual bool needsScreenClear() const { return false; }

	/** Return the numeric dispatcher identifier assigned by the concrete page. */
	int getPageId() const { return _pageId; }
	/** Return the ownership category used by engine and sidebar policy. */
	virtual PageCategory getCategory() const = 0;
	/** Return whether the global sidebar is visible over this page. */
	virtual bool hasSidebar() const { return false; }
	/** Return whether the sidebar includes a Go button for this page. */
	virtual bool hasGoButton() const { return hasSidebar(); }
	/** Return whether the visible Go button currently accepts input. */
	virtual bool canUseGoButton() const { return hasGoButton(); }
	/** Return whether this page owns a shelter flow. */
	virtual bool isShelter() const { return false; }

protected:
	/** Borrowed engine that owns this page. */
	Zoombini2Engine *_engine;
	/** Numeric dispatcher identifier for this page. */
	int _pageId;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PAGE_BASE_H
