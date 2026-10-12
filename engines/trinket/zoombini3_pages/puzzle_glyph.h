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
	static constexpr uint kTileCount = 12;
	static constexpr uint kValueCount = 10;
	static constexpr uint kRowCount = 2;
	static constexpr uint kMaximumGlyphs = 56;
	struct Tile {
		int values[4] = {};
		int slot = -1;
		uint supplyPosition = 0;
		uint matches = 0;
		bool solved = false;
	};
	GlyphRules();
	bool generate(uint level, Shell::Random &random);
	uint level() const { return _level; }
	uint glyphCount() const { return _glyphCount; }
	uint charactersPerTile() const { return _characters; }
	uint columns() const { return _glyphCount / kRowCount; }
	bool clustered() const { return _level == 1; }
	uint allowedMistakes() const;
	int glyph(uint cell) const { return _glyphs[cell]; }
	int decodedGlyph(uint cell) const { return _alphabet[_glyphs[cell]]; }
	int occupancy(uint cell) const { return _occupancy[cell]; }
	const Tile &tile(uint index) const { return _tiles[index]; }
	bool canPlace(uint index, int cell) const;
	bool pickUp(uint index);

	/** A legal placement may be wrong; inspect the tile's matches and solved state. */
	bool place(uint index, int cell);

	/** Find a complete compatible placement without changing the board or RNG. */
	bool findSolution(int *slots) const;

private:
	struct Range {
		int start;
		int count;
	};
	uint _level = 0, _glyphCount = 0, _characters = 0;
	int _glyphs[kMaximumGlyphs] = {};
	int _occupancy[kMaximumGlyphs];
	int _alphabet[kValueCount] = {};
	Tile _tiles[kTileCount] = {};
	void addDuplicatePatterns(int *starts, Shell::Random &random);
	static void shuffleByPairs(int *values, uint count, Shell::Random &random);
	static bool solveRemaining(const GlyphRules &state, int *slots);
};

/** Movable Glyph tiles and board feedback, configured by the owning MPS. */
class GlyphMachine : public Core::Trinket {
public:
	class Listener {
	public:
		virtual ~Listener() {}
		virtual void glyphPickedUp() = 0;
		virtual void glyphPlaced(bool correct) = 0;
		virtual void glyphRevealed() = 0;
	};
	GlyphMachine(Shell::Scene &scene, const GlyphRules &rules);
	GlyphMachine(Shell::Scene &scene, Shell::Random &random, Common::Point position, int depth);
	~GlyphMachine() override;
	bool get(const Common::String &property, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool set(const Common::String &property, const Core::ScriptArguments &arguments, const Core::ScriptValue &value) override;
	bool play(const Common::String &method, const Core::ScriptArguments &arguments, Core::ScriptValue &result) override;
	bool load();
	bool event(const Common::Event &event, uint32 time);
	void update(uint32 time);
	void pause(bool paused, uint32 time);
	void forceDrop();
	void setEnabled(bool allowed) { enabled = allowed; }
	void setListener(Listener *listener) { _listener = listener; }
	bool pollSound(uint16 &sound, int *channel = nullptr);
	int hitTile(Common::Point point) const;
	bool dragging() const { return 0 <= _dragged; }
	bool hasResourceError() const { return _resourceError; }
	Common::String debugAnswer(uint level) const;
	const GlyphRules &rules() const { return _rules; }
	Common::Rect tileBounds(uint index) const;
	Common::Point slotPosition(uint cell) const;

private:
	static constexpr const char *kAnimation = "/z3a2/%u.ao";
	static constexpr const char *kDebugInput = "z3a2in.xml";
	struct Piece {
		Shell::Sprite *base = nullptr, *symbols[4] = {}, *lights[4] = {};
		uint lit = 0;
		uint32 stepTime = 0;
		bool feedback = false;
	};
	struct Reveal : Core::AnimationPlayer::PauseListener {
		GlyphMachine *owner = nullptr;
		Shell::Sprite *sprite = nullptr;
		uint cell = 0;
		bool started = false;
		bool animationPaused() override;
	};
	Shell::Scene &_scene;
	Shell::Random *_random = nullptr;
	GlyphRules _rules;
	Common::HashMap<Common::String, int32> _configuration;
	uint _tileID = 0, _symbolID = 0, _lightID = 0, _glyphID = 0, _indentID = 0, _highlightID = 0;
	Common::Point _indentOffset;
	Listener *_listener = nullptr;
	Common::Array<Shell::Sprite *> _owned;
	Common::Array<uint16> _sounds;
	Common::Array<int> _soundChannels;
	Piece _pieces[GlyphRules::kTileCount] = {};
	Shell::Sprite *_glyphs[GlyphRules::kMaximumGlyphs] = {}, *_highlight = nullptr;
	Reveal _reveals[GlyphRules::kMaximumGlyphs] = {};
	int _dragged = -1, _previousSlot = -1, _target = -1;
	Common::Point _grabOffset, _previousPosition;
	int _tileWidth = 0, _tileHeight = 0, _contentWidth = 0;
	uint32 _time = 0, _pauseTime = 0;
	bool _paused = false, _resourceError = false;
	Shell::Sprite *add(uint id, int depth, int frame = 1);
	void remove(Shell::Sprite *sprite);
	void movePiece(uint index, Common::Point position, int depth);
	void updateLights(uint index);
	void updateTarget();
	void drop();
	void reveal(uint index);
	void revealPaused(uint cell);
	void queueSound(uint16 sound, int channel);
	Common::Point glyphPosition(uint cell, Shell::Sprite &sprite) const;
	int rowStride() const { return _tileHeight + _tileHeight / 2 + 10; }
	int boardWidth() const;
	static int overlapArea(const Common::Rect &first, const Common::Rect &second);
	void initializeEvents();
	void clearPuzzle();
};

} // End of namespace Zoombini3
} // End of namespace Trinket

#endif
