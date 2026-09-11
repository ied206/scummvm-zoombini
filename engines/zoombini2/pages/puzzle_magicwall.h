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

#ifndef ZOOMBINI2_PAGES_PUZZLE_MAGICWALL_H
#define ZOOMBINI2_PAGES_PUZZLE_MAGICWALL_H

#include "common/array.h"
#include "common/path.h"
#include "common/rect.h"

#include "zoombini2/graphics.h"
#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class RleBlock;
class Animation;
class PathObject;

/**
 * Beetle Bug Alley (Route2-1)
 *
 * Use the stone tablets to move every beetle onto its matching color marker.
 */
class PuzzleMagicWall : public PuzzleBase {
public:
	/** Construct Beetle Bug Alley for @p vm. */
	PuzzleMagicWall(Zoombini2Engine *vm);
	/** Release maze elements, paths, graphics, and sounds. */
	~PuzzleMagicWall() override;

	/** Load the selected maze and assign colors to the puzzle roster. */
	void init() override;
	/** Advance the active path, gates, and completion state. */
	void onUpdate() override;
	/** Draw the maze, controls, color guides, gates, and Zoombinis. */
	void onRenderScene(ManagedSurface32 *screen) override;
	/** Restore the page background. */
	void onRenderBackground(ManagedSurface32 *screen) override;
	/** Activate the tablet or lever at @p pos. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;

private:
	/** Color indices shared by doors, dots, bugs, and minimap lights. */
	enum Color {
		/** Blue resource set. */
		kColorBlue = 0,
		/** Green resource set. */
		kColorGreen,
		/** Navy resource set. */
		kColorNavy,
		/** Orange resource set. */
		kColorOrange,
		/** Purple resource set. */
		kColorPurple,
		/** Red resource set. */
		kColorRed,
		/** Rose resource set. */
		kColorRose,
		/** Turquoise resource set. */
		kColorTurquoise,
		/** Violet resource set. */
		kColorViolet,
		/** Yellow resource set. */
		kColorYellow,
		/** Number of color resource sets. */
		kColorCount
	};

	/** Resource-name fragments indexed by @ref MagicWallPuzzle::Color. */
	static const char *kColorNames[kColorCount];

	/** Runtime phase of the Beetle Bug Alley interaction. */
	enum State {
		/** Complete initial maze setup. */
		kStateInit,
		/** Wait for tablet or lever input. */
		kStateIdle,
		/** Move one Zoombini along its selected path. */
		kStateZoombiniMoving,
		/** Play the gate-opening animation. */
		kStateGateOpening,
		/** Hold after every assigned Zoombini reaches a destination. */
		kStateComplete,
		/** Stop accepting input while leaving the page. */
		kStateDone
	};

	/** One occupied or destination slot in the maze. */
	struct ZoombiniSlot {
		/** Index into @ref Puzzle::_puzzleZoombinis, or `-1` when empty. */
		int zoombiniIdx = -1;
		/** Percentage progress along the active path. */
		int pathProgress = 0;
		/** Color required by this Zoombini's destination. */
		int targetColor = -1;
		/** Whether the Zoombini has reached its destination. */
		bool captured = false;
		/** Current screen position. */
		Common::Point32 pos = Common::Point32();
		/** Path currently being traversed. */
		PathObject *path = nullptr;
		/** Time at which path traversal began. */
		uint32 pathStartTime = 0;
	};

	/** One colored destination marker in the maze. */
	struct ColorDot {
		/** Color index used by this marker. */
		int colorIdx;
		/** Screen position. */
		Common::Point32 pos;
		/** Whether the corresponding door light is enabled. */
		bool lightOn = false;
	};

	/** One colored beetle guide in the maze. */
	struct ColorBug {
		/** Color index used by this beetle. */
		int colorIdx;
		/** Screen position. */
		Common::Point32 pos;
		/** Whether the beetle is currently guiding a path. */
		bool active;
	};

	/** Clickable stone tablet that transfers a beetle between slots. */
	struct Tablet {
		/** Clickable tablet area. */
		Common::Rect rect;
		/** Slot from which the tablet moves a beetle. */
		int sourceSlot;
		/** Slot to which the tablet moves a beetle. */
		int destSlot;
		/** Path followed during the transfer. */
		PathObject *path = nullptr;
	};

	/** One destination gate and its animation state. */
	struct Gate {
		/** Gate index corresponding to gates A through D. */
		int gateIdx;
		/** Screen position. */
		Common::Point32 pos;
		/** Whether this gate is open. */
		bool open = false;
		/** Time at which the opening animation began. */
		uint32 animStart = 0;
	};

	/** Load all maze graphics, animations, paths, and sounds. */
	void loadResources();
	/** Configure the maze variant selected by level. */
	void setupMaze();
	/** Place colored destination markers. */
	void placeColorDots();
	/** Place colored beetle guides. */
	void placeColorBugs();
	/** Configure tablet hit areas and transfer paths. */
	void setupTablets();
	/** Assign puzzle-roster entries to entrance slots. */
	void assignZoombiniSlots();

	/** Start path traversal for slot @p slotIdx. */
	void startZoombiniPath(int slotIdx);
	/** Advance the path traversal for slot @p slotIdx. */
	void advanceZoombiniPath(int slotIdx);
	/** Return whether slot @p slotIdx has reached the correct destination. */
	bool checkSlotComplete(int slotIdx);
	/** Mark slot @p slotIdx captured and update gate state. */
	void completeSlot(int slotIdx);
	/** Synchronize door lights with the current color arrangement. */
	void updateLights();
	/** Return the number of slots that reached their destinations. */
	int countCaptured() const;

	/** Draw maze layer @p level. */
	void drawMazeLevel(ManagedSurface32 *screen, int level);
	/** Draw colored destination markers. */
	void drawColorDots(ManagedSurface32 *screen);
	/** Draw colored beetle guides. */
	void drawColorBugs(ManagedSurface32 *screen);
	/** Draw stone tablet controls. */
	void drawTablets(ManagedSurface32 *screen);
	/** Draw the glowworm wall lever. */
	void drawWallLever(ManagedSurface32 *screen);
	/** Draw the minimap and its color lights. */
	void drawMinimap(ManagedSurface32 *screen);
	/** Draw all destination gates. */
	void drawGates(ManagedSurface32 *screen);
	/** Draw waiting and moving Zoombinis. */
	void onRenderActors(ManagedSurface32 *screen) override;

	/** Current interaction phase. */
	State _state = kStateInit;
	/** Level-selected maze variant. */
	int _currentLevel = 0;
	/** Currently moving slot, or `-1` when none is active. */
	int _activeSlot = -1;
	/** Destination slot for the current movement. */
	int _destSlot = -1;
	/** Number of Zoombinis already captured by their matching gates. */
	int _capturedCount = 0;

	/** Four entrance slots followed by four destination slots. */
	ZoombiniSlot _slots[8];

	/** Destination color indexed by puzzle-roster entry. */
	int _zoombiniColors[16];

	/** Colored destination markers. */
	Common::Array<ColorDot> _colorDots;
	/** Colored beetle guides. */
	Common::Array<ColorBug> _colorBugs;
	/** Stone tablet controls. */
	Common::Array<Tablet> _tablets;
	/** Glowworm lever hit-test area. */
	Common::Rect _wallLever = Common::Rect();
	/** Four destination gates. */
	Gate _gates[4];

	/** Destination-dot visuals indexed by color. */
	RleBlock *_dotImage[kColorCount] = {};
	/** Beetle visuals indexed by color. */
	RleBlock *_bugImage[kColorCount] = {};
	/** Minimap background. */
	RleBlock *_miniMapImage = nullptr;
	/** Minimap position marker. */
	RleBlock *_miniMapDotImage = nullptr;
	/** Minimap lights indexed by color. */
	RleBlock *_miniLightImage[kColorCount] = {};
	/** Static glowworm lever visual. */
	RleBlock *_glowwormImage = nullptr;
	/** Animated glowworm lever visual. */
	Animation *_glowwormAnim = nullptr;
	/** Gate-opening animations. */
	Animation *_gateAnims[4] = {};
	/** Crystal feedback animations. */
	Animation *_crystalAnims[5] = {};

	/** Paths from matching gates to the maze exits. */
	PathObject *_exitPaths[4] = {};
	/** Internal movement paths connecting maze slots. */
	PathObject *_bougePaths[4] = {};

	/** Music handle used while Beetle Bug Alley is active. */
	int _musicId = -1;
	/** Approval sounds cycled after successful moves. */
	int _sndApproval[4] = {-1, -1, -1, -1};
	/** Error sounds selected after invalid moves. */
	int _sndError[2] = {-1, -1};
	/** Hint sounds associated with the four gates. */
	int _sndHint[4] = {-1, -1, -1, -1};
	/** Gate-opening sound. */
	int _sndGateOpen = -1;
	/** Zoombini movement sound. */
	int _sndZoombiniMove = -1;
	/** Index of the next approval sound to play. */
	int _nextApprovalIdx = 0;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_PUZZLE_MAGICWALL_H
