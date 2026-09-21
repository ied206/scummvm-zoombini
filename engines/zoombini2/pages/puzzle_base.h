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

struct BoardRecord;
class ZoombiniRunner;
class ZoombiniAnimation;

/** Snapshot of the active puzzle's attempt or resource budget. */
struct PuzzleChanceInfo {
	enum class Type {
		kNone = 0,
		kAmorphous = 1,
		kInfinite = 2,
		kSubmit = 3,
		kMistake = 4,
	};
	Type type = Type::kNone;
	int opportunities = -1;
	int used = -1;
	const char *unit = nullptr;

	PuzzleChanceInfo() = default;
	PuzzleChanceInfo(Type kind, int maximum = -1, int consumed = -1, const char *unitName = nullptr)
		: type(kind), opportunities(maximum), used(consumed), unit(unitName) {}

	int chancesLeft() const { return 0 <= opportunities && 0 <= used ? MAX(0, opportunities - used) : -1; }
	static const char *typeName(Type type);
};

/**
 * Common page base for the nine rescue-mission puzzles.
 *
 * Provides shared puzzle resources and party state.
 */
class PuzzleBase : public InteractiveBase {
public:
	/** Bind shared puzzle state to @p vm, capture its selected difficulty, and record @p puzzleId. */
	PuzzleBase(Zoombini2Engine *vm, int puzzleId);
	/** Release shared puzzle resources. */
	~PuzzleBase() override = default;
	/** Return whether the shared sidebar is visible. */
	bool hasSidebar() const override { return true; }

	/** Initialize the shared puzzle background, sprite grid, and roster. */
	void init() override;
	/** Accept the whole current party and use the regular practice or adventure departure. */
	void debugForceFinish();
	/** Describe generated rules without changing the board or consuming random numbers. */
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
	/** Return the display name belonging to @p puzzleId. */
	static const char *getPuzzleName(int puzzleId);
	/** Return the resource-directory name belonging to @p puzzleId. */
	static const char *getPuzzleDir(int puzzleId);

private:
	/** Resource path format. */
	static constexpr const char *kPuzzleBackgroundFormat = "#bmp/%s";
	/** Resource path. */
	static constexpr const char *kZoombiniAnimationPath = "bmp/zombis/littleZomb.anm";
	/** Resource path. */
	static constexpr const char *kCrazyTurtleBackgroundPath = "crazy_turtle/background";
	/** Resource path. */
	static constexpr const char *kWaterslideBackgroundPath = "waterslide/waterslides";
	/** Resource path. */
	static constexpr const char *kAquacubeBackgroundPath = "aquacube/background";
	/** Resource path. */
	static constexpr const char *kMysticMarshBackgroundPath = "mystic_marsh/background1";
	/** Resource path. */
	static constexpr const char *kMagicWallBackgroundPath = "magic_wall/magic wall";
	/** Resource path. */
	static constexpr const char *kWallOfFleensBackgroundPath = "wall_of_fleens/background";
	/** Resource path. */
	static constexpr const char *kChezNorfBackgroundPath = "chez_norf/baquegund";
	/** Resource path. */
	static constexpr const char *kSnowboardBackgroundPath = "snowboard/snowboard-EASY";
	/** Resource path. */
	static constexpr const char *kBooliesBackgroundPath = "Boolies/background";

protected:
	/** Banner shared by all puzzle answer reports. */
	Common::String debugAnswerHeader() const;
	/** Print a roster entry using stable one-based indices and named traits. */
	Common::String debugActorDescription(int index) const;
	/** Whether a forced departure has already been requested. */
	bool _debugFinishPending = false;
	/** Finish this puzzle's roster using the board selected by the concrete puzzle. */
	void finishPuzzleRoster(BoardRecord **board);
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
	/** Numeric dispatcher identifier for the concrete puzzle. */
	int _puzzleId;
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
