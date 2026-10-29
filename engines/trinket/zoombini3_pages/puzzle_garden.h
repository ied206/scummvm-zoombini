/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_ZOOMBINI3_PAGES_PUZZLE_GARDEN_H
#define TRINKET_ZOOMBINI3_PAGES_PUZZLE_GARDEN_H

#include "common/ptr.h"
#include "trinket/bezier.h"
#include "trinket/frame_clock.h"
#include "trinket/shell/random.h"
#include "trinket/shell/scene.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_world.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

/** The first accepted plants establish the trait assigned to each axis. */
class GardenRules {
public:
	enum Attribute {
		kFlower00 = 0,
		kStem01 = 1,
		kLeaf02 = 2,
		kRoot03 = 3,
	};
	struct Plant {
		/** Four generated plant trait values used by the placement rules. */
		uint trait[4];
		/** Accepted planting-hole index, or a negative marker while unplanted. */
		int hole;
	};
	struct Placement {
		/** Plant index selected by this answer step. */
		uint plant;
		/** Compatible planting hole selected by this answer step. */
		uint hole;
	};
	/** Initialize an empty planting layout and chance allowance. */
	GardenRules();
	/** Choose difficulty-specific trait axes, hole constraints and supply plants. */
	bool generate(uint level, uint party, Shell::Random &random);
	/** Return the generated garden difficulty. */
	uint level() const { return _level; }
	/** Return the number of supply plants generated for the party. */
	uint party() const { return _party; }
	/** Return the number of plants accepted into holes. */
	uint passed() const { return _passed; }
	/** Return the number of incorrect placements still available. */
	uint chances() const { return _chances; }
	/** Return the number of planting holes active at the selected difficulty. */
	uint holeCount() const {
		if (_level == 1)
			return 4;
		if (_level == 2)
			return 16;
		return 64;
	}
	/** Return the plant-trait axis assigned to a hole-layout dimension. */
	uint axis(uint index) const { return _axes[index]; }
	/** Return a supply plant's traits and accepted planting position. */
	const Plant &plant(uint index) const { return _plants[index]; }
	/** Return the number of plants accepted into a hole. */
	uint population(uint hole) const { return _population[hole]; }
	/** Return the packed trait mask accepted by a hole. */
	uint16 allowed(uint hole) const { return _allowed[hole]; }
	/** Check whether a plant satisfies a planting hole's trait constraints. */
	bool accepts(uint plant, uint hole) const;
	/** Apply a plant placement and update progress or the chance allowance. */
	bool place(uint plant, uint hole);

	/** Called after the rejected plant has returned to its source. */
	void reject();
	/** Replace the remaining incorrect-placement allowance. */
	void setChances(uint count) { _chances = count; }
	/** Find compatible holes for the remaining unplanted supply. */
	bool findAnswer(Common::Array<Placement> &plan) const;
	/** Map a difficulty-specific planting-hole index to its game-pixel position. */
	static Common::Point holePosition(uint level, uint hole);

private:
	/** Difficulty controlling the number of planting constraints and holes. */
	uint _level = 1;
	/** Number of supply plants generated for the party. */
	uint _party = 0;
	/** Number of successfully planted supply plants. */
	uint _passed = 0;
	/** Remaining incorrect-placement allowance. */
	uint _chances = 0;
	/** Plant-trait axes assigned to the layout dimensions. */
	uint _axes[3] = {};
	/** Trait bits excluded from placement checks at this difficulty. */
	uint16 _ignored = 0;
	/** Packed accepted trait masks indexed by planting hole. */
	uint16 _allowed[64] = {};
	/** Accepted plant counts indexed by planting hole. */
	uint _population[64] = {};
	/** Supply plant traits and their current accepted-hole assignments. */
	Plant _plants[12] = {};
	/** Pack a plant's active traits into the mask used by hole checks. */
	uint16 value(uint plant) const;
};

/** Native plant containers, drag gestures, return curve and sun counter. */
class GardenMachine : public Shell::MultiTrinket {
public:
	/** Connect garden generation and plant interaction to the engine and scene. */
	GardenMachine(TrinketEngine &vm, Shell::Scene &scene);
	/** Remove the plant, hole and feedback sprites. */
	~GardenMachine() override;
	/** Read planting progress, remaining chances and inherited properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Change supported chance, interaction and inherited properties. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch garden generation and puzzle-control script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Handle plant pickup, dragging and release. */
	void event(const Common::Event &event);
	/** Advance rejected-plant return motion and feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Pause or resume garden interaction and return-motion timing. */
	void pause(bool paused, uint32 time);
	/** Check whether a game-pixel point selects an available supply plant. */
	bool overDraggable(Common::Point point) const;
	/** Return whether a plant is currently being dragged. */
	bool isDragging() const { return 0 <= _selected; }
	/** Return whether required garden resources or setup failed. */
	bool hasResourceError() const { return !_valid; }
	/** Return whether a rejected plant still has return feedback to complete. */
	bool returnPending() const { return 0 <= _rejected; }
	/** Consume the next queued garden sound ID. */
	bool pollSound(uint16 &sound);
	/** Return plant traits, hole constraints and planting progress. */
	const GardenRules &rules() const { return _rules; }
	/** Describe compatible remaining planting positions for debugger output. */
	Common::String debugAnswer() const;

private:
	/** Script event name for correct. */
	static constexpr const char *kEventCorrect = "correct";
	/** Script event name for incorrect Filled. */
	static constexpr const char *kEventIncorrectFilled = "incorrectFilled";
	/** Script event name for incorrect Empty. */
	static constexpr const char *kEventIncorrectEmpty = "incorrectEmpty";
	/** Script event name for finished. */
	static constexpr const char *kEventFinished = "finished";
	/** Script event name for token Picked Up. */
	static constexpr const char *kEventTokenPickedUp = "tokenPickedUp";
	/** Script event name for snap Done. */
	static constexpr const char *kEventSnapDone = "snapDone";
	/** Script identifier for remaining Rejections. */
	static constexpr const char *kRemainingRejectionsName = "remainingRejections";
	/** Script method name for setup Game. */
	static constexpr const char *kMethodSetupGame = "setupGame";
	/** Script method name for start Round. */
	static constexpr const char *kMethodStartRound = "startRound";
	/** Script method name for finish Round. */
	static constexpr const char *kMethodFinishRound = "finishRound";
	/** Script method name for pause Round. */
	static constexpr const char *kMethodPauseRound = "pauseRound";
	/** Script method name for resume Round. */
	static constexpr const char *kMethodResumeRound = "resumeRound";
	/** Script method name for snap Answer. */
	static constexpr const char *kMethodSnapAnswer = "snapAnswer";
	/** Script identifier for flower. */
	static constexpr const char *kFlowerName = "flower";
	/** Script identifier for stem. */
	static constexpr const char *kStemName = "stem";
	/** Script identifier for leaf. */
	static constexpr const char *kLeafName = "leaf";
	/** Script identifier for root. */
	static constexpr const char *kRootName = "root";

	enum PageResourceId : int16 {
		kResSound18156_PlantPickup = 18156,
		kResSound18157_PlantDrop = 18157,
		kResSound18257_Return = 18257,
	};
	/** Animation path pattern for garden resources. */
	static constexpr const char *kAnimation = "/z3a5/%u.ao";
	struct PlantView {
		/** Sprites displaying the plant's four visual trait parts. */
		Shell::Sprite *parts[4];
		/** Current plant anchor position in game pixels. */
		Common::Point position;
		/** Supply position restored after a rejected placement. */
		Common::Point home;
		/** Pixel displacements of the part sprites from the plant anchor. */
		Common::Point offsets[4];
		/** Combined plant hit rectangle in game pixels. */
		Common::Rect bounds;
		/** Current drawing depth of the plant's sprites. */
		int depth;
	};
	/** Engine supplying garden resources and the shared random stream. */
	TrinketEngine &_vm;
	/** Scene displaying supply plants, planting holes and feedback. */
	Shell::Scene &_scene;
	/** Generated plant traits, hole constraints and placement progress. */
	GardenRules _rules;
	/** Sprites removed when the garden is cleared. */
	Common::Array<Shell::Sprite *> _owned;
	/** Queued garden sound resource IDs. */
	Common::Array<uint16> _sounds;
	/** Whether interaction and return motion are paused. */
	bool _paused = false;
	/** Whether required resources and puzzle state are valid. */
	bool _valid = true;
	/** Whether a rejected plant is moving back to supply. */
	bool _returning = false;
	/** Difficulty requested for garden generation. */
	uint _level = 1;
	/** Number of plants requested for the party. */
	uint _party = 0;
	/** Active return-animation milliseconds, excluding paused intervals. */
	FrameClock _clock;
	/** Active millisecond timestamp when the rejected plant began returning. */
	uint32 _returnTime = 0;
	CubicBezierCurve _returnCurve; ///< Return path from the rejected position to the pickup point.
	/** Dragged plant index, or minus one when none is held. */
	int _selected = -1;
	/** Drawing depth assigned to the next lifted plant. */
	int _depth = 20;
	/** Plant waiting for rejection feedback or return motion, or minus one. */
	int _rejected = -1;
	/** Pointer displacement from the dragged plant anchor in pixels. */
	Common::Point _grab;
	/** Plant position recorded before the current pickup. */
	Common::Point _pickup;
	/** Position where the rejected plant's return motion begins. */
	Common::Point _returnStart;
	/** Visual parts, positions and bounds indexed by supply plant. */
	PlantView _plants[12] = {};
	/** Planting-hole sprites indexed by rule position. */
	Shell::Sprite *_holes[64] = {};
	/** Sprite displaying garden progress and chance feedback. */
	Shell::Sprite *_sun = nullptr;
	/** Generate the garden for a difficulty and party size and build its sprites. */
	bool setup(uint level, uint party);
	/** Remove puzzle sprites and reset drag and rejection state. */
	void clearSprites();
	/** Create a garden sprite at a depth and one-based initial frame. */
	Shell::Sprite *add(uint id, int depth, int frame = 1);
	/** Align all of a plant's part sprites with a game-pixel anchor. */
	void positionPlant(uint index, Common::Point position);
	/** Return the available supply plant at a game-pixel point, or minus one. */
	int hitPlant(Common::Point point) const;
	/** Select a planting hole from the dragged plant's overlap rectangle. */
	int hitHole(const Common::Rect &plantBounds) const;
	/** Evaluate the current plant drop and begin acceptance or rejection feedback. */
	void dropPlant();
	/** Place an accepted plant at its hole and update visible progress. */
	void plantAccepted(uint index, uint hole);
	/** Place remaining plants according to the generated answer plan. */
	void snapAnswer();
	/** Queue a garden sound resource ID. */
	void playEffect(uint id) { _sounds.push_back(id); }
	/** Refresh the sun feedback sprite from the current puzzle progress. */
	void updateSun();
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
