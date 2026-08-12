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

#ifndef ZOOMBINI2_PAGES_VIDEO_H
#define ZOOMBINI2_PAGES_VIDEO_H

#include "common/path.h"

#include "zoombini2/pages/page.h"

namespace Video {
class VideoDecoder;
}

namespace Zoombini2 {

/**
 * Play a centered Bink video and transition to the next page.
 * Video audio follows the music mixer volume.
 */
class VideoPage : public Page {
public:
	VideoPage(Zoombini2Engine *engine, const Common::Path &videoPath, int nextPageId);
	~VideoPage() override;

	void init() override;
	void update() override;
	void draw(Graphics::ManagedSurface *screen) override;

	// Videos are centered in the 800x600 viewport on a black background.
	bool needsScreenClear() const override { return true; }

private:
	Common::Path _videoPath;
	int _nextPageId;
	Video::VideoDecoder *_decoder;
	bool _started;

	// Keep the last decoded frame visible until the next frame is ready.
	Graphics::ManagedSurface *_lastFrame;
	int _frameX, _frameY;
};

} // End of namespace Zoombini2

#endif // ZOOMBINI2_PAGES_VIDEO_H
