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

#ifndef ZOOMBINI2_PAGES_PUZZLE_WALLOFFLEENS_H
#define ZOOMBINI2_PAGES_PUZZLE_WALLOFFLEENS_H

#include "zoombini2/pages/puzzle_base.h"
#include "zoombini2/scripts.h"
#include "zoombini2/state.h"

namespace Zoombini2 {

/** Find hidden Fleens by comparing the four traits reported by each mirror. */
class PuzzleWallOfFleens : public PuzzleBase {
public:
	/** Construct Magic Mirrors for @p vm. */
	PuzzleWallOfFleens(Zoombini2Engine *vm);
	/** Release projectile/rail paths and page-local animation resources. */
	~PuzzleWallOfFleens() override;
	/** Return the puzzle name used by debug answer descriptions. */
	const char *getPageName() const override { return kPageName; }
	/** Return the 1-based route containing this puzzle. */
	int getRouteNumber() const override { return kRouteNumber; }
	/** Return the 1-based puzzle number within its route. */
	int getRoutePuzzleIdx() const override { return kRoutePuzzleIdx; }
	/** Generate the selected board, load resources, and arrange the puzzle party. */
	void init() override;
	/** Advance cannon loading, projectile flight, mirror reactions, and speech. */
	void onUpdate() override;
	/** Restore the background before drawing the mirrors and cannon. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Draw grid mirrors, trait-score panels, cannon, and projectile rail. */
	void onRenderContent(ManagedSurface32 *screen) override;
	/** Draw a Fleen or loaded Zoombini at the cannon. */
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Draw projectile and mirror reaction effects above the board. */
	void onRenderForeground(ManagedSurface32 *screen) override;
	/** Advance the party, Fleen and projectile animation frames after drawing. */
	void onActorsRendered() override;
	/** Fire the cannon or select a target mirror after a button release at @p pos. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Select the interactive cursor over the active mirror grid. */
	EventHandleResult onMouseMove(const Common::Point &pos) override;
	/** Complete the current board for the puzzle-console command. */
	void applyDebugPuzzleCompletion() override;
	/** Return whether the engine's walking/departure gate currently enables Go. */
	bool canUseGoButton() const override;
	/** Return through retreat speech or the completed map transition. */
	bool onGoButtonPressed() override;
	/** Describe each target Fleen and its mirror status for the puzzle console. */
	Common::String debugGetAnswer() const override;
	/** Report remaining cannonballs as the puzzle's chance count. */
	PuzzleChanceInfo debugGetChances() const override;
	/** Explain cannonball exhaustion and the resulting loss of party members. */
	Common::String debugGetChanceDetails() const override;
	/** Return whether the current shot state permits changing the cannonball count. */
	bool debugCanSetChances() const override;
	/** Set remaining cannonballs and synchronize the loaded projectile and rail display. */
	bool debugSetChances(int remaining) override;

private:
	/** Static puzzle name used by debug answer descriptions. */
	static constexpr const char *kPageName = "Magic Mirrors";
	/** 1-based route number used by the puzzle console. */
	static constexpr int kRouteNumber = 3;
	/** 1-based puzzle number within the route used by the puzzle console. */
	static constexpr int kRoutePuzzleIdx = 2;
	/** Magic Mirrors board background loaded into the first page layer. */
	static constexpr const char *kBackgroundPath = "#bmp/wall_of_fleens/background";
	/** Loading and projectile movement run independently of mirror reactions. */
	enum class ShotPhase {
		/** Load the next cannonball or Zoombini projectile. */
		kLoading00 = 0,
		/** Cannon is loaded and accepts a mirror click. */
		kReady01 = 1,
		/** Rotate the muzzle toward the selected mirror. */
		kAiming02 = 2,
		/** Projectile follows its cannon-to-mirror rail. */
		kOutbound03 = 3,
		/** Projectile returns after a non-target mirror result. */
		kRebound04 = 4,
		/** No projectile is active. */
		kStopped05 = 5
	};
	/** Independent visual reaction state for the currently hit mirror. */
	enum class MirrorPhase {
		/** No mirror effect is active. */
		kNone00 = 0,
		/** Reveal or rotate a scored mirror. */
		kRotating01 = 1,
		/** Remove a discovered Fleen mirror. */
		kExploding02 = 2,
		/** Play Fleen shouting feedback after a hit. */
		kShouting03 = 3,
		/** Move a found Fleen away from the board. */
		kLeaving04 = 4
	};
	/** Discrete cannon aim positions from the right edge through center to the left edge. */
	enum class CannonAimAngle : int {
		kRight00 = 0,
		kUpRight01 = 1,
		kUpRight02 = 2,
		kUpRight03 = 3,
		kCenter04 = 4,
		kUpLeft05 = 5,
		kUpLeft06 = 6,
		kUpLeft07 = 7,
		kLeft08 = 8
	};
	/** Select the score digits displayed for a mirror. */
	enum ScoreMask {
		/** No score is being displayed. */
		kScoreNone00 = 0,
		/** Score to the primary actual fleen, always active */
		kShowPrimaryScore01 = 1,
		/** Score to the secondary actual fleen, only active in level 4 */
		kShowSecondaryScore02 = 2
	};
	/** Fleen feature values share the renderer's numeric trait layout but use Fleen names. */
	struct FleenTrait : public ZmbTrait {
		/** Return the diagnostic value name for one Fleen trait. */
		static const char *debugTraitValueName(ZmbTrait::TraitKind index, int value);
		/** Return Fleen feature names in hair, eyes, nose, feet order. */
		Common::String toStr() const;
	};
	struct MirrorCell {
		/** Fleen traits compared with the current target's traits. */
		FleenTrait traits;
		/** Mirror cell's top-left anchor in page pixels. */
		Common::Point32 pos;
		/** Trait-match feedback score, or -1 before a shot is evaluated. */
		int score = -1;
		/** Whether this mirror cell has been tested by a shot. */
		bool tried = false;
		/** Whether this mirror's hidden contents are currently visible. */
		bool revealed = false;
		/** Whether this mirror now shows an empty passage. */
		bool empty = false;
		/** Bitmask selecting the primary and secondary scores shown on this mirror. */
		byte scoreMask = kShowPrimaryScore01;
	};
	/** Sprite-grid cell used when a Fleen departs. */
	static constexpr int kFleenDepartureCell = 56;
	/** Fleen nose value whose departure artwork needs the streak correction. */
	static constexpr int kWhiteNoseTrait = 2;
	/** First departure frame containing the stray nose-art streak. */
	static constexpr int kStreakFirstFrame = 2;
	/** Last departure frame containing the stray nose-art streak. */
	static constexpr int kStreakLastFrame = 5;
	/** Background music played while the Magic Mirrors board is active. */
	static constexpr const char *kMusicPath = "#sounds/music/05-BB01.wav";
	/** Cannon muzzle frames, selected from the current aim angle. */
	static constexpr const char *kCannonFormat = "bmp/wall_of_fleens/canon0%d";
	/** Mirror art: gray/normal for hidden cells, black/cracked for revealed cells, and an empty tunnel. */
	static constexpr const char *kMirrorPaths[5] = {
		"bmp/wall_of_fleens/mirror_GRIS",
		"bmp/wall_of_fleens/mirror_nomal",
		"bmp/wall_of_fleens/mirror_NOIR",
		"bmp/wall_of_fleens/mirror_felure",
		"bmp/wall_of_fleens/mirror_empty_tunnel",
	};
	/** Cannonball art: the loaded ball followed by five direction-specific projectile images. */
	static constexpr const char *kBallPaths[6] = {
		"bmp/wall_of_fleens/boulet/boulet",
		"bmp/wall_of_fleens/boulet/boulet4",
		"bmp/wall_of_fleens/boulet/boulet6",
		"bmp/wall_of_fleens/boulet/boulet7",
		"bmp/wall_of_fleens/boulet/boulet8",
		"bmp/wall_of_fleens/boulet/boulet9",
	};
	/** Foreground overlays for the cannon nozzle and cannon body, in that order. */
	static constexpr const char *kOverlayPaths[2] = {
		"bmp/wall_of_fleens/tuyere",
		"bmp/wall_of_fleens/canon_cache",
	};
	/** Red score digit sprites drawn over a revealed mirror; the format value is one digit. */
	static constexpr const char *kScoreFormat = "bmp/wall_of_fleens/LevelRED%d";
	/** Animation played when a mirror rotates to reveal its score. */
	static constexpr const char *kRotatePath = "bmp/wall_of_fleens/mirror_rotate";
	/** Animation played when a mirror containing a Fleen is destroyed. */
	static constexpr const char *kExplodePath = "bmp/wall_of_fleens/mirror_explode";
	/** PAT path that carries a cannonball from the loading rail into the cannon. */
	static constexpr const char *kBallLoadPath = "bmp/wall_of_fleens/bullet_on_cannon.pat";
	/** PAT path that carries the final Zoombini from the loading rail into the cannon. */
	static constexpr const char *kJumpPath = "bmp/wall_of_fleens/jump_in_cannon.pat";
	/** Fleen sprite grid used to render Fleens in the mirror catch animation. */
	static constexpr const char *kFleenPath = "bmp/fleens/fleens.anm";
	/** First Fleen shouting animation grid used after a mirror hit. */
	static constexpr const char *kVocif1Path = "bmp/fleens/vocif/vocif1.anm";
	/** Follow-up Fleen shouting animation grid used after the first shout. */
	static constexpr const char *kVocif2Path = "bmp/fleens/vocif/vocif2.anm";
	/** Zoombini animation grid for the runner loaded into the cannon. */
	static constexpr const char *kCannonZombPath = "bmp/wall_of_fleens/cannon_zomb/cannon_zomb.anm";
	/** Sound effects in order: cannon aim, mirror rotation, reload, runner load, firing, impact, explosion, Fleen departure. */
	static constexpr const char *kSoundPaths[8] = {
		"sounds/fx/05-BB03.wav",
		"sounds/fx/05-BB04.wav",
		"sounds/fx/05-BS01.wav",
		"sounds/fx/05-BS02.wav",
		"sounds/fx/05-BS03.wav",
		"sounds/fx/05-BS04.wav",
		"sounds/fx/05-BS05.wav",
		"sounds/fx/FleenLeavesWall.wav",
	};
	/** Completion speech used when no Zoombini was lost on this board. */
	static constexpr const char *kSuccessSpeechPath = "sounds/8-E1.wav";
	/** Completion speech used when at least one Zoombini was lost on this board. */
	static constexpr const char *kLossSpeechPath = "sounds/8-E2.wav";
	/** Retreat speech variants selected for the final Zoombini, indexed by retreat outcome. */
	static constexpr const char *kRetreatSpeechPaths[2] = {
		"sounds/wlf15.1.wav",
		"sounds/wlf15.3.wav",
	};
	/** Random Fleen idle voice format; the argument is the voice variant number. */
	static constexpr const char *kAmbientFormat = "sounds/wlf12.%d.wav";
	/** Perfect-clear Go-button speech format; the argument is the voice variant number. */
	static constexpr const char *kPerfectGoFormat = "sounds/wld11.%d.wav";
	/** Level-specific score-panel/grid origins and cannon muzzle positions. */
	static constexpr Common::Point32 kPanelOrigins[6] = {
		{182, 88},
		{344, 88},
		{506, 87},
		{183, 230},
		{345, 229},
		{508, 230},
	};
	/** Page-space mirror-grid origins indexed by board variant. */
	static constexpr Common::Point32 kGridOrigins[5] = {
		{0, 0},
		{380, 200},
		{200, 7},
		{100, 7},
		{95, 7},
	};
	/** Projectile launch anchors in page pixels, indexed by cannon aim angle. */
	static constexpr Common::Point32 kMuzzles[9] = {
		{500, 432},
		{500, 432},
		{480, 421},
		{460, 418},
		{441, 404},
		{415, 425},
		{385, 428},
		{379, 459},
		{379, 459},
	};
	/** Original easy-mode trait pattern corpus, indexed by pattern, cell, and trait. */
	static constexpr byte kL1Patterns[123][5][4] = {
		{
			{0, 1, 1, 1},
			{0, 0, 0, 1},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
		},
		{
			{0, 0, 0, 1},
			{0, 1, 1, 1},
			{1, 0, 1, 1},
			{1, 1, 0, 1},
			{1, 1, 1, 0},
		},
		{
			{1, 0, 0, 0},
			{0, 2, 2, 2},
			{1, 0, 1, 1},
			{1, 1, 0, 1},
			{1, 1, 1, 0},
		},
		{
			{0, 1, 0, 0},
			{0, 1, 1, 1},
			{0, 1, 2, 2},
			{1, 0, 1, 1},
			{1, 0, 2, 2},
		},
		{
			{0, 1, 0, 0},
			{0, 1, 1, 2},
			{0, 1, 2, 1},
			{1, 0, 1, 2},
			{1, 0, 2, 1},
		},
		{
			{1, 1, 1, 1},
			{0, 0, 0, 1},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
		},
		{
			{0, 0, 2, 2},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 0, 1},
			{1, 1, 1, 0},
		},
		{
			{1, 1, 2, 2},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 0, 1},
			{1, 1, 1, 0},
		},
		{
			{0, 0, 1, 2},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 0, 1},
			{1, 1, 2, 0},
		},
		{
			{1, 1, 1, 2},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 0, 1},
			{1, 1, 2, 0},
		},
		{
			{0, 0, 1, 1},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 0, 2},
			{1, 1, 2, 0},
		},
		{
			{1, 1, 1, 1},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 0, 2},
			{1, 1, 2, 0},
		},
		{
			{0, 0, 2, 1},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 0, 2},
			{1, 1, 1, 0},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 1},
			{0, 0, 2, 1},
			{0, 1, 0, 2},
			{0, 2, 0, 2},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 2},
			{0, 1, 0, 1},
			{1, 1, 2, 2},
			{1, 2, 1, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 2},
			{0, 1, 2, 0},
			{0, 2, 0, 1},
			{1, 1, 1, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 2, 1},
			{0, 1, 0, 2},
			{0, 2, 1, 0},
			{1, 1, 1, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 1},
			{0, 1, 0, 2},
			{1, 1, 2, 1},
			{1, 2, 1, 2},
		},
		{
			{0, 1, 0, 0},
			{1, 1, 0, 1},
			{1, 1, 0, 2},
			{1, 1, 1, 0},
			{1, 1, 2, 0},
		},
		{
			{1, 0, 0, 0},
			{0, 1, 1, 1},
			{0, 2, 1, 1},
			{1, 0, 1, 1},
			{1, 0, 2, 2},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 2},
			{0, 0, 2, 2},
			{0, 1, 0, 1},
			{0, 2, 0, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 1, 1, 1},
			{0, 1, 2, 2},
			{0, 2, 1, 2},
			{0, 2, 2, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 1, 1, 2},
			{0, 1, 2, 1},
			{0, 2, 1, 1},
			{0, 2, 2, 2},
		},
		{
			{1, 1, 2, 1},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 0, 2},
			{1, 1, 1, 0},
		},
		{
			{1, 0, 1, 1},
			{0, 0, 1, 1},
			{0, 1, 2, 0},
			{0, 2, 0, 2},
			{1, 1, 1, 2},
		},
		{
			{1, 0, 1, 2},
			{0, 0, 1, 2},
			{0, 1, 2, 0},
			{0, 2, 0, 1},
			{1, 1, 1, 1},
		},
		{
			{1, 0, 1, 1},
			{0, 0, 1, 1},
			{0, 1, 0, 2},
			{1, 1, 2, 1},
			{1, 2, 1, 2},
		},
		{
			{1, 1, 1, 1},
			{0, 0, 2, 1},
			{0, 2, 1, 0},
			{1, 0, 0, 2},
			{2, 1, 0, 0},
		},
		{
			{1, 2, 0, 2},
			{0, 0, 2, 1},
			{0, 1, 1, 0},
			{1, 1, 2, 2},
			{1, 2, 1, 1},
		},
		{
			{1, 1, 0, 1},
			{0, 0, 1, 2},
			{0, 2, 2, 0},
			{1, 1, 2, 2},
			{1, 2, 1, 1},
		},
		{
			{0, 0, 1, 1},
			{0, 2, 1, 1},
			{1, 1, 0, 2},
			{1, 1, 2, 0},
			{2, 0, 1, 1},
		},
		{
			{1, 1, 1, 1},
			{0, 0, 1, 2},
			{0, 1, 2, 0},
			{1, 2, 0, 0},
			{2, 0, 0, 1},
		},
		{
			{1, 2, 1, 0},
			{0, 0, 2, 2},
			{0, 1, 0, 1},
			{1, 1, 1, 2},
			{1, 2, 2, 1},
		},
		{
			{1, 0, 1, 2},
			{0, 1, 0, 1},
			{0, 2, 2, 0},
			{1, 1, 2, 2},
			{1, 2, 1, 1},
		},
		{
			{1, 1, 0, 2},
			{0, 0, 1, 1},
			{0, 2, 2, 0},
			{1, 1, 2, 1},
			{1, 2, 1, 2},
		},
		{
			{1, 0, 2, 2},
			{0, 1, 1, 0},
			{0, 2, 0, 1},
			{1, 1, 2, 1},
			{1, 2, 1, 2},
		},
		{
			{1, 0, 2, 1},
			{0, 1, 1, 0},
			{0, 2, 0, 2},
			{1, 1, 2, 2},
			{1, 2, 1, 1},
		},
		{
			{1, 0, 1, 1},
			{0, 1, 0, 2},
			{0, 2, 2, 0},
			{1, 1, 2, 1},
			{1, 2, 1, 2},
		},
		{
			{2, 2, 0, 0},
			{0, 1, 1, 1},
			{0, 1, 2, 2},
			{1, 0, 1, 1},
			{1, 0, 2, 2},
		},
		{
			{1, 0, 2, 2},
			{0, 1, 0, 1},
			{0, 2, 1, 0},
			{1, 1, 1, 2},
			{1, 2, 2, 1},
		},
		{
			{0, 0, 1, 1},
			{1, 1, 1, 2},
			{1, 2, 2, 1},
			{2, 1, 2, 1},
			{2, 2, 1, 2},
		},
		{
			{2, 2, 0, 0},
			{0, 1, 1, 2},
			{0, 1, 2, 1},
			{1, 0, 1, 2},
			{1, 0, 2, 1},
		},
		{
			{1, 1, 0, 1},
			{0, 0, 2, 2},
			{0, 2, 1, 0},
			{1, 1, 1, 2},
			{1, 2, 2, 1},
		},
		{
			{1, 2, 0, 2},
			{0, 0, 1, 1},
			{0, 1, 2, 0},
			{1, 1, 1, 2},
			{1, 2, 2, 1},
		},
		{
			{1, 1, 0, 2},
			{0, 0, 2, 1},
			{0, 2, 1, 0},
			{1, 1, 1, 1},
			{1, 2, 2, 2},
		},
		{
			{1, 2, 0, 1},
			{0, 0, 1, 2},
			{0, 1, 2, 0},
			{1, 1, 1, 1},
			{1, 2, 2, 2},
		},
		{
			{1, 2, 1, 0},
			{0, 0, 2, 1},
			{0, 1, 0, 2},
			{1, 1, 1, 1},
			{1, 2, 2, 2},
		},
		{
			{1, 0, 1, 2},
			{0, 1, 2, 0},
			{0, 2, 0, 1},
			{1, 1, 1, 1},
			{1, 2, 2, 2},
		},
		{
			{1, 1, 1, 0},
			{0, 0, 2, 2},
			{0, 2, 0, 1},
			{1, 1, 2, 1},
			{1, 2, 1, 2},
		},
		{
			{1, 0, 2, 1},
			{0, 1, 0, 2},
			{0, 2, 1, 0},
			{1, 1, 1, 1},
			{1, 2, 2, 2},
		},
		{
			{1, 0, 1, 1},
			{0, 1, 2, 0},
			{0, 2, 0, 2},
			{1, 1, 1, 2},
			{1, 2, 2, 1},
		},
		{
			{1, 2, 0, 1},
			{0, 0, 2, 2},
			{0, 1, 1, 0},
			{1, 1, 2, 1},
			{1, 2, 1, 2},
		},
		{
			{1, 1, 1, 0},
			{0, 0, 2, 1},
			{0, 2, 0, 2},
			{1, 1, 2, 2},
			{1, 2, 1, 1},
		},
		{
			{1, 2, 2, 0},
			{0, 0, 1, 2},
			{0, 1, 0, 1},
			{1, 1, 2, 2},
			{1, 2, 1, 1},
		},
		{
			{0, 0, 1, 1},
			{1, 1, 2, 1},
			{1, 2, 1, 2},
			{2, 1, 1, 2},
			{2, 2, 2, 1},
		},
		{
			{1, 1, 2, 0},
			{0, 0, 1, 2},
			{0, 2, 0, 1},
			{1, 1, 1, 1},
			{1, 2, 2, 2},
		},
		{
			{1, 2, 2, 0},
			{0, 0, 1, 1},
			{0, 1, 0, 2},
			{1, 1, 2, 1},
			{1, 2, 1, 2},
		},
		{
			{1, 1, 2, 0},
			{0, 0, 1, 1},
			{0, 2, 0, 2},
			{1, 1, 1, 2},
			{1, 2, 2, 1},
		},
		{
			{1, 1, 1, 1},
			{0, 1, 1, 2},
			{0, 1, 2, 1},
			{1, 0, 1, 2},
			{1, 0, 2, 1},
		},
		{
			{1, 1, 1, 1},
			{0, 2, 1, 1},
			{1, 1, 0, 2},
			{1, 1, 2, 0},
			{2, 0, 1, 1},
		},
		{
			{1, 1, 1, 1},
			{0, 1, 2, 1},
			{1, 0, 1, 2},
			{1, 2, 1, 0},
			{2, 1, 0, 1},
		},
		{
			{1, 1, 1, 1},
			{0, 1, 1, 2},
			{1, 0, 2, 1},
			{1, 2, 0, 1},
			{2, 1, 1, 0},
		},
		{
			{0, 1, 1, 1},
			{0, 0, 0, 1},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{1, 1, 1, 1},
		},
		{
			{0, 1, 1, 1},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 1, 2},
		},
		{
			{0, 1, 1, 2},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 1, 1},
		},
		{
			{1, 1, 1, 2},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{0, 1, 1, 1},
			{1, 1, 1, 0},
		},
		{
			{1, 1, 1, 2},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{0, 1, 1, 1},
			{1, 0, 1, 1},
		},
		{
			{1, 1, 1, 1},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{0, 1, 1, 2},
			{1, 1, 1, 0},
		},
		{
			{0, 0, 1, 1},
			{0, 2, 1, 1},
			{1, 0, 0, 0},
			{1, 1, 0, 2},
			{1, 1, 2, 0},
		},
		{
			{0, 1, 0, 1},
			{0, 1, 2, 1},
			{1, 0, 0, 0},
			{1, 0, 1, 2},
			{1, 2, 1, 0},
		},
		{
			{0, 1, 0, 0},
			{0, 1, 1, 1},
			{0, 1, 2, 2},
			{1, 0, 1, 1},
			{1, 1, 1, 2},
		},
		{
			{0, 1, 0, 0},
			{0, 1, 1, 2},
			{0, 1, 2, 1},
			{1, 0, 1, 2},
			{1, 1, 1, 1},
		},
		{
			{0, 0, 1, 0},
			{0, 1, 1, 1},
			{1, 0, 1, 1},
			{1, 1, 1, 0},
			{1, 1, 1, 2},
		},
		{
			{0, 0, 1, 0},
			{0, 1, 1, 1},
			{1, 0, 1, 1},
			{1, 1, 0, 1},
			{1, 1, 1, 2},
		},
		{
			{1, 0, 0, 0},
			{0, 1, 1, 2},
			{1, 0, 2, 1},
			{1, 1, 1, 1},
			{1, 2, 0, 1},
		},
		{
			{0, 1, 1, 0},
			{0, 1, 1, 2},
			{1, 0, 0, 0},
			{1, 0, 2, 1},
			{1, 2, 0, 1},
		},
		{
			{1, 1, 1, 2},
			{0, 1, 1, 1},
			{1, 0, 1, 1},
			{1, 1, 0, 1},
			{1, 1, 1, 0},
		},
		{
			{0, 1, 1, 1},
			{0, 0, 0, 1},
			{0, 0, 1, 0},
			{1, 0, 0, 0},
			{1, 1, 1, 1},
		},
		{
			{1, 0, 1, 1},
			{0, 0, 0, 1},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{1, 1, 1, 1},
		},
		{
			{1, 1, 1, 1},
			{0, 0, 0, 1},
			{0, 0, 1, 0},
			{0, 1, 1, 1},
			{1, 0, 1, 1},
		},
		{
			{1, 1, 1, 2},
			{0, 0, 1, 0},
			{0, 1, 1, 1},
			{1, 0, 0, 0},
			{1, 1, 1, 0},
		},
		{
			{1, 1, 1, 1},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{1, 0, 1, 2},
			{1, 1, 1, 0},
		},
		{
			{1, 1, 0, 2},
			{0, 0, 1, 1},
			{0, 0, 2, 1},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
		},
		{
			{0, 0, 1, 1},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 0, 2},
			{1, 1, 2, 1},
		},
		{
			{0, 0, 2, 1},
			{0, 1, 0, 0},
			{1, 0, 0, 0},
			{1, 1, 0, 2},
			{1, 1, 1, 1},
		},
		{
			{1, 1, 1, 1},
			{0, 0, 1, 0},
			{0, 1, 1, 2},
			{1, 0, 0, 0},
			{1, 1, 1, 0},
		},
		{
			{0, 0, 1, 1},
			{0, 1, 0, 0},
			{0, 1, 1, 1},
			{1, 0, 0, 0},
			{1, 0, 1, 1},
		},
		{
			{1, 1, 1, 2},
			{0, 1, 0, 0},
			{0, 1, 1, 1},
			{1, 0, 0, 0},
			{1, 0, 1, 1},
		},
		{
			{1, 1, 1, 1},
			{0, 1, 0, 0},
			{0, 1, 1, 2},
			{0, 1, 2, 1},
			{1, 0, 0, 0},
		},
		{
			{1, 1, 1, 1},
			{0, 1, 0, 0},
			{0, 1, 1, 2},
			{1, 0, 0, 0},
			{1, 0, 2, 1},
		},
		{
			{1, 1, 1, 2},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{1, 0, 1, 1},
			{1, 1, 1, 0},
		},
		{
			{1, 1, 1, 1},
			{0, 1, 0, 0},
			{0, 1, 2, 1},
			{1, 0, 0, 0},
			{1, 0, 1, 2},
		},
		{
			{1, 1, 1, 2},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{1, 0, 1, 1},
			{1, 1, 0, 1},
		},
		{
			{1, 1, 1, 2},
			{0, 0, 1, 0},
			{0, 1, 1, 1},
			{1, 0, 0, 0},
			{1, 1, 0, 1},
		},
		{
			{0, 1, 0, 0},
			{0, 0, 1, 1},
			{0, 1, 1, 1},
			{1, 0, 0, 2},
			{1, 1, 2, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 2},
			{0, 0, 2, 2},
			{0, 1, 0, 1},
			{1, 1, 0, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 2},
			{0, 1, 0, 1},
			{1, 1, 0, 1},
			{1, 1, 2, 2},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 1},
			{0, 1, 0, 2},
			{0, 2, 2, 0},
			{1, 0, 1, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 2, 2},
			{0, 1, 0, 1},
			{1, 1, 0, 1},
			{1, 1, 1, 2},
		},
		{
			{0, 0, 1, 1},
			{1, 0, 0, 0},
			{1, 0, 1, 1},
			{1, 1, 1, 2},
			{1, 2, 2, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 1},
			{0, 1, 2, 0},
			{0, 2, 0, 2},
			{1, 0, 1, 1},
		},
		{
			{0, 1, 0, 0},
			{0, 0, 1, 1},
			{0, 0, 2, 1},
			{1, 0, 0, 2},
			{1, 1, 0, 2},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 1},
			{0, 0, 2, 1},
			{0, 1, 0, 2},
			{1, 1, 0, 2},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 1},
			{0, 1, 0, 2},
			{1, 1, 0, 2},
			{1, 1, 2, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 2},
			{0, 2, 0, 1},
			{1, 1, 1, 1},
			{1, 1, 2, 0},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 2, 1},
			{0, 1, 0, 2},
			{1, 1, 1, 1},
			{1, 2, 1, 0},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 2, 1},
			{0, 2, 1, 0},
			{1, 1, 0, 2},
			{1, 1, 1, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 1, 0, 2},
			{0, 2, 1, 0},
			{1, 0, 2, 1},
			{1, 1, 1, 1},
		},
		{
			{0, 1, 0, 0},
			{0, 0, 2, 2},
			{1, 0, 0, 1},
			{1, 1, 0, 1},
			{1, 1, 1, 2},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 2, 1},
			{0, 1, 0, 2},
			{1, 1, 0, 2},
			{1, 1, 1, 1},
		},
		{
			{0, 1, 0, 0},
			{0, 0, 1, 2},
			{0, 0, 2, 2},
			{1, 0, 0, 1},
			{1, 1, 0, 1},
		},
		{
			{0, 1, 0, 0},
			{0, 0, 1, 2},
			{1, 0, 0, 1},
			{1, 1, 0, 1},
			{1, 1, 2, 2},
		},
		{
			{1, 0, 0, 0},
			{0, 1, 2, 0},
			{0, 2, 0, 1},
			{1, 0, 1, 2},
			{1, 1, 1, 1},
		},
		{
			{0, 1, 0, 0},
			{0, 0, 2, 1},
			{1, 0, 0, 2},
			{1, 1, 0, 2},
			{1, 1, 1, 1},
		},
		{
			{1, 0, 0, 0},
			{0, 0, 1, 2},
			{0, 1, 2, 0},
			{1, 1, 1, 1},
			{1, 2, 0, 1},
		},
		{
			{0, 1, 0, 0},
			{0, 0, 1, 1},
			{1, 0, 0, 2},
			{1, 1, 0, 2},
			{1, 1, 2, 1},
		},
		{
			{1, 1, 0, 1},
			{0, 0, 2, 2},
			{0, 2, 1, 0},
			{1, 0, 0, 1},
			{1, 1, 1, 2},
		},
		{
			{1, 1, 0, 2},
			{0, 0, 2, 1},
			{0, 2, 1, 0},
			{1, 0, 0, 2},
			{1, 1, 1, 1},
		},
		{
			{1, 0, 1, 1},
			{0, 1, 2, 0},
			{0, 2, 0, 2},
			{1, 0, 0, 1},
			{1, 1, 1, 2},
		},
		{
			{1, 0, 1, 2},
			{0, 1, 2, 0},
			{0, 2, 0, 1},
			{1, 0, 0, 2},
			{1, 1, 1, 1},
		},
		{
			{0, 1, 1, 1},
			{0, 0, 1, 0},
			{0, 1, 0, 0},
			{1, 0, 0, 2},
			{1, 1, 1, 1},
		},
		{
			{0, 0, 1, 1},
			{0, 1, 0, 0},
			{0, 1, 1, 1},
			{1, 0, 0, 0},
			{1, 1, 1, 2},
		},
		{
			{0, 1, 0, 1},
			{0, 0, 1, 0},
			{0, 1, 1, 1},
			{1, 0, 0, 0},
			{1, 1, 1, 2},
		},
	};

	/** Load board art, runner grids, paths, effects, and music. */
	void loadResources();
	/** Convert a row-major cell index to zero-based grid coordinates (x = column, y = row). */
	Common::Point32 toCellCoordinates(int cellIndex) const;
	/** Construct the selected level grid and its easy-mode panel sequence. */
	void buildGrid();
	/** Generate the next six-mirror panel for level 1. */
	void generateEasyPanel();
	/** Shuffle the supplied candidate array using the puzzle's random stream. */
	void permute(int *values, int count);
	/** Return how many of the four traits match between the two mirror cells. */
	int compareTraits(int first, int second) const;
	/** Load the next cannonball or party member as the projectile. */
	void loadNextProjectile();
	/** Replace and start the projectile's straight path between page-pixel anchors. */
	void startFlight(const Common::Point32 &start, const Common::Point32 &end, int step);
	/** Resolve a completed shot and prepare the next projectile when play continues. */
	void finishShot();
	/** Commit the caught target and advance to the next panel, target or puzzle completion. */
	void finishCatch();
	/** Begin the retreat sequence after ammunition and expendable party members are exhausted. */
	void startRetreat();
	/** Play the indexed loaded sound effect. */
	void playEffect(int index);
	/** Append a speech resource to the engine's serialized narration queue. */
	void queueSpeech(const Common::String &path);
	/** Draw one mirror's revealed traits, score and active-selection state. */
	void drawCell(ManagedSurface32 *screen, const MirrorCell &cell, bool active) const;
	/** Update the cursor for the mirror or control under the page-pixel pointer. */
	void updateHoverCursor(const Common::Point32 &pos);
	/** Quantize a page-space movement vector to one of the supplied direction sectors. */
	static int direction(const Common::Point32 &start, const Common::Point32 &end, int sectors);
	/** Return the adjacent aim code toward a distinct target angle. */
	static CannonAimAngle stepCannonAimAngle(CannonAimAngle current, CannonAimAngle target);
	/** Advance the Fleen shout/departure chain or finalize its capture. */
	static void onFleenAnimationDone(void *context, ZoombiniRunner *runner);
	/** Allocate a straight path between page-pixel anchors using the supplied raw Q10 step. */
	PathObject *makeLine(const Common::Point32 &start, const Common::Point32 &end, int step);

	/** Live cells: L1 uses 6, L2 uses 54, and L3/L4 use 72. */
	Common::Array<MirrorCell> _cells;
	/** Completed level-1 panels retain 0 to 30 cells in groups of 6; other levels retain none. */
	Common::Array<MirrorCell> _easyHistory;
	/** Number of mirror columns in the current board. */
	int _columns = 3;
	/** Number of active mirrors in the current board. */
	int _cellCount = 6;
	/** Current level-1 panel index from zero through five. */
	int _panel = 0;
	/** Mirror index containing the primary target Fleen. */
	int _target = 0;
	/** Mirror index containing the secondary target Fleen, when used. */
	int _alternateTarget = 0;
	/** Number of mirror selections made on the current board. */
	int _clickCount = 0;
	/** Currently selected mirror index, or -1 when none is selected. */
	int _selected = -1;
	/** Cannonballs remaining before shots must use party members. */
	int _ballsLeft = 0;
	/** Cannonball allowance selected for the current puzzle level. */
	int _initialBallCount = 0;
	/** Party size retained at puzzle entry for progress and shot accounting. */
	int _initialPartyCount = 0;
	/** Loaded party member and cannon/projectile orientation/image state. */
	int _loadedRunner = -1;
	/** Cannon angle currently shown on screen. */
	CannonAimAngle _angle = CannonAimAngle::kCenter04;
	/** Cannon angle toward which the aiming animation is moving. */
	CannonAimAngle _targetAngle = CannonAimAngle::kCenter04;
	/** Sprite-grid cell selected for the current projectile presentation. */
	int _projectileCell = 4;
	/** Current projectile image or frame selection. */
	int _projectileImage = 1;
	/** Found targets, reset/finish/retreat flags, and the projectile-consumption outcome. */
	int _caughtTargets = 0;
	/** Number of Fleen shouts performed in the current reaction sequence. */
	int _shoutCount = 0;
	/** Whether a new board is being prepared after the prior one. */
	bool _resetting = false;
	/** Whether the required Fleen targets have been found. */
	bool _finished = false;
	/** Whether at least one Fleen catch makes this visit eligible for perfect-clear progression. */
	bool _perfectClearEligible = false;
	/** Whether the current departure sequence is retreating. */
	bool _retreating = false;
	/** Whether the current shot has already consumed its projectile. */
	bool _shotConsumed = true;
	/** Whether the primary target Fleen has been found. */
	bool _foundPrimary = false;
	/** Whether the secondary target Fleen has been found. */
	bool _foundSecondary = false;
	/** Whether the Go departure must wait for the current presentation. */
	bool _goTransitionPending = false;
	/** Current cannon and mirror state with their animation/ambient scheduling ticks. */
	ShotPhase _shotPhase = ShotPhase::kStopped05;
	/** Current mirror opening, feedback or closing stage. */
	MirrorPhase _mirrorPhase = MirrorPhase::kNone00;
	/** Gameplay tick in milliseconds used to pace cannon aiming. */
	uint32 _aimTick = 0;
	/** Gameplay tick in milliseconds used to pace mirror reactions. */
	uint32 _mirrorTick = 0;
	/** Gameplay tick in milliseconds when another ambient sound may start. */
	uint32 _nextAmbientTick = 0;
	/** Active projectile path and the per-direction cannon rail paths/positions. */
	PathObject *_projectilePath = nullptr;
	/** Initial rail entries: L1 uses 12, L2 uses 8, L3 uses 6, and L4 uses 8. */
	Common::Array<PathObject *> _railPaths;
	/** Initial rail positions: L1 uses 12, L2 uses 8, L3 uses 6, and L4 uses 8. */
	Common::Array<Common::Point32> _railPositions;
	/** Current projectile anchor in page pixels. */
	Common::Point32 _projectilePos;
	/** Fleen/projectile runners and their animation grids. */
	ZoombiniRunner _fleensRunner;
	/** Runner used to animate a Zoombini fired from the cannon. */
	ZoombiniRunner _projectileRunner;
	/** Borrowed cached grid for the ordinary Fleen presentation. */
	const ZoombiniAnimation *_fleensAnimation = nullptr;
	/** Borrowed cached grid for the first Fleen shouting motion. */
	const ZoombiniAnimation *_vocif1 = nullptr;
	/** Borrowed cached grid for the second Fleen shouting motion. */
	const ZoombiniAnimation *_vocif2 = nullptr;
	/** Borrowed cached grid for the Zoombini cannon-entry motion. */
	const ZoombiniAnimation *_cannonAnimation = nullptr;
	/** Mirror-reaction animation resources. */
	Animation *_rotate = nullptr;
	/** Mirror-break animation released with the page. */
	Animation *_explode = nullptr;
	/** Indexed effects and randomized ambient handles. */
	int _sounds[8];
	/** Loaded ambient shout handles indexed by the three sound variants. */
	int _ambientSounds[3];
};

} // namespace Zoombini2

#endif
