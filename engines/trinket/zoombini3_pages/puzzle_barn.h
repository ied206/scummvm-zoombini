/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_ZOOMBINI3_PAGES_PUZZLE_BARN_H
#define TRINKET_ZOOMBINI3_PAGES_PUZZLE_BARN_H

#include "common/ptr.h"
#include "trinket/frame_clock.h"
#include "trinket/shell/hotspot.h"
#include "trinket/shell/random.h"
#include "trinket/shell/scene.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_world.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Zoombini3 {

/** Cyclic trait dominance and four-child matching, with one parent per pen. */
class BarnRules {
public:
	struct Parent {
		/** Visible trait values used to draw this parent. */
		int32 traits[3];
		/** Two allele values for the trait used by the current puzzle. */
		uint genes[2];
		/** Supply side to which this parent belongs. */
		uint side;
		/** Script tag identifying the corresponding party character. */
		int tag;
		/** Whether this parent has already completed a correct pairing. */
		bool paired;
	};
	struct Pair {
		/** Parent indices from the two supply sides. */
		uint parent[2];
	};
	struct Input {
		/** Authored parent traits, with negative values requesting generated traits. */
		int32 traits[3] = {
			-1,
			-1,
			-1,
		};
		/** Script tag to retain for the generated parent, or minus one when unset. */
		int tag = -1;
	};
	class Generator {
	public:
		/** Allow destruction through the parent-generation callback interface. */
		virtual ~Generator() {}
		/** Create presentation for a newly generated parent and report whether setup succeeded. */
		virtual bool barnParentGenerated(uint index) = 0;
	};
	enum Placement {
		kUnavailable00 = 0,
		kSingle01 = 1,
		kWrong02 = 2,
		kCorrect03 = 3,
	};
	/** Initialize an empty parent supply and pairing challenge. */
	BarnRules();
	/** Choose difficulty-specific genetics, mistake limits and party parent inputs. */
	bool generate(uint level, uint party, Shell::Random &random, const World::Token *tokens = nullptr, const int *tags = nullptr,
				  Generator *generator = nullptr);
	/** Generate parent alleles and target offspring from explicit puzzle settings. */
	bool generatePuzzle(uint pairs, uint trait, bool showGenes, uint limit, Shell::Random &random, const Common::Array<Input> &inputs,
						Generator *generator = nullptr);
	/** Return the genetics difficulty of the current puzzle. */
	uint level() const { return _level; }
	/** Return the number of parent pairs required by the puzzle. */
	uint pairs() const { return _pairs; }
	/** Return the total number of supplied parents. */
	uint party() const { return _pairs * 2; }
	/** Return the trait whose inheritance is being tested. */
	uint trait() const { return _trait; }
	/** Return whether allele information is visible to the player. */
	bool showGenes() const { return _showGenes; }
	/** Return the number of mistakes still available. */
	uint chances() const { return _limit - _wrong; }
	/** Return the number of incorrect pair submissions. */
	uint wrong() const { return _wrong; }
	/** Return the configured mistake limit. */
	uint limit() const { return _limit; }
	/** Return the number of correctly completed pairs. */
	uint completed() const { return _completed; }
	/** Return a parent's visible traits, alleles and pairing state. */
	const Parent &parent(uint index) const { return _parents[index]; }
	/** Return one of the four target offspring trait values. */
	uint target(uint index) const { return _target[index]; }
	/** Return the selected parent on a supply side, or minus one when unset. */
	int selected(uint side) const { return _selected[side]; }
	/** Place a parent in its selection area and report the placement outcome. */
	Placement place(uint index);
	/** Remove a parent from its current selection area. */
	bool remove(uint index);
	/** Return an eligible parent to its supply state. */
	bool restore(uint index);
	/** Evaluate the selected parents and apply the resulting progress or mistake. */
	bool commitPair();
	/** Calculate the four allele combinations produced by two parents. */
	void offspring(int first, int second, int values[4]) const;
	/** Check whether two parents produce the required offspring distribution. */
	bool matches(uint first, uint second) const;
	/** Find compatible parent pairs for the remaining puzzle. */
	bool findAnswer(Common::Array<Pair> &plan) const;
	/** Select the visible trait expressed by a pair of alleles. */
	static uint dominant(uint first, uint second);
	/** Return a parent's initial supply position in game pixels. */
	static Common::Point home(uint index);

private:
	/** Difficulty controlling trait selection and gene visibility. */
	uint _level = 1;
	/** Number of parent pairs required to finish. */
	uint _pairs = 0;
	/** Maximum number of incorrect submissions available. */
	uint _limit = 0;
	/** Incorrect pair submissions made so far. */
	uint _wrong = 0;
	/** Correct parent pairs already completed. */
	uint _completed = 0;
	/** Trait whose allele combinations determine the target offspring. */
	uint _trait = 1;
	/** Whether the puzzle displays parent alleles. */
	bool _showGenes = true;
	/** Generated parents and their pairing state. */
	Parent _parents[12];
	/** Saved random choice offsets for target selection on each supply side. */
	uint _choices[2][6];
	/** Four offspring trait values required by the current challenge. */
	uint _target[4] = {};
	/** Selected parent index for each supply side, or minus one when empty. */
	int _selected[2] = {
		-1,
		-1,
	};
	/** Choose the next target offspring distribution from remaining parents. */
	void chooseTarget();
};

/** Native Barn parent slots, genes, target reels and offspring/departure tasks. */
class BarnMachine : public Core::ScriptableObject, private BarnRules::Generator {
public:
	/** Configure the genetics display and its base drawing depth. */
	BarnMachine(TrinketEngine &vm, Shell::Scene &scene, int depth);
	/** Stop reel audio and remove parent and feedback sprites. */
	~BarnMachine() override;
	/** Read pairing progress, result tags and inherited properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign supported genetics display and cursor settings. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch puzzle generation, parent setup and playback-control script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Handle parent pickup, dragging and placement at a millisecond timestamp. */
	void event(const Common::Event &event, uint32 time);
	/** Advance gene reveals, offspring previews and result feedback. */
	void update(uint32 time);
	/** Pause or resume genetics animations and adjust their deadlines. */
	void pause(bool paused, uint32 time);
	/** Check whether a game-pixel point selects an available parent. */
	bool overDraggable(Common::Point point) const;
	/** Return whether a parent is currently being dragged. */
	bool isDragging() const { return 0 <= _selected; }
	/** Return whether a required genetics resource or setup failed. */
	bool hasResourceError() const { return !_valid; }
	/** Consume the next queued genetics sound ID. */
	bool pollSound(uint16 &sound);
	/** Return parent genetics, target offspring and pairing progress. */
	const BarnRules &rules() const { return _rules; }
	/** Return the number of mistakes already reflected in water-drain feedback. */
	uint drained() const { return _drained; }
	/** Describe compatible remaining parent pairs for debugger answer output. */
	Common::String debugAnswer() const;

private:
	/** Script event name for dropped. */
	static constexpr const char *kEventDropped = "dropped";
	/** Script event name for picked Up. */
	static constexpr const char *kEventPickedUp = "pickedUp";
	/** Script event name for solved. */
	static constexpr const char *kEventSolved = "solved";
	/** Script event name for wrong. */
	static constexpr const char *kEventWrong = "wrong";
	/** Script event name for out Of Turns. */
	static constexpr const char *kEventOutOfTurns = "outOfTurns";
	/** Script property name for solved Zerble Tag. */
	static constexpr const char *kPropertySolvedZerbleTag = "solvedZerbleTag";
	/** Script property name for debug Mode. */
	static constexpr const char *kPropertyDebugMode = "debugMode";
	/** Script property name for piece Grab AOCursor ID. */
	static constexpr const char *kPropertyPieceGrabAOCursorID = "pieceGrabAOCursorID";
	/** Script property name for piece Spot AOCursor ID. */
	static constexpr const char *kPropertyPieceSpotAOCursorID = "pieceSpotAOCursorID";
	/** Script property name for next Zerble Trait. */
	static constexpr const char *kPropertyNextZerbleTrait = "nextZerbleTrait";
	/** Script property name for enabled. */
	static constexpr const char *kPropertyEnabled = "enabled";
	/** Script method name for generate Puzzle. */
	static constexpr const char *kMethodGeneratePuzzle = "generatePuzzle";
	/** Script method name for add Next Zerble. */
	static constexpr const char *kMethodAddNextZerble = "addNextZerble";
	/** Script method name for pause. */
	static constexpr const char *kMethodPause = "pause";
	/** Script method name for resume. */
	static constexpr const char *kMethodResume = "resume";
	/** Script method name for force Drop. */
	static constexpr const char *kMethodForceDrop = "forceDrop";
	/** Script method name for clear Puzzle. */
	static constexpr const char *kMethodClearPuzzle = "clearPuzzle";
	/** Script method name for interrupt. */
	static constexpr const char *kMethodInterrupt = "interrupt";
	/** Script identifier for single. */
	static constexpr const char *kSingleName = "single";
	/** Script identifier for correct. */
	static constexpr const char *kCorrectName = "correct";
	/** Script identifier for tail. */
	static constexpr const char *kTailName = "tail";
	/** Script identifier for feet. */
	static constexpr const char *kFeetName = "feet";

	enum PageResourceId : int16 {
		kResSound21052_StoneOpen = 21052,
		kResSound21065_StoneClose = 21065,
		kResSound21081_WrongPair = 21081,
		kResSound21093_CorrectPair = 21093,
		kResSound21094_ParentRemoved = 21094,
		kResSound21096_Machine = 21096,
		kResSound21097_ParentExit = 21097,
		kResSound21099_StoneMove = 21099,
		kResSound21100_GeneReveal = 21100,
		kResSound21101_GeneClose = 21101,
		kResSound2726_CommonGo = 2726,
	};
	/** Animation path pattern for genetics puzzle resources. */
	static constexpr const char *kAnimation = "/z3a7/%u.ao";
	/** Animation path pattern for genetics close-up resources. */
	static constexpr const char *kCDAnimation = "/z3a7cd/%u.ao";
	/** Sound path pattern for genetics puzzle resource IDs. */
	static constexpr const char *kSound = "/z3a7/%u.wav";
	enum State {
		kSpinning00 = 0,
		kPlaying01 = 1,
		kGenes02 = 2,
		kPreview03 = 3,
		kCelebrating04 = 4,
		kExitDelay05 = 5,
		kExiting06 = 6,
	};
	/** Engine supplying character resources and shared randomness. */
	TrinketEngine &_vm;
	/** Scene displaying parent characters and genetics feedback. */
	Shell::Scene &_scene;
	/** Parent alleles, selected pair and target offspring state. */
	BarnRules _rules;
	/** Parent traits and script tags supplied before generation. */
	Common::Array<BarnRules::Input> _inputs;
	/** Sprites removed when the puzzle is cleared. */
	Common::Array<Shell::Sprite *> _owned;
	/** Animations to resume after the puzzle is unpaused. */
	Common::Array<Shell::Sprite *> _pausedSprites;
	/** Queued genetics sound resource IDs. */
	Common::Array<uint16> _sounds;
	/** Base drawing depth of the genetics display. */
	int _depth;
	/** Script-selected genetics debug mode. */
	int _debugMode = 0;
	/** Script tags of the most recently accepted parent pair. */
	int _solvedTags[2] = {};
	/** Cursor resource selected while dragging a parent. */
	Common::String _grabCursor = "0";
	/** Cursor resource selected over an available parent. */
	Common::String _spotCursor = "0";
	/** Whether input and timed feedback are paused. */
	bool _paused = false;
	/** Whether required resources and puzzle state are valid. */
	bool _valid = true;
	/** Whether the most recent parent submission matched the target. */
	bool _correct = false;
	/** Whether the current puzzle has exhausted its available chances. */
	bool _exhausted = false;
	/** Whether parent genetics and target offspring have been generated. */
	bool _generated = false;
	/** Whether the player may select and drag parents. */
	bool _enabled = true;
	/** Whether the offspring reels are waiting before their next spin stage. */
	bool _spinWaiting = false;
	/** Difficulty requested for genetics generation. */
	uint _level = 1;
	/** Current step of the offspring preview or result animation. */
	uint _step = 0;
	/** Number of mistakes already represented by the water display. */
	uint _drained = 0;
	/** One-based water sprite frame corresponding to the remaining chances. */
	uint _waterFrame = 1;
	/** Most recent host timestamp in milliseconds. */
	uint32 _time = 0;
	/** Active millisecond timestamp when the current feedback state began. */
	uint32 _stateTime = 0;
	/** Active feedback milliseconds, separate from character animation source time. */
	FrameClock _clock;
	/** Current parent-selection, preview or result-feedback stage. */
	State _state = kPlaying01;
	/** Dragged parent index, or minus one when no parent is held. */
	int _selected = -1;
	/** Parent's selection position before the current pickup. */
	int _previousSlot = -1;
	/** Pointer displacement from the dragged parent's origin in pixels. */
	Common::Point _grab;
	/** Parent position to restore after an invalid drop. */
	Common::Point _pickup;
	/** Character sprites indexed by supplied parent. */
	AttributeCharacter *_parents[12] = {};
	/** Parent-selection stone sprites indexed by supply side. */
	Shell::Sprite *_stones[2] = {};
	/** Allele reveal sprites indexed by side and gene. */
	Shell::Sprite *_genes[2][2] = {};
	/** Visible trait faces displayed over the allele reveals. */
	Shell::Sprite *_geneFaces[2][2] = {};
	/** Sprites displaying the four required offspring traits. */
	Shell::Sprite *_target[4] = {};
	/** Reel sprites used by the offspring preview. */
	Shell::Sprite *_reels[4] = {};
	/** Sprites displaying the four preview offspring. */
	Shell::Sprite *_children[4] = {};
	/** Sprite displaying the remaining mistake allowance. */
	Shell::Sprite *_water = nullptr;
	/** Sprite used for successful-pair feedback. */
	Shell::Sprite *_celebration = nullptr;
	/** Sprites emphasizing the selected parents' alleles. */
	Shell::Sprite *_emphasis[2] = {};
	/** Active millisecond reveal timestamps indexed by supply side. */
	uint32 _geneTime[2] = {};
	/** Whether each side's allele display is opening. */
	bool _geneOpening[2] = {};
	/** Whether each allele reveal has begun. */
	bool _geneStarted[2][2] = {};
	/** Playback state for the offspring reel sound. */
	OM::Sound _reelSound;
	/** Generate genetics and build the display from explicit puzzle parameters. */
	bool setup(uint pairs, uint trait, bool showGenes, uint limit);
	/** Remove puzzle sprites and reset selection and feedback state. */
	void clearPuzzle();
	/** Remove one genetics sprite from tracking and the scene. */
	void removeSprite(Shell::Sprite *sprite);
	/** Create a genetics sprite at a depth, optionally using close-up resources. */
	Shell::Sprite *add(uint id, int depth, bool cd = false);
	/** Build the character presentation for a newly generated parent. */
	bool barnParentGenerated(uint index) override;
	/** Return the selectable parent at a game-pixel point, or minus one. */
	int hitParent(Common::Point point) const;
	/** Begin dragging a parent and retain its prior placement. */
	void pick(uint index, Common::Point point);
	/** Place the dragged parent or restore its previous position. */
	void drop();
	/** End an active parent drag before a script-driven transition. */
	void forceDrop();
	/** Advance the current offspring-preview or result-feedback stage. */
	void updateState();
	/** Begin the offspring reel animation. */
	void startSpin();
	/** Prepare the four offspring combinations for the selected pair. */
	void startPreview();
	/** Display one offspring trait combination in the preview. */
	void showChild(uint index);
	/** Remove the offspring preview sprites. */
	void clearChildren();
	/** Open or close one parent's allele display, optionally without animation. */
	void updateGenes(uint side, bool opening, bool animated = true);
	/** Submit the selected pair and start its result feedback. */
	void commitPair();
	/** Update the water display for mistakes not yet animated. */
	void drainWater();
	/** Queue a genetics sound resource ID. */
	void playEffect(uint id) { _sounds.push_back(id); }
	/** Return a selection stone's game-pixel position. */
	static Common::Point stonePosition(uint side);
	/** Return a selection stone's game-pixel hit rectangle. */
	static Common::Rect stoneBounds(uint side);
	/** Return the game-pixel bounds of a parent supply pen. */
	static Common::Rect penBounds(uint side);
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
