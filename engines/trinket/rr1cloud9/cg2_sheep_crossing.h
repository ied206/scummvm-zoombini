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

#ifndef TRINKET_RR1CLOUD9_CG2_SHEEP_CROSSING_H
#define TRINKET_RR1CLOUD9_CG2_SHEEP_CROSSING_H

#include "trinket/ompp/sound.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** Whistle-controlled sheep crossing between the palace and rainbow slide. */
class SheepCrossing {
public:
	/** Connect the sheep-crossing activity to its page services. */
	explicit SheepCrossing(PageScene &host) : _host(host) {}
	/** Release the sheep-crossing presentation state. */
	~SheepCrossing() { delete _sound; }
	/** Load the sheep-crossing resources and initialize the current round. */
	bool enter();
	/** Advance sheep-crossing animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Handle an inventory item offered to the sheep and report whether it was consumed as input. */
	bool donate(uint item);
	/** Pause or resume sheep animations and sound, adjusting clocks by the elapsed pause in milliseconds. */
	void pause(bool paused, uint32 elapsed);
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _phase != kIdle; }
	/** Return the page selected by the completed crossing interaction. */
	Common::String destination() const { return _destination; }

private:
	/** World-state key for r1 CG2 State. */
	static constexpr const char *kR1CG2StateKey = "R1CG2State";
	/** World-state key for item:5. */
	static constexpr const char *kItem5Key = "item:5";
	/** World-state key for r1 CG2 Idle Count. */
	static constexpr const char *kR1CG2IdleCountKey = "R1CG2IdleCount";
	/** World-state key for r1 CG2 Main Clicks. */
	static constexpr const char *kR1CG2MainClicksKey = "R1CG2MainClicks";
	/** World-state key for r1 CG2 Sheep Clicks. */
	static constexpr const char *kR1CG2SheepClicksKey = "R1CG2SheepClicks";
	/** Authored data key for Max Sheep. */
	static constexpr const char *kMaxSheepKey = "kMaxSheep";
	/** Authored data key for Sheep Loc.%d. */
	static constexpr const char *kSheepLocSignedIndexKey = "kSheepLoc.%d.";
	/** Authored data key for Sheep ID. */
	static constexpr const char *kSheepIDKey = "kSheepID";
	/** Script identifier for 3. */
	static constexpr const char *k3Name = "3";
	/** Authored data key for Sheep Standing ID. */
	static constexpr const char *kSheepStandingIDKey = "kSheepStandingID";
	/** Script identifier for 1. */
	static constexpr const char *k1Name = "1";
	/** Script identifier for 4. */
	static constexpr const char *k4Name = "4";
	/** Script identifier for 2. */
	static constexpr const char *k2Name = "2";
	/** Authored data key for Blow Whistle ID. */
	static constexpr const char *kBlowWhistleIDKey = "kBlowWhistleID";
	/** Authored data key for Sam Z. */
	static constexpr const char *kSamZKey = "kSamZ";
	/** World-state key for r1 CG2 Whistle Pending. */
	static constexpr const char *kR1CG2WhistlePendingKey = "R1CG2WhistlePending";
	/** Authored data key for Enter Char ID. */
	static constexpr const char *kEnterCharIDKey = "kEnterCharID";
	/** Authored data key for Enter Spch ID. */
	static constexpr const char *kEnterSpchIDKey = "kEnterSpchID";
	/** Authored data key for Max Entry Spchs. */
	static constexpr const char *kMaxEntrySpchsKey = "kMaxEntrySpchs";
	/** World-state key for r1 CG2 State2 Count. */
	static constexpr const char *kR1CG2State2CountKey = "R1CG2State2Count";
	/** World-state key for previous Location. */
	static constexpr const char *kPreviousLocationKey = "previousLocation";
	/** World-state key for r1 CG2 State3 Count. */
	static constexpr const char *kR1CG2State3CountKey = "R1CG2State3Count";
	/** Authored data key for Re Enter Char IDs. */
	static constexpr const char *kReEnterCharIDsKey = "kReEnterCharIDs";
	/** Authored data key for Re Enter Spch IDs. */
	static constexpr const char *kReEnterSpchIDsKey = "kReEnterSpchIDs";
	/** Authored data key for Num Re Enter Spchs. */
	static constexpr const char *kNumReEnterSpchsKey = "kNumReEnterSpchs";
	/** Format for script names or values: .%d.%d. */
	static constexpr const char *kSignedIndexSignedIndexFormat = ".%d.%d";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Authored data key for Sheep Standing SFXID. */
	static constexpr const char *kSheepStandingSFXIDKey = "kSheepStandingSFXID";
	/** Authored data key for Sheep Loc.%u. */
	static constexpr const char *kSheepLocUnsignedIndexKey = "kSheepLoc.%u.";
	/** World-state key for r1 CG2 Blow Count. */
	static constexpr const char *kR1CG2BlowCountKey = "R1CG2BlowCount";
	/** Authored data key for Works Char ID. */
	static constexpr const char *kWorksCharIDKey = "kWorksCharID";
	/** Authored data key for Works Spch ID. */
	static constexpr const char *kWorksSpchIDKey = "kWorksSpchID";
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
	/** Format for script names or values: .%d. */
	static constexpr const char *kSignedIndexFormat = ".%d";
	/** Authored data key for Previous Locations.1. */
	static constexpr const char *kPreviousLocations1Key = "kPreviousLocations.1";
	/** Authored data key for Max Sheep Index. */
	static constexpr const char *kMaxSheepIndexKey = "kMaxSheepIndex";
	/** Authored data key for Max Main Index. */
	static constexpr const char *kMaxMainIndexKey = "kMaxMainIndex";
	/** Authored data key for Sheep Char ID. */
	static constexpr const char *kSheepCharIDKey = "kSheepCharID";
	/** Authored data key for Main Char ID. */
	static constexpr const char *kMainCharIDKey = "kMainCharID";
	/** Authored data key for Sheep Spch ID. */
	static constexpr const char *kSheepSpchIDKey = "kSheepSpchID";
	/** Authored data key for Main Spch ID. */
	static constexpr const char *kMainSpchIDKey = "kMainSpchID";
	/** Authored data key for Max Sheep Index Speeches. */
	static constexpr const char *kMaxSheepIndexSpeechesKey = "kMaxSheepIndexSpeeches";
	/** Authored data key for Max Main Index Speeches. */
	static constexpr const char *kMaxMainIndexSpeechesKey = "kMaxMainIndexSpeeches";
	/** Script identifier for hSX1. */
	static constexpr const char *kHSX1Name = "HSX1";
	/** Script identifier for hSY1. */
	static constexpr const char *kHSY1Name = "HSY1";
	/** Script identifier for hSX2. */
	static constexpr const char *kHSX2Name = "HSX2";
	/** Script identifier for hSY2. */
	static constexpr const char *kHSY2Name = "HSY2";
	/** Authored data key for Sign. */
	static constexpr const char *kSignKey = "kSign";
	/** Script identifier for wAVE COMSOFTCLICK. */
	static constexpr const char *kWAVECOMSOFTCLICKName = "WAVE_COMSOFTCLICK";
	/** Authored data key for Sign Char ID. */
	static constexpr const char *kSignCharIDKey = "kSignCharID";
	/** Authored data key for Sign Spch ID. */
	static constexpr const char *kSignSpchIDKey = "kSignSpchID";
	/** Authored data key for Sheep. */
	static constexpr const char *kSheepKey = "kSheep";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Authored data key for Num Help Spchs. */
	static constexpr const char *kNumHelpSpchsKey = "kNumHelpSpchs";
	/** Authored data key for Help Spch Char IDs. */
	static constexpr const char *kHelpSpchCharIDsKey = "kHelpSpchCharIDs";
	/** Authored data key for Help Spch IDs. */
	static constexpr const char *kHelpSpchIDsKey = "kHelpSpchIDs";
	/** Authored data key for INV1 Whistle. */
	static constexpr const char *kINV1WhistleKey = "kINV1Whistle";
	/** Authored data key for Wrong Item Char ID. */
	static constexpr const char *kWrongItemCharIDKey = "kWrongItemCharID";
	/** Authored data key for Wrong Item Spch ID. */
	static constexpr const char *kWrongItemSpchIDKey = "kWrongItemSpchID";
	/** Authored data key for Max Item Spchs. */
	static constexpr const char *kMaxItemSpchsKey = "kMaxItemSpchs";
	/** Authored data key for Item Char Spch. */
	static constexpr const char *kItemCharSpchKey = "kItemCharSpch";
	/** Authored data key for Item Spchs ID. */
	static constexpr const char *kItemSpchsIDKey = "kItemSpchsID";

	enum Phase {
		kIdle,
		kWait,
		kBlow,
		kStand,
		kSpeech,
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Sprites used while the sheep are walking across the path. */
	Common::Array<Shell::Sprite *> _walking;
	/** Sprites used while the sheep stand clear of the path. */
	Common::Array<Shell::Sprite *> _standing;
	/** Sprite used for the whistle-blowing interaction. */
	Shell::Sprite *_whistle = nullptr;
	/** Sound played during the sheep-crossing presentation. */
	OM::Sound *_sound = nullptr;
	/** Pending page transition, or an empty string while remaining here. */
	Common::String _destination;
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kIdle;
	/** Saved progress through finding the whistle and passing the sheep. */
	int _state = 1;
	/** Whether the sheep are still moving across the path. */
	bool _moving = true;
	/** Whether activity timestamps have been initialized. */
	bool _clock = false;
	/** Millisecond timestamp when the current timed stage began. */
	uint32 _start = 0;
	/** Millisecond timestamp used to schedule idle feedback. */
	uint32 _idle = 0;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Increment a saved world counter and return its new value. */
	int increment(const char *key);
	/** Queue an authored actor and speech sequence selected by a key suffix. */
	void group(const char *actors, const char *sounds, const char *count, const Common::String &suffix);
	/** Queue context-sensitive speech for a sheep or the main characters. */
	void talk(bool sheep);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
