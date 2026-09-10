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
class ZoombiniAnimation;

/**
 * Common page base for the nine rescue-mission puzzles.
 *
 * Provides shared puzzle resources and party state.
 */
class PuzzleBase : public InteractiveBase {
public:
	/** Bind shared puzzle state to @p vm and @p puzzleId. */
	PuzzleBase(Zoombini2Engine *vm, int puzzleId);
	/** Release shared puzzle resources and roster entries. */
	~PuzzleBase() override;
	/** Return whether the shared three-button controls are visible. */
	bool hasThreeButtons() const override { return true; }

	/** Initialize the shared puzzle background, sprite grid, and roster. */
	void init() override;
	/** Return the display name belonging to @p puzzleId. */
	static const char *getPuzzleName(int puzzleId);
	/** Return the resource-directory name belonging to @p puzzleId. */
	static const char *getPuzzleDir(int puzzleId);

protected:
	/** Render active roster entries in stable ascending logical-Y order, with the held entry last. */
	void renderZoombinis(ManagedSurface32 *screen) const;
	/** Numeric dispatcher identifier for the concrete puzzle. */
	int _puzzleId;
	/** Shared full-screen puzzle background. */
	BitBlock *_background;
	/** Borrowed immutable sprite grid owned by the engine cache. */
	const ZoombiniAnimation *_zoombiniAnimation;
	/** Active party entries mirrored from the engine's roster for this puzzle. */
	Common::Array<ZoombiniState *> _puzzleZoombinis;
	/** Gameplay deadline used by the concrete puzzle state machine. */
	uint32 _stateTimer;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_BASE_H
