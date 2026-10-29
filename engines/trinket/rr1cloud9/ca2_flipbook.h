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

#ifndef TRINKET_RR1CLOUD9_CA2_FLIPBOOK_H
#define TRINKET_RR1CLOUD9_CA2_FLIPBOOK_H

#include "trinket/ompp/font.h"
#include "trinket/rr1cloud9/artwork.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** CA2 drawing, onion skin and flipbook playback. */
class Flipbook {
public:
	/** Connect the flipbook editor activity to its page services. */
	explicit Flipbook(PageScene &host) : _host(host) {}
	/** Release the flipbook editor presentation state. */
	~Flipbook();
	/** Load the flipbook editor resources and initialize the current round. */
	bool enter();
	/** Return whether activity cursor state overrides the default arrow. */
	bool hasCustomCursor() const { return 0 <= _cursor || _cursorHidden; }
	/** Save the activity's current checkpoint state. */
	void checkpoint();
	/** Advance the flipbook editor presentation to its next ready stage. */
	void advance(uint32 time);
	/** Advance flipbook editor animations and timed feedback at a millisecond timestamp. */
	void update(uint32 time);
	/** Draw the flipbook editor activity's additional graphics and text. */
	void draw(OM::Canvas &canvas);
	/** Handle an activity click and report whether it was consumed. */
	bool click(Common::Point point);
	/** Handle activity pointer or keyboard input and report whether it was consumed. */
	bool event(const Common::Event &event);
	/** Return whether the current activity state blocks ordinary page interaction. */
	bool busy() const { return _dragging || _turn != 0; }
	/** Pause or unpause activity feedback and sprite playback. */
	void pause(bool paused);
	/** Resume activity feedback and restore its animation timing. */
	void resume(uint32 time);

private:
	/** World-state key for r1 CA2. */
	static constexpr const char *kR1CA2Key = "R1CA2";
	/** World-state key for used. */
	static constexpr const char *kUsedKey = "Used";
	/** Script identifier for reader. */
	static constexpr const char *kReaderName = "reader";
	/** Script identifier for sam. */
	static constexpr const char *kSamName = "sam";
	/** Script identifier for paige. */
	static constexpr const char *kPaigeName = "paige";
	/** Format for script names or values: %s.%u. */
	static constexpr const char *kNameUnsignedIndexFormat = "%s.%u";
	/** World-state key for r1 CA2 Dataset. */
	static constexpr const char *kR1CA2DatasetKey = "R1CA2Dataset";
	/** World-state key for r1 CA2 Pic Used%d. */
	static constexpr const char *kR1CA2PicUsedSignedIndexKey = "R1CA2PicUsed%d";
	/** World-state key for r1 CA2 Pic Max%d. */
	static constexpr const char *kR1CA2PicMaxSignedIndexKey = "R1CA2PicMax%d";
	/** Authored data key for Brush Cursor. */
	static constexpr const char *kBrushCursorKey = "kBrushCursor";
	/** Authored data key for Fill Cursor. */
	static constexpr const char *kFillCursorKey = "kFillCursor";
	/** Authored data key for Eraser Cursor. */
	static constexpr const char *kEraserCursorKey = "kEraserCursor";
	/** Authored data key for Finger Cursor ID. */
	static constexpr const char *kFingerCursorIDKey = "kFingerCursorID";
	/** Script property name for animate Settled. */
	static constexpr const char *kPropertyAnimateSettled = "animateSettled";
	/** World-state key for r1 CA2 Enter%d Count. */
	static constexpr const char *kR1CA2EnterSignedIndexCountKey = "R1CA2Enter%dCount";
	/** Authored data key for Enter Spch IDs.%d. */
	static constexpr const char *kEnterSpchIDsSignedIndexKey = "kEnterSpchIDs.%d";
	/** Script identifier for r. */
	static constexpr const char *kRName = "r";
	/** Script identifier for sr. */
	static constexpr const char *kSrName = "sr";
	/** Script identifier for s. */
	static constexpr const char *kSName = "s";
	/** Format for script names or values: .%d.%d. */
	static constexpr const char *kSignedIndexSignedIndexFormat = ".%d.%d";
	/** Authored data key for Re Enter Char IDs. */
	static constexpr const char *kReEnterCharIDsKey = "kReEnterCharIDs";
	/** Authored data key for Re Enter Spch IDs. */
	static constexpr const char *kReEnterSpchIDsKey = "kReEnterSpchIDs";
	/** World-state key for r1 CN3 Num Frames. */
	static constexpr const char *kR1CN3NumFramesKey = "R1CN3NumFrames";
	/** World-state key for r1 CN3 From Street. */
	static constexpr const char *kR1CN3FromStreetKey = "R1CN3FromStreet";
	/** World-state key for r1 CN31st Edit. */
	static constexpr const char *kR1CN31stEditKey = "R1CN31stEdit";
	/** Script identifier for two Pages. */
	static constexpr const char *kTwoPagesName = "TwoPages";
	/** Authored data key for Page Flip ID. */
	static constexpr const char *kPageFlipIDKey = "kPageFlipID";
	/** World-state key for r1 CA2 Next Clicks. */
	static constexpr const char *kR1CA2NextClicksKey = "R1CA2NextClicks";
	/** Authored data key for Arrow Spch IDs.1. */
	static constexpr const char *kArrowSpchIDs1Key = "kArrowSpchIDs.1";
	/** Script identifier for psr. */
	static constexpr const char *kPsrName = "psr";
	/** Authored data key for Arrow Spch IDs.2.1. */
	static constexpr const char *kArrowSpchIDs21Key = "kArrowSpchIDs.2.1";
	/** Script identifier for paint. */
	static constexpr const char *kPaintName = "Paint";
	/** Script identifier for fill. */
	static constexpr const char *kFillName = "Fill";
	/** Script identifier for stamp. */
	static constexpr const char *kStampName = "Stamp";
	/** Script identifier for play. */
	static constexpr const char *kPlayName = "Play";
	/** Format for script names or values: .%u. */
	static constexpr const char *kUnsignedIndexFormat = ".%u";
	/** Authored data key for Idle Char ID. */
	static constexpr const char *kIdleCharIDKey = "kIdleCharID";
	/** Authored data key for Idle Spch ID. */
	static constexpr const char *kIdleSpchIDKey = "kIdleSpchID";
	/** Authored data key for Help Spch IDs.%u. */
	static constexpr const char *kHelpSpchIDsUnsignedIndexKey = "kHelpSpchIDs.%u";
	/** Script identifier for erase. */
	static constexpr const char *kEraseName = "Erase";
	/** Authored data key for Paint SFXID. */
	static constexpr const char *kPaintSFXIDKey = "kPaintSFXID";
	/** Authored data key for Fill SFXID. */
	static constexpr const char *kFillSFXIDKey = "kFillSFXID";
	/** Authored data key for Erase SFXID. */
	static constexpr const char *kEraseSFXIDKey = "kEraseSFXID";
	/** Authored data key for Stamp SFXID. */
	static constexpr const char *kStampSFXIDKey = "kStampSFXID";
	/** World-state key for r1 CA2 Paige Clicks. */
	static constexpr const char *kR1CA2PaigeClicksKey = "R1CA2PaigeClicks";
	/** Authored data key for Paige Spch IDs.%d. */
	static constexpr const char *kPaigeSpchIDsSignedIndexKey = "kPaigeSpchIDs.%d";
	/** Authored data key for Color SFXID.%d. */
	static constexpr const char *kColorSFXIDSignedIndexKey = "kColorSFXID.%d";
	/** Authored data key for Pick Shape SFXID. */
	static constexpr const char *kPickShapeSFXIDKey = "kPickShapeSFXID";
	/** Script identifier for bomb. */
	static constexpr const char *kBombName = "Bomb";
	/** Authored data key for Explode SFXID. */
	static constexpr const char *kExplodeSFXIDKey = "kExplodeSFXID";
	/** Authored data key for Undo SFXID. */
	static constexpr const char *kUndoSFXIDKey = "kUndoSFXID";
	/** World-state key for is Printer Off. */
	static constexpr const char *kIsPrinterOffKey = "isPrinterOff";
	/** Script identifier for rs. */
	static constexpr const char *kRsName = "rs";
	/** Script identifier for srr. */
	static constexpr const char *kSrrName = "srr";
	/** Authored data key for Main Spch IDs.%d. */
	static constexpr const char *kMainSpchIDsSignedIndexKey = "kMainSpchIDs.%d";

	/** Font for flipbook labels. */
	static constexpr const char *kFont = "common/1018.nfnt";
	/** Flipbook palette path indexed by resource ID. */
	static constexpr const char *kPaletteFormat = "r1ca2/%d.rgb";

	enum Tool {
		kPaint,
		kFill,
		kErase,
		kStamp,
	};
	/** Page services for activity resources, scene objects, audio and progression. */
	PageScene &_host;
	/** Temporary artwork used without modifying saved story books during practice. */
	Artwork _practiceArtwork;
	/** Editable RGB555 pixels of the current frame. */
	Common::Array<uint16> _pixels;
	/** Previous-frame pixels used for onion-skin drawing guidance. */
	Common::Array<uint16> _onion;
	/** Pixel snapshot restored by the undo action. */
	Common::Array<uint16> _undo;
	/** Current frame converted to drawable pixels. */
	Graphics::Surface _image;
	/** Decoded image of the selected stamp. */
	Graphics::Surface _stampImage;
	/** Cached recolored tool or stamp images. */
	Graphics::Surface _colored[8];
	/** Decoded drawing-tool cursor images. */
	Graphics::Surface _cursors[4];
	/** Pointer hotspots corresponding to the drawing-tool cursors. */
	Common::Point _cursorHotspots[4];
	/** Current drawing-tool cursor index, or minus one when unset. */
	int _cursor = -1;
	/** Whether activity input and timed playback are paused. */
	bool _paused = false;
	/** Whether the activity temporarily hides the pointer. */
	bool _cursorHidden = false;
	/** FontMan-owned strike used for frame labels. */
	const Graphics::Font *_font = nullptr;
	/** Color palette button sprites. */
	Shell::Sprite *_colors[12] = {};
	/** Stamp-selection button sprites. */
	Shell::Sprite *_stamps[6] = {};
	/** Drawing-tool button sprites. */
	Shell::Sprite *_tools[3] = {};
	/** Button that selects the next flipbook frame. */
	Shell::Sprite *_next = nullptr;
	/** Button that selects the previous flipbook frame. */
	Shell::Sprite *_previous = nullptr;
	/** Button that starts or stops flipbook playback. */
	Shell::Sprite *_play = nullptr;
	/** Button that clears the current artwork frame. */
	Shell::Sprite *_bomb = nullptr;
	/** Button that restores the most recent pixel snapshot. */
	Shell::Sprite *_undoButton = nullptr;
	/** Button that exports the flipbook contact sheet. */
	Shell::Sprite *_print = nullptr;
	/** Sprite used for page-turn feedback. */
	Shell::Sprite *_flip = nullptr;
	/** Previous drawing position used to connect stroke segments. */
	Common::Point _last;
	/** Most recent pointer position in game pixels. */
	Common::Point _mouse;
	/** Control sprite held by the current pointer press. */
	Shell::Sprite *_pressed = nullptr;
	/** One-based book currently being edited. */
	int _book = 4;
	/** One-based artwork frame currently being edited. */
	int _frame = 1;
	/** Last frame available to the current book presentation. */
	int _maximum = 1;
	/** Selected palette color index. */
	int _color = 0;
	/** Selected stamp index, or minus one when no stamp is active. */
	int _stamp = -1;
	/** Current page-turn direction, or zero outside a page transition. */
	int _turn = 0;
	/** Saved flipbook interaction state. */
	int _state = 2;
	/** Current paint, fill or stamp tool. */
	Tool _tool = kPaint;
	/** Drawing tool restored after a temporary selection. */
	Tool _previousTool = kPaint;
	/** Whether edited pixels need to be saved or redrawn. */
	bool _dirty = false;
	/** Whether the current drawing gesture changed artwork pixels. */
	bool _painted = false;
	/** Whether a drawing or tool gesture is active. */
	bool _dragging = false;
	/** Whether the book is automatically advancing through frames. */
	bool _playing = false;
	/** Whether the editor is ready for ordinary drawing input. */
	bool _ready = false;
	/** Whether introductory help still needs to begin. */
	bool _helpPending = true;
	/** Millisecond deadline for the next flipbook playback frame. */
	uint32 _nextFrame = 0;
	/** Millisecond timestamp used to schedule idle feedback. */
	uint32 _idle = 0;
	/** Millisecond timestamp when the current help sequence began. */
	uint32 _helpStart = 0;
	/** Millisecond timestamp when editor playback was paused. */
	uint32 _pauseStart = 0;
	/** Next prompt in the help sequence. */
	uint _helpIndex = 0;
	/** Current phase of the help presentation. */
	uint _helpPhase = 0;
	/** Whether an idle prompt is waiting for another presentation to finish. */
	bool _idleWaiting = false;
	/** Main character click count used to select responses. */
	int _mainClicks = 0;
	/** Return story artwork or the temporary practice books. */
	Artwork &artwork();
	/** Read an authored numeric page constant. */
	int number(const Common::String &key) const;
	/** Record that the player used the named editor control. */
	void used(const char *key);
	/** Play the authored spoken line for the selected actor. */
	void say(const char *actor, const Common::String &key);
	/** Play a grouped prompt using the specified actors. */
	void group(const Common::String &key, const char *actors);
	/** Load the selected frame pixels and prepare onion-skin and undo data. */
	bool loadFrame();
	/** Refresh editor button frames and enabled states. */
	void buttons();
	/** Cancel pending editor help after player interaction. */
	void cancelHelp();
	/** Advance the timed editor help presentation. */
	void updateHelp(uint32 time);
	/** Select and position the cursor for the active drawing tool. */
	void updateCursor();
	/** Restore the default cursor after editor interaction ends. */
	void releaseCursor();
	/** Begin moving to an adjacent frame in the selected direction. */
	void turn(int direction);
	/** Stop automatic flipbook playback. */
	void stop();
	/** Select paint, fill or stamp mode and update tool feedback. */
	void selectTool(Tool tool);
	/** Load the currently selected stamp image. */
	bool stampImage();
	/** Decode and recolor a tool or stamp image for the selected palette. */
	bool coloredImage(Shell::Sprite &sprite, int resource, Graphics::Surface &image) const;
	/** Return the selected paint color in RGB555 format. */
	uint16 color() const;
	/** Draw a continuous paint stroke between two artwork-pixel points. */
	void stroke(Common::Point from, Common::Point to);
	/** Flood-fill the connected area at an artwork-pixel point. */
	void fill(Common::Point point);
	/** Apply the selected stamp at an artwork-pixel point. */
	void stamp(Common::Point point);
	/** Handle a click on Paige and select the appropriate editor prompt. */
	void paige();
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
