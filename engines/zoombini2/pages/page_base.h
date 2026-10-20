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
class Zoombini2Engine;
class ManagedSurface32;

/** One background and its page-managed general-object animation runners. */
class PageLayer : public Common::NonCopyable {
public:
	/** Initialize a layer with its horizontal scroll direction and optional page background.
	 * Layers without a background use @ref Gfx::kScreenSize.
	 * @param backgroundPath Scrolling or fixed background bitmap for this layer, or empty for a backgroundless layer.
	 */
	PageLayer(Zoombini2Engine *vm, byte scrollDirection, const Common::Path &backgroundPath = Common::Path());
	/** Release every animation runner registered with this layer. */
	~PageLayer();

	/** Replace the graphics background surface.
	 * @remark Also refreshs this layer's dimensions.
	 * @param path Bitmap resource used as this layer's background.
	 */
	bool loadBackground(const Common::Path &path);
	/** Return whether this layer has no background bitmap. */
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
	void drawBackgroundRegion(ManagedSurface32 *screen, int srcX, int srcY) const;
	/**
	 * Draw the layer background and update every enabled animation runner.
	 * @param forceBackgroundRedraw Restore even a cached fixed background before runners when recomposing a complete frame.
	 */
	void drawAndUpdate(ManagedSurface32 *screen, bool forceBackgroundRedraw = false);

private:
	/** Convert a screen pointer X coordinate to the layer's background coordinate. */
	int getWorldX(int screenX) const;
	/** Apply the original single-step horizontal wrapping rules. */
	void wrapScrollX();

	/** Borrowed engine interface used by resources, timing, and drawing. */
	Zoombini2Engine *_vm;
	/** Signed horizontal position applied to this layer. */
	int16 _scrollX = 0;
	/** Background bitmap dimensions, or the fixed screen size when no bitmap is loaded. */
	Size32 _backgroundSize = Gfx::kScreenSize;
	/** Multiplier applied by @ref PageLayer::scrollBy. */
	int16 _scrollByMultiplier;
	/** Animation runners released with this layer. */
	Common::Array<AnimationRunner *> _animationRunners;
	/** Whether this layer has no background bitmap. */
	bool _backgroundless = true;
	/** Whether a non-scrolling background has already been materialized. */
	bool _backgroundDrawn = false;
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
	/** Create and append one page layer.
	 * @param backgroundPath Optional bit-block background resource for the new layer.
	 */
	PageLayer *addLayer(byte scrollDirection, const Common::Path &backgroundPath = Common::Path());
	/** Return a page layer, or nullptr when @p layerIndex is outside the collection. */
	PageLayer *getLayer(int layerIndex) const;
	/** Return the number of page layers. */
	uint getLayerCount() const { return _layers.size(); }
	/** Replace one layer's bit-block background.
	 * @param path Bitmap resource used as the selected layer's background.
	 */
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
	/** Borrowed engine interface passed to page-layer resources. */
	Zoombini2Engine *_vm;
	/** Layers released with the current page. */
	Common::Array<PageLayer *> _layers;
	/** Whether a pressed pointer has already been dispatched. */
	bool _pointerPressLatched = false;
	/** Whether @ref PageLayerStack::scrollBy is currently suppressed. */
	bool _scrollLocked = false;
};

/** Page lifecycle categories used by engine and sidebar policy. */
enum class PageCategory {
	/** Player-controlled map or menu screen. */
	kInteractive = 1,
	/** Timed travel, title, video, or credits page. */
	kTransition = 2,
	/** Modal overlay retaining its underlying page. */
	kDialog = 3,
	/** Level-dependent rescue-mission puzzle. */
	kPuzzle = 4,
	/** Party assembly, storage, or arrival hub. */
	kShelter = 5,
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
	/** Release an input handler through its common dispatch interface. */
	virtual ~PageEventHandler() {}
	/** Dispatch one backend event to its typed callback. */
	EventHandleResult handleEvent(const Common::Event &event);
	/** Handle a left-button press. */
	virtual EventHandleResult onLButtonDown(const Common::Point &pos) {
		(void)pos;
		return EventHandleResult::kPassthrough;
	}
	/** Handle a right-button press. */
	virtual EventHandleResult onRButtonDown(const Common::Point &pos) {
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
 * Defines the input, update, and drawing hooks for a page.
 * The engine dispatches input through @ref PageEventHandler before the frame pass.
 * Shared @ref DialogBase overlays use a separate modal input and rendering route.
 *
 * @par Frame entry points and hook order
 * @verbatim
 * onFrame(screen, true)  -> onUpdate() -> renderFrame(screen, true)
 * onFrame(screen, false) ---------------> renderFrame(screen, false)
 * render(screen)         ---------------> renderFrame(screen, false)
 *
 * renderFrame(screen, advanceState):
 *   clear target if needsScreenClear()
 *     -> onRenderBackground(screen)
 *     -> onRenderContent(screen)
 *     -> onRenderActors(screen)
 *     -> onActorsRendered()       [only if advanceState]
 *     -> onRenderForeground(screen)
 *     -> onPostRender()           [only if advanceState]
 * @endverbatim
 * Later drawing covers earlier drawing. The engine composes the sidebar, shared dialogs,
 * and hold overlay after this page pass, then presents the screen.
 */
class PageBase : public PageEventHandler, public Common::NonCopyable {
public:
	/** Bind this page to @p vm and record its @p pageCategory. */
	PageBase(Zoombini2Engine *vm, PageCategory pageCategory);
	/** Stop and unload this page's music; concrete destructors release their own additional resources. */
	virtual ~PageBase();

	/**
	 * Dispatch the concrete event handler, then update the page-layer pointer latch for left-button events even if the handler consumed them.
	 * Dialogs skip layer dispatch because those layers belong to the page underneath the modal.
	 * Implement input behavior in the inherited mouse/key callbacks rather than replacing this dispatcher.
	 *
	 * @param[in] event Backend event with pointer coordinates already expressed in game pixels.
	 * @return Whether the page callback or a page-layer runner consumed the event.
	 */
	EventHandleResult handleEvent(const Common::Event &event);
	/**
	 * Run the optional simulation step, then the ordered page render pass shown in the class documentation.
	 * The engine passes false when a page change is already pending before this pass.
	 * A transition requested by a hook does not interrupt this pass or change its @p advanceState value.
	 *
	 * @param[in,out] screen Non-null destination surface in page pixel coordinates.
	 * @param[in] advanceState Run @ref onUpdate, @ref onActorsRendered, and @ref onPostRender when true.
	 *                         Drawing hooks run regardless of this flag.
	 */
	void onFrame(ManagedSurface32 *screen, bool advanceState);
	/**
	 * Recompose the page through the same drawing hooks, skipping all three update/completion hooks.
	 * Used by the modal rendering route. Animation helpers called by drawing hooks retain their own update behavior.
	 *
	 * @param[in,out] screen Non-null destination surface, which may already contain the underlying page.
	 */
	void render(ManagedSurface32 *screen);
	/**
	 * Draw a held actor above shared controls, separately from the main page render pass.
	 * The engine calls this after page, sidebar, and shared-dialog composition when the hold route permits it.
	 *
	 * @param[in,out] screen Non-null destination surface containing the composed page and controls.
	 * @param[in] advanceState Whether the held actor may advance after its current pose is drawn.
	 */
	virtual void renderHoldOverlay(ManagedSurface32 *screen, bool advanceState) {
		(void)screen;
		(void)advanceState;
	}

	/**
	 * Initialize page state, resources, and deadlines before its first frame.
	 * The dispatcher calls this after construction; derived pages must call any required family initialization.
	 * @ref DialogBase uses opener-driven configuration instead of this dispatcher step.
	 */
	virtual void init() = 0;
	/**
	 * Request a full target clear before @ref onRenderBackground in both frame and redraw entry paths.
	 * Leave false when drawing restores its own background or an overlay must retain underlying pixels.
	 */
	virtual bool needsScreenClear() const { return false; }
	/** Return whether a panel belonging to this page exclusively handles its input. */
	virtual bool hasActiveDialog() const { return false; }

	/** Return the numeric dispatcher identifier assigned by the concrete page. */
	PageId getPageId() const { return _pageId; }
	/** Return the lifecycle category recorded when this page was constructed. */
	PageCategory getCategory() const { return _pageCategory; }
	/** Return whether the shared sidebar is visible over this page. */
	virtual bool hasSidebar() const { return false; }
	/** Return whether the sidebar includes a Go button for this page. */
	virtual bool hasGoButton() const { return hasSidebar(); }
	/** Return whether the visible Go button currently accepts input. */
	virtual bool canUseGoButton() const { return hasGoButton(); }
	/** Apply page-local state that must be settled before Map saves or leaves the page. */
	virtual void onMapButtonPressed() {}
	/** Return whether the page's selected travelers permit a debugger transfer. */
	virtual bool debugCanDepart() const { return true; }
	/** Handle page-local Go speech; return true to continue the shared transition. */
	virtual bool onGoButtonPressed() { return true; }
	/** Return whether page-local state prevents the sidebar from accepting pointer input. */
	virtual bool blocksSidebarInteraction() const { return hasActiveDialog(); }
	/** Return whether this page implements a shelter flow. */
	virtual bool isShelter() const { return false; }
	/** Apply any page-local state required by the global debug-completion hotkey. */
	virtual void applyDebugPuzzleCompletion() {}
	/**
	 * Load or replace the 1-bit area mask used by Zoombini drop handling.
	 * @param path Resource file whose set pixels mark the permitted drop area.
	 */
	bool loadAreaMask(const Common::Path &path);
	/** Release the active area mask retained by Gfx. */
	void clearAreaMask();
	/** Return whether this page has an area mask loaded. */
	bool hasAreaMask() const;
	/** Return the drop-acceptance area mask, or nullptr when the page has none. */
	const AreaMask *getAreaMask() const;
	/**
	 * Return the squared pixel distance between two screen-space points.
	 * Omit the square root for callers that use squared thresholds or distance-based weights.
	 *
	 * @param[in] first First point in screen pixels.
	 * @param[in] second Second point in the same coordinate space.
	 * @return Squared Euclidean distance in pixels squared.
	 */
	static int32 squaredDistance(const Common::Point32 &first, const Common::Point32 &second);

protected:
	/**
	 * Start or replace this page's looping music, normally from @ref init.
	 * This is a resource helper, not a frame hook; PageBase unloads the previous track on replacement
	 * and releases the retained track at page destruction.
	 * @param[in] path Audio resource played at the configured music volume.
	 */
	void startPageMusic(const Common::Path &path);
	/**
	 * Advance simulation, timers, and page state before any pixels are cleared or drawn.
	 * Called only by @ref onFrame with advanceState=true. A queued transition still leaves this pass's drawing to complete.
	 */
	virtual void onUpdate() {}
	/**
	 * Restore the backdrop before content and actors; the default does nothing.
	 * @param[in,out] screen Destination after the optional full clear, shared by all drawing hooks in this pass.
	 */
	virtual void onRenderBackground(ManagedSurface32 *screen) { (void)screen; }
	/**
	 * Draw board elements, scenery, and other content above the backdrop but behind actors.
	 * This is the required drawing hook; simple pages may draw the whole screen here.
	 * @param[in,out] screen Destination containing the background drawn earlier in this pass.
	 */
	virtual void onRenderContent(ManagedSurface32 *screen) = 0;
	/**
	 * Draw the current actor poses over content, before actor completion and foreground work.
	 * The page or family base owns actor selection and ordering; PageBase draws no actors automatically.
	 * @param[in,out] screen Destination containing background and content.
	 */
	virtual void onRenderActors(ManagedSurface32 *screen) { (void)screen; }
	/**
	 * Advance or finalize actors after their current poses have been drawn, before foreground composition.
	 * Use this for after-draw animation steps, completion callbacks, and resulting page-state changes.
	 * Called only when the pass's advanceState is true; @ref render skips this hook.
	 */
	virtual void onActorsRendered() {}
	/**
	 * Draw occluders and other page elements that must cover actors, after actor completion work.
	 * @param[in,out] screen Destination containing the actor poses drawn in this pass.
	 */
	virtual void onRenderForeground(ManagedSurface32 *screen) { (void)screen; }
	/**
	 * Finish page-local work after foreground composition, only when the pass's advanceState is true.
	 * Shared controls, hold overlays, and screen presentation still follow in the engine.
	 * This is the last page hook, not a notification that the screen has been presented; @ref render skips it.
	 */
	virtual void onPostRender() {}

	/** Borrowed engine interface, which outlives this page and the resources released by its destructor. */
	Zoombini2Engine *_vm;
	/** Category assigned to this page for lifecycle and UI policy. */
	const PageCategory _pageCategory;
	/** Numeric dispatcher identifier for this page. */
	PageId _pageId = PageId::kPageNone_M1;

private:
	/** Stop and unload music started by this page. */
	void releasePageMusic();
	/**
	 * Run the clear policy and drawing hooks in fixed order, inserting the two gated completion hooks.
	 * @param[in,out] screen Non-null destination shared by every drawing hook.
	 * @param[in] advanceState Whether to run @ref onActorsRendered and @ref onPostRender; @ref onUpdate is handled by the caller.
	 */
	void renderFrame(ManagedSurface32 *screen, bool advanceState);
	/** Music buffer released when this page is destroyed. */
	int _pageMusicId = -1;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PAGE_BASE_H
