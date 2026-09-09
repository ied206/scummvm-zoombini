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

#ifndef ZOOMBINI2_PAGES_SHELTER_BOOLIEWOOD_FINAL_H
#define ZOOMBINI2_PAGES_SHELTER_BOOLIEWOOD_FINAL_H

#include "common/rect.h"

#include "zoombini2/pages/shelter_base.h"

namespace Zoombini2 {

class Animation;
class BitBlock;
class RleBlock;
class ZoombiniGraphics;
class ZoombiniState;

/** Separate world-23 Booliewood celebration shown after 400 rescues. */
class BooliewoodFinalPage : public ShelterPage {
public:
	/** Construct the final Booliewood shelter for @p engine. */
	explicit BooliewoodFinalPage(Zoombini2Engine *engine);
	/** Clear decorative Zoombinis, save the profile, and release celebration resources. */
	~BooliewoodFinalPage() override;

	/** Load the fixed celebration, decorative Zoombinis, music, and speech. */
	void init() override;
	/** Advance dancers, decorative Zoombinis, fireworks, and speech deadlines. */
	void update() override;
	/** Draw the complete fixed celebration scene. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Return to the sign-in menu on any click. */
	void handleClick(const Common::Point &pos) override;

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
	/** Number of page-owned decorative Zoombinis. */
	static const int kDecorativeZoombiniCount = 2;

	/** Runtime position, animation, and parabolic motion for one firework. */
	struct FireworkState {
		Animation *animation;
		Common::Rect collisionRect;
		int startY;
		int timeStep;
		int frame;
		uint32 nextFrameTime;
		bool active;
	};

	/** Fixed locations of the three lower-edge dancers. */
	static const int kDancingBooliePositions[kDancingBoolieCount][2];
	/** Fixed locations of the two page-owned Zoombinis. */
	static const int kDecorativeZoombiniPositions[kDecorativeZoombiniCount][2];
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
	bool isFireworkPositionFree(int x, int y) const;
	/** Return whether one point lies strictly within the padded collision rectangle. */
	static bool pointInsidePaddedRect(int x, int y, const Common::Rect &rect);

	/** Schedule the next randomized 10-through-19-second ambient deadline. */
	void scheduleNextAmbient(uint32 now);
	/** Play one randomized celebration ambience clip. */
	void playRandomAmbient();
	/** Draw one plain AN animation frame at a fixed position. */
	void drawAnimation(const Animation *animation, int frameIndex, int x, int y, Graphics::ManagedSurface *screen) const;
	/** Draw one Little-Zoombini state at a fixed position. */
	void drawZoombini(const ZoombiniState &zoombini, int decorativeIndex, Graphics::ManagedSurface *screen) const;

	/** Owned fixed celebration background. */
	BitBlock *_background;
	/** Owned persistent Grand Boolie foreground. */
	BitBlock *_fullBigBool;
	/** Owned dormant cell-reveal image. */
	RleBlock *_revealBigBool;
	/** Owned lower-edge dancing Boolie animation. */
	Animation *_dancingBoolie;
	/** Owned upper Boolie dance animation. */
	Animation *_boolDance;
	/** Owned dormant first reveal flare. */
	Animation *_revealFlare1;
	/** Owned dormant second reveal flare. */
	Animation *_revealFlare2;
	/** Owned Little-Zoombini sprite grid. */
	ZoombiniGraphics *_zoombiniGfx;
	/** Owned attenteZomb sprite grid used while a decorative Zoombini walks. */
	ZoombiniGraphics *_walkingZoombiniGfx;
	/** Current attenteZomb frame for each decorative Zoombini. */
	int _decorativeAnimationFrames[kDecorativeZoombiniCount];
	/** Deadline for each decorative Zoombini's next attenteZomb frame. */
	uint32 _decorativeNextFrameTimes[kDecorativeZoombiniCount];
	/** Firework states in blue, green, red order. */
	FireworkState _fireworks[kFireworkCount];

	/** Gameplay tick at which looping dance animations started. */
	uint32 _animationStartTime;
	/** Whether the opening speech has completed once. */
	bool _openingSpeechFinished;
	/** Deadline for the one closing speech, or zero after it starts. */
	uint32 _closingSpeechTime;
	/** Dormant reveal gate retained by the matched celebration state. */
	bool _revealStarted;
	/** Dormant reveal callback-ready flag. */
	bool _revealCellReady;
	/** Dormant reveal row. */
	int _revealRow;
	/** Dormant reveal column. */
	int _revealColumn;

	/** Looping finale music identifier. */
	int _musicId;
	/** Opening FIN11 speech identifier. */
	int _openingSpeechId;
	/** Closing INT11.17 speech identifier. */
	int _closingSpeechId;
	/** Celebration ambience identifiers. */
	int _ambientSoundIds[kAmbientSoundCount];
	/** Deadline for the next randomized ambience clip. */
	uint32 _nextAmbientTime;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_BOOLIEWOOD_FINAL_H
