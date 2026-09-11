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

#ifndef ZOOMBINI2_PAGES_TRANSITION_MAPTRANS_H
#define ZOOMBINI2_PAGES_TRANSITION_MAPTRANS_H

#include "common/array.h"
#include "common/str.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/transition_base.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;
class ZoombiniAnimation;

/** Transition page that walks the current party along a route on the mountain map. */
class TransitionMapTrans : public TransitionBase {
public:
	/** Construct the map transition for @p vm. */
	TransitionMapTrans(Zoombini2Engine *vm);
	/** Release the composited map and per-Zoombini paths. */
	~TransitionMapTrans() override;

	/** Load the route, compose map overlays, and initialize the walking party. */
	void init() override;
	/** Start and advance staggered walkers until the transition completes. */
	void onUpdate() override;
	/** Draw the composited map and active walkers. */
	void onRenderScene(ManagedSurface32 *screen) override;
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Skip the remaining walking animation and enter the resolved destination. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	/** Resolve the next route page from the source page, Rescue Site I branch, and rescue progress. */
	static PageId getDestPage(PageId src, RouteBranch routeBranch, int rescuedBoolies);

private:
	/** Draw one named overlay at @p pos. */
	void drawOverlaySprite(Graphics::ManagedSurface *dst, const Common::String &name, const Common::Point32 &pos);
	/** Compose overlays appropriate to @p src and @p mapRegion. */
	void drawMapOverlays(Graphics::ManagedSurface *dst, PageId src, int mapRegion);
	/** Start any due walkers and update all active party paths. */
	void walkZoombinis();
	/** Clear the paths and walking state assigned to the walking Zoombinis. */
	void cleanupPaths();
	/** Return the page entered after this transition. */
	PageId getPostTransitionPage() const;
	/** Commit the destination page after the last walker finishes. */
	void finishTransition();

	/** Background with route-specific overlays already applied. */
	Graphics::ManagedSurface *_compositedBg = nullptr;

	/** Route shared as the template for each party member's path. */
	Common::Path _patPath;

	/** Index of the next party member waiting to start. */
	int _nextWalkIndex = 0;
	/** Time at which the next walker may start. */
	uint32 _nextWalkTime = 0;
	/** Number of walkers that have reached the route endpoint. */
	int _completedCount = 0;

	/** Page whose entry follows this route. */
	PageId _targetPageId = kPageNone;
	/** Whether the destination page has already been requested. */
	bool _transitionFinished = false;
	/** Music handle used during the map transition. */
	int _musicId = -1;

	/** Borrowed immutable sprite grid owned by the engine cache. */
	const ZoombiniAnimation *_zoombiniAnimation = nullptr;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_TRANSITION_MAPTRANS_H
