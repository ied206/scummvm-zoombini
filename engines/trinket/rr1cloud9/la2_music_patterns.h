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

#ifndef TRINKET_RR1CLOUD9_LA2_MUSIC_PATTERNS_H
#define TRINKET_RR1CLOUD9_LA2_MUSIC_PATTERNS_H

#include "trinket/ompp/sound.h"
#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** LA2: repeat Marvin's musical patterns using reusable collected marbles. */
class MusicPatterns {
public:
	/** Connect the musical pattern activity to its page services. */
	explicit MusicPatterns(PageScene &host) : _host(host) {}
	/** Release the musical pattern presentation state. */
	~MusicPatterns();
	/** Load the musical pattern resources and initialize the current round. */
	bool enter();
	/** Draw the background's colored grid rows before the independently recolored sprites. */
	void draw(OM::Canvas &canvas);
	/** Advance the musical pattern presentation to its next ready stage. */
	void advance(uint32 time);
	/** Advance musical pattern animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Handle activity pointer or keyboard input and report whether it was consumed. */
	bool event(const Common::Event &event);
	/** Accept available inventory marbles and report whether the donation was handled. */
	bool donate(uint item);
	/** Interrupt skippable speech or finish the current playback instructions. */
	void skipSpeech();
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _phase != kIdle || 0 <= _dragged; }
	/** Pause or unpause activity feedback and sprite playback. */
	void pause(bool paused);
	/** Resume activity feedback and restore its animation timing. */
	void resume(uint32 elapsed);

private:
	/** Format for script names or values: %s.%d. */
	static constexpr const char *kNameSignedIndexFormat = "%s.%d";
	/** Format for script names or values: .%u. */
	static constexpr const char *kUnsignedIndexFormat = ".%u";
	/** Format for script names or values: k%sCharIDs.%d. */
	static constexpr const char *kKNameCharIDsSignedIndexFormat = "k%sCharIDs.%d";
	/** Format for script names or values: k%sSpchIDs.%d. */
	static constexpr const char *kKNameSpchIDsSignedIndexFormat = "k%sSpchIDs.%d";
	/** Script property name for grid. */
	static constexpr const char *kPropertyGrid = "grid";
	/** Authored data key for Local. */
	static constexpr const char *kLocalKey = "kLocal";
	/** Script property name for z. */
	static constexpr const char *kPropertyZ = "z";
	/** Authored data key for Grid Z. */
	static constexpr const char *kGridZKey = "kGridZ";
	/** Script property name for animate Settled. */
	static constexpr const char *kPropertyAnimateSettled = "animateSettled";
	/** World-state key for r1 LA2 Level. */
	static constexpr const char *kR1LA2LevelKey = "R1LA2Level";
	/** World-state key for r1 LA2 Dataset. */
	static constexpr const char *kR1LA2DatasetKey = "R1LA2Dataset";
	/** World-state key for r1 LA2 Count. */
	static constexpr const char *kR1LA2CountKey = "R1LA2Count";
	/** World-state key for r1 LA2 State. */
	static constexpr const char *kR1LA2StateKey = "R1LA2State";
	/** World-state key for r1 LA2 Pillow Here. */
	static constexpr const char *kR1LA2PillowHereKey = "R1LA2PillowHere";
	/** World-state key for r1 LA2 Marble%d Count. */
	static constexpr const char *kR1LA2MarbleSignedIndexCountKey = "R1LA2Marble%dCount";
	/** Authored data key for Door ID.%d. */
	static constexpr const char *kDoorIDSignedIndexKey = "kDoorID.%d";
	/** Authored data key for Door Z. */
	static constexpr const char *kDoorZKey = "kDoorZ";
	/** Authored data key for Column Lights ID. */
	static constexpr const char *kColumnLightsIDKey = "kColumnLightsID";
	/** Authored data key for Column Light X. */
	static constexpr const char *kColumnLightXKey = "kColumnLightX";
	/** Authored data key for Column Light Delta X. */
	static constexpr const char *kColumnLightDeltaXKey = "kColumnLightDeltaX";
	/** Authored data key for Column Light Y. */
	static constexpr const char *kColumnLightYKey = "kColumnLightY";
	/** Authored data key for Lock ID. */
	static constexpr const char *kLockIDKey = "kLockID";
	/** Authored data key for Lock Z. */
	static constexpr const char *kLockZKey = "kLockZ";
	/** Authored data key for Grid X. */
	static constexpr const char *kGridXKey = "kGridX";
	/** Authored data key for Grid Delta X. */
	static constexpr const char *kGridDeltaXKey = "kGridDeltaX";
	/** Authored data key for Grid Y. */
	static constexpr const char *kGridYKey = "kGridY";
	/** Authored data key for Grid Delta Y. */
	static constexpr const char *kGridDeltaYKey = "kGridDeltaY";
	/** Authored data key for Button ID. */
	static constexpr const char *kButtonIDKey = "kButtonID";
	/** Authored data key for Door Lights ID. */
	static constexpr const char *kDoorLightsIDKey = "kDoorLightsID";
	/** Authored data key for Activate Door ID. */
	static constexpr const char *kActivateDoorIDKey = "kActivateDoorID";
	/** Authored data key for Spin ID. */
	static constexpr const char *kSpinIDKey = "kSpinID";
	/** Authored data key for Spin Z. */
	static constexpr const char *kSpinZKey = "kSpinZ";
	/** Authored data key for Pillow ID. */
	static constexpr const char *kPillowIDKey = "kPillowID";
	/** Authored data key for Pillow Z. */
	static constexpr const char *kPillowZKey = "kPillowZ";
	/** Authored data key for Jewel ID. */
	static constexpr const char *kJewelIDKey = "kJewelID";
	/** Authored data key for Jewel Z. */
	static constexpr const char *kJewelZKey = "kJewelZ";
	/** Authored data key for Sparkle ID. */
	static constexpr const char *kSparkleIDKey = "kSparkleID";
	/** Authored data key for Tray Mask ID. */
	static constexpr const char *kTrayMaskIDKey = "kTrayMaskID";
	/** Authored data key for Tray Mask Z. */
	static constexpr const char *kTrayMaskZKey = "kTrayMaskZ";
	/** Authored data key for Sparkle Delta Y. */
	static constexpr const char *kSparkleDeltaYKey = "kSparkleDeltaY";
	/** Authored data key for Jewel Delta Y. */
	static constexpr const char *kJewelDeltaYKey = "kJewelDeltaY";
	/** Authored data key for Re Enter Spch IDs.4.1. */
	static constexpr const char *kReEnterSpchIDs41Key = "kReEnterSpchIDs.4.1";
	/** Authored data key for Local Spch IDs.4.1.1. */
	static constexpr const char *kLocalSpchIDs411Key = "kLocalSpchIDs.4.1.1";
	/** Script identifier for local. */
	static constexpr const char *kLocalName = "Local";
	/** Authored data key for Local Char IDs.1.6. */
	static constexpr const char *kLocalCharIDs16Key = "kLocalCharIDs.1.6";
	/** Authored data key for Local Spch IDs.1.8. */
	static constexpr const char *kLocalSpchIDs18Key = "kLocalSpchIDs.1.8";
	/** Authored data key for Max Lights. */
	static constexpr const char *kMaxLightsKey = "kMaxLights";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Authored data key for Re Enter Spch IDs.3.1. */
	static constexpr const char *kReEnterSpchIDs31Key = "kReEnterSpchIDs.3.1";
	/** Authored data key for Re Enter Spch IDs.%d.%d. */
	static constexpr const char *kReEnterSpchIDsSignedIndexSignedIndexKey = "kReEnterSpchIDs.%d.%d";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Authored data key for Re Enter Spch IDs.4.2. */
	static constexpr const char *kReEnterSpchIDs42Key = "kReEnterSpchIDs.4.2";
	/** Authored data key for Re Enter Spch IDs.5.1. */
	static constexpr const char *kReEnterSpchIDs51Key = "kReEnterSpchIDs.5.1";
	/** Authored data key for Re Enter Spch IDs.6.1. */
	static constexpr const char *kReEnterSpchIDs61Key = "kReEnterSpchIDs.6.1";
	/** Script property name for data Set/Level %d/Word List %d/Set 1. */
	static constexpr const char *kPropertyDataSetLevelSignedIndexWordListSignedIndexSet1 = "DataSet/Level_%d/WordList_%d/Set_1";
	/** Authored data key for Num Given. */
	static constexpr const char *kNumGivenKey = "kNumGiven";
	/** Authored data key for Marble ID.%d. */
	static constexpr const char *kMarbleIDSignedIndexKey = "kMarbleID.%d";
	/** Authored data key for Marble Z. */
	static constexpr const char *kMarbleZKey = "kMarbleZ";
	/** Authored data key for Marble Delta Z. */
	static constexpr const char *kMarbleDeltaZKey = "kMarbleDeltaZ";
	/** Authored data key for Marble X. */
	static constexpr const char *kMarbleXKey = "kMarbleX";
	/** Authored data key for Marble Delta X. */
	static constexpr const char *kMarbleDeltaXKey = "kMarbleDeltaX";
	/** Authored data key for Marble Y. */
	static constexpr const char *kMarbleYKey = "kMarbleY";
	/** Authored data key for Marble Delta Y. */
	static constexpr const char *kMarbleDeltaYKey = "kMarbleDeltaY";
	/** Authored data key for INV1 Marble1. */
	static constexpr const char *kINV1Marble1Key = "kINV1Marble1";
	/** Format for script names or values: item:%u. */
	static constexpr const char *kItemUnsignedIndexFormat = "item:%u";
	/** Authored data key for Scene Sfx ID. */
	static constexpr const char *kSceneSfxIDKey = "kSceneSfxID";
	/** Authored data key for Go Home Sfx ID. */
	static constexpr const char *kGoHomeSfxIDKey = "kGoHomeSfxID";
	/** Authored data key for Snap Sfx ID. */
	static constexpr const char *kSnapSfxIDKey = "kSnapSfxID";
	/** Authored data key for Column Light Bad ID. */
	static constexpr const char *kColumnLightBadIDKey = "kColumnLightBadID";
	/** Authored data key for Marble Sing Frame. */
	static constexpr const char *kMarbleSingFrameKey = "kMarbleSingFrame";
	/** Authored data key for Marble ID. */
	static constexpr const char *kMarbleIDKey = "kMarbleID";
	/** Authored data key for Blank Sfx ID. */
	static constexpr const char *kBlankSfxIDKey = "kBlankSfxID";
	/** Authored data key for Marble Eye Close Frame. */
	static constexpr const char *kMarbleEyeCloseFrameKey = "kMarbleEyeCloseFrame";
	/** Authored data key for Wrong Spch IDs.%d. */
	static constexpr const char *kWrongSpchIDsSignedIndexKey = "kWrongSpchIDs.%d";
	/** Authored data key for Scene Char IDs.2. */
	static constexpr const char *kSceneCharIDs2Key = "kSceneCharIDs.2";
	/** Authored data key for Scene Spch IDs.2. */
	static constexpr const char *kSceneSpchIDs2Key = "kSceneSpchIDs.2";
	/** Authored data key for No Move Spch ID.%d. */
	static constexpr const char *kNoMoveSpchIDSignedIndexKey = "kNoMoveSpchID.%d";
	/** World-state key for r1 LA2 Streak Kind. */
	static constexpr const char *kR1LA2StreakKindKey = "R1LA2StreakKind";
	/** World-state key for r1 LA2 Streak. */
	static constexpr const char *kR1LA2StreakKey = "R1LA2Streak";
	/** Authored data key for Autolevel Down Count. */
	static constexpr const char *kAutolevelDownCountKey = "kAutolevelDownCount";
	/** Authored data key for Autolevel Up Count. */
	static constexpr const char *kAutolevelUpCountKey = "kAutolevelUpCount";
	/** World-state key for r1 LA2. */
	static constexpr const char *kR1LA2Key = "R1LA2";
	/** World-state key for r1 LA2 Fixed Level. */
	static constexpr const char *kR1LA2FixedLevelKey = "R1LA2FixedLevel";
	/** Authored data key for Have Spch.1.1. */
	static constexpr const char *kHaveSpch11Key = "kHaveSpch.1.1";
	/** Script identifier for g Chester Open ID. */
	static constexpr const char *kGChesterOpenIDName = "gChesterOpenID";
	/** Script identifier for g Closed Chester Z. */
	static constexpr const char *kGClosedChesterZName = "gClosedChesterZ";
	/** World-state key for item:%d. */
	static constexpr const char *kItemSignedIndexKey = "item:%d";
	/** Authored data key for Have Spch.2.1. */
	static constexpr const char *kHaveSpch21Key = "kHaveSpch.2.1";
	/** Authored data key for Have Spch.3.1. */
	static constexpr const char *kHaveSpch31Key = "kHaveSpch.3.1";
	/** Authored data key for Have Spch.2.2. */
	static constexpr const char *kHaveSpch22Key = "kHaveSpch.2.2";
	/** Authored data key for Have Spch.3.2. */
	static constexpr const char *kHaveSpch32Key = "kHaveSpch.3.2";
	/** Authored data key for Scene Spch IDs.1.%d. */
	static constexpr const char *kSceneSpchIDs1SignedIndexKey = "kSceneSpchIDs.1.%d";
	/** Authored data key for Empty Grid Sfx ID. */
	static constexpr const char *kEmptyGridSfxIDKey = "kEmptyGridSfxID";
	/** Authored data key for Congrats Spch ID.%d. */
	static constexpr const char *kCongratsSpchIDSignedIndexKey = "kCongratsSpchID.%d";
	/** Authored data key for Door Sfx ID.1. */
	static constexpr const char *kDoorSfxID1Key = "kDoorSfxID.1";
	/** Authored data key for Door Sfx ID.2. */
	static constexpr const char *kDoorSfxID2Key = "kDoorSfxID.2";
	/** Script identifier for door Open. */
	static constexpr const char *kDoorOpenName = "DoorOpen";
	/** Authored data key for Fan Fare Sfx ID. */
	static constexpr const char *kFanFareSfxIDKey = "kFanFareSfxID";
	/** Authored data key for Last Door Char IDs. */
	static constexpr const char *kLastDoorCharIDsKey = "kLastDoorCharIDs";
	/** Authored data key for Last Door Spch IDs. */
	static constexpr const char *kLastDoorSpchIDsKey = "kLastDoorSpchIDs";
	/** Authored data key for Door Open Char IDs.4.%d. */
	static constexpr const char *kDoorOpenCharIDs4SignedIndexKey = "kDoorOpenCharIDs.4.%d";
	/** Authored data key for Door Open Spch IDs.4. */
	static constexpr const char *kDoorOpenSpchIDs4Key = "kDoorOpenSpchIDs.4";
	/** Authored data key for New Pattern Sfx ID. */
	static constexpr const char *kNewPatternSfxIDKey = "kNewPatternSfxID";
	/** Authored data key for Idle%d. */
	static constexpr const char *kIdleSignedIndexKey = "kIdle%d";
	/** Format for script names or values: CharID.%d. */
	static constexpr const char *kCharIDSignedIndexFormat = "CharID.%d";
	/** Format for script names or values: SpchID.%d. */
	static constexpr const char *kSpchIDSignedIndexFormat = "SpchID.%d";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Authored data key for Paige Spch IDs.2.%d. */
	static constexpr const char *kPaigeSpchIDs2SignedIndexKey = "kPaigeSpchIDs.2.%d";
	/** Authored data key for Paige Spch IDs.3. */
	static constexpr const char *kPaigeSpchIDs3Key = "kPaigeSpchIDs.3";
	/** Authored data key for Paige Spch IDs.4.2. */
	static constexpr const char *kPaigeSpchIDs42Key = "kPaigeSpchIDs.4.2";
	/** Authored data key for Paige Spch IDs.4.1.1. */
	static constexpr const char *kPaigeSpchIDs411Key = "kPaigeSpchIDs.4.1.1";
	/** Authored data key for Paige Spch IDs.4.1.2. */
	static constexpr const char *kPaigeSpchIDs412Key = "kPaigeSpchIDs.4.1.2";
	/** Authored data key for Paige Spch IDs.4.1.3. */
	static constexpr const char *kPaigeSpchIDs413Key = "kPaigeSpchIDs.4.1.3";
	/** Authored data key for Wrong Sfx ID. */
	static constexpr const char *kWrongSfxIDKey = "kWrongSfxID";
	/** Script identifier for play. */
	static constexpr const char *kPlayName = "Play";
	/** Authored data key for Pick Up Sfx ID. */
	static constexpr const char *kPickUpSfxIDKey = "kPickUpSfxID";
	/** Authored data key for COMSoft Click Sfx ID. */
	static constexpr const char *kCOMSoftClickSfxIDKey = "kCOMSoftClickSfxID";
	/** Authored data key for Door Clk Spch IDs.4. */
	static constexpr const char *kDoorClkSpchIDs4Key = "kDoorClkSpchIDs.4";
	/** Authored data key for Local Spch IDs.3.1. */
	static constexpr const char *kLocalSpchIDs31Key = "kLocalSpchIDs.3.1";
	/** Authored data key for Local Spch IDs.3.2. */
	static constexpr const char *kLocalSpchIDs32Key = "kLocalSpchIDs.3.2";
	/** Script identifier for door Clk. */
	static constexpr const char *kDoorClkName = "DoorClk";
	/** Authored data key for Max Door Clk Spch. */
	static constexpr const char *kMaxDoorClkSpchKey = "kMaxDoorClkSpch";
	/** Authored data key for INV1 Pillow. */
	static constexpr const char *kINV1PillowKey = "kINV1Pillow";
	/** Authored data key for Jewel Drop Sfx ID. */
	static constexpr const char *kJewelDropSfxIDKey = "kJewelDropSfxID";
	/** Authored data key for Pillow Spch ID. */
	static constexpr const char *kPillowSpchIDKey = "kPillowSpchID";
	/** Authored data key for Light Spch ID. */
	static constexpr const char *kLightSpchIDKey = "kLightSpchID";
	/** Script identifier for jewel. */
	static constexpr const char *kJewelName = "Jewel";
	/** Authored data key for Num Jewel Spchs. */
	static constexpr const char *kNumJewelSpchsKey = "kNumJewelSpchs";
	/** Script identifier for scene. */
	static constexpr const char *kSceneName = "Scene";
	/** Authored data key for Local Spch IDs.5.1.1. */
	static constexpr const char *kLocalSpchIDs511Key = "kLocalSpchIDs.5.1.1";
	/** Authored data key for Local Spch IDs.5.1.2. */
	static constexpr const char *kLocalSpchIDs512Key = "kLocalSpchIDs.5.1.2";
	/** Authored data key for Local Spch IDs.5.2.%d. */
	static constexpr const char *kLocalSpchIDs52SignedIndexKey = "kLocalSpchIDs.5.2.%d";
	/** Script identifier for spp. */
	static constexpr const char *kSppName = "spp";
	/** Script identifier for rg. */
	static constexpr const char *kRgName = "rg";
	/** Script identifier for rgg. */
	static constexpr const char *kRggName = "rgg";
	/** Script identifier for sgr. */
	static constexpr const char *kSgrName = "sgr";
	/** Authored data key for Main Spch IDs.%d.%d.%u. */
	static constexpr const char *kMainSpchIDsSignedIndexSignedIndexUnsignedIndexKey = "kMainSpchIDs.%d.%d.%u";

	/** Music-pattern puzzle datasets. */
	static constexpr const char *kDataXML = "r1la2.xml";

	enum Phase {
		kEntryGreeting,
		kInventoryStart,
		kChestOpening,
		kBackpackOpening,
		kInventorySpeech,
		kChestClosing,
		kDonationSpeech,
		kIntroduction,
		kIdle,
		kPlayStart,
		kPlayback,
		kWrongSpeech,
		kSpin,
		kCongratulations,
		kDoor,
		kNext,
	};
	enum Result {
		kListen,
		kWrong,
		kCorrect,
		kLocked,
	};
	struct Marble {
		/** Sprite displaying this musical marble. */
		Shell::Sprite *sprite = nullptr;
		/** Game-pixel bank position to restore after an invalid drop. */
		Common::Point home;
		/** Zero-based musical note row associated with the marble. */
		int row = 0;
		/** Zero-based grid column, or minus one while the marble is in the bank. */
		int column = -1;
		/** Normal scene depth restored when the marble is no longer dragged. */
		int depth = 0;
		/** Whether this marble belongs to the uneditable given pattern. */
		bool fixed = false;
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Parsed XML data used to construct the current activity round. */
	OM::XMLParser _data;
	/** Fixed pattern marbles and draggable marbles supplied by the player. */
	Common::Array<Marble> _marbles;
	/** Animations to resume after activity playback is unpaused. */
	Common::Array<Shell::Sprite *> _pausedSprites;
	/** Playback lights indexed by the twelve pattern columns. */
	Shell::Sprite *_lights[12] = {};
	/** Lock markers for fixed notes in the given pattern. */
	Shell::Sprite *_locks[5][8] = {};
	/** Door sprites showing successive groups of completed patterns. */
	Shell::Sprite *_doors[3] = {};
	/** Control sprite used to play or check the current pattern. */
	Shell::Sprite *_button = nullptr;
	/** Sprite displaying progress toward opening all three doors. */
	Shell::Sprite *_doorLights = nullptr;
	/** Sprite used when activating the pattern mechanism. */
	Shell::Sprite *_activate = nullptr;
	/** Sprite used during mechanism-spin feedback. */
	Shell::Sprite *_spin = nullptr;
	/** Collectible pillow displayed beneath the jewel. */
	Shell::Sprite *_pillow = nullptr;
	/** Jewel sprite used for completion feedback. */
	Shell::Sprite *_jewel = nullptr;
	/** Sparkle effect accompanying the jewel. */
	Shell::Sprite *_sparkle = nullptr;
	/** Sprite used during the inventory-chest opening sequence. */
	Shell::Sprite *_chestOpening = nullptr;
	/** Sound played for each of the five musical note rows. */
	OM::Sound *_notes[5] = {};
	/** Required notes indexed by row and pattern column. */
	bool _target[5][12] = {};
	/** Number of donated marbles available in each note row. */
	int _bank[5] = {};
	/** Number of editable target notes required in each row. */
	int _needed[5] = {};
	/** Difficulty selected for the current activity round. */
	int _level = 1;
	/** Authored dataset selected for the current difficulty. */
	int _dataset = 1;
	/** Number of columns supplied as the fixed repeating pattern. */
	int _given = 0;
	/** Number of musical patterns completed by the player. */
	int _count = 0;
	/** Saved progress through marble donation and pattern play. */
	int _state = 3;
	/** Incorrect pattern checks in the current exercise. */
	int _mistakes = 0;
	/** Index of the piece being dragged, or minus one when no piece is held. */
	int _dragged = -1;
	/** Current playback column, or minus one before playback begins. */
	int _column = -1;
	/** Exclusive end of the requested playback range. */
	int _lastColumn = 12;
	/** Note row played alone, or minus one to play all rows. */
	int _soloRow = -1;
	/** Counter selecting responses to the playback control. */
	int _playClicks = 0;
	/** Counter selecting responses to door clicks. */
	int _doorClicks = 0;
	/** Counter selecting the resident character's responses. */
	int _localClicks = 0;
	/** Counter selecting Paige's help responses. */
	int _paigeClicks = 0;
	/** Counter selecting Reader and Sam's responses. */
	int _mainClicks = 0;
	/** Counter selecting jewel responses. */
	int _jewelClicks = 0;
	/** Counter selecting the next idle reminder. */
	int _idleIndex = 0;
	/** Pointer displacement from the dragged piece's anchor, in pixels. */
	Common::Point _dragOffset;
	/** Millisecond timestamp used to schedule idle feedback. */
	uint32 _idle = 0;
	/** Millisecond timestamp when the chest-closing delay began, or zero before it starts. */
	uint32 _closeStart = 0;
	/** Whether the inventory introduction still requires guidance. */
	bool _inventoryHelp = false;
	/** Whether the given pattern should play after entry speech. */
	bool _playOnEntry = false;
	/** Whether inventory contains any musical marbles when checked. */
	bool _haveMarbles = false;
	/** Whether the current inventory interaction supplied any marbles. */
	bool _donated = false;
	/** Whether the current donation is the player's first. */
	bool _firstDonation = false;
	/** Whether guidance should follow the current pattern playback. */
	bool _helpAfterPlayback = false;
	/** Whether the collectible pillow remains on the page. */
	bool _pillowHere = true;
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kEntryGreeting;
	/** Purpose and expected result of the current playback sequence. */
	Result _result = kListen;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Read an authored numeric constant selected by an indexed name. */
	int indexed(const char *key, int index) const;
	/** Create a musical-pattern sprite from an authored resource key and depth. */
	Shell::Sprite *sprite(const Common::String &key, int depth);
	/** Play the authored spoken line for the selected actor. */
	void say(const char *actor, const Common::String &key);
	/** Queue a spoken resource for the selected actor. */
	void speech(const Common::String &actor, uint resource);
	/** Queue a fixed number of lines using authored actor and resource sequences. */
	void speeches(const Common::String &actors, const Common::String &resources, uint count);
	/** Play an effect identified by an authored sound key. */
	void sound(const Common::String &key);
	/** Queue an indexed speech group described by an authored key prefix. */
	void group(const char *prefix, int index, int count);
	/** Prepare and display the next musical pattern round. */
	bool setup();
	/** Remove the current marble sprites and clear their placement records. */
	void clearMarbles();
	/** Return a marble selected by index to its bank position. */
	void home(int index);
	/** Place the dragged marble in the grid or return it to the bank. */
	void drop();
	/** Return the marble index at a grid row and column, or minus one when empty. */
	int resident(int row, int column) const;
	/** Refresh the playback lights from the current column and note state. */
	void lights();
	/** Refresh door and progress-light pictures from completed patterns. */
	void doors();
	/** Write the activity's current saved state to the world. */
	void persist();
	/** Record pattern performance and update difficulty when required. */
	void assess();
	/** Begin playback over a column range, optionally restricted to one note row. */
	void play(Result result, int first = 0, int last = 12, int soloRow = -1);
	/** Start the notes and visual feedback for the current playback column. */
	void playColumn();
	/** Stop the current column's notes and visual feedback. */
	void endColumn();
	/** Finish the requested playback and continue its result sequence. */
	void finishPlayback();
	/** Record a correct pattern and begin completion or door-opening feedback. */
	void accept();
	/** Queue context-sensitive guidance for completing the pattern. */
	void hint();
	/** Start the inventory chest's closing sequence. */
	void closeInventory();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
