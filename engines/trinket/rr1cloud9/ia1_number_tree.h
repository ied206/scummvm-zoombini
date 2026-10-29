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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_RR1CLOUD9_IA1_NUMBER_TREE_H
#define TRINKET_RR1CLOUD9_IA1_NUMBER_TREE_H

#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** IA1: balance both fruit branches, or remove one cluster from each branch. */
class NumberTree {
public:
	/** Connect the number-tree balancing activity to its page services. */
	explicit NumberTree(PageScene &host);
	/** Release the number-tree balancing presentation state. */
	~NumberTree();
	/** Load the number-tree balancing resources and initialize the current round. */
	bool enter();
	/** Advance number-tree balancing animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Draw the number-tree balancing activity's additional graphics and text. */
	void draw(OM::Canvas &canvas);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Handle activity pointer or keyboard input and report whether it was consumed. */
	bool event(const Common::Event &event);
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _phase != kReady || 0 <= _dragged; }
	/** Stop the current spoken instruction or equation sequence. */
	void skipSpeech();
	/** Pause or unpause activity feedback and sprite playback. */
	void pause(bool paused);
	/** Resume activity feedback and restore its animation timing. */
	void resume(uint32 elapsed) {
		_start += elapsed;
		_idle += elapsed;
	}

private:
	/** Format for script names or values: %s.%d. */
	static constexpr const char *kNameSignedIndexFormat = "%s.%d";
	/** Script property name for color. */
	static constexpr const char *kPropertyColor = "color";
	/** Script identifier for squirrel. */
	static constexpr const char *kSquirrelName = "squirrel";
	/** Authored data key for Squirrel. */
	static constexpr const char *kSquirrelKey = "kSquirrel";
	/** World-state key for r1 IA1 Level. */
	static constexpr const char *kR1IA1LevelKey = "R1IA1Level";
	/** World-state key for r1 IA1 Dataset. */
	static constexpr const char *kR1IA1DatasetKey = "R1IA1Dataset";
	/** World-state key for r1 IA1 Set. */
	static constexpr const char *kR1IA1SetKey = "R1IA1Set";
	/** World-state key for r1 IA1 Rounds. */
	static constexpr const char *kR1IA1RoundsKey = "R1IA1Rounds";
	/** World-state key for r1 IA1 Show Marble. */
	static constexpr const char *kR1IA1ShowMarbleKey = "R1IA1ShowMarble";
	/** Authored data key for Tree Trunk ID. */
	static constexpr const char *kTreeTrunkIDKey = "kTreeTrunkID";
	/** Authored data key for Tree Trunk Z. */
	static constexpr const char *kTreeTrunkZKey = "kTreeTrunkZ";
	/** Authored data key for Tree Move ID. */
	static constexpr const char *kTreeMoveIDKey = "kTreeMoveID";
	/** Authored data key for Tree Move Z. */
	static constexpr const char *kTreeMoveZKey = "kTreeMoveZ";
	/** Authored data key for Nut ID. */
	static constexpr const char *kNutIDKey = "kNutID";
	/** Authored data key for Reward Z. */
	static constexpr const char *kRewardZKey = "kRewardZ";
	/** Authored data key for Marble ID. */
	static constexpr const char *kMarbleIDKey = "kMarbleID";
	/** Authored data key for Sam Eat ID. */
	static constexpr const char *kSamEatIDKey = "kSamEatID";
	/** Authored data key for Sam Z. */
	static constexpr const char *kSamZKey = "kSamZ";
	/** Authored data key for Lever ID.%d. */
	static constexpr const char *kLeverIDSignedIndexKey = "kLeverID.%d";
	/** Authored data key for Lever Z. */
	static constexpr const char *kLeverZKey = "kLeverZ";
	/** Format for script names or values: .%d.%d. */
	static constexpr const char *kSignedIndexSignedIndexFormat = ".%d.%d";
	/** Authored data key for Value Text X. */
	static constexpr const char *kValueTextXKey = "kValueTextX";
	/** Authored data key for Value Text Y. */
	static constexpr const char *kValueTextYKey = "kValueTextY";
	/** Authored data key for Target Num Text X. */
	static constexpr const char *kTargetNumTextXKey = "kTargetNumTextX";
	/** Authored data key for Target Num Text Y. */
	static constexpr const char *kTargetNumTextYKey = "kTargetNumTextY";
	/** Authored data key for Nut X. */
	static constexpr const char *kNutXKey = "kNutX";
	/** Authored data key for Nut Y. */
	static constexpr const char *kNutYKey = "kNutY";
	/** Authored data key for Marble X. */
	static constexpr const char *kMarbleXKey = "kMarbleX";
	/** Authored data key for Marble Y. */
	static constexpr const char *kMarbleYKey = "kMarbleY";
	/** World-state key for r1 IA1 State. */
	static constexpr const char *kR1IA1StateKey = "R1IA1State";
	/** Authored data key for Max Entry Spchs. */
	static constexpr const char *kMaxEntrySpchsKey = "kMaxEntrySpchs";
	/** Authored data key for Reentry Spch ID.1.%d. */
	static constexpr const char *kReentrySpchID1SignedIndexKey = "kReentrySpchID.1.%d";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Authored data key for Reentry Spch ID.2.1. */
	static constexpr const char *kReentrySpchID21Key = "kReentrySpchID.2.1";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Authored data key for Reentry Spch ID.2.2. */
	static constexpr const char *kReentrySpchID22Key = "kReentrySpchID.2.2";
	/** Authored data key for Reset Spch ID.4.1. */
	static constexpr const char *kResetSpchID41Key = "kResetSpchID.4.1";
	/** Authored data key for Reset Spch ID.4.2. */
	static constexpr const char *kResetSpchID42Key = "kResetSpchID.4.2";
	/** Format for script names or values: .%d. */
	static constexpr const char *kSignedIndexFormat = ".%d";
	/** Authored data key for Enter Char ID. */
	static constexpr const char *kEnterCharIDKey = "kEnterCharID";
	/** Authored data key for Enter Spch ID. */
	static constexpr const char *kEnterSpchIDKey = "kEnterSpchID";
	/** Authored data key for Max Data Sets. */
	static constexpr const char *kMaxDataSetsKey = "kMaxDataSets";
	/** Authored data key for Num Sets. */
	static constexpr const char *kNumSetsKey = "kNumSets";
	/** Script property name for data Set/Level %d/Word List %d/Set %d. */
	static constexpr const char *kPropertyDataSetLevelSignedIndexWordListSignedIndexSetSignedIndex = "DataSet/Level_%d/WordList_%d/Set_%d";
	/** Authored data key for Num Values. */
	static constexpr const char *kNumValuesKey = "kNumValues";
	/** Authored data key for Answer ID.%d. */
	static constexpr const char *kAnswerIDSignedIndexKey = "kAnswerID.%d";
	/** Authored data key for Answer Z. */
	static constexpr const char *kAnswerZKey = "kAnswerZ";
	/** Authored data key for Answer X. */
	static constexpr const char *kAnswerXKey = "kAnswerX";
	/** Authored data key for Answer Y. */
	static constexpr const char *kAnswerYKey = "kAnswerY";
	/** Script property name for text. */
	static constexpr const char *kPropertyText = "text";
	/** Authored data key for Container X. */
	static constexpr const char *kContainerXKey = "kContainerX";
	/** Authored data key for Container Y.%d.%d. */
	static constexpr const char *kContainerYSignedIndexSignedIndexKey = "kContainerY.%d.%d";
	/** Authored data key for Container W. */
	static constexpr const char *kContainerWKey = "kContainerW";
	/** Authored data key for Container H. */
	static constexpr const char *kContainerHKey = "kContainerH";
	/** Authored data key for Branch Move SFX. */
	static constexpr const char *kBranchMoveSFXKey = "kBranchMoveSFX";
	/** Authored data key for Correct SFX. */
	static constexpr const char *kCorrectSFXKey = "kCorrectSFX";
	/** Authored data key for Wrong Sfx ID. */
	static constexpr const char *kWrongSfxIDKey = "kWrongSfxID";
	/** Authored data key for Oops Spch ID.%d. */
	static constexpr const char *kOopsSpchIDSignedIndexKey = "kOopsSpchID.%d";
	/** Authored data key for Wrong Spch ID.%d. */
	static constexpr const char *kWrongSpchIDSignedIndexKey = "kWrongSpchID.%d";
	/** Authored data key for Eq Spch ID. */
	static constexpr const char *kEqSpchIDKey = "kEqSpchID";
	/** Authored data key for Zero Spch ID. */
	static constexpr const char *kZeroSpchIDKey = "kZeroSpchID";
	/** Authored data key for Type Speech.2. */
	static constexpr const char *kTypeSpeech2Key = "kTypeSpeech.2";
	/** Authored data key for Blank SFX. */
	static constexpr const char *kBlankSFXKey = "kBlankSFX";
	/** Authored data key for Type Speech.1. */
	static constexpr const char *kTypeSpeech1Key = "kTypeSpeech.1";
	/** Authored data key for Type Speech.3. */
	static constexpr const char *kTypeSpeech3Key = "kTypeSpeech.3";
	/** Authored data key for Char Congrat Spch ID.%d. */
	static constexpr const char *kCharCongratSpchIDSignedIndexKey = "kCharCongratSpchID.%d";
	/** Authored data key for Reward Spch ID.3. */
	static constexpr const char *kRewardSpchID3Key = "kRewardSpchID.3";
	/** Authored data key for Reward Spch ID.2. */
	static constexpr const char *kRewardSpchID2Key = "kRewardSpchID.2";
	/** Authored data key for Marble SFX. */
	static constexpr const char *kMarbleSFXKey = "kMarbleSFX";
	/** Authored data key for Reward Spch ID.1. */
	static constexpr const char *kRewardSpchID1Key = "kRewardSpchID.1";
	/** World-state key for r1 IA1 Streak Kind. */
	static constexpr const char *kR1IA1StreakKindKey = "R1IA1StreakKind";
	/** World-state key for r1 IA1 Streak. */
	static constexpr const char *kR1IA1StreakKey = "R1IA1Streak";
	/** World-state key for r1 IA1. */
	static constexpr const char *kR1IA1Key = "R1IA1";
	/** World-state key for r1 IA1 Fixed Level. */
	static constexpr const char *kR1IA1FixedLevelKey = "R1IA1FixedLevel";
	/** Authored data key for Reset Spch ID.1.1. */
	static constexpr const char *kResetSpchID11Key = "kResetSpchID.1.1";
	/** Authored data key for Put Cluster Tree SFX. */
	static constexpr const char *kPutClusterTreeSFXKey = "kPutClusterTreeSFX";
	/** Authored data key for Cluster Snap Home SFX. */
	static constexpr const char *kClusterSnapHomeSFXKey = "kClusterSnapHomeSFX";
	/** Authored data key for Instruction Spch ID.1. */
	static constexpr const char *kInstructionSpchID1Key = "kInstructionSpchID.1";
	/** Authored data key for Instruction Spch ID.%d. */
	static constexpr const char *kInstructionSpchIDSignedIndexKey = "kInstructionSpchID.%d";
	/** Format for script names or values: item:%d. */
	static constexpr const char *kItemSignedIndexFormat = "item:%d";
	/** Authored data key for INV1 Marble2. */
	static constexpr const char *kINV1Marble2Key = "kINV1Marble2";
	/** Authored data key for Marble Spch ID. */
	static constexpr const char *kMarbleSpchIDKey = "kMarbleSpchID";
	/** Authored data key for Eat Nut Spch ID.1. */
	static constexpr const char *kEatNutSpchID1Key = "kEatNutSpchID.1";
	/** Authored data key for Sub Container X. */
	static constexpr const char *kSubContainerXKey = "kSubContainerX";
	/** Authored data key for Sub Container Y. */
	static constexpr const char *kSubContainerYKey = "kSubContainerY";
	/** Authored data key for Pickup Cluster SFX. */
	static constexpr const char *kPickupClusterSFXKey = "kPickupClusterSFX";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Authored data key for Help Spch ID.%d. */
	static constexpr const char *kHelpSpchIDSignedIndexKey = "kHelpSpchID.%d";
	/** Authored data key for Idle Spch IDs.3.1. */
	static constexpr const char *kIdleSpchIDs31Key = "kIdleSpchIDs.3.1";
	/** Authored data key for Idle Spch IDs.3.2. */
	static constexpr const char *kIdleSpchIDs32Key = "kIdleSpchIDs.3.2";
	/** Authored data key for Main Spch ID.1. */
	static constexpr const char *kMainSpchID1Key = "kMainSpchID.1";
	/** Authored data key for Idle Spch IDs.2.1. */
	static constexpr const char *kIdleSpchIDs21Key = "kIdleSpchIDs.2.1";
	/** Authored data key for Idle Spch IDs.2.2. */
	static constexpr const char *kIdleSpchIDs22Key = "kIdleSpchIDs.2.2";
	/** Authored data key for Main Spch ID.2. */
	static constexpr const char *kMainSpchID2Key = "kMainSpchID.2";
	/** Authored data key for Main Spch ID.3. */
	static constexpr const char *kMainSpchID3Key = "kMainSpchID.3";
	/** Authored data key for Reset Spch ID.3.3. */
	static constexpr const char *kResetSpchID33Key = "kResetSpchID.3.3";
	/** Authored data key for Main Spch ID.4. */
	static constexpr const char *kMainSpchID4Key = "kMainSpchID.4";
	/** Authored data key for Main Spch ID.5. */
	static constexpr const char *kMainSpchID5Key = "kMainSpchID.5";
	/** Authored data key for Eq HPL. */
	static constexpr const char *kEqHPLKey = "kEqHPL";
	/** Authored data key for Eq HPT. */
	static constexpr const char *kEqHPTKey = "kEqHPT";
	/** Authored data key for Eq HPW. */
	static constexpr const char *kEqHPWKey = "kEqHPW";
	/** Authored data key for Eq HPH. */
	static constexpr const char *kEqHPHKey = "kEqHPH";
	/** Authored data key for Present New Clusters SFX. */
	static constexpr const char *kPresentNewClustersSFXKey = "kPresentNewClustersSFX";
	/** Authored data key for Reset Spch ID.1.%d. */
	static constexpr const char *kResetSpchID1SignedIndexKey = "kResetSpchID.1.%d";
	/** Authored data key for Reset Spch ID.%d.1. */
	static constexpr const char *kResetSpchIDSignedIndex1Key = "kResetSpchID.%d.1";
	/** Authored data key for Reset Spch ID.%d.%d. */
	static constexpr const char *kResetSpchIDSignedIndexSignedIndexKey = "kResetSpchID.%d.%d";
	/** Authored data key for Eat Nut Spch ID.%d. */
	static constexpr const char *kEatNutSpchIDSignedIndexKey = "kEatNutSpchID.%d";
	/** Authored data key for Idle Spch IDs.%d.1. */
	static constexpr const char *kIdleSpchIDsSignedIndex1Key = "kIdleSpchIDs.%d.1";
	/** Authored data key for Idle Spch IDs.%d.2. */
	static constexpr const char *kIdleSpchIDsSignedIndex2Key = "kIdleSpchIDs.%d.2";

	/** Font for number-tree equations. */
	static constexpr const char *kFont = "common/1018.nfnt";
	/** Large font for the target number. */
	static constexpr const char *kLargeFont = "common/1036.nfnt";
	/** Number-tree puzzle datasets. */
	static constexpr const char *kDataXML = "r1ia1.xml";

	enum Phase {
		kReady,
		kTilting,
		kWrong,
		kRestoring,
		kReward,
		kNewTree,
		kEating,
		kIntroSpeech,
		kIntroLeft,
		kIntroRight,
	};
	struct Cluster {
		/** Sprite displaying the draggable number cluster. */
		Shell::Sprite *sprite = nullptr;
		/** Game-pixel supply position restored after an invalid drop. */
		Common::Point home;
		/** Starting position of the current cluster movement. */
		Common::Point from;
		/** Destination position of the current cluster movement. */
		Common::Point to;
		/** Number represented by this cluster. */
		int value = 0;
		/** Branch slot occupied by the cluster, or minus one while in supply. */
		int slot = -1;
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Resource-path resolver for the activity's current page. */
	Shell::ResourcePaths _paths;
	/** Parsed XML data used to construct the current activity round. */
	OM::XMLParser _data;
	/** Font used for activity text and labels. */
	Common::SharedPtr<OM::NFNTFont> _font;
	/** Font used for the target and prominent equation values. */
	Common::SharedPtr<OM::NFNTFont> _largeFont;
	/** Text object displaying the required numeric result. */
	Shell::TextObject *_targetText = nullptr;
	/** Text objects forming each side's displayed equation. */
	Shell::TextObject *_equations[2][7] = {};
	/** Sprite displaying the tree's central trunk. */
	Shell::Sprite *_trunk = nullptr;
	/** Sprite used for tree-motion feedback. */
	Shell::Sprite *_treeMove = nullptr;
	/** Nut sprite used for successful balancing feedback. */
	Shell::Sprite *_nut = nullptr;
	/** Marble character sprite in the number-tree presentation. */
	Shell::Sprite *_marble = nullptr;
	/** Sprite used for Marble's eating feedback. */
	Shell::Sprite *_eat = nullptr;
	/** Branch or lever sprites indexed by side. */
	Shell::Sprite *_levers[2] = {};
	/** Animations to resume after activity playback is unpaused. */
	Common::Array<Shell::Sprite *> _pausedSprites;
	/** Draggable number clusters and their supply and branch positions. */
	Common::Array<Cluster> _clusters;
	/** Pointer displacement from the dragged piece's anchor, in pixels. */
	Common::Point _dragOffset;
	/** Difficulty selected for the current activity round. */
	int _level = 1;
	/** Authored dataset selected for the current difficulty. */
	int _dataset = 1;
	/** Exercise set selected within the dataset. */
	int _set = 1;
	/** Numeric result required by the current tree challenge. */
	int _target = 0;
	/** Index of the piece being dragged, or minus one when no piece is held. */
	int _dragged = -1;
	/** Current branch tilt indexed by side. */
	int _tilt[2] = {};
	/** Branch tilt at the start of the current movement. */
	int _oldTilt[2] = {};
	/** Initial branch values used to form the displayed equations. */
	int _initial[2] = {};
	/** Cluster removed from each side during the current placement change. */
	int _removed[2] = {
															   -1,
															   -1,
														   };
	/** Side whose placement triggered the current branch movement. */
	int _changedSide = 0;
	/** Branch slot restored after a rejected placement. */
	int _restoreSlot = -1;
	/** Number of completed number-tree rounds. */
	int _rounds = 0;
	/** Next instruction variant selected for the current challenge. */
	int _instruction = 1;
	/** Whether the current equations use subtraction. */
	bool _subtract = false;
	/** Whether the current round has included an incorrect response. */
	bool _imperfect = false;
	/** Whether Marble is displayed in the current presentation. */
	bool _showMarble = false;
	/** Whether the first successful nut presentation is still pending. */
	bool _firstNut = true;
	/** Whether each side's correct response has already been announced. */
	bool _heardCorrect[2] = {};
	/** State controlling sound feedback during branch movement. */
	bool _branchSound = false;
	/** Millisecond timestamp when the current timed stage began. */
	uint32 _start = 0;
	/** Millisecond timestamp used to schedule idle feedback. */
	uint32 _idle = 0;
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kReady;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Read an authored numeric constant selected by an indexed name. */
	int indexed(const char *key, int index) const;
	/** Create a number-tree sprite from an authored resource key and depth. */
	Shell::Sprite *sprite(const Common::String &key, int depth);
	/** Create a numeric text object at a game-pixel position with the selected font size. */
	Shell::TextObject *text(int x, int y, bool large = false);
	/** Play the authored spoken line for the selected actor. */
	void say(const char *actor, const Common::String &key);
	/** Play an effect identified by an authored sound key. */
	void sound(const char *key);
	/** Play the spoken form of a numeric value. */
	void sayNumber(int value);
	/** Read the displayed equation on the selected tree side. */
	void readEquation(int side);
	/** Prepare and display the next number-tree balancing round. */
	bool setup(bool announce = true);
	/** Return the number of datasets available at the current difficulty. */
	int datasetCount() const;
	/** Choose the next exercise dataset using saved activity progress. */
	void chooseDataset();
	/** Write the activity's current saved state to the world. */
	void persist();
	/** Calculate the current value of the selected tree side. */
	int sum(int side) const;
	/** Return the cluster occupying a branch slot, or minus one when empty. */
	int occupant(int slot) const;
	/** Return a branch slot's game-pixel hit rectangle. */
	Common::Rect slotRect(int slot) const;
	/** Calculate a cluster's game-pixel position for its current placement. */
	Common::Point position(const Cluster &cluster) const;
	/** Refresh both displayed equations from the current branch values. */
	void equations();
	/** Begin branch and cluster motion after a placement or restoration. */
	void moveBranches(int side, bool restoring = false);
	/** Interpolate branch motion and report whether it has completed. */
	bool advanceBranches(uint32 elapsed, bool finish = false);
	/** Play the selected range of introductory instruction lines. */
	void entrySpeech(int first, int last);
	/** Evaluate the current branch equations against the target. */
	void checkAnswer();
	/** Start successful-round feedback and update activity progress. */
	void solved();
	/** Place the dragged cluster in a branch slot or return it to supply. */
	void drop();
	/** Play context-sensitive number-tree guidance. */
	void help();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
