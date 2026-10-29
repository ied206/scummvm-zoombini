/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#ifndef TRINKET_ZOOMBINI3_PAGES_PUZZLE_GLYPH_H
#define TRINKET_ZOOMBINI3_PAGES_PUZZLE_GLYPH_H

#include "common/ptr.h"
#include "common/scummsys.h"
#include "trinket/frame_clock.h"
#include "trinket/shell/scene.h"
#include "trinket/zoombini3_character.h"
#include "trinket/zoombini3_world.h"
#include "trinket/zoombini3_world_port.h"

namespace Trinket {
namespace Shell {

class Random;
}
namespace Zoombini3 {

/** Glyph board, substitution alphabet and movable tile state. */
class GlyphRules {
public:
	/** Number of tiles supplied for a Glyph puzzle. */
	static constexpr uint kTileCount = 12;
	/** Number of distinct decoded symbol values. */
	static constexpr uint kValueCount = 10;
	/** Number of rows in the glyph board. */
	static constexpr uint kRowCount = 2;
	/** Maximum number of board cells across supported difficulties. */
	static constexpr uint kMaximumGlyphs = 56;
	struct Tile {
		/** Decoded symbol values carried by the tile, in reading order. */
		int values[4] = {};
		/** Starting board cell of the placed tile, or minus one while unplaced. */
		int slot = -1;
		/** Tile's position in the initial supply arrangement. */
		uint supplyPosition = 0;
		/** Number of symbols matching the tile's current board placement. */
		uint matches = 0;
		/** Whether the tile has been accepted and revealed. */
		bool solved = false;
	};
	/** Initialize an empty glyph board and unplaced tile state. */
	GlyphRules();
	/** Generate a difficulty-specific glyph board and shuffled supply tiles. */
	bool generate(uint level, Shell::Random &random);
	/** Return the difficulty used to generate the current board. */
	uint level() const { return _level; }
	/** Return the number of active glyph cells. */
	uint glyphCount() const { return _glyphCount; }
	/** Return the number of symbols carried by each tile. */
	uint charactersPerTile() const { return _characters; }
	/** Return the number of glyph cells in one board row. */
	uint columns() const { return _glyphCount / kRowCount; }
	/** Return whether placement uses the clustered rule of the first difficulty. */
	bool clustered() const { return _level == 1; }
	/** Return how many mismatched symbols a placement may contain. */
	uint allowedMistakes() const;
	/** Return the encoded glyph ID at a board cell. */
	int glyph(uint cell) const { return _glyphs[cell]; }
	/** Return the symbol value represented by a board cell's glyph. */
	int decodedGlyph(uint cell) const { return _alphabet[_glyphs[cell]]; }
	/** Return the tile index covering a board cell, or the empty-cell marker. */
	int occupancy(uint cell) const { return _occupancy[cell]; }
	/** Return a supplied tile's symbols and placement state. */
	const Tile &tile(uint index) const { return _tiles[index]; }
	/** Check whether a tile can occupy the proposed starting cell. */
	bool canPlace(uint index, int cell) const;
	/** Remove an unaccepted tile from its board cells. */
	bool pickUp(uint index);

	/** A legal placement may be wrong; inspect the tile's matches and solved state. */
	bool place(uint index, int cell);

	/** Find a complete compatible placement without changing the board or RNG. */
	bool findSolution(int *slots) const;

private:
	struct Range {
		/** First board cell in a candidate placement range. */
		int start;
		/** Number of consecutive cells in the placement range. */
		int count;
	};
	/** Difficulty used to generate symbols and placement constraints. */
	uint _level = 0;
	/** Number of active board cells. */
	uint _glyphCount = 0;
	/** Number of symbols used on each tile. */
	uint _characters = 0;
	/** Encoded glyph IDs indexed by board cell. */
	int _glyphs[kMaximumGlyphs] = {};
	/** Tile covering each board cell, with a negative marker for empty cells. */
	int _occupancy[kMaximumGlyphs];
	/** Mapping from encoded glyph IDs to decoded symbol values. */
	int _alphabet[kValueCount] = {};
	/** Supplied tile values, placement positions and acceptance state. */
	Tile _tiles[kTileCount] = {};
	/** Introduce repeated symbol patterns while retaining the generated solution. */
	void addDuplicatePatterns(int *starts, Shell::Random &random);
	/** Shuffle values using paired random selections. */
	static void shuffleByPairs(int *values, uint count, Shell::Random &random);
	/** Find compatible starting cells for tiles not yet solved. */
	static bool solveRemaining(const GlyphRules &state, int *slots);
};

/** Display movable Glyph tiles and board feedback configured by MPS scripts. */
class GlyphMachine : public Core::Trinket {
public:
	class Listener {
	public:
		/** Allow destruction through the Glyph feedback interface. */
		virtual ~Listener() {}
		/** Notify the page that the player picked up a tile. */
		virtual void glyphPickedUp() = 0;
		/** Notify the page whether the tile placement was accepted. */
		virtual void glyphPlaced(bool correct) = 0;
		/** Notify the page after a placed tile's glyph reveal completes. */
		virtual void glyphRevealed() = 0;
	};
	/** Configure the glyph board with generated rules or a shared random stream. */
	GlyphMachine(Shell::Scene &scene, const GlyphRules &rules);
	/** Configure the glyph board with generated rules or a shared random stream. */
	GlyphMachine(Shell::Scene &scene, Shell::Random &random, Common::Point position, int depth);
	/** Remove the board's tile, glyph and feedback sprites. */
	~GlyphMachine() override;
	/** Read glyph-board configuration and inherited object properties. */
	bool get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Assign glyph-board configuration and inherited object properties. */
	bool set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) override;
	/** Dispatch puzzle initialization, generation and tile-control script methods. */
	bool play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) override;
	/** Create the tile supply and board sprites from the current rules and configuration. */
	bool load();
	/** Handle tile pickup, dragging and release at a millisecond timestamp. */
	bool event(const Common::Event &event, uint32 time);
	/** Advance placement lights and glyph reveal animations. */
	void update(uint32 time);
	/** Pause or resume feedback and reveal timing. */
	void pause(bool paused, uint32 time);
	/** End any active tile drag using the current drop target. */
	void forceDrop();
	/** Enable or disable tile interaction. */
	void setEnabled(bool allowed) { enabled = allowed; }
	/** Set the page callback for pickup, placement and reveal feedback. */
	void setListener(Listener *listener) { _listener = listener; }
	/** Consume a queued sound ID and its optional channel selection. */
	bool pollSound(uint16 &sound, int *channel = nullptr);
	/** Return the tile at a game-pixel point, or minus one when none is hit. */
	int hitTile(Common::Point point) const;
	/** Return whether a tile is currently being dragged. */
	bool dragging() const { return 0 <= _dragged; }
	/** Return whether a required puzzle sprite failed to load. */
	bool hasResourceError() const { return _resError; }
	/** Describe valid tile placements for debugger answer output. */
	Common::String debugAnswer(uint level) const;
	/** Return the generated board and current tile placement state. */
	const GlyphRules &rules() const { return _rules; }
	/** Return a tile's combined game-pixel bounds. */
	Common::Rect tileBounds(uint index) const;
	/** Return the game-pixel position of a board cell. */
	Common::Point slotPosition(uint cell) const;

private:
	/** Script identifier for value Count. */
	static constexpr const char *kValueCountName = "valueCount";
	/** Script identifier for glyph Count. */
	static constexpr const char *kGlyphCountName = "glyphCount";
	/** Script identifier for row Count. */
	static constexpr const char *kRowCountName = "rowCount";
	/** Script identifier for char Count Per Tile. */
	static constexpr const char *kCharCountPerTileName = "charCountPerTile";
	/** Script identifier for tile Count. */
	static constexpr const char *kTileCountName = "tileCount";
	/** Script event name for tile Picked Up. */
	static constexpr const char *kEventTilePickedUp = "tilePickedUp";
	/** Script event name for tile Dropped. */
	static constexpr const char *kEventTileDropped = "tileDropped";
	/** Script event name for tile Solved. */
	static constexpr const char *kEventTileSolved = "tileSolved";
	/** Script event name for tile Wrong. */
	static constexpr const char *kEventTileWrong = "tileWrong";
	/** Script event name for anim Finished. */
	static constexpr const char *kEventAnimFinished = "animFinished";
	/** Script property name for enabled. */
	static constexpr const char *kPropertyEnabled = "enabled";
	/** Script identifier for cluster. */
	static constexpr const char *kClusterName = "cluster";
	/** Script identifier for do Stamp. */
	static constexpr const char *kDoStampName = "doStamp";
	/** Script identifier for need To Restore Stamp. */
	static constexpr const char *kNeedToRestoreStampName = "needToRestoreStamp";
	/** Script identifier for piece Grab AOCursor ID. */
	static constexpr const char *kPieceGrabAOCursorIDName = "pieceGrabAOCursorID";
	/** Script identifier for piece Spot AOCursor ID. */
	static constexpr const char *kPieceSpotAOCursorIDName = "pieceSpotAOCursorID";
	/** Script identifier for debug Mode. */
	static constexpr const char *kDebugModeName = "debugMode";
	/** Script method name for dbg Read Puzzle Attributes. */
	static constexpr const char *kMethodDbgReadPuzzleAttributes = "dbgReadPuzzleAttributes";
	/** Script method name for select Node. */
	static constexpr const char *kMethodSelectNode = "selectNode";
	/** Script method name for z3 A2 Data. */
	static constexpr const char *kMethodZ3A2Data = "Z3A2Data";
	/** Script property name for node Tag. */
	static constexpr const char *kPropertyNodeTag = "nodeTag";
	/** Script property name for attribute Value. */
	static constexpr const char *kPropertyAttributeValue = "attributeValue";
	/** Script method name for pause. */
	static constexpr const char *kMethodPause = "pause";
	/** Script method name for resume. */
	static constexpr const char *kMethodResume = "resume";
	/** Script method name for force Drop. */
	static constexpr const char *kMethodForceDrop = "forceDrop";
	/** Script method name for clear Puzzle. */
	static constexpr const char *kMethodClearPuzzle = "clearPuzzle";
	/** Script method name for generate Puzzle. */
	static constexpr const char *kMethodGeneratePuzzle = "generatePuzzle";
	/** Script method name for set Indent AO. */
	static constexpr const char *kMethodSetIndentAO = "setIndentAO";
	/** Script method name for set Tile AO. */
	static constexpr const char *kMethodSetTileAO = "setTileAO";
	/** Script method name for set Tile Characters AO. */
	static constexpr const char *kMethodSetTileCharactersAO = "setTileCharactersAO";
	/** Script method name for set Tile Lights AO. */
	static constexpr const char *kMethodSetTileLightsAO = "setTileLightsAO";
	/** Script method name for set Glyphs AO. */
	static constexpr const char *kMethodSetGlyphsAO = "setGlyphsAO";
	/** Script method name for set Highlight AO. */
	static constexpr const char *kMethodSetHighlightAO = "setHighlightAO";
	/** Format for script names or values: %s%d. */
	static constexpr const char *kNameSignedIndexFormat = "%s%d";

	/** Animation path pattern for Glyph resource IDs. */
	static constexpr const char *kAnimation = "/z3a2/%u.ao";
	/** XML resource supplying authored Glyph test input. */
	static constexpr const char *kDebugInput = "z3a2in.xml";
	struct Piece {
		/** Sprite displaying the tile background. */
		Shell::Sprite *base = nullptr;
		/** Sprites displaying the tile's decoded symbols. */
		Shell::Sprite *symbols[4] = {};
		/** Feedback lights aligned with the tile's symbols. */
		Shell::Sprite *lights[4] = {};
		/** Number of feedback lights currently illuminated. */
		uint lit = 0;
		/** Active millisecond timestamp of the most recent feedback-light step. */
		uint32 stepTime = 0;
		/** Whether the tile is showing placement feedback. */
		bool feedback = false;
	};
	struct Reveal : Core::AnimationPlayer::PauseListener {
		/** Glyph machine that handles the reveal callback. */
		GlyphMachine *owner = nullptr;
		/** Glyph sprite whose reveal animation is being watched. */
		Shell::Sprite *sprite = nullptr;
		/** Board cell associated with this reveal animation. */
		uint cell = 0;
		/** Whether the cell's reveal animation has started. */
		bool started = false;
		/** Forward a reveal pause marker to the board's reveal handling. */
		bool animationPaused() override;
	};
	/** Scene displaying the tile supply, board and feedback sprites. */
	Shell::Scene &_scene;
	/** Shared gameplay random stream used when generating a new board. */
	Shell::Random *_random = nullptr;
	/** Generated symbols, placement constraints and current tile state. */
	GlyphRules _rules;
	/** Script-supplied resource IDs and layout settings indexed by property name. */
	Common::HashMap<Common::String, int32> _configMap;
	/** Resource ID used to create tile backgrounds. */
	uint _tileID = 0;
	/** Base resource ID for the tile's decoded symbol sprites. */
	uint _symbolID = 0;
	/** Resource ID used for placement-feedback lights. */
	uint _lightID = 0;
	/** Base resource ID for encoded board glyph sprites. */
	uint _glyphID = 0;
	/** Resource ID used for board-cell indentation sprites. */
	uint _indentID = 0;
	/** Resource ID used to mark the proposed drop target. */
	uint _highlightID = 0;
	/** Pixel offset applied to board-cell indentation sprites. */
	Common::Point _indentOffset;
	/** Page callback receiving tile and reveal feedback. */
	Listener *_listener = nullptr;
	/** Sprites removed when the current puzzle is cleared. */
	Common::Array<Shell::Sprite *> _owned;
	/** Queued sound resource IDs in playback order. */
	Common::Array<uint16> _sounds;
	/** Channel selections corresponding to queued sound IDs. */
	Common::Array<int> _soundChans;
	/** Tile backgrounds, symbol sprites and placement feedback state. */
	Piece _pieces[GlyphRules::kTileCount] = {};
	/** Glyph sprites indexed by board cell. */
	Shell::Sprite *_glyphs[GlyphRules::kMaximumGlyphs] = {};
	/** Sprite marking the proposed tile drop target. */
	Shell::Sprite *_highlight = nullptr;
	/** Pause callbacks and reveal progress indexed by board cell. */
	Reveal _reveals[GlyphRules::kMaximumGlyphs] = {};
	/** Index of the dragged tile, or minus one when no drag is active. */
	int _dragged = -1;
	/** Tile placement recorded before the current pickup. */
	int _prevSlot = -1;
	/** Proposed starting board cell for the dragged tile, or minus one when invalid. */
	int _target = -1;
	/** Pointer displacement from the dragged tile's origin, in pixels. */
	Common::Point _grabOffset;
	/** Tile position recorded before the current pickup. */
	Common::Point _prevPos;
	/** Tile background width in pixels. */
	int _tileW = 0;
	/** Tile background height in pixels. */
	int _tileH = 0;
	/** Width of the tile's symbol content in pixels. */
	int _contentW = 0;
	/** Active milliseconds for tile feedback, excluding paused intervals. */
	FrameClock _clock;
	/** Whether placement feedback and reveals are paused. */
	bool _paused = false;
	/** Whether a required puzzle resource could not be loaded. */
	bool _resError = false;
	/** Create a puzzle sprite at a depth with a one-based initial frame. */
	Shell::Sprite *add(uint id, int depth, int frame = 1);
	/** Remove a puzzle sprite from tracking and the scene. */
	void remove(Shell::Sprite *sprite);
	/** Move a tile and its symbol and light sprites to a position and depth. */
	void movePiece(uint index, Common::Point position, int depth);
	/** Advance the placement-feedback lights for one tile. */
	void updateLights(uint index);
	/** Choose and highlight the dragged tile's best overlapping board slot. */
	void updateTarget();
	/** Apply or reject the dragged tile's current placement. */
	void drop();
	/** Start the glyph reveal for an accepted tile. */
	void reveal(uint index);
	/** Complete the reveal step associated with a board cell. */
	void revealPaused(uint cell);
	/** Queue a puzzle sound on its selected playback channel. */
	void queueSound(uint16 sound, int channel);
	/** Calculate a glyph sprite's game-pixel position within a board cell. */
	Common::Point glyphPosition(uint cell, Shell::Sprite &sprite) const;
	/** Return the vertical distance between board rows in pixels. */
	int rowStride() const { return _tileH + _tileH / 2 + 10; }
	/** Return the active board width in pixels. */
	int boardWidth() const;
	/** Calculate the intersection area of two game-pixel rectangles. */
	static int overlapArea(const Common::Rect &first, const Common::Rect &second);
	/** Declare the puzzle's script feedback events. */
	void initializeEvents();
	/** Remove current sprites and reset drag, sound and feedback state. */
	void clearPuzzle();
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
