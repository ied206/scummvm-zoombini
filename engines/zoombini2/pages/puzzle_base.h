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

#ifndef ZOOMBINI2_PAGES_PUZZLE_BASE_H
#define ZOOMBINI2_PAGES_PUZZLE_BASE_H

#include "common/array.h"

#include "zoombini2/pages/interactive_base.h"

namespace Zoombini2 {

class BitBlock;
class ZoombiniState;
class ZoombiniGraphics;

/**
 * Common resources and party state for the nine puzzle activities.
 * Shelters, including Booliewood, have their own page implementations.
 */
class ZoombiniPuzzle : public ZoombiniInteractive {
public:
	/** Bind shared puzzle state to @p engine and @p puzzleId. */
	ZoombiniPuzzle(Zoombini2Engine *engine, int puzzleId);
	/** Release shared puzzle resources and roster entries. */
	~ZoombiniPuzzle() override;
	/** Return whether the shared sidebar is visible. */
	bool hasSidebar() const override { return true; }

	/** Initialize the shared puzzle background, sprite grid, and roster. */
	void init() override;
	/** Advance the fallback puzzle state machine. */
	void update() override;
	/** Draw the shared background and party representation. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Handle the fallback puzzle-completion click. */
	void handleClick(const Common::Point &pos) override;

	/** Return the display name belonging to @p puzzleId. */
	static const char *getPuzzleName(int puzzleId);
	/** Return the resource-directory name belonging to @p puzzleId. */
	static const char *getPuzzleDir(int puzzleId);

protected:
	/** Numeric dispatcher identifier for the concrete puzzle. */
	int _puzzleId;
	/** Shared full-screen puzzle background. */
	BitBlock *_background;
	/** Shared Zoombini sprite-grid owner. */
	ZoombiniGraphics *_zoombiniGfx;
	/** Puzzle-owned party entries transferred from the engine or profile state. */
	Common::Array<ZoombiniState *> _puzzleZoombinis;
	/** Shared fallback puzzle stage. */
	int _puzzleState;
	/** Gameplay deadline used by the shared fallback puzzle stage. */
	uint32 _stateTimer;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_BASE_H
