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

#ifndef ZOOMBINI2_PAGES_PAGE_BASE_H
#define ZOOMBINI2_PAGES_PAGE_BASE_H

#include "common/array.h"
#include "common/events.h"
#include "common/noncopyable.h"
#include "common/path.h"
#include "common/rect.h"
#include "common/scummsys.h"

#include "zoombini2/scripts.h"

namespace Zoombini2 {

class AreaMask;
class BitBlock;
class Zoombini2Engine;
class ManagedSurface32;

/** One background and its page-managed general-object animation runners. */
class PageLayer : public Common::NonCopyable {
public:
	/** Initialize a layer with its horizontal scroll direction and optional background. */
	PageLayer(Zoombini2Engine *vm, byte scrollDirection, const Common::Path &backgroundPath = Common::Path());
	/** Release the background and every animation runner registered with this layer. */
	~PageLayer();

	/** Replace the background and refresh its dimensions. */
	bool loadBackground(const Common::Path &path);
	/** Return the background retained by this layer, or nullptr for a backgroundless layer. */
	BitBlock *getBackground() const { return _background; }
	/** Return the background dimensions copied from the first layer when this layer has no bitmap. */
	const Size32 &getBackgroundSize() const { return _backgroundSize; }
	/** Return whether this layer was created without a background bitmap. */
	bool isBackgroundless() const { return _backgroundless; }

	/** Create and append one general-object animation runner. */
	AnimationRunner *createAnimationRunner(const Common::Point32 &position, AnimationRunnerMode mode);
	/** Return a registered runner, or nullptr when @p runnerIndex is outside this layer. */
	AnimationRunner *getAnimationRunner(int runnerIndex) const;
	/** Return the number of runners registered with this layer. */
	uint getAnimationRunnerCount() const { return _animationRunners.size(); }

	/** Invoke the indexed runner's interaction callback. */
	void invokeInteractionCallback(int runnerIndex);
	/** Restart the indexed runner at the current game tick. */
	void resetRunner(int runnerIndex);
	/** Replace the indexed runner's playback mode. */
	void setRunnerMode(int runnerIndex, AnimationRunnerMode mode);
	/** Replace the indexed runner's page position. */
	void setRunnerPosition(int runnerIndex, const Common::Point32 &position);
	/** Stop and hide the indexed runner. */
	void stopRunner(int runnerIndex);
	/** Return the indexed runner's X coordinate, or zero for an invalid index. */
	int getRunnerX(int runnerIndex) const;
	/** Return the indexed runner's Y coordinate, or zero for an invalid index. */
	int getRunnerY(int runnerIndex) const;
	/** Replace the indexed runner's completion callback. */
	void setRunnerCompletionCallback(int runnerIndex, AnimationRunner::Callback callback, void *context = nullptr);
	/** Return the indexed runner's hit rectangle, or an empty rectangle for an invalid index. */
	Common::Rect32 getRunnerRect(int runnerIndex) const;
	/** Return the indexed runner's selected animation, or nullptr for an invalid index. */
	const Animation *getRunnerAnimation(int runnerIndex) const;
	/** Replace the indexed runner's selected animation. */
	void setRunnerAnimation(int runnerIndex, const Animation *animation);
	/** Return one image frame from the indexed runner's selected animation. */
	const RleBlock *getRunnerFrame(int runnerIndex, int frameIndex) const;
	/** Return whether the indexed runner is active. */
	bool isRunnerActive(int runnerIndex) const;
	/** Return whether any runner registered with this layer is active. */
	bool hasActiveRunner() const;
	/** Start the indexed runner at @p position and the current game tick. */
	void startRunnerAt(int runnerIndex, const Common::Point32 &position);

	/** Set and wrap the absolute horizontal scroll position. */
	void setScrollX(int16 scrollX);
	/** Advance horizontal scrolling according to this layer's direction and wrap it. */
	void scrollBy(int16 delta);
	/** Return the current horizontal scroll position. */
	int16 getScrollX() const { return _scrollX; }

	/** Restore saved background pixels for inactive runners in registration order. */
	void restoreRunnerBackgrounds(ManagedSurface32 *screen) const;
	/** Return whether an input-enabled runner has a hit rectangle under @p point. */
	bool hasInteractiveRunnerAt(const Common::Point32 &point) const;
	/** Run callbacks and start every inactive runner whose rectangle contains @p point. */
	bool activateRunnersAt(const Common::Point32 &point);
	/** Draw an 800x600 subregion of the layer background. */
	void drawBackgroundRegion(ManagedSurface32 *screen, int sourceX, int sourceY) const;
	/**
	 * Draw the layer background and update every enabled animation runner.
	 * @param forceBackgroundRedraw Restore even a cached fixed background before runners when recomposing a complete frame.
	 */
	void drawAndUpdate(ManagedSurface32 *screen, bool forceBackgroundRedraw = false);

private:
	friend class PageLayerStack;

	/** Replace dimensions copied from the stack's first layer. */
	void setBackgroundSize(const Size32 &size);
	/** Convert a screen pointer X coordinate to the layer's background coordinate. */
	int getWorldX(int screenX) const;
	/** Apply the original single-step horizontal wrapping rules. */
	void wrapScrollX();

	/** Borrowed engine interface used by resources, timing, and drawing. */
	Zoombini2Engine *_vm;
	/** Signed horizontal position applied to this layer. */
	int16 _scrollX = 0;
	/** Background dimensions retained locally or copied from the first stack layer. */
	Size32 _backgroundSize = Size32();
	/** Multiplier applied by @ref scrollBy. */
	byte _scrollDirection;
	/** Animation runners released with this layer. */
	Common::Array<AnimationRunner *> _animationRunners;
	/** Whether this layer was created without a background bitmap. */
	bool _backgroundless = true;
	/** Whether a non-scrolling background has already been materialized. */
	bool _backgroundDrawn = false;
	/** Optional bitmap released with this layer. */
	BitBlock *_background = nullptr;
};

/** Page-level collection that coordinates backgrounds, pointer input, scrolling, and animation runners. */
class PageLayerStack : public Common::NonCopyable {
public:
	/** Initialize an empty page-level layer collection. */
	explicit PageLayerStack(Zoombini2Engine *vm);
	/** Release every page layer and the optional area mask. */
	~PageLayerStack();

	/** Remove every page layer and the optional area mask, and reset transient stack state. */
	void clear();
	/** Create and append one page layer. */
	PageLayer *addLayer(byte scrollDirection, const Common::Path &backgroundPath = Common::Path());
	/** Return a page layer, or nullptr when @p layerIndex is outside the collection. */
	PageLayer *getLayer(int layerIndex) const;
	/** Return the number of page layers. */
	uint getLayerCount() const { return _layers.size(); }
	/** Replace one layer's background and propagate first-layer dimensions to backgroundless followers. */
	bool loadLayerBackground(int layerIndex, const Common::Path &path);

	/** Dispatch the first pressed state to all layers and reset the latch on release. */
	bool handlePointerButton(const Common::Point32 &point, bool pressed);
	/** Return whether any layer has an input-enabled animation runner under @p point. */
	bool hasInteractiveRunnerAt(const Common::Point32 &point) const;

	/** Lock or unlock group scrolling. */
	void setScrollLocked(bool locked) { _scrollLocked = locked; }
	/** Return whether group scrolling is locked. */
	bool isScrollLocked() const { return _scrollLocked; }
	/** Scroll every layer unless group scrolling is locked. */
	void scrollBy(int16 delta);
	/** Set the absolute horizontal scroll position on every layer. */
	void setScrollX(int16 scrollX);

	/**
	 * Draw and update the first layer, if present.
	 * @param forceBackgroundRedraw Forward the complete-frame restoration policy to @ref PageLayer::drawAndUpdate.
	 */
	void drawFirstLayer(ManagedSurface32 *screen, bool forceBackgroundRedraw = false);

private:
	/** Copy the first layer dimensions into every backgroundless follower. */
	void propagateFirstLayerDimensions();

	/** Borrowed engine interface passed to page-layer resources. */
	Zoombini2Engine *_vm;
	/** Layers released with the current page. */
	Common::Array<PageLayer *> _layers;
	/** Whether a pressed pointer has already been dispatched. */
	bool _pointerPressLatched = false;
	/** Whether @ref scrollBy is currently suppressed. */
	bool _scrollLocked = false;
};

/** Page lifecycle categories used by engine and sidebar policy. */
enum class PageCategory {
	/** Player-controlled map, shelter, menu, or puzzle. */
	kInteractive = 1,
	/** Timed travel, title, video, or credits page. */
	kTransition = 2,
	/** Modal overlay retaining its underlying page. */
	kDialog = 3,
};

/** Result returned by an input handler after it processes one event. */
enum class EventHandleResult {
	/** This handler did not claim the event. */
	kPassthrough,
	/** This handler claimed the event. */
	kConsumed,
};

/** Common ordered input dispatch for pages and their shared controls. */
class PageEventHandler {
public:
	virtual ~PageEventHandler() {}
	/** Dispatch one backend event to its typed callback. */
	EventHandleResult handleEvent(const Common::Event &event);
	/** Handle a left-button press. */
	virtual EventHandleResult onLButtonDown(const Common::Point &pos) {
		(void)pos;
		return EventHandleResult::kPassthrough;
	}
	/** Handle a left-button release. */
	virtual EventHandleResult onLButtonUp(const Common::Point &pos) {
		(void)pos;
		return EventHandleResult::kPassthrough;
	}
	/** Handle a pointer movement in game coordinates. */
	virtual EventHandleResult onMouseMove(const Common::Point &pos) {
		(void)pos;
		return EventHandleResult::kPassthrough;
	}
	/** Handle a key press with its backend repeat flag. */
	virtual EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) {
		(void)key;
		(void)repeat;
		return EventHandleResult::kPassthrough;
	}
	/** Handle a key release. */
	virtual EventHandleResult onKeyUp(const Common::KeyState &key) {
		(void)key;
		return EventHandleResult::kPassthrough;
	}
};

/**
 * Coordinates one screen-facing update, drawing, and input lifecycle.
 * The engine dispatches one regular page at a time. Modal dialogs derive from
 * this base but remain separately retained by the sidebar while preserving the
 * underlying dispatched page.
 */
class PageBase : public PageEventHandler {
public:
	/** Bind this page to @p vm and record its @p pageCategory. */
	PageBase(Zoombini2Engine *vm, PageCategory pageCategory);
	/** Release resources retained by the concrete page. */
	virtual ~PageBase();

	/** Dispatch one backend event through the concrete page and its page-layer stack. */
	EventHandleResult handleEvent(const Common::Event &event);
	/** Advance page state when permitted, then execute the complete render pass. */
	void onFrame(ManagedSurface32 *screen, bool advanceState);
	/** Recompose the selected visuals without advancing simulation or completion callbacks. */
	void render(ManagedSurface32 *screen);

	/** Initialize page-local state and resources. */
	virtual void init() = 0;
	/** Return whether the engine must clear the screen before @ref PageBase::render. */
	virtual bool needsScreenClear() const { return false; }
	/** Return whether a panel belonging to this page exclusively handles its input. */
	virtual bool hasActiveDialog() const { return false; }

	/** Return the numeric dispatcher identifier assigned by the concrete page. */
	int getPageId() const { return _pageId; }
	/** Return the lifecycle category recorded when this page was constructed. */
	PageCategory getCategory() const { return _pageCategory; }
	/** Return whether the shared sidebar is visible over this page. */
	virtual bool hasSidebar() const { return false; }
	/** Return whether the sidebar includes a Go button for this page. */
	virtual bool hasGoButton() const { return hasSidebar(); }
	/** Return whether the visible Go button currently accepts input. */
	virtual bool canUseGoButton() const { return hasGoButton(); }
	/** Return whether page-local state prevents the sidebar from accepting pointer input. */
	virtual bool blocksSidebarInteraction() const { return hasActiveDialog(); }
	/** Return whether this page implements a shelter flow. */
	virtual bool isShelter() const { return false; }
	/** Apply any page-local state required by the global debug-completion hotkey. */
	virtual void applyDebugPuzzleCompletion() {}
	/** Load or replace the 1-bit area mask used by Zoombini drop handling. */
	bool loadAreaMask(const Common::Path &path);
	/** Release the area mask retained by this page. */
	void clearAreaMask();
	/** Return whether this page has an area mask loaded. */
	bool hasAreaMask() const { return _areaMask != nullptr; }
	/** Return the drop-acceptance area mask, or nullptr when the page has none. */
	const AreaMask *getAreaMask() const { return _areaMask; }

protected:
	/** Advance this page's simulation before rendering. */
	virtual void onUpdate() {}
	/** Restore or draw this page's background pixels. */
	virtual void onRenderBackground(ManagedSurface32 *screen) { (void)screen; }
	/** Draw this page's content elements behind its actors. */
	virtual void onRenderContent(ManagedSurface32 *screen) = 0;
	/** Draw actors at the page's actor boundary. */
	virtual void onRenderActors(ManagedSurface32 *screen) { (void)screen; }
	/** Advance actor completion work after drawing, before foreground composition, on an active frame only. */
	virtual void onActorsRendered() {}
	/** Draw this page's elements that cover its actors. */
	virtual void onRenderForeground(ManagedSurface32 *screen) { (void)screen; }
	/** Complete work after composition on an active frame only. */
	virtual void onPostRender() {}

	/** Reference to the engine interface. */
	Zoombini2Engine *_vm;
	/** Category assigned to this page for lifecycle and UI policy. */
	PageCategory _pageCategory;
	/** Numeric dispatcher identifier for this page. */
	int _pageId = -1;
	/** Optional page-area mask released with this page. */
	AreaMask *_areaMask = nullptr;

private:
	/** Run visual hooks and, when requested, the two completion boundaries. */
	void renderFrame(ManagedSurface32 *screen, bool advanceState);
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PAGE_BASE_H
