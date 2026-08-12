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

#include "common/scummsys.h"
#include "common/rect.h"

#include "graphics/managed_surface.h"

namespace Zoombini2 {

class Zoombini2Engine;

enum class PageCategory {
	kNone00 = 0,
	kInteractive01 = 1,
	kTransition02 = 2,
	kDialog03 = 3
};

/**
 * Owns one dispatched screen and its update, drawing, and input lifecycle.
 * Modal dialogs retain the underlying page and have a separate lifetime.
 */
class Page {
public:
	Page(Zoombini2Engine *engine);
	virtual ~Page();

	virtual void init() = 0;
	virtual void update() = 0;
	virtual void draw(Graphics::ManagedSurface *screen) = 0;
	virtual void handleClick(const Common::Point &pos) {}

	/**
	 * Returns true if the screen should be cleared before draw().
	 * Default is false to preserve the frame buffer (double buffering like
	 * the original game). Most pages draw a full-screen background each
	 * frame anyway, so they effectively clear by overdrawing.
	 * Override to return true if a page needs explicit screen clear.
	 */
	virtual bool needsScreenClear() const { return false; }

	int getPageId() const { return _pageId; }
	virtual PageCategory getCategory() const = 0;
	virtual bool hasSidebar() const { return false; }
	virtual bool hasGoButton() const { return hasSidebar(); }
	virtual bool canUseGoButton() const { return hasGoButton(); }
	virtual bool isShelter() const { return false; }

protected:
	Zoombini2Engine *_engine;
	int _pageId;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PAGE_BASE_H
