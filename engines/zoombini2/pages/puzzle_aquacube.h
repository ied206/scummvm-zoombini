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

#ifndef ZOOMBINI2_PAGES_PUZZLE_AQUACUBE_H
#define ZOOMBINI2_PAGES_PUZZLE_AQUACUBE_H

#include "zoombini2/pages/puzzle_base.h"

namespace Zoombini2 {

class Animation;
class AnimationRunner;
struct PathObject;

/** Move through a randomized cube, rescuing its occupants before the movement allowance expires. */
class PuzzleAquacube : public PuzzleBase {
public:
	PuzzleAquacube(Zoombini2Engine *vm);
	~PuzzleAquacube() override;
	void init() override;
	void onUpdate() override;
	void onRenderBackground(ManagedSurface32 *screen) override;
	void onRenderContent(ManagedSurface32 *screen) override;
	void onRenderActors(ManagedSurface32 *screen) override;
	void onActorsRendered() override;
	void onRenderForeground(ManagedSurface32 *screen) override;
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	bool canUseGoButton() const override;
	bool onGoButtonPressed() override;
	Common::String debugGetAnswer() const override;
	PuzzleChanceInfo debugGetChances() const override;
	bool debugCanSetChances() const override;
	bool debugSetChances(int remaining) override;
	Common::String debugGetChanceDetails() const override;

private:
	static constexpr const char *kMusicPath = "#sounds/music/03-BB01.wav";
	static constexpr const char *kLightPath = "bmp/aquacube/light";
	static constexpr const char *kBallPaths[2] = {
		"bmp/aquacube/ballBIG",
		"bmp/aquacube/ball",
	};
	static constexpr const char *kCubeFormat = "bmp/aquacube/kub_%s_%02d";
	static constexpr const char *kLeverPaths[2] = {
		"bmp/aquacube/control_manetteOFF",
		"bmp/aquacube/control_manetteON",
	};
	static constexpr const char *kIndicatorPaths[2] = {
		"bmp/aquacube/control_manette_lightGREY",
		"bmp/aquacube/control_manette_lightRED",
	};
	static constexpr const char *kShotPaths[2] = {
		"bmp/aquacube/control_shotsOFF",
		"bmp/aquacube/control_shotsON",
	};
	static constexpr const char *kWarpPaths[3] = {
		"bmp/aquacube/control_warpBUTTON_OFF",
		"bmp/aquacube/control_warpBUTTON_ON",
		"bmp/aquacube/control_warpBUTTON_DISABLE",
	};
	static constexpr const char *kTimerEmptyPath = "bmp/aquacube/control_warpTIMER_empty";
	static constexpr const char *kTimerPath = "bmp/aquacube/control_warpTIMER";
	static constexpr const char *kFlareFormat = "bmp/aquacube/flare%d";
	static constexpr const char *kBubbleFormat = "bmp/aquacube/bubble%d";
	static constexpr const char *kFleenFormat = "bmp/aquacube/fleen/fixe/f%dfixe";
	static constexpr const char *kAngryFormat = "bmp/aquacube/fleen/vener/f%dma66";
	static constexpr const char *kChaseFormat = "bmp/aquacube/fleen/marche/f%dco66";
	static constexpr const char *kSmallestPath = "bmp/aquacube/smallest/smallest.anm";
	static constexpr const char *kIdlePath = "bmp/aquacube/smallest/attente/attente.anm";
	static constexpr const char *kSoundPaths[5] = {
		"sounds/fx/03-BS01.wav",
		"sounds/fx/03-BS03.wav",
		"sounds/fx/03-BS05.wav",
		"sounds/fx/03-BB02.wav",
		"sounds/fx/FleenChasesZs.wav",
	};
	static constexpr const char *kPraisePaths[2] = {
		"sounds/8-E2.wav",
		"sounds/8-E1.wav",
	};
	static constexpr const char *kRetreatPath = "sounds/DW-Zville.wav";

	enum NodeState {
		kOccupied00 = 0,
		kEmpty01 = 1,
		kStart02 = 2,
		kFleen03 = 3
	};
	enum WarpButtonState {
		kWarpButtonOff00 = 0,
		kWarpButtonOn01 = 1,
		kWarpButtonDisabled02 = 2
	};
	struct Node {
		int adj[4] = {};
		Common::Point32 pos;
		NodeState state = kEmpty01;
		int occupantCount = 0;
		int occupants[3] = {};
		int coordinates = 0;
		int fleenType = 0;
	};
	struct NodeLayout {
		int adj[4];
		int x, y;
		const char *labels;
	};
	struct Bubble {
		bool active = false;
		int type = 0;
		int originX = 100;
		float x = 100;
		float y = 640;
		float phase = 0;
	};
	static constexpr NodeLayout kEasyNodes[8] = {
		{{1, 3, 4, -1}, 162, 530, "DLF"},
		{{0, 2, 5, -1}, 524, 530, "DRF"},
		{{3, 1, 6, -1}, 591, 175, "URF"},
		{{2, 0, 7, -1}, 92, 175, "ULF"},
		{{5, 7, 0, -1}, 238, 350, "DLB"},
		{{4, 6, 1, -1}, 441, 350, "DRB"},
		{{7, 5, 2, -1}, 458, 136, "URB"},
		{{6, 4, 3, -1}, 218, 136, "ULB"},
	};
	static constexpr NodeLayout kHardNodes[16] = {
		{{1, 3, 12, 4}, 165, 533, "DLFX"},
		{{0, 2, 13, 5}, 527, 533, "DRFX"},
		{{3, 1, 14, 6}, 594, 178, "URFX"},
		{{2, 0, 15, 7}, 95, 178, "ULFX"},
		{{5, 7, 8, 0}, 275, 403, "DLFC"},
		{{4, 6, 9, 1}, 406, 403, "DRFC"},
		{{7, 5, 10, 2}, 419, 260, "URFC"},
		{{6, 4, 11, 3}, 269, 260, "ULFC"},
		{{9, 11, 4, 12}, 293, 335, "DLBC"},
		{{8, 10, 5, 13}, 393, 335, "DRBC"},
		{{11, 9, 6, 14}, 403, 223, "URBC"},
		{{10, 8, 7, 15}, 283, 223, "ULBC"},
		{{13, 15, 0, 8}, 241, 353, "DLBX"},
		{{12, 14, 1, 9}, 444, 353, "DRBX"},
		{{15, 13, 2, 10}, 461, 139, "URBX"},
		{{14, 12, 3, 11}, 221, 139, "ULBX"},
	};
	static constexpr Common::Point32 kRescuePositions[16] = {
		{775, 38},
		{778, 62},
		{760, 40},
		{763, 59},
		{750, 62},
		{744, 41},
		{734, 64},
		{730, 46},
		{722, 63},
		{715, 49},
		{707, 64},
		{695, 53},
		{692, 66},
		{676, 59},
		{674, 66},
		{663, 62},
	};
	static constexpr Common::Point32 kLeverPositions[4] = {
		{649, 483},
		{674, 484},
		{698, 483},
		{725, 482},
	};
	static constexpr Common::Point32 kIndicatorPositions[4] = {
		{648, 526},
		{676, 527},
		{701, 526},
		{727, 525},
	};
	static constexpr int kShotX[11] = {
		647,
		658,
		667,
		676,
		685,
		695,
		705,
		713,
		723,
		730,
		738,
	};

	void setupBoard();
	void loadResources();
	int findNode(int coordinates) const;
	void putFleen(int coordinates, int type);
	PathObject *makePath(const Common::Point32 &start, const Common::Point32 &end, int speed);
	void moveBall(int axis);
	void resolveArrival();
	void finishPuzzle();
	void beginChase();
	void updateBubbles(uint32 elapsed);
	void playSound(int index);
	int countFreeZoombinis() const;
	AnimationRunner *loadRunner(const Common::Path &path, int frames, int repeats, uint32 delay);
	static void onFlareComplete(void *context, AnimationRunner *runner);
	static void onAngryComplete(void *context, AnimationRunner *runner);
	static void onTimerComplete(void *context, AnimationRunner *runner);

	int _level = 1;
	int _numNodes = 8;
	int _dimensions = 3;
	int _maxSteps = 6;
	int _warpQuota = 0;
	int _ballNode = 0;
	int _stepsUsed = 0;
	int _warpsUsed = 0;
	int _freedCount = 0;
	int _fleenIndex = -1;
	int _axisMap[4] = {};
	bool _leverOn[4] = {};
	bool _warpPending[4] = {};
	bool _warpPlanning = false;
	bool _warpExecuting = false;
	bool _finished = false;
	bool _goPending = false;
	bool _actorsEscaping = false;
	bool _pendingInitialArrival = false;
	Node _nodes[16];
	Bubble _bubbles[12];
	Common::Array<int> _rescued;
	Common::Point32 _ballPos;
	PathObject *_ballPath = nullptr;
	PathObject *_chasePath = nullptr;
	uint32 _lastTick = 0;
	int _sounds[5] = {
		-1,
		-1,
		-1,
		-1,
		-1,
	};
	int _praiseSpeech = -1;
	int _goSpeech = -1;

	AnimationRunner *_timer = nullptr;
	AnimationRunner *_flare[2] = {};
	AnimationRunner *_angry[4] = {};
	AnimationRunner *_chase[4] = {};
	/** Frame data retained until the associated page runners have been released. */
	Common::Array<Animation *> _animations;
	const ZoombiniAnimation *_smallest = nullptr;
	const ZoombiniAnimation *_idle = nullptr;
};

} // End of namespace Zoombini2

#endif
