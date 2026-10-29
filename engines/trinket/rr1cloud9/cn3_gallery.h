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

#ifndef TRINKET_RR1CLOUD9_CN3_GALLERY_H
#define TRINKET_RR1CLOUD9_CN3_GALLERY_H

#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** CN3 gallery, flipbook preview and artist story transitions. */
class Gallery {
public:
	/** Connect the art gallery activity to its page services. */
	explicit Gallery(PageScene &host) : _host(host) {}
	/** Release the art gallery presentation state. */
	~Gallery();
	/** Load the art gallery resources and initialize the current round. */
	bool enter();
	/** Advance the art gallery presentation to its next ready stage. */
	void advance(uint32 time);
	/** Advance art gallery animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Draw the art gallery activity's additional graphics and text. */
	void draw(OM::Canvas &canvas);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Handle activity pointer or keyboard input and report whether it was consumed. */
	bool event(const Common::Event &event);
	/** Pause or unpause activity feedback and sprite playback. */
	void pause(bool paused);
	/** Restart gallery animation timing from the supplied millisecond timestamp. */
	void resume(uint32 time);
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _phase != kIdle || _destination != 0; }
	/** Return whether the gallery requests its movie presentation. */
	bool wantsMovie() const { return _phase == kMovie; }
	/** Return the pending gallery destination code, or zero while remaining here. */
	int destination() const { return _destination; }

private:
	/** Format for script names or values: %s.%d. */
	static constexpr const char *kNameSignedIndexFormat = "%s.%d";
	/** World-state key for artist. */
	static constexpr const char *kArtistKey = "artist";
	/** Format for script names or values: %s.%u. */
	static constexpr const char *kNameUnsignedIndexFormat = "%s.%u";
	/** World-state key for r1 CN3 State. */
	static constexpr const char *kR1CN3StateKey = "R1CN3State";
	/** Authored data key for Artist Set ID. */
	static constexpr const char *kArtistSetIDKey = "kArtistSetID";
	/** Authored data key for Artist Z. */
	static constexpr const char *kArtistZKey = "kArtistZ";
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
	/** Authored data key for Artist Lips ID. */
	static constexpr const char *kArtistLipsIDKey = "kArtistLipsID";
	/** Authored data key for Artist Body ID. */
	static constexpr const char *kArtistBodyIDKey = "kArtistBodyID";
	/** Script method name for add Fidget Group. */
	static constexpr const char *kMethodAddFidgetGroup = "addFidgetGroup";
	/** Script method name for add Fidget. */
	static constexpr const char *kMethodAddFidget = "addFidget";
	/** Authored data key for Artist Fidg1 ID. */
	static constexpr const char *kArtistFidg1IDKey = "kArtistFidg1ID";
	/** Authored data key for Artist Fidg2 ID. */
	static constexpr const char *kArtistFidg2IDKey = "kArtistFidg2ID";
	/** World-state key for r1 CN3 Num Frames. */
	static constexpr const char *kR1CN3NumFramesKey = "R1CN3NumFrames";
	/** Authored data key for Num Min Pages. */
	static constexpr const char *kNumMinPagesKey = "kNumMinPages";
	/** World-state key for r1 CN3 Said Act4. */
	static constexpr const char *kR1CN3SaidAct4Key = "R1CN3SaidAct4";
	/** Script identifier for aSEQ CN3 EASEL. */
	static constexpr const char *kASEQCN3EASELName = "ASEQ_CN3EASEL";
	/** Script identifier for aSEQ CN3 HAT. */
	static constexpr const char *kASEQCN3HATName = "ASEQ_CN3HAT";
	/** Script identifier for aSEQ CN3 TAKEOFF. */
	static constexpr const char *kASEQCN3TAKEOFFName = "ASEQ_CN3TAKEOFF";
	/** Format for script names or values: ASEQ_CN3PICFRAME1.%d. */
	static constexpr const char *kASEQCN3PICFRAME1SignedIndexFormat = "ASEQ_CN3PICFRAME1.%d";
	/** Authored data key for Pic X. */
	static constexpr const char *kPicXKey = "kPicX";
	/** Authored data key for Pic Y. */
	static constexpr const char *kPicYKey = "kPicY";
	/** World-state key for r1 CA2 Pic Max%d. */
	static constexpr const char *kR1CA2PicMaxSignedIndexKey = "R1CA2PicMax%d";
	/** Resource path for samples/SampleB%dI%02d.DAT. */
	static constexpr const char *kSamplesSampleBSignedIndexISignedIndexDATPath = "samples/SampleB%dI%02d.DAT";
	/** World-state key for r1 CA2 Dataset. */
	static constexpr const char *kR1CA2DatasetKey = "R1CA2Dataset";
	/** World-state key for r1 CN3 Hat Pending. */
	static constexpr const char *kR1CN3HatPendingKey = "R1CN3HatPending";
	/** World-state key for r1 CN3 Movie Pending. */
	static constexpr const char *kR1CN3MoviePendingKey = "R1CN3MoviePending";
	/** Authored data key for Enter Spch IDs. */
	static constexpr const char *kEnterSpchIDsKey = "kEnterSpchIDs";
	/** Script identifier for ssr. */
	static constexpr const char *kSsrName = "ssr";
	/** World-state key for r1 CN3 From Street. */
	static constexpr const char *kR1CN3FromStreetKey = "R1CN3FromStreet";
	/** World-state key for previous Location. */
	static constexpr const char *kPreviousLocationKey = "previousLocation";
	/** Authored data key for Re Enter Spch IDs.2. */
	static constexpr const char *kReEnterSpchIDs2Key = "kReEnterSpchIDs.2";
	/** Script identifier for asa. */
	static constexpr const char *kAsaName = "asa";
	/** Authored data key for Re Enter Spch IDs.3.1. */
	static constexpr const char *kReEnterSpchIDs31Key = "kReEnterSpchIDs.3.1";
	/** Authored data key for Re Enter Spch IDs.3. */
	static constexpr const char *kReEnterSpchIDs3Key = "kReEnterSpchIDs.3";
	/** Authored data key for Re Enter Spch IDs.3.4. */
	static constexpr const char *kReEnterSpchIDs34Key = "kReEnterSpchIDs.3.4";
	/** World-state key for r1 CN3 Said SS4. */
	static constexpr const char *kR1CN3SaidSS4Key = "R1CN3SaidSS4";
	/** Authored data key for Re Enter Spch IDs.4. */
	static constexpr const char *kReEnterSpchIDs4Key = "kReEnterSpchIDs.4";
	/** Script identifier for aa. */
	static constexpr const char *kAaName = "aa";
	/** World-state key for r1 CN31st Edit. */
	static constexpr const char *kR1CN31stEditKey = "R1CN31stEdit";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Authored data key for Re Enter Spch IDs A.3.1. */
	static constexpr const char *kReEnterSpchIDsA31Key = "kReEnterSpchIDsA.3.1";
	/** Authored data key for Re Enter Spch IDs A.4. */
	static constexpr const char *kReEnterSpchIDsA4Key = "kReEnterSpchIDsA.4";
	/** Script identifier for aara. */
	static constexpr const char *kAaraName = "aara";
	/** Authored data key for Re Enter Spch IDs A.5. */
	static constexpr const char *kReEnterSpchIDsA5Key = "kReEnterSpchIDsA.5";
	/** World-state key for r1 CA2 Pic Used%d. */
	static constexpr const char *kR1CA2PicUsedSignedIndexKey = "R1CA2PicUsed%d";
	/** Format for script names or values: item:%d. */
	static constexpr const char *kItemSignedIndexFormat = "item:%d";
	/** Authored data key for INV1 Hat. */
	static constexpr const char *kINV1HatKey = "kINV1Hat";
	/** Script property name for visible. */
	static constexpr const char *kPropertyVisible = "visible";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Authored data key for Re Enter Spch IDs A.4.5. */
	static constexpr const char *kReEnterSpchIDsA45Key = "kReEnterSpchIDsA.4.5";
	/** Authored data key for Re Enter Spch IDs A.4.6. */
	static constexpr const char *kReEnterSpchIDsA46Key = "kReEnterSpchIDsA.4.6";
	/** World-state key for talked To Rock Head. */
	static constexpr const char *kTalkedToRockHeadKey = "talkedToRockHead";
	/** Authored data key for Re Enter Spch IDs A.4.7. */
	static constexpr const char *kReEnterSpchIDsA47Key = "kReEnterSpchIDsA.4.7";
	/** Authored data key for Idle Char ID.%d. */
	static constexpr const char *kIdleCharIDSignedIndexKey = "kIdleCharID.%d";
	/** Authored data key for Idle Spch ID.%d. */
	static constexpr const char *kIdleSpchIDSignedIndexKey = "kIdleSpchID.%d";
	/** Authored data key for Max Idle Index Speeches. */
	static constexpr const char *kMaxIdleIndexSpeechesKey = "kMaxIdleIndexSpeeches";
	/** Authored data key for Artist Char IDs.1. */
	static constexpr const char *kArtistCharIDs1Key = "kArtistCharIDs.1";
	/** Authored data key for Artist Spch IDs.1. */
	static constexpr const char *kArtistSpchIDs1Key = "kArtistSpchIDs.1";
	/** Authored data key for Artist Spch IDs.2.1. */
	static constexpr const char *kArtistSpchIDs21Key = "kArtistSpchIDs.2.1";
	/** Authored data key for Artist Spch IDs.3.1. */
	static constexpr const char *kArtistSpchIDs31Key = "kArtistSpchIDs.3.1";
	/** World-state key for r1 CN3 From Sample. */
	static constexpr const char *kR1CN3FromSampleKey = "R1CN3FromSample";
	/** Authored data key for Artist Spch IDs.3.4. */
	static constexpr const char *kArtistSpchIDs34Key = "kArtistSpchIDs.3.4";
	/** Authored data key for Artist Spch IDs.3.2. */
	static constexpr const char *kArtistSpchIDs32Key = "kArtistSpchIDs.3.2";
	/** Script identifier for aaa. */
	static constexpr const char *kAaaName = "aaa";
	/** Authored data key for Artist Spch IDs.3.3. */
	static constexpr const char *kArtistSpchIDs33Key = "kArtistSpchIDs.3.3";
	/** Authored data key for Artist Spch IDs.4. */
	static constexpr const char *kArtistSpchIDs4Key = "kArtistSpchIDs.4";
	/** Script identifier for ar. */
	static constexpr const char *kArName = "ar";
	/** Authored data key for Main Char IDs.2.%d. */
	static constexpr const char *kMainCharIDs2SignedIndexKey = "kMainCharIDs.2.%d";
	/** Authored data key for Main Spch IDs.2.%d. */
	static constexpr const char *kMainSpchIDs2SignedIndexKey = "kMainSpchIDs.2.%d";
	/** Authored data key for Max Main2 Speeches. */
	static constexpr const char *kMaxMain2SpeechesKey = "kMaxMain2Speeches";
	/** Authored data key for Main Spch IDs.4.1. */
	static constexpr const char *kMainSpchIDs41Key = "kMainSpchIDs.4.1";
	/** Authored data key for Main Spch IDs.5.1. */
	static constexpr const char *kMainSpchIDs51Key = "kMainSpchIDs.5.1";
	/** Script identifier for sa. */
	static constexpr const char *kSaName = "sa";
	/** Authored data key for Main Spch IDs.5.2. */
	static constexpr const char *kMainSpchIDs52Key = "kMainSpchIDs.5.2";
	/** Script identifier for sr. */
	static constexpr const char *kSrName = "sr";
	/** Authored data key for COMSoft Click Sfx ID. */
	static constexpr const char *kCOMSoftClickSfxIDKey = "kCOMSoftClickSfxID";
	/** Authored data key for Hat Spch IDs.1.1. */
	static constexpr const char *kHatSpchIDs11Key = "kHatSpchIDs.1.1";
	/** Authored data key for Hat Spch IDs.3.1. */
	static constexpr const char *kHatSpchIDs31Key = "kHatSpchIDs.3.1";
	/** Script identifier for raaasaa. */
	static constexpr const char *kRaaasaaName = "raaasaa";
	/** Authored data key for Hat Spch IDs.2.1. */
	static constexpr const char *kHatSpchIDs21Key = "kHatSpchIDs.2.1";
	/** Script identifier for raaa. */
	static constexpr const char *kRaaaName = "raaa";
	/** Authored data key for Hat Spch IDs.2.2. */
	static constexpr const char *kHatSpchIDs22Key = "kHatSpchIDs.2.2";
	/** Script identifier for raa. */
	static constexpr const char *kRaaName = "raa";
	/** Authored data key for Hat Spch IDs.4. */
	static constexpr const char *kHatSpchIDs4Key = "kHatSpchIDs.4";
	/** Script identifier for sar. */
	static constexpr const char *kSarName = "sar";
	/** Authored data key for Easel Spch ID. */
	static constexpr const char *kEaselSpchIDKey = "kEaselSpchID";
	/** Script identifier for rra. */
	static constexpr const char *kRraName = "rra";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Authored data key for Paige Spch IDs. */
	static constexpr const char *kPaigeSpchIDsKey = "kPaigeSpchIDs";

	enum Phase {
		kIdle,
		kIntroduction,
		kBeforeMovie,
		kMovie,
		kOfferSpeech,
		kTakeOff,
		kCollectSpeech,
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Highlightable picture-frame sprites for the five paintings. */
	Shell::Sprite *_frames[5] = {};
	/** Easel sprite used to enter the artwork editor. */
	Shell::Sprite *_easel = nullptr;
	/** Collectible hat sprite. */
	Shell::Sprite *_hat = nullptr;
	/** Sprite used during the hat-removal presentation. */
	Shell::Sprite *_takeOff = nullptr;
	/** Decoded sample images indexed by sample book and animation frame. */
	Common::Array<Graphics::Surface *> _samples[3];
	/** Cached rendered surfaces for the player's five artwork books. */
	Graphics::Surface _edited[5];
	/** One-based artwork frame currently rendered in each cached surface. */
	int _editedFrame[5] = {};
	/** Game-pixel drawing position of each painting. */
	Common::Point _positions[5];
	/** Number of available animation frames in each painting. */
	int _maximum[5] = {};
	/** One-based painting currently selected for animation or editing. */
	int _painting = 1;
	/** One-based animation frame displayed in the selected painting. */
	int _frame = 1;
	/** One-based painting beneath the pointer, or zero when none is highlighted. */
	int _hover = 0;
	/** Saved progress through gallery introductions, editing and reward collection. */
	int _state = 1;
	/** Counter selecting responses to the collectible hat. */
	int _hatClicks = 0;
	/** Counter selecting the next idle conversation. */
	int _idleIndex = 0;
	/** Pending destination code for leaving the gallery or entering the editor. */
	int _destination = 0;
	/** Millisecond deadline for advancing the selected painting's animation. */
	uint32 _nextFrame = 0;
	/** Millisecond timestamp used to schedule idle feedback. */
	uint32 _idle = 0;
	/** Whether gallery animation timestamps have been initialized. */
	bool _clockStarted = false;
	/** Whether the hat-removal animation should resume after a pause. */
	bool _takeOffPaused = false;
	/** Current introduction, interaction or result-feedback stage. */
	Phase _phase = kIdle;
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Read an authored numeric constant selected by an indexed name. */
	int indexed(const char *key, int index) const;
	/** Play the authored spoken line for the selected actor. */
	void say(const char *actor, const Common::String &key);
	/** Queue an authored speech group using the supplied actor sequence. */
	void group(const char *key, const char *actors);
	/** Create or replace the artist character for the requested pose. */
	bool artist(int pose);
	/** Queue the artist's response for the current gallery progress. */
	void artistSpeech();
	/** Queue Reader and Sam's response for the current gallery progress. */
	void mainSpeech();
	/** Handle the hat interaction and begin collection when permitted. */
	void hatClick();
	/** Select a one-based painting and prepare its editing transition. */
	void choose(int painting);
	/** Refresh frame highlights for the selected and hovered paintings. */
	void highlight();
	/** Set gallery progress and store it in the world. */
	void setState(int state);
	/** Collect the hat and update inventory and gallery progress. */
	void collect();
	/** Decode a gallery image stream into a surface and report success. */
	static bool loadImage(Common::SeekableReadStream &stream, Graphics::Surface &image);
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
