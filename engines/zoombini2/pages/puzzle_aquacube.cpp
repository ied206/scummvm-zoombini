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
#include "zoombini2/pages/puzzle_aquacube.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// Graph one is an eight-node cube for levels one and two.
// Graph two is a sixteen-node double cube for levels three and four.
//
// Each entry: { adj[0], adj[1], adj[2], adj[3], position }
// ============================================================================

struct GraphInitData {
	int adj[4];
	Common::Point32 position;
};

static const GraphInitData kGraph1Data[8] = {
	// Node 0: adj=(1,3,4,-1) pos=(162,530)
	{{1, 3, 4, -1}, Common::Point32(162, 530)},
	// Node 1: adj=(0,2,5,-1) pos=(524,530)
	{{0, 2, 5, -1}, Common::Point32(524, 530)},
	// Node 2: adj=(3,1,6,-1) pos=(591,175)
	{{3, 1, 6, -1}, Common::Point32(591, 175)},
	// Node 3: adj=(2,0,7,-1) pos=(92,175)
	{{2, 0, 7, -1}, Common::Point32(92, 175)},
	// Node 4: adj=(5,7,0,-1) pos=(238,350)
	{{5, 7, 0, -1}, Common::Point32(238, 350)},
	// Node 5: adj=(4,6,1,-1) pos=(441,350)
	{{4, 6, 1, -1}, Common::Point32(441, 350)},
	// Node 6: adj=(7,5,2,-1) pos=(458,136)
	{{7, 5, 2, -1}, Common::Point32(458, 136)},
	// Node 7: adj=(6,4,3,-1) pos=(218,136)
	{{6, 4, 3, -1}, Common::Point32(218, 136)}};

static const GraphInitData kGraph2Data[16] = {
	// Node 0-3: Outer front face
	{{1, 3, 12, -1}, Common::Point32(162, 530)},
	{{0, 2, 13, -1}, Common::Point32(524, 530)},
	{{3, 1, 14, -1}, Common::Point32(591, 175)},
	{{2, 0, 15, -1}, Common::Point32(92, 175)},
	// Node 4-7: Inner front face
	{{5, 7, 0, 8}, Common::Point32(238, 350)},
	{{4, 6, 1, 9}, Common::Point32(441, 350)},
	{{7, 5, 2, 10}, Common::Point32(458, 136)},
	{{6, 4, 3, 11}, Common::Point32(218, 136)},
	// Node 8-11: Inner back face
	{{9, 11, 4, 12}, Common::Point32(160, 403)},
	{{8, 10, 5, 13}, Common::Point32(384, 403)},
	{{11, 9, 6, 14}, Common::Point32(417, 137)},
	{{10, 8, 7, 15}, Common::Point32(197, 137)},
	// Node 12-15: Outer back face
	{{13, 15, 0, 8}, Common::Point32(88, 336)},
	{{12, 14, 1, 9}, Common::Point32(372, 336)},
	{{15, 13, 2, 10}, Common::Point32(393, 79)},
	{{14, 12, 3, 11}, Common::Point32(109, 79)}};

// Map each geometric vertex to one direction label per active dimension.
const char AquacubePuzzle::kGraph1DirLabels[8][4] = {
	{'D', 'L', 'F', 0}, // Node 0
	{'D', 'R', 'F', 0}, // Node 1
	{'U', 'R', 'F', 0}, // Node 2
	{'U', 'L', 'F', 0}, // Node 3
	{'D', 'L', 'B', 0}, // Node 4
	{'D', 'R', 'B', 0}, // Node 5
	{'U', 'R', 'B', 0}, // Node 6
	{'U', 'L', 'B', 0}  // Node 7
};

const char AquacubePuzzle::kGraph2DirLabels[16][4] = {
	{'D', 'L', 'F', 'X'}, // Node 0
	{'D', 'R', 'F', 'X'}, // Node 1
	{'U', 'R', 'F', 'X'}, // Node 2
	{'U', 'L', 'F', 'X'}, // Node 3
	{'D', 'L', 'F', 'C'}, // Node 4
	{'D', 'R', 'F', 'C'}, // Node 5
	{'U', 'R', 'F', 'C'}, // Node 6
	{'U', 'L', 'F', 'C'}, // Node 7
	{'D', 'L', 'B', 'C'}, // Node 8
	{'D', 'R', 'B', 'C'}, // Node 9
	{'U', 'R', 'B', 'C'}, // Node 10
	{'U', 'L', 'B', 'C'}, // Node 11
	{'D', 'L', 'B', 'X'}, // Node 12
	{'D', 'R', 'B', 'X'}, // Node 13
	{'U', 'R', 'B', 'X'}, // Node 14
	{'U', 'L', 'B', 'X'}  // Node 15
};

// Difficulty-specific actor and movement limits.
struct DiffParams {
	int numZoombinis;
	int field1;
	int field2;
	int totalSteps;
	int numFleens;
};

static const DiffParams kDiffParams[5] = {
	{0, 0, 0, 0, 0},   // Diff 0 (unused)
	{3, 7, 1, 6, 0},   // Diff 1: 3 zoombinis, 6 steps, 0 fleens
	{3, 7, 1, 6, 1},   // Diff 2: 3 zoombinis, 6 steps, 1 fleen
	{4, 12, 4, 11, 2}, // Diff 3: 4 zoombinis, 11 steps, 2 fleens
	{4, 12, 4, 11, 2}  // Diff 4: 4 zoombinis, 11 steps, 2 fleens
};

// Direction-arrow button positions.
static const Common::Point32 kArrowPositions[4] = {
	Common::Point32(649, 483), Common::Point32(674, 484), Common::Point32(698, 483), Common::Point32(725, 482)};

// Step-arrow positions.
static const Common::Point32 kStepArrowPositions[4] = {
	Common::Point32(648, 526), Common::Point32(676, 527), Common::Point32(701, 526), Common::Point32(727, 525)};

// Positions for up to eleven step indicators.
static const Common::Point32 kStepIndicatorPositions[11] = {
	Common::Point32(647, 565), Common::Point32(658, 565), Common::Point32(667, 565), Common::Point32(676, 565),
	Common::Point32(685, 565), Common::Point32(695, 565), Common::Point32(705, 565), Common::Point32(713, 565),
	Common::Point32(723, 565), Common::Point32(730, 565), Common::Point32(738, 565)};

// Warp-control positions.
static const Common::Point32 kWarpOverlayPosition(641, 541);
static const Common::Point32 kWarpTimerPosition(666, 544);

// Cube background draw position.
static const Common::Point32 kCubeDrawPosition(96, 152);

// Direction-arrow hit-test dimensions.
static const int kArrowHitW = 17;
static const int kArrowHitH = 41;

// ============================================================================
// Constructor / Destructor
// ============================================================================

AquacubePuzzle::AquacubePuzzle(Zoombini2Engine *engine)
	: PuzzlePage(engine, kPageAquacube),
	  _difficulty(1), _numNodes(8),
	  _ballNode(0), _targetNode(-1),
	  _ballPosition(0, 0),
	  _ballStartPosition(0, 0),
	  _ballEndPosition(0, 0),
	  _moveStartTime(0),
	  _numZoombinisToPlace(3), _totalSteps(6), _numFleens(0),
	  _stepsUsed(0), _maxSteps(6),
	  _zoombiniOffset(-2, 6),
	  _fleenOffset(10, 6),
	  _nodeOffset(0, 0),
	  _warpAvailable(false), _warpActive(false),
	  _lightGfx(nullptr),
	  _manetteOnGfx(nullptr), _manetteOffGfx(nullptr),
	  _ballGfx(nullptr), _ballBigGfx(nullptr),
	  _shotsOnGfx(nullptr), _shotsOffGfx(nullptr),
	  _lightRedGfx(nullptr), _lightGreyGfx(nullptr),
	  _warpOnGfx(nullptr), _warpOffGfx(nullptr),
	  _warpDisableGfx(nullptr), _warpTimerAnim(nullptr),
	  _gameState(kStateIdle), _freedCount(0), _musicId(-1) {

	memset(_cubeGfx, 0, sizeof(_cubeGfx));
	memset(_bubbleGfx, 0, sizeof(_bubbleGfx));
	memset(_fleenGfx, 0, sizeof(_fleenGfx));
	memset(_flareAnims, 0, sizeof(_flareAnims));
}

AquacubePuzzle::~AquacubePuzzle() {
	if (_musicId >= 0) {
		SoundManager *snd = _engine->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	delete _lightGfx;
	for (int i = 0; i < 3; i++)
		delete _cubeGfx[i];
	delete _manetteOnGfx;
	delete _manetteOffGfx;
	delete _ballGfx;
	delete _ballBigGfx;
	delete _shotsOnGfx;
	delete _shotsOffGfx;
	delete _lightRedGfx;
	delete _lightGreyGfx;
	delete _warpOnGfx;
	delete _warpOffGfx;
	delete _warpDisableGfx;
	delete _warpTimerAnim;
	for (int i = 0; i < 3; i++)
		delete _bubbleGfx[i];
	for (int i = 0; i < 4; i++)
		delete _fleenGfx[i];
	for (int i = 0; i < 2; i++)
		delete _flareAnims[i];
}

// ============================================================================
// Initialization
// ============================================================================

void AquacubePuzzle::init() {
	PuzzlePage::init();

	// Start the Aqua Cube music.
	if (SoundManager *snd = _engine->getSoundManager()) {
		_musicId = snd->load(true, Common::Path("sounds/music/03-BB01.wav"), true);
		if (_musicId >= 0) {
			snd->playLoop(_musicId);
			snd->setVolume(_musicId, snd->_volumeMusic);
		}
	}

	_difficulty = CLIP(_engine->getGameState()->_gameMode, 1, 4);
	debug(1, "AquacubePuzzle::init - difficulty %d", _difficulty);

	// Apply the selected difficulty's actor and movement limits.
	const DiffParams &dp = kDiffParams[_difficulty];
	_numZoombinisToPlace = dp.numZoombinis;
	_totalSteps = dp.totalSteps;
	_maxSteps = dp.totalSteps;
	_numFleens = dp.numFleens;

	// The larger graph uses a slightly different actor alignment.
	if (_difficulty <= 2) {
		_zoombiniOffset = Common::Point32(-2, 6);
	} else {
		_zoombiniOffset = Common::Point32(-5, 5);
	}
	_fleenOffset = Common::Point32(10, 6);
	_nodeOffset = Common::Point32(0, 0);

	_warpAvailable = (_difficulty > 1);

	loadResources();
	loadGraph();
	placeZoombinis();
	placeFleens();
	placeBallStart();

	_stepsUsed = 0;
	_freedCount = 0;
	_gameState = kStateIdle;
	_stateTimer = _engine->getGameTickCount();
}

// ============================================================================
// Graph loading
// ============================================================================

void AquacubePuzzle::loadGraph() {
	// Select graph based on difficulty (Init at line ~202)
	const GraphInitData *srcData;
	if (_difficulty <= 2) {
		srcData = kGraph1Data;
		_numNodes = 8;
	} else {
		srcData = kGraph2Data;
		_numNodes = 16;
	}

	for (int i = 0; i < _numNodes; i++) {
		GraphNode &n = _nodes[i];
		for (int j = 0; j < 4; j++)
			n.adj[j] = srcData[i].adj[j];
		n.position = srcData[i].position;
		n.state = 1; // All nodes start as empty
		n.occupantCount = 0;
		n.occupants[0] = -1;
		n.occupants[1] = -1;
		n.occupants[2] = -1;
		n.fleenType = 0;

		// Initialize direction values to -1
		for (int j = 0; j < 4; j++)
			n.dirValues[j] = -1;
	}

	debug(2, "AquacubePuzzle: Loaded %d-node graph for difficulty %d",
		  _numNodes, _difficulty);
}

// ============================================================================
// Zoombini and Fleen placement
// ============================================================================

void AquacubePuzzle::placeZoombinis() {
	// Pick random vertices and build binary coordinates from their direction labels.
	Common::RandomSource rnd("aquacube_place");

	// Track which nodes are used for zoombinis
	bool usedNodes[16];
	memset(usedNodes, 0, sizeof(usedNodes));

	// Random binary direction choices per zoombini (Init v14 = rand()%2)
	int dirChoices[4];

	int numZoombinis = MIN(_numZoombinisToPlace, (int)_puzzleZoombinis.size());

	for (int z = 0; z < numZoombinis; z++) {
		// Pick a random unused node
		int nodeIdx;
		int attempts = 50;
		do {
			nodeIdx = rnd.getRandomNumber(_numNodes - 1);
			attempts--;
		} while (attempts > 0 && usedNodes[nodeIdx]);

		if (attempts <= 0)
			break;

		usedNodes[nodeIdx] = true;
		dirChoices[z] = rnd.getRandomNumber(1); // 0 or 1

		// Place zoombini at this node
		GraphNode &node = _nodes[nodeIdx];
		node.state = 0; // Occupied
		node.occupants[node.occupantCount] = z;
		node.occupantCount++;

		debug(2, "AquacubePuzzle: Zoombini %d placed at node %d (%d,%d) dir=%d",
			  z, nodeIdx, node.position.x, node.position.y, dirChoices[z]);
	}

	// Build direction values for all nodes based on direction labels
	// Map direction labels to the chosen binary coordinate values.
	const char (*labels)[4] = (_difficulty <= 2) ? kGraph1DirLabels : kGraph2DirLabels;

	// Map direction chars to zoombini choices:
	// Each opposing direction pair uses complementary binary values.

	for (int i = 0; i < _numNodes; i++) {
		for (int z = 0; z < numZoombinis; z++) {
			char label = labels[i][z];
			int val = 0;

			// Map label to direction value
			// Each pair (D/U, L/R, F/B, C/X) maps to complementary values
			switch (label) {
			case 'D':
				val = dirChoices[0];
				break;
			case 'U':
				val = 1 - dirChoices[0];
				break;
			case 'L':
				val = dirChoices[1];
				break;
			case 'R':
				val = 1 - dirChoices[1];
				break;
			case 'F':
				val = dirChoices[2];
				break;
			case 'B':
				val = 1 - dirChoices[2];
				break;
			case 'C':
				val = (numZoombinis > 3) ? dirChoices[3] : 0;
				break;
			case 'X':
				val = (numZoombinis > 3) ? (1 - dirChoices[3]) : 0;
				break;
			default:
				val = 0;
				break;
			}

			_nodes[i].dirValues[z] = val;
		}
	}
}

void AquacubePuzzle::placeFleens() {
	if (_numFleens == 0)
		return;

	// Prefer the all-ones vertex for the first Fleen.
	// For diff 2: 1 fleen at (1,1,1)
	// For diff 3-4: multiple fleens at various positions

	if (_difficulty == 2) {
		int fleenNode = findNodeByDirValues3(1, 1, 1);
		if (fleenNode >= 0) {
			_nodes[fleenNode].state = 3;
			_nodes[fleenNode].fleenType = 1;
			debug(2, "AquacubePuzzle: Fleen placed at node %d", fleenNode);
		}
	} else if (_difficulty >= 3) {
		// First fleen at (1,1,1,1)
		int fleenNode1 = findNodeByDirValues4(1, 1, 1, 1);
		if (fleenNode1 >= 0) {
			_nodes[fleenNode1].state = 3;
			_nodes[fleenNode1].fleenType = 1;
		}

		// Place additional fleens at other high-value positions
		Common::RandomSource rnd("aquacube_fleen");
		for (int f = 1; f < _numFleens && f < 3; f++) {
			// Find another node that is empty (state=1) to place a fleen
			int attempts = 50;
			int nodeIdx;
			do {
				nodeIdx = rnd.getRandomNumber(_numNodes - 1);
				attempts--;
			} while (attempts > 0 && _nodes[nodeIdx].state != 1);

			if (attempts > 0) {
				_nodes[nodeIdx].state = 3;
				_nodes[nodeIdx].fleenType = f + 1;
				debug(2, "AquacubePuzzle: Fleen %d placed at node %d", f + 1, nodeIdx);
			}
		}
	}
}

void AquacubePuzzle::placeBallStart() {
	// Select a difficulty-specific starting coordinate.
	// Diff 1: dirValues = (0,0,0)
	// Diff 2: random from (0,0,1), (0,1,0), (1,0,0)
	// Diff 3: dirValues = (0,0,0,0)
	// Diff 4: random pattern, avoid fleens

	Common::RandomSource rnd("aquacube_start");
	int startNode = -1;

	if (_difficulty == 1) {
		startNode = findNodeByDirValues3(0, 0, 0);
	} else if (_difficulty == 2) {
		int pattern = rnd.getRandomNumber(2);
		switch (pattern) {
		case 0:
			startNode = findNodeByDirValues3(0, 0, 1);
			break;
		case 1:
			startNode = findNodeByDirValues3(0, 1, 0);
			break;
		case 2:
			startNode = findNodeByDirValues3(1, 0, 0);
			break;
		}
	} else if (_difficulty == 3) {
		startNode = findNodeByDirValues4(0, 0, 0, 0);
	} else {
		// Diff 4: random empty node
		int attempts = 50;
		do {
			startNode = rnd.getRandomNumber(_numNodes - 1);
			attempts--;
		} while (attempts > 0 && _nodes[startNode].state == 3);
	}

	// Fallback: find any empty node
	if (startNode < 0 || _nodes[startNode].state == 3) {
		for (int i = 0; i < _numNodes; i++) {
			if (_nodes[i].state == 1) {
				startNode = i;
				break;
			}
		}
	}

	if (startNode >= 0) {
		_nodes[startNode].state = 2;
		_ballNode = startNode;
		_ballPosition = _nodes[startNode].position;
		debug(1, "AquacubePuzzle: Ball start at node %d (%d,%d)",
			  startNode, _ballPosition.x, _ballPosition.y);
	}
}

// ============================================================================
// Binary-coordinate vertex lookup
// ============================================================================

int AquacubePuzzle::findNodeByDirValues3(int a, int b, int c) const {
	for (int i = 0; i < 8; i++) {
		if (_nodes[i].dirValues[0] == a &&
			_nodes[i].dirValues[1] == b &&
			_nodes[i].dirValues[2] == c)
			return i;
	}
	return -1;
}

int AquacubePuzzle::findNodeByDirValues4(int a, int b, int c, int d) const {
	for (int i = 0; i < 16; i++) {
		if (_nodes[i].dirValues[0] == a &&
			_nodes[i].dirValues[1] == b &&
			_nodes[i].dirValues[2] == c &&
			_nodes[i].dirValues[3] == d)
			return i;
	}
	return -1;
}

// ============================================================================
// Resource loading
// ============================================================================

void AquacubePuzzle::loadResources() {
	// Ball sprites
	_ballGfx = new RleBlock();
	if (!_ballGfx->loadFromFile(Common::Path("bmp/aquacube/ball"))) {
		delete _ballGfx;
		_ballGfx = nullptr;
	}

	_ballBigGfx = new RleBlock();
	if (!_ballBigGfx->loadFromFile(Common::Path("bmp/aquacube/ballBIG"))) {
		delete _ballBigGfx;
		_ballBigGfx = nullptr;
	}

	// Light sprite
	_lightGfx = new RleBlock();
	if (!_lightGfx->loadFromFile(Common::Path("bmp/aquacube/light"))) {
		delete _lightGfx;
		_lightGfx = nullptr;
	}

	// Cube background layers (3 layers per difficulty range)
	if (_difficulty <= 2) {
		const char *names[] = {"bmp/aquacube/kub_easy_01",
							   "bmp/aquacube/kub_easy_02",
							   "bmp/aquacube/kub_easy_03"};
		for (int i = 0; i < 3; i++) {
			_cubeGfx[i] = new RleBlock();
			if (!_cubeGfx[i]->loadFromFile(Common::Path(names[i]))) {
				delete _cubeGfx[i];
				_cubeGfx[i] = nullptr;
			}
		}
	} else {
		const char *names[] = {"bmp/aquacube/kub_hard_01",
							   "bmp/aquacube/kub_hard_02",
							   "bmp/aquacube/kub_hard_03"};
		for (int i = 0; i < 3; i++) {
			_cubeGfx[i] = new RleBlock();
			if (!_cubeGfx[i]->loadFromFile(Common::Path(names[i]))) {
				delete _cubeGfx[i];
				_cubeGfx[i] = nullptr;
			}
		}
	}

	// Joystick controls.
	_manetteOnGfx = new RleBlock();
	if (!_manetteOnGfx->loadFromFile(Common::Path("bmp/aquacube/control_manetteON"))) {
		delete _manetteOnGfx;
		_manetteOnGfx = nullptr;
	}

	_manetteOffGfx = new RleBlock();
	if (!_manetteOffGfx->loadFromFile(Common::Path("bmp/aquacube/control_manetteOFF"))) {
		delete _manetteOffGfx;
		_manetteOffGfx = nullptr;
	}

	// Remaining-step controls.
	_shotsOnGfx = new RleBlock();
	if (!_shotsOnGfx->loadFromFile(Common::Path("bmp/aquacube/control_shotsON"))) {
		delete _shotsOnGfx;
		_shotsOnGfx = nullptr;
	}

	_shotsOffGfx = new RleBlock();
	if (!_shotsOffGfx->loadFromFile(Common::Path("bmp/aquacube/control_shotsOFF"))) {
		delete _shotsOffGfx;
		_shotsOffGfx = nullptr;
	}

	// Direction lights
	_lightRedGfx = new RleBlock();
	if (!_lightRedGfx->loadFromFile(Common::Path("bmp/aquacube/control_manette_lightRED"))) {
		delete _lightRedGfx;
		_lightRedGfx = nullptr;
	}

	_lightGreyGfx = new RleBlock();
	if (!_lightGreyGfx->loadFromFile(Common::Path("bmp/aquacube/control_manette_lightGREY"))) {
		delete _lightGreyGfx;
		_lightGreyGfx = nullptr;
	}

	// Warp buttons
	_warpOnGfx = new RleBlock();
	if (!_warpOnGfx->loadFromFile(Common::Path("bmp/aquacube/control_warpBUTTON_ON"))) {
		delete _warpOnGfx;
		_warpOnGfx = nullptr;
	}

	_warpOffGfx = new RleBlock();
	if (!_warpOffGfx->loadFromFile(Common::Path("bmp/aquacube/control_warpBUTTON_OFF"))) {
		delete _warpOffGfx;
		_warpOffGfx = nullptr;
	}

	_warpDisableGfx = new RleBlock();
	if (!_warpDisableGfx->loadFromFile(Common::Path("bmp/aquacube/control_warpBUTTON_DISABLE"))) {
		delete _warpDisableGfx;
		_warpDisableGfx = nullptr;
	}

	// Warp timer animation (replaces static empty timer)
	_warpTimerAnim = new Animation();
	if (!_warpTimerAnim->loadFromFile(Common::Path("bmp/aquacube/control_warpTIMER"))) {
		debug(2, "AquacubePuzzle: Failed to load warp timer animation");
		delete _warpTimerAnim;
		_warpTimerAnim = nullptr;
	}

	// Flare visual effects
	for (int i = 0; i < 2; i++) {
		_flareAnims[i] = new Animation();
		Common::Path path(Common::String::format("bmp/aquacube/FLARE%d", i + 1));
		if (!_flareAnims[i]->loadFromFile(path)) {
			debug(2, "AquacubePuzzle: Failed to load FLARE%d.AN", i + 1);
			delete _flareAnims[i];
			_flareAnims[i] = nullptr;
		}
	}

	// Bubbles
	for (int i = 0; i < 3; i++) {
		_bubbleGfx[i] = new RleBlock();
		Common::Path path(Common::String::format("bmp/aquacube/bubble%d", i + 1));
		if (!_bubbleGfx[i]->loadFromFile(path)) {
			delete _bubbleGfx[i];
			_bubbleGfx[i] = nullptr;
		}
	}

	// Fleen sprites (4 types)
	for (int i = 0; i < 4; i++) {
		_fleenGfx[i] = new RleBlock();
		Common::Path path(Common::String::format("bmp/aquacube/fleen/fixe/f%dfixe", i + 1));
		if (!_fleenGfx[i]->loadFromFile(path)) {
			delete _fleenGfx[i];
			_fleenGfx[i] = nullptr;
		}
	}
}

// ============================================================================
// Ball movement
// ============================================================================

void AquacubePuzzle::startBallMove(int targetNode) {
	_targetNode = targetNode;
	_ballStartPosition = _ballPosition;
	_ballEndPosition = _nodes[targetNode].position;
	_moveStartTime = _engine->getGameTickCount();
	if (_gameState != kStateWarpExecuting)
		_gameState = kStateBallMoving;

	debug(2, "AquacubePuzzle: Moving ball from node %d to %d", _ballNode, _targetNode);
}

void AquacubePuzzle::finishBallMove() {
	_ballNode = _targetNode;
	_targetNode = -1;
	_ballPosition = _nodes[_ballNode].position;
	_stepsUsed++;

	// If we are executing a warp queue, advance to the next planned move
	if (_gameState == kStateWarpExecuting) {
		_warpQueueIdx += 1;
		if (_warpQueueIdx < static_cast<int>(_warpQueue.size())) {
			int nextDir = _warpQueue[_warpQueueIdx];
			int adjNode = _nodes[_ballNode].adj[nextDir];
			if (adjNode >= 0) {
				startBallMove(adjNode);
				return;
			}
		} else {
			_gameState = kStateIdle;
			_warpQueue.clear();
		}
	}

	// Resolve the actor at the destination vertex.
	GraphNode &node = _nodes[_ballNode];

	if (node.state == 0 && node.occupantCount > 0) {
		// Release every Zoombini assigned to this vertex without an additional feature check.
		while (node.occupantCount > 0) {
			freeZoombini(_ballNode);
		}
		_gameState = kStateZoombiniFreed;
		_stateTimer = _engine->getGameTickCount();
		return;
	} else if (node.state == 3) {
		// Enter the Fleen penalty phase.
		node.state = 1;
		node.fleenType = 0;
		_gameState = kStateFleenHit;
		_stateTimer = _engine->getGameTickCount();
		return;
	}

	// Check if steps exhausted
	if (_stepsUsed >= _maxSteps) {
		_gameState = kStateDone;
		_stateTimer = _engine->getGameTickCount();
	} else {
		_gameState = kStateIdle;
	}
}

// ============================================================================
// Occupied vertices release their Zoombinis unconditionally without feature matching.
// ============================================================================

void AquacubePuzzle::freeZoombini(int nodeIdx) {
	GraphNode &node = _nodes[nodeIdx];
	if (node.occupantCount <= 0)
		return;

	int zIdx = node.occupants[0];
	debug(1, "AquacubePuzzle: Freed zoombini %d at node %d", zIdx, nodeIdx);

	if (zIdx >= 0 && zIdx < (int)_puzzleZoombinis.size()) {
		_puzzleZoombinis[zIdx]->_freeStatus = 0; // Mark as free
	}

	// Shift remaining occupants
	for (int i = 0; i < node.occupantCount - 1; i++)
		node.occupants[i] = node.occupants[i + 1];
	node.occupants[node.occupantCount - 1] = -1;
	node.occupantCount--;

	if (node.occupantCount == 0)
		node.state = 1; // Now empty

	_freedCount++;
}

int AquacubePuzzle::countFreeZoombinis() const {
	int count = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_freeStatus == 0)
			count++;
	}
	return count;
}

// ============================================================================
// State-machine update
// ============================================================================

void AquacubePuzzle::update() {
	uint32 now = _engine->getGameTickCount();
	uint32 elapsed = now - _stateTimer;

	switch (_gameState) {
	case kStateIdle:
		// Input is handled by handleClick().
		break;

	case kStateBallMoving:
	case kStateWarpExecuting: {
		uint32 moveElapsed = now - _moveStartTime;

		if (moveElapsed >= kMoveAnimDuration) {
			finishBallMove();
		} else {
			// The control points reduce this movement to linear interpolation.
			float progress = static_cast<float>(moveElapsed) / kMoveAnimDuration;
			_ballPosition.x = _ballStartPosition.x + static_cast<int32>((_ballEndPosition.x - _ballStartPosition.x) * progress);
			_ballPosition.y = _ballStartPosition.y + static_cast<int32>((_ballEndPosition.y - _ballStartPosition.y) * progress);
		}
		break;
	}

	case kStateMatchCheck:
		// Immediate transition
		_gameState = kStateIdle;
		break;

	case kStateZoombiniFreed:
		if (elapsed > 800) {
			if (_freedCount >= _numZoombinisToPlace) {
				debug(1, "AquacubePuzzle: Win! %d zoombinis freed", _freedCount);
				_gameState = kStateDone;
				_stateTimer = now;
			} else if (_stepsUsed >= _maxSteps) {
				_gameState = kStateDone;
				_stateTimer = now;
			} else {
				_gameState = kStateIdle;
			}
		}
		break;

	case kStateFleenHit:
		if (elapsed > 1000) {
			if (_stepsUsed >= _maxSteps) {
				_gameState = kStateDone;
				_stateTimer = now;
			} else {
				_gameState = kStateIdle;
			}
		}
		break;

	case kStateWarpPlanning:
		// Direction input is collected by handleClick().
		break;

	case kStateDone:
		if (elapsed > 2000) {
			debug(1, "AquacubePuzzle: Complete, returning to map");
			_engine->_returningFromPuzzle = true;
			_engine->_maptransSourceWorld = _pageId;
			_engine->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

// ============================================================================
// Draw
// ============================================================================

void AquacubePuzzle::draw(Graphics::ManagedSurface *screen) {
	// Draw background
	if (_background)
		_background->drawToSurface(screen, 0, 0);

	const byte(*lut)[256] = _engine->getAlphaLUT();

	// Draw the three overlapping cube layers at their shared origin.
	if (_difficulty <= 2) {
		// Easy cube: draw layer 3, then 2, then 1 (back to front)
		if (_cubeGfx[2])
			_cubeGfx[2]->drawToScreen(screen, kCubeDrawPosition.x, kCubeDrawPosition.y, lut);
		if (_cubeGfx[1])
			_cubeGfx[1]->drawToScreen(screen, kCubeDrawPosition.x, kCubeDrawPosition.y, lut);
		if (_cubeGfx[0])
			_cubeGfx[0]->drawToScreen(screen, kCubeDrawPosition.x, kCubeDrawPosition.y, lut);
	} else {
		// Hard cube: draw in depth order for 3 node groups
		if (_cubeGfx[0])
			_cubeGfx[0]->drawToScreen(screen, kCubeDrawPosition.x, kCubeDrawPosition.y, lut);
		if (_cubeGfx[1])
			_cubeGfx[1]->drawToScreen(screen, kCubeDrawPosition.x, kCubeDrawPosition.y, lut);
		if (_cubeGfx[2])
			_cubeGfx[2]->drawToScreen(screen, kCubeDrawPosition.x, kCubeDrawPosition.y, lut);
	}

	// Draw node markers (ball circles at each vertex)
	RleBlock *nodeGfx = (_difficulty <= 2) ? _ballBigGfx : _ballGfx;
	if (nodeGfx) {
		for (int i = 0; i < _numNodes; i++) {
			if (_nodes[i].state != 2) { // Don't draw at ball start position
				Common::Point32 nodePosition = _nodes[i].position + _nodeOffset;
				nodeGfx->drawToScreen(screen, nodePosition.x, nodePosition.y, lut);
			}
		}
	}

	// Draw fleens at fleen nodes
	for (int i = 0; i < _numNodes; i++) {
		if (_nodes[i].state == 3 && _nodes[i].fleenType > 0) {
			int fleenIdx = _nodes[i].fleenType - 1;
			if (fleenIdx < 4 && _fleenGfx[fleenIdx]) {
				Common::Point32 fleenPosition = _nodes[i].position + _fleenOffset;
				_fleenGfx[fleenIdx]->drawToScreen(screen, fleenPosition.x, fleenPosition.y, lut);
			}
		}
	}

	// Draw the ball at current position
	if (_ballBigGfx) {
		_ballBigGfx->drawToScreen(screen, _ballPosition.x, _ballPosition.y, lut);
	} else if (_ballGfx) {
		_ballGfx->drawToScreen(screen, _ballPosition.x, _ballPosition.y, lut);
	}

	// Draw direction-arrow controls.
	int numArrows = MIN(_numZoombinisToPlace, 4);
	for (int i = 0; i < numArrows; i++) {
		// Draw arrow connector
		if (_manetteOnGfx)
			_manetteOnGfx->drawToScreen(screen, kArrowPositions[i].x, kArrowPositions[i].y, lut);

		// Draw step arrow
		if (_shotsOnGfx)
			_shotsOnGfx->drawToScreen(screen, kStepArrowPositions[i].x, kStepArrowPositions[i].y, lut);
	}

	// Draw step indicators
	for (int i = 0; i < _totalSteps; i++) {
		RleBlock *indicGfx;
		if (i < _stepsUsed)
			indicGfx = _lightRedGfx; // Used step
		else
			indicGfx = _lightGreyGfx; // Available step

		if (indicGfx && i < 11) {
			indicGfx->drawToScreen(screen, kStepIndicatorPositions[i].x, kStepIndicatorPositions[i].y, lut);
		}
	}

	// Draw warp button (diff > 1)
	if (_warpAvailable) {
		RleBlock *warpGfx;
		if (_warpActive)
			warpGfx = _warpOnGfx;
		else if (_stepsUsed < _maxSteps)
			warpGfx = _warpOffGfx;
		else
			warpGfx = _warpDisableGfx;

		if (warpGfx)
			warpGfx->drawToScreen(screen, kWarpOverlayPosition.x, kWarpOverlayPosition.y, lut);

		// Draw animated warp timer
		if (_warpTimerAnim) {
			uint32 now = _engine->getGameTickCount();
			int frameCount = _warpTimerAnim->getFrameCount();
			if (frameCount > 0) {
				int frameIdx = (now / 80) % frameCount; // ~12.5 fps
				const RleBlock *frame = _warpTimerAnim->getFrame(frameIdx);
				if (frame)
					frame->drawToScreen(screen, kWarpTimerPosition.x, kWarpTimerPosition.y, lut);
			}
		}
	}

	// Draw flare visual effects (decorative)
	// Position flares at strategic points on the screen for visual polish
	uint32 now = _engine->getGameTickCount();
	static const Common::Point32 flarePositions[2] = {
		Common::Point32(50, 400), // FLARE1 - lower left
		Common::Point32(650, 100) // FLARE2 - upper right
	};
	for (int i = 0; i < 2; i++) {
		if (_flareAnims[i]) {
			int frameCount = _flareAnims[i]->getFrameCount();
			if (frameCount > 0) {
				// Different timing for variety
				int timing = (i == 0) ? 90 : 110;
				int frameIdx = (now / timing) % frameCount;
				const RleBlock *frame = _flareAnims[i]->getFrame(frameIdx);
				if (frame)
					frame->drawToScreen(screen, flarePositions[i].x, flarePositions[i].y, lut);
			}
		}
	}
}

// ============================================================================
// Input handling
// ============================================================================

void AquacubePuzzle::handleClick(const Common::Point &pos) {
	if (_gameState == kStateWarpPlanning) {
		// In warp planning, direction arrows add moves to the queue
		int numDirections = (_difficulty <= 2) ? 3 : 4;
		for (int d = 0; d < numDirections && d < 4; d++) {
			Common::Rect hitbox(
				static_cast<int16>(kArrowPositions[d].x), static_cast<int16>(kArrowPositions[d].y),
				static_cast<int16>(kArrowPositions[d].x + kArrowHitW), static_cast<int16>(kArrowPositions[d].y + kArrowHitH));
			if (hitbox.contains(pos)) {
				_warpQueue.push_back(d);
				debug(2, "AquacubePuzzle: Added direction %d to warp queue", d);
				return;
			}
		}

		// Clicking warp button again executes the queue
		Common::Rect warpHitbox(
			static_cast<int16>(kWarpOverlayPosition.x), static_cast<int16>(kWarpOverlayPosition.y),
			static_cast<int16>(kWarpOverlayPosition.x + 60), static_cast<int16>(kWarpOverlayPosition.y + 30));
		if (warpHitbox.contains(pos)) {
			if (!_warpQueue.empty()) {
				_warpQueueIdx = 0;
				_gameState = kStateWarpExecuting;
				_stateTimer = _engine->getGameTickCount();
				debug(2, "AquacubePuzzle: Executing warp queue of %d moves", static_cast<int>(_warpQueue.size()));

				// Start first move in queue
				int firstDir = _warpQueue[0];
				int adjNode = _nodes[_ballNode].adj[firstDir];
				if (adjNode >= 0)
					startBallMove(adjNode);
			}
			return;
		}
		return;
	}

	if (_gameState != kStateIdle)
		return;

	// Check direction arrow buttons.
	int numDirections = (_difficulty <= 2) ? 3 : 4;
	for (int d = 0; d < numDirections && d < 4; d++) {
		Common::Rect hitbox(
			static_cast<int16>(kArrowPositions[d].x), static_cast<int16>(kArrowPositions[d].y),
			static_cast<int16>(kArrowPositions[d].x + kArrowHitW), static_cast<int16>(kArrowPositions[d].y + kArrowHitH));

		if (hitbox.contains(pos)) {
			int adjNode = _nodes[_ballNode].adj[d];
			if (adjNode >= 0 && adjNode < _numNodes) {
				debug(2, "AquacubePuzzle: Direction %d clicked -> node %d", d, adjNode);
				startBallMove(adjNode);
			}
			return;
		}
	}

	// Check warp button
	if (_warpAvailable) {
		Common::Rect warpHitbox(
			static_cast<int16>(kWarpOverlayPosition.x), static_cast<int16>(kWarpOverlayPosition.y),
			static_cast<int16>(kWarpOverlayPosition.x + 60), static_cast<int16>(kWarpOverlayPosition.y + 30));

		if (warpHitbox.contains(pos)) {
			_warpQueue.clear();
			_gameState = kStateWarpPlanning;
			_stateTimer = _engine->getGameTickCount();
			debug(2, "AquacubePuzzle: Entered warp planning mode");
			return;
		}
	}

	// Check if clicked on a graph node directly (for accessibility)
	for (int i = 0; i < _numNodes; i++) {
		Common::Rect nodeHitbox(
			static_cast<int16>(_nodes[i].position.x - 20), static_cast<int16>(_nodes[i].position.y - 20),
			static_cast<int16>(_nodes[i].position.x + 20), static_cast<int16>(_nodes[i].position.y + 20));

		if (nodeHitbox.contains(pos)) {
			for (int d = 0; d < 4; d++) {
				if (_nodes[_ballNode].adj[d] == i) {
					debug(2, "AquacubePuzzle: Node %d clicked (adj dir %d)", i, d);
					startBallMove(i);
					return;
				}
			}
		}
	}
}

} // End of namespace Zoombini2
