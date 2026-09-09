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

#ifndef ZOOMBINI2_PAGES_PUZZLE_WALLOFFLEENS_H
#define ZOOMBINI2_PAGES_PUZZLE_WALLOFFLEENS_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * Magic Mirrors asks the player to fire each Zoombini at a matching Fleen.
 *
 * Difficulty controls the grid size, mirror allowance, and matching rules.
 * A complete feature match captures the selected Fleen, while a miss consumes
 * a mirror. Releasing at least four Zoombinis completes the puzzle.
 */
class WallOfFleensPuzzle : public PuzzlePage {
public:
	/** Construct Magic Mirrors for @p engine. */
	WallOfFleensPuzzle(Zoombini2Engine *engine);
	/** Release grid, cannon, mirror, and animation resources. */
	~WallOfFleensPuzzle() override;

	/** Build the difficulty-selected grid and initialize its mirror allowance. */
	void init() override;
	/** Advance aiming, projectile, feedback, and completion phases. */
	void update() override;
	/** Draw the grid, cannon, projectile, mirrors, and active Zoombini. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Aim the cannon at the selected uncaught Fleen. */
	void handleClick(const Common::Point &pos) override;

	/** Maximum number of grid columns. */
	static const int kMaxGridCols = 12;
	/** Maximum number of grid rows. */
	static const int kMaxGridRows = 6;
	/** Maximum Fleen capacity of the grid. */
	static const int kMaxFleens = 72;
	/** Number of cannon-angle visuals. */
	static const int kNumCannonAngles = 9;
	/** Number of mirror damage visuals. */
	static const int kNumMirrorStates = 6;
	/** Minimum number of released Zoombinis required for completion. */
	static const int kMinFreed = 4;
	/** Horizontal spacing between grid cells. */
	static const int kCellWidth = 52;
	/** Vertical spacing between grid cells. */
	static const int kCellHeight = 68;
	/** Number of visible Zoombini features compared with a Fleen. */
	static const int kNumFeatures = 4;
	/** Maximum value of a visible feature. */
	static const int kMaxFeatureVal = 5;
	/** Number of level-progress indicator visuals. */
	static const int kNumLevelIndicators = 5;
	/** Number of grid panels cycled by difficulty one. */
	static const int kNumDiff1Panels = 6;

	/** Runtime phase of the Magic Mirrors interaction. */
	enum GameState {
		/** Wait for a grid-cell click. */
		kStateIdle00 = 0,
		/** Rotate the cannon toward the selected cell. */
		kStateAiming01 = 1,
		/** Move the cannonball toward the selected cell. */
		kStateFiring02 = 2,
		/** Show successful capture feedback. */
		kStateHit03 = 3,
		/** Show miss feedback and consume a mirror. */
		kStateMiss04 = 4,
		/** Prepare the next Zoombini at the cannon. */
		kStateNextZoombini05 = 5,
		/** Stop accepting input while leaving the page. */
		kStateDone06 = 6
	};

	/** Visual damage state of one remaining-chance mirror. */
	enum MirrorState {
		/** Intact mirror. */
		kMirrorNormal00 = 0,
		/** Gray mirror damage state. */
		kMirrorGris01 = 1,
		/** Dark mirror damage state. */
		kMirrorNoir02 = 2,
		/** Cracked mirror damage state. */
		kMirrorFelure03 = 3,
		/** Exploding mirror state. */
		kMirrorExplode04 = 4,
		/** Empty mirror slot. */
		kMirrorEmpty05 = 5
	};

	/** One Fleen's features, grid position, and capture state. */
	struct FleenCell {
		/** Visible feature values. */
		byte features[kNumFeatures];
		/** Whether this Fleen has already been captured. */
		bool caught;
		/** Grid column. */
		int gridCol;
		/** Grid row. */
		int gridRow;
		/** Clickable grid-cell area. */
		Common::Rect hitbox;

		/** Initialize an uncaught Fleen with zeroed features at grid origin. */
		FleenCell() : caught(false), gridCol(0), gridRow(0) {
			memset(features, 0, sizeof(features));
		}
	};

private:
	/** Load cannon, grid, mirror, animation, and audio resources. */
	void loadResources();

	/** Configure cell geometry for the selected difficulty. */
	void buildGrid();
	/** Generate visible features for each active Fleen. */
	void generateFleenFeatures();

	/** Convert target position into the nearest cannon angle. */
	int computeCannonAngle(const Common::Point32 &targetPosition) const;
	/** Count features shared by the current Zoombini and Fleen @p fleenIdx. */
	int countMatchingFeatures(int fleenIdx) const;
	/** Start the projectile phase after the cannon finishes aiming. */
	void fireCannon();
	/** Capture Fleen @p fleenIdx and release the current Zoombini. */
	void catchFleen(int fleenIdx);
	/** Apply miss feedback and consume one mirror. */
	void missFleen();
	/** Move the next puzzle-roster entry to the cannon. */
	void advanceToNextZoombini();
	/** Finish the round when its success or failure condition is met. */
	void checkCompletion();
	/** Convert grid coordinates to a Fleen index, or return `-1`. */
	int fleenIndexAt(int col, int row) const;

	/** Draw every active Fleen grid cell. */
	void drawGrid(Graphics::ManagedSurface *screen);
	/** Draw the cannon at its current angle. */
	void drawCannon(Graphics::ManagedSurface *screen);
	/** Draw the projectile while it is in flight. */
	void drawCannonball(Graphics::ManagedSurface *screen);
	/** Draw remaining and consumed chance mirrors. */
	void drawMirrors(Graphics::ManagedSurface *screen);
	/** Draw the current and already released Zoombinis. */
	void drawZoombinis(Graphics::ManagedSurface *screen);

	/** Difficulty level in the range one through four. */
	int _difficulty;
	/** Number of columns in the active grid. */
	int _gridCols;
	/** Number of rows in the active grid. */
	int _gridRows;
	/** Number of active Fleens. */
	int _numFleens;
	/** Puzzle-roster size before any Zoombinis are released. */
	int _initialZoombiniCount;
	/** Number of Zoombinis already released. */
	int _freedCount;
	/** Puzzle-roster index currently at the cannon. */
	int _currentZoombini;
	/** Selected Fleen index, or `-1` when none is selected. */
	int _selectedFleen;
	/** Current interaction phase. */
	GameState _gameState;
	/** Time at which the current phase began. */
	uint32 _actionTimer;

	/** Current grid panel cycled by difficulty one. */
	int _gridPage;

	/** Fleen grid storage sized for the largest difficulty. */
	FleenCell _fleens[kMaxFleens];

	/** Screen origin of the active grid. */
	Common::Point32 _gridOrigin;

	/** Current cannon angle index. */
	int _cannonAngle;
	/** Target cannon angle index. */
	int _targetAngle;
	/** Selected Fleen grid column. */
	int _targetCol;
	/** Selected Fleen grid row. */
	int _targetRow;

	/** Current projectile position. */
	Common::Point32 _cannonballPosition;
	/** Projectile position at the muzzle. */
	Common::Point32 _cannonballStartPosition;
	/** Projectile target at the selected Fleen. */
	Common::Point32 _cannonballEndPosition;
	/** Fixed-point projectile progress from zero through one thousand. */
	int _cannonballProgress;

	/** Number of chance mirrors still available. */
	int _mirrorsLeft;
	/** Initial mirror allowance for the selected difficulty. */
	int _mirrorsTotal;

	/** Cannon visuals indexed by angle. */
	RleBlock *_cannonGfx[kNumCannonAngles];
	/** Background restored around the rotating cannon. */
	RleBlock *_cannonCache;
	/** Active grid-cell background. */
	RleBlock *_slotActiveGfx;
	/** Empty grid-cell background. */
	RleBlock *_slotEmptyGfx;
	/** Selected-cell cursor overlay. */
	RleBlock *_slotCursorGfx;
	/** Mirror visuals indexed by @ref WallOfFleensPuzzle::MirrorState. */
	RleBlock *_mirrorGfx[kNumMirrorStates];
	/** Cannon nozzle visual. */
	RleBlock *_tuyereGfx;
	/** Progress indicators for released Zoombinis. */
	RleBlock *_levelRedGfx[kNumLevelIndicators];
	/** Successful-capture highlight. */
	RleBlock *_highlightGfx;

	/** Background lava-bubble animation. */
	Animation *_lavaBubbleAnim;
	/** Mirror-breaking animation. */
	Animation *_mirrorExplodeAnim;

	/** Music handle used while Magic Mirrors is active. */
	int _musicId;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_WALLOFFLEENS_H
