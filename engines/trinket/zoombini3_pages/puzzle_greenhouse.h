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
#include "trinket/frame_clock.h"
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
		/** Flower trait that this moth can pollinate. */
		uint trait = 0;
		/** Animation variant selected for this moth. */
		uint animation = 0;
		/** Current greenhouse grid column. */
		int column = 0;
		/** Current greenhouse grid row. */
		int row = 0;
		/** Direction used when selecting the moth's next cell. */
		Direction direction = kRight03;
		/** Supply, traversal or completion state of the moth. */
		MothState state = kSupply00;
		/** Whether repeated visits have left the moth without a useful route. */
		bool stuck = false;
		/** Most recent eligible update sequence at which each cell was visited. */
		uint32 visits[144] = {};
		/** Visit counts used to detect repeated traversal of a cell. */
		uint16 counts[144] = {};
		/** Initial game-pixel position in the moth supply. */
		Common::Point supply;
	};
	struct Swap {
		/** First plant cell exchanged by this scramble step. */
		uint first;
		/** Second plant cell exchanged by this scramble step. */
		uint second;
	};
	/** Initialize an empty greenhouse grid and moth population. */
	GreenhouseRules();
	/** Load and scramble a difficulty-specific plant template and arrange the moth supply. */
	bool generate(uint level, const OM::XMLParser &templates, Shell::Random &random, Common::Point mothSize);
	/** Return the generated greenhouse difficulty. */
	uint level() const { return _level; }
	/** Return the plant type stored at a grid cell. */
	uint cell(uint index) const { return _cells[index]; }
	/** Return the number of plant exchanges still available. */
	uint remainingSwaps() const { return _swaps; }
	/** Return the number of moths that have completed their route. */
	uint passed() const { return _passed; }
	/** Return a moth's trait, position and traversal history. */
	const Moth &moth(uint index) const { return _moths[index]; }
	/** Return the entry column assigned to a beetle type. */
	int beetleEntry(uint index) const { return _beetleEntries[index]; }
	/** Return the allowed beetle movement mask at a grid cell. */
	uint beetleFlow(uint cell) const { return _flow[cell]; }
	/** Return whether a moth or beetle currently occupies the grid cell. */
	bool occupied(uint index) const { return _mothOccupancy[index] || _beetleOccupancy[index]; }
	/** Check whether the plant at a cell matches the supplied moth trait. */
	bool matches(uint cell, uint trait) const;
	/** Check whether a supply moth can enter the requested greenhouse row. */
	bool canEnter(uint moth, int row) const;
	/** Move a supply moth into the requested greenhouse row when it is available. */
	bool enter(uint moth, int row);

	/** Turn first; a later call moves along that heading. */
	Direction nextDirection(uint moth, int &column, int &row) const;
	/** Advance a moth by one simulation step and record its cell visits. */
	bool advance(uint moth, uint32 step);

	/** No supply remains, no swaps remain, and every crossing moth is stuck. */
	bool allMothsStuck() const;
	/** Exchange two valid plant cells and consume a remaining swap. */
	bool swapPlants(uint first, uint second);
	/** Mark or clear beetle occupancy for a grid cell. */
	void setBeetleOccupied(uint cell, bool value) { _beetleOccupancy[cell] = value; }
	/** Return whether a moth currently occupies the grid cell. */
	bool mothOccupied(uint cell) const { return _mothOccupancy[cell]; }
	/** Return whether a beetle currently occupies the grid cell. */
	bool beetleOccupied(uint cell) const { return _beetleOccupancy[cell]; }
	/** Find a traversable cell route for the selected moth. */
	bool findPath(uint moth, Common::Array<uint> &path) const;

	/** Recover the authored complete board, preserving the remaining swap budget. */
	bool findRestoration(Common::Array<Swap> &swaps) const;
	/** Convert a greenhouse column and row to a game-pixel cell origin. */
	static Common::Point cellOrigin(uint column, uint row) { return Common::Point(165 + 44 * column, 90 + 42 * row); }

private:
	/** Format for script names or values: Z3A4Data/Template%u. */
	static constexpr const char *kZ3A4DataTemplateUnsignedIndexFormat = "Z3A4Data/Template%u";
	/** Script identifier for num. */
	static constexpr const char *kNumName = "Num";
	/** Format for script names or values: /T%u. */
	static constexpr const char *kTUnsignedIndexFormat = "/T%u";
	/** Script identifier for bE. */
	static constexpr const char *kBEName = "BE";
	/** Format for script names or values: line%u. */
	static constexpr const char *kLineUnsignedIndexFormat = "line%u";

	struct FlowNode {
		/** Index of the reachable cell below, or a negative marker when blocked. */
		int down;
		/** Index of the reachable cell to the left, or a negative marker when blocked. */
		int left;
		/** Index of the reachable cell to the right, or a negative marker when blocked. */
		int right;
		/** Number of reachable neighboring flow nodes. */
		uint children;
		/** Distance used to select a route through the flow graph. */
		uint distance;
	};
	/** Difficulty used to select the plant template and movement rules. */
	uint _level = 0;
	/** Number of plant exchanges still available. */
	uint _swaps = 30;
	/** Number of moths that have completed the greenhouse route. */
	uint _passed = 0;
	/** Current plant types indexed by grid cell. */
	uint _cells[144] = {};
	/** Plant arrangement before the generated scramble exchanges. */
	uint _original[144] = {};
	/** Allowed beetle movement masks indexed by grid cell. */
	uint _flow[144] = {};
	/** Cells currently occupied by moths. */
	bool _mothOccupancy[144] = {};
	/** Cells currently occupied by beetles. */
	bool _beetleOccupancy[144] = {};
	/** Entry columns selected for the three beetle types. */
	int _beetleEntries[3] = {
		-1,
		-1,
		-1,
	};
	/** Traits, supply positions and traversal state of the moth population. */
	Moth _moths[12] = {};
	/** Plant exchanges used to scramble the generated template. */
	Common::Array<Swap> _scramble;
	/** Select a template family, trait and orientation and load its plant grid. */
	bool loadTemplate(const OM::XMLParser &templates, uint family, uint attribute, int orientation, Shell::Random &random);
	/** Check whether a grid position can be entered by the supplied moth trait. */
	bool traversable(uint trait, int column, int row) const;
	/** Update a moth's per-cell visit time and repetition count. */
	void logVisit(Moth &moth, uint32 step);
	/** Rebuild beetle route masks for the current plant arrangement. */
	void calculateFlow();
	/** Build reachable downward and horizontal neighbors for one trait's flow graph. */
	int buildFlow(uint trait, int column, int row, uint *types, FlowNode *nodes, uint &depth, uint &maximum);
	/** Trace the flow graph to select directions that lead through the greenhouse. */
	bool traverseFlow(int index, uint back, FlowNode *nodes, uint &depth, uint maximum);
	/** Shuffle the selected suffix of a value array using the shared random stream. */
	static void shuffle(uint *values, uint count, uint first, Shell::Random &random);
};

/** Native plant traversal, moths, beetles and wand gestures. */
class GreenhouseMachine : public Core::ScriptableObject {
public:
	/** Configure greenhouse resources and simulation for a difficulty. */
	GreenhouseMachine(TrinketEngine &vm, Shell::Scene &scene, uint level);
	/** Remove greenhouse sprites and stop pending visual feedback. */
	~GreenhouseMachine() override;
	/** Read greenhouse progress, swap counts and inherited properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign supported greenhouse and inherited object properties. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch greenhouse generation, moth release and control script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Handle moth dragging and the plant-exchange wand. */
	void event(const Common::Event &event);
	/** Advance moths, beetles and exchange feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Pause or resume greenhouse timing and sprite animations. */
	void pause(bool paused, uint32 time);
	/** Check whether a game-pixel point selects a moth or the exchange wand. */
	bool overDraggable(Common::Point point) const;
	/** Return whether a moth or the exchange wand is being dragged. */
	bool isDragging() const { return 0 <= _dragged || _wandHeld; }
	/** Return whether a required greenhouse resource or setup failed. */
	bool hasResourceError() const { return !_valid; }
	/** Consume the next queued greenhouse sound ID. */
	bool pollSound(uint16 &sound);
	/** Describe plant exchanges and moth routes for debugger answer output. */
	Common::String debugAnswer() const;
	/** Return the plant arrangement and logical moth traversal state. */
	const GreenhouseRules &rules() const { return _rules; }
	/** Return a moth body's current game-pixel bounds. */
	Common::Rect mothBounds(uint index) const { return bounds(_moths[index].body); }

private:
	/** Script event name for moth Done. */
	static constexpr const char *kEventMothDone = "mothDone";
	/** Script event name for moth Stuck. */
	static constexpr const char *kEventMothStuck = "mothStuck";
	/** Script property name for seed. */
	static constexpr const char *kPropertySeed = "seed";
	/** Script property name for wand. */
	static constexpr const char *kPropertyWand = "wand";
	/** Script method name for start. */
	static constexpr const char *kMethodStart = "start";
	/** Script method name for stop. */
	static constexpr const char *kMethodStop = "stop";
	/** Script method name for pause. */
	static constexpr const char *kMethodPause = "pause";
	/** Script method name for resume. */
	static constexpr const char *kMethodResume = "resume";
	/**
	 * Map a zero-based flower color to its matching flower and moth display conversion.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 */
	static OM::Canvas::ColorRemap flowerColor(uint color);
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
	/** Animation path pattern for greenhouse resource IDs. */
	static constexpr const char *kAnimation = "/z3a4/%u.ao";
	/** XML resource containing greenhouse plant templates. */
	static constexpr const char *kTemplate = "z3a4data.xml";
	struct Moving {
		/** Sprite displaying the moving moth or beetle. */
		Shell::Sprite *body = nullptr;
		/** Trait overlay sprite attached to the moving body. */
		Shell::Sprite *trait = nullptr;
		/** Starting position of the current motion in game pixels. */
		Common::Point from;
		/** Destination position of the current motion in game pixels. */
		Common::Point to;
		/** Game-time millisecond timestamp when the current motion began. */
		uint32 started = 0;
		/** Duration of the current motion in milliseconds. */
		uint32 duration = 0;
		/** Animation direction selected for the current motion. */
		uint direction = 3;
		/** Grid cell reached when the current motion finishes. */
		uint destination = 0;
		/** Whether the body currently has an active movement or animation. */
		bool active = false;
		/** Whether the moth is using its flying motion. */
		bool flying = false;
		/** Whether the moth is showing a flower-pollination animation. */
		bool pollinating = false;
		/** Whether this moth has finished its greenhouse route. */
		bool done = false;
	};
	struct Beetle {
		/** Sprites and interpolation state for beetle movement. */
		Moving motion;
		/** Current beetle grid column. */
		int column;
		/** Current beetle grid row. */
		int row;
		/** Plant trait used to choose the beetle's route. */
		uint trait;
		/** Direction selected for the next movement. */
		uint direction;
		/** Game-time millisecond deadline for the next beetle step. */
		uint32 deadline;
		/** Whether the beetle is leaving the greenhouse grid. */
		bool exiting;
	};
	/** Engine supplying greenhouse resources and the shared random stream. */
	TrinketEngine &_vm;
	/** Scene displaying greenhouse plants, moths, beetles and controls. */
	Shell::Scene &_scene;
	/** Plant arrangement, remaining swaps and moth traversal state. */
	GreenhouseRules _rules;
	/** Sprites removed when the greenhouse is rebuilt or destroyed. */
	Common::Array<Shell::Sprite *> _owned;
	/** Queued greenhouse sound resource IDs. */
	Common::Array<uint16> _sounds;
	/** Flower sprites indexed by grid cell. */
	Shell::Sprite *_flowers[144] = {};
	/** Plant foliage sprites indexed by grid cell. */
	Shell::Sprite *_leaves[144] = {};
	/** Pot sprites along the greenhouse supply edge. */
	Shell::Sprite *_pots[12] = {};
	/** Sprite used to exchange two plants. */
	Shell::Sprite *_wand = nullptr;
	/** Sprite marking the selected exchange cell. */
	Shell::Sprite *_highlight = nullptr;
	/** Visual motion state indexed by moth. */
	Moving _moths[12] = {};
	/** Active beetles with their grid and visual motion state. */
	Common::Array<Beetle> _beetles;
	/** Index of the dragged moth, or minus one when none is held. */
	int _dragged = -1;
	/** First plant cell selected for an exchange, or minus one when unset. */
	int _selected = -1;
	/** Second plant cell while an exchange animation is pending. */
	int _swapSecond = -1;
	/** Pointer displacement from the dragged object in pixels. */
	Common::Point _dragOffset;
	/** Most recent pointer position in game pixels. */
	Common::Point _mouse;
	/** Difficulty requested for greenhouse generation. */
	uint _level;
	/** Seed retained for the current generated greenhouse. */
	uint32 _seed = 0;
	/** Most recent host timestamp in milliseconds. */
	uint32 _time = 0;
	/** Active movement milliseconds, excluding pause and plant-selection intervals. */
	FrameClock _clock;
	/** Eligible update sequence used to order cell visits, independent of elapsed time. */
	uint32 _step = 0;
	/** Game-time millisecond deadline for the next beetle arrival. */
	uint32 _nextBeetle = 10000;
	/** Host millisecond timestamp when the plant exchange began. */
	uint32 _swapStarted = 0;
	/** Whether greenhouse simulation and interaction are paused. */
	bool _paused = false;
	/** Whether the player is holding the plant-exchange wand. */
	bool _wandHeld = false;
	/** Whether the generated greenhouse simulation is active. */
	bool _running = false;
	/** Whether required resources and generated state are valid. */
	bool _valid = true;
	/** Generate a greenhouse from a seed and build its sprites. */
	bool start(uint32 seed);
	/** Remove all greenhouse sprites and clear visual motion state. */
	void clearSprites();
	/** Create a greenhouse sprite at a depth and one-based frame. */
	Shell::Sprite *add(uint id, int depth, int frame = 1);
	/** Remove one greenhouse sprite from tracking and the scene. */
	void remove(Shell::Sprite *sprite);
	/** Refresh flower and foliage sprites for a changed grid cell. */
	void rebuildPlant(uint index);
	/** Advance moth motion, pollination and logical cell traversal. */
	void updateMoths();
	/** Spawn due beetles and advance their routes through the grid. */
	void updateBeetles();
	/** Interpolate a moth or beetle body and select its motion animation. */
	void updateMotion(Moving &motion, bool beetle);
	/** Align the trait overlay with its moving body sprite. */
	void positionTrait(Moving &motion);
	/** Start a timed move to a game-pixel destination with the requested direction. */
	void beginMotion(Moving &motion, Common::Point target, uint duration, uint direction);
	/** Return the exchange wand and clear its selected plant cells. */
	void cancelWand();
	/** Select the first exchange cell or begin an exchange with the second. */
	void selectPlant(uint cell);
	/** Complete a moth's route and update its final visual state. */
	void finishMoth(uint index);
	/** Place the dragged supply moth into a valid greenhouse entry. */
	void releaseMoth();
	/** Queue a greenhouse sound resource ID. */
	void playSound(uint id) { _sounds.push_back(id); }
	/** Return the moth at a game-pixel point, or minus one if none is hit. */
	int hitMoth(Common::Point point) const;
	/** Convert a game-pixel point to a greenhouse cell, or return an invalid marker. */
	static int hitCell(Common::Point point);
	/** Return a sprite's game-pixel rectangle, or an empty rectangle for null. */
	static Common::Rect bounds(const Shell::Sprite *sprite);
	/** Map a moth trait index to its animation resource ID. */
	static uint traitResource(uint index);
	/** Flower animation resource IDs indexed by plant type. */
	static const uint kFlowers[36];
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
