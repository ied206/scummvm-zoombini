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

#ifndef ZOOMBINI2_PAGES_PUZZLE_CRAZYTURTLE_H
#define ZOOMBINI2_PAGES_PUZZLE_CRAZYTURTLE_H

#include "common/rect.h"

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class Animation;

/**
 * Turtle Hurdle (Route1-1)
 *
 * Place the Zoombinis on the baby turtles in the required order.
 */
class PuzzleCrazyTurtle : public PuzzleBase {
public:
	/** Construct Turtle Hurdle for @p vm. */
	PuzzleCrazyTurtle(Zoombini2Engine *vm);
	/** Release turtle, bridge, and trait resources. */
	~PuzzleCrazyTurtle() override;

	/** Load the dock and generate the level-selected ordering rules. */
	void init() override;
	/** Format active feature ordering and turtle assignments for the puzzle console. */
	Common::String debugGetAnswer() const override;
	/** Report remaining incorrect placements as the puzzle's chance count. */
	PuzzleChanceInfo debugGetChances() const override;
	/** Return whether the console may alter mistakes while no placement is resolving. */
	bool debugCanSetChances() const override;
	/** Set remaining mistakes and update the dock state for the puzzle console. */
	bool debugSetChances(int remaining) override;
	/** Advance the fall sequence, turtle spins, and departure start. */
	void onUpdate() override;
	/** Keep the press state unchanged; releases place the Zoombini. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Release a held Zoombini onto the dock. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Keep a held Zoombini following the pointer. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Report whether the shared Go button currently accepts input. */
	bool canUseGoButton() const override;
	/** Wait for departure speech before continuing the saved-game route. */
	bool onGoButtonPressed() override;
	/** Advance the roster sprite animations after the actor pass. */
	void onActorsRendered() override;

private:
	/** Perfect and retreat Go speech selected after the player leaves Turtle Hurdle. */
	static constexpr const char *kGoSpeechFormat = "sounds/wld11.%d.wav";
	static constexpr const char *kRetreatSpeech = "sounds/DW-Zville.wav";
	/** Deferred Go transition flag. */
	bool _goPending = false;
	/** Music, turtle/mother/dock art, trait icons, area mask, and Zoombini animations. */
	static constexpr const char *kMusicPath = "#sounds/music/08-BS01.wav";
	static constexpr const char *kTurtleIdleFormat = "Bmp/crazy_turtle/TORTUES/ATTENTE/%d/%d.AN";
	static constexpr const char *kTurtleSpinFormat = "Bmp/crazy_turtle/TORTUES/tourbillonne/%d/%d.AN";
	static constexpr const char *kTurtleFixedFormat = "Bmp/crazy_turtle/TORTUES/tourbillonne/%d/FIXE%d.RB";
	static constexpr const char *kMotherPath = "Bmp/crazy_turtle/TORTUES/MERE/mere.an";
	static constexpr const char *kMotherSpeechPath = "Bmp/crazy_turtle/TORTUES/MERE/PARLE/PARLE.AN";
	static constexpr const char *kSmokePath = "Bmp/crazy_turtle/smokey.an";
	static constexpr const char *kMotherStartPath = "Bmp/crazy_turtle/TORTUES/MERE/meredebut.rb";
	static constexpr const char *kMotherEndPath = "Bmp/crazy_turtle/TORTUES/MERE/MEREFIN.RB";
	static constexpr const char *kBridgePath = "Bmp/crazy_turtle/PONT.RB";
	static constexpr const char *kCollapsedBridgePath = "Bmp/crazy_turtle/pontKC.rb";
	static constexpr const char *kBeamPath = "Bmp/crazy_turtle/poutrelle.rb";
	static constexpr const char *kTraitFormat = "Bmp/mystic_marsh/TRAITS/%d-%d.RB";
	static constexpr const char *kAreaMaskPath = "bmp/crazy_turtle/area.bmt";
	static constexpr const char *kIdleZombAnimationPath = "bmp/zombis/attente/attenteZomb.anm";
	static constexpr const char *kPickupZombAnimationPath = "bmp/zombis/pris/pris.anm";
	static constexpr const char *kTombeAnimationPath = "bmp/zombis/tombe/tombe.anm";
	/** Turtle, dock, placement, completion, and partial-completion audio resources. */
	static constexpr const char *kTurtleIdleSoundPath = "sounds/fx/08-BS02.wav";
	static constexpr const char *kSmokeSoundPath = "sounds/fx/08-BS03.wav";
	static constexpr const char *kTurtleSpinSoundPath = "sounds/fx/08-BS04.wav";
	static constexpr const char *kTransitionSoundPath = "sounds/fx/08-BS05.wav";
	static constexpr const char *kMismatchSoundPath = "sounds/fx/08-BS06.wav";
	static constexpr const char *kFallSoundPath = "sounds/fx/08-BS07.wav";
	static constexpr const char *kCollapseSoundPath = "sounds/fx/PierCollapse.wav";
	static constexpr const char *kMotherSuccessSpeechPath = "sounds/8-E1.wav";
	static constexpr const char *kMotherPartialSpeechPath = "sounds/8-E2.wav";

	/** Restore the dock background. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw dock condition, rule hints, turtles, and mother. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Draw the party in common depth order. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Turtle resource type and draw position. */
	struct TurtlePlacement {
		/** Index of the turtle animation and fixed visual. */
		int type;
		/** Screen position of the fixed visual. */
		Common::Point32 pos;
	};

	/** Number of visible Zoombini features. */
	static constexpr int kFeatureCount = 4;
	/** Number of values for each visible feature. */
	static constexpr int kFeatureValueCount = 5;
	/** Number of simultaneous ordering-rule groups. */
	static constexpr int kRuleGroupCount = 2;
	/** Number of positions in each ordering rule. */
	static constexpr int kRuleSlotCount = 5;
	/** Number of turtles spanning the dock. */
	static constexpr int kTurtleCount = 16;
	/** Maximum party size used by this puzzle. */
	static constexpr int kZoombiniCount = 16;

	/** Turtle types and screen positions. */
	static constexpr TurtlePlacement kTurtlePlacements[kTurtleCount] = {
		{3, Common::Point32(0, 322)},
		{3, Common::Point32(8, 383)},
		{3, Common::Point32(29, 448)},
		{4, Common::Point32(104, 482)},
		{4, Common::Point32(178, 463)},
		{2, Common::Point32(264, 444)},
		{1, Common::Point32(245, 378)},
		{3, Common::Point32(233, 308)},
		{4, Common::Point32(248, 238)},
		{4, Common::Point32(325, 196)},
		{2, Common::Point32(412, 175)},
		{1, Common::Point32(492, 165)},
		{3, Common::Point32(576, 172)},
		{2, Common::Point32(548, 245)},
		{2, Common::Point32(461, 301)},
		{3, Common::Point32(529, 346)},
	};
	/** Screen positions used for Zoombinis standing on turtles. */
	static constexpr Common::Point32 kZoombiniPos[kZoombiniCount] = {
		Common::Point32(247, 120),
		Common::Point32(245, 74),
		Common::Point32(202, 117),
		Common::Point32(200, 64),
		Common::Point32(166, 42),
		Common::Point32(166, 87),
		Common::Point32(146, 131),
		Common::Point32(99, 120),
		Common::Point32(131, 75),
		Common::Point32(128, 30),
		Common::Point32(86, 66),
		Common::Point32(62, 105),
		Common::Point32(49, 48),
		Common::Point32(18, 135),
		Common::Point32(25, 91),
		Common::Point32(8, 55),
	};
	/** Draw positions for primary-rule feature icons. */
	static constexpr Common::Point32 kPrimaryIconPos[kRuleSlotCount] = {
		Common::Point32(365, 15),
		Common::Point32(395, 22),
		Common::Point32(423, 28),
		Common::Point32(450, 35),
		Common::Point32(477, 42),
	};
	/** Draw positions for secondary-rule feature icons. */
	static constexpr Common::Point32 kSecondaryIconPos[kRuleSlotCount] = {
		Common::Point32(365, 40),
		Common::Point32(395, 47),
		Common::Point32(423, 54),
		Common::Point32(450, 60),
		Common::Point32(477, 67),
	};
	/** Fall-back landing positions indexed by turtle. */
	static constexpr Common::Point32 kLandingPos[kTurtleCount] = {
		Common::Point32(26, 327),
		Common::Point32(36, 392),
		Common::Point32(56, 451),
		Common::Point32(125, 498),
		Common::Point32(199, 475),
		Common::Point32(288, 453),
		Common::Point32(263, 392),
		Common::Point32(260, 317),
		Common::Point32(268, 254),
		Common::Point32(346, 209),
		Common::Point32(435, 181),
		Common::Point32(510, 182),
		Common::Point32(603, 179),
		Common::Point32(573, 251),
		Common::Point32(484, 306),
		Common::Point32(558, 352),
	};
	/** Delay before a mismatched placement reacts. */
	static constexpr uint32 kFallReactionDelayMs = 1200;
	/** Frames in one turtle spin cycle. */
	static constexpr int kTurtleSpinFrameCount = 11;
	/** Per-frame turtle spin duration in milliseconds. */
	static constexpr uint32 kTurtleSpinFrameDelayMs = 120;
	/** Frames in one dock-damage smoke cycle. */
	static constexpr int kSmokeFrameCount = 6;
	/** Per-frame smoke duration in milliseconds. */
	static constexpr uint32 kSmokeFrameDelayMs = 130;
	/** Frames in the mother turtle's completion animation. */
	static constexpr int kMotherFrameCount = 10;
	/** Per-frame mother turtle duration in milliseconds. */
	static constexpr uint32 kMotherFrameDelayMs = 110;
	/** Duration of the final held mother turtle frame in milliseconds. */
	static constexpr uint32 kMotherFinalHoldMs = 500;
	/** Draw position of the mother turtle. */
	static constexpr Common::Point32 kMotherPos = Common::Point32(488, 310);
	/** Draw position of the dock-damage smoke effect. */
	static constexpr Common::Point32 kSmokePos = Common::Point32(30, 235);

	/** Load an animation from @p path and report whether it succeeded. */
	bool loadAnimationResource(Animation *&resource, const Common::Path &path);
	/** Preload an RLE sprite from @p path and report whether it succeeded. */
	bool loadRleResource(const Common::String &path);

	/** Load all turtle, dock, mother, and trait resources. */
	void loadResources();
	/** Load the Zoombini interaction grids, area mask, sounds, and runners. */
	void loadInteractionResources();
	/** Register one drop target per turtle. */
	void buildDropTargets();
	/** Select active features and generate their ordering rules. */
	void generateRules();
	/** Generate the value order for rule @p group. */
	void generateFeatureOrder(int group);
	/** Enable @p count random entries in @p slots. */
	void activateRandomRuleSlots(bool *slots, int count);
	/** Position the puzzle roster before interaction begins. */
	void placeZoombinis();
	/** Assign each turtle the party Zoombini the rule ordering requires. */
	void buildTurtleAssignments();
	/** Return whether @p zoombini satisfies @p turtleIndex's requirement. */
	bool evaluateTurtleMatch(const ZoombiniRunner *zoombini, int turtleIndex) const;
	/** Handle a Zoombini released over @p turtleIndex. */
	void handleTurtleClick(int turtleIndex, int zoombiniIndex);
	/** Start the mother turtle, departure sound, and idle walking phase. */
	void startTransitionSequence();
	/** Play one-shot sound @p soundId at the current SFX volume. */
	void playSound(int soundId) const;
	/** Count the party entries not yet placed on a turtle. */
	int countFreeZoombinis() const;
	/** Advance the mismatch fall state machine. */
	void updateFallSequence(uint32 tick);
	/** Start a random idle turtle spin when the page is otherwise quiet. */
	void updateIdleTurtleSpin(uint32 tick);
	/** Schedule roster sprite frame advances for @p tick. */
	void updateZoombiniAnimations(uint32 tick);
	/** Return whether @p turtleIndex currently has an active spin runner. */
	bool hasActiveTurtleRunner(int turtleIndex) const;
	/** Draw the active turtle and smoke animation runners. */
	void drawTurtleRunners(ManagedSurface32 *screen) const;
	/** Drop-target callback forwarding to @ref PuzzleCrazyTurtle::handleTurtleClick. */
	static void onTurtleDrop(void *context, int targetIndex, int zoombiniIndex);
	/** Matched-placement completion: settle and check the full board. */
	static void onWalkComplete(void *context, ZoombiniRunner *zoombini);
	/** Fall-animation completion: replay or finish the fall sequence. */
	static void onTurtleFallComplete(void *context, ZoombiniRunner *zoombini);
	/** Idle turtle spin completion: clear the active spin index. */
	static void onTurtleIdleSpinComplete(void *context, AnimationRunner *runner);
	/** Reaction turtle spin completion: mirror the original ready flag. */
	static void onTurtleSpinComplete(void *context, AnimationRunner *runner);
	/** Show the retracted mother turtle and play the completion speech. */
	static void onMotherAnimationComplete(void *context, AnimationRunner *runner);
	/** Draw the intact or damaged dock for the mirrored mistake count. */
	void drawBridgeState(ManagedSurface32 *screen);
	/** Draw the active feature-order hints. */
	void drawRuleHints(ManagedSurface32 *screen) const;
	/** Draw each fixed turtle visual, skipping any active spin runner. */
	void drawTurtles(ManagedSurface32 *screen) const;
	/** Draw the mother turtle and feedback effects. */
	void drawMother(ManagedSurface32 *screen) const;

	/** Level in the range one through four. */
	int _level = 1;
	/** Feature index used by the primary ordering rule. */
	int _primaryFeature = 0;
	/** Feature index used by the secondary ordering rule. */
	int _secondaryFeature = 1;
	/** Feature-value ordering for each rule group. */
	int _ruleValues[kRuleGroupCount][kRuleSlotCount] = {};
	/** Active hint positions in the primary rule. */
	bool _primaryRuleActive[kRuleSlotCount] = {};
	/** Active hint positions in the secondary rule. */
	bool _secondaryRuleActive[kRuleSlotCount] = {};
	/** Incorrect placements still allowed before the dock collapses. */
	int _remainingMistakes = 0;
	/** Initial incorrect-placement allowance for the selected level. */
	int _initialMistakes = 0;

	/** Idle turtle animations indexed by turtle type. */
	Animation *_turtleIdleAnimations[kFeatureCount] = {};
	/** Feedback turtle animations indexed by turtle type. */
	Animation *_turtleSpinAnimations[kFeatureCount] = {};
	/** Mother turtle animation. */
	Animation *_motherAnimation = nullptr;
	/** Mother turtle completion runner. */
	AnimationRunner *_motherRunner = nullptr;
	/** Mother turtle speech animation. */
	Animation *_motherSpeechAnimation = nullptr;
	/** Dock-damage smoke animation. */
	Animation *_smokeAnimation = nullptr;
	/** Whether the mother turtle has completed its neck animation. */
	bool _motherFinished = false;
	/** Zoombini grid for walking in place on a settled turtle. */
	const ZoombiniAnimation *_idleZombAnimation = nullptr;
	/** Zoombini grid used while a Zoombini is held. */
	const ZoombiniAnimation *_pickupZombAnimation = nullptr;
	/** Zoombini grid used while a Zoombini falls back to the dock. */
	const ZoombiniAnimation *_tombeAnimation = nullptr;
	/** One drop target per turtle. */
	Common::Array<ZmbDropTarget> _turtleDropTargets;
	/** Party index assigned to each turtle by the rule ordering. */
	int _turtleAssignments[kTurtleCount] = {};
	/** Input locked while a placement or fall sequence runs. */
	bool _inputLocked = false;
	/** Departure sequence start pending. */
	bool _transitionRequested = false;
	/** Placed Zoombinis walk in place once the departure starts. */
	bool _idlePhaseEnabled = false;
	/** Turtle playing the idle spin. -1 means none. */
	int _idleTurtleIndex = -1;
	/** Set when the active turtle animation completes. */
	bool _turtleReady = false;
	/** Fallback turtle for the Zoombini currently falling. */
	int _fallbackTurtleIndex = -1;
	/** Party index of the Zoombini in the fall sequence. */
	int _fallZoombiniIndex = -1;
	/** Fall sequence phase: -1 inactive, 0 waiting, 1 rising, 2 descending. */
	int _fallPhase = -1;
	/** Tick at which the mismatch reaction starts. */
	uint32 _fallTimerTick = 0xFFFFFFFFU;
	/** Turtle clicked on the latest mismatch. */
	int _activeTurtleIndex = -1;
	/** Self-rescheduling count for the fall animation. */
	int _fallCounter = 0;
	/** Party index of the last Zoombini that fell. */
	int _lastFallZoombiniIndex = -1;
	/** Mistake count used for the dock redraw, refreshed after each fall. */
	int _mistakesMirror = 0;
	/** Dock-damage smoke pending after the latest fall completes. */
	bool _smokePending = false;
	/** Idle-spin turtle runners indexed by turtle type. */
	AnimationRunner *_turtleIdleRunners[kFeatureCount] = {};
	/** Reaction-spin turtle runners indexed by turtle type. */
	AnimationRunner *_turtleSpinRunners[kFeatureCount] = {};
	/** Dock-damage smoke runner. */
	AnimationRunner *_smokeRunner = nullptr;
	/** One-shot sound: idle turtle spin start. */
	int _sndTurtleIdle = -1;
	/** One-shot sound: dock-damage smoke. */
	int _sndSmoke = -1;
	/** One-shot sound: mismatch reaction spin start. */
	int _sndTurtleSpin = -1;
	/** One-shot sound: departure sequence start. */
	int _sndTransition = -1;
	/** One-shot sound: mismatched placement. */
	int _sndMismatch = -1;
	/** One-shot sound: fall animation start. */
	int _sndFall = -1;
	/** One-shot sound: dock collapse. */
	int _sndCollapse = -1;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_CRAZYTURTLE_H
