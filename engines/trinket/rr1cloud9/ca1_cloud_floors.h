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

#ifndef TRINKET_RR1CLOUD9_CA1_CLOUD_FLOORS_H
#define TRINKET_RR1CLOUD9_CA1_CLOUD_FLOORS_H

#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** CA1: fill word gaps before a cloud brick reaches each floor's checkpoint. */
class CloudFloors {
public:
	/** Connect the word-floor construction activity to its page services. */
	explicit CloudFloors(PageScene &host);
	/** Release the word-floor construction presentation state. */
	~CloudFloors();
	/** Load the word-floor construction resources and initialize the current round. */
	bool enter();
	/** Advance the word-floor construction presentation to its next ready stage. */
	void advance(uint32 time);
	/** Advance word-floor construction animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Draw the word-floor construction activity's additional graphics and text. */
	void draw(OM::Canvas &canvas);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Handle activity pointer or keyboard input and report whether it was consumed. */
	bool event(const Common::Event &event);
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _phase != kTravelling || _correctRoute || 0 <= _dragged; }
	/** Return whether activity cursor state overrides the default arrow. */
	bool hasCustomCursor() const { return 0 < _cursor; }
	/** Pause or unpause activity feedback and sprite playback. */
	void pause(bool paused);
	/** Resume activity feedback and restore its animation timing. */
	void resume(uint32 elapsed) {
		_nextTick += elapsed;
		_idle += elapsed;
	}

private:
	/** Format for script names or values: %s.%d. */
	static constexpr const char *kNameSignedIndexFormat = "%s.%d";
	/** Format for script names or values: .%u. */
	static constexpr const char *kUnsignedIndexFormat = ".%u";
	/** Script property name for worker. */
	static constexpr const char *kPropertyWorker = "worker";
	/** Authored data key for CWSet ID. */
	static constexpr const char *kCWSetIDKey = "kCWSetID";
	/** Authored data key for CWZ. */
	static constexpr const char *kCWZKey = "kCWZ";
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
	/** Authored data key for CWLips ID. */
	static constexpr const char *kCWLipsIDKey = "kCWLipsID";
	/** Authored data key for CWBody ID. */
	static constexpr const char *kCWBodyIDKey = "kCWBodyID";
	/** Script method name for add Fidget Group. */
	static constexpr const char *kMethodAddFidgetGroup = "addFidgetGroup";
	/** Script method name for add Fidget. */
	static constexpr const char *kMethodAddFidget = "addFidget";
	/** Authored data key for CWFidg ID.%d.%d. */
	static constexpr const char *kCWFidgIDSignedIndexSignedIndexKey = "kCWFidgID.%d.%d";
	/** Authored data key for CA1 Words ID. */
	static constexpr const char *kCA1WordsIDKey = "kCA1WordsID";
	/** Authored data key for Tile Box ID. */
	static constexpr const char *kTileBoxIDKey = "kTileBoxID";
	/** Script property name for center. */
	static constexpr const char *kPropertyCenter = "center";
	/** Script property name for color. */
	static constexpr const char *kPropertyColor = "color";
	/** Script identifier for tEXT CA1 WTILES. */
	static constexpr const char *kTEXTCA1WTILESName = "TEXT_CA1WTILES";
	/** Script identifier for tEXT CA1 WORDS. */
	static constexpr const char *kTEXTCA1WORDSName = "TEXT_CA1WORDS";
	/** World-state key for r1 CA1 Level. */
	static constexpr const char *kR1CA1LevelKey = "R1CA1Level";
	/** World-state key for r1 CA1 Dataset. */
	static constexpr const char *kR1CA1DatasetKey = "R1CA1Dataset";
	/** World-state key for r1 CA1 Set. */
	static constexpr const char *kR1CA1SetKey = "R1CA1Set";
	/** Authored data key for Num Sets. */
	static constexpr const char *kNumSetsKey = "kNumSets";
	/** Authored data key for Max Data Sets. */
	static constexpr const char *kMaxDataSetsKey = "kMaxDataSets";
	/** World-state key for r1 CA1 State. */
	static constexpr const char *kR1CA1StateKey = "R1CA1State";
	/** World-state key for r1 CA1 Round. */
	static constexpr const char *kR1CA1RoundKey = "R1CA1Round";
	/** World-state key for r1 CA1 Ladder Here. */
	static constexpr const char *kR1CA1LadderHereKey = "R1CA1LadderHere";
	/** Authored data key for Cloud Hilite ID. */
	static constexpr const char *kCloudHiliteIDKey = "kCloudHiliteID";
	/** Authored data key for Belt1 ID. */
	static constexpr const char *kBelt1IDKey = "kBelt1ID";
	/** Authored data key for Mask1 ID.%d. */
	static constexpr const char *kMask1IDSignedIndexKey = "kMask1ID.%d";
	/** Authored data key for Cloud Piece ID.%d. */
	static constexpr const char *kCloudPieceIDSignedIndexKey = "kCloudPieceID.%d";
	/** Authored data key for Word Tile ID. */
	static constexpr const char *kWordTileIDKey = "kWordTileID";
	/** Authored data key for Brick ID. */
	static constexpr const char *kBrickIDKey = "kBrickID";
	/** Authored data key for Cloud Brick Z. */
	static constexpr const char *kCloudBrickZKey = "kCloudBrickZ";
	/** Authored data key for Reach ID.%d. */
	static constexpr const char *kReachIDSignedIndexKey = "kReachID.%d";
	/** Script identifier for cloud Brick X. */
	static constexpr const char *kCloudBrickXName = "cloudBrickX";
	/** Script identifier for cloud Brick Y. */
	static constexpr const char *kCloudBrickYName = "cloudBrickY";
	/** Authored data key for Num Elevators. */
	static constexpr const char *kNumElevatorsKey = "kNumElevators";
	/** Authored data key for Elevator0 ID. */
	static constexpr const char *kElevator0IDKey = "kElevator0ID";
	/** Authored data key for Catcher ID. */
	static constexpr const char *kCatcherIDKey = "kCatcherID";
	/** Authored data key for Ladder ID. */
	static constexpr const char *kLadderIDKey = "kLadderID";
	/** Authored data key for Marble ID. */
	static constexpr const char *kMarbleIDKey = "kMarbleID";
	/** Authored data key for Down Ladder ID. */
	static constexpr const char *kDownLadderIDKey = "kDownLadderID";
	/** Authored data key for Toss ID. */
	static constexpr const char *kTossIDKey = "kTossID";
	/** Authored data key for Marble X. */
	static constexpr const char *kMarbleXKey = "kMarbleX";
	/** Authored data key for Marble Y. */
	static constexpr const char *kMarbleYKey = "kMarbleY";
	/** Authored data key for PEnter Spch ID.%d. */
	static constexpr const char *kPEnterSpchIDSignedIndexKey = "kPEnterSpchID.%d";
	/** Authored data key for Enter Char ID. */
	static constexpr const char *kEnterCharIDKey = "kEnterCharID";
	/** Authored data key for Enter Spch ID. */
	static constexpr const char *kEnterSpchIDKey = "kEnterSpchID";
	/** Authored data key for Max Entry Spchs. */
	static constexpr const char *kMaxEntrySpchsKey = "kMaxEntrySpchs";
	/** Authored data key for Re Enter Spch IDs.1.%d. */
	static constexpr const char *kReEnterSpchIDs1SignedIndexKey = "kReEnterSpchIDs.1.%d";
	/** World-state key for r1 CA1 Reentry. */
	static constexpr const char *kR1CA1ReentryKey = "R1CA1Reentry";
	/** Authored data key for Re Enter Spch IDs.2.1. */
	static constexpr const char *kReEnterSpchIDs21Key = "kReEnterSpchIDs.2.1";
	/** Script property name for data Set/Level %d/Word List %d/Set %d. */
	static constexpr const char *kPropertyDataSetLevelSignedIndexWordListSignedIndexSetSignedIndex = "DataSet/Level_%d/WordList_%d/Set_%d";
	/** Authored data key for Num Words. */
	static constexpr const char *kNumWordsKey = "kNumWords";
	/** Authored data key for Target X.%d.%d. */
	static constexpr const char *kTargetXSignedIndexSignedIndexKey = "kTargetX.%d.%d";
	/** Authored data key for Target Y. */
	static constexpr const char *kTargetYKey = "kTargetY";
	/** Authored data key for Answer Start X. */
	static constexpr const char *kAnswerStartXKey = "kAnswerStartX";
	/** Authored data key for Answer Y. */
	static constexpr const char *kAnswerYKey = "kAnswerY";
	/** World-state key for r1 CA1 Streak Kind. */
	static constexpr const char *kR1CA1StreakKindKey = "R1CA1StreakKind";
	/** World-state key for r1 CA1 Streak. */
	static constexpr const char *kR1CA1StreakKey = "R1CA1Streak";
	/** World-state key for r1 CA1. */
	static constexpr const char *kR1CA1Key = "R1CA1";
	/** World-state key for r1 CA1 Fixed Level. */
	static constexpr const char *kR1CA1FixedLevelKey = "R1CA1FixedLevel";
	/** Authored data key for End Pt. */
	static constexpr const char *kEndPtKey = "kEndPt";
	/** Format for script names or values: x.%d.%d. */
	static constexpr const char *kXSignedIndexSignedIndexFormat = "x.%d.%d";
	/** Format for script names or values: y.%d.%d. */
	static constexpr const char *kYSignedIndexSignedIndexFormat = "y.%d.%d";
	/** Script identifier for x.3.6. */
	static constexpr const char *kX36Name = "x.3.6";
	/** Script identifier for y.3.6. */
	static constexpr const char *kY36Name = "y.3.6";
	/** Authored data key for Path DX. */
	static constexpr const char *kPathDXKey = "kPathDX";
	/** Authored data key for Path DY. */
	static constexpr const char *kPathDYKey = "kPathDY";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Authored data key for Ladder HSW. */
	static constexpr const char *kLadderHSWKey = "kLadderHSW";
	/** Authored data key for Ladder HSH. */
	static constexpr const char *kLadderHSHKey = "kLadderHSH";
	/** Authored data key for Correct Sfx ID. */
	static constexpr const char *kCorrectSfxIDKey = "kCorrectSfxID";
	/** Authored data key for Congrats Char ID.%d. */
	static constexpr const char *kCongratsCharIDSignedIndexKey = "kCongratsCharID.%d";
	/** Authored data key for Congrats Spch ID. */
	static constexpr const char *kCongratsSpchIDKey = "kCongratsSpchID";
	/** Authored data key for Max Oops Speeches. */
	static constexpr const char *kMaxOopsSpeechesKey = "kMaxOopsSpeeches";
	/** Authored data key for Oops Char ID.%d. */
	static constexpr const char *kOopsCharIDSignedIndexKey = "kOopsCharID.%d";
	/** Authored data key for Oops Spch ID. */
	static constexpr const char *kOopsSpchIDKey = "kOopsSpchID";
	/** Authored data key for CA1 SFX10 ID.%d. */
	static constexpr const char *kCA1SFX10IDSignedIndexKey = "kCA1SFX10ID.%d";
	/** Script property name for visible. */
	static constexpr const char *kPropertyVisible = "visible";
	/** Authored data key for Reach Sfx ID.%d. */
	static constexpr const char *kReachSfxIDSignedIndexKey = "kReachSfxID.%d";
	/** Authored data key for Already Spch ID.1. */
	static constexpr const char *kAlreadySpchID1Key = "kAlreadySpchID.1";
	/** Authored data key for Already Spch ID.2. */
	static constexpr const char *kAlreadySpchID2Key = "kAlreadySpchID.2";
	/** Authored data key for Max Marble Speeches. */
	static constexpr const char *kMaxMarbleSpeechesKey = "kMaxMarbleSpeeches";
	/** Authored data key for Have Marble Spch ID.%d. */
	static constexpr const char *kHaveMarbleSpchIDSignedIndexKey = "kHaveMarbleSpchID.%d";
	/** Authored data key for Drop Sfx ID. */
	static constexpr const char *kDropSfxIDKey = "kDropSfxID";
	/** Authored data key for Go Home Sfx ID. */
	static constexpr const char *kGoHomeSfxIDKey = "kGoHomeSfxID";
	/** Format for script names or values: item:%d. */
	static constexpr const char *kItemSignedIndexFormat = "item:%d";
	/** Authored data key for COMPick Up Sfx ID. */
	static constexpr const char *kCOMPickUpSfxIDKey = "kCOMPickUpSfxID";
	/** World-state key for r1 CA1 Marble Click. */
	static constexpr const char *kR1CA1MarbleClickKey = "R1CA1MarbleClick";
	/** Authored data key for COMSome Use ID. */
	static constexpr const char *kCOMSomeUseIDKey = "kCOMSomeUseID";
	/** Authored data key for INV1 Marble5. */
	static constexpr const char *kINV1Marble5Key = "kINV1Marble5";
	/** Authored data key for Ladder Char ID.1. */
	static constexpr const char *kLadderCharID1Key = "kLadderCharID.1";
	/** Authored data key for Ladder Spch ID.1. */
	static constexpr const char *kLadderSpchID1Key = "kLadderSpchID.1";
	/** Authored data key for Ladder Spch ID.1.%d. */
	static constexpr const char *kLadderSpchID1SignedIndexKey = "kLadderSpchID.1.%d";
	/** Authored data key for Ladder Char ID.2. */
	static constexpr const char *kLadderCharID2Key = "kLadderCharID.2";
	/** Authored data key for Ladder Spch ID.2. */
	static constexpr const char *kLadderSpchID2Key = "kLadderSpchID.2";
	/** Authored data key for INV1 Ladder. */
	static constexpr const char *kINV1LadderKey = "kINV1Ladder";
	/** Authored data key for Pick Up Sfx ID. */
	static constexpr const char *kPickUpSfxIDKey = "kPickUpSfxID";
	/** Authored data key for CWSpch ID.%d. */
	static constexpr const char *kCWSpchIDSignedIndexKey = "kCWSpchID.%d";
	/** Authored data key for Paige Help ID. */
	static constexpr const char *kPaigeHelpIDKey = "kPaigeHelpID";
	/** Authored data key for Max Main Char Index. */
	static constexpr const char *kMaxMainCharIndexKey = "kMaxMainCharIndex";
	/** Format for script names or values: .%d.%d. */
	static constexpr const char *kSignedIndexSignedIndexFormat = ".%d.%d";
	/** Authored data key for Main Char ID. */
	static constexpr const char *kMainCharIDKey = "kMainCharID";
	/** Authored data key for Main Spch ID. */
	static constexpr const char *kMainSpchIDKey = "kMainSpchID";
	/** Authored data key for Done Spch ID. */
	static constexpr const char *kDoneSpchIDKey = "kDoneSpchID";
	/** Authored data key for CA1 SFX07 ID. */
	static constexpr const char *kCA1SFX07IDKey = "kCA1SFX07ID";
	/** Authored data key for Thanks Spch ID.1. */
	static constexpr const char *kThanksSpchID1Key = "kThanksSpchID.1";
	/** Authored data key for Thanks Spch ID.2. */
	static constexpr const char *kThanksSpchID2Key = "kThanksSpchID.2";
	/** Authored data key for Marble5 ID. */
	static constexpr const char *kMarble5IDKey = "kMarble5ID";
	/** Authored data key for Max Idle Index. */
	static constexpr const char *kMaxIdleIndexKey = "kMaxIdleIndex";
	/** Authored data key for Idle Char ID.%d. */
	static constexpr const char *kIdleCharIDSignedIndexKey = "kIdleCharID.%d";
	/** Authored data key for Idle Spch ID. */
	static constexpr const char *kIdleSpchIDKey = "kIdleSpchID";
	/** Script property name for text. */
	static constexpr const char *kPropertyText = "text";
	/** Script property name for x. */
	static constexpr const char *kPropertyX = "x";
	/** Script property name for y. */
	static constexpr const char *kPropertyY = "y";
	/** Authored data key for Text Offset Y. */
	static constexpr const char *kTextOffsetYKey = "kTextOffsetY";

	/** Text resource path indexed by resource ID. */
	static constexpr const char *kTextFormat = "r1ca1/%d.txt";
	/** Digit animation path indexed by resource ID. */
	static constexpr const char *kDigitFormat = "common/%u.ao";
	/** Font for cloud-floor answers. */
	static constexpr const char *kFont = "common/1024.nfnt";
	/** Cloud-floor puzzle datasets. */
	static constexpr const char *kDataXML = "r1ca1.xml";

	enum Phase {
		kIntroduction,
		kTargetSpeech,
		kTravelling,
		kWrongWord,
		kWrongTarget,
		kCorrect,
		kReach,
		kDoneSpeech,
		kDescend,
		kRewardSpeech,
		kToss,
		kMarbleFlash,
		kNext,
	};
	struct Tile {
		/** Sprite displaying the word or picture tile. */
		Shell::Sprite *sprite = nullptr;
		/** Text object displaying the tile's word when needed. */
		Shell::TextObject *text = nullptr;
		/** Authored word or picture value represented by the tile. */
		Common::String value;
		/** Game-pixel position restored when the tile returns to supply. */
		Common::Point home;
		/** Destination slot index, or minus one while the tile is unplaced. */
		int slot = -1;
	};
	struct Slot {
		/** Visual tile state displayed at this construction slot. */
		Tile tile;
		/** Word or picture value required by the slot. */
		Common::String expected;
		/** Supply-answer index currently assigned to the slot, or minus one. */
		int answer = -1;
		/** Whether an accepted tile can no longer be removed. */
		bool locked = false;
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Resource-path resolver for the activity's current page. */
	Shell::ResourcePaths _paths;
	/** Parsed XML data used to construct the current activity round. */
	OM::XMLParser _data;
	/** FontMan-owned font used for activity text and labels. */
	const Graphics::Font *_font = nullptr;
	/** Picture values available in the current dataset. */
	Common::StringArray _pictures;
	/** Word values available in the current dataset. */
	Common::StringArray _words;
	/** Required tile slots indexed by construction floor. */
	Common::Array<Slot> _slots[3];
	/** Draggable answer tiles in the current supply. */
	Common::Array<Tile> _answers;
	/** Target word or picture value for each construction floor. */
	Common::String _targets[3];
	/** Conveyor sprites for the three construction floors. */
	Shell::Sprite *_belts[3] = {};
	/** Sprites masking conveyor and tile edges. */
	Shell::Sprite *_masks[3] = {};
	/** Construction-piece sprites indexed by floor. */
	Shell::Sprite *_pieces[3] = {};
	/** Picture-clue animations indexed by floor. */
	Shell::Sprite *_picturesAO[3] = {};
	/** Brick-stack sprites indexed by floor. */
	Shell::Sprite *_bricks[3] = {};
	/** Worker reaching animations indexed by floor. */
	Shell::Sprite *_reaches[3] = {};
	/** Brick sprite used by the current transfer animation. */
	Shell::Sprite *_brick = nullptr;
	/** Sprite marking the current tile destination. */
	Shell::Sprite *_highlight = nullptr;
	/** Ladder sprite used by the worker's floor transitions. */
	Shell::Sprite *_ladder = nullptr;
	/** Marble character sprite used in activity feedback. */
	Shell::Sprite *_marble = nullptr;
	/** Animation used when the worker descends. */
	Shell::Sprite *_descend = nullptr;
	/** Animation used to toss the current brick. */
	Shell::Sprite *_toss = nullptr;
	/** Animations to resume after activity playback is unpaused. */
	Common::Array<Shell::Sprite *> _pausedSprites;
	/** Game-pixel checkpoints along the worker's route. */
	Common::Point _points[5];
	/** Pointer displacement from the dragged piece's anchor, in pixels. */
	Common::Point _dragOffset;
	/** Decoded cursor images for tile and construction interaction. */
	Graphics::Surface _cursors[5];
	/** Most recent pointer position in game pixels. */
	Common::Point _mouse;
	/** Current activity cursor index, or minus one before selection. */
	int _cursor = -1;
	/** Whether activity input and timed playback are paused. */
	bool _paused = false;
	/** Difficulty selected for the current activity round. */
	int _level = 1;
	/** Authored dataset selected for the current difficulty. */
	int _dataset = 1;
	/** Selected exercise set within the dataset. */
	int _set = 1;
	/** Construction floor currently being assembled. */
	int _floor = 1;
	/** Saved construction progress state for the current floor. */
	int _state = 1;
	/** Number of construction bricks delivered so far. */
	int _brickCount = 0;
	/** Starting checkpoint of the current worker movement. */
	int _from = 0;
	/** Destination checkpoint of the current worker movement. */
	int _to = 1;
	/** Final checkpoint of the current worker route. */
	int _end = 3;
	/** Index of the piece being dragged, or minus one when no piece is held. */
	int _dragged = -1;
	/** Incorrect tile placements recorded in the current round. */
	int _mistakes = 0;
	/** Next incorrect-action speech variant. */
	int _oops = 0;
	/** Worker click count used to select responses. */
	int _workerClicks = 0;
	/** Ladder click count used to select responses. */
	int _ladderClicks = 0;
	/** Main character click count used to select responses. */
	int _mainClicks = 0;
	/** Paige click count used to select responses. */
	int _paigeClicks = 0;
	/** Next Marble speech variant in the feedback sequence. */
	int _marbleSpeech = 0;
	/** Next idle speech variant. */
	int _idleIndex = 0;
	/** Whether the worker is moving along the selected route. */
	bool _moving = false;
	/** Whether the current construction sequence follows the accepted route. */
	bool _correctRoute = false;
	/** Whether the ladder is available at the worker's current position. */
	bool _ladderHere = true;
	/** Whether incorrect-action feedback alternates its response. */
	bool _alternateOops = true;
	/** Millisecond deadline for the next construction movement step. */
	uint32 _nextTick = 0;
	/** Millisecond timestamp used to schedule idle feedback. */
	uint32 _idle = 0;
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kIntroduction;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Read an authored numeric constant selected by an indexed name. */
	int indexed(const char *key, int index) const;
	/** Create an activity sprite from a named page resource at the supplied depth. */
	Shell::Sprite *sprite(const Common::String &key, int depth);
	/** Select the worker animation pose and report whether it loaded. */
	bool worker(int pose);
	/** Load a word or picture list from an XML resource. */
	bool loadList(int resource, Common::StringArray &list);
	/** Find a word in an authored list, returning minus one when absent. */
	static int index(const Common::StringArray &list, const Common::String &word);
	/** Return a sprite's game-pixel bounds, or an empty rectangle for null. */
	static Common::Rect bounds(Shell::Sprite *sprite);
	/** Create a word or picture tile at a position and depth. */
	bool createTile(Tile &tile, const Common::String &value, int length, Common::Point position, int depth);
	/** Remove the tile's sprite and text display. */
	void deleteTile(Tile &tile);
	/** Draw the tile's text or image content. */
	void drawTile(Tile &tile, OM::Canvas &canvas);
	/** Remove current answer tiles from the supply. */
	void clearAnswers();
	/** Remove tile content belonging to one construction floor. */
	void clearFloor(int floor);
	/** Play the authored spoken line for the selected actor. */
	void say(const char *actor, const Common::String &key);
	/** Play a numeric speech resource for the named actor. */
	void speech(const Common::String &actor, uint resource);
	/** Play a bounded authored sequence of actors and speech resources. */
	void speeches(const Common::String &actors, const Common::String &resources, uint count);
	/** Select the cursor matching the current tile or construction interaction. */
	void updateCursor();
	/** Restore the default arrow and release activity cursor state. */
	void releaseCursor();
	/** Play the spoken form of an authored word. */
	void sayWord(const Common::String &word);
	/** Play an effect identified by an authored sound key. */
	void sound(const Common::String &key);
	/** Write the activity's current saved state to the world. */
	void persist();
	/** Record a correct or incorrect construction response. */
	void assess(bool correct);
	/** Prepare and display the next word-floor construction round. */
	bool setup();
	/** Advance to the next construction target or completed activity stage. */
	void next();
	/** Build the worker's checkpoint route for the current floor. */
	void initPath();
	/** Begin moving the worker between two route checkpoints. */
	void segment(int from, int to);
	/** Save the activity's current checkpoint state. */
	void checkpoint();
	/** Reject the current tile placement and start corrective feedback. */
	void reject();
	/** Start the worker's falling feedback animation. */
	void fall();
	/** Complete the current construction sequence and update story progress. */
	void finish();
	/** Apply the dragged answer to a slot or return it to supply. */
	void drop();
	/** Return an answer tile to its recorded supply position. */
	void home(int answer);
	/** Record an activity reward or inventory item. */
	void collect(const char *item);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
