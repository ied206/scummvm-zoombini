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
 */

#ifndef TRINKET_RR1CLOUD9_RSS_SKY_RIDE_H
#define TRINKET_RR1CLOUD9_RSS_SKY_RIDE_H

#include "trinket/ompp/sound.h"
#include "trinket/shell/scene.h"

namespace Trinket {
namespace RR1Cloud9 {

class PageScene;

/** Shared slide/catapult transition, including its two simultaneous voices. */
class SkyRide {
public:
	/** Connect the transit presentation to its page host. */
	explicit SkyRide(PageScene &host) : _host(host) {}
	/** Stop transit speech and remove its animation. */
	~SkyRide();
	/** Load the selected sky-ride route and begin its presentation. */
	bool enter();
	/** Advance route animation and speech at a millisecond timestamp. */
	void update(uint32 time);
	/** Pause or resume transit playback and adjust deadlines by the elapsed interval. */
	void pause(bool paused, uint32 elapsed);
	/** Return the page name reached by the selected route. */
	Common::String destination() const;

private:
	/** World-state key for previous Location. */
	static constexpr const char *kPreviousLocationKey = "previousLocation";
	/** Authored data key for Anim ID.%d. */
	static constexpr const char *kAnimIDSignedIndexKey = "kAnimID.%d";
	/** Authored data key for Ambient Z. */
	static constexpr const char *kAmbientZKey = "kAmbientZ";
	/** Authored data key for Sam Spch ID. */
	static constexpr const char *kSamSpchIDKey = "kSamSpchID";
	/** Authored data key for RRSpch ID. */
	static constexpr const char *kRRSpchIDKey = "kRRSpchID";
	/** Format for script names or values: %s.%d. */
	static constexpr const char *kNameSignedIndexFormat = "%s.%d";
	/** Authored data key for Ambient Volume. */
	static constexpr const char *kAmbientVolumeKey = "kAmbientVolume";
	/** Authored data key for Previous Locations.%d. */
	static constexpr const char *kPreviousLocationsSignedIndexKey = "kPreviousLocations.%d";

	/** Page host supplying scene, resources and story state. */
	PageScene &_host;
	/** Sprite displaying the sky-ride route animation. */
	Shell::Sprite *_anim = nullptr;
	/** Sound handles for the route's spoken presentation. */
	OM::Sound *_voices[2] = {};
	/** Selected authored sky-ride route. */
	int _route = 1;
	/** Whether the route's initial animation and speech have begun. */
	bool _started = false;
	/** Whether the route is waiting for its current presentation step. */
	bool _waiting = false;
	/** Whether transit has finished and the destination can be entered. */
	bool _ready = false;
	/** Millisecond timestamp when the current route presentation began. */
	uint32 _start = 0;
};

} // End of namespace RR1Cloud9
} // End of namespace Trinket

#endif
