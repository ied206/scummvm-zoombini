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

#ifndef ZOOMBINI2_PAGES_MAPTRANS_H
#define ZOOMBINI2_PAGES_MAPTRANS_H

#include "common/array.h"
#include "common/str.h"

#include "zoombini2/pages/page.h"
#include "zoombini2/path.h"

namespace Zoombini2 {

class BitBlock;
class RleBlock;
class ZoombiniGfx;

/**
 * MapTransition — map transition (page ID 22).
 * Object size: 0x110 (272 bytes).
 * Init: CL_maptrans__Init_4224A0 (10,574 bytes).
 *
 * Loads bigmap background, overlay sprites, .PAT bezier path.
 * Walks zoombinis one-by-one along the path with 800ms stagger.
 * When all zoombinis finish walking, transitions to target world.
 */
/**
 * Per-zoombini walking state for map transition.
 * Tracks screen position and direction for sprite rendering.
 */
struct ZoombiniWalkState {
	int screenX, screenY;      // Current screen position
	int prevX, prevY;          // Previous position (for direction)
	int cellIndex;             // Direction cell index (e.g. 66=right, 88=down)
	bool active;               // Currently walking
};

class MapTransition : public Page {
public:
	MapTransition(Zoombini2Engine *engine);
	~MapTransition() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	void drawOverlaySprite(Graphics::ManagedSurface *dst,
	                       const Common::String &name, int x, int y);
	void drawMapOverlays(Graphics::ManagedSurface *dst, int source, int mapRegion);
	void walkZoombinis();
	void cleanupPaths();
	int getPostTransitionPage() const;
	void finishTransition();
	int computeDirectionCell(int dx, int dy) const;
	void drawZoombiniSprite(Graphics::ManagedSurface *dst,
	                        int zoombiniIdx, int cellIndex,
	                        int x, int y) const;

	// Composited background (background + overlay sprites)
	Graphics::ManagedSurface *_compositedBg;

	// PAT path for zoombini walking
	Common::Path _patPath;

	// Per-zoombini walk state
	Common::Array<PathObject *> _zoombiniPaths;
	Common::Array<ZoombiniWalkState> _walkStates;
	int _nextWalkIndex;
	uint32 _nextWalkTime;
	int _completedCount;

	int _targetWorld;
	bool _transitionFinished;
	int _musicId;

	// Zoombini sprite graphics (littleZomb.anm)
	ZoombiniGfx *_zoombiniGfx;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_MAPTRANS_H
