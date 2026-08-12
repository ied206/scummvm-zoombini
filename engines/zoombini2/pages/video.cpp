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

#include "common/debug.h"

#include "audio/mixer.h"

#include "video/bink_decoder.h"

#include "zoombini2/pages/video.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

VideoPage::VideoPage(Zoombini2Engine *engine, const Common::Path &videoPath, int nextPageId)
	: Page(engine), _videoPath(videoPath), _nextPageId(nextPageId),
	  _decoder(nullptr), _started(false),
	  _lastFrame(nullptr), _frameX(0), _frameY(0) {
}

VideoPage::~VideoPage() {
	if (_decoder) {
		_decoder->close();
		delete _decoder;
	}
	delete _lastFrame;
}

void VideoPage::init() {
	debug(1, "VideoPage::init — %s", _videoPath.toString('/').c_str());

	_decoder = new Video::BinkDecoder();
	_decoder->setSoundType(Audio::Mixer::kMusicSoundType);

	if (!_decoder->loadFile(_videoPath)) {
		warning("VideoPage: Failed to load %s", _videoPath.toString('/').c_str());
		// Skip to next page if video is missing
		_engine->requestPageChange(_nextPageId);
		return;
	}

	_decoder->start();
	_started = true;
}

void VideoPage::update() {
	if (!_started || !_decoder)
		return;

	// Allow skip on click or key
	if (_engine->isMouseClicked() || _engine->getLastKeyPressed()) {
		_engine->requestPageChange(_nextPageId);
		return;
	}

	if (_decoder->endOfVideo()) {
		_engine->requestPageChange(_nextPageId);
		return;
	}
}

void VideoPage::draw(Graphics::ManagedSurface *screen) {
	if (!_started || !_decoder)
		return;

	if (_decoder->needsUpdate()) {
		const Graphics::Surface *frame = _decoder->decodeNextFrame();
		if (frame) {
			_frameX = (kScreenWidth - frame->w) / 2;
			_frameY = (kScreenHeight - frame->h) / 2;

			// Cache the decoded frame in screen format
			if (!_lastFrame || _lastFrame->w != frame->w || _lastFrame->h != frame->h) {
				delete _lastFrame;
				_lastFrame = new Graphics::ManagedSurface(frame->w, frame->h, screen->format);
			}
			_lastFrame->blitFrom(*frame);
		}
	}

	if (_lastFrame)
		screen->blitFrom(*_lastFrame, Common::Point(_frameX, _frameY));
}

} // End of namespace Zoombini2
