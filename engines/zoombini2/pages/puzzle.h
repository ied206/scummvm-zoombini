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

#ifndef ZOOMBINI2_PAGES_PUZZLE_H
#define ZOOMBINI2_PAGES_PUZZLE_H

#include "common/array.h"

#include "zoombini2/pages/page.h"

namespace Zoombini2 {

class BitBlock;
class Zoombini;
class ZoombiniGfx;

/**
 * PuzzlePage — base class for the 10 puzzle worlds.
 * Each puzzle has resource directory under Bmp/{name}/.
 *
 * Puzzle IDs: 1=CrazyTurtle, 2=Waterslide, 3=Aquacube,
 *             5=MysticMarsh, 6=MagicWall, 7=WallOfFleens,
 *             8=ChezNorf, 10=Snowboard, 11=Boolies, 12=Booliewood.
 *
 * Two interaction models (from KB):
 *   - Blocking: logic runs in Init, Tick is nullsub
 *   - Non-blocking: Tick override for frame-by-frame logic
 */
class PuzzlePage : public Page {
public:
	PuzzlePage(Zoombini2Engine *engine, int puzzleId);
	~PuzzlePage() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

	static const char *getPuzzleName(int puzzleId);
	static const char *getPuzzleDir(int puzzleId);

protected:
	int _puzzleId;
	BitBlock *_background;
	ZoombiniGfx *_zoombiniGfx;
	Common::Array<Zoombini *> _puzzleZoombinis;
	int _puzzleState;
	uint32 _stateTimer;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_H
