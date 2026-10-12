/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_ZOOMBINI3_PAGES_PUZZLE_GREENHOUSE_H
#define TRINKET_ZOOMBINI3_PAGES_PUZZLE_GREENHOUSE_H

#include "common/ptr.h"
#include "trinket/ompp/xml.h"
#include "trinket/shell/hotspot.h"
#include "trinket/shell/random.h"
#include "trinket/shell/scene.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_world.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

/** Plant attributes and the independent moth and beetle traversals. */
class GreenhouseRules {
public:
	enum Direction {
		kNone00 = 0,
		kUp01 = 1,
		kDown02 = 2,
		kRight03 = 3,
		kLeft04 = 4,
	};
	enum MothState {
		kSupply00 = 0,
		kCrossing01 = 1,
		kPassed02 = 2,
	};
	struct Moth {
		uint trait = 0, animation = 0;
		int column = 0, row = 0;
		Direction direction = kRight03;
		MothState state = kSupply00;
		bool stuck = false;
		uint32 visits[144] = {};
		uint16 counts[144] = {};
		Common::Point supply;
	};
	struct Swap {
		uint first, second;
	};
	GreenhouseRules();
	bool generate(uint level, const OM::XMLParser &templates, Shell::Random &random, Common::Point mothSize);
	uint level() const { return _level; }
	uint cell(uint index) const { return _cells[index]; }
	uint remainingSwaps() const { return _swaps; }
	uint passed() const { return _passed; }
	const Moth &moth(uint index) const { return _moths[index]; }
	int beetleEntry(uint index) const { return _beetleEntries[index]; }
	uint beetleFlow(uint cell) const { return _flow[cell]; }
	bool occupied(uint index) const { return _mothOccupancy[index] || _beetleOccupancy[index]; }
	bool matches(uint cell, uint trait) const;
	bool canEnter(uint moth, int row) const;
	bool enter(uint moth, int row);

	/** Turn first; a later call moves along that heading. */
	Direction nextDirection(uint moth, int &column, int &row) const;
	bool advance(uint moth, uint32 tick);

	/** No supply remains, no swaps remain, and every crossing moth is stuck. */
	bool allMothsStuck() const;
	bool swapPlants(uint first, uint second);
	void setBeetleOccupied(uint cell, bool value) { _beetleOccupancy[cell] = value; }
	bool mothOccupied(uint cell) const { return _mothOccupancy[cell]; }
	bool beetleOccupied(uint cell) const { return _beetleOccupancy[cell]; }
	bool findPath(uint moth, Common::Array<uint> &path) const;

	/** Recover the authored complete board, preserving the remaining swap budget. */
	bool findRestoration(Common::Array<Swap> &swaps) const;
	static Common::Point cellOrigin(uint column, uint row) { return Common::Point(165 + 44 * column, 90 + 42 * row); }

private:
	struct FlowNode {
		int down, left, right;
		uint children, distance;
	};
	uint _level = 0, _swaps = 30, _passed = 0;
	uint _cells[144] = {}, _original[144] = {}, _flow[144] = {};
	bool _mothOccupancy[144] = {}, _beetleOccupancy[144] = {};
	int _beetleEntries[3] = {
		-1,
		-1,
		-1,
	};
	Moth _moths[12] = {};
	Common::Array<Swap> _scramble;
	bool loadTemplate(const OM::XMLParser &templates, uint family, uint attribute, int orientation, Shell::Random &random);
	bool traversable(uint trait, int column, int row) const;
	void logVisit(Moth &moth, uint32 tick);
	void calculateFlow();
	int buildFlow(uint trait, int column, int row, uint *types, FlowNode *nodes, uint &depth, uint &maximum);
	bool traverseFlow(int index, uint back, FlowNode *nodes, uint &depth, uint maximum);
	static void shuffle(uint *values, uint count, uint first, Shell::Random &random);
};

/** Native plant traversal, moths, beetles and wand gestures. */
class GreenhouseMachine : public Core::ScriptableObject {
public:
	GreenhouseMachine(TrinketEngine &vm, Shell::Scene &scene, uint level);
	~GreenhouseMachine() override;
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	void event(const Common::Event &event);
	void update(uint32 time);
	void pause(bool paused, uint32 time);
	bool overDraggable(Common::Point point) const;
	bool isDragging() const { return 0 <= _dragged || _wandHeld; }
	bool hasResourceError() const { return !_valid; }
	bool pollSound(uint16 &sound);
	Common::String debugAnswer() const;
	const GreenhouseRules &rules() const { return _rules; }
	Common::Rect mothBounds(uint index) const { return bounds(_moths[index].body); }

private:
	/** Activity-local sound resource IDs. */
	enum PageResourceId : int16 {
		kResSound16000_BGM = 16000,
		kResSound16050_WandUse = 16050,
		kResSound16052_WandPickup = 16052,
		kResSound16235_WandCancel = 16235,
		kResSound16236_MothStuck = 16236,
		kResSound16237_SwapRejected = 16237,
		kResSound16238_SwapAccepted = 16238,
		kResSound16239_MothFree = 16239,
		kResSound16240_SwapComplete = 16240,
		kResSound16241_NoSwaps = 16241,
	};
	static constexpr const char *kAnimation = "/z3a4/%u.ao";
	static constexpr const char *kTemplate = "z3a4data.xml";
	struct Moving {
		Shell::Sprite *body = nullptr, *trait = nullptr;
		Common::Point from, to;
		uint32 started = 0, duration = 0;
		uint direction = 3, destination = 0;
		bool active = false, flying = false, pollinating = false, done = false;
	};
	struct Beetle {
		Moving motion;
		int column, row;
		uint trait, direction;
		uint32 deadline;
		bool exiting;
	};
	TrinketEngine &_vm;
	Shell::Scene &_scene;
	GreenhouseRules _rules;
	Common::Array<Shell::Sprite *> _owned;
	Common::Array<uint16> _sounds;
	Shell::Sprite *_flowers[144] = {}, *_leaves[144] = {}, *_pots[12] = {};
	Shell::Sprite *_wand = nullptr, *_highlight = nullptr;
	Moving _moths[12] = {};
	Common::Array<Beetle> _beetles;
	int _dragged = -1, _selected = -1, _swapSecond = -1;
	Common::Point _dragOffset, _mouse;
	uint _level;
	uint32 _seed = 0, _time = 0, _lastTime = 0, _gameTime = 0, _tick = 0, _nextBeetle = 10000, _swapStarted = 0;
	bool _paused = false, _wandHeld = false, _running = false, _valid = true;
	bool start(uint32 seed);
	void clearSprites();
	Shell::Sprite *add(uint id, int depth, int frame = 1);
	void remove(Shell::Sprite *sprite);
	void rebuildPlant(uint index);
	void updateMoths();
	void updateBeetles();
	void updateMotion(Moving &motion, bool beetle);
	void positionTrait(Moving &motion);
	void beginMotion(Moving &motion, Common::Point target, uint duration, uint direction);
	void cancelWand();
	void selectPlant(uint cell);
	void finishMoth(uint index);
	void releaseMoth();
	void playSound(uint id) { _sounds.push_back(id); }
	int hitMoth(Common::Point point) const;
	static int hitCell(Common::Point point);
	static Common::Rect bounds(const Shell::Sprite *sprite);
	static uint traitResource(uint index);
	static const uint kFlowers[36];
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
