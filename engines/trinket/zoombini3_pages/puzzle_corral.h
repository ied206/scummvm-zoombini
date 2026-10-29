/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_ZOOMBINI3_PAGES_PUZZLE_CORRAL_H
#define TRINKET_ZOOMBINI3_PAGES_PUZZLE_CORRAL_H

#include "common/ptr.h"
#include "trinket/shell/random.h"
#include "trinket/shell/scene.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_world.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

/** Snozzleberry preferences, projector charges and overlapping light regions. */
class CorralRules {
public:
	struct Fruit {
		/** Fruit shape, color and leaf trait values. */
		uint traits[3];
		/** Whether this fruit has been used to feed a zerble. */
		bool consumed;
		/** Projector currently holding the fruit, or a negative marker while in supply. */
		int projector;
		/** Initial fruit-supply position in game pixels. */
		Common::Point supply;
	};
	struct Zerble {
		/** Foot trait used to identify the zerble's food preference. */
		uint feet;
		/** Tail trait used to identify the zerble's food preference. */
		uint tail;
		/** Whether the zerble has received a matching fruit. */
		bool fed;
		/** Corral position currently occupied by the zerble. */
		int slot;
		/** Current game-pixel position within the corral. */
		Common::Point position;
	};
	struct Feeding {
		/** Fruit index selected by this feeding step. */
		uint fruit;
		/** Zerble index that can eat the selected fruit. */
		uint zerble;
	};
	class Generator {
	public:
		/** Allow destruction through the corral-generation callback interface. */
		virtual ~Generator() {}
		/** Create the character here to preserve its timer RNG consumption. */
		virtual bool corralGenerated(uint index) = 0;
	};
	/** Initialize an empty fruit supply and zerble population. */
	CorralRules();
	/** Generate difficulty-specific projector traits, fruit and zerble positions. */
	bool generate(uint level, uint party, Shell::Random &random, Generator *generator = nullptr);
	/** Return the current corral difficulty. */
	uint level() const { return _level; }
	/** Return the number of fruit and zerble pairs in this puzzle. */
	uint party() const { return _party; }
	/** Return the number of turns still available. */
	int turns() const { return _turns; }
	/** Set the turn allowance exposed by the script interface. */
	void setTurns(int count) { _turns = count; }
	/** Return the number of projectors active at this difficulty. */
	uint projectorCount() const { return _level == 1 ? 2 : 3; }
	/** Return the fruit-trait axis revealed by a projector. */
	uint axis(uint projector) const { return _axes[projector]; }
	/** Return the fruit held by a projector, or its empty marker. */
	int projectedFruit(uint projector) const { return _projectors[projector]; }
	/** Return a fruit's traits, supply position and consumption state. */
	const Fruit &fruit(uint index) const { return _fruits[index]; }
	/** Return a zerble's traits, position and feeding state. */
	const Zerble &zerble(uint index) const { return _zerbles[index]; }
	/** Place a fruit in a projector and apply the difficulty's reveal and relocation rules. */
	bool project(uint fruit, uint projector, Shell::Random &random);
	/** Remove an available fruit from its projector. */
	bool remove(uint fruit);
	/** Check whether a fruit satisfies the selected zerble's preference. */
	bool matches(uint fruit, uint zerble) const;
	/** Consume a matching fruit and mark its zerble fed. */
	bool feed(uint fruit, uint zerble);
	/** Count fruit that has not yet been consumed. */
	uint fruitRemaining() const;
	/** Count zerbles that have received a matching fruit. */
	uint fedCount() const;
	/** Return the corral region occupied by a zerble. */
	uint region(uint zerble) const;
	/** Build a valid sequence of fruit-to-zerble feedings. */
	void findAnswer(Common::Array<Feeding> &plan) const;
	/** Return the game-pixel position of a corral slot. */
	static Common::Point slotPosition(uint slot);

private:
	/** Difficulty controlling projector count and zerble relocation. */
	uint _level = 1;
	/** Number of fruit and zerble pairs generated for the party. */
	uint _party = 0;
	/** Trait axis revealed by each projector. */
	uint _axes[3] = {};
	/** Remaining turn allowance. */
	int _turns = 0;
	/** Fruit index held by each projector, with a negative marker for empty slots. */
	int _projectors[3] = {
		-1,
		-1,
		-1,
	};
	/** Generated fruit traits and current use state. */
	Fruit _fruits[12];
	/** Generated zerble traits, feeding state and corral placement. */
	Zerble _zerbles[12];
	/** Move eligible zerbles to randomly selected valid corral slots. */
	bool relocate(Shell::Random &random);
	/** List the corral slots permitted for a zerble's traits and difficulty. */
	void allowedSlots(uint zerble, Common::Array<uint> &slots) const;
};

/** Native Corral fruit, projector, herd and feeding callbacks. */
class CorralMachine : public Shell::MultiTrinket, private CorralRules::Generator {
public:
	/** Connect corral generation and visual interaction to the engine and scene. */
	CorralMachine(TrinketEngine &vm, Shell::Scene &scene);
	/** Remove fruit, zerble and projector feedback sprites. */
	~CorralMachine() override;
	/** Read corral progress, feeding results and inherited properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign supported corral progress and interaction properties. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch corral generation, demonstration and round-control script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Handle fruit pickup, dragging, projection and feeding. */
	void event(const Common::Event &event, uint32 time);
	/** Advance zerble movement, blinking and feeding feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Pause or resume corral interaction and animation timing. */
	void pause(bool paused, uint32 time);
	/** Check whether a game-pixel point selects an available fruit. */
	bool overDraggable(Common::Point point) const;
	/** Return whether a fruit is currently being dragged. */
	bool isDragging() const { return 0 <= _selected; }
	/** Return whether required resources or corral setup failed. */
	bool hasResourceError() const { return !_valid; }
	/** Return whether feeding feedback is waiting to finish. */
	bool feedingPending() const { return 0 <= _feeding; }
	/** Consume the next queued corral sound ID. */
	bool pollSound(uint16 &sound);
	/** Return fruit traits, projector assignments and zerble feeding state. */
	const CorralRules &rules() const { return _rules; }
	/** Describe valid remaining fruit-to-zerble feedings for debugger output. */
	Common::String debugAnswer() const;

private:
	/** Script event name for correct. */
	static constexpr const char *kEventCorrect = "correct";
	/** Script event name for out Of Turns. */
	static constexpr const char *kEventOutOfTurns = "outOfTurns";
	/** Script event name for finished. */
	static constexpr const char *kEventFinished = "finished";
	/** Script event name for token Picked Up. */
	static constexpr const char *kEventTokenPickedUp = "tokenPickedUp";
	/** Script property name for remaining Turns. */
	static constexpr const char *kPropertyRemainingTurns = "remainingTurns";
	/** Script property name for current Zerble Feet. */
	static constexpr const char *kPropertyCurrentZerbleFeet = "currentZerbleFeet";
	/** Script property name for current Zerble Tail. */
	static constexpr const char *kPropertyCurrentZerbleTail = "currentZerbleTail";
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
	/** Script identifier for shape. */
	static constexpr const char *kShapeName = "shape";
	/** Script identifier for color. */
	static constexpr const char *kColorName = "color";
	/** Script identifier for leaf. */
	static constexpr const char *kLeafName = "leaf";

	/**
	 * Map a zero-based fruit color to its matching fruit and projector display conversion.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 */
	static OM::Canvas::ColorRemap fruitColor(uint color);
	enum PageResourceId : int16 {
		kResSound20112_RejectEarly = 20112,
		kResSound20113_RejectLate = 20113,
		kResSound20114_FruitPickup = 20114,
		kResSound20115_ProjectorReady = 20115,
		kResSound20116_FruitDrop = 20116,
		kResSound20117_TrayHover = 20117,
		kResSound20400_Projection = 20400,
		kResSound2726_CommonGo = 2726,
	};
	/** Animation path pattern for corral resources. */
	static constexpr const char *kAnimation = "/z3a6/%u.ao";
	/** Resource path of the projector beam animation. */
	static constexpr const char *kBeamAnimation = "/z3a6cd/20103.ao";
	/** Animation path pattern for shared character resources. */
	static constexpr const char *kCommonAnimation = "/z3common/%u.ao";
	struct FruitView {
		/** Sprite displaying the fruit body. */
		Shell::Sprite *body;
		/** Sprite displaying the fruit's leaf trait. */
		Shell::Sprite *leaf;
		/** Current anchor position of the combined fruit sprites. */
		Common::Point position;
		/** Supply position restored after an invalid drop. */
		Common::Point home;
		/** Body sprite displacement from the fruit anchor in pixels. */
		Common::Point bodyOffset;
		/** Leaf sprite displacement from the fruit anchor in pixels. */
		Common::Point leafOffset;
		/** Combined fruit hit rectangle in game pixels. */
		Common::Rect bounds;
	};
	/** Engine supplying corral resources and shared randomness. */
	TrinketEngine &_vm;
	/** Scene displaying fruit, zerbles and projectors. */
	Shell::Scene &_scene;
	/** Generated trait rules and current feeding and projection state. */
	CorralRules _rules;
	/** Sprites removed when the corral is cleared. */
	Common::Array<Shell::Sprite *> _owned;
	/** Queued corral sound resource IDs. */
	Common::Array<uint16> _sounds;
	/** Whether corral input and animation timing are paused. */
	bool _paused = false;
	/** Whether required resources and puzzle state are valid. */
	bool _valid = true;
	/** Whether projector feedback is running an introductory demonstration. */
	bool _demonstrating = false;
	/** Whether the pending fruit feeding matches its zerble. */
	bool _correct = false;
	/** Difficulty requested for corral generation. */
	uint _level = 1;
	/** Fruit count retained for round progress feedback. */
	uint _fruitRemaining = 0;
	/** Unfed zerble count retained for round progress feedback. */
	uint _zerblesRemaining = 0;
	/** Current corral timestamp in milliseconds. */
	uint32 _time = 0;
	/** Source-time millisecond deadline for the next blink, retained across pause. */
	uint32 _blinkDeadline = 0;
	/** Number of feedback blink steps still to perform. */
	uint _blinkCount = 0;
	/** Dragged fruit index, or minus one when none is held. */
	int _selected = -1;
	/** Zerble index receiving the pending feeding feedback. */
	int _feeding = -1;
	/** Fruit index used by the pending feeding feedback. */
	int _feedingFruit = -1;
	/** Most recent zerble to leave after a successful feeding. */
	int _lastExited = -1;
	/** Zerble index currently selected for hover feedback. */
	int _hover = -1;
	/** Projector tray currently highlighted by the dragged fruit. */
	int _hoverTray = -1;
	/** Pointer displacement from the fruit anchor in pixels. */
	Common::Point _grab;
	/** Fruit position recorded before the current drag. */
	Common::Point _pickup;
	/** Game-pixel rectangle used by the current hover target. */
	Common::Rect _hoverBounds;
	/** Body, leaf and placement sprites indexed by fruit. */
	FruitView _fruits[12] = {};
	/** Character presentations indexed by zerble. */
	AttributeCharacter *_zerbles[12] = {};
	/** Sprite displaying the projector beam effects. */
	Shell::Sprite *_beams = nullptr;
	/** Projector mechanism sprites. */
	Shell::Sprite *_projectors[3] = {};
	/** Fruit input tray sprites indexed by projector. */
	Shell::Sprite *_trays[3] = {};
	/** Trait clue sprites indexed by projector and revealed value. */
	Shell::Sprite *_clues[3][5] = {};
	/** Projector result-feedback sprites. */
	Shell::Sprite *_feedback[3] = {};
	/** Sprites displaying zerble sniffing feedback. */
	Shell::Sprite *_sniff[3] = {};
	/** Whether each zerble is leaving after a successful feeding. */
	bool _exiting[12] = {};
	/** Whether each zerble is moving to a new corral slot. */
	bool _moving[12] = {};
	/** Generate the corral for a difficulty and party size and build its sprites. */
	bool setup(uint level, uint party);
	/** Remove the current puzzle sprites and clear drag and feeding state. */
	void clearSprites();
	/** Remove a corral sprite from tracking and the scene. */
	void removeSprite(Shell::Sprite *sprite);
	/** Create a corral sprite at a depth, optionally using shared resources. */
	Shell::Sprite *add(uint id, int depth, bool common = false);
	/** Build a newly generated fruit and zerble presentation. */
	bool corralGenerated(uint index) override;
	/** Move a fruit's body and leaf sprites to the supplied anchor. */
	void positionFruit(uint index, Common::Point position);
	/** Return the combined game-pixel bounds of a fruit. */
	Common::Rect fruitBounds(uint index) const;
	/** Return the available fruit at a game-pixel point, or minus one. */
	int hitFruit(Common::Point point) const;
	/** Project or feed the dragged fruit when its current target permits it. */
	void drop();
	/** Refresh projector clues and currently projected fruit. */
	void updateProjectors();
	/** Advance the introductory or result-feedback blink sequence. */
	void updateBlink();
	/** Refresh zerble and tray feedback beneath the dragged fruit. */
	void updateHover();
	/** Clear the current zerble and tray hover feedback. */
	void stopHover();
	/** Animate zerbles toward the positions selected by the rules. */
	void moveZerbles();
	/** Start feedback for a fruit offered to a zerble. */
	void beginFeeding(uint fruit, uint zerble);
	/** Apply the pending feeding outcome and update remaining counts. */
	void finishFeeding();
	/** Advance feedback sprites and append their triggered sounds. */
	void updateFeedback(Common::Array<uint16> &sounds);
	/** Queue a corral sound resource ID. */
	void playEffect(uint id) { _sounds.push_back(id); }
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
