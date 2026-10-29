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

#ifndef TRINKET_RR1CLOUD9_PAGE_SCENE_H
#define TRINKET_RR1CLOUD9_PAGE_SCENE_H

#include "trinket/rr1cloud9/page.h"
#include "trinket/rr1cloud9/world.h"
#include "trinket/shell/character.h"
#include "trinket/shell/text_object.h"

namespace Trinket {

class TrinketEngine;
namespace RR1Cloud9 {

class SkyRide;
class RainGame;
class Catapult;
class SheepCrossing;
class RockHead;
class RainbowSlide;
class Gumball;
class Sandwich;
class NumberTree;
class WordPots;
class CloudFloors;
class Gallery;
class Flipbook;
class MusicPatterns;

class PageScene : private Shell::Character::Resources {
public:
	/** Connect RR1 page presentation to the engine, scene and saved world state. */
	PageScene(TrinketEngine &vm, Shell::Scene &scene, World &world);
	/** Clear the current page presentation and activity state. */
	~PageScene() { clear(); }
	/** Stop page playback and release activities, characters, inventory graphics and cursors. */
	void clear();
	/** Save pending artwork edits from the active flipbook editor. */
	void checkpoint() const;
	/** Initialize the named page and its activity, optionally in practice mode; return success. */
	bool enter(const Page &page, const Common::String &name, bool practice);
	/** Advance page speech, inventory, activity and transition state at a millisecond timestamp. */
	void update(uint32 time);
	/** Draw activity graphics and inventory item counts over the scene. */
	void draw(OM::Canvas &canvas);
	/** Start the supplied sound resource IDs as page sound effects. */
	void playSounds(const Common::Array<uint16> &sounds);
	/** Pause or resume page playback, adjusting timers using the supplied millisecond timestamp. */
	void pause(bool paused, uint32 time);
	/** Handle a game-pixel click and return a requested destination, or an empty string. */
	Common::String click(Common::Point point);
	/** Dispatch pointer or keyboard input to the active page activity and report whether it was consumed. */
	bool event(const Common::Event &event);
	/** Record the latest pointer position in game pixels. */
	void setMousePosition(Common::Point point) { _mouse = point; }
	/** Display the authored exit cursor when the pointer and page state permit navigation. */
	void updateNavigationCursor();
	/** Remove the active exit cursor and restore the preceding cursor. */
	void releaseNavigationCursor();
	/** Return whether a page is currently initialized. */
	bool active() const { return !_name.empty(); }
	/** Return whether inventory or activity state prevents opening the map. */
	bool blocksMap() const;
	/** Return whether the Queen conversation requests its movie. */
	bool wantsQueenMovie() const { return !_queenArriving && _queenPhase == kQueenMovie; }
	/** Return whether the Queen departure sequence is ready to leave the page. */
	bool wantsQueenExit() const { return !_queenArriving && _queenPhase == kQueenExitReady; }
	/** Return whether the inventor conversation requests its movie. */
	bool wantsInventorMovie() const { return _inventorPhase == kInventorMovie; }
	/** Continue the Queen conversation after its movie completes. */
	void queenMovieFinished();
	/** Return whether the active gallery requests its movie. */
	bool wantsGalleryMovie() const;
	/** Return the gallery's pending destination code, or zero when none is requested. */
	int galleryDestination() const;
	/** Return an activity's pending page transition, or an empty string. */
	Common::String destination() const;

	/**
	 * Map a zero-based music marble color consistently across rewards, inventory and the music puzzle.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 */
	static OM::Canvas::ColorRemap marbleColor(uint color);

private:
	/** Script identifier for queen. */
	static constexpr const char *kQueenName = "queen";
	/** Authored data key for Queen. */
	static constexpr const char *kQueenKey = "kQueen";
	/** World-state key for r1 CN2 State. */
	static constexpr const char *kR1CN2StateKey = "R1CN2State";
	/** World-state key for talked To Inventor. */
	static constexpr const char *kTalkedToInventorKey = "talkedToInventor";
	/** Script identifier for rBands. */
	static constexpr const char *kRBandsName = "RBands";
	/** Script identifier for hula Hoop. */
	static constexpr const char *kHulaHoopName = "HulaHoop";
	/** Script identifier for pillow. */
	static constexpr const char *kPillowName = "Pillow";
	/** Authored data key for INV1. */
	static constexpr const char *kINV1Key = "kINV1";
	/** World-state key for item:%d. */
	static constexpr const char *kItemSignedIndexKey = "item:%d";
	/** World-state key for r1 LN2. */
	static constexpr const char *kR1LN2Key = "R1LN2";
	/** World-state key for delivered. */
	static constexpr const char *kDeliveredKey = "Delivered";
	/** World-state key for r1 CN2 Reentry. */
	static constexpr const char *kR1CN2ReentryKey = "R1CN2Reentry";
	/** World-state key for previous Location. */
	static constexpr const char *kPreviousLocationKey = "previousLocation";
	/** World-state key for r1 CN2 Catapult Reentry. */
	static constexpr const char *kR1CN2CatapultReentryKey = "R1CN2CatapultReentry";
	/** World-state key for r1 CN3 From Street. */
	static constexpr const char *kR1CN3FromStreetKey = "R1CN3FromStreet";
	/** Authored data key for Litter ID. */
	static constexpr const char *kLitterIDKey = "kLitterID";
	/** Authored data key for Litter Z. */
	static constexpr const char *kLitterZKey = "kLitterZ";
	/** Authored data key for Sign ID. */
	static constexpr const char *kSignIDKey = "kSignID";
	/** Authored data key for Sign Z. */
	static constexpr const char *kSignZKey = "kSignZ";
	/** Authored data key for Ambient IDs.1. */
	static constexpr const char *kAmbientIDs1Key = "kAmbientIDs.1";
	/** Authored data key for Ambient Z. */
	static constexpr const char *kAmbientZKey = "kAmbientZ";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Authored data key for Reader Walk In ID. */
	static constexpr const char *kReaderWalkInIDKey = "kReaderWalkInID";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Authored data key for Sam Walk In ID. */
	static constexpr const char *kSamWalkInIDKey = "kSamWalkInID";
	/** World-state key for r1 CN2 Welcome Pending. */
	static constexpr const char *kR1CN2WelcomePendingKey = "R1CN2WelcomePending";
	/** World-state key for r1 CN2 Exit Pending. */
	static constexpr const char *kR1CN2ExitPendingKey = "R1CN2ExitPending";
	/** Authored data key for Bye ID. */
	static constexpr const char *kByeIDKey = "kByeID";
	/** Authored data key for Enter Spch IDs. */
	static constexpr const char *kEnterSpchIDsKey = "kEnterSpchIDs";
	/** Authored data key for Delay Time. */
	static constexpr const char *kDelayTimeKey = "kDelayTime";
	/** Authored data key for Ambient IDs.%u. */
	static constexpr const char *kAmbientIDsUnsignedIndexKey = "kAmbientIDs.%u";
	/** Authored data key for Re Enter Spch IDs.%u.%u. */
	static constexpr const char *kReEnterSpchIDsUnsignedIndexUnsignedIndexKey = "kReEnterSpchIDs.%u.%u";
	/** Script identifier for rsr. */
	static constexpr const char *kRsrName = "rsr";
	/** Authored data key for Re Enter Spch IDs.%d. */
	static constexpr const char *kReEnterSpchIDsSignedIndexKey = "kReEnterSpchIDs.%d";
	/** Script identifier for qrq. */
	static constexpr const char *kQrqName = "qrq";
	/** Script identifier for qsrsrsrsrsq. */
	static constexpr const char *kQsrsrsrsrsqName = "qsrsrsrsrsq";
	/** Script identifier for qr. */
	static constexpr const char *kQrName = "qr";
	/** Script identifier for qrs. */
	static constexpr const char *kQrsName = "qrs";
	/** World-state key for r1 LN2 Hula Hoop Delivered. */
	static constexpr const char *kR1LN2HulaHoopDeliveredKey = "R1LN2HulaHoopDelivered";
	/** World-state key for r1 LN2 RBands Delivered. */
	static constexpr const char *kR1LN2RBandsDeliveredKey = "R1LN2RBandsDelivered";
	/** World-state key for r1 LN2 Pillow Delivered. */
	static constexpr const char *kR1LN2PillowDeliveredKey = "R1LN2PillowDelivered";
	/** Authored data key for Re Enter Spch IDs.11.1. */
	static constexpr const char *kReEnterSpchIDs111Key = "kReEnterSpchIDs.11.1";
	/** Authored data key for Re Enter Spch IDs.11.%u. */
	static constexpr const char *kReEnterSpchIDs11UnsignedIndexKey = "kReEnterSpchIDs.11.%u";
	/** Authored data key for Re Enter Spch IDs.11.5. */
	static constexpr const char *kReEnterSpchIDs115Key = "kReEnterSpchIDs.11.5";
	/** Authored data key for Re Enter Spch IDs.12.%u. */
	static constexpr const char *kReEnterSpchIDs12UnsignedIndexKey = "kReEnterSpchIDs.12.%u";
	/** Authored data key for Re Enter Spch IDs.12.4. */
	static constexpr const char *kReEnterSpchIDs124Key = "kReEnterSpchIDs.12.4";
	/** Script identifier for qqsrq. */
	static constexpr const char *kQqsrqName = "qqsrq";
	/** Script identifier for qqr. */
	static constexpr const char *kQqrName = "qqr";
	/** Authored data key for Welcome Char ID.1. */
	static constexpr const char *kWelcomeCharID1Key = "kWelcomeCharID.1";
	/** Authored data key for Welcome Spch ID.1. */
	static constexpr const char *kWelcomeSpchID1Key = "kWelcomeSpchID.1";
	/** Authored data key for Num Welcome Spch.1. */
	static constexpr const char *kNumWelcomeSpch1Key = "kNumWelcomeSpch.1";
	/** Authored data key for Welcome Char ID.2. */
	static constexpr const char *kWelcomeCharID2Key = "kWelcomeCharID.2";
	/** Authored data key for Welcome Spch ID.2. */
	static constexpr const char *kWelcomeSpchID2Key = "kWelcomeSpchID.2";
	/** Authored data key for Num Welcome Spch.2. */
	static constexpr const char *kNumWelcomeSpch2Key = "kNumWelcomeSpch.2";
	/** Authored data key for Song Intro ID. */
	static constexpr const char *kSongIntroIDKey = "kSongIntroID";
	/** Script identifier for g Idle Help Time. */
	static constexpr const char *kGIdleHelpTimeName = "gIdleHelpTime";
	/** Authored data key for Idle Spch IDs.%u. */
	static constexpr const char *kIdleSpchIDsUnsignedIndexKey = "kIdleSpchIDs.%u";
	/** Script identifier for .1. */
	static constexpr const char *k1Name = ".1";
	/** Script identifier for .2. */
	static constexpr const char *k2Name = ".2";
	/** Authored path suffix selecting the third speech entry. */
	static constexpr const char *k3Name = ".3";
	/** Authored data key for Background Music IDs.2. */
	static constexpr const char *kBackgroundMusicIDs2Key = "kBackgroundMusicIDs.2";
	/** Authored data key for Welcome Char ID.4. */
	static constexpr const char *kWelcomeCharID4Key = "kWelcomeCharID.4";
	/** Authored data key for Welcome Spch ID.4. */
	static constexpr const char *kWelcomeSpchID4Key = "kWelcomeSpchID.4";
	/** Authored data key for Num Welcome Spch.4. */
	static constexpr const char *kNumWelcomeSpch4Key = "kNumWelcomeSpch.4";
	/** Authored data key for Welcome Char ID.5. */
	static constexpr const char *kWelcomeCharID5Key = "kWelcomeCharID.5";
	/** Authored data key for Welcome Spch ID.5. */
	static constexpr const char *kWelcomeSpchID5Key = "kWelcomeSpchID.5";
	/** Authored data key for Num Welcome Spch.5. */
	static constexpr const char *kNumWelcomeSpch5Key = "kNumWelcomeSpch.5";
	/** World-state key for r1 CN2 Sign Click Count. */
	static constexpr const char *kR1CN2SignClickCountKey = "R1CN2SignClickCount";
	/** Authored data key for Nav Loc CN3 Sign. */
	static constexpr const char *kNavLocCN3SignKey = "kNavLocCN3Sign";
	/** Authored data key for Num Nav Hotspots.%d. */
	static constexpr const char *kNumNavHotspotsSignedIndexKey = "kNumNavHotspots.%d";
	/** Format for script names or values: .%d.%d. */
	static constexpr const char *kSignedIndexSignedIndexFormat = ".%d.%d";
	/** Authored data key for Nav HSX1. */
	static constexpr const char *kNavHSX1Key = "kNavHSX1";
	/** Authored data key for Nav HSY1. */
	static constexpr const char *kNavHSY1Key = "kNavHSY1";
	/** Authored data key for Nav HSX2. */
	static constexpr const char *kNavHSX2Key = "kNavHSX2";
	/** Authored data key for Nav HSY2. */
	static constexpr const char *kNavHSY2Key = "kNavHSY2";
	/** Script identifier for handled. */
	static constexpr const char *kHandledName = "handled";
	/** Authored data key for Instr Spch IDs.2.%u. */
	static constexpr const char *kInstrSpchIDs2UnsignedIndexKey = "kInstrSpchIDs.2.%u";
	/** Authored data key for Instr Spch IDs.3. */
	static constexpr const char *kInstrSpchIDs3Key = "kInstrSpchIDs.3";
	/** World-state key for whistle. */
	static constexpr const char *kWhistleKey = "Whistle";
	/** Authored data key for Instr Spch IDs.%u.1. */
	static constexpr const char *kInstrSpchIDsUnsignedIndex1Key = "kInstrSpchIDs.%u.1";
	/** Authored data key for Instr Spch IDs.%u.2. */
	static constexpr const char *kInstrSpchIDsUnsignedIndex2Key = "kInstrSpchIDs.%u.2";
	/** Authored data key for Instr Spch IDs.4.3. */
	static constexpr const char *kInstrSpchIDs43Key = "kInstrSpchIDs.4.3";
	/** World-state key for talked To Rock Head. */
	static constexpr const char *kTalkedToRockHeadKey = "talkedToRockHead";
	/** World-state key for ladder. */
	static constexpr const char *kLadderKey = "Ladder";
	/** World-state key for r1 IG1 Ladder Here. */
	static constexpr const char *kR1IG1LadderHereKey = "R1IG1LadderHere";
	/** Authored data key for Instr Spch IDs.5.1. */
	static constexpr const char *kInstrSpchIDs51Key = "kInstrSpchIDs.5.1";
	/** Authored data key for Instr Spch IDs.5.2. */
	static constexpr const char *kInstrSpchIDs52Key = "kInstrSpchIDs.5.2";
	/** World-state key for wagon. */
	static constexpr const char *kWagonKey = "Wagon";
	/** World-state key for r1 CN4 Wagon Here. */
	static constexpr const char *kR1CN4WagonHereKey = "R1CN4WagonHere";
	/** Authored data key for Instr Spch IDs.7.3. */
	static constexpr const char *kInstrSpchIDs73Key = "kInstrSpchIDs.7.3";
	/** Authored data key for Instr Spch IDs.7.4. */
	static constexpr const char *kInstrSpchIDs74Key = "kInstrSpchIDs.7.4";
	/** Authored data key for Instr Spch IDs.%u.3. */
	static constexpr const char *kInstrSpchIDsUnsignedIndex3Key = "kInstrSpchIDs.%u.3";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Authored data key for Help Spch IDs.%u. */
	static constexpr const char *kHelpSpchIDsUnsignedIndexKey = "kHelpSpchIDs.%u";
	/** Authored data key for Main Spch IDs.%u. */
	static constexpr const char *kMainSpchIDsUnsignedIndexKey = "kMainSpchIDs.%u";
	/** Authored data key for Main Spch IDs.%u.1. */
	static constexpr const char *kMainSpchIDsUnsignedIndex1Key = "kMainSpchIDs.%u.1";
	/** Authored data key for Main Spch IDs.%u.2. */
	static constexpr const char *kMainSpchIDsUnsignedIndex2Key = "kMainSpchIDs.%u.2";
	/** World-state key for item:%u. */
	static constexpr const char *kItemUnsignedIndexKey = "item:%u";
	/** Authored data key for Ambient ID. */
	static constexpr const char *kAmbientIDKey = "kAmbientID";
	/** Authored data key for Enter Spch IDs.1. */
	static constexpr const char *kEnterSpchIDs1Key = "kEnterSpchIDs.1";
	/** Authored data key for Enter Spch IDs.2. */
	static constexpr const char *kEnterSpchIDs2Key = "kEnterSpchIDs.2";
	/** Authored data key for Enter Spch IDs.3. */
	static constexpr const char *kEnterSpchIDs3Key = "kEnterSpchIDs.3";
	/** Authored data key for Re Enter Spch IDs A.1. */
	static constexpr const char *kReEnterSpchIDsA1Key = "kReEnterSpchIDsA.1";
	/** Authored data key for Re Enter Spch IDs A.2. */
	static constexpr const char *kReEnterSpchIDsA2Key = "kReEnterSpchIDsA.2";
	/** Authored data key for Re Enter Spch IDs.1.1. */
	static constexpr const char *kReEnterSpchIDs11Key = "kReEnterSpchIDs.1.1";
	/** Authored data key for Re Enter Spch IDs.1.2. */
	static constexpr const char *kReEnterSpchIDs12Key = "kReEnterSpchIDs.1.2";
	/** Authored data key for Re Enter Spch IDs.1.3. */
	static constexpr const char *kReEnterSpchIDs13Key = "kReEnterSpchIDs.1.3";
	/** Authored data key for Re Enter Spch IDs.2. */
	static constexpr const char *kReEnterSpchIDs2Key = "kReEnterSpchIDs.2";
	/** Authored data key for Re Enter Spch IDs.4. */
	static constexpr const char *kReEnterSpchIDs4Key = "kReEnterSpchIDs.4";
	/** Authored data key for Ambient Volume. */
	static constexpr const char *kAmbientVolumeKey = "kAmbientVolume";
	/** Authored data key for Paige Help ID.1. */
	static constexpr const char *kPaigeHelpID1Key = "kPaigeHelpID.1";
	/** Authored data key for Max Paige Index. */
	static constexpr const char *kMaxPaigeIndexKey = "kMaxPaigeIndex";
	/** Authored data key for Paige Help ID.2.1. */
	static constexpr const char *kPaigeHelpID21Key = "kPaigeHelpID.2.1";
	/** Authored data key for Paige Help ID. */
	static constexpr const char *kPaigeHelpIDKey = "kPaigeHelpID";
	/** Authored data key for Max Main Char Index. */
	static constexpr const char *kMaxMainCharIndexKey = "kMaxMainCharIndex";
	/** Authored data key for Main Spch ID.%u. */
	static constexpr const char *kMainSpchIDUnsignedIndexKey = "kMainSpchID.%u";
	/** Authored data key for Main Char ID.1. */
	static constexpr const char *kMainCharID1Key = "kMainCharID.1";
	/** Authored data key for Main Char ID.2. */
	static constexpr const char *kMainCharID2Key = "kMainCharID.2";
	/** Script identifier for irvin. */
	static constexpr const char *kIrvinName = "irvin";
	/** Format for script names or values: %s.%u. */
	static constexpr const char *kNameUnsignedIndexFormat = "%s.%u";
	/** Authored data key for Irvin. */
	static constexpr const char *kIrvinKey = "kIrvin";
	/** Authored data key for Rubber Bands. */
	static constexpr const char *kRubberBandsKey = "kRubberBands";
	/** Prefix of authored page constants. */
	static constexpr const char *kConstantPrefix = "k";
	/** Script identifier for iD. */
	static constexpr const char *kIDName = "ID";
	/** Script identifier for z. */
	static constexpr const char *kZName = "Z";
	/** World-state key for r1 LN2 State. */
	static constexpr const char *kR1LN2StateKey = "R1LN2State";
	/** World-state key for r1 LN2 Reentry. */
	static constexpr const char *kR1LN2ReentryKey = "R1LN2Reentry";
	/** World-state key for r1 LN2 Catapult Reentry. */
	static constexpr const char *kR1LN2CatapultReentryKey = "R1LN2CatapultReentry";
	/** World-state key for r1 LN2 Intro Pending. */
	static constexpr const char *kR1LN2IntroPendingKey = "R1LN2IntroPending";
	/** World-state key for r1 LN2 Movie Pending. */
	static constexpr const char *kR1LN2MoviePendingKey = "R1LN2MoviePending";
	/** World-state key for r1 LN2 Delivery Pending. */
	static constexpr const char *kR1LN2DeliveryPendingKey = "R1LN2DeliveryPending";
	/** Authored data key for Re Enter Spch IDs.10. */
	static constexpr const char *kReEnterSpchIDs10Key = "kReEnterSpchIDs.10";
	/** Script identifier for iri. */
	static constexpr const char *kIriName = "iri";
	/** Authored data key for Re Enter Spch IDs.1. */
	static constexpr const char *kReEnterSpchIDs1Key = "kReEnterSpchIDs.1";
	/** Script identifier for rri. */
	static constexpr const char *kRriName = "rri";
	/** Script identifier for si. */
	static constexpr const char *kSiName = "si";
	/** Authored data key for Re Enter Spch IDs.3. */
	static constexpr const char *kReEnterSpchIDs3Key = "kReEnterSpchIDs.3";
	/** Script identifier for siri. */
	static constexpr const char *kSiriName = "siri";
	/** Authored data key for Re Enter Spch IDs.7. */
	static constexpr const char *kReEnterSpchIDs7Key = "kReEnterSpchIDs.7";
	/** Authored data key for Re Enter Spch IDs.8. */
	static constexpr const char *kReEnterSpchIDs8Key = "kReEnterSpchIDs.8";
	/** Script identifier for sii. */
	static constexpr const char *kSiiName = "sii";
	/** Script identifier for ir. */
	static constexpr const char *kIrName = "ir";
	/** Authored data key for Re Enter Spch IDs.%u. */
	static constexpr const char *kReEnterSpchIDsUnsignedIndexKey = "kReEnterSpchIDs.%u";
	/** Script identifier for iir. */
	static constexpr const char *kIirName = "iir";
	/** Script identifier for ri. */
	static constexpr const char *kRiName = "ri";
	/** Authored data key for Initial Instr Spch IDs.1. */
	static constexpr const char *kInitialInstrSpchIDs1Key = "kInitialInstrSpchIDs.1";
	/** Script identifier for iirsiriisirirsr. */
	static constexpr const char *kIirsiriisirirsrName = "iirsiriisirirsr";
	/** Authored data key for Initial Instr Spch IDs.2. */
	static constexpr const char *kInitialInstrSpchIDs2Key = "kInitialInstrSpchIDs.2";
	/** Script identifier for iiisi. */
	static constexpr const char *kIiisiName = "iiisi";
	/** Authored data key for Initial Instr Spch IDs.2.7. */
	static constexpr const char *kInitialInstrSpchIDs27Key = "kInitialInstrSpchIDs.2.7";
	/** Authored data key for Initial Instr Spch IDs.2.6. */
	static constexpr const char *kInitialInstrSpchIDs26Key = "kInitialInstrSpchIDs.2.6";
	/** Authored data key for Initial Instr Spch IDs.2.8. */
	static constexpr const char *kInitialInstrSpchIDs28Key = "kInitialInstrSpchIDs.2.8";
	/** Authored data key for Instr Spch IDs.5. */
	static constexpr const char *kInstrSpchIDs5Key = "kInstrSpchIDs.5";
	/** Script identifier for ii. */
	static constexpr const char *kIiName = "ii";
	/** Script identifier for ris. */
	static constexpr const char *kRisName = "ris";
	/** Authored data key for Instr Spch IDs.%u. */
	static constexpr const char *kInstrSpchIDsUnsignedIndexKey = "kInstrSpchIDs.%u";
	/** Format for script names or values: collectibles.%u.. */
	static constexpr const char *kCollectiblesUnsignedIndexFormat = "collectibles.%u.";
	/** Inventory record field containing the animation resource ID. */
	static constexpr const char *kInventoryImageField = "1";
	/** Inventory record field containing the horizontal position. */
	static constexpr const char *kInventoryXField = "2";
	/** Inventory record field containing the vertical position. */
	static constexpr const char *kInventoryYField = "3";
	/** Authored data key for Chester YOffset. */
	static constexpr const char *kChesterYOffsetKey = "kChesterYOffset";
	/** Authored data key for Initial Instr Spch IDs.3.%u. */
	static constexpr const char *kInitialInstrSpchIDs3UnsignedIndexKey = "kInitialInstrSpchIDs.3.%u";
	/** World-state key for show Inventory Outlines. */
	static constexpr const char *kShowInventoryOutlinesKey = "ShowInventoryOutlines";
	/** Authored data key for Initial Instr Spch IDs.3.6. */
	static constexpr const char *kInitialInstrSpchIDs36Key = "kInitialInstrSpchIDs.3.6";
	/** Script identifier for bands. */
	static constexpr const char *kBandsName = "Bands";
	/** Script identifier for hoop. */
	static constexpr const char *kHoopName = "Hoop";
	/** Script identifier for char ID. */
	static constexpr const char *kCharIDName = "CharID";
	/** Script identifier for spch ID. */
	static constexpr const char *kSpchIDName = "SpchID";
	/** Authored data key for Num. */
	static constexpr const char *kNumKey = "kNum";
	/** Script identifier for spch. */
	static constexpr const char *kSpchName = "Spch";
	/** Authored data key for Items Char IDs.2. */
	static constexpr const char *kItemsCharIDs2Key = "kItemsCharIDs.2";
	/** Authored data key for Items Spch IDs.2. */
	static constexpr const char *kItemsSpchIDs2Key = "kItemsSpchIDs.2";
	/** Authored data key for Num Item Spch.2. */
	static constexpr const char *kNumItemSpch2Key = "kNumItemSpch.2";
	/** Authored data key for Items Char IDs.1.1. */
	static constexpr const char *kItemsCharIDs11Key = "kItemsCharIDs.1.1";
	/** Authored data key for Items Spch IDs.1.1. */
	static constexpr const char *kItemsSpchIDs11Key = "kItemsSpchIDs.1.1";
	/** Authored data key for Items Char IDs.1.5. */
	static constexpr const char *kItemsCharIDs15Key = "kItemsCharIDs.1.5";
	/** Authored data key for Items Spch IDs.1.5. */
	static constexpr const char *kItemsSpchIDs15Key = "kItemsSpchIDs.1.5";
	/** Format for script names or values: .1.%u. */
	static constexpr const char *k1UnsignedIndexFormat = ".1.%u";
	/** Authored data key for Items Char IDs. */
	static constexpr const char *kItemsCharIDsKey = "kItemsCharIDs";
	/** Authored data key for Items Spch IDs. */
	static constexpr const char *kItemsSpchIDsKey = "kItemsSpchIDs";
	/** Authored data key for No Use Char ID. */
	static constexpr const char *kNoUseCharIDKey = "kNoUseCharID";
	/** Authored data key for No Use Spch ID. */
	static constexpr const char *kNoUseSpchIDKey = "kNoUseSpchID";
	/** Authored data key for Idle Spch IDs.3. */
	static constexpr const char *kIdleSpchIDs3Key = "kIdleSpchIDs.3";
	/** Authored data key for Idle Spch IDs.1. */
	static constexpr const char *kIdleSpchIDs1Key = "kIdleSpchIDs.1";
	/** Script identifier for rs. */
	static constexpr const char *kRsName = "rs";
	/** Authored data key for Idle Spch IDs.2. */
	static constexpr const char *kIdleSpchIDs2Key = "kIdleSpchIDs.2";
	/** Script identifier for srs. */
	static constexpr const char *kSrsName = "srs";
	/** Authored data key for Help Spch IDs.2. */
	static constexpr const char *kHelpSpchIDs2Key = "kHelpSpchIDs.2";
	/** Authored data key for Help Spch IDs.1. */
	static constexpr const char *kHelpSpchIDs1Key = "kHelpSpchIDs.1";
	/** Script identifier for sr. */
	static constexpr const char *kSrName = "sr";
	/** Script identifier for set ID. */
	static constexpr const char *kSetIDName = "SetID";
	/** Script identifier for lips ID. */
	static constexpr const char *kLipsIDName = "LipsID";
	/** Script property name for register Fidgets To Settle. */
	static constexpr const char *kPropertyRegisterFidgetsToSettle = "registerFidgetsToSettle";
	/** Script property name for register Speech To Settle. */
	static constexpr const char *kPropertyRegisterSpeechToSettle = "registerSpeechToSettle";
	/** Script property name for load Speech Anims In Bkgd. */
	static constexpr const char *kPropertyLoadSpeechAnimsInBkgd = "loadSpeechAnimsInBkgd";
	/** Script identifier for body ID. */
	static constexpr const char *kBodyIDName = "BodyID";
	/** Script method name for set Speech Anims. */
	static constexpr const char *kMethodSetSpeechAnims = "setSpeechAnims";
	/** Script method name for add Fidget Group. */
	static constexpr const char *kMethodAddFidgetGroup = "addFidgetGroup";
	/** Authored data key for Speak. */
	static constexpr const char *kSpeakKey = "kSpeak";
	/** Script identifier for fidg1 ID. */
	static constexpr const char *kFidg1IDName = "Fidg1ID";
	/** Script method name for add Fidget. */
	static constexpr const char *kMethodAddFidget = "addFidget";
	/** Format for script names or values: %sFidg%uID. */
	static constexpr const char *kNameFidgUnsignedIndexIDFormat = "%sFidg%uID";
	/** Authored data key for Max Speak Spchs. */
	static constexpr const char *kMaxSpeakSpchsKey = "kMaxSpeakSpchs";
	/** Format for script names or values: .%u. */
	static constexpr const char *kUnsignedIndexFormat = ".%u";
	/** Authored data key for Max Speak Idle Spchs. */
	static constexpr const char *kMaxSpeakIdleSpchsKey = "kMaxSpeakIdleSpchs";
	/** Script identifier for speak. */
	static constexpr const char *kSpeakName = "speak";
	/** Authored data key for Speak Idle Spchs ID. */
	static constexpr const char *kSpeakIdleSpchsIDKey = "kSpeakIdleSpchsID";
	/** Authored data key for Max Idle Index. */
	static constexpr const char *kMaxIdleIndexKey = "kMaxIdleIndex";
	/** Authored data key for Max Idle Index Speeches. */
	static constexpr const char *kMaxIdleIndexSpeechesKey = "kMaxIdleIndexSpeeches";
	/** Authored data key for Idle Char ID. */
	static constexpr const char *kIdleCharIDKey = "kIdleCharID";
	/** Authored data key for Idle Spch ID. */
	static constexpr const char *kIdleSpchIDKey = "kIdleSpchID";
	/** World-state key for r1 CN1 Bell Click. */
	static constexpr const char *kR1CN1BellClickKey = "R1CN1BellClick";
	/** World-state key for r1 CN1 Bell Pending. */
	static constexpr const char *kR1CN1BellPendingKey = "R1CN1BellPending";
	/** World-state key for item:4. */
	static constexpr const char *kItem4Key = "item:4";
	/** Authored data key for Reader. */
	static constexpr const char *kReaderKey = "kReader";
	/** Authored data key for Sam. */
	static constexpr const char *kSamKey = "kSam";
	/** Authored data key for Paige. */
	static constexpr const char *kPaigeKey = "kPaige";
	/** Script identifier for g Chester Fidg ID. */
	static constexpr const char *kGChesterFidgIDName = "gChesterFidgID";
	/** Authored data key for Chester Z. */
	static constexpr const char *kChesterZKey = "kChesterZ";
	/** Script identifier for g Closed Chester Z. */
	static constexpr const char *kGClosedChesterZName = "gClosedChesterZ";
	/** Authored data key for Enter Char ID. */
	static constexpr const char *kEnterCharIDKey = "kEnterCharID";
	/** Authored data key for Enter Spch ID. */
	static constexpr const char *kEnterSpchIDKey = "kEnterSpchID";
	/** World-state key for r1 CG1 State. */
	static constexpr const char *kR1CG1StateKey = "R1CG1State";
	/** Authored data key for Open Gates ID. */
	static constexpr const char *kOpenGatesIDKey = "kOpenGatesID";
	/** Authored data key for Fence Z. */
	static constexpr const char *kFenceZKey = "kFenceZ";
	/** Authored data key for Used Bell ID. */
	static constexpr const char *kUsedBellIDKey = "kUsedBellID";
	/** Authored data key for Bell Z. */
	static constexpr const char *kBellZKey = "kBellZ";
	/** World-state key for r1 CG1 Opening. */
	static constexpr const char *kR1CG1OpeningKey = "R1CG1Opening";
	/** Authored data key for Max Entry Spchs. */
	static constexpr const char *kMaxEntrySpchsKey = "kMaxEntrySpchs";
	/** Format for script names or values: .%d. */
	static constexpr const char *kSignedIndexFormat = ".%d";
	/** Authored data key for Re Enter Char IDs. */
	static constexpr const char *kReEnterCharIDsKey = "kReEnterCharIDs";
	/** Authored data key for Re Enter Spch IDs. */
	static constexpr const char *kReEnterSpchIDsKey = "kReEnterSpchIDs";
	/** Authored data key for Re Enter Max Spchs. */
	static constexpr const char *kReEnterMaxSpchsKey = "kReEnterMaxSpchs";
	/** Authored data key for Bell ID. */
	static constexpr const char *kBellIDKey = "kBellID";
	/** Authored data key for Cloud Mask ID. */
	static constexpr const char *kCloudMaskIDKey = "kCloudMaskID";
	/** Authored data key for Cloud Mask Z. */
	static constexpr const char *kCloudMaskZKey = "kCloudMaskZ";
	/** World-state key for r1 CN1 State. */
	static constexpr const char *kR1CN1StateKey = "R1CN1State";
	/** Authored data key for Re Enter Spch IDs.2.1. */
	static constexpr const char *kReEnterSpchIDs21Key = "kReEnterSpchIDs.2.1";
	/** Authored data key for Ring Bell ID. */
	static constexpr const char *kRingBellIDKey = "kRingBellID";
	/** Authored data key for Gate Open Char ID. */
	static constexpr const char *kGateOpenCharIDKey = "kGateOpenCharID";
	/** Authored data key for Gate Open Spch ID. */
	static constexpr const char *kGateOpenSpchIDKey = "kGateOpenSpchID";
	/** Authored data key for Cloud SFXID. */
	static constexpr const char *kCloudSFXIDKey = "kCloudSFXID";
	/** Authored data key for Target Move Frames. */
	static constexpr const char *kTargetMoveFramesKey = "kTargetMoveFrames";
	/** Authored data key for Target Move FPS. */
	static constexpr const char *kTargetMoveFPSKey = "kTargetMoveFPS";
	/** Authored data key for Target Move Offset X. */
	static constexpr const char *kTargetMoveOffsetXKey = "kTargetMoveOffsetX";
	/** Authored data key for Target Move Offset Y. */
	static constexpr const char *kTargetMoveOffsetYKey = "kTargetMoveOffsetY";
	/** Script identifier for map. */
	static constexpr const char *kMapName = "map";
	/** Authored data key for Outline Spch Char.%u. */
	static constexpr const char *kOutlineSpchCharUnsignedIndexKey = "kOutlineSpchChar.%u";
	/** Authored data key for Outline Spch ID.%u. */
	static constexpr const char *kOutlineSpchIDUnsignedIndexKey = "kOutlineSpchID.%u";
	/** Authored data key for Bell HSX1. */
	static constexpr const char *kBellHSX1Key = "kBellHSX1";
	/** Authored data key for Bell HSY1. */
	static constexpr const char *kBellHSY1Key = "kBellHSY1";
	/** Authored data key for Bell HSX2. */
	static constexpr const char *kBellHSX2Key = "kBellHSX2";
	/** Authored data key for Bell HSY2. */
	static constexpr const char *kBellHSY2Key = "kBellHSY2";
	/** Authored data key for Bell Spch Char IDs. */
	static constexpr const char *kBellSpchCharIDsKey = "kBellSpchCharIDs";
	/** Authored data key for Bell Spch IDs. */
	static constexpr const char *kBellSpchIDsKey = "kBellSpchIDs";
	/** Authored data key for Max Bell Spchs. */
	static constexpr const char *kMaxBellSpchsKey = "kMaxBellSpchs";
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
	/** Authored data key for Dont Exit Spch Char IDs. */
	static constexpr const char *kDontExitSpchCharIDsKey = "kDontExitSpchCharIDs";
	/** Authored data key for Dont Exit Spch IDs. */
	static constexpr const char *kDontExitSpchIDsKey = "kDontExitSpchIDs";
	/** Authored data key for Bell Char ID. */
	static constexpr const char *kBellCharIDKey = "kBellCharID";
	/** Authored data key for Bell Spch ID. */
	static constexpr const char *kBellSpchIDKey = "kBellSpchID";
	/** World-state key for r1 CG1 Paige Clicks. */
	static constexpr const char *kR1CG1PaigeClicksKey = "R1CG1PaigeClicks";
	/** World-state key for r1 CG1 Main Clicks. */
	static constexpr const char *kR1CG1MainClicksKey = "R1CG1MainClicks";
	/** Authored data key for Max Help Spchs. */
	static constexpr const char *kMaxHelpSpchsKey = "kMaxHelpSpchs";
	/** Authored data key for Sign HSX1. */
	static constexpr const char *kSignHSX1Key = "kSignHSX1";
	/** Authored data key for Sign HSY1. */
	static constexpr const char *kSignHSY1Key = "kSignHSY1";
	/** Authored data key for Sign HSX2. */
	static constexpr const char *kSignHSX2Key = "kSignHSX2";
	/** Authored data key for Sign HSY2. */
	static constexpr const char *kSignHSY2Key = "kSignHSY2";
	/** Authored data key for Big Sign Char IDs.1. */
	static constexpr const char *kBigSignCharIDs1Key = "kBigSignCharIDs.1";
	/** Authored data key for Big Sign IDs.1. */
	static constexpr const char *kBigSignIDs1Key = "kBigSignIDs.1";
	/** World-state key for r1 CG1 Big Sign Clicks. */
	static constexpr const char *kR1CG1BigSignClicksKey = "R1CG1BigSignClicks";
	/** Authored data key for Big Sign Char IDs.2. */
	static constexpr const char *kBigSignCharIDs2Key = "kBigSignCharIDs.2";
	/** Authored data key for Big Sign IDs.2. */
	static constexpr const char *kBigSignIDs2Key = "kBigSignIDs.2";
	/** Authored data key for Sm Sign Char IDs.1. */
	static constexpr const char *kSmSignCharIDs1Key = "kSmSignCharIDs.1";
	/** Authored data key for Sm Sign IDs.1. */
	static constexpr const char *kSmSignIDs1Key = "kSmSignIDs.1";
	/** Authored data key for Max Sm Sign Spchs.1. */
	static constexpr const char *kMaxSmSignSpchs1Key = "kMaxSmSignSpchs.1";
	/** Authored data key for Sm Sign Char IDs. */
	static constexpr const char *kSmSignCharIDsKey = "kSmSignCharIDs";
	/** Authored data key for Sm Sign IDs. */
	static constexpr const char *kSmSignIDsKey = "kSmSignIDs";
	/** Authored data key for Max Sm Sign Spchs. */
	static constexpr const char *kMaxSmSignSpchsKey = "kMaxSmSignSpchs";
	/** Script identifier for g Chester Open ID. */
	static constexpr const char *kGChesterOpenIDName = "gChesterOpenID";
	/** Format for script names or values: drawNumbers.%u. */
	static constexpr const char *kDrawNumbersUnsignedIndexFormat = "drawNumbers.%u";
	/** Script property name for color. */
	static constexpr const char *kPropertyColor = "color";
	/** Script identifier for 4. */
	static constexpr const char *k4Name = "4";
	/** Authored data key for Nav Hotspot Z. */
	static constexpr const char *kNavHotspotZKey = "kNavHotspotZ";

	/** Map navigation button shared by page scenes. */
	static constexpr const char *kMapButton = "r1map/1852.ao";
	/** Font for inventory item labels. */
	static constexpr const char *kChestFont = "common/1018.nfnt";
	/** Closed inventory chest lid animation. */
	static constexpr const char *kChestClosedLid = "common/7001.ao";
	/** Inventory chest button animation. */
	static constexpr const char *kChestButton = "common/7002.ao";
	/** Open inventory chest lid animation. */
	static constexpr const char *kChestOpenLid = "common/7003.ao";
	/** Inventory chest tray animation. */
	static constexpr const char *kChestTray = "common/7004.ao";
	/** Shared animation path indexed by navigation cursor resource ID. */
	static constexpr const char *kNavigationCursorFormat = "common/%d.ao";
	/** Inventory animation path indexed by resource ID. */
	static constexpr const char *kInventoryAnimationFormat = "r1inventory/%d.ao";
	/** Character resource filename indexed by ID and extension. */
	static constexpr const char *kCharacterResourceFormat = "%u.%s";
	/** Extension of character animation resources. */
	static constexpr const char *kAnimationExtension = "ao";
	/** Extension of character speech resources. */
	static constexpr const char *kSoundExtension = "wav";
	/** Extension of character lip animation resources. */
	static constexpr const char *kLipsExtension = "lips";
	/** Extension of character body animation resources. */
	static constexpr const char *kBodyExtension = "tc";
	/** Suffix of page speech libraries. */
	static constexpr const char *kSpeechLibrarySuffix = "spch";
	/** Suffix of page sound-effect libraries. */
	static constexpr const char *kEffectsLibrarySuffix = "sfx";
	/** Suffix of page music libraries. */
	static constexpr const char *kMusicLibrarySuffix = "tunz";
	/** Suffix of page word-speech libraries. */
	static constexpr const char *kWordLibrarySuffix = "words";
	/** Suffix of the first gumball speech library. */
	static constexpr const char *kFirstSpeechLibrarySuffix = "spch1";
	/** Suffix of the second gumball speech library. */
	static constexpr const char *kSecondSpeechLibrarySuffix = "spch2";
	/** Shared resource-library alias used by character lookup. */
	static constexpr const char *kCommonLibrary = "common";
	/** Page music path indexed by page name and resource ID. */
	static constexpr const char *kPageMusicFormat = "%stunz/%u.wav";

	friend class RainGame;
	/** Rain-catching activity initialized on its matching page. */
	RainGame *_rainGame = nullptr;
	friend class SkyRide;
	/** Sky-ride sequence initialized on its matching page. */
	SkyRide *_skyRide = nullptr;
	friend class Catapult;
	/** Catapult-ride sequence initialized on its matching page. */
	Catapult *_catapult = nullptr;
	friend class SheepCrossing;
	/** Sheep-crossing interaction initialized on its matching page. */
	SheepCrossing *_sheepCrossing = nullptr;
	friend class RockHead;
	/** Rock Head interaction initialized on its matching page. */
	RockHead *_rockHead = nullptr;
	friend class RainbowSlide;
	/** Rainbow-slide interaction initialized on its matching page. */
	RainbowSlide *_rainbowSlide = nullptr;
	friend class MusicPatterns;
	/** Musical-pattern activity initialized on its matching page. */
	MusicPatterns *_musicPatterns = nullptr;
	friend class Flipbook;
	/** Artwork editor initialized on its matching page. */
	Flipbook *_flipbook = nullptr;
	friend class Gallery;
	/** Art-gallery presentation initialized on its matching page. */
	Gallery *_gallery = nullptr;
	friend class CloudFloors;
	/** Word-floor construction activity initialized on its matching page. */
	CloudFloors *_cloudFloors = nullptr;
	friend class Gumball;
	friend class Sandwich;
	friend class WordPots;
	/** Word-part transformation activity initialized on its matching page. */
	WordPots *_wordPots = nullptr;
	friend class NumberTree;
	/** Number-tree balancing activity initialized on its matching page. */
	NumberTree *_numberTree = nullptr;
	/** Gumball sentence-completion activity initialized on its matching page. */
	Gumball *_gumball = nullptr;
	/** Sandwich-shop payment activity initialized on its matching page. */
	Sandwich *_sandwich = nullptr;
	struct Speech {
		/** Character name for a spoken line, or an empty string for a delay or sound. */
		Common::String actor;
		/** Speech or sound resource ID; zero represents a delay. */
		uint resource;
		/** Whether the following queue entry plays alongside this line. */
		bool together;
		/** Silent interval in milliseconds for a delay entry. */
		uint delay;
	};
	/** Engine services for resources, audio, fonts and random choices. */
	TrinketEngine &_vm;
	/** Scene used to display page characters, backgrounds and activity sprites. */
	Shell::Scene &_scene;
	/** Saved story, inventory, visit and activity progress. */
	World &_world;
	/** Authored constants, text and exits for the current page. */
	const Page *_page = nullptr;
	/** Current page name, or an empty string when no page is active. */
	Common::String _name;
	/** Latest game-pixel pointer position, initially outside the page. */
	Common::Point _mouse = Common::Point(-1, -1);
	/** Decoded navigation cursor images indexed by exit direction. */
	Graphics::Surface _navCursors[8];
	/** Active navigation cursor index, or minus one when no exit cursor is installed. */
	int _navCursor = -1;
	/** Page characters indexed by the names used in authored conversations. */
	Common::HashMap<Common::String, Shell::Character *> _actors;
	/** Queued spoken lines, sounds and silent delays. */
	Common::Array<Speech> _speech;
	/** Sound currently playing as part of the speech queue. */
	Common::ScopedPtr<OM::Sound> _queuedSound;
	/** Independent page sound effects awaiting completion. */
	Common::Array<OM::Sound *> _sounds;
	/** Character currently delivering the primary queued line. */
	Shell::Character *_speaking = nullptr;
	/** Second character speaking alongside the primary character. */
	Shell::Character *_speakingAlso = nullptr;
	/** Collectible bell sprite on the arrival page. */
	Shell::Sprite *_bell = nullptr;
	/** Cloud sprite used during the arrival conversation. */
	Shell::Sprite *_cloud = nullptr;
	/** Gate sprite used during departure from the arrival page. */
	Shell::Sprite *_gate = nullptr;
	/** Current stage of the arrival gate's opening and departure sequence. */
	uint _openingGate = 0;
	/** Millisecond timestamp when the current gate delay began. */
	uint32 _gateStart = 0;
	/** Whether the current gate delay has a start timestamp. */
	bool _gateDelayStarted = false;
	/** Whether completing the speech should reveal the bell. */
	bool _placeBell = false;
	/** Chester character sprite replaced while the inventory chest is displayed. */
	Shell::Sprite *_chester = nullptr;
	/** Inventory chest's animated lid sprite. */
	Shell::Sprite *_chestLid = nullptr;
	/** Inventory chest's item tray sprite. */
	Shell::Sprite *_chestTray = nullptr;
	/** Inventory item sprite currently following the pointer. */
	Shell::Sprite *_dragged = nullptr;
	/** Character animation used while opening the inventory chest. */
	Shell::Sprite *_chestMotion = nullptr;
	/** Inventory chest's close-button sprite. */
	Shell::Sprite *_chestButton = nullptr;
	enum ChestPhase {
		kChestClosed,
		kChestCharacter,
		kChestOpening,
		kChestOpen,
		kChestClosing,
	};
	/** Current inventory opening, interaction or closing stage. */
	ChestPhase _chestPhase = kChestClosed;
	/** Whether closing the chest should show Chester again. */
	bool _restoreChester = true;
	/** Millisecond deadline for the next inventory highlight step. */
	uint32 _highlightDue = 0;
	/** Inventory item sprites, duplicate-item underlays and missing-part outlines. */
	Common::Array<Shell::Sprite *> _items;
	/** Resource-path resolver used to create inventory count labels. */
	Shell::ResourcePaths _chestPaths;
	/** FontMan-owned font used for inventory item quantities. */
	const Graphics::Font *_chestFont = nullptr;
	/** Text labels showing inventory quantities greater than one. */
	Common::Array<Shell::TextObject *> _itemCounts;
	/** Whether an item-collection chest presentation is waiting to begin. */
	bool _putAwayPending = false;
	/** Whether the item-collection chest presentation is in progress. */
	bool _putAwayActive = false;
	/** Pointer displacement from the dragged item's anchor, in pixels. */
	Common::Point _dragOffset;
	/** Game-pixel position to restore when the dragged item is returned. */
	Common::Point _dragHome;
	/** Current game-pixel displacement of the inventory chest from its default position. */
	Common::Point _chestOffset;
	/** Previous game-pixel pointer position while dragging the chest. */
	Common::Point _chestDragPoint;
	/** Whether the inventory chest follows pointer movement. */
	bool _draggingChest = false;
	/** Whether the inventory tray is open for item interaction. */
	bool _chestOpen = false;
	/** Index of the next speech-queue entry to start. */
	uint _nextSpeech = 0;
	/** Millisecond timestamp when the current queued delay began. */
	uint32 _speechDelayStart = 0;
	/** Whether a queued silent delay is currently running. */
	bool _speechDelayStarted = false;
	/** Counter selecting the arrival page's next idle conversation. */
	uint _arrivalIdleIdx = 0;
	/** Whether interrupting or completing the current conversation should hide the cloud. */
	bool _clearCloud = false;
	/** Whether the arrival cloud's timed movement is in progress. */
	bool _cloudMoving = false;
	/** Index selecting a spoken response from the arrival cloud. */
	uint _cloudSpeechIdx = 0;
	/** Millisecond timestamp when the arrival cloud began moving. */
	uint32 _cloudStart = 0;
	/** Millisecond timestamp when page playback was paused. */
	uint32 _pauseStart = 0;
	/** Game-pixel position from which the arrival cloud begins moving. */
	Common::Point _cloudOrigin;
	/** Whether completing speech should add the bell to inventory. */
	bool _collectBell = false;
	/** Whether the page runs as a practice activity without ordinary story progression. */
	bool _practice = false;
	/** Whether page playback and activity timing are paused. */
	bool _paused = false;
	/** Most recent page update timestamp, in milliseconds. */
	uint32 _time = 0;
	enum QueenPhase {
		kQueenIdle,
		kQueenGreeting,
		kQueenWelcome,
		kQueenMovie,
		kQueenAfterMovie,
		kQueenExitSpeech,
		kQueenExitReady,
	};
	/** Current stage of the Queen conversation, movie or departure sequence. */
	QueenPhase _queenPhase = kQueenIdle;
	/** Index of the next line in the Queen's song sequence. */
	uint _queenSongIndex = 0;
	/** Whether the Queen's current song sequence has begun. */
	bool _queenSongStarted = false;
	/** Counter selecting responses to Queen interactions. */
	uint _queenClicks = 0;
	/** Counter selecting Reader and Sam's page responses. */
	uint _mainClicks = 0;
	/** Counter selecting Paige's page guidance. */
	uint _paigeClicks = 0;
	/** Whether the arrival presentation temporarily blocks the Queen's ordinary interaction. */
	bool _queenArriving = false;
	/** Temporary sprite used by a selected Queen ambient animation. */
	Shell::Sprite *_queenAmbient = nullptr;
	/** Selected Queen ambient variant, or zero between variants. */
	uint _queenAmbientChoice = 0;
	/** Sprite used for the page's recurring ambient animation. */
	Shell::Sprite *_ambient = nullptr;
	/** Sound accompanying the page's ambient animation. */
	OM::Sound *_ambientSound = nullptr;
	/** Ambient variants remaining before the random-choice cycle is refilled. */
	Common::Array<uint> _ambientChoices;
	/** Idle conversations remaining before the random-choice cycle is refilled. */
	Common::Array<uint> _idleChoices;
	/** Millisecond timestamp when the current ambient-animation wait began. */
	uint32 _ambientStart = 0;
	/** Millisecond timestamp when the current idle-conversation wait began. */
	uint32 _idleStart = 0;
	/** Whether the page is waiting to start another ambient animation. */
	bool _ambientWaiting = false;
	/** Whether the page is waiting to start another idle conversation. */
	bool _idleWaiting = false;
	/** Whether the current page's ambient timing has been initialized. */
	bool _hubStarted = false;
	/** Initialize the hub's ambient presentation and arrival conversation. */
	bool enterHub();
	/** Advance hub ambient animations and sounds at a millisecond timestamp. */
	void updateHub(uint32 time);
	/** Schedule the hub's next idle conversation at a millisecond timestamp. */
	void hubIdle(uint32 time);
	/** Handle a hub click and return a requested destination, or an empty string. */
	Common::String hubClick(Common::Point point);
	/** Handle a grove click and return a requested destination, or an empty string. */
	Common::String groveClick(Common::Point point);
	/** Choose and remove a random variant, refilling the list with one-based choices when empty. */
	uint chooseAction(Common::Array<uint> &choices, uint count = 3);
	enum InventorPhase {
		kInventorIdle,
		kInventorIntro,
		kInventorChestWait,
		kInventorHoop,
		kInventorBands,
		kInventorPillow,
		kInventorCloseWait,
		kInventorOutro,
		kInventorDeliveryWait,
		kInventorDeliverySpeech,
		kInventorMoreItems,
		kInventorMovie,
	};
	/** Current stage of the inventor's conversation, donation or movie sequence. */
	InventorPhase _inventorPhase = kInventorIdle;
	/** Sprites for the three parts delivered to the inventor. */
	Shell::Sprite *_parts[3] = {};
	/** Highlight used to identify an inventor part. */
	Shell::Sprite *_highlight = nullptr;
	/** Counter selecting Irvin's repeated responses. */
	uint _irvinClicks = 0;
	/** Number of gear-animation repetitions in the current ambient sequence. */
	uint _gearPlays = 0;
	/** Millisecond timestamp when the current inventor delay began. */
	uint32 _inventorStart = 0;
	/** Whether the current inventor delay has a start timestamp. */
	bool _inventorTimerStarted = false;
	/** Initialize the inventor's characters, delivered parts and arrival conversation. */
	bool enterInventor();
	/** Queue Irvin's response for the current delivery progress. */
	void inventorSpeech();
	/** Queue an inventor speech group using the supplied actor sequence. */
	void inventorGroup(const char *prefix, const char *actors);
	/** Queue guidance about an inventory item offered to the inventor. */
	void inventorHint(uint item);
	/** Begin delivery feedback for an inventory item dropped on the inventor. */
	void inventorDrop(uint item);
	/** Queue the request for parts still missing from the invention. */
	void inventorMoreItems();
	/** Advance inventor conversation and delivery stages at a millisecond timestamp. */
	void updateInventor(uint32 time);
	/** Advance inventor and gear ambient animations at a millisecond timestamp. */
	void updateInventorAmbient(uint32 time);
	/** Schedule the inventor's next idle conversation at a millisecond timestamp. */
	void inventorIdle(uint32 time);
	/** Handle an inventor-page click and return a requested destination, or an empty string. */
	Common::String inventorClick(Common::Point point);
	/** Return the authored name for inventor part IDs one through three, or an empty string. */
	static const char *partName(uint item);
	/** Test whether a game-pixel point lies inside the displaced inventory chest. */
	bool chestContains(Common::Point point) const;
	/** Move all displayed chest layers, items and quantity labels by a game-pixel delta. */
	void moveChest(Common::Point delta);
	/** Resolve a character resource by type and part through the page's ordered libraries. */
	Common::String characterResource(uint resource, Kind kind, uint part = 1) const override;
	/** Create and register a named character from authored pose and speech resources; return success. */
	bool actor(const Common::String &name, const Common::String &prefix, bool body = true);
	/** Queue a spoken resource when the named actor exists. */
	void say(const Common::String &actor, uint resource);
	/** Append a silent interval to the speech queue, in milliseconds. */
	void delaySpeech(uint milliseconds);
	/** Append a sound resource to the speech queue. */
	void queueSound(uint resource);
	/** Queue the selected arrival-speaker group, choosing a random group when zero is supplied. */
	void arrivalSpeaker(uint group = 0);
	/** Schedule the arrival page's idle conversation at a millisecond timestamp. */
	void arrivalIdle(uint32 time);
	/** Queue a fixed number of lines using authored actor and resource keys. */
	void speeches(const Common::String &actors, const Common::String &resources, uint count);
	/** Stop queued speech and sound, clear pending lines and complete deferred collection effects. */
	void interrupt();
	/** Apply bell reveal or collection effects deferred until speech completes. */
	void completeSpeech();
	/** Open the inventory tray, optionally playing Chester's opening animation first. */
	void openChest(bool animateCharacter = false);
	/** Begin closing the open inventory tray and optionally restore Chester afterward. */
	void closeChest(bool restoreCharacter = true);
	/** Remove the chest presentation and restore its closed state. */
	void finishChest();
	/** Advance inventory opening, highlight and collection timing at a millisecond timestamp. */
	void updateChest(uint32 time);
	/** Draw quantity labels for the displayed inventory items. */
	void drawInventoryCounts(OM::Canvas &canvas);
	/** Draw the active activity's additional graphics and text. */
	void drawPage(OM::Canvas &canvas);
	/** Request the chest presentation for a newly collected item. */
	void putAway() { _putAwayPending = true; }
	/** Return whether current activity or conversation state blocks ordinary page interaction. */
	bool sceneBusy() const;
	/** Return whether the inventory chest is opening or closing. */
	bool chestBusy() const { return _chestPhase == kChestCharacter || _chestPhase == kChestOpening || _chestPhase == kChestClosing; }
	/** Handle arrival-gate interaction and return a requested destination, or an empty string. */
	Common::String gateClick(Common::Point point);
	/** Initialize the Queen's presentation and arrival or return conversation. */
	bool enterQueen();
	/** Advance the Queen page's ambient animations at a millisecond timestamp. */
	void updateQueenAmbient(uint32 time);
	/** Queue an indexed Queen conversation using the supplied actor sequence. */
	void queenGroup(uint group, const char *actors);
	/** Queue a Queen return conversation selected by progress, visit and catapult arrival. */
	void queenReentry(int state, int reentry, bool fromCatapult);
	/** Queue the Queen's welcome conversation. */
	void queenWelcome();
	/** Advance the Queen's conversation, song and departure stages. */
	void updateQueen();
	/** Schedule the Queen page's next idle conversation at a millisecond timestamp. */
	void queenIdle(uint32 time);
	/** Handle a Queen-page click and return a requested destination, or an empty string. */
	Common::String queenClick(Common::Point point);
	/** Start the selected background music resource, or stop music when zero is supplied. */
	void music(uint resource);
	/** Return whether an item is in inventory or its optional delivery flag is set. */
	bool hasItem(const Common::String &name, const Common::String &delivered = Common::String()) const;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
