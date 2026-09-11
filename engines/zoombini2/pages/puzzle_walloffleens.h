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
#include "zoombini2/state.h"

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * Magic Mirrors (Route3-2)
 * 
 * Find the real Fleens behind the strange mirrors.
 */
class PuzzleWallOfFleens : public PuzzleBase {
public:
	/** Construct Magic Mirrors for @p vm. */
	PuzzleWallOfFleens(Zoombini2Engine *vm);
	/** Release grid, cannon, mirror, and animation resources. */
	~PuzzleWallOfFleens() override;

	/** Build the level-selected grid and initialize its mirror allowance. */
	void init() override;
	/** Advance aiming, projectile, feedback, and completion phases. */
	void onUpdate() override;
	/** Draw the grid, cannon, projectile, mirrors, and active Zoombini. */
	void onRenderScene(ManagedSurface32 *screen) override;
	/** Draw the level indicator above the actors. */
	void onRenderForeground(ManagedSurface32 *screen) override;
	/** Aim the cannon at the selected uncaught Fleen. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Mark the terminal board successful for the global debug-completion hotkey. */
	void applyDebugPuzzleCompletion() override;

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
	/** Maximum value of a visible trait. */
	static const int kMaxTraitValue = 5;
	/** Number of level-progress indicator visuals. */
	static const int kNumLevelIndicators = 5;
	/** Number of grid panels cycled by level one. */
	static const int kNumLevel1Panels = 6;

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

	/** One Fleen's traits, grid position, and capture state. */
	struct FleenCell {
		/** Visible traits compared with a Zoombini. */
		ZmbTrait traits;
		/** Whether this Fleen has already been captured. */
		bool caught = false;
		/** Grid column. */
		int gridCol = 0;
		/** Grid row. */
		int gridRow = 0;
		/** Clickable grid-cell area. */
		Common::Rect hitbox = Common::Rect();

		/** Initialize an uncaught Fleen with empty traits at grid origin. */
		FleenCell() = default;
	};

private:
	/** Load cannon, grid, mirror, animation, and audio resources. */
	void loadResources();

	/** Configure cell geometry for the selected level. */
	void buildGrid();
	/** Generate visible traits for each active Fleen. */
	void generateFleenTraits();

	/** Convert target position into the nearest cannon angle. */
	int computeCannonAngle(const Common::Point32 &targetPos) const;
	/** Count traits shared by the current Zoombini and Fleen @p fleenIdx. */
	int countMatchingTraits(int fleenIdx) const;
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
	void drawGrid(ManagedSurface32 *screen);
	/** Draw the cannon at its current angle. */
	void drawCannon(ManagedSurface32 *screen);
	/** Draw the projectile while it is in flight. */
	void drawCannonball(ManagedSurface32 *screen);
	/** Draw remaining and consumed chance mirrors. */
	void drawMirrors(ManagedSurface32 *screen);
	/** Draw the current and already released Zoombinis. */
	void onRenderActors(ManagedSurface32 *screen) override;

	/** Level in the range one through four. */
	int _level = 1;
	/** Number of columns in the active grid. */
	int _gridCols = 3;
	/** Number of rows in the active grid. */
	int _gridRows = 2;
	/** Number of active Fleens. */
	int _numFleens = 6;
	/** Puzzle-roster size before any Zoombinis are released. */
	int _initialZoombiniCount = 0;
	/** Number of Zoombinis already released. */
	int _freedCount = 0;
	/** Puzzle-roster index currently at the cannon. */
	int _currentZoombini = 0;
	/** Selected Fleen index, or `-1` when none is selected. */
	int _selectedFleen = -1;
	/** Current interaction phase. */
	GameState _gameState = kStateIdle00;
	/** Time at which the current phase began. */
	uint32 _actionTimer = 0;

	/** Current grid panel cycled by level one. */
	int _gridPage = 0;

	/** Fleen grid storage sized for the largest level. */
	FleenCell _fleens[kMaxFleens] = {};

	/** Screen origin of the active grid. */
	Common::Point32 _gridOrigin = Common::Point32(380, 200);

	/** Current cannon angle index. */
	int _cannonAngle = 4;
	/** Target cannon angle index. */
	int _targetAngle = 4;
	/** Selected Fleen grid column. */
	int _targetCol = 0;
	/** Selected Fleen grid row. */
	int _targetRow = 0;

	/** Current projectile position. */
	Common::Point32 _cannonballPos = Common::Point32();
	/** Projectile position at the muzzle. */
	Common::Point32 _cannonballStartPos = Common::Point32();
	/** Projectile target at the selected Fleen. */
	Common::Point32 _cannonballEndPos = Common::Point32();
	/** Fixed-point projectile progress from zero through one thousand. */
	int _cannonballProgress = 0;

	/** Number of chance mirrors still available. */
	int _mirrorsLeft = 12;
	/** Initial mirror allowance for the selected level. */
	int _mirrorsTotal = 12;

	/** Cannon visuals indexed by angle. */
	RleBlock *_cannonImage[kNumCannonAngles] = {};
	/** Background restored around the rotating cannon. */
	RleBlock *_cannonCache = nullptr;
	/** Active grid-cell background. */
	RleBlock *_slotActiveImage = nullptr;
	/** Empty grid-cell background. */
	RleBlock *_slotEmptyImage = nullptr;
	/** Selected-cell cursor overlay. */
	RleBlock *_slotCursorImage = nullptr;
	/** Mirror visuals indexed by @ref PuzzleWallOfFleens::MirrorState. */
	RleBlock *_mirrorImage[kNumMirrorStates] = {};
	/** Cannon nozzle visual. */
	RleBlock *_tuyereImage = nullptr;
	/** Progress indicators for released Zoombinis. */
	RleBlock *_levelRedImage[kNumLevelIndicators] = {};
	/** Successful-capture highlight. */
	RleBlock *_highlightImage = nullptr;

	/** Background lava-bubble animation. */
	Animation *_lavaBubbleAnim = nullptr;
	/** Mirror-breaking animation. */
	Animation *_mirrorExplodeAnim = nullptr;

	/** Music handle used while Magic Mirrors is active. */
	int _musicId = -1;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_WALLOFFLEENS_H
