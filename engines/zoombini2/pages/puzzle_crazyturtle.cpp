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
#include "common/util.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_crazyturtle.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *PuzzleCrazyTurtle::kPageName;
constexpr const char *PuzzleCrazyTurtle::kBackgroundPath;
constexpr const char *PuzzleCrazyTurtle::kMusicPath;
constexpr const char *PuzzleCrazyTurtle::kGoSpeechFormat;
constexpr const char *PuzzleCrazyTurtle::kRetreatSpeech;
constexpr const char *PuzzleCrazyTurtle::kTurtleIdleFormat;
constexpr const char *PuzzleCrazyTurtle::kTurtleSpinFormat;
constexpr const char *PuzzleCrazyTurtle::kTurtleFixedFormat;
constexpr const char *PuzzleCrazyTurtle::kMotherPath;
constexpr const char *PuzzleCrazyTurtle::kMotherSpeechPath;
constexpr const char *PuzzleCrazyTurtle::kSmokePath;
constexpr const char *PuzzleCrazyTurtle::kMotherStartPath;
constexpr const char *PuzzleCrazyTurtle::kMotherEndPath;
constexpr const char *PuzzleCrazyTurtle::kMotherSuccessSpeechPath;
constexpr const char *PuzzleCrazyTurtle::kMotherPartialSpeechPath;
constexpr const char *PuzzleCrazyTurtle::kBridgePath;
constexpr const char *PuzzleCrazyTurtle::kCollapsedBridgePath;
constexpr const char *PuzzleCrazyTurtle::kBeamPath;
constexpr const char *PuzzleCrazyTurtle::kTraitFormat;

constexpr PuzzleCrazyTurtle::TurtlePlacement PuzzleCrazyTurtle::kTurtlePlacements[kTurtleCount];
constexpr Common::Point32 PuzzleCrazyTurtle::kZoombiniPos[kZoombiniCount];
constexpr Common::Point32 PuzzleCrazyTurtle::kPrimaryIconPos[kRuleSlotCount];
constexpr Common::Point32 PuzzleCrazyTurtle::kSecondaryIconPos[kRuleSlotCount];
constexpr Common::Point32 PuzzleCrazyTurtle::kLandingPos[kTurtleCount];
constexpr Common::Point32 PuzzleCrazyTurtle::kSmokePos;
constexpr Common::Point32 PuzzleCrazyTurtle::kMotherPos;

PuzzleCrazyTurtle::PuzzleCrazyTurtle(Zoombini2Engine *vm)
	: PuzzleBase(vm, PageId::kPageCrazyTurtle_01) {
	_initialBackgroundPath = kBackgroundPath;
}

PuzzleCrazyTurtle::~PuzzleCrazyTurtle() {
	for (int type = 0; type < kFeatureCount; type++) {
		delete _turtleIdleAnimations[type];
		delete _turtleSpinAnimations[type];
	}
	delete _motherAnimation;
	delete _motherSpeechAnimation;
	delete _smokeAnimation;
	for (int type = 0; type < kFeatureCount; type++) {
		delete _turtleIdleRunners[type];
		delete _turtleSpinRunners[type];
	}
	delete _smokeRunner;
	delete _motherRunner;
	finishPuzzleRoster(nullptr);
}

bool PuzzleCrazyTurtle::loadAnimationResource(Animation *&resource, const Common::Path &path) {
	resource = new Animation(_vm);
	if (resource->loadFromFile(path))
		return true;
	warning("CrazyTurtlePuzzle: cannot load animation '%s'", path.toString().c_str());
	delete resource;
	resource = nullptr;
	return false;
}

bool PuzzleCrazyTurtle::loadRleResource(const Common::String &path) {
	if (_vm->_gfx->loadPageRleBlock(path))
		return true;
	warning("CrazyTurtlePuzzle: cannot load RLE graphic '%s'", path.c_str());
	return false;
}

void PuzzleCrazyTurtle::init() {
	PuzzleBase::init();

	GameState *gameState = _vm->_state;
	generateRules();
	loadResources();
	loadInteractionResources();
	placeZoombinis();
	buildDropTargets();
	buildTurtleAssignments();
	_mistakesMirror = _remainingMistakes;

	startPageMusic(Common::Path(kMusicPath));

	if (gameState && _vm->inJourneyMode())
		gameState->registerPageVisit(PageId::kPageCrazyTurtle_01, 1);

	debug(1, "CrazyTurtlePuzzle::init: level=%d primary=%d secondary=%d mistakes=%d party=%u", _puzzleLevel, _primaryFeature,
		  _secondaryFeature, _remainingMistakes, _puzzleZoombinis.size());
}

void PuzzleCrazyTurtle::loadResources() {
	for (int type = 0; type < kFeatureCount; type++) {
		const int resourceNumber = type + 1;
		loadAnimationResource(_turtleIdleAnimations[type],
							  Common::Path(Common::String::format(kTurtleIdleFormat, resourceNumber, resourceNumber)));
		loadAnimationResource(_turtleSpinAnimations[type],
							  Common::Path(Common::String::format(kTurtleSpinFormat, resourceNumber, resourceNumber)));
		loadRleResource(Common::String::format(kTurtleFixedFormat, resourceNumber, resourceNumber));
	}

	loadAnimationResource(_motherAnimation, Common::Path(kMotherPath));
	loadAnimationResource(_motherSpeechAnimation, Common::Path(kMotherSpeechPath));
	loadAnimationResource(_smokeAnimation, Common::Path(kSmokePath));
	loadRleResource(kMotherStartPath);
	loadRleResource(kMotherEndPath);
	loadRleResource(kBridgePath);
	loadRleResource(kCollapsedBridgePath);
	loadRleResource(kBeamPath);

	for (int feature = 0; feature < kFeatureCount; feature++) {
		for (int value = 0; value < kFeatureValueCount; value++) {
			loadRleResource(Common::String::format(kTraitFormat, feature + 1, value + 1));
		}
	}
}

void PuzzleCrazyTurtle::loadInteractionResources() {
	loadAreaMask(Common::Path(kAreaMaskPath));

	if (SoundManager *soundManager = _vm->getSoundManager()) {
		_sndTurtleIdle = soundManager->load(false, Common::Path(kTurtleIdleSoundPath), false);
		_sndSmoke = soundManager->load(false, Common::Path(kSmokeSoundPath), false);
		_sndTurtleSpin = soundManager->load(false, Common::Path(kTurtleSpinSoundPath), false);
		_sndTransition = soundManager->load(false, Common::Path(kTransitionSoundPath), false);
		_sndMismatch = soundManager->load(false, Common::Path(kMismatchSoundPath), false);
		_sndFall = soundManager->load(false, Common::Path(kFallSoundPath), false);
		_sndCollapse = soundManager->load(false, Common::Path(kCollapseSoundPath), false);
	}

	for (int type = 0; type < kFeatureCount; type++) {
		_turtleIdleRunners[type] = new AnimationRunner(_vm, Common::Point32(), AnimationRunnerMode::kPlayOnce00);
		if (_turtleIdleAnimations[type]) {
			_turtleIdleRunners[type]->setAnimation(_turtleIdleAnimations[type]);
			for (int frame = 0; frame < kTurtleSpinFrameCount; frame++)
				_turtleIdleRunners[type]->addTimedFrame(frame, kTurtleSpinFrameDelayMs);
		}
		_turtleIdleRunners[type]->setCompletionCallback(&onTurtleIdleSpinComplete, this);

		_turtleSpinRunners[type] = new AnimationRunner(_vm, Common::Point32(), AnimationRunnerMode::kPlayOnce00);
		if (_turtleSpinAnimations[type]) {
			_turtleSpinRunners[type]->setAnimation(_turtleSpinAnimations[type]);
			for (int frame = 0; frame < kTurtleSpinFrameCount; frame++)
				_turtleSpinRunners[type]->addTimedFrame(frame, kTurtleSpinFrameDelayMs);
		}
		_turtleSpinRunners[type]->setCompletionCallback(&onTurtleSpinComplete, this);
	}

	_smokeRunner = new AnimationRunner(_vm, Common::Point32(), AnimationRunnerMode::kPlayOnce00);
	if (_smokeAnimation) {
		_smokeRunner->setAnimation(_smokeAnimation);
		for (int frame = 0; frame < kSmokeFrameCount; frame++)
			_smokeRunner->addTimedFrame(frame, kSmokeFrameDelayMs);
	}

	_motherRunner = new AnimationRunner(_vm, kMotherPos, AnimationRunnerMode::kPlayOnce00);
	if (_motherAnimation) {
		_motherRunner->setAnimation(_motherAnimation);
		for (int frame = 0; frame < kMotherFrameCount; frame++)
			_motherRunner->addTimedFrame(frame, kMotherFrameDelayMs);
		_motherRunner->addTimedFrame(kMotherFrameCount - 1, kMotherFinalHoldMs);
		_motherRunner->setCompletionCallback(&onMotherAnimationComplete, this);
	}
}

void PuzzleCrazyTurtle::buildDropTargets() {
	for (int i = 0; i < kTurtleCount; i++) {
		const TurtlePlacement &placement = kTurtlePlacements[i];
		ZmbDropTarget target;
		target.rect = Common::Rect32(placement.pos.x + 15, placement.pos.y + 25, placement.pos.x + 15 + 60, placement.pos.y + 25 + 62);
		target.occupied = false;
		target.callback = &onTurtleDrop;
		target.callbackContext = this;
		target.zoombiniIndex = -1;
		_turtleDropTargets.push_back(target);
	}
}

void PuzzleCrazyTurtle::buildTurtleAssignments() {
	const int partySize = MIN<int>(kZoombiniCount, _puzzleZoombinis.size());
	bool used[kZoombiniCount] = {};
	int assignedCount = 0;
	for (int slot = 0; slot < kRuleSlotCount && assignedCount < kTurtleCount; slot++) {
		const int secondarySlots = 3 <= _puzzleLevel ? kRuleSlotCount : 1;
		for (int secondarySlot = 0; secondarySlot < secondarySlots; secondarySlot++) {
			for (int i = 0; i < partySize && assignedCount < kTurtleCount; i++) {
				const ZmbTrait &traits = _puzzleZoombinis[i]->getTraits();
				if (used[i] || traits.getValue(static_cast<ZmbTrait::TraitKind>(_primaryFeature)) != _ruleValues[0][slot])
					continue;
				if (3 <= _puzzleLevel && traits.getValue(static_cast<ZmbTrait::TraitKind>(_secondaryFeature)) != _ruleValues[1][secondarySlot])
					continue;
				used[i] = true;
				_turtleAssignments[assignedCount] = i;
				assignedCount += 1;
			}
		}
	}
	for (int i = assignedCount; i < kTurtleCount; i++)
		_turtleAssignments[i] = -1;
}

void PuzzleCrazyTurtle::generateRules() {
	_primaryFeature = _vm->_rnd->getRandomNumber(kFeatureCount - 1);
	generateFeatureOrder(0);
	int remainingFeatures[kFeatureCount - 1];
	int remainingCount = 0;
	for (int feature = 0; feature < kFeatureCount; feature++) {
		if (feature != _primaryFeature) {
			remainingFeatures[remainingCount] = feature;
			remainingCount += 1;
		}
	}
	_secondaryFeature = remainingFeatures[_vm->_rnd->getRandomNumber(remainingCount - 1)];
	generateFeatureOrder(1);
	memset(_primaryRuleActive, 0, sizeof(_primaryRuleActive));
	memset(_secondaryRuleActive, 0, sizeof(_secondaryRuleActive));

	switch (_puzzleLevel) {
	case 1:
		_remainingMistakes = 3;
		for (int slot = 0; slot < kRuleSlotCount; slot++)
			_primaryRuleActive[slot] = true;
		break;
	case 2: {
		const int visibleRuleCount = _vm->_rnd->getRandomNumber(1) + 1;
		_remainingMistakes = visibleRuleCount == 1 ? 5 : 4;
		activateRandomRuleSlots(_primaryRuleActive, visibleRuleCount);
		break;
	}
	case 3:
		_remainingMistakes = 6;
		activateRandomRuleSlots(_primaryRuleActive, 1);
		activateRandomRuleSlots(_secondaryRuleActive, 1);
		break;
	case 4:
		_remainingMistakes = 7;
		break;
	default:
		break;
	}
	_initialMistakes = _remainingMistakes;
}

void PuzzleCrazyTurtle::generateFeatureOrder(int group) {
	int values[kFeatureValueCount] = {
		1,
		2,
		3,
		4,
		5,
	};
	int remainingCount = kFeatureValueCount;
	for (int slot = 0; slot < kRuleSlotCount; slot++) {
		const int selected = _vm->_rnd->getRandomNumber(remainingCount - 1);
		_ruleValues[group][slot] = values[selected];
		for (int value = selected; value + 1 < remainingCount; value++)
			values[value] = values[value + 1];
		remainingCount -= 1;
	}
}

void PuzzleCrazyTurtle::activateRandomRuleSlots(bool *slots, int count) {
	int available[kRuleSlotCount] = {
		0,
		1,
		2,
		3,
		4,
	};
	int availableCount = kRuleSlotCount;
	for (int selectedCount = 0; selectedCount < count; selectedCount++) {
		const int selected = _vm->_rnd->getRandomNumber(availableCount - 1);
		slots[available[selected]] = true;
		for (int slot = selected; slot + 1 < availableCount; slot++)
			available[slot] = available[slot + 1];
		availableCount -= 1;
	}
}

void PuzzleCrazyTurtle::placeZoombinis() {
	const uint count = MIN<uint>(kZoombiniCount, _puzzleZoombinis.size());
	for (uint i = 0; i < count; i++) {
		ZoombiniRunner *zoombini = _puzzleZoombinis[i];
		zoombini->setPosition(kZoombiniPos[i]);
		zoombini->setDefaultAnimation(ZmbAnimationKind::kBasic, 33);
		zoombini->setInputEnabled(true);
		zoombini->setCanAdvanceFromPage(false);
	}
}

void PuzzleCrazyTurtle::onUpdate() {
	const uint32 tick = _vm->getTotalPlayTime();
	SoundManager *sound = _vm->getSoundManager();
	if (_goPending && (!sound || !sound->hasPendingSpeech())) {
		_vm->_mapTransitSrcPageId = PageId::kPageCrazyTurtle_01;
		_vm->requestPageChange(PageId::kPageMapTrans_22);
		return;
	}

	if (_smokePending) {
		_smokePending = false;
		playSound(_sndSmoke);
		if (_smokeRunner)
			_smokeRunner->startAt(kSmokePos, tick);
		_mistakesMirror = _remainingMistakes;
	}

	if (_transitionRequested) {
		_transitionRequested = false;
		startTransitionSequence();
	}

	if (_idlePhaseEnabled) {
		for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
			ZoombiniRunner *zoombini = _puzzleZoombinis[i];
			zoombini->tryStartCelebrationAnimation(ZmbAnimationKind::kCelebration, *_vm->_rnd, tick, _vm->getFrameDeltaMs(), _vm->getLogicPacingHz());
		}
		updateZoombiniAnimations(tick);
		return;
	}

	updateFallSequence(tick);
	updateIdleTurtleSpin(tick);
	updateZoombiniAnimations(tick);
}

void PuzzleCrazyTurtle::onRenderBackground(ManagedSurface32 *screen) {
	drawPrimaryPageLayer(screen);
}

void PuzzleCrazyTurtle::onRenderContent(ManagedSurface32 *screen) {
	drawBridgeState(screen);
	drawMother(screen);
	drawTurtles(screen);
	drawTurtleRunners(screen);
	drawRuleHints(screen);
}

void PuzzleCrazyTurtle::onRenderActors(ManagedSurface32 *screen) {
	renderZoombinis(screen);
}

void PuzzleCrazyTurtle::onActorsRendered() {
	for (uint i = 0; i < _puzzleZoombinis.size(); i++)
		_puzzleZoombinis[i]->advanceAnimationAfterDraw();
}

void PuzzleCrazyTurtle::updateFallSequence(uint32 tick) {
	if (_fallPhase == 0 && _fallTimerTick < tick) {
		_fallPhase = 1;
		_fallTimerTick = 0xFFFFFFFFU;
		const TurtlePlacement &placement = kTurtlePlacements[_activeTurtleIndex];
		if (_turtleSpinRunners[placement.type - 1])
			_turtleSpinRunners[placement.type - 1]->startAt(placement.pos, tick);
		playSound(_sndTurtleSpin);
	}

	if (_fallPhase == 1) {
		ZoombiniRunner *rising = nullptr;
		for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
			if (_puzzleZoombinis[i]->hasMovementPath()) {
				rising = _puzzleZoombinis[i];
				break;
			}
		}
		if (!rising)
			return;
		if (!rising->isMovementFinished()) {
			rising->advanceMovement(tick);
			return;
		}
		rising->clearMovement();
		_fallPhase = 2;
		const int fallback = _fallbackTurtleIndex;
		const Common::Point32 landing = 0 <= fallback && fallback < kTurtleCount ? kLandingPos[fallback] : Common::Point32();
		const int mid = (landing.y + 400) / 3;
		PathObject *descent = new PathObject(_vm);
		descent->appendSegment(Common::Point32(landing.x, -400), Common::Point32(landing.x, mid - 400),
							   Common::Point32(landing.x, landing.y - mid), Common::Point32(landing.x, landing.y), 6, 0);
		_puzzleZoombinis[_fallZoombiniIndex]->startMovement(descent, tick);
	}

	if (_fallPhase == 2) {
		int fallingIndex = -1;
		for (int i = 0; i < static_cast<int>(_puzzleZoombinis.size()); i++) {
			if (_puzzleZoombinis[i]->hasMovementPath()) {
				fallingIndex = i;
				break;
			}
		}
		if (fallingIndex < 0)
			return;
		ZoombiniRunner *falling = _puzzleZoombinis[fallingIndex];
		if (!falling->isMovementFinished()) {
			falling->advanceMovement(tick);
			return;
		}
		falling->clearMovement();
		falling->resetAnimation();
		falling->startAnimation(ZmbAnimationKind::kFalling, 33, tick);
		falling->setAnimationCompleteCallback(&onTurtleFallComplete, this);
		_lastFallZoombiniIndex = fallingIndex;
		_fallCounter = 0;
		_fallPhase = 0;
		playSound(_sndFall);
		_vm->_zoombiniWalkingFlag = true;
	}
}

void PuzzleCrazyTurtle::updateIdleTurtleSpin(uint32 tick) {
	if (_idleTurtleIndex != -1)
		return;
	int eligible = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (_puzzleZoombinis[i]->isInputEnabled())
			eligible += 1;
	}
	if (eligible == 1 || _inputLocked)
		return;
	const int turtleIndex = _vm->_rnd->getRandomNumber(kTurtleCount - 1);
	_idleTurtleIndex = turtleIndex;
	const TurtlePlacement &placement = kTurtlePlacements[turtleIndex];
	if (_turtleIdleRunners[placement.type - 1])
		_turtleIdleRunners[placement.type - 1]->startAt(placement.pos, tick);
	playSound(_sndTurtleIdle);
}

void PuzzleCrazyTurtle::updateZoombiniAnimations(uint32 tick) {
	for (uint i = 0; i < _puzzleZoombinis.size(); i++)
		_puzzleZoombinis[i]->updateAnimation(tick);
}

void PuzzleCrazyTurtle::startTransitionSequence() {
	_vm->restartGoBlink();
	playSound(_sndTransition);
	if (_motherRunner && _motherAnimation)
		_motherRunner->reset(_vm->getTotalPlayTime());
	_idlePhaseEnabled = true;
}

void PuzzleCrazyTurtle::playSound(int soundId) const {
	if (soundId < 0)
		return;
	SoundManager *soundManager = _vm->getSoundManager();
	if (soundManager)
		soundManager->playWithVolume(soundId, soundManager->getSfxVolume());
}

int PuzzleCrazyTurtle::countFreeZoombinis() const {
	int freeCount = 0;
	for (uint i = 0; i < _puzzleZoombinis.size(); i++) {
		if (!_puzzleZoombinis[i]->canAdvanceFromPage())
			freeCount += 1;
	}
	return freeCount;
}

bool PuzzleCrazyTurtle::canUseGoButton() const {
	return !_goPending && _vm->_zoombiniWalkingFlag;
}

bool PuzzleCrazyTurtle::onGoButtonPressed() {
	if (_vm->inPracticeMode())
		return true;
	if (_goPending)
		return false;
	const int freeCount = countFreeZoombinis();
	if (0 < freeCount && freeCount < 4)
		return true;
	Common::String speech = kRetreatSpeech;
	if (freeCount == 0) {
		const int variant = _vm->_rnd->getRandomNumber(4) + 1;
		speech = Common::String::format(kGoSpeechFormat, variant);
	}
	SoundManager *sound = _vm->getSoundManager();
	if (sound)
		sound->queueSpeech(Common::Path(speech));
	_goPending = true;
	return false;
}

bool PuzzleCrazyTurtle::evaluateTurtleMatch(const ZoombiniRunner *zoombini, int turtleIndex) const {
	if (!zoombini || turtleIndex < 0 || kTurtleCount <= turtleIndex)
		return false;
	const int assigned = _turtleAssignments[turtleIndex];
	if (assigned < 0)
		return false;
	const ZoombiniRunner *required = _puzzleZoombinis[assigned];
	if (!required)
		return false;
	if (zoombini->getTraits().getValue(static_cast<ZmbTrait::TraitKind>(_primaryFeature)) != required->getTraits().getValue(static_cast<ZmbTrait::TraitKind>(_primaryFeature)))
		return false;
	if (3 <= _puzzleLevel && zoombini->getTraits().getValue(static_cast<ZmbTrait::TraitKind>(_secondaryFeature)) !=
								 required->getTraits().getValue(static_cast<ZmbTrait::TraitKind>(_secondaryFeature)))
		return false;
	return true;
}

void PuzzleCrazyTurtle::handleTurtleClick(int turtleIndex, int zoombiniIndex) {
	const uint32 tick = _vm->getTotalPlayTime();
	if (turtleIndex < 0 || kTurtleCount <= turtleIndex || zoombiniIndex < 0 || _puzzleZoombinis.size() <= static_cast<uint>(zoombiniIndex) || _mistakesMirror <= 0)
		return;
	_inputLocked = true;

	ZoombiniRunner *zoombini = _puzzleZoombinis[zoombiniIndex];
	zoombini->setInputEnabled(false);
	const TurtlePlacement &placement = kTurtlePlacements[turtleIndex];
	if (1 <= placement.type && placement.type <= kFeatureCount)
		zoombini->setPosition(Common::Point32(placement.pos.x + 20, placement.pos.y));

	if (evaluateTurtleMatch(zoombini, turtleIndex)) {
		zoombini->startAnimation(ZmbAnimationKind::kCelebration, 33, tick);
		zoombini->setAnimationCompleteCallback(&onCelebrationComplete, this);
		zoombini->setCanAdvanceFromPage(true);
		_vm->_zoombiniWalkingFlag = true;
		int placedCount = 0;
		for (const ZoombiniRunner *actor : _puzzleZoombinis) {
			if (actor->canAdvanceFromPage())
				placedCount += 1;
		}
		if (placedCount == static_cast<int>(_puzzleZoombinis.size()))
			_vm->restartGoBlink();
		return;
	}

	_remainingMistakes -= 1;
	if (_remainingMistakes == 0) {
		for (uint i = 0; i < _puzzleZoombinis.size(); i++)
			_puzzleZoombinis[i]->setInputEnabled(false);
	}

	int fallback = -1;
	for (int candidate = 0; candidate < kTurtleCount; candidate++) {
		if (_turtleDropTargets[candidate].occupied)
			continue;
		if (evaluateTurtleMatch(zoombini, candidate))
			fallback = candidate;
	}

	_fallbackTurtleIndex = fallback;
	_fallZoombiniIndex = zoombiniIndex;
	_fallPhase = 0;
	_fallTimerTick = tick + kFallReactionDelayMs;
	_activeTurtleIndex = turtleIndex;
	playSound(_sndMismatch);

	_turtleDropTargets[turtleIndex].occupied = false;
	if (0 <= fallback) {
		_turtleDropTargets[fallback].occupied = true;
		_turtleDropTargets[fallback].zoombiniIndex = zoombiniIndex;
	}

	const Common::Point32 start = zoombini->getScreenPosition();
	PathObject *ascent = new PathObject(_vm);
	ascent->appendSegment(start, Common::Point32(start.x, start.y + (10 - start.y) / 10),
						  Common::Point32(start.x, 10 - (10 - start.y) / 5), Common::Point32(start.x, 10), 6, 0);
	zoombini->startMovement(ascent, tick);
	zoombini->setCanAdvanceFromPage(true);
}

void PuzzleCrazyTurtle::onTurtleDrop(void *context, int targetIndex, int zoombiniIndex) {
	static_cast<PuzzleCrazyTurtle *>(context)->handleTurtleClick(targetIndex, zoombiniIndex);
}

void PuzzleCrazyTurtle::onCelebrationComplete(void *context, ZoombiniRunner *zoombini) {
	(void)zoombini;
	PuzzleCrazyTurtle *page = static_cast<PuzzleCrazyTurtle *>(context);
	page->_inputLocked = false;
	page->applyPendingDebugChanceSet();
	for (uint i = 0; i < page->_puzzleZoombinis.size(); i++) {
		if (page->_puzzleZoombinis[i]->isInputEnabled())
			return;
	}
	page->_transitionRequested = true;
	page->_vm->_zoombiniWalkingFlag = true;
}

void PuzzleCrazyTurtle::onTurtleFallComplete(void *context, ZoombiniRunner *zoombini) {
	PuzzleCrazyTurtle *page = static_cast<PuzzleCrazyTurtle *>(context);
	if (page->_fallCounter < 2) {
		page->_fallCounter += 1;
		zoombini->startAnimation(ZmbAnimationKind::kFalling, 33, page->_vm->getTotalPlayTime());
		zoombini->setAnimationCompleteCallback(&onTurtleFallComplete, page);
		return;
	}
	page->_smokePending = true;
	page->_inputLocked = false;
	page->applyPendingDebugChanceSet();
	if (page->_remainingMistakes <= 0)
		page->playSound(page->_sndCollapse);
	if (page->_mistakesMirror <= 0)
		page->_transitionRequested = true;
	for (const ZoombiniRunner *actor : page->_puzzleZoombinis)
		if (actor->canAdvanceFromPage())
			page->_vm->_zoombiniWalkingFlag = true;
}

void PuzzleCrazyTurtle::onTurtleIdleSpinComplete(void *context, AnimationRunner *runner) {
	(void)runner;
	PuzzleCrazyTurtle *page = static_cast<PuzzleCrazyTurtle *>(context);
	page->_idleTurtleIndex = -1;
	page->_turtleReady = true;
}

void PuzzleCrazyTurtle::onTurtleSpinComplete(void *context, AnimationRunner *runner) {
	(void)runner;
	PuzzleCrazyTurtle *page = static_cast<PuzzleCrazyTurtle *>(context);
	page->_idleTurtleIndex = -1;
	page->_turtleReady = true;
}

void PuzzleCrazyTurtle::onMotherAnimationComplete(void *context, AnimationRunner *runner) {
	(void)runner;
	PuzzleCrazyTurtle *page = static_cast<PuzzleCrazyTurtle *>(context);
	page->_motherFinished = true;
	const char *speechPath = page->countFreeZoombinis() == 0 ? kMotherSuccessSpeechPath : kMotherPartialSpeechPath;
	SoundManager *sound = page->_vm->getSoundManager();
	if (sound)
		sound->queueSpeech(Common::Path(speechPath));
}

void PuzzleCrazyTurtle::drawBridgeState(ManagedSurface32 *screen) {
	// The dock redraw consumes the mirrored mistake count, which the frame
	// handler refreshes after a fall, or a deferred debugger edit after feedback.
	Common::Point32 beamPos(11 * _mistakesMirror + 9, 310 - 12 * _mistakesMirror);
	for (int beam = 0; beam < _mistakesMirror; beam++) {
		_vm->_gfx->drawPageRleBlock(screen, kBeamPath, beamPos);
		beamPos.x -= 11;
		beamPos.y += 12;
	}
	if (0 < _mistakesMirror) {
		_vm->_gfx->drawPageRleBlock(screen, kBridgePath, Common::Point32(10, 220));
	} else {
		_vm->_gfx->drawPageRleBlock(screen, kCollapsedBridgePath, Common::Point32(10, 220));
		if (!_idlePhaseEnabled)
			_transitionRequested = true;
	}
}

void PuzzleCrazyTurtle::drawRuleHints(ManagedSurface32 *screen) const {
	for (int slot = 0; slot < kRuleSlotCount; slot++) {
		if (!_primaryRuleActive[slot])
			continue;
		const Common::String primaryPath = Common::String::format(kTraitFormat, _primaryFeature + 1, _ruleValues[0][slot]);
		const Common::Point32 primaryPos(kPrimaryIconPos[slot].x - 5, kPrimaryIconPos[slot].y);
		if (_primaryFeature == static_cast<int>(ZmbTrait::TraitKind::kNose01))
			_vm->_gfx->drawPageNoseTraitSprite(screen, primaryPath, primaryPos, static_cast<byte>(_ruleValues[0][slot]));
		else
			_vm->_gfx->drawPageRleBlock(screen, primaryPath, primaryPos);
		if (3 <= _puzzleLevel) {
			const Common::String secondaryPath = Common::String::format(kTraitFormat, _secondaryFeature + 1, _ruleValues[1][slot]);
			const Common::Point32 secondaryPos(kSecondaryIconPos[slot].x - 5, kSecondaryIconPos[slot].y);
			if (_secondaryFeature == static_cast<int>(ZmbTrait::TraitKind::kNose01))
				_vm->_gfx->drawPageNoseTraitSprite(screen, secondaryPath, secondaryPos, static_cast<byte>(_ruleValues[1][slot]));
			else
				_vm->_gfx->drawPageRleBlock(screen, secondaryPath, secondaryPos);
		}
	}
}

bool PuzzleCrazyTurtle::hasActiveTurtleRunner(int turtleIndex) const {
	const TurtlePlacement &placement = kTurtlePlacements[turtleIndex];
	const AnimationRunner *idleRunner = _turtleIdleRunners[placement.type - 1];
	if (idleRunner && idleRunner->isActive() && _idleTurtleIndex == turtleIndex)
		return true;
	const AnimationRunner *spinRunner = _turtleSpinRunners[placement.type - 1];
	return spinRunner && spinRunner->isActive() && _activeTurtleIndex == turtleIndex;
}

void PuzzleCrazyTurtle::drawTurtles(ManagedSurface32 *screen) const {
	for (int i = 0; i < kTurtleCount; i++) {
		if (hasActiveTurtleRunner(i))
			continue;
		const TurtlePlacement &placement = kTurtlePlacements[i];
		const Common::String imagePath = Common::String::format(kTurtleFixedFormat, placement.type, placement.type);
		_vm->_gfx->drawPageRleBlock(screen, imagePath, placement.pos);
	}
}

void PuzzleCrazyTurtle::drawTurtleRunners(ManagedSurface32 *screen) const {
	const uint32 tick = _vm->getTotalPlayTime();
	for (int type = 0; type < kFeatureCount; type++) {
		_vm->_gfx->drawAndUpdateAnimationRunner(screen, _turtleIdleRunners[type], tick, 0, Gfx::kScreenSize.width);
		_vm->_gfx->drawAndUpdateAnimationRunner(screen, _turtleSpinRunners[type], tick, 0, Gfx::kScreenSize.width);
	}
	_vm->_gfx->drawAndUpdateAnimationRunner(screen, _smokeRunner, tick, 0, Gfx::kScreenSize.width);
	_vm->_gfx->drawAndUpdateAnimationRunner(screen, _motherRunner, tick, 0, Gfx::kScreenSize.width);
}

void PuzzleCrazyTurtle::drawMother(ManagedSurface32 *screen) const {
	if (_motherFinished) {
		_vm->_gfx->drawPageRleBlock(screen, kMotherEndPath, kMotherPos);
	} else if (!_motherRunner || !_motherRunner->isActive()) {
		_vm->_gfx->drawPageRleBlock(screen, kMotherStartPath, kMotherPos);
	}
}

EventHandleResult PuzzleCrazyTurtle::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	// Placement is release-driven: the press only arms the shared sidebar.
	return EventHandleResult::kPassthrough;
}

EventHandleResult PuzzleCrazyTurtle::onLButtonUp(const Common::Point &pos) {
	// While a placement or fall sequence runs, the original blocks pickup and
	// treats the release as movement-only input.
	ZmbHoldDropInput input;
	if (!_inputLocked)
		input = ZmbHoldDropInput::kLeftButtonRelease;
	else
		input = ZmbHoldDropInput::kMoveOnly;
	const ZmbDropResult inputResult =
		ZoombiniRunner::handleHoldAndDrop(_vm, _puzzleZoombinis, Common::Point32(pos.x, pos.y), input, ZmbAnimationKind::kPickup,
										  &_turtleDropTargets, getAreaMask());
	switch (inputResult) {
	case ZmbDropResult::kIgnored00:
		return EventHandleResult::kPassthrough;
	default:
		return EventHandleResult::kConsumed;
	}
}

EventHandleResult PuzzleCrazyTurtle::onMouseMove(const Common::Point &pos) {
	const ZmbDropResult inputResult =
		ZoombiniRunner::handleHoldAndDrop(_vm, _puzzleZoombinis, Common::Point32(pos.x, pos.y), ZmbHoldDropInput::kMoveOnly, ZmbAnimationKind::kPickup,
										  &_turtleDropTargets, getAreaMask());
	switch (inputResult) {
	case ZmbDropResult::kIgnored00:
		return EventHandleResult::kPassthrough;
	default:
		return EventHandleResult::kConsumed;
	}
}

Common::String PuzzleCrazyTurtle::debugGetAnswer() const {
	Common::String answer = debugAnswerHeader();
	const ZmbTrait::TraitKind primaryFeature = static_cast<ZmbTrait::TraitKind>(_primaryFeature);
	const ZmbTrait::TraitKind secondaryFeature = static_cast<ZmbTrait::TraitKind>(_secondaryFeature);
	Common::String primaryName = ZmbTrait::debugTraitName(primaryFeature);
	Common::String secondaryName = ZmbTrait::debugTraitName(secondaryFeature);
	primaryName.toUppercase();
	secondaryName.toUppercase();
	answer += "\n  [*] Sorting order:\n";
	const int ruleGroups = 3 <= _puzzleLevel ? kRuleGroupCount : 1;
	for (int group = 0; group < ruleGroups; group++) {
		const ZmbTrait::TraitKind feature = group == 0 ? primaryFeature : secondaryFeature;
		const Common::String &featureName = group == 0 ? primaryName : secondaryName;
		answer += Common::String::format("    %-10s %s (", group == 0 ? "Primary:" : "Secondary:", featureName.c_str());
		for (int slot = 0; slot < kRuleSlotCount; slot++) {
			if (0 < slot)
				answer += " -> ";
			answer += ZmbTrait::debugTraitValueName(feature, _ruleValues[group][slot]);
		}
		answer += ")\n";
	}
	if (3 <= _puzzleLevel)
		answer += "  Secondary order applies within each Primary group.\n";
	answer += '\n';
	answer += "  [*] Ordered turtle groups for playable turtles (left -> right):\n";
	for (uint first = 0; first < _puzzleZoombinis.size();) {
		const ZmbTrait &traits = _puzzleZoombinis[_turtleAssignments[first]]->getTraits();
		uint last = first;
		while (last + 1 < _puzzleZoombinis.size() && evaluateTurtleMatch(_puzzleZoombinis[_turtleAssignments[last + 1]], first))
			last += 1;
		if (first == last)
			answer += Common::String::format("    Turtle %u: ", first + 1);
		else
			answer += Common::String::format("    Turtles %u-%u: ", first + 1, last + 1);
		answer += Common::String::format("(%s) %s", primaryName.c_str(),
										 ZmbTrait::debugTraitValueName(primaryFeature, traits.getValue(primaryFeature)));
		if (3 <= _puzzleLevel)
			answer += Common::String::format(" (%s) %s", secondaryName.c_str(),
											 ZmbTrait::debugTraitValueName(secondaryFeature, traits.getValue(secondaryFeature)));
		answer += "\n      Matchable Zoombinis:\n";
		for (uint actor = 0; actor < _puzzleZoombinis.size(); actor++) {
			if (evaluateTurtleMatch(_puzzleZoombinis[actor], first))
				answer += Common::String::format("        %s\n", debugActorDescription(actor).c_str());
		}
		first = last + 1;
	}
	return answer;
}

PuzzleChanceInfo PuzzleCrazyTurtle::debugGetChances() const {
	return PuzzleChanceInfo(PuzzleChanceInfo::Type::kMistake, _initialMistakes, _initialMistakes - _remainingMistakes, "incorrect placement");
}

bool PuzzleCrazyTurtle::debugCanSetChances() const {
	return !_debugFinishPending && !_idlePhaseEnabled && !_goPending && !_transitionRequested && 0 < _remainingMistakes && 0 < countFreeZoombinis();
}

bool PuzzleCrazyTurtle::debugSetChances(int remaining) {
	if (!debugCanSetChances() || remaining < 1 || _initialMistakes < remaining)
		return false;
	_remainingMistakes = remaining;
	_debugChanceSetBridgeUpdatePending = true;
	applyPendingDebugChanceSet();
	return true;
}

void PuzzleCrazyTurtle::applyPendingDebugChanceSet() {
	if (!_debugChanceSetBridgeUpdatePending || _inputLocked)
		return;
	_mistakesMirror = _remainingMistakes;
	_debugChanceSetBridgeUpdatePending = false;
}

} // End of namespace Zoombini2
