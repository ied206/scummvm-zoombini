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

#ifndef TRINKET_RR1CLOUD9_LA3_RAIN_GAME_H
#define TRINKET_RR1CLOUD9_LA3_RAIN_GAME_H

#include "trinket/ompp/xml.h"
#include "trinket/shell/scene.h"
#include "trinket/shell/text_object.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** LA3: sort falling rain gear into three moving attribute bins. */
class RainGame {
public:
	/** Connect the rain-catching activity to its page services. */
	explicit RainGame(PageScene &host);
	/** Release the rain-catching presentation state. */
	~RainGame();
	/** Load the rain-catching resources and initialize the current round. */
	bool enter();
	/** Advance the rain-catching presentation to its next ready stage. */
	void advance(uint32 time);
	/** Advance rain-catching animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Draw the rain-catching activity's additional graphics and text. */
	void draw(OM::Canvas &canvas);
	/** Pause or resume the activity, adjusting millisecond deadlines by the elapsed pause. */
	void pause(bool paused, uint32 elapsed);
	/** Handle activity pointer or keyboard input and report whether it was consumed. */
	bool event(const Common::Event &event);
	/** Handle a game-pixel click on activity characters or controls. */
	void click(Common::Point point);
	/** Return whether the final conversation has begun or completed. */
	bool ending() const { return _phase == kEnding || _phase == kDone; }
	/** Return the ending page name once the activity is done, or an empty string. */
	Common::String destination() const;

private:
	/** Script property name for color. */
	static constexpr const char *kPropertyColor = "color";
	/** Authored data key for Barrels ID. */
	static constexpr const char *kBarrelsIDKey = "kBarrelsID";
	/** Authored data key for Barrels Z. */
	static constexpr const char *kBarrelsZKey = "kBarrelsZ";
	/** Authored data key for Flag2 ID. */
	static constexpr const char *kFlag2IDKey = "kFlag2ID";
	/** Authored data key for Flag1 ID. */
	static constexpr const char *kFlag1IDKey = "kFlag1ID";
	/** Authored data key for Flag2 Z. */
	static constexpr const char *kFlag2ZKey = "kFlag2Z";
	/** Authored data key for Flag1 Z. */
	static constexpr const char *kFlag1ZKey = "kFlag1Z";
	/** Authored data key for Flag Offset X.%d. */
	static constexpr const char *kFlagOffsetXSignedIndexKey = "kFlagOffsetX.%d";
	/** Authored data key for Flag Offset Y.%d. */
	static constexpr const char *kFlagOffsetYSignedIndexKey = "kFlagOffsetY.%d";
	/** World-state key for r1 LA3 Level. */
	static constexpr const char *kR1LA3LevelKey = "R1LA3Level";
	/** World-state key for r1 LA3 Dataset. */
	static constexpr const char *kR1LA3DatasetKey = "R1LA3Dataset";
	/** World-state key for r1 LA3 Round. */
	static constexpr const char *kR1LA3RoundKey = "R1LA3Round";
	/** World-state key for r1 LA3 Wins. */
	static constexpr const char *kR1LA3WinsKey = "R1LA3Wins";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Authored data key for Last Spch IDs.1. */
	static constexpr const char *kLastSpchIDs1Key = "kLastSpchIDs.1";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Authored data key for Last Spch IDs.2. */
	static constexpr const char *kLastSpchIDs2Key = "kLastSpchIDs.2";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Authored data key for Last Spch IDs.3. */
	static constexpr const char *kLastSpchIDs3Key = "kLastSpchIDs.3";
	/** Script identifier for trapezoid. */
	static constexpr const char *kTrapezoidName = "trapezoid";
	/** Script identifier for circle. */
	static constexpr const char *kCircleName = "circle";
	/** Script identifier for square. */
	static constexpr const char *kSquareName = "square";
	/** Script identifier for triangle. */
	static constexpr const char *kTriangleName = "triangle";
	/** Script identifier for red. */
	static constexpr const char *kRedName = "red";
	/** Script identifier for yellow. */
	static constexpr const char *kYellowName = "yellow";
	/** Script identifier for blue. */
	static constexpr const char *kBlueName = "blue";
	/** Script identifier for green. */
	static constexpr const char *kGreenName = "green";
	/** Script identifier for boot. */
	static constexpr const char *kBootName = "boot";
	/** Script identifier for umbrella. */
	static constexpr const char *kUmbrellaName = "umbrella";
	/** Script identifier for hat. */
	static constexpr const char *kHatName = "hat";
	/** Script identifier for coat. */
	static constexpr const char *kCoatName = "coat";
	/** Format for script names or values: %d%d%d. */
	static constexpr const char *kSignedIndexSignedIndexSignedIndexFormat = "%d%d%d";
	/** Script property name for data Set/Level %d/Word List %d. */
	static constexpr const char *kPropertyDataSetLevelSignedIndexWordListSignedIndex = "DataSet/Level_%d/WordList_%d";
	/** Script property name for text. */
	static constexpr const char *kPropertyText = "text";
	/** Authored data key for Enter Spch IDs.1. */
	static constexpr const char *kEnterSpchIDs1Key = "kEnterSpchIDs.1";
	/** Authored data key for Enter Spch IDs.2. */
	static constexpr const char *kEnterSpchIDs2Key = "kEnterSpchIDs.2";
	/** Authored data key for Enter Spch IDs.3. */
	static constexpr const char *kEnterSpchIDs3Key = "kEnterSpchIDs.3";
	/** Authored data key for Enter Spch IDs.4. */
	static constexpr const char *kEnterSpchIDs4Key = "kEnterSpchIDs.4";
	/** Authored data key for Setup Spch IDs.1. */
	static constexpr const char *kSetupSpchIDs1Key = "kSetupSpchIDs.1";
	/** Authored data key for Setup Spch IDs.2. */
	static constexpr const char *kSetupSpchIDs2Key = "kSetupSpchIDs.2";
	/** Authored data key for Setup Spch IDs.%d. */
	static constexpr const char *kSetupSpchIDsSignedIndexKey = "kSetupSpchIDs.%d";
	/** Authored data key for Begin Spch IDs.1. */
	static constexpr const char *kBeginSpchIDs1Key = "kBeginSpchIDs.1";
	/** Authored data key for Begin Spch IDs.2. */
	static constexpr const char *kBeginSpchIDs2Key = "kBeginSpchIDs.2";
	/** Authored data key for Background Music ID2. */
	static constexpr const char *kBackgroundMusicID2Key = "kBackgroundMusicID2";
	/** Authored data key for Distracter Spch ID. */
	static constexpr const char *kDistracterSpchIDKey = "kDistracterSpchID";
	/** Authored data key for Background Music ID. */
	static constexpr const char *kBackgroundMusicIDKey = "kBackgroundMusicID";
	/** Authored data key for Over Spch ID. */
	static constexpr const char *kOverSpchIDKey = "kOverSpchID";
	/** Authored data key for Min Correct. */
	static constexpr const char *kMinCorrectKey = "kMinCorrect";
	/** Authored data key for More Spch IDs.%d. */
	static constexpr const char *kMoreSpchIDsSignedIndexKey = "kMoreSpchIDs.%d";
	/** Authored data key for Less Spch IDs.1. */
	static constexpr const char *kLessSpchIDs1Key = "kLessSpchIDs.1";
	/** Authored data key for Less Spch IDs.%d. */
	static constexpr const char *kLessSpchIDsSignedIndexKey = "kLessSpchIDs.%d";
	/** World-state key for r1 LA3 Streak Kind. */
	static constexpr const char *kR1LA3StreakKindKey = "R1LA3StreakKind";
	/** World-state key for r1 LA3 Streak. */
	static constexpr const char *kR1LA3StreakKey = "R1LA3Streak";
	/** World-state key for r1 LA3. */
	static constexpr const char *kR1LA3Key = "R1LA3";
	/** World-state key for r1 LA3 Fixed Level. */
	static constexpr const char *kR1LA3FixedLevelKey = "R1LA3FixedLevel";
	/** Authored data key for Dev Spch IDs.%d. */
	static constexpr const char *kDevSpchIDsSignedIndexKey = "kDevSpchIDs.%d";
	/** Authored data key for Soon Spch ID. */
	static constexpr const char *kSoonSpchIDKey = "kSoonSpchID";
	/** Authored data key for Help Spch IDs.%d. */
	static constexpr const char *kHelpSpchIDsSignedIndexKey = "kHelpSpchIDs.%d";
	/** Authored data key for Main Spch IDs.%d.1. */
	static constexpr const char *kMainSpchIDsSignedIndex1Key = "kMainSpchIDs.%d.1";
	/** Authored data key for Main Spch IDs.%d.2. */
	static constexpr const char *kMainSpchIDsSignedIndex2Key = "kMainSpchIDs.%d.2";
	/** Script identifier for r1end. */
	static constexpr const char *kR1endName = "r1end";

	/** Object-label path indexed by resource ID. */
	static constexpr const char *kTextFormat = "r1la3/%d.txt";
	/** Font for rain-game object labels. */
	static constexpr const char *kFont = "common/1018.nfnt";
	/** Rain-game puzzle datasets. */
	static constexpr const char *kDataXML = "r1la3.xml";

	/**
	 * Map a zero-based object color to its display conversion; unspecified colors remain unchanged.
	 * @note Added by ScummVM for color-blind assistance; absent from the original engine.
	 */
	static OM::Canvas::ColorRemap objectColor(int color);
	enum Phase {
		kGreeting,
		kBinIntro,
		kBinAttributes,
		kWaitDelay,
		kDelay,
		kBegin,
		kPlaying,
		kFeedback,
		kEnding,
		kDone,
	};
	struct Attributes {
		/** Zero-based shape index, or minus one when the shape is unspecified. */
		int shape = -1;
		/** Zero-based color index, or minus one when the color is unspecified. */
		int color = -1;
		/** Zero-based object index, or minus one when the object is unspecified. */
		int object = -1;
	};
	struct Answer {
		/** Sprite displaying this falling answer object. */
		Shell::Sprite *sprite = nullptr;
		/** Combined attribute key used to match the object to a bin. */
		Common::String key;
		/** Whether this object belongs to one of the three requested bin groups. */
		bool target = false;
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Parsed XML data used to construct the current activity round. */
	OM::XMLParser _data;
	/** Resource-path resolver for the activity's current page. */
	Shell::ResourcePaths _paths;
	/** Font used for activity text and labels. */
	Common::SharedPtr<OM::NFNTFont> _font;
	/** Text displaying the current catching score. */
	Shell::TextObject *_scoreText = nullptr;
	/** Movable catcher sprite beneath the falling answers. */
	Shell::Sprite *_catcher = nullptr;
	/** Sprite displaying feedback for a caught answer. */
	Shell::Sprite *_feedback = nullptr;
	/** Sprite displaying the receiving barrels. */
	Shell::Sprite *_barrels = nullptr;
	/** Highlight identifying the currently selected catcher bin. */
	Shell::Sprite *_highlight = nullptr;
	/** Attribute-label sprites for the three bins and their optional second attributes. */
	Shell::Sprite *_labels[6] = {};
	/** Progress flags indexed by display position and flag variant. */
	Shell::Sprite *_flags[6][2] = {};
	/** Catcher's original game-pixel position. */
	Common::Point _catcherOrigin;
	/** Falling objects in their emission order, including distractors. */
	Common::Array<Answer> _answers;
	/** Attribute key accepted by each of the three bins. */
	Common::String _binKeys[3];
	/** Authored word lists used for bin attributes and spoken feedback. */
	Common::StringArray _words[3];
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kGreeting;
	/** Difficulty selected for the current activity round. */
	int _level = 1;
	/** Authored dataset selected for the current difficulty. */
	int _dataset = 1;
	/** Current catching round within the activity. */
	int _round = 0;
	/** Number of successful catching rounds recorded in progress. */
	int _wins = 0;
	/** Number of points earned in the current catching round. */
	int _score = 0;
	/** Zero-based bin currently selected by the catcher. */
	int _bin = 0;
	/** Current horizontal catcher position, in game pixels. */
	int _catcherX = 0;
	/** Latest horizontal pointer position, in game pixels. */
	int _mouseX = 0;
	/** Number of elapsed fifty-millisecond falling-object steps. */
	int _ticks = 0;
	/** Index of the last emitted answer, or minus one before the first emission. */
	int _emitted = -1;
	/** Number of distractors caught in the current feedback sequence. */
	int _distractors = 0;
	/** Whether the first round's introductory presentation is pending. */
	bool _first = true;
	/** Millisecond deadline for the next catcher movement step. */
	uint32 _moveDue = 0;
	/** Millisecond deadline for the next falling-object step. */
	uint32 _fallDue = 0;
	/** Millisecond timestamp at the start of the inter-stage delay. */
	uint32 _delay = 0;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Play the authored spoken line for the selected actor. */
	void say(const char *actor, const Common::String &key);
	/** Create an answer or control sprite from a numeric resource and scene depth. */
	Shell::Sprite *sprite(int id, int depth);
	/** Load an authored text resource into a word list. */
	bool loadWords(int id, Common::StringArray &words);
	/** Decode an authored shape, color or object word into its attribute index. */
	static Attributes attribute(const Common::String &word);
	/** Combine two attribute specifications into an object-matching key. */
	static Common::String key(const Attributes &first, const Attributes &second);
	/** Return the picture frame for a specified attribute, or zero when absent. */
	static int label(const Attributes &attribute);
	/** Prepare and display the next rain-catching round. */
	bool setup();
	/** Write the activity's current saved state to the world. */
	void persist();
	/** Begin the catching round and initialize its millisecond movement deadlines. */
	void start(uint32 time);
	/** Evaluate the round score and begin result or ending feedback. */
	void finish();
	/** Refresh the progress flags from completed catching rounds. */
	void flags();
	/** Move the catcher horizontally by a game-pixel delta within its allowed range. */
	void move(int delta);
	/** Set the horizontal catcher position and update the attached presentation. */
	void position(int x);
	/** Queue the spoken attributes requested by a zero-based bin index. */
	void attributes(int bin);
	/** Queue instructions for the current round's requested attributes. */
	void setupSpeech();
	/** Score the selected falling answer against a zero-based bin index. */
	void catchAnswer(uint index, int bin);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
