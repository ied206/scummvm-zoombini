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
class ZoombiniAnimation;
class ZoombiniState;

/** 
 * Booliewood - In its normal state
 * 
 * Arrival shelter that presents the rescued community across a scrolling panorama.
 * 
 * @remark The save is considered complete after 400 Zoombinis arrive;
 * This page is only shown if the save is not complete.
 */
class ShelterBooliewood : public ShelterBase {
public:
	/** Construct the page-12 Booliewood shelter for @p vm. */
	explicit ShelterBooliewood(Zoombini2Engine *vm);
	/** Release this page's scene, path, visual, and audio resources. */
	~ShelterBooliewood() override;

	/** Build the community scene from the active profile's rescue history. */
	void init() override;
	/** Advance scrolling, attractions, walking Boolies, and crowd speech. */
	void onUpdate() override;
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	/** Draw the current cyclic viewport of the Booliewood panorama. */
	void onRenderScene(ManagedSurface32 *screen) override;
	void onRenderBackground(ManagedSurface32 *screen) override;
	void onRenderActors(ManagedSurface32 *screen) override;
	void onRenderForeground(ManagedSurface32 *screen) override;
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
	static const int kSceneWidth = 4000;
	/** Little-Zoombini cell used by seated community members. */
	static const int kSeatedZoombiniCell = 33;

	/** Initial position, horizontal limit, and row kind for one seat row. */
	struct SeatDefinition {
		/** Initial screen position for the first assigned Zoombini. */
		Common::Point32 initialPos;
		/** Largest permitted horizontal position for the row. */
		int maximumX;
		/** Animation or visual row kind. */
		int rowKind;
	};

	/** Allocation state for one horizontal row of seats. */
	struct Seat {
		/** Next screen position available in this row. */
		Common::Point32 nextPos;
		/** Largest permitted horizontal position for the row. */
		int maximumX;
		/** Animation or visual row kind. */
		int rowKind;
		/** Number of Zoombinis assigned to this row. */
		int assignedCount;
		/** Whether no further position remains in this row. */
		bool full;
	};

	/** Runtime state for one attraction animation. */
	struct AttractionState {
		Animation *animation;
		Common::Point32 pos;
		int frame;
		uint32 nextFrameTime;
		bool active;
	};

	/** Runtime state for one Boolie that alternates between waiting and walking. */
	struct CrowdActorState {
		PathObject *path;
		Common::Point32 waitPos;
		Common::Point32 pos;
		uint32 nextWalkTime;
		uint32 nextFrameTime;
		int frame;
		bool walking;
	};

	/** Initial position, maximum X, and row kind for every seat row. */
	static const SeatDefinition kSeatDefinitions[kNumSeats];
	/** Rescued-crowd marker pattern, stored from top row to bottom row. */
	static const char *const kCrowdPattern[27];

	/** Reset all seat rows to their initial allocation state. */
	void resetSeats();
	/** Assign one randomized available seat to @p pos. */
	bool assignSeat(Common::Point32 &pos);
	/** Seat incoming and historical Zoombinis from the active profile. */
	void buildSeatedCommunity(uint32 now);
	/** Reconstruct one historical Zoombini from @p traitHash. */
	static ZoombiniState *createHistoricalZoombini(int32 traitHash);
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
	void drawBackground(ManagedSurface32 *screen) const;
	/** Draw one animation frame at a cyclic scene-space position. */
	void drawAnimationInScene(const Animation *animation, int frameIndex, const Common::Point32 &pos, ManagedSurface32 *screen) const;
	/** Draw one RLE marker at a cyclic scene-space position. */
	void drawRleInScene(const RleBlock *frame, const Common::Point32 &pos, ManagedSurface32 *screen) const;
	/** Draw the stage-gated attractions. */
	void drawAttractions(ManagedSurface32 *screen) const;
	/** Draw the rescued-total marker crowd. */
	void drawRescuedCrowd(ManagedSurface32 *screen) const;
	/** Draw the seated community in vertical order. */
	void drawSeatedCommunity(ManagedSurface32 *screen) const;
	/** Draw all waiting and walking decorative Boolies. */
	void drawCrowdActors(ManagedSurface32 *screen) const;
	/** Draw one Little-Zoombini state from @p animation at a cyclic scene position. */
	void drawZoombiniInScene(const ZoombiniState &zoombini, const ZoombiniAnimation *animation, int cell, int animationFrame,
							 const Common::Point32 &pos, ManagedSurface32 *screen) const;

	/** Current horizontal origin within the cyclic panorama. */
	int _scrollX;
	int _pendingScrollDelta = 0;
	/** Development stage selected from the rescued total. */
	int _developmentStage;
	/** Seat allocation state. */
	Seat _seats[kNumSeats];
	/** Attraction animation state. */
	AttractionState _attractions[kAttractionCount];
	/** Decorative walking actor state. */
	CrowdActorState _crowdActors[kCrowdActorCount];

	/** Panorama background managed by this page. */
	BitBlock *_background;
	/** Borrowed immutable seated sprite grid owned by the engine cache. */
	const ZoombiniAnimation *_zoombiniAnimation;
	/** Borrowed immutable walking sprite grid owned by the engine cache. */
	const ZoombiniAnimation *_walkingZoombiniAnimation;
	/** Marker managed by this page for an ordinary rescued crowd cell. */
	RleBlock *_contentMarker;
	/** Marker managed by this page for a special rescued crowd cell. */
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
