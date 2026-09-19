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

#include "zoombini2/pages/page_base.h"
#include "zoombini2/graphics.h"
#include "zoombini2/zoombini2.h"

#include "common/debug.h"

namespace Zoombini2 {

PageLayer::PageLayer(Zoombini2Engine *vm, byte scrollDirection, const Common::Path &backgroundPath)
	: _vm(vm), _scrollDirection(scrollDirection) {
	if (!backgroundPath.empty())
		loadBackground(backgroundPath);
}

PageLayer::~PageLayer() {
	for (uint index = 0; index < _animationRunners.size(); index++)
		delete _animationRunners[index];
	delete _background;
}

bool PageLayer::loadBackground(const Common::Path &path) {
	BitBlock *background = new BitBlock(_vm);
	if (!background->load(path)) {
		delete background;
		return false;
	}

	delete _background;
	_background = background;
	_backgroundSize = background->getSize();
	_backgroundless = false;
	_backgroundDrawn = false;
	wrapScrollX();
	return true;
}

AnimationRunner *PageLayer::createAnimationRunner(const Common::Point32 &position, AnimationRunnerMode mode) {
	AnimationRunner *runner = new AnimationRunner(_vm, position, mode);
	runner->setLayerIndex(_animationRunners.size());
	_animationRunners.push_back(runner);
	return runner;
}

AnimationRunner *PageLayer::getAnimationRunner(int runnerIndex) const {
	if (runnerIndex < 0 || static_cast<int>(_animationRunners.size()) <= runnerIndex)
		return nullptr;
	return _animationRunners[runnerIndex];
}

void PageLayer::invokeInteractionCallback(int runnerIndex) {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	if (runner)
		runner->invokeInteractionCallback();
}

void PageLayer::resetRunner(int runnerIndex) {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	if (runner)
		runner->reset(_vm->getGameTickCount());
}

void PageLayer::setRunnerMode(int runnerIndex, AnimationRunnerMode mode) {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	if (runner)
		runner->setMode(mode);
}

void PageLayer::setRunnerPosition(int runnerIndex, const Common::Point32 &position) {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	if (runner)
		runner->setPosition(position);
}

void PageLayer::stopRunner(int runnerIndex) {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	if (runner)
		runner->stop();
}

int PageLayer::getRunnerX(int runnerIndex) const {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	return runner ? runner->getPosition().x : 0;
}

int PageLayer::getRunnerY(int runnerIndex) const {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	return runner ? runner->getPosition().y : 0;
}

void PageLayer::setRunnerCompletionCallback(int runnerIndex, AnimationRunner::Callback callback, void *context) {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	if (runner)
		runner->setCompletionCallback(callback, context);
}

Common::Rect32 PageLayer::getRunnerRect(int runnerIndex) const {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	return runner ? runner->getHitRect() : Common::Rect32();
}

const Animation *PageLayer::getRunnerAnimation(int runnerIndex) const {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	return runner ? runner->getAnimation() : nullptr;
}

void PageLayer::setRunnerAnimation(int runnerIndex, const Animation *animation) {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	if (runner)
		runner->setAnimation(animation);
}

const RleBlock *PageLayer::getRunnerFrame(int runnerIndex, int frameIndex) const {
	const Animation *animation = getRunnerAnimation(runnerIndex);
	return animation ? animation->getFrame(frameIndex) : nullptr;
}

bool PageLayer::isRunnerActive(int runnerIndex) const {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	return runner && runner->isActive();
}

bool PageLayer::hasActiveRunner() const {
	for (uint index = 0; index < _animationRunners.size(); index++) {
		if (_animationRunners[index]->isActive())
			return true;
	}
	return false;
}

void PageLayer::startRunnerAt(int runnerIndex, const Common::Point32 &position) {
	AnimationRunner *runner = getAnimationRunner(runnerIndex);
	if (runner)
		runner->startAt(position, _vm->getGameTickCount());
}

void PageLayer::setScrollX(int16 scrollX) {
	_scrollX = scrollX;
	wrapScrollX();
}

void PageLayer::scrollBy(int16 delta) {
	_scrollX = static_cast<int16>(_scrollX + delta * _scrollDirection);
	wrapScrollX();
}

void PageLayer::restoreRunnerBackgrounds(ManagedSurface32 *screen) const {
	for (uint index = 0; index < _animationRunners.size(); index++)
		_animationRunners[index]->restoreBackgroundIfInactive(screen);
}

bool PageLayer::hasInteractiveRunnerAt(const Common::Point32 &point) const {
	const Common::Point32 worldPoint(getWorldX(point.x), point.y);
	for (uint index = 0; index < _animationRunners.size(); index++) {
		const AnimationRunner *runner = _animationRunners[index];
		if (runner->isInputEnabled() && runner->isHitTestEnabled() && runner->containsHitPoint(worldPoint))
			return true;
	}
	return false;
}

bool PageLayer::activateRunnersAt(const Common::Point32 &point) {
	const Common::Point32 worldPoint(getWorldX(point.x), point.y);
	bool hit = false;
	for (uint index = 0; index < _animationRunners.size(); index++) {
		AnimationRunner *runner = _animationRunners[index];
		if (!runner->containsHitPoint(worldPoint))
			continue;
		hit = true;
		if (!runner->isActive())
			runner->invokeInteractionCallback();
		if (index < _animationRunners.size())
			runner = _animationRunners[index];
		if (!runner->isActive() && runner->getMode() != AnimationRunnerMode::kDisabled04) {
			runner->start(_vm->getGameTickCount());
			runner->captureBackground(_vm->getCurrentScreen(), _scrollX, _backgroundSize.width);
		}
	}
	return hit;
}

void PageLayer::drawBackgroundRegion(ManagedSurface32 *screen, int sourceX, int sourceY) const {
	if (!_background || !screen)
		return;
	const Common::Rect sourceRect(sourceX, sourceY, sourceX + ManagedSurface32::kScreenSize.width, sourceY + ManagedSurface32::kScreenSize.height);
	_vm->_gfx->drawBitBlockSubRect(screen, _background, Common::Point32(0, 0), sourceRect);
}

void PageLayer::drawAndUpdate(ManagedSurface32 *screen, bool forceBackgroundRedraw) {
	if (!screen)
		return;

	if (_background) {
		if (_backgroundSize.width <= ManagedSurface32::kScreenSize.width) {
			if (forceBackgroundRedraw || !_backgroundDrawn) {
				drawBackgroundRegion(screen, 0, 0);
				_backgroundDrawn = true;
			}
		} else if (!_backgroundless) {
			if (0 < _scrollX && _scrollX < _backgroundSize.width - ManagedSurface32::kScreenSize.width) {
				drawBackgroundRegion(screen, _scrollX, 0);
			} else if (0 < _scrollX) {
				const int tailWidth = _backgroundSize.width - _scrollX;
				_vm->_gfx->drawBitBlockSubRect(screen, _background, Common::Point32(0, 0), Common::Rect(_scrollX, 0, _backgroundSize.width, ManagedSurface32::kScreenSize.height));
				_vm->_gfx->drawBitBlockSubRect(screen, _background, Common::Point32(tailWidth, 0), Common::Rect(0, 0, ManagedSurface32::kScreenSize.width - tailWidth, ManagedSurface32::kScreenSize.height));
			} else {
				const int tailStart = _backgroundSize.width + _scrollX;
				const int tailWidth = -_scrollX;
				_vm->_gfx->drawBitBlockSubRect(screen, _background, Common::Point32(0, 0), Common::Rect(tailStart, 0, _backgroundSize.width, ManagedSurface32::kScreenSize.height));
				_vm->_gfx->drawBitBlockSubRect(screen, _background, Common::Point32(tailWidth, 0), Common::Rect(0, 0, ManagedSurface32::kScreenSize.width - tailWidth, ManagedSurface32::kScreenSize.height));
			}
		}
	}

	const uint32 tickCount = _vm->getGameTickCount();
	for (uint index = 0; index < _animationRunners.size(); index++) {
		AnimationRunner *runner = _animationRunners[index];
		if (runner->getMode() != AnimationRunnerMode::kDisabled04)
			_vm->_gfx->drawAndUpdateAnimationRunner(screen, runner, tickCount, _scrollX, _backgroundSize.width);
	}
}

void PageLayer::setBackgroundSize(const Size32 &size) {
	if (!_backgroundless)
		return;
	_backgroundSize = size;
	wrapScrollX();
}

int PageLayer::getWorldX(int screenX) const {
	if (_backgroundSize.width == ManagedSurface32::kScreenSize.width)
		return screenX;
	const int backgroundOrigin = 0 < _scrollX ? _scrollX : _backgroundSize.width + _scrollX;
	return backgroundOrigin + screenX;
}

void PageLayer::wrapScrollX() {
	if (_backgroundSize.width <= 0)
		return;
	if (_scrollX < -ManagedSurface32::kScreenSize.width)
		_scrollX = static_cast<int16>(_scrollX + _backgroundSize.width);
	if (_backgroundSize.width - 1 < _scrollX)
		_scrollX = static_cast<int16>(_scrollX - _backgroundSize.width + 1);
}

PageLayerStack::PageLayerStack(Zoombini2Engine *vm) : _vm(vm) {
}

PageLayerStack::~PageLayerStack() {
	clear();
}

void PageLayerStack::clear() {
	for (uint index = 0; index < _layers.size(); index++)
		delete _layers[index];
	_layers.clear();
	_pointerPressLatched = false;
	_scrollLocked = false;
}

PageLayer *PageLayerStack::addLayer(byte scrollDirection, const Common::Path &backgroundPath) {
	PageLayer *layer = new PageLayer(_vm, scrollDirection, backgroundPath);
	_layers.push_back(layer);
	propagateFirstLayerDimensions();
	return layer;
}

PageLayer *PageLayerStack::getLayer(int layerIndex) const {
	if (layerIndex < 0 || static_cast<int>(_layers.size()) <= layerIndex)
		return nullptr;
	return _layers[layerIndex];
}

bool PageLayerStack::loadLayerBackground(int layerIndex, const Common::Path &path) {
	PageLayer *layer = getLayer(layerIndex);
	if (!layer || !layer->loadBackground(path))
		return false;
	if (layerIndex == 0)
		propagateFirstLayerDimensions();
	return true;
}

bool PageLayerStack::handlePointerButton(const Common::Point32 &point, bool pressed) {
	if (!pressed) {
		_pointerPressLatched = false;
		return false;
	}
	if (_pointerPressLatched)
		return false;

	bool hit = false;
	for (uint index = 0; index < _layers.size(); index++)
		hit = _layers[index]->activateRunnersAt(point) || hit;
	_pointerPressLatched = true;
	return hit;
}

bool PageLayerStack::hasInteractiveRunnerAt(const Common::Point32 &point) const {
	for (uint index = 0; index < _layers.size(); index++) {
		if (_layers[index]->hasInteractiveRunnerAt(point))
			return true;
	}
	return false;
}

void PageLayerStack::scrollBy(int16 delta) {
	if (_scrollLocked)
		return;
	for (uint index = 0; index < _layers.size(); index++)
		_layers[index]->scrollBy(delta);
}

void PageLayerStack::setScrollX(int16 scrollX) {
	for (uint index = 0; index < _layers.size(); index++)
		_layers[index]->setScrollX(scrollX);
}

void PageLayerStack::drawFirstLayer(ManagedSurface32 *screen, bool forceBackgroundRedraw) {
	PageLayer *layer = getLayer(0);
	if (layer)
		layer->drawAndUpdate(screen, forceBackgroundRedraw);
}

void PageLayerStack::propagateFirstLayerDimensions() {
	PageLayer *firstLayer = getLayer(0);
	if (!firstLayer)
		return;
	for (uint index = 1; index < _layers.size(); index++)
		_layers[index]->setBackgroundSize(firstLayer->getBackgroundSize());
}

PageBase::PageBase(Zoombini2Engine *vm, PageCategory pageCategory)
	: _vm(vm), _pageCategory(pageCategory) {
}

PageBase::~PageBase() {
	clearAreaMask();
}

bool PageBase::loadAreaMask(const Common::Path &path) {
	AreaMask *areaMask = new AreaMask(_vm);
	if (!areaMask->loadFromFile(path)) {
		warning("PageBase: Failed to load area mask '%s'", path.toString().c_str());
		delete areaMask;
		return false;
	}
	clearAreaMask();
	_areaMask = areaMask;
	return true;
}

void PageBase::clearAreaMask() {
	delete _areaMask;
	_areaMask = nullptr;
}

EventHandleResult PageEventHandler::handleEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		return onLButtonDown(event.mouse);
	case Common::EVENT_LBUTTONUP:
		return onLButtonUp(event.mouse);
	case Common::EVENT_MOUSEMOVE:
		return onMouseMove(event.mouse);
	case Common::EVENT_KEYDOWN:
		return onKeyDown(event.kbd, event.kbdRepeat);
	case Common::EVENT_KEYUP:
		return onKeyUp(event.kbd);
	default:
		return EventHandleResult::kPassthrough;
	}
}

EventHandleResult PageBase::handleEvent(const Common::Event &event) {
	const EventHandleResult pageResult = PageEventHandler::handleEvent(event);
	// Modal dialogs share the graphics page-layer collection with the page beneath them, so
	// only non-dialog pages dispatch pointer input to the layer runners.
	if (_pageCategory == PageCategory::kDialog)
		return pageResult;
	bool layerHandled = false;
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
		layerHandled = _vm->_gfx->getPageLayerStack()->handlePointerButton(Common::Point32(event.mouse), true);
		break;
	case Common::EVENT_LBUTTONUP:
		_vm->_gfx->getPageLayerStack()->handlePointerButton(Common::Point32(event.mouse), false);
		break;
	default:
		break;
	}
	return layerHandled ? EventHandleResult::kConsumed : pageResult;
}

void PageBase::onFrame(ManagedSurface32 *screen, bool advanceState) {
	if (advanceState)
		onUpdate();
	renderFrame(screen, advanceState);
}

void PageBase::render(ManagedSurface32 *screen) {
	renderFrame(screen, false);
}

void PageBase::renderFrame(ManagedSurface32 *screen, bool advanceState) {
	if (needsScreenClear())
		_vm->_gfx->fillRect(screen, Common::Rect32(screen->w, screen->h), 0);
	onRenderBackground(screen);
	onRenderContent(screen);
	onRenderActors(screen);
	if (advanceState)
		onActorsRendered();
	onRenderForeground(screen);
	if (advanceState)
		onPostRender();
}

} // End of namespace Zoombini2
