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

#ifndef ZOOMBINI2_PAGES_PUZZLE_MAGICWALL_H
#define ZOOMBINI2_PAGES_PUZZLE_MAGICWALL_H

#include "common/array.h"
#include "common/rect.h"
#include "common/path.h"

#include "zoombini2/pages/puzzle_base.h"
#include "zoombini2/path.h"

namespace Zoombini2 {

class RleBlock;
class Animation;

/**
 * Beetle Bug Alley: stone tablets permute beetles to match colored markers.
 * Matching the markers opens doors for the Zoombinis.
 * The internal MagicWall name identifies the bmp/magic_wall resources.
 */
class MagicWallPuzzle : public PuzzlePage {
public:
	MagicWallPuzzle(Zoombini2Engine *engine);
	~MagicWallPuzzle() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	/**
	 * Color indices matching resource naming.
	 */
	enum Color {
		kColorBlue = 0,
		kColorGreen,
		kColorNavy,
		kColorOrange,
		kColorPurple,
		kColorRed,
		kColorRose,
		kColorTurquoise,
		kColorViolet,
		kColorYellow,
		kColorCount
	};

	static const char *kColorNames[kColorCount];

	/**
	 * Puzzle state machine.
	 */
	enum State {
		kStateInit,
		kStateIdle,               // Waiting for player input
		kStateZoombiniMoving,     // Zoombini traversing path
		kStateGateOpening,        // Gate animation playing
		kStateComplete,           // All zoombinis through
		kStateDone                // Transitioning out
	};

	/**
	 * Slot for a zoombini in the maze (4 slots, directions 0-3).
	 */
	struct ZoombiniSlot {
		int zoombiniIdx;          // Index into _puzzleZoombinis (-1 = empty)
		int pathProgress;         // Current position along path (0-100)
		int targetColor;          // Destination color for the zoombini
		bool captured;             // Has reached its destination
		int x, y;                 // Current screen position
		PathObject *path;         // Current path being traversed
		uint32 pathStartTime;     // Tick when path started
	};

	/**
	 * Color dot placed in the maze.
	 */
	struct ColorDot {
		int colorIdx;             // Color enum value
		int x, y;                 // Screen position
		bool lightOn;             // Light above door is on
	};

	/**
	 * Colored bug that guides zoombinis.
	 */
	struct ColorBug {
		int colorIdx;             // Color enum value
		int x, y;                 // Screen position
		bool active;              // Currently guiding
	};

	/**
	 * Stone tablet that moves beetles.
	 */
	struct Tablet {
		Common::Rect rect;        // Clickable region
		int sourceSlot;           // Slot to move from
		int destSlot;             // Slot to move to
		PathObject *path;         // Path to follow
	};

	/**
	 * Gate/door in the maze.
	 */
	struct Gate {
		int gateIdx;              // Gate index (0-3 = A-D)
		int x, y;                 // Screen position
		bool open;                // Gate state
		uint32 animStart;         // Animation start time
	};

	// Puzzle setup
	void loadResources();
	void setupMaze();
	void placeColorDots();
	void placeColorBugs();
	void setupTablets();
	void assignZoombiniSlots();

	// Game logic
	void startZoombiniPath(int slotIdx);
	void advanceZoombiniPath(int slotIdx);
	bool checkSlotComplete(int slotIdx);
	void completeSlot(int slotIdx);
	void updateLights();
	int countCaptured() const;

	// Drawing helpers
	void drawMazeLevel(Graphics::ManagedSurface *screen, int level);
	void drawColorDots(Graphics::ManagedSurface *screen);
	void drawColorBugs(Graphics::ManagedSurface *screen);
	void drawTablets(Graphics::ManagedSurface *screen);
	void drawWallLever(Graphics::ManagedSurface *screen);
	void drawMinimap(Graphics::ManagedSurface *screen);
	void drawGates(Graphics::ManagedSurface *screen);
	void drawZoombinis(Graphics::ManagedSurface *screen);

	// State
	State _state;
	int _currentLevel;            // Current maze level (difficulty based)
	int _activeSlot;              // Currently moving slot (-1 = none)
	int _destSlot;                // Destination slot for current movement
	int _capturedCount;           // Number captured

	// Slots for zoombinis (4 internal, 4 exit)
	ZoombiniSlot _slots[8];

	// Map from zoombini index to color
	int _zoombiniColors[16];

	// Maze elements
	Common::Array<ColorDot> _colorDots;
	Common::Array<ColorBug> _colorBugs;
	Common::Array<Tablet> _tablets;
	Common::Rect _wallLever;
	Gate _gates[4];

	// Graphics
	RleBlock *_dotGfx[kColorCount];        // DOT-{color}
	RleBlock *_bugGfx[kColorCount];        // bug_c_{color}
	RleBlock *_miniMapGfx;                 // mini-map
	RleBlock *_miniMapDotGfx;              // mini-map-dot
	RleBlock *_miniLightGfx[kColorCount];  // mini-light-{color}
	RleBlock *_glowwormGfx;                // le_vier_luisant
	Animation *_glowwormAnim;              // le_vier
	Animation *_gateAnims[4];              // porte-A/B/C/D
	Animation *_crystalAnims[5];           // Crystal1-5

	// PAT bezier paths for zoombini movement
	PathObject *_exitPaths[4];             // EXIT1-4.PAT (exit paths)
	PathObject *_bougePaths[4];            // BOUGE1-4.PAT (movement paths)

	// Sounds
	int _musicId;                          // BGM: sounds/music/06-BB01.wav
	int _sndApproval[4];                   // sounds/6-A1.wav through 6-A4.wav
	int _sndError[2];                      // sounds/6-E1.wav, 6-E2.wav
	int _sndHint[4];                       // sounds/6-H1.wav through 6-H4.wav
	int _sndGateOpen;                      // sounds/fx/06-BS01.wav (gate opening)
	int _sndZoombiniMove;                  // sounds/fx/06-BS02.wav (movement)
	int _nextApprovalIdx;                  // Cycles through approval sounds
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_MAGICWALL_H
