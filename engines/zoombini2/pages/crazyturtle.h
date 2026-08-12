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

#ifndef ZOOMBINI2_PAGES_CRAZYTURTLE_H
#define ZOOMBINI2_PAGES_CRAZYTURTLE_H

#include <vector>

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle.h"

namespace Zoombini2 {

class Animation;
class RleBlock;

/**
 * CrazyTurtlePuzzle — Bridge building puzzle (ID 1).
 *
 * Original: CrazyTurtle__Init_416660 (0xBC9C = 48284 bytes).
 *
 * Mechanics:
 *   - 10 clickable turtles of 4 types (tourbillonne 1-4)
 *   - Mother turtle (mere) provides hints and narration
 *   - Click turtles to activate them (spin animation)
 *   - Correct sequence of turtles builds bridge segments
 *   - Zoombinis can cross on completed bridge segments
 *   - Matching based on zoombini traits vs turtle types
 *
 * Resources (bmp/crazy_turtle/):
 *   - tortues/tourbillonne/1-4/: Turtle type animations (spin) + fixed sprites
 *   - tortues/attente/1-4/: Turtle waiting/idle animations
 *   - tortues/mere/: Mother turtle (mere, meredebut, merefin, parle)
 *   - pont.rb, poutrelle.rb, pontKC.rb: Bridge segment sprites
 *   - Area.bmt: Clickable region definitions
 *   - 1.PAT, 2.PAT, 3.PAT, EXIT.PAT: Path files for zoombini movement
 *   - smokey.an: Smoke effect animation
 *
 * vtable at 0x4803B8:
 *   +0: CheckFreeZoombinis (0x417920)
 *   +4: nullsub (blocking model)
 *   +8: Destructor (0x415200)
 */
class CrazyTurtlePuzzle : public PuzzlePage {
public:
	CrazyTurtlePuzzle(Zoombini2Engine *engine);
	~CrazyTurtlePuzzle() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;
	void handleClick(const Common::Point &pos) override;

private:
	// --- Turtle Types ---
	// 4 types of turtles, each with different visual style
	// Type corresponds to zoombini feature matching
	enum TurtleType {
		kTurtleType1 = 0,
		kTurtleType2 = 1,
		kTurtleType3 = 2,
		kTurtleType4 = 3,
		kTurtleTypeCount = 4
	};

	// --- Turtle State ---
	enum TurtleState {
		kTurtleIdle,       // Waiting (attente animation)
		kTurtleSpinning,   // Being clicked (tourbillonne animation)
		kTurtleFixed,      // Part of bridge (fixe sprite)
		kTurtleInactive    // Not usable
	};

	// --- Puzzle States ---
	enum PuzzleState {
		kStateInit,
		kStateIdle,
		kStateTurtleClicked,
		kStateTurtleSpinning,
		kStateZoombiniCrossing,
		kStateZoombiniFalling,
		kStateMotherSpeaking,
		kStateDone
	};

	// --- Turtle Info ---
	struct Turtle {
		int x, y;                  // Position
		TurtleType type;           // Which of 4 types
		TurtleState state;         // Current state
		Common::Rect hitbox;       // Clickable region
		uint32 animStart;          // Animation start time
		bool active;               // Is this turtle in play
	};

	// --- Internal Methods ---
	void loadResources();
	void loadTurtleResources();
	void setupTurtles();
	
	void clickTurtle(int turtleIdx);
	void updateTurtleAnimation(int turtleIdx);
	
	void startZoombiniCrossing();
	void startZoombiniFalling();
	void freeZoombini(int zoombiniIdx);
	int countFreeZoombinis() const;

	void drawTurtles(Graphics::ManagedSurface *screen);
	void drawMother(Graphics::ManagedSurface *screen);
	void drawZoombinis(Graphics::ManagedSurface *screen);

	// --- State ---
	PuzzleState _state;
	int _freedCount;
	int _currentTurtle;
	uint32 _stateTimer;
	int _wrongCount;
	int _maxWrongCount;
	
	// Target sequence for the puzzle
	std::vector<int> _targetSequence;
	int _currentSequenceIdx;

	// --- Turtles ---
	static const int kMaxTurtles = 10;
	Turtle _turtles[kMaxTurtles];
	int _numTurtles;
	int _activatedCount;          // How many turtles have been activated


	// --- Mother Turtle State ---
	int _motherState;             // 0=debut, 1=normal, 2=fin
	bool _motherSpeaking;

	// --- Graphics Resources ---
	// Turtle idle animations (attente)
	Animation *_turtleIdleAnim[kTurtleTypeCount];

	// Turtle spinning animations (tourbillonne)
	Animation *_turtleSpinAnim[kTurtleTypeCount];

	// Turtle fixed sprites (fixe)
	RleBlock *_turtleFixedGfx[kTurtleTypeCount];

	// Mother turtle
	Animation *_motherAnim;
	RleBlock *_motherDebutGfx;
	RleBlock *_motherFinGfx;
	Animation *_motherSpeakAnim;

	// Effects
	Animation *_smokeyAnim;

	// Zoombini graphics
	RleBlock *_zombisTombeGfx;   // Zoombini falling/crossing graphics

	int _musicId;  // BGM: sounds/music/08-BS01.wav
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_CRAZYTURTLE_H
