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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ZOOMBINI2_PAGES_SHELTER_FINAL_H
#define ZOOMBINI2_PAGES_SHELTER_FINAL_H

#include "common/rect.h"

#include "zoombini2/pages/shelter_base.h"

namespace Zoombini2 {

class Animation;
class BitBlock;
class RleBlock;
class ZoombiniAnimation;
class ZoombiniState;

/** 
 * Booliewood - Celebration shown after 400 rescues.
 * 
 * @remark The save is considered complete after 400 Zoombinis arrive;
 * This page is only shown if the save is complete.
 */
class ShelterFinal : public ShelterBase {
public:
	/** Construct the final Booliewood shelter for @p vm. */
	explicit ShelterFinal(Zoombini2Engine *vm);
	/** Clear decorative Zoombinis, save the profile, and release celebration resources. */
	~ShelterFinal() override;

	/** Load the fixed celebration, decorative Zoombinis, music, and speech. */
	void init() override;
	/** Advance dancers, decorative Zoombinis, fireworks, and speech deadlines. */
	void onUpdate() override;
	/** Draw the complete fixed celebration scene. */
	void onRenderScene(ManagedSurface32 *screen) override;
	void onRenderActors(ManagedSurface32 *screen) override;
	/** Return to the sign-in menu on any click. */
	EventHandleResult onLButtonDown(const Common::Point &pos) override;

	/** Hide the shared shelter sidebar on the final celebration. */
	bool hasSidebar() const override { return false; }
	/** Suppress the shelter Go action on the final celebration. */
	bool hasGoButton() const override { return false; }

private:
	/** Number of independent firework animations. */
	static const int kFireworkCount = 3;
	/** Number of small dancing Boolies along the lower edge. */
	static const int kDancingBoolieCount = 3;
	/** Number of randomized celebration ambience clips. */
	static const int kAmbientSoundCount = 7;
	/** Number of decorative Zoombinis created for the celebration. */
	static const int kDecorativeZoombiniCount = 2;

	/** Runtime position, animation, and parabolic motion for one firework. */
	struct FireworkState {
		Animation *animation = nullptr;
		Common::Rect collisionRect = Common::Rect(1000, 1000, 1001, 1001);
		/** Initial screen position used by the parabolic trajectory. */
		Common::Point32 startPos = Common::Point32();
		int timeStep = 0;
		int frame = 0;
		uint32 nextFrameTime = 0;
		bool active = true;
	};

	/** Fixed locations of the three lower-edge dancers. */
	static const Common::Point32 kDancingBooliePos[kDancingBoolieCount];
	/** Fixed locations of the two decorative Zoombinis in the celebration. */
	static const Common::Point32 kDecorativeZoombiniPos[kDecorativeZoombiniCount];
	/** Resting cells restored after the two decorative Zoombinis finish walking. */
	static const int kDecorativeZoombiniCells[kDecorativeZoombiniCount];

	/** Allocate and initialize the two decorative Zoombinis. */
	void createDecorativeZoombinis();
	/** Apply the original randomized idle-to-walk and walk-to-idle tests. */
	void updateDecorativeZoombinis(uint32 now);
	/** Advance firework animation activity, restart one completed firework, or move all active fireworks. */
	void updateFireworks(uint32 now);
	/** Advance one firework's 40-millisecond non-looping animation. */
	static void advanceFireworkAnimation(FireworkState &firework, uint32 now);
	/** Give @p fireworkIndex a randomized non-overlapping origin and restart its animation. */
	void resetFirework(int fireworkIndex, uint32 now);
	/** Advance one active firework by one parabolic motion step. */
	static void moveFirework(FireworkState &firework);
	/** Return whether the original four-corner test accepts a candidate firework origin. */
	bool isFireworkPositionFree(const Common::Point32 &pos) const;
	/** Return whether one point lies strictly within the padded collision rectangle. */
	static bool pointInsidePaddedRect(const Common::Point32 &pos, const Common::Rect &rect);

	/** Schedule the next randomized 10-through-19-second ambient deadline. */
	void scheduleNextAmbient(uint32 now);
	/** Play one randomized celebration ambience clip. */
	void playRandomAmbient();
	/** Draw one plain AN animation frame at a fixed position. */
	void drawAnimation(const Animation *animation, int frameIndex, const Common::Point32 &pos, ManagedSurface32 *screen) const;
	/** Draw one Little-Zoombini state at a fixed position. */
	void drawZoombini(const ZoombiniState &zoombini, ManagedSurface32 *screen) const;

	/** Fixed celebration background managed by this page. */
	BitBlock *_background = nullptr;
	/** Persistent Grand Boolie foreground managed by this page. */
	BitBlock *_fullBigBool = nullptr;
	/** Dormant cell-reveal image managed by this page. */
	RleBlock *_revealBigBool = nullptr;
	/** Lower-edge dancing Boolie animation managed by this page. */
	Animation *_dancingBoolie = nullptr;
	/** Upper Boolie dance animation managed by this page. */
	Animation *_boolDance = nullptr;
	/** Dormant first reveal flare managed by this page. */
	Animation *_revealFlare1 = nullptr;
	/** Dormant second reveal flare managed by this page. */
	Animation *_revealFlare2 = nullptr;
	/** Borrowed immutable seated sprite grid owned by the engine cache. */
	const ZoombiniAnimation *_zoombiniAnimation = nullptr;
	/** Borrowed immutable walking sprite grid owned by the engine cache. */
	const ZoombiniAnimation *_walkingZoombiniAnimation = nullptr;
	/** Firework states in blue, green, red order. */
	FireworkState _fireworks[kFireworkCount] = {};

	/** Gameplay tick at which looping dance animations started. */
	uint32 _animationStartTime = 0;
	/** Whether the opening speech has completed once. */
	bool _openingSpeechFinished = false;
	/** Deadline for the one closing speech, or zero after it starts. */
	uint32 _closingSpeechTime = 0;
	/** Dormant reveal gate retained by the matched celebration state. */
	bool _revealStarted = false;
	/** Dormant reveal callback-ready flag. */
	bool _revealCellReady = false;
	/** Dormant reveal row. */
	int _revealRow = 0;
	/** Dormant reveal column. */
	int _revealColumn = 0;

	/** Looping finale music identifier. */
	int _musicId = -1;
	/** Opening FIN11 speech identifier. */
	int _openingSpeechId = -1;
	/** Closing INT11.17 speech identifier. */
	int _closingSpeechId = -1;
	/** Celebration ambience identifiers. */
	int _ambientSoundIds[kAmbientSoundCount] = {-1, -1, -1, -1, -1, -1, -1};
	/** Deadline for the next randomized ambience clip. */
	uint32 _nextAmbientTime = 0;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_BOOLIEWOOD_FINAL_H
