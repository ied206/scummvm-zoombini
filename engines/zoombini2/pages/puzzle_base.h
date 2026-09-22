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

#ifndef ZOOMBINI2_PAGES_PUZZLE_BASE_H
#define ZOOMBINI2_PAGES_PUZZLE_BASE_H

#include "common/array.h"
#include "common/path.h"

#include "zoombini2/pages/interactive_base.h"

namespace Zoombini2 {

struct StorageRecord;
class ZoombiniRunner;
class ZoombiniAnimation;

/** Snapshot of the active puzzle's attempt or resource budget. */
struct PuzzleChanceInfo {
	/** Categories of opportunity accounting used by puzzle diagnostics. */
	enum class Type {
		/** The puzzle has no reportable opportunity budget. */
		kNone = 0,
		/** The puzzle exposes a progress value without a finite allowance. */
		kAmorphous = 1,
		/** The puzzle accepts unlimited attempts. */
		kInfinite = 2,
		/** The puzzle consumes one opportunity for each submitted action. */
		kSubmit = 3,
		/** The puzzle consumes one opportunity for each mistake. */
		kMistake = 4,
	};
	/** Opportunity accounting model reported by the concrete puzzle. */
	Type type = Type::kNone;
	/** Maximum number of opportunities, or a negative value when no finite maximum applies. */
	int opportunities = -1;
	/** Number of opportunities already consumed, or a negative value when it is not tracked. */
	int used = -1;
	/** Singular label for one reported opportunity. */
	const char *unit = nullptr;

	/** Construct a report with no opportunity accounting. */
	PuzzleChanceInfo() = default;
	/** Construct a report with an optional finite maximum and consumed count. */
	PuzzleChanceInfo(Type kind, int maximum = -1, int consumed = -1, const char *unitName = nullptr)
		: type(kind), opportunities(maximum), used(consumed), unit(unitName) {}

	/** Return remaining finite opportunities, or -1 when the model has no finite count. */
	int chancesLeft() const { return 0 <= opportunities && 0 <= used ? MAX(0, opportunities - used) : -1; }
	/** Return the display label associated with @p type. */
	static const char *typeName(Type type);
};

/**
 * Common page base for the nine rescue-mission puzzles.
 *
 * Provides shared puzzle resources and party state.
 */
class PuzzleBase : public InteractiveBase {
public:
	/** Bind shared puzzle state to @p vm, capture its selected difficulty, and record @p pageId. */
	PuzzleBase(Zoombini2Engine *vm, PageId pageId);
	/** Release shared puzzle resources. */
	~PuzzleBase() override = default;
	/** Return whether the shared sidebar is visible. */
	bool hasSidebar() const override { return true; }

	/** Initialize the shared puzzle background, sprite grid, and roster. */
	void init() override;
	/** Accept the whole current party and use the regular practice or adventure departure. */
	void debugForceFinish();
	/** Describe generated rules without changing puzzle state or consuming random numbers. */
	virtual Common::String debugGetAnswer() const = 0;
	/** Describe the page's actual opportunity model. */
	virtual PuzzleChanceInfo debugGetChances() const { return PuzzleChanceInfo(PuzzleChanceInfo::Type::kInfinite); }
	/** Whether the current state permits editing a finite budget. */
	virtual bool debugCanSetChances() const { return false; }
	/** Set a validated remaining budget, including its visual and completion state. */
	virtual bool debugSetChances(int remaining) {
		(void)remaining;
		return false;
	}
	/** Additional resources not represented by the primary chance count. */
	virtual Common::String debugGetChanceDetails() const { return Common::String(); }
	/** Return the display name belonging to @p pageId. */
	static const char *getPuzzleName(PageId pageId);
	/** Return the resource-directory name belonging to @p pageId. */
	static const char *getPuzzleDir(PageId pageId);

private:
	/** Format the common primary background path from a page resource-directory name. */
	static constexpr const char *kPuzzleBackgroundFormat = "#bmp/%s";
	/** Shared small-Zoombini animation used to compose each puzzle roster. */
	static constexpr const char *kZoombiniAnimationPath = "bmp/zombis/littleZomb.anm";
	/** Crazy Turtle primary background resource. */
	static constexpr const char *kCrazyTurtleBackgroundPath = "crazy_turtle/background";
	/** Water Slide primary background resource. */
	static constexpr const char *kWaterslideBackgroundPath = "waterslide/waterslides";
	/** AquaCube primary background resource. */
	static constexpr const char *kAquacubeBackgroundPath = "aquacube/background";
	/** Mystic Marsh primary background resource. */
	static constexpr const char *kMysticMarshBackgroundPath = "mystic_marsh/background1";
	/** Magic Wall primary background resource. */
	static constexpr const char *kMagicWallBackgroundPath = "magic_wall/magic wall";
	/** Wall of Fleens primary background resource. */
	static constexpr const char *kWallOfFleensBackgroundPath = "wall_of_fleens/background";
	/** Chez Norf primary background resource. */
	static constexpr const char *kChezNorfBackgroundPath = "chez_norf/baquegund";
	/** Snowboard primary background resource. */
	static constexpr const char *kSnowboardBackgroundPath = "snowboard/snowboard-EASY";
	/** Boolies primary background resource. */
	static constexpr const char *kBooliesBackgroundPath = "Boolies/background";

protected:
	/** Banner shared by all puzzle answer reports. */
	Common::String debugAnswerHeader() const;
	/** Print a roster entry using stable one-based indices and named traits. */
	Common::String debugActorDescription(int index) const;
	/** Whether a forced departure has already been requested. */
	bool _debugFinishPending = false;
	/** Finish this puzzle's roster using the selected storage and its page-specific perfect-clear eligibility. */
	void finishPuzzleRoster(StorageRecord **storage, bool perfectClearEligible = true);
	/** Replace the first page layer's background and refresh the borrowed compatibility pointer. */
	bool loadPrimaryLayerBackground(const Common::Path &path);
	/**
	 * Restore the primary background on every call, then draw and advance its general-object animation runners.
	 * Call once at the start of complete puzzle composition, before other lower content and actors.
	 * Page-local overlays and subsequent layers must be recomposed afterward.
	 */
	void drawPrimaryPageLayer(ManagedSurface32 *screen);
	/** Render active roster entries in stable ascending logical-Y order, with the held entry last. */
	void renderZoombinis(ManagedSurface32 *screen) const;
	/** Selected difficulty captured for this puzzle instance. */
	int _puzzleLevel;
	/** Path of the first-layer background used by puzzle-specific redraws. */
	Common::String _backgroundPath;
	/** Borrowed immutable sprite grid retained by the engine cache. */
	const ZoombiniAnimation *_zoombiniAnimation = nullptr;
	/** Active party entries mirrored from the engine's roster for this puzzle. */
	Common::Array<ZoombiniRunner *> _puzzleZoombinis;
	/** Gameplay deadline used by the concrete puzzle state machine. */
	uint32 _stateTimer = 0;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_BASE_H
