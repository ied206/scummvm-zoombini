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

#ifndef TRINKET_RR1CLOUD9_LA1_GUMBALL_H
#define TRINKET_RR1CLOUD9_LA1_GUMBALL_H

#include "trinket/ompp/sound.h"
#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** LA1 word ramps, solved-round assessment and collectible presentation. */
class Gumball {
public:
	/** Connect the gumball machine activity to its page services. */
	explicit Gumball(PageScene &host);
	/** Release the gumball machine presentation state. */
	~Gumball();
	/** Load the gumball machine resources and initialize the current round. */
	bool enter();
	/** Advance gumball machine animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Draw the gumball machine activity's additional graphics and text. */
	void draw(OM::Canvas &canvas);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Update the ramp highlight beneath a game-pixel pointer position. */
	void hover(Common::Point point);
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _phase != kReady; }
	/** Interrupt skippable speech and retain required activity transitions. */
	void skipSpeech();
	/** Pause or unpause activity feedback and sprite playback. */
	void pause(bool paused);
	/** Resume activity feedback and restore its animation timing. */
	void resume(uint32 elapsed) { _deadline += elapsed; }

private:
	/** Actor identifier for the gumball-machine character. */
	static constexpr const char *kGumActor = "gum";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Format for script names or values: %s.%u. */
	static constexpr const char *kNameUnsignedIndexFormat = "%s.%u";
	/** Authored data key for Parts List ID. */
	static constexpr const char *kPartsListIDKey = "kPartsListID";
	/** Authored data key for Words List ID. */
	static constexpr const char *kWordsListIDKey = "kWordsListID";
	/** Authored data key for TLC18 Font. */
	static constexpr const char *kTLC18FontKey = "kTLC18Font";
	/** World-state key for r1 LA1 Level. */
	static constexpr const char *kR1LA1LevelKey = "R1LA1Level";
	/** World-state key for r1 LA1 Dataset. */
	static constexpr const char *kR1LA1DatasetKey = "R1LA1Dataset";
	/** World-state key for r1 LA1 State. */
	static constexpr const char *kR1LA1StateKey = "R1LA1State";
	/** World-state key for r1 LA1 CCount. */
	static constexpr const char *kR1LA1CCountKey = "R1LA1CCount";
	/** World-state key for r1 LA1 TCCount. */
	static constexpr const char *kR1LA1TCCountKey = "R1LA1TCCount";
	/** Authored data key for Gum. */
	static constexpr const char *kGumKey = "kGum";
	/** Script method name for set Speech Anims. */
	static constexpr const char *kMethodSetSpeechAnims = "setSpeechAnims";
	/** Authored data key for Gum Lips ID. */
	static constexpr const char *kGumLipsIDKey = "kGumLipsID";
	/** Authored data key for Gum Set ID. */
	static constexpr const char *kGumSetIDKey = "kGumSetID";
	/** Script method name for add Fidget Group. */
	static constexpr const char *kMethodAddFidgetGroup = "addFidgetGroup";
	/** Authored data key for Fidget Min Time2. */
	static constexpr const char *kFidgetMinTime2Key = "kFidgetMinTime2";
	/** Authored data key for Fidget Max Time2. */
	static constexpr const char *kFidgetMaxTime2Key = "kFidgetMaxTime2";
	/** Script method name for add Fidget. */
	static constexpr const char *kMethodAddFidget = "addFidget";
	/** Authored data key for Gum Fidg1 ID. */
	static constexpr const char *kGumFidg1IDKey = "kGumFidg1ID";
	/** Authored data key for Token Release ID. */
	static constexpr const char *kTokenReleaseIDKey = "kTokenReleaseID";
	/** Authored data key for Token Release Z. */
	static constexpr const char *kTokenReleaseZKey = "kTokenReleaseZ";
	/** Authored data key for Token ID. */
	static constexpr const char *kTokenIDKey = "kTokenID";
	/** Authored data key for Token Z. */
	static constexpr const char *kTokenZKey = "kTokenZ";
	/** Authored data key for Slider In ID. */
	static constexpr const char *kSliderInIDKey = "kSliderInID";
	/** Authored data key for Slider Settle Z. */
	static constexpr const char *kSliderSettleZKey = "kSliderSettleZ";
	/** Authored data key for Slider Z. */
	static constexpr const char *kSliderZKey = "kSliderZ";
	/** Authored data key for Slider Out ID. */
	static constexpr const char *kSliderOutIDKey = "kSliderOutID";
	/** Authored data key for Whistle Sparkle ID. */
	static constexpr const char *kWhistleSparkleIDKey = "kWhistleSparkleID";
	/** Authored data key for Item Z. */
	static constexpr const char *kItemZKey = "kItemZ";
	/** Authored data key for Whistle Roll ID. */
	static constexpr const char *kWhistleRollIDKey = "kWhistleRollID";
	/** Authored data key for Marble ID. */
	static constexpr const char *kMarbleIDKey = "kMarbleID";
	/** Authored data key for Candy Roll ID. */
	static constexpr const char *kCandyRollIDKey = "kCandyRollID";
	/** Authored data key for Blow Whistle ID. */
	static constexpr const char *kBlowWhistleIDKey = "kBlowWhistleID";
	/** Authored data key for Sam Anim Z. */
	static constexpr const char *kSamAnimZKey = "kSamAnimZ";
	/** Authored data key for Eat Candy ID. */
	static constexpr const char *kEatCandyIDKey = "kEatCandyID";
	/** Authored data key for Marble Roll ID. */
	static constexpr const char *kMarbleRollIDKey = "kMarbleRollID";
	/** Authored data key for Token Release X. */
	static constexpr const char *kTokenReleaseXKey = "kTokenReleaseX";
	/** Authored data key for Token Release Closed. */
	static constexpr const char *kTokenReleaseClosedKey = "kTokenReleaseClosed";
	/** Authored data key for Token X. */
	static constexpr const char *kTokenXKey = "kTokenX";
	/** Authored data key for Whistle Settle Frame. */
	static constexpr const char *kWhistleSettleFrameKey = "kWhistleSettleFrame";
	/** Authored data key for Candy Settle Frame. */
	static constexpr const char *kCandySettleFrameKey = "kCandySettleFrame";
	/** Authored data key for Marble X. */
	static constexpr const char *kMarbleXKey = "kMarbleX";
	/** Authored data key for Marble Y. */
	static constexpr const char *kMarbleYKey = "kMarbleY";
	/** World-state key for r1 LA1 Show Marble. */
	static constexpr const char *kR1LA1ShowMarbleKey = "R1LA1ShowMarble";
	/** Authored data key for First Ramp Y. */
	static constexpr const char *kFirstRampYKey = "kFirstRampY";
	/** Format for script names or values: bValidRamp.%u. */
	static constexpr const char *kBValidRampUnsignedIndexFormat = "bValidRamp.%u";
	/** Authored data key for Ramp Rows. */
	static constexpr const char *kRampRowsKey = "kRampRows";
	/** Authored data key for Ramp Row Change.%d. */
	static constexpr const char *kRampRowChangeSignedIndexKey = "kRampRowChange.%d";
	/** Authored data key for Ramp Delta Y. */
	static constexpr const char *kRampDeltaYKey = "kRampDeltaY";
	/** Authored data key for Ramp ID. */
	static constexpr const char *kRampIDKey = "kRampID";
	/** Authored data key for Ramp Z. */
	static constexpr const char *kRampZKey = "kRampZ";
	/** Authored data key for Ramp XValues.%d. */
	static constexpr const char *kRampXValuesSignedIndexKey = "kRampXValues.%d";
	/** Authored data key for Corr Token Z. */
	static constexpr const char *kCorrTokenZKey = "kCorrTokenZ";
	/** Authored data key for Corr Token X.%u. */
	static constexpr const char *kCorrTokenXUnsignedIndexKey = "kCorrTokenX.%u";
	/** Authored data key for Corr Token Y. */
	static constexpr const char *kCorrTokenYKey = "kCorrTokenY";
	/** Authored data key for Instruction Spch IDs.1.4. */
	static constexpr const char *kInstructionSpchIDs14Key = "kInstructionSpchIDs.1.4";
	/** Authored data key for Instruction Spch IDs.1.5. */
	static constexpr const char *kInstructionSpchIDs15Key = "kInstructionSpchIDs.1.5";
	/** Script identifier for rs. */
	static constexpr const char *kRsName = "rs";
	/** Authored data key for Enter Spch IDs. */
	static constexpr const char *kEnterSpchIDsKey = "kEnterSpchIDs";
	/** Script identifier for sgrs. */
	static constexpr const char *kSgrsName = "sgrs";
	/** Authored data key for Re Enter Spch IDs.2. */
	static constexpr const char *kReEnterSpchIDs2Key = "kReEnterSpchIDs.2";
	/** Script identifier for rsgg. */
	static constexpr const char *kRsggName = "rsgg";
	/** Authored data key for Re Enter Spch IDs.3. */
	static constexpr const char *kReEnterSpchIDs3Key = "kReEnterSpchIDs.3";
	/** Script identifier for rg. */
	static constexpr const char *kRgName = "rg";
	/** Authored data key for Re Enter Spch IDs.4.1. */
	static constexpr const char *kReEnterSpchIDs41Key = "kReEnterSpchIDs.4.1";
	/** Script identifier for gg. */
	static constexpr const char *kGgName = "gg";
	/** Authored data key for Re Enter Spch IDs.4.3. */
	static constexpr const char *kReEnterSpchIDs43Key = "kReEnterSpchIDs.4.3";
	/** Script identifier for sgg. */
	static constexpr const char *kSggName = "sgg";
	/** Authored data key for Re Enter Spch IDs.4.4. */
	static constexpr const char *kReEnterSpchIDs44Key = "kReEnterSpchIDs.4.4";
	/** Authored data key for Max Data Sets.%d. */
	static constexpr const char *kMaxDataSetsSignedIndexKey = "kMaxDataSets.%d";
	/** Authored data key for Num Sets. */
	static constexpr const char *kNumSetsKey = "kNumSets";
	/** Authored data key for Right Fixed Ramp.%d.%u. */
	static constexpr const char *kRightFixedRampSignedIndexUnsignedIndexKey = "kRightFixedRamp.%d.%u";
	/** Authored data key for Left Fixed Ramp.%d.%u. */
	static constexpr const char *kLeftFixedRampSignedIndexUnsignedIndexKey = "kLeftFixedRamp.%d.%u";
	/** Authored data key for Fixed Ramp.3.%d.%d. */
	static constexpr const char *kFixedRamp3SignedIndexSignedIndexKey = "kFixedRamp.3.%d.%d";
	/** Authored data key for Ramp Right. */
	static constexpr const char *kRampRightKey = "kRampRight";
	/** Authored data key for Ramp Left. */
	static constexpr const char *kRampLeftKey = "kRampLeft";
	/** Authored data key for Token XValues.%u. */
	static constexpr const char *kTokenXValuesUnsignedIndexKey = "kTokenXValues.%u";
	/** Authored data key for Token YValues.%u. */
	static constexpr const char *kTokenYValuesUnsignedIndexKey = "kTokenYValues.%u";
	/** Script property name for color. */
	static constexpr const char *kPropertyColor = "color";
	/** Script property name for data Set/Level %d/Data Set %d/Set %d. */
	static constexpr const char *kPropertyDataSetLevelSignedIndexDataSetSignedIndexSetSignedIndex = "DataSet/Level_%d/DataSet_%d/Set_%d";
	/** Authored data key for Words X.%u. */
	static constexpr const char *kWordsXUnsignedIndexKey = "kWordsX.%u";
	/** Authored data key for Words Y. */
	static constexpr const char *kWordsYKey = "kWordsY";
	/** Authored data key for Parts XStart. */
	static constexpr const char *kPartsXStartKey = "kPartsXStart";
	/** Authored data key for Parts Y. */
	static constexpr const char *kPartsYKey = "kPartsY";
	/** Authored data key for Init Sentence Spch ID. */
	static constexpr const char *kInitSentenceSpchIDKey = "kInitSentenceSpchID";
	/** Authored data key for Word Blank SFX. */
	static constexpr const char *kWordBlankSFXKey = "kWordBlankSFX";
	/** Authored data key for Idle Spch IDs.%d. */
	static constexpr const char *kIdleSpchIDsSignedIndexKey = "kIdleSpchIDs.%d";
	/** Authored data key for Idle Spch IDs.%d.1. */
	static constexpr const char *kIdleSpchIDsSignedIndex1Key = "kIdleSpchIDs.%d.1";
	/** Authored data key for Idle Spch IDs.%d.2. */
	static constexpr const char *kIdleSpchIDsSignedIndex2Key = "kIdleSpchIDs.%d.2";
	/** World-state key for r1 LA1 History Count. */
	static constexpr const char *kR1LA1HistoryCountKey = "R1LA1HistoryCount";
	/** World-state key for r1 LA1 History:%d. */
	static constexpr const char *kR1LA1HistorySignedIndexKey = "R1LA1History:%d";
	/** World-state key for r1 LA1. */
	static constexpr const char *kR1LA1Key = "R1LA1";
	/** World-state key for r1 LA1 Fixed Level. */
	static constexpr const char *kR1LA1FixedLevelKey = "R1LA1FixedLevel";
	/** Authored data key for Slider SFX. */
	static constexpr const char *kSliderSFXKey = "kSliderSFX";
	/** Authored data key for Token Release Open. */
	static constexpr const char *kTokenReleaseOpenKey = "kTokenReleaseOpen";
	/** Authored data key for Release Token SFX. */
	static constexpr const char *kReleaseTokenSFXKey = "kReleaseTokenSFX";
	/** Authored data key for Token Bump SFX. */
	static constexpr const char *kTokenBumpSFXKey = "kTokenBumpSFX";
	/** Authored data key for Correct Spch IDs.%u. */
	static constexpr const char *kCorrectSpchIDsUnsignedIndexKey = "kCorrectSpchIDs.%u";
	/** Authored data key for Wrong Spch IDs.%u. */
	static constexpr const char *kWrongSpchIDsUnsignedIndexKey = "kWrongSpchIDs.%u";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Authored data key for Paige Spch IDs.1. */
	static constexpr const char *kPaigeSpchIDs1Key = "kPaigeSpchIDs.1";
	/** Authored data key for Auto Help IDs.%u. */
	static constexpr const char *kAutoHelpIDsUnsignedIndexKey = "kAutoHelpIDs.%u";
	/** Authored data key for Oops Spch IDs.%u. */
	static constexpr const char *kOopsSpchIDsUnsignedIndexKey = "kOopsSpchIDs.%u";
	/** Authored data key for Correct SFX. */
	static constexpr const char *kCorrectSFXKey = "kCorrectSFX";
	/** Authored data key for Wrong SFX. */
	static constexpr const char *kWrongSFXKey = "kWrongSFX";
	/** Script identifier for grg. */
	static constexpr const char *kGrgName = "grg";
	/** Authored data key for Instruction Spch IDs.1. */
	static constexpr const char *kInstructionSpchIDs1Key = "kInstructionSpchIDs.1";
	/** Authored data key for Token Release Highlight. */
	static constexpr const char *kTokenReleaseHighlightKey = "kTokenReleaseHighlight";
	/** Authored data key for Instruction Spch IDs.1.6. */
	static constexpr const char *kInstructionSpchIDs16Key = "kInstructionSpchIDs.1.6";
	/** Authored data key for Token Roll SFX. */
	static constexpr const char *kTokenRollSFXKey = "kTokenRollSFX";
	/** Authored data key for Path Delay. */
	static constexpr const char *kPathDelayKey = "kPathDelay";
	/** Authored data key for Path DX. */
	static constexpr const char *kPathDXKey = "kPathDX";
	/** Authored data key for Path DY. */
	static constexpr const char *kPathDYKey = "kPathDY";
	/** Authored data key for Gum Cycle SFX. */
	static constexpr const char *kGumCycleSFXKey = "kGumCycleSFX";
	/** Authored data key for Got Whistle Spch ID. */
	static constexpr const char *kGotWhistleSpchIDKey = "kGotWhistleSpchID";
	/** Authored data key for Marble SFX. */
	static constexpr const char *kMarbleSFXKey = "kMarbleSFX";
	/** Authored data key for Got Marble Spch IDs.%d. */
	static constexpr const char *kGotMarbleSpchIDsSignedIndexKey = "kGotMarbleSpchIDs.%d";
	/** Script identifier for rr. */
	static constexpr const char *kRrName = "rr";
	/** Authored data key for Put Away Spch IDs. */
	static constexpr const char *kPutAwaySpchIDsKey = "kPutAwaySpchIDs";
	/** Authored data key for Eat Candy Spch IDs.%d. */
	static constexpr const char *kEatCandySpchIDsSignedIndexKey = "kEatCandySpchIDs.%d";
	/** Authored data key for Eat Candy Spch IDs.%u. */
	static constexpr const char *kEatCandySpchIDsUnsignedIndexKey = "kEatCandySpchIDs.%u";
	/** Authored data key for Eat Candy Spch IDs.7. */
	static constexpr const char *kEatCandySpchIDs7Key = "kEatCandySpchIDs.7";
	/** Authored data key for Eat Candy Spch IDs.8. */
	static constexpr const char *kEatCandySpchIDs8Key = "kEatCandySpchIDs.8";
	/** Authored data key for Eat Candy Spch IDs.9. */
	static constexpr const char *kEatCandySpchIDs9Key = "kEatCandySpchIDs.9";
	/** Authored data key for COMPick Up Sfx ID. */
	static constexpr const char *kCOMPickUpSfxIDKey = "kCOMPickUpSfxID";
	/** Authored data key for INV1 Whistle. */
	static constexpr const char *kINV1WhistleKey = "kINV1Whistle";
	/** Authored data key for INV1 Marble4. */
	static constexpr const char *kINV1Marble4Key = "kINV1Marble4";
	/** Format for script names or values: item:%u. */
	static constexpr const char *kItemUnsignedIndexFormat = "item:%u";
	/** Authored data key for Put Away Spch IDs.2. */
	static constexpr const char *kPutAwaySpchIDs2Key = "kPutAwaySpchIDs.2";
	/** Authored data key for Instruction Spch IDs.3.%d. */
	static constexpr const char *kInstructionSpchIDs3SignedIndexKey = "kInstructionSpchIDs.3.%d";
	/** Authored data key for Instruction Spch IDs.2.%d. */
	static constexpr const char *kInstructionSpchIDs2SignedIndexKey = "kInstructionSpchIDs.2.%d";
	/** Authored data key for Instruction Spch IDs.2.4. */
	static constexpr const char *kInstructionSpchIDs24Key = "kInstructionSpchIDs.2.4";
	/** Authored data key for Instruction Spch IDs.2.5. */
	static constexpr const char *kInstructionSpchIDs25Key = "kInstructionSpchIDs.2.5";
	/** Authored data key for Instruction Spch IDs.2.6. */
	static constexpr const char *kInstructionSpchIDs26Key = "kInstructionSpchIDs.2.6";
	/** Authored data key for Paige Spch IDs.2. */
	static constexpr const char *kPaigeSpchIDs2Key = "kPaigeSpchIDs.2";
	/** Authored data key for Paige Spch IDs.3. */
	static constexpr const char *kPaigeSpchIDs3Key = "kPaigeSpchIDs.3";
	/** Authored data key for Paige Spch IDs.4. */
	static constexpr const char *kPaigeSpchIDs4Key = "kPaigeSpchIDs.4";
	/** Authored data key for Paige Spch IDs.%d. */
	static constexpr const char *kPaigeSpchIDsSignedIndexKey = "kPaigeSpchIDs.%d";
	/** Authored data key for Paige Spch IDs.11. */
	static constexpr const char *kPaigeSpchIDs11Key = "kPaigeSpchIDs.11";
	/** Authored data key for Paige Spch State4 IDs.%d. */
	static constexpr const char *kPaigeSpchState4IDsSignedIndexKey = "kPaigeSpchState4IDs.%d";
	/** Authored data key for Ramp Locked SFX. */
	static constexpr const char *kRampLockedSFXKey = "kRampLockedSFX";
	/** Authored data key for Paige Spch IDs.13. */
	static constexpr const char *kPaigeSpchIDs13Key = "kPaigeSpchIDs.13";
	/** Authored data key for Ramp Flip SFX. */
	static constexpr const char *kRampFlipSFXKey = "kRampFlipSFX";
	/** Authored data key for Gum HSX.1. */
	static constexpr const char *kGumHSX1Key = "kGumHSX.1";
	/** Authored data key for Gum HSY.1. */
	static constexpr const char *kGumHSY1Key = "kGumHSY.1";
	/** Authored data key for Gum HSX.2. */
	static constexpr const char *kGumHSX2Key = "kGumHSX.2";
	/** Authored data key for Gum HSY.2. */
	static constexpr const char *kGumHSY2Key = "kGumHSY.2";
	/** Authored data key for Main Spch IDs.3. */
	static constexpr const char *kMainSpchIDs3Key = "kMainSpchIDs.3";
	/** Authored data key for Enter Spch IDs.3. */
	static constexpr const char *kEnterSpchIDs3Key = "kEnterSpchIDs.3";
	/** Authored data key for Main Spch IDs.%d. */
	static constexpr const char *kMainSpchIDsSignedIndexKey = "kMainSpchIDs.%d";
	/** Authored data key for Idle Spch IDs.1. */
	static constexpr const char *kIdleSpchIDs1Key = "kIdleSpchIDs.1";
	/** Authored data key for Main Spch IDs.4. */
	static constexpr const char *kMainSpchIDs4Key = "kMainSpchIDs.4";
	/** Authored data key for Main Spch IDs.5. */
	static constexpr const char *kMainSpchIDs5Key = "kMainSpchIDs.5";
	/** Authored data key for Ramp RHighlight. */
	static constexpr const char *kRampRHighlightKey = "kRampRHighlight";
	/** Authored data key for Ramp LHighlight. */
	static constexpr const char *kRampLHighlightKey = "kRampLHighlight";

	/** Text resource path indexed by resource ID. */
	static constexpr const char *kTextFormat = "r1la1/%u.txt";
	/** Gumball font path indexed by resource ID. */
	static constexpr const char *kFontFormat = "common/%d.nfnt";
	/** Gumball puzzle datasets. */
	static constexpr const char *kDataXML = "r1la1data.xml";

	enum Phase {
		kIntro,
		kInstructions,
		kTrayOut,
		kTokenExplain,
		kTokenBlink,
		kReady,
		kRelease,
		kBump,
		kRolling,
		kAnswer,
		kTrayIn,
		kRewardRoll,
		kRewardSpeech,
		kPickup,
		kPutAway,
	};
	enum Reward {
		kNoReward,
		kWhistle,
		kMarble,
		kCandy,
	};
	struct Ramp {
		/** Sprite displaying this token-routing ramp. */
		Shell::Sprite *sprite = nullptr;
		/** Whether the ramp directs the token to its right-hand exit. */
		bool right = false;
		/** Whether the current exercise prevents the player from changing this ramp. */
		bool locked = false;
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Resource-path resolver for the activity's current page. */
	Shell::ResourcePaths _paths;
	/** FontMan-owned font used for activity text and labels. */
	const Graphics::Font *_font = nullptr;
	/** Parsed XML data used to construct the current activity round. */
	OM::XMLParser _data;
	/** Word list used to resolve candidate-word speech resources. */
	Common::StringArray _wordsList;
	/** Sentence-part list used to resolve phrase speech resources. */
	Common::StringArray _partsList;
	/** Text objects displaying candidate words and the current sentence. */
	Common::Array<Shell::TextObject *> _text;
	/** Spoken resource associated with each displayed text object. */
	Common::Array<uint> _textSpeech;
	struct TextReading {
		/** First speech-queue entry associated with this text highlight. */
		uint first;
		/** Exclusive end of the speech-queue range for this highlight. */
		uint end;
		/** Bit mask of text objects highlighted during this speech range. */
		uint mask;
	};
	/** Speech-queue ranges and their corresponding text highlight masks. */
	Common::Array<TextReading> _readings;
	/** Sprites created for the gumball-machine presentation. */
	Common::Array<Shell::Sprite *> _sprites;
	/** Animations to resume after activity playback is unpaused. */
	Common::Array<Shell::Sprite *> _pausedSprites;
	/** Routing nodes indexed by the authored token-path numbering. */
	Ramp _ramps[23];
	/** Token-release control sprite. */
	Shell::Sprite *_release = nullptr;
	/** Token sprite at the release position. */
	Shell::Sprite *_token = nullptr;
	/** Token sprite animated along the selected ramp path. */
	Shell::Sprite *_movingToken = nullptr;
	/** Tray displaying the machine's current reward. */
	Shell::Sprite *_tray = nullptr;
	/** Sprite used while retracting the reward tray. */
	Shell::Sprite *_trayIn = nullptr;
	/** Sprite used while extending the reward tray. */
	Shell::Sprite *_trayOut = nullptr;
	/** Sparkle effect for reward feedback. */
	Shell::Sprite *_sparkle = nullptr;
	/** Collectible whistle sprite. */
	Shell::Sprite *_whistle = nullptr;
	/** Collectible marble sprite. */
	Shell::Sprite *_marble = nullptr;
	/** Collectible candy sprite. */
	Shell::Sprite *_candy = nullptr;
	/** Sprites for the rolling reward presentation. */
	Shell::Sprite *_rolls[4] = {};
	/** Sprite used for blowing a whistle. */
	Shell::Sprite *_blow = nullptr;
	/** Sprite used for eating candy. */
	Shell::Sprite *_eat = nullptr;
	/** Token markers above candidate slots already answered correctly. */
	Shell::Sprite *_correctTokens[3] = {};
	/** Looping sound played while the token rolls. */
	Common::ScopedPtr<OM::Sound> _rollingSound;
	/** Routing-node indices visited by the released token. */
	Common::Array<uint> _path;
	/** Remaining correct-answer speech variants. */
	Common::Array<uint> _correctChoices;
	/** Remaining incorrect-answer speech variants. */
	Common::Array<uint> _wrongChoices;
	/** Remaining token-routing mistake speech variants. */
	Common::Array<uint> _oopsChoices;
	/** Remaining automatic-help speech variants. */
	Common::Array<uint> _helpChoices;
	/** Remaining unpleasant-candy response variants. */
	Common::Array<uint> _yuckChoices;
	/** Three candidate words in their displayed order. */
	Common::String _words[3];
	/** Sentence prefix, blank placeholder and suffix. */
	Common::String _parts[3];
	/** One-based authored answer numbers in displayed candidate order. */
	int _order[3] = {
		1,
		2,
		3,
	};
	/** Difficulty selected for the current activity round. */
	int _level = 1;
	/** Authored dataset selected for the current difficulty. */
	int _dataset = 0;
	/** Exercise set selected within the sentence dataset. */
	int _set = 1;
	/** Saved activity progress through introduction, puzzles and rewards. */
	int _state = 1;
	/** Correct responses counted toward the next reward. */
	int _correct = 0;
	/** Total responses recorded by the activity. */
	int _total = 0;
	/** Incorrect responses counted for activity feedback. */
	int _wrong = 0;
	/** Previously chosen answer slot, or zero when no repeat is pending. */
	int _previousSlot = 0;
	/** Consecutive token exits at the leftmost missed destination. */
	int _missLeft = 0;
	/** Consecutive token exits at the rightmost missed destination. */
	int _missRight = 0;
	/** Counter selecting gumball-machine help responses. */
	int _gumClicks = 0;
	/** Counter selecting Paige's help responses. */
	int _paigeClicks = 0;
	/** Number of ramp interactions used to select guidance. */
	int _rampClicks = 0;
	/** Number of candidate-word interactions used to select guidance. */
	int _wordClicks = 0;
	/** Counter selecting candy pickup responses. */
	int _candyClicks = 0;
	/** Index of the marble reward announcement. */
	int _marbleSpeech = 0;
	/** Whether a ramp has been tried during the current guidance stage. */
	bool _rampClicked = false;
	/** Whether token release has been tried during the current guidance stage. */
	bool _tokenClicked = false;
	/** Whether sentence reading has been tried during the current guidance stage. */
	bool _sentenceClicked = false;
	/** Whether the current puzzle has included an incorrect answer. */
	bool _imperfect = false;
	/** Whether the most recent token destination contains the correct word. */
	bool _answerCorrect = false;
	/** Whether sentence and candidate text are displayed. */
	bool _showText = false;
	/** Whether the introductory instructions still require completion. */
	bool _finishInstrs = false;
	/** One-based candidate slot reached by the token, or zero for a missed exit. */
	int _chosenSlot = 0;
	/** Current segment of the token's routing path. */
	uint _segment = 0;
	/** Current step of the timed reward blink sequence. */
	uint _blinkStep = 0;
	/** Millisecond deadline for the next timed activity transition. */
	uint32 _deadline = 0;
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kIntro;
	/** Reward selected for the current dispensing sequence. */
	Reward _reward = kNoReward;
	/** Reward currently being collected or used. */
	Reward _pickup = kNoReward;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Create a gumball-machine sprite using authored resource and depth keys. */
	Shell::Sprite *sprite(const char *resource, const char *depth);
	/** Play an effect identified by an authored sound key. */
	void sound(const char *key);
	/** Play the authored spoken line for the selected actor. */
	void say(const char *actor, const Common::String &key);
	/** Queue spoken lines using the authored actor sequence. */
	void group(const char *actors, const Common::String &key);
	/** Load a text resource into a word or sentence-part list. */
	bool loadList(uint resource, Common::StringArray &list);
	/** Find a normalized word in an authored list, returning minus one when absent. */
	static int wordIndex(const Common::StringArray &list, Common::String word);
	/** Resolve the spoken resource for a candidate word or sentence part. */
	uint wordSpeech(const Common::String &word, bool part) const;
	/** Choose an exercise dataset for the current difficulty. */
	void randomDataset();
	/** Choose ramp directions and lock the difficulty-specific fixed ramps. */
	void setupRamps();
	/** Refresh ramp pictures from their direction, lock and rolling state. */
	void rampFrames();
	/** Follow the current ramp directions to construct the token's route. */
	void buildPath();
	/** Return a routing node's authored game-pixel position. */
	Common::Point tokenPoint(uint node) const;
	/** Load a sentence exercise, permute its candidate words and display it. */
	bool puzzle();
	/** Delete the displayed text objects and clear speech-highlight ranges. */
	void clearText();
	/** Create and register a text object with its position, alignment and spoken resource. */
	Shell::TextObject *text(const Common::String &value, int x, int y, bool centered, uint speech);
	/** Display the sentence with a blank or the word from a one-based candidate slot. */
	void sentence(int slot = 0);
	/** Queue the sentence reading for a blank or a one-based candidate slot. */
	void readSentence(int slot = 0);
	/** Queue a spoken line or sound and associate it with a text highlight mask. */
	void readText(uint resource, uint mask, bool sound = false);
	/** Return the bit mask of text objects associated with the current speech. */
	uint highlightedText() const;
	/** Schedule an idle reminder at a millisecond timestamp when page interaction permits it. */
	void idle(uint32 time);
	/** Begin extending the token-release slider. */
	void sliderOut();
	/** Build the token path and start release feedback. */
	void releaseToken();
	/** Evaluate the reached candidate slot and queue answer feedback. */
	void checkAnswer();
	/** Record puzzle performance and update difficulty when required. */
	void assess();
	/** Write the activity's current saved state to the world. */
	void persist();
	/** Begin the selected reward's dispensing sequence. */
	void dispense();
	/** Advance the exercise set and initialize the next sentence puzzle. */
	void nextPuzzle();
	/** Collect or use the selected reward and update inventory when applicable. */
	void pickup(Reward item);
	/** Queue context-sensitive guidance from the gumball machine. */
	void gumHelp();
	/** Queue Paige's context-sensitive gumball-machine guidance. */
	void paigeHelp();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
