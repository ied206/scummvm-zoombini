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

#ifndef ZOOMBINI2_PAGES_TRANSITION_VIDEO_H
#define ZOOMBINI2_PAGES_TRANSITION_VIDEO_H

#include "common/path.h"
#include "common/rect.h"

#include "zoombini2/pages/transition_base.h"

namespace Video {
class VideoDecoder;
}

namespace Zoombini2 {

/**
 * Play a centered Bink video and transition to the next page.
 * Video audio follows the music mixer volume.
 */
class TransitionVideo : public TransitionBase {
public:
	/** Configure the video and the page dispatched after playback for @p pageId. */
	TransitionVideo(Zoombini2Engine *vm, int pageId);
	/** Stop and release the decoder and retained frame. */
	~TransitionVideo() override;

	/** Open the video, select its audio volume, and begin playback. */
	void init() override;
	/** Decode due frames and request the next page after playback. */
	void onUpdate() override;
	/** Draw the most recently decoded frame at its centered position. */
	void onRenderScene(ManagedSurface32 *screen) override;
	EventHandleResult onLButtonDown(const Common::Point &pos) override;
	EventHandleResult onKeyDown(const Common::KeyState &key, bool repeat) override;

	/** Request a black screen behind the centered video frame. */
	bool needsScreenClear() const override { return true; }

private:
	EventHandleResult skip();
	/** Select the full-size movie when present, otherwise use the half-size path. */
	Common::Path selectMoviePath(const char *fullSizePath, const char *halfSizePath) const;
	/** Original logical video resource name selected for the current page. */
	Common::Path _videoPath = Common::Path();
	/** Page requested when playback finishes or fails to start. */
	int _nextPageId = -1;
	/** Bink Video decoder. */
	Video::VideoDecoder *_decoder = nullptr;
	/** Whether decoder playback started successfully. */
	bool _started = false;
	/** Retain the most recently decoded frame. */
	Graphics::ManagedSurface *_lastFrame = nullptr;
	/** Signed screen coordinate used to center @ref TransitionVideo::_lastFrame. */
	Common::Point32 _framePos = Common::Point32();
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_TRANSITION_VIDEO_H
