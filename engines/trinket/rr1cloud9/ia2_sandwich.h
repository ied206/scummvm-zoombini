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

#ifndef TRINKET_RR1CLOUD9_IA2_SANDWICH_H
#define TRINKET_RR1CLOUD9_IA2_SANDWICH_H

#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** IA2 exact-payment activity and sandwich assembly. */
class Sandwich {
public:
	/** Connect the sandwich shop activity to its page services. */
	explicit Sandwich(PageScene &host);
	/** Release the sandwich shop presentation state. */
	~Sandwich();
	/** Load the sandwich shop resources and initialize the current round. */
	bool enter();
	/** Advance sandwich shop animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Draw the sandwich shop activity's additional graphics and text. */
	void draw(OM::Canvas &canvas);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Handle activity pointer or keyboard input and report whether it was consumed. */
	bool event(const Common::Event &event);
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _phase != kReady || _dragged; }
	/** Interrupt speech when neither bun closure nor dragging prevents it. */
	void skipSpeech();
	/** Resume activity feedback and restore its animation timing. */
	void resume(uint32 elapsed) { _bunStart += elapsed; }

private:
	/** Format for script names or values: %s.%d. */
	static constexpr const char *kNameSignedIndexFormat = "%s.%d";
	/** Script property name for color. */
	static constexpr const char *kPropertyColor = "color";
	/** World-state key for r1 IA2 Level. */
	static constexpr const char *kR1IA2LevelKey = "R1IA2Level";
	/** World-state key for r1 IA2 Items Unlocked. */
	static constexpr const char *kR1IA2ItemsUnlockedKey = "R1IA2ItemsUnlocked";
	/** World-state key for r1 IA2 Show Marble. */
	static constexpr const char *kR1IA2ShowMarbleKey = "R1IA2ShowMarble";
	/** Script identifier for lizard. */
	static constexpr const char *kLizardName = "lizard";
	/** Authored data key for Lizard. */
	static constexpr const char *kLizardKey = "kLizard";
	/** Authored data key for Bell ID. */
	static constexpr const char *kBellIDKey = "kBellID";
	/** Authored data key for Bell Z. */
	static constexpr const char *kBellZKey = "kBellZ";
	/** Authored data key for Cash Register ID. */
	static constexpr const char *kCashRegisterIDKey = "kCashRegisterID";
	/** Authored data key for Cash Register Z. */
	static constexpr const char *kCashRegisterZKey = "kCashRegisterZ";
	/** Authored data key for Indicator ID. */
	static constexpr const char *kIndicatorIDKey = "kIndicatorID";
	/** Authored data key for Indicator Z. */
	static constexpr const char *kIndicatorZKey = "kIndicatorZ";
	/** Authored data key for Glow ID. */
	static constexpr const char *kGlowIDKey = "kGlowID";
	/** Authored data key for Glow Z. */
	static constexpr const char *kGlowZKey = "kGlowZ";
	/** Authored data key for Claw ID. */
	static constexpr const char *kClawIDKey = "kClawID";
	/** Authored data key for Claw Z. */
	static constexpr const char *kClawZKey = "kClawZ";
	/** Authored data key for Top Bun ID. */
	static constexpr const char *kTopBunIDKey = "kTopBunID";
	/** Authored data key for Top Bun Z. */
	static constexpr const char *kTopBunZKey = "kTopBunZ";
	/** Authored data key for Btm Bun ID. */
	static constexpr const char *kBtmBunIDKey = "kBtmBunID";
	/** Authored data key for Btm Bun Z. */
	static constexpr const char *kBtmBunZKey = "kBtmBunZ";
	/** Authored data key for Items ID. */
	static constexpr const char *kItemsIDKey = "kItemsID";
	/** Authored data key for Items Z. */
	static constexpr const char *kItemsZKey = "kItemsZ";
	/** Authored data key for COM1 Marble3 ID. */
	static constexpr const char *kCOM1Marble3IDKey = "kCOM1Marble3ID";
	/** Authored data key for Marble Z. */
	static constexpr const char *kMarbleZKey = "kMarbleZ";
	/** Authored data key for Arrow ID.1. */
	static constexpr const char *kArrowID1Key = "kArrowID.1";
	/** Authored data key for Arrow Z. */
	static constexpr const char *kArrowZKey = "kArrowZ";
	/** Authored data key for Arrow ID.2. */
	static constexpr const char *kArrowID2Key = "kArrowID.2";
	/** Authored data key for Marble X. */
	static constexpr const char *kMarbleXKey = "kMarbleX";
	/** Authored data key for Marble Y. */
	static constexpr const char *kMarbleYKey = "kMarbleY";
	/** Authored data key for Item Text X. */
	static constexpr const char *kItemTextXKey = "kItemTextX";
	/** Authored data key for Item Text Y. */
	static constexpr const char *kItemTextYKey = "kItemTextY";
	/** Authored data key for Item Text W. */
	static constexpr const char *kItemTextWKey = "kItemTextW";
	/** Authored data key for Item Text H. */
	static constexpr const char *kItemTextHKey = "kItemTextH";
	/** Authored data key for Total Text X. */
	static constexpr const char *kTotalTextXKey = "kTotalTextX";
	/** Authored data key for Total Text Y. */
	static constexpr const char *kTotalTextYKey = "kTotalTextY";
	/** Script property name for center. */
	static constexpr const char *kPropertyCenter = "center";
	/** Authored data key for Enter Char ID. */
	static constexpr const char *kEnterCharIDKey = "kEnterCharID";
	/** Authored data key for Enter Spch ID. */
	static constexpr const char *kEnterSpchIDKey = "kEnterSpchID";
	/** Authored data key for Max Entry Spchs. */
	static constexpr const char *kMaxEntrySpchsKey = "kMaxEntrySpchs";
	/** Authored data key for Re Enter Spch ID.%d. */
	static constexpr const char *kReEnterSpchIDSignedIndexKey = "kReEnterSpchID.%d";
	/** Script property name for data Set/Level %d/Word List %d/Set %d. */
	static constexpr const char *kPropertyDataSetLevelSignedIndexWordListSignedIndexSetSignedIndex = "DataSet/Level_%d/WordList_%d/Set_%d";
	/** Format for script names or values: %d%c. */
	static constexpr const char *kSignedIndexCFormat = "%d%c";
	/** Format for script names or values: $%d.%02d. */
	static constexpr const char *kSignedIndexSignedIndexFormat = "$%d.%02d";
	/** Script property name for text. */
	static constexpr const char *kPropertyText = "text";
	/** Authored data key for Max Data Sets. */
	static constexpr const char *kMaxDataSetsKey = "kMaxDataSets";
	/** Authored data key for Num Sets. */
	static constexpr const char *kNumSetsKey = "kNumSets";
	/** Authored data key for Coin ID. */
	static constexpr const char *kCoinIDKey = "kCoinID";
	/** Authored data key for Coin Sfx2 ID. */
	static constexpr const char *kCoinSfx2IDKey = "kCoinSfx2ID";
	/** Authored data key for Here Spch ID. */
	static constexpr const char *kHereSpchIDKey = "kHereSpchID";
	/** Authored data key for Ring Spch ID. */
	static constexpr const char *kRingSpchIDKey = "kRingSpchID";
	/** Authored data key for Counter Coin X. */
	static constexpr const char *kCounterCoinXKey = "kCounterCoinX";
	/** Authored data key for Counter Coin Y. */
	static constexpr const char *kCounterCoinYKey = "kCounterCoinY";
	/** Authored data key for Counter Coin W. */
	static constexpr const char *kCounterCoinWKey = "kCounterCoinW";
	/** Authored data key for Counter Coin H. */
	static constexpr const char *kCounterCoinHKey = "kCounterCoinH";
	/** Authored data key for Coin Delta Y. */
	static constexpr const char *kCoinDeltaYKey = "kCoinDeltaY";
	/** Authored data key for Coin Z. */
	static constexpr const char *kCoinZKey = "kCoinZ";
	/** Authored data key for RRCoin X. */
	static constexpr const char *kRRCoinXKey = "kRRCoinX";
	/** Authored data key for RRCoin Y. */
	static constexpr const char *kRRCoinYKey = "kRRCoinY";
	/** Script property name for x. */
	static constexpr const char *kPropertyX = "x";
	/** Authored data key for Total Text R. */
	static constexpr const char *kTotalTextRKey = "kTotalTextR";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Authored data key for Sam Ring Spch ID. */
	static constexpr const char *kSamRingSpchIDKey = "kSamRingSpchID";
	/** Authored data key for One Spch ID. */
	static constexpr const char *kOneSpchIDKey = "kOneSpchID";
	/** Authored data key for Dollar Spch ID. */
	static constexpr const char *kDollarSpchIDKey = "kDollarSpchID";
	/** Authored data key for Dollars Spch ID. */
	static constexpr const char *kDollarsSpchIDKey = "kDollarsSpchID";
	/** Authored data key for And Spch ID. */
	static constexpr const char *kAndSpchIDKey = "kAndSpchID";
	/** Authored data key for Twenty Spch ID. */
	static constexpr const char *kTwentySpchIDKey = "kTwentySpchID";
	/** Authored data key for Cent Spch ID. */
	static constexpr const char *kCentSpchIDKey = "kCentSpchID";
	/** Authored data key for Cents Spch ID. */
	static constexpr const char *kCentsSpchIDKey = "kCentsSpchID";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Authored data key for COMZero Spch ID. */
	static constexpr const char *kCOMZeroSpchIDKey = "kCOMZeroSpchID";
	/** Authored data key for Lizard Spch ID.1.1. */
	static constexpr const char *kLizardSpchID11Key = "kLizardSpchID.1.1";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Authored data key for Paige Spch ID.2.1. */
	static constexpr const char *kPaigeSpchID21Key = "kPaigeSpchID.2.1";
	/** Authored data key for Congrats Spch ID.%d. */
	static constexpr const char *kCongratsSpchIDSignedIndexKey = "kCongratsSpchID.%d";
	/** Authored data key for Coin Sfx1 ID. */
	static constexpr const char *kCoinSfx1IDKey = "kCoinSfx1ID";
	/** Authored data key for Not Enough Spch ID. */
	static constexpr const char *kNotEnoughSpchIDKey = "kNotEnoughSpchID";
	/** Authored data key for Too Much Spch ID. */
	static constexpr const char *kTooMuchSpchIDKey = "kTooMuchSpchID";
	/** Authored data key for Wrong Spch ID. */
	static constexpr const char *kWrongSpchIDKey = "kWrongSpchID";
	/** Authored data key for Amount Spch ID. */
	static constexpr const char *kAmountSpchIDKey = "kAmountSpchID";
	/** Authored data key for Short Spch ID.1. */
	static constexpr const char *kShortSpchID1Key = "kShortSpchID.1";
	/** Authored data key for More Spch ID.1. */
	static constexpr const char *kMoreSpchID1Key = "kMoreSpchID.1";
	/** Authored data key for Short Spch ID.2. */
	static constexpr const char *kShortSpchID2Key = "kShortSpchID.2";
	/** Authored data key for More Spch ID.2. */
	static constexpr const char *kMoreSpchID2Key = "kMoreSpchID.2";
	/** Authored data key for Add Spch ID. */
	static constexpr const char *kAddSpchIDKey = "kAddSpchID";
	/** Authored data key for Take Off Spch ID. */
	static constexpr const char *kTakeOffSpchIDKey = "kTakeOffSpchID";
	/** Authored data key for Stumped Spch ID.%d.1. */
	static constexpr const char *kStumpedSpchIDSignedIndex1Key = "kStumpedSpchID.%d.1";
	/** Authored data key for Stumped Spch ID.2.2. */
	static constexpr const char *kStumpedSpchID22Key = "kStumpedSpchID.2.2";
	/** Authored data key for Tough Spch ID.1. */
	static constexpr const char *kToughSpchID1Key = "kToughSpchID.1";
	/** Authored data key for Tough Spch ID.2. */
	static constexpr const char *kToughSpchID2Key = "kToughSpchID.2";
	/** World-state key for r1 IA2 Streak Kind. */
	static constexpr const char *kR1IA2StreakKindKey = "R1IA2StreakKind";
	/** World-state key for r1 IA2 Streak. */
	static constexpr const char *kR1IA2StreakKey = "R1IA2Streak";
	/** World-state key for r1 IA2. */
	static constexpr const char *kR1IA2Key = "R1IA2";
	/** World-state key for r1 IA2 Fixed Level. */
	static constexpr const char *kR1IA2FixedLevelKey = "R1IA2FixedLevel";
	/** Authored data key for Unlock Sfx ID. */
	static constexpr const char *kUnlockSfxIDKey = "kUnlockSfxID";
	/** Authored data key for Correct Spch ID.%d. */
	static constexpr const char *kCorrectSpchIDSignedIndexKey = "kCorrectSpchID.%d";
	/** Authored data key for Unlock Spch ID.1. */
	static constexpr const char *kUnlockSpchID1Key = "kUnlockSpchID.1";
	/** Authored data key for Unlock Spch ID.2. */
	static constexpr const char *kUnlockSpchID2Key = "kUnlockSpchID.2";
	/** Authored data key for Lizard Spch ID.3.1. */
	static constexpr const char *kLizardSpchID31Key = "kLizardSpchID.3.1";
	/** Authored data key for Unlock3 Spch ID. */
	static constexpr const char *kUnlock3SpchIDKey = "kUnlock3SpchID";
	/** World-state key for r1 IA2 Dataset. */
	static constexpr const char *kR1IA2DatasetKey = "R1IA2Dataset";
	/** Authored data key for No Top Char ID. */
	static constexpr const char *kNoTopCharIDKey = "kNoTopCharID";
	/** Authored data key for No Top Spch ID. */
	static constexpr const char *kNoTopSpchIDKey = "kNoTopSpchID";
	/** Authored data key for No Top Char ID.%u. */
	static constexpr const char *kNoTopCharIDUnsignedIndexKey = "kNoTopCharID.%u";
	/** Authored data key for Not Done Char ID.1. */
	static constexpr const char *kNotDoneCharID1Key = "kNotDoneCharID.1";
	/** Authored data key for Not Done Spch ID.1. */
	static constexpr const char *kNotDoneSpchID1Key = "kNotDoneSpchID.1";
	/** Authored data key for Not Done Spch ID.%u. */
	static constexpr const char *kNotDoneSpchIDUnsignedIndexKey = "kNotDoneSpchID.%u";
	/** Authored data key for Sandwich Container Y. */
	static constexpr const char *kSandwichContainerYKey = "kSandwichContainerY";
	/** Authored data key for Sandwich Container H. */
	static constexpr const char *kSandwichContainerHKey = "kSandwichContainerH";
	/** Authored data key for Bun Extra Delta Y. */
	static constexpr const char *kBunExtraDeltaYKey = "kBunExtraDeltaY";
	/** Authored data key for Done Sfx ID. */
	static constexpr const char *kDoneSfxIDKey = "kDoneSfxID";
	/** Format for script names or values: item:%d. */
	static constexpr const char *kItemSignedIndexFormat = "item:%d";
	/** Authored data key for INV1 Food. */
	static constexpr const char *kINV1FoodKey = "kINV1Food";
	/** Authored data key for Special Spch ID.1. */
	static constexpr const char *kSpecialSpchID1Key = "kSpecialSpchID.1";
	/** Authored data key for Special Spch ID.2. */
	static constexpr const char *kSpecialSpchID2Key = "kSpecialSpchID.2";
	/** Authored data key for Marble There Spch ID. */
	static constexpr const char *kMarbleThereSpchIDKey = "kMarbleThereSpchID";
	/** Authored data key for New Bun Sfx ID. */
	static constexpr const char *kNewBunSfxIDKey = "kNewBunSfxID";
	/** Authored data key for Top Bun Move Frames. */
	static constexpr const char *kTopBunMoveFramesKey = "kTopBunMoveFrames";
	/** Authored data key for Top Bun Move FPS. */
	static constexpr const char *kTopBunMoveFPSKey = "kTopBunMoveFPS";
	/** Authored data key for Stand Spch ID. */
	static constexpr const char *kStandSpchIDKey = "kStandSpchID";
	/** Format for script names or values: .%d. */
	static constexpr const char *kSignedIndexFormat = ".%d";
	/** Authored data key for Yes LG2 Char ID. */
	static constexpr const char *kYesLG2CharIDKey = "kYesLG2CharID";
	/** Authored data key for No LG2 Char ID. */
	static constexpr const char *kNoLG2CharIDKey = "kNoLG2CharID";
	/** Authored data key for Yes LG2 Spch ID. */
	static constexpr const char *kYesLG2SpchIDKey = "kYesLG2SpchID";
	/** Authored data key for No LG2 Spch ID. */
	static constexpr const char *kNoLG2SpchIDKey = "kNoLG2SpchID";
	/** Authored data key for Paige Spch ID.1.%u. */
	static constexpr const char *kPaigeSpchID1UnsignedIndexKey = "kPaigeSpchID.1.%u";
	/** Authored data key for Paige Spch ID.3.%d. */
	static constexpr const char *kPaigeSpchID3SignedIndexKey = "kPaigeSpchID.3.%d";
	/** Authored data key for Lizard Spch ID.6.1. */
	static constexpr const char *kLizardSpchID61Key = "kLizardSpchID.6.1";
	/** Authored data key for Lizard Spch ID.5.1. */
	static constexpr const char *kLizardSpchID51Key = "kLizardSpchID.5.1";
	/** Authored data key for Lizard Spch ID.5.2. */
	static constexpr const char *kLizardSpchID52Key = "kLizardSpchID.5.2";
	/** Authored data key for Lizard Spch ID.1.2. */
	static constexpr const char *kLizardSpchID12Key = "kLizardSpchID.1.2";
	/** Authored data key for Lizard Spch ID.2.2. */
	static constexpr const char *kLizardSpchID22Key = "kLizardSpchID.2.2";
	/** Authored data key for Lizard Spch ID.2.1. */
	static constexpr const char *kLizardSpchID21Key = "kLizardSpchID.2.1";
	/** Authored data key for Lizard Spch ID.3.2. */
	static constexpr const char *kLizardSpchID32Key = "kLizardSpchID.3.2";
	/** Authored data key for Lizard Spch ID.4.1. */
	static constexpr const char *kLizardSpchID41Key = "kLizardSpchID.4.1";
	/** Authored data key for Coin Value. */
	static constexpr const char *kCoinValueKey = "kCoinValue";
	/** Authored data key for COMPick Up Sfx ID. */
	static constexpr const char *kCOMPickUpSfxIDKey = "kCOMPickUpSfxID";
	/** Authored data key for INV1 Marble3. */
	static constexpr const char *kINV1Marble3Key = "kINV1Marble3";
	/** Authored data key for Remove Spch ID. */
	static constexpr const char *kRemoveSpchIDKey = "kRemoveSpchID";
	/** Authored data key for Arrow Click Sfx ID. */
	static constexpr const char *kArrowClickSfxIDKey = "kArrowClickSfxID";
	/** Authored data key for Total Spch ID. */
	static constexpr const char *kTotalSpchIDKey = "kTotalSpchID";
	/** Authored data key for Items Hotspot X1. */
	static constexpr const char *kItemsHotspotX1Key = "kItemsHotspotX1";
	/** Authored data key for Items Hotspot Y1. */
	static constexpr const char *kItemsHotspotY1Key = "kItemsHotspotY1";
	/** Authored data key for Items Hotspot X2. */
	static constexpr const char *kItemsHotspotX2Key = "kItemsHotspotX2";
	/** Authored data key for Items Hotspot Y2. */
	static constexpr const char *kItemsHotspotY2Key = "kItemsHotspotY2";
	/** Authored data key for Max Item Placed. */
	static constexpr const char *kMaxItemPlacedKey = "kMaxItemPlaced";
	/** Authored data key for Max Item Answers. */
	static constexpr const char *kMaxItemAnswersKey = "kMaxItemAnswers";
	/** Authored data key for Limit Spch ID. */
	static constexpr const char *kLimitSpchIDKey = "kLimitSpchID";
	/** Authored data key for Items Answer Z. */
	static constexpr const char *kItemsAnswerZKey = "kItemsAnswerZ";
	/** Authored data key for Buy Spch ID. */
	static constexpr const char *kBuySpchIDKey = "kBuySpchID";
	/** Authored data key for COMSelect Sfx ID. */
	static constexpr const char *kCOMSelectSfxIDKey = "kCOMSelectSfxID";
	/** Authored data key for Topping Spch ID. */
	static constexpr const char *kToppingSpchIDKey = "kToppingSpchID";
	/** Authored data key for Comment Spch ID.%d. */
	static constexpr const char *kCommentSpchIDSignedIndexKey = "kCommentSpchID.%d";
	/** Authored data key for Cost Spch ID. */
	static constexpr const char *kCostSpchIDKey = "kCostSpchID";
	/** Format for script names or values: .%u. */
	static constexpr const char *kUnsignedIndexFormat = ".%u";
	/** Authored data key for Main Char ID. */
	static constexpr const char *kMainCharIDKey = "kMainCharID";
	/** Authored data key for Main Spch ID. */
	static constexpr const char *kMainSpchIDKey = "kMainSpchID";
	/** Authored data key for Sandwich Container X. */
	static constexpr const char *kSandwichContainerXKey = "kSandwichContainerX";
	/** Authored data key for Sandwich Container W. */
	static constexpr const char *kSandwichContainerWKey = "kSandwichContainerW";
	/** Authored data key for Sandwich Container Delta Y. */
	static constexpr const char *kSandwichContainerDeltaYKey = "kSandwichContainerDeltaY";
	/** Authored data key for COMDrop Sfx ID. */
	static constexpr const char *kCOMDropSfxIDKey = "kCOMDropSfxID";
	/** Authored data key for COMSnap Home Sfx ID. */
	static constexpr const char *kCOMSnapHomeSfxIDKey = "kCOMSnapHomeSfxID";

	/** Font for sandwich orders. */
	static constexpr const char *kFont = "common/1018.nfnt";
	/** Ingredient names indexed by ingredient ID. */
	static constexpr const char *kNamesText = "r1ia2/19200.txt";
	/** Sandwich puzzle datasets. */
	static constexpr const char *kDataXML = "r1ia2.xml";

	enum Phase {
		kReady,
		kSelecting,
		kPaying,
		kClosing,
		kReward,
	};
	struct Coin {
		/** Sprite displaying this draggable coin. */
		Shell::Sprite *sprite = nullptr;
		/** Zero-based denomination index in the authored coin tables. */
		int type = 0;
		/** Whether this coin contributes to the amount on the counter. */
		bool paid = false;
	};
	struct Topping {
		/** Sprite displaying a topping placed on the sandwich. */
		Shell::Sprite *sprite = nullptr;
		/** One-based item number identifying the topping. */
		int item = 0;
		/** Zero-based sandwich column containing this topping. */
		int column = 0;
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Resource-path resolver for the activity's current page. */
	Shell::ResourcePaths _paths;
	/** FontMan-owned font used for activity text and labels. */
	const Graphics::Font *_font = nullptr;
	/** Parsed XML data used to construct the current activity round. */
	OM::XMLParser _data;
	/** Display names indexed by the thirty sandwich items. */
	Common::StringArray _names;
	/** Text displaying the selected topping's name. */
	Shell::TextObject *_itemText = nullptr;
	/** Text displaying the selected topping's price. */
	Shell::TextObject *_costText = nullptr;
	/** Text displaying the amount currently offered on the counter. */
	Shell::TextObject *_totalText = nullptr;
	/** Bell sprite used to submit the payment. */
	Shell::Sprite *_bell = nullptr;
	/** Cash-register sprite used to request the payment total. */
	Shell::Sprite *_register = nullptr;
	/** Sprite indicating whether the selected topping has been unlocked. */
	Shell::Sprite *_indicator = nullptr;
	/** Highlight used during payment feedback. */
	Shell::Sprite *_glow = nullptr;
	/** Claw sprite used to finish the sandwich. */
	Shell::Sprite *_claw = nullptr;
	/** Top bun animated when the sandwich is finished. */
	Shell::Sprite *_top = nullptr;
	/** Bottom bun beneath the placed toppings. */
	Shell::Sprite *_bottom = nullptr;
	/** Preview sprite for the selected topping. */
	Shell::Sprite *_item = nullptr;
	/** Collectible marble displayed after the required progress. */
	Shell::Sprite *_marble = nullptr;
	/** Sprites for browsing backward and forward through toppings. */
	Shell::Sprite *_arrows[2] = {};
	/** Coin or topping sprite currently following the pointer. */
	Shell::Sprite *_dragged = nullptr;
	/** Available coins and their payment-counter state. */
	Common::Array<Coin> _coins;
	/** Toppings placed on the current sandwich. */
	Common::Array<Topping> _toppings;
	/** Pointer displacement from the dragged piece's anchor, in pixels. */
	Common::Point _dragOffset;
	/** Index of the dragged coin, or minus one when dragging a topping. */
	int _dragCoin = -1;
	/** Difficulty selected for the current activity round. */
	int _level = 1;
	/** Authored dataset selected for the current difficulty. */
	int _dataset = 0;
	/** Exercise set selected within the price dataset. */
	int _set = 1;
	/** One-based number of the currently selected topping. */
	int _current = 29;
	/** Required payment for the selected topping, in cents. */
	int _price = 0;
	/** Value of coins on the counter, in cents. */
	int _total = 0;
	/** Incorrect payment attempts for the current topping. */
	int _wrong = 0;
	/** Price dataset selected for each topping; zero selects a new dataset. */
	int _itemDatasets[30] = {};
	/** Number of available coins for each denomination. */
	int _supply[4] = {};
	/** Number of copies of each topping placed on this sandwich. */
	int _placed[30] = {};
	/** Number of toppings stacked in each sandwich column. */
	int _columns[5] = {};
	/** Accumulated topping height in each column, in pixels. */
	int _columnHeights[5] = {};
	/** Bit mask of toppings available for placement after correct payment. */
	uint32 _unlocked = 0;
	/** Millisecond timestamp when the top bun began closing. */
	uint32 _bunStart = 0;
	/** Top bun's original game-pixel y coordinate. */
	int _bunY = 0;
	/** Top bun's final game-pixel y coordinate above the toppings. */
	int _bunTarget = 0;
	/** Whether the current topping has been selected for payment. */
	bool _selected = false;
	/** Whether the offered amount is currently revealed. */
	bool _totalVisible = false;
	/** Whether the collectible marble is available on the page. */
	bool _showMarble = false;
	/** Whether the short-payment hint has already been given. */
	bool _shortHelp = false;
	/** Whether the excess-payment hint has already been given. */
	bool _excessHelp = false;
	/** Whether the first topping-selection instruction has been played. */
	bool _firstTopping = false;
	/** Whether the bell instruction has already been played. */
	bool _ringHelp = false;
	/** Counter cycling the main character's click responses. */
	uint _mainClicks = 0;
	/** Counter selecting feedback for the sandwich-finishing control. */
	uint _clawClicks = 0;
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kReady;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Read an authored numeric constant selected by an indexed name. */
	int indexed(const char *key, int index) const;
	/** Create a sandwich-shop sprite using authored resource and depth keys. */
	Shell::Sprite *sprite(const char *resource, const char *depth);
	/** Create a text object at a game-pixel position with optional dimensions. */
	Shell::TextObject *text(int x, int y, int width = 0, int height = 0);
	/** Play the authored spoken line for the selected actor. */
	void say(const char *actor, const Common::String &key);
	/** Queue the spoken dollar and cent amounts for a price in cents. */
	void sayPrice(int price);
	/** Play an effect identified by an authored sound key. */
	void sound(const char *key);
	/** Load the topping names and report whether the list is valid. */
	bool loadNames();
	/** Read and validate the current price and denomination supplies. */
	bool readData();
	/** Display the selected topping and load its price exercise. */
	bool showItem();
	/** Create the supplied coins and begin payment interaction. */
	bool startPayment();
	/** Remove all coin sprites and reset the offered payment. */
	void clearCoins();
	/** Arrange coins in supply and payment-counter positions. */
	void layoutCoins();
	/** Update the offered amount and optionally reveal it. */
	void refreshTotal(bool reveal);
	/** Check the submitted payment and start the appropriate response. */
	void ringBell();
	/** Explain an incorrect payment using the amount offered. */
	void wrongPrice();
	/** Make the paid topping available and announce progress. */
	void unlock();
	/** Record payment performance and update difficulty when required. */
	void assess();
	/** Check the placed toppings and begin closing the top bun. */
	void finishSandwich();
	/** Award the completed sandwich and reset its topping and payment state. */
	void award();
	/** Restore the buns and controls for a new sandwich. */
	void newSandwich();
	/** Queue context-sensitive guidance from Paige or the shopkeeper. */
	void help(bool paige);
	/** Place the dragged coin or topping at a game-pixel pointer position. */
	void drop(Common::Point point);
	/** Write the activity's current saved state to the world. */
	void persist();
	/** Return the game-pixel payment-counter rectangle for a denomination. */
	Common::Rect counter(int type) const;
	/** Format a price in cents for the shop's price display. */
	static Common::String priceText(int price);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
