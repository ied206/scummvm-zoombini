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
#include "trinket/frame_clock.h"
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
		/** Number of hours or days advanced by this piece. */
		uint value;
		/** Exhibit controlled by this piece. */
		uint exhibit;
		/** Piece position within its supply layout. */
		uint position;
		/** Whether the piece advances hours or days. */
		Denomination type;
		/** Whether the player has already used this piece. */
		bool consumed;
		/** Whether this piece can still be selected. */
		bool enabled;
	};
	struct Result {
		/** Exhibit that produced this time-setting result. */
		uint exhibit;
		/** Hour or day target checked by this result. */
		Denomination type;
		/** Whether the exhibit currently matches the checked target. */
		bool correct;
		/** Whether the target was reached without a remaining advance of the same type. */
		bool hitTarget;
	};
	/** Initialize both exhibits with no generated time pieces. */
	PlanetariumRules();
	/** Choose difficulty-specific target times, exhibit times and usable pieces. */
	bool generate(uint level, Shell::Random &random);
	/** Return the generated planetarium difficulty. */
	uint level() const { return _level; }
	/** Return an exhibit's current hour in the twenty-four-hour cycle. */
	int hour(uint exhibit) const { return _hour[exhibit]; }
	/** Return an exhibit's current day in the twenty-eight-day lunar cycle. */
	int day(uint exhibit) const { return _day[exhibit]; }
	/** Return the hour both exhibits must reach. */
	int targetHour() const { return _targetHour; }
	/** Return the lunar day both exhibits must reach. */
	int targetDay() const { return _targetDay; }
	/** Check whether an exhibit matches the specified hour or day target. */
	bool solved(uint exhibit, Denomination type) const;
	/** Return whether the difficulty displays lunar position information. */
	bool showMoon() const { return 1 < _level; }
	/** Return the number of generated time-advance pieces. */
	uint pieceCount() const { return _pieces.size(); }
	/** Return a time piece's value, exhibit and availability. */
	const Piece &piece(uint index) const { return _pieces[index]; }
	/** Check whether the selected time piece may currently be applied. */
	bool canUse(uint index) const;
	/** Consume a valid piece and queue its hour or day advance. */
	bool use(uint index);
	/** Return whether an exhibit has remaining time steps to perform. */
	bool advancing(uint exhibit) const { return _remaining[exhibit] != 0; }
	/** Return whether the exhibit is currently advancing hours or days. */
	Denomination advancingType(uint exhibit) const { return _mode[exhibit]; }
	/** Return the number of time steps still queued for an exhibit. */
	uint remaining(uint exhibit) const { return _remaining[exhibit]; }
	/** Return whether any unconsumed enabled piece remains. */
	bool hasTurns() const;

	/** Advance one whole hour or day; the presentation handles interpolation and timing. */
	bool step(uint exhibit, Common::Array<Result> &results);

	/** Return a subset of currently usable pieces reaching this target exactly. */
	bool findAnswer(uint exhibit, Denomination type, Common::Array<uint> &pieces) const;

	/** Plan both denominations, including midnight's effect on available bills. */
	bool findCompleteAnswer(Common::Array<uint> &pieces) const;

private:
	/** Difficulty controlling target generation and piece values. */
	uint _level = 0;
	/** Hour that both exhibits must reach. */
	int _targetHour = 0;
	/** Lunar day that both exhibits must reach. */
	int _targetDay = 0;
	/** Current hour indexed by exhibit. */
	int _hour[2] = {};
	/** Current lunar day indexed by exhibit. */
	int _day[2] = {};
	/** Queued time steps indexed by exhibit. */
	uint _remaining[2] = {};
	/** Hour or day advancement mode indexed by exhibit. */
	Denomination _mode[2] = {
		kHour00,
		kHour00,
	};
	/** Generated time pieces with their current use and availability flags. */
	Common::Array<Piece> _pieces;
	/** Append pieces of the supplied values for one exhibit and time unit. */
	void addPieces(uint exhibit, Denomination type, const uint *values, uint count);
	/** Disable unused pieces for an exhibit's completed time unit. */
	void disablePieces(uint exhibit, Denomination type);
	/** Append the current hour or day outcome and disable pieces after a final target hit. */
	void checkTarget(uint exhibit, Denomination type, Common::Array<Result> &results);
	/** Draw a bounded value while excluding the supplied value. */
	static int randomExcept(Shell::Random &random, uint bound, int excluded);
	/** Apply a candidate piece order while checking that each move is usable. */
	static bool applyAnswer(PlanetariumRules &state, const Common::Array<uint> &pieces);
	/** Allowed hour pairs indexed by lunar day when the Moon is hidden. */
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
	/** Configure the two time exhibits, shared random source and base drawing depth. */
	PlanetariumMachine(Shell::Scene &scene, Shell::Random &random, int depth);
	/** Read planetarium target results and inherited object properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign planetarium cursor, debug and display settings. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch puzzle generation and time-piece control script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Remove exhibit, piece and slot-cover sprites. */
	~PlanetariumMachine();
	/** Build the generated time pieces and both exhibit displays. */
	bool load();
	/** Advance time steps, cover feedback and celestial positions at a millisecond timestamp. */
	void update(uint32 time);
	/** Handle time-piece pickup, dragging and slot release. */
	void event(const Common::Event &event, uint32 time);
	/** Pause or resume exhibit animation and adjust its deadlines. */
	void pause(bool paused, uint32 time);
	/** Enable or disable time-piece interaction. */
	void setEnabled(bool enabled) { _enabled = enabled; }
	/** End the active drag at its current slot or return the piece to supply. */
	void forceDrop();
	/** Return whether a time piece is currently being dragged. */
	bool dragging() const { return 0 <= _dragged; }
	/** Return the selectable piece at a game-pixel point, or minus one if none is hit. */
	int hitPiece(Common::Point point) const;
	/** Return a time piece's current game-pixel bounds. */
	Common::Rect pieceBounds(uint index) const;
	/** Return an exhibit slot's game-pixel bounds. */
	Common::Rect slotBounds(uint index) const;
	/** Describe a usable time-piece sequence for debugger answer output. */
	Common::String debugAnswer(uint level) const;
	/** Return the targets, exhibit times and current piece state. */
	const PlanetariumRules &rules() const { return _rules; }
	/** Consume the next queued planetarium sound ID. */
	bool pollSound(uint16 &sound);
	/** Return whether required resources or puzzle setup failed. */
	bool hasResourceError() const { return !_valid; }

private:
	/** Script event name for picked Up. */
	static constexpr const char *kEventPickedUp = "pickedUp";
	/** Script event name for dropped. */
	static constexpr const char *kEventDropped = "dropped";
	/** Script event name for solved. */
	static constexpr const char *kEventSolved = "solved";
	/** Script event name for wrong. */
	static constexpr const char *kEventWrong = "wrong";
	/** Script property name for is Solved. */
	static constexpr const char *kPropertyIsSolved = "isSolved";
	/** Script property name for last Solved Exhibit. */
	static constexpr const char *kPropertyLastSolvedExhibit = "lastSolvedExhibit";
	/** Script property name for last Solved Denom Type. */
	static constexpr const char *kPropertyLastSolvedDenomType = "lastSolvedDenomType";
	/** Script property name for last Solved By Hitting Target. */
	static constexpr const char *kPropertyLastSolvedByHittingTarget = "lastSolvedByHittingTarget";
	/** Script property name for turns Left. */
	static constexpr const char *kPropertyTurnsLeft = "turnsLeft";
	/** Script property name for is Exhibit Advancing. */
	static constexpr const char *kPropertyIsExhibitAdvancing = "isExhibitAdvancing";
	/** Script property name for display Text. */
	static constexpr const char *kPropertyDisplayText = "displayText";
	/** Script property name for debug Mode. */
	static constexpr const char *kPropertyDebugMode = "debugMode";
	/** Script property name for piece Grab AOCursor ID. */
	static constexpr const char *kPropertyPieceGrabAOCursorID = "pieceGrabAOCursorID";
	/** Script property name for piece Spot AOCursor ID. */
	static constexpr const char *kPropertyPieceSpotAOCursorID = "pieceSpotAOCursorID";
	/** Script property name for enabled. */
	static constexpr const char *kPropertyEnabled = "enabled";
	/** Script method name for pause. */
	static constexpr const char *kMethodPause = "pause";
	/** Script method name for resume. */
	static constexpr const char *kMethodResume = "resume";
	/** Script method name for force Drop. */
	static constexpr const char *kMethodForceDrop = "forceDrop";
	/** Script method name for clear Puzzle. */
	static constexpr const char *kMethodClearPuzzle = "clearPuzzle";
	/** Script method name for generate Puzzle. */
	static constexpr const char *kMethodGeneratePuzzle = "generatePuzzle";
	/** Script class name for hours. */
	static constexpr const char *kClassHours = "hours";
	/** Script class name for days. */
	static constexpr const char *kClassDays = "days";

	/** Animation path pattern for planetarium exhibit resources. */
	static constexpr const char *kAnimation = "/z3a3/%u.ao";
	/** Animation path pattern for planetarium close-up resources. */
	static constexpr const char *kCDAnimation = "/z3a3cd/%u.ao";
	struct Cover {
		/** Sprite used to animate the slot cover opening. */
		Shell::Sprite *opening = nullptr;
		/** Sprite used to animate the slot cover closing. */
		Shell::Sprite *closing = nullptr;
		/** Current opening, open, closing or closed cover state. */
		uint state = 3;
		/** Whether the cover closes again after its opening animation. */
		bool closeAfterOpening = false;
	};
	/** Scene displaying the exhibits and time-piece supply. */
	Shell::Scene &_scene;
	/** Target times, current exhibit times and usable time pieces. */
	PlanetariumRules _rules;
	/** Shared random stream used to generate the puzzle. */
	Shell::Random &_random;
	/** Base drawing depth for planetarium sprites. */
	int _depth;
	/** Most recent target result exposed to script properties. */
	PlanetariumRules::Result _lastResult = {};
	/** Script cursor value selected while dragging a time piece. */
	Core::ScriptValue _grabCursor;
	/** Script cursor value selected over an interactive piece. */
	Core::ScriptValue _spotCursor;
	/** Whether scripted puzzle debug behavior is enabled. */
	bool _debugMode = false;
	/** Whether the configured text display is enabled. */
	bool _displayText = false;
	/** Sprites removed when the current puzzle is cleared. */
	Common::Array<Shell::Sprite *> _owned;
	/** Sprites corresponding to the generated time pieces. */
	Common::Array<Shell::Sprite *> _pieces;
	/** Animations that must resume after the puzzle is unpaused. */
	Common::Array<Shell::Sprite *> _pausedSprites;
	/** Queued planetarium sound resource IDs. */
	Common::Array<uint16> _sounds;
	/** Earth sprite in the model exhibit. */
	Shell::Sprite *_earth = nullptr;
	/** Moon sprite in the model exhibit. */
	Shell::Sprite *_earthMoon = nullptr;
	/** Background sprite of the sky-view exhibit. */
	Shell::Sprite *_sky = nullptr;
	/** Sun sprite positioned by the sky exhibit's hour. */
	Shell::Sprite *_sun = nullptr;
	/** Lunar-phase sprites indexed by day. */
	Shell::Sprite *_moon[28] = {};
	/** Foreground layers that cover parts of the sky exhibit. */
	Shell::Sprite *_skyFront[2] = {};
	/** Hour and day input slots for both exhibits. */
	Shell::Sprite *_slots[4] = {};
	/** Opening and closing feedback state for each input slot. */
	Cover _covers[4] = {};
	/** Active millisecond start time of each exhibit's current step. */
	uint32 _stepTime[2] = {};
	/** Active exhibit time in milliseconds, excluding paused intervals. */
	FrameClock _clock;
	/** Whether each exhibit is waiting for feedback before its next time step. */
	bool _waiting[2] = {};
	/** Whether the player may pick up and use time pieces. */
	bool _enabled = true;
	/** Whether exhibit advancement and interaction are paused. */
	bool _paused = false;
	/** Whether required resources and generated state are valid. */
	bool _valid = true;
	/** Index of the dragged time piece, or minus one when none is held. */
	int _dragged = -1;
	/** Pointer displacement from the piece origin in pixels. */
	Common::Point _grabOffset;
	/** Supply position to restore after an invalid drop. */
	Common::Point _oldPosition;
	/** Remove current exhibit sprites and reset drag and feedback state. */
	void clearPuzzle();
	/** Create a resource sprite at a depth and one-based frame, optionally using the close-up library. */
	Shell::Sprite *add(uint id, int depth, bool cd = false, int frame = 1);
	/** Interpolate an exhibit's visual time through the current step fraction. */
	void updateExhibit(uint exhibit, float fraction);
	/** Advance slot-cover animations and their queued close operations. */
	void updateCovers();
	/** Open a slot cover and optionally schedule it to close again. */
	void openCover(uint index, bool closeAfter);
	/** Start closing a slot cover. */
	void closeCover(uint index);
	/** Update slot highlighting and feedback for the currently dragged piece. */
	void updateSlot();
	/** Apply a valid time piece or return it to its supply position. */
	void drop();
	/** Start timing an exhibit's next queued hour or day step. */
	void beginStep(uint exhibit);
	/** Return the current exhibit step's duration in milliseconds. */
	uint stepDuration(uint exhibit) const;
	/** Place a sprite's center at a game-pixel position. */
	static void centerAt(Shell::Sprite *sprite, Common::Point position);
	/** Map an angle in degrees to the sky exhibit's elliptical pixel path. */
	static Common::Point celestialPosition(float angle);
	/** Convert an hour to the sky exhibit's angle in degrees. */
	static float hourAngle(float hour);
	/** Lunar angular offsets in degrees indexed by day. */
	static const int kMoonAngles[28];
	/** Hour-piece supply positions indexed by exhibit and piece position. */
	static const Common::Point kHourPositions[2][9];
	/** Day-piece supply positions indexed by exhibit and piece position. */
	static const Common::Point kDayPositions[2][9];
};

/** Native incubator presentation with independently configured row populations. */
class Incubator : public Core::Trinket {
public:
	/** Configure the transformation display's drawing depth and number of rows. */
	Incubator(TrinketEngine &vm, Shell::Scene &scene, int depth, uint rows);
	/** Remove pupa and door sprites. */
	~Incubator() override;
	/** Create the incubator doors and initial pupa display. */
	bool load();
	/** Read the pupa population of a selected row. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Change the pupa population of a selected row. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch transformation requests for a row and pupa count. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Advance transformation batches and pupa flights at a millisecond timestamp. */
	void update(uint32 time);
	/** Pause or resume transformation timing and sprite playback. */
	void pause(bool paused, uint32 time);
	/** Return whether a required incubator resource or setup failed. */
	bool hasResourceError() const { return !_valid; }

private:
	/** Script property name for pupae Count. */
	static constexpr const char *kPropertyPupaeCount = "pupaeCount";
	/** Script property name for pupae Left To Transform. */
	static constexpr const char *kPropertyPupaeLeftToTransform = "pupaeLeftToTransform";
	/** Script property name for door Count. */
	static constexpr const char *kPropertyDoorCount = "doorCount";
	/** Script method name for transform Pupae. */
	static constexpr const char *kMethodTransformPupae = "transformPupae";
	/** Script method name for pause. */
	static constexpr const char *kMethodPause = "pause";
	/** Script method name for resume. */
	static constexpr const char *kMethodResume = "resume";

	/** Animation path pattern for incubator resources. */
	static constexpr const char *kAnimation = "/z3a3/%u.ao";
	struct Pupa {
		/** Sprite displaying the pupa and its transformation. */
		Shell::Sprite *sprite = nullptr;
		/** Initial game-pixel position of the current motion. */
		Common::Point start;
		/** Target game-pixel position of the current motion. */
		Common::Point destination;
		/** Current resting, transforming or moving stage. */
		uint state = 0;
		/** Active millisecond timestamp when the current stage began. */
		uint32 started = 0;
		/** Initial phase in radians of the flight's vertical oscillation. */
		float phase = 0;
		/** Vertical flight oscillation amplitude in pixels. */
		float amplitude = 0;
		/** Flight oscillation frequency in radians per millisecond. */
		float frequency = 0;
		/** Straight-line motion distance in game pixels. */
		float distance = 0;
	};
	struct Batch {
		/** Incubator row whose pupae are being transformed. */
		uint row;
		/** First pupa index included in the transformation batch. */
		uint first;
		/** One-past-last pupa index included in the transformation batch. */
		uint end;
		/** Active millisecond timestamp controlling the batch's next stage. */
		uint32 time;
	};
	/** Engine supplying transformation resources and random flight parameters. */
	TrinketEngine &_vm;
	/** Scene displaying the incubator doors and pupae. */
	Shell::Scene &_scene;
	/** Number of incubator rows enabled by this display. */
	uint _rows;
	/** Door sprites indexed by incubator row. */
	Shell::Sprite *_doors[6] = {};
	/** Pupa sprites and transformation state indexed by row and position. */
	Pupa _pupae[6][4] = {};
	/** Current pupa population indexed by incubator row. */
	uint _population[6] = {};
	/** Number of pupae already transformed in each row. */
	uint _transformed[6] = {};
	/** Current opening or closing stage of each row's door. */
	uint _doorState[6] = {};
	/** Transformation requests waiting for their animation stages. */
	Common::Array<Batch> _batches;
	/** Sprites removed when the incubator is cleared. */
	Common::Array<Shell::Sprite *> _owned;
	/** Animations to resume after the incubator is unpaused. */
	Common::Array<Shell::Sprite *> _pausedSprites;
	/** Active transformation time in milliseconds, excluding paused intervals. */
	FrameClock _clock;
	/** Whether transformation batches and flights are paused. */
	bool _paused = false;
	/** Whether required resources and display state are valid. */
	bool _valid = true;
	/** Create an incubator sprite at a depth and one-based frame. */
	Shell::Sprite *add(uint id, int depth, int frame = 1);
	/** Remove an incubator sprite from tracking and the scene. */
	void remove(Shell::Sprite *sprite);
	/** Set a valid row's pupa count and refresh its supply sprites. */
	bool setPopulation(uint row, uint count);
	/** Queue the selected number of pupae in a row for transformation. */
	void transform(uint row, uint count);
	/** Pupa supply positions indexed by layout, row and position. */
	static const Common::Point kPupaPositions[2][6][4];
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
