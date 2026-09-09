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

#include "common/debug.h"
#include "common/random.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_snowboard.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// SnowboardPuzzle - binary decision tree classifier.
//
// Core algorithm:
//   - Tree nodes stored in array, each with: featureIdx, matchVal1, matchVal2
//   - A match selects the left child; a nonmatch selects the right child.
//   - The leaf index selects the destination snowboard lane.
//   - Difficulty 3 accepts matchVal1 OR matchVal2
// ============================================================================

// Number of lanes indexed by difficulty, with index zero unused.
static const int kLanesByDifficulty[] = {0, 2, 4, 4}; // diff 0(unused), 1, 2, 3

SnowboardPuzzle::SnowboardPuzzle(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageSnowboard), _treeDepth(0), _numLanes(0),
	  _boardGfx(nullptr), _boardAnim(nullptr), _engineAnim(nullptr),
	  _currentZoombini(0), _animFrame(0), _lastFrameTime(0),
	  _state(kStateInit), _musicId(-1) {

	for (int f = 0; f < 4; f++) {
		for (int v = 0; v < 5; v++) {
			_traitIcons[f][v] = nullptr;
		}
	}
	for (int i = 0; i < 5; i++) {
		_decorAnims[i] = nullptr;
	}
}

SnowboardPuzzle::~SnowboardPuzzle() {
	if (_musicId >= 0) {
		SoundManager *snd = _engine->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	for (int f = 0; f < 4; f++) {
		for (int v = 0; v < 5; v++) {
			delete _traitIcons[f][v];
		}
	}
	delete _boardGfx;
	delete _boardAnim;
	delete _engineAnim;
	for (int i = 0; i < 5; i++) {
		delete _decorAnims[i];
	}
}

void SnowboardPuzzle::init() {
	// Call base init for background and zoombini loading
	PuzzlePage::init();

	// Start the Snowboard Gulch music.
	if (SoundManager *snd = _engine->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("sounds/music/01-BS06.wav"), true);
		if (_musicId >= 0) {
			snd->playLoop(_musicId);
			snd->setVolume(_musicId, snd->_volumeMusic);
		}
	}

	int diff = CLIP(_engine->getGameState()->_gameMode, 1, 3);
	debug(1, "SnowboardPuzzle::init - difficulty %d", diff);
	_numLanes = kLanesByDifficulty[diff];
	_treeDepth = _numLanes - 1; // Binary tree: depth = numLeaves - 1

	// Load trait icons
	loadLaneGraphics();

	// Generate the decision tree
	generateTree();

	// Assign zoombinis to lanes
	assignZoombinisToLanes();

	_currentZoombini = 0;
	_state = kStateSliding;
	_stateTimer = _engine->getGameTickCount();
}

void SnowboardPuzzle::loadLaneGraphics() {
	// Load trait icons: bmp/snowboard/traits/{feature}-{value}.rb
	for (int f = 0; f < 4; f++) {
		for (int v = 0; v < 5; v++) {
			Common::Path path(Common::String::format("bmp/snowboard/traits/%d-%d", f + 1, v + 1));
			_traitIcons[f][v] = new RleBlock();
			if (!_traitIcons[f][v]->loadFromFile(path)) {
				debug(1, "SnowboardPuzzle: Failed to load trait %d-%d", f + 1, v + 1);
				delete _traitIcons[f][v];
				_traitIcons[f][v] = nullptr;
			}
		}
	}

	// Load board graphics
	Common::Path boardPath("bmp/snowboard/board01");
	_boardGfx = new BitBlock();
	if (!_boardGfx->load(boardPath)) {
		delete _boardGfx;
		_boardGfx = nullptr;
	}

	// Load board animation (BOARD.AN)
	Common::Path boardAnimPath("bmp/snowboard/BOARD");
	_boardAnim = new Animation();
	if (!_boardAnim->loadFromFile(boardAnimPath)) {
		debug(1, "SnowboardPuzzle: Failed to load BOARD.AN");
		delete _boardAnim;
		_boardAnim = nullptr;
	}

	// Load engine animation (ENGINE.AN)
	Common::Path engineAnimPath("bmp/snowboard/ENGINE");
	_engineAnim = new Animation();
	if (!_engineAnim->loadFromFile(engineAnimPath)) {
		debug(1, "SnowboardPuzzle: Failed to load ENGINE.AN");
		delete _engineAnim;
		_engineAnim = nullptr;
	}

	// Load decoration/scenery animations (N1So-1, N1So-3, N1So-4, N1So-5, N1So-6)
	// Note: N1So-2 doesn't exist in resources
	static const int decorNumbers[] = {1, 3, 4, 5, 6};
	for (int i = 0; i < 5; i++) {
		Common::Path decorPath(Common::String::format("bmp/snowboard/N1So-%d", decorNumbers[i]));
		_decorAnims[i] = new Animation();
		if (!_decorAnims[i]->loadFromFile(decorPath)) {
			debug(2, "SnowboardPuzzle: Failed to load N1So-%d", decorNumbers[i]);
			delete _decorAnims[i];
			_decorAnims[i] = nullptr;
		}
	}
}

void SnowboardPuzzle::generateTree() {
	// Generate a binary decision tree for the current difficulty.
	// Tree structure: internal nodes at indices 0 to (depth-1),
	// leaves at indices depth to (2*depth).
	//
	// The current implementation selects features intended to distribute the roster evenly.

	_tree.clear();
	_tree.resize(_treeDepth);

	Common::RandomSource rnd("snowboard");

	// For each internal node, pick a random feature and a random match value
	for (int i = 0; i < _treeDepth; i++) {
		TreeNode &node = _tree[i];
		node.featureIdx = rnd.getRandomNumber(3); // 0-3 = hair, eyes, nose, feet
		node.matchVal1 = rnd.getRandomNumber(4);  // 0-4 = feature variants
		node.matchVal2 = rnd.getRandomNumber(4);  // For difficulty 3
	}

	debug(2, "SnowboardPuzzle: Generated tree with %d internal nodes", _treeDepth);
	for (int i = 0; i < _treeDepth; i++) {
		debug(2, "  Node %d: feature=%d match1=%d match2=%d",
			  i, _tree[i].featureIdx, _tree[i].matchVal1, _tree[i].matchVal2);
	}
}

int SnowboardPuzzle::classifyZoombini(const ZoombiniState *z) const {
	// Traverse the binary decision tree to determine the destination lane.
	// Algorithm:
	//   v = 0 (root)
	//   while v < depth:
	//     node = tree[v]
	//     zoombiniVal = z->feature[node.featureIdx]
	//     if match: v = 2*v + 1 (left child)
	//     else: v = 2*v + 2 (right child)
	//   lane = v - depth

	if (_tree.empty())
		return 0;

	int v = 0;
	int diff = _engine->getGameState()->_gameMode;

	// Get zoombini features
	const byte features[4] = {z->_featureA, z->_featureB, z->_featureC, z->_featureD};

	while (v < _treeDepth) {
		const TreeNode &node = _tree[v];
		byte zoombiniVal = features[node.featureIdx];

		bool match;
		if (diff >= 3) {
			// Difficulty 3: match either value
			match = (zoombiniVal == node.matchVal1 || zoombiniVal == node.matchVal2);
		} else {
			// Difficulty 1-2: single value match
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

void SnowboardPuzzle::assignZoombinisToLanes() {
	_laneAssignments.clear();
	_laneAssignments.resize(_puzzleZoombinis.size());

	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		_laneAssignments[i] = classifyZoombini(_puzzleZoombinis[i]);
		debug(2, "  Zoombini %d -> Lane %d", i, _laneAssignments[i]);
	}
}

void SnowboardPuzzle::update() {
	uint32 now = _engine->getGameTickCount();
	uint32 elapsed = now - _stateTimer;

	switch (_state) {
	case kStateInit:
		// Should not happen after init()
		break;

	case kStateSliding:
		// Auto-advance each zoombini every 500ms
		if (elapsed > 500) {
			_currentZoombini++;
			_stateTimer = now;

			if (_currentZoombini >= (int)_puzzleZoombinis.size()) {
				_state = kStateDone;
				debug(1, "SnowboardPuzzle: All zoombinis assigned");
			}
		}
		break;

	case kStateDone:
		// Wait 2 seconds then exit
		if (elapsed > 2000) {
			debug(1, "SnowboardPuzzle: Complete, returning to map");
			_engine->_returningFromPuzzle = true;
			_engine->_maptransSourceWorld = kPageSnowboard;
			_engine->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void SnowboardPuzzle::draw(Graphics::ManagedSurface *screen) {
	// Draw background
	if (_background)
		_background->drawToSurface(screen, 0, 0);

	const byte(*lut)[256] = _engine->getAlphaLUT();
	uint32 now = _engine->getGameTickCount();

	// Draw decorative/scenery animations (positioned across the scene)
	// These add visual polish with animated background elements
	static const Common::Point32 decorPositions[5] = {
		Common::Point32(50, 350),  // N1So-1 - bottom left
		Common::Point32(450, 100), // N1So-3 - top right
		Common::Point32(600, 200), // N1So-4 - right side
		Common::Point32(200, 400), // N1So-5 - bottom middle
		Common::Point32(100, 150)  // N1So-6 - left middle
	};
	for (int i = 0; i < 5; i++) {
		if (_decorAnims[i]) {
			int frameCount = _decorAnims[i]->getFrameCount();
			if (frameCount > 0) {
				// Different timing for variety (80ms, 120ms, 90ms, 110ms, 100ms per frame)
				int timings[] = {80, 120, 90, 110, 100};
				int frameIdx = (now / timings[i]) % frameCount;
				const RleBlock *frame = _decorAnims[i]->getFrame(frameIdx);
				if (frame)
					frame->drawToScreen(screen, decorPositions[i].x, decorPositions[i].y, lut);
			}
		}
	}

	// Draw board animation or static sprite
	static const Common::Point32 kBoardPosition(150, 100);
	if (_boardAnim) {
		// Cycle through board animation frames
		int frameCount = _boardAnim->getFrameCount();
		if (frameCount > 0) {
			int frameIdx = (now / 100) % frameCount; // ~10 fps animation
			const RleBlock *frame = _boardAnim->getFrame(frameIdx);
			if (frame)
				frame->drawToScreen(screen, kBoardPosition.x, kBoardPosition.y, lut);
		}
	} else if (_boardGfx) {
		// Fallback to static board sprite
		_boardGfx->drawToSurface(screen, kBoardPosition.x, kBoardPosition.y);
	}

	// Draw engine animation (lift mechanism)
	static const Common::Point32 kEnginePosition(50, 400);
	if (_engineAnim) {
		int frameCount = _engineAnim->getFrameCount();
		if (frameCount > 0) {
			int frameIdx = (now / 100) % frameCount;
			const RleBlock *frame = _engineAnim->getFrame(frameIdx);
			if (frame)
				frame->drawToScreen(screen, kEnginePosition.x, kEnginePosition.y, lut);
		}
	}

	// Draw decision tree visualization
	// Show which features are being checked at each level
	static const Common::Point32 kTreePosition(50, 50);

	for (int i = 0; i < _treeDepth; i++) {
		const TreeNode &node = _tree[i];
		// Draw the trait icon for this node's match value
		if (_traitIcons[node.featureIdx][node.matchVal1]) {
			_traitIcons[node.featureIdx][node.matchVal1]->drawToScreen(
				screen, kTreePosition.x + i * 60, kTreePosition.y, lut);
		}
	}

	// Draw lane indicators
	int laneSpacing = 150;
	const Common::Point32 laneStartPosition((800 - (_numLanes - 1) * laneSpacing) / 2, 400);

	for (int lane = 0; lane < _numLanes; lane++) {
		const Common::Point32 lanePosition(laneStartPosition.x + lane * laneSpacing, laneStartPosition.y);

		// Count zoombinis in this lane
		int count = 0;
		for (uint i = 0; i < _laneAssignments.size() && (int)i <= _currentZoombini; i++) {
			if (_laneAssignments[i] == lane)
				count++;
		}

		// Draw zoombinis in this lane
		if (_zoombiniGfx) {
			for (int z = 0; z < count && z < 4; z++) {
				// Find the z-th zoombini assigned to this lane
				int zoombiniIdx = -1;
				int c = 0;
				for (uint i = 0; i < _laneAssignments.size() && (int)i <= _currentZoombini; i++) {
					if (_laneAssignments[i] == lane) {
						if (c == z) {
							zoombiniIdx = i;
							break;
						}
						c++;
					}
				}

				if (zoombiniIdx >= 0 && zoombiniIdx < (int)_puzzleZoombinis.size()) {
					const ZoombiniState *zb = _puzzleZoombinis[zoombiniIdx];
					const Common::Point32 position(lanePosition.x + (z % 2) * 25, lanePosition.y + (z / 2) * 30);

					// Draw body
					const RleBlock *frame = _zoombiniGfx->getFrame(0, 0);
					if (frame)
						frame->drawToScreen(screen, position.x, position.y, lut);

					// Draw features
					const byte features[4] = {zb->_featureA, zb->_featureB, zb->_featureC, zb->_featureD};
					for (int slot = 1; slot <= 4; slot++) {
						int featIdx = slot * ZoombiniGraphics::kDim2 + features[slot - 1];
						frame = _zoombiniGfx->getFrame(featIdx, 0);
						if (frame)
							frame->drawToScreen(screen, position.x, position.y, lut);
					}
				}
			}
		}
	}

	// Draw current zoombini being processed
	if (_state == kStateSliding && _currentZoombini < (int)_puzzleZoombinis.size()) {
		const ZoombiniState *z = _puzzleZoombinis[_currentZoombini];
		static const Common::Point32 kCurrentZoombiniPosition(400, 200);

		if (_zoombiniGfx) {
			const RleBlock *frame = _zoombiniGfx->getFrame(0, 0);
			if (frame)
				frame->drawToScreen(screen, kCurrentZoombiniPosition.x, kCurrentZoombiniPosition.y, lut);

			const byte features[4] = {z->_featureA, z->_featureB, z->_featureC, z->_featureD};
			for (int slot = 1; slot <= 4; slot++) {
				int featIdx = slot * ZoombiniGraphics::kDim2 + features[slot - 1];
				frame = _zoombiniGfx->getFrame(featIdx, 0);
				if (frame)
					frame->drawToScreen(screen, kCurrentZoombiniPosition.x, kCurrentZoombiniPosition.y, lut);
			}
		}
	}
}

void SnowboardPuzzle::handleClick(const Common::Point &pos) {
	// Click to advance faster
	if (_state == kStateSliding) {
		_currentZoombini++;
		_stateTimer = _engine->getGameTickCount();

		if (_currentZoombini >= (int)_puzzleZoombinis.size()) {
			_state = kStateDone;
		}
	} else if (_state == kStateDone) {
		// Skip wait and exit
		_engine->_returningFromPuzzle = true;
		_engine->_maptransSourceWorld = kPageSnowboard;
		_engine->requestPageChange(kPageMapTrans);
	}
}

void SnowboardPuzzle::drawTraitIcon(Graphics::ManagedSurface *screen,
									int feature, int value, const Common::Point32 &position) {
	if (feature < 0 || feature > 3 || value < 0 || value > 4)
		return;

	RleBlock *icon = _traitIcons[feature][value];
	if (icon) {
		const byte(*lut)[256] = _engine->getAlphaLUT();
		icon->drawToScreen(screen, position.x, position.y, lut);
	}
}

} // End of namespace Zoombini2
