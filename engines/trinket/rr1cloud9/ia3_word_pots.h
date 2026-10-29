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

#ifndef TRINKET_RR1CLOUD9_IA3_WORD_POTS_H
#define TRINKET_RR1CLOUD9_IA3_WORD_POTS_H

#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** IA3: exchange word parts in three pots and pack the requested word. */
class WordPots {
public:
	/** Connect the word-pots activity to its page services. */
	explicit WordPots(PageScene &host);
	/** Release the word-pots presentation state. */
	~WordPots();
	/** Load the word-pots resources and initialize the current round. */
	bool enter();
	/** Advance the word-pots presentation to its next ready stage. */
	void advance(uint32 time);
	/** Advance word-pots animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Draw the word-pots activity's additional graphics and text. */
	void draw(OM::Canvas &canvas);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Handle activity pointer or keyboard input and report whether it was consumed. */
	bool event(const Common::Event &event);
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _phase != kReady || _dragging; }
	/** Interrupt the current spoken instruction or feedback. */
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
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Script property name for center. */
	static constexpr const char *kPropertyCenter = "center";
	/** Script property name for color. */
	static constexpr const char *kPropertyColor = "color";
	/** Script property name for text. */
	static constexpr const char *kPropertyText = "text";
	/** Script property name for x. */
	static constexpr const char *kPropertyX = "x";
	/** Script property name for y. */
	static constexpr const char *kPropertyY = "y";
	/** Script identifier for p. */
	static constexpr const char *kPName = "p";
	/** Script identifier for bloom. */
	static constexpr const char *kBloomName = "bloom";
	/** Authored data key for Word Name Spch ID. */
	static constexpr const char *kWordNameSpchIDKey = "kWordNameSpchID";
	/** Script identifier for tEXT IA3 WORDS. */
	static constexpr const char *kTEXTIA3WORDSName = "TEXT_IA3WORDS";
	/** Script identifier for tEXT IA3 WORDTYPES. */
	static constexpr const char *kTEXTIA3WORDTYPESName = "TEXT_IA3WORDTYPES";
	/** Script identifier for tEXT IA3 PICTUREWORDS. */
	static constexpr const char *kTEXTIA3PICTUREWORDSName = "TEXT_IA3PICTUREWORDS";
	/** Authored data key for Bloom. */
	static constexpr const char *kBloomKey = "kBloom";
	/** World-state key for r1 IA3 Level. */
	static constexpr const char *kR1IA3LevelKey = "R1IA3Level";
	/** World-state key for r1 IA3 Dataset. */
	static constexpr const char *kR1IA3DatasetKey = "R1IA3Dataset";
	/** Authored data key for Max Data Sets. */
	static constexpr const char *kMaxDataSetsKey = "kMaxDataSets";
	/** World-state key for r1 IA3 Set. */
	static constexpr const char *kR1IA3SetKey = "R1IA3Set";
	/** World-state key for r1 IA3 State. */
	static constexpr const char *kR1IA3StateKey = "R1IA3State";
	/** World-state key for r1 IA3 Round. */
	static constexpr const char *kR1IA3RoundKey = "R1IA3Round";
	/** World-state key for r1 IA3 Wagon Here. */
	static constexpr const char *kR1IA3WagonHereKey = "R1IA3WagonHere";
	/** World-state key for r1 IA3 Show Marble. */
	static constexpr const char *kR1IA3ShowMarbleKey = "R1IA3ShowMarble";
	/** Authored data key for Items ID. */
	static constexpr const char *kItemsIDKey = "kItemsID";
	/** Authored data key for Target Box ID. */
	static constexpr const char *kTargetBoxIDKey = "kTargetBoxID";
	/** Authored data key for Target Mask ID. */
	static constexpr const char *kTargetMaskIDKey = "kTargetMaskID";
	/** Authored data key for Grow ID. */
	static constexpr const char *kGrowIDKey = "kGrowID";
	/** Authored data key for Shrink ID. */
	static constexpr const char *kShrinkIDKey = "kShrinkID";
	/** Authored data key for Arrow ID. */
	static constexpr const char *kArrowIDKey = "kArrowID";
	/** Authored data key for Packages ID. */
	static constexpr const char *kPackagesIDKey = "kPackagesID";
	/** Authored data key for Red Wagon ID. */
	static constexpr const char *kRedWagonIDKey = "kRedWagonID";
	/** Authored data key for Red Wagon Mask ID. */
	static constexpr const char *kRedWagonMaskIDKey = "kRedWagonMaskID";
	/** Authored data key for Old Wagon ID. */
	static constexpr const char *kOldWagonIDKey = "kOldWagonID";
	/** Authored data key for Old Wagon Mask ID. */
	static constexpr const char *kOldWagonMaskIDKey = "kOldWagonMaskID";
	/** Authored data key for Marble ID. */
	static constexpr const char *kMarbleIDKey = "kMarbleID";
	/** Authored data key for Pot IDs.%d. */
	static constexpr const char *kPotIDsSignedIndexKey = "kPotIDs.%d";
	/** Authored data key for Marble X. */
	static constexpr const char *kMarbleXKey = "kMarbleX";
	/** Authored data key for Marble Y. */
	static constexpr const char *kMarbleYKey = "kMarbleY";
	/** Authored data key for Instr Spch IDs.6. */
	static constexpr const char *kInstrSpchIDs6Key = "kInstrSpchIDs.6";
	/** Authored data key for Instr Spch IDs.8. */
	static constexpr const char *kInstrSpchIDs8Key = "kInstrSpchIDs.8";
	/** Authored data key for Instr Spch IDs.5.2. */
	static constexpr const char *kInstrSpchIDs52Key = "kInstrSpchIDs.5.2";
	/** Authored data key for Instr Spch IDs.7. */
	static constexpr const char *kInstrSpchIDs7Key = "kInstrSpchIDs.7";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Authored data key for Enter Spch IDs. */
	static constexpr const char *kEnterSpchIDsKey = "kEnterSpchIDs";
	/** Script identifier for rb. */
	static constexpr const char *kRbName = "rb";
	/** Authored data key for Re Enter Spch IDs.3. */
	static constexpr const char *kReEnterSpchIDs3Key = "kReEnterSpchIDs.3";
	/** Authored data key for Re Enter Spch IDs.%d. */
	static constexpr const char *kReEnterSpchIDsSignedIndexKey = "kReEnterSpchIDs.%d";
	/** Authored data key for Next Spch IDs.1. */
	static constexpr const char *kNextSpchIDs1Key = "kNextSpchIDs.1";
	/** Script property name for data Set/Level %d/Word List %d/Set %d. */
	static constexpr const char *kPropertyDataSetLevelSignedIndexWordListSignedIndexSetSignedIndex = "DataSet/Level_%d/WordList_%d/Set_%d";
	/** Authored data key for Package X. */
	static constexpr const char *kPackageXKey = "kPackageX";
	/** Authored data key for Package Y. */
	static constexpr const char *kPackageYKey = "kPackageY";
	/** Authored data key for Red Wagon Move Offset. */
	static constexpr const char *kRedWagonMoveOffsetKey = "kRedWagonMoveOffset";
	/** Authored data key for Target Answer X. */
	static constexpr const char *kTargetAnswerXKey = "kTargetAnswerX";
	/** Authored data key for Target Answer Y. */
	static constexpr const char *kTargetAnswerYKey = "kTargetAnswerY";
	/** Script identifier for brbbrbsb. */
	static constexpr const char *kBrbbrbsbName = "brbbrbsb";
	/** Authored data key for Instr Spch IDs.1. */
	static constexpr const char *kInstrSpchIDs1Key = "kInstrSpchIDs.1";
	/** Authored data key for Container X. */
	static constexpr const char *kContainerXKey = "kContainerX";
	/** Authored data key for Container Y. */
	static constexpr const char *kContainerYKey = "kContainerY";
	/** Authored data key for Container W. */
	static constexpr const char *kContainerWKey = "kContainerW";
	/** Authored data key for Container H. */
	static constexpr const char *kContainerHKey = "kContainerH";
	/** Authored data key for Old Word X. */
	static constexpr const char *kOldWordXKey = "kOldWordX";
	/** Authored data key for Old Word Y. */
	static constexpr const char *kOldWordYKey = "kOldWordY";
	/** Authored data key for New Word X. */
	static constexpr const char *kNewWordXKey = "kNewWordX";
	/** Authored data key for New Word Y. */
	static constexpr const char *kNewWordYKey = "kNewWordY";
	/** Authored data key for Drop Sfx ID. */
	static constexpr const char *kDropSfxIDKey = "kDropSfxID";
	/** Script identifier for bbr. */
	static constexpr const char *kBbrName = "bbr";
	/** Authored data key for Instr Spch IDs.3. */
	static constexpr const char *kInstrSpchIDs3Key = "kInstrSpchIDs.3";
	/** Script identifier for bss. */
	static constexpr const char *kBssName = "bss";
	/** Script identifier for bb. */
	static constexpr const char *kBbName = "bb";
	/** Script identifier for bbbbsb. */
	static constexpr const char *kBbbbsbName = "bbbbsb";
	/** Authored data key for Instr Spch IDs.5. */
	static constexpr const char *kInstrSpchIDs5Key = "kInstrSpchIDs.5";
	/** Authored data key for Go Home Sfx ID. */
	static constexpr const char *kGoHomeSfxIDKey = "kGoHomeSfxID";
	/** Authored data key for Wrong Sfx ID. */
	static constexpr const char *kWrongSfxIDKey = "kWrongSfxID";
	/** Authored data key for Incorrect Spch IDs. */
	static constexpr const char *kIncorrectSpchIDsKey = "kIncorrectSpchIDs";
	/** Script identifier for br. */
	static constexpr const char *kBrName = "br";
	/** Authored data key for Incorrect Spch IDs.5. */
	static constexpr const char *kIncorrectSpchIDs5Key = "kIncorrectSpchIDs.5";
	/** Authored data key for Optimum Transformations. */
	static constexpr const char *kOptimumTransformationsKey = "kOptimumTransformations";
	/** World-state key for r1 IA3 Streak Kind. */
	static constexpr const char *kR1IA3StreakKindKey = "R1IA3StreakKind";
	/** World-state key for r1 IA3 Streak. */
	static constexpr const char *kR1IA3StreakKey = "R1IA3Streak";
	/** Authored data key for Autolevel Down Count. */
	static constexpr const char *kAutolevelDownCountKey = "kAutolevelDownCount";
	/** Authored data key for Autolevel Up Count. */
	static constexpr const char *kAutolevelUpCountKey = "kAutolevelUpCount";
	/** World-state key for r1 IA3. */
	static constexpr const char *kR1IA3Key = "R1IA3";
	/** World-state key for r1 IA3 Fixed Level. */
	static constexpr const char *kR1IA3FixedLevelKey = "R1IA3FixedLevel";
	/** Authored data key for Correct Sfx ID. */
	static constexpr const char *kCorrectSfxIDKey = "kCorrectSfxID";
	/** Authored data key for Correct Spch IDs.%d. */
	static constexpr const char *kCorrectSpchIDsSignedIndexKey = "kCorrectSpchIDs.%d";
	/** Authored data key for Answer Text Offset Y. */
	static constexpr const char *kAnswerTextOffsetYKey = "kAnswerTextOffsetY";
	/** Authored data key for Red Wagon Sfx ID. */
	static constexpr const char *kRedWagonSfxIDKey = "kRedWagonSfxID";
	/** Authored data key for Target Move Offset. */
	static constexpr const char *kTargetMoveOffsetKey = "kTargetMoveOffset";
	/** Authored data key for Old Wagon Move Offset. */
	static constexpr const char *kOldWagonMoveOffsetKey = "kOldWagonMoveOffset";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Authored data key for Help Spch IDs.1.%d. */
	static constexpr const char *kHelpSpchIDs1SignedIndexKey = "kHelpSpchIDs.1.%d";
	/** Authored data key for Instr Spch IDs.2. */
	static constexpr const char *kInstrSpchIDs2Key = "kInstrSpchIDs.2";
	/** Authored data key for Instr Spch IDs.4. */
	static constexpr const char *kInstrSpchIDs4Key = "kInstrSpchIDs.4";
	/** Authored data key for Instr Spch IDs.%d. */
	static constexpr const char *kInstrSpchIDsSignedIndexKey = "kInstrSpchIDs.%d";
	/** Script identifier for sbr. */
	static constexpr const char *kSbrName = "sbr";
	/** Authored data key for Instr Spch IDs.10. */
	static constexpr const char *kInstrSpchIDs10Key = "kInstrSpchIDs.10";
	/** Authored data key for Instr Spch IDs.11. */
	static constexpr const char *kInstrSpchIDs11Key = "kInstrSpchIDs.11";
	/** Authored data key for Pick Up Sfx ID. */
	static constexpr const char *kPickUpSfxIDKey = "kPickUpSfxID";
	/** Format for script names or values: item:%d. */
	static constexpr const char *kItemSignedIndexFormat = "item:%d";
	/** Authored data key for INV1 Marble1. */
	static constexpr const char *kINV1Marble1Key = "kINV1Marble1";
	/** Authored data key for INV1 Wagon. */
	static constexpr const char *kINV1WagonKey = "kINV1Wagon";
	/** Authored data key for Wagon Spch IDs.2. */
	static constexpr const char *kWagonSpchIDs2Key = "kWagonSpchIDs.2";
	/** Script identifier for sb. */
	static constexpr const char *kSbName = "sb";
	/** Authored data key for Wagon Spch IDs.1. */
	static constexpr const char *kWagonSpchIDs1Key = "kWagonSpchIDs.1";
	/** Authored data key for Wagon Spch IDs.1.3. */
	static constexpr const char *kWagonSpchIDs13Key = "kWagonSpchIDs.1.3";
	/** Script identifier for sr. */
	static constexpr const char *kSrName = "sr";
	/** Authored data key for Main Spch IDs.%d. */
	static constexpr const char *kMainSpchIDsSignedIndexKey = "kMainSpchIDs.%d";
	/** Authored data key for Pot%d Spch ID. */
	static constexpr const char *kPotSignedIndexSpchIDKey = "kPot%dSpchID";
	/** Authored data key for Pot Spch ID. */
	static constexpr const char *kPotSpchIDKey = "kPotSpchID";
	/** Authored data key for Package Open ID. */
	static constexpr const char *kPackageOpenIDKey = "kPackageOpenID";
	/** Authored data key for Instr Spch IDs.1.9. */
	static constexpr const char *kInstrSpchIDs19Key = "kInstrSpchIDs.1.9";
	/** Authored data key for Correct Spch IDs.5. */
	static constexpr const char *kCorrectSpchIDs5Key = "kCorrectSpchIDs.5";
	/** Authored data key for Correct Spch IDs.9. */
	static constexpr const char *kCorrectSpchIDs9Key = "kCorrectSpchIDs.9";
	/** Authored data key for Correct Spch IDs.8. */
	static constexpr const char *kCorrectSpchIDs8Key = "kCorrectSpchIDs.8";
	/** Authored data key for Correct Spch IDs.10. */
	static constexpr const char *kCorrectSpchIDs10Key = "kCorrectSpchIDs.10";
	/** Authored data key for Marble1 ID. */
	static constexpr const char *kMarble1IDKey = "kMarble1ID";
	/** Authored data key for Max Correct Speech. */
	static constexpr const char *kMaxCorrectSpeechKey = "kMaxCorrectSpeech";
	/** Authored data key for Next Spch IDs.%d. */
	static constexpr const char *kNextSpchIDsSignedIndexKey = "kNextSpchIDs.%d";
	/** Authored data key for Idle Spch IDs.1. */
	static constexpr const char *kIdleSpchIDs1Key = "kIdleSpchIDs.1";
	/** Script identifier for rs. */
	static constexpr const char *kRsName = "rs";
	/** Authored data key for Idle Spch IDs.2. */
	static constexpr const char *kIdleSpchIDs2Key = "kIdleSpchIDs.2";

	/** Word-list path indexed by resource ID. */
	static constexpr const char *kTextFormat = "r1ia3/%d.txt";
	/** Font for word-pot labels. */
	static constexpr const char *kFont = "common/1016.nfnt";
	/** Font for words displayed on pots. */
	static constexpr const char *kPotFont = "common/1014.nfnt";
	/** Word-pot puzzle datasets. */
	static constexpr const char *kDataXML = "r1ia3.xml";

	enum Phase {
		kReady,
		kIntroduction,
		kUnpacking,
		kGrowing,
		kCorrect,
		kTargetOut,
		kBatchSpeech,
		kRedWagonOut,
		kRedWagonSpeech,
		kMarbleSpeech,
		kMarbleFlash,
		kWagonOut,
		kTargetIn,
		kWagonIn,
		kNextSpeech,
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Resource-path resolver for the activity's current page. */
	Shell::ResourcePaths _paths;
	/** Parsed XML data used to construct the current activity round. */
	OM::XMLParser _data;
	/** Font used for activity text and labels. */
	Common::SharedPtr<OM::NFNTFont> _font;
	/** Font used for word labels beside the transformation pots. */
	Common::SharedPtr<OM::NFNTFont> _potFont;
	/** Word list used to resolve spoken word resources. */
	Common::StringArray _words;
	/** Word categories used to locate the corresponding pictures. */
	Common::StringArray _types;
	/** Picture names matched to words and their categories. */
	Common::StringArray _pictures;
	/** Word currently formed by the selected parts. */
	Common::String _word;
	/** Word requested by the current exercise. */
	Common::String _targetWord;
	/** Two alternative word parts for each of the three pots. */
	Common::String _parts[2][3];
	/** Whether each pot currently selects its alternative word part. */
	bool _alternate[3] = {};
	/** Sprites for the three word-part transformation pots. */
	Shell::Sprite *_pots[3] = {};
	/** Picture of the word currently formed by the player. */
	Shell::Sprite *_user = nullptr;
	/** Picture of the requested word. */
	Shell::Sprite *_target = nullptr;
	/** Container surrounding the target answer. */
	Shell::Sprite *_box = nullptr;
	/** Mask used to cover the target picture at the harder difficulty. */
	Shell::Sprite *_mask = nullptr;
	/** Sprite displaying the growing transformation effect. */
	Shell::Sprite *_grow = nullptr;
	/** Sprite displaying the shrinking transformation effect. */
	Shell::Sprite *_shrink = nullptr;
	/** Arrow guiding the current transformation. */
	Shell::Sprite *_arrow = nullptr;
	/** Sprite displaying the remaining word packages. */
	Shell::Sprite *_packages = nullptr;
	/** Red wagon available for collection. */
	Shell::Sprite *_redWagon = nullptr;
	/** Foreground mask for the red wagon. */
	Shell::Sprite *_redMask = nullptr;
	/** Wagon used to present later exercise packages. */
	Shell::Sprite *_oldWagon = nullptr;
	/** Foreground mask for the later wagon. */
	Shell::Sprite *_oldMask = nullptr;
	/** Collectible marble displayed by this activity. */
	Shell::Sprite *_marble = nullptr;
	/** Text displaying the player's current word. */
	Shell::TextObject *_userText = nullptr;
	/** Text displaying the requested word. */
	Shell::TextObject *_targetText = nullptr;
	/** Word label shown before a pot transformation. */
	Shell::TextObject *_oldText = nullptr;
	/** Word label shown after a pot transformation. */
	Shell::TextObject *_newText = nullptr;
	/** Animations to resume after activity playback is unpaused. */
	Common::Array<Shell::Sprite *> _pausedSprites;
	/** Game-pixel return position of the current answer picture. */
	Common::Point _home;
	/** Pointer displacement from the dragged piece's anchor, in pixels. */
	Common::Point _dragOffset;
	/** Original game-pixel position of the growing effect. */
	Common::Point _growHome;
	/** Original game-pixel position of the shrinking effect. */
	Common::Point _shrinkHome;
	/** Original game-pixel position of the guidance arrow. */
	Common::Point _arrowHome;
	/** Original game-pixel position of the target container. */
	Common::Point _boxHome;
	/** Original game-pixel position of the target mask. */
	Common::Point _maskHome;
	/** Original game-pixel position of the red wagon. */
	Common::Point _wagonHome;
	/** Original game-pixel position of the later wagon. */
	Common::Point _oldWagonHome;
	/** Original game-pixel position of the later wagon's mask. */
	Common::Point _oldMaskHome;
	/** Original game-pixel position of the package sprite. */
	Common::Point _packagesHome;
	/** Untransformed answer picture's width and height, in pixels. */
	Common::Point _answerSize;
	/** Vertical pixel offset applied to the answer during pot transformation. */
	static constexpr int kAnswerGrowOffset = -140;
	/** Difficulty selected for the current activity round. */
	int _level = 1;
	/** Authored dataset selected for the current difficulty. */
	int _dataset = 1;
	/** Exercise set selected within the word dataset. */
	int _set = 1;
	/** Saved progress through unpacking, practice transformations and target exercises. */
	int _state = 1;
	/** One-based number of the current word exercise. */
	int _round = 1;
	/** Number of pot transformations made in the current exercise. */
	int _transformations = 0;
	/** One-based answer-attempt counter for the current exercise. */
	int _attempts = 1;
	/** Counter selecting incorrect-answer feedback. */
	int _wrongSpeech = 0;
	/** Counter selecting repeated help responses. */
	int _help = 0;
	/** Index of the active transformation pot, or minus one between transformations. */
	int _pot = -1;
	/** Round most recently completed before the next exercise is prepared. */
	int _completedRound = 0;
	/** Counter selecting the next idle reminder. */
	int _idleIndex = 0;
	/** Index of the next correct-answer announcement variant. */
	int _nextSpeechIdx = 1;
	/** Whether the player's answer picture follows the pointer. */
	bool _dragging = false;
	/** Whether the current and requested word labels are visible. */
	bool _showWords = false;
	/** Whether the collectible red wagon remains on the page. */
	bool _wagonHere = true;
	/** Whether the collectible marble remains on the page. */
	bool _showMarble = false;
	/** Millisecond timestamp when the current timed stage began. */
	uint32 _start = 0;
	/** Millisecond timestamp used to schedule idle feedback. */
	uint32 _idle = 0;
	/** Delay before an idle reminder, in milliseconds. */
	uint32 _idleDelay = 5000;
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kReady;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Read an authored numeric constant selected by an indexed name. */
	int indexed(const char *key, int index) const;
	/** Create a word-pots sprite from an authored resource key and depth. */
	Shell::Sprite *sprite(const char *key, int depth);
	/** Create a word label using the ordinary or pot-label font. */
	Shell::TextObject *text(bool pot);
	/** Set a label's contents and game-pixel position. */
	static void setText(Shell::TextObject *text, const Common::String &value, int x, int y);
	/** Return a sprite's game-pixel rectangle. */
	static Common::Rect bounds(Shell::Sprite *sprite);
	/** Return the current answer picture's game-pixel hit rectangle. */
	Common::Rect answerBounds() const;
	/** Return the current picture frame's offset from the sprite anchor. */
	static Common::Point frameOffset(Shell::Sprite *sprite);
	/** Place an answer picture at a game-pixel origin, accounting for its frame offset. */
	static void placeAnswer(Shell::Sprite *sprite, Common::Point origin);
	/** Load a text resource into a word list and report success. */
	bool loadList(int resource, Common::StringArray &list);
	/** Find a word in a list, returning minus one when absent. */
	static int index(const Common::StringArray &list, const Common::String &word);
	/** Resolve a word's one-based picture frame, or zero when absent. */
	int picture(const Common::String &word) const;
	/** Play the authored spoken line for the selected actor. */
	void say(const char *actor, const Common::String &key);
	/** Queue a sequence of spoken lines using the authored actor sequence. */
	void group(const char *actors, const Common::String &key, int first = 1);
	/** Queue the spoken name of a word. */
	void sayWord(const Common::String &word);
	/** Play an effect identified by an authored sound key. */
	void sound(const char *key);
	/** Write the activity's current saved state to the world. */
	void persist();
	/** Prepare and display the next word-pots round. */
	bool setup();
	/** Refresh picture, package, wagon and reward visibility from progress. */
	void present();
	/** Move the target presentation by a horizontal pixel offset. */
	void targetPosition(int offset);
	/** Move the later wagon, mask and packages by a horizontal pixel offset. */
	void wagonPosition(int offset);
	/** Open the current package and begin the transformation instructions. */
	void unpack();
	/** Exchange the word part selected by a zero-based pot index. */
	void transform(int pot);
	/** Finish a pot transformation and advance the instructional state. */
	void grown();
	/** Check the released answer against pots, target and return positions. */
	void drop();
	/** Record a correct word and begin round-completion feedback. */
	void solved();
	/** Begin the transition from a completed exercise to the next package. */
	void next();
	/** Choose and initialize the next exercise set and round. */
	void prepareNext();
	/** Present the next target word and queue its announcement. */
	void announceNext();
	/** Queue Paige's context-sensitive word-pots guidance. */
	void help();
	/** Queue Bloom's instructions for the current progress stage. */
	void bloomHelp();
	/** Update the cursor for a game-pixel pointer position. */
	void hover(Common::Point point);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
