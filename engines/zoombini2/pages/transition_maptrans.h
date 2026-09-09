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

#include "zoombini2/pages/transition_base.h"
#include "zoombini2/path.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;
class ZoombiniGraphics;

/** Runtime position and animation state for one Zoombini walking across the map. */
struct ZoombiniWalkState {
	/** Current screen position. */
	Common::Point32 screenPosition;
	/** Previous screen position used to determine direction. */
	Common::Point32 previousScreenPosition;
	/** Animation cell selected from the current movement direction. */
	int cellIndex;
	/** Whether this walker is still advancing along its path. */
	bool active;
};

/** Transition page that walks the current party along a route on the mountain map. */
class MapTransition : public TransitionPage {
public:
	/** Construct the map transition for @p engine. */
	MapTransition(Zoombini2Engine *engine);
	/** Release the composited map and per-Zoombini paths. */
	~MapTransition() override;

	/** Load the route, compose map overlays, and initialize the walking party. */
	void init() override;
	/** Start and advance staggered walkers until the transition completes. */
	void update() override;
	/** Draw the composited map and active walkers. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Skip the remaining walking animation and enter the resolved destination. */
	void handleClick(const Common::Point &pos) override;
	/** Resolve the next world page from the source world, route, and rescue progress. */
	static int getDestPage(int source, int route, int rescuedBoolies);

private:
	/** Draw one named overlay at @p position. */
	void drawOverlaySprite(Graphics::ManagedSurface *dst,
						   const Common::String &name, const Common::Point32 &position);
	/** Compose overlays appropriate to @p source and @p mapRegion. */
	void drawMapOverlays(Graphics::ManagedSurface *dst, int source, int mapRegion);
	/** Start any due walkers and update all active party paths. */
	void walkZoombinis();
	/** Delete all owned path objects and clear walking state. */
	void cleanupPaths();
	/** Return the page entered after this transition. */
	int getPostTransitionPage() const;
	/** Commit the destination page after the last walker finishes. */
	void finishTransition();
	/** Convert a movement delta into a directional animation cell. */
	int computeDirectionCell(int dx, int dy) const;
	/** Draw one party member with the cell selected for its direction. */
	void drawZoombiniSprite(Graphics::ManagedSurface *dst,
							int zoombiniIdx, int cellIndex,
							const Common::Point32 &position) const;

	/** Background with route-specific overlays already applied. */
	Graphics::ManagedSurface *_compositedBg;

	/** Route shared as the template for each party member's path. */
	Common::Path _patPath;

	/** Independently timed paths for the party. */
	Common::Array<PathObject *> _zoombiniPaths;
	/** Screen and direction state corresponding to @ref MapTransition::_zoombiniPaths. */
	Common::Array<ZoombiniWalkState> _walkStates;
	/** Index of the next party member waiting to start. */
	int _nextWalkIndex;
	/** Time at which the next walker may start. */
	uint32 _nextWalkTime;
	/** Number of walkers that have reached the route endpoint. */
	int _completedCount;

	/** World whose entry page follows this route. */
	int _targetWorld;
	/** Whether the destination page has already been requested. */
	bool _transitionFinished;
	/** Music handle used during the map transition. */
	int _musicId;

	/** Shared little-Zoombini animation cells used for the walking party. */
	ZoombiniGraphics *_zoombiniGfx;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_TRANSITION_MAPTRANS_H
