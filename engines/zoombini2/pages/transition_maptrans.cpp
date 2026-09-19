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
#include "common/file.h"
#include "common/tokenizer.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/transition_maptrans.h"
#include "zoombini2/scripts.h"
#include "zoombini2/sound.h"
#include "zoombini2/state.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

constexpr const char *TransitionMapTrans::kSpeechFormat;
constexpr const char *TransitionMapTrans::kRouteFormat;
constexpr const char *TransitionMapTrans::kZoombiniAnimationPath;
constexpr const char *TransitionMapTrans::kMusicPath;

static constexpr int kRescue1MovieMinimumZoombinis = 8;

// ============================================================================
// TransitionMapTrans - route-map transition.
// ============================================================================

TransitionMapTrans::TransitionMapTrans(Zoombini2Engine *vm)
	: TransitionBase(vm) {
	_pageId = kPageMapTrans;
}

TransitionMapTrans::~TransitionMapTrans() {
	cleanupSpeech();
	if (_musicId >= 0) {
		SoundManager *snd = _vm->getSoundManager();
		snd->stop(_musicId);
		snd->unload(_musicId);
	}
	cleanupPaths();
	delete _compositedBg;
}

void TransitionMapTrans::queueSpeech(const Common::String &name) {
	SoundManager *sound = _vm->getSoundManager();
	if (!sound)
		return;
	const Common::Path path(Common::String::format(kSpeechFormat, name.c_str()));
	_speechIds.push_back(sound->load(true, path, false));
}

void TransitionMapTrans::queueTravelSpeech(PageId sourcePage) {
	GameState *state = _vm->getGameState();
	if (!state)
		return;

	switch (sourcePage) {
	case kPageZombiniville:
		if (!state->hasPageVisit(kPageCrazyTurtle, 1)) {
			queueSpeech("tur11");
		} else {
			queueSpeech(state->hasPageVisit(kPageBooliewood, 1) ? "zbv31.3" : "zbv31.2");
			queueSpeech(Common::String::format("tur21.%d", getRandomBinarySpeechVariant()));
		}
		break;
	case kPageCrazyTurtle:
		queueSpeech(state->hasPageVisit(kPageWaterslide, 1) ? Common::String::format("wsl21.%d", getRandomBinarySpeechVariant()) : "wsl11");
		break;
	case kPageWaterslide:
		if (!state->hasPageVisit(kPageAquacube, 1)) {
			queueSpeech("aqu11.1.1");
			queueSpeech("aqu11.1.2");
			queueSpeech("aqu11.1.3");
		} else {
			queueSpeech(Common::String::format("aqu21.%d", getRandomBinarySpeechVariant()));
		}
		break;
	case kPageAquacube:
		queueSpeech("aqu31");
		break;
	case kPageRescue1:
		if (_vm->_routeDirection == RouteBranch::kLeft01)
			queueSpeech(state->hasPageVisit(kPageMagicWall, 1) ? Common::String::format("mgw21.%d", getRandomBinarySpeechVariant()) : "mgw11");
		else
			queueSpeech(state->hasPageVisit(kPageMysticMarsh, 1) ? Common::String::format("mym21.%d", getRandomBinarySpeechVariant()) : "mym11");
		break;
	case kPageMysticMarsh:
		queueSpeech(state->hasPageVisit(kPageWallOfFleens, 1) ? Common::String::format("wlf21.%d", getRandomBinarySpeechVariant()) : "wlf11");
		break;
	case kPageMagicWall:
		if (!state->hasPageVisit(kPageChezNorf, 1)) {
			queueSpeech("czn11.1");
			queueSpeech("czn11.2");
		} else {
			queueSpeech(Common::String::format("czn21.%d", getRandomBinarySpeechVariant()));
		}
		break;
	case kPageWallOfFleens:
		queueSpeech("bc211");
		break;
	case kPageChezNorf:
		queueSpeech("bc212");
		break;
	case kPageRescue2:
		if (!state->hasPageVisit(kPageSnowboard, 1)) {
			queueSpeech("swb11");
			queueSpeech("swb11B");
		} else {
			queueSpeech(Common::String::format("swb21.%d", getRandomBinarySpeechVariant()));
		}
		break;
	case kPageSnowboard:
		if (!state->hasPageVisit(kPageBoolies, 1)) {
			queueSpeech("blp11");
			queueSpeech("blp11B");
		} else {
			queueSpeech(Common::String::format("blp21.%d", getRandomBinarySpeechVariant()));
		}
		break;
	case kPageBoolies:
		if (!state->hasPageVisit(kPageBooliewood, 1)) {
			queueSpeech("blw11.1");
			queueSpeech("blw11.3");
		} else {
			queueSpeech(Common::String::format("blw12.%d", _vm->_rnd->getRandomNumber(2) + 1));
		}
		break;
	default:
		break;
	}
}

int TransitionMapTrans::getRandomBinarySpeechVariant() {
	return _vm->_rnd->getRandomNumber(1) == 0 ? 2 : 1;
}

void TransitionMapTrans::updateSpeechQueue() {
	SoundManager *sound = _vm->getSoundManager();
	if (!sound) {
		_activeSpeechIndex = -1;
		_nextSpeechIndex = _speechIds.size();
		return;
	}
	if (_activeSpeechIndex != -1) {
		const int soundId = _speechIds[_activeSpeechIndex];
		if (0 <= soundId && sound->isPlaying(soundId))
			return;
		if (0 <= soundId)
			sound->unload(soundId);
		_speechIds[_activeSpeechIndex] = -1;
		_activeSpeechIndex = -1;
	}
	while (_nextSpeechIndex < _speechIds.size()) {
		const uint speechIndex = _nextSpeechIndex;
		_nextSpeechIndex += 1;
		if (_speechIds[speechIndex] < 0)
			continue;
		_activeSpeechIndex = static_cast<int>(speechIndex);
		sound->playWithVolume(_speechIds[speechIndex], sound->_volumeSpeech);
		return;
	}
}

bool TransitionMapTrans::hasPendingSpeech() const {
	return _activeSpeechIndex != -1 || _nextSpeechIndex < _speechIds.size();
}

void TransitionMapTrans::cleanupSpeech() {
	SoundManager *sound = _vm->getSoundManager();
	if (sound) {
		for (uint i = 0; i < _speechIds.size(); i++) {
			if (0 <= _speechIds[i]) {
				sound->stop(_speechIds[i]);
				sound->unload(_speechIds[i]);
			}
		}
	}
	_speechIds.clear();
	_nextSpeechIndex = 0;
	_activeSpeechIndex = -1;
}

void TransitionMapTrans::cleanupPaths() {
	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _vm->_globalZoombinis[i];
		if (!zoombini)
			continue;
		zoombini->clearMovement();
		zoombini->resetAnimation();
		zoombini->_hidden = false;
	}
}

void TransitionMapTrans::init() {
	debug(1, "MapTransition::init (source=%d, route=%d)",
		  static_cast<int>(_vm->_mapTransitionSourcePageId), static_cast<int>(_vm->_routeDirection));

	const PageId src = _vm->_mapTransitionSourcePageId;
	const RouteBranch routeBranch = _vm->_routeDirection;
	_vm->_skipMode = false;
	_transitionFinished = false;
	cleanupSpeech();

	// Select the map region from the source page.
	int mapRegion;
	if (kPageZombiniville <= src && src <= kPageAquacube)
		mapRegion = 1;
	else if (kPageRescue1 <= src && src <= kPageChezNorf)
		mapRegion = 2;
	else
		mapRegion = 3;

	// Select the walking path from the source page.
	Common::String patName;
	switch (src) {
	case kPageZombiniville:
		patName = "tr1 - map1.pat";
		break;
	case kPageCrazyTurtle:
		patName = "tr2 - map1.pat";
		break;
	case kPageWaterslide:
		patName = "tr3 - map1.pat";
		break;
	case kPageAquacube:
		patName = "tr4 - map1.pat";
		break;
	case kPageRescue1:
		if (routeBranch == RouteBranch::kLeft01)
			patName = "tr5 - map2.pat";
		else if (routeBranch == RouteBranch::kRight02)
			patName = "tr8 - map2.pat";
		break;
	case kPageMysticMarsh:
		patName = "tr9 - map2.pat";
		break;
	case kPageMagicWall:
		patName = "tr6 - map2.pat";
		break;
	case kPageWallOfFleens:
		patName = "tr10 - map2.pat";
		break;
	case kPageChezNorf:
		patName = "tr7 - map2.pat";
		break;
	case kPageRescue2:
		patName = "tr11 - map3.pat";
		break;
	case kPageSnowboard:
		patName = "tr12 - map3.pat";
		break;
	case kPageBoolies:
		patName = "tr13 - map3.pat";
		break;
	default:
		patName = "Transition02.pat";
		break;
	}
	_patPath = Common::Path(Common::String::format(kRouteFormat, patName.c_str()));

	_targetPageId = getDestPage(src, routeBranch, _vm->getGameState()->_rescuedBoolieCount);
	if (_targetPageId == kPageNone) {
		warning("MapTransition: refusing invalid source/branch combination (%d, %d)",
				static_cast<int>(src), static_cast<int>(routeBranch));
		_vm->requestPageChange(src);
		return;
	}

	// Request the graphics interface to compose the background and overlays.
	delete _compositedBg;
	_compositedBg = _vm->_gfx->createMapTransitionBackground(src, mapRegion, routeBranch);

	_zoombiniAnimation = _vm->loadZoombiniAnimation(Common::Path(kZoombiniAnimationPath), 50);
	if (!_zoombiniAnimation)
		warning("MapTransition: Failed to load PitiZomb3.anm");

	// Preserve the previous page grid for cleanup, then install the map-only grid.
	const uint32 now = _vm->getGameTickCount();
	int numZoombinis = static_cast<int>(_vm->_globalZoombinis.size());
	for (int i = 0; i < numZoombinis; i++) {
		ZoombiniRunner *zoombini = _vm->_globalZoombinis[i];
		zoombini->clearMovement();
		zoombini->_hidden = true;
		zoombini->_inputEnabled = false;
		zoombini->startDirectionTrackedAnimation(now);
		zoombini->setActiveAnimation(_zoombiniAnimation);
	}
	_nextWalkIndex = 0;
	_nextWalkTime = 0;
	_completedCount = 0;

	debug(1, "MapTransition: source=%d -> target=%d (region %d, path=%s, zoombinis=%d)",
		  src, _targetPageId, mapRegion, _patPath.toString().c_str(), numZoombinis);
	queueTravelSpeech(src);

	// Play transition music
	_musicId = _vm->getSoundManager()->load(true,
											Common::Path(kMusicPath), true);
	if (_musicId >= 0)
		_vm->getSoundManager()->play(_musicId);
	updateSpeechQueue();
}

/** Start and advance one independent path per Zoombini with an 800-millisecond stagger. */
void TransitionMapTrans::walkZoombinis() {
	uint32 now = _vm->getGameTickCount();
	int numZoombinis = static_cast<int>(_vm->_globalZoombinis.size());

	// Start the next Zoombini walking if its strict 800-millisecond gate has passed.
	if (_nextWalkIndex < numZoombinis && now > _nextWalkTime) {
		_nextWalkTime = now + 800;

		PathObject *path = PathObject::loadFromPAT(_vm, _patPath);
		if (path) {
			ZoombiniRunner *zoombini = _vm->_globalZoombinis[_nextWalkIndex];
			zoombini->_hidden = false;
			path->setStepValueForAllSegments(2);
			zoombini->startMovement(path, now);
		} else {
			warning("MapTransition: cannot start walker %d without path '%s'", _nextWalkIndex, _patPath.toString().c_str());
			_completedCount += 1;
		}
		_nextWalkIndex += 1;
	}

	// Update all walking zoombinis
	for (int i = 0; i < numZoombinis; i++) {
		ZoombiniRunner *zoombini = _vm->_globalZoombinis[i];
		if (!zoombini->_movementPath)
			continue;
		if (zoombini->_movementPath->finished) {
			zoombini->_hidden = true;
			zoombini->clearMovement();
			_completedCount += 1;
			continue;
		}
		zoombini->advanceMovement(now, Common::Point32(13, 13));
		zoombini->updateAnimation(now);
	}

	updateSpeechQueue();
	if (numZoombinis <= _completedCount && !hasPendingSpeech())
		finishTransition();
}

void TransitionMapTrans::onUpdate() {
	walkZoombinis();
}

PageId TransitionMapTrans::getPostTransitionPage() const {
	GameState *gameState = _vm->getGameState();
	if (!gameState)
		return _targetPageId;

	const PageId src = _vm->_mapTransitionSourcePageId;
	if (src == kPageAquacube && !gameState->hasPlayedRescue1Movie()) {
		const int zoombiniCount = static_cast<int>(_vm->_globalZoombinis.size()) + gameState->_rescue1ArrivalCount;
		if (kRescue1MovieMinimumZoombinis <= zoombiniCount)
			return kPageCutsceneSecond;
	}

	if ((src == kPageWallOfFleens || src == kPageChezNorf) && !gameState->hasPlayedRescue2Movie()) {
		return kPageCutsceneThird;
	}

	return _targetPageId;
}

void TransitionMapTrans::finishTransition() {
	if (_transitionFinished)
		return;
	_transitionFinished = true;

	const PageId nextPage = getPostTransitionPage();
	_vm->_mapTransitionSourcePageId = _targetPageId;
	_vm->requestPageChange(nextPage);
}

void TransitionMapTrans::onRenderContent(ManagedSurface32 *screen) {
	// Draw composited background (map + overlays)
	if (_compositedBg)
		screen->blitFrom(*_compositedBg, Common::Point32(0, 0));
}

void TransitionMapTrans::onRenderActors(ManagedSurface32 *screen) {
	// Sort walkers by vertical position so lower sprites overlap higher ones.
	int numZoombinis = static_cast<int>(_vm->_globalZoombinis.size());

	// Build sort order by Y
	Common::Array<int> drawOrder;
	for (int i = 0; i < numZoombinis; i++) {
		const ZoombiniRunner *zoombini = _vm->_globalZoombinis[i];
		if (zoombini->_movementPath && !zoombini->_hidden)
			drawOrder.push_back(i);
	}

	// The party is small enough for a direct pairwise sort.
	for (uint i = 0; i < drawOrder.size(); i++) {
		for (uint j = i + 1; j < drawOrder.size(); j++) {
			if (_vm->_globalZoombinis[drawOrder[j]]->_screenPos.y < _vm->_globalZoombinis[drawOrder[i]]->_screenPos.y)
				SWAP(drawOrder[i], drawOrder[j]);
		}
	}

	// Draw each zoombini in Y-sorted order
	for (uint i = 0; i < drawOrder.size(); i++)
		_vm->_gfx->drawZoombiniRunner(screen, _vm->_globalZoombinis[drawOrder[i]]);
}

void TransitionMapTrans::onActorsRendered() {
	for (uint i = 0; i < _vm->_globalZoombinis.size(); i++) {
		ZoombiniRunner *zoombini = _vm->_globalZoombinis[i];
		if (zoombini->_movementPath && !zoombini->_hidden)
			zoombini->advanceAnimationAfterDraw();
	}
}

EventHandleResult TransitionMapTrans::onKeyDown(const Common::KeyState &key, bool repeat) {
	(void)repeat;
	if (key.keycode != Common::KEYCODE_SPACE)
		return EventHandleResult::kPassthrough;
	finishTransition();
	return EventHandleResult::kConsumed;
}
EventHandleResult TransitionMapTrans::onLButtonUp(const Common::Point &pos) {
	(void)pos;
	finishTransition();
	return EventHandleResult::kConsumed;
}

PageId TransitionMapTrans::getDestPage(PageId src, RouteBranch routeBranch, int rescuedBoolies) {
	switch (src) {
	case kPageZombiniville:
		return kPageCrazyTurtle;
	case kPageCrazyTurtle:
		return kPageWaterslide;
	case kPageWaterslide:
		return kPageAquacube;
	case kPageAquacube:
		return kPageRescue1;
	case kPageRescue1: {
		if (routeBranch == RouteBranch::kLeft01)
			return kPageMagicWall;
		else if (routeBranch == RouteBranch::kRight02)
			return kPageMysticMarsh;
		return kPageNone;
	}
	case kPageMysticMarsh:
		return kPageWallOfFleens;
	case kPageMagicWall:
		return kPageChezNorf;
	case kPageWallOfFleens:
	case kPageChezNorf:
		return kPageRescue2;
	case kPageRescue2:
		return kPageSnowboard;
	case kPageSnowboard:
		return kPageBoolies;
	case kPageBoolies:
		return rescuedBoolies < 400 ? kPageBooliewood : kPageFinal;
	default:
		return kPageNone;
	}
}

} // End of namespace Zoombini2
