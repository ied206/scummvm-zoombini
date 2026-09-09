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

#ifndef ZOOMBINI2_PAGES_SHELTER_BOOLIEWOOD_H
#define ZOOMBINI2_PAGES_SHELTER_BOOLIEWOOD_H

#include "common/rect.h"

#include "zoombini2/pages/shelter_base.h"

namespace Zoombini2 {

class Animation;
class BitBlock;
class PathObject;
class RleBlock;
class ZoombiniGraphics;
class ZoombiniState;

/** Arrival shelter that presents the rescued community across a scrolling panorama. */
class BooliewoodPage : public ShelterPage {
public:
	/** Construct the world-12 Booliewood shelter for @p engine. */
	explicit BooliewoodPage(Zoombini2Engine *engine);
	/** Release page-owned scene, path, visual, and audio resources. */
	~BooliewoodPage() override;

	/** Build the community scene from the active profile's rescue history. */
	void init() override;
	/** Advance scrolling, attractions, walking Boolies, and crowd speech. */
	void update() override;
	/** Draw the current cyclic viewport of the Booliewood panorama. */
	void draw(Graphics::ManagedSurface *screen) override;
	/** Report that Booliewood has no forward Go action. */
	bool hasGoButton() const override { return false; }

private:
	/** Number of horizontal seating rows distributed through the panorama. */
	static const int kNumSeats = 23;
	/** Maximum number of seated Zoombinis shown by this page. */
	static const int kMaximumVisibleZoombinis = 80;
	/** Number of independent decorative path actors. */
	static const int kCrowdActorCount = 17;
	/** Number of stage-gated attraction animations. */
	static const int kAttractionCount = 7;
	/** Number of randomized Booliewood crowd clips. */
	static const int kAmbientSpeechCount = 6;
	/** Width of the cyclic panorama in pixels. */
	static const int kWorldWidth = 4000;
	/** Little-Zoombini cell used by seated community members. */
	static const int kSeatedZoombiniCell = 33;

	/** Allocation state for one horizontal row of seats. */
	struct Seat {
		int initialX;
		int y;
		int maximumX;
		int rowKind;
		int assignedCount;
		int nextX;
		bool full;
	};

	/** Runtime state for one attraction animation. */
	struct AttractionState {
		Animation *animation;
		int worldX;
		int y;
		int frame;
		uint32 nextFrameTime;
		bool active;
	};

	/** Runtime state for one Boolie that alternates between waiting and walking. */
	struct CrowdActorState {
		PathObject *path;
		Common::Point32 waitPosition;
		Common::Point32 position;
		uint32 nextWalkTime;
		uint32 nextFrameTime;
		int frame;
		bool walking;
	};

	/** Initial X, Y, maximum X, and row kind for every seat row. */
	static const int kSeatDefinitions[kNumSeats][4];
	/** Rescued-crowd marker pattern, stored from top row to bottom row. */
	static const char *const kCrowdPattern[27];

	/** Reset all seat rows to their initial allocation state. */
	void resetSeats();
	/** Assign one randomized available seat to @p position. */
	bool assignSeat(Common::Point32 &position);
	/** Seat incoming and historical Zoombinis from the active profile. */
	void buildSeatedCommunity(uint32 now);
	/** Reconstruct one historical Zoombini from @p featureHash. */
	static ZoombiniState *createHistoricalZoombini(int32 featureHash);
	/** Advance the looping attenteZomb frames assigned to historical Zoombinis. */
	void updateSeatedAnimations(uint32 now);

	/** Load and initialize all development-stage attraction animations. */
	void loadAttractions(uint32 now);
	/** Advance chained rail animations and looping stage attractions. */
	void updateAttractions(uint32 now);
	/** Return the frame delay for the selected attraction state. */
	static uint32 getAttractionFrameDelay(int attractionIndex, int frame);

	/** Load all 17 PAT routes and initialize their waiting actors. */
	void loadCrowdActors(uint32 now);
	/** Advance every decorative crowd actor. */
	void updateCrowdActors(uint32 now);
	/** Return the waiting-animation delay for @p frame. */
	static uint32 getWaitingFrameDelay(int frame);

	/** Apply one frame of edge-driven cyclic panorama scrolling. */
	void updateScroll();
	/** Schedule the next randomized crowd speech deadline. */
	void scheduleAmbientSpeech(uint32 now);
	/** Play one randomized crowd speech clip. */
	void playAmbientSpeech();

	/** Draw the cyclic background window at the current scroll offset. */
	void drawBackground(Graphics::ManagedSurface *screen) const;
	/** Draw one animation frame at a cyclic world-space position. */
	void drawAnimationAtWorld(const Animation *animation, int frameIndex, int worldX, int y, Graphics::ManagedSurface *screen) const;
	/** Draw one RLE marker at a cyclic world-space position. */
	void drawRleAtWorld(const RleBlock *frame, int worldX, int y, Graphics::ManagedSurface *screen) const;
	/** Draw the stage-gated attractions. */
	void drawAttractions(Graphics::ManagedSurface *screen) const;
	/** Draw the rescued-total marker crowd. */
	void drawRescuedCrowd(Graphics::ManagedSurface *screen) const;
	/** Draw the seated community in vertical order. */
	void drawSeatedCommunity(Graphics::ManagedSurface *screen) const;
	/** Draw all waiting and walking decorative Boolies. */
	void drawCrowdActors(Graphics::ManagedSurface *screen) const;
	/** Draw one Little-Zoombini state from @p graphics at a cyclic world position. */
	void drawZoombiniAtWorld(const ZoombiniState &zoombini, const ZoombiniGraphics *graphics, int cell, int animationFrame, int worldX, int y,
						  Graphics::ManagedSurface *screen) const;

	/** Current horizontal origin within the cyclic panorama. */
	int _scrollX;
	/** Development stage selected from the rescued total. */
	int _developmentStage;
	/** Seat allocation state. */
	Seat _seats[kNumSeats];
	/** Attraction animation state. */
	AttractionState _attractions[kAttractionCount];
	/** Decorative walking actor state. */
	CrowdActorState _crowdActors[kCrowdActorCount];

	/** Owned panorama background. */
	BitBlock *_background;
	/** Owned Little-Zoombini sprite grid. */
	ZoombiniGraphics *_zoombiniGfx;
	/** Owned attenteZomb sprite grid used by selected historical Zoombinis. */
	ZoombiniGraphics *_walkingZoombiniGfx;
	/** Whether each visible Zoombini uses the looping attenteZomb grid. */
	bool _seatedWalking[kMaximumVisibleZoombinis];
	/** Current attenteZomb frame for each visible Zoombini. */
	int _seatedAnimationFrames[kMaximumVisibleZoombinis];
	/** Deadline for each visible Zoombini's next attenteZomb frame. */
	uint32 _seatedNextFrameTimes[kMaximumVisibleZoombinis];
	/** Owned marker for an ordinary rescued crowd cell. */
	RleBlock *_contentMarker;
	/** Owned marker for a special rescued crowd cell. */
	RleBlock *_pascontentMarker;
	/** Shared walking animation used by the 17 decorative actors. */
	Animation *_walkingAnimation;
	/** Shared waiting animation used by the 17 decorative actors. */
	Animation *_waitingAnimation;

	/** Looping Booliewood music identifier. */
	int _musicId;
	/** First-visit narration identifier. */
	int _introSpeechId;
	/** Random crowd speech identifiers. */
	int _ambientSpeechIds[kAmbientSpeechCount];
	/** Deadline for the next random crowd speech. */
	uint32 _nextAmbientSpeechTime;
	/** Whether progress enables the random crowd speech loop. */
	bool _ambientSpeechEnabled;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_SHELTER_BOOLIEWOOD_H
