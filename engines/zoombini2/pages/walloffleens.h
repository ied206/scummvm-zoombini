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

#ifndef ZOOMBINI2_PAGES_WALLOFFLEENS_H
#define ZOOMBINI2_PAGES_WALLOFFLEENS_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle.h"

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * WallOfFleensPuzzle — Cannon/mirror grid puzzle (ID 7).
 *
 * Original: WallOfFleens__Init_432150, object size 0x3AC4C (240,716 bytes).
 * vtable at 0x48040C. Non-blocking model (per-frame Update).
 *
 * Mechanics (from IDA analysis of HandleInput_42FE40):
 *   - A wall/grid of fleens is displayed, each with 4 features (1-5)
 *   - Player clicks a fleen cell on the grid
 *   - Cannon rotates to aim at the clicked cell (one step per 200ms)
 *   - When aimed, cannonball fires along a trajectory to the cell
 *   - Feature comparison: match zoombini's 4 features vs fleen's 4 features
 *   - 4/4 match → fleen caught, zoombini placed successfully
 *   - <4/4 match → miss, lose one mirror/chance
 *   - Round ends when all mirrors lost or puzzle requirements met
 *
 * Grid dimensions by difficulty:
 *   - Diff 1: 3x2 = 6 fleens, 12 mirrors (cycles through 6 grid panels)
 *   - Diff 2: 9x6 = 54 fleens, 8 mirrors
 *   - Diff 3: 12x6 = 72 fleens, 6 mirrors
 *   - Diff 4: 12x6 = 72 fleens, 6 mirrors (dual-target matching)
 *
 * Completion: When >= 4 zoombinis are freed.
 */
class WallOfFleensPuzzle : public PuzzlePage {
public:
	WallOfFleensPuzzle(Zoombini2Engine *engine);
	~WallOfFleensPuzzle() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

	// === Constants ===
	static const int kMaxGridCols = 12;
	static const int kMaxGridRows = 6;
	static const int kMaxFleens = 72;       // 12 * 6
	static const int kNumCannonAngles = 9;  // canon00-08
	static const int kNumMirrorStates = 6;
	static const int kMinFreed = 4;         // Minimum zoombinis to free for completion
	static const int kCellWidth = 52;       // From DrawSlot_42F0F0: 52 * col
	static const int kCellHeight = 68;      // From DrawSlot_42F0F0: 68 * row
	static const int kNumFeatures = 4;      // 4 zoombini features (hair/eyes/nose/feet)
	static const int kMaxFeatureVal = 5;    // Feature values 1-5
	static const int kNumLevelIndicators = 5;
	static const int kNumDiff1Panels = 6;   // Cycling panels for difficulty 1

	// Cannon positions (from IDA)
	static const int kCannonDrawX = 385;    // CRleBlock draw position
	static const int kCannonDrawY = 465;
	static const int kCannonCenterX = 470;  // Angle computation center
	static const int kCannonCenterY = 550;

	// === Enums ===
	enum GameState {
		kStateIdle = 0,       // Waiting for player click on grid cell
		kStateAiming = 1,     // Cannon rotating towards target angle
		kStateFiring = 2,     // Cannonball traveling to target cell
		kStateHit = 3,        // Fleen caught — show result
		kStateMiss = 4,       // Miss — show result, lose mirror
		kStateNextZoombini = 5, // Preparing next zoombini at cannon
		kStateDone = 6        // Puzzle complete, transitioning out
	};

	enum MirrorState {
		kMirrorNormal = 0,
		kMirrorGris = 1,
		kMirrorNoir = 2,
		kMirrorFelure = 3,
		kMirrorExplode = 4,
		kMirrorEmpty = 5
	};

	// === Structs ===
	struct FleenCell {
		byte features[kNumFeatures]; // Feature values 1-5
		bool caught;
		int gridCol;
		int gridRow;
		Common::Rect hitbox;

		FleenCell() : caught(false), gridCol(0), gridRow(0) {
			memset(features, 0, sizeof(features));
		}
	};

private:
	// --- Resources ---
	void loadResources();

	// --- Setup ---
	void buildGrid();
	void generateFleenFeatures();

	// --- Game Logic ---
	int computeCannonAngle(int targetX, int targetY) const;
	int countMatchingFeatures(int fleenIdx) const;
	void fireCannon();
	void catchFleen(int fleenIdx);
	void missFleen();
	void advanceToNextZoombini();
	void checkCompletion();
	int fleenIndexAt(int col, int row) const;

	// --- Drawing Helpers ---
	void drawGrid(Graphics::ManagedSurface *screen);
	void drawCannon(Graphics::ManagedSurface *screen);
	void drawCannonball(Graphics::ManagedSurface *screen);
	void drawMirrors(Graphics::ManagedSurface *screen);
	void drawZoombinis(Graphics::ManagedSurface *screen);

	// --- State ---
	int _difficulty;            // 1-4 from gameMode
	int _gridCols;              // Grid columns for current difficulty
	int _gridRows;              // Grid rows for current difficulty
	int _numFleens;             // Total fleens = cols * rows
	int _initialZoombiniCount;  // Original count before freeing
	int _freedCount;            // Number freed so far
	int _currentZoombini;       // Index of current zoombini being played
	int _selectedFleen;         // Currently selected fleen (-1 = none)
	GameState _gameState;
	uint32 _actionTimer;

	// Grid panel cycling (difficulty 1 only)
	int _gridPage;              // Current grid page (0-5 for diff 1)

	// Grid data
	FleenCell _fleens[kMaxFleens];

	// Grid position offset (from IDA dword_48FF9C/dword_48FFA0)
	int _gridOriginX;
	int _gridOriginY;

	// Cannon state (from IDA HandleInput_42FE40)
	int _cannonAngle;           // Current cannon angle 0-8 (4 = center/up)
	int _targetAngle;           // Target angle to rotate towards
	int _targetCol;             // Clicked fleen grid column
	int _targetRow;             // Clicked fleen grid row

	// Cannonball projectile state
	int _cannonballX;           // Current cannonball X
	int _cannonballY;           // Current cannonball Y
	int _cannonballStartX;      // Muzzle position X
	int _cannonballStartY;      // Muzzle position Y
	int _cannonballEndX;        // Target fleen center X
	int _cannonballEndY;        // Target fleen center Y
	int _cannonballProgress;    // 0-1000 interpolation (fixed point)

	// Mirror/chance system (from IDA offset +240228)
	int _mirrorsLeft;           // Remaining chances
	int _mirrorsTotal;          // Initial mirror count for this difficulty

	// --- Graphics ---
	RleBlock *_cannonGfx[kNumCannonAngles];     // canon00-08.rb
	RleBlock *_cannonCache;                      // canon_cache.rb
	RleBlock *_slotActiveGfx;                    // Slot active cell background
	RleBlock *_slotEmptyGfx;                     // Slot empty cell background
	RleBlock *_slotCursorGfx;                    // Slot cursor overlay
	RleBlock *_mirrorGfx[kNumMirrorStates];      // Mirror state sprites
	RleBlock *_tuyereGfx;                        // Nozzle sprite
	RleBlock *_levelRedGfx[kNumLevelIndicators]; // LevelRED0-4
	RleBlock *_highlightGfx;                     // Catch highlight

	// Animations
	Animation *_lavaBubbleAnim;                  // lava_bubble.an (background decoration)
	Animation *_mirrorExplodeAnim;               // mirror_explode.an (mirror breaking effect)

	int _musicId;  // BGM: sounds/music/05-BB01.wav
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_WALLOFFLEENS_H
