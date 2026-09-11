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

#include "zoombini2/pages/transition_video.h"
#include "zoombini2/zoombini2.h"

namespace Zoombini2 {

TransitionVideo::TransitionVideo(Zoombini2Engine *vm, int pageId)
	: TransitionBase(vm) {
	_pageId = pageId;

	switch (pageId) {
	case kPageLogoTLC:
		_videoPath = selectMoviePath("movies/tlclogo.bik", "movies/tlclogo50%.bik");
		_nextPageId = kPageLogoPolygon;
		break;
	case kPageLogoPolygon:
		_videoPath = Common::Path("movies/logopoly.bik");
		_nextPageId = kPageTitleScreen;
		break;
	case kPageCutsceneFirst:
		_videoPath = selectMoviePath("movies/zoom_movie1_100%.bik", "movies/zoom_movie1_50%.bik");
		_nextPageId = kPageZombiniville;
		break;
	case kPageCutsceneSecond:
		_videoPath = selectMoviePath("movies/zoom_movie2_100%.bik", "movies/zoom_movie2_50%.bik");
		_nextPageId = kPageRescue1;
		break;
	case kPageCutsceneThird:
		_videoPath = selectMoviePath("movies/zoom_movie3_100%.bik", "movies/zoom_movie3_50%.bik");
		_nextPageId = kPageRescue2;
		break;
	case kPageLogoArisuMedia:
		_videoPath = Common::Path("movies/arisu.bik");
		_nextPageId = kPageLogoTLC;
		break;
	default:
		warning("TransitionVideo: Unknown page %d", pageId);
		break;
	}
}

Common::Path TransitionVideo::selectMoviePath(const char *fullSizePath, const char *halfSizePath) const {
	if (_vm->hasResource(fullSizePath))
		return Common::Path(fullSizePath);

	return Common::Path(halfSizePath);
}

TransitionVideo::~TransitionVideo() {
	if (_decoder) {
		_decoder->close();
		delete _decoder;
	}
	delete _lastFrame;
}

void TransitionVideo::init() {
	debug(1, "TransitionVideo::init - %s", _videoPath.toString('/').c_str());

	_decoder = new Video::BinkDecoder();
	_decoder->setSoundType(Audio::Mixer::kMusicSoundType);

	Common::SeekableReadStream *stream = _vm->openResourceFile(_videoPath.toString('/'));
	if (!stream || !_decoder->loadStream(stream)) {
		delete stream;
		warning("TransitionVideo: Failed to load %s", _videoPath.toString('/').c_str());
		// Skip to next page if video is missing
		_vm->requestPageChange(_nextPageId);
		return;
	}

	_decoder->start();
	_started = true;
}

void TransitionVideo::onUpdate() {
	if (!_started || !_decoder)
		return;

	if (_decoder->endOfVideo()) {
		_vm->requestPageChange(_nextPageId);
		return;
	}

	if (_decoder->needsUpdate()) {
		const Graphics::Surface *frame = _decoder->decodeNextFrame();
		if (frame) {
			_framePos = Common::Point32((kScreenWidth - frame->w) / 2, (kScreenHeight - frame->h) / 2);

			// Cache the decoded frame in screen format
			if (!_lastFrame || _lastFrame->w != frame->w || _lastFrame->h != frame->h) {
				delete _lastFrame;
				_lastFrame = new Graphics::ManagedSurface(frame->w, frame->h, _vm->getScreen()->format);
			}
			_lastFrame->blitFrom(*frame);
		}
	}
}

void TransitionVideo::onRenderScene(ManagedSurface32 *screen) {
	if (_lastFrame)
		screen->blitFrom(*_lastFrame, _framePos);
}

EventHandleResult TransitionVideo::skip() {
	if (!_started)
		return EventHandleResult::kPassthrough;
	_vm->requestPageChange(_nextPageId);
	return EventHandleResult::kConsumed;
}

EventHandleResult TransitionVideo::onLButtonDown(const Common::Point &pos) {
	(void)pos;
	return skip();
}

EventHandleResult TransitionVideo::onKeyDown(const Common::KeyState &key, bool repeat) {
	(void)key;
	(void)repeat;
	return skip();
}

} // End of namespace Zoombini2
