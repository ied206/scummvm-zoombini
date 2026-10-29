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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_ZOOMBINI3_PAGES_PUZZLE_CATAPULT_H
#define TRINKET_ZOOMBINI3_PAGES_PUZZLE_CATAPULT_H

#include "common/ptr.h"
#include "common/rect.h"
#include "trinket/frame_clock.h"
#include "trinket/shell/scene.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_world.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

/** Dataset configuration and frame predicates for the Catapult machine. */
class CatapultRules {
public:
	/** Number of authored catapult machine configurations. */
	static constexpr uint kDatasetCount = 13;
	/** Maximum number of rocks represented by one party. */
	static constexpr uint kPartyLimit = 12;

	/** Initialize the default machine families and difficulty. */
	CatapultRules();
	/** Select machine families and timing rules for the difficulty, party size and dataset. */
	bool configure(uint level, uint partySize, uint dataset, int overrideDataset = -1);
	/** Return the selected catapult difficulty. */
	uint level() const { return _level; }
	/** Return the number of party members represented by rocks. */
	uint partySize() const { return _partySize; }
	/** Return the selected authored machine dataset. */
	uint dataset() const { return _dataset; }
	/** Return the resource family used by primary catchers. */
	uint primaryFamily() const { return _primaryFamily; }
	/** Return the resource family used by the receiving paddle. */
	uint paddleFamily() const { return _paddleFamily; }
	/** Return the resource family used by the machine drive. */
	uint driveFamily() const { return _driveFamily; }
	/** Return the number of primary catchers used by this configuration. */
	uint primaryCount() const { return _primaryCount; }
	/** Return the phase divisor used to stagger primary catcher frames. */
	uint primaryPhaseDivisor() const { return _primaryPhaseDiv; }
	/** Calculate a primary catcher's initial one-based animation frame. */
	uint primaryStartFrame(uint index, uint frameCount) const;
	/** Calculate a secondary catcher's initial one-based animation frame. */
	uint secondaryStartFrame(uint index, uint frameCount) const;
	/** Check whether the primary catcher's frame releases its paddle latch. */
	bool releasesPrimaryLatch(uint frame) const;
	/** Check whether the primary catcher's frame can accept an object from the chute. */
	bool canReceiveFromChute(uint frame) const;
	/** Check whether a secondary frame can receive an object from a primary catcher. */
	bool canReceiveFromPrimary(uint frame) const;
	/** Return the primary animation frame used while transporting an object. */
	uint primaryTransportFrame() const;
	/** Map a catcher frame to the transported object's secondary frame. */
	int secondaryTransportFrame(uint catcherFrame) const;
	/** Check whether the paddle frame is open for an incoming object. */
	bool paddleIsOpen(uint frame, uint frameCount) const;
	/** Return the configured paddle height in game pixels. */
	int paddleHeight() const;
	/** Return the horizontal launch velocity in game pixels per second. */
	int flightVelocity() const;
	/** Check whether two chute-object rectangles have collided. */
	static bool chuteCollision(const Common::Rect &first, const Common::Rect &second);

	/** Resolve one script draw; -1 requests another draw with the same history. */
	static int datasetForDraw(uint draw, const bool *seen);

private:
	/** Selected difficulty controlling machine timing and families. */
	uint _level = 1;
	/** Number of party members represented by the rock supply. */
	uint _partySize = 0;
	/** Selected authored machine configuration. */
	uint _dataset = 0;
	/** Animation resource family for the primary catchers. */
	uint _primaryFamily = 0;
	/** Animation resource family for the receiving paddle. */
	uint _paddleFamily = 0;
	/** Animation resource family for the drive mechanism. */
	uint _driveFamily = 0;
	/** Number of primary catchers used by this machine. */
	uint _primaryCount = 2;
	/** Divisor used to stagger primary catcher animation phases. */
	uint _primaryPhaseDiv = 2;
};

/** Walk and bounce state for one Catapult projectile, independent of its animation. */
class CatapultMotion {
public:
	enum Boundary {
		kBounce,
		kBounceStopped,
		kRight,
		kLeft,
		kBottom,
		kTop,
	};
	class Listener {
	public:
		/** Allow destruction through the catapult-motion callback interface. */
		virtual ~Listener() {}
		/** May reconfigure motion and position; defer destruction until update returns. */
		virtual void motionBoundary(CatapultMotion &motion, Common::Rect &position, Boundary boundary, uint32 time) = 0;
	};
	/** Initialize stopped motion with default walking and bounce parameters. */
	CatapultMotion();
	/** Set the game-pixel rectangle used for motion boundary checks. */
	void setBounds(const Common::Rect &bounds) { _bounds = bounds; }
	/** Begin constant pixel-per-second motion at the supplied millisecond timestamp. */
	void startWalk(int xVelocity, int yVelocity, uint32 time);
	/** Begin a timed bounce with the supplied velocity, acceleration and per-bounce decay. */
	void startBounce(int velocity, int acceleration, int decay, const Common::Rect &position, uint32 time);
	/** Pause motion at a millisecond timestamp. */
	void pause(uint32 time);
	/** Resume motion while excluding the paused interval from bounce timing. */
	void resume(uint32 time);
	/** Move the rectangle and report boundary crossings at the supplied millisecond time. */
	void update(Common::Rect &position, uint32 time, Listener &listener);
	/** Return horizontal walking velocity in game pixels per second. */
	int walkXVelocity() const { return _walkX; }
	/** Return whether motion is currently paused. */
	bool isPaused() const { return _paused; }

private:
	/** Game-pixel rectangle used for motion boundary checks. */
	Common::Rect _bounds = Common::Rect(800, 600);
	/** Active motion intervals sampled from the caller's millisecond clock. */
	FrameClock _clock;
	/** Milliseconds advanced within the current bounce. */
	uint32 _bounceElapsed = 0;
	/** Fractional horizontal pixels retained between updates. */
	double _remainderX = 0;
	/** Fractional vertical pixels retained between updates. */
	double _remainderY = 0;
	/** Vertical pixel position where the current bounce began. */
	double _bounceOriginY = 0;
	/** Previously evaluated vertical bounce position in pixels. */
	double _prevBounceY = 0;
	/** Horizontal walking velocity in pixels per second. */
	int _walkX = 60;
	/** Vertical walking velocity in pixels per second. */
	int _walkY = 0;
	/** Upward velocity available for the next bounce, in pixels per second. */
	int _bounceVelocity = 300;
	/** Upward velocity used by the current bounce, in pixels per second. */
	int _curBounceVelocity = 0;
	/** Quadratic vertical-motion coefficient in pixels per second squared. */
	int _acceleration = 400;
	/** Upward velocity removed after each bounce. */
	int _decay = 0;
	/** Whether constant walking motion is enabled. */
	bool _walking = false;
	/** Whether the bounce trajectory is being advanced. */
	bool _bouncing = false;
	/** Whether both walking and bouncing are paused. */
	bool _paused = false;
};

/** Native MultiTrinket round state; MPS scripts handle party, dialogue and progress. */
class CatapultMachine : public Shell::MultiTrinket {
public:
	enum Action {
		kInsertMud01 = 1,
		kInsertRock02 = 2,
		kToggle03 = 3,
	};

	/** Page-local sound resource IDs used by the machine and page. */
	enum PageResourceId : int16 {
		kResSound10000_BGM = 10000,
		kResSound10402_RockContact = 10402,
		kResSound10403_MudDrop = 10403,
		kResSound10404_Splash = 10404,
		kResSound10405_RockSplash = 10405,
		kResSound10407_Launch = 10407,
		kResSound10411_PaddleHit = 10411,
		kResSound10412_MudSplash = 10412,
		kResSound10413_Trigger = 10413,
		kResSound10414_Miss = 10414,
		kResSound10415_Impact = 10415,
		kResSound10416_GateClosed = 10416,
		kResSound10417_GateOpen = 10417,
		kResSoundRange10000_AmbientBase = 10000,
		kResSound10099_AmbientLast = 10099,
		kResSound10400_ChuteLoop = 10400,
		kResSound10409_GearLoop = 10409,
	};
	struct Result {
		/** Whether the resolved rock landed successfully. */
		bool correct;
		/** Whether this result resolves the final outstanding rock. */
		bool finished;
	};
	class ResultListener {
	public:
		/** Allow destruction through the catapult-result callback interface. */
		virtual ~ResultListener() {}
		/** Called during the mechanical update; must not destroy the machine. */
		virtual void catapultResult(const Result &result, uint32 time) = 0;
	};
	/** Configure catapult interaction and sprites using the scene and optional selected rules. */
	CatapultMachine(Shell::Scene &scene, const CatapultRules &rules);
	/** Configure catapult interaction and sprites using the scene and optional selected rules. */
	explicit CatapultMachine(Shell::Scene &scene);
	/** Remove moving objects, machine sprites and feedback effects. */
	~CatapultMachine();
	/** Dispatch machine start, release and control script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Return the selected machine configuration and timing rules. */
	const CatapultRules &rules() const { return _rules; }
	/** Build the machine, supplies and catcher sprites from the selected configuration. */
	bool load();
	/** Handle a catapult control's input action. */
	void action(int action);
	/** Advance chute motion, catcher transfers, flights and round completion. */
	void update(uint32 time);
	/** Pause machine motion and animations at a millisecond timestamp. */
	void pause(uint32 time);
	/** Resume machine motion while adjusting paused deadlines. */
	void resume(uint32 time);
	/** Consume the next queued catapult sound ID. */
	bool pollSound(uint16 &sound);
	/** Consume a completed rock's result when no result listener handles it directly. */
	bool pollResult(Result &result);
	/** Set the callback notified when a rock is resolved. */
	void setResultListener(ResultListener *listener) { _resultListener = listener; }
	/** Return whether the chute supply is currently advancing. */
	bool chuteMoving() const { return _chuteMoving; }
	/** Return whether the machine is enabled and unpaused. */
	bool machineRunning() const { return _running && !_paused; }
	/** Return the number of rocks that have not produced a result. */
	uint unresolvedRocks() const { return _unresolved; }
	/** Describe the machine's release timing for debugger answer output. */
	Common::String debugAnswer() const;
	/** Describe the current catcher and paddle timing windows. */
	Common::String debugChance() const;
	/** Return whether a required machine resource failed to load. */
	bool hasResourceError() const { return _resError; }

private:
	/** Script identifier for running. */
	static constexpr const char *kRunningName = "running";
	/** Script identifier for stopped. */
	static constexpr const char *kStoppedName = "stopped";
	/** Script identifier for open. */
	static constexpr const char *kOpenName = "open";
	/** Script identifier for closed. */
	static constexpr const char *kClosedName = "closed";
	/** Script event name for correct. */
	static constexpr const char *kEventCorrect = "correct";
	/** Script event name for incorrect. */
	static constexpr const char *kEventIncorrect = "incorrect";
	/** Script event name for finished Correct. */
	static constexpr const char *kEventFinishedCorrect = "finishedCorrect";
	/** Script event name for finished Incorrect. */
	static constexpr const char *kEventFinishedIncorrect = "finishedIncorrect";
	/** Script identifier for token Picked Up. */
	static constexpr const char *kTokenPickedUpName = "tokenPickedUp";
	/** Script method name for setup Game. */
	static constexpr const char *kMethodSetupGame = "setupGame";
	/** Script method name for start Round. */
	static constexpr const char *kMethodStartRound = "startRound";
	/** Script method name for pause Round. */
	static constexpr const char *kMethodPauseRound = "pauseRound";
	/** Script method name for finish Round. */
	static constexpr const char *kMethodFinishRound = "finishRound";
	/** Script method name for resume Round. */
	static constexpr const char *kMethodResumeRound = "resumeRound";

	/** Animation path pattern for catapult machine resources. */
	static constexpr const char *kAnimation = "/z3a1/%u.ao";
	/** Animation path pattern for catapult close-up resources. */
	static constexpr const char *kDiscAnimation = "/z3a1cd/%u.ao";
	enum Stage {
		kSupply,
		kChute,
		kPrimary,
		kSecondary,
		kMissFlight,
		kFlight,
		kMissSplash,
		kWaterSplash,
	};
	enum Landing {
		kWater,
		kPaddle,
		kTrigger,
		kNoLanding,
	};
	struct Object : Core::AnimationPlayer::PauseListener, Shell::Sprite::UpdateListener, CatapultMotion::Listener {
		/** Connect a moving rock or effect sprite to its machine stage. */
		Object(CatapultMachine &owner, Shell::Sprite *image, Stage state, bool isRock);
		/** Advance the object's machine stage after an authored animation marker. */
		bool animationPaused() override;
		/** Advance the object's motion before its sprite animation updates. */
		void spriteUpdating(Shell::Sprite &image, uint32 time) override;
		/** Report a moving object's collision to its machine. */
		void motionBoundary(CatapultMotion &motion, Common::Rect &position, CatapultMotion::Boundary boundary, uint32 time) override;
		/** Return the object's current game-pixel rectangle. */
		Common::Rect position() const;
		/** Catapult controller that handles this object's transfers and landings. */
		CatapultMachine &machine;
		/** Sprite displaying the moving object or effect. */
		Shell::Sprite *sprite;
		/** Current chute, catcher, flight or splash stage. */
		Stage stage;
		/** Landing area selected for the object's current flight. */
		Landing landing = kWater;
		/** Walking and bounce state applied to the sprite rectangle. */
		CatapultMotion motion;
		/** Whether this object represents a party rock rather than mud or an effect. */
		bool rock;
		/** Whether this object is held by the primary paddle latch. */
		bool paddleLatched = false;
	};
	/** Scene displaying the catapult mechanism and moving objects. */
	Shell::Scene &_scene;
	/** Selected machine families and transfer timing rules. */
	CatapultRules _rules;
	/** Machine sprites removed when the puzzle is cleared. */
	Common::Array<Shell::Sprite *> _owned;
	/** Drive and gear sprites advanced with the machine. */
	Common::Array<Shell::Sprite *> _gears;
	/** Moving rocks, mud and effect controllers tracked by the machine. */
	Common::Array<Object *> _objects;
	/** Objects currently queued in the supply chute. */
	Common::Array<Object *> _chute;
	/** Queued catapult sound resource IDs. */
	Common::Array<uint16> _sounds;
	/** Completed rock results waiting for polling. */
	Common::Array<Result> _results;
	/** Callback used for immediate rock-result delivery. */
	ResultListener *_resultListener = nullptr;
	/** Primary catcher sprites indexed by machine position. */
	Shell::Sprite *_primary[4] = {};
	/** Secondary catcher sprites indexed by machine position. */
	Shell::Sprite *_secondary[6] = {};
	/** Receiving paddle sprite whose frame selects the landing window. */
	Shell::Sprite *_paddle = nullptr;
	/** Sprite displaying machine control feedback. */
	Shell::Sprite *_indicator = nullptr;
	/** Sprite used to release a chute object. */
	Shell::Sprite *_trigger = nullptr;
	/** Sprite displaying mud impact feedback. */
	Shell::Sprite *_mudSplash = nullptr;
	/** Sprite used to change machine operation. */
	Shell::Sprite *_switch = nullptr;
	/** Rock controllers indexed by party member. */
	Object *_rocks[12] = {};
	/** Mud supply object waiting for release. */
	Object *_mud = nullptr;
	/** Controller used for a missed landing animation. */
	Object *_miss = nullptr;
	/** Mud object currently in flight. */
	Object *_mudFlight = nullptr;
	/** Rock object currently in flight. */
	Object *_rockFlight = nullptr;
	/** Water effect for a missed landing. */
	Object *_missSplash = nullptr;
	/** Water effect for the other landing path. */
	Object *_waterSplash = nullptr;
	/** Initial rock-supply position in game pixels. */
	Common::Point _rockOrigin;
	/** Initial mud-supply position in game pixels. */
	Common::Point _mudOrigin;
	/** Primary catcher slots currently carrying objects. */
	bool _occupied[4] = {};
	/** Whether the machine mechanism is enabled. */
	bool _running = true;
	/** Whether motion and animation advancement are paused. */
	bool _paused = true;
	/** Whether objects are advancing through the chute. */
	bool _chuteMoving = false;
	/** Whether the current miss animation has completed. */
	bool _missDone = false;
	/** Whether the secondary transfer animation has completed. */
	bool _secondaryDone = false;
	/** Whether the missed-landing water effect has completed. */
	bool _missSplashDone = false;
	/** Whether the other water effect has completed. */
	bool _waterSplashDone = false;
	/** Whether a required catapult resource could not be loaded. */
	bool _resError = false;
	/** Number of rocks still waiting for a landing result. */
	uint _unresolved = 0;
	/** Current machine timestamp in milliseconds. */
	uint32 _time = 0;
	/** Create a machine sprite and optionally register it as a drive gear. */
	Shell::Sprite *add(uint id, int z, bool gear = false);
	/** Create a moving object with its initial stage and rock classification. */
	Object *addObject(uint id, int z, Stage stage, bool rock);
	/** Remove an object controller and its sprite from machine tracking. */
	void remove(Object *object);
	/** Create the next mud object at its supply position. */
	void createMudSupply();
	/** Apply pause state to the machine's animations and motion controllers. */
	void applyMachinePause(bool paused);
	/** Move a chute object into an available primary catcher. */
	void transferPrimary(Object *object);
	/** Transfer an object from a primary catcher when the secondary window permits it. */
	bool transferSecondary(Object *object);
	/** Launch an object toward its selected successful or missed landing. */
	void startFlight(Object *object, bool missed);
	/** Handle the object's motion boundary and start its landing effects. */
	void land(Object &object, Common::Rect &position, CatapultMotion::Boundary boundary, uint32 time);
	/** Start the water effect corresponding to a successful or missed flight. */
	void showWaterSplash(bool missed);
	/** Record a rock's outcome and emit the matching progress or completion event. */
	void resolveRock(bool correct);
	/** Advance round completion after outstanding motion and effects finish. */
	void updateRound();
	/** Handle an object's completed animation stage and report whether it remains valid. */
	bool complete(Object &object);
	/** Remove round objects and reset catcher, flight and result state. */
	void clearRound();
	/** Prevent accidental copies of this stateful object. */
	CatapultMachine(const CatapultMachine &) = delete;
	/** Prevent accidental copies of this stateful object. */
	CatapultMachine &operator=(const CatapultMachine &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
