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

#ifndef ZOOMBINI2_PAGES_PUZZLE_BOOLIES_H
#define ZOOMBINI2_PAGES_PUZZLE_BOOLIES_H

#include "common/array.h"
#include "common/rect.h"

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class RleBlock;
class Animation;

/**
 * Boolie Boggle (Route4-2)
 *
 * Use pinballs to make each Boolie group happy and ready to board a boat.
 */
class PuzzleBoolies : public PuzzleBase {
public:
	/** Construct Boolie Boggle for @p vm. */
	PuzzleBoolies(Zoombini2Engine *vm);
	/** Release all bowling and boat resources. */
	~PuzzleBoolies() override;

	/** Load the lane and assign the active Zoombinis. */
	void init() override;
	/** Advance the ball, pins, released Zoombini, and boat phases. */
	void onUpdate() override;
	/** Draw the launch spots, obstacles, pins, actors, and boat. */
	void onRenderScene(ManagedSurface32 *screen) override;
	/** Restore the page background. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Launch the current ball from the selected active spot. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Return the rescued-Boolie credit assigned to each party member at @p level. */
	static int getRescuedBooliesPerZoombini(int level);

private:
	/** Polarity assigned to a rolling Zoombini. */
	enum BallType {
		/** No ball is currently assigned. */
		kBallNone = 0,
		/** Use the positive ball path and visual. */
		kBallPositive,
		/** Use the negative ball path and visual. */
		kBallNegative
	};

	/** Runtime phase of the Boolie Boggle interaction. */
	enum State {
		/** Complete initial placement before accepting input. */
		kStateInit,
		/** Wait for the player to select a launch spot. */
		kStateIdle,
		/** Move the current ball toward the pins. */
		kStateBallRolling,
		/** Hold while knocked pins settle. */
		kStatePinsKnocked,
		/** Move a successfully released Zoombini toward the boat. */
		kStateZoombiniFreed,
		/** Move the completed party away by boat. */
		kStateBoatLeaving,
		/** Stop accepting input after completion. */
		kStateDone
	};

	/** One bowling pin's position and feedback state. */
	struct Pin {
		/** Screen position. */
		Common::Point32 pos;
		/** Whether the pin has been knocked down. */
		bool knocked = false;
		/** Whether the pin is shown as an active target. */
		bool lighted = false;
	};

	/** One selectable ball launch position. */
	struct Spot {
		/** Screen position. */
		Common::Point32 pos;
		/** Clickable area. */
		Common::Rect hitbox;
		/** Whether this spot can launch the current ball. */
		bool active = false;
	};

	/** Position and timing for the currently rolling Zoombini. */
	struct Ball {
		/** Positive or negative path selection. */
		BallType type = kBallNone;
		/** Puzzle-roster index represented by this ball. */
		int zoombiniIdx = -1;
		/** Current screen position. */
		Common::Point32 pos;
		/** Screen position at the start of the roll. */
		Common::Point32 startPos;
		/** Target screen position. */
		Common::Point32 endPos;
		/** Time at which the roll began. */
		uint32 rollStart = 0;
	};

	/** Load all puzzle graphics and animations. */
	void loadResources();
	/** Initialize launch positions and their hit-test areas. */
	void setupSpots();
	/** Initialize the active pin layout. */
	void setupPins();
	/** Leave the puzzle roster in base-page order for sequential ball selection. */
	void assignZoombinis();

	/** Launch the current Zoombini from spot @p spotIdx. */
	void launchBall(int spotIdx);
	/** Advance the current roll and resolve its endpoint. */
	void advanceBallRoll();
	/** Return whether the current ball intersects an active pin. */
	bool checkPinCollision();
	/** Mark all pins reached by the current ball as knocked down. */
	void knockDownPins();
	/** Release puzzle-roster entry @p zoombiniIdx. */
	void freeZoombini(int zoombiniIdx);
	/** Return the number of puzzle-roster entries already released. */
	int countFreeZoombinis() const;

	/** Draw the five launch spots. */
	void drawSpots(ManagedSurface32 *screen);
	/** Draw standing and knocked pins. */
	void drawPins(ManagedSurface32 *screen);
	/** Draw the currently rolling ball. */
	void drawBall(ManagedSurface32 *screen);
	/** Draw the escape boat when visible. */
	void drawBoat(ManagedSurface32 *screen);
	/** Draw lane blockers. */
	void drawBlockers(ManagedSurface32 *screen);
	/** Draw waiting and released Zoombinis. */
	void onRenderActors(ManagedSurface32 *screen) override;

	/** Current interaction phase. */
	State _state = kStateInit;
	/** Selected launch spot, or `-1` when none is selected. */
	int _currentSpot = -1;
	/** Number of Zoombinis already released. */
	int _freedCount = 0;
	/** Total number of pins already knocked down. */
	int _pinsKnocked = 0;
	/** Launch positions. */
	Spot _spots[5] = {};
	/** Active bowling pin layout. */
	Common::Array<Pin> _pins;
	/** Currently rolling Zoombini state. */
	Ball _activeBall;

	/** Current escape-boat position. */
	Common::Point32 _boatPos = Common::Point32(550, 100);
	/** Whether the escape boat is currently drawn. */
	bool _boatVisible = true;

	/** Screen positions of lane blockers. */
	Common::Array<Common::Point32> _blockers;

	/** Positive ball visual. */
	RleBlock *_ballPosImage = nullptr;
	/** Negative ball visual. */
	RleBlock *_ballNegImage = nullptr;
	/** Normal pin visual. */
	RleBlock *_pinImage = nullptr;
	/** Highlighted active-pin visual. */
	RleBlock *_pinLightedImage = nullptr;
	/** Escape-boat visual. */
	RleBlock *_boatImage = nullptr;
	/** Launch-spot visuals. */
	RleBlock *_spotImage[5] = {};
	/** Static blocker visual. */
	RleBlock *_blockerImage = nullptr;
	/** First fixed lane overlay. */
	RleBlock *_fixeImage = nullptr;
	/** Second fixed lane overlay. */
	RleBlock *_fixe2Image = nullptr;
	/** First walking animation. */
	Animation *_marcheAnim = nullptr;
	/** Second walking animation. */
	Animation *_marche2Anim = nullptr;
	/** First waiting animation. */
	Animation *_attendAnim = nullptr;
	/** Second waiting animation. */
	Animation *_attend2Anim = nullptr;
	/** Positive rolling animation. */
	Animation *_rollAnim = nullptr;
	/** Negative rolling animation. */
	Animation *_roll2Anim = nullptr;
	/** Animated blocker effect. */
	Animation *_blockerAnim = nullptr;

	/** Music handle used while Boolie Boggle is active. */
	int _musicId = -1;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_BOOLIES_H
