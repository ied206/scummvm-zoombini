/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers. Please refer to the
 * COPYRIGHT file distributed with this source distribution.
 * This program is free software, distributed under the GNU General Public
 * License, version 3 or later. See <http://www.gnu.org/licenses/>.
 */

#include "trinket/zoombini3_pages/puzzle_glyph.h"

#include "common/array.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "common/util.h"
#include "trinket/shell/random.h"
#include "trinket/shell/xml_file.h"
#include "trinket/trinket.h"

namespace Trinket {
namespace Zoombini3 {

constexpr const char *GlyphMachine::kValueCountName;
constexpr const char *GlyphMachine::kGlyphCountName;
constexpr const char *GlyphMachine::kRowCountName;
constexpr const char *GlyphMachine::kCharCountPerTileName;
constexpr const char *GlyphMachine::kTileCountName;
constexpr const char *GlyphMachine::kEventTilePickedUp;
constexpr const char *GlyphMachine::kEventTileDropped;
constexpr const char *GlyphMachine::kEventTileSolved;
constexpr const char *GlyphMachine::kEventTileWrong;
constexpr const char *GlyphMachine::kEventAnimFinished;
constexpr const char *GlyphMachine::kPropertyEnabled;
constexpr const char *GlyphMachine::kClusterName;
constexpr const char *GlyphMachine::kDoStampName;
constexpr const char *GlyphMachine::kNeedToRestoreStampName;
constexpr const char *GlyphMachine::kPieceGrabAOCursorIDName;
constexpr const char *GlyphMachine::kPieceSpotAOCursorIDName;
constexpr const char *GlyphMachine::kDebugModeName;
constexpr const char *GlyphMachine::kMethodDbgReadPuzzleAttributes;
constexpr const char *GlyphMachine::kMethodSelectNode;
constexpr const char *GlyphMachine::kMethodZ3A2Data;
constexpr const char *GlyphMachine::kPropertyNodeTag;
constexpr const char *GlyphMachine::kPropertyAttributeValue;
constexpr const char *GlyphMachine::kMethodPause;
constexpr const char *GlyphMachine::kMethodResume;
constexpr const char *GlyphMachine::kMethodForceDrop;
constexpr const char *GlyphMachine::kMethodClearPuzzle;
constexpr const char *GlyphMachine::kMethodGeneratePuzzle;
constexpr const char *GlyphMachine::kMethodSetIndentAO;
constexpr const char *GlyphMachine::kMethodSetTileAO;
constexpr const char *GlyphMachine::kMethodSetTileCharactersAO;
constexpr const char *GlyphMachine::kMethodSetTileLightsAO;
constexpr const char *GlyphMachine::kMethodSetGlyphsAO;
constexpr const char *GlyphMachine::kMethodSetHighlightAO;
constexpr const char *GlyphMachine::kNameSignedIndexFormat;

bool GlyphRules::findSolution(int *slots) const {
	GlyphRules state = *this;
	for (uint i = 0; i < kTileCount; i++) {
		slots[i] = state.tile(i).solved ? state.tile(i).slot : -1;
		if (!state.tile(i).solved)
			state.pickUp(i);
	}
	return solveRemaining(state, slots);
}

bool GlyphRules::solveRemaining(const GlyphRules &state, int *slots) {
	uint selected = kTileCount;
	uint fewest = kMaximumGlyphs + 1;
	int candidates[kMaximumGlyphs];
	for (uint i = 0; i < kTileCount; i++) {
		if (state.tile(i).solved)
			continue;
		int available[kMaximumGlyphs];
		uint count = 0;
		for (uint cell = 0; cell < state.glyphCount(); cell++) {
			if (!state.canPlace(i, cell))
				continue;
			bool matches = true;
			for (uint j = 0; j < state.charactersPerTile(); j++)
				matches = matches && state.tile(i).values[j] == state.decodedGlyph(cell + j);
			if (matches)
				available[count++] = cell;
		}
		if (!count)
			return false;
		if (count < fewest) {
			selected = i;
			fewest = count;
			for (uint j = 0; j < count; j++)
				candidates[j] = available[j];
		}
	}
	if (selected == kTileCount)
		return true;
	for (uint i = 0; i < fewest; i++) {
		GlyphRules next = state;
		next.place(selected, candidates[i]);
		slots[selected] = candidates[i];
		if (solveRemaining(next, slots))
			return true;
	}
	slots[selected] = -1;
	return false;
}

GlyphRules::GlyphRules() {
	memset(_occupancy, -1, sizeof(_occupancy));
	for (uint i = 0; i < kTileCount; i++)
		_tiles[i].slot = -1;
}

void GlyphRules::shuffleByPairs(int *values, uint count, Shell::Random &random) {
	for (uint i = 0; i < count; i++) {
		uint first = random.draw() % count;
		uint second = random.draw() % count;
		SWAP(values[first], values[second]);
	}
}

bool GlyphRules::generate(uint level, Shell::Random &random) {
	if (level < 1 || 3 < level)
		return false;
	_level = level;
	_glyphCount = level == 2 ? 56 : 48;
	_characters = level == 3 ? 3 : 4;
	for (uint i = 0; i < kMaximumGlyphs; i++) {
		_glyphs[i] = -1;
		_occupancy[i] = -1;
	}
	for (uint i = 0; i < kValueCount; i++)
		_glyphs[i] = i;
	// Seed each symbol once before filling the remaining cells.
	for (uint i = 0; i < kValueCount; i++) {
		uint other = random.draw() % _glyphCount;
		SWAP(_glyphs[i], _glyphs[other]);
	}
	uint cell = 0;
	for (uint i = 0; i < _glyphCount - kValueCount; i++) {
		while (0 <= _glyphs[cell])
			cell += 1;
		_glyphs[cell] = random.draw() % kValueCount;
		cell += 1;
	}
	for (uint i = 0; i < kValueCount; i++)
		_alphabet[i] = i;
	shuffleByPairs(_alphabet, kValueCount, random);
	int starts[kTileCount];
	int positions[kTileCount];
	for (uint i = 0; i < kTileCount; i++) {
		starts[i] = i * _characters;
		positions[i] = i;
	}
	if (kTileCount * _characters < _glyphCount)
		addDuplicatePatterns(starts, random);
	shuffleByPairs(positions, kTileCount, random);
	for (uint i = 0; i < kTileCount; i++) {
		Tile &tile = _tiles[i];
		for (uint j = 0; j < 4; j++)
			tile.values[j] = j < _characters ? decodedGlyph(starts[i] + j) : -1;
		tile.slot = -1;
		tile.supplyPosition = positions[i];
		tile.matches = 0;
		tile.solved = false;
	}
	return true;
}

void GlyphRules::addDuplicatePatterns(int *starts, Shell::Random &random) {
	// Mark gaps, tile starts and continuations separately while choosing duplicates.
	enum Marker {
		kEmpty,
		kGap,
		kTileStart,
		kTileRest,
		kDuplicateStart,
		kDuplicateRest,
	};
	int markers[kMaximumGlyphs] = {};
	static constexpr uint tilesPerRow = kTileCount / kRowCount;
	uint gaps = (_glyphCount - _characters * kTileCount) / kRowCount;
	uint cell = 0;
	uint tile = 0;
	for (uint row = 0; row < kRowCount; row++) {
		uint gapCounts[tilesPerRow + 1] = {};
		for (uint i = 0; i < gaps; i++)
			gapCounts[random.draw() % (tilesPerRow + 1)] += 1;
		for (uint gap = 0; gap <= tilesPerRow; gap++) {
			for (uint i = 0; i < gapCounts[gap]; i++) {
				markers[cell] = kGap;
				cell += 1;
			}
			if (gap < tilesPerRow) {
				starts[tile] = cell;
				tile += 1;
				markers[cell] = kTileStart;
				cell += 1;
				for (uint i = 1; i < _characters; i++) {
					markers[cell] = kTileRest;
					cell += 1;
				}
			}
		}
	}
	int selected[kTileCount];
	for (uint i = 0; i < kTileCount; i++)
		selected[i] = i;
	shuffleByPairs(selected, kTileCount, random);
	static constexpr uint duplicateCount = tilesPerRow / 2;
	for (uint i = 0; i < duplicateCount; i++) {
		uint start = starts[selected[i]];
		markers[start] = kDuplicateStart;
		for (uint j = 1; j < _characters; j++)
			markers[start + j] = kDuplicateRest;
	}
	for (uint duplicate = 0; duplicate < duplicateCount; duplicate++) {
		Common::Array<Range> ranges;
		int total = 0;
		for (uint row = 0; row < kRowCount; row++) {
			int first = row * columns();
			int available = 0;
			uint position = first;
			while (position < (row + 1) * columns()) {
				if (markers[position] == kDuplicateStart) {
					if (static_cast<int>(_characters) <= available) {
						Range range = {first, available - static_cast<int>(_characters) + 1};
						ranges.push_back(range);
						total += range.count;
					}
					position += _characters;
					first = position;
					available = 0;
				} else {
					if (markers[position] != kTileStart)
						available += 1;
					position += 1;
				}
			}
			if (static_cast<int>(_characters) <= available) {
				Range range = {first, available - static_cast<int>(_characters) + 1};
				ranges.push_back(range);
				total += range.count;
			}
		}
		assert(0 < total);
		int choice = random.draw() % total + 1;
		for (uint i = 0; i < ranges.size(); i++) {
			if (ranges[i].count < choice) {
				choice -= ranges[i].count;
				continue;
			}
			uint destination = ranges[i].start;
			for (int j = 0; j < choice; j++) {
				while (markers[destination] == kTileStart)
					destination += 1;
				destination += 1;
			}
			destination -= 1;
			uint source = starts[selected[duplicate]];
			for (uint j = 0; j < _characters; j++)
				_glyphs[destination + j] = _glyphs[source + j];
			markers[destination] = kDuplicateStart;
			for (uint j = 1; j < _characters; j++)
				markers[destination + j] = kDuplicateRest;
			break;
		}
	}
}

uint GlyphRules::allowedMistakes() const {
	static constexpr uint limits[] = {
		0,
		5,
		3,
		2,
	};
	return limits[_level];
}

bool GlyphRules::canPlace(uint index, int cell) const {
	if (!_level || kTileCount <= index || _tiles[index].solved || cell < 0 || _glyphCount <= static_cast<uint>(cell))
		return false;
	if (columns() < cell % columns() + _characters || (clustered() && cell % _characters))
		return false;
	for (uint i = 0; i < _characters; i++) {
		if (0 <= _occupancy[cell + i])
			return false;
	}
	return true;
}

bool GlyphRules::pickUp(uint index) {
	if (!_level || kTileCount <= index || _tiles[index].solved)
		return false;
	Tile &tile = _tiles[index];
	if (0 <= tile.slot) {
		for (uint i = 0; i < _characters; i++)
			_occupancy[tile.slot + i] = -1;
		tile.slot = -1;
		tile.matches = 0;
	}
	return true;
}

bool GlyphRules::place(uint index, int cell) {
	if (!canPlace(index, cell) || 0 <= _tiles[index].slot)
		return false;
	Tile &tile = _tiles[index];
	tile.slot = cell;
	tile.matches = 0;
	for (uint i = 0; i < _characters; i++) {
		_occupancy[cell + i] = tile.values[i];
		if (tile.values[i] == decodedGlyph(cell + i))
			tile.matches += 1;
	}
	tile.solved = tile.matches == _characters;
	return true;
}

constexpr const char *GlyphMachine::kAnimation;

GlyphMachine::GlyphMachine(Shell::Scene &scene, const GlyphRules &rules) : _scene(scene), _rules(rules) {
	x = 180;
	y = 100;
	z = 100;
	initializeEvents();
	_tileID = rules.charactersPerTile() == 3 ? 12431 : 12425;
	_symbolID = 12400;
	_lightID = 12410;
	_glyphID = 12401;
	if (rules.clustered())
		_indentID = 12427;
	else if (rules.charactersPerTile() == 3)
		_indentID = 12430;
	else
		_indentID = 12428;
	_highlightID = rules.charactersPerTile() == 3 ? 12433 : 12429;
	_indentOffset = Common::Point(rules.clustered() ? 3 : 5, rules.clustered() ? 4 : 3);
	for (uint i = 0; i < ARRAYSIZE(_reveals); i++) {
		_reveals[i].owner = this;
		_reveals[i].cell = i;
	}
}

GlyphMachine::GlyphMachine(Shell::Scene &scene, Shell::Random &random, Common::Point position, int depth) : _scene(scene), _random(&random) {
	x = position.x;
	y = position.y;
	z = depth;
	initializeEvents();
	static constexpr const char *properties[] = {
		kValueCountName,
		kGlyphCountName,
		kRowCountName,
		kCharCountPerTileName,
		kTileCountName,
	};
	for (uint i = 0; i < ARRAYSIZE(properties); i++)
		_configMap[properties[i]] = 1;
	for (uint i = 0; i < ARRAYSIZE(_reveals); i++) {
		_reveals[i].owner = this;
		_reveals[i].cell = i;
	}
}

GlyphMachine::~GlyphMachine() {
	clearPuzzle();
}

void GlyphMachine::initializeEvents() {
	static constexpr const char *events[] = {
		kEventTilePickedUp,
		kEventTileDropped,
		kEventTileSolved,
		kEventTileWrong,
		kEventAnimFinished,
	};
	for (uint i = 0; i < ARRAYSIZE(events); i++)
		addEvent(events[i]);
}

void GlyphMachine::clearPuzzle() {
	for (uint i = 0; i < _owned.size(); i++)
		_scene.remove(_owned[i]);
	_owned.clear();
	_sounds.clear();
	_soundChans.clear();
	for (uint i = 0; i < ARRAYSIZE(_pieces); i++)
		_pieces[i] = Piece();
	for (uint i = 0; i < ARRAYSIZE(_glyphs); i++) {
		_glyphs[i] = nullptr;
		_reveals[i].sprite = nullptr;
		_reveals[i].started = false;
	}
	_highlight = nullptr;
	_dragged = _prevSlot = _target = -1;
}

bool GlyphMachine::get(const Common::String &property, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	if (property.equals(kPropertyEnabled))
		result = Core::ScriptValue(static_cast<int32>(enabled));
	else if (_configMap.contains(property))
		result = Core::ScriptValue(_configMap[property]);
	else
		return Core::Trinket::get(property, sargs, result);
	return sargs.empty();
}

bool GlyphMachine::set(const Common::String &property, const Core::ScriptArguments &sargs, const Core::ScriptValue &value) {
	if (!sargs.empty())
		return Core::Trinket::set(property, sargs, value);
	if (property.equals(kPropertyEnabled)) {
		setEnabled(value.toInt32() != 0);
		return true;
	}
	static constexpr const char *properties[] = {
		kValueCountName,
		kGlyphCountName,
		kRowCountName,
		kCharCountPerTileName,
		kTileCountName,
		kClusterName,
		kDoStampName,
		kNeedToRestoreStampName,
		kPieceGrabAOCursorIDName,
		kPieceSpotAOCursorIDName,
		kDebugModeName,
	};
	for (uint i = 0; i < ARRAYSIZE(properties); i++) {
		if (property.equals(properties[i])) {
			_configMap[property] = value.toInt32();
			return true;
		}
	}
	return Core::Trinket::set(property, sargs, value);
}

bool GlyphMachine::play(const Common::String &method, const Core::ScriptArguments &sargs, Core::ScriptValue &result) {
	_clock.update(callTime(), !_paused);
	if (sargs.empty()) {
		if (method.equals(kMethodDbgReadPuzzleAttributes)) {
			if (!_configMap[kDebugModeName])
				return true;
			Shell::XMLFile file;
			if (!file.load(kDebugInput))
				return false;
			file.play(kMethodSelectNode, {Core::ScriptValue(Common::String(kMethodZ3A2Data))}, result);
			file.get(kPropertyNodeTag, {}, result);
			if (!result.toText().equals(kMethodZ3A2Data))
				return true;
			clearPuzzle();
			static constexpr const char *properties[] = {
				kValueCountName,
				kRowCountName,
				kGlyphCountName,
				kTileCountName,
				kCharCountPerTileName,
				kClusterName,
			};
			for (const char *property : properties) {
				file.get(kPropertyAttributeValue, {Core::ScriptValue(Common::String(property))}, result);
				_configMap[property] = result.toInt32();
			}
			return true;
		}
		if (method.equals(kMethodPause) || method.equals(kMethodResume)) {
			pause(method.equals(kMethodPause), callTime());
			return true;
		}
		if (method.equals(kMethodForceDrop)) {
			forceDrop();
			return true;
		}
		if (method.equals(kMethodClearPuzzle)) {
			clearPuzzle();
			return true;
		}
		if (method.equals(kMethodGeneratePuzzle)) {
			if (!_random || _configMap[kValueCountName] != 10 || _configMap[kRowCountName] != 2 || _configMap[kTileCountName] != 12)
				return false;
			uint level = 0;
			for (uint candidate = 1; candidate <= 3; candidate++) {
				if (_configMap[kGlyphCountName] == (candidate == 2 ? 56 : 48) &&
					_configMap[kCharCountPerTileName] == (candidate == 3 ? 3 : 4) &&
					(_configMap[kClusterName] != 0) == (candidate == 1))
					level = candidate;
			}
			if (!level)
				return false;
			clearPuzzle();
			_resError = false;
			return _rules.generate(level, *_random) && load();
		}
	}
	if (method.equals(kMethodSetIndentAO) && sargs.size() == 3) {
		_indentID = sargs[0].toInt32();
		_indentOffset = Common::Point(sargs[1].toInt32(), sargs[2].toInt32());
		return true;
	}
	if (sargs.size() == 1) {
		uint *resource = nullptr;
		if (method.equals(kMethodSetTileAO))
			resource = &_tileID;
		else if (method.equals(kMethodSetTileCharactersAO))
			resource = &_symbolID;
		else if (method.equals(kMethodSetTileLightsAO))
			resource = &_lightID;
		else if (method.equals(kMethodSetGlyphsAO))
			resource = &_glyphID;
		else if (method.equals(kMethodSetHighlightAO))
			resource = &_highlightID;
		if (resource) {
			*resource = sargs[0].toInt32();
			return true;
		}
	}
	return Core::Trinket::play(method, sargs, result);
}

Shell::Sprite *GlyphMachine::add(uint id, int depth, int frame) {
	Shell::Sprite *sprite = _scene.add(Common::String::format(kAnimation, id), depth);
	if (!sprite) {
		_resError = true;
		return nullptr;
	}
	_owned.push_back(sprite);
	sprite->enabled = false;
	sprite->setFrame(frame);
	return sprite;
}

void GlyphMachine::remove(Shell::Sprite *sprite) {
	for (uint i = 0; i < _owned.size(); i++) {
		if (_owned[i] == sprite) {
			_owned.remove_at(i);
			_scene.remove(sprite);
			return;
		}
	}
}

int GlyphMachine::boardWidth() const {
	uint groups = (_rules.columns() + _rules.charactersPerTile() - 1) / _rules.charactersPerTile();
	return groups * _contentW + (_rules.clustered() ? (groups - 1) * 10 : 0);
}

Common::Point GlyphMachine::slotPosition(uint cell) const {
	uint column = cell % _rules.columns();
	uint group = column / _rules.charactersPerTile();
	float width = static_cast<float>(_contentW) / _rules.charactersPerTile();
	int anchorX = x + group * _contentW + static_cast<int>((column % _rules.charactersPerTile()) * width) - 1;
	if (_rules.clustered())
		anchorX += group * 10;
	return Common::Point(anchorX, y + _tileH / 2 + cell / _rules.columns() * rowStride());
}

Common::Point GlyphMachine::glyphPosition(uint cell, Shell::Sprite &sprite) const {
	uint column = cell % _rules.columns();
	uint group = column / _rules.charactersPerTile();
	float width = static_cast<float>(_contentW) / _rules.charactersPerTile();
	int anchorX = x + group * _contentW + static_cast<int>((column % _rules.charactersPerTile()) * width + width * 0.5f - sprite.width() * 0.5f);
	if (_rules.clustered())
		anchorX += group * 10;
	int anchorY = y + _tileH / 2 - (_tileH / 2) / 2 + cell / _rules.columns() * rowStride();
	anchorY += (column % 2 ? 4 : -4) - sprite.height() / 2;
	return Common::Point(anchorX, anchorY);
}

bool GlyphMachine::load() {
	static constexpr int supplyX[] = {
		197,
		303,
		408,
		446,
		574,
		690,
		106,
		223,
		325,
		428,
		544,
		644,
	};
	static constexpr int supplyY[] = {
		422,
		421,
		390,
		448,
		453,
		450,
		525,
		503,
		520,
		520,
		519,
		523,
	};
	for (uint i = 0; i < GlyphRules::kTileCount; i++) {
		Piece &piece = _pieces[i];
		piece.base = add(_tileID, z);
		if (!piece.base)
			return false;
		_tileW = piece.base->width();
		_tileH = piece.base->height();
		_contentW = _tileW - 3;
		float width = static_cast<float>(_contentW) / _rules.charactersPerTile();
		for (uint j = 0; j < _rules.charactersPerTile(); j++) {
			Shell::Sprite *symbol = add(_symbolID, z, _rules.tile(i).values[j] + 1);
			if (!symbol)
				return false;
			symbol->x = piece.base->x + 1 + static_cast<int>(j * width + width * 0.5f - symbol->width() * 0.5f);
			symbol->y = piece.base->y + 16 - symbol->height() / 2;
			piece.symbols[j] = symbol;
			if (_rules.clustered()) {
				piece.lights[j] = add(_lightID, z, 2 * j + 1);
				if (!piece.lights[j])
					return false;
			}
		}
		uint position = _rules.tile(i).supplyPosition;
		movePiece(i, Common::Point(supplyX[position], supplyY[position]), supplyY[position] + _tileH + z - 375);
	}
	for (uint row = 0; row < GlyphRules::kRowCount; row++) {
		uint groups = _rules.clustered() ? _rules.columns() / _rules.charactersPerTile() : 1;
		for (uint group = 0; group < groups; group++) {
			Shell::Sprite *indent = add(_indentID, z);
			if (!indent)
				return false;
			indent->x = x + group * (_contentW + 10) - _indentOffset.x;
			indent->y = y + _tileH / 2 + row * rowStride() - _indentOffset.y;
		}
	}
	for (uint i = 0; i < _rules.glyphCount(); i++) {
		_glyphs[i] = add(_glyphID, z, _rules.glyph(i) + 1);
		if (!_glyphs[i])
			return false;
		Common::Point position = glyphPosition(i, *_glyphs[i]);
		_glyphs[i]->x = position.x;
		_glyphs[i]->y = position.y;
	}
	_highlight = add(_highlightID, z);
	if (!_highlight)
		return false;
	_highlight->visible = false;
	return true;
}

void GlyphMachine::movePiece(uint index, Common::Point position, int depth) {
	Piece &piece = _pieces[index];
	int dx = position.x - piece.base->x;
	int dy = position.y - piece.base->y;
	Shell::Sprite *members[9] = {
		piece.base,
		piece.symbols[0],
		piece.symbols[1],
		piece.symbols[2],
		piece.symbols[3],
		piece.lights[0],
		piece.lights[1],
		piece.lights[2],
		piece.lights[3],
	};
	for (uint i = 0; i < ARRAYSIZE(members); i++) {
		if (!members[i])
			continue;
		members[i]->x += dx;
		members[i]->y += dy;
		_scene.setDepth(members[i], depth);
	}
}

Common::Rect GlyphMachine::tileBounds(uint index) const {
	const Shell::Sprite *base = _pieces[index].base;
	return Common::Rect(base->x, base->y, base->x + _tileW, base->y + _tileH);
}

int GlyphMachine::hitTile(Common::Point point) const {
	if (!enabled || _paused)
		return -1;
	int hit = -1;
	for (uint i = 0; i < GlyphRules::kTileCount; i++) {
		if (!_rules.tile(i).solved && tileBounds(i).contains(point) && (hit < 0 || _pieces[hit].base->z <= _pieces[i].base->z))
			hit = i;
	}
	return hit;
}

int GlyphMachine::overlapArea(const Common::Rect &first, const Common::Rect &second) {
	return MAX<int>(0, MIN(first.right, second.right) - MAX(first.left, second.left)) *
		   MAX<int>(0, MIN(first.bottom, second.bottom) - MAX(first.top, second.top));
}

void GlyphMachine::updateTarget() {
	_target = -1;
	int row = -1;
	int area = 0;
	Common::Rect tile = tileBounds(_dragged);
	for (uint i = 0; i < GlyphRules::kRowCount; i++) {
		int anchorY = y + _tileH / 2 + i * rowStride();
		int candidate = overlapArea(tile, Common::Rect(x, anchorY, x + boardWidth(), anchorY + _tileH));
		if (area < candidate) {
			area = candidate;
			row = i;
		}
	}
	if (0 <= row) {
		if (_rules.clustered()) {
			area = 0;
			for (uint column = 0; column < _rules.columns(); column += _rules.charactersPerTile()) {
				int cell = row * _rules.columns() + column;
				Common::Point position = slotPosition(cell);
				int candidate = overlapArea(tile, Common::Rect(position.x + 1, position.y, position.x + 1 + _contentW, position.y + _tileH));
				if (area < candidate && _rules.canPlace(_dragged, cell)) {
					area = candidate;
					_target = cell;
				}
			}
		} else {
			float width = static_cast<float>(_contentW) / _rules.charactersPerTile();
			int column = static_cast<int>((tile.left + 1 + width * 0.5f - x) / width);
			column = CLIP<int>(column, 0, _rules.columns() - _rules.charactersPerTile());
			int cell = row * _rules.columns() + column;
			if (_rules.canPlace(_dragged, cell))
				_target = cell;
		}
	}
	_highlight->visible = 0 <= _target;
	if (0 <= _target) {
		Common::Point position = slotPosition(_target);
		_highlight->x = position.x;
		_highlight->y = position.y;
	}
}

bool GlyphMachine::event(const Common::Event &event, uint32 time) {
	_clock.update(time, !_paused);
	if (_paused)
		return false;
	if (event.type == Common::EVENT_LBUTTONDOWN && _dragged < 0) {
		int tile = hitTile(event.mouse);
		if (tile < 0)
			return false;
		_dragged = tile;
		_prevSlot = _rules.tile(tile).slot;
		_prevPos = Common::Point(_pieces[tile].base->x, _pieces[tile].base->y);
		_grabOffset = event.mouse - _prevPos;
		_rules.pickUp(tile);
		_pieces[tile].feedback = false;
		_pieces[tile].lit = 0;
		updateLights(tile);
		movePiece(tile, _prevPos, 10000);
		updateTarget();
		queueSound(12408, 0);
		if (_listener)
			_listener->glyphPickedUp();
		emitEvent(kEventTilePickedUp);
		return true;
	}
	if (0 <= _dragged && (event.type == Common::EVENT_MOUSEMOVE || event.type == Common::EVENT_LBUTTONUP)) {
		movePiece(_dragged, event.mouse - _grabOffset, 10000);
		updateTarget();
		if (event.type == Common::EVENT_LBUTTONUP)
			drop();
		return true;
	}
	return false;
}

void GlyphMachine::forceDrop() {
	if (_dragged < 0)
		return;
	if (0 <= _prevSlot) {
		_rules.place(_dragged, _prevSlot);
		movePiece(_dragged, slotPosition(_prevSlot), z + 1);
	} else {
		movePiece(_dragged, _prevPos, _prevPos.y + _tileH + z - 375);
	}
	_pieces[_dragged].lit = _rules.tile(_dragged).matches;
	updateLights(_dragged);
	_dragged = -1;
	_highlight->visible = false;
}

void GlyphMachine::drop() {
	uint tile = _dragged;
	_highlight->visible = false;
	if (_target < 0) {
		Common::Rect bounds = tileBounds(tile);
		if (!overlapArea(bounds, Common::Rect(106, 375, 800, 600))) {
			forceDrop();
		} else {
			Common::Point position(CLIP<int>(bounds.left, 106, 800 - _tileW), CLIP<int>(bounds.top, 375, 600 - _tileH));
			movePiece(tile, position, position.y + _tileH + z - 375);
			_dragged = -1;
		}
		queueSound(12409, 0);
		emitEvent(kEventTileDropped);
		return;
	}
	_rules.place(tile, _target);
	movePiece(tile, slotPosition(_target), z + 1);
	_dragged = -1;
	queueSound(12407, 0);
	Piece &piece = _pieces[tile];
	piece.lit = 0;
	piece.feedback = _rules.clustered() && 0 < _rules.tile(tile).matches;
	piece.stepTime = _clock.now() - 500;
	updateLights(tile);
	bool correct = _rules.tile(tile).solved;
	if (correct && !_rules.clustered())
		reveal(tile);
	if (_listener)
		_listener->glyphPlaced(correct);
	emitEvent(correct ? kEventTileSolved : kEventTileWrong);
	emitEvent(kEventTileDropped);
}

void GlyphMachine::updateLights(uint index) {
	Piece &piece = _pieces[index];
	if (!_rules.clustered())
		return;
	for (uint i = 0; i < _rules.charactersPerTile(); i++)
		piece.lights[i]->setFrame(2 * i + (i < piece.lit ? 2 : 1));
}

void GlyphMachine::reveal(uint index) {
	uint first = _rules.tile(index).slot;
	for (uint i = 0; i < _rules.charactersPerTile(); i++) {
		uint cell = first + i;
		Reveal &reveal = _reveals[cell];
		reveal.sprite = add(12411 + _rules.glyph(cell), 100);
		if (!reveal.sprite)
			return;
		reveal.sprite->setRepeat(1);
		reveal.sprite->setPauseListener(&reveal);
		reveal.started = false;
		Common::Point position = glyphPosition(cell, *reveal.sprite);
		reveal.sprite->x = position.x;
		reveal.sprite->y = position.y;
		_glyphs[cell]->visible = false;
	}
	queueSound(12410, 1);
}

bool GlyphMachine::Reveal::animationPaused() {
	owner->revealPaused(cell);
	return false;
}

void GlyphMachine::revealPaused(uint cell) {
	int first = -1;
	for (uint i = 0; i < GlyphRules::kTileCount; i++) {
		int slot = _rules.tile(i).slot;
		if (0 <= slot && slot <= static_cast<int>(cell) && cell < slot + _rules.charactersPerTile()) {
			first = slot;
			break;
		}
	}
	if (first < 0)
		return;
	uint count = 0;
	for (uint i = 0; i < _rules.charactersPerTile(); i++) {
		Reveal &reveal = _reveals[first + i];
		if (reveal.sprite) {
			count += 1;
			if (!reveal.sprite->isRunning()) {
				remove(reveal.sprite);
				reveal.sprite = nullptr;
				_glyphs[first + i]->visible = true;
			}
		}
	}
	if (count == _rules.charactersPerTile()) {
		if (_listener)
			_listener->glyphRevealed();
		emitEvent(kEventAnimFinished);
	}
}

void GlyphMachine::update(uint32 time) {
	_clock.update(time, !_paused);
	if (!_paused) {
		for (uint i = 0; i < GlyphRules::kTileCount; i++) {
			Piece &piece = _pieces[i];
			if (!piece.feedback || _clock.elapsed(piece.stepTime) < 500)
				continue;
			piece.lit += 1;
			queueSound(12399 + piece.lit, 2 + i);
			updateLights(i);
			if (piece.lit == _rules.tile(i).matches) {
				piece.feedback = false;
				if (_rules.tile(i).solved)
					reveal(i);
			} else {
				piece.stepTime = _clock.now();
			}
		}
	}
	_scene.update(time, _sounds);
	while (_soundChans.size() < _sounds.size())
		_soundChans.push_back(-1);
	if (!_paused) {
		for (uint i = 0; i < _rules.glyphCount(); i++) {
			Reveal &reveal = _reveals[i];
			if (reveal.sprite && !reveal.started) {
				reveal.started = true;
				reveal.sprite->pauseAnimation(false);
			}
		}
	}
}

void GlyphMachine::pause(bool paused, uint32 time) {
	if (_paused == paused)
		return;
	_clock.update(time, !_paused);
	_paused = paused;
	for (uint i = 0; i < _rules.glyphCount(); i++) {
		if (_reveals[i].sprite && _reveals[i].started)
			_reveals[i].sprite->pauseAnimation(paused);
	}
}

void GlyphMachine::queueSound(uint16 sound, int channel) {
	_sounds.push_back(sound);
	_soundChans.push_back(channel);
}

bool GlyphMachine::pollSound(uint16 &sound, int *channel) {
	if (_sounds.empty())
		return false;
	sound = _sounds[0];
	if (channel)
		*channel = _soundChans[0];
	_sounds.remove_at(0);
	_soundChans.remove_at(0);
	return true;
}

Common::String GlyphMachine::debugAnswer(uint level) const {
	const GlyphRules &rules = this->rules();
	int slots[GlyphRules::kTileCount];
	Common::String result = Common::String::format("The Wall: level %u, %u symbols per tile. Rows and columns are one-based.\n",
												   level, rules.charactersPerTile());
	if (!rules.findSolution(slots))
		return result + "No complete solution remains with the locked tiles. Restart this round to recover.\n";
	for (uint i = 0; i < GlyphRules::kTileCount; i++) {
		const GlyphRules::Tile &tile = rules.tile(i);
		Common::Rect bounds = tileBounds(i);
		Common::Point target = slotPosition(slots[i]);
		result += Common::String::format("Tile %u%s [", i + 1, tile.solved ? " solved" : "");
		for (uint j = 0; j < rules.charactersPerTile(); j++)
			result += Common::String::format(kNameSignedIndexFormat, j ? "," : "", tile.values[j]);
		result += Common::String::format("] -> row %u column %u; drag (%d,%d) to (%d,%d)\n",
										 slots[i] / rules.columns() + 1, slots[i] % rules.columns() + 1, bounds.left + 10, bounds.top + 10, target.x + 10, target.y + 10);
	}
	return result;
}

} // namespace Zoombini3
} // namespace Trinket
