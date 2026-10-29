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

#ifndef TRINKET_RR1CLOUD9_LG2_CATAPULT_H
#define TRINKET_RR1CLOUD9_LG2_CATAPULT_H

#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** Feeding the catapult's guardian and launching back to Cloud Nine. */
class Catapult {
public:
	/** Connect the catapult ride activity to its page services. */
	explicit Catapult(PageScene &host) : _host(host) {}
	/** Load the catapult ride resources and initialize the current round. */
	bool enter();
	/** Advance the catapult ride presentation to its next ready stage. */
	void advance(uint32 time);
	/** Advance catapult ride animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Handle food or another offered inventory item and report whether the input was handled. */
	bool donate(uint item);
	/** Pause or resume catapult animations, adjusting clocks by the elapsed pause in milliseconds. */
	void pause(bool paused, uint32 elapsed);
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _phase != kIdle; }
	/** Return the next page after a completed catapult ride, or an empty string. */
	Common::String destination() const;

private:
	/** Format for script names or values: .%d. */
	static constexpr const char *kSignedIndexFormat = ".%d";
	/** World-state key for r1 LG2 Pet Sleeping. */
	static constexpr const char *kR1LG2PetSleepingKey = "R1LG2PetSleeping";
	/** World-state key for r1 LG2 Catapult Ready. */
	static constexpr const char *kR1LG2CatapultReadyKey = "R1LG2CatapultReady";
	/** World-state key for r1 LG2 Feed Pending. */
	static constexpr const char *kR1LG2FeedPendingKey = "R1LG2FeedPending";
	/** Script property name for pet.1. */
	static constexpr const char *kPropertyPet1 = "pet.1";
	/** Script identifier for g Pet. */
	static constexpr const char *kGPetName = "gPet";
	/** Script property name for z. */
	static constexpr const char *kPropertyZ = "z";
	/** Authored data key for Pet Z. */
	static constexpr const char *kPetZKey = "kPetZ";
	/** Authored data key for Bowl ID. */
	static constexpr const char *kBowlIDKey = "kBowlID";
	/** Authored data key for Bowl Z. */
	static constexpr const char *kBowlZKey = "kBowlZ";
	/** Authored data key for Sandwich ID. */
	static constexpr const char *kSandwichIDKey = "kSandwichID";
	/** Authored data key for Sandwich Z. */
	static constexpr const char *kSandwichZKey = "kSandwichZ";
	/** Authored data key for Eating ID. */
	static constexpr const char *kEatingIDKey = "kEatingID";
	/** Script identifier for g Pet Sleep ID. */
	static constexpr const char *kGPetSleepIDName = "gPetSleepID";
	/** Authored data key for Clump Static ID. */
	static constexpr const char *kClumpStaticIDKey = "kClumpStaticID";
	/** Authored data key for Clump Z. */
	static constexpr const char *kClumpZKey = "kClumpZ";
	/** Authored data key for Clump ID. */
	static constexpr const char *kClumpIDKey = "kClumpID";
	/** Authored data key for Catapult ID. */
	static constexpr const char *kCatapultIDKey = "kCatapultID";
	/** Authored data key for Catapult Z. */
	static constexpr const char *kCatapultZKey = "kCatapultZ";
	/** Authored data key for Exit Anim ID. */
	static constexpr const char *kExitAnimIDKey = "kExitAnimID";
	/** World-state key for r1 LG2 Exit Pending. */
	static constexpr const char *kR1LG2ExitPendingKey = "R1LG2ExitPending";
	/** Authored data key for Enter Char ID. */
	static constexpr const char *kEnterCharIDKey = "kEnterCharID";
	/** Authored data key for Enter Spch ID. */
	static constexpr const char *kEnterSpchIDKey = "kEnterSpchID";
	/** Authored data key for Max Entry Spchs. */
	static constexpr const char *kMaxEntrySpchsKey = "kMaxEntrySpchs";
	/** Authored data key for Re Enter Char IDs. */
	static constexpr const char *kReEnterCharIDsKey = "kReEnterCharIDs";
	/** Authored data key for Re Enter Spch IDs. */
	static constexpr const char *kReEnterSpchIDsKey = "kReEnterSpchIDs";
	/** Authored data key for Max Re Enter Spchs. */
	static constexpr const char *kMaxReEnterSpchsKey = "kMaxReEnterSpchs";
	/** Authored data key for Pet Sleep ID. */
	static constexpr const char *kPetSleepIDKey = "kPetSleepID";
	/** Authored data key for Feeding SFXID. */
	static constexpr const char *kFeedingSFXIDKey = "kFeedingSFXID";
	/** World-state key for r1 LG2 Feed Creature. */
	static constexpr const char *kR1LG2FeedCreatureKey = "R1LG2FeedCreature";
	/** Authored data key for Sandwich Char ID. */
	static constexpr const char *kSandwichCharIDKey = "kSandwichCharID";
	/** Authored data key for Sandwich Spch ID. */
	static constexpr const char *kSandwichSpchIDKey = "kSandwichSpchID";
	/** Authored data key for Max Sandwich Spchs. */
	static constexpr const char *kMaxSandwichSpchsKey = "kMaxSandwichSpchs";
	/** Authored data key for Post Eat Char ID. */
	static constexpr const char *kPostEatCharIDKey = "kPostEatCharID";
	/** Authored data key for Post Eat Spch ID. */
	static constexpr const char *kPostEatSpchIDKey = "kPostEatSpchID";
	/** Authored data key for Max Post Eat Spchs. */
	static constexpr const char *kMaxPostEatSpchsKey = "kMaxPostEatSpchs";
	/** Authored data key for Lets Go Char ID. */
	static constexpr const char *kLetsGoCharIDKey = "kLetsGoCharID";
	/** Authored data key for Lets Go Spch ID. */
	static constexpr const char *kLetsGoSpchIDKey = "kLetsGoSpchID";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Script identifier for g Idle Help Time. */
	static constexpr const char *kGIdleHelpTimeName = "gIdleHelpTime";
	/** Authored data key for Idle Char ID. */
	static constexpr const char *kIdleCharIDKey = "kIdleCharID";
	/** Authored data key for Idle Spch ID. */
	static constexpr const char *kIdleSpchIDKey = "kIdleSpchID";
	/** Authored data key for Max Idle Spchs. */
	static constexpr const char *kMaxIdleSpchsKey = "kMaxIdleSpchs";
	/** Authored data key for INV1 Food. */
	static constexpr const char *kINV1FoodKey = "kINV1Food";
	/** World-state key for item:9. */
	static constexpr const char *kItem9Key = "item:9";
	/** Authored data key for Item Char Spch2. */
	static constexpr const char *kItemCharSpch2Key = "kItemCharSpch2";
	/** Authored data key for Item Spchs ID2. */
	static constexpr const char *kItemSpchsID2Key = "kItemSpchsID2";
	/** Authored data key for Max Item Spchs2. */
	static constexpr const char *kMaxItemSpchs2Key = "kMaxItemSpchs2";
	/** Authored data key for Item Char Spch. */
	static constexpr const char *kItemCharSpchKey = "kItemCharSpch";
	/** Authored data key for Item Spchs ID. */
	static constexpr const char *kItemSpchsIDKey = "kItemSpchsID";
	/** Authored data key for Item Spch Count. */
	static constexpr const char *kItemSpchCountKey = "kItemSpchCount";
	/** Authored data key for Max Item Spchs. */
	static constexpr const char *kMaxItemSpchsKey = "kMaxItemSpchs";
	/** World-state key for r1 LG2 Rode Catapult. */
	static constexpr const char *kR1LG2RodeCatapultKey = "R1LG2RodeCatapult";
	/** Authored data key for First Exit Char ID. */
	static constexpr const char *kFirstExitCharIDKey = "kFirstExitCharID";
	/** Authored data key for First Exit Spch ID. */
	static constexpr const char *kFirstExitSpchIDKey = "kFirstExitSpchID";
	/** Authored data key for Sub Seq Exit Char ID. */
	static constexpr const char *kSubSeqExitCharIDKey = "kSubSeqExitCharID";
	/** Authored data key for Sub Seq Exit Spch ID. */
	static constexpr const char *kSubSeqExitSpchIDKey = "kSubSeqExitSpchID";
	/** Authored data key for Bowl Char ID. */
	static constexpr const char *kBowlCharIDKey = "kBowlCharID";
	/** Authored data key for Bowl Spch ID. */
	static constexpr const char *kBowlSpchIDKey = "kBowlSpchID";
	/** Authored data key for Max Bowl Spchs. */
	static constexpr const char *kMaxBowlSpchsKey = "kMaxBowlSpchs";
	/** Authored data key for Pet Char IDs. */
	static constexpr const char *kPetCharIDsKey = "kPetCharIDs";
	/** Authored data key for Pet Spch IDs. */
	static constexpr const char *kPetSpchIDsKey = "kPetSpchIDs";
	/** Authored data key for Max Pet Spchs. */
	static constexpr const char *kMaxPetSpchsKey = "kMaxPetSpchs";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Script identifier for .2.1. */
	static constexpr const char *k21Name = ".2.1";
	/** Format for script names or values: .1.%d. */
	static constexpr const char *k1SignedIndexFormat = ".1.%d";
	/** Authored data key for Max Help Spchs.1. */
	static constexpr const char *kMaxHelpSpchs1Key = "kMaxHelpSpchs.1";
	/** Authored data key for Help Spch Char IDs. */
	static constexpr const char *kHelpSpchCharIDsKey = "kHelpSpchCharIDs";
	/** Authored data key for Help Spch IDs. */
	static constexpr const char *kHelpSpchIDsKey = "kHelpSpchIDs";
	/** Authored data key for Main Spch Char IDs. */
	static constexpr const char *kMainSpchCharIDsKey = "kMainSpchCharIDs";
	/** Authored data key for Main Spch IDs. */
	static constexpr const char *kMainSpchIDsKey = "kMainSpchIDs";
	/** Authored data key for Max Main Spchs. */
	static constexpr const char *kMaxMainSpchsKey = "kMaxMainSpchs";
	/** Authored data key for Dont Exit Char IDs. */
	static constexpr const char *kDontExitCharIDsKey = "kDontExitCharIDs";
	/** Authored data key for Dont Exit Spch IDs. */
	static constexpr const char *kDontExitSpchIDsKey = "kDontExitSpchIDs";
	/** Authored data key for Max Dont Exit Spchs. */
	static constexpr const char *kMaxDontExitSpchsKey = "kMaxDontExitSpchs";
	/** Authored data key for Num Dont Exit Spchs. */
	static constexpr const char *kNumDontExitSpchsKey = "kNumDontExitSpchs";

	enum Phase {
		kIdle,
		kFeedWait,
		kFeedSpeech,
		kEating,
		kSpeech,
		kExitSpeech,
		kExiting,
		kExitReady,
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Sprite displaying the pet's food bowl. */
	Shell::Sprite *_bowl = nullptr;
	/** Sprite displaying the food offered to the pet. */
	Shell::Sprite *_sandwich = nullptr;
	/** Sprite used during the pet's eating sequence. */
	Shell::Sprite *_eating = nullptr;
	/** Sprite used for the sleeping pet and snoring animation. */
	Shell::Sprite *_sleep = nullptr;
	/** Static sprite for the ambient clump presentation. */
	Shell::Sprite *_clump = nullptr;
	/** Animated sprite for the ambient clump presentation. */
	Shell::Sprite *_clumpAnim = nullptr;
	/** Catapult mechanism sprite used during the ride. */
	Shell::Sprite *_catapult = nullptr;
	/** Sprite used for the characters' departure. */
	Shell::Sprite *_exit = nullptr;
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kIdle;
	/** Whether the pet is asleep and the catapult ride is available. */
	bool _sleeping = false;
	/** Whether activity timestamps have been initialized. */
	bool _clock = false;
	/** Whether the clump's ambient animation is currently playing. */
	bool _ambientPlaying = false;
	/** Whether the pet's snoring animation is currently playing. */
	bool _snoring = false;
	/** Millisecond timestamp when the current timed stage began. */
	uint32 _start = 0;
	/** Millisecond timestamp used to schedule idle feedback. */
	uint32 _idle = 0;
	/** Millisecond timestamp used to schedule the next clump animation. */
	uint32 _ambientStart = 0;
	/** Millisecond timestamp used to schedule the next snoring animation. */
	uint32 _sleepStart = 0;
	/** Delay between clump animations, in milliseconds. */
	uint _ambientDelay = 1000;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Queue one authored actor and speech sequence with an optional variant index. */
	void line(const char *actors, const char *sounds, int index = 0);
	/** Queue an authored actor and speech group with an optional variant index. */
	void group(const char *actors, const char *sounds, const char *count, int index = 0);
	/** Begin the catapult ride and queue its departure conversation. */
	void ride();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
