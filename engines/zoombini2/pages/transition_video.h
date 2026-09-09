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
class VideoPage : public TransitionPage {
public:
	/** Configure the video resource and the page dispatched after playback. */
	VideoPage(Zoombini2Engine *engine, const Common::Path &videoPath, int nextPageId);
	/** Stop and release the decoder and retained frame. */
	~VideoPage() override;

	/** Open the video, select its audio volume, and begin playback. */
	void init() override;
	/** Decode due frames and request the next page after playback. */
	void update() override;
	/** Draw the most recently decoded frame at its centered position. */
	void draw(Graphics::ManagedSurface *screen) override;

	/** Request a black screen behind the centered video frame. */
	bool needsScreenClear() const override { return true; }

private:
	/** SearchMan-relative video path. */
	Common::Path _videoPath;
	/** Page requested when playback finishes or fails to start. */
	int _nextPageId;
	/** Owned video decoder. */
	Video::VideoDecoder *_decoder;
	/** Whether decoder playback started successfully. */
	bool _started;
	/** Owned copy of the most recently decoded frame. */
	Graphics::ManagedSurface *_lastFrame;
	/** Signed screen coordinate used to center @ref VideoPage::_lastFrame. */
	Common::Point32 _framePosition;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_TRANSITION_VIDEO_H
