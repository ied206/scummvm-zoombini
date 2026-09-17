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

#include "zoombini2/pages/puzzle_snowboard.h"
#include "common/debug.h"
#include "zoombini2/graphics.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleSnowboard::kMusicPath;
constexpr const char *PuzzleSnowboard::kTraitFormat;
constexpr const char *PuzzleSnowboard::kBoardPath;
constexpr const char *PuzzleSnowboard::kBoardAnimPath;
constexpr const char *PuzzleSnowboard::kEngineAnimPath;
constexpr const char *PuzzleSnowboard::kDecorFormat;

// ============================================================================
// PuzzleSnowboard - binary decision tree classifier.
//
// Core algorithm:
//   - Tree nodes stored in array, each with: traitIndex, matchVal1, matchVal2
//   - A match selects the left child; a nonmatch selects the right child.
//   - The leaf index selects the destination snowboard lane.
//   - Level 3 accepts matchVal1 OR matchVal2
// ============================================================================

// Number of lanes indexed by level, with index zero unused.
static constexpr int kLanesByLevel[] = {0, 2, 4, 4}; // level 0(unused), 1, 2, 3

PuzzleSnowboard::PuzzleSnowboard(Zoombini2Engine *vm)
	: PuzzleBase(vm, kPageSnowboard) {
}

PuzzleSnowboard::~PuzzleSnowboard() {
	if (_musicId >= 0) {
		SoundManager *snd = _vm->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	for (int f = 0; f < ZmbTrait::kTraitCount; f++) {
		for (int v = 0; v < ZmbTrait::kTraitValueCount; v++) {
			delete _traitIcons[f][v];
		}
	}
	delete _boardBitmap;
	delete _boardAnim;
	delete _engineAnim;
	for (int i = 0; i < 5; i++) {
		delete _decorAnims[i];
	}
}

void PuzzleSnowboard::init() {
	// Call base init for background and zoombini loading
	PuzzleBase::init();

	// Start the Snowboard Gulch music.
	if (SoundManager *snd = _vm->getSoundManager()) {
		_musicId = snd->load(true, Common::Path(kMusicPath), true);
		if (_musicId >= 0) {
			snd->playLoop(_musicId);
			snd->setVolume(_musicId, snd->_volumeMusic);
		}
	}

	int level = CLIP(_vm->getGameState()->_level, 1, 3);
	debug(1, "PuzzleSnowboard::init - level %d", level);
	_numLanes = kLanesByLevel[level];
	_treeDepth = _numLanes - 1; // Binary tree: depth = numLeaves - 1

	// Load trait icons
	loadLaneGraphics();

	// Generate the decision tree
	generateTree();

	// Assign zoombinis to lanes
	assignZoombinisToLanes();

	_currentZoombini = 0;
	_state = kStateSliding;
	_stateTimer = _vm->getGameTickCount();
}

void PuzzleSnowboard::loadLaneGraphics() {
	// Load trait icons: bmp/snowboard/traits/{trait}-{value}.rb
	for (int f = 0; f < ZmbTrait::kTraitCount; f++) {
		for (int v = 0; v < ZmbTrait::kTraitValueCount; v++) {
			Common::Path path(Common::String::format(kTraitFormat, f + 1, v + 1));
			_traitIcons[f][v] = new RleBlock(_vm);
			if (!_traitIcons[f][v]->loadFromFile(path)) {
				debug(1, "PuzzleSnowboard: Failed to load trait %d-%d", f + 1, v + 1);
				delete _traitIcons[f][v];
				_traitIcons[f][v] = nullptr;
			}
		}
	}

	// Load board graphics
	Common::Path boardPath(kBoardPath);
	_boardBitmap = new BitBlock(_vm);
	if (!_boardBitmap->load(boardPath)) {
		delete _boardBitmap;
		_boardBitmap = nullptr;
	}

	// Load board animation (BOARD.AN)
	Common::Path boardAnimPath(kBoardAnimPath);
	_boardAnim = new Animation(_vm);
	if (!_boardAnim->loadFromFile(boardAnimPath)) {
		debug(1, "PuzzleSnowboard: Failed to load BOARD.AN");
		delete _boardAnim;
		_boardAnim = nullptr;
	}

	// Load engine animation (ENGINE.AN)
	Common::Path engineAnimPath(kEngineAnimPath);
	_engineAnim = new Animation(_vm);
	if (!_engineAnim->loadFromFile(engineAnimPath)) {
		debug(1, "PuzzleSnowboard: Failed to load ENGINE.AN");
		delete _engineAnim;
		_engineAnim = nullptr;
	}

	// Load decoration/scenery animations (N1So-1, N1So-3, N1So-4, N1So-5, N1So-6)
	// Note: N1So-2 doesn't exist in resources
	static constexpr int decorNumbers[] = {1, 3, 4, 5, 6};
	for (int i = 0; i < 5; i++) {
		Common::Path decorPath(Common::String::format(kDecorFormat, decorNumbers[i]));
		_decorAnims[i] = new Animation(_vm);
		if (!_decorAnims[i]->loadFromFile(decorPath)) {
			debug(2, "PuzzleSnowboard: Failed to load N1So-%d", decorNumbers[i]);
			delete _decorAnims[i];
			_decorAnims[i] = nullptr;
		}
	}
}

void PuzzleSnowboard::generateTree() {
	// Generate a binary decision tree for the current level.
	// Tree structure: internal nodes at indices 0 to (depth-1),
	// leaves at indices depth to (2*depth).
	//
	// The current implementation selects traits intended to distribute the roster evenly.

	_tree.clear();
	_tree.resize(_treeDepth);

	// For each internal node, pick a random trait and a random match value.
	for (int i = 0; i < _treeDepth; i++) {
		TreeNode &node = _tree[i];
		node.traitIndex = static_cast<ZmbTrait::TraitIndex>(_vm->_rnd->getRandomNumber(ZmbTrait::kTraitCount - 1));
		node.matchVal1 = _vm->_rnd->getRandomNumber(ZmbTrait::kTraitValueCount - 1); // 0-4 = trait variants
		node.matchVal2 = _vm->_rnd->getRandomNumber(ZmbTrait::kTraitValueCount - 1); // For level 3
	}

	debug(2, "PuzzleSnowboard: Generated tree with %d internal nodes", _treeDepth);
	for (int i = 0; i < _treeDepth; i++) {
		debug(2, "  Node %d: trait=%d match1=%d match2=%d",
			  i, static_cast<int>(_tree[i].traitIndex), _tree[i].matchVal1, _tree[i].matchVal2);
	}
}

int PuzzleSnowboard::classifyZoombini(const ZoombiniRunner *z) const {
	// Traverse the binary decision tree to determine the destination lane.
	// Algorithm:
	//   v = 0 (root)
	//   while v < depth:
	//     node = tree[v]
	//     zoombiniVal = z->traits[node.traitIndex]
	//     if match: v = 2*v + 1 (left child)
	//     else: v = 2*v + 2 (right child)
	//   lane = v - depth

	if (_tree.empty())
		return 0;

	int v = 0;
	int level = _vm->getGameState()->_level;

	while (v < _treeDepth) {
		const TreeNode &node = _tree[v];
		const byte zoombiniVal = z->_traits.getValue(node.traitIndex);

		bool match;
		if (level >= 3) {
			// Level 3: match either value
			match = (zoombiniVal == node.matchVal1 || zoombiniVal == node.matchVal2);
		} else {
			// Levels 1-2: single value match
			match = (zoombiniVal == node.matchVal1);
		}

		if (match) {
			v = 2 * v + 1; // Left child
		} else {
			v = 2 * v + 2; // Right child
		}
	}

	int lane = v - _treeDepth;
	return CLIP(lane, 0, _numLanes - 1);
}

void PuzzleSnowboard::assignZoombinisToLanes() {
	_laneAssignments.clear();
	_laneAssignments.resize(_puzzleZoombinis.size());

	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		_laneAssignments[i] = classifyZoombini(_puzzleZoombinis[i]);
		debug(2, "  Zoombini %d -> Lane %d", i, _laneAssignments[i]);
	}
}

void PuzzleSnowboard::onUpdate() {
	uint32 now = _vm->getGameTickCount();
	uint32 elapsed = now - _stateTimer;

	switch (_state) {
	case kStateInit:
		// Should not happen after init()
		break;

	case kStateSliding:
		// Auto-advance each zoombini every 500ms
		if (500 < elapsed) {
			_currentZoombini += 1;
			_stateTimer = now;

			if (static_cast<int>(_puzzleZoombinis.size()) <= _currentZoombini) {
				_state = kStateDone;
				debug(1, "PuzzleSnowboard: All zoombinis assigned");
			}
		}
		break;

	case kStateDone:
		// Wait 2 seconds then exit
		if (2000 < elapsed) {
			debug(1, "PuzzleSnowboard: Complete, returning to map");
			_vm->_returningFromPuzzle = true;
			_vm->_mapTransitionSourcePageId = kPageSnowboard;
			_vm->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void PuzzleSnowboard::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

void PuzzleSnowboard::onRenderContent(ManagedSurface32 *screen) {
	uint32 now = _vm->getGameTickCount();

	// Draw decorative/scenery animations (positioned across the scene)
	// These add visual polish with animated background elements
	static constexpr Common::Point32 decorPos[5] = {
		Common::Point32(50, 350),  // N1So-1 - bottom left
		Common::Point32(450, 100), // N1So-3 - top right
		Common::Point32(600, 200), // N1So-4 - right side
		Common::Point32(200, 400), // N1So-5 - bottom middle
		Common::Point32(100, 150)  // N1So-6 - left middle
	};
	for (int i = 0; i < 5; i++) {
		if (_decorAnims[i]) {
			int frameCount = _decorAnims[i]->getFrameCount();
			if (0 < frameCount) {
				// Different timing for variety (80ms, 120ms, 90ms, 110ms, 100ms per frame)
				int timings[] = {80, 120, 90, 110, 100};
				int frameIdx = (now / timings[i]) % frameCount;
				_vm->_gfx->drawAnimationFrame(screen, _decorAnims[i], frameIdx, decorPos[i]);
			}
		}
	}

	// Draw board animation or static sprite
	static constexpr Common::Point32 kBoardPos = Common::Point32(150, 100);
	if (_boardAnim) {
		// Cycle through board animation frames
		int frameCount = _boardAnim->getFrameCount();
		if (0 < frameCount) {
			int frameIdx = (now / 100) % frameCount; // ~10 fps animation
			_vm->_gfx->drawAnimationFrame(screen, _boardAnim, frameIdx, kBoardPos);
		}
	} else if (_boardBitmap) {
		// Fallback to static board sprite
		_vm->_gfx->drawBitBlock(screen, _boardBitmap, kBoardPos);
	}

	// Draw engine animation (lift mechanism)
	static constexpr Common::Point32 kEnginePos = Common::Point32(50, 400);
	if (_engineAnim) {
		int frameCount = _engineAnim->getFrameCount();
		if (0 < frameCount) {
			int frameIdx = (now / 100) % frameCount;
			_vm->_gfx->drawAnimationFrame(screen, _engineAnim, frameIdx, kEnginePos);
		}
	}

	// Draw decision tree visualization
	// Show which traits are being checked at each level.
	static constexpr Common::Point32 kTreePos = Common::Point32(50, 50);

	for (int i = 0; i < _treeDepth; i++) {
		const TreeNode &node = _tree[i];
		const int traitIndex = static_cast<int>(node.traitIndex);
		// Draw the trait icon for this node's match value
		_vm->_gfx->drawRleBlock(screen, _traitIcons[traitIndex][node.matchVal1], Common::Point32(kTreePos.x + i * 60, kTreePos.y));
	}
}

void PuzzleSnowboard::onRenderActors(ManagedSurface32 *screen) {
	// Draw lane indicators
	int laneSpacing = 150;
	const Common::Point32 laneStartPos((800 - (_numLanes - 1) * laneSpacing) / 2, 400);

	for (int lane = 0; lane < _numLanes; lane++) {
		const Common::Point32 lanePos(laneStartPos.x + lane * laneSpacing, laneStartPos.y);

		// Count zoombinis in this lane
		int count = 0;
		for (uint i = 0; i < _laneAssignments.size() && static_cast<int>(i) <= _currentZoombini; i++) {
			if (_laneAssignments[i] == lane)
				count += 1;
		}

		// Draw zoombinis in this lane
		if (_zoombiniAnimation) {
			for (int z = 0; z < count && z < 4; z++) {
				// Find the z-th zoombini assigned to this lane
				int zoombiniIdx = -1;
				int c = 0;
				for (uint i = 0; i < _laneAssignments.size() && static_cast<int>(i) <= _currentZoombini; i++) {
					if (_laneAssignments[i] == lane) {
						if (c == z) {
							zoombiniIdx = i;
							break;
						}
						c += 1;
					}
				}

				if (0 <= zoombiniIdx && zoombiniIdx < static_cast<int>(_puzzleZoombinis.size())) {
					const ZoombiniRunner *zb = _puzzleZoombinis[zoombiniIdx];
					const Common::Point32 pos(lanePos.x + (z % 2) * 25, lanePos.y + (z / 2) * 30);

					_vm->_gfx->drawZoombini(screen, _zoombiniAnimation, zb->_traits, pos, 0, 0);
				}
			}
		}
	}

	// Draw current zoombini being processed
	if (_state == kStateSliding && _currentZoombini < static_cast<int>(_puzzleZoombinis.size())) {
		const ZoombiniRunner *z = _puzzleZoombinis[_currentZoombini];
		static constexpr Common::Point32 kCurrentZoombiniPos = Common::Point32(400, 200);

		if (_zoombiniAnimation) {
			_vm->_gfx->drawZoombini(screen, _zoombiniAnimation, z->_traits, kCurrentZoombiniPos, 0, 0);
		}
	}
}

EventHandleResult PuzzleSnowboard::onLButtonDown(const Common::Point &pos) {
	// Click to advance faster
	if (_state == kStateSliding) {
		_currentZoombini += 1;
		_stateTimer = _vm->getGameTickCount();

		if (static_cast<int>(_puzzleZoombinis.size()) <= _currentZoombini) {
			_state = kStateDone;
		}
		return EventHandleResult::kConsumed;
	} else if (_state == kStateDone) {
		// Skip wait and exit
		_vm->_returningFromPuzzle = true;
		_vm->_mapTransitionSourcePageId = kPageSnowboard;
		_vm->requestPageChange(kPageMapTrans);
		return EventHandleResult::kConsumed;
	}
	return EventHandleResult::kPassthrough;
}

void PuzzleSnowboard::drawTraitIcon(ManagedSurface32 *screen,
									ZmbTrait::TraitIndex traitIndex, int value, const Common::Point32 &pos) {
	const int traitOrdinal = static_cast<int>(traitIndex);
	if (traitOrdinal < 0 || ZmbTrait::kTraitCount <= traitOrdinal || value < 0 || ZmbTrait::kTraitValueCount <= value)
		return;

	RleBlock *icon = _traitIcons[traitOrdinal][value];
	_vm->_gfx->drawRleBlock(screen, icon, pos);
}

} // End of namespace Zoombini2
