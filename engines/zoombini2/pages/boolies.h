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

#ifndef ZOOMBINI2_PAGES_BOOLIES_H
#define ZOOMBINI2_PAGES_BOOLIES_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle.h"

namespace Zoombini2 {

class RleBlock;
class Animation;

/**
 * BooliesPuzzle — Bowling puzzle (World ID 11).
 *
 * Original: Boolies__Init_406F40, Boolies__CheckFreeZoombinis_40DED0.
 *
 * Concept:
 *   - Bowling-inspired puzzle with positive and negative balls
 *   - Zoombinis roll as balls to knock down pins
 *   - 5 target spots for launching
 *   - Blockers can obstruct paths
 *   - Boat (bateau) for escape after freeing zoombinis
 *   - Use path files (b_boolies*.pat, jump*.pat) for ball movement
 *   - Goal: Free at least 4 zoombinis by knocking down pins
 *
 * Object size: 0x4D8 bytes. vtable at 0x480398.
 * Blocking model: logic runs in Init, Update is nullsub.
 *
 * Resources:
 *   - marche.an, marche2.an: Walking animations
 *   - attend.an, attend2.an: Waiting animations
 *   - roll.an, roll2.an: Rolling animations
 *   - fixe.rb, fixe2.rb: Fixed position sprites
 *   - blocker.rb, blocker.an: Blocker sprites
 *   - pin.rb, pin_lighted.rb: Bowling pins
 *   - ball_neg.rb, ball_pos.rb: Negative/positive balls
 *   - bateau.rb: Escape boat
 *   - spot01-05.rb: Launch spots
 *   - b_boolies{1-4}.pat, b_boolies{1-4}_exit.pat: Movement paths
 *   - jump{0-2}.pat: Jump paths
 */
class BooliesPuzzle : public PuzzlePage {
public:
	BooliesPuzzle(Zoombini2Engine *engine);
	~BooliesPuzzle() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	/**
	 * Ball type (positive or negative).
	 */
	enum BallType {
		kBallNone = 0,
		kBallPositive,
		kBallNegative
	};

	/**
	 * Puzzle state machine.
	 */
	enum State {
		kStateInit,
		kStateIdle,               // Waiting for player to select spot
		kStateBallRolling,        // Ball is rolling toward pins
		kStatePinsKnocked,        // Pins knocked down animation
		kStateZoombiniFreed,      // Zoombini freed, moving to boat
		kStateBoatLeaving,        // Boat departing
		kStateDone                // Puzzle complete
	};

	/**
	 * Bowling pin state and position.
	 */
	struct Pin {
		int x, y;                 // Screen position
		bool knocked;             // True if knocked down
		bool lighted;             // True if pin is lit (active target)
	};

	/**
	 * Launch spot for rolling the ball.
	 */
	struct Spot {
		int x, y;                 // Screen position
		Common::Rect hitbox;      // Clickable region
		bool active;              // Available for launch
	};

	/**
	 * Rolling ball state.
	 */
	struct Ball {
		BallType type;            // Positive or negative
		int zoombiniIdx;          // Which zoombini is the ball
		int x, y;                 // Current position
		int startX, startY;       // Start position
		int endX, endY;           // Target position
		uint32 rollStart;         // Animation start time
	};

	// Setup
	void loadResources();
	void setupSpots();
	void setupPins();
	void assignZoombinis();

	// Game logic
	void launchBall(int spotIdx);
	void advanceBallRoll();
	bool checkPinCollision();
	void knockDownPins();
	void freeZoombini(int zoombiniIdx);
	int countFreeZoombinis() const;

	// Drawing
	void drawSpots(Graphics::ManagedSurface *screen);
	void drawPins(Graphics::ManagedSurface *screen);
	void drawBall(Graphics::ManagedSurface *screen);
	void drawBoat(Graphics::ManagedSurface *screen);
	void drawBlockers(Graphics::ManagedSurface *screen);
	void drawZoombinis(Graphics::ManagedSurface *screen);

	// State
	State _state;
	int _currentSpot;             // Selected launch spot (-1 = none)
	int _freedCount;              // Zoombinis freed
	int _pinsKnocked;             // Total pins knocked
	int _maxAttempts;             // Max attempts per zoombini (difficulty-based)

	// Spots and pins
	Spot _spots[5];
	Common::Array<Pin> _pins;
	Ball _activeBall;

	// Boat position (for escape)
	int _boatX, _boatY;
	bool _boatVisible;

	// Blocker positions
	Common::Array<Common::Point> _blockers;

	// Graphics
	RleBlock *_ballPosGfx;        // Positive ball
	RleBlock *_ballNegGfx;        // Negative ball
	RleBlock *_pinGfx;            // Normal pin
	RleBlock *_pinLightedGfx;     // Lighted pin
	RleBlock *_boatGfx;           // Escape boat
	RleBlock *_spotGfx[5];        // Launch spots (spot01-05)
	RleBlock *_blockerGfx;        // Blocker sprite
	RleBlock *_fixeGfx;           // Fixed position sprite
	RleBlock *_fixe2Gfx;          // Fixed position sprite 2
	Animation *_marcheAnim;       // Walking animation
	Animation *_marche2Anim;      // Walking animation 2
	Animation *_attendAnim;       // Waiting animation
	Animation *_attend2Anim;      // Waiting animation 2
	Animation *_rollAnim;         // Rolling animation
	Animation *_roll2Anim;        // Rolling animation 2
	Animation *_blockerAnim;      // Blocker animation

	int _musicId;  // BGM: sounds/music/09-BB01.wav
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_BOOLIES_H
