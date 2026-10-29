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

#ifndef TRINKET_RR1CLOUD9_IG1_ROCK_HEAD_H
#define TRINKET_RR1CLOUD9_IG1_ROCK_HEAD_H

#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** Ladder, summit conversation and the hat-for-hoop exchange. */
class RockHead {
public:
	/** Connect the Rock Head activity to its page services. */
	explicit RockHead(PageScene &host) : _host(host) {}
	/** Load the Rock Head resources and initialize the current round. */
	bool enter();
	/** Advance Rock Head animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Handle a ladder, hat or other offered inventory item and report whether the input was handled. */
	bool donate(uint item);
	/** Resume activity feedback and restore its animation timing. */
	void resume(uint32 elapsed) {
		_start += elapsed;
		_idle += elapsed;
	}
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _phase != kIdle; }

private:
	/** Format for script names or values: .%d. */
	static constexpr const char *kSignedIndexFormat = ".%d";
	/** Script identifier for set ID. */
	static constexpr const char *kSetIDName = "SetID";
	/** Script identifier for lips ID. */
	static constexpr const char *kLipsIDName = "LipsID";
	/** Script identifier for z. */
	static constexpr const char *kZName = "Z";
	/** Script identifier for register Fidgets To Settle. */
	static constexpr const char *kRegisterFidgetsToSettleName = "registerFidgetsToSettle";
	/** Script identifier for register Speech To Settle. */
	static constexpr const char *kRegisterSpeechToSettleName = "registerSpeechToSettle";
	/** Script identifier for load Speech Anims In Bkgd. */
	static constexpr const char *kLoadSpeechAnimsInBkgdName = "loadSpeechAnimsInBkgd";
	/** Script identifier for animate Settled. */
	static constexpr const char *kAnimateSettledName = "animateSettled";
	/** Script method name for set Speech Anims. */
	static constexpr const char *kMethodSetSpeechAnims = "setSpeechAnims";
	/** Data-key suffix selecting a character body resource ID. */
	static constexpr const char *kBodyIDSuffix = "BodyID";
	/** Script method name for add Fidget Group. */
	static constexpr const char *kMethodAddFidgetGroup = "addFidgetGroup";
	/** Script method name for add Fidget. */
	static constexpr const char *kMethodAddFidget = "addFidget";
	/** Script identifier for fidg1 ID. */
	static constexpr const char *kFidg1IDName = "Fidg1ID";
	/** Data-key suffix indexed by character fidget number. */
	static constexpr const char *kFidgetIDSuffixFormat = "Fidg%dID";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Authored data key for Sam. */
	static constexpr const char *kSamKey = "kSam";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Authored data key for Paige. */
	static constexpr const char *kPaigeKey = "kPaige";
	/** Authored data key for Chester Fidg ID.%d. */
	static constexpr const char *kChesterFidgIDSignedIndexKey = "kChesterFidgID.%d";
	/** Script identifier for g Closed Chester Z. */
	static constexpr const char *kGClosedChesterZName = "gClosedChesterZ";
	/** World-state key for r1 IG1 Hat Here. */
	static constexpr const char *kR1IG1HatHereKey = "R1IG1HatHere";
	/** Actor identifier for the rock head. */
	static constexpr const char *kRockActor = "rock";
	/** Authored data key for Rock. */
	static constexpr const char *kRockKey = "kRock";
	/** Script method name for remove Fidget Group. */
	static constexpr const char *kMethodRemoveFidgetGroup = "removeFidgetGroup";
	/** Authored data key for Rock Fdgt ID. */
	static constexpr const char *kRockFdgtIDKey = "kRockFdgtID";
	/** World-state key for r1 IG1 State. */
	static constexpr const char *kR1IG1StateKey = "R1IG1State";
	/** World-state key for r1 IG1 Hoop Here. */
	static constexpr const char *kR1IG1HoopHereKey = "R1IG1HoopHere";
	/** World-state key for item:6. */
	static constexpr const char *kItem6Key = "item:6";
	/** World-state key for r1 IG1 Ladder Here. */
	static constexpr const char *kR1IG1LadderHereKey = "R1IG1LadderHere";
	/** World-state key for r1 IG1 Pending. */
	static constexpr const char *kR1IG1PendingKey = "R1IG1Pending";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Authored data key for Reader. */
	static constexpr const char *kReaderKey = "kReader";
	/** Authored data key for Ladder ID. */
	static constexpr const char *kLadderIDKey = "kLadderID";
	/** Authored data key for Ladder Z. */
	static constexpr const char *kLadderZKey = "kLadderZ";
	/** Authored data key for Hat ID. */
	static constexpr const char *kHatIDKey = "kHatID";
	/** Authored data key for Hat Z. */
	static constexpr const char *kHatZKey = "kHatZ";
	/** Authored data key for Hoop2 ID. */
	static constexpr const char *kHoop2IDKey = "kHoop2ID";
	/** Authored data key for Hoop Z. */
	static constexpr const char *kHoopZKey = "kHoopZ";
	/** World-state key for previous Location. */
	static constexpr const char *kPreviousLocationKey = "previousLocation";
	/** Authored data key for Reader Walk In ID. */
	static constexpr const char *kReaderWalkInIDKey = "kReaderWalkInID";
	/** Authored data key for Sam Walk In ID. */
	static constexpr const char *kSamWalkInIDKey = "kSamWalkInID";
	/** Authored data key for Enter Char ID. */
	static constexpr const char *kEnterCharIDKey = "kEnterCharID";
	/** Authored data key for Enter Spch ID. */
	static constexpr const char *kEnterSpchIDKey = "kEnterSpchID";
	/** Authored data key for Max Entry Spchs. */
	static constexpr const char *kMaxEntrySpchsKey = "kMaxEntrySpchs";
	/** World-state key for r1 IG1 Rainbow Slide Count. */
	static constexpr const char *kR1IG1RainbowSlideCountKey = "R1IG1RainbowSlideCount";
	/** Authored data key for Re Enter Char IDs. */
	static constexpr const char *kReEnterCharIDsKey = "kReEnterCharIDs";
	/** Authored data key for Re Enter Spch IDs. */
	static constexpr const char *kReEnterSpchIDsKey = "kReEnterSpchIDs";
	/** Authored data key for Max Re Enter Spchs. */
	static constexpr const char *kMaxReEnterSpchsKey = "kMaxReEnterSpchs";
	/** Authored data key for Reader Climb ID. */
	static constexpr const char *kReaderClimbIDKey = "kReaderClimbID";
	/** Authored data key for Reader Decend ID. */
	static constexpr const char *kReaderDecendIDKey = "kReaderDecendID";
	/** Authored data key for Sam Climb ID. */
	static constexpr const char *kSamClimbIDKey = "kSamClimbID";
	/** Authored data key for Sam Decend ID. */
	static constexpr const char *kSamDecendIDKey = "kSamDecendID";
	/** Authored data key for Ladder Char ID. */
	static constexpr const char *kLadderCharIDKey = "kLadderCharID";
	/** Authored data key for Ladder Spch ID. */
	static constexpr const char *kLadderSpchIDKey = "kLadderSpchID";
	/** Authored data key for Hat Char ID. */
	static constexpr const char *kHatCharIDKey = "kHatCharID";
	/** Authored data key for Hat Spch ID. */
	static constexpr const char *kHatSpchIDKey = "kHatSpchID";
	/** Authored data key for Num Hat Spchs. */
	static constexpr const char *kNumHatSpchsKey = "kNumHatSpchs";
	/** Authored data key for Hoop Presented ID. */
	static constexpr const char *kHoopPresentedIDKey = "kHoopPresentedID";
	/** Authored data key for Hoop Presented Char ID. */
	static constexpr const char *kHoopPresentedCharIDKey = "kHoopPresentedCharID";
	/** Authored data key for Hoop Presented Spch ID. */
	static constexpr const char *kHoopPresentedSpchIDKey = "kHoopPresentedSpchID";
	/** World-state key for item:2. */
	static constexpr const char *kItem2Key = "item:2";
	/** Script identifier for g Idle Help Time. */
	static constexpr const char *kGIdleHelpTimeName = "gIdleHelpTime";
	/** Authored data key for Max Idle Index. */
	static constexpr const char *kMaxIdleIndexKey = "kMaxIdleIndex";
	/** Authored data key for Idle Char ID. */
	static constexpr const char *kIdleCharIDKey = "kIdleCharID";
	/** Authored data key for Idle Spch ID. */
	static constexpr const char *kIdleSpchIDKey = "kIdleSpchID";
	/** Authored data key for Max Idle Index Speeches. */
	static constexpr const char *kMaxIdleIndexSpeechesKey = "kMaxIdleIndexSpeeches";
	/** Format for script names or values: .1.%d. */
	static constexpr const char *k1SignedIndexFormat = ".1.%d";
	/** Authored data key for Main Spch Char IDs. */
	static constexpr const char *kMainSpchCharIDsKey = "kMainSpchCharIDs";
	/** Authored data key for Main Spch IDs. */
	static constexpr const char *kMainSpchIDsKey = "kMainSpchIDs";
	/** Authored data key for Max Main Spchs. */
	static constexpr const char *kMaxMainSpchsKey = "kMaxMainSpchs";
	/** Authored data key for Main Spch Char IDs.2. */
	static constexpr const char *kMainSpchCharIDs2Key = "kMainSpchCharIDs.2";
	/** Authored data key for Main Spch IDs.2. */
	static constexpr const char *kMainSpchIDs2Key = "kMainSpchIDs.2";
	/** Format for script names or values: .3.%d. */
	static constexpr const char *k3SignedIndexFormat = ".3.%d";
	/** World-state key for talked To Rock Head. */
	static constexpr const char *kTalkedToRockHeadKey = "talkedToRockHead";
	/** Script identifier for .2.1. */
	static constexpr const char *k21Name = ".2.1";
	/** Authored data key for Rock Char ID.3.1. */
	static constexpr const char *kRockCharID31Key = "kRockCharID.3.1";
	/** Authored data key for Rock Spch ID.3.1. */
	static constexpr const char *kRockSpchID31Key = "kRockSpchID.3.1";
	/** World-state key for r1 IG1 Rock Clicks. */
	static constexpr const char *kR1IG1RockClicksKey = "R1IG1RockClicks";
	/** World-state key for r1 LN2 State. */
	static constexpr const char *kR1LN2StateKey = "R1LN2State";
	/** World-state key for item:8. */
	static constexpr const char *kItem8Key = "item:8";
	/** Format for script names or values: .3.2.%d. */
	static constexpr const char *k32SignedIndexFormat = ".3.2.%d";
	/** Format for script names or values: .4.%d. */
	static constexpr const char *k4SignedIndexFormat = ".4.%d";
	/** Authored data key for Rock Char ID. */
	static constexpr const char *kRockCharIDKey = "kRockCharID";
	/** Authored data key for Rock Spch ID. */
	static constexpr const char *kRockSpchIDKey = "kRockSpchID";
	/** Authored data key for Max Rock Spch. */
	static constexpr const char *kMaxRockSpchKey = "kMaxRockSpch";
	/** Script identifier for wAVE COMSOFTCLICK. */
	static constexpr const char *kWAVECOMSOFTCLICKName = "WAVE_COMSOFTCLICK";
	/** Authored data key for Max Hoop Spchs. */
	static constexpr const char *kMaxHoopSpchsKey = "kMaxHoopSpchs";
	/** Authored data key for Hoop Char ID. */
	static constexpr const char *kHoopCharIDKey = "kHoopCharID";
	/** Authored data key for Hoop Spch ID. */
	static constexpr const char *kHoopSpchIDKey = "kHoopSpchID";
	/** Authored data key for Num Hoop Spchs. */
	static constexpr const char *kNumHoopSpchsKey = "kNumHoopSpchs";
	/** Script identifier for wAVE COMPICKUP. */
	static constexpr const char *kWAVECOMPICKUPName = "WAVE_COMPICKUP";
	/** Authored data key for Get Hoop Char ID. */
	static constexpr const char *kGetHoopCharIDKey = "kGetHoopCharID";
	/** Authored data key for Get Hoop Spch ID. */
	static constexpr const char *kGetHoopSpchIDKey = "kGetHoopSpchID";
	/** Script identifier for hSX1. */
	static constexpr const char *kHSX1Name = "HSX1";
	/** Script identifier for hSY1. */
	static constexpr const char *kHSY1Name = "HSY1";
	/** Script identifier for hSX2. */
	static constexpr const char *kHSX2Name = "HSX2";
	/** Script identifier for hSY2. */
	static constexpr const char *kHSY2Name = "HSY2";
	/** Authored data key for Hoop. */
	static constexpr const char *kHoopKey = "kHoop";
	/** Authored data key for Max Help Spchs.%d. */
	static constexpr const char *kMaxHelpSpchsSignedIndexKey = "kMaxHelpSpchs.%d";
	/** Format for script names or values: .%d.%d. */
	static constexpr const char *kSignedIndexSignedIndexFormat = ".%d.%d";
	/** Authored data key for Help Spch Char IDs. */
	static constexpr const char *kHelpSpchCharIDsKey = "kHelpSpchCharIDs";
	/** Authored data key for Help Spch IDs. */
	static constexpr const char *kHelpSpchIDsKey = "kHelpSpchIDs";
	/** Authored data key for Ladder. */
	static constexpr const char *kLadderKey = "kLadder";
	/** Script identifier for .1. */
	static constexpr const char *k1Name = ".1";
	/** Script identifier for .2. */
	static constexpr const char *k2Name = ".2";
	/** Script identifier for .3. */
	static constexpr const char *k3Name = ".3";
	/** Format for script names or values: item:%u. */
	static constexpr const char *kItemUnsignedIndexFormat = "item:%u";
	/** Authored data key for Rock Char ID.2.1.1. */
	static constexpr const char *kRockCharID211Key = "kRockCharID.2.1.1";
	/** Authored data key for Rock Spch ID.2.1.1. */
	static constexpr const char *kRockSpchID211Key = "kRockSpchID.2.1.1";
	/** Authored data key for Max Wrong Color Spchs. */
	static constexpr const char *kMaxWrongColorSpchsKey = "kMaxWrongColorSpchs";
	/** Authored data key for Wrong Color Char ID. */
	static constexpr const char *kWrongColorCharIDKey = "kWrongColorCharID";
	/** Authored data key for Wrong Color Spch ID. */
	static constexpr const char *kWrongColorSpchIDKey = "kWrongColorSpchID";
	/** Authored data key for Max Item Spchs. */
	static constexpr const char *kMaxItemSpchsKey = "kMaxItemSpchs";
	/** Authored data key for Item Char Spch. */
	static constexpr const char *kItemCharSpchKey = "kItemCharSpch";
	/** Authored data key for Item Spchs ID. */
	static constexpr const char *kItemSpchsIDKey = "kItemSpchsID";

	enum Phase {
		kIdle,
		kArrival,
		kReaderClimb,
		kSamClimb,
		kLadderWait,
		kHatWait,
		kHatSpeech,
		kCollect,
		kSpeech,
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Sprite displaying the ladder used to climb beside Rock Head. */
	Shell::Sprite *_ladder = nullptr;
	/** Sprite displaying the hat brought to Rock Head. */
	Shell::Sprite *_hat = nullptr;
	/** Collectible hoop sprite. */
	Shell::Sprite *_hoop = nullptr;
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kIdle;
	/** Saved progress through ladder, hat and hoop interactions. */
	int _state = 1;
	/** Current lower or upper character position beside Rock Head. */
	int _pose = 1;
	/** Counter cycling Rock Head's repeated requests. */
	int _moreClicks = 1;
	/** Millisecond timestamp when the current timed stage began. */
	uint32 _start = 0;
	/** Millisecond timestamp used to schedule idle feedback. */
	uint32 _idle = 0;
	/** Whether activity timestamps have been initialized. */
	bool _clock = false;
	/** Whether this is the first recorded visit to Rock Head. */
	bool _firstVisit = false;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Queue one authored actor and speech sequence. */
	void line(const Common::String &actors, const Common::String &sounds);
	/** Queue the requested number of lines from authored actor and speech sequences. */
	void group(const Common::String &actors, const Common::String &sounds, int count);
	/** Create or replace a named character using authored resources for a pose. */
	bool character(const char *name, const char *prefix, int pose);
	/** Create or replace the companion characters for the requested pose. */
	bool companions(int pose);
	/** Create or replace Rock Head's presentation for current progress. */
	bool rock();
	/** Queue the arrival conversation for the current visit and progress. */
	void entrySpeech();
	/** Queue Reader and Sam's response for the current progress. */
	void mainSpeech();
	/** Queue Rock Head's response for the current progress and character position. */
	void rockSpeech();
	/** Begin climbing or descending the ladder. */
	void climb();
	/** Handle the hoop interaction for the current character position. */
	void hoopClick();
	/** Test a game-pixel point against an authored rectangle selected by prefix and suffix. */
	bool hotspot(const char *prefix, const Common::String &suffix, Common::Point point) const;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
