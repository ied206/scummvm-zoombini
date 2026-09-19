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

#include "zoombini2/pages/puzzle_mysticmarsh.h"
#include "common/debug.h"
#include "zoombini2/graphics.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

// ============================================================================
// PuzzleMysticMarsh - swamp symbol grid puzzle.
//
// The grid has 16 columns and 12 rows in column-major order.
// Screen positions: X = 44*col + 10*row, Y = 1 + 31*row (diagonal layout).
// Crater hitboxes are 43 by 50 pixels and offset from their cell positions.
//
// Game modes select an internal level and one of six background variants.
// ============================================================================

// Launch-slot hit-test dimensions.
static constexpr Size32 kSlotHitSize = Size32(43, 50);

// Placement animation delay (ms)
static constexpr uint32 kPlaceDelay = 1000;

// Completion delay before transitioning (ms)
static constexpr uint32 kDoneDelay = 2000;

// Minimum number of released Zoombinis required for success.
static constexpr int kMinFreed = 4;

constexpr const char *PuzzleMysticMarsh::kMusicPath;
constexpr const char *PuzzleMysticMarsh::kBackgroundFormat;
constexpr const char *PuzzleMysticMarsh::kCraterPath;
constexpr const char *PuzzleMysticMarsh::kBubbleCraterPath;
constexpr const char *PuzzleMysticMarsh::kTourbiPath;
constexpr const char *PuzzleMysticMarsh::kTraitFormat;
constexpr const char *PuzzleMysticMarsh::kSymbolFormat;
constexpr const char *PuzzleMysticMarsh::kBubbleFormat;
constexpr const char *PuzzleMysticMarsh::kPickupZombAnimationPath;
constexpr const char *PuzzleMysticMarsh::kAreaMaskFormat;
constexpr const char *PuzzleMysticMarsh::kPlacementSoundPath;
constexpr const char *PuzzleMysticMarsh::kSymbolNames[kNumSymbols];

const Common::Point32 PuzzleMysticMarsh::kStartingPositions[6][8] = {
	{
		Common::Point32(127, 397),
		Common::Point32(176, 411),
		Common::Point32(232, 411),
		Common::Point32(99, 437),
		Common::Point32(151, 449),
		Common::Point32(210, 447),
		Common::Point32(117, 484),
		Common::Point32(186, 490),
	},
	{
		Common::Point32(127, 397),
		Common::Point32(176, 411),
		Common::Point32(232, 411),
		Common::Point32(99, 437),
		Common::Point32(151, 449),
		Common::Point32(210, 447),
		Common::Point32(117, 484),
		Common::Point32(186, 490),
	},
	{
		Common::Point32(127, 397),
		Common::Point32(176, 411),
		Common::Point32(232, 411),
		Common::Point32(99, 437),
		Common::Point32(151, 449),
		Common::Point32(210, 447),
		Common::Point32(117, 484),
		Common::Point32(186, 490),
	},
	{
		Common::Point32(52, 458),
		Common::Point32(103, 463),
		Common::Point32(148, 463),
		Common::Point32(200, 475),
		Common::Point32(53, 514),
		Common::Point32(90, 514),
		Common::Point32(132, 520),
		Common::Point32(175, 513),
	},
	{
		Common::Point32(106, 452),
		Common::Point32(155, 465),
		Common::Point32(211, 465),
		Common::Point32(78, 491),
		Common::Point32(131, 503),
		Common::Point32(189, 501),
		Common::Point32(95, 538),
		Common::Point32(164, 543),
	},
	{
		Common::Point32(127, 397),
		Common::Point32(176, 411),
		Common::Point32(232, 411),
		Common::Point32(99, 437),
		Common::Point32(151, 449),
		Common::Point32(210, 447),
		Common::Point32(117, 484),
		Common::Point32(186, 490),
	}};

PuzzleMysticMarsh::PuzzleMysticMarsh(Zoombini2Engine *vm)
	: PuzzleBase(vm, kPageMysticMarsh) {
}

PuzzleMysticMarsh::~PuzzleMysticMarsh() {
	if (0 <= _placementSoundId)
		_vm->getSoundManager()->unload(_placementSoundId);
	for (int i = 0; i < kNumSymbols; i++)
		delete _symbolImage[i];

	for (int f = 0; f < 4; f++)
		for (int v = 0; v < 5; v++)
			delete _traitImage[f][v];

	for (int i = 0; i < 3; i++)
		delete _bubbleImage[i];

	delete _craterImage;
	delete _bubbleCraterAnim;
	delete _tourbiAnim;
	finishPuzzleRoster(_vm->_state->_rescue1Board);
}

void PuzzleMysticMarsh::init() {
	// Call base init for zoombini graphics and zoombini list.
	PuzzleBase::init();

	// Start the Bubble Bumpers music.
	startPageMusic(Common::Path(kMusicPath));
	if (SoundManager *snd = _vm->getSoundManager())
		_placementSoundId = snd->load(false, Common::Path(kPlacementSoundPath), false);

	int gameMode = CLIP(_vm->_state->_level, 1, 3);

	// Map game mode to level (1-4)
	switch (gameMode) {
	case 1:
		_level = 1;
		break;
	case 2:
		_level = 2;
		break;
	case 3:
		_level = 3;
		break;
	default:
		_level = 1;
		break;
	}

	// Select background index from level sub-level
	switch (_level) {
	case 1:
		_bgIndex = 1;
		break;
	case 2:
		_bgIndex = 3;
		break;
	case 3:
		_bgIndex = 4;
		break;
	case 4:
		_bgIndex = 6;
		break;
	default:
		_bgIndex = 1;
		break;
	}

	// Reload background with level-appropriate variant
	const Common::Path bgPath(Common::String::format(kBackgroundFormat, _bgIndex));
	if (!loadPrimaryLayerBackground(bgPath)) {
		debug(1, "PuzzleMysticMarsh: Failed to load background%d", _bgIndex);
	}

	// Load puzzle resources
	loadResources();
	_pickupZombAnimation = _vm->loadZoombiniAnimation(Common::Path(kPickupZombAnimationPath), 100);
	loadAreaMask(Common::Path(Common::String::format(kAreaMaskFormat, _bgIndex)));
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _puzzleZoombinis[i];
		if (!zoombini)
			continue;
		if (i < 8)
			zoombini->setPosition(kStartingPositions[_bgIndex - 1][i]);
		zoombini->setDefaultAnimation(_zoombiniAnimation);
		zoombini->_inputEnabled = true;
		zoombini->_puzzleStatus = 0;
		zoombini->_hidden = false;
		zoombini->_dragging = false;
		zoombini->clearMovement();
	}

	// Setup grid
	setupGrid();
	buildSlots();

	_freedCount = 0;
	_hasActiveZ = false;
	_activeSlotIdx = -1;
	_slotUnlockTime = 0;
	_state = kStateIdle;
	_stateTimer = _vm->getGameTickCount();
}

void PuzzleMysticMarsh::loadResources() {
	loadTraits();
	loadSymbols();
	loadBubbles();

	// Load crater sprite
	Common::Path craterPath(kCraterPath);
	_craterImage = new RleBlock(_vm);
	if (!_craterImage->loadFromFile(craterPath)) {
		delete _craterImage;
		_craterImage = nullptr;
	}

	// Load BubbleCrater animation
	Common::Path bubbleCraterPath(kBubbleCraterPath);
	_bubbleCraterAnim = new Animation(_vm);
	if (!_bubbleCraterAnim->loadFromFile(bubbleCraterPath)) {
		delete _bubbleCraterAnim;
		_bubbleCraterAnim = nullptr;
	}

	// Load tourbi (whirlpool) animation
	Common::Path tourbiPath(kTourbiPath);
	_tourbiAnim = new Animation(_vm);
	if (!_tourbiAnim->loadFromFile(tourbiPath)) {
		delete _tourbiAnim;
		_tourbiAnim = nullptr;
	}

	debug(2, "PuzzleMysticMarsh: Resources loaded");
}

void PuzzleMysticMarsh::loadTraits() {
	// Load five values for each of the four visible features.
	for (int f = 0; f < 4; f++) {
		for (int v = 0; v < 5; v++) {
			Common::Path traitPath(Common::String::format(
				kTraitFormat, f + 1, v + 1));
			_traitImage[f][v] = new RleBlock(_vm);
			if (!_traitImage[f][v]->loadFromFile(traitPath)) {
				delete _traitImage[f][v];
				_traitImage[f][v] = nullptr;
			}
		}
	}
}

void PuzzleMysticMarsh::loadSymbols() {
	// Load symbol sprites from bmp/mystic_marsh/symbols/%s.bmp
	for (int i = 0; i < kNumSymbols; i++) {
		Common::Path symPath(Common::String::format(
			kSymbolFormat, kSymbolNames[i]));
		_symbolImage[i] = new RleBlock(_vm);
		if (!_symbolImage[i]->loadFromFile(symPath)) {
			delete _symbolImage[i];
			_symbolImage[i] = nullptr;
		}
	}
}

void PuzzleMysticMarsh::loadBubbles() {
	// Load bubble sprites: bubble1.bmp through bubble3.bmp
	for (int i = 0; i < 3; i++) {
		Common::Path bubblePath(Common::String::format(
			kBubbleFormat, i + 1));
		_bubbleImage[i] = new RleBlock(_vm);
		if (!_bubbleImage[i]->loadFromFile(bubblePath)) {
			delete _bubbleImage[i];
			_bubbleImage[i] = nullptr;
		}
	}
}

void PuzzleMysticMarsh::setupGrid() {
	// Initialize the grid before generating the current approximate layout.
	for (int i = 0; i < kMaxCells; i++) {
		_grid[i].type = 0;
		_grid[i].symbolIdx = -1;
		_grid[i].pos = Common::Point32();
	}

	// Compute screen positions for each cell.
	// Grid is column-major: index = col*kGridRows + row.
	// Offset each row horizontally to produce the diagonal grid.
	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			int idx = col * kGridRows + row;
			_grid[idx].pos = Common::Point32(44 * col + 10 * row, 1 + 31 * row);
		}
	}

	// Place the current level's predefined crater slots.
	generateRules();
}

void PuzzleMysticMarsh::generateRules() {
	// The current implementation places crater slots at regular intervals.
	// Craters are where zoombinis are placed. We place enough for
	// the puzzle pack size.

	const int numZoombinis = MIN<int>(_puzzleZoombinis.size(), 8);
	int numCraters = MAX(numZoombinis, 4);

	// Place craters in a staggered pattern across the grid
	static constexpr int craterPositions[][2] = {
		// {column, row}, distributed across the full grid.
		{2, 3},
		{5, 2},
		{8, 4},
		{11, 3},
		{3, 7},
		{6, 6},
		{9, 8},
		{12, 7},
		{4, 10},
		{7, 9},
		{10, 11},
		{13, 10},
	};

	int maxCraters = MIN(numCraters, 12);

	for (int i = 0; i < maxCraters; i++) {
		int col = craterPositions[i][0];
		int row = craterPositions[i][1];
		int idx = col * kGridRows + row;
		_grid[idx].type = 60;
	}

	// Place some decorative symbols between craters
	_vm->reseedRandomForV10();
	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			int idx = col * kGridRows + row;
			if (_grid[idx].type != 0)
				continue;

			// Sparse symbol placement (about 30% of empty cells)
			if (_vm->_rnd->getRandomNumber(99) < 30) {
				// Pick a random symbol type (2-47, excluding tourbi/edge/entry)
				int symType = _vm->_rnd->getRandomNumber(43) + 2;
				_grid[idx].type = symType;
				_grid[idx].symbolIdx = symType - 2;
			}
		}
	}

	debug(2, "PuzzleMysticMarsh: Generated rules, %d craters", maxCraters);
}

void PuzzleMysticMarsh::buildSlots() {
	// Build the fixed-capacity slot array from crater cells.
	_numSlots = 0;
	_dropTargets.clear();

	for (int col = 0; col < kGridCols && _numSlots < kMaxSlots; col++) {
		for (int row = 0; row < kGridRows && _numSlots < kMaxSlots; row++) {
			int idx = col * kGridRows + row;
			if (_grid[idx].type != 60 && _grid[idx].type != 61)
				continue;

			Slot &slot = _slots[_numSlots];
			slot.cellCol = col;
			slot.cellRow = row;

			// Offset the clickable crater area from the cell's drawing position.
			const Common::Point32 cellPos = _grid[idx].pos;
			slot.pos = Common::Point32(cellPos.x - 25, cellPos.y - 10);

			slot.hitbox = Common::Rect32(cellPos.x - 8, cellPos.y + 52, cellPos.x - 8 + kSlotHitSize.width, cellPos.y + 52 + kSlotHitSize.height);

			ZoombiniDropTarget target;
			target.rect = slot.hitbox;
			target.callback = &slotDropCallback;
			target.callbackContext = this;
			_dropTargets.push_back(target);

			_numSlots += 1;
		}
	}

	debug(2, "PuzzleMysticMarsh: Built %d slots", _numSlots);
}

void PuzzleMysticMarsh::slotDropCallback(void *context, int slotIdx, int zoombiniIdx) {
	PuzzleMysticMarsh *page = static_cast<PuzzleMysticMarsh *>(context);
	if (page && 0 <= slotIdx && slotIdx < page->_numSlots && page->_dropTargets[slotIdx].occupied)
		page->placeZoombiniAtSlot(slotIdx, zoombiniIdx);
}

void PuzzleMysticMarsh::placeZoombiniAtSlot(int slotIdx, int zoombiniIdx) {
	if (_hasActiveZ || slotIdx < 0 || _numSlots <= slotIdx)
		return;
	if (zoombiniIdx < 0 || _puzzleZoombinis.size() <= static_cast<uint>(zoombiniIdx))
		return;

	const Slot &slot = _slots[slotIdx];
	const int cellIdx = slot.cellCol * kGridRows + slot.cellRow;
	if (_grid[cellIdx].type != 60 && _grid[cellIdx].type != 61)
		return;
	const uint32 now = _vm->getGameTickCount();
	for (uint i = 0; i < _dropTargets.size(); i++) {
		_dropTargets[i].occupied = true;
		_dropTargets[i].zoombiniIndex = -1;
	}
	_slotUnlockTime = now + 4000;
	if (0 <= _placementSoundId) {
		SoundManager *soundManager = _vm->getSoundManager();
		soundManager->playWithVolume(_placementSoundId, soundManager->_volumeSFX);
	}
	_hasActiveZ = true;
	_activeSlotIdx = slotIdx;
	_activeZ.zoombiniIdx = zoombiniIdx;
	_activeZ.cellCol = slot.cellCol;
	_activeZ.cellRow = slot.cellRow;
	_activeZ.targetPos = Common::Point32(slot.pos.x + 10, slot.pos.y + 30);
	_activeZ.moveStartTime = now;
	ZoombiniRunner *zoombini = _puzzleZoombinis[zoombiniIdx];
	zoombini->_inputEnabled = false;
	zoombini->setPosition(_activeZ.targetPos);

	_state = kStateMoving;
	_stateTimer = now;

	debug(2, "PuzzleMysticMarsh: Placed zoombini %d at crater %d", zoombiniIdx, slotIdx);
}

void PuzzleMysticMarsh::moveZoombini() {
	if (!_hasActiveZ)
		return;

	uint32 now = _vm->getGameTickCount();
	uint32 elapsed = now - _activeZ.moveStartTime;

	// Move one cell every 500ms
	if (elapsed > 500) {
		// Simple movement: move upwards (towards exit)
		// In a full implementation, this would check _grid[idx].type for arrows/diverters
		_activeZ.cellRow -= 1;

		if (_activeZ.cellRow < 0) {
			// Reached the exit!
			_state = kStateFreeing;
			_stateTimer = now;
			return;
		}

		_activeZ.targetPos = _grid[_activeZ.cellCol * kGridRows + _activeZ.cellRow].pos;
		_puzzleZoombinis[_activeZ.zoombiniIdx]->setPosition(_activeZ.targetPos);
		_activeZ.moveStartTime = now;
	}
}

void PuzzleMysticMarsh::freeZoombini(int zoombiniIdx) {
	_puzzleZoombinis[zoombiniIdx]->_puzzleStatus = 1;
	_puzzleZoombinis[zoombiniIdx]->_hidden = true;
	_freedCount += 1;
	_hasActiveZ = false;
	debug(1, "PuzzleMysticMarsh: Freed zoombini %d (total: %d)", zoombiniIdx, _freedCount);
}

int PuzzleMysticMarsh::countFreeZoombinis() const {
	int count = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->_puzzleStatus == 1)
			count += 1;
	}
	return count;
}

EventHandleResult PuzzleMysticMarsh::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	return EventHandleResult::kPassthrough;
}

EventHandleResult PuzzleMysticMarsh::onLButtonUp(const Common::Point &pos) {
	const bool clickReleased = _state == kStateIdle;
	const ZoombiniInputResult result = ZoombiniRunner::handlePointerInput(_puzzleZoombinis, Common::Point32(pos.x, pos.y), clickReleased,
																		  _pickupZombAnimation, _vm->getGameTickCount(), &_dropTargets, getAreaMask());
	return result == ZoombiniInputResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

EventHandleResult PuzzleMysticMarsh::onMouseMove(const Common::Point &pos) {
	const ZoombiniInputResult result = ZoombiniRunner::handlePointerInput(_puzzleZoombinis, Common::Point32(pos.x, pos.y), false,
																		  _pickupZombAnimation, _vm->getGameTickCount(), &_dropTargets, getAreaMask());
	return result == ZoombiniInputResult::kIgnored00 ? EventHandleResult::kPassthrough : EventHandleResult::kConsumed;
}

void PuzzleMysticMarsh::onUpdate() {
	uint32 now = _vm->getGameTickCount();
	uint32 elapsed = now - _stateTimer;
	if (_slotUnlockTime != 0 && _slotUnlockTime < now) {
		for (uint i = 0; i < _dropTargets.size(); i++)
			_dropTargets[i].occupied = false;
		_slotUnlockTime = 0;
	}
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i])
			_puzzleZoombinis[i]->updateAnimation(now);
	}

	switch (_state) {
	case kStateInit:
		break;

	case kStateIdle:
		break;

	case kStateMoving:
		if (kPlaceDelay < elapsed)
			moveZoombini();
		if (_state == kStateFreeing) {
			_stateTimer = now;
		}
		break;

	case kStateFreeing:
		if (elapsed > kPlaceDelay) {
			freeZoombini(_activeZ.zoombiniIdx);
			_state = kStateIdle;
			_stateTimer = now;

			if (_freedCount >= kMinFreed) {
				_state = kStateDone;
			}
		}
		break;

	case kStatePopping:
		if (elapsed > kPlaceDelay) {
			_hasActiveZ = false;
			_state = kStateIdle;
			_stateTimer = now;
		}
		break;

	case kStateDone:
		if (elapsed > kDoneDelay) {
			debug(1, "PuzzleMysticMarsh: Complete, %d zoombinis freed", _freedCount);
			_vm->_returningFromPuzzle = true;
			_vm->_mapTransitionSourcePageId = kPageMysticMarsh;
			_vm->requestPageChange(kPageMapTrans);
		}
		break;
	}
}

void PuzzleMysticMarsh::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

void PuzzleMysticMarsh::onRenderContent(ManagedSurface32 *screen) {
	// Draw grid elements
	drawGrid(screen);
	if (_state == kStateMoving && 0 <= _activeSlotIdx && _activeSlotIdx < _numSlots && _bubbleCraterAnim && 0 < _bubbleCraterAnim->getFrameCount()) {
		const uint32 elapsed = _vm->getGameTickCount() - _stateTimer;
		if (elapsed < kPlaceDelay) {
			const int frame = MIN<int>(static_cast<int>(elapsed / 100), _bubbleCraterAnim->getFrameCount() - 1);
			_vm->_gfx->drawAnimationFrame(screen, _bubbleCraterAnim, frame, _slots[_activeSlotIdx].pos);
		}
	}
}

void PuzzleMysticMarsh::drawGrid(ManagedSurface32 *screen) {
	for (int col = 0; col < kGridCols; col++) {
		for (int row = 0; row < kGridRows; row++) {
			int idx = col * kGridRows + row;
			const GridCell &cell = _grid[idx];

			if (cell.type < 2 || cell.type == 62)
				continue;

			if (cell.type == 60 || cell.type == 61)
				continue; // Craters drawn separately

			// Draw symbol sprite
			int symIdx = cell.type - 2;
			if (0 <= symIdx && symIdx < kNumSymbols && _symbolImage[symIdx]) {
				_vm->_gfx->drawRleBlock(screen, _symbolImage[symIdx], cell.pos);
			}
		}
	}

	// Draw tourbi animation if present
	if (_tourbiAnim && 0 < _tourbiAnim->getFrameCount()) {
		uint32 now = _vm->getGameTickCount();
		int frame = (now / 100) % _tourbiAnim->getFrameCount();
		// Draw the whirlpool at its fixed origin.
		// Actual position depends on grid cell with TOURBI type
		for (int i = 0; i < kMaxCells; i++) {
			if (_grid[i].type == 46) { // TOURBI = symbol index 44, type = 44+2 = 46
				_vm->_gfx->drawAnimationFrame(screen, _tourbiAnim, frame, _grid[i].pos);
			}
		}
	}
}

void PuzzleMysticMarsh::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleMysticMarsh::onActorsRendered() {
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i])
			_puzzleZoombinis[i]->advanceAnimationAfterDraw();
	}
}

} // End of namespace Zoombini2
