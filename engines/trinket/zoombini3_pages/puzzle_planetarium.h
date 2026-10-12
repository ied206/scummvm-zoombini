/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_ZOOMBINI3_PAGES_PUZZLE_PLANETARIUM_H
#define TRINKET_ZOOMBINI3_PAGES_PUZZLE_PLANETARIUM_H

#include "common/ptr.h"
#include "trinket/shell/hotspot.h"
#include "trinket/shell/random.h"
#include "trinket/shell/scene.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_world.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

/** Time exhibits and consumable denominations. */
class PlanetariumRules {
public:
	enum Denomination {
		kHour00 = 0,
		kDay01 = 1,
	};
	struct Piece {
		uint value, exhibit, position;
		Denomination type;
		bool consumed, enabled;
	};
	struct Result {
		uint exhibit;
		Denomination type;
		bool correct, hitTarget;
	};
	PlanetariumRules();
	bool generate(uint level, Shell::Random &random);
	uint level() const { return _level; }
	int hour(uint exhibit) const { return _hour[exhibit]; }
	int day(uint exhibit) const { return _day[exhibit]; }
	int targetHour() const { return _targetHour; }
	int targetDay() const { return _targetDay; }
	bool solved(uint exhibit, Denomination type) const;
	bool showMoon() const { return 1 < _level; }
	uint pieceCount() const { return _pieces.size(); }
	const Piece &piece(uint index) const { return _pieces[index]; }
	bool canUse(uint index) const;
	bool use(uint index);
	bool advancing(uint exhibit) const { return _remaining[exhibit] != 0; }
	Denomination advancingType(uint exhibit) const { return _mode[exhibit]; }
	uint remaining(uint exhibit) const { return _remaining[exhibit]; }
	bool hasTurns() const;

	/** Advance one whole hour/day; the presentation owns interpolation and timing. */
	bool step(uint exhibit, Common::Array<Result> &results);

	/** Return a subset of currently usable pieces reaching this target exactly. */
	bool findAnswer(uint exhibit, Denomination type, Common::Array<uint> &pieces) const;

	/** Plan both denominations, including midnight's effect on available bills. */
	bool findCompleteAnswer(Common::Array<uint> &pieces) const;

private:
	uint _level = 0;
	int _targetHour = 0, _targetDay = 0, _hour[2] = {}, _day[2] = {};
	uint _remaining[2] = {};
	Denomination _mode[2] = {
		kHour00,
		kHour00,
	};
	Common::Array<Piece> _pieces;
	void addPieces(uint exhibit, Denomination type, const uint *values, uint count);
	void disablePieces(uint exhibit, Denomination type);
	void checkTarget(uint exhibit, Denomination type, Common::Array<Result> &results);
	static int randomExcept(Shell::Random &random, uint bound, int excluded);
	static bool applyAnswer(PlanetariumRules &state, const Common::Array<uint> &pieces);
	static const int kMoonHiddenHours[28][2];
};

/** The two time exhibits and their consumable coins and bills. */
class PlanetariumMachine : public Core::ScriptableObject {
public:
	/** Page-local sound resource IDs used by the machine and page. */
	enum PageResourceId : int16 {
		kResSound14000_BGM = 14000,
		kResSound14010_HourMove = 14010,
		kResSound14011_HourPlace = 14011,
		kResSound14052_ExhibitA = 14052,
		kResSound14054_ExhibitB = 14054,
		kResSound14055_ExhibitC = 14055,
		kResSound14084_ExhibitD = 14084,
		kResSound14085_ExhibitE = 14085,
		kResSound14086_DayMove = 14086,
		kResSound14087_DayPlace = 14087,
		kResSound14203_BonusReward = 14203,
		kResSound14204_Reward = 14204,
		kResSoundCommon0001_Click = 1,
		kResSoundCommon0006_Help = 6,
	};
	PlanetariumMachine(Shell::Scene &scene, Shell::Random &random, int depth);
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	~PlanetariumMachine();
	bool load();
	void update(uint32 time);
	void event(const Common::Event &event, uint32 time);
	void pause(bool paused, uint32 time);
	void setEnabled(bool enabled) { _enabled = enabled; }
	void forceDrop();
	bool dragging() const { return 0 <= _dragged; }
	int hitPiece(Common::Point point) const;
	Common::Rect pieceBounds(uint index) const;
	Common::Rect slotBounds(uint index) const;
	Common::String debugAnswer(uint level) const;
	const PlanetariumRules &rules() const { return _rules; }
	bool pollSound(uint16 &sound);
	bool hasResourceError() const { return !_valid; }

private:
	static constexpr const char *kAnimation = "/z3a3/%u.ao";
	static constexpr const char *kCDAnimation = "/z3a3cd/%u.ao";
	struct Cover {
		Shell::Sprite *opening = nullptr, *closing = nullptr;
		uint state = 3;
		bool closeAfterOpening = false;
	};
	Shell::Scene &_scene;
	PlanetariumRules _rules;
	Shell::Random &_random;
	int _depth;
	PlanetariumRules::Result _lastResult = {};
	Core::ScriptValue _grabCursor, _spotCursor;
	bool _debugMode = false, _displayText = false;
	Common::Array<Shell::Sprite *> _owned, _pieces, _pausedSprites;
	Common::Array<uint16> _sounds;
	Shell::Sprite *_earth = nullptr, *_earthMoon = nullptr, *_sky = nullptr, *_sun = nullptr;
	Shell::Sprite *_moon[28] = {}, *_skyFront[2] = {}, *_slots[4] = {};
	Cover _covers[4] = {};
	uint32 _stepTime[2] = {}, _time = 0, _pauseTime = 0;
	bool _waiting[2] = {}, _enabled = true, _paused = false, _valid = true;
	int _dragged = -1;
	Common::Point _grabOffset, _oldPosition;
	void clearPuzzle();
	Shell::Sprite *add(uint id, int depth, bool cd = false, int frame = 1);
	void updateExhibit(uint exhibit, float fraction);
	void updateCovers();
	void openCover(uint index, bool closeAfter);
	void closeCover(uint index);
	void updateSlot();
	void drop();
	void beginStep(uint exhibit, uint32 time);
	uint stepDuration(uint exhibit) const;
	static void centerAt(Shell::Sprite *sprite, Common::Point position);
	static Common::Point celestialPosition(float angle);
	static float hourAngle(float hour);
	static const int kMoonAngles[28];
	static const Common::Point kHourPositions[2][9], kDayPositions[2][9];
};

/** Native incubator presentation with independently configured row populations. */
class Incubator : public Core::Trinket {
public:
	Incubator(TrinketEngine &vm, Shell::Scene &scene, int depth, uint rows);
	~Incubator() override;
	bool load();
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	void update(uint32 time);
	void pause(bool paused, uint32 time);
	bool hasResourceError() const { return !_valid; }

private:
	static constexpr const char *kAnimation = "/z3a3/%u.ao";
	struct Pupa {
		Shell::Sprite *sprite = nullptr;
		Common::Point start, destination;
		uint state = 0;
		uint32 started = 0;
		float phase = 0, amplitude = 0, frequency = 0, distance = 0;
	};
	struct Batch {
		uint row, first, end;
		uint32 time;
	};
	TrinketEngine &_vm;
	Shell::Scene &_scene;
	uint _rows;
	Shell::Sprite *_doors[6] = {};
	Pupa _pupae[6][4] = {};
	uint _population[6] = {}, _transformed[6] = {}, _doorState[6] = {};
	Common::Array<Batch> _batches;
	Common::Array<Shell::Sprite *> _owned, _pausedSprites;
	uint32 _time = 0, _pauseTime = 0;
	bool _paused = false, _valid = true;
	Shell::Sprite *add(uint id, int depth, int frame = 1);
	void remove(Shell::Sprite *sprite);
	bool setPopulation(uint row, uint count);
	void transform(uint row, uint count);
	static const Common::Point kPupaPositions[2][6][4];
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
