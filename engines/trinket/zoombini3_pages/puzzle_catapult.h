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
#include "trinket/shell/scene.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_world.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

/** Dataset configuration and frame predicates for the Catapult machine. */
class CatapultRules {
public:
	static constexpr uint kDatasetCount = 13;
	static constexpr uint kPartyLimit = 12;

	CatapultRules();
	bool configure(uint level, uint partySize, uint dataset, int overrideDataset = -1);
	uint level() const { return _level; }
	uint partySize() const { return _partySize; }
	uint dataset() const { return _dataset; }
	uint primaryFamily() const { return _primaryFamily; }
	uint paddleFamily() const { return _paddleFamily; }
	uint driveFamily() const { return _driveFamily; }
	uint primaryCount() const { return _primaryCount; }
	uint primaryPhaseDivisor() const { return _primaryPhaseDivisor; }
	uint primaryStartFrame(uint index, uint frameCount) const;
	uint secondaryStartFrame(uint index, uint frameCount) const;
	bool releasesPrimaryLatch(uint frame) const;
	bool canReceiveFromChute(uint frame) const;
	bool canReceiveFromPrimary(uint frame) const;
	uint primaryTransportFrame() const;
	int secondaryTransportFrame(uint catcherFrame) const;
	bool paddleIsOpen(uint frame, uint frameCount) const;
	int paddleHeight() const;
	int flightVelocity() const;
	static bool chuteCollision(const Common::Rect &first, const Common::Rect &second);

	/** Resolve one script draw; -1 requests another draw with the same history. */
	static int datasetForDraw(uint draw, const bool *seen);

private:
	uint _level = 1, _partySize = 0, _dataset = 0;
	uint _primaryFamily = 0, _paddleFamily = 0, _driveFamily = 0;
	uint _primaryCount = 2, _primaryPhaseDivisor = 2;
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
		virtual ~Listener() {}
		/** May reconfigure motion and position; defer destruction until update returns. */
		virtual void motionBoundary(CatapultMotion &motion, Common::Rect &position, Boundary boundary, uint32 time) = 0;
	};
	CatapultMotion();
	void setBounds(const Common::Rect &bounds) { _bounds = bounds; }
	void startWalk(int xVelocity, int yVelocity);
	void startBounce(int velocity, int acceleration, int decay, const Common::Rect &position, uint32 time);
	void pause(uint32 time);
	void resume(uint32 time);
	void update(Common::Rect &position, uint32 time, Listener &listener);
	int walkXVelocity() const { return _walkX; }
	bool isPaused() const { return _paused; }

private:
	Common::Rect _bounds = Common::Rect(800, 600);
	uint32 _lastUpdate = 0;
	double _pauseTime = 0, _bounceTime = 0;
	double _samples[10] = {}, _remainderX = 0, _remainderY = 0;
	double _bounceOriginY = 0, _previousBounceY = 0;
	int _walkX = 60, _walkY = 0, _bounceVelocity = 300, _currentBounceVelocity = 0, _acceleration = 400, _decay = 0;
	bool _walking = false, _bouncing = false, _paused = false;
};

/** Native MultiTrinket round state; MPS owns party, dialogue and progress. */
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
		bool correct, finished;
	};
	class ResultListener {
	public:
		virtual ~ResultListener() {}
		/** Called during the mechanical update; must not destroy the machine. */
		virtual void catapultResult(const Result &result, uint32 time) = 0;
	};
	CatapultMachine(Shell::Scene &scene, const CatapultRules &rules);
	explicit CatapultMachine(Shell::Scene &scene);
	~CatapultMachine();
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	const CatapultRules &rules() const { return _rules; }
	bool load();
	void action(int action);
	void update(uint32 time);
	void pause(uint32 time);
	void resume(uint32 time);
	bool pollSound(uint16 &sound);
	bool pollResult(Result &result);
	void setResultListener(ResultListener *listener) { _resultListener = listener; }
	bool chuteMoving() const { return _chuteMoving; }
	bool machineRunning() const { return _running && !_paused; }
	uint unresolvedRocks() const { return _unresolved; }
	Common::String debugAnswer() const;
	Common::String debugChance() const;
	bool hasResourceError() const { return _resourceError; }

private:
	static constexpr const char *kAnimation = "/z3a1/%u.ao";
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
		Object(CatapultMachine &owner, Shell::Sprite *image, Stage state, bool isRock);
		bool animationPaused() override;
		void spriteUpdating(Shell::Sprite &image, uint32 time) override;
		void motionBoundary(CatapultMotion &motion, Common::Rect &position, CatapultMotion::Boundary boundary, uint32 time) override;
		Common::Rect position() const;
		CatapultMachine &machine;
		Shell::Sprite *sprite;
		Stage stage;
		Landing landing = kWater;
		CatapultMotion motion;
		bool rock, paddleLatched = false;
	};
	Shell::Scene &_scene;
	CatapultRules _rules;
	Common::Array<Shell::Sprite *> _owned, _gears;
	Common::Array<Object *> _objects, _chute;
	Common::Array<uint16> _sounds;
	Common::Array<Result> _results;
	ResultListener *_resultListener = nullptr;
	Shell::Sprite *_primary[4] = {}, *_secondary[6] = {}, *_paddle = nullptr, *_indicator = nullptr, *_trigger = nullptr,
				  *_mudSplash = nullptr, *_switch = nullptr;
	Object *_rocks[12] = {}, *_mud = nullptr, *_miss = nullptr, *_mudFlight = nullptr, *_rockFlight = nullptr,
		   *_missSplash = nullptr, *_waterSplash = nullptr;
	Common::Point _rockOrigin, _mudOrigin;
	bool _occupied[4] = {}, _running = true, _paused = true, _chuteMoving = false, _missDone = false, _secondaryDone = false,
		 _missSplashDone = false, _waterSplashDone = false, _resourceError = false;
	uint _unresolved = 0;
	uint32 _time = 0;
	Shell::Sprite *add(uint id, int z, bool gear = false);
	Object *addObject(uint id, int z, Stage stage, bool rock);
	void remove(Object *object);
	void createMudSupply();
	void applyMachinePause(bool paused);
	void transferPrimary(Object *object);
	bool transferSecondary(Object *object);
	void startFlight(Object *object, bool missed);
	void land(Object &object, Common::Rect &position, CatapultMotion::Boundary boundary, uint32 time);
	void showWaterSplash(bool missed);
	void resolveRock(bool correct);
	void updateRound();
	bool complete(Object &object);
	void clearRound();
	CatapultMachine(const CatapultMachine &) = delete;
	CatapultMachine &operator=(const CatapultMachine &) = delete;
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
