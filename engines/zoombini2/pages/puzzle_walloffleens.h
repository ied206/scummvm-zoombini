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
	void onActorsRendered() override;
	/** Fire the cannon or select a target mirror after a button release at @p pos. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	/** Complete the current board for the puzzle-console command. */
	void applyDebugPuzzleCompletion() override;
	bool canUseGoButton() const override;
	/** Return through retreat speech or the completed map transition. */
	bool onGoButtonPressed() override;
	/** Describe target traits and current mirror scores for the puzzle console. */
	Common::String debugGetAnswer() const override;
	PuzzleChanceInfo debugGetChances() const override { return PuzzleChanceInfo(PuzzleChanceInfo::Type::kAmorphous); }
	Common::String debugGetChanceDetails() const override;

private:
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
	struct Cell {
		/** Target-comparison traits, screen origin, score, and display flags for one mirror cell. */
		ZmbTrait traits;
		Common::Point32 pos;
		int score = -1;
		bool tried = false;
		bool revealed = false;
		bool empty = false;
		byte scoreMask = 1;
	};
	/** Music, cannon/mirror/projectile art, overlays, and score marker format. */
	static constexpr const char *kMusicPath = "#sounds/music/05-BB01.wav";
	static constexpr const char *kCannonFormat = "bmp/wall_of_fleens/canon0%d";
	static constexpr const char *kMirrorPaths[5] = {
		"bmp/wall_of_fleens/mirror_GRIS",
		"bmp/wall_of_fleens/mirror_nomal",
		"bmp/wall_of_fleens/mirror_NOIR",
		"bmp/wall_of_fleens/mirror_felure",
		"bmp/wall_of_fleens/mirror_empty_tunnel",
	};
	static constexpr const char *kBallPaths[6] = {
		"bmp/wall_of_fleens/boulet/boulet",
		"bmp/wall_of_fleens/boulet/boulet4",
		"bmp/wall_of_fleens/boulet/boulet6",
		"bmp/wall_of_fleens/boulet/boulet7",
		"bmp/wall_of_fleens/boulet/boulet8",
		"bmp/wall_of_fleens/boulet/boulet9",
	};
	static constexpr const char *kOverlayPaths[2] = {
		"bmp/wall_of_fleens/tuyere",
		"bmp/wall_of_fleens/canon_cache",
	};
	static constexpr const char *kScoreFormat = "bmp/wall_of_fleens/LevelRED%d";
	static constexpr const char *kRotatePath = "bmp/wall_of_fleens/mirror_rotate";
	static constexpr const char *kExplodePath = "bmp/wall_of_fleens/mirror_explode";
	/** Projectile rails, Fleen/cannon/Zoombini grids, and mirror reaction animations. */
	static constexpr const char *kBallLoadPath = "bmp/wall_of_fleens/bullet_on_cannon.pat";
	static constexpr const char *kJumpPath = "bmp/wall_of_fleens/jump_in_cannon.pat";
	static constexpr const char *kFleenPath = "bmp/fleens/fleens.anm";
	static constexpr const char *kVocif1Path = "bmp/fleens/vocif/vocif1.anm";
	static constexpr const char *kVocif2Path = "bmp/fleens/vocif/vocif2.anm";
	static constexpr const char *kCannonZombPath = "bmp/wall_of_fleens/cannon_zomb/cannon_zomb.anm";
	static constexpr const char *kJumpAnimationPath = "bmp/zombis/saut/saut.anm";
	static constexpr const char *kCelebratePath = "bmp/zombis/attente/attenteZomb.anm";
	/** Effects, success/loss/retreat/ambient speech, and perfect-Go speech. */
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
	static constexpr const char *kSuccessSpeechPath = "sounds/8-E1.wav";
	static constexpr const char *kLossSpeechPath = "sounds/8-E2.wav";
	static constexpr const char *kRetreatSpeechPaths[2] = {
		"sounds/wlf15.1.wav",
		"sounds/wlf15.3.wav",
	};
	static constexpr const char *kAmbientFormat = "sounds/wlf12.%d.wav";
	static constexpr const char *kPerfectGoFormat = "sounds/wld11.%d.wav";
	/** Difficulty-specific score-panel/grid origins and cannon muzzle positions. */
	static constexpr Common::Point32 kPanelOrigins[6] = {
		{182, 88},
		{344, 88},
		{506, 87},
		{183, 230},
		{345, 229},
		{508, 230},
	};
	static constexpr Common::Point32 kGridOrigins[5] = {
		{0, 0},
		{380, 200},
		{200, 7},
		{100, 7},
		{95, 7},
	};
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
	static constexpr byte kEasyPatterns[123][5][4] = {
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
	/** Construct the selected difficulty grid and its easy-mode panel sequence. */
	void buildGrid();
	void generateEasyPanel();
	/** Shuffle candidate values and compare a cell's four target traits. */
	void permute(int *values, int count);
	int compareTraits(int first, int second) const;
	/** Load/fire the next projectile and advance hit, catch, or retreat outcomes. */
	void loadNextProjectile();
	void startFlight(const Common::Point32 &start, const Common::Point32 &end, int step);
	void finishShot();
	void finishCatch();
	void startRetreat();
	/** Play effects, queue engine-lifetime speech, and draw each mirror cell. */
	void playEffect(int index);
	void queueSpeech(const Common::String &path);
	void drawCell(ManagedSurface32 *screen, const Cell &cell, bool active) const;
	/** Convert a rail vector to an animation direction, clear Fleen completion, or make a line path. */
	static int direction(const Common::Point32 &start, const Common::Point32 &end, int sectors);
	static void onFleenAnimationDone(void *context, ZoombiniRunner *runner);
	PathObject *makeLine(const Common::Point32 &start, const Common::Point32 &end, int step);

	/** Live board cells and the six-panel easy-mode history. */
	Cell _cells[72];
	Cell _easyHistory[36];
	/** Difficulty/grid dimensions, active panel/targets, player input, and ammunition state. */
	int _level = 1;
	int _columns = 3;
	int _cellCount = 6;
	int _panel = 0;
	int _target = 0;
	int _alternateTarget = 0;
	int _clickCount = 0;
	int _selected = -1;
	int _ballsLeft = 0;
	int _initialPartyCount = 0;
	/** Loaded party member and cannon/projectile orientation/image state. */
	int _loadedRunner = -1;
	int _angle = 4;
	int _targetAngle = 4;
	int _projectileCell = 4;
	int _projectileImage = 1;
	/** Found targets, reset/finish/retreat flags, and the projectile-consumption outcome. */
	int _caughtTargets = 0;
	int _shoutCount = 0;
	bool _resetting = false;
	bool _finished = false;
	/** Whether at least one Fleen catch makes this visit eligible for perfect-clear progression. */
	bool _perfectClearEligible = false;
	bool _retreating = false;
	bool _shotConsumed = true;
	bool _foundPrimary = false;
	bool _foundSecondary = false;
	bool _goTransitionPending = false;
	/** Current cannon and mirror state with their animation/ambient scheduling ticks. */
	ShotPhase _shotPhase = ShotPhase::kStopped05;
	MirrorPhase _mirrorPhase = MirrorPhase::kNone00;
	uint32 _aimTick = 0;
	uint32 _mirrorTick = 0;
	uint32 _nextAmbientTick = 0;
	/** Active projectile path and the per-direction cannon rail paths/positions. */
	PathObject *_projectilePath = nullptr;
	PathObject *_railPaths[12] = {};
	Common::Point32 _railPositions[12];
	Common::Point32 _projectilePos;
	/** Fleen/projectile runners and their animation grids. */
	ZoombiniRunner _fleensRunner;
	ZoombiniRunner _projectileRunner;
	const ZoombiniAnimation *_fleensAnimation = nullptr;
	const ZoombiniAnimation *_vocif1 = nullptr;
	const ZoombiniAnimation *_vocif2 = nullptr;
	const ZoombiniAnimation *_cannonAnimation = nullptr;
	const ZoombiniAnimation *_jumpAnimation = nullptr;
	const ZoombiniAnimation *_celebrationAnimation = nullptr;
	/** Mirror-reaction animation resources. */
	Animation *_rotate = nullptr;
	Animation *_explode = nullptr;
	/** Indexed effects and randomized ambient handles. */
	int _sounds[8];
	int _ambientSounds[3];
};

} // namespace Zoombini2

#endif
