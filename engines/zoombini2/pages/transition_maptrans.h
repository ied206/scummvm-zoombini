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

#ifndef ZOOMBINI2_PAGES_TRANSITION_MAPTRANS_H
#define ZOOMBINI2_PAGES_TRANSITION_MAPTRANS_H

#include "common/array.h"
#include "common/str.h"

#include "zoombini2/pages/transition_base.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

class ZoombiniAnimation;

/** Transition page that walks the current party along a route on the mountain map. */
class TransitionMapTrans : public TransitionBase {
public:
	/** Construct the map transition for @p vm. */
	TransitionMapTrans(Zoombini2Engine *vm);
	/** Release the composited map and per-Zoombini paths. */
	~TransitionMapTrans() override;

	/** Load the route, compose map overlays, and initialize the walking party. */
	void init() override;
	/** Start and advance staggered walkers until the transition completes. */
	void onUpdate() override;
	/** Draw the composited map and active walkers. */
	void onRenderContent(ManagedSurface32 *screen) override;
	void onRenderActors(ManagedSurface32 *screen) override;
	void onActorsRendered() override;
	/** Skip the remaining walking animation and enter the resolved destination on button release. */
	EventHandleResult onLButtonUp(const Common::Point &pos) override;
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;
	/** Resolve the next route page from the source page, Rescue Site I branch, and rescue progress. */
	static PageId getDestPage(PageId src, Zoombini2Engine::RouteBranch routeBranch, int rescuedBoolies);

private:
	/** Minimum party size before the first rescue-site movie is selected. */
	static constexpr int kRescue1MovieMinimumZoombinis = 8;
	/** Audio path format that appends a speech clip name and .wav extension. */
	static constexpr const char *kSpeechFormat = "sounds/%s.wav";
	/** PAT route format for the source page's mountain-map travel path. */
	static constexpr const char *kRouteFormat = "bmp/maptrans/%s";
	/** Zoombini sprite grid used while the party walks along the mountain map. */
	static constexpr const char *kZoombiniAnimationPath = "bmp/transition1/PitiZomb3.anm";
	/** Background music played during map travel and queued travel speech. */
	static constexpr const char *kMusicPath = "#sounds/music/ZMR-Transition.wav";
	/** First-visit speech queued when leaving Zoombiniville for Crazy Turtle. */
	static constexpr const char *kSpeechTur11 = "tur11";
	/** Revisit greeting queued when leaving Zoombiniville before Booliewood was visited. */
	static constexpr const char *kSpeechZbv31_2 = "zbv31.2";
	/** Revisit greeting queued when leaving Zoombiniville after Booliewood was visited. */
	static constexpr const char *kSpeechZbv31_3 = "zbv31.3";
	/** Random revisit speech format queued after the Zoombiniville greeting. */
	static constexpr const char *kSpeechTur21Format = "tur21.%d";
	/** First-visit speech queued when leaving Crazy Turtle for Waterslide. */
	static constexpr const char *kSpeechWsl11 = "wsl11";
	/** Random revisit speech format queued when leaving Crazy Turtle for Waterslide. */
	static constexpr const char *kSpeechWsl21Format = "wsl21.%d";
	/** First clip of the three-part first-visit Waterslide-to-AquaCube speech. */
	static constexpr const char *kSpeechAqu11_1_1 = "aqu11.1.1";
	/** Second clip of the three-part first-visit Waterslide-to-AquaCube speech. */
	static constexpr const char *kSpeechAqu11_1_2 = "aqu11.1.2";
	/** Third clip of the three-part first-visit Waterslide-to-AquaCube speech. */
	static constexpr const char *kSpeechAqu11_1_3 = "aqu11.1.3";
	/** Random revisit speech format queued when leaving Waterslide for AquaCube. */
	static constexpr const char *kSpeechAqu21Format = "aqu21.%d";
	/** Speech queued when leaving AquaCube for Rescue Site I. */
	static constexpr const char *kSpeechAqu31 = "aqu31";
	/** First-visit speech queued for the north branch from Rescue Site I to Magic Wall. */
	static constexpr const char *kSpeechMgw11 = "mgw11";
	/** Random revisit speech format for the north branch to Magic Wall. */
	static constexpr const char *kSpeechMgw21Format = "mgw21.%d";
	/** First-visit speech queued for the south branch from Rescue Site I to Mystic Marsh. */
	static constexpr const char *kSpeechMym11 = "mym11";
	/** Random revisit speech format for the south branch to Mystic Marsh. */
	static constexpr const char *kSpeechMym21Format = "mym21.%d";
	/** First-visit speech queued when leaving Mystic Marsh for Wall of Fleens. */
	static constexpr const char *kSpeechWlf11 = "wlf11";
	/** Random revisit speech format when leaving Mystic Marsh for Wall of Fleens. */
	static constexpr const char *kSpeechWlf21Format = "wlf21.%d";
	/** First clip of the two-part first-visit Magic Wall-to-Chez Norf speech. */
	static constexpr const char *kSpeechCzn11_1 = "czn11.1";
	/** Second clip of the two-part first-visit Magic Wall-to-Chez Norf speech. */
	static constexpr const char *kSpeechCzn11_2 = "czn11.2";
	/** Random revisit speech format when leaving Magic Wall for Chez Norf. */
	static constexpr const char *kSpeechCzn21Format = "czn21.%d";
	/** Speech queued when leaving Wall of Fleens for Rescue Site II. */
	static constexpr const char *kSpeechBc211 = "bc211";
	/** Speech queued when leaving Chez Norf for Rescue Site II. */
	static constexpr const char *kSpeechBc212 = "bc212";
	/** First clip of the two-part first-visit Rescue Site II-to-Snowboard speech. */
	static constexpr const char *kSpeechSwb11 = "swb11";
	/** Second clip of the two-part first-visit Rescue Site II-to-Snowboard speech. */
	static constexpr const char *kSpeechSwb11B = "swb11B";
	/** Random revisit speech format when leaving Rescue Site II for Snowboard. */
	static constexpr const char *kSpeechSwb21Format = "swb21.%d";
	/** First clip of the two-part first-visit Snowboard-to-Boolies speech. */
	static constexpr const char *kSpeechBlp11 = "blp11";
	/** Second clip of the two-part first-visit Snowboard-to-Boolies speech. */
	static constexpr const char *kSpeechBlp11B = "blp11B";
	/** Random revisit speech format when leaving Snowboard for Boolies. */
	static constexpr const char *kSpeechBlp21Format = "blp21.%d";
	/** First clip of the two-part first-visit Boolies-to-Booliewood speech. */
	static constexpr const char *kSpeechBlw11_1 = "blw11.1";
	/** Second clip of the two-part first-visit Boolies-to-Booliewood speech. */
	static constexpr const char *kSpeechBlw11_3 = "blw11.3";
	/** Ambient revisit speech format when leaving Boolies for Booliewood. */
	static constexpr const char *kSpeechBlw12Format = "blw12.%d";

	/** Start any due walkers and update all active party paths. */
	void walkZoombinis();
	/** Clear the paths and walking state assigned to the walking Zoombinis. */
	void cleanupPaths();
	/** Load and append one logical speech clip name. */
	void queueSpeech(const Common::String &name);
	/** Reproduce the source-world first-visit or revisit speech selection. */
	void queueTravelSpeech(PageId srcPage);
	/** Finish a completed clip and start the next queued speech clip. */
	void updateSpeechQueue();
	/** Return whether queued travel speech is playing or waiting to start. */
	bool hasPendingSpeech() const;
	/** Stop and release every travel speech handle retained by this page. */
	void cleanupSpeech();
	/** Return the original even-to-variant-two, odd-to-variant-one random choice. */
	int getRandomBinarySpeechVariant();
	/** Return the page entered after this transition. */
	PageId getPostTransitionPage() const;
	/** Commit the destination page after the last walker finishes. */
	void finishTransition();

	/** Background with route-specific overlays already applied. */
	ManagedSurface32 *_compositedBg = nullptr;

	/** Route shared as the template for each party member's path. */
	Common::Path _patPath;

	/** Index of the next party member waiting to start. */
	uint _nextWalkIndex = 0;
	/** Time at which the next walker may start. */
	uint32 _nextWalkTime = 0;
	/** Number of walkers that have reached the route endpoint. */
	uint _completedCount = 0;

	/** Page whose entry follows this route. */
	PageId _targetPageId = kPageNone;
	/** Whether the destination page has already been requested. */
	bool _transitionFinished = false;
	/** Serial destination-speech handles in original enqueue order. */
	Common::Array<int> _speechIds;
	/** Index of the next queued speech handle to start. */
	uint _nextSpeechIndex = 0;
	/** Index of the currently playing speech handle, or -1. */
	int _activeSpeechIndex = -1;

	/** Borrowed immutable sprite grid retained in the engine cache. */
	const ZoombiniAnimation *_zoombiniAnimation = nullptr;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_TRANSITION_MAPTRANS_H
