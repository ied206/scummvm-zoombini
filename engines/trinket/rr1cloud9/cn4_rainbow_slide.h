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

#ifndef TRINKET_RR1CLOUD9_CN4_RAINBOW_SLIDE_H
#define TRINKET_RR1CLOUD9_CN4_RAINBOW_SLIDE_H

#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** Carrie's wagon exchange and the entrance to the rainbow slide. */
class RainbowSlide {
public:
	/** Connect the rainbow slide activity to its page services. */
	explicit RainbowSlide(PageScene &host) : _host(host) {}
	/** Load the rainbow slide resources and initialize the current round. */
	bool enter();
	/** Advance rainbow slide animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Handle a wagon or another offered inventory item and report whether the input was handled. */
	bool donate(uint item);
	/** Pause or resume slide animations, adjusting clocks by the elapsed pause in milliseconds. */
	void pause(bool paused, uint32 elapsed);
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _phase != kIdle; }
	/** Return the next page after a completed slide ride, or an empty string. */
	Common::String destination() const;

private:
	/** World-state key for r1 CN4 Game State. */
	static constexpr const char *kR1CN4GameStateKey = "R1CN4GameState";
	/** Format for script names or values: .%d. */
	static constexpr const char *kSignedIndexFormat = ".%d";
	/** World-state key for item:7. */
	static constexpr const char *kItem7Key = "item:7";
	/** World-state key for r1 CN4 Carrie Clicks. */
	static constexpr const char *kR1CN4CarrieClicksKey = "R1CN4CarrieClicks";
	/** Actor identifier for Carrie. */
	static constexpr const char *kCarrieActor = "carrie";
	/** Authored data key for Carrie. */
	static constexpr const char *kCarrieKey = "kCarrie";
	/** Authored data key for Stack ID. */
	static constexpr const char *kStackIDKey = "kStackID";
	/** Authored data key for Stack Z. */
	static constexpr const char *kStackZKey = "kStackZ";
	/** Authored data key for Stuff ID. */
	static constexpr const char *kStuffIDKey = "kStuffID";
	/** Authored data key for Stuff Z. */
	static constexpr const char *kStuffZKey = "kStuffZ";
	/** Script property name for animate Settled. */
	static constexpr const char *kPropertyAnimateSettled = "animateSettled";
	/** Script method name for add Fidget Group. */
	static constexpr const char *kMethodAddFidgetGroup = "addFidgetGroup";
	/** Script method name for add Fidget. */
	static constexpr const char *kMethodAddFidget = "addFidget";
	/** Authored data key for Carrie Wrap ID. */
	static constexpr const char *kCarrieWrapIDKey = "kCarrieWrapID";
	/** Authored data key for Bands ID. */
	static constexpr const char *kBandsIDKey = "kBandsID";
	/** Authored data key for Bands Z. */
	static constexpr const char *kBandsZKey = "kBandsZ";
	/** Authored data key for Wagon ID. */
	static constexpr const char *kWagonIDKey = "kWagonID";
	/** Authored data key for Wagon Z. */
	static constexpr const char *kWagonZKey = "kWagonZ";
	/** Authored data key for Walk To Slide ID. */
	static constexpr const char *kWalkToSlideIDKey = "kWalkToSlideID";
	/** World-state key for r1 CN4 Wagon Pending. */
	static constexpr const char *kR1CN4WagonPendingKey = "R1CN4WagonPending";
	/** World-state key for r1 CN4 Bands Pending. */
	static constexpr const char *kR1CN4BandsPendingKey = "R1CN4BandsPending";
	/** Authored data key for Bands Char ID. */
	static constexpr const char *kBandsCharIDKey = "kBandsCharID";
	/** Authored data key for Bands Spch ID. */
	static constexpr const char *kBandsSpchIDKey = "kBandsSpchID";
	/** World-state key for r1 CN4 Exit Pending. */
	static constexpr const char *kR1CN4ExitPendingKey = "R1CN4ExitPending";
	/** Authored data key for Enter Char ID. */
	static constexpr const char *kEnterCharIDKey = "kEnterCharID";
	/** Authored data key for Enter Spch ID. */
	static constexpr const char *kEnterSpchIDKey = "kEnterSpchID";
	/** Authored data key for Max Entry Spchs. */
	static constexpr const char *kMaxEntrySpchsKey = "kMaxEntrySpchs";
	/** World-state key for r1 CN4 State%d Count. */
	static constexpr const char *kR1CN4StateSignedIndexCountKey = "R1CN4State%dCount";
	/** Authored data key for Re Enter Char IDs. */
	static constexpr const char *kReEnterCharIDsKey = "kReEnterCharIDs";
	/** Authored data key for Re Enter Spch IDs. */
	static constexpr const char *kReEnterSpchIDsKey = "kReEnterSpchIDs";
	/** Authored data key for Max Re Enter Spchs.%d. */
	static constexpr const char *kMaxReEnterSpchsSignedIndexKey = "kMaxReEnterSpchs.%d";
	/** Script method name for remove Fidget Group. */
	static constexpr const char *kMethodRemoveFidgetGroup = "removeFidgetGroup";
	/** World-state key for r1 CN4 Wagon Here. */
	static constexpr const char *kR1CN4WagonHereKey = "R1CN4WagonHere";
	/** Authored data key for Wagon Char ID. */
	static constexpr const char *kWagonCharIDKey = "kWagonCharID";
	/** Authored data key for Wagon Spch ID. */
	static constexpr const char *kWagonSpchIDKey = "kWagonSpchID";
	/** Authored data key for Max Wagon Spch. */
	static constexpr const char *kMaxWagonSpchKey = "kMaxWagonSpch";
	/** World-state key for item:1. */
	static constexpr const char *kItem1Key = "item:1";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Script identifier for g Idle Help Time. */
	static constexpr const char *kGIdleHelpTimeName = "gIdleHelpTime";
	/** Authored data key for Max Idle Index. */
	static constexpr const char *kMaxIdleIndexKey = "kMaxIdleIndex";
	/** Authored data key for Idle Char ID. */
	static constexpr const char *kIdleCharIDKey = "kIdleCharID";
	/** Authored data key for Idle Spch ID. */
	static constexpr const char *kIdleSpchIDKey = "kIdleSpchID";
	/** Authored data key for Max Idle Index Speeches.%d. */
	static constexpr const char *kMaxIdleIndexSpeechesSignedIndexKey = "kMaxIdleIndexSpeeches.%d";
	/** Authored data key for Carrie Char ID.3.1. */
	static constexpr const char *kCarrieCharID31Key = "kCarrieCharID.3.1";
	/** Authored data key for Carrie Spch ID.3.1. */
	static constexpr const char *kCarrieSpchID31Key = "kCarrieSpchID.3.1";
	/** Authored data key for Carrie Char ID.3.%d. */
	static constexpr const char *kCarrieCharID3SignedIndexKey = "kCarrieCharID.3.%d";
	/** Authored data key for Carrie Spch ID.3.%d. */
	static constexpr const char *kCarrieSpchID3SignedIndexKey = "kCarrieSpchID.3.%d";
	/** Authored data key for Carrie Char ID. */
	static constexpr const char *kCarrieCharIDKey = "kCarrieCharID";
	/** Authored data key for Carrie Spch ID. */
	static constexpr const char *kCarrieSpchIDKey = "kCarrieSpchID";
	/** Authored data key for Max Carrie Spch.%d. */
	static constexpr const char *kMaxCarrieSpchSignedIndexKey = "kMaxCarrieSpch.%d";
	/** Script identifier for wAVE COMPICKUP. */
	static constexpr const char *kWAVECOMPICKUPName = "WAVE_COMPICKUP";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Authored data key for Help Char ID.%d. */
	static constexpr const char *kHelpCharIDSignedIndexKey = "kHelpCharID.%d";
	/** Authored data key for Help Spch ID.%d. */
	static constexpr const char *kHelpSpchIDSignedIndexKey = "kHelpSpchID.%d";
	/** Authored data key for Main Char ID. */
	static constexpr const char *kMainCharIDKey = "kMainCharID";
	/** Authored data key for Main Spch ID. */
	static constexpr const char *kMainSpchIDKey = "kMainSpchID";
	/** Authored data key for Max Main Spch.%d. */
	static constexpr const char *kMaxMainSpchSignedIndexKey = "kMaxMainSpch.%d";
	/** World-state key for r1 CN4 Slide Clicks. */
	static constexpr const char *kR1CN4SlideClicksKey = "R1CN4SlideClicks";
	/** Authored data key for Slide Char ID. */
	static constexpr const char *kSlideCharIDKey = "kSlideCharID";
	/** Authored data key for Slide Spch ID. */
	static constexpr const char *kSlideSpchIDKey = "kSlideSpchID";
	/** Authored data key for Max Slide Spch. */
	static constexpr const char *kMaxSlideSpchKey = "kMaxSlideSpch";
	/** Authored data key for INV1 Wagon. */
	static constexpr const char *kINV1WagonKey = "kINV1Wagon";
	/** Authored data key for No Use Char ID. */
	static constexpr const char *kNoUseCharIDKey = "kNoUseCharID";
	/** Authored data key for No Use Spch ID. */
	static constexpr const char *kNoUseSpchIDKey = "kNoUseSpchID";
	/** Authored data key for Max No Use Spch.%d. */
	static constexpr const char *kMaxNoUseSpchSignedIndexKey = "kMaxNoUseSpch.%d";

	enum Phase {
		kIdle,
		kDeliveryWait,
		kDeliverySpeech,
		kCollect,
		kExitSpeech,
		kExitAnimation,
		kExitReady,
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Sprite displaying the slide's rainbow bands. */
	Shell::Sprite *_bands = nullptr;
	/** Sprite displaying the donated wagon. */
	Shell::Sprite *_wagon = nullptr;
	/** Sprite used for the characters' walking transition. */
	Shell::Sprite *_walk = nullptr;
	/** Saved progress through wagon delivery and slide preparation. */
	int _state = 1;
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kIdle;
	/** Millisecond timestamp when the current timed stage began. */
	uint32 _start = 0;
	/** Millisecond timestamp used to schedule idle feedback. */
	uint32 _idle = 0;
	/** Whether activity timestamps have been initialized. */
	bool _clock = false;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Set rainbow-slide progress and store it in the world. */
	void setState(int state);
	/** Queue an authored actor and speech group with an optional variant index. */
	void group(const char *characters, const char *speech, int count, int index = 0);
	/** Queue Carrie's response for the current slide progress. */
	void carrieClick();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
